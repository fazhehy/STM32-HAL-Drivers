#include "usb_cdc.h"

#include <stddef.h>

// 接收侧环形缓冲：生产者在 USB 中断（CDC_Receive_FS），消费者在主循环，
// 单生产单消费，用 volatile 头尾指针即可，不需要临界区。
// 128 是 2 的幂，取模编译成掩码；头尾相等表示空，所以实际能存 127 字节。
// 不丢字节的办法是把反压交给 USB 本身：usb_cdc_receive 要么整包收下、要么一个不放，
// 调用方据返回值决定要不要重新挂 OUT 接收；空位不足时端点保持 NAK，主机的写会阻塞
// 等着，字节留在主机那边重发，而不是丢在设备里。
#define USB_CDC_RX_BUFFER_SIZE 128U

static volatile uint8_t rx_buffer[USB_CDC_RX_BUFFER_SIZE];
static volatile uint32_t rx_head;
static volatile uint32_t rx_tail;
static volatile uint32_t rx_dropped;

// 头尾之间还剩多少字节可写。留一个字节区分“空”和“满”，所以最大是 SIZE - 1。
static uint32_t free_space(void)
{
    return (rx_tail + USB_CDC_RX_BUFFER_SIZE - rx_head - 1U) % USB_CDC_RX_BUFFER_SIZE;
}

void usb_cdc_reset(void)
{
    rx_head = 0U;
    rx_tail = 0U;
}

bool usb_cdc_receive(const uint8_t* data, uint32_t length)
{
    if (data == NULL || length == 0U) {
        return false;
    }

    // 装不下就一个字节都不放：调用方收到 false 后不重新挂接收，主机重发这一包。
    // 所以 rx_dropped 正常应该恒为 0，它不为 0 说明挂接收时的空位判断漏了。
    if (length > free_space()) {
        rx_dropped += length;
        return false;
    }

    for (uint32_t index = 0U; index < length; ++index) {
        rx_buffer[rx_head] = data[index];
        rx_head = (rx_head + 1U) % USB_CDC_RX_BUFFER_SIZE;
    }

    return true;
}

uint32_t usb_cdc_read(uint8_t* buffer, uint32_t length)
{
    if (buffer == NULL) {
        return 0U;
    }

    uint32_t count = 0U;
    while (count < length && rx_tail != rx_head) {
        buffer[count] = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1U) % USB_CDC_RX_BUFFER_SIZE;
        ++count;
    }

    return count;
}

uint32_t usb_cdc_get_free_space(void)
{
    return free_space();
}

uint32_t usb_cdc_get_dropped_count(void)
{
    return rx_dropped;
}
