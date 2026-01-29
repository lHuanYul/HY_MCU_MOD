#pragma once
#include "HY_MOD/dht/basic.h"
#ifdef HY_MOD_STM32_DHT

void dht_tim_PE_cb(DhtParametar *dht11, TIM_HandleTypeDef *htim);
void dht_tim_IC_cb(DhtParametar *dht11, TIM_HandleTypeDef *htim);

#endif