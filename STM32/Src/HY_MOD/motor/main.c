#include "HY_MOD/motor/main.h"
#ifdef HY_MOD_STM32_MOTOR

#include "tim.h"
#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/motor/rotor.h"
#include "HY_MOD/motor/ctrl.h"
#include "HY_MOD/adc/main.h"
#include "HY_MOD/timer/dataS.h"

static void motor_init_system(MotorParameter *motor)
{
    uint8_t i;
    const MotorConst *const_h = &motor->system.const_h;

    TIM_TypeDef *PWM_Inst = const_h->PWM_htimx->Instance;
    for (i = 0; i < 3; i++)
    {
        motor->system.PWM_Addr_CCR_uvw[i] = TIMER_GET_CCR_ADDR(PWM_Inst, const_h->PWM_uvw[i].pwm_ch);
    }
    motor->system.pwm_freq =
        (float32_t)*const_h->PWM_tim_clk /
        (float32_t)(const_h->PWM_htimx->Init.Prescaler + 1U);
    motor->system.pwm_period =
        (float32_t)(const_h->PWM_htimx->Init.Prescaler + 1U) /
        (float32_t)*const_h->PWM_tim_clk;

    TIM_TypeDef *Hall_Inst = const_h->Hall_htimx->Instance;
    motor->system.Hall_Addr_IT = &(Hall_Inst->EGR);
    motor->system.Hall_Addr_CCR = TIMER_GET_CCR_ADDR(Hall_Inst, const_h->Hall_tim_val_ch);
    motor->system.hall_freq =
        (float32_t)*const_h->Hall_tim_clk /
        (float32_t)(const_h->Hall_htimx->Init.Prescaler + 1U);
    motor->system.hall_period =
        (float32_t)(const_h->Hall_htimx->Init.Prescaler + 1U) /
        (float32_t)*const_h->Hall_tim_clk;
    motor->system.pwm_it_f =
        motor->system.pwm_freq / (const_h->PWM_htimx->Init.Period * 2);

    motor->system.omega_fbk = // 單次霍爾
        (motor->system.hall_freq * PI_MUL(2)) /
        (6.0f * ((float32_t)const_h->model->pole / 2.0f) * const_h->model->gear); // 運算時再/total時間
    motor->system.foc_it_angle_itpl = // 單次霍爾
        motor->system.pwm_period * (float32_t)(const_h->PWM_htimx->Init.Period * 2.0f)
        * PI_DIV(3) / motor->system.hall_period;

    motor->deg_h.pi_omega.Kp = const_h->model->deg_spd_Kp;
    motor->deg_h.pi_omega.Ki = const_h->model->deg_spd_Ki;
    motor->deg_h.pi_omega.max = 1.0f;
    motor->deg_h.pi_omega.min = 0.0f;
    motor_foc_pi_init(motor);
}

static void motor_init_adc(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
        adcm_init_load(&motor->adc_h.uvw[i].adc);
    RESULT_HAL_CHECK_HANDLE(HAL_ADCEx_Calibration_Start(
        motor->adc_h.v_cur.adc.basic.const_h.hadcx, ADC_SINGLE_ENDED));
    RESULT_HAL_CHECK_HANDLE(HAL_ADCEx_Calibration_Start(
        motor->adc_h.u_cur.adc.basic.const_h.hadcx, ADC_SINGLE_ENDED));
    RESULT_HAL_CHECK_HANDLE(
        HAL_ADCEx_InjectedStart(motor->adc_h.v_cur.adc.basic.const_h.hadcx));
    RESULT_HAL_CHECK_HANDLE(
        HAL_ADCEx_InjectedStart_IT(motor->adc_h.u_cur.adc.basic.const_h.hadcx));
}

static void motor_init_timer(MotorParameter *motor)
{
    uint8_t i;
    const MotorConst *const_h = &motor->system.const_h;
    __HAL_TIM_SET_COMPARE(const_h->PWM_htimx, const_h->PWM_mid_ch,
        const_h->PWM_htimx->Init.Period - TIM1_ADC_TRI_DL);
    RESULT_HAL_CHECK_HANDLE(HAL_TIM_Base_Start(const_h->PWM_htimx));
    RESULT_HAL_CHECK_HANDLE(
        HAL_TIM_PWM_Start(const_h->PWM_htimx, const_h->PWM_mid_ch));
    for (i = 0; i < 3; i++)
    {
        HAL_TIM_PWM_Start(const_h->PWM_htimx, const_h->PWM_uvw[i].pwm_ch);
        HAL_TIMEx_PWMN_Start(const_h->PWM_htimx, const_h->PWM_uvw[i].pwm_ch);
    }
    __HAL_TIM_SET_AUTORELOAD(const_h->Hall_htimx, motor->rotor.tim_ar);
    __HAL_TIM_ENABLE_IT(const_h->Hall_htimx, TIM_IT_UPDATE);
    __HAL_TIM_URS_ENABLE(const_h->Hall_htimx);
    HAL_TIMEx_HallSensor_Start_IT(const_h->Hall_htimx);
}

