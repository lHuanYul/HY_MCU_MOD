#include "HY_MOD/motor/rotor.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/buffer.h"

static const uint8_t angle_hall_to_pu[8] = {UINT8_MAX, 4, 2, 3, 0, 5, 1, UINT8_MAX};

uint8_t motor_rotor_hall_get(MotorParameter *motor)
{
    uint8_t hall =
          (GPIO_READ_R(motor->const_h.Hall_GPIO.u) ? 4U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.v) ? 2U : 0U)
        | (GPIO_READ_R(motor->const_h.Hall_GPIO.w) ? 1U : 0U);
    hall = angle_hall_to_pu[hall];
    return hall;
}

inline void motor_rotor_hall_curr_upd(MotorParameter *motor)
{
    motor->rotor_h.curr = motor_rotor_hall_get(motor);
}

inline void motor_rotor_hall_prev_set(MotorParameter *motor, uint8_t value)
{
    motor->rotor_h.prev = value;
}

void motor_rotor_hall_upd(MotorParameter *motor)
{
    uint8_t hall = motor_rotor_hall_get(motor);
    if (hall == motor->rotor_h.curr) return;
    motor->rotor_h.curr = hall;
}

#define ROTOR_HISTORY_STORE() \
    HISTORY_STORE_SUM( \
        motor->rotor_h.times.datas, \
        motor->rotor_h.times.head, \
        motor->rotor_h.times.len, \
        MOTOR_SPD_CNT, \
        motor->rotor_h.times.sum, \
        time \
    )
void motor_rotor_speed_upd(MotorParameter *motor)
{
    uint32_t time = __HAL_TIM_GET_COMPARE(motor->const_h.Hall_htimx, TIM_CHANNEL_1);
    uint8_t reverse = 0;
    if (motor->rotor_h.curr == motor->rotor_h.prev)
    {
        if (motor->speed_h.fbk_omega < 0) reverse = 1;
    }
    else if (motor->rotor_h.curr == (motor->rotor_h.prev + 1))
    {
        motor->rotor_h.wrong = 0;
        ROTOR_HISTORY_STORE();
    }
    else if (motor->rotor_h.curr == (motor->rotor_h.prev - 1))
    {
        motor->rotor_h.wrong = 0;
        ROTOR_HISTORY_STORE();
        reverse = 1;
    }
    else
    {
        motor->rotor_h.wrong++;
        if (motor->rotor_h.wrong >= 3)
        {
            motor->rotor_h.wrong = 3;
            // omega = 0.0f;
            // motor->foc_h.rad_itpl = 0.0f;
        }

        HISTORY_STORE(motor->dbg_h.hall_wrong, motor->dbg_h.hall_wrong_c,
            20, motor->rotor_h.prev * 10 + motor->rotor_h.curr);
    }
    uint32_t total = motor->rotor_h.times.sum;
    if (total == 0) return;
    float32_t total_i = 1.0f / (float32_t)total;
    float32_t omega =
        motor->rotor_h.times.len * motor->calcu_h.omega_fbk * total_i;
    if (reverse) omega *= -1.0f;
    motor->speed_h.fbk_omega    = omega;
    motor->speed_h.fbk_rpm      = omega * OMEGA_TO_RPM;
    motor->foc_h.rad_itpl       = (omega >= 0.0f ? 1.0f : -1.0f) *
        (motor->rotor_h.times.len * motor->calcu_h.foc_it_angle_itpl * total_i);
}

void motor_rotor_stop(MotorParameter *motor)
{
    motor->rotor_h.stop_tick = HAL_GetTick();
    motor->rotor_h.times = (typeof(motor->rotor_h.times)){0};
    motor->speed_h.fbk_omega = 0.0f;
    motor->speed_h.fbk_rpm = 0.0f;
}

#endif