#include "HY_MOD/robotic_arm/main.h"
#ifdef HY_MOD_STM32_ROB_ARM

static const ArmMotionData motion_data_unknown = {0};

static const ArmMotionData motion_data_idle = {
    .total_step = 3,
    .data = {
        {50, 50, 50, 50, 50, 50},
        {25, 50, 25, 25, 50, 25},
        {25, 75, 00,  0, 50,  0},
    },
};

static const ArmMotionData motion_data_catch_o = {
    .total_step = 10,
    .data = {
        {50, 50, 50, 50, 50, 50},
        {75, 58, 25, 28, 50, 25},
        {75, 58, 00, 28, 50, 00},
        {75, 58, 00, 28, 50, 00},
        {75, 58, 00, 28, 50, 50},
        {75, 75, 00, 30, 50, 50},
        {50, 75, 00, 30, 50, 50},
        {26, 75, 00, 30, 50, 50},
        {26, 75, 00, 26, 50, 50},
        {26, 75, 00, 26, 50, 00},
    },
};

ArmParameter robotic_arm_h;

static void arm_setup(ArmMotorParameter* arm)
{
    const ArmMotorConst* const_h = &arm->const_h;
    HAL_TIM_PWM_Start(const_h->htimx, const_h->TIM_CHANNEL_x);
}

static void arm_turn(ArmMotorParameter* arm)
{
    const ArmMotorConst *const_h = &arm->const_h;
    if (arm->tim_current != arm->tim_setpoint)
    {
        int16_t dtim = arm->tim_setpoint - arm->tim_current;
        if      (dtim >  const_h->tim_step) arm->tim_current += const_h->tim_step;
        else if (dtim < -const_h->tim_step) arm->tim_current -= const_h->tim_step;
        else                                arm->tim_current  = arm->tim_setpoint;
    }
    __HAL_TIM_SET_COMPARE(const_h->htimx, const_h->TIM_CHANNEL_x, arm->tim_current);
}

static bool arm_ready = 0;
void StartArmMotorTask(void *argument)
{
    HAL_TIM_Base_Start_IT(arm_bottom.const_h.htimx);
    HAL_TIM_Base_Start_IT(arm_elbow_top.const_h.htimx);
    arm_setup(&arm_bottom);
    arm_setup(&arm_shoulder);
    arm_setup(&arm_elbow_btm);
    arm_setup(&arm_elbow_top);
    arm_setup(&arm_wrist);
    arm_setup(&arm_finger);
    arm_ready = 1;
    for(;;)
    {
        arm_turn(&arm_bottom);
        arm_turn(&arm_shoulder);
        arm_turn(&arm_elbow_btm);
        arm_turn(&arm_elbow_top);
        arm_turn(&arm_wrist);
        arm_turn(&arm_finger);
        osDelay(20);
    }
}

void arm_motion_switch(const ArmMotionData *motion_h)
{}

void arm_motion_set(ArmMotion motion)
{
    const ArmMotionData *motion_h;
    switch (motion)
    {
        case ARM_MOTION_UNKNOWN:
        {
            motion_h = &motion_data_unknown;
            robotic_arm_h.motion_inner = ARM_MOTION_UNKNOWN;
            robotic_arm_h.step_inner = 0;
            break;
        }
        case ARM_MOTION_IDLE:
        {
            motion_h = &motion_data_idle;
            break;
        }
        case ARM_MOTION_00:
        {
            motion_h = &motion_data_catch_o;
            break;
        }
        default: return;
    }
    robotic_arm_h.motion = motion;
    robotic_arm_h.step = motion_h->total_step;
}

static void motion_set_inner(ArmMotion motion, uint8_t step)
{
    const ArmMotionData *motion_h;
    switch (motion)
    {
        case ARM_MOTION_IDLE:
        {
            motion_h = &motion_data_idle;
            break;
        }
        case ARM_MOTION_00:
        {
            motion_h = &motion_data_catch_o;
            break;
        }
        default: return;
    }
    if (step >= motion_h->total_step) return;
    arm_set_pos(&arm_bottom, motion_h->data[step][arm_bottom.const_h.id]);
    arm_set_pos(&arm_shoulder, motion_h->data[step][arm_shoulder.const_h.id]);
    arm_set_pos(&arm_elbow_btm, motion_h->data[step][arm_elbow_btm.const_h.id]);
    arm_set_pos(&arm_elbow_top, motion_h->data[step][arm_elbow_top.const_h.id]);
    arm_set_pos(&arm_wrist, motion_h->data[step][arm_wrist.const_h.id]);
    arm_set_pos(&arm_finger, motion_h->data[step][arm_finger.const_h.id]);
}

static void delay_until_reach(void)
{
    for(;;)
    {
        if (
               arm_bottom.tim_current == arm_bottom.tim_setpoint
            && arm_shoulder.tim_current == arm_shoulder.tim_setpoint
            && arm_elbow_btm.tim_current == arm_elbow_btm.tim_setpoint
            && arm_elbow_top.tim_current == arm_elbow_top.tim_setpoint
            && arm_wrist.tim_current == arm_wrist.tim_setpoint
            && arm_finger.tim_current == arm_finger.tim_setpoint
        ) break;
        osDelay(50);
    }
}

void StartArmTask(void *argument)
{
    for(;;)
    {
        if (arm_ready) break;
        osDelay(50);
    }
    osDelay(1000);
    arm_motion_set(ARM_MOTION_IDLE);
    for(;;)
    {
        // !
        if (robotic_arm_h.motion_inner != robotic_arm_h.motion)
        {
            robotic_arm_h.motion_inner = robotic_arm_h.motion;
            robotic_arm_h.step = 0;
            robotic_arm_h.step_inner = 0;
        }
        else if (robotic_arm_h.step_inner < robotic_arm_h.step)
        {
            motion_set_inner(robotic_arm_h.motion_inner, robotic_arm_h.step_inner++);
            delay_until_reach();
        }
        osDelay(50);
    }
}

#endif