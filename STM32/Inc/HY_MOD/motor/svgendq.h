#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

typedef struct SVGENDQ	{
    float32_t   Ualpha; // Input: reference alpha-axis phase voltage 
    float32_t   Ubeta;  // Input: reference beta-axis phase voltage 
    float32_t   Va;     // Output: reference phase-a switching function		
    float32_t   Vb;     // Output: reference phase-b switching function 
    float32_t   Vc;     // Output: reference phase-c switching function
    uint16_t    Sector; // Sector is treated as Q0 - independently with global Q
} SVGENDQ;

void SVGEN_run(SVGENDQ *svgq);

#endif