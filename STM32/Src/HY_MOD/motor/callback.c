#include "HY_MOD/motor/callback.h"
#ifdef HY_MOD_STM32_MOTOR

#include "main/main.h"
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
        motor->rpm_h.fb.reverse = 0;
        motor->hall_h.wrong = 0;
    }
    else if (motor->hall_h.current == hall_seq_clw[motor->hall_h.chk_last])
    {
        motor->rpm_h.fb.reverse = 1;
        motor->hall_h.wrong = 0;
    }
    else
    {
        motor->hall_h.wrong++;
        if (motor->hall_h.wrong >= 6)
        {
            motor->hall_h.wrong = 6;
            motor->rpm_h.fb.reverse = 0;
            motor->rpm_h.fb.value = 0.0f;
            motor->foc_h.angle_itpl = 0.0f;
        }
    }
    motor->hall_h.chk_last = motor->hall_h.current;
}

static void rpm_update(MotorParameter *motor)
{
    motor->hall_h.time_cnt +=
        __HAL_TIM_GET_COMPARE(motor->const_h.Hall_htimx, TIM_CHANNEL_1);
    motor->hall_h.it_cnt++;
    if (motor->hall_h.it_cnt >= MOTOR_RPM_CNT)
    {
        motor->hall_h.it_cnt = 0;
        motor->rpm_h.fb.value =
            motor->tfm_h.rpm_fbk / (float32_t)motor->hall_h.time_cnt;
        motor->foc_h.angle_itpl = (!motor->rpm_h.fb.reverse) ?
             motor->tfm_h.foc_it_angle_itpl / (float32_t)motor->hall_h.time_cnt :
            -motor->tfm_h.foc_it_angle_itpl / (float32_t)motor->hall_h.time_cnt;
        motor->hall_h.time_cnt = 0;
    }
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
    motor->rpm_h.fb.value = 0;
    motor->foc_h.pi_Iq_h.out_fix = 0;
    motor->foc_h.angle_acc = 0.0f;
    PI_reset(&motor->deg_h.pi_rpm);
    PI_reset(&motor->foc_h.pi_rpm);
}

static void direction_update(MotorParameter *motor)
{
    motor->rotate_h.ref_fix = motor->rotate_h.ref_ori;
    motor->rpm_h.ref_fix.reverse = motor->rpm_h.ref_ori.reverse;
    motor->rpm_h.ref_fix.value = motor->rpm_h.ref_ori.value;
    bool ref_fbk_same_dir =
        (motor->rpm_h.ref_ori.reverse == motor->rpm_h.fb.reverse) ? 1 : 0;
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
            if (motor->rotate_h.ref_fix == MOTOR_ROT_COAST) break;
            if (motor->rpm_h.fb.value < motor->rpm_h.save_stop_val)
            {
                motor->dict_state = DIRECT_NORMAL;
                motor->hall_h.it_cnt = 0;
                motor->rpm_h.fb.value = 0;
                break;
            }
            motor->rotate_h.ref_fix = MOTOR_ROT_BREAK;
            break;
        }
    }
}

