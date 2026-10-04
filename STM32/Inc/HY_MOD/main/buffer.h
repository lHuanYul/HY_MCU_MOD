#pragma once
#include "main/config.h"

#define HISTORY_STORE(store, pos, total_l, value) \
    do { \
        store[pos] = (value); \
        if (++(pos) >= (total_l)) (pos) = 0; \
    } while (0)

#define HISTORY_STORE_L(store, pos, len, total_l, value) \
    do { \
        store[pos] = (value); \
        if (++(pos) >= (total_l)) (pos) = 0; \
        if ((len) < (total_l)) (len)++; \
    } while (0)

#define HISTORY_STORE_SUM(store, pos, len, total_l, sum, value) \
    do { \
        sum = sum - store[pos] + (value); \
        store[pos] = (value); \
        if (++(pos) >= (total_l)) (pos) = 0; \
        if ((len) < (total_l)) (len)++; \
    } while (0)

#ifdef STM32H7
    #ifndef ALIGN_32
        #define ALIGN_32(x)  ((((x) + 31) / 32) * 32)
    #endif
    #define ATTR_RAM_D1_ALIGN_32 __attribute__((section(".RAM_D1"), aligned(32)))
    #define ATTR_RAM_D2_ALIGN_32 __attribute__((section(".dma_nc"), aligned(32)))
#else
    #define ATTR_RAM_D1_ALIGN_32
    #define ALIGN_32(x) x
#endif