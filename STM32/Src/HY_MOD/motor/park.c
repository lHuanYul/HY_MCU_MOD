#include "HY_MOD/motor/park.h"
#ifdef HY_MOD_STM32_MOTOR

// Id = Ialpha * cos(theta) + Ibata * sin(theta)
// Iq = Ibata * cos(theta) - Ialpha * sin(theta)
inline void PARK_run(PARK *park)
{
    park->Ds = (park->Alpha * park->Cos) + (park->Beta  * park->Sin);
    park->Qs = (park->Beta  * park->Cos) - (park->Alpha * park->Sin);
}

// Valpha = Vd * cos(theta) - Vq * sin(theta)
// Vbata = Vd * sin(theta) + Vq * cos(theta)
inline void IPARK_run(IPARK *ipark)
{
    ipark->Alpha = (ipark->Vdref * ipark->Cos) - (ipark->Vqref * ipark->Sin);
    ipark->Beta  = (ipark->Vdref * ipark->Sin) + (ipark->Vqref * ipark->Cos);
}

#endif