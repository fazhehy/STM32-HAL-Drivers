/*
#include "cm7_main.h"

#include "log.h"
#include "zt2628.h"

void cm7_main(void)
{
    const zt2628_pins_t pins = {
        GPIOB, GPIO_PIN_10,
        GPIOB, GPIO_PIN_11,
        GPIOB, GPIO_PIN_12,
    };

    if (zt2628_init(&pins) != 0) {
        log_error("ZT2628 init failed");
    }

    for (;;) {
        zt2628_scan_point(0);
    }
}
*/
