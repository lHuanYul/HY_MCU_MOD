#include "HY_MOD/motor/ctrl_foc.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/motor/main.h"
#include "HY_MOD/motor/trigonometric.h"
#include "tim.h"
#include "dac.h"

inline void motor_foc_pi_setup(MotorParameter *motor)
{
    const float32_t bw =
        motor->calculate_h.pwm_it_f / MOTOR_I_BW_INDEX * PI_MUL_2;
    const float32_t Vbase = MOTOR_VBUS / SQRT3;
    motor->foc_h.pi_Id_h.Kp = MOTOR_LL * bw / Vbase;
    motor->foc_h.pi_Id_h.Ki = 1.0f / MOTOR_TAU * motor->calculate_h.pwm_T;
    motor->foc_h.pi_Id_h.max =  MOTOR_MAX_MODULATION_INDEX;
    motor->foc_h.pi_Id_h.min = -MOTOR_MAX_MODULATION_INDEX;
    motor->foc_h.pi_Iq_h.Kp = MOTOR_LL * bw / Vbase;
    motor->foc_h.pi_Iq_h.Ki = 1.0f / MOTOR_TAU * motor->calculate_h.pwm_T;
}

inline void motor_foc_reset(MotorParameter *motor)
{
    motor->foc_h.rad_itpl = 0.0f;
    motor->foc_h.rad_acc = 0.0f;
    motor->foc_h.pi_Iq_h.out_fix = 0.0f;
    motor->foc_h.pi_Id_h.out_fix = 0.0f;
}

inline void motor_foc_hall_exti_cb(MotorParameter *motor)
{
    motor->foc_h.rad_acc = 0.0f;
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
    uint8_t hall = motor->hall_h.current;
    if (hall == UINT8_MAX)
    {
        if (motor->ctrl_h.ref_fix == MOTOR_CTRL_FOC_SIM) hall = 4;
        else return RESULT_ERROR(RES_ERR_NOT_FOUND);
    }
    motor->foc_h.hall_rad = hall_elec_angle[hall];
    return RESULT_OK(motor);
}

static inline void motor_vec_ctrl_clarke(MotorParameter *motor)
{
    uint8_t i;
    motor->adc_h.total = 0.0f;
    for (i = 0; i < 3; i++)
    {
        motor->adc_h.total += motor->adc_h.adc_uvw[i]->current;
    }
    float32_t avg = motor->adc_h.total / 3.0f;
    for (i = 0; i < 3; i++)
    {
        // To Per-Unit
        motor->adc_h.uvw[i] =
            (motor->adc_h.adc_uvw[i]->current - avg) / motor->const_h.rated_current;
        motor->foc_h.clarke_h.ABC[i] = motor->adc_h.uvw[i];
    }
    CLARKE_run_ideal(&motor->foc_h.clarke_h);
}

static inline void motor_vec_ctrl_park(MotorParameter *motor)
{
    motor->foc_h.park_h.Alpha = motor->foc_h.clarke_h.Alpha;
    motor->foc_h.park_h.Beta = motor->foc_h.clarke_h.Beta;
    motor->foc_h.rad_acc += motor->foc_h.rad_itpl;
    VAR_CLAMPF(motor->foc_h.rad_acc, -PI_DIV_3, PI_DIV_3);
    switch (motor->ctrl_h.ref_fix)
    {
        case MOTOR_CTRL_FOC_POS:
        {
            motor->foc_h.rotor_rad = 0.0f;
            break;
        }
        case MOTOR_CTRL_FOC_ROT_CMD:
            break;
        case MOTOR_CTRL_FOC_ROT_AUTO:
        case MOTOR_CTRL_FOC_OL_VDQ:
        {
            motor->dbg_h.foc_rad[motor->dbg_h.foc_rad_id][0] = motor->foc_h.rotor_rad;
            motor->dbg_h.foc_rad[motor->dbg_h.foc_rad_id][1] = (float32_t)motor->hall_h.current;
            if (++motor->dbg_h.foc_rad_id >= 100) motor->dbg_h.foc_rad_id = 0;

            motor->foc_h.rotor_rad += 0.001f * PI;
            motor->foc_h.rotor_rad = var_wrap_pos(motor->foc_h.rotor_rad, PI_MUL_2);
            break;
        }
        default:
        {
            motor->foc_h.rotor_rad = var_wrap_pos(
                motor->foc_h.hall_rad + motor->foc_h.rad_acc + MOTOR_FOC_ANGLE,
                PI_MUL_2
            );
            break;
        }
    }
    RESULT_CHECK_HANDLE(trigo_sin_cosf(
        motor->foc_h.rotor_rad,
        &motor->foc_h.park_h.Sin, &motor->foc_h.park_h.Cos));
    PARK_run(&motor->foc_h.park_h);
}

