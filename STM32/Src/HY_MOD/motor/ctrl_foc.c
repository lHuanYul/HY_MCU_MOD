#include "HY_MOD/motor/ctrl_foc.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/motor/main.h"
#include "HY_MOD/motor/trigonometric.h"
#include "HY_MOD/adc_current/main.h"
#include "tim.h"
#include "dac.h"

void motor_foc_set(MotorParameter *motor)
{
    motor->foc_h.angle_itpl = 0.0f;
    motor->foc_h.angle_acc = 0.0f;
    motor->foc_h.pi_Iq_h.out_fix = 0.0f;
    motor->foc_h.pi_Id_h.out_fix = 0.0f;
}

inline void motor_vec_ctrl_adcs_reset(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
        adc_current_reset(motor->foc_h.adc_h.uvw[i]);
}

inline void motor_vec_ctrl_adcs_upd(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
        RESULT_CHECK_RET_VOID(adc_current_upd(motor->foc_h.adc_h.uvw[i]));
}

static const float32_t hall_elec_angle[8] = {
    F32_MAX,
    4.0f * PI_DIV_3,
    2.0f * PI_DIV_3,
    3.0f * PI_DIV_3,
    0.0f,
    5.0f * PI_DIV_3,
    1.0f * PI_DIV_3,
    F32_MAX,
};
static inline Result motor_vec_ctrl_angle_upd(MotorParameter *motor)
{
    if (motor->hall_h.current == UINT8_MAX) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    motor->foc_h.hall_rad = hall_elec_angle[motor->hall_h.current];
    return RESULT_OK(motor);
}

static inline void motor_vec_ctrl_clarke(MotorParameter *motor)
{
    // 電流進motor為 正
    uint8_t i;
    for (i = 0; i < 3; i++)
        motor->foc_h.clarke_h.ABC[i] = motor->foc_h.adc_h.uvw[i]->current;
    
    CLARKE_run_ideal(&motor->foc_h.clarke_h);
}

static inline void motor_vec_ctrl_park(MotorParameter *motor)
{
    motor->foc_h.park_h.Alpha = motor->foc_h.clarke_h.Alpha;
    motor->foc_h.park_h.Beta = motor->foc_h.clarke_h.Beta;
    motor->foc_h.angle_acc += motor->foc_h.angle_itpl;
    VAR_CLAMPF(motor->foc_h.angle_acc, -PI_DIV_3, PI_DIV_3);
    // Todo 轉子定位測試
    // motor->foc_h.rotor_rad = 0.0f;
    // Todo 強制旋轉測試
    // motor->foc_h.rotor_rad += 0.002f;
    // motor->foc_h.rotor_rad = var_wrap_pos(motor->foc_h.rotor_rad, PI_MUL_2);
    motor->foc_h.rotor_rad = motor->foc_h.hall_rad + motor->foc_h.angle_acc + MOTOR_FOC_ANGLE;
    RESULT_CHECK_HANDLE(trigo_sin_cosf(
        motor->foc_h.rotor_rad,
        &motor->foc_h.park_h.Sin, &motor->foc_h.park_h.Cos));
    PARK_run(&motor->foc_h.park_h);
}

#define SQUARE(x) (x*x)
static inline void motor_vec_ctrl_pi_id_iq(MotorParameter *motor)
{
    motor->foc_h.pi_Id_h.reference = 0.0f;
    // motor->foc_h.pi_Iq_h.reference = 0.4;
    motor->foc_h.pi_Iq_h.reference = motor->foc_h.pi_rpm.out_fix;
    motor->foc_h.pi_Id_h.feedback  = motor->foc_h.park_h.Ds;
    motor->foc_h.pi_Iq_h.feedback  = motor->foc_h.park_h.Qs;
    PI_run(&motor->foc_h.pi_Id_h);
    float32_t Iq_lim;
    arm_sqrt_f32(SQUARE(motor->foc_h.pi_Id_h.max) - SQUARE(motor->foc_h.pi_Id_h.out_fix), &Iq_lim);
    motor->foc_h.pi_Iq_h.max =  Iq_lim;
    motor->foc_h.pi_Iq_h.min = -Iq_lim;
    PI_run(&motor->foc_h.pi_Iq_h);
}

static inline void motor_vec_ctrl_ipark(MotorParameter *motor)
{
    // Todo 轉子定位測試
    // motor->foc_h.ipark_h.Vdref = 0.1f; // 歸一化電壓，給 10% 即可
    // motor->foc_h.ipark_h.Vqref = 0.0f;
    // Todo 強制旋轉測試
    // motor->foc_h.ipark_h.Vdref = 0.0f;
    // motor->foc_h.ipark_h.Vqref = 0.15f;
    motor->foc_h.ipark_h.Vdref = motor->foc_h.pi_Id_h.out_fix;
    motor->foc_h.ipark_h.Vqref = motor->foc_h.pi_Iq_h.out_fix;
    motor->foc_h.ipark_h.Sin = motor->foc_h.park_h.Sin;
    motor->foc_h.ipark_h.Cos = motor->foc_h.park_h.Cos;
    IPARK_run(&motor->foc_h.ipark_h);
    RESULT_CHECK_HANDLE(trigo_atan(
        motor->foc_h.ipark_h.Alpha, motor->foc_h.ipark_h.Beta, &motor->foc_h.magn_rad));
}

