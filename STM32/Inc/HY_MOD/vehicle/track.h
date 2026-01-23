#pragma once
#include "HY_MOD/vehicle/basic.h"
#ifdef HY_MOD_STM32_VEHICLE

void vehicle_track_normal(VehicleParameter *vehicle, uint32_t unfind_ms);
void vehicle_track_rotate(VehicleParameter *vehicle, uint32_t unfind_ms);
void vehicle_search
(
    VehicleParameter *vehicle,
    VehicleDirection direction,
    VehicleMode reach_next,
    VehicleMode timeout_next,
    uint32_t unfind_ms
);

#endif