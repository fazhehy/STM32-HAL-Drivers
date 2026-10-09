#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#include <stdint.h>

void delay_init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
uint32_t delay_get_tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H */
