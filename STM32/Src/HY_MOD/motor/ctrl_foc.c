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
    adc_current_reset(motor->adc_a);
    adc_current_reset(motor->adc_b);
    adc_current_reset(motor->adc_c);
}

inline void motor_vec_ctrl_adcs_upd(MotorParameter *motor)
{
    RESULT_CHECK_RET_VOID(adc_current_upd(motor->adc_a));
    RESULT_CHECK_RET_VOID(adc_current_upd(motor->adc_b));
    RESULT_CHECK_RET_VOID(adc_current_upd(motor->adc_c));
}

inline void motor_vec_ctrl_hall_angle_trf(MotorParameter *motor)
{
    motor->exti_hall_rad = hall_elec_angle[motor->hall_current];
}

inline Result motor_vec_ctrl_hall_angle_chk(MotorParameter *motor)
{
    if (motor->exti_hall_rad == F32_MAX) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    return RESULT_OK(motor);
}

float32_t current_zero;
inline void motor_vec_ctrl_clarke(MotorParameter *motor)
{
    // 電流進motor為 正
    current_zero = (motor->adc_a->current + motor->adc_b->current + motor->adc_c->current) / 3.0f;
    motor->clarke.As = (motor->adc_a->current - current_zero);
    motor->clarke.Bs = (motor->adc_b->current - current_zero);
    motor->clarke.Cs = (motor->adc_c->current - current_zero);
    CLARKE_run_ideal(&motor->clarke);
    return;
}

inline void motor_vec_ctrl_park(MotorParameter *motor)
{
    motor->park.Alpha = motor->clarke.Alpha;
    motor->park.Beta = motor->clarke.Beta;
    motor->foc_angle_acc = motor->foc_angle_acc + motor->foc_angle_itpl;
    VAR_CLAMPF(motor->foc_angle_acc, -PI_DIV_3, PI_DIV_3);
    // 電壓向量應提前90度 +PI_DIV_2
    RESULT_CHECK_HANDLE(trigo_sin_cosf(
        motor->exti_hall_rad + motor->foc_angle_acc + MOTOR_42BLF01_ANGLE + PI_DIV_2,
        &motor->park.Sin, &motor->park.Cos
    ));
    PARK_run(&motor->park);
}

inline void motor_vec_ctrl_pi_id_iq(MotorParameter *motor)
{
    if(motor->rpm_feedback.value == 0.0f)
    {
        motor->pi_Iq.out = (!motor->rpm_reference.reverse) ?
            motor->const_h.peak_current : -motor->const_h.peak_current;
        return;
    }
        motor->pi_Id.reference = 0.0f,
        motor->pi_Id.feedback = motor->park.Ds;
        PI_run(&motor->pi_Id);

        motor->pi_Iq.feedback = motor->park.Qs;
        PI_run(&motor->pi_Iq);

        VAR_CLAMPF(motor->pi_Iq.Term_p, -0.1f, 0.1f);
        motor->pi_Iq.out = motor->pi_Iq.reference + motor->pi_Iq.Term_p;
        VAR_CLAMPF(motor->pi_Iq.out, -0.75f, 0.75f);
}

inline void motor_vec_ctrl_ipark(MotorParameter *motor)
{
    motor->ipark.Vdref += motor->pi_Id.out;
    VAR_CLAMPF(motor->ipark.Vdref, -0.06f, 0.06f);
    motor->ipark.Vqref = motor->pi_Iq.out;
    motor->ipark.Sin = motor->park.Sin;
    motor->ipark.Cos = motor->park.Cos;
    IPARK_run(&motor->ipark);
    RESULT_CHECK_HANDLE(trigo_atan(motor->ipark.Alpha, motor->ipark.Beta, &motor->elec_theta_rad));
    // motor->elec_theta_rad = var_wrap_pos(motor->elec_theta_rad, PI_MUL_2);
    motor->elec_theta_rad = var_wrap_pos(motor->elec_theta_rad, PI_DIV_3);
}

float32_t sec_chk[30] = {0};
uint8_t chk_cnt = 0;
uint8_t sec_mem = 0;
inline void motor_vec_ctrl_svgen(MotorParameter *motor)
{
    motor->svgendq.Ualpha = motor->ipark.Alpha;
    motor->svgendq.Ubeta  = motor->ipark.Beta ;
    SVGEN_run(&motor->svgendq);

    if (motor->svgendq.Sector != sec_mem)
    {
        sec_chk[chk_cnt++] = motor->hall_current;
        // sec_chk[chk_cnt++] = motor->elec_theta_rad;
        sec_chk[chk_cnt++] = motor->svgendq.Sector;
        if (chk_cnt >= 30) chk_cnt = 0;
    }
    sec_mem = motor->svgendq.Sector;
}

#define SQUARE(x) (x*x)
inline void motor_vec_ctrl_svpwm(MotorParameter *motor)
{
    if (
        arm_sqrt_f32(
            SQUARE(motor->svgendq.Ualpha) + SQUARE(motor->svgendq.Ubeta),
            &motor->v_ref
        ) != ARM_MATH_SUCCESS
    ) while (1) {};
    // float32_t theta = var_wrap_pos(motor->elec_theta_rad, PI_DIV_3);
    // T1: 第一個有源向量導通時間 在該sector內靠近前一個主向量的時間比例(由sin(π/3−θ)決定)
    // T2: 第二個有源向量導通時間 在該sector內靠近下一個主向量的時間比例(由sin(θ)決定)
    float32_t T1, T2;
    RESULT_CHECK_HANDLE(trigo_sin_cosf(PI_DIV_3 - motor->elec_theta_rad, &T1, NULL));
    RESULT_CHECK_HANDLE(trigo_sin_cosf(motor->elec_theta_rad, &T2, NULL));
    T1 *= motor->v_ref;
    T2 *= motor->v_ref;
    // T0div2: 零向量時間的一半 將整個零向量時間平均分配到PWM週期的前後兩端 讓波形中心對稱
    float32_t T0div2 = (1.0f - (T1 + T2)) * 0.5f;
    switch (motor->svgendq.Sector)
    {
        case 6:
        {
            motor->duty_foc.u = T0div2 + T1 + T2;
            motor->duty_foc.v = T0div2 + T2;
            motor->duty_foc.w = T0div2;
            break;
        }
        case 2:
        {
            motor->duty_foc.u = T0div2 + T1;
            motor->duty_foc.v = T0div2 + T1 + T2;
            motor->duty_foc.w = T0div2;
            break;
        }
        case 3:
        {
            motor->duty_foc.u = T0div2;
            motor->duty_foc.v = T0div2 + T1 + T2;
            motor->duty_foc.w = T0div2 + T2;
            break;
        }
        case 1:
        {
            motor->duty_foc.u = T0div2;
            motor->duty_foc.v = T0div2 + T1;
            motor->duty_foc.w = T0div2 + T1 + T2;
            break;
        }
        case 5:
        {
            motor->duty_foc.u = T0div2 + T2;
            motor->duty_foc.v = T0div2;
            motor->duty_foc.w = T0div2 + T1 + T2;
            break;
        }
        case 4:
        {
            motor->duty_foc.u = T0div2 + T1 + T2;
            motor->duty_foc.v = T0div2;
            motor->duty_foc.w = T0div2 + T1;
            break;
        }
    }
}

#endif