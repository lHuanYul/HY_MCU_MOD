#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/motor/pi.h"
#include "HY_MOD/motor/clarke.h"
#include "HY_MOD/motor/park.h"
#include "HY_MOD/motor/svgendq.h"
#include "HY_MOD/adc_current/basic.h"
#include "cordic.h"

extern const uint8_t hall_seq_clw[8];
extern const uint8_t hall_seq_ccw[8];

typedef struct MotorConst
{
    // HALL PIN
    GPIO_TypeDef        *Hall_GPIOx[3];
    uint16_t            Hall_GPIO_Pin_x[3];
    // PWM timer
    TIM_HandleTypeDef   *PWM_htimx;
    uint32_t            PWM_TIM_CHANNEL_x[3];
    uint32_t            *PWM_tim_clk;
    uint32_t            PWM_MID_TIM_CH_x;
    // deg control L
    GPIO_TypeDef        *Coil_GPIOx[3];
    uint16_t            Coil_GPIO_Pin_x[3];
    // 霍爾計時器
    TIM_HandleTypeDef   *Hall_htimx;
    uint32_t            *Hall_tim_clk;
    // 馬達data sheet
    float32_t           rated_trorque;
    float32_t           rated_current;
    float32_t           peak_trorque;
    float32_t           peak_current;
} MotorConst;

typedef enum MotorModeControl
{
    MOTOR_CTRL_INIT,
    MOTOR_CTRL_120,
    MOTOR_CTRL_180,
    MOTOR_CTRL_FOC_RATED,
    MOTOR_CTRL_FOC_PEAK,
} MotorModeControl;

typedef enum MotorModeRotate
{
    MOTOR_ROT_COAST,
    MOTOR_ROT_BREAK,
    MOTOR_ROT_NORMAL,
    MOTOR_ROT_LOCK,
    MOTOR_ROT_LOCK_FIN,
} MotorModeRotate;

typedef enum DirectionState
{
    DIRECTION_NORMAL,
    DIRECTION_SWITCHING,
} DirectionState;

typedef struct MotorRpm
{
    volatile bool reverse;
    volatile float32_t value;
} MotorRpm;

typedef struct MotorHistoryData
{
    float32_t spd_ref;
    float32_t spd_fbk;
} MotorHistoryData;

typedef struct MotorHistoryArray
{
    MotorHistoryData data[MOTOR_HISTORY_LEN];
    uint16_t head;
    bool is_full;
    uint32_t cnt;
} MotorHistoryArray;

typedef struct MotorParameter
{
    // 常數
    const MotorConst    const_h;
    // 霍爾間隔 → 輸出軸轉速(RPM) 轉換常數
    // RPM = [SPD_tim_f * 60] / [6 × (POLE/2) × GEAR × htim_cnt]
    float32_t           tfm_rpm_fbk;

    float32_t           tfm_pwm_period;
    // PWM 週期 → 電角度內插轉換常數
    // Δθ_elec(rad) = [ (TIM_tim_t * ARR) / ELE_tim_t ] × (π/3) / htim_cnt
    float32_t           tfm_foc_it_angle_itpl;

    float32_t           tfm_duty_Iq;

    float32_t           dbg_pwm_freq;
    // 計時器頻率
    float32_t           dbg_tim_it_freq;

    bool                fdcan_enable;

    bool                fdcan_send;
    
    uint32_t            alive_tick;
    // 馬達控制模式
    MotorModeControl    mode_control;
    // 馬達旋轉模式
    MotorModeRotate     mode_rot_user;
    // 馬達旋轉模式
    MotorModeRotate     mode_rot_ref;

    MotorRpm            rpm_user;

    MotorRpm            rpm_feedback;

    MotorRpm            rpm_reference;

    float32_t           rpm_save_stop;

    DirectionState      dict_state;
    // 目前霍爾相位
    volatile float32_t  exti_hall_rad;
    // 電角度
    float32_t           elec_theta_rad;
    // FOC 應補角度 (Angle Interpolation)
    volatile float32_t  foc_angle_itpl;
    // FOC 角度累積插值
    // foc_angle_acc += foc_angle_itpl; 過一霍爾中斷後重置
    float32_t           foc_angle_acc;
    // 霍爾計數
    uint8_t             exti_hall_acc;
    // 計時中斷計數
    uint32_t            tim_it_acc;
    // 停轉時間
    uint32_t            stop_spin_time;

    volatile uint32_t   hall_wrong;
    // 目前霍爾相位
    volatile uint8_t    hall_current;

    volatile uint16_t   hall_delay;

    uint8_t             hall_start;
    // 上次霍爾相位
    uint8_t             hall_chk_last;

    // uint8_t             hall_chk_curent;
    // 從尾往轉子 順時針value為負
    PI_CTRL             pi_speed;
    
    // 電流 ADC
    AdcCurrentParameter *adc_a;
    // 電流 ADC
    AdcCurrentParameter *adc_b;
    // 電流 ADC
    AdcCurrentParameter *adc_c;
    // clarke
    CLARKE              clarke;
    // park
    PARK                park;

    PI_CTRL             pi_Iq;

    PI_CTRL             pi_Id;
    // ipark
    IPARK               ipark;
    // svgendq
    SVGENDQ             svgendq;
    
    float32_t           v_ref;
    // DEG duty
    volatile float32_t  deg_duty;
    // FOC duty
    float32_t           foc_duty_u;
    // FOC duty
    float32_t           foc_duty_v;
    // FOC duty
    float32_t           foc_duty_w;
    // PWM duty
    float32_t           pwm_duty_u;
    // PWM duty
    float32_t           pwm_duty_v;
    // PWM duty
    float32_t           pwm_duty_w;

    MotorHistoryArray   history;
} MotorParameter;
extern MotorParameter motor_h;
void motor_init(MotorParameter *motor);

// 從尾往轉子 順時針為負
void motor_set_rpm(MotorParameter *motor, bool reverse, float32_t speed);
void motor_set_rotate_mode(MotorParameter *motor, MotorModeRotate mode);
void motor_alive(MotorParameter *motor);
void motor_switch_ctrl(MotorParameter *motor, MotorModeControl ctrl);
void motor_pwm_load(MotorParameter *motor);
void motor_history_write(MotorParameter *motor);

#endif