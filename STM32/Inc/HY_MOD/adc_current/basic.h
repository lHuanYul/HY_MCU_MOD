#pragma once
#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

typedef struct AdcIModelData
{
    float32_t sensitive;
    // 5->3.3 --> 2/3
    float32_t proportion;
} AdcIModelData;
extern const AdcIModelData adc_I_acs712;
extern const AdcIModelData adc_I_opa325;

typedef struct AdcCurrentParameter
{
    AdcParameter basic;

    const AdcIModelData *model;
    // 電流為0時的 adc 值
    uint16_t val_offset;
    // 透過 sensitive 等常數計算出的值 用以直接換算
    float32_t current_trs;
    // 換算後的電流值
    float32_t current;
} AdcCurrentParameter;

#endif