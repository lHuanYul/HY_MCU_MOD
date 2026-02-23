#include "HY_MOD/infrared/callback.h"
#ifdef HY_MOD_IR_TRSM

#include "HY_MOD/main/variable_cal.h"

void ir_trsm_cb(IRParameter *ir, TIM_HandleTypeDef *htimx)
{
    if (htimx != ir->const_h.Master_htimx) return;
    uint32_t TIM_CHANNEL_x = ir->const_h.Master_TIM_CHANNEL_x;
    uint16_t cmp = __HAL_TIM_GET_COMPARE(htimx, TIM_CHANNEL_x);
    if (ir->id < ir->len)
    {
        cmp += ir_code_time(ir->data[++ir->id]);
        __HAL_TIM_SET_COMPARE(htimx, TIM_CHANNEL_x, cmp);
    }
    else
    {
        HAL_TIM_PWM_Stop(ir->const_h.Slave_htimx, ir->const_h.Slave_TIM_CHANNEL_x);
        HAL_TIM_OC_Stop_IT(htimx, TIM_CHANNEL_x);
        ir->state = IR_STATE_CD;
    }
}

#endif