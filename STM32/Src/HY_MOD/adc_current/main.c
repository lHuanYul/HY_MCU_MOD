#include "HY_MOD/adc_current/main.h"
#ifdef HY_MOD_STM32_ADC_CURRENT

#include "HY_MOD/adc/main.h"

static uint16_t ADC_Values[ADC_COUNT * ADC_NEED_LEN] = {0};

// static uint16_t adc.wnt[2560] = {0};
// static ATTR_UNUSED Result middle(uint8_t adc_id, uint16_t *adc_store)
// {
//     memset(adc.wnt, 0, sizeof(adc.wnt));
//     uint16_t i, val;
//     for (i = 0; i < ADC_NEED_LEN; i++)
//     {
//         val = ADC_Values[i * ADC_COUNT + adc_id];
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
        total += ADC_Values[i * ADC_COUNT + adc_id];
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

Result adc_current_upd(AdcCurrentParameter *adc)
{
    adc_upd_injected(&adc->basic);
    // iir(adc->const_h.rankx, &adc->adc_value);
    adc->current = ((float32_t)(adc->basic.value - adc->val_offset)) * adc->current_trs;
    return RESULT_OK(NULL);
}

void adc_current_reset(AdcCurrentParameter *adc)
{
    adc->current_trs = ADC_TO_VOL / ADC_VOL_SEP / adc->sensitive;
    // average(adc->const_h.id, &adc->adc_value);
    adc->val_offset = adc->basic.value;
}

#endif