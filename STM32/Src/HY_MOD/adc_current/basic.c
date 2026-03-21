#include "HY_MOD/adc_current/basic.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

#include "adc.h"

AdcCurrentParameter adc_current_h[ADC_COUNT] = {
    {
        // ADC1 CH11 PB12 0.097
        .basic = {
            .hadcx = &hadc1,
            .rankx = ADC_INJECTED_RANK_1,
        },
        .sensitive = 0.1f,
        .current_trs = 1.0f,
    },
    {
        // ADC2 CH12 PB2
        .basic = {
            .hadcx = &hadc2,
            .rankx = ADC_INJECTED_RANK_1,
        },
        .sensitive = 0.1f,
        .current_trs = 1.0f,
    },
    {
        // ADC1 CH14 PB11
        .basic = {
            .hadcx = &hadc1,
            .rankx = ADC_INJECTED_RANK_2,
        },
        .sensitive = 0.1f,
        .current_trs = 1.0f,
    },
};

#endif