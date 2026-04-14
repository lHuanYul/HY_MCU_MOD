#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/cmds.h"
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "HY_MOD/main/typedef.h"
#include "fdcan.h"

static uint32_t len_to_dlc(uint8_t len) 
{
    if (len <= 0)  return FDCAN_DLC_BYTES_0;
    if (len <= 1)  return FDCAN_DLC_BYTES_1;
    if (len <= 2)  return FDCAN_DLC_BYTES_2;
    if (len <= 3)  return FDCAN_DLC_BYTES_3;
    if (len <= 4)  return FDCAN_DLC_BYTES_4;
    if (len <= 5)  return FDCAN_DLC_BYTES_5;
    if (len <= 6)  return FDCAN_DLC_BYTES_6;
    if (len <= 7)  return FDCAN_DLC_BYTES_7;
    if (len <= 8)  return FDCAN_DLC_BYTES_8;
    if (len <= 12) return FDCAN_DLC_BYTES_12;
    if (len <= 16) return FDCAN_DLC_BYTES_16;
    if (len <= 20) return FDCAN_DLC_BYTES_20;
    if (len <= 24) return FDCAN_DLC_BYTES_24;
    if (len <= 32) return FDCAN_DLC_BYTES_32;
    if (len <= 48) return FDCAN_DLC_BYTES_48;
    return FDCAN_DLC_BYTES_64; // 大於 48 都當作 64
}

Result fdcan_pkt_transmit(FdcanParametar *fdcan)
{
    FdcanPkt *pkt = fdcan->tx_pkt;
    FDCAN_TxHeaderTypeDef header = {
        .IdType = FDCAN_EXTENDED_ID,
        .FDFormat = FDCAN_FD_CAN,
        .ErrorStateIndicator = FDCAN_ESI_PASSIVE,
        .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
        .Identifier = pkt->id,
        .DataLength = len_to_dlc(pkt->len),
    };
    ERROR_CHECK_HAL_RET_RES(
        HAL_FDCAN_AddMessageToTxFifoQ(fdcan->const_h.hfdcanx, &header, pkt->data)
    );
    return RESULT_OK(NULL);
}

Result fdcan_trsm_pkts_proc(FdcanParametar *fdcan, FdcanPktPool *pool, FdcanPktBuf *buf)
{
    if (fdcan->tx_pkt != NULL) return RESULT_ERROR(RES_ERR_BUSY);
    Result result = fdcan_pkt_buf_pop(buf);
    if (RESULT_CHECK_RAW(result)) return RESULT_OK(NULL);
    fdcan->tx_pkt = RESULT_UNWRAP_HANDLE(result);
    RESULT_CHECK_RET_RES(fdcan_pkt_transmit(fdcan));
    return RESULT_OK(NULL);
}

ATTR_WEAK Result fdcan_pkt_rcv_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
Result fdcan_recv_pkts_proc(FdcanPktPool *pool, FdcanPktBuf *buf, uint8_t count)
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