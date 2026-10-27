#pragma once
#include "HY_MOD/adc/basic.h"
#ifdef HY_MOD_STM32_ADC

void adc_HAL_start_dma(AdcParam *adc);
void adc_upd_dma_get_arr(AdcParam *adc);
uint16_t adc_upd_dma_get_val(AdcParam *adc, uint32_t loop);

void adc_upd_inject(AdcParam *adc);

void adc_max_min_reset(AdcParam *adc);
void adc_max_min_upd(AdcParam *adc);

void adcm_init_load(AdcModelParam *adc);
void adcm_offset_set(AdcModelParam *adcm);
void adcm_update(AdcModelParam *adcm);
void adcm_max_min_reset(AdcModelParam *adcm);
void adcm_max_min_upd(AdcModelParam *adcm);

#endif