void motor_init(MotorParameter *motor)
{
    motor->tim_tick_init = 0;
    motor_init_system(motor);
    motor_init_adc(motor);
    motor_init_timer(motor);

    uint8_t phase = motor_rotor_hall_get(motor);
    motor->rotor.curr = phase;
    motor->rotor.prev = phase;

    // HAL_DAC_Start(&hdac1, DAC_CHANNEL_1); 
    // HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);

    motor->ctrl.ref_sys = MOTOR_CTRL_INIT;
}

/* ---------- Motor timer ---------- */

inline void motor_timer_load_inner(MotorParameter *motor, uint32_t u, uint32_t v, uint32_t w)
{
    MotorSystemConst *c = &motor->system;
    *(c->PWM_Addr_CCR_u) = u;
    *(c->PWM_Addr_CCR_v) = v;
    *(c->PWM_Addr_CCR_w) = w;
}

void motor_timer_load(MotorParameter *motor)
{
    MotorPhaseDuty *d = &motor->final_duty;
    VAR_CLAMPF(d->u_h, 0.0f, 1.0f);
    VAR_CLAMPF(d->v_h, 0.0f, 1.0f);
    VAR_CLAMPF(d->w_h, 0.0f, 1.0f);
    uint32_t Period = motor->system.const_h.PWM_htimx->Init.Period;
    motor_timer_load_inner(
        motor,
        (uint32_t)(Period * (1.0f - d->u_h)),
        (uint32_t)(Period * (1.0f - d->v_h)),
        (uint32_t)(Period * (1.0f - d->w_h))
    );
}

/**
 * @brief 設定 PWMN 腳位為 Output 模式 (關閉定時器 AF，輸出固定電平)
 *        MODE = 01b (MODEx_0)
 */
static inline void phase_pwmn_off(const MotorPhaseConst *phase)
{
    FLAGS_FULLSET(phase->pwmn_gpio.GPIOx->MODER, phase->pwmn_mode.MODEx, phase->pwmn_mode.MODEx_0);
}

/**
 * @brief 設定 PWMN 腳位為 Alternate Function 模式 (開啟定時器互補輸出)
 *        MODE = 10b (MODEx_1)
 */
static inline void phase_pwmn_on(const MotorPhaseConst *phase)
{
    FLAGS_FULLSET(phase->pwmn_gpio.GPIOx->MODER, phase->pwmn_mode.MODEx, phase->pwmn_mode.MODEx_1);
}

/* ---------- Motor adc ---------- */

inline void motor_adcs_init(MotorParameter *motor)
{
    MotorADCParam *a = &motor->adc_h;
    uint8_t i;
    for (i = 0; i < MOTOR_ADCS_CNT; i++)
    {
        adcm_offset_set(&a->uvw[i].adc);
    }
}

inline void motor_adcs_I_inject(MotorParameter *motor)
{
    MotorADCParam *a = &motor->adc_h;
    adc_upd_inject(&a->u_cur.adc.basic);
    adc_upd_inject(&a->v_cur.adc.basic);
    adc_upd_inject(&a->w_cur.adc.basic);
}

inline void motor_adcs_V_inject(MotorParameter *motor)
{
    MotorADCParam *a = &motor->adc_h;
    adc_upd_inject(&a->u_vol.adc.basic);
    adc_upd_inject(&a->v_vol.adc.basic);
    adc_upd_inject(&a->w_vol.adc.basic);
}

void motor_adcs_I_calculate(MotorParameter *motor)
{
    MotorADCParam *a = &motor->adc_h;
    adcm_update(&a->u_cur.adc);
    adcm_update(&a->v_cur.adc);
    adcm_update(&a->w_cur.adc);
    adcm_max_min_upd(&a->u_cur.adc);
    adcm_max_min_upd(&a->v_cur.adc);
    adcm_max_min_upd(&a->w_cur.adc);

    float32_t total =
        a->u_cur.adc.value +
        a->v_cur.adc.value +
        a->w_cur.adc.value;
    float32_t avg = total * VAR_ONE_DIV(3);
    a->total_i = total;

    float32_t inv_i_rated = VAR_ONE_DIV(motor->system.const_h.model->rated_current);
    a->u_cur.pu =
        (a->u_cur.adc.value - avg) * inv_i_rated;
    a->v_cur.pu =
        (a->v_cur.adc.value - avg) * inv_i_rated;
    a->w_cur.pu =
        (a->w_cur.adc.value - avg) * inv_i_rated;
}

