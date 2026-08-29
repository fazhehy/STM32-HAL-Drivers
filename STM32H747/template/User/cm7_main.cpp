#include "cm7_main.h"

#include "delay.h"
#include "log.h"

void cm7_main(void)
{
    for (;;) {
        log_info("CM7 main loop");
        delay_ms(100);
    }
}
