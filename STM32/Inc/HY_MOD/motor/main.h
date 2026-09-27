#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

void motor_setup(MotorParameter *motor);
void motor_main(MotorParameter *motor);

#endif