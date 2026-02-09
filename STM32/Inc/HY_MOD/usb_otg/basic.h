#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_USB_OTG

#include "usbd_def.h"
#include "HY_MOD/packet/raw_bytes.h"

typedef struct UsbOtgConst
{
    USBD_HandleTypeDef *usbd;
} UsbOtgConst;

typedef struct UsbOtgParametar
{
    UsbOtgConst const_h;
    RBytesPkt *rx_pkt;
    RBytesPkt *tx_pkt;
} UsbOtgParametar;

extern UsbOtgParametar usb_otg_h;

uint8_t CDC_Transmit_FS_OWN(UsbOtgParametar *usb_otg);

#endif