#include "HY_MOD/dht/callback.h"
#ifdef HY_MOD_STM32_DHT

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
*/
void dht_tim_PE_cb(DhtParametar *dht11, TIM_HandleTypeDef *htim)
{
    if (htim != dht11->const_h.htimx) return;
    dht11->tim_mode_pwm = 0;
    dht_tim_mode_switch(dht11);
}

/*
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
*/
void dht_tim_IC_cb(DhtParametar *dht11, TIM_HandleTypeDef *htim)
{
    if (htim != dht11->const_h.htimx) return;
    // if (htim->Channel != dht11->const_h.HAL_TIM_ACTIVE_CHANNEL_x) return;

    uint32_t cnt = __HAL_TIM_GET_COMPARE(dht11->const_h.htimx, dht11->const_h.TIM_CHANNEL_x);
    uint32_t diff;
    if (cnt >= dht11->last_cnt)
        diff = cnt - dht11->last_cnt;
    else
        // 處理 Timer 歸零的情況 (ARR + 1 + cnt - last)
        diff = (dht11->const_h.htimx->Init.Period - dht11->last_cnt) + cnt + 1;
    dht11->last_cnt = cnt;

    if (HAL_GPIO_ReadPin(dht11->const_h.gpio.GPIOx, dht11->const_h.gpio.GPIO_Pin_x)) return;

    switch (dht11->state)
    {
        case DHT_STATE_RESPONSE:
        {
            if (diff >= 70 && diff < 95)
            {
                dht11->state = DHT_STATE_DATA;
                dht11->bit_x = 0;
                dht11->byte_x = 0;
                dht11->raw = 0;
            }
            // else dht11->state = DHT_STATE_ERROR;
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
                    if (dht11->raw != (uint8_t)(dht11->chk_sum & 0xFF))
                    {
                        dht11->state = DHT_STATE_ERROR;
                        break;
                    }
                    dht11->state = DHT_STATE_FINISHED;

                    uint16_t raw_hum = ((uint16_t)dht11->wet_i << 8) | dht11->wet_d;
                    dht11->wet = (float32_t)raw_hum / 10.0f;

                    uint16_t raw_temp = ((uint16_t)dht11->tmp_i << 8) | dht11->tmp_d;
                    float32_t temp_val = (float32_t)(raw_temp & 0x7FFF); // 去除符號位
                    if (raw_temp & 0x8000) // 檢查最高位是否為 1 (負溫)
                    {
                        temp_val *= -1.0f;
                    }
                    dht11->tmp = temp_val / 10.0f;
                    // dht11->wet = (float32_t)dht11->wet_i + ((float32_t)dht11->wet_d / 10.0f);
                    // dht11->tmp = (float32_t)dht11->tmp_i + ((float32_t)dht11->tmp_d / 10.0f);
                    __HAL_TIM_DISABLE(dht11->const_h.htimx);
                    break;
                }
                default:
                {
                    dht11->state = DHT_STATE_ERROR;
                    break;
                }
            }
            dht11->byte_x++;
            dht11->raw = 0;
            break;
        }
        default:
        {
            dht11->state = DHT_STATE_ERROR;
            break;
        }
    }
}

#endif