#include "HY_MOD/motor/ctrl_deg.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/motor/main.h"

#define HIGH_PASS   1
#define NONE_PASS   0
#define LOW__PASS  -1
static const int8_t seq_map_120[][3] = {
    { HIGH_PASS, NONE_PASS, LOW__PASS }, // 0-4
    { NONE_PASS, HIGH_PASS, LOW__PASS }, // 1-6
    { LOW__PASS, HIGH_PASS, NONE_PASS }, // 2-2
    { LOW__PASS, NONE_PASS, HIGH_PASS }, // 3-3
    { NONE_PASS, LOW__PASS, HIGH_PASS }, // 4-1
    { HIGH_PASS, LOW__PASS, NONE_PASS }, // 5-5
    { HIGH_PASS, HIGH_PASS, HIGH_PASS }, // 6
    { LOW__PASS, LOW__PASS, LOW__PASS }, // 7
    { NONE_PASS, NONE_PASS, NONE_PASS }, // 8
};
static const uint8_t index_120_ccw[] = {7, 4, 2, 3, 0, 5, 1, 7};
static const uint8_t index_120_cw[]  = {7, 1, 5, 0, 3, 2, 4, 7};

static void ctrl_load(MotorParameter *motor, int8_t seq[3], float32_t duty)
{
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        __HAL_TIM_SET_COMPARE(motor->const_h.PWM_htimx, motor->const_h.PWM_TIM_CHANNEL_x.uvw[i], 0);
    }
    for (i = 0; i < 3; i++)
    {
        if (seq[i] == HIGH_PASS)
        {
            motor->duty_deg.uvw[i] = duty;
            GPIO_WRITE_R(motor->const_h.PWMN_GPIO.uvw[i], 0);
        }
        else if (seq[i] == LOW__PASS)
        {
            motor->duty_deg.uvw[i] = 0.0f;
            GPIO_WRITE_R(motor->const_h.PWMN_GPIO.uvw[i], 1);
        }
        else
        {
            motor->duty_deg.uvw[i] = 0.0f;
            GPIO_WRITE_R(motor->const_h.PWMN_GPIO.uvw[i], 0);
        }
    }
    motor_timer_load(motor);
}

void deg_ctrl_test(MotorParameter *motor)
{
    uint8_t i;
    int8_t seq[3] = {0};
    if (motor->mode_control == MOTOR_CTRL_TEST_H)
        for (i = 0; i < 3; i++) seq[i] = seq_map_120[6][i];
    else
        for (i = 0; i < 3; i++) seq[i] = seq_map_120[7][i];
    motor->duty_load = motor->duty_deg;
    ctrl_load(motor, seq, 1.0f);
}

void deg_ctrl_120_load(MotorParameter *motor, uint8_t id)
{
    uint8_t i;
    int8_t seq[3] = {0};
    switch (motor->mode_rot_ref)
    {
        case MOTOR_ROT_COAST:
        {
            motor->deg_duty = 1.0f;
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[8][i];
            break;
        }
        case MOTOR_ROT_BREAK:
        case MOTOR_ROT_LOCK:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[7][i];
            break;
        }
        case MOTOR_ROT_NORMAL:
        {
            for (i = 0; i < 3; i++)
            {
                if (!motor->rpm_reference.reverse)
                    seq[i] = seq_map_120[index_120_ccw[id]][i];
                else
                    seq[i] = seq_map_120[ index_120_cw[id]][i];
            }
            break;
        }
        case MOTOR_ROT_LOCK_FIN:
        {
            motor->deg_duty = 0.2f;
            // !
            for (i = 0; i < 3; i++)
                seq[i] = seq_map_120[index_120_ccw[id]][i];
            break;
        }
    }
    motor->duty_load = motor->duty_deg;
    ctrl_load(motor, seq, motor->deg_duty);
}

// static const int8_t seq_map_180[][3] = {
//     { HIGH_PASS, LOW__PASS,  HIGH_PASS }, // 0-4
//     { HIGH_PASS, LOW__PASS,  LOW__PASS  }, // 1-6
//     { HIGH_PASS, HIGH_PASS, LOW__PASS  }, // 2-2
//     { LOW__PASS,  HIGH_PASS, LOW__PASS  }, // 3-3
//     { LOW__PASS,  HIGH_PASS, HIGH_PASS }, // 4-1
//     { LOW__PASS,  LOW__PASS,  HIGH_PASS }, // 5-5
//     { HIGH_PASS, HIGH_PASS, HIGH_PASS }, // 6
//     { LOW__PASS,  LOW__PASS,  LOW__PASS  }, // 7
// };
// static const uint8_t index_180_lock[] = {7, 4, 2, 3, 0, 5, 1, 7};
// static const uint8_t index_180_ccw[]  = {7, 0, 4, 5, 2, 1, 3, 7};
// static const uint8_t index_180_cw[]   = {7, 2, 0, 1, 4, 3, 5, 7};
// void deg_ctrl_180_load(MotorParameter *motor)
// {
//     if (motor->hall_current == UINT8_MAX) return;
//     uint8_t i;
//     int8_t seq[3] = {0};
//     switch (motor->mode_rot_ref)
//     {
//         case MOTOR_ROT_COAST:
//         {
//             motor->deg_duty = 1.0f;
//             for (i = 0; i < 3; i++) seq[i] = seq_map_180[6][i];
//             break;
//         }
//         case MOTOR_ROT_BREAK:
//         case MOTOR_ROT_LOCK:
//         {
//             for (i = 0; i < 3; i++) seq[i] = seq_map_180[7][i];
//             break;
//         }
//         case MOTOR_ROT_NORMAL:
//         {
//             for (i = 0; i < 3; i++)
//             {
//                 if (!motor->rpm_reference.reverse)
//                     seq[i] = seq_map_180[index_180_ccw[motor->hall_current]][i];
//                 else
//                     seq[i] = seq_map_180[ index_180_cw[motor->hall_current]][i];
//             }
//             break;
//         }
//         case MOTOR_ROT_LOCK_FIN:
//         {
//             motor->deg_duty = 0.2f;
//             for (i = 0; i < 3; i++)
//                 seq[i] = seq_map_180[index_180_lock[motor->hall_current]][i];
//             break;
//         }
//     }
//     for (i = 0; i < 3; i++)
//     {
//         if (seq[i] == HIGH_PASS)
//         {
//             motor->duty_deg.uvw[i] = motor->deg_duty;
//         }
//         else
//         {
//             motor->duty_deg.uvw[i] = 0;
//         }
//     }
// }

#endif
