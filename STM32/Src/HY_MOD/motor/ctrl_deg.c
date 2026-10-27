#include "HY_MOD/motor/ctrl.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/motor/main.h"

// #define _HIGH_   1
// #define _NONE_   0
// #define _LOW__  -1
// static const int8_t seq_map_120[][3] = {
//     // 註解從頭順序是ccw 標準化角度-霍爾
//     { _HIGH_, _NONE_, _LOW__ }, // 0-4
//     { _NONE_, _HIGH_, _LOW__ }, // 1-6
//     { _LOW__, _HIGH_, _NONE_ }, // 2-2
//     { _LOW__, _NONE_, _HIGH_ }, // 3-3
//     { _NONE_, _LOW__, _HIGH_ }, // 4-1
//     { _HIGH_, _LOW__, _NONE_ }, // 5-5
//     { _HIGH_, _HIGH_, _HIGH_ }, // 6
//     { _LOW__, _LOW__, _LOW__ }, // 7
//     { _NONE_, _NONE_, _NONE_ }, // 8
// };
// static const int8_t zcp_map_120[][3] = {
//     { _NONE_, _HIGH_, _NONE_ },
//     { _LOW__, _NONE_, _NONE_ },
//     { _NONE_, _NONE_, _HIGH_ },
//     { _NONE_, _LOW__, _NONE_ },
//     { _HIGH_, _NONE_, _NONE_ },
//     { _NONE_, _NONE_, _LOW__ },
// };

static const StepDriveLUT STEP_DRIVE_TABLE[] = {
    // Step 0 (4): U=HIGH, V=NONE (Rising), W=LOW
    [0] = {
        .u_h = 1.0f, .v_h = 0.0f, .w_h = 0.0f,
        .u_l = 0.0f, .v_l = 0.0f, .w_l = 1.0f,
        .u_l_en = 0, .v_l_en = 0, .w_l_en = 1,
        .u_zcp  = 0, .v_zcp  = 1, .w_zcp  = 0
    },
    // Step 1 (6): U=NONE (Falling), V=HIGH, W=LOW
    [1] = {
        .u_h = 0.0f, .v_h = 1.0f, .w_h = 0.0f,
        .u_l = 0.0f, .v_l = 0.0f, .w_l = 1.0f,
        .u_l_en = 0, .v_l_en = 0, .w_l_en = 1,
        .u_zcp  =-1, .v_zcp  = 0, .w_zcp  = 0
    },
    // Step 2 (2): U=LOW, V=HIGH, W=NONE (Rising)
    [2] = {
        .u_h = 0.0f, .v_h = 1.0f, .w_h = 0.0f,
        .u_l = 1.0f, .v_l = 0.0f, .w_l = 0.0f,
        .u_l_en = 1, .v_l_en = 0, .w_l_en = 0,
        .u_zcp  = 0, .v_zcp  = 0, .w_zcp  = 1
    },
    // Step 3 (3): U=LOW, V=NONE (Falling), W=HIGH
    [3] = {
        .u_h = 0.0f, .v_h = 0.0f, .w_h = 1.0f,
        .u_l = 1.0f, .v_l = 0.0f, .w_l = 0.0f,
        .u_l_en = 1, .v_l_en = 0, .w_l_en = 0,
        .u_zcp  = 0, .v_zcp  =-1, .w_zcp  = 0
    },
    // Step 4 (1): U=NONE (Rising), V=LOW, W=HIGH
    [4] = {
        .u_h = 0.0f, .v_h = 0.0f, .w_h = 1.0f,
        .u_l = 0.0f, .v_l = 1.0f, .w_l = 0.0f,
        .u_l_en = 0, .v_l_en = 1, .w_l_en = 0,
        .u_zcp  = 1, .v_zcp  = 0, .w_zcp  = 0
    },
    // Step 5 (5): U=HIGH, V=LOW, W=NONE (Falling)
    [5] = {
        .u_h = 1.0f, .v_h = 0.0f, .w_h = 0.0f,
        .u_l = 0.0f, .v_l = 1.0f, .w_l = 0.0f,
        .u_l_en = 0, .v_l_en = 1, .w_l_en = 0,
        .u_zcp  = 0, .v_zcp  = 0, .w_zcp  =-1
    },
    // Cond 6: All HIGH
    [6] = {
        .u_h = 1.0f, .v_h = 1.0f, .w_h = 1.0f,
        .u_l = 0.0f, .v_l = 0.0f, .w_l = 0.0f,
        .u_l_en = 0, .v_l_en = 0, .w_l_en = 0,
        .u_zcp  = 0, .v_zcp  = 0, .w_zcp  = 0
    },
    // Cond 7: All LOW
    [7] = {
        .u_h = 0.0f, .v_h = 0.0f, .w_h = 0.0f,
        .u_l = 1.0f, .v_l = 1.0f, .w_l = 1.0f,
        .u_l_en = 1, .v_l_en = 1, .w_l_en = 1,
        .u_zcp  = 0, .v_zcp  = 0, .w_zcp  = 0
    },
    // Cond 8: All NONE (自由滑行 Coast)
    [8] = {
        .u_h = 0.0f, .v_h = 0.0f, .w_h = 0.0f,
        .u_l = 0.0f, .v_l = 0.0f, .w_l = 0.0f,
        .u_l_en = 0, .v_l_en = 0, .w_l_en = 0,
        .u_zcp  = 0, .v_zcp  = 0, .w_zcp  = 0
    }
};

