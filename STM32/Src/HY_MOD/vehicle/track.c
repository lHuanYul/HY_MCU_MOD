#include "HY_MOD/vehicle/track.h"
#ifdef HY_MOD_STM32_VEHICLE

#include "HY_MOD/cmds.h"

/**
 * @brief 循跡模式
 */
void vehicle_track_normal(VehicleParameter *vehicle, uint32_t unfind_ms)
{
    if (vehicle->track.direction != VEHICLE_MOTION_BACKWARD)
        vehicle->track.direction = VEHICLE_MOTION_FORWARD;
    vehicle->dict_ref = vehicle->track.direction;
    vehicle->motor_left.mode_ref = CMD_WHEEL_B0_NORMAL;
    vehicle->motor_left.value_ref = vehicle->track.speed;
    vehicle->motor_right.mode_ref = CMD_WHEEL_B0_NORMAL;
    vehicle->motor_right.value_ref = vehicle->track.speed;
    if (vehicle->hall_front.state != ADC_HALL_STATE_NONE)
        vehicle->last_tick_on_mag = HAL_GetTick();
    else
    {
        if (
            vehicle->hall_left.state != ADC_HALL_STATE_NONE &&
            vehicle->hall_right.state == ADC_HALL_STATE_NONE
        ) {
            vehicle->last_tick_on_mag = HAL_GetTick();
            vehicle->motor_left.value_ref = 0;
        }
        else if (
            vehicle->hall_left.state == ADC_HALL_STATE_NONE &&
            vehicle->hall_right.state != ADC_HALL_STATE_NONE
        ) {
            vehicle->last_tick_on_mag = HAL_GetTick();
            vehicle->motor_right.value_ref = 0;
        }
        else if (
            vehicle->hall_left.state != ADC_HALL_STATE_NONE &&
            vehicle->hall_right.state != ADC_HALL_STATE_NONE
        ) {
            vehicle->last_tick_on_mag = HAL_GetTick();
        }
        else if (
            vehicle->hall_left.state == ADC_HALL_STATE_NONE &&
            vehicle->hall_right.state == ADC_HALL_STATE_NONE &&
            (HAL_GetTick() - vehicle->last_tick_on_mag >= unfind_ms)
        ) {
            vehicle_set_mode(vehicle, VEHICLE_MODE_SEARCH_LEFT);
        }
    }
}

/**
  * @brief AGV 原地旋轉直到對準方向，根據強磁計數更新 AGV 方向資料
  */
void vehicle_track_rotate(VehicleParameter *vehicle, uint32_t unfind_ms)
{
    if (vehicle->rotate.direction != VEHICLE_MOTION_CLOCKWISE)
        vehicle->rotate.direction = VEHICLE_MOTION_C_CLOCKWISE;
    vehicle->dict_ref = vehicle->rotate.direction;
    vehicle->motor_left.mode_ref = CMD_WHEEL_B0_NORMAL;
    vehicle->motor_left.value_ref = vehicle->rotate.speed;
    vehicle->motor_right.mode_ref = CMD_WHEEL_B0_NORMAL;
    vehicle->motor_right.value_ref = vehicle->rotate.speed;
    if (vehicle->hall_front.state != ADC_HALL_STATE_NONE)
    {
        vehicle->last_tick_on_mag = HAL_GetTick();
        if (vehicle->front_last_state == ADC_HALL_STATE_NONE)
            vehicle->rotate.need_count--;
    }
    vehicle->front_last_state = vehicle->hall_front.state;
    if (vehicle->rotate.need_count <= 0)
    {
        vehicle_set_mode(vehicle, VEHICLE_MODE_TRACK);
    }
    if (HAL_GetTick() - vehicle->last_tick_on_mag >= unfind_ms)
    {
        vehicle_set_mode(vehicle, VEHICLE_MODE_END);
        return;
    }
}

void vehicle_search
(
    VehicleParameter *vehicle,
    VehicleDirection direction,
    VehicleMode reach_next,
    VehicleMode timeout_next,
    uint32_t unfind_ms
) {
    vehicle->search.direction = direction;
    vehicle->dict_ref = direction;
    vehicle->motor_left.mode_ref = CMD_WHEEL_B0_NORMAL;
    vehicle->motor_left.value_ref = vehicle->search.speed;
    vehicle->motor_right.mode_ref = CMD_WHEEL_B0_NORMAL;
    vehicle->motor_right.value_ref = vehicle->search.speed;
    if (vehicle->hall_front.state != ADC_HALL_STATE_NONE)
    {
        vehicle->last_tick_on_mag = HAL_GetTick();
        vehicle_set_mode(vehicle, reach_next);
    }
    else if (HAL_GetTick() - vehicle->last_tick_on_mag >= unfind_ms)
    {
        vehicle_set_mode(vehicle, timeout_next);
    }
}

#endif