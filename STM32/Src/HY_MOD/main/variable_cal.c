#include "HY_MOD/main/variable_cal.h"

// 32 bit --------------------------------------------------------------------------------

uint32_t var_comb_u32(const void* src)
{
    const uint8_t *out = (const uint8_t *)src;
    return    ((uint32_t)out[0] << 24)
            | ((uint32_t)out[1] << 16)
            | ((uint32_t)out[2] <<  8)
            | ((uint32_t)out[3] <<  0);
}

/**
 * @brief 交換 32-bit 整數的大小端
 *
 * @param value 要交換大小端的 32-bit 值
 * 
 * @return uint32_t 交換後的 32-bit 值
 */
uint32_t var_swap_u32(uint32_t value)
{
    return    ((value & 0x000000FFU) << 24)
            | ((value & 0x0000FF00U) <<  8)
            | ((value & 0x00FF0000U) >>  8)
            | ((value & 0xFF000000U) >> 24);
}

int32_t var_swap_i32(int32_t value)
{
    int32_t out;
    memcpy(&out, &value, sizeof out);
    out = var_swap_u32(out);
    memcpy(&value, &out, sizeof value);
    return value;
}

float32_t var_swap_f32(float32_t value)
{
    uint32_t out;
    memcpy(&out, &value, sizeof out);
    out = var_swap_u32(out);
    memcpy(&value, &out, sizeof value);
    return value;
}

void var_u32_to_u8_be(uint32_t value, uint8_t *u8)
{
    u8[0] = (uint8_t)(value >> 24);
    u8[1] = (uint8_t)(value >> 16);
    u8[2] = (uint8_t)(value >> 8);
    u8[3] = (uint8_t)(value);
}

uint32_t var_u8_to_u32_be(const uint8_t *u8)
{
    return ((uint32_t)u8[0] << 24) |
           ((uint32_t)u8[1] << 16) |
           ((uint32_t)u8[2] << 8)  |
           ((uint32_t)u8[3]);
}

void var_f32_to_u8_be(float32_t value, uint8_t* u8)
{
    uint32_t u32;
    memcpy(&u32, &value, sizeof(u32));
    var_u32_to_u8_be(u32, u8);
}

float32_t var_u8_to_f32_be(const uint8_t *u8)
{
    uint32_t u32 = var_u8_to_u32_be(u8);
    float32_t f32;
    memcpy(&f32, &u32, sizeof(f32));
    return f32;
}

float32_t var_wrap_pos(float32_t x, float32_t value)
{
    int32_t n = (int32_t)(x / value);
    x -= (float32_t)n * value;
    if (x < 0) x += value;
    return x;
}

float32_t var_wrap_pi(float32_t x, float32_t value)
{
    int32_t n = (int32_t)(x / value);
    x -= (float32_t)n * value;
    if      (x < -PI) x += value;
    else if (x >= PI) x -= value;
    return x;
}

float32_t var_fabsf(float32_t x)
{
    union {
        float32_t f;
        uint32_t u;
    } v = { x };
    v.u &= 0x7FFFFFFF;  // 清除最高位 sign bit
    return v.f;
}

// 16 bit --------------------------------------------------------------------------------

uint16_t var_comb_u16(const void* src)
{
    const uint8_t *out = (const uint8_t *)src;
    return    ((uint16_t)out[2] <<  8)
            | ((uint16_t)out[3] <<  0);
}

/**
 * @brief 交換 16-bit 整數的大小端
 *
 * @param value 要交換大小端的 16-bit 值
 * 
 * @return uint16_t 交換後的 16-bit 值
 */
uint16_t var_swap_u16(uint16_t value)
{
    return    ((value & 0x00FFU) << 8)
            | ((value & 0xFF00U) >> 8);
}

void var_u16_to_u8_be(uint16_t value, uint8_t *u8)
{
    u8[0] = (uint8_t)(value >> 8);
    u8[1] = (uint8_t)(value);
}

uint16_t var_u8_to_u16_be(const uint8_t *u8)
{
    return ((uint16_t)u8[0] << 8) |
           ((uint16_t)u8[1]);
}

void var_i16_to_u8_be(int16_t value, uint8_t* u8)
{
    uint16_t u16;
    memcpy(&u16, &value, sizeof(u16));
    var_u16_to_u8_be(u16, u8);
}

int16_t var_u8_to_i16_be(const uint8_t *u8)
{
    uint16_t u16 = var_u8_to_u16_be(u8);
    int16_t i16;
    memcpy(&i16, &u16, sizeof(i16));
    return i16;
}
