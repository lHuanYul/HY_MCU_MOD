#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

void motor_deg_test(MotorParameter *motor);
void motor_deg_120_load(MotorParameter *motor, uint8_t id);
// void deg_ctrl_180_load(MotorParameter *motor);
/**
 * @brief 檢查轉速目標與回授方向，若不一致則請求進入換向過渡狀態
 */
void motor_deg_check_rev(MotorParameter *motor);
/**
 * @brief 監控滑行減速至安全轉速後，正式套用反轉並切回正常控制
 */
void motor_deg_proc_safe_rev(MotorParameter *motor);
void motor_deg_stop_cbi(MotorParameter *motor);

#endif