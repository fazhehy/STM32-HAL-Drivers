#include "cm7_main.h"

#include "delay.h"
#include "usbd_cdc_if.h"

extern "C" USBD_HandleTypeDef hUsbDeviceFS;

static void test_usb_cdc(void)
{
    static uint8_t message[] = "CM7 USB CDC OK\r\n";

    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
        (void)CDC_Transmit_FS(message, sizeof(message) - 1U);
    }
}

void cm7_main(void)
{
    for (;;) {
        test_usb_cdc();
        delay_ms(1000);
    }
}
