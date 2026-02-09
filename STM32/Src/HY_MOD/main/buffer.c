#include "HY_MOD/main/buffer.h"

#ifdef HY_MOD_STM32_FDCAN
uint8_t fdcan_buffers[FDCAN_PKT_POOL_CAP][FDCAN_PKT_LEN];
#endif

#ifdef HY_MOD_STM32_JSON
ATTR_RAM_D1_ALIGN_32 uint8_t json_buffers[JSON_PKT_POOL_CAP][ALIGN_32(JSON_PKT_LEN)];
#endif

#ifdef HY_MOD_STM32_USB_OTG
ATTR_RAM_D1_ALIGN_32 uint8_t rbytes_buffers[RBYTES_PKT_POOL_CAP][ALIGN_32(RBYTES_PKT_LEN)];
#endif
