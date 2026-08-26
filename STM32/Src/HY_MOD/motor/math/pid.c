#include "HY_MOD/motor/math/pid.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/variable_cal.h"

void PI_run(PID_CTRL *pid)
{
    pid->Error = pid->reference - pid->feedback;
    pid->Term_p = pid->Kp * pid->Error;
    pid->Term_i_ori = pid->Term_i_fix + (pid->Ki * pid->Term_p);
    pid->out_ori = pid->Term_p + pid->Term_i_ori;

    if (pid->out_ori > pid->max)
    {
        pid->saturation = 1;
        pid->out_fix = pid->max;
    }
    else if (pid->out_ori < pid->min)
    {
        pid->saturation = 1;
        pid->out_fix = pid->min;
    }
    else
    {
        pid->saturation = 0;
        pid->out_fix = pid->out_ori;
        pid->Term_i_fix = pid->Term_i_ori;
    }
}

void PID_run(PID_CTRL *pid)
{
    pid->Error = pid->reference - pid->feedback;
    pid->Term_p = pid->Kp * pid->Error;
    pid->Term_i_ori = pid->Term_i_fix + (pid->Ki * pid->Term_p);
    pid->Term_d = pid->Kd * (pid->Error - pid->Error_prev);
    pid->out_ori = pid->Term_p + pid->Term_i_ori + pid->Term_d;

    if (pid->out_ori > pid->max)
    {
        pid->saturation = 1;
        pid->out_fix = pid->max;
    }
    else if (pid->out_ori < pid->min)
    {
        pid->saturation = 1;
        pid->out_fix = pid->min;
    }
    else
    {
        pid->saturation = 0;
        pid->out_fix = pid->out_ori;
        pid->Term_i_fix = pid->Term_i_ori;
    }
    pid->Error_prev = pid->Error;
}

void PID_reset(PID_CTRL *pid)
{
    pid->Error = 0.0f;
    pid->Error_prev = 0.0f;
    pid->Term_p = 0.0f;
    pid->Term_i_ori = 0.0f;
    pid->Term_i_fix = 0.0f;
    pid->Term_d = 0.0f;
    pid->out_ori = 0.0f;
    pid->out_fix = 0.0f;
    pid->saturation = 0;
}

#endif