#include "HY_MOD/rfid/main.h"
#ifdef HY_MOD_STM32_RFID

Result rfid_trcv_buf_setaddr(RfidTrcvBuf *trcv_buf, uint8_t sector, uint8_t block, uint8_t send)
{
    if (
           sector >= 16
        || block >= 3
        || send > 2
    ) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    trcv_buf->sector = sector;
    trcv_buf->block = block;
    trcv_buf->send = send;
    return RESULT_OK(NULL);
}

Result rfid_trcv_buf_setdata(RfidTrcvBuf *trcv_buf, uint8_t id, uint8_t *data, uint8_t len)
{
    if (id + len > 16) return RESULT_ERROR(RES_ERR_OVERFLOW);
    memcpy(&trcv_buf->data[id], data, len);
    uint8_t i;
    for(i = 0; i < len; i++)
    {
        trcv_buf->flags |= ((uint16_t)1 << (id + i));
    }
    return RESULT_OK(NULL);
}

Result rfid_buf_write(RC522Parametar *rfid)
{
    RfidTrcvBuf *trcv_buf = &rfid->trsm_buf;
    if (trcv_buf->send == 0) return RESULT_ERROR(RES_ERR_INVALID);
    RC522_PCD_Authenticate(&rfid->const_h, PICC_CMD_MF_AUTH_KEY_A, trcv_buf->sector*4, &trcv_buf->key, &rfid->uid);
    if (RC522_MIFARE_Write(&rfid->const_h, (trcv_buf->sector * 4) + trcv_buf->block, trcv_buf->data, 16) != STATUS_Code_OK)
        return RESULT_ERROR(RES_ERR_FAIL);
    trcv_buf->send = 0;
    trcv_buf->flags = 0;
    return RESULT_OK(NULL);
}

Result rfid_buf_read(RC522Parametar *rfid)
{
    RfidTrcvBuf *trcv_buf = &rfid->recv_buf;
    RC522_PCD_Authenticate(&rfid->const_h, PICC_CMD_MF_AUTH_KEY_A, trcv_buf->sector*4, &trcv_buf->key, &rfid->uid);
    trcv_buf->size = 18;
    memset(trcv_buf->data, 0, trcv_buf->size);
    if (RC522_MIFARE_Read(&rfid->const_h, (trcv_buf->sector * 4) + trcv_buf->block, trcv_buf->data, &trcv_buf->size) != STATUS_Code_OK)
        return RESULT_ERROR(RES_ERR_FAIL);
    return RESULT_OK(NULL);
}

#endif