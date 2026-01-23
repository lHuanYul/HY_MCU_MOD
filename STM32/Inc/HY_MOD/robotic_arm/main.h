#pragma once
#include "HY_MOD/robotic_arm/basic.h"
#ifdef HY_MOD_STM32_ROB_ARM

typedef struct ArmMotionData
{
    uint8_t total_step;
    uint8_t data[][ARM_MAX];
} ArmMotionData;

typedef enum ArmMotion
{
    ARM_MOTION_UNKNOWN = -1,
    ARM_MOTION_IDLE,
    ARM_MOTION_00,
} ArmMotion;

typedef struct ArmParameter
{
    ArmMotion motion;
    uint8_t step;
    ArmMotion motion_inner;
    uint8_t step_inner;
} ArmParameter;

void arm_motion_set(ArmMotion motion);

#endif