#include "HY_MOD/motor/callback.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/motor/main.h"
#include "HY_MOD/motor/ctrl_deg.h"
#include "HY_MOD/motor/ctrl_foc.h"
#include "HY_MOD/motor/trigonometric.h"
#include "HY_MOD/fdcan/pkt_write.h"

#define HALL_DELAY 10
#define HALL_DELAY_LOAD motor->hall_h.current // motor->hall_h.current

static void hall_update(MotorParameter *motor)
{
    motor->hall_h.current =
          (GPIO_READ_R(motor->const_h.Hall_GPIO.u) ? 4U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.v) ? 2U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.w) ? 1U : 0U);
    if (motor->hall_h.current == 0 || motor->hall_h.current == 7)
    {
        motor->hall_h.current = UINT8_MAX;
        return;
    }
    motor->hall_h.delay = HALL_DELAY;
}

static void rotate_check(MotorParameter *motor)
{
    if (motor->hall_h.current == hall_seq_ccw[motor->hall_h.chk_last])
    {
        motor->rpm_h.feedback.reverse = 0;
        motor->hall_h.wrong = 0;
    }
    else if (motor->hall_h.current == hall_seq_clw[motor->hall_h.chk_last])
    {
        motor->rpm_h.feedback.reverse = 1;
        motor->hall_h.wrong = 0;
    }
    else
    {
        motor->hall_h.wrong++;
        if (motor->hall_h.wrong >= 6)
        {
            motor->hall_h.wrong = 6;
            motor->rpm_h.feedback.reverse = 0;
            motor->rpm_h.feedback.value = 0.0f;
            motor->foc_h.angle_itpl = 0.0f;
        }
    }
    motor->hall_h.chk_last = motor->hall_h.current;
}

static void rpm_update(MotorParameter *motor)
{
    motor->hall_h.time_cnt +=
        __HAL_TIM_GET_COMPARE(motor->const_h.Hall_htimx, TIM_CHANNEL_1);
    if (motor->hall_h.it_cnt >= MOTOR_RPM_CNT)
    {
        motor->hall_h.it_cnt = 0;
        motor->rpm_h.feedback.value =
            motor->tfm_h.rpm_fbk / (float32_t)motor->hall_h.time_cnt;
        motor->foc_h.angle_itpl = (!motor->rpm_h.feedback.reverse) ?
             motor->tfm_h.foc_it_angle_itpl / (float32_t)motor->hall_h.time_cnt :
            -motor->tfm_h.foc_it_angle_itpl / (float32_t)motor->hall_h.time_cnt;
        motor->hall_h.time_cnt = 0;
    }
    motor->hall_h.it_cnt++;
}

/*
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
*/
void motor_hall_exti_cb(MotorParameter *motor)
{
    hall_update(motor);
    rotate_check(motor);
    rpm_update(motor);
}

/*
void HAL_TIM_PeriodElapsedCallback_OWN(TIM_HandleTypeDef *htim)
*/
void motor_stop_cb(MotorParameter *motor)
{
    motor->hall_h.stop_tick = HAL_GetTick();
    motor->hall_h.it_cnt = 0;
    motor->hall_h.time_cnt = 0;
    motor->rpm_h.feedback.value = 0;
    motor->foc_h.pi_Iq_h.out = 0;
    motor->foc_h.angle_acc = 0.0f;
    PI_reset(&motor->pi_speed);
#ifdef MOTOR_PI_SPEED
    motor->deg_h.duty_h = 0.0f;
#endif
}

