#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

/* ---------- Motor datasheets ---------- */

// Max 36V 150Hz
const MotorModelData motor_vehicle = {
    .pole = 20,
    .gear = 4.4f,
    .rated_current = 1.9f,
    .rl = 0.32f / 2.0f,
    .tau = 0.0025f,
    .ll = (0.32f / 2.0f) * 0.0025f,
    .hall_angle_comp = PI_DIV_3 * 5.0f,
    .deg_spd_Kp = 0.0f,
    .deg_spd_Ki = 0.0f,
    .foc_spd_Kp = 0.5f,
    .foc_spd_Ki = 0.05f,
};

const MotorModelData motor_42BLF01 = {
    .pole = 8,
    .gear = 1.0f,
    // MOTOR_42BLF01_PEAK_I 5.7f
    .rated_current = 1.9f,
    .rl = 2.2f / 2.0f,
    .tau = 0.0011f,
    .ll = (2.2f / 2.0f) * 0.0011f,
    .hall_angle_comp = PI_DIV_3 * 5.0f,
    .deg_spd_Kp = 0.006f,
    .deg_spd_Ki = 0.006f,
    .foc_spd_Kp = 0.0f,
    .foc_spd_Ki = 0.0f,
};

/* ---------- Motor -------------------- */

#include "tim.h"
#include "HY_MOD/motor/rotor.h"
#include "HY_MOD/motor/ctrl_foc.h"
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
        (motor->system.hall_freq * PI_MUL_2) /
        (6.0f * ((float32_t)const_h->model->pole / 2.0f) * const_h->model->gear); // 運算時再/total時間
    motor->system.foc_it_angle_itpl = // 單次霍爾
        motor->system.pwm_period * (float32_t)(const_h->PWM_htimx->Init.Period * 2.0f)
        * PI_DIV_3 / motor->system.hall_period;

    motor->deg_h.pi_omega.Kp = const_h->model->deg_spd_Kp;
    motor->deg_h.pi_omega.Ki = const_h->model->deg_spd_Ki;
    motor->deg_h.pi_omega.max = 1.0f;
    motor->deg_h.pi_omega.min = 0.0f;
    motor_foc_pi_init(motor);
}

static void motor_init_adc(MotorParameter *motor)
{
    RESULT_HAL_CHECK_HANDLE(HAL_ADCEx_Calibration_Start(
        motor->adc_h.adc_vi.basic.hadcx, ADC_SINGLE_ENDED));
    RESULT_HAL_CHECK_HANDLE(HAL_ADCEx_Calibration_Start(
        motor->adc_h.adc_ui.basic.hadcx, ADC_SINGLE_ENDED));
    RESULT_HAL_CHECK_HANDLE(
        HAL_ADCEx_InjectedStart(motor->adc_h.adc_vi.basic.hadcx));
    RESULT_HAL_CHECK_HANDLE(
        HAL_ADCEx_InjectedStart_IT(motor->adc_h.adc_ui.basic.hadcx));
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
    __HAL_TIM_SET_AUTORELOAD(const_h->Hall_htimx, motor->rotor.overflow);
    __HAL_TIM_ENABLE_IT(const_h->Hall_htimx, TIM_IT_UPDATE);
    __HAL_TIM_URS_ENABLE(const_h->Hall_htimx);
    HAL_TIMEx_HallSensor_Start_IT(const_h->Hall_htimx);
}

void motor_init(MotorParameter *motor)
{
    motor->init_cnt = 0;
    motor_init_system(motor);
    motor_init_adc(motor);
    motor_init_timer(motor);

    // Todo Sensorless
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
    *(motor->system.PWM_Addr_CCR_u) = u;
    *(motor->system.PWM_Addr_CCR_v) = v;
    *(motor->system.PWM_Addr_CCR_w) = w;
}

void motor_timer_load(MotorParameter *motor)
{
    VAR_CLAMPF(motor->phases_duty_load.u, 0.0f, 1.0f);
    VAR_CLAMPF(motor->phases_duty_load.v, 0.0f, 1.0f);
    VAR_CLAMPF(motor->phases_duty_load.w, 0.0f, 1.0f);
    uint32_t Period = motor->system.const_h.PWM_htimx->Init.Period;
    motor_timer_load_inner(
        motor,
        (uint32_t)(Period * (1.0f - motor->phases_duty_load.u)),
        (uint32_t)(Period * (1.0f - motor->phases_duty_load.v)),
        (uint32_t)(Period * (1.0f - motor->phases_duty_load.w))
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
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        adc_current_init(&motor->adc_h.adcs[i].basic, motor->adc_h.adcs[i].model);
        // adc_voltage_init(&motor->adc_h.adcs[i+3].basic, motor->adc_h.adcs[i+3].model);
    }
}

inline void motor_adcs_upd(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
        adc_upd_injected(&motor->adc_h.adcs[i].basic);
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
        case MOTOR_CTRL_FOC_INIT:
        {
            motor_rotor_set_overflow(motor, 17000000);
            phase_pwmn_off(&const_h->PWM_u);
            phase_pwmn_off(&const_h->PWM_v);
            phase_pwmn_off(&const_h->PWM_w);
            break;
        }
        case MOTOR_CTRL_120_SIMULATE:
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
        case MOTOR_CTRL_120_SIMULATE:
        case MOTOR_CTRL_120_DIREC_SW:
        case MOTOR_CTRL_FOC_INIT:
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
            motor->foc_h.init_cnt = 0;
            break;
        }
    }
    motor->ctrl.ref_user = ctrl;
}

#endif
