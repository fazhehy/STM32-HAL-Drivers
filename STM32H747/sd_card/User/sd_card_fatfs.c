#include "sd_card.h"

#include "bsp_driver_sd.h"
#include "sdmmc.h"

uint8_t BSP_SD_Init(void)
{
    // main.c 已初始化 SDMMC1，FatFs 在此关联 SD 卡驱动。
    return sd_card_init(&hsd1) ? MSD_OK : MSD_ERROR;
}

uint8_t BSP_SD_ReadBlocks(uint32_t* data, uint32_t block_address, uint32_t block_count, uint32_t timeout_ms)
{
    return sd_card_read_blocks((uint8_t*)data, block_address, block_count, timeout_ms) ? MSD_OK : MSD_ERROR;
}

uint8_t BSP_SD_WriteBlocks(uint32_t* data, uint32_t block_address, uint32_t block_count, uint32_t timeout_ms)
{
    return sd_card_write_blocks((const uint8_t*)data, block_address, block_count, timeout_ms) ? MSD_OK : MSD_ERROR;
}

uint8_t BSP_SD_GetCardState(void)
{
    return sd_card_wait_ready(0U) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY;
}

void BSP_SD_GetCardInfo(BSP_SD_CardInfo* info)
{
    (void)sd_card_get_info(info);
}
