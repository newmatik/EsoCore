/**
 * @file hal_dma.c
 * @brief DMA HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_dma.h"

/* DMA Stream CR bits */
#define DMA_CR_EN       (1UL << 0)
#define DMA_CR_TCIE     (1UL << 4)
#define DMA_CR_HTIE     (1UL << 3)
#define DMA_CR_TEIE     (1UL << 2)
#define DMA_CR_CIRC     (1UL << 8)
#define DMA_CR_DBM      (1UL << 18)
#define DMA_CR_MINC     (1UL << 10)
#define DMA_CR_PINC     (1UL << 9)
#define DMA_CR_CT       (1UL << 19)

/* DMAMUX register base */
#define DMAMUX1_BASE_ADDR   0x40020800UL
#define DMAMUX1_C0CR    (*(volatile uint32_t *)(DMAMUX1_BASE_ADDR + 0x00))

/* Get DMAMUX channel register for a given DMA stream */
static volatile uint32_t *get_dmamux_cr(DMA_Stream_TypeDef *stream)
{
    /* DMA1 streams 0-7 map to DMAMUX1 channels 0-7 */
    /* DMA2 streams 0-7 map to DMAMUX1 channels 8-15 */
    uint32_t offset;
    if ((uint32_t)stream >= DMA2_BASE) {
        offset = ((uint32_t)stream - DMA2_BASE - 0x10) / 0x18;
        offset += 8;
    } else {
        offset = ((uint32_t)stream - DMA1_BASE - 0x10) / 0x18;
    }
    return (volatile uint32_t *)(DMAMUX1_BASE_ADDR + offset * 4);
}

bool hal_dma_init(hal_dma_handle_t *handle, const hal_dma_config_t *config)
{
    if (!handle || !config || !config->stream) return false;

    DMA_Stream_TypeDef *stream = config->stream;
    handle->stream = stream;

    /* Disable stream first */
    stream->CR &= ~DMA_CR_EN;
    while (stream->CR & DMA_CR_EN) { }

    /* Configure DMAMUX request */
    volatile uint32_t *mux_cr = get_dmamux_cr(stream);
    *mux_cr = (uint32_t)config->dmamux_request;

    /* Build CR register value */
    uint32_t cr = 0;

    /* Direction */
    cr |= ((uint32_t)config->direction << 6);

    /* Data sizes */
    cr |= ((uint32_t)config->periph_size << 11);
    cr |= ((uint32_t)config->mem_size << 13);

    /* Increments */
    if (config->periph_inc) cr |= DMA_CR_PINC;
    if (config->mem_inc)    cr |= DMA_CR_MINC;

    /* Circular mode */
    if (config->circular) cr |= DMA_CR_CIRC;

    /* Double buffer mode */
    if (config->double_buffer) cr |= DMA_CR_DBM;

    /* Priority */
    cr |= ((uint32_t)config->priority << 16);

    /* Enable transfer complete interrupt */
    cr |= DMA_CR_TCIE;

    stream->CR = cr;

    handle->initialized = true;
    return true;
}

void hal_dma_deinit(hal_dma_handle_t *handle)
{
    if (!handle || !handle->stream) return;
    handle->stream->CR &= ~DMA_CR_EN;
    handle->initialized = false;
}

bool hal_dma_start(hal_dma_handle_t *handle,
                   uint32_t src_addr, uint32_t dst_addr,
                   uint16_t length)
{
    if (!handle || !handle->initialized) return false;

    DMA_Stream_TypeDef *stream = handle->stream;

    /* Disable stream */
    stream->CR &= ~DMA_CR_EN;
    while (stream->CR & DMA_CR_EN) { }

    /* Configure addresses based on direction */
    uint32_t dir = (stream->CR >> 6) & 0x3;
    if (dir == 0) {  /* Periph to mem */
        stream->PAR  = src_addr;
        stream->M0AR = dst_addr;
    } else {  /* Mem to periph */
        stream->PAR  = dst_addr;
        stream->M0AR = src_addr;
    }

    stream->NDTR = length;

    /* Enable stream */
    stream->CR |= DMA_CR_EN;

    return true;
}

bool hal_dma_start_double_buffer(hal_dma_handle_t *handle,
                                 uint32_t periph_addr,
                                 uint32_t mem0_addr, uint32_t mem1_addr,
                                 uint16_t length)
{
    if (!handle || !handle->initialized) return false;

    DMA_Stream_TypeDef *stream = handle->stream;

    /* Disable stream */
    stream->CR &= ~DMA_CR_EN;
    while (stream->CR & DMA_CR_EN) { }

    stream->PAR  = periph_addr;
    stream->M0AR = mem0_addr;
    stream->M1AR = mem1_addr;
    stream->NDTR = length;

    /* Ensure double-buffer and circular modes are set */
    stream->CR |= DMA_CR_DBM | DMA_CR_CIRC;

    /* Enable stream */
    stream->CR |= DMA_CR_EN;

    return true;
}

void hal_dma_stop(hal_dma_handle_t *handle)
{
    if (!handle || !handle->stream) return;
    handle->stream->CR &= ~DMA_CR_EN;
}

bool hal_dma_is_active(hal_dma_handle_t *handle)
{
    if (!handle || !handle->stream) return false;
    return (handle->stream->CR & DMA_CR_EN) != 0;
}

void hal_dma_register_callbacks(hal_dma_handle_t *handle,
                                hal_dma_callback_t tc_callback,
                                hal_dma_callback_t ht_callback,
                                void *context)
{
    if (!handle) return;
    handle->tc_callback = tc_callback;
    handle->ht_callback = ht_callback;
    handle->callback_ctx = context;
}

uint8_t hal_dma_get_current_buffer(hal_dma_handle_t *handle)
{
    if (!handle || !handle->stream) return 0;
    return (handle->stream->CR & DMA_CR_CT) ? 1 : 0;
}
