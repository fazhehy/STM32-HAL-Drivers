/*
// 前置条件：main.c 已调用 MX_SDMMC1_SD_Init() 初始化 SDMMC1，且设备侧的上电自检已完成。
// 自检期间门闸保持关闭，否则主机会和 FatFs 同时访问同一张卡，破坏文件系统。
#include "cm7_main.h"

#include "fatfs.h"
#include "log.h"
#include "sd_card.h"
#include "usb_msc.h"

void cm7_main(void)
{
    // ... 卡信息自检与 FatFs 读写校验 ...

    f_setlabel("STM32");            // 给卡写卷标，PC 上显示为 STM32（需 _USE_LABEL=1）
    f_mount(NULL, SDPath, 0);       // 卸载 FatFs，避免与主机争用
    usb_msc_set_media_ready(true);  // 自检结束，设备侧不再访问卡

    for (;;) {
        delay_ms(1000);
    }
}

// usbd_storage_if.c 的 7 个回调只做 bool 到 USBD_OK / USBD_FAIL 的转换：
//
//   STORAGE_Init_FS             -> USBD_OK
//   STORAGE_IsReady_FS          -> usb_msc_is_media_ready() ? USBD_OK : USBD_FAIL
//   STORAGE_GetCapacity_FS      -> usb_msc_get_capacity(block_num, block_size) ? USBD_OK : USBD_FAIL
//   STORAGE_IsWriteProtected_FS -> USBD_OK
//   STORAGE_Read_FS             -> usb_msc_read_blocks(buf, blk_addr, blk_len) ? USBD_OK : USBD_FAIL
//   STORAGE_Write_FS            -> usb_msc_write_blocks(buf, blk_addr, blk_len) ? USBD_OK : USBD_FAIL
//   STORAGE_GetMaxLun_FS        -> STORAGE_LUN_NBR - 1
//
// STORAGE_Init_FS 由 MSC_BOT_Init() 调用，每次 SET_CONFIGURATION（每次插拔）都会执行，
// 因此不能在那里复位门闸。
*/
