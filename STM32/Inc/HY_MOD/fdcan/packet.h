#pragma once
#include "HY_MOD/packet/basic.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/fn_state.h"

typedef struct FdcanPkt
{
    uint8_t  idx;
    uint32_t id;
    uint8_t  data[FDCAN_PKT_LEN];
    uint8_t  len;
    struct FdcanPkt *next;
} FdcanPkt;
#define FDCAN_PKT_CHK_LEN(pkt, len) ((pkt)->len < (len))
/**
 * @brief 從封包資料中取得指定索引的 Byte
 * 
 * @param pkt 指向目標 FdcanPkt 結構體指標
 * @param id 目標資料陣列的位元組索引 (0 ~ len-1)
 * @param container 接收讀取位元組數值的指標
 * @return Result 成功返回 RESULT_OK；若索引超出長度則返回 RESULT_ERROR_NOT_FOUND
 */
Result fdcan_pkt_get_byte(FdcanPkt *pkt, uint8_t id, uint8_t* container);
/**
 * @brief 設定 FDCAN 封包的 ID (Identifier)
 * 
 * @param pkt 指向目標 FdcanPkt 結構體指標
 * @param id 欲設定的 CAN ID
 */
void fdcan_pkt_set_id(FdcanPkt *pkt, uint32_t id);
/**
 * @brief 設定 FDCAN 封包的有效資料長度
 * 
 * @param pkt 指向目標 FdcanPkt 結構體指標
 * @param len 欲設定的資料長度 (位元組數，最大不可超過 FDCAN_PKT_LEN)
 * @return Result 成功返回 RESULT_OK；長度超出上限返回 RESULT_ERROR_FULL
 */
Result fdcan_pkt_set_len(FdcanPkt *pkt, uint8_t len);

typedef struct FdcanRing
{
    FdcanPkt            *buf;
    uint8_t             cap;
    volatile uint32_t   out;
    volatile uint32_t   in;
    uint32_t            drop_cnt;
} FdcanRing;
/* 取得目前佇列中的封包數量 */
#define FDCAN_RING_GET_COUNT(ring)      ((uint32_t)((ring)->in - (ring)->out))
/* 檢查佇列是否為空 (true / false) */
#define FDCAN_RING_IS_EMPTY(ring)       ((ring)->in == (ring)->out)
/* 檢查佇列是否已滿 (true / false) */
#define FDCAN_RING_IS_FULL(ring)        (((ring)->in - (ring)->out) >= (ring)->cap)
/* 取得目前佇列剩餘可用空間 */
#define FDCAN_RING_GET_FREE(ring) \
    ((FDCAN_RING_GET_COUNT(ring) >= (ring)->cap) ? 0U : ((ring)->cap - FDCAN_RING_GET_COUNT(ring)))
/**
 * @brief 將封包推入環形緩衝區 (Ring Buffer)
 * 
 * @param self 指向目標 FdcanRing 結構體指標
 * @param pkt 欲寫入的封包指標
 * @param drop 滿載處理模式：非 0 時若緩衝區滿載直接捨棄並回報錯誤；為 0 則覆蓋寫入
 * @return Result 成功返回 RESULT_OK；滿載且啟用 drop 則返回 RESULT_ERROR_FULL
 */
Result fdcan_ring_push(FdcanRing *self, FdcanPkt *pkt, uint8_t drop);
/**
 * @brief 從環形緩衝區 (Ring Buffer) 取出封包
 * 
 * @param self 指向目標 FdcanRing 結構體指標
 * @param pkt 儲存取出資料的 FdcanPkt 容器指標
 * @return Result 成功返回 RESULT_OK；緩衝區為空返回 RESULT_ERROR_EMPTY
 */
Result fdcan_ring_pop(FdcanRing *self, FdcanPkt *pkt);
/**
 * @brief 清空環形緩衝區 (Lock-Free)
 * @note 將讀取指標 (out) 直接同步至寫入指標 (in)，不歸零以避免與中斷寫入衝突
 * 
 * @param self 指向目標 FdcanRing 結構體指標
 */
void fdcan_ring_clear(FdcanRing *self);

#endif