#define SQUARE(x) (x*x)
static inline void motor_vec_ctrl_pi_id_iq(MotorParameter *motor)
{
    motor->foc_h.pi_Id_h.reference = 0.0f;
    switch (motor->ctrl_h.ref_fix)
    {
        case MOTOR_CTRL_FOC_OL_IQ:
        {
            // 0.4f
            motor->foc_h.pi_Iq_h.reference = motor->speed_h.ref_rpm;
            VAR_CLAMPF(motor->foc_h.pi_Iq_h.reference, 0.0f, 1.0f);
            break;
        }
        default:
        {
            motor->foc_h.pi_Iq_h.reference = motor->foc_h.pi_omega.out_fix;
            break;
        }
    }
    motor->foc_h.pi_Id_h.feedback  = motor->foc_h.park_h.Ds;
    motor->foc_h.pi_Iq_h.feedback  = motor->foc_h.park_h.Qs;
    PI_run(&motor->foc_h.pi_Id_h);
    float32_t Iq_lim = 0.0f;
    float32_t diff = SQUARE(motor->foc_h.pi_Id_h.max) - SQUARE(motor->foc_h.pi_Id_h.out_fix);
    if (diff > 0.0f) arm_sqrt_f32(diff, &Iq_lim);
    motor->foc_h.pi_Iq_h.max =  Iq_lim;
    motor->foc_h.pi_Iq_h.min = -Iq_lim;
    PI_run(&motor->foc_h.pi_Iq_h);
}

static inline void motor_vec_ctrl_ipark(MotorParameter *motor)
{
    switch (motor->ctrl_h.ref_fix)
    {
        case MOTOR_CTRL_FOC_POS:
        case MOTOR_CTRL_FOC_ROT_CMD:
        case MOTOR_CTRL_FOC_ROT_AUTO:
        {
            // 0.2f
            motor->foc_h.ipark_h.Vdref = motor->speed_h.ref_rpm;
            motor->foc_h.ipark_h.Vqref = 0.0f;
            break;
        }
        case MOTOR_CTRL_FOC_OL_VDQ:
        {
            motor->foc_h.ipark_h.Vdref = 0.0f;
            // 0.15f
            motor->foc_h.ipark_h.Vqref = motor->speed_h.ref_rpm;
            VAR_CLAMPF(motor->foc_h.ipark_h.Vqref, 0.0f, 1.0f);
            break;
        }
        default:
        {
            motor->foc_h.ipark_h.Vdref = motor->foc_h.pi_Id_h.out_fix;
            motor->foc_h.ipark_h.Vqref = motor->foc_h.pi_Iq_h.out_fix;
            break;
        }
    }
    motor->foc_h.ipark_h.Sin = motor->foc_h.park_h.Sin;
    motor->foc_h.ipark_h.Cos = motor->foc_h.park_h.Cos;
    IPARK_run(&motor->foc_h.ipark_h);
    Result res = trigo_atan(
        motor->foc_h.ipark_h.Alpha, motor->foc_h.ipark_h.Beta, &motor->foc_h.magn_rad);
    if (RESULT_CHECK_RAW(res))
    {
        if (motor->ctrl_h.ref_fix == MOTOR_CTRL_FOC_SIM) motor->foc_h.magn_rad = 0.0f;
        else Error_Handler();
    }
}

