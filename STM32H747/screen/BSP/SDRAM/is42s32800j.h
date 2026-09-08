
#ifndef __IS42S32800J_H
#define __IS42S32800J_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define IS42S32800J_BASE_ADDR 0xD0000000UL
#define IS42S32800J_SIZE_BYTES (32UL * 1024UL * 1024UL)

bool is42s32800j_init(SDRAM_HandleTypeDef* hsdram);
bool is42s32800j_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __IS42S32800J_H */
