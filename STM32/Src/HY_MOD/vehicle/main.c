#include "HY_MOD/vehicle/main.h"
#ifdef HY_MOD_STM32_VEHICLE

#include "HY_MOD/vehicle/track.h"
#include "HY_MOD/vehicle/search.h"
#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/fdcan/pkt_write.h"

// motor mode & spd update
static void mode_update(VehicleParameter *vehicle)
{
    switch (vehicle->mode)
    {
        case VEHICLE_MODE_UNK: break;
        case VEHICLE_MODE_END:
        {
            vehicle->motor_left.mode_ref = CMD_WHEEL_B0_BREAK;
            vehicle->motor_right.mode_ref = CMD_WHEEL_B0_BREAK;
            break;
        }
        case VEHICLE_MODE_FREE:
        {
            vehicle->dict_ref = vehicle->free.direction;
            vehicle->motor_left.mode_ref = CMD_WHEEL_B0_NORMAL;
            vehicle->motor_left.value_ref = vehicle->free.speed;
            vehicle->motor_right.mode_ref = CMD_WHEEL_B0_NORMAL;
            vehicle->motor_right.value_ref = vehicle->free.speed;
            break;
        }
        case VEHICLE_MODE_TRACK:
        {
            vehicle_track_normal(vehicle, 1000);
            break;
        }
        case VEHICLE_MODE_T_ROTATE:
        {
            vehicle_track_rotate(vehicle, 5000);
            break;
        }
        case VEHICLE_MODE_SEARCH_LEFT:
        {
            vehicle_search(
                vehicle,
                VEHICLE_MOTION_C_CLOCKWISE,
                VEHICLE_MODE_SEARCH_L_RET,
                VEHICLE_MODE_SEARCH_RIGHT,
                2000
            );
            break;
        }
        case VEHICLE_MODE_SEARCH_RIGHT:
        {
            vehicle_search(
                vehicle,
                VEHICLE_MOTION_CLOCKWISE,
                VEHICLE_MODE_SEARCH_R_RET,
                VEHICLE_MODE_END,
                4000
            );
            break;
        }
        case VEHICLE_MODE_SEARCH_L_RET:
        {
            vehicle_search(
                vehicle,
                VEHICLE_MOTION_FORWARD,
                VEHICLE_MODE_SEARCH_L_RET_FIX,
                VEHICLE_MODE_SEARCH_LEFT,
                2000
            );
            break;
        }
        case VEHICLE_MODE_SEARCH_R_RET:
        {
            vehicle_search(
                vehicle,
                VEHICLE_MOTION_FORWARD,
                VEHICLE_MODE_SEARCH_R_RET_FIX,
                VEHICLE_MODE_SEARCH_LEFT,
                2000
            );
            break;
        }
        case VEHICLE_MODE_SEARCH_L_RET_FIX:
        {
            vehicle_search(
                vehicle,
                VEHICLE_MOTION_CLOCKWISE,
                VEHICLE_MODE_END,
                VEHICLE_MODE_SEARCH_LEFT,
                2000
            );
            break;
        }
        case VEHICLE_MODE_SEARCH_R_RET_FIX:
        {
            vehicle_search(
                vehicle,
                VEHICLE_MOTION_C_CLOCKWISE,
                VEHICLE_MODE_END,
                VEHICLE_MODE_SEARCH_LEFT,
                2000
            );
            break;
        }
    }
}

// motor break & reverse update
static void direction_update(VehicleParameter *vehicle)
{
    switch (vehicle->dict_state)
    {
        case DIRECTION_NORMAL:
        {
            if (vehicle->dict_ref == vehicle->dict_fbk) break;
            vehicle->dict_state = DIRECTION_SWITCHING;
        }
        case DIRECTION_SWITCHING:
        {
            uint8_t unstop = 2;
            if ((vehicle->motor_left.value_fbk < MOTOR_STOP_GATE))
            {
                unstop--;
                // vehicle->motor_left.mode_ref = CMD_WHEEL_B0_LOCK;
            }
            if ((vehicle->motor_right.value_fbk < MOTOR_STOP_GATE))
            {
                unstop--;
                // vehicle->motor_right.mode_ref = CMD_WHEEL_B0_LOCK;
            }
            if (unstop == 0)
            {
                vehicle->dict_state = DIRECTION_NORMAL;
                break;
            }
            vehicle->dict_ref = vehicle->dict_fbk;
            vehicle->motor_left.mode_ref = CMD_WHEEL_B0_BREAK;
            vehicle->motor_right.mode_ref = CMD_WHEEL_B0_BREAK;
            break;
        }
    }
    vehicle_motor_dir_set(vehicle, vehicle->dict_ref);
}

// emergency stop
static void uss_update(VehicleParameter *vehicle)
{
    if (
        vehicle->us_sensor.state == USS_STATUS_DANGER
        && vehicle->dict_ref == VEHICLE_MOTION_FORWARD
    ) {
        vehicle->motor_left.mode_ref = CMD_WHEEL_B0_BREAK;
        vehicle->motor_right.mode_ref = CMD_WHEEL_B0_BREAK;
    }
}

#define TASK_DELAY_MS 10
uint32_t veh_next_wake;
void StartVehicleTask(void *argument)
{
    const uint32_t osPeriod = pdMS_TO_TICKS(TASK_DELAY_MS);
    veh_next_wake = osKernelGetTickCount() + osPeriod;
    VehicleParameter *vehicle = &vehicle_h;
    for(;;)
    {
        vehicle_motor_dir_fbk(vehicle);

        mode_update(vehicle);
        direction_update(vehicle);

        uss_update(vehicle);

        if (
            vehicle->fdcan_enable &&
            vehicle->mode != VEHICLE_MODE_UNK
        ) vehicle->fdcan_send = 1;

        osDelayUntil(veh_next_wake);
        veh_next_wake += osPeriod;
    }
}

#endif