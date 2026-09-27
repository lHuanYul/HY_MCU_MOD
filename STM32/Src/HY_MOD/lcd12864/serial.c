#include "HY_MOD/lcd12864/basic.h"
#ifdef HY_MOD_STM32_LCD12864

void lcd_write_cmd(Lcd12864Param *lcd, uint8_t cmd)
{
    RESULT_CHECK_RET_VOID(spi_start_transceive_dma(&lcd->const_h.spi_p, &cmd, ));
  
  // 啟動 SPI 傳輸 (設定頻率、資料順序、模式)
  // ST7920 支援的速度不高，設定為 1MHz 或 2MHz 比較穩定。
  SPI.beginTransaction(SPISettings(BAUDRATE, MSBFIRST, SPI_MODE3));
  
  SendByte(0xF8);               // 指令前綴
  SendByte(cmd & 0xF0);         // 高四位
  SendByte((cmd << 4) & 0xF0);  // 低四位
  
  SPI.endTransaction(); // 結束傳輸
    GPIO_WRITE(lcd->const_h.spi_p.const_h.NSS, 0);
    osDelay(1);
}

#endif