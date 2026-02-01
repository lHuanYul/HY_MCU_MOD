#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"

void motor_hall_exti_cb(MotorParameter *motor);
void motor_pwm_cb(MotorParameter *motor);
void motor_stop_cb(MotorParameter *motor);

#endif