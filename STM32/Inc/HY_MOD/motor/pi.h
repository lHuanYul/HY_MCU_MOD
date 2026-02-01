#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

typedef struct PI_CTRL {
    float32_t   reference;    // Input: reference set-point
    float32_t   feedback;    // Input: feedback
    float32_t   out;    // Output: controller output
    float32_t   out_origin;    // Data: pre-saturated controller output 飽和前的控制輸出
    float32_t   max;
    float32_t   min;
    float32_t   Kp;     // Parameter: proportional loop gain 比例增益
    float32_t   Ki;     // Parameter: integral gain 積分增益 Ki'=Ki*Ts/Kp
    float32_t   Error;    // Data: reference - Fbk
    float32_t   Term_p;     // Data: proportional term 比例項 Kp * Error
    float32_t   Term_i;     // Data: integral term 積分項
    float32_t   Term_i_last;    // Data: integrator storage: Term_i(k-1) 積分器狀態
    bool        saturation;    // Data: saturation record: [u(k-1) - v(k-1)] 飽和誤差記錄
} PI_CTRL;

#define PI_CONTROLLER_DEFAULTS { \
    0.0f,   \
    0.0f,   \
    0.0f,   \
    1.0f,   \
    0.0f,   \
    1.0f,   \
    -1.0f,  \
    0.0f,   \
    0.0f,   \
    0.0f,   \
    0.0f,   \
    1.0f,   \
    0.0f    \
}

void PI_run(PI_CTRL *pi);
void PI_reset(PI_CTRL *pi);

#endif