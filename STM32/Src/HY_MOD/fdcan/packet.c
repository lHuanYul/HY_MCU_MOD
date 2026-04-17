#include "HY_MOD/fdcan/packet.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/buffer.h"

bool fdcan_pkt_check_len(FdcanPkt *pkt, uint8_t len)
{
    if (pkt->len < len) return 0;
    return 1;
}

Result fdcan_pkt_get_byte(FdcanPkt *pkt, uint8_t id, uint8_t* container)
{
    if (pkt->len <= id) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    *container = pkt->data[id];
    return RESULT_OK(container);
}

Result fdcan_pkt_set_id(FdcanPkt *pkt, uint16_t id)
{
    pkt->id = id;
    return RESULT_OK(pkt);
}

Result fdcan_pkt_set_len(FdcanPkt *pkt, uint8_t len)
{
    if (len > FDCAN_PKT_LEN) return RESULT_ERROR(RES_ERR_FULL);
    pkt->len = len;
    return RESULT_OK(pkt);
}

static uint8_t pkts_pool[FDCAN_PKT_POOL_CAP][FDCAN_PKT_LEN];
void fdcan_pkt_pool_init(FdcanPktPool *pool)
{
    pool->head = NULL;
    for (uint8_t i = 0; i < FDCAN_PKT_POOL_CAP; i++)
    {
        pool->pkt[i].data = pkts_pool[i];
        pool->pkt[i].next = pool->head;
        pool->head = &pool->pkt[i];
    }
    pool->remain = FDCAN_PKT_POOL_CAP;
}

#define ACT_DISABLE_IRQ() \
    uint32_t primask = __get_PRIMASK(); \
    __disable_irq()
#define ACT_ENABLE_IRQ(res) \
    do { \
        __set_PRIMASK(primask); \
        return (res); \
    } while(0)

#define ACT_ENABLE_IRQ_V() \
    do { \
        __set_PRIMASK(primask); \
        return; \
    } while(0)

Result fdcan_pkt_pool_alloc(FdcanPktPool *pool)
{
    ACT_DISABLE_IRQ();
    if (
        pool->head == NULL ||
        pool->remain == 0
    ) Error_Handler();
    FdcanPkt *pkt = pool->head;
    pool->head = pkt->next;
    pool->remain--;
    pkt->next = NULL;
    pkt->state = 0;
    ACT_ENABLE_IRQ(RESULT_OK(pkt));
}

void fdcan_pkt_pool_free(FdcanPktPool *pool, FdcanPkt *pkt)
{
    ACT_DISABLE_IRQ();
    if (pool->remain >= FDCAN_PKT_POOL_CAP) Error_Handler();
    memset(pkt->data, 0, pkt->len);
    pkt->len = 0;
    pkt->next = pool->head;
    pool->head = pkt;
    pool->remain++;
    ACT_ENABLE_IRQ_V();
}

// drop=1 > oldest; drop=2 > newest
Result fdcan_pkt_buf_push(FdcanPktBuf* self, FdcanPkt *pkt, FdcanPktPool *pool, uint8_t drop)
{
    ACT_DISABLE_IRQ();
    if (self->len >= self->cap)
    {
        switch (drop)
        {
            case 1:
            {
                FdcanPkt *pkt_d = RESULT_UNWRAP_HANDLE(fdcan_pkt_buf_pop(self, 0));
                fdcan_pkt_pool_free(pool, pkt_d);
                break;
            }
            case 2:
            {
                fdcan_pkt_pool_free(pool, pkt);
                ACT_ENABLE_IRQ(RESULT_OK(self));
            }
            default:
            {
                Error_Handler();
            }
        }
    }
    uint8_t tail = (self->head + self->len) % self->cap;
    pkt->number = tail;
    self->buf[tail] = pkt;
    self->len++;
    ACT_ENABLE_IRQ(RESULT_OK(self));
}

Result fdcan_pkt_buf_get(FdcanPktBuf* self, uint8_t id)
{
    ACT_DISABLE_IRQ();

    if (self->len == 0) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_EMPTY));
    if (id >= self->len) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_NOT_FOUND));
    id = (self->head + id) % self->cap;
    FdcanPkt *pkt = self->buf[id];
    if (pkt == NULL) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_NOT_FOUND));

    ACT_ENABLE_IRQ(RESULT_OK(pkt));
}

Result fdcan_pkt_buf_trsm_get(FdcanPktBuf* self)
{
    ACT_DISABLE_IRQ();

    if (self->len == 0) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_EMPTY));
    uint8_t i;
    for (i = 0; i < self->len; i++)
    {
        uint8_t real_idx = (self->head + i) % self->cap;
        FdcanPkt *pkt = self->buf[real_idx];
        // 必須是非 NULL 且 尚未進入硬體 的封包
        if (pkt != NULL && pkt->state == 0)
        {
            ACT_ENABLE_IRQ(RESULT_OK(pkt));
        }
    }
    ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_NOT_FOUND));
}

Result fdcan_pkt_buf_pop(FdcanPktBuf* self, uint8_t id)
{
    ACT_DISABLE_IRQ();

    if (self->len == 0) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_EMPTY));
    if (id >= self->len) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_NOT_FOUND));
    uint8_t real_idx = (self->head + id) % self->cap;
    FdcanPkt *pkt = self->buf[real_idx];
    if (pkt == NULL) ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_NOT_FOUND));

    self->buf[real_idx] = NULL;
    if (id == 0)
    {
        // 如果 pop 的是頭部，就開始往後推，收斂所有為 NULL 的洞
        while (self->len > 0 && self->buf[self->head] == NULL)
        {
            self->head = (self->head + 1) % self->cap;
            self->len--;
        }
    }
    ACT_ENABLE_IRQ(RESULT_OK(pkt));
}

Result fdcan_pkt_buf_trsm_pop(FdcanPktBuf* self, FdcanPktPool *pool, uint8_t number)
{
    ACT_DISABLE_IRQ();
    bool found = false;
    // 遍歷當前 Buffer (使用相對 id)
    uint8_t id;
    for (id = 0; id < self->len; id++)
    {
        uint8_t real_idx = (self->head + id) % self->cap;
        
        // 找到非 NULL 且 number 匹配的封包
        if (self->buf[real_idx] != NULL && self->buf[real_idx]->number == number)
        {
            // 直接呼叫我們剛改好的 pop
            FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_buf_pop(self, id));
            fdcan_pkt_pool_free(pool, pkt);
            found = true;
            break;
        }
    }
    if (!found) 
    {
        // 如果找不到，代表可能早就被滿載 drop 機制丟掉了
        // 這裡可以選擇不報錯，或者印個 warning，就不會 Error_Handler 死機了
        ACT_ENABLE_IRQ(RESULT_ERROR(RES_ERR_NOT_FOUND));
    }
    ACT_ENABLE_IRQ(RESULT_OK(NULL));
}

#endif
