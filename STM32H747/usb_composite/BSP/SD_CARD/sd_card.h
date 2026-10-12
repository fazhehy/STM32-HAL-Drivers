#ifndef __SD_CARD_H
#define __SD_CARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define SD_CARD_BLOCK_SIZE 512U

bool sd_card_init(SD_HandleTypeDef* hsd);
bool sd_card_get_info(HAL_SD_CardInfoTypeDef* info);
bool sd_card_read_blocks(uint8_t* data, uint32_t block_address, uint32_t block_count, uint32_t timeout_ms);
bool sd_card_write_blocks(const uint8_t* data, uint32_t block_address, uint32_t block_count, uint32_t timeout_ms);
bool sd_card_wait_ready(uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __SD_CARD_H */
