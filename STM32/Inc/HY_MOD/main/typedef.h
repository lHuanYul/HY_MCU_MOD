#pragma once
#include "main/config.h"

#ifndef float32_t
#include <float.h>
#define float32_t float
#endif

typedef uint8_t Percentage;

typedef int8_t FncState;
#define FNC_CANCEL  -1
#define FNC_DISABLE 0
#define FNC_ENABLE  1

typedef struct GPIOData
{
    GPIO_TypeDef        *GPIOx;
    uint16_t            GPIO_Pin_x;
} GPIOData;

#define GPIO_TOGGLE(pin)    HAL_GPIO_TogglePin((pin).GPIOx, (pin).GPIO_Pin_x)
#define GPIO_WRITE(pin,set) HAL_GPIO_WritePin((pin).GPIOx, (pin).GPIO_Pin_x, set)
