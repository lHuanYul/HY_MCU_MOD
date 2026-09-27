#pragma once
#include "main/config.h"
#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

// 單一 bit 遮罩常數 (第 0 位元為 1)
#define BIT_SNG_MASK                (0x01UL)
// 將指定 bit 設為 1
#define BIT_SNG_SET(flags, bit)     ((flags) |= (0x01UL << (bit)))
// 將指定 bit 清除為 0
#define BIT_SNG_CLR(flags, bit)     ((flags) &= ~(0x01UL << (bit)))
// 反轉 (Toggle) 指定 bit
#define BIT_SNG_REV(flags, bit)     ((flags) ^= (0x01UL << (bit)))
// 取得指定 bit 的值 (右移至第 0 位，結果為 0 或 1)
#define BIT_SNG_GET(flags, bit)     (((flags) >> (bit)) & 0x01UL)
// 檢查指定 bit 是否為 1 (保留原位元權重，非 0 即為 1)
#define BIT_SNG_CHK(flags, bit)     ((flags) & (0x01UL << (bit)))
// 低 4 位元遮罩 (Bit 0~3)
#define BIT_0_3_MASK                (0x0FUL)
// 次 4 位元遮罩 (Bit 4~7)
#define BIT_4_7_MASK                (0xF0UL)
// flags: 目標變數
// mask : 位元遮罩（定義欲操作的特定位元欄位範圍，對應位元為 1 代表要處理）
// val  : 欲寫入或比對的數值（配合 mask 寫入或與 mask 範圍內的欄位進行比較）
// 依據遮罩寫入指定的多位元數值
#define FLAGS_FULLSET(flags, mask, val) ((flags) = ((flags) & ~(mask)) | ((val) & (mask)))
// 依據遮罩將指定位元強制置 1 (設為 1)
#define FLAGS_SET(flags, mask)      ((flags) |= (mask))
// 依據遮罩清除指定的多位元欄位 (設為 0)
#define FLAGS_CLR(flags, mask)      ((flags) &= ~(mask))
// 依據遮罩擷取指定的多位元欄位數值
#define FLAGS_GET(flags, mask)      ((flags) & (mask))
// 檢查遮罩範圍內的位元值是否等於目標值 val
#define FLAGS_CHK(flags, mask, val) (((flags) & (mask)) == (val))

#define VAR_CLAMPF(val, min, max)   \
({                                  \
    if (val > max) val = max;       \
    else if (val < min) val = min;  \
})

#define VAR_CLAMPF_STATIC(val, equ, min, max)  \
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
#define RPM_TO_OMEGA    (PI / 30.0f)
#define OMEGA_TO_RPM    (30.0f / PI)

uint32_t var_swap_u32(uint32_t value);
void var_u32_to_u8_be(uint32_t value, uint8_t* u8);
uint32_t var_u8_to_u32_be(const uint8_t *u8);
void var_f32_to_u8_be(float32_t value, uint8_t* u8);
float32_t var_u8_to_f32_be(const uint8_t *u8);
float32_t var_wrap_P(float32_t x, float32_t value);
float32_t var_wrap_PN(float32_t x, float32_t value);
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

bool var_f32_same_sign(float a, float b);
