/* 保护宏不能叫 __USB_CDC_H：ST 的 usbd_cdc.h 用的就是它，而 usbd_cdc_if.h 先包含了那个头，
   于是本文件会被整段跳过，只报 usb_cdc_receive 隐式声明，看不出是宏撞了。 */
#ifndef __USB_CDC_BSP_H
#define __USB_CDC_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/* USB 中断里调用：整包收下返回 true；装不下则一个字节都不放并返回 false。 */
bool usb_cdc_receive(const uint8_t* data, uint32_t length);

/* 主循环里调用，返回实际读出的字节数。 */
uint32_t usb_cdc_read(uint8_t* buffer, uint32_t length);

/* 还能放多少字节。CDC 侧用它决定 OUT 接收能不能重新挂上：不足一整包就先不挂，
   端点保持 NAK，主机的写会阻塞等着，字节不会丢在设备里。 */
uint32_t usb_cdc_get_free_space(void);

/* 重新配置（每次 SET_CONFIGURATION / 总线复位）时清空，免得把上一次会话的残留字节发回去。 */
void usb_cdc_reset(void);

/* 因缓冲装不下而被退回的字节数。按上面的约定挂接收时它应该恒为 0。 */
uint32_t usb_cdc_get_dropped_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_CDC_BSP_H */
