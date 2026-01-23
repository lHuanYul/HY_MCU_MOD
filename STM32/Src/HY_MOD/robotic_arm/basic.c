#include "HY_MOD/robotic_arm/basic.h"
#ifdef HY_MOD_STM32_ROB_ARM

#include "HY_MOD/main/variable_cal.h"

ArmMotorParameter arm_bottom = {
    .const_h = {
        .id = 0,
        // PB0(L34) blue
        .htimx = &htim3,
        .TIM_CHANNEL_x = TIM_CHANNEL_1,
        .tim_min = ARM_TIM_MIN,
        .tim_max = ARM_TIM_MAX,
        .tim_step = ARM_TIM_STEP,
    },
    .tim_current = 150,
    .tim_setpoint = 150,
};

ArmMotorParameter arm_shoulder = {
    .const_h = {
        .id = 1,
        // PA4(L32) green
        .htimx = &htim3,
        .TIM_CHANNEL_x = TIM_CHANNEL_2,
        .tim_min = ARM_TIM_MIN,
        .tim_max = ARM_TIM_MAX,
        .tim_step = 3,
    },
    .tim_current = 150,
    .tim_setpoint = 150,
};

ArmMotorParameter arm_elbow_btm = {
    .const_h = {
        .id = 2,
        // PA1(L30) yello
        .htimx = &htim3,
        .TIM_CHANNEL_x = TIM_CHANNEL_3,
        .tim_min = ARM_TIM_MIN,
        .tim_max = ARM_TIM_MAX,
        .tim_step = ARM_TIM_STEP,
    },
    .tim_current = 150,
    .tim_setpoint = 150,
};

ArmMotorParameter arm_elbow_top = {
    .const_h = {
        .id = 3,
        // PB10(R25) gray
        .htimx = &htim4,
        .TIM_CHANNEL_x = TIM_CHANNEL_1,
        .tim_min = ARM_TIM_MIN,
        .tim_max = ARM_TIM_MAX,
        .tim_step = ARM_TIM_STEP,
    },
    .tim_current = 150,
    .tim_setpoint = 150,
};

ArmMotorParameter arm_wrist = {
    .const_h = {
        .id = 4,
        // PA6(R13) purple
        .htimx = &htim4,
        .TIM_CHANNEL_x = TIM_CHANNEL_2,
        .tim_min = ARM_TIM_MIN,
        .tim_max = ARM_TIM_MAX,
        .tim_step = ARM_TIM_STEP,
    },
    .tim_current = 150,
    .tim_setpoint = 150,
};

ArmMotorParameter arm_finger = {
    .const_h = {
        .id = 5,
        // PA0(L28) orange
        .htimx = &htim4,
        .TIM_CHANNEL_x = TIM_CHANNEL_3,
        .tim_min = ARM_TIM_MIN,
        .tim_max = ARM_TIM_MAX / 2,
        .tim_step = ARM_TIM_STEP,
    },
    .tim_current = 150,
    .tim_setpoint = 150,
};

void arm_set_tim(ArmMotorParameter* arm, uint8_t tim)
{
    const ArmMotorConst* const_h = &arm->const_h;
    if      (tim < const_h->tim_min) arm->tim_setpoint = const_h->tim_min;
    else if (tim > const_h->tim_max) arm->tim_setpoint = const_h->tim_max;
    arm->tim_setpoint = tim;
}

void arm_set_pos(ArmMotorParameter* arm, uint8_t pos)
{
    if (pos > 100) pos = 100;
    arm_set_tim(arm, pos*2 + 50);
}

#endif