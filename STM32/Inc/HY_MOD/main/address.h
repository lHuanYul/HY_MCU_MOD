/**
 * DTCM RAM (Data TCM)
 * 存取速度最快 (與 CPU 同頻)，但一般 DMA 無法存取 DTCM (只有 MDMA 可以)
 * 
 * AXI SRAM (RAM_D1 / D1 Domain)
 * 通常作為主要的資料儲存區
 * 
 * SRAM1, SRAM2, SRAM3 (RAM_D2 / D2 Domain)
 * 這是 DMA 搬運資料最推薦的區域
 * 
 * SRAM4 (RAM_D3 / D3 Domain)
 * 通常用於低功耗模式或備份
 * 
 * 64KB
 * 0x10000
 */

// ==========================================
// STM32H753 記憶體配置 (依據 MPU 2的冪次方限制調整)
// ==========================================
#define H753_ITCM_ADDRESS       0x00000000
#define H753_ITCM_SIZE          MPU_REGION_SIZE_64KB

#define H753_DataTCM_ADDRESS    0x20000000 //[cite: 13]
#define H753_DataTCM_SIZE       MPU_REGION_SIZE_128KB //[cite: 13]

#define H753_RAM_D1_ADDRESS     0x24000000
#define H753_RAM_D1_SIZE        MPU_REGION_SIZE_512KB

#define H753_RAM_D2_ADDRESS     0x30000000
// 備註：RAM_D2 實際為 288KB。MPU 必須是 2 的冪次方，因此通常設定為 256KB，
// 若需涵蓋全部，須額外設定一個 32KB 的 SubRegion。
#define H753_RAM_D2_SIZE        MPU_REGION_SIZE_256KB 

#define H753_RAM_D3_ADDRESS     0x38000000
#define H753_RAM_D3_SIZE        MPU_REGION_SIZE_64KB

#define H753_FLASH_ADDRESS      0x08000000
#define H753_FLASH_SIZE         MPU_REGION_SIZE_2MB


// ==========================================
// STM32G431 記憶體配置 (Cortex-M4 具備 MPU)
// ==========================================
#define G431_SRAM_ADDRESS       0x20000000
// 備註：實際 SRAM 總和為 32KB (SRAM1 22KB + SRAM2 10KB，實體位址連續)
#define G431_SRAM_SIZE          MPU_REGION_SIZE_32KB

#define G431_FLASH_ADDRESS      0x08000000
#define G431_FLASH_SIZE         MPU_REGION_SIZE_128KB


// ==========================================
// STM32G0B1 記憶體配置 (Cortex-M0+ 具備 MPU)
// ==========================================
#define G0B1_SRAM_ADDRESS       0x20000000
// 備註：實際 SRAM 為 144KB。受限於 2 的冪次方，單一 MPU Region 最大只能設 128KB，
// 剩餘 16KB 需另開一個 Region 處理。
#define G0B1_SRAM_SIZE          MPU_REGION_SIZE_128KB 

#define G0B1_FLASH_ADDRESS      0x08000000
#define G0B1_FLASH_SIZE         MPU_REGION_SIZE_512KB
