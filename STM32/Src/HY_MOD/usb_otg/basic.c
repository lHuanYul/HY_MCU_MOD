#include "HY_MOD/usb_otg/basic.h"
#ifdef HY_MOD_STM32_USB_OTG

#include "usbd_cdc_if.h"

UsbOtgParametar usb_otg_h = {
    .const_h = {
        .usbd = &hUsbDeviceFS,
    },
};

uint8_t CDC_Transmit_FS_OWN(UsbOtgParametar *usb_otg)
{
    return CDC_Transmit_FS(usb_otg->tx_pkt->data, usb_otg->tx_pkt->len);
}

#endif