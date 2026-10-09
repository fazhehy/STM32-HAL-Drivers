#include "cm7_main.h"

#include "fmc.h"
#include "is42s32800j.h"
#include "kd025egoin001.h"
#include "log.h"
#include "main.h"
#include "mpu.h"
#include "zt2628.h"

void cm7_main(void)
{
    const zt2628_pins_t touch_pins = {
        GPIOB, GPIO_PIN_10, GPIOB, GPIO_PIN_11, GPIOB, GPIO_PIN_12,
    };

    mpu_memory_protection();

    if (!is42s32800j_init(&hsdram1)) {
        log_error("SDRAM init failed");
        Error_Handler();
    }
    if (!kd025egoin001_init()) {
        log_error("screen init failed");
        Error_Handler();
    }
    if (zt2628_init(&touch_pins) != 0) {
        log_error("ZT2628 init failed");
        Error_Handler();
    }

    for (;;) {
        if (!kd025egoin001_test()) {
            log_error("screen test failed");
            Error_Handler();
        }
        // if (ZT2628_INT_IS_LOW() && zt2628_scan(0) != 0) {
        //     for (uint8_t index = 0; index < ZT2628_MAX_TOUCHES; index++) {
        //         if ((zt2628_touch.sta & (1U << index)) != 0U) {
        //             log_info("touch[%u]: x=%u y=%u", (unsigned int)index, (unsigned int)zt2628_touch.x[index],
        //                      (unsigned int)zt2628_touch.y[index]);
        //         }
        //     }
        // }
        HAL_Delay(5);
    }
}
