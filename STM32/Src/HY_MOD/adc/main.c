#include "HY_MOD/adc/main.h"
#ifdef HY_MOD_STM32_ADC

#include "HY_MOD/main/variable_cal.h"

#ifndef ADC_COUNT
#define ADC_COUNT 1
#endif
#ifndef ADC_NEED_LEN
#define ADC_NEED_LEN 1
#endif

#ifndef ADC_TO_VOL
#define ADC_TO_VOL (3.3f/4095.0f)
#endif

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

inline void adc_HAL_start_dma(AdcParam *adc)
{
    HAL_ADC_Start_DMA(adc->const_h.hadcx, (uint32_t*)adc_vals_dma_arr, ADC_COUNT);
}

void adc_upd_dma_get_arr(AdcParam *adc)
{
    uint32_t rank = adc->const_h.rankx;
    uint32_t i;
    for (i = 0; i < ADC_NEED_LEN; i++)
    {
        adc_vals_dma_sig[i] =
            adc_vals_dma_arr[ADC_NEED_LEN * i + rank];
    }
}

inline uint16_t adc_upd_dma_get_val(AdcParam *adc, uint32_t loop)
{
    return adc_vals_dma_arr[ADC_NEED_LEN * loop + adc->const_h.rankx];
}

inline void adc_upd_inject(AdcParam *adc)
{
    const AdcConst *c = &adc->const_h;
    adc->value = (uint16_t)HAL_ADCEx_InjectedGetValue(c->hadcx, c->rankx);
}

inline void adc_max_min_reset(AdcParam *adc)
{
    adc->max = 0;
    adc->min = UINT16_MAX;
}

inline void adc_max_min_upd(AdcParam *adc)
{
    VAR_MAX_MIN_UPD(adc->max, adc->min, adc->value);
}

void adcm_init_load(AdcModelParam *adcm)
{
    const AdcModel *m = adcm->model;
    adcm->scale = ADC_TO_VOL / m->proportion / m->sensitive;
    adcm->shift = m->shift;
}

inline void adcm_offset_set(AdcModelParam *adcm)
{
    adcm->offset = (int32_t)adcm->basic.value;
}

void adcm_update(AdcModelParam *adcm)
{
    adcm->value = (float32_t)((int32_t)adcm->basic.value - adcm->offset) * adcm->scale + adcm->shift;
    VAR_MAX_MIN_UPD(adcm->max, adcm->min, adcm->value);
}

inline void adcm_max_min_reset(AdcModelParam *adcm)
{
    adc_max_min_reset(&adcm->basic);
    adcm->max = 0;
    adcm->min = UINT16_MAX;
}

inline void adcm_max_min_upd(AdcModelParam *adcm)
{
    adc_max_min_upd(&adcm->basic);
    VAR_MAX_MIN_UPD(adcm->max, adcm->min, adcm->value);
}

#endif