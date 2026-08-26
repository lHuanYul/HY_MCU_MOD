#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

uint8_t motor_rotor_hall_get(MotorParameter *motor);
void motor_rotor_hall_curr_upd(MotorParameter *motor);
void motor_rotor_hall_prev_set(MotorParameter *motor, uint8_t value);
void motor_rotor_hall_upd(MotorParameter *motor);
void motor_rotor_speed_upd(MotorParameter *motor);
void motor_rotor_stop(MotorParameter *motor);

#endif