inline bool motor_deg_zcp_match(MotorParameter *motor)
{
    const StepDriveLUT *cfg = &STEP_DRIVE_TABLE[motor->deg_h.load_phase_id];

    // 利用 zcp (0, 1, -1) 自動過濾非懸空相，並自動翻轉下降相的極性
    // 因為非懸空相的 zcp 均為 0，三項相加等同只取出「懸空相的定向投影值」
    float32_t zcp_val =
        (motor->adc_h.u_vol.pu * (float32_t)cfg->u_zcp) +
        (motor->adc_h.v_vol.pu * (float32_t)cfg->v_zcp) +
        (motor->adc_h.w_vol.pu * (float32_t)cfg->w_zcp);
    
    return (zcp_val > ZCP_THRESHOLD_PU) ? 1 : 0;
}

inline void motor_deg_pi_setup(MotorParameter *motor)
{
    motor->deg_h.pi_omega.Kp = motor->system.const_h.model->deg_spd_Kp;
    motor->deg_h.pi_omega.Ki = motor->system.const_h.model->deg_spd_Ki;
    motor->deg_h.pi_omega.max = 1.0f;
    motor->deg_h.pi_omega.min = 0.0f;
}

inline uint8_t motor_deg_get_next_phase(MotorParameter *motor, uint8_t phase)
{
    if (phase >= MOTOR_HALL_OF) return MOTOR_HALL_OF;
    if (!motor->deg_h.reverse)
        return ((phase + 1) % 6);
    else
        return ((phase + 5) % 6);
}

static void ctrl_load(MotorParameter *motor)
{
    const StepDriveLUT  *cfg = &STEP_DRIVE_TABLE[motor->deg_h.load_phase_id];
    const MotorConst    *c   = &motor->system.const_h;
    MotorPhaseDuty      *f_d = &motor->deg_h.final_duty;
    float32_t           duty = motor->deg_h.load_duty_val;

    f_d->u_h = duty * cfg->u_h;
    f_d->v_h = duty * cfg->v_h;
    f_d->w_h = duty * cfg->w_h;
    f_d->u_l = cfg->u_l;
    f_d->v_l = cfg->v_l;
    f_d->w_l = cfg->w_l;

    // 關閉上臂
    motor_timer_load_inner(motor, 0, 0, 0);
    // 更新
    GPIO_WRITE_R(c->PWM_u.pwmn_gpio, cfg->u_l_en);
    GPIO_WRITE_R(c->PWM_v.pwmn_gpio, cfg->v_l_en);
    GPIO_WRITE_R(c->PWM_w.pwmn_gpio, cfg->w_l_en);
    motor->final_duty = motor->deg_h.final_duty;
    motor_timer_load(motor);
}

void motor_deg_test(MotorParameter *motor)
{
    switch (motor->ctrl.ref_sys)
    {
        case MOTOR_CTRL_TEST_HIGH:
        {
            motor->deg_h.load_duty_val = 1.0f;
            motor->deg_h.load_phase_id = 6;
            break;
        }
        case MOTOR_CTRL_TEST_LOW:
        {
            motor->deg_h.load_duty_val = 1.0f;
            motor->deg_h.load_phase_id = 7;
            break;
        }
        case MOTOR_CTRL_TEST_WAVE:
        {
            motor->deg_h.load_duty_val = 0.3f;
            motor->deg_h.load_phase_id = 6;
            break;
        }
        default: return;
    }
    ctrl_load(motor);
}

