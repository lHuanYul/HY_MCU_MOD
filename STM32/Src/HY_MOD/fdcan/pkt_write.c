#include "HY_MOD/fdcan/pkt_write.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/main/variable_cal.h"

static float ftest = 0.0;
Result fdcan_pkt_write_test(FdcanPkt *pkt)
{
    if (pkt == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    pkt->id = FDCAN_TEST_ID;
    pkt->data[0] = 0x00;
    pkt->data[1] = 0xFF;
    var_f32_to_u8_be(ftest++, pkt->data + 2);
    pkt->len = 6;
    return RESULT_OK(pkt);
}

#ifdef MCU_MOTOR_CTRL
static Result motor_fbk(FdcanPkt *pkt, MotorParameter *motor)
{
    if (pkt == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    pkt->id = FDCAN_WHEEL_FBK_ID;
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, 2 + sizeof(float32_t)));
    pkt->data[0] = motor->mode_rot_ref;
    pkt->data[1] = motor->rpm_feedback.reverse;
    var_f32_to_u8_be(motor->rpm_feedback.value, pkt->data + 2);
    return RESULT_OK(pkt);
}

Result fdcan_motor_send(MotorParameter *motor, FdcanPktPool *pool, FdcanPktBuf *buf)
{
    FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_pool_alloc(pool));
    motor_fbk(pkt, &motor_h);
    RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(buf, pkt, pool, 1));
    return RESULT_OK(NULL);
}
#endif

#ifdef MCU_VEHICLE_MAIN
Result fdcan_pkt_write_motor(FdcanPkt *pkt, MotorParameter *motor)
{
    if (pkt == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    pkt->id = motor->fdcan_id;
    pkt->data[0] = motor->mode_ref;
    pkt->data[1] = motor->reverse_ref;
    float32_t spd = (float32_t)motor->value_ref * motor->rpm_max * 0.01f;
    VAR_CLAMPF(spd, 0.0f, motor->rpm_max);
    var_f32_to_u8_be(spd, pkt->data + 2);
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, 2 + sizeof(float32_t)));
    return RESULT_OK(pkt);
}

Result fdcan_vehicle_motor_send(VehicleParameter *vehicle, FdcanPktPool *pool, FdcanPktBuf *buf)
{
    FdcanPkt *pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(pool));
    fdcan_pkt_write_motor(pkt, &vehicle->motor_left);
    RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(buf, pkt, pool, 1));
    pkt = RESULT_UNWRAP_HANDLE(fdcan_pkt_pool_alloc(pool));
    fdcan_pkt_write_motor(pkt, &vehicle->motor_right);
    RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(buf, pkt, pool, 1));
    return RESULT_OK(NULL);
}
#endif

#ifdef MCU_SENSOR
#include "HY_MOD/rfid/basic.h"

Result fdcan_pkt_write_hall_uss(FdcanPkt *pkt)
{
    if (pkt == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    pkt->id = CAN_ID_HALL_ALL_FBK;
    pkt->data[0] = adchall_direction.state;
    pkt->data[1] = adchall_track_left.state;
    pkt->data[2] = adchall_track_right.state;
    pkt->data[3] = us_sensor_head.status;
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, 4));
    return RESULT_OK(NULL);
}

Result fdcan_pkt_write_rfid(FdcanPkt *pkt)
{
    if (pkt == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    pkt->id = CAN_ID_RFID_FBK;
    pkt->data[0] = rfid_h.new_card;
    rfid_h.new_card = 0;
    memcpy(pkt->data + 1, rfid_h.uid.uidByte, 4);
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, 1 + 4));
    return RESULT_OK(NULL);
}
#endif

#endif