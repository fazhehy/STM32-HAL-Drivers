#include "cm7_main.h"

#include "fmc.h"
#include "is42s32800j.h"
#include "log.h"
#include "main.h"
#include "mpu.h"

void cm7_main(void)
{
    mpu_memory_protection();

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
