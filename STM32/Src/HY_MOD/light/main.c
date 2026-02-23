#include "HY_MOD/light/main.h"
#ifdef HY_MOD_LIGHT

void light_load(LightCtrl *light)
{
    if (light->load == 0) return;
    light->load = 0;
    light->cd = LIGHT_CD;
    light->dbg_load++;
    GPIO_WRITE(light->const_h.RCLK, 0);
    uint8_t mask, i;
    bool bit_val;
    for (mask = 0b10000000; mask != 0; mask >>= 1) 
    {
        bit_val = (light->flags & mask) ? 1 : 0;
        GPIO_WRITE(light->const_h.SER, bit_val);
        for(i = 0; i < 5; i++) __NOP();
        GPIO_WRITE(light->const_h.SRCLK, 0);
        for(i = 0; i < 5; i++) __NOP();
        GPIO_WRITE(light->const_h.SRCLK, 1);
        for(i = 0; i < 5; i++) __NOP();
    }
    GPIO_WRITE(light->const_h.RCLK, 1);
}

void light_button(LightCtrl *light, uint16_t GPIO_Pin)
{
    uint8_t i;
    for (i = 0; i < LIGHT_BUTTON_CNT; i++)
    {
        if (
            HAL_GPIO_ReadPin(light->lights.abc[i].gpio.GPIOx, GPIO_Pin) == GPIO_PIN_RESET &&
            light->lights.abc[i].gpio.Pin == GPIO_Pin &&
            light->lights.abc[i].cd == 0
        ) {
            light->lights.abc[i].cd = LIGHT_BUTTON_CD;
            light_trigger(light, 2, light->lights.abc[i].id);
            light->load = 1;
        }
    }
}

void light_cd(LightCtrl *light)
{
    uint8_t i;
    for (i = 0; i < LIGHT_BUTTON_CNT; i++)
    {
        light->cd--;
        if (light->cd <= 0)
        {
            light->cd = LIGHT_CD;
            light->load = 1;
        }
        if (light->lights.abc[i].cd > 0) light->lights.abc[i].cd--;
    }
}

#endif