#include "HY_MOD/dht11/basic.h"
#ifdef HY_MOD_STM32_DHT11

#include "HY_MOD/main/tim.h"
#include "tim.h"

Dht11Parametar dht11_h = {
    .const_h = {
        .htimx = &htim2,
        .TIM_CHANNEL_x = TIM_CHANNEL_1,
        .HAL_TIM_ACTIVE_CHANNEL_x = HAL_TIM_ACTIVE_CHANNEL_1,
        .tim_clk = &tim_clk_APB1,
        .gpio = {
            .GPIOx = GPIOA,
            .GPIO_Pin_x = GPIO_PIN_0,
        },
    },
};

void dh11_tim_mode_switch(Dht11Parametar *dht11)
{
    TIM_HandleTypeDef *htimx = dht11->const_h.htimx;
    TIM_TypeDef *Instance = htimx->Instance;

    __HAL_TIM_DISABLE(htimx);
    Instance->CCER &= ~TIM_CCER_CC1E;
    __HAL_TIM_DISABLE_IT(htimx, TIM_IT_UPDATE | TIM_IT_CC1);
    __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_UPDATE | TIM_FLAG_CC1);

    uint32_t tmpccmr1 = Instance->CCMR1;
    uint32_t tmpccer  = Instance->CCER;

    tmpccmr1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M | TIM_CCMR1_IC1F);
    tmpccer &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);

    if (dht11->tim_mode_pwm)
    {   // ----------------- PWM 發送模式 -----------------
        // 設定為 PWM Mode 2 (111): 
        // 行為: CNT < CCR 時為 Inactive (Low), CNT > CCR 時為 Active (High/Float)
        // 目的: 拉低電位 18ms (Start Signal) -> 放開拉高 (Wait Response) -> Update 中斷
        tmpccmr1 |= (0x7U << TIM_CCMR1_OC1M_Pos);
        // ! 不可移除 否則CPU鎖定
        Instance->CCMR1 = tmpccmr1;
        
        // 設定 ARR 與 CCR
        // ARR = 18030us (整個週期), CCR = 18000us (拉低時間)
        // 18ms Low -> 30us High -> Update Event
        #define SIG_LEN 18000 // 18000 5000000
        __HAL_TIM_SET_AUTORELOAD(htimx, SIG_LEN);
        __HAL_TIM_SET_COMPARE(htimx, TIM_CHANNEL_1, SIG_LEN - 1);

        // 預備開啟的中斷: 只開 Update (週期結束時切換模式)
        __HAL_TIM_ENABLE_IT(htimx, TIM_IT_UPDATE);

        dht11->state = DHT_STATE_SEND;
    }
    else
    {   // ----------------- Input Capture 接收模式 -----------------
        // 設定為 Input (CC1S = 01), 映射到 TI1
        tmpccmr1 |= TIM_CCMR1_CC1S_0;

        // 設定雙邊沿觸發 (Both Edges)
        // 為了測量 High Pulse Width，必須同時捕捉上升與下降緣
        tmpccer |= (TIM_CCER_CC1P | TIM_CCER_CC1NP);

        // 設定 ARR 為最大值，防止測量途中溢位重置
        __HAL_TIM_SET_AUTORELOAD(htimx, UINT32_MAX);

        // 預備開啟的中斷: 只開 Capture (捕捉邊緣)
        __HAL_TIM_ENABLE_IT(htimx, TIM_IT_CC1);

        // 狀態機重置
        dht11->state = DHT_STATE_RESPONSE; // 準備接收 Response
    }

    Instance->CCMR1 = tmpccmr1;
    Instance->CCER  = tmpccer;

    __HAL_TIM_SET_COUNTER(htimx, 0);
    dht11->last_cnt = 0;

    Instance->CCER |= TIM_CCER_CC1E;
    __HAL_TIM_ENABLE(htimx);
}

#endif