void motor_deg_120_load(MotorParameter *motor, uint8_t id)
{
    switch (motor->rotate.ref_sys)
    {
    	case MOTOR_ROTATE_UNINIT: return;
        case MOTOR_ROTATE_COAST:
        {
            motor->deg_h.load_duty_val = 1.0f;
            motor->deg_h.load_phase_id = 8;
            break;
        }
        case MOTOR_ROTATE_BREAK:
        case MOTOR_ROTATE_LOCK:
        {
            motor->deg_h.load_phase_id = 7;
            break;
        }
        case MOTOR_ROTATE_NORMAL:
        {
            if (!motor->deg_h.reverse)
                motor->deg_h.load_phase_id = id;
            else
                motor->deg_h.load_phase_id = (id + 3) % 6;
            break;
        }
        case MOTOR_ROTATE_LOCK_FIN:
        {
            motor->deg_h.load_duty_val = 0.2f;
            // Todo
            // motor->rotor.virtual = motor->rotor.curr;
            motor->deg_h.load_phase_id = motor->rotor.virtual;
            break;
        }
    }
    ctrl_load(motor);
}

/*
static const int8_t seq_map_180[][3] = {
    { _HIGH_, _LOW__,  _HIGH_ }, // 0-4
    { _HIGH_, _LOW__,  _LOW__  }, // 1-6
    { _HIGH_, _HIGH_, _LOW__  }, // 2-2
    { _LOW__,  _HIGH_, _LOW__  }, // 3-3
    { _LOW__,  _HIGH_, _HIGH_ }, // 4-1
    { _LOW__,  _LOW__,  _HIGH_ }, // 5-5
    { _HIGH_, _HIGH_, _HIGH_ }, // 6
    { _LOW__,  _LOW__,  _LOW__  }, // 7
};
static const uint8_t index_180_lock[] = {7, 4, 2, 3, 0, 5, 1, 7};
static const uint8_t index_180_ccw[]  = {7, 0, 4, 5, 2, 1, 3, 7};
static const uint8_t index_180_cw[]   = {7, 2, 0, 1, 4, 3, 5, 7};
void deg_ctrl_180_load(MotorParameter *motor)
{
    if (motor->rotor.curr == UINT8_MAX) return;
    uint8_t i;
    int8_t seq[3] = {0};
    switch (motor->rotate.ref_sys)
    {
        case MOTOR_ROTATE_COAST:
        {
            motor->deg_h.duty_val = 1.0f;
            for (i = 0; i < 3; i++) seq[i] = seq_map_180[6][i];
            break;
        }
        case MOTOR_ROTATE_BREAK:
        case MOTOR_ROTATE_LOCK:
        {
            for (i = 0; i < 3; i++) seq[i] = seq_map_180[7][i];
            break;
        }
        case MOTOR_ROTATE_NORMAL:
        {
            for (i = 0; i < 3; i++)
            {
                if (!motor->deg_h.reverse)
                    seq[i] = seq_map_180[index_180_ccw[motor->rotor.curr]][i];
                else
                    seq[i] = seq_map_180[ index_180_cw[motor->rotor.curr]][i];
            }
            break;
        }
        case MOTOR_ROTATE_LOCK_FIN:
        {
            motor->deg_h.duty_val = 0.2f;
            for (i = 0; i < 3; i++)
                seq[i] = seq_map_180[index_180_lock[motor->rotor.curr]][i];
            break;
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (seq[i] == _HIGH_)
        {
            motor->deg_h.duty_h.uvw[i] = motor->deg_h.duty_val;
        }
        else
        {
            motor->deg_h.duty_h.uvw[i] = 0;
        }
    }
}
*/

#include "HY_MOD/motor/rotor.h"

Result motor_deg_reverse_upd(MotorParameter *motor)
{
    if (fabsf(motor->speed.fbk_omega) > motor->speed.save_stop_omega)
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    
    motor->deg_h.reverse = (motor->speed.ref_omega < 0.0f);
    return RESULT_OK(motor);
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
    if (RESULT_CHECK_ERR(motor_deg_reverse_upd(motor)))
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
