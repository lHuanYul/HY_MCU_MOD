#pragma once
#include "main/config.h"
#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

#define BIT_SNG_MASK                (0x01UL)
#define BIT_SNG_SET(flags, bit)     ((flags) |= (0x01UL << (bit)))
#define BIT_SNG_CLR(flags, bit)     ((flags) &= ~(0x01UL << (bit)))
#define BIT_SNG_REV(flags, bit)     ((flags) ^= (0x01UL << (bit)))
#define BIT_SNG_GET(flags, bit)     (((flags) >> (bit)) & 0x01UL)
#define BIT_SNG_CHK(flags, bit)     ((flags) & (0x01UL << (bit)))
#define BIT_0_3_MASK                (0x0FUL)
#define BIT_4_7_MASK                (0xF0UL)
#define FLAGS_SET(flags, mask, val) ((flags) = ((flags) & ~(mask)) | ((val) & (mask)))
#define FLAGS_CLR(flags, mask)      ((flags) &= ~(mask))
#define FLAGS_GET(flags, mask)      ((flags) & (mask))
#define FLAGS_CHK(flags, mask, val) (((flags) & (mask)) == (val))

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

#ifndef PI // 180 deg
#define PI  3.14159265358979f
// #define PI  3.14159265358979323846f 
#endif
#define PI_MUL_2        (2.0f * PI)     // 360 deg
#define PI_DIV_6        (PI / 6.0f)     // 30 deg
#define PI_DIV_3        (PI / 3.0f)     // 60 deg
#define PI_DIV_2        (PI / 2.0f)     // 90 deg
#define DEG_TO_RAD      (PI / 180.0f)
#define RAD_TO_DEG      (180.0f / PI)
#define DIV_1_3         (1.0f / 3.0f)   // 1/3
#define DIV_2_3         (2.0f / 3.0f)   // 2/3
#define SQRT3           1.73205080756888f   // 根號3
#define ONE_DIV_SQRT3   0.577350269189626f  // 1/(根號3)
#define SQRT3_DIV_2     0.866025403784439f  // (根號3)/2
#define RPM_TO_RAD      (PI_MUL_2 / 60.0f)  //

uint32_t var_swap_u32(uint32_t value);
void var_u32_to_u8_be(uint32_t value, uint8_t* u8);
uint32_t var_u8_to_u32_be(const uint8_t *u8);
void var_f32_to_u8_be(float32_t value, uint8_t* u8);
float32_t var_u8_to_f32_be(const uint8_t *u8);
float32_t var_wrap_pos(float32_t x, float32_t value);
float32_t var_wrap_pi(float32_t x, float32_t value);
float32_t var_fabsf(float32_t x);
uint32_t var_u32_iir(uint32_t old, uint32_t new, float32_t alpha);
float32_t var_average(uint16_t *data, uint32_t len);
uint32_t var_u32_max(uint32_t *data, uint32_t size);
uint32_t var_u32_min(uint32_t *data, uint32_t size);

uint16_t var_swap_u16(uint16_t value);
void var_u16_to_u8_be(uint16_t value, uint8_t *u8);
uint16_t var_u8_to_u16_be(const uint8_t *u8);
void var_i16_to_u8_be(int16_t value, uint8_t* u8);
int16_t var_u8_to_i16_be(const uint8_t *u8);
uint16_t var_u16_max(uint16_t *data, uint32_t size);
uint16_t var_u16_min(uint16_t *data, uint32_t size);
