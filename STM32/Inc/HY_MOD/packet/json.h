#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_JSON
#include "HY_MOD/main/fn_state.h"

typedef struct JsonPkt
{
    uint8_t     data[JSON_PKT_LEN + 1];
    uint16_t    len;
    struct JsonPkt *next;
} JsonPkt;
Result json_pkt_get_num(JsonPkt *pkt, char *id, uint64_t *container);
Result json_pkt_get_char(JsonPkt *pkt, char *id, char *container, uint16_t container_cap);
Result json_pkt_set_len(JsonPkt *pkt, uint16_t len);

typedef struct JsonPktPool
{
    JsonPkt     pkt[JSON_PKT_POOL_CAP];
    JsonPkt*    head;
    uint8_t     remain;
} JsonPktPool;
extern JsonPktPool json_pkt_pool;
void json_pkt_pool_init(JsonPktPool *pool);
Result json_pkt_pool_alloc(JsonPktPool *pool);
void json_pkt_pool_free(JsonPktPool *pool, JsonPkt *pkt);

typedef struct JsonPktBuf
{
    JsonPkt**   buf;
    uint8_t     head;
    uint8_t     len;
    uint8_t     cap;
} JsonPktBuf;
Result json_pkt_buf_push(JsonPktBuf* self, JsonPkt *pkt, JsonPktPool *pool, uint8_t drop);
Result json_pkt_buf_get(JsonPktBuf* self);
Result json_pkt_buf_pop(JsonPktBuf* self);

#endif