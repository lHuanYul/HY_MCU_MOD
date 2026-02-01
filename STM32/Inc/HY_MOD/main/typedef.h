#pragma once
#include "main/config.h"

#ifndef float32_t
#include <float.h>
#define float32_t float
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define ATTR_X(...) __attribute__((__VA_ARGS__))
#define ATTR_UNUSED __attribute__((unused))
#define ATTR_WEAK   __attribute__((weak))

#define ITS_CHECK(its, tag) (((its) & (tag)) != RESET)

typedef uint8_t Percentage;

typedef int8_t FncState;
#define FNC_CANCEL  -1
#define FNC_DISABLE 0
#define FNC_ENABLE  1

#ifdef STM32_DEVICE
typedef struct GPIOData
{
    GPIO_TypeDef        *GPIOx;
    uint16_t            GPIO_Pin_x;
} GPIOData;

#define GPIO_TOGGLE(pin)    HAL_GPIO_TogglePin((pin).GPIOx, (pin).GPIO_Pin_x)
#define GPIO_WRITE(pin,set) HAL_GPIO_WritePin((pin).GPIOx, (pin).GPIO_Pin_x, set)
#define GPIO_READ(pin)      HAL_GPIO_ReadPin((pin).GPIOx, (pin).GPIO_Pin_x)
#define GPIO_IF_RESET(pin)  (!HAL_GPIO_ReadPin((pin).GPIOx, (pin).GPIO_Pin_x))
#define GPIO_IF_SET(pin)    HAL_GPIO_ReadPin((pin).GPIOx, (pin).GPIO_Pin_x)

#ifdef STM32G431RB
#define BOARD_LED_TOGGLE    HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5)
#define BOARD_LED_ON        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET)
#define BOARD_LED_OFF       HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET)
#endif

#endif