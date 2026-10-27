#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/motor/math/pid.h"
#include "HY_MOD/motor/math/clarke.h"
#include "HY_MOD/motor/math/park.h"
#include "HY_MOD/motor/math/svgendq.h"
#include "HY_MOD/adc/basic.h"
#include "cordic.h"

typedef struct MotorModelData
{
    uint8_t     pole;
    float32_t   gear;
    float32_t   rated_current;
    float32_t   rated_voltage;
    // 定子相電阻
    float32_t   rl;
    // 電氣時間常數
    float32_t   tau;
    // 定子相電感 = rl * tau
    float32_t   ll;
    // 霍爾訊號與實際電角補償 霍爾超前實際為負
    float32_t   hall_angle_comp;
    float32_t   deg_spd_Kp;
    float32_t   deg_spd_Ki;
    float32_t   foc_spd_Kp;
    float32_t   foc_spd_Ki;
} MotorModelData;
extern const MotorModelData motor_vehicle;
extern const MotorModelData motor_42BLF01;

typedef struct MotorPwmNGpio
{
    uint32_t    MODEx;
    uint32_t    MODEx_0;
    uint32_t    MODEx_1;
} MotorPwmNGpio;

typedef struct MotorPhaseConst
{
    GPIOData        pwmn_gpio;
    uint32_t        pwm_ch;
    MotorPwmNGpio   pwmn_mode;
} MotorPhaseConst;

typedef struct MotorHallConst
{
    GPIOData    gpio;
    // uint32_t    tim_ch;
} MotorHallConst;

// 預寫好於 Core/Src/main/motor.c
typedef struct MotorConst
{
    // 馬達data sheet
    const MotorModelData    *model;
    // PWM timer
    TIM_HandleTypeDef       *PWM_htimx;
    const uint32_t          *PWM_tim_clk;
    union {
        struct {
            MotorPhaseConst PWM_u, PWM_v, PWM_w;
        };
        MotorPhaseConst     PWM_uvw[3];
    };
    uint32_t                PWM_mid_ch;
    // Hall timer
    TIM_HandleTypeDef       *Hall_htimx;
    uint32_t                *Hall_tim_clk;
    union {
        struct {
            MotorHallConst  Hall_a, Hall_b, Hall_c;
        };
        MotorHallConst      hall_abc[3];
    };
    uint32_t                Hall_tim_val_ch;
    HAL_TIM_ActiveChannel   Hall_Active_ch;
} MotorConst;

typedef struct MotorSystemConst
{
    //
    const MotorConst const_h;
    //
    union {
        struct {
            volatile uint32_t
                *PWM_Addr_CCR_u,
                *PWM_Addr_CCR_v,
                *PWM_Addr_CCR_w;
        };
        volatile uint32_t       *PWM_Addr_CCR_uvw[3];
    };
    //
    float32_t           pwm_freq;
    // PWM 控制定時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / PWM_timer_clock
    float32_t           pwm_period;
    //
    float32_t           pwm_it_f;
    //
    volatile uint32_t   *Hall_Addr_IT;
    //
    volatile uint32_t   *Hall_Addr_CCR;
    // 霍爾計時器的實際計數頻率
    // = HALL_timer_clock / (PSC + 1)
    float32_t           hall_freq;
    // 霍爾計時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / HALL_timer_clock
    float32_t           hall_period;
    // 霍爾間隔 → 輸出軸轉速(omega) 轉換常數
    // OMEGA = [SPD_tim_f * 2 * pi / 60] / [6 × (POLE/2) × GEAR × htim_cnt]
    float32_t           omega_fbk;
    // PWM 週期 → 電角度內插轉換常數
    // Δθ_elec(rad) = [ (TIM_tim_t * ARR) / ELE_tim_t ] × (π/3) / htim_cnt
    float32_t           foc_it_angle_itpl;
} MotorSystemConst;

