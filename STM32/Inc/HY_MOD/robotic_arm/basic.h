#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_ROB_ARM

#include "tim.h"
#include "gpio.h"

#define ARM_MAX 6

#define ARM_TIM_MIN    50
#define ARM_TIM_MAX   250
#define ARM_TIM_STEP    1

typedef struct ArmMotorConst
{
    uint8_t id;
    TIM_HandleTypeDef* htimx;
    uint32_t TIM_CHANNEL_x;
    uint8_t tim_min;
    uint8_t tim_max;
    uint8_t tim_step;
} ArmMotorConst;
typedef struct ArmMotorParameter
{
    const ArmMotorConst const_h;
    uint8_t tim_current;
    uint8_t tim_setpoint;
} ArmMotorParameter;
extern ArmMotorParameter arm_bottom;
extern ArmMotorParameter arm_shoulder;
extern ArmMotorParameter arm_elbow_btm;
extern ArmMotorParameter arm_elbow_top;
extern ArmMotorParameter arm_wrist;
extern ArmMotorParameter arm_finger;

void arm_set_tim(ArmMotorParameter* arm, uint8_t tim);
void arm_set_pos(ArmMotorParameter* arm, uint8_t pos);

#endif