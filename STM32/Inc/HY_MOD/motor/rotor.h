#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

uint8_t motor_rotor_hall_get(MotorParameter *motor);
void motor_rotor_set_overflow(MotorParameter *motor, uint32_t of);
void motor_rotor_mode_change(MotorParameter *motor, MotorSensorMode mode);
void motor_rotor_phase_trigger(MotorParameter *motor);
void motor_rotor_hall_timer_cbi(MotorParameter *motor);
void motor_rotor_pwm_cbi(MotorParameter *motor);
void motor_rotor_stop_cbi(MotorParameter *motor);

#endif