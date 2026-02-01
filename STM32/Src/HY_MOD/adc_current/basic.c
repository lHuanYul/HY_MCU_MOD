#include "HY_MOD/adc_current/basic.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

#include "adc.h"

AdcCurrentParameter adc_test = {
    .adc = {
        .hadcx = &hadc1,
        .rankx = 3,
    },
    .sensitive = 0.1f,
};

// CH11 PB12 0.097
AdcCurrentParameter adc_0 = {
    .adc = {
        .hadcx = &hadc1,
        .rankx = ADC_INJECTED_RANK_1,
    },
    .sensitive = 0.1f,
    .current_trs = 1.0f,
};

// CH12 PB1
AdcCurrentParameter adc_1 = {
    .adc = {
        .hadcx = &hadc1,
        .rankx = ADC_INJECTED_RANK_2,
    },
    .sensitive = 0.1f,
    .current_trs = 1.0f,
};

// CH14 PB11
AdcCurrentParameter adc_2 = {
    .adc = {
        .hadcx = &hadc1,
        .rankx = ADC_INJECTED_RANK_3,
    },
    .sensitive = 0.1f,
    .current_trs = 1.0f,
};

#endif