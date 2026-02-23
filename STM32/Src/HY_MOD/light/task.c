#include "HY_MOD/light/main.h"
#ifdef HY_MOD_LIGHT
#include "main/light.h"

#define TASK_DELAY_MS 50
void StartLightTask(void *argument)
{
    const uint32_t osPeriod = pdMS_TO_TICKS(TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    GPIO_WRITE(light_h.const_h.OE, 0);
    light_h.load = 1;
    for (;;)
    {
        light_load(&light_h);
        light_cd(&light_h);
        osDelayUntil(next_wake);
        next_wake += osPeriod;
    }
    GPIO_WRITE(light_h.const_h.OE, 1);
}

#endif