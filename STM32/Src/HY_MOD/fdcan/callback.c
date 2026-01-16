#include "HY_MOD/fdcan/callback.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "fdcan.h"

void fdcan_error_status_cb(FdcanParametar *fdcan, FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    if (ITS_CHECK(ErrorStatusITs, FDCAN_IT_BUS_OFF))
    {
        fdcan->bus_off = true;
    }
}

void fdcan_tx_fifo_cb(FdcanParametar *fdcan, FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs)
{
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_NEW_DATA))
    {
    }
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_FULL))
    {
        FDCAN_TxEventFifoTypeDef txEvent;
        HAL_FDCAN_GetTxEvent(hfdcan, &txEvent);
    }
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_ELT_LOST))
    {
        Error_Handler();
    }
}

void fdcan_rx_fifo0_cb(
    FdcanParametar *fdcan,
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t RxFifo0ITs,
    FdcanPktPool *pool,
    FdcanPktBuf *buf
) {
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    if(ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
    {
        memset(&fdcan->rx_header, 0, sizeof(fdcan->rx_header));
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            hfdcan, FDCAN_RX_FIFO0, &fdcan->rx_header, fdcan->rx_buf));
        FdcanPkt *pkt = RESULT_UNWRAP_RET_VOID(fdcan_pkt_pool_alloc(pool));
        RESULT_CHECK_RET_VOID(fdcan_pkt_set_len(pkt, fdcan->rx_header.DataLength));
        pkt->id = fdcan->rx_header.Identifier;
        memcpy(pkt->data, fdcan->rx_buf, pkt->len);
        RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(buf, pkt, pool, 0));
    }
}

__attribute__((weak)) Result fdcan_pkt_ist_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
void fdcan_rx_fifo1_cb(FdcanParametar *fdcan, FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    if(ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
        memset(&fdcan->rx_header, 0, sizeof(fdcan->rx_header));
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            hfdcan, FDCAN_RX_FIFO0, &fdcan->rx_header, fdcan->rx_buf));
        FdcanPkt pkt = {0};
        RESULT_CHECK_RET_VOID(fdcan_pkt_set_len(&pkt, fdcan->rx_header.DataLength));
        pkt.id = fdcan->rx_header.Identifier;
        memcpy(pkt.data, fdcan->rx_buf, pkt.len);
        fdcan_pkt_ist_read(&pkt);
    }
}

#endif