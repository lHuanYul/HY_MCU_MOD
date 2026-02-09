#include "HY_MOD/packet/raw_bytes.h"
#ifdef HY_MOD_STM32_RBYTES

#include "HY_MOD/main/buffer.h"

bool rbytes_pkt_check_len(RBytesPkt *pkt, uint8_t len)
{
    if (pkt->len < len) return 0;
    return 1;
}

Result rbytes_pkt_get_byte(RBytesPkt *pkt, uint8_t id, uint8_t* container)
{
    if (pkt->len <= id) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    *container = pkt->data[id];
    return RESULT_OK(container);
}

Result rbytes_pkt_set_len(RBytesPkt *pkt, uint8_t len)
{
    if (len > RBYTES_PKT_LEN) return RESULT_ERROR(RES_ERR_FULL);
    pkt->len = len;
    return RESULT_OK(pkt);
}

void rbytes_pkt_pool_init(RBytesPktPool *pool)
{
    pool->head = NULL;
    for (uint8_t i = 0; i < RBYTES_PKT_POOL_CAP; i++)
    {
        pool->pkt[i].data = rbytes_buffers[i];
        pool->pkt[i].next = pool->head;
        pool->head = &pool->pkt[i];
    }
    pool->remain = RBYTES_PKT_POOL_CAP;
}

Result rbytes_pkt_pool_alloc(RBytesPktPool *pool)
{
    if (
        pool->head == NULL ||
        pool->remain == 0
    ) while(1);
    RBytesPkt *pkt = pool->head;
    pool->head = pkt->next;
    pkt->next = NULL;
    pool->remain--;
    return RESULT_OK(pkt);
}

void rbytes_pkt_pool_free(RBytesPktPool *pool, RBytesPkt *pkt)
{
    if (pool->remain >= RBYTES_PKT_POOL_CAP) while(1);
    pkt->len = 0;
    pkt->next = pool->head;
    pool->head = pkt;
    pool->remain++;
}

// 0:error 1:drop_first 2:drop_current
Result rbytes_pkt_buf_push(RBytesPktBuf* self, RBytesPkt *pkt, RBytesPktPool *pool, uint8_t drop)
{
    if (self->len >= self->cap)
    {
        switch (drop)
        {
            case 1:
            {
                RBytesPkt *pkt_d = RESULT_UNWRAP_HANDLE(rbytes_pkt_buf_pop(self));
                rbytes_pkt_pool_free(pool, pkt_d);
                break;
            }
            case 2:
            {
                rbytes_pkt_pool_free(pool, pkt);
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

Result rbytes_pkt_buf_get(RBytesPktBuf* self)
{
    if (self->len == 0) return RESULT_ERROR(RES_ERR_EMPTY);
    RBytesPkt *pkt = self->buf[self->head];
    return RESULT_OK(pkt);
}

Result rbytes_pkt_buf_pop(RBytesPktBuf* self)
{
    RBytesPkt *pkt = RESULT_UNWRAP_RET_RES(rbytes_pkt_buf_get(self));
    if (--self->len == 0) self->head = 0;
    else self->head = (self->head + 1) % self->cap;
    return RESULT_OK(pkt);
}

#endif