static void status_update(MotorParameter *motor)
{
    // if (HAL_GetTick() - motor->fdcan_alive >= 1000)
    // {
    //     motor_set_rotate_mode(motor, MOTOR_ROT_COAST);
    // }
    if (motor->ctrl_h.ref_fix == MOTOR_CTRL_INIT) return;
    switch (motor->rotate_h.ref_fix)
    {
        case MOTOR_ROT_COAST:
        case MOTOR_ROT_BREAK:
        case MOTOR_ROT_LOCK_FIN:
        {
            motor->deg_h.duty_val = 0.5f;
            motor_switch_ctrl_fix(motor, MOTOR_CTRL_120);
            break;
        }
        case MOTOR_ROT_LOCK:
        {
            motor->rotate_h.ref_fix = MOTOR_ROT_BREAK;
            if (motor->rpm_h.fb.value < motor->rpm_h.save_stop_val)
                motor->rotate_h.ref_fix = MOTOR_ROT_LOCK_FIN;
            motor_switch_ctrl_fix(motor, MOTOR_CTRL_120);
            break;
        }
        case MOTOR_ROT_NORMAL:
        {
            motor->deg_h.pi_rpm.reference = motor->rpm_h.ref_fix.value;
            motor->deg_h.pi_rpm.feedback = motor->rpm_h.fb.value;
            motor->foc_h.pi_rpm.reference = motor->rpm_h.ref_fix.value;
            motor->foc_h.pi_rpm.feedback = motor->rpm_h.fb.value;
            PI_run(&motor->deg_h.pi_rpm);
            PI_run(&motor->foc_h.pi_rpm);
    #ifdef MOTOR_PI_RPM
            motor->deg_h.duty_val = motor->deg_h.pi_rpm.out_fix;
            // motor->foc_h.pi_Iq_h.reference += motor->foc_h.pi_rpm.out_fix * motor->tfm_h.duty_Iq;
            // VAR_CLAMPF(motor->foc_h.pi_Iq_h.reference, motor->foc_h.pi_Iq_h.min, motor->foc_h.pi_Iq_h.max);
            // motor->foc_h.pi_Iq_h.reference = (!motor->rpm_h.ref_ori.reverse) ?
            //     motor->const_h.rated_current : -motor->const_h.rated_current;
            // motor->tfm_h.duty_Iq = var_clampf((motor->tfm_h.duty_Iq + motor->foc_h.pi_rpm.out_fix), 0.15f, 0.2f);
    #else
            motor->deg_h.duty_val = 0.5f;
    #endif
            // Auto start spin
            if (
                motor->rpm_h.fb.value == 0.0f &&
                motor->rpm_h.ref_fix.value != 0.0f
            ) {
                hall_update(motor);
            #ifdef MOTOR_AUTO_SPIN
                motor_switch_ctrl_fix(motor, MOTOR_CTRL_120);
                motor->hall_h.delay = 1;
                motor->deg_h.duty_val = 1.0f;
            #endif
            }
            motor_switch_ctrl_fix(motor, motor->ctrl_h.ref_ori);
            break;
        }
    }
}

static void control_update(MotorParameter *motor)
{
    switch (motor->ctrl_h.ref_fix)
    {
        case MOTOR_CTRL_INIT:
        {
            if (motor->init_cnt > 0)
            {
                motor->init_cnt--;
                if (motor->init_cnt == 0)
                {
                    motor_vec_ctrl_adcs_reset(motor);
                    motor_set_rotate_mode(motor, MOTOR_ROT_NORMAL);
                    motor_switch_ctrl(motor, MOTOR_CTRL_FOC_RATED);
                    motor_set_rpm(motor, 0, 50.0f);
                }
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
            motor_foc_run(motor);
            if (motor->hall_h.delay > 0)
            {
                if (motor->hall_h.delay == HALL_DELAY)
                    deg_ctrl_120_load(motor, HALL_DELAY_LOAD);
                motor->hall_h.delay--;
                if (motor->hall_h.delay == 0)
                    deg_ctrl_120_load(motor, motor->hall_h.current);
            }
            break;
        }
        case MOTOR_CTRL_FOC_RATED:
        {
        #ifndef MOTOR_FOC_SPIN_DEBUG
            motor_foc_run(motor);
            motor->duty_load = motor->foc_h.duty_h;
            motor_timer_load(motor);
        #endif
            break;
        }
        default: return;
    }
}

/* 20kHz
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
*/
#define PWM_TIM_IT_CNT_MAX 20000
void motor_pwm_cb(MotorParameter *motor)
{
    motor_vec_ctrl_adcs_upd(motor);
    if (motor->tim_it_cnt % 200 == 0)
    {
        if (motor->fdcan_enable)
        {
            // motor_history_write(motor);
            fdcan_motor_send(motor, &fdcan_pkt_pool, &fdcan_trsm_pkt_buf);
            motor->fdcan_tick++;
        }
        direction_update(motor);
        status_update(motor);
    }
    control_update(motor);

    motor->tim_it_cnt++;
    if (motor->tim_it_cnt >= PWM_TIM_IT_CNT_MAX) motor->tim_it_cnt = 0;
}

#endif
