#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/cmds.h"
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/pkt_write.h"
#include "HY_MOD/main/typedef.h"
#include "fdcan.h"

void fdcan_setup(FdcanParametar *fdcan)
{
    fdcan_pkt_pool_init(&fdcan->pool);
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
    HAL_FDCAN_ConfigTxDelayCompensation(fdcan->const_h.hfdcanx, FDCAN_TDC, 0);
    HAL_FDCAN_EnableTxDelayCompensation(fdcan->const_h.hfdcanx);
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
    const float32_t PWM_tim_f =
        (float32_t)*fdcan->const_h.tim_clk /
        (float32_t)(fdcan->const_h.htimx->Init.Prescaler + 1U);
    fdcan->dbg_h.tim_freq = PWM_tim_f / (fdcan->const_h.htimx->Init.Period + 1U);
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
    if (pkt->state == 1) return RESULT_ERROR(RES_ERR_BUSY);
    FDCAN_TxHeaderTypeDef header = {
        .IdType                 = FDCAN_EXTENDED_ID,
        .FDFormat               = FDCAN_FD_CAN,
        .ErrorStateIndicator    = FDCAN_ESI_ACTIVE,
        .BitRateSwitch          = FDCAN_BRS_ON,
        .TxEventFifoControl     = FDCAN_STORE_TX_EVENTS,
        .MessageMarker          = pkt->number,
        .Identifier             = pkt->id,
        .DataLength             = len_to_dlc(pkt->len),
    };
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_AddMessageToTxFifoQ(fdcan->const_h.hfdcanx, &header, pkt->data)
    );
    // Todo
    pkt->state = 1;
    return RESULT_OK(NULL);
}

Result fdcan_tx_push(FdcanParametar *fdcan)
{
    for (;;)
    {
        uint32_t fl = HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx);
        if (fl == 0) break;

        Result result = fdcan_pkt_buf_trsm_get(&fdcan->tx_buf);
        if (RESULT_CHECK_RAW(result)) break;
        FdcanPkt *pkt = RESULT_UNWRAP(result);
        RESULT_CHECK_RET_RES(fdcan_pkt_transmit(fdcan, pkt));
        
        uint32_t timeout = 10000;
        while (
            HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx) == fl &&
            --timeout
        ) __NOP();
    }
    return RESULT_OK(NULL);
}

Result fdcan_trsm_pkts_proc(FdcanParametar *fdcan)
{
    if (HAL_FDCAN_GetTxFifoFreeLevel(fdcan->const_h.hfdcanx) != FDCAN_TX_FIFO_SIZE)
        return RESULT_ERROR(RES_ERR_BUSY);
    RESULT_CHECK_RET_RES(fdcan_tx_push(fdcan));
    return RESULT_OK(NULL);
}

ATTR_WEAK Result fdcan_pkt_rcv_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
Result fdcan_recv_pkts_proc(FdcanParametar *fdcan, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_buf_pop(&fdcan->rx_buf, 0));
        fdcan_pkt_rcv_read(pkt);
        fdcan_pkt_pool_free(&fdcan->pool, pkt);
    }
    return RESULT_OK(NULL);
}

#endif
