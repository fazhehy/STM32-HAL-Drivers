#include "cm7_main.h"

#include "fmc.h"
#include "is42s32800j.h"
#include "kd025egoin001.h"
#include "log.h"
#include "main.h"
#include "mpu.h"

void cm7_main(void)
{
    mpu_memory_protection();

    if (!is42s32800j_init(&hsdram1)) {
        log_error("SDRAM init failed");
        Error_Handler();
    }

    if (!kd025egoin001_init()) {
        log_error("screen init failed");
        Error_Handler();
    }

    for (;;) {
        if (!kd025egoin001_test()) {
            log_error("screen test failed");
            Error_Handler();
        }
    }
}