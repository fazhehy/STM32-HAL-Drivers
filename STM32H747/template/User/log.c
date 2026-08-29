#include "log.h"

#if USE_LOG

static char log_buf[LOG_BUF_LEN];

void log_output(const char* file, uint16_t line, const char* str, ...)
{
    char loc_buf[LOG_BUF_LEN - 16U];
    va_list arg;
    va_start(arg, str);
    vsnprintf(loc_buf, sizeof(loc_buf), str, arg);
    va_end(arg);
    (void)file;
    (void)line;
    snprintf(log_buf, LOG_BUF_LEN, "[=LOG=]: %s\r\n", loc_buf);
    LOG_PRINT(log_buf, strlen(log_buf));
}

void log_error_output(const char* file, uint16_t line, const char* str, ...)
{
    char loc_buf[LOG_BUF_LEN - 16U];
    va_list arg;
    va_start(arg, str);
    vsnprintf(loc_buf, sizeof(loc_buf), str, arg);
    va_end(arg);
    (void)file;
    (void)line;
    snprintf(log_buf, LOG_BUF_LEN, "[ERROR]: %s\r\n", loc_buf);
    LOG_PRINT(log_buf, strlen(log_buf));
}

void log_printf_output(const char* str, ...)
{
    va_list arg;
    va_start(arg, str);
    vsnprintf(log_buf, LOG_BUF_LEN, str, arg);
    va_end(arg);
    LOG_PRINT(log_buf, strlen(log_buf));
}

void log_vofa_justfloat_output(const float* data, uint8_t count)
{
    static const uint8_t header[4] = {0x00, 0x00, 0x80, 0x7F};
    LOG_PRINT(header, sizeof(header));
    LOG_PRINT((const uint8_t*)data, count * sizeof(float));
}

#endif /* USE_LOG */
