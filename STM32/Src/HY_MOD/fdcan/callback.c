#include "HY_MOD/fdcan/callback.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "fdcan.h"

void fdcan_error_status_cb(
	FdcanParametar *fdcan,
	FDCAN_HandleTypeDef *hfdcan,
	uint32_t ErrorStatusITs
) {
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    if (ITS_CHECK(ErrorStatusITs, FDCAN_IT_BUS_OFF))
    {
        fdcan->state = FDCAN_STATE_BUS_OFF;
    }
}
	
void fdcan_tx_fifo_cb(
	FdcanParametar *fdcan,
	FDCAN_HandleTypeDef *hfdcan,
	uint32_t TxEventFifoITs,
    FdcanPktPool *pool,
    FdcanPktBuf *buf
) {
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    fdcan->trsming++;
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_NEW_DATA))
    {
        FDCAN_TxEventFifoTypeDef txEvent;
        HAL_FDCAN_GetTxEvent(hfdcan, &txEvent);
		fdcan_pkt_pool_free(pool, fdcan->tx_pkt);
		fdcan->tx_pkt = NULL;
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
    fdcan->recving++;
    if(ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
    {
        FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(pool));
		FDCAN_RxHeaderTypeDef header = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            hfdcan, FDCAN_RX_FIFO0, &header, pkt->data));
        RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, header.DataLength));
		fdcan_pkt_set_id(pkt, header.Identifier);
        RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(buf, pkt, pool, 0));
    }
	if (ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_MESSAGE_LOST))
	{
        Error_Handler();
	}
}

ATTR_WEAK Result fdcan_pkt_ist_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
void fdcan_rx_fifo1_cb(
	FdcanParametar *fdcan,
	FDCAN_HandleTypeDef *hfdcan,
	uint32_t RxFifo1ITs,
    FdcanPktPool *pool,
    FdcanPktBuf *buf
) {
    if (hfdcan != fdcan->const_h.hfdcanx) return;
    fdcan->recving++;
    if(ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
		FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(pool));
		FDCAN_RxHeaderTypeDef header = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            hfdcan, FDCAN_RX_FIFO1, &header, pkt->data));
        RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, header.DataLength));
		fdcan_pkt_set_id(pkt, header.Identifier);
        fdcan_pkt_ist_read(pkt);
		fdcan_pkt_pool_free(pool, pkt);
    }
	if (ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_MESSAGE_LOST))
	{
        Error_Handler();
	}
}

#endif