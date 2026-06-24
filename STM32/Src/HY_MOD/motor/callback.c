#include "HY_MOD/motor/callback.h"
#ifdef HY_MOD_STM32_MOTOR

#include "main/main.h"
#include "HY_MOD/motor/main.h"
#include "HY_MOD/motor/rotor.h"
#include "HY_MOD/motor/ctrl_deg.h"
#include "HY_MOD/motor/ctrl_foc.h"
#include "HY_MOD/motor/trigonometric.h"
#include "HY_MOD/fdcan/pkt_write.h"

/*
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
*/
void motor_hall_exti_cb(MotorParameter *motor)
{
    motor->rotor_h.last = motor->rotor_h.current;
    motor_rotor_hall_upd(motor);
    motor_foc_hall_exti_cb(motor);
    motor_rotor_speed_upd(motor);
}

/*
void HAL_TIM_PeriodElapsedCallback_OWN(TIM_HandleTypeDef *htim)
*/
void inline motor_stop_cb(MotorParameter *motor)
{
    // motor_rotor_hall_upd(motor);
    motor->rotor_h.stop_tick = HAL_GetTick();
    motor->rotor_h.time_hist_len = 0;
    motor->rotor_h.time_hist_head = 0;
    motor->speed_h.fbk_omega = 0.0f;
    motor->speed_h.fbk_rpm = 0.0f;
    PI_reset(&motor->deg_h.pi_omega);
    motor->foc_h.rad_itpl = 0.0f;
    motor->foc_h.rad_acc  = 0.0f;
    PI_reset(&motor->foc_h.pi_omega);
    PI_reset(&motor->foc_h.pi_Id_h);
    PI_reset(&motor->foc_h.pi_Iq_h);
}

static inline void direction_update(MotorParameter *motor)
{
    motor->rotate_h.ref_fix = motor->rotate_h.ref_ori;
    switch (motor->ctrl_h.ref_fix)
    {
        case MOTOR_CTRL_120:
        case MOTOR_CTRL_120_T:
        {
            if (
                motor->speed_h.ref_omega == 0.0f ||
                var_f32_same_sign(motor->speed_h.ref_omega, motor->speed_h.fbk_omega)
            ) break;
            motor_switch_ctrl_fix(motor, MOTOR_CTRL_120_SW);
        }
        case MOTOR_CTRL_120_SW:
        {
            if (motor->speed_h.fbk_omega < motor->speed_h.save_stop_omega)
            {
                if (motor->speed_h.ref_omega >= 0.0f)
                    motor->deg_h.reverse = 0;
                else
                    motor->deg_h.reverse = 1;
                motor->rotor_h.time_hist_len = 0;
                motor->speed_h.fbk_omega = 0;
                motor_switch_ctrl_fix(motor, MOTOR_CTRL_120);
                break;
            }
            motor->rotate_h.ref_fix = MOTOR_ROT_COAST;
            break;
        }
        default: break;
    }
}

static inline void status_update(MotorParameter *motor)
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
            if (motor->speed_h.fbk_omega < motor->speed_h.save_stop_omega)
                motor->rotate_h.ref_fix = MOTOR_ROT_LOCK_FIN;
            motor_switch_ctrl_fix(motor, MOTOR_CTRL_120);
            break;
        }
        case MOTOR_ROT_NORMAL:
        {
            motor->deg_h.pi_omega.reference = motor->speed_h.ref_omega;
            motor->deg_h.pi_omega.feedback = motor->speed_h.fbk_omega;
            motor->foc_h.pi_omega.reference = motor->speed_h.ref_omega;
            motor->foc_h.pi_omega.feedback = motor->speed_h.fbk_omega;
            PI_run(&motor->deg_h.pi_omega);
            PI_run(&motor->foc_h.pi_omega);
            switch (motor->ctrl_h.ref_fix)
            {
                case MOTOR_CTRL_120_DUTY:
                case MOTOR_CTRL_120_SIM:
                {
                    VAR_CLAMPF_STATIC(motor->deg_h.duty_val, motor->speed_h.ref_rpm, 0.0f, 1.0f);
                    break;
                }
                default:
                {
                    motor->deg_h.duty_val = motor->deg_h.pi_omega.out_fix;
                    break;
                }
            }
            motor_switch_ctrl_fix(motor, motor->ctrl_h.ref_ori);
            break;
        }
    }
}

__weak void motor_start_spin(MotorParameter *motor)
{
    motor_set_spd(motor, 0.0f);
    motor_set_rotate_mode(motor, MOTOR_ROT_NORMAL);
    motor_switch_ctrl(motor, MOTOR_CTRL_TEST_LOW);
}

static inline void control_update(MotorParameter *motor)
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
                    motor_adcs_reset(motor);
                    motor_start_spin(motor);
                }
            }
            break;
        }
        case MOTOR_CTRL_TEST_HIGH:
        case MOTOR_CTRL_TEST_LOW:
        {
            if (motor->tim_tick % 1000 == 0)
                motor_deg_test_HL(motor);
            break;
        }
        case MOTOR_CTRL_TEST_WAVE:
        {
            if (motor->tim_tick % 1000 == 0)
                motor_deg_test_WAVE(motor);
            break;
        }
        case MOTOR_CTRL_120:
        case MOTOR_CTRL_120_T:
        case MOTOR_CTRL_120_DUTY:
        case MOTOR_CTRL_120_SW:
        {
            motor_deg_120_load(motor, motor->rotor_h.current);
            break;
        }
        case MOTOR_CTRL_120_SIM:
        {
            if (motor->tim_tick % 20000 == 0)
            // if (motor_h.rotor_h.vir_tri)
            {
                motor_h.rotor_h.vir_tri = 0;
                motor->rotor_h.virtual = (motor->rotor_h.virtual + 1) % 6;
                motor_deg_120_load(motor, motor->rotor_h.virtual);
            }
            break;
        }
        case MOTOR_CTRL_FOC_INIT:
        {
            motor_foc_run(motor);
            motor_deg_120_load(motor, motor->rotor_h.current);
            break;
        }
        case MOTOR_CTRL_FOC:
        case MOTOR_CTRL_FOC_SIM:
        case MOTOR_CTRL_FOC_POS:
        case MOTOR_CTRL_FOC_ROT_CMD:
        case MOTOR_CTRL_FOC_ROT_AUTO:
        case MOTOR_CTRL_FOC_OL_VDQ:
        case MOTOR_CTRL_FOC_OL_IQ:
        {
            motor_foc_run(motor);
            motor_foc_load(motor);
            break;
        }
    }
}

/* 20kHz
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
*/
#define PWM_TIM_IT_CNT_MAX 200000
void motor_pwm_cb(MotorParameter *motor)
{
    motor_adcs_upd(motor);
    // motor_rotor_hall_upd(motor);
    if (motor->tim_tick % 200 == 0)
    {
        direction_update(motor);
        status_update(motor);
    }
    if (motor->tim_tick % 1000 == 0)
    {
        fdcan_h.motor_rpm_en = 1;
    }
    control_update(motor);
    // if (motor->rotor_h.updated)
    // {
    //     motor->dbg_h.hall_rad[motor->rotor_h.current] = motor->foc_h.rotor_rad;
    // }
    if (++motor->tim_tick >= PWM_TIM_IT_CNT_MAX) motor->tim_tick = 0;
}

#endif
