#pragma once
#include "main/config.h"
#if defined(HY_MOD_STM32_ADC) || defined(HY_MOD_STM32_ADC_HALL)
#define HY_MOD_STM32_ADC

typedef struct AdcConst
{
    ADC_HandleTypeDef   *hadcx;
    uint32_t            rankx;
} AdcConst;

#define MAX_MIN_RESET(str, type_max) \
    do { \
        (str)->min = (type_max); \
        (str)->max = 0; \
    } while (0)

typedef struct AdcParam
{
    const AdcConst      const_h;
    uint16_t            value;
    uint16_t            max;
    uint16_t            min;
} AdcParam;

typedef struct AdcModel
{
    // sensitive
    // Rshunt * Gain * Gopa
    float32_t   sensitive;
    // ex. 5->3.3 --> 2/3
    float32_t   proportion;
    // 平移
    float32_t   shift;
} AdcModel;
extern const AdcModel adc_I_acs712;
extern const AdcModel adc_I_opa325;
extern const AdcModel adc_I_amc3302_R1m2;
extern const AdcModel adc_V_R_36to3V3;

typedef struct AdcModelParam
{
    AdcParam        basic;
    const AdcModel  *model;
    // 零點
    int32_t         offset;
    // 比例放大
    float32_t       scale;
    // 平移
    float32_t       shift;
    // 修正值
    float32_t       value;
    float32_t       max;
    float32_t       min;
} AdcModelParam;

#endif