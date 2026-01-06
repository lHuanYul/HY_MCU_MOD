#include "HY_MOD/connectivity/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

#include "main/variable_cal.h"

FdcanPktPool fdcan_pkt_pool;

bool fdcan_bus_off = false;

FncState fdacn_data_store = FNC_DISABLE;

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

Result fdcan_pkt_set_len(FdcanPkt *pkt, uint8_t len)
{
    if (len > FDCAN_PKT_LEN) return RESULT_ERROR(RES_ERR_FULL);
    pkt->len = len;
    return RESULT_OK(pkt);
}

void fdcan_pkt_pool_init(void)
{
    fdcan_pkt_pool.head = NULL;
    for (uint8_t i = 0; i < FDCAN_PKT_POOL_CAP; i++) {
        fdcan_pkt_pool.pkt[i].next = fdcan_pkt_pool.head;
        fdcan_pkt_pool.head = &fdcan_pkt_pool.pkt[i];
    }
    fdcan_pkt_pool.remain = FDCAN_PKT_POOL_CAP;
}

Result fdcan_pkt_pool_alloc(void)
{
    if (
        fdcan_pkt_pool.head == NULL ||
        fdcan_pkt_pool.remain == 0
    ) while(1);
    FdcanPkt *pkt = fdcan_pkt_pool.head;
    fdcan_pkt_pool.head = pkt->next;
    pkt->next = NULL;
    fdcan_pkt_pool.remain--;
    return RESULT_OK(pkt);
}

void fdcan_pkt_pool_free(FdcanPkt *pkt)
{
    if (fdcan_pkt_pool.remain >= FDCAN_PKT_POOL_CAP) while(1);
    memset(pkt->data, 0, sizeof(pkt->data));
    pkt->len = 0;
    pkt->next = fdcan_pkt_pool.head;
    fdcan_pkt_pool.head = pkt;
    fdcan_pkt_pool.remain++;
}

static FdcanPkt* trsm_pkt_buf[FDCAN_TRSM_BUF_CAP];
FdcanPktBuf fdcan_trsm_pkt_buf = {
    .buf = trsm_pkt_buf,
    .cap = FDCAN_TRSM_BUF_CAP,
};
static FdcanPkt* recv_pkt_buf[FDCAN_RECV_BUF_CAP];
FdcanPktBuf fdcan_recv_pkt_buf = {
    .buf = recv_pkt_buf,
    .cap = FDCAN_RECV_BUF_CAP,
};

Result fdcan_pkt_buf_push(FdcanPktBuf* self, FdcanPkt *pkt, uint8_t drop)
{
    if (self->len >= self->cap)
    {
        switch (drop)
        {
            case 1:
            {
                FdcanPkt *pkt_d = RESULT_UNWRAP_HANDLE(fdcan_pkt_buf_pop(self));
                fdcan_pkt_pool_free(pkt_d);
                break;
            }
            case 2:
            {
                fdcan_pkt_pool_free(pkt);
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

Result fdcan_pkt_buf_get(FdcanPktBuf* self)
{
    if (self->len == 0) return RESULT_ERROR(RES_ERR_EMPTY);
    FdcanPkt *pkt = self->buf[self->head];
    return RESULT_OK(pkt);
}

Result fdcan_pkt_buf_pop(FdcanPktBuf* self)
{
    FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_buf_get(self));
    if (--self->len == 0) self->head = 0;
    else self->head = (self->head + 1) % self->cap;
    return RESULT_OK(pkt);
}

#endif