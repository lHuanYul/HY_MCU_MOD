/*
#include "HY_MOD/main/fn_state.h"
*/
#pragma once
#include "main/config.h"
#include "main.h"

// Result --------------------------------------------------
typedef union ResultSuccess
{
    void *obj;
} ResultSuccess;

/**
 * @brief 系統通用與控制模組錯誤型別定義
 */
typedef enum ResultErrorType
{
    /* ---------- 基礎通用錯誤 (Generic Errors) ---------- */

    /** 無效的狀態、參數或控制代碼 */
    RESULT_ERROR_INVALID         = -1,
    /** 未定義錯誤 (Undefined) */
    RESULT_ERROR_UND             = 0,
    /** 常規執行失敗 (General failure) */
    RESULT_ERROR_FAIL            = 1,
    /** 模組/硬體忙碌中，無法處理當前請求 */
    RESULT_ERROR_BUSY            = 2,
    /** 操作超時 (通訊等待或狀態切換逾時) */
    RESULT_ERROR_TIMEOUT         = 3,
    /** 空指標例外 (Null pointer dereference) */
    RESULT_ERROR_NULL_PTR,

    /* ---------- 記憶體與佇列容器 (Memory & Container) ---------- */

    /** 記憶體配置失敗或空間不足 */
    RESULT_ERROR_MEMORY_ERROR,
    /** 佇列/緩衝區為空 (Buffer underflow) */
    RESULT_ERROR_EMPTY,
    /** 佇列/緩衝區已滿 (Buffer overflow) */
    RESULT_ERROR_FULL,
    /** 數值或硬體計數器溢位 (Overflow) */
    RESULT_ERROR_OVERFLOW,
    /** 查無此項目/ID不存在 (例如無效的霍爾扇區) */
    RESULT_ERROR_NOT_FOUND,
    /** 項目未發生位移/指針未移動 */
    RESULT_ERROR_NOT_MOVE,
    /** 節點或項目移除失敗 */
    RESULT_ERROR_REMOVE_FAIL,

    /* ---------- 數值與參數邊界 (Math & Arguments) ---------- */

    /** 除以零錯誤 (Division by zero) */
    RESULT_ERROR_DIV_0,
    /** 參數數值超出合法範圍 (例如 Duty > 1.0 或速度超限) */
    RESULT_ERROR_PARAM_OUT_OF_RANGE,
    /** 數學計算超出定義域 (例如負數開平方根、atan2 奇異點) */
    RESULT_ERROR_MATH_DOMAIN,

    /* ---------- 狀態機與權限 (State Machine & Logic) ---------- */

    /** 當前狀態機不允許執行該指令 (例如運轉中禁止校正) */
    RESULT_ERROR_WRONG_STATE,
    /** 模組未初始化即嘗試呼叫使用 */
    RESULT_ERROR_NOT_INIT,
    /** 資源/軸處於鎖定保護狀態 */
    RESULT_ERROR_LOCKED,

    /* ---------- 硬體周邊與通訊傳輸 (Peripheral & Communication) ---------- */

    /** 底層硬體錯誤 (DMA/Timer/ADC 異常) */
    RESULT_ERROR_HW_FAULT,
    /** 通訊斷線/節點離線 (例如 FDCAN 節點心跳丟失) */
    RESULT_ERROR_COMM_LOST,
    /** 封包資料校驗和/CRC 比對錯誤 */
    RESULT_ERROR_CRC_MISMATCH,
    /** 封包幀格式錯誤或資料長度不匹配 */
    RESULT_ERROR_FRAME_CORRUPT,
    
} ResultErrorType;

extern ResultErrorType last_error;

typedef struct Result
{
    bool is_ok;
    union
    {
        ResultSuccess success;
        ResultErrorType error;
    } result;
} Result;

// RESULT_OK(NULL);
#define RESULT_OK(_obj_)    ((Result){.is_ok = true,  .result.success = {.obj = (_obj_)}})
#define RESULT_ERROR(_err_) ((Result){.is_ok = false, .result.error   = (_err_)})

#define RESULT_CHECK_OK(_res_)      ( (_res_).is_ok)
#define RESULT_CHECK_ERR(_res_)    (!(_res_).is_ok)
#define RESULT_UNWRAP(_res_)        ((_res_).result.success.obj)

#define RESULT_BOOL_TO_RES(_cond_) ((_cond_) ? RESULT_OK(NULL) : RESULT_ERROR(RESULT_ERROR_FAIL))

#define RESULT_UNWRAP_SKIP(expr)\
    ({\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            NULL;\
        } \
        else \
        { \
            RESULT_UNWRAP(_res_);\
        } \
    })

#define RESULT_CHECK_SIMPLE(_res_)\
    do {\
        if (RESULT_CHECK_ERR(_res_))\
            return EXIT_FAILURE;\
    } while (0)

    
#define RESULT_CHECK_HANDLE(expr)\
    do {\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            Error_Handler();\
        }\
    } while (0)


#define RESULT_UNWRAP_HANDLE(expr)\
    ({\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            Error_Handler();\
        }\
        RESULT_UNWRAP(_res_);\
    })

#define RESULT_CHECK_RET_VOID(expr)\
    do {\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            return;\
        }\
    } while (0)

#define RESULT_UNWRAP_RET_VOID(expr)\
    ({\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            return;\
        }\
        RESULT_UNWRAP(_res_);\
    })

#define RESULT_CHECK_RET_RES(expr)\
    do {\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            return _res_;\
        }\
    } while (0)

#define RESULT_UNWRAP_RET_RES(expr)\
    ({\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            return _res_;\
        }\
        RESULT_UNWRAP(_res_);\
    })

#define RESULT_CHECK_GOTO(expr,tag)\
    do {\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            goto tag;\
        }\
    } while (0)

#define RESULT_UNWRAP_GOTO(expr,tag)\
    ({\
        Result _res_ = (expr);\
        if (RESULT_CHECK_ERR(_res_))\
        {\
            last_error = _res_.result.error;\
            goto tag;\
        }\
        RESULT_UNWRAP(_res_);\
    })

// STM32 HAL --------------------------------------------------
#ifdef STM32_DEVICE
#define RESULT_HAL_CHECK_RET_HAL(expr) \
    do { \
        HAL_StatusTypeDef _err = (expr); \
        if (_err != HAL_OK) \
        { \
            return _err; \
        } \
    } while (0)

#define RESULT_HAL_CHECK_RET_RES(expr) \
    do { \
        HAL_StatusTypeDef _err = (expr); \
        if (_err != HAL_OK) \
        { \
            return RESULT_ERROR(_err); \
        } \
    } while (0)

#define RESULT_HAL_CHECK_HANDLE(expr) \
    do { \
        HAL_StatusTypeDef _err = (expr); \
        if (_err != HAL_OK) \
        { \
            last_error = _err; \
            Error_Handler(); \
        } \
    } while (0)

#define INSTANCE_CHK(x, y) ((x)->Instance == (y)->Instance)
#endif