static inline void motor_vec_ctrl_svgen(MotorParameter *motor)
{
    motor->foc_h.svgendq_h.Ualpha = motor->foc_h.ipark_h.Alpha;
    motor->foc_h.svgendq_h.Ubeta  = motor->foc_h.ipark_h.Beta ;
    SVGEN_run(&motor->foc_h.svgendq_h);
}

static inline void motor_vec_ctrl_svpwm(MotorParameter *motor)
{
    if (
        arm_sqrt_f32(
            SQUARE(motor->foc_h.svgendq_h.Ualpha) + SQUARE(motor->foc_h.svgendq_h.Ubeta),
            &motor->foc_h.Vref) != ARM_MATH_SUCCESS
    ) while (1) {};
    float32_t theta = var_wrap_pos(motor->foc_h.magn_rad, PI_DIV_3);
    // T1: 第一個有源向量導通時間 在該sector內靠近前一個主向量的時間比例(由sin(π/3−θ)決定)
    // T2: 第二個有源向量導通時間 在該sector內靠近下一個主向量的時間比例(由sin(θ)決定)
    float32_t T1, T2;
    RESULT_CHECK_HANDLE(trigo_sin_cosf(PI_DIV_3 - theta, &T1, NULL));
    RESULT_CHECK_HANDLE(trigo_sin_cosf(theta, &T2, NULL));
    T1 *= motor->foc_h.Vref;
    T2 *= motor->foc_h.Vref;
    // T0div2: 零向量時間的一半 將整個零向量時間平均分配到PWM週期的前後兩端 讓波形中心對稱
    float32_t T0div2 = (1.0f - (T1 + T2)) * 0.5f;
    switch (motor->foc_h.svgendq_h.Sector)
    {
        case 6:
        {
            motor->foc_h.duty_h.u = T0div2 + T1 + T2;
            motor->foc_h.duty_h.v = T0div2 + T2;
            motor->foc_h.duty_h.w = T0div2;
            break;
        }
        case 2:
        {
            motor->foc_h.duty_h.u = T0div2 + T1;
            motor->foc_h.duty_h.v = T0div2 + T1 + T2;
            motor->foc_h.duty_h.w = T0div2;
            break;
        }
        case 3:
        {
            motor->foc_h.duty_h.u = T0div2;
            motor->foc_h.duty_h.v = T0div2 + T1 + T2;
            motor->foc_h.duty_h.w = T0div2 + T2;
            break;
        }
        case 1:
        {
            motor->foc_h.duty_h.u = T0div2;
            motor->foc_h.duty_h.v = T0div2 + T1;
            motor->foc_h.duty_h.w = T0div2 + T1 + T2;
            break;
        }
        case 5:
        {
            motor->foc_h.duty_h.u = T0div2 + T2;
            motor->foc_h.duty_h.v = T0div2;
            motor->foc_h.duty_h.w = T0div2 + T1 + T2;
            break;
        }
        case 4:
        {
            motor->foc_h.duty_h.u = T0div2 + T1 + T2;
            motor->foc_h.duty_h.v = T0div2;
            motor->foc_h.duty_h.w = T0div2 + T1;
            break;
        }
    }
}

#include "main/main.h"
void motor_foc_run(MotorParameter *motor)
{
    if (motor->tim_tick % 10 == 4)
    {
        fdcan_h.motor_idq_en1 = 1;
        fdcan_h.motor_idq_en2 = 0;
    }
    else if (motor->tim_tick % 10 == 9)
    {
        fdcan_h.motor_idq_en1 = 0;
        fdcan_h.motor_idq_en2 = 1;
    }
    RESULT_CHECK_RET_VOID(motor_vec_ctrl_angle_upd(motor));
    if (motor->foc_h.init_cnt > 0)
    {
        motor->foc_h.init_cnt--;
        if (motor->foc_h.init_cnt == 0)
        // Todo FOC初始角度測試 先用簡單的120度控制等效於60度換相 讓馬達轉起來再說
            motor_switch_ctrl_fix(motor, motor->ctrl_h.ref_ori);
        return;
    }
    motor_vec_ctrl_clarke(motor);
    motor_vec_ctrl_park(motor);
    motor_vec_ctrl_pi_id_iq(motor);

    motor->history.id[motor->tim_tick % 10] = motor->foc_h.pi_Id_h.out_fix;
    motor->history.iq[motor->tim_tick % 10] = motor->foc_h.pi_Iq_h.out_fix;

    float32_t scale = 4095.0f / (2.0f * ONE_DIV_SQRT3);
    float32_t dac_id = motor->foc_h.pi_Id_h.out_fix * scale + 2048.0f;
    float32_t dac_iq = motor->foc_h.pi_Iq_h.out_fix * scale + 2048.0f;
    VAR_CLAMPF(dac_id, 0.0f, 4095.0f);
    VAR_CLAMPF(dac_iq, 0.0f, 4095.0f);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)dac_id); // PA4 輸出 Id
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)dac_iq); // PA5

    motor_vec_ctrl_ipark(motor);
    motor_vec_ctrl_svgen(motor);
    motor_vec_ctrl_svpwm(motor);
}

void motor_foc_load(MotorParameter *motor)
{
    motor->duty_load = motor->foc_h.duty_h;
    // motor->duty_load.u = 0.3f;
    // motor->duty_load.v = 0.3f;
    // motor->duty_load.w = 0.3f;
    motor_timer_load(motor);
}

#endif