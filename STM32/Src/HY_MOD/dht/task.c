#include "HY_MOD/dht/main.h"
#ifdef HY_MOD_STM32_DHT

#include "main/main.h"

void StartDht11Task(void *argument)
{
    init_setup(&dht_h);
    osDelay(3000);
    for(;;)
    {
        dht_h.state = DHT_STATE_FINISHED;
        dht_h.tim_mode_pwm = 1;
        dht_tim_mode_switch(&dht_h);

        osDelay(2000); // DHT22 建議 2秒以上間隔
    }
}

#endif