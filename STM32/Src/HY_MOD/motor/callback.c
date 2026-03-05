#include "HY_MOD/motor/callback.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/motor/ctrl_deg.h"
#include "HY_MOD/motor/ctrl_foc.h"
#include "HY_MOD/motor/trigonometric.h"
#include "HY_MOD/fdcan/pkt_write.h"

#define HALL_DELAY 10
#define HALL_DELAY_LOAD motor->hall_current // motor->hall_current

static void hall_update(MotorParameter *motor)
{
    motor->hall_current =
          (GPIO_READ_R(motor->const_h.Hall_GPIO.u) ? 4U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.v) ? 2U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.w) ? 1U : 0U);
    if (motor->hall_current == 0 || motor->hall_current == 7)
    {
        motor->hall_current = UINT8_MAX;
        return;
    }
    motor->hall_delay = HALL_DELAY;
    motor_vec_ctrl_hall_angle_trf(motor);
}

static void hall_check(MotorParameter *motor)
{
    if (motor->hall_current == hall_seq_ccw[motor->hall_chk_last])
    {
        motor->rpm_feedback.reverse = 0;
        motor->hall_wrong = 0;
    }
    else if (motor->hall_current == hall_seq_clw[motor->hall_chk_last])
    {
        motor->rpm_feedback.reverse = 1;
        motor->hall_wrong = 0;
    }
    else
    {
        motor->hall_wrong++;
        if (motor->hall_wrong >= 6)
        {
            motor->hall_wrong = 6;
            motor->rpm_feedback.reverse = 0;
            motor->rpm_feedback.value = 0.0f;
            motor->foc_angle_itpl = 0.0f;
        }
    }
    motor->hall_chk_last = motor->hall_current;
}

static void rpm_update(MotorParameter *motor)
{
    if (motor->exti_hall_acc == 0)
        __HAL_TIM_SET_COUNTER(motor->const_h.Hall_htimx, 0);
    else 
    if (motor->exti_hall_acc >= MOTOR_RPM_CNT)
    {
        motor->exti_hall_acc = 0;
        uint32_t htim_cnt = __HAL_TIM_GET_COUNTER(motor->const_h.Hall_htimx);
        __HAL_TIM_SET_COUNTER(motor->const_h.Hall_htimx, 0);
        if (htim_cnt == 0)
        {
            motor->rpm_feedback.value = 0.0f;
            motor->foc_angle_itpl = 0.0f;
        }
        else
        {
            motor->rpm_feedback.value = motor->tfm_rpm_fbk / (float32_t)htim_cnt;
            motor->foc_angle_itpl = (!motor->rpm_feedback.reverse) ?
                 motor->tfm_foc_it_angle_itpl / (float32_t)htim_cnt :
                -motor->tfm_foc_it_angle_itpl / (float32_t)htim_cnt;
        }
    }
    motor->exti_hall_acc++;
}

/*
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
*/
void motor_hall_exti_cb(MotorParameter *motor)
{
    hall_update(motor);
    hall_check(motor);
    rpm_update(motor);
// #ifndef MOTOR_FOC_SPIN_DEBUG
//     pwm_update(motor, motor->hall_current);
// #else
//     if (expected && (motor->reverse == reverse))
//     {
//         motor->tim_angle_acc = 0.0f;
//     }
//     RESULT_CHECK_RET_VOID(motor_vec_ctrl_hall_angle_trf(motor));
//     deg_ctrl_load(motor);
// #endif
}

void motor_stop_cb(MotorParameter *motor)
{
    motor->stop_spin_time = HAL_GetTick();
    motor->exti_hall_acc = 0;
    motor->rpm_feedback.value = 0;
    motor->pi_Iq.out = 0;
    motor->foc_angle_acc = 0.0f;
    PI_reset(&motor->pi_speed);
#ifdef MOTOR_PI_SPEED
    motor->duty_deg = 0.0f;
#endif
}

static void ctrl_start(MotorParameter *motor)
{
    // !
    motor_set_rotate_mode(motor, MOTOR_ROT_NORMAL);
    motor_switch_ctrl(motor, MOTOR_CTRL_120);
    motor_set_rpm(motor, 0, 500.0f);
}

