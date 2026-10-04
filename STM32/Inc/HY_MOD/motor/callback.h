#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#define MOTOR_HAL_TIM_PeriodElapsedCB_CALL(motor, htim) \
    do { \
        if (INSTANCE_CHK((htim), (motor).system.const_h.Hall_htimx)) \
        { \
            motor_stop_cb(&(motor)); \
        } \
    } while (0)
/**
 * @brief 馬達停機保護回呼函式 (重置轉子濾波緩衝、120度速度環與 FOC 電流環 PID 狀態)
 * 
 * 在中斷處理常式中呼叫範例：
 * ```c
 * void HAL_TIM_PeriodElapsedCallback_OWN(TIM_HandleTypeDef *htim)
 * {
 *     MOTOR_HAL_TIM_PeriodElapsedCB_CALL(motor_h, htim);
 * }
 * ```
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_stop_cb(MotorParameter *motor);

#define MOTOR_HAL_TIM_IC_CaptureCB_CALL(motor, htim) \
    do { \
        if ( \
            INSTANCE_CHK((htim), (motor).system.const_h.Hall_htimx) && \
            ((htim)->Channel == (motor).system.const_h.Hall_Active_ch) \
        ) { \
            motor_hall_timer_cb(&(motor)); \
        } \
    } while (0)
/**
 * @brief 霍爾定時器輸入捕獲中斷回呼函式 (轉子角度更新、速度換算與 FOC 補償重置)
 * 
 * 在中斷處理常式中呼叫範例：
 * ```c
 * void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
 * {
 *     MOTOR_HAL_TIM_IC_CaptureCB_CALL(motor_h, htim);
 * }
 * ```
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_hall_timer_cb(MotorParameter *motor);

#define MOTOR_HAL_ADCEx_InjectedConvCpltCB_CALL(motor, hadc) \
    do { \
        if (INSTANCE_CHK((hadc), (motor).adc_h.adc_ui.basic.hadcx)) \
        { \
            motor_pwm_cb(&(motor)); \
        } \
    } while (0)
/**
 * @brief PWM / ADC 週期中斷主回呼函式 (20kHz 執行，處理 ADC 換算、速度環控制、反轉保護與換相/FOC 計算)
 * 
 * 在中斷處理常式中呼叫範例：
 * ```c
 * void HAL_ADCEx_InjectedConvCpltCallback(TIM_HandleTypeDef *htim)
 * {
 *     MOTOR_HAL_ADCEx_InjectedConvCpltCB_CALL(motor_h, hadc);
 * }
 * ```
 * 
 * @param motor 馬達控制參數結構體指標
 */
void motor_pwm_cb(MotorParameter *motor);

#endif