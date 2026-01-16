#include "HY_MOD/light/main.h"
#ifdef HY_MOD_STM32_LIGHT

void light_load(LightCtrl *light)
{
    GPIO_WRITE(light->const_h.RCLK, 0);
    uint8_t mask;
    bool bit_val;
    for (mask = 0b10000000; mask != 0; mask >>= 1) 
    {
        bit_val = (light->data & mask) ? 1 : 0;
        GPIO_WRITE(light->const_h.SER, bit_val);
        GPIO_WRITE(light->const_h.SRCLK, 0);
        GPIO_WRITE(light->const_h.SRCLK, 1);
    }
    GPIO_WRITE(light->const_h.RCLK, 1);
}

#endif