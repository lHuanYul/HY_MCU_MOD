#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC

#include "adc.h"

const AdcModel adc_I_acs712 = {
    .sensitive = 0.1f,
    .proportion = 2.0f / 3.0f,
};

const AdcModel adc_I_opa325 = {
    .sensitive = 0.2f,
    .proportion = 1.0f,
};

const AdcModel adc_I_amc3302_R1m2 = {
    .sensitive = 0.0012f * 41.0f * 0.82f,
    .proportion = 1.0f,
};

const AdcModel adc_V_R_36to3V3 = {
    .sensitive = 3.3f / 36.0f,
    .proportion = 1.0f,
};

#endif