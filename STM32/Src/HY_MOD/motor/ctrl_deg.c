#include "HY_MOD/motor/ctrl_deg.h"
#ifdef HY_MOD_STM32_MOTOR

#define HIGH_PASS   1
#define NONE_PASS   0
#define LOW__PASS  -1
static const int8_t seq_map_120[9][3] = {
    // 註解從頭順序是ccw 標準化角度-霍爾
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

static void ctrl_load(MotorParameter *motor, int8_t seq[3], float32_t duty)
{
    // 上臂全關
    motor_timer_load_inner(motor, 0, 0, 0);

    const MotorPhaseConst *phase = motor->system.const_h.PWM_uvw;
    uint8_t i;
    for (i = 0; i < 3; i++)
    {
        if (seq[i] == HIGH_PASS)
        {
            motor->deg_h.phases_duty.uvw[i] = duty;
            motor->deg_h.phases_duty.uvw[i+3] = 0.0f;
            GPIO_WRITE_R(phase[i].pwmn_gpio, 0);
        }
        else if (seq[i] == LOW__PASS)
        {
            motor->deg_h.phases_duty.uvw[i] = 0.0f;
            motor->deg_h.phases_duty.uvw[i+3] = 1.0f;
            GPIO_WRITE_R(phase[i].pwmn_gpio, 1);
        }
        else
        {
            motor->deg_h.phases_duty.uvw[i] = 0.0f;
            motor->deg_h.phases_duty.uvw[i+3] = 0.0f;
            GPIO_WRITE_R(phase[i].pwmn_gpio, 0);
        }
    }
    motor->phases_duty_load = motor->deg_h.phases_duty;
    motor_timer_load(motor);
}

void motor_deg_test(MotorParameter *motor)
{
    uint8_t i;
    float32_t duty = 1.0f;
    int8_t seq[3] = {0};
    switch (motor->ctrl.ref_sys)
    {
        case MOTOR_CTRL_TEST_HIGH:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[6][i];
            break;
        }
        case MOTOR_CTRL_TEST_LOW:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[7][i];
            break;
        }
        case MOTOR_CTRL_TEST_WAVE:
        {
            duty = 0.3f;
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[6][i];
            break;
        }
        default: return;
    }
    ctrl_load(motor, seq, duty);
}

void motor_deg_120_load(MotorParameter *motor, uint8_t id)
{
    uint8_t i;
    int8_t seq[3] = {0};
    switch (motor->rotate.ref_sys)
    {
    	case MOTOR_ROTATE_UNINIT: return;
        case MOTOR_ROTATE_COAST:
        {
            motor->deg_h.duty_val = 1.0f;
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[8][i];
            break;
        }
        case MOTOR_ROTATE_BREAK:
        case MOTOR_ROTATE_LOCK:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_120[7][i];
            break;
        }
        case MOTOR_ROTATE_NORMAL:
        {
            for (i = 0; i < 3; i++)
            {
                if (!motor->deg_h.reverse)
                    seq[i] = seq_map_120[id][i];
                else
                    seq[i] = seq_map_120[(id + 3) % 6][i];
            }
            break;
        }
        case MOTOR_ROTATE_LOCK_FIN:
        {
            motor->deg_h.duty_val = 0.2f;
            // Todo
            // motor->rotor.virtual = motor->rotor.curr;
            for (i = 0; i < 3; i++)
                seq[i] = seq_map_120[motor->rotor.virtual][i];
            break;
        }
    }
    ctrl_load(motor, seq, motor->deg_h.duty_val);
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
//     if (motor->rotor.curr == UINT8_MAX) return;
//     uint8_t i;
//     int8_t seq[3] = {0};
//     switch (motor->rotate.ref_sys)
//     {
//         case MOTOR_ROTATE_COAST:
//         {
//             motor->deg_h.duty_val = 1.0f;
//             for (i = 0; i < 3; i++) seq[i] = seq_map_180[6][i];
//             break;
//         }
//         case MOTOR_ROTATE_BREAK:
//         case MOTOR_ROTATE_LOCK:
//         {
//             for (i = 0; i < 3; i++) seq[i] = seq_map_180[7][i];
//             break;
//         }
//         case MOTOR_ROTATE_NORMAL:
//         {
//             for (i = 0; i < 3; i++)
//             {
//                 if (!motor->deg_h.reverse)
//                     seq[i] = seq_map_180[index_180_ccw[motor->rotor.curr]][i];
//                 else
//                     seq[i] = seq_map_180[ index_180_cw[motor->rotor.curr]][i];
//             }
//             break;
//         }
//         case MOTOR_ROTATE_LOCK_FIN:
//         {
//             motor->deg_h.duty_val = 0.2f;
//             for (i = 0; i < 3; i++)
//                 seq[i] = seq_map_180[index_180_lock[motor->rotor.curr]][i];
//             break;
//         }
//     }
//     for (i = 0; i < 3; i++)
//     {
//         if (seq[i] == HIGH_PASS)
//         {
//             motor->deg_h.duty_h.uvw[i] = motor->deg_h.duty_val;
//         }
//         else
//         {
//             motor->deg_h.duty_h.uvw[i] = 0;
//         }
//     }
// }

#include "HY_MOD/motor/rotor.h"

Result motor_deg_reverse_upd(MotorParameter *motor)
{
    if (fabsf(motor->speed.fbk_omega) > motor->speed.save_stop_omega)
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    
    motor->deg_h.reverse = (motor->speed.ref_omega < 0.0f);
    return RESULT_OK(NULL);
}

void motor_deg_check_reverse(MotorParameter *motor)
{
    if (motor->speed.ref_omega == 0.0f) return;
    if (motor->deg_h.reverse == (motor->speed.ref_omega < 0.0f)) return;

    if (RESULT_CHECK_OK(motor_deg_reverse_upd(motor)))
    {
        motor_rotor_stop_cbi(motor);
        return;
    }

    motor->ctrl.ref_sys_temp = motor->ctrl.ref_sys;
    motor_switch_ctrl_system(motor, MOTOR_CTRL_120_DIREC_SW);
}

void motor_deg_proc_safe_reverse(MotorParameter *motor)
{
    if (RESULT_CHECK_FAIL(motor_deg_reverse_upd(motor)))
    {
        motor->rotate.ref_sys = MOTOR_ROTATE_COAST;
        return;
    }

    motor_rotor_stop_cbi(motor);
    motor_switch_ctrl_system(motor, motor->ctrl.ref_sys_temp);
    return;
}

void motor_deg_stop_cbi(MotorParameter *motor)
{
    PID_reset(&motor->deg_h.pi_omega);
}

#endif
