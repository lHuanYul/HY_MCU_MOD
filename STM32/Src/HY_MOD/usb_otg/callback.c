#include "HY_MOD/usb_otg/basic.h"
#ifdef HY_MOD_STM32_USB_OTG

#include "usbd_cdc_if.h"

void CDC_Receive_FS_OWN(UsbOtgParametar *usb_otg, uint8_t *Buf, uint32_t Len)
{
    if (Buf != usb_otg->rx_pkt->data) Error_Handler();
    rbytes_pkt_set_len(usb_otg->rx_pkt, Len);
    CDC_Transmit_FS_OWN(usb_otg);
    USBD_CDC_SetRxBuffer(usb_otg->const_h.usbd, usb_otg->rx_pkt->data);
}

void CDC_TransmitCplt_FS_OWN(UsbOtgParametar *usb_otg)
{

}

#endif