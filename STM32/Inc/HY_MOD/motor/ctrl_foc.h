#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"

void vec_ctrl_adcs_reset(MotorParameter *motor);
void vec_ctrl_adcs_upd(MotorParameter *motor);
void vec_ctrl_hall_angle_trf(MotorParameter *motor);
Result vec_ctrl_hall_angle_chk(MotorParameter *motor);
void vec_ctrl_clarke(MotorParameter *motor);
void vec_ctrl_park(MotorParameter *motor);
void vec_ctrl_pi_id_iq(MotorParameter *motor);
void vec_ctrl_ipark(MotorParameter *motor);
void vec_ctrl_svgen(MotorParameter *motor);
void vec_ctrl_svpwm(MotorParameter *motor);

#endif