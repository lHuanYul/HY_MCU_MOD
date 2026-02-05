#include "HY_MOD/infrared/main.h"
#ifdef HY_MOD_IR_TRSM

void ir_trsm_start(IRParameter *ir)
{
    if (ir->state != IR_STATE_IDLE) return;
    TIM_HandleTypeDef *htimx = ir->const_h.Master_htimx;
    uint32_t TIM_CHANNEL_x = ir->const_h.Master_TIM_CHANNEL_x;
    ir->state = IR_STATE_LEADER;
    __HAL_TIM_SET_OCMODE(htimx, TIM_CHANNEL_x, TIM_OCMODE_FORCED_ACTIVE);
    __HAL_TIM_SET_OCMODE(htimx, TIM_CHANNEL_x, TIM_OCMODE_TOGGLE);
    __HAL_TIM_SET_COMPARE(htimx, TIM_CHANNEL_x,
        __HAL_TIM_GET_COUNTER(htimx) + IR_NEC_LEADER_H);
    HAL_TIM_PWM_Start(infrared_h.const_h.Slave_htimx, infrared_h.const_h.Slave_TIM_CHANNEL_x);
    HAL_TIM_OC_Start_IT(htimx, TIM_CHANNEL_x);
}

void StartIRTask(void *argument)
{
    IRParameter *ir = &infrared_h;
    ir_code_gen(ir, IR_CODE_HERAN_FAN_POWER);
}

#endif