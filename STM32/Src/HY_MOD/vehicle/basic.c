#include "HY_MOD/vehicle/basic.h"
#ifdef HY_MOD_STM32_VEHICLE

#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/fdcan/pkt_write.h"

VehicleParameter vehicle_h = {
    .free = {
        .direction = VEHICLE_MOTION_FORWARD,
        .speed = 50.0f,
    },
    .search = {
        .speed = 25.0f,
    },
    .motor_left = {
        .fdcan_id = CAN_ID_WHEEL_LEFT_SET_RPM,
        .rpm_max = 1000.0f,
    },
    .motor_right = {
        .fdcan_id = CAN_ID_WHEEL_RIGHT_SET_RPM,
        .rpm_max = 1000.0f,
    },
};

void vehicle_motor_dir_set(VehicleParameter *vehicle, VehicleDirection dict)
{
    switch (dict)
    {
        case VEHICLE_MOTION_FORWARD:
        {
            vehicle->motor_left.reverse_ref = 1;
            vehicle->motor_right.reverse_ref = 0;
            break;
        }
        case VEHICLE_MOTION_BACKWARD:
        {
            vehicle->motor_left.reverse_ref = 0;
            vehicle->motor_right.reverse_ref = 1;
            break;
        }
        case VEHICLE_MOTION_CLOCKWISE:
        {
            vehicle->motor_left.reverse_ref = 1;
            vehicle->motor_right.reverse_ref = 1;
            break;
        }
        case VEHICLE_MOTION_C_CLOCKWISE:
        {
            vehicle->motor_left.reverse_ref = 0;
            vehicle->motor_right.reverse_ref = 0;
            break;
        }
    }
}

void vehicle_motor_dir_fbk(VehicleParameter *vehicle)
{
    uint8_t temp = 0;
    if (vehicle->motor_left.reverse_fbk) temp |= (1 << 1);
    if (vehicle->motor_right.reverse_fbk) temp |= (1 << 0);
    switch (temp)
    {
        case 0b00000010:
        {
            vehicle->dict_fbk = VEHICLE_MOTION_FORWARD;
            break;
        }
        case 0b00000001:
        {
            vehicle->dict_fbk = VEHICLE_MOTION_BACKWARD;
            break;
        }
        case 0b00000011:
        {
            vehicle->dict_fbk = VEHICLE_MOTION_CLOCKWISE;
            break;
        }
        case 0b00000000:
        {
            vehicle->dict_fbk = VEHICLE_MOTION_C_CLOCKWISE;
            break;
        }
    }
}

void vehicle_set_mode(VehicleParameter *vehicle, VehicleMode mode)
{
    switch (mode)
    {
        case VEHICLE_MODE_END:
        case VEHICLE_MODE_FREE:
        {
            vehicle->search.repeat = 0;
            break;
        }
        case VEHICLE_MODE_TRACK:
        // case VEHICLE_MODE_T_LEAVE:
        {
            vehicle->last_tick_on_mag = HAL_GetTick();
            vehicle->search.repeat = 0;
            break;
        }
        case VEHICLE_MODE_T_ROTATE:
        {
            vehicle->last_tick_on_mag = HAL_GetTick();
            vehicle->search.repeat = 0;
            break;
        }
        case VEHICLE_MODE_SEARCH_LEFT:
        {
            vehicle->last_tick_on_mag = HAL_GetTick();
            vehicle->search.repeat++;
            if (vehicle->search.repeat >= 3)
            {
                vehicle->search.repeat = 0;
                mode = VEHICLE_MODE_END;
            }
            break;
        }
        default: break;
    }
    vehicle->mode = mode;
}

void vehicle_set_var_free(VehicleParameter *vehicle, VehicleDirection dict, Percentage spd)
{
    vehicle->free.direction = dict;
    vehicle->free.speed = spd;
}

void vehicle_set_var_track(VehicleParameter *vehicle, VehicleDirection dict, Percentage spd)
{
    vehicle->track.direction = dict;
    vehicle->track.speed = spd;
}

void vehicle_set_var_rotate(
    VehicleParameter *vehicle,
    VehicleDirection dict,
    Percentage spd,
    uint8_t need_count
) {
    vehicle->rotate.direction = dict;
    vehicle->rotate.speed = spd;
    vehicle->rotate.need_count = need_count;
}

void motor_history_write(MotorParameter *motor)
{
    uint16_t idx = motor->history.head;
    motor->history.data[idx].spd_ref = (!motor->reverse_ref) ?
        motor->value_ref : -motor->value_ref;
    motor->history.data[idx].spd_fbk = (!motor->reverse_fbk) ?
        motor->value_fbk  : -motor->value_fbk;
    motor->history.cnt++;
    idx++;
    if (idx >= MOTOR_HISTORY_LEN)
    {
        idx = 0;
        motor->history.is_full = 1;
    }
    motor->history.head = idx;
}

#endif