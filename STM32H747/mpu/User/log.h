#ifndef __LOG_H
#define __LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm32h7xx_hal.h"
#include "usart.h"

#define USE_LOG (1)
#define LOG_BUF_LEN (256)
#define LOG_UART_HANDLE huart1

#define LOG_UART_TIMEOUT_MS 100U
#define LOG_PRINT(str, len) HAL_UART_Transmit(&LOG_UART_HANDLE, (uint8_t*)(str), (len), LOG_UART_TIMEOUT_MS)

void log_output(const char* file, uint16_t line, const char* str, ...);
void log_error_output(const char* file, uint16_t line, const char* str, ...);
void log_printf_output(const char* str, ...);
void log_vofa_justfloat_output(const float* data, uint8_t count);

#if USE_LOG
    #define log_info(str, ...) (log_output(__FILE__, __LINE__, str, ##__VA_ARGS__))
    #define log_error(str, ...) (log_error_output(__FILE__, __LINE__, str, ##__VA_ARGS__))
    #define log_printf(str, ...) (log_printf_output(str, ##__VA_ARGS__))
    #define log_vofa(data, n) (log_vofa_justfloat_output(data, n))
#else
    #define log_info(str, ...) ((void)0)
    #define log_error(str, ...) ((void)0)
    #define log_printf(str, ...) ((void)0)
    #define log_vofa(data, n) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LOG_H */
