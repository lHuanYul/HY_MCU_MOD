#include "HY_MOD/fdcan/packet.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/buffer.h"

Result fdcan_pkt_get_byte(FdcanPkt *pkt, uint8_t id, uint8_t* container)
{
    if (pkt->len <= id) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    *container = pkt->data[id];
    return RESULT_OK(container);
}

inline void fdcan_pkt_set_id(FdcanPkt *pkt, uint32_t id)
{
    pkt->id = id;
}

Result fdcan_pkt_set_len(FdcanPkt *pkt, uint8_t len)
{
    if (len > FDCAN_PKT_LEN) return RESULT_ERROR(RES_ERR_FULL);
    pkt->len = len;
    return RESULT_OK(pkt);
}

#define RING_MASK (self->cap - 1)

// drop != 0 丟新的並回ERR
Result fdcan_ring_push(FdcanRing *self, FdcanPkt *pkt, uint8_t drop)
{
    if (drop && (self->in - self->out) >= self->cap)
        return RESULT_ERROR(RES_ERR_FULL);
    uint32_t real_idx = self->in & RING_MASK;
    self->buf[real_idx] = *pkt;
    __DMB();
    self->in++;
    return RESULT_OK(NULL);
}

Result fdcan_ring_pop(FdcanRing *self, FdcanPkt *pkt)
{
    uint32_t cur_in = self->in;
    uint32_t cur_out = self->out;
    if (cur_in == cur_out) return RESULT_ERROR(RES_ERR_EMPTY);
    // 無號數相減，免疫溢位
    if ((cur_in - cur_out) > self->cap) 
    {
        cur_out = cur_in - self->cap;
    }

    uint32_t real_idx = cur_out & RING_MASK;
    *pkt = self->buf[real_idx];
    __DMB();
    self->out = cur_out + 1;
    return RESULT_OK(NULL);
}

inline void fdcan_ring_clear(FdcanRing *self)
{
    // Lock-Free：不要歸零，只要追上。
    // 直接讓讀取指標 (out) 覆寫為當前的寫入指標 (in)。
    // 瞬間捨棄所有未讀取的資料，且不會與正在 Push 的發生指標衝突。
    self->out = self->in;
}

#endif