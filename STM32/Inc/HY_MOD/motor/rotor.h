#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

/**
 * @brief 讀取硬體三相霍爾 GPIO 腳位並轉換為標么電角度扇區 (0~5)
 * 
 * @param motor 馬達控制參數結構體指標
 * @return uint8_t 轉子電角度扇區索引 (無效霍爾訊號回傳 10)
 */
uint8_t motor_rotor_hall_get(MotorParameter *motor);
/**
 * @brief 設定霍爾定時器自動重裝載值 (ARR / Overflow)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param of    自動重裝載計數值
 */
void motor_rotor_set_overflow(MotorParameter *motor, uint32_t of);
/**
 * @brief 切換轉子位置感測模式 (霍爾中斷、PWM取樣、模擬步進或無感測)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param mode  目標感測模式
 */
void motor_rotor_mode_change(MotorParameter *motor, MotorSensorMode mode);
/**
 * @brief 軟體觸發霍爾定時器捕獲與更新事件 (用於模擬模式或無感測模式步進觸發)
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_rotor_phase_trigger(MotorParameter *motor);
/**
 * @brief 內部霍爾定時器中斷處理程序 (更新扇區、記錄換相間隔並換算角速度回授)
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_rotor_hall_timer_cbi(MotorParameter *motor);
/**
 * @brief 內部 PWM 週期位置更新程序 (用於以 PWM 中斷為週期的霍爾取樣或無感觀測器更新)
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_rotor_pwm_cbi(MotorParameter *motor);
/**
 * @brief 內部轉子停機重置程序 (清空換相時間歷史緩衝區，並將速度回授歸零)
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_rotor_stop_cbi(MotorParameter *motor);

#endif