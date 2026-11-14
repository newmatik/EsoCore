/**
 * @file protocol_frame.c
 * @brief EsoCore protocol frame codec
 *
 * Serialises and parses the RS-485 frames of the EsoCore protocol (layout in
 * protocol.h). Used by both ends of the sensor bus: the protocol layer of the
 * sensor modules (protocol.c) and the Edge's sensor bus manager (sensor_bus.c).
 * No hardware access, so it is unit-tested on the host (tests/host/test_protocol.c).
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "protocol.h"

#include <string.h>

static uint16_t crc16_update(uint16_t crc, const uint8_t *data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1U) {
                crc = (uint16_t)((crc >> 1) ^ 0xA001U);
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

static void put_u16(uint8_t *buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value & 0xFFU);
    buffer[1] = (uint8_t)(value >> 8);
}

static uint16_t get_u16(const uint8_t *buffer) {
    return (uint16_t)(buffer[0] | (uint16_t)(buffer[1] << 8));
}

static void put_u32(uint8_t *buffer, uint32_t value) {
    for (uint8_t i = 0; i < 4; i++) {
        buffer[i] = (uint8_t)(value >> (8U * i));
    }
}

static uint32_t get_u32(const uint8_t *buffer) {
    return (uint32_t)buffer[0] | ((uint32_t)buffer[1] << 8) | ((uint32_t)buffer[2] << 16) |
           ((uint32_t)buffer[3] << 24);
}

static void encode_header(const esocore_message_header_t *header,
                          uint8_t out[ESOCORE_PROTOCOL_HEADER_SIZE]) {
    out[0] = header->start_byte;
    out[1] = header->source_address;
    out[2] = header->destination_address;
    out[3] = header->message_type;
    out[4] = header->sequence_number;
    out[5] = header->flags;
    put_u16(&out[6], header->payload_length);
}

/**
 * @brief CRC-16/MODBUS (polynomial 0xA001 reflected, initial value 0xFFFF)
 */
uint16_t esocore_protocol_calculate_crc16(const uint8_t *data, uint16_t length) {
    return crc16_update(0xFFFFU, data, length);
}

uint16_t esocore_protocol_frame_crc(const esocore_message_t *message) {
    if (!message || message->header.payload_length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE) {
        return 0;
    }

    uint8_t header[ESOCORE_PROTOCOL_HEADER_SIZE];
    encode_header(&message->header, header);

    uint16_t crc = crc16_update(0xFFFFU, header, sizeof(header));
    return crc16_update(crc, message->payload, message->header.payload_length);
}

uint16_t esocore_protocol_encode_frame(const esocore_message_t *message,
                                       uint8_t *buffer, uint16_t buffer_size) {
    if (!message || !buffer || message->header.payload_length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE) {
        return 0;
    }

    uint16_t payload_length = message->header.payload_length;
    uint32_t frame_length = (uint32_t)ESOCORE_PROTOCOL_HEADER_SIZE + payload_length +
                            ESOCORE_PROTOCOL_CRC_SIZE;
    if (frame_length > buffer_size) {
        return 0;
    }

    encode_header(&message->header, buffer);
    buffer[0] = ESOCORE_PROTOCOL_START_BYTE;
    memcpy(&buffer[ESOCORE_PROTOCOL_HEADER_SIZE], message->payload, payload_length);

    uint16_t crc = esocore_protocol_calculate_crc16(
        buffer, (uint16_t)(ESOCORE_PROTOCOL_HEADER_SIZE + payload_length));
    put_u16(&buffer[ESOCORE_PROTOCOL_HEADER_SIZE + payload_length], crc);

    return (uint16_t)frame_length;
}

esocore_frame_status_t esocore_protocol_decode_frame(const uint8_t *buffer, uint32_t length,
                                                     esocore_message_t *message,
                                                     uint32_t *consumed) {
    uint32_t skip = 0;
    esocore_frame_status_t status = ESOCORE_FRAME_INCOMPLETE;

    if (!buffer || !message || length == 0) {
        /* Nothing to parse */
    } else if (buffer[0] != ESOCORE_PROTOCOL_START_BYTE) {
        status = ESOCORE_FRAME_NO_START;
        skip = 1;
    } else if (length >= ESOCORE_PROTOCOL_HEADER_SIZE) {
        uint16_t payload_length = get_u16(&buffer[6]);

        if (payload_length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE) {
            /* Not a frame start after all: resynchronise on the next byte */
            status = ESOCORE_FRAME_INVALID;
            skip = 1;
        } else {
            uint32_t frame_length = (uint32_t)ESOCORE_PROTOCOL_HEADER_SIZE + payload_length +
                                    ESOCORE_PROTOCOL_CRC_SIZE;

            if (length >= frame_length) {
                message->header.start_byte = buffer[0];
                message->header.source_address = buffer[1];
                message->header.destination_address = buffer[2];
                message->header.message_type = buffer[3];
                message->header.sequence_number = buffer[4];
                message->header.flags = buffer[5];
                message->header.payload_length = payload_length;
                memcpy(message->payload, &buffer[ESOCORE_PROTOCOL_HEADER_SIZE], payload_length);
                message->crc = get_u16(&buffer[ESOCORE_PROTOCOL_HEADER_SIZE + payload_length]);

                uint16_t crc = esocore_protocol_calculate_crc16(
                    buffer, (uint16_t)(ESOCORE_PROTOCOL_HEADER_SIZE + payload_length));
                if (crc == message->crc) {
                    status = ESOCORE_FRAME_OK;
                    skip = frame_length;
                } else {
                    /* The length may be the damaged byte: resynchronise on the next byte
                     * rather than dropping a following frame along with this one */
                    status = ESOCORE_FRAME_CRC_ERROR;
                    skip = 1;
                }
            }
        }
    }

    if (consumed) {
        *consumed = skip;
    }
    return status;
}

uint16_t esocore_protocol_encode_device_info(const esocore_device_info_t *device_info,
                                             uint8_t *buffer) {
    if (!device_info || !buffer) {
        return 0;
    }

    buffer[0] = device_info->device_type;
    buffer[1] = device_info->firmware_version_major;
    buffer[2] = device_info->firmware_version_minor;
    buffer[3] = device_info->hardware_version;
    put_u16(&buffer[4], device_info->serial_number);
    buffer[6] = device_info->capabilities;
    buffer[7] = device_info->status_flags;
    put_u32(&buffer[8], device_info->uptime_seconds);

    return ESOCORE_DEVICE_INFO_WIRE_SIZE;
}

bool esocore_protocol_decode_device_info(const uint8_t *payload, uint16_t length,
                                         uint8_t address, esocore_device_info_t *device_info) {
    if (!payload || !device_info || length < ESOCORE_DEVICE_INFO_WIRE_SIZE) {
        return false;
    }

    memset(device_info, 0, sizeof(*device_info));
    device_info->address = address;
    device_info->device_type = payload[0];
    device_info->firmware_version_major = payload[1];
    device_info->firmware_version_minor = payload[2];
    device_info->hardware_version = payload[3];
    device_info->serial_number = get_u16(&payload[4]);
    device_info->capabilities = payload[6];
    device_info->status_flags = payload[7];
    device_info->uptime_seconds = get_u32(&payload[8]);

    return true;
}
