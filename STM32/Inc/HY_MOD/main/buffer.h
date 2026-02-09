#pragma once
#include "main/config.h"

#ifdef STM32H7
    #ifndef ALIGN_32
        #define ALIGN_32(x)  ((((x) + 31) / 32) * 32)
    #endif
    #define ATTR_RAM_D1_ALIGN_32 __attribute__((section(".RAM_D1"), aligned(32)))
#else
    #define ATTR_RAM_D1_ALIGN_32
    #define ALIGN_32(x) x
#endif

#ifdef HY_MOD_STM32_FDCAN
extern uint8_t fdcan_buffers[FDCAN_PKT_POOL_CAP][FDCAN_PKT_LEN];
#endif

#ifdef HY_MOD_STM32_JSON
extern uint8_t json_buffers[JSON_PKT_POOL_CAP][ALIGN_32(JSON_PKT_LEN)];
#endif

#ifdef HY_MOD_STM32_USB_OTG
#include "usbd_cdc_if.h"
extern uint8_t rbytes_buffers[RBYTES_PKT_POOL_CAP][ALIGN_32(RBYTES_PKT_LEN)];
#endif
