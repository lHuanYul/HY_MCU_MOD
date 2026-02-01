#include "HY_MOD/voice_ysm3/basic.h"
#ifdef HY_MOD_STM32_YSM3

#include "HY_MOD/main/variable_cal.h"

YSM3Parametar ysm3_h = {
    .const_h = {
        .A1 = { GPIOC, GPIO_PIN_5 },
        .A2 = { GPIOC, GPIO_PIN_6 },
        .A3 = { GPIOC, GPIO_PIN_8 },
        .A4 = { GPIOC, GPIO_PIN_9 },
        .A5 = { GPIOB, GPIO_PIN_8 },
        .CN = {},
        .ST = {},
        .BY = { GPIOB, GPIO_PIN_9 },
    },
};

static void ysm3_reset(YSM3Parametar *ysm3)
{
    GPIO_WRITE(ysm3->const_h.A1, 1);
    GPIO_WRITE(ysm3->const_h.A2, 1);
    GPIO_WRITE(ysm3->const_h.A3, 1);
    GPIO_WRITE(ysm3->const_h.A4, 1);
    GPIO_WRITE(ysm3->const_h.A5, 1);
}

void ysm3_init(YSM3Parametar *ysm3)
{
    ysm3_reset(ysm3);
    osDelay(1000);
}

void ysm3_load(YSM3Parametar *ysm3)
{
    #define YSM3_BIT(x) (BIT_SNG_GET(ysm3->select, (x)) ? 0 : 1)
    if (ysm3->select == 0) return;
    GPIO_WRITE(ysm3->const_h.A1, YSM3_BIT(0));
    GPIO_WRITE(ysm3->const_h.A2, YSM3_BIT(1));
    GPIO_WRITE(ysm3->const_h.A3, YSM3_BIT(2));
    GPIO_WRITE(ysm3->const_h.A4, YSM3_BIT(3));
    GPIO_WRITE(ysm3->const_h.A5, YSM3_BIT(4));
    osDelay(100);
    ysm3_reset(ysm3);
    osDelay(400);
    do osDelay(250); while(GPIO_IF_SET(ysm3->const_h.BY));
    ysm3->select = 0;
}

#endif