static void status_update(MotorParameter *motor)
{
    motor->mode_rot_ref = motor->mode_rot_user;
    motor->rpm_h.reference.reverse = motor->rpm_h.user_set.reverse;
    motor->rpm_h.reference.value = motor->rpm_h.user_set.value;
    bool save_stop =
        (motor->rpm_h.feedback.value < motor->rpm_h.save_stop_val) ? 1 : 0;
    bool ref_fbk_same_dir =
        (motor->rpm_h.user_set.reverse == motor->rpm_h.feedback.reverse) ? 1 : 0;
    switch (motor->dict_state)
    {
        case DIRECT_NORMAL:
        {
            if (ref_fbk_same_dir) break;
            motor->dict_state = DIRECT_SWITCHING;
        }
        case DIRECT_SWITCHING:
        {
            // rpm來不及算到就煞停了
            if (motor->mode_rot_ref == MOTOR_ROT_COAST) break;
            if (save_stop)
            {
                motor->dict_state = DIRECT_NORMAL;
                motor->hall_h.it_cnt = 0;
                motor->rpm_h.feedback.value = 0;
                break;
            }
            motor->mode_rot_ref = MOTOR_ROT_BREAK;
            break;
        }
    }
    // if (HAL_GetTick() - motor->fdcan_alive >= 1000)
    // {
    //     motor_set_rotate_mode(motor, MOTOR_ROT_COAST);
    // }
    motor->pi_speed.feedback = motor->rpm_h.feedback.value;
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
            motor->pi_speed.reference = motor->rpm_h.reference.value;
            PI_run(&motor->pi_speed);
    #ifdef MOTOR_PI_SPEED
            motor->deg_h.duty_h += motor->pi_speed.out;
            VAR_CLAMPF(motor->deg_h.duty_h, 0.0f, 1.0f);
    #else
            motor->deg_h.duty_val = 0.5f;
    #endif
            motor->foc_h.pi_Iq_h.reference += motor->pi_speed.out * motor->tfm_h.duty_Iq;
            VAR_CLAMPF(motor->foc_h.pi_Iq_h.reference, motor->foc_h.pi_Iq_h.min, motor->foc_h.pi_Iq_h.max);
            break;
        }
        case MOTOR_ROT_LOCK:
        {
            motor->mode_rot_ref = MOTOR_ROT_BREAK;
            if (save_stop) motor->mode_rot_ref = MOTOR_ROT_LOCK_FIN;
            break;
        }
    }

    motor->foc_h.pi_Iq_h.reference = (!motor->rpm_h.user_set.reverse) ?
        motor->const_h.rated_current : -motor->const_h.rated_current;
    // motor->tfm_h.duty_Iq = var_clampf((motor->tfm_h.duty_Iq + motor->pi_speed.out), 0.15f, 0.2f);
}

static void foc_run(MotorParameter *motor)
{
    RESULT_CHECK_RET_VOID(motor_vec_ctrl_angle_upd(motor));
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

    if (
        motor->fdcan_enable &&
        motor->tim_it_cnt % 2000 == 0
    ) {
        motor->fdcan_send = 1;
        motor_history_write(motor);
    }
    
    switch (motor->mode_control)
    {
        case MOTOR_CTRL_INIT:
        {
            if (motor->tim_it_cnt >= 19999)
            {
                motor_vec_ctrl_adcs_reset(motor);
                motor_set_rotate_mode(motor, MOTOR_ROT_NORMAL);
                motor_switch_ctrl(motor, MOTOR_CTRL_120);
                motor_set_rpm(motor, 0, 500.0f);
            }
            break;
        }
        case MOTOR_CTRL_TEST_H:
        case MOTOR_CTRL_TEST_L:
        {
        	if (motor->tim_it_cnt == 0)
        		deg_ctrl_test(motor);
            break;
        }
        case MOTOR_CTRL_120:
        {
            if (motor->tim_it_cnt % 1000 == 0)
            {
                status_update(motor);
            }
            if (motor->tim_it_cnt % 2000 == 0)
            {
                if (
                    motor->rpm_h.feedback.value == 0.0f &&
                    motor->rpm_h.reference.value != 0.0f
                ) {
                    hall_update(motor);
                #ifdef MOTOR_AUTO_SPIN
                    motor_switch_ctrl(motor, MOTOR_CTRL_120);
                    motor->hall_h.delay = 1;
                    motor->deg_h.duty_val = 1.0f;
                #endif
                }
            }
            foc_run(motor);
            if (motor->hall_h.delay > 0)
            {
                if (motor->hall_h.delay == HALL_DELAY)
                    deg_ctrl_120_load(motor, HALL_DELAY_LOAD);
                else if (motor->hall_h.delay == 1)
                    deg_ctrl_120_load(motor, motor->hall_h.current);
                // deg_ctrl_120_load(motor, 4);
                motor->hall_h.delay--;
            }
            break;
        }
        case MOTOR_CTRL_FOC_RATED:
        {
        #ifndef MOTOR_FOC_SPIN_DEBUG
            if (motor->tim_it_cnt % 1000 == 0)
            {
                status_update(motor);
            }
            if (motor->tim_it_cnt % 2000 == 0)
            {
                if (
                    motor->rpm_h.feedback.value == 0.0f &&
                    motor->rpm_h.reference.value != 0.0f
                ) {
                    hall_update(motor);
                #ifdef MOTOR_AUTO_SPIN
                    motor_switch_ctrl(motor, MOTOR_CTRL_120);
                    motor->hall_h.delay = 1;
                    motor->deg_h.duty_val = 1.0f;
                #endif
                }
            }
            foc_run(motor);
            motor->duty_load = motor->foc_h.duty_h;
            motor_timer_load(motor);
        #endif
            break;
        }
        default: return;
    }
    motor->tim_it_cnt++;
    if (motor->tim_it_cnt >= 20000) motor->tim_it_cnt = 0;
}

#endif
