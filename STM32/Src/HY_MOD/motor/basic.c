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
        .PWMN_GPIO = {
            .u = { .GPIOx = GPIOB, .Pin = GPIO_PIN_13 },
            .v = { .GPIOx = GPIOB, .Pin = GPIO_PIN_14 },
            .w = { .GPIOx = GPIOB, .Pin = GPIO_PIN_15 },
        },
        .PWMN_GPIO_set = {
            .u = { .MODEx = GPIO_MODER_MODE13,
              .MODEx_0 = GPIO_MODER_MODE13_0, .MODEx_1 = GPIO_MODER_MODE13_1 },
            .v = { .MODEx = GPIO_MODER_MODE14,
              .MODEx_0 = GPIO_MODER_MODE14_0, .MODEx_1 = GPIO_MODER_MODE14_1 },
            .w = { .MODEx = GPIO_MODER_MODE15,
              .MODEx_0 = GPIO_MODER_MODE15_0, .MODEx_1 = GPIO_MODER_MODE15_1 },
        },
        // PA0     ------> TIM2_CH1
        // PA1     ------> TIM2_CH2
        // PB10     ------> TIM2_CH3
        .Hall_htimx     = &htim2,
        .Hall_tim_clk   = &tim_clk_APB1,
        .Hall_GPIO = {
            .u = { .GPIOx = GPIOC, .Pin = GPIO_PIN_10 },
            .v = { .GPIOx = GPIOC, .Pin = GPIO_PIN_11 },
            .w = { .GPIOx = GPIOC, .Pin = GPIO_PIN_12 },
        },
        // 42BLF01
        .rated_current  = MOTOR_42BLF01_RATED_CURRENT,
        .peak_current   = MOTOR_42BLF01_PEAK_CURRENT,
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
    motor->mode_control = ctrl;
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