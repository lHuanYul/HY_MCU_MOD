#pragma once
#include "HY_MOD/rfid/MFRC522_STM32.h"
#ifdef HY_MOD_STM32_RFID

#include "HY_MOD/main/fn_state.h"

#define SECTOR_MASK(s)      ((uint16_t)1 << (s))

typedef enum CardState
{
    CARD_STATE_NONE,
    CARD_STATE_NEW,
    CARD_STATE_EXIST,
} CardState;

typedef struct RfidTrcvBuf
{
    uint8_t sector;
    uint8_t block;
    uint8_t data[18];
    uint8_t size;
    uint16_t flags;
    uint8_t send;
    RC522MIFARE_Key key;
} RfidTrcvBuf;

typedef struct RC522Parametar
{
    const RC522Const const_h;
    RC522Uid uid;
    uint32_t uid32;
    CardState state;
    uint8_t err_count;
    RfidTrcvBuf recv_buf;
    RfidTrcvBuf trsm_buf;
    bool new_card;
} RC522Parametar;

extern RC522Parametar rfid_h;

#endif