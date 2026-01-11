#include "HY_MOD/spi/callback.h"
#ifdef HY_MOD_ESP32_SPI

// IRAM_ATTR: 強制將此函式放在 RAM 中執行，因為 Flash 讀取速度慢且在某些情況下無法讀取，
// ISR 必須要在 RAM 裡以確保快速與安全。
void IRAM_ATTR spi_done_callback(spi_slave_transaction_t *trans) {
    
    // 1. 取出 Task Handle
    // 在主程式設定 spi_t.user = ... 時，我們把 "任務的控制代碼 (Handle)" 藏在這裡。
    // 因為 Callback 無法直接知道要通知哪個 Task，所以透過這個指標傳遞進來。
    TaskHandle_t task = (TaskHandle_t)trans->user;

    // 2. 準備 Context Switch 標記
    // 這個變數用來記錄 "被喚醒的 Task 優先權是否比當前被中斷的程式高"。
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 3. 發送通知 (Unblock Task)
    // 這是 FreeRTOS 的機制，相當於 "給出一個信號"，叫醒正在睡覺 (Blocked) 的 Task。
    // FromISR 表示這是專門給中斷用的版本。
    vTaskNotifyGiveFromISR(task, &xHigherPriorityTaskWoken);

    // 4. 強制切換任務 (Context Switch)
    // 如果喚醒的 Task 比較緊急 (優先權高)，這一行會讓 CPU 離開中斷後，
    // 不會回到原本被打斷的程式，而是直接跳去執行那個剛醒來的 Task。
    // 這能讓 SPI 的反應幾乎沒有延遲。
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

#endif