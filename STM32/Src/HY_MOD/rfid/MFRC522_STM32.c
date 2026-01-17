#include "HY_MOD/rfid/MFRC522_STM32.h"
#ifdef HY_MOD_STM32_RFID
/**
 * https://github.com/miguelbalboa/rfid
 * 
 * ver 1.4.12
 */

// Custom code Start
#define RC522_CS_SELECT()	HAL_GPIO_WritePin(rc522_const->SDA_GPIOx, rc522_const->SDA_GPIO_PIN_x, GPIO_PIN_RESET)
#define RC522_CS_CANCLE()	HAL_GPIO_WritePin(rc522_const->SDA_GPIOx, rc522_const->SDA_GPIO_PIN_x, GPIO_PIN_SET)
#define RC522_RST_START()	HAL_GPIO_WritePin(rc522_const->RST_GPIOx, rc522_const->RST_GPIO_PIN_x, GPIO_PIN_RESET)
#define RC522_RST_END()		HAL_GPIO_WritePin(rc522_const->RST_GPIOx, rc522_const->RST_GPIO_PIN_x, GPIO_PIN_SET)
static const uint8_t MFRC522_firmware_referenceVx_B2[64] = {
    0x00, 0xE9, 0x0C, 0xB9, 0xF7, 0xCF, 0xDF, 0x0F,
    0xC6, 0x5E, 0xA6, 0x35, 0x90, 0xF2, 0x11, 0x64,
    0xE2, 0x0E, 0x36, 0x2D, 0xCA, 0xDD, 0xA3, 0xD1,
    0x01, 0x1B, 0x61, 0x64, 0x3A, 0xFB, 0xA8, 0x1A,
    0x28, 0x37, 0xEE, 0x53, 0x61, 0x37, 0xA3, 0xC7,
    0xE9, 0x83, 0x63, 0xEC, 0xBE, 0xD6, 0x24, 0x71,
    0x1E, 0xA9, 0x6D, 0xDA, 0xD4, 0xFD, 0xFE, 0xEB,
    0x6D, 0x85, 0x9C, 0xE6, 0x99, 0xF7, 0x1D, 0xD9
};
const RC522MIFARE_Key rc522_default_key = {
	.keyByte = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};
uint8_t rc522_version = 0;
// Custom code End

static const uint8_t MFRC522_firmware_referenceV0_0[64] = {
	0x00, 0x87, 0x98, 0x0f, 0x49, 0xFF, 0x07, 0x19,
	0xBF, 0x22, 0x30, 0x49, 0x59, 0x63, 0xAD, 0xCA,
	0x7F, 0xE3, 0x4E, 0x03, 0x5C, 0x4E, 0x49, 0x50,
	0x47, 0x9A, 0x37, 0x61, 0xE7, 0xE2, 0xC6, 0x2E,
	0x75, 0x5A, 0xED, 0x04, 0x3D, 0x02, 0x4B, 0x78,
	0x32, 0xFF, 0x58, 0x3B, 0x7C, 0xE9, 0x00, 0x94,
	0xB4, 0x4A, 0x59, 0x5B, 0xFD, 0xC9, 0x29, 0xDF,
	0x35, 0x96, 0x98, 0x9E, 0x4F, 0x30, 0x32, 0x8D
};
static const uint8_t MFRC522_firmware_referenceV1_0[64] = {
	0x00, 0xC6, 0x37, 0xD5, 0x32, 0xB7, 0x57, 0x5C,
	0xC2, 0xD8, 0x7C, 0x4D, 0xD9, 0x70, 0xC7, 0x73,
	0x10, 0xE6, 0xD2, 0xAA, 0x5E, 0xA1, 0x3E, 0x5A,
	0x14, 0xAF, 0x30, 0x61, 0xC9, 0x70, 0xDB, 0x2E,
	0x64, 0x22, 0x72, 0xB5, 0xBD, 0x65, 0xF4, 0xEC,
	0x22, 0xBC, 0xD3, 0x72, 0x35, 0xCD, 0xAA, 0x41,
	0x1F, 0xA7, 0xF3, 0x53, 0x14, 0xDE, 0x7E, 0x02,
	0xD9, 0x0F, 0xB5, 0x5E, 0x25, 0x1D, 0x29, 0x79
};
static const uint8_t MFRC522_firmware_referenceV2_0[64] = {
	0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95,
	0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
	0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82,
	0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
	0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81,
	0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
	0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D,
	0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F
};
static const uint8_t FM17522_firmware_reference[64] = {
	0x00, 0xD6, 0x78, 0x8C, 0xE2, 0xAA, 0x0C, 0x18,
	0x2A, 0xB8, 0x7A, 0x7F, 0xD3, 0x6A, 0xCF, 0x0B,
	0xB1, 0x37, 0x63, 0x4B, 0x69, 0xAE, 0x91, 0xC7,
	0xC3, 0x97, 0xAE, 0x77, 0xF4, 0x37, 0xD7, 0x9B,
	0x7C, 0xF5, 0x3C, 0x11, 0x8F, 0x15, 0xC3, 0xD7,
	0xC1, 0x5B, 0x00, 0x2A, 0xD0, 0x75, 0xDE, 0x9E,
	0x51, 0x64, 0xAB, 0x3E, 0xE9, 0x15, 0xB5, 0xAB,
	0x56, 0x9A, 0x98, 0x82, 0x26, 0xEA, 0x2A, 0x62
};

RC522Uid rc522_uid;

/**
 * Writes a number of bytes to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
void RC522_PCD_WriteRegister_full(
	const RC522Const* rc522_const,
	PCD_Register reg,	///< The register to write to. One of the PCD_Register enums.
	uint8_t count,		///< The number of bytes to write to the register
	uint8_t *values		///< The values to write. Byte array.
) {
	uint8_t addr = (reg & 0x7E);  
	// 在 RC522 的 SPI 協定中，address 的最高位 (MSB) = 0 表示寫入
	RC522_CS_SELECT();
	// 傳送寄存器地址
	HAL_SPI_Transmit(rc522_const->hspi, &addr, 1, HAL_MAX_DELAY);
	// 傳送後續資料
	HAL_SPI_Transmit(rc522_const->hspi, values, count, HAL_MAX_DELAY);
	RC522_CS_CANCLE();
} // End RC522_PCD_WriteRegister()

/**
 * Writes a byte to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
inline void RC522_PCD_WriteRegister(
	const RC522Const* rc522_const,
	PCD_Register reg,	///< The register to write to. One of the PCD_Register enums.
	uint8_t value		///< The value to write.
) {
	RC522_PCD_WriteRegister_full(rc522_const, reg, 1, &value);
} // End RC522_PCD_WriteRegister()

/**
 * Reads a number of bytes from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
void RC522_PCD_ReadRegister_full(
	const RC522Const* rc522_const,
	PCD_Register reg,	///< The register to read from. One of the PCD_Register enums.
	uint8_t count,		///< The number of bytes to read
	uint8_t *values,	///< Byte array to store the values in.
	uint8_t rxAlign		///< Only bit positions rxAlign..7 in values[0] are updated.
) {
	if (count == 0) return;
	uint8_t addr = 0x80 | (reg & 0x7E);  
	// MSB=1 表示讀，清掉 LSB 的 R/W bit
	RC522_CS_SELECT();
	// 2. 先送一次 address
	HAL_SPI_Transmit(rc522_const->hspi, &addr, 1, HAL_MAX_DELAY);
	uint8_t index = 0;
	count--;  // 最後一 byte 用不同方式讀
	// 如需對齊，先處理第一個位元組
	if (rxAlign) {
		uint8_t mask  = (0xFF << rxAlign) & 0xFF;
		uint8_t byte0;
		HAL_SPI_TransmitReceive(rc522_const->hspi, &addr, &byte0, 1, HAL_MAX_DELAY);
		values[0] = (values[0] & ~mask) | (byte0 & mask);
		index++;
	}
	// 讀中間 bytes
	while (index < count) {
		HAL_SPI_TransmitReceive(rc522_const->hspi, &addr, &values[index], 1, HAL_MAX_DELAY);
		index++;
	}

	// 讀最後一 byte，送 0x00 作 dummy
	{
		uint8_t dummy = 0x00;
		HAL_SPI_TransmitReceive(rc522_const->hspi, &dummy, &values[index], 1, HAL_MAX_DELAY);
	}
	// 3. CS 拉高
	RC522_CS_CANCLE();
} // End RC522_PCD_ReadRegister()

/**
 * Reads a byte from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
inline uint8_t RC522_PCD_ReadRegister(
	const RC522Const* rc522_const,
	PCD_Register reg	///< The register to read from. One of the PCD_Register enums.
) {
	uint8_t value = 0;
	// 呼叫多位元組讀取，count=1、rxAlign=0
	RC522_PCD_ReadRegister_full(rc522_const, reg, 1, &value, 0);
	return value;
} // End RC522_PCD_ReadRegister()

/**
 * Sets the bits given in mask in register reg.
 */
