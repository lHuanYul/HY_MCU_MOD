#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/motor/rotor.h"
#include "HY_MOD/main/fn_state.h"

/**
 * @brief 初始化馬達控制系統 (包含常數換算、ADC校正、PWM與霍爾定時器啟動)
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_init(MotorParameter *motor);
/**
 * @brief 直接寫入三相 PWM 定時器的比較暫存器 (CCR)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param u     U 相 CCR 原始計數值
 * @param v     V 相 CCR 原始計數值
 * @param w     W 相 CCR 原始計數值
 */
void motor_timer_load_inner(MotorParameter *motor, uint32_t u, uint32_t v, uint32_t w);
/**
 * @brief 根據當前三相占空比 (final_duty) 限幅並計算寫入 PWM 定時器
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_timer_load(MotorParameter *motor);
/**
 * @brief 初始化三相電流 (及電壓) ADC 取樣結構體與校正參數
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_adcs_init(MotorParameter *motor);
/**
 * @brief 更新注入組 ADC 原始取樣值並換算 (規定流入馬達繞組為正電流)
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_adcs_I_inject(MotorParameter *motor);

void motor_adcs_V_inject(MotorParameter *motor);

void motor_adcs_I_calculate(MotorParameter *motor);

#define DEMAG_BLANKING_TICKS 3 // 換相初期消磁遮蔽 (前3個tick不判定)
void motor_adcs_V_calculate(MotorParameter *motor);
/**
 * @brief 設定目標轉速 (從尾端向轉子看，逆時針 CCW 為正)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param rpm   目標轉速 (正負代表轉向): 
 *              在 DUTY 模式下作為占空比使用，範圍 [-1.0, 1.0]，正負同樣代表轉向。
 */
void motor_set_speed(MotorParameter *motor, float32_t rpm);
/**
 * @brief 設定馬達運轉/停機模式 (滑行、煞車、鎖軸、正常運轉等)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param mode  目標旋轉模式
 */
void motor_set_rotate_mode(MotorParameter *motor, MotorRotateMode mode);
/**
 * @brief 使用者請求切換馬達控制模式 (更新 ref_user 並自動配置內部計數與狀態)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param ctrl  目標控制模式 (120度方波、FOC、開環測試等)
 */
void motor_set_ctrl_mode(MotorParameter *motor, MotorCtrlMode ctrl);
/**
 * @brief 系統內部狀態機切換控制模式 (切換下橋 PWMN GPIO/AF 模式並配置計時器溢位週期)
 * 
 * @param motor 馬達控制參數結構體指標
 * @param ctrl  目標控制模式
 */
void motor_switch_ctrl_system(MotorParameter *motor, MotorCtrlMode ctrl);

void motor_main(MotorParameter *motor);

#endif