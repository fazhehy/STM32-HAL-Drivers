#include "kd025egoin001.h"

#include "delay.h"
#include "dma2d.h"
#include "dsihost.h"
#include "ltdc.h"

#define KD025EGOIN001_DSI_CHANNEL 0U
#define KD025EGOIN001_CACHE_LINE_SIZE 32U
#define KD025EGOIN001_FRAMEBUFFER_SIZE (KD025EGOIN001_WIDTH * KD025EGOIN001_HEIGHT * KD025EGOIN001_PIXEL_BYTES)
#define SGM3836A_ELVSS_NEGATIVE_2V2_PULSES 33U

static const uint8_t command_fe_page_20[] = {0x20};
static const uint8_t command_5a_unlock[] = {0x21};
static const uint8_t command_fe_page_00[] = {0x00};
static const uint8_t command_fe_page_d0[] = {0xD0};
static const uint8_t command_c6_bist_control[] = {0x1F};
static const uint8_t command_c7_bist_off[] = {0x00};
static const uint8_t command_3a_rgb888[] = {0x77};
static const uint8_t command_2a_columns[] = {0x00, 0x00, 0x01, 0x8F};
static const uint8_t command_2b_rows[] = {0x00, 0x00, 0x02, 0xC7};
static const uint8_t command_31_partial_columns[] = {0x00, 0x01, 0x01, 0x8E};
static const uint8_t command_30_partial_rows[] = {0x00, 0x01, 0x02, 0xC6};
static const uint8_t command_12_partial_mode[] = {0x00};
static const uint8_t command_35_te_enable[] = {0x00};
static const uint8_t command_51_brightness[] = {0xFF};

typedef struct {
    uint8_t command;
    const uint8_t* data;
    uint8_t data_length;
} kd025egoin001_command_t;

// RM690C0 初始化指令表。
static const kd025egoin001_command_t initialization_commands[] = {
    {0xFE, command_fe_page_d0, sizeof(command_fe_page_d0)},
    {0xC6, command_c6_bist_control, sizeof(command_c6_bist_control)},
    {0xC7, command_c7_bist_off, sizeof(command_c7_bist_off)},
    {0xFE, command_fe_page_20, sizeof(command_fe_page_20)},
    {0x5A, command_5a_unlock, sizeof(command_5a_unlock)},
    {0xFE, command_fe_page_00, sizeof(command_fe_page_00)},
    {0x3A, command_3a_rgb888, sizeof(command_3a_rgb888)},
    {0x2A, command_2a_columns, sizeof(command_2a_columns)},
    {0x2B, command_2b_rows, sizeof(command_2b_rows)},
    {0x31, command_31_partial_columns, sizeof(command_31_partial_columns)},
    {0x30, command_30_partial_rows, sizeof(command_30_partial_rows)},
    {0x12, command_12_partial_mode, sizeof(command_12_partial_mode)},
    {0x35, command_35_te_enable, sizeof(command_35_te_enable)},
    {0x51, command_51_brightness, sizeof(command_51_brightness)},
};

static void enable_bias_power(void)
{
    uint32_t primask;

    KD025EGOIN001_BIAS_CTRL_HIGH();
    delay_us(400);

    primask = __get_PRIMASK();
    __disable_irq();

    for (uint32_t pulse = 0U; pulse < SGM3836A_ELVSS_NEGATIVE_2V2_PULSES; ++pulse) {
        KD025EGOIN001_BIAS_CTRL_LOW();
        delay_us(10);
        KD025EGOIN001_BIAS_CTRL_HIGH();
        delay_us(10);
    }

    delay_us(55);

    if (primask == 0U) {
        __enable_irq();
    }
}

bool kd025egoin001_write_command(uint8_t command, const uint8_t* data, uint16_t data_length)
{
    if (data_length == 0U) {
        return HAL_DSI_ShortWrite(&hdsi, KD025EGOIN001_DSI_CHANNEL, DSI_DCS_SHORT_PKT_WRITE_P0, command, 0U) == HAL_OK;
    }

    if (data == NULL) {
        return false;
    }

    if (data_length == 1U) {
        return HAL_DSI_ShortWrite(&hdsi, KD025EGOIN001_DSI_CHANNEL, DSI_DCS_SHORT_PKT_WRITE_P1, command, data[0]) ==
               HAL_OK;
    }

    return HAL_DSI_LongWrite(&hdsi, KD025EGOIN001_DSI_CHANNEL, DSI_DCS_LONG_PKT_WRITE, data_length, command,
                             (uint8_t*)data) == HAL_OK;
}

