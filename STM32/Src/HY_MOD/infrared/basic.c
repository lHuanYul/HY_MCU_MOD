#include "HY_MOD/infrared/basic.h"
#ifdef HY_MOD_IR_TRSM

#include "tim.h"
#include "HY_MOD/main/tim.h"

IRParameter infrared_h = {
    .const_h = {
        .Master_htimx = &htim17,
        .Master_TIM_CHANNEL_x = TIM_CHANNEL_1,
        .Master_tim_clk = &tim_clk_APB1,
        // PC1     ------> TIM15_CH1
        .Slave_htimx = &htim15,
        .Slave_TIM_CHANNEL_x = TIM_CHANNEL_1,
    },
};

const uint8_t ir_heran_fan_addr = 0x30;
const uint8_t ir_heran_fan_power = 0x88;

void ir_code_gen(IRParameter *ir, IRCodes code)
{
    uint8_t idx = 0;
    #define CODE_WRITE(s, d) \
    do { \
        ir->list[idx].size = (uint8_t)(s); \
        ir->list[idx].data = (uint32_t)(d); \
        idx++; \
    } while(0)
    switch (code)
    {
        case IR_CODE_N: break;
        case IR_CODE_HERAN_FAN_POWER:
        {
            CODE_WRITE(8, ir_heran_fan_addr);
            CODE_WRITE(8, ~ir_heran_fan_addr);
            CODE_WRITE(8, ir_heran_fan_power);
            CODE_WRITE(8, ~ir_heran_fan_power);
            ir->len = idx;
            break;
        }
        case IR_CODE_HERAN_FAN_INCREASE:
        {
            break;
        }
        case IR_CODE_HERAN_FAN_DECREASE:
        {
            break;
        }
    }
}

#endif