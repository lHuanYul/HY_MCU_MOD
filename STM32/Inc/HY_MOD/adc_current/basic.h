#pragma once
#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

typedef struct AdcCurrentParameter
{
    AdcParameter basic;
    // 電流為0時的 adc 值
    uint16_t val_offset;
    
    const float32_t sensitive;
    // 透過 sensitive 等常數計算出的值 用以直接換算
    float32_t current_trs;
    // 換算後的電流值
    float32_t current;
} AdcCurrentParameter;

extern AdcCurrentParameter adc_current_h[ADC_COUNT];

#endif