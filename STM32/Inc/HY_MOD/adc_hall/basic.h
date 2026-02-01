#pragma once
#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC_HALL

typedef enum AdcState
{
    ADC_HALL_STATE_NONE,
    ADC_HALL_STATE_ON_MAG,
} AdcState;

typedef struct AdcHallParameter
{
    AdcParameter    adc;
    uint16_t        value;
    uint16_t        gate;
    AdcState        state;
    uint16_t        dbg_max;
    uint16_t        dbg_min;
    uint16_t        dbg_tim;
} AdcHallParameter;

extern AdcHallParameter adchall_track_left;
extern AdcHallParameter adchall_track_right;
extern AdcHallParameter adchall_node;
extern AdcHallParameter adchall_direction;

#endif