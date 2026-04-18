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

typedef union MotorPhaseGPIOData
{
    struct {
        GPIOData u;
        GPIOData v;
        GPIOData w;
    };
    GPIOData uvw[3];
} MotorPhaseGPIOData;

typedef struct MotorPhasePwmCH
{
    union {
        struct {
            uint32_t u;
            uint32_t v;
            uint32_t w;
        };
        uint32_t uvw[3];
    };
    uint32_t mid;
} MotorPhasePwmCH;

typedef struct MotorPwmNGpio
{
    uint32_t MODEx;
    uint32_t MODEx_0;
    uint32_t MODEx_1;
} MotorPwmNGpio;
typedef union MotorPhaseNPwmGPIO
{
    struct {
        MotorPwmNGpio u;
        MotorPwmNGpio v;
        MotorPwmNGpio w;
    };
    MotorPwmNGpio uvw[3];
} MotorPhaseNPwmGPIO;

// CONST: constant
typedef struct MotorConst
{
    // HALL PIN
    MotorPhaseGPIOData  Hall_GPIO;
    // PWM timer
    TIM_HandleTypeDef   *PWM_htimx;
    uint32_t            *PWM_tim_clk;
    MotorPhasePwmCH     PWM_TIM_CH_x;
    MotorPhaseGPIOData  PWMN_GPIO;
    MotorPhaseNPwmGPIO  PWMN_GPIO_set;
    // 霍爾計時器
    TIM_HandleTypeDef   *Hall_htimx;
    uint32_t            *Hall_tim_clk;
    // 馬達data sheet
    float32_t           rated_trorque;
    float32_t           rated_current;
    float32_t           peak_trorque;
    float32_t           peak_current;
} MotorConst;

// TFM: transformation
typedef struct MotorTfm
{
    // 霍爾間隔 → 輸出軸轉速(RPM) 轉換常數
    // RPM = [SPD_tim_f * 60] / [6 × (POLE/2) × GEAR × htim_cnt]
    float32_t           rpm_fbk;
    // PWM 週期 → 電角度內插轉換常數
    // Δθ_elec(rad) = [ (TIM_tim_t * ARR) / ELE_tim_t ] × (π/3) / htim_cnt
    float32_t           foc_it_angle_itpl;

    float32_t           duty_Iq;
} MotorTfm;

// DBG: debug
typedef struct MotorDbg
{
    // 計時器頻率
    float32_t   pwm_freq;

    float32_t   current_zero;

    uint8_t     his_cnt;
    float32_t   his_test_rad[12];
    float32_t   his_ele_rad[12];
    uint8_t     his_sector[12];
} MotorDbg;

typedef enum MotorCtrl
{
    MOTOR_CTRL_INIT,
    MOTOR_CTRL_TEST_HIGH,
    MOTOR_CTRL_TEST_LOW,
    MOTOR_CTRL_TEST_WAVE,
    MOTOR_CTRL_120, // 普通
    MOTOR_CTRL_120_SIM, // 模擬
    MOTOR_CTRL_120_SW, // 旋轉方向轉換
    MOTOR_CTRL_FOC, // 普通
    MOTOR_CTRL_FOC_SIM, // 模擬
    MOTOR_CTRL_FOC_INIT, // 初始 Todo
    MOTOR_CTRL_FOC_POS, // 轉子定位
    MOTOR_CTRL_FOC_ROT, // 強制旋轉
} MotorCtrl;

// Control Parameter
typedef struct MotorCtrlParameter
{
    MotorCtrl    ref_ori;
    MotorCtrl    ref_fix;
} MotorCtrlParameter;

typedef enum MotorRot
{
    MOTOR_ROT_COAST,
    MOTOR_ROT_BREAK,
    MOTOR_ROT_NORMAL,
    MOTOR_ROT_LOCK,
    MOTOR_ROT_LOCK_FIN,
} MotorRot;

// Rotate Parameter
typedef struct MotorRotParameter
{
    MotorRot    ref_ori;
    MotorRot    ref_fix;
} MotorRotParameter;

typedef struct MotorRpm
{
    volatile bool       reverse;
    volatile float32_t  value;
} MotorRpm;

// RPM Parameter
typedef struct MotorRpmParameter
{
    MotorRpm    ref_ori;
    MotorRpm    ref_fix;
    MotorRpm    fb;
    float32_t   save_stop_val;
} MotorRpmParameter;

// Hall Parameter
typedef struct MotorHallParameter
{
    // 霍爾計數
    uint8_t             it_cnt;
    // 目前霍爾相位
    volatile uint8_t    current;

    volatile uint8_t    wrong;

    volatile uint16_t   delay;

    uint32_t            time_cnt;
    
    uint8_t             auto_spin;
    // 上次霍爾相位
    uint8_t             chk_last;
    // 停轉時間
    uint32_t            stop_tick;
} MotorHallParameter;

typedef union MotorPhaseDuty
{
    struct {
        float32_t u;
        float32_t v;
        float32_t w;
    };
    float32_t uvw[3];
} MotorPhaseDuty;

// DEG Parameter
typedef struct MotorDEGParameter
{
    // DEG duty
    float32_t           duty_val;
    // DEG duty
    MotorPhaseDuty      duty_h;

    PI_CTRL             pi_rpm;
} MotorDEGParameter;

typedef union MotorADC
{
    struct {
        AdcCurrentParameter *u;
        AdcCurrentParameter *v;
        AdcCurrentParameter *w;
    };
    AdcCurrentParameter *uvw[3];
} MotorADC;

// FOC Parameter
typedef struct MotorFOCParameter
{
    // 電流 ADC
    MotorADC            adc_h;

    uint32_t            init_cnt;
    // 目前霍爾相位
    float32_t           hall_rad;
    // 轉子位置
    float32_t           rotor_rad;
    // clarke
    CLARKE              clarke_h;
    // FOC 應補角度 (Angle Interpolation)
    float32_t           angle_itpl;
    // park
    PARK                park_h;
    // FOC 角度累積插值 angle_acc += angle_itpl; 過一霍爾中斷後重置
    float32_t           angle_acc;

    PI_CTRL             pi_rpm;

    PI_CTRL             pi_Id_h;

    PI_CTRL             pi_Iq_h;
    // 磁場位置
    float32_t           magn_rad;
    // ipark
    IPARK               ipark_h;
    // svgendq
    SVGENDQ             svgendq_h;
    
    float32_t           Vref;
    // FOC duty
    MotorPhaseDuty      duty_h;
} MotorFOCParameter;

typedef struct MotorHistoryArray
{
    uint8_t     tick;
    volatile float32_t  id[10];
    volatile float32_t  iq[10];
} MotorHistoryArray;

typedef struct MotorParameter
{
    // 常數
    const MotorConst    const_h;

    MotorTfm            tfm_h;

    MotorDbg            dbg_h;

    uint32_t            init_cnt;
    // 馬達控制模式
    MotorCtrlParameter  ctrl_h;
    // 馬達旋轉模式
    MotorRotParameter   rotate_h;
    // 從尾往轉子 順時針value為負
    MotorRpmParameter   rpm_h;
    // 計時中斷計數
    uint32_t            tim_tick;

    MotorHallParameter  hall_h;

    MotorDEGParameter   deg_h;

    MotorFOCParameter   foc_h;
    // PWM load duty
    MotorPhaseDuty      duty_load;

    MotorHistoryArray   history;
} MotorParameter;

#endif