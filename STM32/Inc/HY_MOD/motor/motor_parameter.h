#pragma once

// MOTOR_FOC_ANGLE
// 霍爾訊號與實際電角校正
// 霍爾超前實際為負

#define MOTOR_42BLF01_POLE          8
#define MOTOR_42BLF01_GEAR          1.0f // 3.0f
#define MOTOR_42BLF01_RATED_CURRENT 1.9f
#define MOTOR_42BLF01_PEAK_CURRENT  5.7f
#define MOTOR_42BLF01_ANGLE         (PI_DIV_6*-5.0f) // 6 // -3/3 -4/2 -5/2 -6/6

#define MOTOR_VEHICLE_POLE          20
#define MOTOR_VEHICLE_GEAR          4.4f
// !
#define MOTOR_VEHICLE_ANGLE         (PI_DIV_6*-5.0f)
