#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_VEHICLE

#include "HY_MOD/main/fn_state.h"

#define MOTOR_STOP_GATE 10.0f

typedef struct VehicleConst
{
    bool t;
} VehicleConst;

typedef enum VehicleMode
{
    VEHICLE_MODE_UNK,
    // 自由
    VEHICLE_MODE_FREE,
    // 停止
    VEHICLE_MODE_END,
    // 循跡
    VEHICLE_MODE_TRACK,
    // 旋轉
    VEHICLE_MODE_T_ROTATE,
    // 離點(前進後循跡)
    // VEHICLE_MODE_T_LEAVE,
    // 尋找
    VEHICLE_MODE_SEARCH_LEFT,
    VEHICLE_MODE_SEARCH_RIGHT,
    VEHICLE_MODE_SEARCH_L_RET,
    VEHICLE_MODE_SEARCH_R_RET,
    VEHICLE_MODE_SEARCH_L_RET_FIX,
    VEHICLE_MODE_SEARCH_R_RET_FIX,
} VehicleMode;

typedef enum VehicleDirection
{
    VEHICLE_MOTION_FORWARD,
    VEHICLE_MOTION_BACKWARD,
    VEHICLE_MOTION_CLOCKWISE,
    VEHICLE_MOTION_C_CLOCKWISE,
} VehicleDirection;

typedef enum DirectionState
{
    DIRECT_NORMAL,
    DIRECT_SWITCHING,
} DirectionState;

typedef struct VehicleFreeP
{
    VehicleDirection    direction;
    Percentage          speed;
} VehicleFreeP;

typedef struct VehicleTrackP
{
    VehicleDirection    direction;
    Percentage          speed;
} VehicleTrackP;

typedef struct VehicleRotateP
{
    VehicleDirection    direction;
    Percentage          speed;
    uint8_t             need_count;
} VehicleRotateP;

typedef struct VehicleSearchP
{
    VehicleDirection    direction;
    Percentage          speed;
    uint8_t             repeat;
} VehicleSearchP;

typedef enum VehicleHallState
{
    ADC_HALL_STATE_UNK,
    ADC_HALL_STATE_NONE,
    ADC_HALL_STATE_ON_MAG,
} VehicleHallState;

typedef struct VehicleHall
{
    VehicleHallState    state;
    uint32_t            alive_tick;
} VehicleHall;

typedef enum USSState
{
    USS_STATUS_UNK,
    USS_STATUS_SAVE,
    USS_STATUS_WARNING,
    USS_STATUS_DANGER,
} USSState;

typedef struct VehicleUSS
{
    USSState    state;
    uint32_t    alive_tick;
} VehicleUSS;

typedef struct VehicleRfid
{
    bool        new;
    uint32_t    rfid;
    uint32_t    alive_tick;
} VehicleRfid;

typedef struct MotorHistoryData
{
    Percentage spd_ref;
    float32_t spd_fbk;
} MotorHistoryData;

typedef struct MotorHistoryArray
{
    MotorHistoryData data[MOTOR_HISTORY_LEN];
    uint16_t head;
    bool is_full;
    uint32_t cnt;
} MotorHistoryArray;

typedef struct MotorParameter
{
    const uint16_t  fdcan_id;
    float32_t       rpm_max;
    uint32_t        alive_tick;
    uint8_t         mode_ref;
    bool            reverse_ref;
    Percentage      value_ref;
    uint8_t         mode_fbk;
    bool            reverse_fbk;
    float32_t       value_fbk;
    MotorHistoryArray history;
} MotorParameter;

typedef struct VehicleParameter
{
    const VehicleConst  const_h;
    VehicleMode         mode;
    bool                fdcan_enable;
    bool                fdcan_send;
    DirectionState      dict_state;
    VehicleFreeP        free;
    VehicleTrackP       track;
    VehicleRotateP      rotate;
    VehicleSearchP      search;
    VehicleDirection    dict_ref;
    VehicleDirection    dict_fbk;
    VehicleHallState    front_last_state;
    VehicleHall         hall_front;
    VehicleHall         hall_left;
    VehicleHall         hall_right;
    VehicleUSS          us_sensor;
    VehicleRfid         rfid;
    MotorParameter      motor_left;
    MotorParameter      motor_right;
    uint32_t            last_tick_on_mag;
} VehicleParameter;

extern VehicleParameter vehicle_h;

void vehicle_motor_dir_set(VehicleParameter *vehicle, VehicleDirection dict);
void vehicle_motor_dir_fbk(VehicleParameter *vehicle);
void vehicle_set_mode(VehicleParameter *vehicle, VehicleMode mode);
void vehicle_set_var_free(VehicleParameter *vehicle, VehicleDirection dict, Percentage spd);
void vehicle_set_var_track(VehicleParameter *vehicle, VehicleDirection dict, Percentage spd);
void vehicle_set_var_rotate(
    VehicleParameter *vehicle,
    VehicleDirection dict,
    Percentage spd,
    uint8_t need_count
);
void motor_history_write(MotorParameter *motor);

#endif