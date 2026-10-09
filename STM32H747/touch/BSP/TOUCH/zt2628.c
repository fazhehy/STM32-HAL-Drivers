#include "zt2628.h"

#include "delay.h"
#include "log.h"

#include <stdbool.h>
#include <stddef.h>

#define ZT2628_ADDRESS_WRITE 0x40U
#define ZT2628_ADDRESS_READ 0x41U

static const zt2628_pins_t* touch_pins;

static void set_scl(GPIO_PinState state)
{
    HAL_GPIO_WritePin(touch_pins->scl_port, touch_pins->scl_pin, state);
}

static void set_sda(GPIO_PinState state)
{
    HAL_GPIO_WritePin(touch_pins->sda_port, touch_pins->sda_pin, state);
}

static void bus_delay(void)
{
    delay_us(2);
}

static void bus_start(void)
{
    set_sda(GPIO_PIN_SET);
    set_scl(GPIO_PIN_SET);
    bus_delay();
    set_sda(GPIO_PIN_RESET);
    bus_delay();
    set_scl(GPIO_PIN_RESET);
    bus_delay();
}

static void bus_stop(void)
{
    set_sda(GPIO_PIN_RESET);
    bus_delay();
    set_scl(GPIO_PIN_SET);
    bus_delay();
    set_sda(GPIO_PIN_SET);
    bus_delay();
}

static bool bus_write_byte(uint8_t value)
{
    for (uint8_t bit = 0; bit < 8U; bit++) {
        set_sda((value & 0x80U) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET);
        bus_delay();
        set_scl(GPIO_PIN_SET);
        bus_delay();
        set_scl(GPIO_PIN_RESET);
        value <<= 1;
    }

    set_sda(GPIO_PIN_SET);
    bus_delay();
    set_scl(GPIO_PIN_SET);
    for (uint16_t attempt = 0; attempt < 250U; attempt++) {
        if (HAL_GPIO_ReadPin(touch_pins->sda_port, touch_pins->sda_pin) == GPIO_PIN_RESET) {
            set_scl(GPIO_PIN_RESET);
            bus_delay();
            return true;
        }
        bus_delay();
    }
    set_scl(GPIO_PIN_RESET);
    return false;
}

static uint8_t bus_read_byte(bool acknowledge)
{
    uint8_t value = 0;

    set_sda(GPIO_PIN_SET);
    for (uint8_t bit = 0; bit < 8U; bit++) {
        value <<= 1;
        set_scl(GPIO_PIN_SET);
        bus_delay();
        if (HAL_GPIO_ReadPin(touch_pins->sda_port, touch_pins->sda_pin) == GPIO_PIN_SET) {
            value |= 1U;
        }
        set_scl(GPIO_PIN_RESET);
        bus_delay();
    }

    set_sda(acknowledge ? GPIO_PIN_RESET : GPIO_PIN_SET);
    bus_delay();
    set_scl(GPIO_PIN_SET);
    bus_delay();
    set_scl(GPIO_PIN_RESET);
    set_sda(GPIO_PIN_SET);
    bus_delay();
    return value;
}

zt2628_touch_state_t zt2628_touch = {
    .x = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
    .y = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF},
    .sta = 0,
    .put_up = 1,
};

uint8_t zt2628_write_register(uint16_t reg, const uint8_t* buf, uint8_t len)
{
    if (touch_pins == NULL || (len != 0U && buf == NULL)) {
        return 1;
    }

    // ZT2628 的两次事务之间至少留 20 us；字节时序与 GT9xxx 相同。
    delay_us(20);
    bus_start();
    if (!bus_write_byte(ZT2628_ADDRESS_WRITE) || !bus_write_byte((uint8_t)reg) ||
        !bus_write_byte((uint8_t)(reg >> 8))) {
        bus_stop();
        return 1;
    }
    for (uint8_t index = 0; index < len; index++) {
        if (!bus_write_byte(buf[index])) {
            bus_stop();
            return 1;
        }
    }
    bus_stop();
    return 0;
}

uint8_t zt2628_write_command(uint16_t reg)
{
    return zt2628_write_register(reg, NULL, 0);
}

uint8_t zt2628_read_register(uint16_t reg, uint8_t* buf, uint8_t len)
{
    if (touch_pins == NULL || buf == NULL || len == 0U) {
        return 1;
    }

    delay_us(20);
    bus_start();
    if (!bus_write_byte(ZT2628_ADDRESS_WRITE) || !bus_write_byte((uint8_t)reg) ||
        !bus_write_byte((uint8_t)(reg >> 8))) {
        bus_stop();
        return 1;
    }
    bus_stop();
    delay_us(50);

    // ZT2628 的寄存器地址写入与数据读取分为两个事务。
    bus_start();
    if (!bus_write_byte(ZT2628_ADDRESS_READ)) {
        bus_stop();
        return 1;
    }
    for (uint8_t index = 0; index < len; index++) {
        buf[index] = bus_read_byte(index + 1U < len);
    }
    bus_stop();
    return 0;
}

