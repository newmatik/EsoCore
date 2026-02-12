/**
 * Linker script for STM32H747IITx Cortex-M7 core
 * EsoCore Edge v1.5.0.0
 *
 * Memory map:
 *   FLASH  (rx)  : 2 MB at 0x08000000  (Bank 1, CM7)
 *   DTCMRAM(rwx) : 128 KB at 0x20000000 (fast, zero-wait-state for M7)
 *   AXI_SRAM(rwx): 512 KB at 0x24000000 (main working RAM)
 *   SRAM1  (rwx) : 128 KB at 0x30000000
 *   SRAM2  (rwx) : 128 KB at 0x30020000
 *   SRAM3  (rwx) : 32 KB at 0x30040000
 *   SRAM4  (rwx) : 64 KB at 0x38000000  (D3 domain, backup SRAM)
 */

/* Entry point */
ENTRY(Reset_Handler)

/* Stack and heap sizes */
_Min_Heap_Size  = 0x2000;  /* 8 KB minimum heap */
_Min_Stack_Size = 0x4000;  /* 16 KB minimum stack */

MEMORY
{
    FLASH    (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
    DTCMRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    AXI_SRAM (rwx) : ORIGIN = 0x24000000, LENGTH = 512K
    SRAM1    (rwx) : ORIGIN = 0x30000000, LENGTH = 128K
    SRAM2    (rwx) : ORIGIN = 0x30020000, LENGTH = 128K
    SRAM3    (rwx) : ORIGIN = 0x30040000, LENGTH = 32K
    SRAM4    (rwx) : ORIGIN = 0x38000000, LENGTH = 64K
}

SECTIONS
{
    /* Interrupt vector table -- must be at start of FLASH */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* Program code and read-only data */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        KEEP(*(.init))
        KEEP(*(.fini))

        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* Read-only data */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    /* ARM exception unwinding tables */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* Pre-init and init arrays */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } >FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } >FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } >FLASH

    /* Used by startup code to initialize .data */
    _sidata = LOADADDR(.data);

    /* Initialized data -- stored in FLASH, copied to AXI_SRAM at startup */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        *(.RamFunc)
        *(.RamFunc*)
        . = ALIGN(4);
        _edata = .;
    } >AXI_SRAM AT> FLASH

    /* Uninitialized data (zeroed at startup) */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } >AXI_SRAM

    /* Heap and stack in AXI_SRAM */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >AXI_SRAM

    /* DTCM RAM section for performance-critical data (DMA buffers, etc.) */
    .dtcmram (NOLOAD) :
    {
        . = ALIGN(4);
        *(.dtcmram)
        *(.dtcmram*)
        . = ALIGN(4);
    } >DTCMRAM

    /* Ethernet DMA descriptors in SRAM1 (non-cacheable region) */
    .eth_dma (NOLOAD) :
    {
        . = ALIGN(32);
        *(.eth_dma)
        *(.eth_dma*)
        . = ALIGN(32);
    } >SRAM1

    /* ADC DMA buffers in SRAM2 */
    .adc_dma (NOLOAD) :
    {
        . = ALIGN(32);
        *(.adc_dma)
        *(.adc_dma*)
        . = ALIGN(32);
    } >SRAM2

    /* Stack top -- end of AXI_SRAM */
    _estack = ORIGIN(AXI_SRAM) + LENGTH(AXI_SRAM);

    /* Discard debug sections from final binary to save space */
    /DISCARD/ :
    {
        libc.a(*)
        libm.a(*)
        libgcc.a(*)
    }
}
