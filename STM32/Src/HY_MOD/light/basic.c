#include "HY_MOD/light/basic.h"
#ifdef HY_MOD_LIGHT

#include "HY_MOD/main/variable_cal.h"

void light_trigger(LightCtrl *light, uint8_t act, uint8_t id)
{
    if (id > 8) return;
    switch (act)
    {
        case LIGHT_OFF:
        {
            BIT_SNG_SET(light->flags, id);
            break;
        }
        case LIGHT_ON:
        {
            BIT_SNG_CLR(light->flags, id);
            break;
        }
        case LIGHT_TOGGLE:
        {
            BIT_SNG_REV(light->flags, id);
            break;
        }
        default: return;
    }
}

#endif