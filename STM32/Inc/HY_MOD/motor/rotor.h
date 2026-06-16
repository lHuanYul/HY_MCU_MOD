#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

void motor_rotor_hall_upd(MotorParameter *motor);
void motor_rotor_speed_upd(MotorParameter *motor);

#endif