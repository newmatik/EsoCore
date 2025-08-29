/**
 * @file http_client.c
 * @brief HTTP Client Implementation
 *
 * This file contains the implementation of the HTTP client for the EsoCore Edge device,
 * providing REST API communication with TLS support, authentication, and error handling.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "http_client.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

static http_client_config_t http_config;
static bool http_initialized = false;

/* HTTP request/response buffers */
static uint8_t http_request_buffer[HTTP_MAX_BODY_SIZE];
static uint8_t http_response_buffer[HTTP_MAX_BODY_SIZE];

/* Statistics */
static uint32_t requests_sent = 0;
static uint32_t responses_received = 0;
static uint32_t errors_count = 0;

/* TLS/SSL context - would be hardware dependent */
static void *tls_context = NULL;

/* ============================================================================
 * HTTP Protocol Implementation
 * ============================================================================ */

/**
 * @brief Initialize TLS/SSL context
 *
 * @return true if TLS initialization successful, false otherwise
 */
static bool http_tls_init(void) {
    /* TODO: Initialize TLS context */
    /* This would typically involve:
     * - Loading CA certificates
     * - Setting up client certificates if needed
     * - Configuring cipher suites
     * - Setting up secure renegotiation
     */
    return true;
}

/**
 * @brief Connect to HTTP server
 *
 * @param server_url Server URL
 * @param port Server port
 * @param use_tls Whether to use TLS
 * @return Socket handle or NULL on failure
 */
static void *http_connect(const char *server_url, uint16_t port, bool use_tls) {
    /* TODO: Implement HTTP connection */
    /* This would typically involve:
     * - DNS resolution
     * - Socket creation
     * - TLS handshake if required
     * - Connection establishment
     */
    return (void *)1; /* Placeholder */
}

/**
 * @brief Send HTTP request
 *
 * @param socket Socket handle
 * @param request HTTP request data
 * @param request_length Request length
 * @return true if send successful, false otherwise
 */
static bool http_send_request(void *socket, const uint8_t *request, uint32_t request_length) {
    /* TODO: Send HTTP request over socket */
    /* This would typically involve:
     * - Writing request to socket
     * - Handling TLS encryption if needed
     * - Error handling and timeouts
     */
    return true;
}

/**
 * @brief Receive HTTP response
 *
 * @param socket Socket handle
 * @param response_buffer Response buffer
 * @param buffer_size Buffer size
 * @param response_length Pointer to store response length
 * @param timeout_ms Receive timeout
 * @return true if receive successful, false otherwise
 */
static bool http_receive_response(void *socket, uint8_t *response_buffer, uint32_t buffer_size,
                                 uint32_t *response_length, uint32_t timeout_ms) {
    /* TODO: Receive HTTP response */
    /* This would typically involve:
     * - Reading from socket with timeout
     * - Handling TLS decryption if needed
     * - Parsing response headers and body
     */
    return true;
}

/**
 * @brief Close HTTP connection
 *
 * @param socket Socket handle
 * @return true if close successful, false otherwise
 */
static bool http_close(void *socket) {
    /* TODO: Close HTTP connection */
    /* This would typically involve:
     * - TLS shutdown if needed
     * - Socket closure
     * - Resource cleanup
     */
    return true;
}

/**
 * @brief Build HTTP request
 *
 * @param request HTTP request structure
 * @param buffer Request buffer
 * @param buffer_size Buffer size
 * @param request_length Pointer to store request length
 * @return true if request built successfully, false otherwise
 */
