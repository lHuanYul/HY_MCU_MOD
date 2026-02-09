#pragma once
#include "HY_MOD/packet/basic.h"
#ifdef HY_MOD_STM32_RBYTES

#include "HY_MOD/main/fn_state.h"

typedef struct RBytesPkt
{
    uint8_t             *data;
    uint16_t            len;
    struct RBytesPkt    *next;
} RBytesPkt;
bool rbytes_pkt_check_len(RBytesPkt *pkt, uint8_t len);
Result rbytes_pkt_get_byte(RBytesPkt *pkt, uint8_t id, uint8_t* container);
Result rbytes_pkt_set_len(RBytesPkt *pkt, uint8_t len);

typedef struct RBytesPktPool
{
    RBytesPkt   pkt[RBYTES_PKT_POOL_CAP];
    RBytesPkt*  head;
    uint8_t     remain;
} RBytesPktPool;
void rbytes_pkt_pool_init(RBytesPktPool *pool);
Result rbytes_pkt_pool_alloc(RBytesPktPool *pool);
void rbytes_pkt_pool_free(RBytesPktPool *pool, RBytesPkt *pkt);

typedef struct RBytesPktBuf
{
    RBytesPkt** buf;
    uint8_t     head;
    uint8_t     len;
    uint8_t     cap;
} RBytesPktBuf;
Result rbytes_pkt_buf_push(RBytesPktBuf* self, RBytesPkt *pkt, RBytesPktPool *pool, uint8_t drop);
Result rbytes_pkt_buf_get(RBytesPktBuf* self);
Result rbytes_pkt_buf_pop(RBytesPktBuf* self);

#endif