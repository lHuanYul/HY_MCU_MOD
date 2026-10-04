#include "HY_MOD/timer/us/basic.h"
#ifdef HY_MOD_TIMER_US

TimerUSParm timer_us_5 = {
    .const_h = {
        .htimx      = &htim5,
        .max_tick   = TIM5_ARR_MAX,
        .tick_us    = 2,
    },
    .ch1 = {
        .ch     = TIM_CHANNEL_1,
        .mode   = TIMER_US_READY,
        .os.attr = {
            .name = "TimUS5Ch1Sem"
        },
    },
    .ch2 = {
        .ch     = TIM_CHANNEL_2,
        .mode   = TIMER_US_READY,
        .os.attr = {
            .name = "TimUS5Ch2Sem"
        },
    },
    .ch3 = {
        .ch     = TIM_CHANNEL_3,
        .mode   = TIMER_US_READY,
        .os.attr = {
            .name = "TimUS5Ch3Sem"
        },
    },
    .ch4 = {
        .ch     = TIM_CHANNEL_4,
        .mode   = TIMER_US_READY,
        .os.attr = {
            .name = "TimUS5Ch4Sem"
        },
    },
};

/**
 * @brief 初始化計時器 Free-Running 模式
 */
Result timer_us_init(TimerUSParm *timer)
{
    uint8_t i;
    for (i = 0; i < 6; i++)
        timer->chX[i].CCR = TIMER_GET_CCR_ADDR(timer->const_h.htimx->Instance, timer->chX[i].ch);
    TIM_HandleTypeDef *htim = timer->const_h.htimx;
    // 啟動 Base 計數器
    RESULT_HAL_CHECK_HANDLE(HAL_TIM_Base_Start(htim));
    return RESULT_OK(timer);
}

/**
 * @brief 尋找目前閒置的輸出比較通道
 */
static Result timer_us_get_free_channel(TimerUSParm *timer)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (timer->chX[i].mode == TIMER_US_READY)
        {
            timer->chX[i].mode = TIMER_US_USING;
            return RESULT_OK(&timer->chX[i]);
        }
    }
    return RESULT_ERROR(RESULT_ERROR_FULL);
}

/**
 * @brief 微秒延遲函式 (us 轉 tick，支援多任務同時等待)
 */
Result timer_us_delay(TimerUSParm *timer, uint32_t us)
{
    if (timer->const_h.tick_us == 0)
        return RESULT_ERROR(RESULT_ERROR_PARAM_OUT_OF_RANGE);

    // 1. us 轉 tick (向上取整避免延遲不足)
    uint32_t ticks = (us + timer->const_h.tick_us - 1) / timer->const_h.tick_us;
    if (ticks == 0) return RESULT_OK(timer);

    TIM_HandleTypeDef *htim = timer->const_h.htimx;
    Result res = timer_us_get_free_channel(timer);
    if (RESULT_CHECK_ERR(res))
    {
        for (volatile uint32_t i = 0; i < us * 18; i++) __NOP();
        return RESULT_OK(timer);
    }
    TimerUSChannel *ch_inst = RESULT_UNWRAP(res);

    // 3. 計算目標 Compare 值 (自動溢位回繞)
    uint32_t current_cnt = __HAL_TIM_GET_COUNTER(htim);
    uint32_t target_compare = (current_cnt + ticks) & timer->const_h.max_tick;
    ch_inst->compare = target_compare;

    // 4. 設定比較暫存器並啟動 Output Compare 中斷
    *ch_inst->CCR = target_compare;
    __HAL_TIM_CLEAR_FLAG(htim, (TIM_FLAG_CC1 << (ch_inst->ch >> 2)));
    HAL_TIM_OC_Start_IT(htim, ch_inst->ch);

    if (ch_inst->os.id == NULL)
    {
        Error_Handler();
    }
    osSemaphoreAcquire(ch_inst->os.id, osWaitForever);

    return RESULT_OK(timer);
}

/**
 * @brief Output Compare 中斷喚醒邏輯 (置於中斷回呼調用)
 */
void timer_us_oc_isr(TimerUSParm *timer, TIM_HandleTypeDef *htim)
{
    if (htim != timer->const_h.htimx) return;

    for (uint8_t i = 0; i < 6; i++)
    {
        TimerUSChannel *ch = &timer->chX[i];
        if (ch->mode == TIMER_US_USING)
        {
            uint32_t flag = TIM_FLAG_CC1 << (ch->ch >> 2);
            if (__HAL_TIM_GET_FLAG(htim, flag) != RESET)
            {
                __HAL_TIM_CLEAR_IT(htim, (TIM_IT_CC1 << (ch->ch >> 2)));
                HAL_TIM_OC_Stop_IT(htim, ch->ch);

                ch->mode = TIMER_US_READY;

                // 釋放信號量喚醒對應 Task
                if (ch->os.id != NULL) osSemaphoreRelease(ch->os.id);
            }
        }
    }
}

#endif