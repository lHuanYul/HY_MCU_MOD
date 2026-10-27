#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

// ---------- Control 120 deg ----------

typedef struct {
    // 1. 上臂 High-side Duty 倍率 (0.0f 或 1.0f)
    union {
        float32_t high[3];
        struct {
            float32_t u_h, v_h, w_h;
        };
    };

    // 2. 下臂 Low-side Duty 基準值 (0.0f 或 1.0f)
    union {
        float32_t low[3];
        struct {
            float32_t u_l, v_l, w_l;
        };
    };

    // 3. 下臂 GPIO 控制狀態 (0: 關斷, 1: 導通)
    union {
        uint8_t l_en[3];
        struct {
            uint8_t u_l_en, v_l_en, w_l_en;
        };
    };

    // 4. BEMF 過零方向 (1: Rising, -1: Falling, 0: 導通相不偵測)
    union {
        int8_t zcp[3];
        struct {
            int8_t u_zcp, v_zcp, w_zcp;
        };
    };
} StepDriveLUT;

/**
 * @brief 檢查懸空相過 0
 * 
 * @return 1: match
 */
#define ZCP_THRESHOLD_PU 0.03f   // ±3% 標么值門檻
bool motor_deg_zcp_match(MotorParameter *motor);

void motor_deg_pi_setup(MotorParameter *motor);

uint8_t motor_deg_get_next_phase(MotorParameter *motor, uint8_t cur);

void motor_deg_test(MotorParameter *motor);

void motor_deg_120_load(MotorParameter *motor, uint8_t id);
// void deg_ctrl_180_load(MotorParameter *motor);
/**
 * @brief 檢查轉速目標與回授方向，若不一致則請求進入換向過渡狀態
 */
void motor_deg_check_reverse(MotorParameter *motor);
Result motor_deg_reverse_upd(MotorParameter *motor);
/**
 * @brief 監控滑行減速至安全轉速後，正式套用反轉並切回正常控制
 */
void motor_deg_proc_safe_reverse(MotorParameter *motor);
void motor_deg_stop_cbi(MotorParameter *motor);

// ---------- Control FOC ----------

void motor_foc_pi_init(MotorParameter *motor);
void motor_foc_reset(MotorParameter *motor);
void motor_foc_hall_timer_cbi(MotorParameter *motor);
void motor_foc_run(MotorParameter *motor);
void motor_foc_load(MotorParameter *motor);
void motor_foc_stop_cbi(MotorParameter *motor);

#endif