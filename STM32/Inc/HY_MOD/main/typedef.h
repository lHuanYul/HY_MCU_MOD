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

