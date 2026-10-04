#include "HY_MOD/motor/callback.h"
#ifdef HY_MOD_STM32_MOTOR

#include "main/main.h"
#include "HY_MOD/motor/rotor.h"
#include "HY_MOD/motor/ctrl_deg.h"
#include "HY_MOD/motor/ctrl_foc.h"
#include "HY_MOD/fdcan/pkt_write.h"

__weak void motor_start_spin(MotorParameter *motor)
{
    motor_set_rotor_mode(motor, MOTOR_SENSOR_SIMULATE);
    motor_set_ctrl_mode(motor, MOTOR_CTRL_120_SIMULATE);
    motor_set_rotate_mode(motor, MOTOR_ROTATE_NORMAL);
    motor_set_speed(motor, -1.0f);
}

/**
 * void HAL_TIM_PeriodElapsedCallback_OWN(TIM_HandleTypeDef *htim)
 */
void motor_stop_cb(MotorParameter *motor)
{
    motor_rotor_stop_cbi(motor);
    motor_deg_stop_cbi(motor);
    motor_foc_stop_cbi(motor);
}

/**
 * void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
 */
void motor_hall_timer_cb(MotorParameter *motor)
{
    motor_rotor_hall_timer_cbi(motor);
    motor_foc_hall_timer_cbi(motor);
}

static inline void rotate_status_upd(MotorParameter *motor)
{
    // if (HAL_GetTick() - motor->fdcan_alive >= 1000)
    // {
    //     motor_set_rotate_mode(motor, MOTOR_ROTATE_COAST);
    // }

    if (motor->ctrl.ref_sys == MOTOR_CTRL_INIT) return;
    motor->rotate.ref_sys = motor->rotate.ref_user;
    switch (motor->rotate.ref_sys)
    {
    	case MOTOR_ROTATE_UNINIT: return;
        case MOTOR_ROTATE_COAST:
        case MOTOR_ROTATE_BREAK:
        case MOTOR_ROTATE_LOCK_FIN:
        {
            motor->deg_h.duty_val = 0.5f;
            motor_switch_ctrl_system(motor, MOTOR_CTRL_120_NORMAL);
            break;
        }
        case MOTOR_ROTATE_LOCK:
        {
            motor->rotate.ref_sys = MOTOR_ROTATE_BREAK;
            if (motor->speed.fbk_omega < motor->speed.save_stop_omega)
                motor->rotate.ref_sys = MOTOR_ROTATE_LOCK_FIN;
            motor_switch_ctrl_system(motor, MOTOR_CTRL_120_NORMAL);
            break;
        }
        case MOTOR_ROTATE_NORMAL:
        {
            motor->deg_h.pi_omega.reference = motor->speed.ref_omega;
            motor->deg_h.pi_omega.feedback = motor->speed.fbk_omega;
            motor->foc_h.pi_omega.reference = motor->speed.ref_omega;
            motor->foc_h.pi_omega.feedback = motor->speed.fbk_omega;
            PI_run(&motor->deg_h.pi_omega);
            PI_run(&motor->foc_h.pi_omega);
            switch (motor->ctrl.ref_sys)
            {
                case MOTOR_CTRL_120_SIMULATE:
                case MOTOR_CTRL_120_DUTY:
                {
                    VAR_CLAMPF_STATIC(
                        motor->deg_h.duty_val,
                        fabsf(motor->speed.ref_rpm),
                        0.0f, 1.0f
                    );
                    break;
                }
                default:
                {
                    motor->deg_h.duty_val = motor->deg_h.pi_omega.out_fix;
                    break;
                }
            }
            motor_switch_ctrl_system(motor, motor->ctrl.ref_user);
            break;
        }
    }
}

/**
 * 20kHz
 * void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
 */
#define PWM_TIM_IT_CNT_MAX  200000 // 10s
#define ROTATE_SIM_SPEED    10000
void motor_pwm_cb(MotorParameter *motor)
{
    motor_adcs_upd(motor);
    motor_rotor_pwm_cbi(motor);
    if (motor->tim_tick % 200 == 0) rotate_status_upd(motor);
    if (motor->tim_tick % 1000 == 0) fdcan_h.motor_rpm_en = 1;

    switch (motor->ctrl.ref_sys)
    {
    	case MOTOR_CTRL_UNINIT: break;
        case MOTOR_CTRL_INIT:
        {
            motor->init_cnt++;
            if (motor->init_cnt >= 20000)
            {
                motor->init_cnt = 0;
                motor_adcs_init(motor);
                motor_start_spin(motor);
            }
            break;
        }
        case MOTOR_CTRL_TEST_HIGH:
        case MOTOR_CTRL_TEST_LOW:
        case MOTOR_CTRL_TEST_WAVE:
        {
            motor_deg_test(motor);
            break;
        }
        case MOTOR_CTRL_120_NORMAL:
        case MOTOR_CTRL_120_DUTY:
        {
            if (motor->tim_tick % 200 == 0) motor_deg_check_reverse(motor);
            motor_deg_120_load(motor, motor->rotor.curr);
            break;
        }
        case MOTOR_CTRL_120_DIREC_SW:
        {
            if (motor->tim_tick % 200 == 0) motor_deg_proc_safe_reverse(motor);
            motor_deg_120_load(motor, motor->rotor.curr);
            break;
        }
        case MOTOR_CTRL_120_SIMULATE:
        {
            if (motor->tim_tick % 200 == 0) motor_deg_check_reverse(motor);
            motor_deg_120_load(motor, motor->rotor.curr);
            if (motor->tim_tick % ROTATE_SIM_SPEED == 0)
            {
                if (!motor->deg_h.reverse)
                    motor->rotor.virtual = (motor->rotor.virtual + 1) % 6;
                else
                    motor->rotor.virtual = (motor->rotor.virtual + 5) % 6;
                motor_rotor_phase_trigger(motor);
            }
            break;
        }
        case MOTOR_CTRL_FOC_INIT:
        {
            motor_foc_run(motor);
            motor_deg_120_load(motor, motor->rotor.curr);
            // Todo
            motor->foc_h.init_cnt++;
            if (motor->foc_h.init_cnt >= 20000)
            {
                motor->foc_h.init_cnt = 0;
                motor_foc_reset(motor);
            }
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
            motor_foc_run(motor);
            motor_foc_load(motor);
            break;
        }
    }
    if (++motor->tim_tick >= PWM_TIM_IT_CNT_MAX) motor->tim_tick = 0;
}

#endif
