#include "HY_MOD/motor/main.h"
#ifdef HY_MOD_STM32_MOTOR

#include "tim.h"

// Motor setup
void motor_setup(MotorParameter *motor)
{
    motor_init(motor);
    const float32_t PWM_tim_f =
        (float32_t)*motor->const_h.PWM_tim_clk /
        (float32_t)(motor->const_h.PWM_htimx->Init.Prescaler + 1U);

    // ELE_tim_f：霍爾計時器的實際計數頻率 (Hz)
    // = ELE_timer_clock / (PSC + 1)
    const float32_t SPD_tim_f =
        (float32_t)*motor->const_h.Hall_tim_clk /
        (float32_t)(motor->const_h.Hall_htimx->Init.Prescaler + 1U);

    // PWM_tim_t：PWM 控制定時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / ELE_timer_clock
    const float32_t PWM_tim_t =
        (float32_t)(motor->const_h.PWM_htimx->Init.Prescaler + 1U) /
        (float32_t)*motor->const_h.PWM_tim_clk;

    // SPD_tim_t：霍爾計時器每個計數週期的時間 (秒/計數)
    // = (PSC + 1) / ELE_timer_clock
    const float32_t SPD_tim_t =
        (float32_t)(motor->const_h.Hall_htimx->Init.Prescaler + 1U) /
        (float32_t)*motor->const_h.Hall_tim_clk;

    motor->dbg_pwm_freq = PWM_tim_f / (motor->const_h.PWM_htimx->Init.Period * 2);
    // motor->dbg_tim_it_freq = FOC_tim_f / motor->const_h.IT20k_htimx->Init.Period;

    motor->tfm_pwm_period = motor->const_h.PWM_htimx->Init.Period;
    motor->tfm_rpm_fbk =
        ((float32_t)MOTOR_RPM_CNT * SPD_tim_f * 60.0f) /
        ((float32_t)MOTOR_POLE / 2.0f * 6.0f * (float32_t)MOTOR_GEAR);
    motor->tfm_foc_it_angle_itpl =
        PWM_tim_t / SPD_tim_t * (float32_t)(motor->const_h.PWM_htimx->Init.Period) * PI_DIV_3;

    ERROR_CHECK_HAL_HANDLE(HAL_ADCEx_Calibration_Start(motor->adc_a->adc.hadcx, ADC_SINGLE_ENDED));
    ERROR_CHECK_HAL_HANDLE(HAL_ADCEx_Calibration_Start(motor->adc_b->adc.hadcx, ADC_SINGLE_ENDED));
    ERROR_CHECK_HAL_HANDLE(HAL_ADCEx_Calibration_Start(motor->adc_c->adc.hadcx, ADC_SINGLE_ENDED));
    ERROR_CHECK_HAL_HANDLE(HAL_ADCEx_InjectedStart_IT(motor->adc_a->adc.hadcx));

    __HAL_TIM_SET_COMPARE(motor->const_h.PWM_htimx, motor->const_h.PWM_MID_TIM_CH_x,
        motor->const_h.PWM_htimx->Init.Period - 1);
    ERROR_CHECK_HAL_HANDLE(HAL_TIM_Base_Start(motor->const_h.PWM_htimx));
    ERROR_CHECK_HAL_HANDLE(HAL_TIM_PWM_Start(motor->const_h.PWM_htimx, motor->const_h.PWM_MID_TIM_CH_x));
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        HAL_TIM_PWM_Start(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
        HAL_TIMEx_PWMN_Start(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i]);
    }
    HAL_TIMEx_HallSensor_Start_IT(motor->const_h.Hall_htimx);
}

void motor_timer_load(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        VAR_CLAMPF(motor->duty_load.uvw[i], 0.0f, 1.0f);
        __HAL_TIM_SET_COMPARE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i],
            (uint32_t)(motor->tfm_pwm_period * motor->duty_load.uvw[i]));
    }
}

#endif