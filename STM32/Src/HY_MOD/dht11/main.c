#include "HY_MOD/dht11/main.h"
#ifdef HY_MOD_STM32_DHT11

static void init_setup(Dht11Parametar *dht11)
{
    dht11->dbg_tim_freq =
        (float32_t)*dht11->const_h.tim_clk /
        (float32_t)(dht11->const_h.htimx->Init.Prescaler + 1U);
}

void StartDht11Task(void *argument)
{
    init_setup(&dht11_h);
    osDelay(1000);
    for(;;)
    {
        if (dht11_h.state == DHT_STATE_FINISH || dht11_h.state == DHT_STATE_FINISHED)
        {
            dht11_h.tim_mode_pwm = 1;
            dh11_tim_mode_switch(&dht11_h);
        }
        osDelay(5000);
    }
}

#endif