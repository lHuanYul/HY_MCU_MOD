#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

Result fdcan_pkt_write_test(FdcanPkt *pkt);

#ifdef MCU_MOTOR_CTRL
#include "motor/basic.h"

Result fdcan_motor_send(MotorParameter *motor, FdcanPktPool *pool, FdcanPktBuf *buf);
#endif

#ifdef MCU_VEHICLE_MAIN
#include "vehicle/basic.h"

Result fdcan_pkt_write_motor(FdcanPkt *pkt, MotorParameter *motor);
Result fdcan_vehicle_motor_send(VehicleParameter *vehicle, FdcanPktPool *pool, FdcanPktBuf *buf);
#endif

#ifdef MCU_SENSOR
#include "analog/adc1/main.h"
#include "HY_MOD/spi/rfid/main.h"
#include "us_sensor/main.h"

Result fdcan_pkt_write_hall_uss(FdcanPkt *pkt);
Result fdcan_pkt_write_rfid(FdcanPkt *pkt);
#endif

#endif