static void state_update(MotorParameter *motor)
{
    motor->mode_rot_ref = motor->mode_rot_user;
    motor->rpm_reference.reverse = motor->rpm_user.reverse;
    motor->rpm_reference.value = motor->rpm_user.value;
    bool save_stop = (motor->rpm_feedback.value < motor->rpm_save_stop) ? 1 : 0;
    bool ref_fbk_same_dir = (motor->rpm_user.reverse == motor->rpm_feedback.reverse) ? 1 : 0;
    switch (motor->dict_state)
    {
        case DIRECTION_NORMAL:
        {
            if (ref_fbk_same_dir) break;
            motor->dict_state = DIRECTION_SWITCHING;
        }
        case DIRECTION_SWITCHING:
        {
            // rpm來不及算到就煞停了
            if (motor->mode_rot_ref == MOTOR_ROT_COAST) break;
            if (save_stop)
            {
                motor->dict_state = DIRECTION_NORMAL;
                motor->exti_hall_acc = 0;
                motor->rpm_feedback.value = 0;
                break;
            }
            motor->mode_rot_ref = MOTOR_ROT_BREAK;
            break;
        }
    }
    // if (HAL_GetTick() - motor->alive_tick >= 1000)
    // {
    //     motor_set_rotate_mode(motor, MOTOR_ROT_COAST);
    // }
    motor->pi_speed.feedback = motor->rpm_feedback.value;
    switch (motor->mode_rot_ref)
    {
        case MOTOR_ROT_COAST:
        case MOTOR_ROT_BREAK:
        case MOTOR_ROT_LOCK_FIN:
        {
            PI_reset(&motor->pi_speed);
            break;
        }
        case MOTOR_ROT_NORMAL:
        {
            motor->pi_speed.reference = motor->rpm_reference.value;
            PI_run(&motor->pi_speed);
    #ifdef MOTOR_PI_SPEED
            motor->duty_deg += motor->pi_speed.out;
            VAR_CLAMPF(motor->duty_deg, 0.0f, 1.0f);
    #else
            motor->deg_duty = 1.0f;
    #endif
            motor->pi_Iq.reference += motor->pi_speed.out * motor->tfm_duty_Iq;
            VAR_CLAMPF(motor->pi_Iq.reference, motor->pi_Iq.min, motor->pi_Iq.max);
            break;
        }
        case MOTOR_ROT_LOCK:
        {
            motor->mode_rot_ref = MOTOR_ROT_BREAK;
            if (save_stop) motor->mode_rot_ref = MOTOR_ROT_LOCK_FIN;
            break;
        }
    }

    motor->pi_Iq.reference = (!motor->rpm_user.reverse) ?
        motor->const_h.rated_current : -motor->const_h.rated_current;
    // motor->tfm_duty_Iq = var_clampf((motor->tfm_duty_Iq + motor->pi_speed.out), 0.15f, 0.2f);
}

static void foc_run(MotorParameter *motor)
{
    RESULT_CHECK_RET_VOID(motor_vec_ctrl_hall_angle_chk(motor));
    motor_vec_ctrl_clarke(motor);
    motor_vec_ctrl_park(motor);
    motor_vec_ctrl_pi_id_iq(motor);
    motor_vec_ctrl_ipark(motor);
    motor_vec_ctrl_svgen(motor);
    motor_vec_ctrl_svpwm(motor);
}

/* 20kHz
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
*/
void motor_pwm_cb(MotorParameter *motor)
{
    motor_vec_ctrl_adcs_upd(motor);
    if (motor->mode_control == MOTOR_CTRL_INIT)
    {
        if (motor->tim_it_acc >= 20000)
        {
            motor->tim_it_acc = 0;
            motor_vec_ctrl_adcs_reset(motor);
            ctrl_start(motor);
        }
        else
        {
            motor->tim_it_acc++;
            return;
        }
    }
    if (motor->tim_it_acc % 1000 == 0)
    {
        state_update(motor);
    }
    if (motor->tim_it_acc % 2000 == 0)
    {
        if (
            motor->rpm_feedback.value == 0.0f &&
            motor->rpm_reference.value != 0.0f
        ) {
            hall_update(motor);
    #ifdef MOTOR_AUTO_SPIN
            motor_switch_ctrl(motor, MOTOR_CTRL_120);
            motor->hall_delay = 1;
//            motor->deg_duty = 1.0f;
    #endif
        }
        if (
            motor->fdcan_enable
        ) {
            motor->fdcan_send = 1;
            motor_history_write(motor);
        }
    }
    motor->tim_it_acc++;
    if (motor->tim_it_acc >= 20000) motor->tim_it_acc = 0;
    
    foc_run(motor);

    uint8_t i;
    switch (motor->mode_control)
    {
        case MOTOR_CTRL_120:
        {
            uint16_t hall_delay = motor->hall_delay;
            if (hall_delay > 0)
            {
                if (hall_delay == HALL_DELAY)
                    deg_ctrl_120_load(motor, HALL_DELAY_LOAD);
                else if (motor->hall_delay == 1)
                    deg_ctrl_120_load(motor, motor->hall_current);
                // deg_ctrl_120_load(motor, 4);
                motor->duty_load = motor->duty_deg;
                motor->hall_delay--;
            }
            break;
        }
    #ifndef MOTOR_FOC_SPIN_DEBUG
        case MOTOR_CTRL_FOC_RATED:
        {
            motor->duty_load = motor->duty_foc;
            break;
        }
    #endif
        default: return;
    }
    for (i = 0; i < 3; i++)
    {
        VAR_CLAMPF(motor->duty_load.uvw[i], 0.0f, 1.0f);
        __HAL_TIM_SET_COMPARE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x[i],
            (uint32_t)(motor->tfm_pwm_period * motor->duty_load.uvw[i]));
    }
}

#endif
