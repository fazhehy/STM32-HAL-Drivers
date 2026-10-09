#ifndef __ZT2628_H
#define __ZT2628_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx_hal.h"

#include <stdint.h>

#define ZT2628_INT_PORT GPIOB
#define ZT2628_INT_PIN GPIO_PIN_5

#define ZT2628_INT_IS_LOW() (HAL_GPIO_ReadPin(ZT2628_INT_PORT, ZT2628_INT_PIN) == GPIO_PIN_RESET)

#define ZT2628_MAX_TOUCHES 5
#define ZT2628_TP_PRES_DOWN 0x80
#define ZT2628_TP_CATH_PRES 0x40

typedef struct {
    GPIO_TypeDef* scl_port;
    uint16_t scl_pin;
    GPIO_TypeDef* sda_port;
    uint16_t sda_pin;
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
} zt2628_pins_t;

typedef struct {
    uint16_t x[ZT2628_MAX_TOUCHES];
    uint16_t y[ZT2628_MAX_TOUCHES];
    uint8_t sta;
    uint8_t put_up;
} zt2628_touch_state_t;

extern zt2628_touch_state_t zt2628_touch;

uint8_t zt2628_write_register(uint16_t reg, const uint8_t* buf, uint8_t len);
uint8_t zt2628_write_command(uint16_t reg);
uint8_t zt2628_read_register(uint16_t reg, uint8_t* buf, uint8_t len);
uint8_t zt2628_init(const zt2628_pins_t* pins);
uint8_t zt2628_scan(uint8_t mode);
void zt2628_scan_point(uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif // __ZT2628_H
