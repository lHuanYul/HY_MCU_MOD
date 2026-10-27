#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

/* ---------- Motor datasheets ---------- */

// Max 36V 150Hz
// HALL  U:Yellow V:Green W:Blue
// MOTOR U:Green  V:Blue  W:Yellow
const MotorModelData motor_vehicle = {
    .pole = 20,
    .gear = 4.4f,
    .rated_current = 1.9f,
    .rated_voltage = 36.0f,
    .rl = 0.32f / 2.0f,
    .tau = 0.0025f,
    .ll = (0.32f / 2.0f) * 0.0025f,
    .hall_angle_comp = PI_DIV(3) * 5.0f,
    .deg_spd_Kp = 0.05f,
    .deg_spd_Ki = 0.02f,
    .foc_spd_Kp = 0.5f,
    .foc_spd_Ki = 0.05f,
};

// HALL  U:Yellow V:Green W:Blue
// MOTOR U:Yellow V:Green W:Blue
const MotorModelData motor_42BLF01 = {
    .pole = 8,
    .gear = 1.0f,
    // MOTOR_42BLF01_PEAK_I 5.7f
    .rated_current = 1.9f,
    .rated_voltage = 36.0f, // ?
    .rl = 2.2f / 2.0f,
    .tau = 0.0011f,
    .ll = (2.2f / 2.0f) * 0.0011f,
    .hall_angle_comp = PI_DIV(3) * 5.0f,
    .deg_spd_Kp = 0.006f,
    .deg_spd_Ki = 0.006f,
    .foc_spd_Kp = 0.0f,
    .foc_spd_Ki = 0.0f,
};

#endif
