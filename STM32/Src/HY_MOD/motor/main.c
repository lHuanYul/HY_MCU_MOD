#include "HY_MOD/motor/main.h"
#ifdef HY_MOD_STM32_MOTOR

#include "tim.h"

// Motor setup
void motor_setup(MotorParameter *motor)
{
    const float32_t PWM_tim_f =
        (float32_t)*motor->const_h.PWM_tim_clk /
        (float32_t)(motor->const_h.PWM_htimx->Init.Prescaler + 1U);

    // HALL_tim_f：霍爾計時器的實際計數頻率 (Hz)
    // = HALL_timer_clock / (PSC + 1)
    const float32_t HALL_tim_f =
        (float32_t)*motor->const_h.Hall_tim_clk /
        (float32_t)(motor->const_h.Hall_htimx->Init.Prescaler + 1U);

    // PWM_tim_t：PWM 控制定時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / PWM_timer_clock
    const float32_t PWM_tim_t =
        (float32_t)(motor->const_h.PWM_htimx->Init.Prescaler + 1U) /
        (float32_t)*motor->const_h.PWM_tim_clk;

    // HALL_tim_t：霍爾計時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / HALL_timer_clock
    const float32_t HALL_tim_t =
        (float32_t)(motor->const_h.Hall_htimx->Init.Prescaler + 1U) /
        (float32_t)*motor->const_h.Hall_tim_clk;

    motor->dbg_h.pwm_freq = PWM_tim_f / (motor->const_h.PWM_htimx->Init.Period * 2);

    motor->tfm_h.rpm_fbk =
        ((float32_t)MOTOR_RPM_CNT * HALL_tim_f * 60.0f) /
        ((float32_t)MOTOR_POLE / 2.0f * 6.0f * (float32_t)MOTOR_GEAR);
    motor->tfm_h.foc_it_angle_itpl =
        PWM_tim_t /
        HALL_tim_t * (float32_t)(motor->const_h.PWM_htimx->Init.Period * 2.0f) * PI_DIV_3;

    motor->foc_h.pi_Iq_h.Kp = MOTOR_LL * 1000.0f;
    motor->foc_h.pi_Iq_h.Ki = 1.0f / MOTOR_TAU * PWM_tim_t;
    motor->foc_h.pi_Iq_h.max = 0.577f;
    motor->foc_h.pi_Iq_h.min = -0.577f;
    motor->foc_h.pi_Id_h.Kp = MOTOR_LL * 1000.0f;
    motor->foc_h.pi_Id_h.Ki = 1.0f / MOTOR_TAU * PWM_tim_t;
    motor->foc_h.pi_Id_h.max = 0.577f;
    motor->foc_h.pi_Id_h.min = -0.577f;

    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        ERROR_CHECK_HAL_HANDLE(HAL_ADCEx_Calibration_Start(
            motor->foc_h.adc_h.uvw[i]->basic.hadcx, ADC_SINGLE_ENDED));
    }
    ERROR_CHECK_HAL_HANDLE(
        HAL_ADCEx_InjectedStart_IT(motor->foc_h.adc_h.v->basic.hadcx));
    ERROR_CHECK_HAL_HANDLE(
        HAL_ADCEx_InjectedStart_IT(motor->foc_h.adc_h.u->basic.hadcx));

    __HAL_TIM_SET_COMPARE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CH_x.mid,
        motor->const_h.PWM_htimx->Init.Period - 1);
    ERROR_CHECK_HAL_HANDLE(HAL_TIM_Base_Start(motor->const_h.PWM_htimx));
    ERROR_CHECK_HAL_HANDLE(
        HAL_TIM_PWM_Start(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CH_x.mid));
    for (i = 0; i < 3; i++)
    {
        HAL_TIM_PWM_Start(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CH_x.uvw[i]);
        HAL_TIMEx_PWMN_Start(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CH_x.uvw[i]);
    }
    __HAL_TIM_ENABLE_IT(motor->const_h.Hall_htimx, TIM_IT_UPDATE);
    __HAL_TIM_URS_ENABLE(motor->const_h.Hall_htimx);
    HAL_TIMEx_HallSensor_Start_IT(motor->const_h.Hall_htimx);
}

void motor_timer_load(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        VAR_CLAMPF(motor->duty_load.uvw[i], 0.0f, 1.0f);
        __HAL_TIM_SET_COMPARE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CH_x.uvw[i],
            (uint32_t)(motor->const_h.PWM_htimx->Init.Period * motor->duty_load.uvw[i]));
    }
}

void motor_set_rpm(MotorParameter *motor, bool reverse, float32_t speed)
{
    if (speed != 0) motor->rpm_h.ref_ori.reverse = reverse;
    motor->rpm_h.ref_ori.value = speed;
}

void motor_set_rotate_mode(MotorParameter *motor, MotorRot mode)
{
    if (
        mode != MOTOR_ROT_COAST &&
        mode != MOTOR_ROT_BREAK &&
        mode != MOTOR_ROT_NORMAL &&
        mode != MOTOR_ROT_LOCK &&
        mode != MOTOR_ROT_LOCK_FIN
    ) return;
    if (mode == MOTOR_ROT_LOCK_FIN) mode = MOTOR_ROT_LOCK;
    motor->rotate_h.ref_ori = mode;
}

void motor_switch_ctrl(MotorParameter *motor, MotorCtrl ctrl)
{
    const MotorConst *const_h = &motor->const_h;
    uint8_t i;
    uint32_t temp;
    #define SET_PWM_OFF(pin, data) \
    do { \
        temp = (pin).GPIOx->MODER; \
        temp &= ~(data.MODEx); \
        temp |= (data.MODEx_0); \
        (pin).GPIOx->MODER = temp; \
    } while(0)
    #define SET_PWM_ON(pin, data) \
    do { \
        temp = (pin).GPIOx->MODER; \
        temp &= ~(data.MODEx); \
        temp |= (data.MODEx_1); \
        (pin).GPIOx->MODER = temp; \
    } while(0)

    switch (ctrl)
    {
        case MOTOR_CTRL_TEST_H:
        case MOTOR_CTRL_TEST_L:
        case MOTOR_CTRL_120:
        {
            for (i = 0; i < 3; i++)
                SET_PWM_OFF(const_h->PWMN_GPIO.uvw[i], const_h->PWMN_GPIO_set.uvw[i]);
            break;
        }
        case MOTOR_CTRL_FOC_RATED:
        {
            for (i = 0; i < 3; i++)
                SET_PWM_ON(const_h->PWMN_GPIO.uvw[i], const_h->PWMN_GPIO_set.uvw[i]);
            break;
        }
        default: return;
    }
    motor->ctrl_h.ref_fix = ctrl;
}

void motor_history_write(MotorParameter *motor)
{
    uint16_t idx = motor->history.head;
    motor->history.data[idx].spd_ref = (!motor->rpm_h.ref_fix.reverse) ?
        motor->rpm_h.ref_fix.value : -motor->rpm_h.ref_fix.value;
    motor->history.data[idx].spd_fbk = (!motor->rpm_h.fb.reverse) ?
        motor->rpm_h.fb.value  : -motor->rpm_h.fb.value;
    motor->history.cnt++;
    idx++;
    if (idx >= MOTOR_HISTORY_LEN)
    {
        idx = 0;
        motor->history.is_full = 1;
    }
    motor->history.head = idx;
}

#endif