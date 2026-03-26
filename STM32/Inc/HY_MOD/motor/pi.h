#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

typedef struct PI_CTRL {
    // Parameter: maximum output value 最大輸出值
    float32_t   max;
    // Parameter: minimum output value 最小輸出值
    float32_t   min;
    // Parameter: proportional loop gain 比例增益
    float32_t   Kp;
    // Parameter: integral gain 積分增益 Ki'=Ki*Ts/Kp
    float32_t   Ki;
    // Input: reference set-point
    float32_t   reference;
    // Input: feedback
    float32_t   feedback;
    // Data: reference - Fbk
    float32_t   Error;
    // Data: proportional term 比例項 Kp * Error
    float32_t   Term_p;
    // Data: integral term 積分項
    float32_t   Term_i_fix;
    float32_t   Term_i_ori;
    // Data: pre-saturated controller output 飽和前的控制輸出
    float32_t   out_ori;
    // Output: controller output fixed by saturation 飽和後的控制輸出
    float32_t   out_fix;
    // Output: saturation record: 飽和誤差記錄
    bool        saturation;
} PI_CTRL;

void PI_run(PI_CTRL *pi);
void PI_reset(PI_CTRL *pi);

#endif