#include "HY_MOD/motor/ctrl_foc.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/motor/trigonometric.h"
#include "HY_MOD/adc_current/main.h"
#include "tim.h"

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

inline void motor_vec_ctrl_adcs_reset(MotorParameter *motor)
{
    adc_current_reset(motor->foc_h.adc_h.u);
    adc_current_reset(motor->foc_h.adc_h.v);
    adc_current_reset(motor->foc_h.adc_h.w);
}

inline void motor_vec_ctrl_adcs_upd(MotorParameter *motor)
{
    RESULT_CHECK_RET_VOID(adc_current_upd(motor->foc_h.adc_h.u));
    RESULT_CHECK_RET_VOID(adc_current_upd(motor->foc_h.adc_h.v));
    RESULT_CHECK_RET_VOID(adc_current_upd(motor->foc_h.adc_h.w));
}

inline Result motor_vec_ctrl_angle_upd(MotorParameter *motor)
{
    if (motor->hall_h.current == UINT8_MAX) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    motor->foc_h.hall_rad = hall_elec_angle[motor->hall_h.current];
    return RESULT_OK(motor);
}

float32_t current_zero;
inline void motor_vec_ctrl_clarke(MotorParameter *motor)
{
    // 電流進motor為 正
    current_zero = (motor->foc_h.adc_h.u->current + motor->foc_h.adc_h.v->current + motor->foc_h.adc_h.w->current) / 3.0f;
    motor->foc_h.clarke_h.As = (motor->foc_h.adc_h.u->current - current_zero);
    motor->foc_h.clarke_h.Bs = (motor->foc_h.adc_h.v->current - current_zero);
    motor->foc_h.clarke_h.Cs = (motor->foc_h.adc_h.w->current - current_zero);
    CLARKE_run_ideal(&motor->foc_h.clarke_h);
    return;
}

inline void motor_vec_ctrl_park(MotorParameter *motor)
{
    motor->foc_h.park_h.Alpha = motor->foc_h.clarke_h.Alpha;
    motor->foc_h.park_h.Beta = motor->foc_h.clarke_h.Beta;
    motor->foc_h.angle_acc = motor->foc_h.angle_acc + motor->foc_h.angle_itpl;
    VAR_CLAMPF(motor->foc_h.angle_acc, -PI_DIV_3, PI_DIV_3);
    // 電壓向量應提前90度 +PI_DIV_2
    RESULT_CHECK_HANDLE(trigo_sin_cosf(
        motor->foc_h.hall_rad + motor->foc_h.angle_acc + MOTOR_42BLF01_ANGLE + PI_DIV_2,
        &motor->foc_h.park_h.Sin, &motor->foc_h.park_h.Cos
    ));
    PARK_run(&motor->foc_h.park_h);
}

inline void motor_vec_ctrl_pi_id_iq(MotorParameter *motor)
{
    if(motor->rpm_h.feedback.value == 0.0f)
    {
        motor->foc_h.pi_Iq_h.out = (!motor->rpm_h.reference.reverse) ?
            motor->const_h.peak_current : -motor->const_h.peak_current;
        return;
    }
        motor->foc_h.pi_Id_h.reference = 0.0f,
        motor->foc_h.pi_Id_h.feedback = motor->foc_h.park_h.Ds;
        PI_run(&motor->foc_h.pi_Id_h);

        motor->foc_h.pi_Iq_h.feedback = motor->foc_h.park_h.Qs;
        PI_run(&motor->foc_h.pi_Iq_h);

        VAR_CLAMPF(motor->foc_h.pi_Iq_h.Term_p, -0.1f, 0.1f);
        motor->foc_h.pi_Iq_h.out = motor->foc_h.pi_Iq_h.reference + motor->foc_h.pi_Iq_h.Term_p;
        VAR_CLAMPF(motor->foc_h.pi_Iq_h.out, -0.75f, 0.75f);
}

inline void motor_vec_ctrl_ipark(MotorParameter *motor)
{
    motor->foc_h.ipark_h.Vdref += motor->foc_h.pi_Id_h.out;
    VAR_CLAMPF(motor->foc_h.ipark_h.Vdref, -0.06f, 0.06f);
    motor->foc_h.ipark_h.Vqref = motor->foc_h.pi_Iq_h.out;
    motor->foc_h.ipark_h.Sin = motor->foc_h.park_h.Sin;
    motor->foc_h.ipark_h.Cos = motor->foc_h.park_h.Cos;
    IPARK_run(&motor->foc_h.ipark_h);
    RESULT_CHECK_HANDLE(trigo_atan(motor->foc_h.ipark_h.Alpha, motor->foc_h.ipark_h.Beta, &motor->foc_h.elec_theta_rad));
    // motor->foc_h.elec_theta_rad = var_wrap_pos(motor->foc_h.elec_theta_rad, PI_MUL_2);
    motor->foc_h.elec_theta_rad = var_wrap_pos(motor->foc_h.elec_theta_rad, PI_DIV_3);
}

float32_t sec_chk[30] = {0};
uint8_t chk_cnt = 0;
uint8_t sec_mem = 0;
inline void motor_vec_ctrl_svgen(MotorParameter *motor)
{
    motor->foc_h.svgendq_h.Ualpha = motor->foc_h.ipark_h.Alpha;
    motor->foc_h.svgendq_h.Ubeta  = motor->foc_h.ipark_h.Beta ;
    SVGEN_run(&motor->foc_h.svgendq_h);

    if (motor->foc_h.svgendq_h.Sector != sec_mem)
    {
        sec_chk[chk_cnt++] = motor->hall_h.current;
        // sec_chk[chk_cnt++] = motor->foc_h.elec_theta_rad;
        sec_chk[chk_cnt++] = motor->foc_h.svgendq_h.Sector;
        if (chk_cnt >= 30) chk_cnt = 0;
    }
    sec_mem = motor->foc_h.svgendq_h.Sector;
}

#define SQUARE(x) (x*x)
inline void motor_vec_ctrl_svpwm(MotorParameter *motor)
{
    if (
        arm_sqrt_f32(
            SQUARE(motor->foc_h.svgendq_h.Ualpha) + SQUARE(motor->foc_h.svgendq_h.Ubeta),
            &motor->foc_h.Vref
        ) != ARM_MATH_SUCCESS
    ) while (1) {};
    // float32_t theta = var_wrap_pos(motor->foc_h.elec_theta_rad, PI_DIV_3);
    // T1: 第一個有源向量導通時間 在該sector內靠近前一個主向量的時間比例(由sin(π/3−θ)決定)
    // T2: 第二個有源向量導通時間 在該sector內靠近下一個主向量的時間比例(由sin(θ)決定)
    float32_t T1, T2;
    RESULT_CHECK_HANDLE(trigo_sin_cosf(PI_DIV_3 - motor->foc_h.elec_theta_rad, &T1, NULL));
    RESULT_CHECK_HANDLE(trigo_sin_cosf(motor->foc_h.elec_theta_rad, &T2, NULL));
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

#endif