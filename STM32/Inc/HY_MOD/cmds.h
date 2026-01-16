#pragma once

#include <stdint.h>

#define CMD_BX_EMPTY                (uint8_t)0x00

#define FDCAN_TEST_ID               (uint16_t)0x001

//----------------------------------------------------------------------------------------------------
#define CMD_DATA_B0_CONTROL         (uint8_t)0x10   // 回傳資料
#define CMD_DATA_B0_STOP            (uint8_t)0x11   // 資料收集停止(can)
#define CMD_DATA_B0_START           (uint8_t)0x12   // 資料收集開始(can)

#define CMD_DATA_B1_VEHI_POSI       (uint8_t)0x00   // 車輛位置
#define CMD_DATA_B1_LEFT_SPEED      (uint8_t)0x10   // 左馬達速度f32
#define CMD_DATA_B1_RIGHT_SPEED     (uint8_t)0x11   // 右馬達速度f32
#define CMD_DATA_B1_LEFT_DUTY       (uint8_t)0x20   // 左馬達功率u8
#define CMD_DATA_B1_RIGHT_DUTY      (uint8_t)0x21   // 右馬達功率u8
#define CMD_DATA_B1_ARM_BOTTOM      (uint8_t)0x40   // 手臂馬達 基座左右
#define CMD_DATA_B1_ARM_SHOULDER    (uint8_t)0x41   // 手臂馬達 基座上下
#define CMD_DATA_B1_ARM_ELBOW_BTM   (uint8_t)0x42   // 手臂馬達
#define CMD_DATA_B1_ARM_ELBOW_TOP   (uint8_t)0x43   // 手臂馬達
#define CMD_DATA_B1_ARM_WRIST       (uint8_t)0x44   // 手臂馬達
#define CMD_DATA_B1_ARM_FINGER      (uint8_t)0x45   // 手臂馬達

//      CMD_DATA_B2_VALUE                           // 資料總計         // 位置ID(車輛位置)

//      CMD_DATA_B3_VALUE                           // 資料編號(can)    // 目標ID(車輛位置)

//----------------------------------------------------------------------------------------------------
#define CAN_ID_WHEEL_LEFT_SPD_FBK   (uint16_t)0x100
#define CAN_ID_WHEEL_RIGHT_SPD_FBK  (uint16_t)0x101
#define CAN_ID_WHEEL_LEFT_SET       (uint16_t)0x140
#define CAN_ID_WHEEL_RIGHT_SET      (uint16_t)0x141
#define CMD_WHEEL_B0_COAST          (uint8_t)0x00
#define CMD_WHEEL_B0_BREAK          (uint8_t)0x01
#define CMD_WHEEL_B0_NORMAL         (uint8_t)0x02
#define CMD_WHEEL_B0_LOCK           (uint8_t)0x03
#define CMD_WHEEL_B0_FDCAN          (uint8_t)0xFF
#define CMD_WHEEL_B1_REVERSE
#define CMD_WHEEL_B25_SPD

//----------------------------------------------------------------------------------------------------

#define CAN_ID_HALL_ALL_FBK             (uint16_t)0x104
#define CMD_HALL_B0_FRONT
#define CMD_HALL_B1_LEFT
#define CMD_HALL_B2_RIGHT
// #define CMD_HALL_B3_BATTRY

//----------------------------------------------------------------------------------------------------
#define CAN_ID_RFID_FBK                 (uint16_t)0x105
#define CMD_RFID_B0_NEW
#define CMD_RFID_B14_ID

#define CAN_ID_SENSOR

//----------------------------------------------------------------------------------------------------
#define CAN_ID_VEHICLE              (uint16_t)0x107
#define CMD_VEHI_B0_SET_MODE        (uint8_t)0x00
#define CMD_VEHI_B0_SET_FREE_VAR    (uint8_t)0x01
#define CMD_VEHI_B0_SET_TRACK_VAR   (uint8_t)0x02
#define CMD_VEHI_B0_SET_ROTATE_VAR  (uint8_t)0x03
#define CMD_VEHI_B0_FDCAN           (uint8_t)0xFF
#define CMD_VEHI_B1_MODE_FREE       (uint8_t)0x00
#define CMD_VEHI_B1_MODE_TRACK      (uint8_t)0x01
#define CMD_VEHI_B1_MODE_ROTATE     (uint8_t)0x02
#define CMD_VEHI_B1_MODE_SEARCH     (uint8_t)0x03
#define CMD_VEHI_B1_VAR_DICT
#define CMD_VEHI_B25_VAR_SPD
#define CMD_VEHI_B6_VAR_CNT

//----------------------------------------------------------------------------------------------------
#define CMD_MAP_B0_CONTROL          (uint8_t)0x30   // 地圖控制

#define CMD_MAP_B1_INFO             (uint8_t)0x00   // RFID
#define CMD_MAP_B1_SET              (uint8_t)0x01

//      CMD_MAP_B2_VALUE                            // UID
#define CMD_MAP_B2_FORWARD          (uint8_t)0x00
#define CMD_MAP_B2_BACKWARD         (uint8_t)0x01
#define CMD_MAP_B2_LEFT             (uint8_t)0x02
#define CMD_MAP_B2_RIGHT            (uint8_t)0x03

//----------------------------------------------------------------------------------------------------
#define CMD_ARM_B0_CONTROL          (uint8_t)0x31   // 手臂控制

#define CMD_ARM_B1_BOTTOM           (uint8_t)0x00   // 馬達
#define CMD_ARM_B1_SHOULDER         (uint8_t)0x01   // 馬達
#define CMD_ARM_B1_ELBOW_BTM        (uint8_t)0x02   // 馬達
#define CMD_ARM_B1_ELBOW_TOP        (uint8_t)0x03   // 馬達
#define CMD_ARM_B1_WRIST            (uint8_t)0x04   // 馬達
#define CMD_ARM_B1_FINGER           (uint8_t)0x05   // 馬達
#define CMD_ARM_B1_ARM              (uint8_t)0x10   // 手臂

#define CMD_ARM_B2_STOP             (uint8_t)0x00   // 停止
#define CMD_ARM_B2_SET              (uint8_t)0x01   // 方位設定

//      CMD_ARM_B3_VALUE                            // 方位值0-100

//----------------------------------------------------------------------------------------------------
#define CMD_RFID_B0_CONTROL         (uint8_t)0x32   // RFID控制

#define CMD_RFID_B1_SELECT          (uint8_t)0x00
#define CMD_RFID_B1_INP_DATA        (uint8_t)0x01

//      CMD_RFID_B2_SECTOR                          // 扇區選擇 0-15
//      CMD_RFID_B2_NUMBER                          // DATA號碼 0-3

//      CMD_RFID_B3_BLOCK                           // 區塊選擇 0-2
//      CMD_RFID_(B3-B6)_DATA                       // 4 byte DATA

//      CMD_RFID_B4_CONFIRM                         // 直接送出 0 或 1
#define CMD_RFID_B4_ONLY_SET        (uint8_t)0x00
#define CMD_RFID_B4_WRITE           (uint8_t)0x01

//----------------------------------------------------------------------------------------------------
#define CMD_B0_TEST                 (uint8_t)0xFF

#define CMD_B1_TEST0                (uint8_t)0x00
