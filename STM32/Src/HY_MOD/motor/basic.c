#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "tim.h"
#include "HY_MOD/main/tim.h"

const uint8_t hall_seq_ccw[8] = {UINT8_MAX, 5, 3, 1, 6, 4, 2, UINT8_MAX};
const uint8_t hall_seq_clw[8] = {UINT8_MAX, 3, 6, 2, 5, 1, 4, UINT8_MAX};
MotorParameter motor_h = {
    .const_h = {
        // PC0     ------> TIM1_CH1
        // PC1     ------> TIM1_CH2
        // PC2     ------> TIM1_CH3
        // PB13     ------> TIM1_CH1N
        // PB14     ------> TIM1_CH2N
        // PB15     ------> TIM1_CH3N
        .PWM_htimx          = &htim1,
        .PWM_TIM_CHANNEL_x  = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3 },
        .PWM_tim_clk        = &tim_clk_APB2,
        .PWM_MID_TIM_CH_x   = TIM_CHANNEL_4,  
        // PA0     ------> TIM2_CH1
        // PA1     ------> TIM2_CH2
        // PB10     ------> TIM2_CH3
        .Hall_htimx         = &htim2,
        .Hall_tim_clk       = &tim_clk_APB1,
        .Hall_GPIOx         = { GPIOC,       GPIOC,       GPIOC       },
        .Hall_GPIO_Pin_x    = { GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12 },
        // 42BLF01
        .rated_current      = MOTOR_42BLF01_RATED_CURRENT,
        .peak_current       = MOTOR_42BLF01_PEAK_CURRENT,
    },
    // Yellow
    .adc_a = &adc_0,
    // Green
    .adc_b = &adc_1,
    // Blue
    .adc_c = &adc_2,
    .pi_speed = {
        .reference = 0.0f,
        .Kp = 0.000025f,
        .Ki = 0.002f,
        .max = 0.1f,
        .min = -0.1f,
        .saturation = 1.0f,
    },
    .tfm_duty_Iq = 1.0f,
    .pi_Iq = {
        .Kp = 0.3f,
        .Ki = 0.001f,
        // .max = 0.75f, In motor_init
        // .min = -0.75f, In motor_init
        .saturation = 1.0f,
    },
    .pi_Id = {
        .Kp = 0.2f,
        .Ki = 0.001f,
        .max = 0.01f,
        .min = -0.01f,
        .saturation = 1.0f,
    },
    .hall_start = 4,
    .rpm_save_stop = 10.0f,
};

void motor_init(MotorParameter *motor)
{
    motor->pi_Iq.max =  motor->const_h.rated_current;
    motor->pi_Iq.min = -motor->const_h.rated_current;
}

void motor_set_rpm(MotorParameter *motor, bool reverse, float32_t speed)
{
    if (speed != 0) motor->rpm_user.reverse = reverse;
    motor->rpm_user.value = speed;
}

void motor_set_rotate_mode(MotorParameter *motor, MotorModeRotate mode)
{
    if (
        mode != MOTOR_ROT_COAST &&
        mode != MOTOR_ROT_BREAK &&
        mode != MOTOR_ROT_NORMAL &&
        mode != MOTOR_ROT_LOCK &&
        mode != MOTOR_ROT_LOCK_FIN
    ) return;
    if (mode == MOTOR_ROT_LOCK_FIN) mode = MOTOR_ROT_LOCK;
    motor->mode_rot_user = mode;
}

void motor_alive(MotorParameter *motor)
{
    motor->alive_tick = HAL_GetTick();
}

void motor_switch_ctrl(MotorParameter *motor, MotorModeControl ctrl)
{
    motor->mode_control = ctrl;
}

void motor_pwm_load(MotorParameter *motor)
{
    TIM_HandleTypeDef *htimx = motor->const_h.PWM_htimx;
    VAR_CLAMPF(motor->pwm_duty_u, 0.0f, 1.0f);
    VAR_CLAMPF(motor->pwm_duty_v, 0.0f, 1.0f);
    VAR_CLAMPF(motor->pwm_duty_w, 0.0f, 1.0f);
    __HAL_TIM_SET_COMPARE(htimx, motor->const_h.PWM_TIM_CHANNEL_x[0],
        (uint32_t)(motor->tfm_pwm_period * motor->pwm_duty_u));
    __HAL_TIM_SET_COMPARE(htimx, motor->const_h.PWM_TIM_CHANNEL_x[1],
        (uint32_t)(motor->tfm_pwm_period * motor->pwm_duty_v));
    __HAL_TIM_SET_COMPARE(htimx, motor->const_h.PWM_TIM_CHANNEL_x[2],
        (uint32_t)(motor->tfm_pwm_period * motor->pwm_duty_w));
}

void motor_history_write(MotorParameter *motor)
{
    uint16_t idx = motor->history.head;
    motor->history.data[idx].spd_ref = (!motor->rpm_reference.reverse) ?
        motor->rpm_reference.value : -motor->rpm_reference.value;
    motor->history.data[idx].spd_fbk = (!motor->rpm_feedback.reverse) ?
        motor->rpm_feedback.value  : -motor->rpm_feedback.value;
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