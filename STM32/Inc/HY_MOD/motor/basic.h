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

typedef struct MotorCalcuConst
{
    //
    float32_t   pwm_f;
    // PWM 控制定時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / PWM_timer_clock
    float32_t   pwm_T;
    // 霍爾計時器的實際計數頻率
    // = HALL_timer_clock / (PSC + 1)
    float32_t   hall_f;
    // 霍爾計時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / HALL_timer_clock
    float32_t   hall_T;
    //
    float32_t   pwm_it_f;
    // 霍爾間隔 → 輸出軸轉速(omega) 轉換常數
    // OMEGA = [SPD_tim_f * 2 * pi / 60] / [6 × (POLE/2) × GEAR × htim_cnt]
    float32_t   omega_fbk;
    // PWM 週期 → 電角度內插轉換常數
    // Δθ_elec(rad) = [ (TIM_tim_t * ARR) / ELE_tim_t ] × (π/3) / htim_cnt
    float32_t   foc_it_angle_itpl;
} MotorCalcuConst;

typedef enum MotorCtrl
{
    MOTOR_CTRL_INIT,
    MOTOR_CTRL_TEST_HIGH,
    MOTOR_CTRL_TEST_LOW,
    MOTOR_CTRL_TEST_WAVE,
    MOTOR_CTRL_120,         // 普通
    MOTOR_CTRL_120_T,       // 普通
    MOTOR_CTRL_120_DUTY,    // 旋轉 
    MOTOR_CTRL_120_SIM,     // 模擬
    MOTOR_CTRL_120_SW,      // 旋轉方向轉換
    MOTOR_CTRL_FOC_INIT,    // 初始 Todo
    MOTOR_CTRL_FOC,         // 普通
    MOTOR_CTRL_FOC_SIM,     // 模擬
    MOTOR_CTRL_FOC_POS,     // 轉子定位
    MOTOR_CTRL_FOC_ROT_CMD, // 轉子定位(外部指令增加角度)
    MOTOR_CTRL_FOC_ROT_AUTO,// 旋轉(內部自動加角度)
    MOTOR_CTRL_FOC_OL_VDQ,  // 旋轉
    MOTOR_CTRL_FOC_OL_IQ,   // 旋轉
} MotorCtrl;

// Control Parameter
typedef struct MotorCtrlParam
{
    MotorCtrl    ref_ori;
    MotorCtrl    ref_fix;
} MotorCtrlParam;

typedef enum MotorRot
{
    MOTOR_ROT_COAST,
    MOTOR_ROT_BREAK,
    MOTOR_ROT_NORMAL,
    MOTOR_ROT_LOCK,
    MOTOR_ROT_LOCK_FIN,
} MotorRot;

// Rotate Parameter
typedef struct MotorRotParam
{
    MotorRot    ref_ori;
    MotorRot    ref_fix;
} MotorRotParam;

// SPD Parameter
typedef struct MotorSpdParame
{
    float32_t   ref_rpm;
    float32_t   ref_omega;
    float32_t   fbk_rpm;
    float32_t   fbk_omega;
    float32_t   save_stop_omega;
} MotorSpdParame;

typedef struct MotorADC
{
    union {
        struct {
            AdcCurrentParameter *adc_u;
            AdcCurrentParameter *adc_v;
            AdcCurrentParameter *adc_w;
        };
        AdcCurrentParameter *adc_uvw[3];
    };
    union {
        struct {
            // Per-Unit
            float32_t u;
            // Per-Unit
            float32_t v;
            // Per-Unit
            float32_t w;
        };
        // Per-Unit
        float32_t uvw[3];
    };
    // 應接近0
    float32_t   total;
} MotorADC;

// Hall Parameter
typedef struct MotorHallParam
{
    // 霍爾跳變間隔 頭id
    volatile uint8_t    time_hist_head;
    // 霍爾跳變間隔 長度
    volatile uint8_t    time_hist_len;
    // 霍爾跳變間隔時間
    uint32_t            time_hist[MOTOR_SPD_CNT];
    // 目前霍爾相位
    volatile uint8_t    current;
    // 上次霍爾相位
    uint8_t             last;

    volatile uint8_t    wrong;
    // Todo
    uint8_t             auto_spin;
    // 停轉時間
    uint32_t            stop_tick;
} MotorHallParam;

typedef struct MotorSLessParam
{
} MotorSLessParam;

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
typedef struct MotorDEGParam
{
    // 反轉
    bool                reverse;
    // DEG duty值
    float32_t           duty_val;
    // DEG uvw duty
    MotorPhaseDuty      duty_h;

    PI_CTRL             pi_omega;

    PI_CTRL             pi_current;
} MotorDEGParam;

// FOC Parameter
typedef struct MotorFOCParam
{
    uint32_t            init_cnt;
    // clarke
    CLARKE              clarke_h;
    // 目前霍爾相位
    float32_t           hall_rad;
    // 轉子位置
    float32_t           rotor_rad;
    // FOC 應補角度 (Angle Interpolation)
    volatile float32_t  rad_itpl;
    // FOC 角度累積插值 rad_acc += rad_itpl; 過一霍爾中斷後重置
    float32_t           rad_acc;
    // park
    PARK                park_h;

    PI_CTRL             pi_omega;
    // 
    PI_CTRL             pi_Id_h;
    // 
    PI_CTRL             pi_Iq_h;
    // 磁場位置
    float32_t           magn_rad;
    // ipark
    IPARK               ipark_h;
    // svgendq
    SVGENDQ             svgendq_h;
    // Vref_s = SQRT3 * Vref / Vbus
    float32_t           Vref_s;
    // FOC duty
    MotorPhaseDuty      duty_h;
} MotorFOCParam;

// DBG: debug
typedef struct MotorDbg
{
    float32_t   hall_rad[8];
    uint8_t     hall_last;
} MotorDbg;

typedef struct MotorHistoryArray
{
    uint8_t             tick;
    volatile float32_t  id[10];
    volatile float32_t  iq[10];
} MotorHistoryArray;

typedef struct MotorParameter
{
    // 常數
    const MotorConst    const_h;
    // 計算常數
    MotorCalcuConst     calcu_h;
    
    uint32_t            init_cnt;
    // 馬達控制模式 (120度與foc以及細部)
    MotorCtrlParam      ctrl_h;
    // 馬達旋轉模式 (滑行與剎車等)
    MotorRotParam       rotate_h;
    // 從座往轉子 順時針為負
    MotorSpdParame      speed_h;
    // 計時中斷計數
    uint32_t            tim_tick;
    // 電流 ADC
    MotorADC            adc_h;
    // 霍爾
    MotorHallParam      hall_h;
    // 無感測
    MotorSLessParam     sless_h;
    // 120度控制
    MotorDEGParam       deg_h;
    // FOC控制
    MotorFOCParam       foc_h;
    // PWM load duty
    MotorPhaseDuty      duty_load;

    MotorDbg            dbg_h;

    MotorHistoryArray   history;
} MotorParameter;

#endif