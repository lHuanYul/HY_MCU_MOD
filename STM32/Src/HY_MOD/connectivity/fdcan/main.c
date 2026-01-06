#include "HY_MOD/connectivity/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/connectivity/cmds.h"
#include "HY_MOD/connectivity/fdcan/pkt_read.h"
#include "HY_MOD/connectivity/fdcan/pkt_write.h"
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

static Result trsm_pkts_proc(void)
{
    Result result = fdcan_pkt_buf_get(&fdcan_trsm_pkt_buf);
    if (RESULT_CHECK_RAW(result)) return RESULT_OK(NULL);
    FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(result);
    RESULT_CHECK_RET_RES(fdcan_pkt_transmit(pkt));
    fdcan_pkt_buf_pop(&fdcan_trsm_pkt_buf);
    fdcan_pkt_pool_free(pkt);
    return RESULT_OK(NULL);
}

ATTR_WEAK Result fdcan_pkt_rcv_read(FdcanPkt *pkt) { return RESULT_ERROR(RES_ERR_NOT_FOUND); }
static Result recv_pkts_proc(uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_buf_pop(&fdcan_recv_pkt_buf));
        fdcan_pkt_rcv_read(pkt);
        fdcan_pkt_pool_free(pkt);
    }
    return RESULT_OK(NULL);
}

static Result auto_pkt_proc(void)
{
    Result result = RESULT_OK(NULL);
    if (vehicle_h.fdcan_send)
    {
        vehicle_h.fdcan_send = 0;
        fdcan_vehicle_motor_send(&vehicle_h);
    }
    if (fdacn_data_store == FNC_ENABLE)
    {
        FdcanPkt *pkt;
        #ifdef ENABLE_CON_PKT_TEST
        pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc());
        fdcan_pkt_write(pkt, DATA_TYPE_TEST);
        fdcan_pkt_buf_push(&fdcan_trsm_pkt_buf, pkt);
        #else
        #endif
    }
    return result;
}

uint32_t fdcan_tick;
#define FDCAN_TASK_DELAY_MS 5
void StartFdCanTask(void *argument)
{
    #ifdef DISABLE_FDCAN
    StopTask();
    #else
    fdcan_pkt_pool_init();
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE));
    FDCAN_FilterTypeDef fifo0_filter0 = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_RANGE,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = FDCAN_FIFO0_FILTER0_ID_MIN,
        .FilterID2 = FDCAN_FIFO0_FILTER0_ID_MAX,
    };
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_ConfigFilter(&hfdcan1, &fifo0_filter0));
    FDCAN_FilterTypeDef fifo1_filter0 = {
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 1,
        .FilterType = FDCAN_FILTER_RANGE,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO1,
        .FilterID1 = 0x000,
        .FilterID2 = 0x7FF,
        // .FilterID1 = FDCAN_FIFO1_FILTER0_ID_MIN,
        // .FilterID2 = FDCAN_FIFO1_FILTER0_ID_MAX,
    };
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_ConfigFilter(&hfdcan1, &fifo1_filter0));
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_Start(&hfdcan1));
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(&hfdcan1,
              FDCAN_IT_BUS_OFF
            | FDCAN_IT_TX_EVT_FIFO_NEW_DATA
            | FDCAN_IT_TX_EVT_FIFO_FULL
            | FDCAN_IT_TX_EVT_FIFO_ELT_LOST
        , 0)
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_TX_COMPLETE,
              FDCAN_TX_BUFFER0
            | FDCAN_TX_BUFFER1
            | FDCAN_TX_BUFFER2
        )
    );
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0));
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0));
    fdcan_tick = 0;
    const uint32_t osPeriod = pdMS_TO_TICKS(FDCAN_TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    for(;;)
    {
        if (fdcan_bus_off)
        {
            fdcan_bus_off = false;
            HAL_FDCAN_Stop(&hfdcan1);
            HAL_FDCAN_Start(&hfdcan1);
        }
        auto_pkt_proc();
        trsm_pkts_proc();
        if (fdcan_tick % 10 == 0)
        {
            recv_pkts_proc(5);
        }
        if (fdcan_tick % 20 == 0)
        {
            fdcan_tick = 0;
            if (fdacn_data_store == FNC_ENABLE);
        }
        osDelayUntil(next_wake);
        next_wake += osPeriod;
        fdcan_tick++;
    }
    #endif
}

#endif