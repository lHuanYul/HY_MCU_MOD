#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/typedef.h"
#include "HY_MOD/fdcan/packet.h"
#include "fdcan.h"

typedef enum FdcanState
{
    FDCAN_STATE_RUNNING,
    FDCAN_STATE_BUS_OFF,
    FDCAN_STATE_RESTART,
} FdcanState;

typedef struct FdcanConst
{
    FDCAN_HandleTypeDef *hfdcanx;
    TIM_HandleTypeDef   *htimx;
    uint32_t            *tim_clk;
} FdcanConst;

// DBG: debug
typedef struct FdcanDbg
{
    // 計時器頻率
    float32_t   tim_freq;
} FdcanDbg;

typedef struct FdcanParametar
{
    const FdcanConst const_h;
    FdcanDbg    dbg_h;
    FdcanState  state;
    FdcanRing   tx_buf;
    uint8_t     tx_cb;
    uint32_t    tx_lost;
    FdcanRing   rx_buf;
    uint8_t     rx_cb;
    uint32_t    tim_tick;
    uint32_t    alive_tick;
    volatile bool   test_en;
#ifdef MCU_MOTOR_CTRL
    uint32_t        motor_alive;
    bool            motor_ret_en;
    volatile bool   motor_rpm_en;
    volatile bool   motor_idq_en1;
    volatile bool   motor_idq_en2;
#endif
} FdcanParametar;
/**
 * @brief 初始化 FDCAN 硬體周邊與過濾器設定
 * @details 配置全域過濾器規則 (Global Filter)、FIFO0/1 範圍過濾器、發送延遲補償 (TDC)，
 *          並啟動 FDCAN 實例及相關發送、接收與中斷通知。
 * 
 * @param fdcan 指向 FDCAN 控制結構體指標
 */
void fdcan_setup(FdcanParametar *fdcan);
/**
 * @brief 計算計時器頻率並啟動基礎定時中斷 (IT)
 * 
 * @param fdcan 指向 FDCAN 控制結構體指標
 */
void fdcan_tim_start(FdcanParametar *fdcan);
/**
 * @brief 將發送環形緩衝區 (tx_buf) 內的封包推送至硬體 Tx FIFO
 * 
 * @param fdcan 指向 FDCAN 控制結構體指標
 * @return Result 成功推送或無待發封包時返回 RESULT_OK
 */
Result fdcan_tx_push(FdcanParametar *fdcan);
/**
 * @brief FDCAN 模組的主狀態機與週期任務處理常式
 * @details 負責處理 Bus-Off 復位與重啟延遲、自動週期封包發送、Tx FIFO 佇列推送及 Rx FIFO 封包讀取。
 * 
 * @param fdcan 指向 FDCAN 控制結構體指標
 */
void fdcan_main(FdcanParametar *fdcan);

#endif