#ifndef __KD025EGOIN001_H
#define __KD025EGOIN001_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#include <stdbool.h>
#include <stdint.h>

#define KD025EGOIN001_WIDTH 400
#define KD025EGOIN001_HEIGHT 712
#define KD025EGOIN001_FB_ADDRESS 0xD0000000
#define KD025EGOIN001_PIXEL_BYTES 3
#define KD025EGOIN001_FRAMEBUFFER_SIZE (KD025EGOIN001_WIDTH * KD025EGOIN001_HEIGHT * KD025EGOIN001_PIXEL_BYTES)
#define KD025EGOIN001_BACK_FB_ADDRESS (KD025EGOIN001_FB_ADDRESS + KD025EGOIN001_FRAMEBUFFER_SIZE)

// 屏幕主电源和 IOVCC 使能引脚。
#define KD025EGOIN001_PWREN_PORT GPIOI
#define KD025EGOIN001_PWREN_PIN GPIO_PIN_11

// SGM3836A CTRL 引脚，用于使能 OLED 偏压并设置 ELVSS。
#define KD025EGOIN001_BIAS_CTRL_PORT GPIOI
#define KD025EGOIN001_BIAS_CTRL_PIN GPIO_PIN_12

// 屏幕复位引脚，低电平有效。
#define KD025EGOIN001_RST_PORT GPIOH
#define KD025EGOIN001_RST_PIN GPIO_PIN_5

#define KD025EGOIN001_POWER_ON() HAL_GPIO_WritePin(KD025EGOIN001_PWREN_PORT, KD025EGOIN001_PWREN_PIN, GPIO_PIN_SET)
#define KD025EGOIN001_POWER_OFF() HAL_GPIO_WritePin(KD025EGOIN001_PWREN_PORT, KD025EGOIN001_PWREN_PIN, GPIO_PIN_RESET)
#define KD025EGOIN001_BIAS_CTRL_HIGH()                                                                                 \
    HAL_GPIO_WritePin(KD025EGOIN001_BIAS_CTRL_PORT, KD025EGOIN001_BIAS_CTRL_PIN, GPIO_PIN_SET)
#define KD025EGOIN001_BIAS_CTRL_LOW()                                                                                  \
    HAL_GPIO_WritePin(KD025EGOIN001_BIAS_CTRL_PORT, KD025EGOIN001_BIAS_CTRL_PIN, GPIO_PIN_RESET)
#define KD025EGOIN001_RST_HIGH() HAL_GPIO_WritePin(KD025EGOIN001_RST_PORT, KD025EGOIN001_RST_PIN, GPIO_PIN_SET)
#define KD025EGOIN001_RST_LOW() HAL_GPIO_WritePin(KD025EGOIN001_RST_PORT, KD025EGOIN001_RST_PIN, GPIO_PIN_RESET)

// RGB888 帧缓冲颜色格式为 0xAARRGGBB，最高字节不会送往屏幕。
#define KD025EGOIN001_COLOR_BLACK 0xFF000000
#define KD025EGOIN001_COLOR_WHITE 0xFFFFFFFF
#define KD025EGOIN001_COLOR_RED 0xFFFF0000
#define KD025EGOIN001_COLOR_GREEN 0xFF00FF00
#define KD025EGOIN001_COLOR_BLUE 0xFF0000FF
#define KD025EGOIN001_COLOR_YELLOW 0xFFFFFF00
#define KD025EGOIN001_COLOR_CYAN 0xFF00FFFF
#define KD025EGOIN001_COLOR_MAGENTA 0xFFFF00FF

bool kd025egoin001_init(void);
bool kd025egoin001_write_command(uint8_t command, const uint8_t* data, uint16_t data_length);
// 所有绘图操作写入后台缓冲，完成一帧后调用 present 在垂直消隐期显示。
bool kd025egoin001_clear(uint32_t color);
bool kd025egoin001_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
bool kd025egoin001_present(void);
void kd025egoin001_draw_pixel(uint16_t x, uint16_t y, uint32_t color);
uint32_t kd025egoin001_read_pixel(uint16_t x, uint16_t y);
void kd025egoin001_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
void kd025egoin001_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint32_t color);
void kd025egoin001_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
void kd025egoin001_draw_circle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint32_t color);
void kd025egoin001_fill_circle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint32_t color);
bool kd025egoin001_test(void);

#ifdef __cplusplus
}
#endif

#endif // __KD025EGOIN001_H
