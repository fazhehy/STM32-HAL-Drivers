#include "sd_card.h"

static SD_HandleTypeDef* sd_handle;

bool sd_card_init(SD_HandleTypeDef* hsd)
{
    sd_handle = NULL;

    if (hsd == NULL || HAL_SD_GetState(hsd) != HAL_SD_STATE_READY) {
        return false;
    }

    if (HAL_SD_GetCardState(hsd) != HAL_SD_CARD_TRANSFER) {
        return false;
    }

    sd_handle = hsd;
    return true;
}

bool sd_card_get_info(HAL_SD_CardInfoTypeDef* info)
{
    if (sd_handle == NULL || info == NULL) {
        return false;
    }

    return HAL_SD_GetCardInfo(sd_handle, info) == HAL_OK;
}

bool sd_card_read_blocks(uint8_t* data, uint32_t block_address, uint32_t block_count, uint32_t timeout_ms)
{
    if (sd_handle == NULL || data == NULL || block_count == 0U) {
        return false;
    }

    return HAL_SD_ReadBlocks(sd_handle, data, block_address, block_count, timeout_ms) == HAL_OK;
}

bool sd_card_write_blocks(const uint8_t* data, uint32_t block_address, uint32_t block_count, uint32_t timeout_ms)
{
    if (sd_handle == NULL || data == NULL || block_count == 0U) {
        return false;
    }

    if (HAL_SD_WriteBlocks(sd_handle, data, block_address, block_count, timeout_ms) != HAL_OK) {
        return false;
    }

    return sd_card_wait_ready(timeout_ms);
}

bool sd_card_wait_ready(uint32_t timeout_ms)
{
    if (sd_handle == NULL) {
        return false;
    }

    uint32_t start = HAL_GetTick();

    do {
        if (HAL_SD_GetCardState(sd_handle) == HAL_SD_CARD_TRANSFER) {
            return true;
        }
    } while ((HAL_GetTick() - start) < timeout_ms);

    return false;
}
