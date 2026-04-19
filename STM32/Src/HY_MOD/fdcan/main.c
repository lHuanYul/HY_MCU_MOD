#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/cmds.h"
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "HY_MOD/main/typedef.h"
#include "fdcan.h"

void fdcan_setup(FdcanParametar *fdcan)
{
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ConfigGlobalFilter(
            fdcan->const_h.hfdcanx,
            FDCAN_REJECT,
            FDCAN_REJECT,
            FDCAN_FILTER_REMOTE,
            FDCAN_FILTER_REMOTE
        )
    );
    FDCAN_FilterTypeDef fifo0_filter0 = {
        .IdType = FDCAN_EXTENDED_ID,
        .FilterIndex = 1,
        .FilterType = FDCAN_FILTER_RANGE,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = FDCAN_FIFO0_FILTER0_ID_MIN,
        .FilterID2 = FDCAN_FIFO0_FILTER0_ID_MAX,
    };
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ConfigFilter(
            fdcan->const_h.hfdcanx, &fifo0_filter0)
    );
    // Inmidiet filter
    FDCAN_FilterTypeDef fifo1_filter0 = {
        .IdType = FDCAN_EXTENDED_ID,
        .FilterIndex = 1,
        .FilterType = FDCAN_FILTER_RANGE,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO1,
        .FilterID1 = 0x0,
        .FilterID2 = 0x1FFFFFFF,
        // .FilterID1 = FDCAN_FIFO1_FILTER0_ID_MIN,
        // .FilterID2 = FDCAN_FIFO1_FILTER0_ID_MAX,
    };
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ConfigFilter(
            fdcan->const_h.hfdcanx, &fifo1_filter0)
    );
#ifdef MY_FDCAN_TDCR
    HAL_FDCAN_ConfigTxDelayCompensation(fdcan->const_h.hfdcanx, MY_FDCAN_TDCR, 0);
    HAL_FDCAN_EnableTxDelayCompensation(fdcan->const_h.hfdcanx);
#endif
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_Start(fdcan->const_h.hfdcanx));
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan->const_h.hfdcanx,
              FDCAN_IT_BUS_OFF
            | FDCAN_IT_TX_EVT_FIFO_NEW_DATA
            | FDCAN_IT_TX_EVT_FIFO_FULL
            | FDCAN_IT_TX_EVT_FIFO_ELT_LOST
        , 0)
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan->const_h.hfdcanx,
            FDCAN_IT_TX_COMPLETE,
              FDCAN_TX_BUFFER0
            | FDCAN_TX_BUFFER1
            | FDCAN_TX_BUFFER2
        )
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan->const_h.hfdcanx, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan->const_h.hfdcanx, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0)
    );
}

void fdcan_tim_start(FdcanParametar *fdcan)
{
    const float32_t tim_f =
        (float32_t)*fdcan->const_h.tim_clk /
        (float32_t)(fdcan->const_h.htimx->Init.Prescaler + 1U);
    fdcan->dbg_h.tim_freq = tim_f / (fdcan->const_h.htimx->Init.Period + 1U);
    ERROR_CHECK_HAL_HANDLE(HAL_TIM_Base_Start_IT(fdcan->const_h.htimx));
}

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

static Result fdcan_pkt_transmit(FdcanParametar *fdcan, FdcanPkt *pkt)
{
    if (pkt == NULL) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    FDCAN_TxHeaderTypeDef header = {
        .IdType                 = FDCAN_EXTENDED_ID,
        .FDFormat               = FDCAN_FD_CAN,
        .ErrorStateIndicator    = FDCAN_ESI_ACTIVE,
        .BitRateSwitch          = FDCAN_BRS_ON,
        .TxEventFifoControl     = FDCAN_STORE_TX_EVENTS,
        .MessageMarker          = pkt->idx,
        .Identifier             = pkt->id,
        .DataLength             = len_to_dlc(pkt->len),
    };
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_AddMessageToTxFifoQ(fdcan->const_h.hfdcanx, &header, pkt->data)
    );
    return RESULT_OK(NULL);
}

Result fdcan_tx_push(FdcanParametar *fdcan)
{
    for (;;)
    {
        uint32_t fl = HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx);
        if (fl == 0) break;

        FdcanPkt pkt = {0};
        Result result = fdcan_ring_pop(&fdcan->tx_buf, &pkt);
        if (RESULT_CHECK_RAW(result)) break;
        RESULT_CHECK_RET_RES(fdcan_pkt_transmit(fdcan, &pkt));
        
        uint32_t timeout = 50;
        while (HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx) == fl)
        {
            if (--timeout == 0) break;
        }
    }
    return RESULT_OK(NULL);
}

static Result trsm_pkts_proc(FdcanParametar *fdcan)
{
    if (HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx) != FDCAN_TX_FIFO_SIZE)
        return RESULT_ERROR(RES_ERR_BUSY);
    RESULT_CHECK_RET_RES(fdcan_tx_push(fdcan));
    return RESULT_OK(NULL);
}

ATTR_WEAK Result fdcan_pkt_rcv_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
static Result recv_pkts_proc(FdcanParametar *fdcan, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        FdcanPkt pkt = {0};
        RESULT_CHECK_RET_RES(fdcan_ring_pop(&fdcan->rx_buf, &pkt));
        fdcan_pkt_rcv_read(&pkt);
    }
    return RESULT_OK(NULL);
}

#include "main/main.h"
static Result auto_pkts_proc(FdcanParametar *fdcan)
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
#define ENABLE_CON_PKT_TEST
#ifdef ENABLE_CON_PKT_TEST
    if (fdcan->test_en)
    {
        fdcan->test_en = 0;
        fdcan_pkt_write_test(fdcan);
    }
#endif
    return result;
}

void fdcan_main(FdcanParametar *fdcan)
{
    switch (fdcan->state)
    {
        case FDCAN_STATE_BUS_OFF:
        {
            fdcan->state = FDCAN_STATE_RESTART;
            fdcan->tim_tick = 0;
            return;
        }
        case FDCAN_STATE_RESTART:
        {
            uint32_t freq = (uint32_t)fdcan->dbg_h.tim_freq;
            if (fdcan->tim_tick >= freq)
            {
                fdcan->state = FDCAN_STATE_RUNNING;
                fdcan_ring_clear(&fdcan->tx_buf);
                HAL_FDCAN_Stop(fdcan->const_h.hfdcanx);
                HAL_FDCAN_Start(fdcan->const_h.hfdcanx);
            }
            return;
        }
        case FDCAN_STATE_RUNNING: break;
    }
    auto_pkts_proc(fdcan);
    trsm_pkts_proc(fdcan);
    recv_pkts_proc(fdcan, 5);
}

#endif