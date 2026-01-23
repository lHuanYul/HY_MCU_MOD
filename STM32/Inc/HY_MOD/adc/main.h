#pragma once
#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC

void adc_hal_start_dma(AdcParameter *adc);
void adc_upd_dma_get_arr(AdcParameter *adc);
uint16_t adc_upd_dma_get_val(AdcParameter *adc, uint32_t loop);

#endif