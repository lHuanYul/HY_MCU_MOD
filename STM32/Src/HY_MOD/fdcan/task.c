#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN

#include "main/fdcan.h"
#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/fdcan/pkt_write.h"

#ifdef MCU_VEHICLE_MAIN
#include "HY_MOD/vehicle/main.h"
#endif

static void filter_set(FdcanParametar *fdcan)
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
        .IdType = FDCAN_STANDARD_ID,
        .FilterIndex = 0,
        .FilterType = FDCAN_FILTER_RANGE,
        .FilterConfig = FDCAN_FILTER_TO_RXFIFO0,
        .FilterID1 = FDCAN_FIFO0_FILTER0_ID_MIN,
        .FilterID2 = FDCAN_FIFO0_FILTER0_ID_MAX,
    };
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ConfigFilter(
            fdcan->const_h.hfdcanx, &fifo0_filter0)
    );
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
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ConfigFilter(
            fdcan->const_h.hfdcanx, &fifo1_filter0)
    );
}

static void act_notification(FdcanParametar *fdcan)
{
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

static Result fdcan_auto_pkt_proc(FdcanParametar *fdcan)
{
    Result result = RESULT_OK(NULL);
#ifdef MCU_VEHICLE_MAIN
    if (vehicle_h.fdcan_send)
    {
        vehicle_h.fdcan_send = 0;
        fdcan_vehicle_motor_send(&vehicle_h, &fdcan_pkt_pool, &fdcan_trsm_pkt_buf);
    }
#endif
#ifdef MCU_MOTOR_CTRL
    if (motor_h.fdcan_send)
    {
        motor_h.fdcan_send = 0;
        fdcan_motor_send(&motor_h, &fdcan_pkt_pool, &fdcan_trsm_pkt_buf);
    }
#endif
    if (fdcan->data_store == FNC_ENABLE)
    {
        fdcan->data_store = FNC_DISABLE;
        #ifdef ENABLE_CON_PKT_TEST
        FdcanPkt *pkt;
        pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(&fdcan_pkt_pool));
        fdcan_pkt_write_test(pkt);
        fdcan_pkt_buf_push(&fdcan_trsm_pkt_buf, pkt, &fdcan_pkt_pool, 1);
        #endif
    }
    return result;
}

#define TASK_DELAY_MS 5
void StartFdCanTask(void *argument)
{
    FdcanParametar *fdcan = &fdcan_h;
    #ifdef DISABLE_FDCAN
    StopTask();
    #else
    fdcan_pkt_pool_init(&fdcan_pkt_pool);
    filter_set(fdcan);
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_Start(fdcan->const_h.hfdcanx));
    act_notification(fdcan);
    const uint32_t osPeriod = pdMS_TO_TICKS(TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    for(;;)
    {
        if (fdcan->state == FDCAN_STATE_BUS_OFF)
        {
            fdcan->state = FDCAN_STATE_FINISH;
            HAL_FDCAN_Stop(fdcan->const_h.hfdcanx);
            HAL_FDCAN_Start(fdcan->const_h.hfdcanx);
        }
        fdcan_auto_pkt_proc(fdcan);
        fdcan_trsm_pkts_proc(fdcan, &fdcan_pkt_pool, &fdcan_trsm_pkt_buf);
        if (fdcan->task_tick % 10 == 0)
        {
            fdcan_recv_pkts_proc(&fdcan_pkt_pool, &fdcan_recv_pkt_buf, 5);
        }
        if (fdcan->task_tick % 20 == 0)
        {
            fdcan->task_tick = 0;
            fdcan->data_store = FNC_ENABLE;
        }
        osDelayUntil(next_wake);
        next_wake += osPeriod;
        fdcan->task_tick++;
    }
    #endif
}

#endif