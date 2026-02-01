#include "HY_MOD/motor/pi.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"

void PI_run(PI_CTRL *pi)
{
    pi->Error = pi->reference - pi->feedback;
    pi->Term_p = pi->Kp * pi->Error;
    if (pi->out == pi->out_origin)
    {
        pi->Term_i = pi->Term_i_last + pi->Ki * pi->Term_p;
        // pi->Term_i = pi->Term_i_last + pi->Ki * pi->Error;
    }
    else
    {
        pi->Term_i = pi->Term_i_last;
    }
    pi->Term_i_last = pi->Term_i;
    pi->out_origin = pi->Term_p + pi->Term_i;
    pi->out = pi->out_origin;
    VAR_CLAMPF(pi->out, pi->min, pi->max);
    pi->saturation = (pi->out != pi->out_origin) ? 1 : 0;
}

void PI_reset(PI_CTRL *pi)
{
    pi->Term_i = 0.0f;
    pi->Term_i_last = 0.0f;
    pi->out_origin = 0.0f;
    pi->out = 0.0f;
    pi->saturation = 0;
}

#endif