#include <Arduino.h>
#include <SPI.h>

// 定義控制腳位
const int RS_CS = 10;
const int RW_SID = 11;
const int EN_CLK = 13;
const int RST = 12;

const int DB[] = {9, 8, 7, 6, 5, 4, 3, 2};
const int PSB = A0;

const int DELAYUS = 100;
const int BAUDRATE = 2000000;

// 依據 Datasheet 實作串列位元發送 (Software SPI)
// 使用硬體 SPI 傳送單一 Byte
void SendByte(byte data) {
  SPI.transfer(data);
}

// 輸出 8 位元資料到資料線上
void SetDataBus(byte val) {
  for(int i = 0; i < 8; i++) {
    digitalWrite(DB[i], (val >> i) & 0x01);
  }
}

// 發送指令 (開頭位元組為 0xF8)
void WriteCommand(byte cmd)
{
  digitalWrite(RS_CS, HIGH);
  
  // 啟動 SPI 傳輸 (設定頻率、資料順序、模式)
  // ST7920 支援的速度不高，設定為 1MHz 或 2MHz 比較穩定。
  SPI.beginTransaction(SPISettings(BAUDRATE, MSBFIRST, SPI_MODE3));
  
  SendByte(0xF8);               // 指令前綴
  SendByte(cmd & 0xF0);         // 高四位
  SendByte((cmd << 4) & 0xF0);  // 低四位
  
  SPI.endTransaction(); // 結束傳輸
  digitalWrite(RS_CS, LOW);
  
  delayMicroseconds(DELAYUS);
}

// 寫入指令 (RS = 0, R/W = 0)
void WriteCommand_P(byte cmd) {
  digitalWrite(RS_CS, LOW);
  digitalWrite(RW_SID, LOW);
  SetDataBus(cmd);
  
  // 致能脈衝 (Datasheet: E 脈衝寬度 > 140ns)
  digitalWrite(EN_CLK, HIGH);
  delayMicroseconds(2); 
  digitalWrite(EN_CLK, LOW);
  
  delayMicroseconds(DELAYUS); // Datasheet: 一般指令執行時間 > 72us
}

// 發送資料 (開頭位元組為 0xFA)
void WriteData(byte data) {
  digitalWrite(RS_CS, HIGH);
  
  SPI.beginTransaction(SPISettings(BAUDRATE, MSBFIRST, SPI_MODE3));
  
  SendByte(0xFA);               // 資料前綴
  SendByte(data & 0xF0);        // 高四位
  SendByte((data << 4) & 0xF0); // 低四位
  
  SPI.endTransaction();
  digitalWrite(RS_CS, LOW);
  
  delayMicroseconds(DELAYUS);
}

// 寫入資料 (RS = 1, R/W = 0)
void WriteData_P(byte data) {
  digitalWrite(RS_CS, HIGH);
  digitalWrite(RW_SID, LOW);
  SetDataBus(data);
  
  digitalWrite(EN_CLK, HIGH);
  delayMicroseconds(2);
  digitalWrite(EN_CLK, LOW);
  
  delayMicroseconds(DELAYUS); 
}

void display001()
{
  // --- 顯示英文 "Arduino" ---
  WriteCommand(0x80); // 將游標移至第 1 行起點
  char engStr[] = "Arduino";
  for (int i = 0; i < 7; i++) {
    WriteData(engStr[i]);
  }

  // --- 顯示中文 "測試" ---
  WriteCommand(0x90); // 將游標移至第 2 行起點
  byte chStr[] = {0xB4, 0xFA, 0xB8, 0xD5}; 
  for (int i = 0; i < 4; i++) {
    WriteData(chStr[i]);
  }

  WriteCommand(0x88); // 將游標移至第 1 行起點
  char hy3Str[] = "1234";
  for (int i = 0; i < 4; i++) {
    WriteData(hy3Str[i]);
  }
  WriteCommand(0x98); // 將游標移至第 1 行起點
  char hy4Str[] = "5678";
  for (int i = 0; i < 4; i++) {
    WriteData(hy4Str[i]);
  }
}

// 操作 GDRAM 繪圖函數
// data1: 高 8 像素, data2: 低 8 像素 (1=亮, 0=暗)
void FillGDRAM(byte data1, byte data2)
{
  // WriteCommand(0x34); // 4. 關閉繪圖顯示
  WriteCommand(0x36); // 1. 開啟擴充指令集&開啟繪圖顯示

  for (byte y = 0; y < 32; y++) {
    WriteCommand(0x80 + y); // 2. 設定 Y 座標 (0~31)
    WriteCommand(0x80);     // 3. 設定 X 座標起點 (0x80)

    // ST7920 具有 X 座標自動遞增功能。
    // 迴圈 16 次 (16 Words = 32 Bytes)，剛好會把:
    // X=0x80~0x87 (上半螢幕) 與 X=0x88~0x8F (下半螢幕) 一次寫滿。
    for (byte x = 0; x < 16; x++) { 
      WriteData(data1); // 寫入前 8 個像素
      WriteData(data2); // 寫入後 8 個像素
    }
  }

  WriteCommand(0x36); // 4. 開啟繪圖顯示
  WriteCommand(0x30); // 5. 切回基本指令集 (若後續還要用內建字庫)
}

void display002()
{
  // 測試 2：畫虛線/橫條紋 (10101010)
  FillGDRAM(0xFF, 0x00); 
  delay(2000);

  // 測試 1：全螢幕塗黑
  FillGDRAM(0xFF, 0xFF); 
  delay(2000);

  // 測試 2：畫虛線/橫條紋 (10101010)
  FillGDRAM(0x00, 0xFF);
  delay(2000);
  
  // 測試 3：清除 GDRAM (全白)
  FillGDRAM(0x00, 0x00);
}

void setup() {
  // 初始化硬體 SPI
  SPI.begin();
  // 1. 設定控制腳位為輸出
  pinMode(RS_CS, OUTPUT);
  // pinMode(RW_SID, OUTPUT);
  // pinMode(EN_CLK, OUTPUT);
  // pinMode(PSB, OUTPUT);
  pinMode(RST, OUTPUT);

  // 2. 設定 8 條資料線 (DB0~DB7) 為輸出
  // for(int i = 0; i < 8; i++) {
  //   pinMode(DB[i], OUTPUT);
  // }

  // 3. 初始狀態設定
  // digitalWrite(EN, LOW);       // E 腳位預設低電平
  // digitalWrite(PSB, HIGH);     // PSB 高電平，強制進入「並列模式」
  digitalWrite(RS_CS, LOW);

  // --- 執行硬體復位 ---
  digitalWrite(RST, LOW);  // 觸發低電平復位
  delay(50);
  digitalWrite(RST, HIGH); // 恢復高電平正常工作
  delay(50);
  
  // 初始化 ST7920 (依據 Datasheet 指令集)
  WriteCommand(0x30); // 喚醒、設定基本指令集
  delay(5);
  WriteCommand(0x0C); // 開啟顯示，關閉游標
  delay(5);
  WriteCommand(0x01); // 清除螢幕
  delay(10);

  display002();
}

void loop() {
  // put your main code here, to run repeatedly:
}
