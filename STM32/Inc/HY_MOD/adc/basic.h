#pragma once
#include "main/config.h"
#if defined(HY_MOD_STM32_ADC_HALL)
#define HY_MOD_STM32_ADC

typedef struct AdcConst
{
    ADC_HandleTypeDef *hadcx;
    uint32_t rankx;
} AdcConst;

typedef struct AdcParameter
{
    const AdcConst const_h;
    uint16_t    value;
} AdcParameter;

#endif