#include "HY_MOD/fdcan/main.h"
#ifdef HY_MOD_STM32_FDCAN

#include "main/main.h"
#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/fdcan/callback.h"

#ifdef MCU_VEHICLE_MAIN
#include "HY_MOD/vehicle/main.h"
#endif

#define TASK_DELAY_MS 1
void StartFdCanTask(void *argument)
{
    FdcanParametar *fdcan = &fdcan_h;
    // StopTask();
    fdcan_setup(fdcan);
    const uint32_t osPeriod = pdMS_TO_TICKS(TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    for(;;)
    {
        fdcan_task_cb(fdcan);
        osDelayUntil(next_wake);
        next_wake += osPeriod;
    }
}

#endif