#include "cm7_main.h"

#include "delay.h"
#include "fatfs.h"
#include "log.h"
#include "main.h"
#include "sd_card.h"
#include "sdmmc.h"
#include "usb_cdc.h"
#include "usb_device.h"
#include "usb_msc.h"
#include "usbd_cdc_if.h"

#include <cstdio>
#include <cstring>

void cm7_main(void)
{
    if (retSD != 0U) {
        log_error("FatFs driver link failed: %u", retSD);
        Error_Handler();
    }

    // 卡信息自检，确认卡已就绪并打印容量。
    HAL_SD_CardInfoTypeDef info;
    if (!sd_card_init(&hsd1) || !sd_card_get_info(&info)) {
        log_error("SD init failed");
        Error_Handler();
    }
    log_info("SD: block_number=%lu block_size=%u", info.LogBlockNbr, info.LogBlockSize);

    FRESULT result = f_mount(&SDFatFS, SDPath, 1);
    if (result != FR_OK) {
        log_error("FatFs mount failed: %d", (int)result);
        Error_Handler();
    }

    FIL file;
    char path[24];
    bool created = false;

    // 选择不存在的 8.3 文件名，避免覆盖卡上的已有文件。
    for (unsigned long index = 0; index < 1000UL; ++index) {
        std::snprintf(path, sizeof(path), "%s/TST%05lu.TXT", SDPath, index);
        result = f_open(&file, path, FA_CREATE_NEW | FA_WRITE);
        if (result == FR_OK) {
            created = true;
            break;
        }
        if (result != FR_EXIST) {
            log_error("FatFs create failed: %d", (int)result);
            Error_Handler();
        }
    }

    if (!created) {
        log_error("FatFs test filename exhausted");
        Error_Handler();
    }

    static const char test_text[] = "STM32H747 FatFs read/write OK\r\n";
    UINT written = 0;
    result = f_write(&file, test_text, sizeof(test_text) - 1U, &written);
    if (result != FR_OK || written != sizeof(test_text) - 1U) {
        log_error("FatFs write failed: %d, bytes: %u", (int)result, written);
        f_close(&file);
        Error_Handler();
    }

    result = f_close(&file);
    if (result != FR_OK) {
        log_error("FatFs close failed: %d", (int)result);
        Error_Handler();
    }

    result = f_open(&file, path, FA_READ);
    if (result != FR_OK) {
        log_error("FatFs reopen failed: %d", (int)result);
        Error_Handler();
    }

    char read_text[sizeof(test_text)] = {0};
    UINT read = 0;
    result = f_read(&file, read_text, sizeof(test_text) - 1U, &read);
    FRESULT close_result = f_close(&file);
    if (result != FR_OK || close_result != FR_OK || read != sizeof(test_text) - 1U ||
        std::memcmp(read_text, test_text, sizeof(test_text) - 1U) != 0) {
        log_error("FatFs verify failed: read=%d, close=%d, bytes=%u", (int)result, (int)close_result, read);
        Error_Handler();
    }

    log_info("FatFs test passed: %s", path);

    // 给卡写卷标，PC 上显示为 STM32；失败只影响名字，不影响读写，不作为致命错误。
    result = f_setlabel("STM32");
    if (result != FR_OK) {
        log_error("FatFs set label failed: %d", (int)result);
    }

    // 自检结束，卸载 FatFs 并永久把卡交给 USB MSC，此后设备侧不再访问。
    f_mount(NULL, SDPath, 0);
    usb_msc_set_media_ready(true);

    // 发送缓冲区放成静态的，且回环和日志各用各的：
    // CDC_Transmit_FS 只是把句柄的发送指针指过来，不拷贝数据，发送期间写同一块缓冲区
    // 会把正在发的内容改掉。每轮循环先用 CDC_IsTransmitReady_FS 确认上一次发完了再写，
    // 两块分开则保证回环刚发出去的那次传输不会被本轮日志覆盖。
    // 64 与 USB_CDC 环形缓冲一次能取的量一致；日志行最长约 40 字节，48 足够且 snprintf 会截断。
    static uint8_t echo_buffer[64];
    static uint8_t log_buffer[48];
    uint32_t tick = 0U;
    uint32_t counter = 0U;

    for (;;) {
        const bool configured = (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);

        // 每秒一行日志，证明虚拟串口和盘在同一个口上并存；顺便带出环形缓冲被退回的字节数，
        // 主机灌得比 10 ms 一轮快时能从这一项看出来。
        // 日志排在回环前面：回环字节晚一轮只是延迟，日志这一秒错过了就真没了。
        if (++tick >= 100U) {
            tick = 0U;
            if (configured && CDC_IsTransmitReady_FS()) {
                int length = std::snprintf((char*)log_buffer, sizeof(log_buffer), "CM7 USB CDC OK %lu drop=%lu\r\n",
                                           (unsigned long)counter, (unsigned long)usb_cdc_get_dropped_count());
                if (length > 0) {
                    CDC_Transmit_FS(log_buffer, (uint16_t)length);
                    ++counter;
                }
            }
        }

        // 主机发来的字节原样发回。发送忙（比如上一句刚发完日志）就整轮跳过：
        // 字节留在环形缓冲里，既不丢，也不会覆盖正在发送的那一块。
        if (configured && CDC_IsTransmitReady_FS()) {
            uint32_t count = usb_cdc_read(echo_buffer, sizeof(echo_buffer));
            if (count > 0U) {
                CDC_Transmit_FS(echo_buffer, (uint16_t)count);
            }
        }

        // 环形缓冲腾出空位后重新挂 OUT 接收。没挂上的那段时间端点保持 NAK，
        // 主机的写会阻塞等着而不是被丢掉，收到的字节一个都不会少。
        if (configured) {
            CDC_ResumeReceive_FS();
        }

        delay_ms(10);
    }
}