bool kd025egoin001_init(void)
{
    DSI_LPCmdTypeDef lp_commands = {0};

    KD025EGOIN001_BIAS_CTRL_LOW();

    // 打开屏幕主电源和 IOVCC 电源路径。
    KD025EGOIN001_POWER_ON();
    delay_ms(100);

    if (!kd025egoin001_clear(KD025EGOIN001_COLOR_BLACK)) {
        return false;
    }

    __HAL_LTDC_ENABLE(&hltdc);

    KD025EGOIN001_RST_HIGH();
    delay_ms(1);
    KD025EGOIN001_RST_LOW();
    delay_ms(10);
    KD025EGOIN001_RST_HIGH();
    delay_ms(200);

    if (HAL_DSI_Start(&hdsi) != HAL_OK) {
        return false;
    }

    lp_commands.LPDcsShortWriteNoP = DSI_LP_DSW0P_ENABLE;
    lp_commands.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
    lp_commands.LPDcsLongWrite = DSI_LP_DLW_ENABLE;
    lp_commands.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;

    if (HAL_DSI_ConfigCommand(&hdsi, &lp_commands) != HAL_OK) {
        return false;
    }

    SET_BIT(hdsi.Instance->VMCR, DSI_VMCR_LPCE);
    delay_ms(10);

    for (uint32_t index = 0U; index < (sizeof(initialization_commands) / sizeof(initialization_commands[0])); ++index) {
        const kd025egoin001_command_t* entry = &initialization_commands[index];

        if (!kd025egoin001_write_command(entry->command, entry->data, entry->data_length)) {
            return false;
        }
    }

    if (!kd025egoin001_write_command(0x11, NULL, 0U)) {
        return false;
    }
    delay_ms(120);

    if (!kd025egoin001_write_command(0x29, NULL, 0U)) {
        return false;
    }
    delay_ms(20);

    enable_bias_power();
    return true;
}

bool kd025egoin001_clear(uint32_t color)
{
    return kd025egoin001_fill_rect(0U, 0U, KD025EGOIN001_WIDTH, KD025EGOIN001_HEIGHT, color);
}

bool kd025egoin001_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    uint32_t destination;

    if ((width == 0U) || (height == 0U) || (x >= KD025EGOIN001_WIDTH) || (y >= KD025EGOIN001_HEIGHT) ||
        (((uint32_t)x + width) > KD025EGOIN001_WIDTH) || (((uint32_t)y + height) > KD025EGOIN001_HEIGHT)) {
        return false;
    }

    destination = KD025EGOIN001_FB_ADDRESS + ((((uint32_t)y * KD025EGOIN001_WIDTH) + x) * KD025EGOIN001_PIXEL_BYTES);

    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
    hdma2d.Init.OutputOffset = KD025EGOIN001_WIDTH - width;

    if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
        return false;
    }
    if (HAL_DMA2D_Start(&hdma2d, color, destination, width, height) != HAL_OK) {
        return false;
    }
    if (HAL_DMA2D_PollForTransfer(&hdma2d, 1000U) != HAL_OK) {
        return false;
    }

    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) {
        SCB_InvalidateDCache_by_Addr((uint32_t*)KD025EGOIN001_FB_ADDRESS, (int32_t)KD025EGOIN001_FRAMEBUFFER_SIZE);
    }
    return true;
}

void kd025egoin001_draw_pixel(uint16_t x, uint16_t y, uint32_t color)
{
    volatile uint8_t* const framebuffer = (volatile uint8_t*)KD025EGOIN001_FB_ADDRESS;
    uint32_t pixel_offset;

    if ((x >= KD025EGOIN001_WIDTH) || (y >= KD025EGOIN001_HEIGHT)) {
        return;
    }

    pixel_offset = (((uint32_t)y * KD025EGOIN001_WIDTH) + x) * KD025EGOIN001_PIXEL_BYTES;
    framebuffer[pixel_offset] = (uint8_t)color;
    framebuffer[pixel_offset + 1U] = (uint8_t)(color >> 8U);
    framebuffer[pixel_offset + 2U] = (uint8_t)(color >> 16U);

    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) {
        const uint32_t pixel_address = KD025EGOIN001_FB_ADDRESS + pixel_offset;
        const uint32_t cache_line_address = pixel_address & ~(KD025EGOIN001_CACHE_LINE_SIZE - 1U);

        SCB_CleanDCache_by_Addr((uint32_t*)cache_line_address, (int32_t)KD025EGOIN001_CACHE_LINE_SIZE);
    }
}

