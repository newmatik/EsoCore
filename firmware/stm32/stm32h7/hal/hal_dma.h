/**
 * @file hal_dma.h
 * @brief DMA Hardware Abstraction Layer for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_DMA_H
#define ESOCORE_HAL_DMA_H

#include <stdint.h>
#include <stdbool.h>
#include "../cmsis/stm32h747xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DMA_DIR_PERIPH_TO_MEM = 0,
    DMA_DIR_MEM_TO_PERIPH = 1,
    DMA_DIR_MEM_TO_MEM    = 2,
} hal_dma_direction_t;

typedef enum {
    DMA_DATA_BYTE     = 0,
    DMA_DATA_HALFWORD = 1,
    DMA_DATA_WORD     = 2,
} hal_dma_data_size_t;

typedef enum {
    DMA_PRIORITY_LOW    = 0,
    DMA_PRIORITY_MEDIUM = 1,
    DMA_PRIORITY_HIGH   = 2,
    DMA_PRIORITY_VERY_HIGH = 3,
} hal_dma_priority_t;

typedef void (*hal_dma_callback_t)(void *context);

typedef struct {
    DMA_Stream_TypeDef  *stream;
    hal_dma_direction_t direction;
    hal_dma_data_size_t periph_size;
    hal_dma_data_size_t mem_size;
    bool                periph_inc;
    bool                mem_inc;
    bool                circular;
    bool                double_buffer;
    hal_dma_priority_t  priority;
    uint8_t             dmamux_request;   /* DMAMUX request ID */
} hal_dma_config_t;

typedef struct {
    DMA_Stream_TypeDef  *stream;
    hal_dma_callback_t  tc_callback;      /* Transfer complete */
    hal_dma_callback_t  ht_callback;      /* Half transfer */
    void                *callback_ctx;
    bool                initialized;
} hal_dma_handle_t;

bool hal_dma_init(hal_dma_handle_t *handle, const hal_dma_config_t *config);
void hal_dma_deinit(hal_dma_handle_t *handle);

bool hal_dma_start(hal_dma_handle_t *handle,
                   uint32_t src_addr, uint32_t dst_addr,
                   uint16_t length);

bool hal_dma_start_double_buffer(hal_dma_handle_t *handle,
                                 uint32_t periph_addr,
                                 uint32_t mem0_addr, uint32_t mem1_addr,
                                 uint16_t length);

void hal_dma_stop(hal_dma_handle_t *handle);

bool hal_dma_is_active(hal_dma_handle_t *handle);

void hal_dma_register_callbacks(hal_dma_handle_t *handle,
                                hal_dma_callback_t tc_callback,
                                hal_dma_callback_t ht_callback,
                                void *context);

uint8_t hal_dma_get_current_buffer(hal_dma_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_DMA_H */
