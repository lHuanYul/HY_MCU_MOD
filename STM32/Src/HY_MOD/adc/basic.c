#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC

#include "adc.h"

const AdcCurrentModel adc_I_acs712 = {
    .sensitive = 0.1f,
    .proportion = 2.0f / 3.0f,
};

const AdcCurrentModel adc_I_opa325 = {
    .sensitive = 0.2f,
    .proportion = 1.0f,
};

#endif