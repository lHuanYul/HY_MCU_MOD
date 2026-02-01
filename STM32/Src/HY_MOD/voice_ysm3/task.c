#include "HY_MOD/voice_ysm3/main.h"
#ifdef HY_MOD_STM32_YSM3

uint8_t spkt = 0;
void StartSpkTask(void *argument)
{
    YSM3Parametar *ysm3 = &ysm3_h;
    ysm3_init(ysm3);
    for(;;)
    {
        spkt++;
        ysm3->select = 20;
        ysm3_load(ysm3);
        ysm3->select = 11;
        ysm3_load(ysm3);
        ysm3->select = 1;
        ysm3_load(ysm3);
        ysm3->select = 13;
        ysm3_load(ysm3);
        ysm3->select = 21;
        ysm3_load(ysm3);
        osDelay(1000);
        ysm3->select = 20;
        ysm3_load(ysm3);
        ysm3->select = 12;
        ysm3_load(ysm3);
        ysm3->select = 1;
        ysm3_load(ysm3);
        ysm3->select = 14;
        ysm3_load(ysm3);
        ysm3->select = 21;
        ysm3_load(ysm3);
        osDelay(1000);
    }
}

#endif