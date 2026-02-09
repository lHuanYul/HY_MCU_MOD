#include "HY_MOD/infrared/callback.h"
#ifdef HY_MOD_IR_TRSM

#include "HY_MOD/main/variable_cal.h"

void ir_trsm_cb(IRParameter *ir, TIM_HandleTypeDef *htim)
{
    if (htim != ir->const_h.Master_htimx) return;
    uint32_t TIM_CHANNEL_x = ir->const_h.Master_TIM_CHANNEL_x;
    switch (ir->state)
    {
        case IR_STATE_LEADER:
        {
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_x,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_x) + ir->time->header_space);
            ir->state = IR_STATE_DATA;
            ir->byte = 0;
            ir->bit = 0;
            break;
        }
        case IR_STATE_DATA:
        {
            if (ir->byte < ir->len)
            {
                uint8_t last = ir->list[ir->byte].size * 2 - 1;
                if (ir->bit % 2 == 0)
                {
                    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_x,
                            __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_x) + ir->time->bit_mark);
                }
                else
                {
                    if(BIT_SNG_CHK(ir->list[ir->byte].data, ir->bit / 2))
                    {
                        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_x,
                            __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_x) + ir->time->bit_1);
                    }
                    else
                    {
                        __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_x,
                            __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_x) + ir->time->bit_0);
                    }
                }
                if (ir->bit >= last)
                {
                    ir->byte++;
                    ir->bit = 0;
                }
                else ir->bit++;
                break;
            }
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_x,
                __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_x) + ir->time->bit_mark);
            ir->state = IR_STATE_END;
            break;
        }
        case IR_STATE_END:
        {
            // __HAL_TIM_SET_COMPARE(ir->const_h.Slave_htimx,
            //     ir->const_h.Slave_TIM_CHANNEL_x, 0);
            HAL_TIM_PWM_Stop(ir->const_h.Slave_htimx, ir->const_h.Slave_TIM_CHANNEL_x);
            HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_x);
            ir->state = IR_STATE_CD;
        }
        default: break;
    }
}

#endif