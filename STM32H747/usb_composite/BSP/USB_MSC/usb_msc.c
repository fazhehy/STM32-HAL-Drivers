#include "usb_msc.h"

#include "sd_card.h"

// 设备侧上电自检完成前，介质不交给主机。
static volatile bool media_ready;

void usb_msc_set_media_ready(bool ready)
{
    media_ready = ready;
}

bool usb_msc_is_media_ready(void)
{
    if (!media_ready) {
        return false;
    }

    return sd_card_wait_ready(0U);
}

bool usb_msc_get_capacity(uint32_t* block_number, uint16_t* block_size)
{
    if (block_number == NULL || block_size == NULL) {
        return false;
    }

    HAL_SD_CardInfoTypeDef info;
    if (!sd_card_get_info(&info)) {
        return false;
    }

    *block_number = info.LogBlockNbr;
    *block_size = (uint16_t)info.LogBlockSize;
    return true;
}

bool usb_msc_read_blocks(uint8_t* data, uint32_t block_address, uint32_t block_count)
{
    return sd_card_read_blocks(data, block_address, block_count, USB_MSC_TIMEOUT_MS);
}

bool usb_msc_write_blocks(const uint8_t* data, uint32_t block_address, uint32_t block_count)
{
    return sd_card_write_blocks(data, block_address, block_count, USB_MSC_TIMEOUT_MS);
}
