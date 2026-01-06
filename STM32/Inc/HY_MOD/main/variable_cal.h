#pragma once
#include "main/config.h"

#include "HY_MOD/main/fn_state.h"

#define VAR_CLAMPF(val, min, max)   \
({                                  \
    if (val > max) val = max;       \
    else if (val < min) val = min;  \
})

#define VAR_EQU_CLAMPF(val, equ, min, max)  \
({                                          \
    val = (equ);                            \
    if (val > max) val = max;               \
    else if (val < min) val = min;          \
})

uint32_t var_swap_u32(uint32_t value);
void var_u32_to_u8_be(uint32_t value, uint8_t* u8);
uint32_t var_u8_to_u32_be(const uint8_t *u8);
void var_f32_to_u8_be(float32_t value, uint8_t* u8);
float32_t var_u8_to_f32_be(const uint8_t *u8);
float32_t var_wrap_pos(float32_t x, float32_t value);
float32_t var_wrap_pi(float32_t x, float32_t value);
float32_t var_fabsf(float32_t x);

uint16_t var_swap_u16(uint16_t value);
void var_u16_to_u8_be(uint16_t value, uint8_t *u8);
uint16_t var_u8_to_u16_be(const uint8_t *u8);
void var_i16_to_u8_be(int16_t value, uint8_t* u8);
int16_t var_u8_to_i16_be(const uint8_t *u8);
