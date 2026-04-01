#pragma once
#include "main/config.h"
#if defined(HY_MOD_STM32_ADC_HALL) || defined(HY_MOD_STM32_ADC_CURRENT)
#define HY_MOD_STM32_ADC

#define ADC_NEED_LEN    5 // 511

typedef struct AdcParameter
{
    ADC_HandleTypeDef *hadcx;
    const uint32_t rankx;
    uint16_t    value;
    uint16_t    max;
    uint16_t    min;
} AdcParameter;

#endif