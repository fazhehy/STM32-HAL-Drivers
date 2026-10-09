
#ifndef __IS42S32800J_H
#define __IS42S32800J_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define IS42S32800J_BASE_ADDR 0xD0000000
#define IS42S32800J_SIZE_BYTES (32 * 1024 * 1024)

bool is42s32800j_init(SDRAM_HandleTypeDef* hsdram);
bool is42s32800j_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __IS42S32800J_H */
