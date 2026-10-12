/*
// 前置条件：usb_device.c 里已按复合方式注册 CDC 类，且 usbd_cdc_if.c 的 CDC_Receive_FS
// 已经把收到的字节交给 usb_cdc_receive()。发送侧由调用方直接调 CDC_Transmit_FS()。
#include "usb_cdc.h"

#include "delay.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <stdio.h>

// 中断侧（usbd_cdc_if.c 的 CDC_Receive_FS 里）：只入队，不在中断里发送。
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t* Len)
{
    usb_cdc_receive(Buf, *Len);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}

// 主循环侧：取出来原样发回，顺便每秒打一行日志，证明串口和盘在同一个口上并存。
void cm7_main_usb_cdc(void)
{
    uint8_t buffer[64];
    uint32_t tick = 0U;
    uint32_t counter = 0U;

    for (;;) {
        // 主机发来的字节原样发回。
        uint32_t count = usb_cdc_read(buffer, sizeof(buffer));
        if (count > 0U && hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
            (void)CDC_Transmit_FS(buffer, (uint16_t)count);
        }

        // 丢字节说明主循环没跟上，把丢弃数带在日志里便于发现。
        if (++tick >= 100U) {
            tick = 0U;
            char line[48];
            int length = snprintf(line, sizeof(line), "CM7 USB CDC OK %lu drop=%lu\r\n",
                                  (unsigned long)counter++, (unsigned long)usb_cdc_get_dropped_count());
            if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
                (void)CDC_Transmit_FS((uint8_t*)line, (uint16_t)length);
            }
        }

        // 回环延迟上限就是一个循环周期。
        delay_ms(10);
    }
}
*/
