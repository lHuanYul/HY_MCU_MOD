#pragma once
#include "main/config.h"
#ifdef HY_MOD_TIMER_US

#include "tim.h"
#include "HY_MOD/main/free_rtos.h"
#include "HY_MOD/main/fn_state.h"

typedef struct TimerUSConst
{
    TIM_HandleTypeDef   *htimx;
    uint32_t            max_tick;
    // N us/tick
    uint32_t            tick_us;
} TimerUSConst;

typedef enum TimerUSMode
{
    TIMER_US_UNUSE,
    TIMER_US_READY,
    TIMER_US_USING,
} TimerUSMode;

typedef struct TimerUSChannel
{
    const uint32_t      ch;
    volatile uint32_t   *CCR;
    uint8_t             mode;
    uint32_t            compare;
    OsSmpParametar      os;
} TimerUSChannel;

typedef struct TimerUSParm
{
    const TimerUSConst const_h;
    union {
        struct {
            TimerUSChannel  ch1;
            TimerUSChannel  ch2;
            TimerUSChannel  ch3;
            TimerUSChannel  ch4;
            TimerUSChannel  ch5;
            TimerUSChannel  ch6;
        };
        TimerUSChannel     chX[6];
    };
} TimerUSParm;

extern TIM_HandleTypeDef htim5 __weak;
extern TimerUSParm timer_us_5;

/**
 * @brief 初始化 Timer 自由運行並啟動計數器
 */
Result timer_us_init(TimerUSParm *timer);

/**
 * @brief 阻塞型微秒延遲 (內部由 us 轉 tick，透過 Output Compare 中斷與信號量喚醒)
 * @param timer 計時器物件
 * @param us 延遲微秒數
 */
Result timer_us_delay(TimerUSParm *timer, uint32_t us);

#define TIMER_US_HAL_TIM_OC_DelayElapsedCB_CALL(timer, htim) \
    do { \
        if ( \
            INSTANCE_CHK((htim), (timer).const_h.htimx) \
        ) { \
            timer_us_oc_cb(&(timer), (htim)); \
        } \
    } while (0)
/**
 * @brief 供 HAL_TIM_OC_DelayElapsedCallback 調用的輸出比較中斷回呼
 */
void timer_us_oc_cb(TimerUSParm *timer, TIM_HandleTypeDef *htim);

#endif