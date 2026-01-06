#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"
#include "HY_MOD/connectivity/cmds.h"

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
extern FdcanPktPool fdcan_pkt_pool;
void fdcan_pkt_pool_init(void);
Result fdcan_pkt_pool_alloc(void);
void fdcan_pkt_pool_free(FdcanPkt *pkt);

typedef struct FdcanPktBuf
{
    FdcanPkt**  buf;
    uint8_t     head;
    uint8_t     len;
    uint8_t     cap;
} FdcanPktBuf;
extern FdcanPktBuf fdcan_trsm_pkt_buf;
extern FdcanPktBuf fdcan_recv_pkt_buf;
Result fdcan_pkt_buf_push(FdcanPktBuf* self, FdcanPkt *pkt, uint8_t drop);
Result fdcan_pkt_buf_get(FdcanPktBuf* self);
Result fdcan_pkt_buf_pop(FdcanPktBuf* self);

extern bool fdcan_bus_off;
extern FncState fdacn_data_store;

#endif