static inline void motor_vec_ctrl_svgen(MotorParameter *motor)
{
    motor->foc_h.svgendq_h.Ualpha = motor->foc_h.ipark_h.Alpha;
    motor->foc_h.svgendq_h.Ubeta  = motor->foc_h.ipark_h.Beta ;
    SVGEN_run(&motor->foc_h.svgendq_h);
}

#define MAX_MODULATION_INDEX  0.95f
static inline void motor_vec_ctrl_svpwm(MotorParameter *motor)
{
    if (
        arm_sqrt_f32(
            SQUARE(motor->foc_h.svgendq_h.Ualpha) + SQUARE(motor->foc_h.svgendq_h.Ubeta),
            &motor->foc_h.Vref_s) != ARM_MATH_SUCCESS
    ) {
        if (motor->ctrl_h.ref_fix == MOTOR_CTRL_FOC_SIM) motor->foc_h.Vref_s = 0.0f;
        else Error_Handler();
    }
    float32_t theta = var_wrap_pos(motor->foc_h.magn_rad, PI_DIV_3);
    // T1: 第一個有源向量導通時間 在該sector內靠近前一個主向量的時間比例(由sin(π/3−θ)決定)
    // T2: 第二個有源向量導通時間 在該sector內靠近下一個主向量的時間比例(由sin(θ)決定)
    float32_t T1, T2;
    RESULT_CHECK_HANDLE(trigo_sin_cosf(PI_DIV_3 - theta, &T1, NULL));
    RESULT_CHECK_HANDLE(trigo_sin_cosf(theta, &T2, NULL));
    T1 *= motor->foc_h.Vref_s;
    T2 *= motor->foc_h.Vref_s;
    // 過調變保護 (Overmodulation Protection)
    float32_t sum_T1_T2 = T1 + T2;
    if (sum_T1_T2 > MAX_MODULATION_INDEX)
    {
        // 等比例縮放 T1 與 T2，維持電壓向量的方向，但限制大小
        T1 = T1 / sum_T1_T2;
        T2 = T2 / sum_T1_T2;
        sum_T1_T2 = MAX_MODULATION_INDEX; 
    }
    // T0div2: 零向量時間的一半 將整個零向量時間平均分配到PWM週期的前後兩端 讓波形中心對稱
    float32_t T0div2 = (1.0f - sum_T1_T2) * 0.5f;
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
#define RECORD_INTERVAL 20
void motor_foc_run(MotorParameter *motor)
{
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

    if (motor->tim_tick % RECORD_INTERVAL == 0)
    {
        // 計算降採樣後對應的 0~9 Buffer Index
        // 利用除法消除掉沒記錄的 tick，再取餘數限制在 0~9
        uint8_t buf_idx = (motor->tim_tick / RECORD_INTERVAL) % 10;

        motor->history.id[buf_idx] = motor->foc_h.pi_Id_h.feedback;
        motor->history.iq[buf_idx] = motor->foc_h.pi_Iq_h.feedback;

        // 前半段 (0~4) 填滿時觸發 en1
        if (buf_idx == 4)
        {
            fdcan_h.motor_idq_en1 = 1;
            fdcan_h.motor_idq_en2 = 0;
        }
        // 後半段 (5~9) 填滿時觸發 en2
        else if (buf_idx == 9)
        {
            fdcan_h.motor_idq_en1 = 0;
            fdcan_h.motor_idq_en2 = 1;
        }
    }
    float32_t scale = 4095.0f / (2.0f * ONE_DIV_SQRT3);
    float32_t dac_id = motor->foc_h.pi_Id_h.feedback * scale + 2048.0f;
    float32_t dac_iq = motor->foc_h.pi_Iq_h.feedback * scale + 2048.0f;
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

    motor_timer_load(motor);
}

#endif