#include "HY_MOD/infrared/main.h"
#ifdef HY_MOD_IR_TRSM

#include "main/infrared.h"

#define TASK_DELAY_MS 100
void StartIRTask(void *argument)
{
    IRParameter *ir = &infrared_h;
    ir_code_gen(ir, IR_CODE_HERAN_FAN_POWER);
    const uint32_t osPeriod = pdMS_TO_TICKS(TASK_DELAY_MS);
    uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    for (;;)
    {
        if (ir->cd > 0)
        {
            ir->cd--;
            if (ir->cd == 0) ir->state == IR_STATE_IDLE;
        }
        else if (ir->state == IR_STATE_CD) ir->cd = 10;
        osDelayUntil(next_wake);
        next_wake += osPeriod;
    }
}

#endif