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
    __HAL_TIM_DISABLE(htimx);
    TIM_TypeDef *Instance = htimx->Instance;
    Instance->CCER &= ~TIM_CCER_CC1E;
    uint32_t tmpccmr1 = Instance->CCMR1;

    if (dht11->tim_mode_pwm)
    {
        // 清除 CC1S (Bit 1:0) -> 設為 00 (Output)
        // 清除 OC1M (Bit 6:4) -> 設為 110 (PWM Mode 1) 或 111 (PWM Mode 2)
        tmpccmr1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M); 
        
        // 修改後: PWM Mode 2 (先 Low 再 High)
        tmpccmr1 |= (0x7U << TIM_CCMR1_OC1M_Pos);

        // 【修正點】: 切回輸出時，必須清除 Input 留下的極性設定！
        // 確保 PWM 是 Active High (CC1P=0)，且關閉互補輸出相關位元 (CC1NP=0)
        Instance->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);

        __HAL_TIM_DISABLE_IT(htimx, TIM_IT_CC1);   // 關閉 捕獲中斷 (避免 PWM 比較時誤觸發)
        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_UPDATE); // 清除舊的 Update 旗標
        __HAL_TIM_ENABLE_IT(htimx, TIM_IT_UPDATE); // 開啟 Update 中斷

        __HAL_TIM_SET_AUTORELOAD(htimx, 18030);
        __HAL_TIM_SET_COMPARE(htimx, dht11->const_h.TIM_CHANNEL_x, 18000);
    }
    else
    {
        // 清除 CC1S -> 設為 01 (Input, CC1 mapped on TI1)
        // 清除 IC1F (Input Filter) -> 設為 0 (無濾波，若有雜訊可設為 0x3)
        tmpccmr1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_IC1F);
        
        // 設定 CC1S 為 01 (Input)
        tmpccmr1 |= TIM_CCMR1_CC1S_0; 

        // 3. 設定雙邊沿觸發 (Both Edges) - 重要！
        // CCER 暫存器控制極性 (CC1P 與 CC1NP)
        // 00: 上升, 01: 下降, 11: 雙邊 (視系列而定，通常是 CC1P=1, CC1NP=1)
        // 在 HAL 庫中 BOTHEDGE 通常對應：TIM_CCER_CC1P | TIM_CCER_CC1NP
        Instance->CCER |= (TIM_CCER_CC1P | TIM_CCER_CC1NP);

        __HAL_TIM_DISABLE_IT(htimx, TIM_IT_UPDATE); // 關閉 Update 中斷 (任務完成，不需要再進來了)
        __HAL_TIM_CLEAR_FLAG(htimx, TIM_FLAG_CC1); // 清除舊的 CC1 旗標
        __HAL_TIM_ENABLE_IT(htimx, TIM_IT_CC1); // 開啟 Capture 中斷 (開始收網)

        __HAL_TIM_SET_AUTORELOAD(htimx, UINT32_MAX);

        dht11->state = DHT_STATE_IGNORE;
    }

    Instance->CCMR1 = tmpccmr1;
    Instance->CCER |= TIM_CCER_CC1E;
    __HAL_TIM_SET_COUNTER(htimx, 0);
    dht11->last_cnt = 0;
    __HAL_TIM_ENABLE(htimx);
}

#endif