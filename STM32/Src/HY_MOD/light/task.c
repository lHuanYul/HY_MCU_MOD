#include "HY_MOD/light/main.h"
#ifdef HY_MOD_STM32_LIGHT
#include "main/light.h"

#define DEFALT_TASK_DELAY_MS 1000
uint32_t defalt_running;
void StartDefaultTask(void *argument)
{
    const uint32_t osPeriod = pdMS_TO_TICKS(DEFALT_TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    GPIO_WRITE(light_h0.const_h.OE, 0);
    light_load(&light_h0);
    for (;;)
    {
        defalt_running++;
        // GPIO_TOGGLE(light_h0.const_h.SER);
        
        light_load(&light_h0);
        defalt_running = HAL_GetTick();
        osDelayUntil(next_wake);
        next_wake += osPeriod;
    }
    GPIO_WRITE(light_h0.const_h.OE, 1);
}

#endif