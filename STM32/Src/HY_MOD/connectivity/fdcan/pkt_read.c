#include "HY_MOD/connectivity/fdcan/pkt_read.h"
#ifdef HY_MOD_STM32_FDCAN
#include "HY_MOD/connectivity/cmds.h"
#include "HY_MOD/main/variable_cal.h"

#ifdef MCU_MOTOR_CTRL
#include "motor/basic.h"

Result fdcan_pkt_ist_read(FdcanPkt *pkt)
{
    uint8_t code;
    switch (pkt->id)
    {
        case FDCAN_WHEEL_SET_ID:
        {
            motor_alive(&motor_h);
            RESULT_CHECK_RET_RES(fdcan_pkt_get_byte(pkt, 0, &code));
            switch (code)
            {
                case CMD_WHEEL_B0_COAST:
                {
                    motor_set_rotate_mode(&motor_h, MOTOR_ROT_COAST);
                    return RESULT_OK(NULL);
                }
                case CMD_WHEEL_B0_BREAK:
                {
                    motor_set_rotate_mode(&motor_h, MOTOR_ROT_BREAK);
                    return RESULT_OK(NULL);
                }
                case CMD_WHEEL_B0_NORMAL:
                {
                    if (pkt->len < 6) break;
                    motor_set_rotate_mode(&motor_h, MOTOR_ROT_NORMAL);
                    code = (pkt->data[1]) ? 1 : 0;
                    uint8_t u8s[sizeof(float32_t)];
                    memcpy(u8s, pkt->data + 2, sizeof(float32_t));
                    motor_set_rpm(&motor_h, code, var_u8_to_f32_be(u8s));
                    return RESULT_OK(NULL);
                }
                case CMD_WHEEL_B0_LOCK:
                {
                    motor_set_rotate_mode(&motor_h, MOTOR_ROT_LOCK);
                    return RESULT_OK(NULL);
                }
                case CMD_WHEEL_B0_FDCAN:
                {
                    RESULT_CHECK_RET_RES(fdcan_pkt_get_byte(pkt, 1, &code));
                    if (code == 0) motor_h.fdcan_enable = 0;
                    else motor_h.fdcan_enable = 1;
                    return RESULT_OK(NULL);
                }
                default: break;
            }
        }
        default: break;
    }
    return RESULT_ERROR(RES_ERR_NOT_FOUND);
}
#endif

#ifdef MCU_VEHICLE_MAIN
#include "vehicle/main.h"

static Result motor_pkt(FdcanPkt *pkt, MotorParameter *motor)
{
    motor->alive_tick = HAL_GetTick();
    if (!fdcan_pkt_check_len(pkt, 2 + sizeof(float32_t))) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    motor->mode_fbk = pkt->data[0];
    motor->reverse_fbk = pkt->data[1];
    uint8_t u8s[sizeof(float32_t)];
    memcpy(u8s, pkt->data + 2, sizeof(float32_t));
    motor->value_fbk = var_u8_to_f32_be(u8s);
    motor_history_write(motor);
    return RESULT_OK(NULL);
}

static void hall_read(uint8_t code, VehicleHall *hall)
{
    hall->alive_tick = HAL_GetTick();
    if (
        code != ADC_HALL_STATE_NONE &&
        code != ADC_HALL_STATE_ON_MAG
    ) code = ADC_HALL_STATE_UNK;
    hall->state = code;
}

static void uss_read(uint8_t code, VehicleUSS *uss)
{
    uss->alive_tick = HAL_GetTick();
    if (
        code != USS_STATUS_SAVE &&
        code != USS_STATUS_WARNING &&
        code != USS_STATUS_DANGER
    ) code = USS_STATUS_UNK;
    uss->state = code;
}