inline void RC522_PCD_SetRegisterBitMask(
	const RC522Const* rc522_const,
	PCD_Register reg,	///< The register to update. One of the PCD_Register enums.
	uint8_t mask		///< The bits to set.
) { 
	uint8_t tmp = RC522_PCD_ReadRegister(rc522_const, reg);
	RC522_PCD_WriteRegister(rc522_const, reg, tmp | mask);
} // End RC522_PCD_SetRegisterBitMask()

/**
 * Clears the bits given in mask from register reg.
 */
inline void RC522_PCD_ClearRegisterBitMask(
	const RC522Const* rc522_const,
	PCD_Register reg,	///< The register to update. One of the PCD_Register enums.
	uint8_t mask		///< The bits to clear.
) {
	uint8_t tmp = RC522_PCD_ReadRegister(rc522_const, reg);
	RC522_PCD_WriteRegister(rc522_const, reg, tmp & (~mask));
} // End RC522_PCD_ClearRegisterBitMask()

/**
 * Use the CRC coprocessor in the MFRC522 to calculate a CRC_A.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_PCD_CalculateCRC(
	const RC522Const* rc522_const,
	uint8_t *data,		///< In: Pointer to the data to transfer to the FIFO for CRC calculation.
	uint8_t length,		///< In: The number of bytes to transfer.
	uint8_t *result		///< Out: Pointer to result buffer. Result is written to result[0..1], low byte first.
) {
	// 停止任何執行中之命令
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_Idle);
	// 清除 CRC 中斷旗標
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_DivIrqReg, 0x04);
	// 清空 FIFO
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_FIFOLevelReg, 0x80);
	// 塞入要計算的資料
	RC522_PCD_WriteRegister_full(rc522_const, PCD_Reg_FIFODataReg, length, data);
	// 開始 CRC 計算
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_CalcCRC);

	// 等待 CRC 完成，最多等 90 ms
	uint32_t tickStart = HAL_GetTick();
	do {
		uint8_t n = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_DivIrqReg);
		if (n & 0x04) {
			// 計算完成，停止 CRC
			RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_Idle);
			// 讀出低位與高位
			result[0] = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CRCResultRegL);
			result[1] = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CRCResultRegH);
			return STATUS_Code_OK;
		}
	} while ((HAL_GetTick() - tickStart) < 90);

	return STATUS_Code_TIMEOUT;
} // End RC522_PCD_CalculateCRC()

/////////////////////////////////////////////////////////////////////////////////////
// Functions for manipulating the MFRC522
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the MFRC522 chip.
 */
void RC522_PCD_Init(const RC522Const* rc522_const)
{
	// 1. CS 預設拉高（不選擇 RC522）
	RC522_CS_CANCLE();
	// 2. 復位
	RC522_RST_START();	// 拉低  :contentReference[oaicite:8]{index=8}
	osDelay(1);			// Todo 約 2 μs
	RC522_RST_END();	// 退出復位  :contentReference[oaicite:9]{index=9}
	osDelay(50);		// 等待晶片 oscillator 穩定
	RC522_PCD_Reset(rc522_const);
	// 4. 重置通訊與定時器參數
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TxModeReg,		0x00);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_RxModeReg,		0x00);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_ModWidthReg,	0x26);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TModeReg,		0x80);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TPrescalerReg, 0xA9);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TReloadRegH,	0x03);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TReloadRegL,	0xE8);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TxASKReg,		0x40);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_ModeReg,		0x3D);
	// 5. 啟動天線
	RC522_PCD_AntennaOn(rc522_const);
} // End RC522_PCD_Init()

/**
 * Performs a soft reset on the MFRC522 chip and waits for it to be ready again.
 */
void RC522_PCD_Reset(const RC522Const* rc522_const)
{
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_SoftReset);	// Issue the SoftReset command.
	// The datasheet does not mention how long the SoftRest command takes to complete.
	// But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg) 
	// Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74μs. Let us be generous: 50ms.
	uint8_t count = 0;
	do {
		// Wait for the PowerDown bit in CommandReg to be cleared (max 3x50ms)
		osDelay(50);
	} while ((RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CommandReg) & (1 << 4)) && (++count) < 3);
} // End RC522_PCD_Reset()

/**
 * Turns the antenna on by enabling pins TX1 and TX2.
 * After a reset these pins are disabled.
 */
inline void RC522_PCD_AntennaOn(const RC522Const* rc522_const)
{
	uint8_t value = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_TxControlReg);
	if ((value & 0x03) != 0x03) RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TxControlReg, value | 0x03);
} // End RC522_PCD_AntennaOn()

/**
 * Turns the antenna off by disabling pins TX1 and TX2.
 */
inline void RC522_PCD_AntennaOff(const RC522Const* rc522_const)
{
	RC522_PCD_ClearRegisterBitMask(rc522_const, PCD_Reg_TxControlReg, 0x03);
} // End RC522_PCD_AntennaOff()

/**
 * Get the current MFRC522 Receiver Gain (RxGain[2:0]) value.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Return value scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 * 
 * @return Value of the RxGain, scrubbed to the 3 bits used.
 */
uint8_t RC522_PCD_GetAntennaGain(const RC522Const* rc522_const) {
	return RC522_PCD_ReadRegister(rc522_const, PCD_Reg_RFCfgReg) & (0x07<<4);
} // End PCD_GetAntennaGain()

/**
 * Set the MFRC522 Receiver Gain (RxGain) to value specified by given mask.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Given mask is scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 */
void RC522_PCD_SetAntennaGain(const RC522Const* rc522_const, uint8_t mask) {
	if (RC522_PCD_GetAntennaGain(rc522_const) != mask) {						// only bother if there is a change
		RC522_PCD_ClearRegisterBitMask(rc522_const, PCD_Reg_RFCfgReg, (0x07<<4));		// clear needed to allow 000 pattern
		RC522_PCD_SetRegisterBitMask(rc522_const, PCD_Reg_RFCfgReg, mask & (0x07<<4));	// only set RxGain[2:0] bits
	}
} // End PCD_SetAntennaGain()

/**
 * Performs a self-test of the MFRC522
 * See 16.1.1 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * 
 * @return Whether or not the test passed. Or false if no firmware reference is available.
 */
