#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

typedef struct PARK {
    float32_t Alpha;    // Input: stationary d-axis stator variable 
    float32_t Beta;     // Input: stationary q-axis stator variable 
    float32_t Angle;    // Input: rotating angle (pu) 
    float32_t Ds;       // Output: rotating d-axis stator variable 
    float32_t Qs;       // Output: rotating q-axis stator variable
    float32_t Sin;
    float32_t Cos;
} PARK;

void PARK_run(PARK *park);

typedef struct IPARK {
    float32_t Alpha;    // Output: stationary d-axis stator variable
    float32_t Beta;     // Output: stationary q-axis stator variable
    float32_t Angle;    // Input: rotating angle (pu)
    float32_t Vdref;    // Input: rotating d-axis stator variable
    float32_t Vdref_min;
    float32_t Vdref_max;
    float32_t Vqref;    // Input: rotating q-axis stator variable
    float32_t Vqref_min;
    float32_t Vqref_max;
    float32_t Sin;     // Input: Sine term
    float32_t Cos;   // Input: Cosine term
} IPARK;

void IPARK_run(IPARK *ipark);

#endif