uint32_t kd025egoin001_read_pixel(uint16_t x, uint16_t y)
{
    volatile const uint8_t* const framebuffer = (volatile const uint8_t*)KD025EGOIN001_FB_ADDRESS;
    uint32_t pixel_offset;

    if ((x >= KD025EGOIN001_WIDTH) || (y >= KD025EGOIN001_HEIGHT)) {
        return 0U;
    }

    pixel_offset = (((uint32_t)y * KD025EGOIN001_WIDTH) + x) * KD025EGOIN001_PIXEL_BYTES;
    return ((uint32_t)framebuffer[pixel_offset + 2U] << 16U) | ((uint32_t)framebuffer[pixel_offset + 1U] << 8U) |
           framebuffer[pixel_offset];
}

void kd025egoin001_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color)
{
    int32_t current_x = x0;
    int32_t current_y = y0;
    const int32_t end_x = x1;
    const int32_t end_y = y1;
    const int32_t delta_x = (current_x < end_x) ? (end_x - current_x) : (current_x - end_x);
    const int32_t step_x = (current_x < end_x) ? 1 : -1;
    const int32_t delta_y_abs = (current_y < end_y) ? (end_y - current_y) : (current_y - end_y);
    const int32_t delta_y = -delta_y_abs;
    const int32_t step_y = (current_y < end_y) ? 1 : -1;
    int32_t error = delta_x + delta_y;

    for (;;) {
        if ((current_x >= 0) && (current_x < (int32_t)KD025EGOIN001_WIDTH) && (current_y >= 0) &&
            (current_y < (int32_t)KD025EGOIN001_HEIGHT)) {
            kd025egoin001_draw_pixel((uint16_t)current_x, (uint16_t)current_y, color);
        }

        if ((current_x == end_x) && (current_y == end_y)) {
            break;
        }

        const int32_t doubled_error = error * 2;
        if (doubled_error >= delta_y) {
            error += delta_y;
            current_x += step_x;
        }
        if (doubled_error <= delta_x) {
            error += delta_x;
            current_y += step_y;
        }
    }
}

void kd025egoin001_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint32_t color)
{
    uint16_t clipped_length;

    if ((length == 0U) || (x >= KD025EGOIN001_WIDTH) || (y >= KD025EGOIN001_HEIGHT)) {
        return;
    }

    clipped_length = length;
    if (((uint32_t)x + clipped_length) > KD025EGOIN001_WIDTH) {
        clipped_length = (uint16_t)(KD025EGOIN001_WIDTH - x);
    }
    (void)kd025egoin001_fill_rect(x, y, clipped_length, 1U, color);
}

void kd025egoin001_draw_rectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color)
{
    uint16_t temporary;

    if (x0 > x1) {
        temporary = x0;
        x0 = x1;
        x1 = temporary;
    }
    if (y0 > y1) {
        temporary = y0;
        y0 = y1;
        y1 = temporary;
    }

    kd025egoin001_draw_line(x0, y0, x1, y0, color);
    kd025egoin001_draw_line(x1, y0, x1, y1, color);
    kd025egoin001_draw_line(x1, y1, x0, y1, color);
    kd025egoin001_draw_line(x0, y1, x0, y0, color);
}

static void draw_pixel_clipped(int32_t x, int32_t y, uint32_t color)
{
    if ((x >= 0) && (x < (int32_t)KD025EGOIN001_WIDTH) && (y >= 0) && (y < (int32_t)KD025EGOIN001_HEIGHT)) {
        kd025egoin001_draw_pixel((uint16_t)x, (uint16_t)y, color);
    }
}

static void draw_span_clipped(int32_t x0, int32_t x1, int32_t y, uint32_t color)
{
    if ((y < 0) || (y >= (int32_t)KD025EGOIN001_HEIGHT) || (x1 < 0) || (x0 >= (int32_t)KD025EGOIN001_WIDTH)) {
        return;
    }

    if (x0 < 0) {
        x0 = 0;
    }
    if (x1 >= (int32_t)KD025EGOIN001_WIDTH) {
        x1 = (int32_t)KD025EGOIN001_WIDTH - 1;
    }
    (void)kd025egoin001_fill_rect((uint16_t)x0, (uint16_t)y, (uint16_t)(x1 - x0 + 1), 1U, color);
}