bool RC522_PCD_PerformSelfTest(const RC522Const* rc522_const)
{
	// This follows directly the steps outlined in 16.1.1
	// 1. Perform a soft reset.
	RC522_PCD_Reset(rc522_const);
	
	// 2. Clear the internal buffer by writing 25 bytes of 00h
	uint8_t ZEROES[25] = {0x00};
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_FIFOLevelReg, 0x80);		// flush the FIFO buffer
	RC522_PCD_WriteRegister_full(rc522_const, PCD_Reg_FIFODataReg, 25, ZEROES);	// write 25 bytes of 00h to FIFO
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_Mem);		// transfer to internal buffer
	
	// 3. Enable self-test
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_AutoTestReg, 0x09);
	
	// 4. Write 00h to FIFO buffer
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_FIFODataReg, 0x00);
	
	// 5. Start self-test by issuing the CalcCRC command
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_CalcCRC);
	
	// 6. Wait for self-test to complete
	uint8_t n;
	for (uint8_t i = 0; i < 0xFF; i++) {
		// The datasheet does not specify exact completion condition except
		// that FIFO buffer should contain 64 bytes.
		// While selftest is initiated by CalcCRC command
		// it behaves differently from normal CRC computation,
		// so one can't reliably use DivIrqReg to check for completion.
		// It is reported that some devices does not trigger CRCIRq flag
		// during selftest.
		n = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_FIFOLevelReg);
		if (n >= 64) {
			break;
		}
	}
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_Idle);		// Stop calculating CRC for new content in the FIFO.
	
	// 7. Read out resulting 64 bytes from the FIFO buffer.
	uint8_t result[64];
	RC522_PCD_ReadRegister_full(rc522_const, PCD_Reg_FIFODataReg, 64, result, 0);
	
	// Auto self-test done
	// Reset AutoTestReg register to be 0 again. Required for normal operation.
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_AutoTestReg, 0x00);
	
	// Determine firmware version (see section 9.3.4.8 in spec)
	uint8_t rc522_version = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_VersionReg);
	
	// Pick the appropriate reference values
	const uint8_t *reference;
	switch (rc522_version) {
		case 0x88:	// Fudan Semiconductor FM17522 clone
			reference = FM17522_firmware_reference;
			break;
		case 0x90:	// Version 0.0
			reference = MFRC522_firmware_referenceV0_0;
			break;
		case 0x91:	// Version 1.0
			reference = MFRC522_firmware_referenceV1_0;
			break;
		case 0x92:	// Version 2.0
			reference = MFRC522_firmware_referenceV2_0;
			break;
		// Custom Code Start
		case 0xB2:
			reference = MFRC522_firmware_referenceVx_B2;
			break;
		// Custom Code End
		default:	// Unknown version
			return false; // abort test
	}
	
	// Verify that the results match up to our expectations
	for (uint8_t i = 0; i < 64; i++) {
		if (result[i] != reference[i]) {
			return false;
		}
	}
	
	// 8. Perform a re-init, because PCD does not work after test.
	// Reset does not work as expected.
	// "Auto self-test done" does not work as expected.
	RC522_PCD_Init(rc522_const);
	
	// Test passed; all is good.
	return true;
} // End PCD_PerformSelfTest()

/////////////////////////////////////////////////////////////////////////////////////
// Power control
/////////////////////////////////////////////////////////////////////////////////////

//IMPORTANT NOTE!!!!
//Calling any other function that uses CommandReg will disable soft power down mode !!!
//For more details about power control, refer to the datasheet - page 33 (8.6)

void RC522_PCD_SoftPowerDown(const RC522Const* rc522_const){//Note : Only soft power down mode is available throught software
	uint8_t val = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CommandReg); // Read state of the command register 
	val |= (1<<4);// set PowerDown bit ( bit 4 ) to 1 
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, val);//write new value to the command register
}

void RC522_PCD_SoftPowerUp(const RC522Const* rc522_const){
	uint8_t val = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CommandReg); // Read state of the command register 
	val &= ~(1<<4);// set PowerDown bit ( bit 4 ) to 0 
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, val);//write new value to the command register
	// wait until PowerDown bit is cleared (this indicates end of wake up procedure) 
	uint32_t tickStart = HAL_GetTick();
	do { // set timeout to 500 ms 
		val = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CommandReg);// Read state of the command register
		if(!(val & (1<<4))){ // if powerdown bit is 0 
			break;// wake up procedure is finished 
		}
		osDelay(1);
	} while((HAL_GetTick() - tickStart) < 500);
}

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Executes the Transceive command.
 * CRC validation can only be done if backData and backLen are specified.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
