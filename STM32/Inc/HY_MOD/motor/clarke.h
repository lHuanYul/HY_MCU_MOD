#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

typedef struct CLARKE {
    union {
        struct {
            float32_t As;       // Input: phase-a stator variable
            float32_t Bs;       // Input: phase-b stator variable
            float32_t Cs;       // Input: phase-c stator variable  
        };
        float32_t ABC[3];
    };
    float32_t Alpha;    // Output: stationary d-axis stator variable 
    float32_t Beta;     // Output: stationary q-axis stator variable
} CLARKE;

void CLARKE_run_ideal(CLARKE *clarke);
void CLARKE_run_nideal(CLARKE *clarke);

#endif