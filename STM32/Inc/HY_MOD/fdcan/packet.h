#pragma once
#include "HY_MOD/packet/basic.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/fn_state.h"

typedef struct FdcanPkt
{
    uint8_t  idx;
    uint32_t id;
    uint8_t  data[FDCAN_PKT_LEN];
    uint8_t  len;
    struct FdcanPkt *next;
} FdcanPkt;
#define FDCAN_PKT_CHK_LEN(pkt, len) ((pkt)->len < (len))
Result fdcan_pkt_get_byte(FdcanPkt *pkt, uint8_t id, uint8_t* container);
void fdcan_pkt_set_id(FdcanPkt *pkt, uint32_t id);
Result fdcan_pkt_set_len(FdcanPkt *pkt, uint8_t len);

typedef struct FdcanRing
{
    FdcanPkt *buf;
    uint8_t  cap;
    volatile uint32_t out;
    volatile uint32_t in;
} FdcanRing;
Result fdcan_ring_push(FdcanRing *self, FdcanPkt *pkt, uint8_t drop);
Result fdcan_ring_pop(FdcanRing *self, FdcanPkt *pkt);
void fdcan_ring_clear(FdcanRing *self);

#endif