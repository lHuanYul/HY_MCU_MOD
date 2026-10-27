#include "HY_MOD/motor/callback.h"
#ifdef HY_MOD_STM32_MOTOR

#include "main/_main.h"
#include "HY_MOD/motor/rotor.h"
#include "HY_MOD/motor/ctrl.h"
#include "HY_MOD/motor/main.h"
#include "HY_MOD/fdcan/pkt_write.h"

__weak void motor_start_spin(MotorParameter *motor)
{
    motor_set_rotor_mode(motor, MOTOR_SENSOR_SIMULATE);
    motor_set_ctrl_mode(motor, MOTOR_CTRL_120_I_F_SAME);
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
            motor->deg_h.load_duty_val = 0.5f;
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
                case MOTOR_CTRL_120_I_F_SAME:
                case MOTOR_CTRL_120_DUTY:
                {
                    VAR_CLAMPF_STATIC(
                        motor->deg_h.load_duty_val,
                        fabsf(motor->speed.ref_rpm),
                        0.0f, 1.0f
                    );
                    break;
                }
                default:
                {
                    motor->deg_h.load_duty_val = motor->deg_h.pi_omega.out_fix;
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
#define ROTATE_I_F_SAME_T 10000
void motor_pwm_cb(MotorParameter *motor)
{
    // ADC 取值
    motor_adcs_I_inject(motor);
    motor_adcs_V_inject(motor);
    // 轉子
    motor_rotor_pwm_cbi(motor);
    // ADC 運算更新
    motor_adcs_I_calculate(motor);

    uint32_t *t = &motor->tim_tick;
    MotorRotorParam *r = &motor->rotor;

    // 前幾周期消磁遮蔽
    if (motor->zcp.comm_ticks >= DEMAG_BLANKING_TICKS)
    {
        motor_adcs_V_calculate(motor);
    }

    if ((*t) % 200 == 0) rotate_status_upd(motor);
    if ((*t) % 1000 == 0) fdcan_h.motor_rpm_en = 1;

    switch (motor->ctrl.ref_sys)
    {
    	case MOTOR_CTRL_UNINIT: break;
        case MOTOR_CTRL_INIT:
        {
            motor->tim_tick_init++;
            if (motor->tim_tick_init >= 20000)
            {
                motor->tim_tick_init = 0;
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
        case MOTOR_CTRL_120_I_F_SAME:
        {
            if ((*t) % 200 == 0) motor_deg_check_reverse(motor);
            motor_deg_120_load(motor, r->curr);

            r->I_F_trigger_T = ROTATE_I_F_SAME_T;
            if ((*t) % r->I_F_trigger_T == 0)
            {
                r->virtual = motor_deg_get_next_phase(motor, r->virtual);
                motor_rotor_hall_tim_tri(motor);
            }
            break;
        }
        case MOTOR_CTRL_120_I_F_ACCE:
        {
            if ((*t) % 200 == 0) motor_deg_check_reverse(motor);
            motor_deg_120_load(motor, r->curr);

            if ((*t) % r->I_F_trigger_T == 0)
            {
                r->virtual = motor_deg_get_next_phase(motor, r->virtual);
                // r->I_F_trigger_T = ;
                motor_rotor_hall_tim_tri(motor);
            }
            break;
        }
        case MOTOR_CTRL_120_NORMAL:
        case MOTOR_CTRL_120_DUTY:
        {
            if ((*t) % 200 == 0) motor_deg_check_reverse(motor);
            motor_deg_120_load(motor, r->curr);
            break;
        }
        case MOTOR_CTRL_120_DIREC_SW:
        {
            if ((*t) % 200 == 0) motor_deg_proc_safe_reverse(motor);
            motor_deg_120_load(motor, r->curr);
            break;
        }
        case MOTOR_CTRL_FOC_I_F_ACCE:
        {
            motor_foc_run(motor);
            motor_deg_120_load(motor, r->curr);
            // Todo
            motor->foc_h.tim_tick_init++;
            if (motor->foc_h.tim_tick_init >= 20000)
            {
                motor->foc_h.tim_tick_init = 0;
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
    if (++(*t) >= PWM_TIM_IT_CNT_MAX) (*t) = 0;
}

#endif