static bool http_build_request(const http_request_t *request, uint8_t *buffer, uint32_t buffer_size,
                              uint32_t *request_length) {
    if (!request || !buffer || !request_length) {
        return false;
    }

    char *buf_ptr = (char *)buffer;
    uint32_t remaining = buffer_size;

    /* HTTP method and URL */
    int len = snprintf(buf_ptr, remaining, "%s %s HTTP/1.1\r\n",
                      request->method == HTTP_METHOD_GET ? "GET" :
                      request->method == HTTP_METHOD_POST ? "POST" :
                      request->method == HTTP_METHOD_PUT ? "PUT" :
                      request->method == HTTP_METHOD_DELETE ? "DELETE" :
                      request->method == HTTP_METHOD_PATCH ? "PATCH" : "GET",
                      request->url);

    if (len < 0 || len >= (int)remaining) {
        return false;
    }
    buf_ptr += len;
    remaining -= len;

    /* Host header */
    len = snprintf(buf_ptr, remaining, "Host: %s\r\n", http_config.server_url);
    if (len < 0 || len >= (int)remaining) {
        return false;
    }
    buf_ptr += len;
    remaining -= len;

    /* Content-Type header */
    if (request->content_type[0] != '\0') {
        len = snprintf(buf_ptr, remaining, "Content-Type: %s\r\n", request->content_type);
        if (len < 0 || len >= (int)remaining) {
            return false;
        }
        buf_ptr += len;
        remaining -= len;
    }

    /* Content-Length header */
    if (request->body_length > 0) {
        len = snprintf(buf_ptr, remaining, "Content-Length: %u\r\n", request->body_length);
        if (len < 0 || len >= (int)remaining) {
            return false;
        }
        buf_ptr += len;
        remaining -= len;
    }

    /* Custom headers */
    for (uint8_t i = 0; i < request->num_headers; i++) {
        len = snprintf(buf_ptr, remaining, "%s: %s\r\n",
                      request->headers[i].name, request->headers[i].value);
        if (len < 0 || len >= (int)remaining) {
            return false;
        }
        buf_ptr += len;
        remaining -= len;
    }

    /* Authorization header */
    if (http_config.api_key[0] != '\0') {
        len = snprintf(buf_ptr, remaining, "Authorization: Bearer %s\r\n", http_config.api_key);
        if (len < 0 || len >= (int)remaining) {
            return false;
        }
        buf_ptr += len;
        remaining -= len;
    }

    /* End of headers */
    len = snprintf(buf_ptr, remaining, "\r\n");
    if (len < 0 || len >= (int)remaining) {
        return false;
    }
    buf_ptr += len;
    remaining -= len;

    /* Request body */
    if (request->body && request->body_length > 0) {
        if (request->body_length >= remaining) {
            return false;
        }
        memcpy(buf_ptr, request->body, request->body_length);
        buf_ptr += request->body_length;
        remaining -= request->body_length;
    }

    *request_length = buffer_size - remaining;
    return true;
}

/**
 * @brief Parse HTTP response
 *
 * @param response_data Raw response data
 * @param data_length Response data length
 * @param response HTTP response structure to fill
 * @return true if response parsed successfully, false otherwise
 */
