#pragma once
#include "main/config.h"
#if defined(HY_MOD_STM32_ADC) || defined(HY_MOD_STM32_ADC_HALL)
#define HY_MOD_STM32_ADC

typedef struct AdcParameter
{
    ADC_HandleTypeDef *hadcx;
    const uint32_t rankx;
    uint16_t    offset;
    uint16_t    value_ori;
    uint16_t    max_ori;
    uint16_t    min_ori;
    uint8_t     need_fix;
    float32_t   scale;
    float32_t   shift;
    float32_t   value_fix;
    float32_t   max_fix;
    float32_t   min_fix;
} AdcParameter;

typedef struct AdcCurrentModel
{
    float32_t sensitive;
    // 5->3.3 --> 2/3
    float32_t proportion;
} AdcCurrentModel;
extern const AdcCurrentModel adc_I_acs712;
extern const AdcCurrentModel adc_I_opa325;

#endif