#include "HY_MOD/infrared/basic.h"
#ifdef HY_MOD_IR_TRSM

#include "HY_MOD/main/variable_cal.h"

const uint8_t ir_heran_fan_addr = 0x30;
const uint8_t ir_heran_fan_power = 0x88;

const IRTime ir_heran_fan = {
    .header_mark = 9000,
    .header_space = 4500,
    .bit_mark = 560,
    .bit_0 = 560,
    .bit_1 = 1690,
};

static void code_load(IRParameter *ir, const IRTime *time, uint8_t *id, uint32_t data, uint8_t len)
{
    uint8_t i, t;
    if (((*id) + len) > (IR_CODE_LIST_MAX / 2) || len > 32) Error_Handler();
    for (i = 0; i < len; i++)
    {
        ir->data[(*id) + i * 2] = 3;
        if(BIT_SNG_CHK(data, i))
            t = 5;
        else
            t = 4;
        ir->data[(*id) + i * 2 + 1] = t;
    }
    (*id) += len;
}

void ir_code_gen(IRParameter *ir, IRCodes code)
{
    uint8_t idx = 0;
    #define CODE_LODE(time, data) code_load(ir, (time), &idx, (data), sizeof(data));
    switch (code)
    {
        case IR_CODE_N: break;
        case IR_CODE_HERAN_FAN_POWER:
        {
            ir->data[0] = 1;
            ir->data[1] = 2;
            idx += 2;
            CODE_LODE(&ir_heran_fan, ir_heran_fan_addr);
            CODE_LODE(&ir_heran_fan, ~ir_heran_fan_addr);
            CODE_LODE(&ir_heran_fan, ir_heran_fan_power);
            CODE_LODE(&ir_heran_fan, ~ir_heran_fan_power);
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

uint16_t ir_code_time(uint8_t data)
{
    switch (data)
    {
        case 0:
        {
            break;
        }
        case 1: return ir_heran_fan.header_mark;
        case 2: return ir_heran_fan.header_space;
        case 3: return ir_heran_fan.bit_mark;
        case 4: return ir_heran_fan.bit_0;
        case 5: return ir_heran_fan.bit_1;
        default: break;
    }
    return 10000;
}

#endif