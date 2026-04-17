#include "HY_MOD/fdcan/pkt_write.h"
#ifdef HY_MOD_STM32_FDCAN

#include "main/main.h"
#include "HY_MOD/fdcan/pkt_read.h"
#include "HY_MOD/fdcan/main.h"
#include "HY_MOD/main/variable_cal.h"

static float ftest = 0.0;
Result fdcan_pkt_write_test(FdcanParametar *fdcan)
{
    FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_pool_alloc(&fdcan->pool));
    fdcan_pkt_set_id(pkt, FDCAN_TEST_ID);
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, 2 + sizeof(float32_t)));
    pkt->data[0] = 0xFF;
    pkt->data[1] = 0x00;
    var_f32_to_u8_be(ftest++, pkt->data + 2);
    RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(&fdcan->tx_buf, pkt, &fdcan->pool, 1));
    return RESULT_OK(NULL);
}

#ifdef MCU_MOTOR_CTRL

Result fdcan_motor_rpm_send(FdcanParametar *fdcan, MotorParameter *motor)
{
    FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_pool_alloc(&fdcan->pool));
    fdcan_pkt_set_id(pkt, CAN_ID_WHEEL_RET_RPM);
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, sizeof(uint32_t) + sizeof(float32_t) * 2));

    var_u32_to_u8_be(fdcan->tim_tick, pkt->data);

    float32_t f32 = (motor->rpm_h.ref_fix.reverse) ?
        -motor->rpm_h.ref_fix.value : motor->rpm_h.ref_fix.value;
    var_f32_to_u8_be(f32, pkt->data + 4);
    f32 = (motor->rpm_h.fb.reverse) ?
        -motor->rpm_h.fb.value : motor->rpm_h.fb.value;
    var_f32_to_u8_be(f32, pkt->data + 8);

    RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(&fdcan->tx_buf, pkt, &fdcan->pool, 1));
    return RESULT_OK(NULL);
}

Result fdcan_motor_idq_send(FdcanParametar *fdcan, MotorParameter *motor, uint8_t idq_sel)
{
    FdcanPkt *pkt = RESULT_UNWRAP_RET_RES(fdcan_pkt_pool_alloc(&fdcan->pool));
    fdcan_pkt_set_id(pkt, CAN_ID_WHEEL_RET_IDQ);
    RESULT_CHECK_HANDLE(fdcan_pkt_set_len(pkt, sizeof(uint32_t) + sizeof(float32_t) * 10));

    var_u32_to_u8_be(fdcan->tim_tick, pkt->data);
    uint8_t i;
    for (i = idq_sel; i < 5 + idq_sel; i++)
    {
        var_f32_to_u8_be(motor->history.id[i], pkt->data + 4 + i * 4);
        var_f32_to_u8_be(motor->history.iq[i], pkt->data + 4 + (i + 5) * 4);
    }

    RESULT_CHECK_HANDLE(fdcan_pkt_buf_push(&fdcan->tx_buf, pkt, &fdcan->pool, 0));
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