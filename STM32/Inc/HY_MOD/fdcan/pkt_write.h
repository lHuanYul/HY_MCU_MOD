#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

Result fdcan_pkt_write_test(FdcanPkt *pkt);

#ifdef MCU_MOTOR_CTRL
#include "HY_MOD/motor/basic.h"

Result fdcan_motor_rpm_send(FdcanParametar *fdcan, MotorParameter *motor);
Result fdcan_motor_idq_send(FdcanParametar *fdcan, MotorParameter *motor, uint8_t idq_sel);
#endif

#ifdef MCU_VEHICLE_MAIN
#include "HY_MOD/vehicle/basic.h"

Result fdcan_pkt_write_motor(FdcanPkt *pkt, MotorParameter *motor);
Result fdcan_vehicle_motor_send(VehicleParameter *vehicle, FdcanPktPool *pool, FdcanPktBuf *buf);
#endif

#ifdef MCU_SENSOR
#include "main/adc_hall.h"
#include "HY_MOD/rfid/main.h"
#include "HY_MOD/us_sensor/main.h"

Result fdcan_pkt_write_hall_uss(FdcanPkt *pkt);
Result fdcan_pkt_write_rfid(FdcanPkt *pkt);
#endif

#endif