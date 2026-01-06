#include "HY_MOD/connectivity/fdcan/callback.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/connectivity/fdcan/pkt_read.h"
#include "HY_MOD/connectivity/fdcan/pkt_write.h"
#include "fdcan.h"

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    if (hfdcan == &hfdcan1)
    {
        if (ITS_CHECK(ErrorStatusITs, FDCAN_IT_BUS_OFF))
        {
            fdcan_bus_off = true;
        }
    }
}

void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs)
{
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

void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t BufferIndexes)
{
}

uint32_t hychk;
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    hychk++;
    if(ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
    {
        FDCAN_RxHeaderTypeDef header = {0};
        FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc());
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            hfdcan, FDCAN_RX_FIFO0, &header, pkt->data));
        pkt->id = header.Identifier;
        pkt->len = header.DataLength;
        RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(&fdcan_recv_pkt_buf, pkt, 0));
    }
}

ATTR_WEAK Result fdcan_pkt_ist_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    hychk++;
    if(ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
        FDCAN_RxHeaderTypeDef header = {0};
        FdcanPkt pkt = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            hfdcan, FDCAN_RX_FIFO1, &header, pkt.data));
        pkt.id = header.Identifier;
        pkt.len = header.DataLength;
        fdcan_pkt_ist_read(&pkt);
    }
}

#endif