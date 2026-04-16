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
    fdcan->trsming++;
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_NEW_DATA))
    {
        FDCAN_TxEventFifoTypeDef txEvent;
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetTxEvent(fdcan->const_h.hfdcanx, &txEvent));
        RESULT_CHECK_HANDLE(
            fdcan_pkt_buf_trsm_pop(&fdcan->trsm_buf, &fdcan->pool, txEvent.MessageMarker));
        
        uint32_t fl = HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx);
        RESULT_CHECK_RET_VOID(fdcan_tx_push(fdcan, fl));
    }
    if (ITS_CHECK(TxEventFifoITs, FDCAN_IT_TX_EVT_FIFO_ELT_LOST))
    {
        Error_Handler();
    }
}

void fdcan_rx_fifo0_cb(FdcanParametar *fdcan, uint32_t RxFifo0ITs)
{
    fdcan->alive_tick = HAL_GetTick();
    fdcan->recving++;
    if(ITS_CHECK(RxFifo0ITs, FDCAN_IT_RX_FIFO0_NEW_MESSAGE))
    {
        FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(&fdcan->pool));
		FDCAN_RxHeaderTypeDef header = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            fdcan->const_h.hfdcanx, FDCAN_RX_FIFO0, &header, pkt->data));
        RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, header.DataLength));
		fdcan_pkt_set_id(pkt, header.Identifier);
        RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(&fdcan->recv_buf, pkt, &fdcan->pool, 0));
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
    fdcan->recving++;
    if(ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_NEW_MESSAGE))
    {
		FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(&fdcan->pool));
		FDCAN_RxHeaderTypeDef header = {0};
        ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_GetRxMessage(
            fdcan->const_h.hfdcanx, FDCAN_RX_FIFO1, &header, pkt->data));
        RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, header.DataLength));
		fdcan_pkt_set_id(pkt, header.Identifier);
        fdcan_pkt_ist_read(fdcan, pkt);
		fdcan_pkt_pool_free(&fdcan->pool, pkt);
    }
	if (ITS_CHECK(RxFifo1ITs, FDCAN_IT_RX_FIFO1_MESSAGE_LOST))
	{
        Error_Handler();
	}
}

#include "main/main.h"
static Result fdcan_auto_pkt_proc(FdcanParametar *fdcan)
{
    Result result = RESULT_OK(NULL);
#ifdef MCU_MOTOR_CTRL
    if (fdcan->motor_ret_en)
    {
        if (fdcan->motor_rpm_en)
        {
            fdcan->motor_rpm_en = 0;
            fdcan_motor_rpm_send(&fdcan_h, &motor_h);
        }
        if (fdcan->motor_idq_en1)
        {
            fdcan->motor_idq_en1 = 0;
            fdcan_motor_idq_send(&fdcan_h, &motor_h, 0);
        }
        if (fdcan->motor_idq_en2)
        {
            fdcan->motor_idq_en2 = 0;
            fdcan_motor_idq_send(&fdcan_h, &motor_h, 5);
        }
    }
#endif
#ifdef MCU_VEHICLE_MAIN
    if (vehicle_h.fdcan_send)
    {
        vehicle_h.fdcan_send = 0;
        fdcan_vehicle_motor_send(&vehicle_h, &fdcan_pkt_pool, &fdcan_trsm_pkt_buf);
    }
#endif
        #ifdef ENABLE_CON_PKT_TEST
        FdcanPkt *pkt;
        pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(&fdcan_pkt_pool));
        fdcan_pkt_write_test(pkt);
        fdcan_pkt_buf_push(&fdcan_trsm_pkt_buf, pkt, &fdcan_pkt_pool, 1);
        #endif
    return result;
}

void fdcan_task_cb(FdcanParametar *fdcan)
{
    if (fdcan->state == FDCAN_STATE_BUS_OFF)
    {
        fdcan->state = FDCAN_STATE_FINISH;
        HAL_FDCAN_Stop(fdcan->const_h.hfdcanx);
        HAL_FDCAN_Start(fdcan->const_h.hfdcanx);
    }
    fdcan_auto_pkt_proc(fdcan);
    fdcan_trsm_pkts_proc(fdcan);
    fdcan_recv_pkts_proc(fdcan, 5);
    if (++fdcan->tim_tick >= 10*1000*1000) fdcan->tim_tick = 0;
}

#endif