void kd025egoin001_draw_circle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint32_t color)
{
    int32_t x = 0;
    int32_t y = radius;
    int32_t decision = 1 - (int32_t)radius;
    const int32_t center_x_signed = center_x;
    const int32_t center_y_signed = center_y;

    while (x <= y) {
        draw_pixel_clipped(center_x_signed + x, center_y_signed + y, color);
        draw_pixel_clipped(center_x_signed - x, center_y_signed + y, color);
        draw_pixel_clipped(center_x_signed + x, center_y_signed - y, color);
        draw_pixel_clipped(center_x_signed - x, center_y_signed - y, color);
        draw_pixel_clipped(center_x_signed + y, center_y_signed + x, color);
        draw_pixel_clipped(center_x_signed - y, center_y_signed + x, color);
        draw_pixel_clipped(center_x_signed + y, center_y_signed - x, color);
        draw_pixel_clipped(center_x_signed - y, center_y_signed - x, color);

        ++x;
        if (decision < 0) {
            decision += (2 * x) + 1;
        } else {
            --y;
            decision += (2 * (x - y)) + 1;
        }
    }
}

void kd025egoin001_fill_circle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint32_t color)
{
    int32_t x = 0;
    int32_t y = radius;
    int32_t decision = 1 - (int32_t)radius;
    const int32_t center_x_signed = center_x;
    const int32_t center_y_signed = center_y;

    while (x <= y) {
        draw_span_clipped(center_x_signed - x, center_x_signed + x, center_y_signed + y, color);
        draw_span_clipped(center_x_signed - x, center_x_signed + x, center_y_signed - y, color);
        draw_span_clipped(center_x_signed - y, center_x_signed + y, center_y_signed + x, color);
        draw_span_clipped(center_x_signed - y, center_x_signed + y, center_y_signed - x, color);

        ++x;
        if (decision < 0) {
            decision += (2 * x) + 1;
        } else {
            --y;
            decision += (2 * (x - y)) + 1;
        }
    }
}

bool kd025egoin001_test(void)
{
    if (!kd025egoin001_clear(KD025EGOIN001_COLOR_BLACK)) {
        return false;
    }

    if (!kd025egoin001_fill_rect(20U, 20U, 110U, 120U, KD025EGOIN001_COLOR_RED) ||
        !kd025egoin001_fill_rect(145U, 20U, 110U, 120U, KD025EGOIN001_COLOR_GREEN) ||
        !kd025egoin001_fill_rect(270U, 20U, 110U, 120U, KD025EGOIN001_COLOR_BLUE)) {
        return false;
    }

    kd025egoin001_draw_hline(20U, 165U, 360U, KD025EGOIN001_COLOR_YELLOW);
    kd025egoin001_draw_hline(20U, 175U, 360U, KD025EGOIN001_COLOR_CYAN);
    kd025egoin001_draw_hline(20U, 185U, 360U, KD025EGOIN001_COLOR_MAGENTA);
    kd025egoin001_draw_rectangle(0U, 0U, KD025EGOIN001_WIDTH - 1U, KD025EGOIN001_HEIGHT - 1U,
                                 KD025EGOIN001_COLOR_WHITE);
    kd025egoin001_draw_rectangle(20U, 220U, 379U, 450U, KD025EGOIN001_COLOR_WHITE);
    kd025egoin001_draw_line(20U, 220U, 379U, 450U, KD025EGOIN001_COLOR_YELLOW);
    kd025egoin001_draw_line(379U, 220U, 20U, 450U, KD025EGOIN001_COLOR_CYAN);
    kd025egoin001_fill_circle(200U, 540U, 70U, KD025EGOIN001_COLOR_BLUE);
    kd025egoin001_draw_circle(200U, 540U, 70U, KD025EGOIN001_COLOR_WHITE);
    kd025egoin001_fill_circle(90U, 650U, 35U, KD025EGOIN001_COLOR_MAGENTA);
    kd025egoin001_fill_circle(310U, 650U, 35U, KD025EGOIN001_COLOR_CYAN);
    kd025egoin001_draw_pixel(200U, 690U, KD025EGOIN001_COLOR_YELLOW);

    return true;
}
