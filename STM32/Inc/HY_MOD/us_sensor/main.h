#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_USS

#include "HY_MOD/main/fn_state.h"

typedef struct USSConst
{
    TIM_HandleTypeDef*  htimx;
    uint32_t            TIM_CHANNEL_x;
    HAL_TIM_ActiveChannel HAL_TIM_ACTIVE_CHANNEL_x;
    GPIO_TypeDef*       trig_GPIOx;
    uint16_t            trig_GPIO_Pin_x;
    GPIO_TypeDef*       echo_GPIOx;
    uint16_t            echo_GPIO_Pin_x;
    float               warning_gate;
    float               danger_gate;
} USSConst;

typedef enum USSState
{
    USS_STATE_STOP,
    USS_STATE_RUNNING,
    USS_STATE_TRIGGER,
    USS_STATE_WAITING,
} USSState;

typedef enum USSStatus
{
    USS_STATUS_SAVE,
    USS_STATUS_WARNING,
    USS_STATUS_DANGER,
} USSStatus;

typedef struct USSensor
{
    const USSConst const_h;
    USSState state;
    uint32_t time;
    uint32_t distance;
    USSStatus status;
} USSensor;

extern USSensor us_sensor_head;

Result us_sensor_start(void);
Result us_sensor_tri_off(void);
Result us_sensor_overflow(void);
Result us_sensor_echo(void);

#endif