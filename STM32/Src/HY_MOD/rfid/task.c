#include "HY_MOD/rfid/main.h"
#ifdef HY_MOD_STM32_RFID

#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/main/variable_cal.h"

void StartRfidTask(void *argument)
{
    RC522Parametar *rfid = &rfid_h;
    RC522_PCD_Init(&rfid->const_h);
    memcpy(&rfid->trsm_buf.key, &rc522_default_key, sizeof(RC522MIFARE_Key));
    memcpy(&rfid->recv_buf.key, &rc522_default_key, sizeof(RC522MIFARE_Key));
    // if (RC522_PCD_PerformSelfTest(&rfid->const_h)) {};
    // Result result;
    for(;;)
    {
        switch (rfid->state)
        {
            case CARD_STATE_NONE:
            {
                if (
                       !RC522_PICC_IsNewCardPresent(&rfid->const_h)
                    || !RC522_PICC_ReadCardSerial(&rfid->const_h)
                ) break;
                rfid->state = CARD_STATE_EXIST;
                rfid->err_count = 0;
                memcpy(&rfid->uid, &rc522_uid, sizeof(RC522Uid));
                uint32_t new_id = var_u8_to_u32_be(rfid->uid.uidByte);
                if (rfid->uid32 != new_id)
                {
                    rfid->new_card = 1;
                    rfid->uid32 = new_id;
                }
                RC522_PICC_HaltA(&rfid->const_h);
                // simple_point_select(rfid->uid32);
                // VecByte vec_byte;
                // RESULT_CHECK_CLEANUP(vec_byte_new(&vec_byte, FDCAN_VEC_BYTE_CAP));
                // RESULT_CHECK_CLEANUP(pkt_map_info(&vec_byte, rfid->uid32, 0));
                // RESULT_CHECK_CLEANUP(fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, &vec_byte, 0x11));
                // cleanup:
                // vec_byte_free(&vec_byte);
                break;
            }
            case CARD_STATE_EXIST:
            {
                uint8_t atqa_answer[2];
	            uint8_t atqa_size = 2;
                if (RC522_PICC_WakeupA(&rfid->const_h, atqa_answer, &atqa_size) != STATUS_Code_OK)
                {
                    if (++rfid->err_count > 9)
                    {
                        RC522_PICC_HaltA(&rfid->const_h);
                        RC522_PCD_StopCrypto1(&rfid->const_h);
                        rfid->state = CARD_STATE_NONE;
                    }
                    break;
                }
                rfid->err_count = 0;
                rfid_buf_write(&rfid_h);
                rfid->recv_buf.sector = rfid->trsm_buf.sector;
                rfid->recv_buf.block  = rfid->trsm_buf.block;
                rfid_buf_read(&rfid_h);
                RC522_PICC_HaltA(&rfid->const_h);
                break;
            }
            default: break;
        }
        osDelay(50);
    }
}

#endif