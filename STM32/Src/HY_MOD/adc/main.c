#include "HY_MOD/adc/main.h"
#ifdef HY_MOD_STM32_ADC

static uint16_t adc_vals_dma_arr[ADC_NEED_LEN * ADC_COUNT] = {0};
static uint16_t adc_vals_dma_sig[ADC_NEED_LEN] = {0};

void adc_max_min_rst(AdcParameter *adc)
{
    adc->max = 0;
    adc->min = UINT16_MAX;
}

void adc_hal_start_dma(AdcParameter *adc)
{
    HAL_ADC_Start_DMA(adc->hadcx, (uint32_t*)adc_vals_dma_arr, ADC_COUNT);
}

void adc_upd_dma_get_arr(AdcParameter *adc)
{
    uint32_t i;
    for (i = 0; i < ADC_NEED_LEN; i++)
    {
        adc_vals_dma_sig[i] =
            adc_vals_dma_arr[ADC_NEED_LEN * i + adc->rankx];
    }
}

uint16_t adc_upd_dma_get_val(AdcParameter *adc, uint32_t loop)
{
    return adc_vals_dma_arr[ADC_NEED_LEN * loop + adc->rankx];
}

inline void adc_upd_injected(AdcParameter *adc)
{
    adc->value = (uint16_t)HAL_ADCEx_InjectedGetValue(adc->hadcx, adc->rankx);
    if (adc->value > adc->max) adc->max = adc->value;
    if (adc->value < adc->min) adc->min = adc->value;
}

#endif