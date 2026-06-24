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
}

#define HISTORY_STORE(store, pos, total_l, value)\
    do {\
        store[pos] = value;\
        if (++pos >= total_l) pos = 0;\
    } while (0)

void motor_rotor_speed_upd(MotorParameter *motor)
{
    uint32_t time = __HAL_TIM_GET_COMPARE(motor->const_h.Hall_htimx, TIM_CHANNEL_1);

    HISTORY_STORE(motor->rotor_h.time_hist, motor->rotor_h.time_hist_head, MOTOR_SPD_CNT, time);
    if (motor->rotor_h.time_hist_len < MOTOR_SPD_CNT)
        motor->rotor_h.time_hist_len++;

    uint8_t reverse = 0;
    if (1)
    // if (motor->rotor_h.current != motor->rotor_h.last)
    {
        if      (motor->rotor_h.current == (motor->rotor_h.last + 1))
        {
            motor->rotor_h.wrong = 0;
            motor->rotor_h.time_hist[motor->rotor_h.current] = time;
        }
        else if (motor->rotor_h.current == (motor->rotor_h.last - 1))
        {
            motor->rotor_h.wrong = 0;
            motor->rotor_h.time_hist[motor->rotor_h.current] = time;
            reverse = 1;
        }
        else
        {
            motor->rotor_h.wrong++;

            if (motor->rotor_h.current != motor->rotor_h.last)
            {
                HISTORY_STORE(motor->dbg_h.hall_wrong, motor->dbg_h.hall_wrong_c,
                    20, motor->rotor_h.last * 10 + motor->rotor_h.current);
            }
            else
            {
                HISTORY_STORE(motor->dbg_h.hall_s_wrong, motor->dbg_h.hall_s_wrong_c,
                    20, motor->rotor_h.last * 10 + motor->rotor_h.current);
            }

            if (motor->rotor_h.wrong >= 3)
            {
                motor->rotor_h.wrong = 3;
                // omega = 0.0f;
                // motor->foc_h.rad_itpl = 0.0f;
            }
        }
    }
    else
    {
    }
    uint8_t i, c = 0;
    uint32_t total = 0;
    for (i = 0; i < 6; i++)
    {
        if (motor->rotor_h.time_hist[i] != 0)
        {
            c++;
            total += motor->rotor_h.time_hist[i];
        }
    }
    float32_t omega =
        c * motor->calcu_h.omega_fbk / (float32_t)total;
    if (reverse) omega *= -1.0f;

    motor->speed_h.fbk_omega = omega;
    motor->speed_h.fbk_rpm = omega * OMEGA_TO_RPM;

    motor->foc_h.rad_itpl = (motor->speed_h.fbk_omega >= 0.0f) ?
         motor->rotor_h.time_hist_len * motor->calcu_h.foc_it_angle_itpl / (float32_t)total :
        -motor->rotor_h.time_hist_len * motor->calcu_h.foc_it_angle_itpl / (float32_t)total;
}

void motor_rotor_stop(MotorParameter *motor)
{
    uint8_t i;
    for (i = 0; i < 6 ; i++)
    {
        motor->rotor_h.time_hist[i] = 0;
    }
}

#endif