#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/cmds.h"
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "fdcan.h"

Result fdcan_pkt_transmit(FdcanPkt *pkt)
{
    FDCAN_TxHeaderTypeDef header = {
        .ErrorStateIndicator = FDCAN_ESI_PASSIVE,
        .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
    };
    header.Identifier = pkt->id;
    header.DataLength = pkt->len;
    ERROR_CHECK_HAL_RET_RES(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &header, pkt->data));
    return RESULT_OK(NULL);
}

Result trsm_pkts_proc(FdcanPktPool *pool, FdcanPktBuf *buf)
{
    Result result = fdcan_pkt_buf_get(buf);
    if (RESULT_CHECK_RAW(result)) return RESULT_OK(NULL);
    FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(result);
    RESULT_CHECK_RET_RES(fdcan_pkt_transmit(pkt));
    fdcan_pkt_buf_pop(buf);
    fdcan_pkt_pool_free(pool, pkt);
    return RESULT_OK(NULL);
}

ATTR_WEAK Result fdcan_pkt_rcv_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
Result recv_pkts_proc(FdcanPktPool *pool, FdcanPktBuf *buf, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_buf_pop(buf));
        fdcan_pkt_rcv_read(pkt);
        fdcan_pkt_pool_free(pool, pkt);
    }
    return RESULT_OK(NULL);
}

#endif