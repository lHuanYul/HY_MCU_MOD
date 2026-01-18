#pragma once
#include "HY_MOD/dht11/basic.h"
#ifdef HY_MOD_STM32_DHT11

void dht11_tim_PE_cb(Dht11Parametar *dht11, TIM_HandleTypeDef *htim);
void dht11_tim_IC_cb(Dht11Parametar *dht11, TIM_HandleTypeDef *htim);

#endif