typedef enum MotorCtrlMode
{
    // 未初始化
    MOTOR_CTRL_UNINIT,
    // 初始化中
    MOTOR_CTRL_INIT,
    // 上臂輸出測試 ( PWM = 1.0 )
    MOTOR_CTRL_TEST_HIGH,
    // 下臂輸出測試
    MOTOR_CTRL_TEST_LOW,
    // 上臂輸出測試 ( PWM = 0.3 )
    MOTOR_CTRL_TEST_WAVE,
    // 120度輸出 I-F 強拖固定
    MOTOR_CTRL_120_I_F_SAME,
    // 120度輸出 I-F 強拖加速
    MOTOR_CTRL_120_I_F_ACCE,
    // 120度輸出 全閉環控制
    MOTOR_CTRL_120_NORMAL,
    // 120度輸出 DUTY開環控制
    MOTOR_CTRL_120_DUTY,
    // 120度輸出 旋轉方向轉換
    MOTOR_CTRL_120_DIREC_SW,
    // FOC輸出 轉子位置開環控制 ( 無ADC回授 )
    MOTOR_CTRL_FOC_SIM,
    // FOC輸出 I-F 強拖加速
    MOTOR_CTRL_FOC_I_F_ACCE,
    // FOC輸出 全閉環控制
    MOTOR_CTRL_FOC_NORMAL,
    // FOC輸出 轉子定位
    MOTOR_CTRL_FOC_POS,
    // FOC輸出 轉子位置開環控制 ( 外部指令增加角度 )
    MOTOR_CTRL_FOC_ROT_CMD,
    // FOC輸出 轉子位置開環控制 ( 內部自動加角度 )
    MOTOR_CTRL_FOC_ROT_AUTO,
    // FOC輸出 VD&VQ開環控制
    MOTOR_CTRL_FOC_OL_VDQ,
    // FOC輸出 IQ開環控制
    MOTOR_CTRL_FOC_OL_IQ,
} MotorCtrlMode;

// Control Parameter
typedef struct MotorCtrlParam
{
    MotorCtrlMode   ref_user;
    MotorCtrlMode   ref_sys;
    MotorCtrlMode   ref_sys_temp;
} MotorCtrlParam;

typedef enum MotorRotateMode
{
    MOTOR_ROTATE_UNINIT,
    MOTOR_ROTATE_COAST,
    MOTOR_ROTATE_BREAK,
    MOTOR_ROTATE_NORMAL,
    MOTOR_ROTATE_LOCK,
    MOTOR_ROTATE_LOCK_FIN,
} MotorRotateMode;

// Rotate Parameter
typedef struct MotorRotateParam
{
    MotorRotateMode ref_user;
    MotorRotateMode ref_sys;
} MotorRotateParam;

typedef struct MotorADCData
{
    AdcModelParam   adc;
    float32_t       pu;
} MotorADCData;

#define MOTOR_ADCS_CNT      6
#define MOTOR_ADCS_CNT_DIV2 (MOTOR_ADCS_CNT / 2)
typedef struct MotorADCParam
{
    union {
        struct {
            MotorADCData
                u_cur, v_cur, w_cur,
                u_vol, v_vol, w_vol;
        };
        MotorADCData uvw[MOTOR_ADCS_CNT];
    };
    // 應接近0
    float32_t   total_i;
    // 
    float32_t   avg_v;
} MotorADCParam;

typedef enum MotorSensorMode
{
    MOTOR_SENSOR_UNINIT,

    MOTOR_SENSOR_SIMULATE,
    // Hall EXTI
    MOTOR_SENSOR_HALL_EXTI,
    // PWM Period trigger
    MOTOR_SENSOR_HALL_PWM_T,

    MOTOR_SENSOR_LESS_VOLTAGE,
    
    MOTOR_SENSOR_LESS_CURRENT,
} MotorSensorMode;

typedef struct MotorPhaseTimeHistory
{
    uint32_t    datas[MOTOR_SPD_CNT];
    // 長度
    uint8_t     len;
    // 最舊id
    uint8_t     head;
    // 總和
    uint32_t    sum;
} MotorPhaseTimeHistory;

// Hall Parameter
#define MOTOR_HALL_OF 7
typedef struct MotorRotorParam
{
    // 位置取得模式
    volatile MotorSensorMode        mode;
    // 換相時間記錄
    volatile MotorPhaseTimeHistory  times_histoy;
    // 目前轉子位置
    volatile uint8_t        curr;
    // 上次轉子位置
    volatile uint8_t        prev;
    // 虛擬轉子位置 用於模擬旋轉
    volatile uint8_t        virtual;
    // 錯誤跳變次數
    uint8_t                 wrong;
    // 正確跳變累計
    uint8_t                 right;
    // I-F 強拖跳變週期
    uint32_t                I_F_trigger_T;
    // Hall timer AutoReload
    uint32_t                tim_ar;
    // 停轉時間
    uint32_t                stop_tick;
} MotorRotorParam;

