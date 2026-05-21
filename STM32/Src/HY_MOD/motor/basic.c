#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

const MotorModelData motor_vehicle = {
    .pole = 20,
    .gear = 4.4f,
    .rated_current = 1.9f,
    .rl = 0.32f / 2.0f,
    .tau = 0.0025f,
    .ll = (0.32f / 2.0f) * 0.0025f,
    .hall_angle_comp = PI_DIV_3 * 5.0f,
    .deg_spd_Kp = 0.0f,
    .deg_spd_Ki = 0.0f,
    .foc_spd_Kp = 0.5f,
    .foc_spd_Ki = 0.05f,
};

const MotorModelData motor_42BLF01 = {
    .pole = 8,
    .gear = 1.0f,
    // MOTOR_42BLF01_PEAK_I 5.7f
    .rated_current = 1.9f,
    .rl = 2.2f / 2.0f,
    .tau = 0.0011f,
    .ll = (2.2f / 2.0f) * 0.0011f,
    .hall_angle_comp = PI_DIV_3 * 5.0f,
    .deg_spd_Kp = 0.006f,
    .deg_spd_Ki = 0.006f,
    .foc_spd_Kp = 0.0f,
    .foc_spd_Ki = 0.0f,
};

const uint8_t motor_hall_seq_ccw[8] = {UINT8_MAX, 5, 3, 1, 6, 4, 2, UINT8_MAX};
const uint8_t motor_hall_seq_clw[8] = {UINT8_MAX, 3, 6, 2, 5, 1, 4, UINT8_MAX};

#endif