uint8_t zt2628_init(const zt2628_pins_t* pins)
{
    uint8_t temp[6];
    uint16_t chip_code;
    uint16_t x_max;
    uint16_t y_max;

    if (pins == NULL || pins->scl_port == NULL || pins->sda_port == NULL || pins->reset_port == NULL) {
        return 1;
    }
    touch_pins = pins;
    set_sda(GPIO_PIN_SET);
    set_scl(GPIO_PIN_SET);

    // 按 RESETn 时序先保持高电平 20 ms，再拉低 100 ms，释放后等待 50 ms。
    HAL_GPIO_WritePin(pins->reset_port, pins->reset_pin, GPIO_PIN_SET);
    delay_ms(20);
    HAL_GPIO_WritePin(pins->reset_port, pins->reset_pin, GPIO_PIN_RESET);
    delay_ms(100);
    HAL_GPIO_WritePin(pins->reset_port, pins->reset_pin, GPIO_PIN_SET);
    delay_ms(50);

    temp[0] = 0x01;
    temp[1] = 0x00;
    if (zt2628_write_register(0xC000, temp, 2) != 0) {
        log_error("ZT2628 0xC000 failed");
        return 1;
    }
    delay_ms(10);

    if (zt2628_write_command(0xC004) != 0) {
        log_error("ZT2628 0xC004 failed");
        return 1;
    }
    delay_ms(10);

    if (zt2628_write_register(0xC002, temp, 2) != 0) {
        log_error("ZT2628 0xC002 failed");
        return 1;
    }
    delay_ms(10);

    if (zt2628_write_register(0xC001, temp, 2) != 0) {
        log_error("ZT2628 0xC001 failed");
        return 1;
    }
    delay_ms(10);

    if (zt2628_read_register(0xCC00, temp, 2) != 0) {
        log_error("ZT2628 read chip code failed");
        return 1;
    }
    chip_code = (uint16_t)temp[0] | ((uint16_t)temp[1] << 8);
    log_info("ZT2628 chip code=0x%04X", (unsigned int)chip_code);

    if (zt2628_read_register(0x00C0, temp, 2) != 0) {
        log_error("ZT2628 read TP_X failed");
        return 1;
    }
    x_max = (uint16_t)temp[0] | ((uint16_t)temp[1] << 8);
    log_info("ZT2628 TP_X=%u", (unsigned int)x_max);

    if (zt2628_read_register(0x00C1, temp, 2) != 0) {
        log_error("ZT2628 read TP_Y failed");
        return 1;
    }
    y_max = (uint16_t)temp[0] | ((uint16_t)temp[1] << 8);
    log_info("ZT2628 TP_Y=%u", (unsigned int)y_max);

    for (uint8_t attempt = 0; attempt < 3U; attempt++) {
        if (zt2628_write_command(0x0003) != 0) {
            log_error("ZT2628 clear interrupt failed");
            return 1;
        }
    }
    return 0;
}

uint8_t zt2628_scan(uint8_t mode)
{
    uint8_t buf[36];
    uint8_t i = 0;
    uint8_t point_num = 0;
    uint8_t result = 0;
    uint8_t temp;

    if (zt2628_read_register(0x0080, buf, 32) != 0) {
        return 0;
    }
    if (zt2628_write_command(0x0003) != 0 || zt2628_write_command(0x0003) != 0 || zt2628_write_command(0x0003) != 0) {
        return 0;
    }

    mode = buf[2];
    point_num = buf[2] & 0x0F;

    if (point_num != 0 && point_num <= ZT2628_MAX_TOUCHES) {
        temp = 0xFF << (mode & 0x0F);
        zt2628_touch.sta = (~temp) | ZT2628_TP_PRES_DOWN | ZT2628_TP_CATH_PRES;

        for (i = 0; i < point_num; ++i) {
            zt2628_touch.x[i] = buf[i * 6 + 4] + (buf[i * 6 + 5] << 8);
            zt2628_touch.y[i] = buf[i * 6 + 6] + (buf[i * 6 + 7] << 8);
        }
        result = 1;
        if (zt2628_touch.x[0] == 0 && zt2628_touch.y[0] == 0) {
            mode = 0;
        }
    }

    if ((mode & 0x1F) == 0) {
        if ((zt2628_touch.sta & ZT2628_TP_PRES_DOWN) != 0) {
            zt2628_touch.sta &= ~ZT2628_TP_PRES_DOWN;
        } else {
            zt2628_touch.x[0] = 0xFFFF;
            zt2628_touch.y[0] = 0xFFFF;
            zt2628_touch.sta &= 0xE0;
        }
    }

    return result;
}

void zt2628_scan_point(uint8_t mode)
{
    uint8_t buf[33];
    uint8_t i = 0;

    if (ZT2628_INT_IS_LOW()) {
        if (zt2628_read_register(0x0080, buf, 17) != 0) {
            return;
        }
        if (zt2628_write_command(0x0003) != 0 || zt2628_write_command(0x0003) != 0 ||
            zt2628_write_command(0x0003) != 0) {
            return;
        }

        mode = buf[2];
        if ((mode & 0x0F) != 0 && (mode & 0x0F) < 6) {
            zt2628_touch.x[i] = buf[i * 6 + 4] + (buf[i * 6 + 5] << 8);
            zt2628_touch.y[i] = buf[i * 6 + 6] + (buf[i * 6 + 7] << 8);
            zt2628_touch.put_up = 0;
        }
    } else {
        zt2628_touch.x[0] = 0xFFFF;
        zt2628_touch.y[0] = 0xFFFF;
        zt2628_touch.put_up = 1;
    }
}
