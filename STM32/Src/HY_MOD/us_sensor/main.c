#include "HY_MOD/us_sensor/main.h"
#ifdef HY_MOD_STM32_USS

#include "tim.h"
#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/fdcan/pkt_write.h"

USSensor us_sensor_head = {
    .const_h = {
        .htimx = &htim4,
        .TIM_CHANNEL_x = TIM_CHANNEL_1,
        .HAL_TIM_ACTIVE_CHANNEL_x = HAL_TIM_ACTIVE_CHANNEL_1,
        .trig_GPIOx = GPIOC,
        .trig_GPIO_Pin_x = GPIO_PIN_5,
        .echo_GPIOx = GPIOC,
        .echo_GPIO_Pin_x = GPIO_PIN_6,
        .warning_gate = 800.0f,
        .danger_gate = 550.0f,
    },
    .state = USS_STATE_STOP,
};

uint32_t hyrun[5] = {0};

Result us_sensor_start(void)
{
    USSensor *uss = &us_sensor_head;
    if (uss->state != USS_STATE_WAITING) return RESULT_ERROR(RES_ERR_INVALID);
    hyrun[0]++;
    uss->state = USS_STATE_RUNNING;
    HAL_GPIO_WritePin(uss->const_h.trig_GPIOx, uss->const_h.trig_GPIO_Pin_x, GPIO_PIN_SET);
    return RESULT_OK(uss);
}

const Result us_sensor_enable(void)
{
    USSensor *uss = &us_sensor_head;
    if (uss->state != USS_STATE_STOP) return RESULT_ERROR(RES_ERR_BUSY);
    HAL_TIM_Base_Start_IT(uss->const_h.htimx);
    HAL_TIM_PWM_Start_IT(uss->const_h.htimx, uss->const_h.TIM_CHANNEL_x);
    __HAL_TIM_SET_COMPARE(uss->const_h.htimx, uss->const_h.TIM_CHANNEL_x, 10);
    uss->state = USS_STATE_WAITING;
    us_sensor_start();
    return RESULT_OK(uss);
}

Result us_sensor_tri_off(void)
{
    USSensor *uss = &us_sensor_head;
    if (uss->state != USS_STATE_RUNNING) return RESULT_ERROR(RES_ERR_INVALID);
    hyrun[1]++;
    HAL_GPIO_WritePin(uss->const_h.trig_GPIOx, uss->const_h.trig_GPIO_Pin_x, GPIO_PIN_RESET);
    return RESULT_OK(uss);
}

Result us_sensor_overflow(void)
{
    USSensor *uss = &us_sensor_head;
    if (uss->state == USS_STATE_RUNNING)
    {
        hyrun[3]++;
        uss->state = USS_STATE_WAITING;
        HAL_GPIO_WritePin(uss->const_h.trig_GPIOx, uss->const_h.trig_GPIO_Pin_x, GPIO_PIN_RESET);
        uss->time = UINT32_MAX;
        uss->distance = UINT32_MAX;
        us_sensor_start();
        return RESULT_OK(uss);
    }
    else if (uss->state == USS_STATE_STOP)
    {
        hyrun[4]++;
        uss->state = USS_STATE_WAITING;
        HAL_GPIO_WritePin(uss->const_h.trig_GPIOx, uss->const_h.trig_GPIO_Pin_x, GPIO_PIN_RESET);
        us_sensor_start();
        return RESULT_OK(uss);
    }
    else return RESULT_ERROR(RES_ERR_INVALID);
}

int text_a = 0;
Result us_sensor_echo(void)
{
    text_a++;
    USSensor *uss = &us_sensor_head;
    if (uss->state != USS_STATE_RUNNING) return RESULT_ERROR(RES_ERR_INVALID);
    hyrun[2]++;
    uss->state = USS_STATE_STOP;
    const USSConst *const_h = &uss->const_h;
    HAL_GPIO_WritePin(const_h->trig_GPIOx, const_h->trig_GPIO_Pin_x, GPIO_PIN_RESET);
    uss->time = __HAL_TIM_GET_COUNTER(uss->const_h.htimx);
    uss->distance = (uint32_t)((float)uss->time * 0.343f / 2.0f);  // uint:mm
    if (uss->distance <= const_h->danger_gate)
        uss->status = USS_STATUS_DANGER;
    else if (uss->distance <= const_h->warning_gate)
        uss->status = USS_STATUS_WARNING;
    else
        uss->status = USS_STATUS_SAVE;
    return RESULT_OK(uss);
}

int tick_text = 0;
void StartUsTask(void *argument)
{
    us_sensor_enable();
    for(;;)
    {
        tick_text++;

        osDelay(1);
    }
}

#endif