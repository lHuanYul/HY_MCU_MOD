#include "HY_MOD/motor/pi.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"

void PI_run(PI_CTRL *pi)
{
    pi->Error = pi->reference - pi->feedback;
    pi->Term_p = pi->Kp * pi->Error;
    pi->Term_i_ori = pi->Term_i_fix + (pi->Ki * pi->Term_p);
    pi->out_ori = pi->Term_p + pi->Term_i_ori;
    if (pi->out_ori > pi->max)
    {
        pi->saturation = 1;
        pi->out_fix = pi->max;
    }
    else if (pi->out_ori < pi->min)
    {
        pi->saturation = 1;
        pi->out_fix = pi->min;
    }
    else
    {
        pi->saturation = 0;
        pi->out_fix = pi->out_ori;
        pi->Term_i_fix = pi->Term_i_ori;
    }

    // if (pi->out_fix == pi->out_ori)
    // {
    //     pi->Term_i_fix = pi->Term_i_last + pi->Ki * pi->Term_p;
    //     // pi->Term_i_fix = pi->Term_i_last + pi->Ki * pi->Error;
    // }
    // else
    // {
    //     pi->Term_i_fix = pi->Term_i_last;
    // }
    // pi->Term_i_last = pi->Term_i_fix;
    // pi->out_ori = pi->Term_p + pi->Term_i_fix;
    // pi->out_fix = pi->out_ori;
    // VAR_CLAMPF(pi->out_fix, pi->min, pi->max);
    // pi->saturation = (pi->out_fix != pi->out_ori) ? 1 : 0;
}

void PI_reset(PI_CTRL *pi)
{pi->Error = 0.0f;
    pi->Term_p = 0.0f;
    pi->Term_i_ori = 0.0f;
    pi->Term_i_fix = 0.0f;
    pi->out_ori = 0.0f;
    pi->out_fix = 0.0f;
    pi->saturation = 0;
}

#endif