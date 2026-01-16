#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN

#include "main/mod_cfg.h"
#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/fdcan/pkt_write.h"

#ifdef MCU_VEHICLE_MAIN
#include "vehicle/main.h"
#endif

static Result auto_pkt_proc(void)
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
    if (fdcan_h.data_store == FNC_ENABLE)
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
    fdcan_pkt_pool_init(&fdcan_pkt_pool);
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ConfigGlobalFilter(
            fdcan_h.const_h.hfdcanx,
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
            fdcan_h.const_h.hfdcanx, &fifo0_filter0)
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
            fdcan_h.const_h.hfdcanx, &fifo1_filter0)
    );
    ERROR_CHECK_HAL_HANDLE(HAL_FDCAN_Start(fdcan_h.const_h.hfdcanx));
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan_h.const_h.hfdcanx,
              FDCAN_IT_BUS_OFF
            | FDCAN_IT_TX_EVT_FIFO_NEW_DATA
            | FDCAN_IT_TX_EVT_FIFO_FULL
            | FDCAN_IT_TX_EVT_FIFO_ELT_LOST
        , 0)
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan_h.const_h.hfdcanx,
            FDCAN_IT_TX_COMPLETE,
              FDCAN_TX_BUFFER0
            | FDCAN_TX_BUFFER1
            | FDCAN_TX_BUFFER2
        )
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan_h.const_h.hfdcanx, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)
    );
    ERROR_CHECK_HAL_HANDLE(
        HAL_FDCAN_ActivateNotification(
            fdcan_h.const_h.hfdcanx, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0)
    );
    fdcan_tick = 0;
    const uint32_t osPeriod = pdMS_TO_TICKS(FDCAN_TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    for(;;)
    {
        if (fdcan_h.bus_off)
        {
            fdcan_h.bus_off = false;
            HAL_FDCAN_Stop(fdcan_h.const_h.hfdcanx);
            HAL_FDCAN_Start(fdcan_h.const_h.hfdcanx);
        }
        auto_pkt_proc();
        trsm_pkts_proc(&fdcan_pkt_pool, &fdcan_trsm_pkt_buf);
        if (fdcan_tick % 10 == 0)
        {
            recv_pkts_proc(&fdcan_pkt_pool, &fdcan_recv_pkt_buf, 5);
        }
        if (fdcan_tick % 20 == 0)
        {
            fdcan_tick = 0;
            if (fdcan_h.data_store == FNC_ENABLE);
        }
        osDelayUntil(next_wake);
        next_wake += osPeriod;
        fdcan_tick++;
    }
    #endif
}

#endif