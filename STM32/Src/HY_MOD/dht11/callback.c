#include "HY_MOD/dht11/callback.h"
#ifdef HY_MOD_STM32_DHT11

uint32_t hycb[2] = {0};
void dht11_tim_PE_cb(Dht11Parametar *dht11, TIM_HandleTypeDef *htim)
{
    if (htim != dht11->const_h.htimx) return;
    hycb[0]++;
    dht11->tim_mode_pwm = 0;
    dh11_tim_mode_switch(dht11);
}

void dht11_tim_IC_cb(Dht11Parametar *dht11, TIM_HandleTypeDef *htim)
{
    if (htim->Channel != dht11->const_h.HAL_TIM_ACTIVE_CHANNEL_x) return;
    hycb[1]++;
    uint32_t cnt = __HAL_TIM_GET_COMPARE(dht11->const_h.htimx, dht11->const_h.TIM_CHANNEL_x);
    uint32_t diff;
    if (cnt >= dht11->last_cnt)
        diff = cnt - dht11->last_cnt;
    else
        // 處理 Timer 歸零的情況 (ARR + 1 + cnt - last)
        diff = (dht11->const_h.htimx->Init.Period - dht11->last_cnt) + cnt + 1;
    dht11->last_cnt = cnt;

    if (HAL_GPIO_ReadPin(dht11->const_h.gpio.GPIOx, dht11->const_h.gpio.GPIO_Pin_x) != GPIO_PIN_RESET) return;
    switch (dht11->state)
    {
        case DHT_STATE_IGNORE:
        {
            dht11->state = DHT_STATE_RESPONSE;
            break;
        }
        case DHT_STATE_RESPONSE:
        {
            if (diff >= 70 && diff <= 90)
            {
                dht11->state = DHT_STATE_DATA;
                dht11->bit_x = 0;
                dht11->byte_x = 0;
            }
            else dht11->state = DHT_STATE_ERROR;
            break;
        }
        case DHT_STATE_DATA:
        {
            uint8_t bit_val = (diff > 50) ? 1 : 0;
            dht11->raw = (dht11->raw << 1) | bit_val;
            dht11->bit_x++;
            if (dht11->bit_x < 8) break;
            dht11->bit_x = 0;
            switch (dht11->byte_x)
            {
                case 0: dht11->wet_i = dht11->raw; break;
                case 1: dht11->wet_d = dht11->raw; break;
                case 2: dht11->tmp_i = dht11->raw; break;
                case 3: dht11->tmp_d = dht11->raw; break;
                case 4:
                {
                    dht11->chk_sum =
                        dht11->wet_i + dht11->wet_d +
                        dht11->tmp_i + dht11->tmp_d;
                    if (dht11->raw != (dht11->chk_sum & 0xFF))
                    {
                        dht11->state = DHT_STATE_ERROR;
                        break;
                    }
                    dht11->state = DHT_STATE_FINISHED;
                    dht11->wet = (float32_t)dht11->wet_i + ((float32_t)dht11->wet_d / 10.0f);
                    dht11->tmp = (float32_t)dht11->tmp_i + ((float32_t)dht11->tmp_d / 10.0f);
                    break;
                }
                default:
                {
                    dht11->state = DHT_STATE_ERROR;
                    break;
                }
            }
            dht11->byte_x++;
            break;
        }
        default:
        {
            dht11->state = DHT_STATE_ERROR;
            break;
        }
    }
    
    switch (dht11->state)
    {
        case DHT_STATE_ERROR:
        case DHT_STATE_FINISHED:
        {
            __HAL_TIM_DISABLE(dht11->const_h.htimx);
            break;
        }
        default: break;
    }
}

#endif