#ifndef __USB_MSC_H
#define __USB_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define USB_MSC_BLOCK_SIZE 512U
#define USB_MSC_TIMEOUT_MS 1000U

void usb_msc_set_media_ready(bool ready);
bool usb_msc_is_media_ready(void);
bool usb_msc_get_capacity(uint32_t* block_number, uint16_t* block_size);
bool usb_msc_read_blocks(uint8_t* data, uint32_t block_address, uint32_t block_count);
bool usb_msc_write_blocks(const uint8_t* data, uint32_t block_address, uint32_t block_count);

#ifdef __cplusplus
}
#endif

#endif /* __USB_MSC_H */
