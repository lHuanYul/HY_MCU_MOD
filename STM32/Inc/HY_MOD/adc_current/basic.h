#pragma once
#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

typedef struct AdcCurrentParameter
{
    AdcParameter adc;
    const float32_t sensitive;
    // 透過 sensitive 等常數計算出的值 用以直接換算
    float32_t current_trs;
    // 電流為0時的 adc 值
    uint16_t val_zero;
    // 換算後的電流值
    float32_t current;
} AdcCurrentParameter;

extern AdcCurrentParameter adc_test;
extern AdcCurrentParameter adc_0;
extern AdcCurrentParameter adc_1;
extern AdcCurrentParameter adc_2;

#endif