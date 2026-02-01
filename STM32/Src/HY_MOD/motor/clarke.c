#include "HY_MOD/motor/clarke.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/motor/basic.h"

// Ialpha = Ia
// Ibata = (Ia+2Ib)/(根號3)
inline void CLARKE_run_ideal(CLARKE *clarke)
{
    clarke->Alpha = clarke->As;
    clarke->Beta = ONE_DIV_SQRT3 * (clarke->As + clarke->Bs * 2.0f);
}

// Ialpha = 2/3Ia - 1/3Ib - 1/3Ic
// Ibata = (Ib-Ic)/(根號3)
inline void CLARKE_run_nideal(CLARKE *clarke)
{
    clarke->Alpha = DIV_1_3 * (2.0f * clarke->As - clarke->Bs - clarke->Cs);
    clarke->Beta  = ONE_DIV_SQRT3 * (clarke->Bs - clarke->Cs);
}

#endif