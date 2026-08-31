
/*
// =========================================================================
// Example 1 — init + test
// =========================================================================
//
// init() 内部完成 I/O 补偿 + MPU + SDRAM 命令序列.
// test() 依次跑 data_bus / address_bus / block_fill.

#include "cm7_main.h"

#include "delay.h"
#include "log.h"

#include "fmc.h"
#include "is42s32800j.h"

void cm7_main()
{
    log_info("SDRAM init start");

    if (!is42s32800j_init(&hsdram1)) {
        log_error("SDRAM init failed");
        Error_Handler();
    }

    log_info("SDRAM init done, testing...");

    if (!is42s32800j_test()) {
        log_error("SDRAM test failed");
        Error_Handler();
    }

    log_info("SDRAM test passed");

    for (;;) {
    }
}

// =========================================================================
// Example 2 — direct pointer access
// =========================================================================

#include "cm7_main.h"

#include "delay.h"
#include "log.h"

#include "fmc.h"
#include "is42s32800j.h"

void cm7_main()
{
    log_info("SDRAM init start");

    if (!is42s32800j_init(&hsdram1)) {
        log_error("SDRAM init failed");
        Error_Handler();
    }

    log_info("SDRAM init done");

    volatile uint32_t* sdram = (volatile uint32_t*)IS42S32800J_BASE_ADDR;
    uint32_t size_words = IS42S32800J_SIZE_BYTES / sizeof(uint32_t);

    for (uint32_t i = 0; i < size_words; i++)
        sdram[i] = i;

    for (uint32_t i = 0; i < size_words; i++) {
        if (sdram[i] != i) {
            log_error("SDRAM verify failed at word %lu", (unsigned long)i);
            Error_Handler();
        }
    }

    log_info("SDRAM read/write passed");

    for (;;) {
    }
}

// =========================================================================
// Example 3 — framebuffer
// =========================================================================

#include "cm7_main.h"

#include "delay.h"
#include "log.h"

#include "fmc.h"
#include "is42s32800j.h"

void cm7_main()
{
    log_info("SDRAM init start");

    if (!is42s32800j_init(&hsdram1)) {
        log_error("SDRAM init failed");
        Error_Handler();
    }

    log_info("SDRAM init done");

    volatile uint32_t* fb = (volatile uint32_t*)IS42S32800J_BASE_ADDR;

    for (int i = 0; i < 800 * 480; i++)
        fb[i] = 0xFF0000FF;

    log_info("framebuffer written");

    for (;;) {
    }
}

// =========================================================================
// Example 4 — heap
// =========================================================================

#include "cm7_main.h"

#include "delay.h"
#include "log.h"

#include "fmc.h"
#include "is42s32800j.h"

void cm7_main()
{
    log_info("SDRAM init start");

    if (!is42s32800j_init(&hsdram1)) {
        log_error("SDRAM init failed");
        Error_Handler();
    }

    log_info("SDRAM init done");

    void* heap_start = (void*)IS42S32800J_BASE_ADDR;
    uint32_t heap_size = IS42S32800J_SIZE_BYTES;

    // my_allocator_init(heap_start, heap_size);

    log_info("SDRAM heap ready: start=0x%08lX size=%lu",
             (unsigned long)(uintptr_t)heap_start, (unsigned long)heap_size);

    for (;;) {
    }
}
*/
