#include "HY_MOD/adc/main.h"
#ifdef HY_MOD_STM32_ADC

#ifndef ADC_COUNT
#define ADC_COUNT 1
#endif
#ifndef ADC_NEED_LEN
#define ADC_NEED_LEN 1
#endif

#ifndef ADC_TO_VOL
#define ADC_TO_VOL (3.3f/4095.0f)
#endif

#define MAX_MIN_UPD(max, min, value) \
    do { \
        if ((value) > (max)) (max) = (value); \
        if ((value) < (min)) (min) = (value); \
    } while(0)

static uint16_t adc_vals_dma_arr[ADC_NEED_LEN * ADC_COUNT] = {0};
static uint16_t adc_vals_dma_sig[ADC_NEED_LEN] = {0};
static uint16_t adc_vals_math[ADC_COUNT * ADC_NEED_LEN] = {0};

// static uint16_t adc.wnt[2560] = {0};
// static ATTR_UNUSED Result middle(uint8_t adc_id, uint16_t *adc_store)
// {
//     memset(adc.wnt, 0, sizeof(adc.wnt));
//     uint16_t i, val;
//     for (i = 0; i < ADC_NEED_LEN; i++)
//     {
//         val = adc_vals_math[i * ADC_COUNT + adc_id];
//         if(val > 2559) continue;
//         adc.wnt[val]++;
//     }
//     const uint16_t target = (ADC_NEED_LEN-1)/2;
//     uint16_t acc = 0;
//     for (i = 500; i < 2500; i++)
//     {
//         acc += adc.wnt[i];
//         if (acc > target)
//         {
//             *adc_store = i;
//             break;
//         }
//     }
//     return RESULT_OK(NULL);
// }

static void average(uint8_t adc_id, float32_t *adc_store)
{
    uint16_t i;
    float32_t total = 0;
    for (i = 0; i < ADC_NEED_LEN; i++)
    {
        total += adc_vals_math[i * ADC_COUNT + adc_id];
    }
    *adc_store = total / (float32_t)ADC_NEED_LEN;
}

#define ALPHA 0.1f
static inline void iir(uint8_t adc_id, float32_t *adc_store)
{
    float32_t avg;
    average(adc_id, &avg);
    *adc_store += ALPHA * (avg - *adc_store);
}

void adc_max_min_reset(AdcParameter *adc)
{
    adc->max_ori = 0;
    adc->min_ori = UINT16_MAX;
    adc->max_fix = 0;
    adc->min_fix = UINT16_MAX;
}

void adc_HAL_start_dma(AdcParameter *adc)
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

void adc_upd_injected(AdcParameter *adc)
{
    adc->value_ori = (uint16_t)HAL_ADCEx_InjectedGetValue(adc->hadcx, adc->rankx);
    MAX_MIN_UPD(adc->max_ori, adc->min_ori, adc->value_ori);
    if (adc->need_fix == 0) return;
    adc->value_fix =
        (float32_t)((int32_t)adc->value_ori - (int32_t)adc->offset) * adc->scale
        + adc->shift;
    MAX_MIN_UPD(adc->max_fix, adc->min_fix, adc->value_fix);
}

void adc_current_init(AdcParameter *adc, const AdcCurrentModel *model)
{
    adc->need_fix = 1;
    adc->scale = ADC_TO_VOL / model->proportion / model->sensitive;
    adc->shift = 0.0f;
    adc->offset = adc->value_ori;
}

#endif