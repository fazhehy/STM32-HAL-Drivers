#include "cm7_main.h"

#include "fatfs.h"
#include "log.h"
#include "main.h"

#include <cstdio>
#include <cstring>

void cm7_main(void)
{
    if (retSD != 0U) {
        log_error("FatFs driver link failed: %u", retSD);
        Error_Handler();
    }

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
        (void)f_close(&file);
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

    for (;;) {
    }
}
