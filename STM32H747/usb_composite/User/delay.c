#include "delay.h"
#include <stdbool.h>

static bool dwt_initialized = false;

void delay_init(void)
{
    if (dwt_initialized) {
        return;
    }

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    dwt_initialized = true;
}

void delay_us(uint32_t us)
{
    uint32_t start;
    uint32_t ticks;

    delay_init();

    start = DWT->CYCCNT;
    ticks = us * (SystemCoreClock / 1000000U);

    while ((DWT->CYCCNT - start) < ticks) {
    }
}

void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

uint32_t delay_get_tick(void)
{
    return HAL_GetTick();
}
