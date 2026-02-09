#include "HY_MOD/infrared/basic.h"
#ifdef HY_MOD_IR_TRSM

const uint8_t ir_heran_fan_addr = 0x30;
const uint8_t ir_heran_fan_power = 0x88;

const IRTime ir_heran_fan = {
    .header_mark = 9000,
    .header_space = 4500,
    .bit_mark = 560,
    .bit_0 = 560,
    .bit_1 = 1690,
};

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
            ir->time = &ir_heran_fan;
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