#pragma once
#include "HY_MOD/adc_current/basic.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

#include "HY_MOD/main/fn_state.h"

Result adc_current_upd(AdcCurrentParameter *adc);
void adc_current_reset(AdcCurrentParameter *adc);

#endif