static bool http_parse_response(const uint8_t *response_data, uint32_t data_length,
                               http_response_t *response) {
    if (!response_data || !response || data_length == 0) {
        return false;
    }

    const char *data = (const char *)response_data;
    const char *line_end;
    uint32_t consumed = 0;

    /* Parse status line */
    line_end = strstr(data, "\r\n");
    if (!line_end) {
        return false;
    }

    /* Simple status line parsing - HTTP/1.1 200 OK */
    if (sscanf(data, "HTTP/1.1 %d", &response->status_code) != 1) {
        return false;
    }

    /* Find status message */
    const char *status_start = strchr(data, ' ');
    if (status_start) {
        status_start = strchr(status_start + 1, ' ');
        if (status_start) {
            const char *status_end = line_end;
            uint32_t status_len = status_end - (status_start + 1);
            if (status_len < sizeof(response->status_message)) {
                memcpy(response->status_message, status_start + 1, status_len);
                response->status_message[status_len] = '\0';
            }
        }
    }

    consumed = (line_end - data) + 2; /* +2 for \r\n */
    data += consumed;

    /* Parse headers */
    response->num_headers = 0;
    while (response->num_headers < HTTP_MAX_HEADER_COUNT) {
        line_end = strstr(data, "\r\n");
        if (!line_end || line_end == data) {
            break; /* End of headers */
        }

        char header_line[HTTP_MAX_HEADER_LENGTH * 2];
        uint32_t line_len = line_end - data;
        if (line_len >= sizeof(header_line)) {
            return false;
        }

        memcpy(header_line, data, line_len);
        header_line[line_len] = '\0';

        /* Parse header name and value */
        char *colon_pos = strchr(header_line, ':');
        if (colon_pos) {
            *colon_pos = '\0';
            char *value_start = colon_pos + 1;

            /* Skip leading whitespace */
            while (*value_start == ' ') {
                value_start++;
            }

            /* Copy header name */
            if (strlen(header_line) < HTTP_MAX_HEADER_LENGTH) {
                strcpy(response->headers[response->num_headers].name, header_line);
            }

            /* Copy header value */
            uint32_t value_len = strlen(value_start);
            if (value_len < HTTP_MAX_HEADER_LENGTH) {
                strcpy(response->headers[response->num_headers].value, value_start);
            }

            response->num_headers++;
        }

        consumed = (line_end - data) + 2;
        data += consumed;
    }

    /* Skip empty line after headers */
    if (data[0] == '\r' && data[1] == '\n') {
        data += 2;
    }

    /* Remaining data is the response body */
    uint32_t body_length = data_length - (data - (const char *)response_data);
    if (body_length > 0 && body_length <= HTTP_MAX_BODY_SIZE) {
        response->body = http_response_buffer;
        memcpy(response->body, data, body_length);
        response->body_length = body_length;
    } else {
        response->body = NULL;
        response->body_length = 0;
    }

    return true;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize HTTP client
 */
bool http_client_init(const http_client_config_t *config) {
    if (!config || http_initialized) {
        return false;
    }

    memcpy(&http_config, config, sizeof(http_client_config_t));

    /* Initialize TLS if required */
    if (http_config.use_https) {
        if (!http_tls_init()) {
            return false;
        }
    }

    /* Reset statistics */
    requests_sent = 0;
    responses_received = 0;
    errors_count = 0;

    http_initialized = true;
    return true;
}

/**
 * @brief Deinitialize HTTP client
 */
bool http_client_deinit(void) {
    if (!http_initialized) {
        return false;
    }

    /* Clean up TLS context */
    if (tls_context) {
        /* TODO: Free TLS context */
        tls_context = NULL;
    }

    http_initialized = false;
    return true;
}

/**
 * @brief Send HTTP request
 */
bool http_client_send_request(const http_request_t *request, http_response_t *response) {
    if (!http_initialized || !request || !response) {
        return false;
    }

    /* Build HTTP request */
    uint32_t request_length;
    if (!http_build_request(request, http_request_buffer, sizeof(http_request_buffer), &request_length)) {
        errors_count++;
        return false;
    }

    /* Connect to server */
    void *socket = http_connect(http_config.server_url, http_config.server_port, http_config.use_https);
    if (!socket) {
        errors_count++;
        return false;
    }

    /* Send request */
    if (!http_send_request(socket, http_request_buffer, request_length)) {
        http_close(socket);
        errors_count++;
        return false;
    }

    requests_sent++;

    /* Receive response */
    uint32_t response_length;
    if (!http_receive_response(socket, http_response_buffer, sizeof(http_response_buffer),
                              &response_length, http_config.response_timeout_ms)) {
        http_close(socket);
        errors_count++;
        return false;
    }

    /* Close connection */
    http_close(socket);

    /* Parse response */
    if (!http_parse_response(http_response_buffer, response_length, response)) {
        errors_count++;
        return false;
    }

    responses_received++;
    return true;
}

/**
 * @brief Send GET request
 */
bool http_client_get(const char *url, http_response_t *response) {
    http_request_t request = {
        .method = HTTP_METHOD_GET,
        .url = url,
        .num_headers = 0,
        .body = NULL,
        .body_length = 0,
        .content_type = ""
    };

    return http_client_send_request(&request, response);
}

/**
 * @brief Send POST request
 */
bool http_client_post(const char *url, const uint8_t *body, uint16_t body_length,
                     const char *content_type, http_response_t *response) {
    http_request_t request = {
        .method = HTTP_METHOD_POST,
        .url = url,
        .num_headers = 0,
        .body = body,
        .body_length = body_length
    };

    if (content_type) {
        strcpy(request.content_type, content_type);
    }

    return http_client_send_request(&request, response);
}

/**
 * @brief Send PUT request
 */
bool http_client_put(const char *url, const uint8_t *body, uint16_t body_length,
                    const char *content_type, http_response_t *response) {
    http_request_t request = {
        .method = HTTP_METHOD_PUT,
        .url = url,
        .num_headers = 0,
        .body = body,
        .body_length = body_length
    };

    if (content_type) {
        strcpy(request.content_type, content_type);
    }

    return http_client_send_request(&request, response);
}

/**
 * @brief Add custom header to request
 */
bool http_client_add_header(http_request_t *request, const char *name, const char *value) {
    if (!request || !name || !value || request->num_headers >= HTTP_MAX_HEADER_COUNT) {
        return false;
    }

    if (strlen(name) >= HTTP_MAX_HEADER_LENGTH || strlen(value) >= HTTP_MAX_HEADER_LENGTH) {
        return false;
    }

    strcpy(request->headers[request->num_headers].name, name);
    strcpy(request->headers[request->num_headers].value, value);
    request->num_headers++;

    return true;
}

/**
 * @brief Set authorization header (API key)
 */
bool http_client_set_authorization(http_request_t *request) {
    if (!request || http_config.api_key[0] == '\0') {
        return false;
    }

    return http_client_add_header(request, "Authorization", http_config.api_key);
}

/**
 * @brief Update HTTP client configuration
 */
bool http_client_update_config(const http_client_config_t *config) {
    if (!config) {
        return false;
    }

    memcpy(&http_config, config, sizeof(http_client_config_t));

    /* Reinitialize TLS if HTTPS settings changed */
    if (http_config.use_https && !tls_context) {
        return http_tls_init();
    }

    return true;
}

/**
 * @brief Get current HTTP client configuration
 */
bool http_client_get_config(http_client_config_t *config) {
    if (!config || !http_initialized) {
        return false;
    }

    memcpy(config, &http_config, sizeof(http_client_config_t));
    return true;
}

/* ============================================================================
 * EsoCore API Functions
 * ============================================================================ */

/**
 * @brief Send authentication handshake to server
 */
bool esocore_api_auth_handshake(const char *device_id, http_response_t *response) {
    char json_payload[256];
    int len = snprintf(json_payload, sizeof(json_payload),
                      "{\"device_id\":\"%s\",\"timestamp\":%lu}",
                      device_id, 0); /* TODO: Add actual timestamp */

    if (len < 0 || len >= (int)sizeof(json_payload)) {
        return false;
    }

    return http_client_post(API_ENDPOINT_AUTH_HANDSHAKE,
                           (const uint8_t *)json_payload, len,
                           "application/json", response);
}

/**
 * @brief Send telemetry data batch to server
 */
bool esocore_api_send_telemetry(const uint8_t *telemetry_data, uint16_t data_length,
                               http_response_t *response) {
    return http_client_post(API_ENDPOINT_TELEMETRY_BATCH,
                           telemetry_data, data_length,
                           "application/json", response);
}

/**
 * @brief Retrieve device configuration from server
 */
bool esocore_api_get_config(uint8_t *config_data, uint16_t max_length,
                           http_response_t *response) {
    if (!http_client_get(API_ENDPOINT_CONFIG, response)) {
        return false;
    }

    /* Copy response body to config_data if successful */
    if (response->status_code == HTTP_STATUS_OK &&
        response->body && response->body_length <= max_length) {
        memcpy(config_data, response->body, response->body_length);
        return true;
    }

    return false;
}

/**
 * @brief Check for firmware updates
 */
bool esocore_api_check_ota(const char *current_version, http_response_t *response) {
    char url[256];
    int len = snprintf(url, sizeof(url), "%s?version=%s", API_ENDPOINT_OTA_CHECK, current_version);

    if (len < 0 || len >= (int)sizeof(url)) {
        return false;
    }

    return http_client_get(url, response);
}

/**
 * @brief Report OTA update status
 */
bool esocore_api_report_ota(const char *status, const char *error_message,
                           http_response_t *response) {
    char json_payload[512];
    int len;

    if (error_message) {
        len = snprintf(json_payload, sizeof(json_payload),
                      "{\"status\":\"%s\",\"error\":\"%s\",\"timestamp\":%lu}",
                      status, error_message, 0); /* TODO: Add actual timestamp */
    } else {
        len = snprintf(json_payload, sizeof(json_payload),
                      "{\"status\":\"%s\",\"timestamp\":%lu}",
                      status, 0); /* TODO: Add actual timestamp */
    }

    if (len < 0 || len >= (int)sizeof(json_payload)) {
        return false;
    }

    return http_client_post(API_ENDPOINT_OTA_REPORT,
                           (const uint8_t *)json_payload, len,
                           "application/json", response);
}

/**
 * @brief Send heartbeat to server
 */
bool esocore_api_send_heartbeat(const char *status, http_response_t *response) {
    char json_payload[512];
    int len = snprintf(json_payload, sizeof(json_payload),
                      "{\"status\":\"%s\",\"timestamp\":%lu}",
                      status, 0); /* TODO: Add actual timestamp */

    if (len < 0 || len >= (int)sizeof(json_payload)) {
        return false;
    }

    return http_client_post(API_ENDPOINT_heartbeat,
                           (const uint8_t *)json_payload, len,
                           "application/json", response);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Create JSON payload for sensor data
 */
uint16_t http_create_sensor_json(const uint8_t *sensor_data, uint16_t data_length,
                                char *json_buffer, uint16_t max_length) {
    /* TODO: Implement JSON creation from sensor data */
    /* This would typically involve:
     * - Parsing sensor data format
     * - Creating JSON structure
     * - Adding timestamps and metadata
     */
    return 0;
}

/**
 * @brief Parse JSON response from server
 */
bool http_parse_json_value(const char *json_data, uint16_t data_length,
                          const char *key, char *value_buffer, uint16_t max_length) {
    /* TODO: Implement JSON parsing */
    /* This would typically involve:
     * - Finding the key in JSON
     * - Extracting the value
     * - Handling different data types
     */
    return false;
}

/**
 * @brief URL encode a string
 */
uint16_t http_url_encode(const char *input, char *output, uint16_t max_length) {
    /* TODO: Implement URL encoding */
    /* This would typically involve:
     * - Converting special characters to %XX format
     * - Handling UTF-8 encoding
     */
    return 0;
}

/**
 * @brief Get HTTP status code description
 */
const char *http_get_status_description(http_status_code_t status_code) {
    switch (status_code) {
        case HTTP_STATUS_OK: return "OK";
        case HTTP_STATUS_CREATED: return "Created";
        case HTTP_STATUS_NO_CONTENT: return "No Content";
        case HTTP_STATUS_BAD_REQUEST: return "Bad Request";
        case HTTP_STATUS_UNAUTHORIZED: return "Unauthorized";
        case HTTP_STATUS_FORBIDDEN: return "Forbidden";
        case HTTP_STATUS_NOT_FOUND: return "Not Found";
        case HTTP_STATUS_METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case HTTP_STATUS_CONFLICT: return "Conflict";
        case HTTP_STATUS_INTERNAL_ERROR: return "Internal Server Error";
        case HTTP_STATUS_BAD_GATEWAY: return "Bad Gateway";
        case HTTP_STATUS_SERVICE_UNAVAILABLE: return "Service Unavailable";
        default: return "Unknown";
    }
}

/**
 * @brief Check if HTTP status indicates success
 */
bool http_is_success_status(http_status_code_t status_code) {
    return (status_code >= 200 && status_code < 300);
}

/**
 * @brief Get HTTP client statistics
 */
bool http_client_get_statistics(uint32_t *requests_sent_out, uint32_t *responses_received_out,
                               uint32_t *errors_count_out) {
    if (!http_initialized) {
        return false;
    }

    if (requests_sent_out) {
        *requests_sent_out = requests_sent;
    }
    if (responses_received_out) {
        *responses_received_out = responses_received;
    }
    if (errors_count_out) {
        *errors_count_out = errors_count;
    }

    return true;
}