// SPD Parameter
typedef struct MotorSpeedParame
{
    float32_t       ref_rpm;
    float32_t       ref_omega;
    float32_t       fbk_rpm;
    float32_t       fbk_omega;
    const float32_t save_stop_omega;
} MotorSpeedParame;

typedef union MotorPhaseDuty
{
    struct {
        float32_t
            u_h, v_h, w_h,
            u_l, v_l, w_l;
    };
    float32_t uvw[6];
} MotorPhaseDuty;

// DEG Parameter
typedef struct MotorDEGParam
{
    // 反轉
    bool                reverse;
    // 速度環
    PID_CTRL            pi_omega;
    // 電流環
    PID_CTRL            pi_current;
    // DEG duty值
    float32_t           load_duty_val;
    // 當前載入相位 id
    uint8_t             load_phase_id;
    // DEG uvw duty
    MotorPhaseDuty      final_duty;
} MotorDEGParam;

// FOC Parameter
typedef struct MotorFOCParam
{
    uint32_t            tim_tick_init;
    // clarke
    CLARKE              clarke_h;
    // 目前轉子位置
    float32_t           rotor_rad;
    // 轉子預計位置
    float32_t           rotor_exp_rad;
    // FOC 應補角度 (Angle Interpolation)
    volatile float32_t  rad_itpl;
    // FOC 角度累積插值 rad_acc += rad_itpl; 過一霍爾中斷後重置
    float32_t           rad_acc;
    // park
    PARK                park_h;

    PID_CTRL            pi_omega;
    // 
    PID_CTRL            pi_Id_h;
    // 
    PID_CTRL            pi_Iq_h;
    // 磁場位置
    float32_t           magn_rad;
    // ipark
    IPARK               ipark_h;
    // svgendq
    SVGENDQ             svgendq_h;
    // Vref_s = SQRT3 * Vref / Vbus
    float32_t           Vref_s;
    // FOC duty
    MotorPhaseDuty      final_duty;
} MotorFOCParam;

// DBG: debug
typedef struct MotorDbg
{
    float32_t   hall_rad[8];
    uint8_t     hall_wrong_c;
    uint8_t     hall_wrong[20];
    uint8_t     hall_s_wrong_c;
    uint8_t     hall_s_wrong[20];
} MotorDbg;

typedef struct MotorHistoryArray
{
    uint8_t             tick;
    volatile float32_t  id[10];
    volatile float32_t  iq[10];
} MotorHistoryArray;

#define ZCP_CONFIRM_TIMES       2       // 連續確認 2 次
typedef struct MotorVZcpParam
{
    uint8_t zcp_cnt;           // 連續確認計數
    uint8_t zcp_detected;      // 過零點偵測完成標記
    uint16_t comm_ticks;       // 當前相步距已走過的 PWM tick 數
    uint16_t delay_30deg_cnt;  // 30度換相延遲倒數
} MotorVZcpParam;

typedef struct MotorParameter
{
    // 系統常數
    MotorSystemConst        system;
    // PWM 中斷計數
    uint32_t                tim_tick;
    // PWM 中斷 初始化遞增計數
    uint32_t                tim_tick_init;
    // PWM 中斷 自上次換相已過計數
    uint32_t                tim_tick_ch_phase;
    // 馬達控制模式 (120度與foc以及細部)
    MotorCtrlParam          ctrl;
    // 馬達旋轉模式 (滑行與剎車等)
    MotorRotateParam        rotate;
    // ADC
    MotorADCParam          adc_h;
    // 轉子
     MotorRotorParam        rotor;
    // 從座往轉子 順時針為負
    MotorSpeedParame        speed;
    // 120度控制
    MotorDEGParam           deg_h;
    // FOC控制
    MotorFOCParam           foc_h;
    // PWM load duty
    MotorPhaseDuty          final_duty;
    // 除錯用
    MotorDbg                dbg_h;
    // 記錄用
    MotorHistoryArray       history;
    // Todo 須想放在哪
    MotorVZcpParam          zcp;
} MotorParameter;

#endif