Result fdcan_pkt_ist_read(FdcanPkt *pkt)
{
    uint8_t code;
    switch (pkt->id)
    {
        case CAN_ID_WHEEL_LEFT_SPD_FBK:
        {
            return motor_pkt(pkt, &vehicle_h.motor_left);
        }
        case CAN_ID_WHEEL_RIGHT_SPD_FBK:
        {
            return motor_pkt(pkt, &vehicle_h.motor_right);
        }
        case CAN_ID_HALL_ALL_FBK:
        {
            if (!fdcan_pkt_check_len(pkt, 4)) break;
            hall_read(pkt->data[0], &vehicle_h.hall_front);
            hall_read(pkt->data[1], &vehicle_h.hall_left);
            hall_read(pkt->data[2], &vehicle_h.hall_right);
            uss_read(pkt->data[3], &vehicle_h.us_sensor);
            return RESULT_OK(NULL);
        }
        case CAN_ID_RFID_FBK:
        {
            vehicle_h.rfid.alive_tick = HAL_GetTick();
            if (!fdcan_pkt_check_len(pkt, 1 + sizeof(uint32_t))) break;
            if (pkt->data[0] != 0) vehicle_h.rfid.new = 1;
            uint8_t u8s[sizeof(uint32_t)];
            memcpy(u8s, pkt->data + 1, sizeof(uint32_t));
            vehicle_h.rfid.rfid = var_u8_to_u32_be(u8s);
        }
        case CAN_ID_VEHICLE:
        {
            RESULT_CHECK_RET_RES(fdcan_pkt_get_byte(pkt, 0, &code));
            switch (code)
            {
                case CMD_VEHI_B0_SET_MODE:
                {
                    RESULT_CHECK_RET_RES(fdcan_pkt_get_byte(pkt, 1, &code));
                    switch (code)
                    {
                        case CMD_VEHI_B1_MODE_FREE:
                        {
                            vehicle_set_mode(&vehicle_h, VEHICLE_MODE_FREE);
                            return RESULT_OK(NULL);
                        }
                        case CMD_VEHI_B1_MODE_TRACK:
                        {
                            vehicle_set_mode(&vehicle_h, VEHICLE_MODE_TRACK);
                            return RESULT_OK(NULL);
                        }
                        case CMD_VEHI_B1_MODE_ROTATE:
                        {
                            vehicle_set_mode(&vehicle_h, VEHICLE_MODE_T_ROTATE);
                            return RESULT_OK(NULL);
                        }
                        case CMD_VEHI_B1_MODE_SEARCH:
                        {
                            vehicle_set_mode(&vehicle_h, VEHICLE_MODE_SEARCH_LEFT);
                            return RESULT_OK(NULL);
                        }
                        default: break;
                    }
                    break;
                }
                case CMD_VEHI_B0_SET_FREE_VAR:
                {
                    if (pkt->len < 3) return RESULT_ERROR(RES_ERR_NOT_FOUND);
                    vehicle_set_var_free(&vehicle_h, pkt->data[1], pkt->data[2]);
                    return RESULT_OK(NULL);
                }
                case CMD_VEHI_B0_SET_TRACK_VAR:
                {
                    if (pkt->len < 3) return RESULT_ERROR(RES_ERR_NOT_FOUND);
                    vehicle_set_var_track(&vehicle_h, pkt->data[1], pkt->data[2]);
                    return RESULT_OK(NULL);
                }
                case CMD_VEHI_B0_SET_ROTATE_VAR:
                {
                    if (pkt->len < 4) return RESULT_ERROR(RES_ERR_NOT_FOUND);
                    vehicle_set_var_rotate(&vehicle_h, pkt->data[1], pkt->data[2], pkt->data[3]);
                    return RESULT_OK(NULL);
                }
                case CMD_VEHI_B0_FDCAN:
                {
                    RESULT_CHECK_RET_RES(fdcan_pkt_get_byte(pkt, 1, &code));
                    if (code == 0) vehicle_h.fdcan_enable = 0;
                    else vehicle_h.fdcan_enable = 1;
                    return RESULT_OK(NULL);
                }
                default: break;
            }
        }
        default: break;
    }
    return RESULT_ERROR(RES_ERR_NOT_FOUND);
}

Result fdcan_pkt_rcv_read(FdcanPkt *pkt)
{
    uint8_t code;
    RESULT_CHECK_RET_RES(fdcan_pkt_get_byte(pkt, 0, &code));
    
    return RESULT_ERROR(RES_ERR_NOT_FOUND);
}
#endif

#ifdef MCU_SENSOR
Result fdcan_pkt_ist_read(FdcanPkt *pkt)
{
    uint8_t code;
    switch (pkt->id)
    {
        
        default: break;
    }
    return RESULT_ERROR(RES_ERR_NOT_FOUND);
}
#endif

#endif