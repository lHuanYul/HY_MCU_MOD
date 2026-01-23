#include "HY_MOD/adc/main.h"
#ifdef HY_MOD_STM32_ADC

static uint16_t adc_vals_dma_arr[ADC_NEED_LEN * ADC_COUNT] = {0};
static uint16_t adc_vals_dma_sig[ADC_NEED_LEN] = {0};

void adc_hal_start_dma(AdcParameter *adc)
{
    HAL_ADC_Start_DMA(adc->const_h.hadcx, (uint32_t*)adc_vals_dma_arr, ADC_COUNT);
}

void adc_upd_dma_get_arr(AdcParameter *adc)
{
    uint32_t i;
    for (i = 0; i < ADC_NEED_LEN; i++)
    {
        adc_vals_dma_sig[i] =
            adc_vals_dma_arr[ADC_NEED_LEN * i + adc->const_h.rankx];
    }
}

uint16_t adc_upd_dma_get_val(AdcParameter *adc, uint32_t loop)
{
    return adc_vals_dma_arr[ADC_NEED_LEN * loop + adc->const_h.rankx];
}

#endif