inline RC522Status RC522_PCD_TransceiveData(
	const RC522Const* rc522_const,
	uint8_t *sendData,	///< Pointer to the data to transfer to the FIFO.
	uint8_t sendLen,	///< Number of bytes to transfer to the FIFO.
	uint8_t *backData,	///< NULL or pointer to buffer if data should be read back after executing the command.
	uint8_t *backLen,	///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
	uint8_t *validBits,	///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits. Default NULL.
	uint8_t rxAlign,	///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
	bool checkCRC		///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
) {
	uint8_t waitIRq = 0x30;		// RxIRq and IdleIRq
	return RC522_PCD_CommunicateWithPICC(rc522_const, PCD_Cmd_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
} // End RC522_PCD_TransceiveData()

/**
 * Transfers data to the MFRC522 FIFO, executes a command, waits for completion and transfers data back from the FIFO.
 * CRC validation can only be done if backData and backLen are specified.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_PCD_CommunicateWithPICC(
	const RC522Const* rc522_const,
	uint8_t command,	///< The command to execute. One of the PCD_Command enums.
	uint8_t waitIRq,	///< The bits in the ComIrqReg register that signals successful completion of the command.
	uint8_t *sendData,	///< Pointer to the data to transfer to the FIFO.
	uint8_t sendLen,	///< Number of bytes to transfer to the FIFO.
	uint8_t *backData,	///< NULL or pointer to buffer if data should be read back after executing the command.
	uint8_t *backLen,	///< In: Max number of bytes to write to *backData. Out: The number of bytes returned.
	uint8_t *validBits,	///< In/Out: The number of valid bits in the last byte. 0 for 8 valid bits.
	uint8_t rxAlign,	///< In: Defines the bit position in backData[0] for the first bit received. Default 0.
	bool checkCRC		///< In: True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
) {
	// Prepare values for BitFramingReg
	uint8_t txLastBits = validBits ? *validBits : 0;
	uint8_t bitFraming = (rxAlign << 4) + txLastBits;		// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]
	
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, PCD_Cmd_Idle);			// Stop any active command.
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_ComIrqReg, 0x7F);					// Clear all seven interrupt request bits
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_FIFOLevelReg, 0x80);					// FlushBuffer = 1, FIFO initialization
	RC522_PCD_WriteRegister_full(rc522_const, PCD_Reg_FIFODataReg, sendLen, sendData);	// Write sendData to the FIFO
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_BitFramingReg, bitFraming);			// Bit adjustments
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_CommandReg, command);				// Execute the command
	if (command == PCD_Cmd_Transceive) {
		RC522_PCD_SetRegisterBitMask(rc522_const, PCD_Reg_BitFramingReg, 0x80);	// StartSend=1, transmission of data starts
	}
	
	// In RC522_PCD_Init() we set the TAuto flag in TModeReg. This means the timer
	// automatically starts when the PCD stops transmitting.
	//
	// Wait here for the command to complete. The bits specified in the
	// `waitIRq` parameter define what bits constitute a completed command.
	// When they are set in the ComIrqReg register, then the command is
	// considered complete. If the command is not indicated as complete in
	// ~36ms, then consider the command as timed out.
	uint32_t tickStart = HAL_GetTick();
	bool completed = false;
	do {
		uint8_t n = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_ComIrqReg);	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
		if (n & waitIRq) {					// One of the interrupts that signal success has been set.
			completed = true;
			break;
		}
		if (n & 0x01) {						// Timer interrupt - nothing received in 25ms
			return STATUS_Code_TIMEOUT;
		}
		osDelay(1);
	} while ((HAL_GetTick() - tickStart) < 36);

	// 36ms and nothing happened. Communication with the MFRC522 might be down.
	if (!completed) {
		return STATUS_Code_TIMEOUT;
	}
	
	// Stop now if any errors except collisions were detected.
	uint8_t errorRegValue = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
	if (errorRegValue & 0x13) {	 // BufferOvfl ParityErr ProtocolErr
		return STATUS_Code_ERROR;
	}
  
	uint8_t _validBits = 0;
	
	// If the caller wants data back, get it from the MFRC522.
	if (backData && backLen) {
		uint8_t n = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_FIFOLevelReg);	// Number of bytes in the FIFO
		if (n > *backLen) {
			return STATUS_Code_NO_ROOM;
		}
		*backLen = n;											// Number of bytes returned
		RC522_PCD_ReadRegister_full(rc522_const, PCD_Reg_FIFODataReg, n, backData, rxAlign);	// Get received data from FIFO
		_validBits = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_ControlReg) & 0x07;		// RxLastBits[2:0] indicates the number of valid bits in the last received byte. If this value is 000b, the whole byte is valid.
		if (validBits) {
			*validBits = _validBits;
		}
	}
	
	// Tell about collisions
	if (errorRegValue & 0x08) {		// CollErr
		return STATUS_Code_COLLISION;
	}
	
	// Perform CRC_A validation if requested.
	if (backData && backLen && checkCRC) {
		// In this case a MIFARE Classic NAK is not OK.
		if (*backLen == 1 && _validBits == 4) {
			return STATUS_Code_MIFARE_NACK;
		}
		// We need at least the CRC_A value and all 8 bits of the last byte must be received.
		if (*backLen < 2 || _validBits != 0) {
			return STATUS_Code_CRC_WRONG;
		}
		// Verify CRC_A - do our own calculation and store the control in controlBuffer.
		uint8_t controlBuffer[2];
		RC522Status status = RC522_PCD_CalculateCRC(rc522_const, &backData[0], *backLen - 2, &controlBuffer[0]);
		if (status != STATUS_Code_OK) {
			return status;
		}
		if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) {
			return STATUS_Code_CRC_WRONG;
		}
	}
	
	return STATUS_Code_OK;
} // End RC522_PCD_CommunicateWithPICC()

/**
 * Transmits a REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
inline RC522Status RC522_PICC_RequestA(
	const RC522Const* rc522_const,
	uint8_t *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
	uint8_t *bufferSize		///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
) {
	return RC522_PICC_REQA_or_WUPA(rc522_const, PICC_CMD_REQA, bufferATQA, bufferSize);
} // End RC522_PICC_RequestA()

/**
 * Transmits a Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
inline RC522Status RC522_PICC_WakeupA(
	const RC522Const* rc522_const,
	uint8_t *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
	uint8_t *bufferSize		///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
) {
	return RC522_PICC_REQA_or_WUPA(rc522_const, PICC_CMD_WUPA, bufferATQA, bufferSize);
} // End RC522_PICC_WakeupA()

/**
 * Transmits REQA or WUPA commands.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */ 
RC522Status RC522_PICC_REQA_or_WUPA(
	const RC522Const* rc522_const,
	uint8_t command, 		///< The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
	uint8_t *bufferATQA,	///< The buffer to store the ATQA (Answer to request) in
	uint8_t *bufferSize		///< Buffer size, at least two bytes. Also number of bytes returned if STATUS_OK.
) {
	if (bufferATQA == NULL || *bufferSize < 2) {	// The ATQA response is 2 bytes long.
		return STATUS_Code_NO_ROOM;
	}
	RC522_PCD_ClearRegisterBitMask(rc522_const, PCD_Reg_CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.
	uint8_t validBits = 7;									// For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
	RC522Status status = RC522_PCD_TransceiveData(rc522_const, &command, 1, bufferATQA, bufferSize, &validBits, 0, false);
	if (status != STATUS_Code_OK) {
		return status;
	}
	if (*bufferSize != 2 || validBits != 0) {		// ATQA must be exactly 16 bits.
		return STATUS_Code_ERROR;
	}
	return STATUS_Code_OK;
} // End RC522_PICC_REQA_or_WUPA()

/**
 * Transmits SELECT/ANTICOLLISION commands to select a single PICC.
 * Before calling this function the PICCs must be placed in the READY(*) state by calling RC522_PICC_RequestA() or RC522_PICC_WakeupA().
 * On success:
 * 		- The chosen PICC is in state ACTIVE(*) and all other PICCs have returned to state IDLE/HALT. (Figure 7 of the ISO/IEC 14443-3 draft.)
 * 		- The UID size and value of the chosen PICC is returned in *uid along with the SAK.
 * 
 * A PICC UID consists of 4, 7 or 10 bytes.
 * Only 4 bytes can be specified in a SELECT command, so for the longer UIDs two or three iterations are used:
 * 		UID size	Number of UID bytes		Cascade levels		Example of PICC
 * 		========	===================		==============		===============
 * 		single				 4						1				MIFARE Classic
 * 		double				 7						2				MIFARE Ultralight
 * 		triple				10						3				Not currently in use?
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_PICC_Select(
	const RC522Const* rc522_const,
	RC522Uid *uid,			///< Pointer to Uid struct. Normally output, but can also be used to supply a known UID.
	uint8_t validBits		///< The number of known UID bits supplied in *uid. Normally 0. If set you must also supply uid->size.
) {
	bool uidComplete;
	bool selectDone;
	bool useCascadeTag;
	uint8_t cascadeLevel = 1;
	RC522Status result;
	uint8_t count;
	uint8_t checkBit;
	uint8_t index;
	uint8_t uidIndex;					// The first index in uid->uidByte[] that is used in the current Cascade Level.
	int8_t currentLevelKnownBits;		// The number of known UID bits in the current Cascade Level.
	uint8_t buffer[9];					// The SELECT/ANTICOLLISION commands uses a 7 byte standard frame + 2 bytes CRC_A
	uint8_t bufferUsed;				// The number of bytes used in the buffer, ie the number of bytes to transfer to the FIFO.
	uint8_t rxAlign;					// Used in BitFramingReg. Defines the bit position for the first bit received.
	uint8_t txLastBits;				// Used in BitFramingReg. The number of valid bits in the last transmitted byte. 
	uint8_t *responseBuffer;
	uint8_t responseLength;
	
	// Description of buffer structure:
	//		Byte 0: SEL 				Indicates the Cascade Level: PICC_CMD_SEL_CL1, PICC_CMD_SEL_CL2 or PICC_CMD_SEL_CL3
	//		Byte 1: NVB					Number of Valid Bits (in complete command, not just the UID): High nibble: complete bytes, Low nibble: Extra bits. 
	//		Byte 2: UID-data or CT		See explanation below. CT means Cascade Tag.
	//		Byte 3: UID-data
	//		Byte 4: UID-data
	//		Byte 5: UID-data
	//		Byte 6: BCC					Block Check Character - XOR of bytes 2-5
	//		Byte 7: CRC_A
	//		Byte 8: CRC_A
	// The BCC and CRC_A are only transmitted if we know all the UID bits of the current Cascade Level.
	//
	// Description of bytes 2-5: (Section 6.5.4 of the ISO/IEC 14443-3 draft: UID contents and cascade levels)
	//		UID size	Cascade level	Byte2	Byte3	Byte4	Byte5
	//		========	=============	=====	=====	=====	=====
	//		 4 bytes		1			uid0	uid1	uid2	uid3
	//		 7 bytes		1			CT		uid0	uid1	uid2
	//						2			uid3	uid4	uid5	uid6
	//		10 bytes		1			CT		uid0	uid1	uid2
	//						2			CT		uid3	uid4	uid5
	//						3			uid6	uid7	uid8	uid9
	
	// Sanity checks
	if (validBits > 80) {
		return STATUS_Code_INVALID;
	}
	
	// Prepare MFRC522
	RC522_PCD_ClearRegisterBitMask(rc522_const, PCD_Reg_CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.
	
	// Repeat Cascade Level loop until we have a complete UID.
	uidComplete = false;
	while (!uidComplete) {
		// Set the Cascade Level in the SEL byte, find out if we need to use the Cascade Tag in byte 2.
		switch (cascadeLevel) {
			case 1:
				buffer[0] = PICC_CMD_SEL_CL1;
				uidIndex = 0;
				useCascadeTag = validBits && uid->size > 4;	// When we know that the UID has more than 4 bytes
				break;
			
			case 2:
				buffer[0] = PICC_CMD_SEL_CL2;
				uidIndex = 3;
				useCascadeTag = validBits && uid->size > 7;	// When we know that the UID has more than 7 bytes
				break;
			
			case 3:
				buffer[0] = PICC_CMD_SEL_CL3;
				uidIndex = 6;
				useCascadeTag = false;						// Never used in CL3.
				break;
			
			default:
				return STATUS_Code_INTERNAL_ERROR;
				break;
		}
		
		// How many UID bits are known in this Cascade Level?
		currentLevelKnownBits = validBits - (8 * uidIndex);
		if (currentLevelKnownBits < 0) {
			currentLevelKnownBits = 0;
		}
		// Copy the known bits from uid->uidByte[] to buffer[]
		index = 2; // destination index in buffer[]
		if (useCascadeTag) {
			buffer[index++] = PICC_CMD_CT;
		}
		uint8_t bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of bytes needed to represent the known bits for this level.
		if (bytesToCopy) {
			uint8_t maxBytes = useCascadeTag ? 3 : 4; // Max 4 bytes in each Cascade Level. Only 3 left if we use the Cascade Tag
			if (bytesToCopy > maxBytes) {
				bytesToCopy = maxBytes;
			}
			for (count = 0; count < bytesToCopy; count++) {
				buffer[index++] = uid->uidByte[uidIndex + count];
			}
		}
		// Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
		if (useCascadeTag) {
			currentLevelKnownBits += 8;
		}
		
		// Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
		selectDone = false;
		while (!selectDone) {
			// Find out how many bits and bytes to send and receive.
			if (currentLevelKnownBits >= 32) { // All UID bits in this Cascade Level are known. This is a SELECT.
				//Serial.print(F("SELECT: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				buffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole bytes
				// Calculate BCC - Block Check Character
				buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
				// Calculate CRC_A
				result = RC522_PCD_CalculateCRC(rc522_const, buffer, 7, &buffer[7]);
				if (result != STATUS_Code_OK) {
					return result;
				}
				txLastBits		= 0; // 0 => All 8 bits are valid.
				bufferUsed		= 9;
				// Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
				responseBuffer	= &buffer[6];
				responseLength	= 3;
			}
			else { // This is an ANTICOLLISION.
				//Serial.print(F("ANTICOLLISION: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				txLastBits		= currentLevelKnownBits % 8;
				count			= currentLevelKnownBits / 8;	// Number of whole bytes in the UID part.
				index			= 2 + count;					// Number of whole bytes: SEL + NVB + UIDs
				buffer[1]		= (index << 4) + txLastBits;	// NVB - Number of Valid Bits
				bufferUsed		= index + (txLastBits ? 1 : 0);
				// Store response in the unused part of buffer
				responseBuffer	= &buffer[index];
				responseLength	= sizeof(buffer) - index;
			}
			
			// Set bit adjustments
			rxAlign = txLastBits;											// Having a separate variable is overkill. But it makes the next line easier to read.
			RC522_PCD_WriteRegister(rc522_const, PCD_Reg_BitFramingReg, (rxAlign << 4) + txLastBits);	// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]
			
			// Transmit the buffer and receive the response.
			result = RC522_PCD_TransceiveData(rc522_const, buffer, bufferUsed, responseBuffer, &responseLength, &txLastBits, rxAlign, false);
			if (result == STATUS_Code_COLLISION) { // More than one PICC in the field => collision.
				uint8_t valueOfCollReg = RC522_PCD_ReadRegister(rc522_const, PCD_Reg_CollReg); // CollReg[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
				if (valueOfCollReg & 0x20) { // CollPosNotValid
					return STATUS_Code_COLLISION; // Without a valid collision position we cannot continue
				}
				uint8_t collisionPos = valueOfCollReg & 0x1F; // Values 0-31, 0 means bit 32.
				if (collisionPos == 0) {
					collisionPos = 32;
				}
				if (collisionPos <= currentLevelKnownBits) { // No progress - should not happen 
					return STATUS_Code_INTERNAL_ERROR;
				}
				// Choose the PICC with the bit set.
				currentLevelKnownBits	= collisionPos;
				count			= currentLevelKnownBits % 8; // The bit to modify
				checkBit		= (currentLevelKnownBits - 1) % 8;
				index			= 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First byte is index 0.
				buffer[index]	|= (1 << checkBit);
			}
			else if (result != STATUS_Code_OK) {
				return result;
			}
			else { // STATUS_OK
				if (currentLevelKnownBits >= 32) { // This was a SELECT.
					selectDone = true; // No more anticollision 
					// We continue below outside the while.
				}
				else { // This was an ANTICOLLISION.
					// We now have all 32 bits of the UID in this Cascade Level
					currentLevelKnownBits = 32;
					// Run loop again to do the SELECT.
				}
			}
		} // End of while (!selectDone)
		
		// We do not check the CBB - it was constructed by us above.
		
		// Copy the found UID bytes from buffer[] to uid->uidByte[]
		index			= (buffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
		bytesToCopy		= (buffer[2] == PICC_CMD_CT) ? 3 : 4;
		for (count = 0; count < bytesToCopy; count++) {
			uid->uidByte[uidIndex + count] = buffer[index++];
		}
		
		// Check response SAK (Select Acknowledge)
		if (responseLength != 3 || txLastBits != 0) { // SAK must be exactly 24 bits (1 byte + CRC_A).
			return STATUS_Code_ERROR;
		}
		// Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those bytes are not needed anymore.
		result = RC522_PCD_CalculateCRC(rc522_const, responseBuffer, 1, &buffer[2]);
		if (result != STATUS_Code_OK) {
			return result;
		}
		if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2])) {
			return STATUS_Code_CRC_WRONG;
		}
		if (responseBuffer[0] & 0x04) { // Cascade bit set - UID not complete yes
			cascadeLevel++;
		}
		else {
			uidComplete = true;
			uid->sak = responseBuffer[0];
		}
	} // End of while (!uidComplete)
	
	// Set correct uid->size
	uid->size = 3 * cascadeLevel + 1;

	return STATUS_Code_OK;
} // End RC522_PICC_Select()

/**
 * Instructs a PICC in state ACTIVE(*) to go to state HALT.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */ 
RC522Status RC522_PICC_HaltA(const RC522Const* rc522_const) {
	RC522Status result;
	uint8_t buffer[4];
	
	// Build command buffer
	buffer[0] = PICC_CMD_HLTA;
	buffer[1] = 0;
	// Calculate CRC_A
	result = RC522_PCD_CalculateCRC(rc522_const, buffer, 2, &buffer[2]);
	if (result != STATUS_Code_OK) {
		return result;
	}
	
	// Send the command.
	// The standard says:
	//		If the PICC responds with any modulation during a period of 1 ms after the end of the frame containing the
	//		HLTA command, this response shall be interpreted as 'not acknowledge'.
	// We interpret that this way: Only STATUS_TIMEOUT is a success.
	result = RC522_PCD_TransceiveData(rc522_const, buffer, sizeof(buffer), NULL, 0, NULL, 0, false);
	if (result == STATUS_Code_TIMEOUT) {
		return STATUS_Code_OK;
	}
	if (result == STATUS_Code_OK) { // That is ironically NOT ok in this case ;-)
		return STATUS_Code_ERROR;
	}
	return result;
} // End PICC_HaltA()

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with MIFARE PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Executes the MFRC522 MFAuthent command.
 * This command manages MIFARE authentication to enable a secure communication to any MIFARE Mini, MIFARE 1K and MIFARE 4K card.
 * The authentication is described in the MFRC522 datasheet section 10.3.1.9 and http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1.
 * For use with MIFARE Classic PICCs.
 * The PICC must be selected - ie in state ACTIVE(*) - before calling this function.
 * Remember to call PCD_StopCrypto1() after communicating with the authenticated PICC - otherwise no new communications can start.
 * 
 * All keys are set to FFFFFFFFFFFFh at chip delivery.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise. Probably STATUS_TIMEOUT if you supply the wrong key.
 */
RC522Status RC522_PCD_Authenticate(
	const RC522Const* rc522_const,
	uint8_t command,		///< PICC_CMD_MF_AUTH_KEY_A or PICC_CMD_MF_AUTH_KEY_B
	uint8_t blockAddr, 		///< The block number. See numbering in the comments in the .h file.
	RC522MIFARE_Key *key,	///< Pointer to the Crypteo1 key to use (6 bytes)
	RC522Uid *uid			///< Pointer to Uid struct. The first 4 bytes of the UID is used.
) {
	uint8_t waitIRq = 0x10;		// IdleIRq
	
	// Build command buffer
	uint8_t sendData[12];
	sendData[0] = command;
	sendData[1] = blockAddr;
	for (uint8_t i = 0; i < MF_Misc_KEY_SIZE; i++) {	// 6 key bytes
		sendData[2+i] = key->keyByte[i];
	}
	// Use the last uid bytes as specified in http://cache.nxp.com/documents/application_note/AN10927.pdf
	// section 3.2.5 "MIFARE Classic Authentication".
	// The only missed case is the MF1Sxxxx shortcut activation,
	// but it requires cascade tag (CT) byte, that is not part of uid.
	for (uint8_t i = 0; i < 4; i++) {				// The last 4 bytes of the UID
		sendData[8+i] = uid->uidByte[i+uid->size-4];
	}
	
	// Start the authentication.
	return RC522_PCD_CommunicateWithPICC(rc522_const, PCD_Cmd_MFAuthent, waitIRq, &sendData[0], sizeof(sendData), NULL, 0, NULL, 0, false);
} // End PCD_Authenticate()

/**
 * Used to exit the PCD from its authenticated state.
 * Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start.
 */
inline void RC522_PCD_StopCrypto1(const RC522Const* rc522_const) {
	// Clear MFCrypto1On bit
	RC522_PCD_ClearRegisterBitMask(rc522_const, PCD_Reg_Status2Reg, 0x08); // Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved MFCrypto1On ModemState[2:0]
} // End PCD_StopCrypto1()

/**
 * Reads 16 bytes (+ 2 bytes CRC_A) from the active PICC.
 * 
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 * 
 * For MIFARE Ultralight only addresses 00h to 0Fh are decoded.
 * The MF0ICU1 returns a NAK for higher addresses.
 * The MF0ICU1 responds to the READ command by sending 16 bytes starting from the page address defined by the command argument.
 * For example; if blockAddr is 03h then pages 03h, 04h, 05h, 06h are returned.
 * A roll-back is implemented: If blockAddr is 0Eh, then the contents of pages 0Eh, 0Fh, 00h and 01h are returned.
 * 
 * The buffer must be at least 18 bytes because a CRC_A is also returned.
 * Checks the CRC_A before returning STATUS_OK.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_MIFARE_Read(
	const RC522Const* rc522_const,
	uint8_t blockAddr, 	///< MIFARE Classic: The block (0-0xff) number. MIFARE Ultralight: The first page to return data from.
	uint8_t *buffer,	///< The buffer to store the data in
	uint8_t *bufferSize	///< Buffer size, at least 18 bytes. Also number of bytes returned if STATUS_OK.
) {
	RC522Status result;
	
	// Sanity check
	if (buffer == NULL || *bufferSize < 18) {
		return STATUS_Code_NO_ROOM;
	}
	
	// Build command buffer
	buffer[0] = PICC_CMD_MF_READ;
	buffer[1] = blockAddr;
	// Calculate CRC_A
	result = RC522_PCD_CalculateCRC(rc522_const, buffer, 2, &buffer[2]);
	if (result != STATUS_Code_OK) {
		return result;
	}
	
	// Transmit the buffer and receive the response, validate CRC_A.
	return RC522_PCD_TransceiveData(rc522_const, buffer, 4, buffer, bufferSize, NULL, 0, true);
} // End MIFARE_Read()

/**
 * Writes 16 bytes to the active PICC.
 * 
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 * 
 * For MIFARE Ultralight the operation is called "COMPATIBILITY WRITE".
 * Even though 16 bytes are transferred to the Ultralight PICC, only the least significant 4 bytes (bytes 0 to 3)
 * are written to the specified address. It is recommended to set the remaining bytes 04h to 0Fh to all logic 0.
 * * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_MIFARE_Write(
	const RC522Const* rc522_const,
	uint8_t blockAddr, ///< MIFARE Classic: The block (0-0xff) number. MIFARE Ultralight: The page (2-15) to write to.
	uint8_t *buffer,	///< The 16 bytes to write to the PICC
	uint8_t bufferSize	///< Buffer size, must be at least 16 bytes. Exactly 16 bytes are written.
) {
	RC522Status result;
	
	// Sanity check
	if (buffer == NULL || bufferSize < 16) {
		return STATUS_Code_INVALID;
	}
	
	// Mifare Classic protocol requires two communications to perform a write.
	// Step 1: Tell the PICC we want to write to block blockAddr.
	uint8_t cmdBuffer[2];
	cmdBuffer[0] = PICC_CMD_MF_WRITE;
	cmdBuffer[1] = blockAddr;
	result = RC522_PCD_MIFARE_Transceive(rc522_const, cmdBuffer, 2, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_Code_OK) {
		return result;
	}
	
	// Step 2: Transfer the data
	result = RC522_PCD_MIFARE_Transceive(rc522_const, buffer, bufferSize, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_Code_OK) {
		return result;
	}
	
	return STATUS_Code_OK;
} // End MIFARE_Write()

/**
 * Writes a 4 byte page to the active MIFARE Ultralight PICC.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_MIFARE_Ultralight_Write(
	const RC522Const* rc522_const,
	uint8_t page, 		///< The page (2-15) to write to.
	uint8_t *buffer,	///< The 4 bytes to write to the PICC
	uint8_t bufferSize	///< Buffer size, must be at least 4 bytes. Exactly 4 bytes are written.
) {
	RC522Status result;
	
	// Sanity check
	if (buffer == NULL || bufferSize < 4) {
		return STATUS_Code_INVALID;
	}
	
	// Build commmand buffer
	uint8_t cmdBuffer[6];
	cmdBuffer[0] = PICC_CMD_UL_WRITE;
	cmdBuffer[1] = page;
	memcpy(&cmdBuffer[2], buffer, 4);
	
	// Perform the write
	result = RC522_PCD_MIFARE_Transceive(rc522_const, cmdBuffer, 6, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_Code_OK) {
		return result;
	}
	return STATUS_Code_OK;
} // End MIFARE_Ultralight_Write()

/**
 * MIFARE Decrement subtracts the delta from the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
inline RC522Status RC522_MIFARE_Decrement(
	const RC522Const* rc522_const,
	uint8_t blockAddr, ///< The block (0-0xff) number.
	int32_t delta		///< This number is subtracted from the value of block blockAddr.
) {
	return RC522_MIFARE_TwoStepHelper(rc522_const, PICC_CMD_MF_DECREMENT, blockAddr, delta);
} // End MIFARE_Decrement()

/**
 * MIFARE Increment adds the delta to the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
inline RC522Status RC522_MIFARE_Increment(
	const RC522Const* rc522_const,
	uint8_t blockAddr, ///< The block (0-0xff) number.
	int32_t delta		///< This number is added to the value of block blockAddr.
) {
	return RC522_MIFARE_TwoStepHelper(rc522_const, PICC_CMD_MF_INCREMENT, blockAddr, delta);
} // End MIFARE_Increment()

/**
 * MIFARE Restore copies the value of the addressed block into a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
inline RC522Status RC522_MIFARE_Restore(
	const RC522Const* rc522_const,
	uint8_t blockAddr ///< The block (0-0xff) number.
) {
	// The datasheet describes Restore as a two step operation, but does not explain what data to transfer in step 2.
	// Doing only a single step does not work, so I chose to transfer 0L in step two.
	return RC522_MIFARE_TwoStepHelper(rc522_const, PICC_CMD_MF_RESTORE, blockAddr, 0L);
} // End MIFARE_Restore()

/**
 * Helper function for the two-step MIFARE Classic protocol operations Decrement, Increment and Restore.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_MIFARE_TwoStepHelper(
	const RC522Const* rc522_const,
	uint8_t command,	///< The command to use
	uint8_t blockAddr,	///< The block (0-0xff) number.
	int32_t data	///< The data to transfer in step 2
) {
	RC522Status result;
	uint8_t cmdBuffer[2]; // We only need room for 2 bytes.
	
	// Step 1: Tell the PICC the command and block address
	cmdBuffer[0] = command;
	cmdBuffer[1] = blockAddr;
	result = RC522_PCD_MIFARE_Transceive(rc522_const, cmdBuffer, 2, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_Code_OK) {
		return result;
	}
	
	// Step 2: Transfer the data
	result = RC522_PCD_MIFARE_Transceive(rc522_const, (uint8_t *)&data, 4, true); // Adds CRC_A and accept timeout as success.
	if (result != STATUS_Code_OK) {
		return result;
	}
	
	return STATUS_Code_OK;
} // End MIFARE_TwoStepHelper()

/**
 * MIFARE Transfer writes the value stored in the volatile memory into one MIFARE Classic block.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_MIFARE_Transfer(
	const RC522Const* rc522_const,
	uint8_t blockAddr ///< The block (0-0xff) number.
) {
	RC522Status result;
	uint8_t cmdBuffer[2]; // We only need room for 2 bytes.
	
	// Tell the PICC we want to transfer the result into block blockAddr.
	cmdBuffer[0] = PICC_CMD_MF_TRANSFER;
	cmdBuffer[1] = blockAddr;
	result = RC522_PCD_MIFARE_Transceive(rc522_const, cmdBuffer, 2, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_Code_OK) {
		return result;
	}
	return STATUS_Code_OK;
} // End MIFARE_Transfer()

/**
 * Helper routine to read the current value from a Value Block.
 * 
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function. 
 * 
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[out]  value       Current value of the Value Block.
 * @return STATUS_OK on success, STATUS_??? otherwise.
  */
RC522Status RC522_MIFARE_GetValue(const RC522Const* rc522_const, uint8_t blockAddr, int32_t *value) {
	RC522Status status;
	uint8_t buffer[18];
	uint8_t size = sizeof(buffer);
	
	// Read the block
	status = RC522_MIFARE_Read(rc522_const, blockAddr, buffer, &size);
	if (status == STATUS_Code_OK) {
		// Extract the value
		*value =  ((int32_t)buffer[3] << 24)
				| ((int32_t)buffer[2] << 16)
				| ((int32_t)buffer[1] <<  8)
				| ((int32_t)buffer[0]      );
	}
	return status;
} // End MIFARE_GetValue()

/**
 * Helper routine to write a specific value into a Value Block.
 * 
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function. 
 * 
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[in]   value       New value of the Value Block.
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_MIFARE_SetValue(const RC522Const* rc522_const, uint8_t blockAddr, int32_t value) {
	uint8_t buffer[18];
	
	// Translate the int32_t into 4 bytes; repeated 2x in value block
	buffer[0] = buffer[ 8] = (value & 0xFF);
	buffer[1] = buffer[ 9] = (value & 0xFF00) >> 8;
	buffer[2] = buffer[10] = (value & 0xFF0000) >> 16;
	buffer[3] = buffer[11] = (value & 0xFF000000) >> 24;
	// Inverse 4 bytes also found in value block
	buffer[4] = ~buffer[0];
	buffer[5] = ~buffer[1];
	buffer[6] = ~buffer[2];
	buffer[7] = ~buffer[3];
	// Address 2x with inverse address 2x
	buffer[12] = buffer[14] = blockAddr;
	buffer[13] = buffer[15] = ~blockAddr;
	
	// Write the whole data block
	return RC522_MIFARE_Write(rc522_const, blockAddr, buffer, 16);
} // End MIFARE_SetValue()

/**
 * Authenticate with a NTAG216.
 * 
 * Only for NTAG216. First implemented by Gargantuanman.
 * 
 * @param[in]   passWord   password.
 * @param[in]   pACK       result success???.
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_PCD_NTAG216_AUTH(const RC522Const* rc522_const, uint8_t* passWord, uint8_t pACK[]) //Authenticate with 32bit password
{
	// TODO: Fix cmdBuffer length and rxlength. They really should match.
	//       (Better still, rxlength should not even be necessary.)

	RC522Status result;
	uint8_t cmdBuffer[18]; // We need room for 16 bytes data and 2 bytes CRC_A.
	
	cmdBuffer[0] = 0x1B; //Comando de autentificacion
	
	for (uint8_t i = 0; i<4; i++)
		cmdBuffer[i+1] = passWord[i];
	
	result = RC522_PCD_CalculateCRC(rc522_const, cmdBuffer, 5, &cmdBuffer[5]);
	
	if (result!=STATUS_Code_OK) {
		return result;
	}
	
	// Transceive the data, store the reply in cmdBuffer[]
	uint8_t waitIRq		= 0x30;	// RxIRq and IdleIRq
//	byte cmdBufferSize	= sizeof(cmdBuffer);
	uint8_t validBits		= 0;
	uint8_t rxlength		= 5;
	result = RC522_PCD_CommunicateWithPICC(rc522_const, PCD_Cmd_Transceive, waitIRq, cmdBuffer, 7, cmdBuffer, &rxlength, &validBits, 0, false);
	
	pACK[0] = cmdBuffer[0];
	pACK[1] = cmdBuffer[1];
	
	if (result!=STATUS_Code_OK) {
		return result;
	}
	
	return STATUS_Code_OK;
} // End PCD_NTAG216_AUTH()

/////////////////////////////////////////////////////////////////////////////////////
// Support functions
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Wrapper for MIFARE protocol communication.
 * Adds CRC_A, executes the Transceive command and checks that the response is MF_ACK or a timeout.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RC522Status RC522_PCD_MIFARE_Transceive(
	const RC522Const* rc522_const,
	uint8_t *sendData,	///< Pointer to the data to transfer to the FIFO. Do NOT include the CRC_A.
	uint8_t sendLen,	///< Number of bytes in sendData.
	bool acceptTimeout	///< True => A timeout is also success
) {
	RC522Status result;
	uint8_t cmdBuffer[18]; // We need room for 16 bytes data and 2 bytes CRC_A.
	
	// Sanity check
	if (sendData == NULL || sendLen > 16) {
		return STATUS_Code_INVALID;
	}
	
	// Copy sendData[] to cmdBuffer[] and add CRC_A
	memcpy(cmdBuffer, sendData, sendLen);
	result = RC522_PCD_CalculateCRC(rc522_const, cmdBuffer, sendLen, &cmdBuffer[sendLen]);
	if (result != STATUS_Code_OK) { 
		return result;
	}
	sendLen += 2;
	
	// Transceive the data, store the reply in cmdBuffer[]
	uint8_t waitIRq = 0x30;		// RxIRq and IdleIRq
	uint8_t cmdBufferSize = sizeof(cmdBuffer);
	uint8_t validBits = 0;
	result = RC522_PCD_CommunicateWithPICC(rc522_const, PCD_Cmd_Transceive, waitIRq, cmdBuffer, sendLen, cmdBuffer, &cmdBufferSize, &validBits, 0, false);
	if (acceptTimeout && result == STATUS_Code_TIMEOUT) {
		return STATUS_Code_OK;
	}
	if (result != STATUS_Code_OK) {
		return result;
	}
	// The PICC must reply with a 4 bit ACK
	if (cmdBufferSize != 1 || validBits != 4) {
		return STATUS_Code_ERROR;
	}
	if (cmdBuffer[0] != MF_Misc_ACK) {
		return STATUS_Code_MIFARE_NACK;
	}
	return STATUS_Code_OK;
} // End PCD_MIFARE_Transceive()

/**
 * Calculates the bit pattern needed for the specified access bits. In the [C1 C2 C3] tuples C1 is MSB (=4) and C3 is LSB (=1).
 */
void MIFARE_SetAccessBits(
	uint8_t *accessBitBuffer,	///< Pointer to byte 6, 7 and 8 in the sector trailer. Bytes [0..2] will be set.
	uint8_t g0,		///< Access bits [C1 C2 C3] for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
	uint8_t g1,		///< Access bits C1 C2 C3] for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
	uint8_t g2,		///< Access bits C1 C2 C3] for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
	uint8_t g3		///< Access bits C1 C2 C3] for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
) {
	uint8_t c1 = ((g3 & 4) << 1) | ((g2 & 4) << 0) | ((g1 & 4) >> 1) | ((g0 & 4) >> 2);
	uint8_t c2 = ((g3 & 2) << 2) | ((g2 & 2) << 1) | ((g1 & 2) << 0) | ((g0 & 2) >> 1);
	uint8_t c3 = ((g3 & 1) << 3) | ((g2 & 1) << 2) | ((g1 & 1) << 1) | ((g0 & 1) << 0);
	
	accessBitBuffer[0] = (~c2 & 0xF) << 4 | (~c1 & 0xF);
	accessBitBuffer[1] =          c1 << 4 | (~c3 & 0xF);
	accessBitBuffer[2] =          c3 << 4 | c2;
} // End MIFARE_SetAccessBits()

/**
 * Performs the "magic sequence" needed to get Chinese UID changeable
 * Mifare cards to allow writing to sector 0, where the card UID is stored.
 *
 * Note that you do not need to have selected the card through REQA or WUPA,
 * this sequence works immediately when the card is in the reader vicinity.
 * This means you can use this method even on "bricked" cards that your reader does
 * not recognise anymore (see MIFARE_UnbrickUidSector).
 * 
 * Of course with non-bricked devices, you're free to select them before calling this function.
 */
bool RC522_MIFARE_OpenUidBackdoor(const RC522Const* rc522_const) {
	// Magic sequence:
	// > 50 00 57 CD (HALT + CRC)
	// > 40 (7 bits only)
	// < A (4 bits only)
	// > 43
	// < A (4 bits only)
	// Then you can write to sector 0 without authenticating
	
	RC522_PICC_HaltA(rc522_const); // 50 00 57 CD
	
	uint8_t cmd = 0x40;
	uint8_t validBits = 7; /* Our command is only 7 bits. After receiving card response,
						  this will contain amount of valid response bits. */
	uint8_t response[32]; // Card's response is written here
	uint8_t received = sizeof(response);
	RC522Status status = RC522_PCD_TransceiveData(rc522_const, &cmd, 1, response, &received, &validBits, 0, false); // 40
	if(status != STATUS_Code_OK) {
		// if(logErrors) {
		// 	Serial.println(F("Card did not respond to 0x40 after HALT command. Are you sure it is a UID changeable one?"));
		// 	Serial.print(F("Error name: "));
		// 	Serial.println(GetStatusCodeName(status));
		// }
		return false;
	}
	if (received != 1 || response[0] != 0x0A) {
		// if (logErrors) {
		// 	Serial.print(F("Got bad response on backdoor 0x40 command: "));
		// 	Serial.print(response[0], HEX);
		// 	Serial.print(F(" ("));
		// 	Serial.print(validBits);
		// 	Serial.print(F(" valid bits)\r\n"));
		// }
		return false;
	}
	
	cmd = 0x43;
	validBits = 8;
	status = RC522_PCD_TransceiveData(rc522_const, &cmd, 1, response, &received, &validBits, 0, false); // 43
	if(status != STATUS_Code_OK) {
		// if(logErrors) {
		// 	Serial.println(F("Error in communication at command 0x43, after successfully executing 0x40"));
		// 	Serial.print(F("Error name: "));
		// 	Serial.println(GetStatusCodeName(status));
		// }
		return false;
	}
	if (received != 1 || response[0] != 0x0A) {
		// if (logErrors) {
		// 	Serial.print(F("Got bad response on backdoor 0x43 command: "));
		// 	Serial.print(response[0], HEX);
		// 	Serial.print(F(" ("));
		// 	Serial.print(validBits);
		// 	Serial.print(F(" valid bits)\r\n"));
		// }
		return false;
	}
	
	// You can now write to sector 0 without authenticating!
	return true;
} // End MIFARE_OpenUidBackdoor()

/**
 * Reads entire block 0, including all manufacturer data, and overwrites
 * that block with the new UID, a freshly calculated BCC, and the original
 * manufacturer data.
 *
 * It assumes a default KEY A of 0xFFFFFFFFFFFF.
 * Make sure to have selected the card before this function is called.
 */
bool RC522_MIFARE_SetUid(const RC522Const* rc522_const, uint8_t *newUid, uint8_t uidSize) {
	// UID + BCC byte can not be larger than 16 together
	if (!newUid || !uidSize || uidSize > 15) {
		// if (logErrors) {
		// 	Serial.println(F("New UID buffer empty, size 0, or size > 15 given"));
		// }
		return false;
	}
	
	// Authenticate for reading
	RC522MIFARE_Key key = {
		.keyByte = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
	};
	RC522Status status = RC522_PCD_Authenticate(rc522_const, PICC_CMD_MF_AUTH_KEY_A, 1, &key, &rc522_uid);
	if (status != STATUS_Code_OK) {
		
		if (status == STATUS_Code_TIMEOUT) {
			// We get a read timeout if no card is selected yet, so let's select one
			
			// Wake the card up again if sleeping
//			  byte atqa_answer[2];
//			  byte atqa_size = 2;
//			  PICC_WakeupA(atqa_answer, &atqa_size);
			
			if (!RC522_PICC_IsNewCardPresent(rc522_const) || !RC522_PICC_ReadCardSerial(rc522_const)) {
				// Serial.println(F("No card was previously selected, and none are available. Failed to set UID."));
				return false;
			}
			
			status = RC522_PCD_Authenticate(rc522_const, PICC_CMD_MF_AUTH_KEY_A, 1, &key, &rc522_uid);
			if (status != STATUS_Code_OK) {
				// We tried, time to give up
				// if (logErrors) {
				// 	Serial.println(F("Failed to authenticate to card for reading, could not set UID: "));
				// 	Serial.println(RC522_GetStatusCodeName(status));
				// }
				return false;
			}
		}
		else {
			// if (logErrors) {
			// 	Serial.print(F("PCD_Authenticate() failed: "));
			// 	Serial.println(RC522_GetStatusCodeName(status));
			// }
			return false;
		}
	}
	
	// Read block 0
	uint8_t block0_buffer[18];
	uint8_t byteCount = sizeof(block0_buffer);
	status = RC522_MIFARE_Read(rc522_const, 0, block0_buffer, &byteCount);
	if (status != STATUS_Code_OK) {
		// if (logErrors) {
		// 	Serial.print(F("MIFARE_Read() failed: "));
		// 	Serial.println(RC522_GetStatusCodeName(status));
		// 	Serial.println(F("Are you sure your KEY A for sector 0 is 0xFFFFFFFFFFFF?"));
		// }
		return false;
	}
	
	// Write new UID to the data we just read, and calculate BCC byte
	uint8_t bcc = 0;
	for (uint8_t i = 0; i < uidSize; i++) {
		block0_buffer[i] = newUid[i];
		bcc ^= newUid[i];
	}
	
	// Write BCC byte to buffer
	block0_buffer[uidSize] = bcc;
	
	// Stop encrypted traffic so we can send raw bytes
	RC522_PCD_StopCrypto1(rc522_const);
	
	// Activate UID backdoor
	if (!RC522_MIFARE_OpenUidBackdoor(rc522_const)) {
		return false;
	}
	
	// Write modified block 0 back to card
	status = RC522_MIFARE_Write(rc522_const, 0, block0_buffer, 16);
	if (status != STATUS_Code_OK) {
		return false;
	}
	
	// Wake the card up again
	uint8_t atqa_answer[2];
	uint8_t atqa_size = 2;
	RC522_PICC_WakeupA(rc522_const, atqa_answer, &atqa_size);
	
	return true;
}

/**
 * Resets entire sector 0 to zeroes, so the card can be read again by readers.
 */
bool RC522_MIFARE_UnbrickUidSector(const RC522Const* rc522_const) {
	RC522_MIFARE_OpenUidBackdoor(rc522_const);
	
	uint8_t block0_buffer[] = {0x01, 0x02, 0x03, 0x04, 0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	
	
	// Write modified block 0 back to card
	RC522Status status = RC522_MIFARE_Write(rc522_const, 0, block0_buffer, 16);
	if (status != STATUS_Code_OK) {
		// if (logErrors) {
		// 	Serial.print(F("MIFARE_Write() failed: "));
		// 	Serial.println(GetStatusCodeName(status));
		// }
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Convenience functions - does not add extra functionality
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns true if a PICC responds to PICC_CMD_REQA.
 * Only "new" cards in state IDLE are invited. Sleeping cards in state HALT are ignored.
 * 
 * @return bool
 */
bool RC522_PICC_IsNewCardPresent(const RC522Const* rc522_const)
{
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);

	// Reset baud rates
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_TxModeReg, 0x00);
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_RxModeReg, 0x00);
	// Reset ModWidthReg
	RC522_PCD_WriteRegister(rc522_const, PCD_Reg_ModWidthReg, 0x26);

	RC522Status result = RC522_PICC_RequestA(rc522_const, bufferATQA, &bufferSize);
	return (result == STATUS_Code_OK || result == STATUS_Code_COLLISION);
} // End RC522_PICC_IsNewCardPresent()

/**
 * Simple wrapper around RC522_PICC_Select.
 * Returns true if a UID could be read.
 * Remember to call RC522_PICC_IsNewCardPresent(), RC522_PICC_RequestA() or RC522_PICC_WakeupA() first.
 * The read UID is available in the class variable uid.
 * 
 * @return bool
 */
inline bool RC522_PICC_ReadCardSerial(const RC522Const* rc522_const)
{
	RC522Status result = RC522_PICC_Select(rc522_const, &rc522_uid, 0);
	return (result == STATUS_Code_OK);
} // End 

#endif