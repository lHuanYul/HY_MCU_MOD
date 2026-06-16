#include "HY_MOD/motor/rotor.h"
#ifdef HY_MOD_STM32_MOTOR

static const uint8_t angle_hall_to_pu[8] = {UINT8_MAX, 4, 2, 3, 0, 5, 1, UINT8_MAX};
void motor_rotor_hall_upd(MotorParameter *motor)
{
    uint8_t hall =
          (GPIO_READ_R(motor->const_h.Hall_GPIO.u) ? 4U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.v) ? 2U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.w) ? 1U : 0U);
    motor->rotor_h.current = angle_hall_to_pu[hall];
    motor->rotor_h.hall_current = (hall == 0 || hall == 7) ? UINT8_MAX : hall; //rm
}

void motor_rotor_speed_upd(MotorParameter *motor)
{
    motor->rotor_h.time_hist[motor->rotor_h.time_hist_head++] =
        __HAL_TIM_GET_COMPARE(motor->const_h.Hall_htimx, TIM_CHANNEL_1);
    motor->rotor_h.time_hist_head %= MOTOR_SPD_CNT;
    if (motor->rotor_h.time_hist_len < MOTOR_SPD_CNT)
        motor->rotor_h.time_hist_len++;
    
    uint8_t i;
    uint32_t total = 0;
    for (i = 0; i < motor->rotor_h.time_hist_len; i++)
    {
        total += motor->rotor_h.time_hist[i];
    }
    float32_t omega =
        motor->rotor_h.time_hist_len * motor->calcu_h.omega_fbk / (float32_t)total;
    if      (motor->rotor_h.current == (motor->rotor_h.last + 1))
    {
        motor->rotor_h.wrong = 0;
    }
    else if (motor->rotor_h.current == (motor->rotor_h.last - 1))
    {
        omega = -omega;
        motor->rotor_h.wrong = 0;
    }
    else
    {
        motor->rotor_h.wrong++;
        motor->dbg_h.hall_wrong[motor->dbg_h.hall_wrong_c++] =
            motor->rotor_h.last * 10 + motor->rotor_h.current;
        if (motor->dbg_h.hall_wrong_c >= 20) motor->dbg_h.hall_wrong_c = 0;
        if (motor->rotor_h.wrong >= 3)
        {
            motor->rotor_h.wrong = 3;
            // omega = 0.0f;
            // motor->foc_h.rad_itpl = 0.0f;
        }
    }
    // if      (motor->rotor_h.hall_current == motor_hall_seq_ccw[motor->rotor_h.hall_last])
    // {
    //     motor->rotor_h.wrong = 0;
    // }
    // else if (motor->rotor_h.hall_current == motor_hall_seq_clw[motor->rotor_h.hall_last])
    // {
    //     omega = -omega;
    //     motor->rotor_h.wrong = 0;
    // }
    // else
    // {
    //     motor->rotor_h.wrong++;
    //     motor->dbg_h.hall_wrong[motor->dbg_h.hall_wrong_c++] =
    //         motor->rotor_h.hall_last * 10 + motor->rotor_h.hall_current;
    //     if (motor->dbg_h.hall_wrong_c >= 20) motor->dbg_h.hall_wrong_c = 0;
    //     if (motor->rotor_h.wrong >= 3)
    //     {
    //         motor->rotor_h.wrong = 3;
    //         // omega = 0.0f;
    //         // motor->foc_h.rad_itpl = 0.0f;
    //     }
    // }
    motor->speed_h.fbk_omega = omega;
    motor->speed_h.fbk_rpm = omega * OMEGA_TO_RPM;

    motor->foc_h.rad_itpl = (motor->speed_h.fbk_omega >= 0.0f) ?
         motor->rotor_h.time_hist_len * motor->calcu_h.foc_it_angle_itpl / (float32_t)total :
        -motor->rotor_h.time_hist_len * motor->calcu_h.foc_it_angle_itpl / (float32_t)total;
}

#endif