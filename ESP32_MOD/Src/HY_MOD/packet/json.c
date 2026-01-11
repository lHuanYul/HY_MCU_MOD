#include "HY_MOD/packet/json.h"
#ifdef HY_MOD_ESP32_JSON

Result json_pkt_set_len(JsonPkt *pkt, uint16_t len)
{
    if (len > JSON_PKT_LEN) return RESULT_ERROR(RES_ERR_FULL);
    pkt->len = len;
    return RESULT_OK(pkt);
}

void json_pkt_pool_init(JsonPktPool *pool)
{
    pool->head = NULL;
    for (uint8_t i = 0; i < JSON_PKT_POOL_CAP; i++) {
        pool->pkt[i].next = pool->head;
        pool->head = &pool->pkt[i];
    }
    pool->remain = JSON_PKT_POOL_CAP;
}

Result json_pkt_pool_alloc(JsonPktPool *pool)
{
    if (
        pool->head == NULL ||
        pool->remain == 0
    ) while(1);
    JsonPkt *pkt = pool->head;
    pool->head = pkt->next;
    pkt->next = NULL;
    pool->remain--;
    return RESULT_OK(pkt);
}

void json_pkt_pool_free(JsonPktPool *pool, JsonPkt *pkt)
{
    if (pool->remain >= JSON_PKT_POOL_CAP) while(1);
    memset(pkt->data, 0, sizeof(pkt->data));
    pkt->len = 0;
    pkt->next = pool->head;
    pool->head = pkt;
    pool->remain++;
}

// 0:error 1:drop_first 2:drop_current
Result json_pkt_buf_push(JsonPktBuf* self, JsonPkt *pkt, JsonPktPool *pool, uint8_t drop)
{
    if (self->len >= self->cap)
    {
        switch (drop)
        {
            case 1:
            {
                JsonPkt *pkt_d = RESULT_UNWRAP_HANDLE(json_pkt_buf_pop(self));
                json_pkt_pool_free(pool, pkt_d);
                break;
            }
            case 2:
            {
                json_pkt_pool_free(pool, pkt);
                return RESULT_OK(self);
            }
            default: return RESULT_ERROR(RES_ERR_OVERFLOW);
        }
    }
    uint8_t tail = (self->head + self->len) % self->cap;
    self->buf[tail] = pkt;
    self->len++;
    return RESULT_OK(self);
}

Result json_pkt_buf_get(JsonPktBuf* self)
{
    if (self->len == 0) return RESULT_ERROR(RES_ERR_EMPTY);
    JsonPkt *pkt = self->buf[self->head];
    return RESULT_OK(pkt);
}

Result json_pkt_buf_pop(JsonPktBuf* self)
{
    JsonPkt *pkt = RESULT_UNWRAP_RET_RES(json_pkt_buf_get(self));
    if (--self->len == 0) self->head = 0;
    else self->head = (self->head + 1) % self->cap;
    return RESULT_OK(pkt);
}

#endif