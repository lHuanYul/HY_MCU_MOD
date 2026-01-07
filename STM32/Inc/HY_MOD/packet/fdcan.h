#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/main/fn_state.h"

typedef struct FdcanPkt
{
    uint32_t id;
    uint8_t data[FDCAN_PKT_LEN];
    uint8_t len;
    struct FdcanPkt *next;
} FdcanPkt;
bool fdcan_pkt_check_len(FdcanPkt *pkt, uint8_t len);
Result fdcan_pkt_get_byte(FdcanPkt *pkt, uint8_t id, uint8_t* container);
Result fdcan_pkt_set_len(FdcanPkt *pkt, uint8_t len);

typedef struct FdcanPktPool
{
    FdcanPkt    pkt[FDCAN_PKT_POOL_CAP];
    FdcanPkt*   head;
    uint8_t     remain;
} FdcanPktPool;
void fdcan_pkt_pool_init(FdcanPktPool *pool);
Result fdcan_pkt_pool_alloc(FdcanPktPool *pool);
void fdcan_pkt_pool_free(FdcanPktPool *pool, FdcanPkt *pkt);

typedef struct FdcanPktBuf
{
    FdcanPkt**  buf;
    uint8_t     head;
    uint8_t     len;
    uint8_t     cap;
} FdcanPktBuf;
Result fdcan_pkt_buf_push(FdcanPktBuf* self, FdcanPkt *pkt, FdcanPktPool *pool, uint8_t drop);
Result fdcan_pkt_buf_get(FdcanPktBuf* self);
Result fdcan_pkt_buf_pop(FdcanPktBuf* self);

#endif