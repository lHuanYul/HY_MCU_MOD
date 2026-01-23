#include "HY_MOD/adc_hall/main.h"
#ifdef HY_MOD_STM32_ADC_HALL

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/adc/main.h"

void adc_hall_update(AdcHallParameter *hall)
{
    uint16_t val = adc_upd_dma_get_val(&hall->adc, 0);
    hall->value = (uint16_t)var_u32_iir((uint32_t)hall->value, (uint32_t)val, 0.1f);
    hall->dbg_tim++;
    if (hall->dbg_tim >= 100)
    {
        hall->dbg_tim = 0;
        hall->dbg_max = 0;
        hall->dbg_min = 4095;
    }

    if(hall->value > hall->dbg_max) hall->dbg_max = hall->value;
    else if(hall->value < hall->dbg_min) hall->dbg_min = hall->value;
    
    if (hall->value <= hall->gate) hall->state = ADC_HALL_STATE_ON_MAG;
    else hall->state = ADC_HALL_STATE_NONE;
}

#endif