#pragma once

#include "main/config.h"

void HAL_TIM_PeriodElapsedCallback_OWN(TIM_HandleTypeDef *htim);

extern uint32_t tim_clk_APB1;
extern uint32_t tim_clk_APB2;

void INIT_OWN_TIM(void);