void motor_adcs_V_calculate(MotorParameter *motor)
{
    // 消磁遮蔽
    motor->tim_tick_ch_phase++;
    if (motor->tim_tick_ch_phase < DEMAG_BLANKING_TICKS) return;

    MotorADCParam *a = &motor->adc_h;

    adcm_update(&a->u_vol.adc);
    adcm_update(&a->v_vol.adc);
    adcm_update(&a->w_vol.adc);
    adcm_max_min_upd(&a->u_vol.adc);
    adcm_max_min_upd(&a->v_vol.adc);
    adcm_max_min_upd(&a->w_vol.adc);

    float32_t total =
        a->u_vol.adc.value +
        a->v_vol.adc.value +
        a->w_vol.adc.value;
    float32_t avg = total * VAR_ONE_DIV(3);
    a->avg_v = avg;

    float32_t inv_v_rated = VAR_ONE_DIV(motor->system.const_h.model->rated_voltage);
    a->u_vol.pu =
        (a->u_vol.adc.value - avg) * inv_v_rated;
    a->v_vol.pu =
        (a->v_vol.adc.value - avg) * inv_v_rated;
    a->w_vol.pu =
        (a->w_vol.adc.value - avg) * inv_v_rated;
}

/* ---------- Motor set ---------- */

void motor_set_speed(MotorParameter *motor, float32_t rpm)
{
    motor->speed.ref_rpm = rpm;
    motor->speed.ref_omega = rpm * RPM_TO_OMEGA;
}

void motor_set_rotate_mode(MotorParameter *motor, MotorRotateMode mode)
{
    if (motor->rotate.ref_user == mode) return;
    switch (mode)
    {
    	case MOTOR_ROTATE_UNINIT: return;
        case MOTOR_ROTATE_LOCK_FIN:
        {
            mode = MOTOR_ROTATE_LOCK;
        }
        case MOTOR_ROTATE_COAST:
        case MOTOR_ROTATE_BREAK:
        case MOTOR_ROTATE_LOCK:
        case MOTOR_ROTATE_NORMAL:
        {
            break;
        }
    }
    motor->rotate.ref_user = mode;
}

void motor_switch_ctrl_system(MotorParameter *motor, MotorCtrlMode ctrl)
{
    const MotorConst *const_h = &motor->system.const_h;
    switch (ctrl)
    {
        case MOTOR_CTRL_UNINIT:
        case MOTOR_CTRL_INIT:
            return;
        case MOTOR_CTRL_TEST_HIGH:
        case MOTOR_CTRL_TEST_LOW:
        case MOTOR_CTRL_120_NORMAL:
        case MOTOR_CTRL_120_DUTY:
        case MOTOR_CTRL_120_DIREC_SW:
        case MOTOR_CTRL_FOC_I_F_ACCE:
        {
            motor_rotor_set_overflow(motor, 17000000);
            phase_pwmn_off(&const_h->PWM_u);
            phase_pwmn_off(&const_h->PWM_v);
            phase_pwmn_off(&const_h->PWM_w);
            break;
        }
        case MOTOR_CTRL_120_I_F_SAME:
        {
            motor_rotor_set_overflow(motor, 1700000000);
            phase_pwmn_off(&const_h->PWM_u);
            phase_pwmn_off(&const_h->PWM_v);
            phase_pwmn_off(&const_h->PWM_w);
            break;
        }
        case MOTOR_CTRL_TEST_WAVE:
        case MOTOR_CTRL_FOC_NORMAL:
        case MOTOR_CTRL_FOC_SIM:
        case MOTOR_CTRL_FOC_POS:
        case MOTOR_CTRL_FOC_ROT_CMD:
        case MOTOR_CTRL_FOC_ROT_AUTO:
        case MOTOR_CTRL_FOC_OL_VDQ:
        case MOTOR_CTRL_FOC_OL_IQ:
        {
            phase_pwmn_on(&const_h->PWM_u);
            phase_pwmn_on(&const_h->PWM_v);
            phase_pwmn_on(&const_h->PWM_w);
            break;
        }
    }
    motor->ctrl.ref_sys = ctrl;
}

void motor_set_ctrl_mode(MotorParameter *motor, MotorCtrlMode ctrl)
{
    switch (ctrl)
    {
        case MOTOR_CTRL_UNINIT:
        case MOTOR_CTRL_INIT:
            return;
        case MOTOR_CTRL_TEST_HIGH:
        case MOTOR_CTRL_TEST_LOW:
        case MOTOR_CTRL_TEST_WAVE:
        case MOTOR_CTRL_120_NORMAL:
        case MOTOR_CTRL_120_DUTY:
        case MOTOR_CTRL_120_I_F_SAME:
        case MOTOR_CTRL_120_DIREC_SW:
        case MOTOR_CTRL_FOC_I_F_ACCE:
        {
            motor_switch_ctrl_system(motor, ctrl);
            break;
        }
        case MOTOR_CTRL_FOC_NORMAL:
        case MOTOR_CTRL_FOC_SIM:
        case MOTOR_CTRL_FOC_POS:
        case MOTOR_CTRL_FOC_ROT_CMD:
        case MOTOR_CTRL_FOC_ROT_AUTO:
        case MOTOR_CTRL_FOC_OL_VDQ:
        case MOTOR_CTRL_FOC_OL_IQ:
        {
            motor_switch_ctrl_system(motor, ctrl);
            motor->foc_h.tim_tick_init = 0;
            break;
        }
    }
    motor->ctrl.ref_user = ctrl;
}

/* ---------- Motor main ---------- */

// 主要程式皆由 callback 處理
__weak void motor_main(MotorParameter *motor)
{
}

#endif
