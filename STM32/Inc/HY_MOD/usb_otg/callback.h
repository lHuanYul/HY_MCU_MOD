#pragma once
#include "HY_MOD/usb_otg/basic.h"
#ifdef HY_MOD_STM32_USB_OTG

void CDC_Receive_FS_OWN(UsbOtgParametar *usb_otg, uint8_t *Buf, uint32_t Len);
void CDC_TransmitCplt_FS_OWN(UsbOtgParametar *usb_otg);

#endif