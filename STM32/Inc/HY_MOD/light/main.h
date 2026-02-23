#pragma once
#include "HY_MOD/light/basic.h"
#ifdef HY_MOD_LIGHT

void light_load(LightCtrl *light);
void light_button(LightCtrl *light, uint16_t GPIO_Pin);
void light_cd(LightCtrl *light);

#endif