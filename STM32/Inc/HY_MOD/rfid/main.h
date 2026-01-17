#pragma once
#include "HY_MOD/rfid/basic.h"
#ifdef HY_MOD_STM32_RFID

Result rfid_trcv_buf_setaddr(RfidTrcvBuf* trcv_buf, uint8_t sector, uint8_t block, uint8_t send);
Result rfid_trcv_buf_setdata(RfidTrcvBuf* trcv_buf, uint8_t id, uint8_t *data, uint8_t len);
Result rfid_buf_write(RC522Parametar *rfid);
Result rfid_buf_read(RC522Parametar *rfid);

#endif
