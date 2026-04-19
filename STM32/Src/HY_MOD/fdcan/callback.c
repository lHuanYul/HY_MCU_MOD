#include "HY_MOD/fdcan/callback.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "fdcan.h"

void fdcan_error_status_cb(FdcanParametar *fdcan, uint32_t ErrorStatusITs)
{
    if (ITS_CHECK(ErrorStatusITs, FDCAN_IT_BUS_OFF))
    {
        fdcan->state = FDCAN_STATE_BUS_OFF;
    }
}
	
void fdcan_tx_fifo_cb(FdcanParametar *fdcan, uint32_t TxEventFifoITs)
{
    fdcan->alive_tick = HAL_GetTick();
    fdcan->tx_cb++;
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_NEW_DATA))
    {
        FDCAN_TxEventFifoTypeDef txEvent;
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetTxEvent(fdcan->const_h.hfdcanx, &txEvent));
        
        RESULT_CHECK_HANDLE(fdcan_tx_push(fdcan));
    }
    else if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_ELT_LOST))
    {
        fdcan->tx_lost++;
    }
}

void fdcan_rx_fifo0_cb(FdcanParametar *fdcan, uint32_t RxFifo0ITs)
{
    fdcan->alive_tick = HAL_GetTick();
    fdcan->rx_cb++;
    if(ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
    {
        FdcanPkt pkt = {0};
		FDCAN_RxHeaderTypeDef header = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            fdcan->const_h.hfdcanx, FDCAN_RX_FIFO0, &header, pkt.data));
        RESULT_CHECK_HANDLE(fdcan_pkt_set_len(&pkt, header.DataLength));
		fdcan_pkt_set_id(&pkt, header.Identifier);
        RESULT_CHECK_HANDLE(fdcan_ring_push(&fdcan->rx_buf, &pkt, 0));
    }
	if (ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_MESSAGE_LOST))
	{
        Error_Handler();
	}
}

ATTR_WEAK Result fdcan_pkt_ist_read(FdcanParametar *fdcan, FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
void fdcan_rx_fifo1_cb(FdcanParametar *fdcan, uint32_t RxFifo1ITs)
{
    fdcan->alive_tick = HAL_GetTick();
    fdcan->rx_cb++;
    if(ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
		FdcanPkt pkt = {0};
		FDCAN_RxHeaderTypeDef header = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            fdcan->const_h.hfdcanx, FDCAN_RX_FIFO1, &header, pkt.data));
        RESULT_CHECK_HANDLE(fdcan_pkt_set_len(&pkt, header.DataLength));
		fdcan_pkt_set_id(&pkt, header.Identifier);
        fdcan_pkt_ist_read(fdcan, &pkt);
    }
	if (ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_MESSAGE_LOST))
	{
        Error_Handler();
	}
}

void fdcan_tim_cb(FdcanParametar *fdcan)
{
    uint32_t freq = (uint32_t)fdcan->dbg_h.tim_freq;
    if (fdcan->tim_tick % freq == 0) fdcan->test_en = 1;
    if (++fdcan->tim_tick >= freq * 10) fdcan->tim_tick = 0;
}

#endif