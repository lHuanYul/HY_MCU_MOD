#include "main/config.h"
#ifdef HY_MOD_STM32_RFID
/**
 * https://github.com/miguelbalboa/rfid
 * 
 * ver 1.4.12
 * 
 * Library to use Arduino MFRC522 module.
 * 
 * @authors Dr.Leong, Miguel Balboa, Søren Thing Andersen, Tom Clement, many more! See GitLog.
 * 
 * For more information read the README.
 * 
 * Please read this file for an overview and then MFRC522.cpp for comments on the specific functions.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// Custom code Start
#include "spi.h"
// Custom code End

#define MFRC522_SPICLOCK (4000000u)	// MFRC522 accept upto 10MHz, set to 4MHz.

// Size of the MFRC522 FIFO
#define FIFO_SIZE   64		// The FIFO is 64 bytes.
// Default value for unused pin
#define UNUSED_PIN  UINT8_MAX

// MFRC522 registers. Described in chapter 9 of the datasheet.
// When using SPI all addresses are shifted one bit left in the "SPI address byte" (section 8.1.2.3)
typedef uint8_t PCD_Register;
// Page 0: Command and status
//									((PCD_Register)(0x00))		// reserved for future use
#define PCD_Reg_CommandReg			((PCD_Register)(0x01 << 1))	// starts and stops command execution
#define PCD_Reg_ComIEnReg			((PCD_Register)(0x02 << 1))	// enable and disable interrupt request control bits
#define PCD_Reg_DivIEnReg			((PCD_Register)(0x03 << 1))	// enable and disable interrupt request control bits
#define PCD_Reg_ComIrqReg			((PCD_Register)(0x04 << 1))	// interrupt request bits
#define PCD_Reg_DivIrqReg			((PCD_Register)(0x05 << 1))	// interrupt request bits
#define PCD_Reg_ErrorReg			((PCD_Register)(0x06 << 1))	// error bits showing the error status of the last command executed 
#define PCD_Reg_Status1Reg			((PCD_Register)(0x07 << 1))	// communication status bits
#define PCD_Reg_Status2Reg			((PCD_Register)(0x08 << 1))	// receiver and transmitter status bits
#define PCD_Reg_FIFODataReg			((PCD_Register)(0x09 << 1))	// input and output of 64 byte FIFO buffer
#define PCD_Reg_FIFOLevelReg		((PCD_Register)(0x0A << 1))	// number of bytes stored in the FIFO buffer
#define PCD_Reg_WaterLevelReg		((PCD_Register)(0x0B << 1))	// level for FIFO underflow and overflow warning
#define PCD_Reg_ControlReg			((PCD_Register)(0x0C << 1))	// miscellaneous control registers
#define PCD_Reg_BitFramingReg		((PCD_Register)(0x0D << 1))	// adjustments for bit-oriented frames
#define PCD_Reg_CollReg				((PCD_Register)(0x0E << 1))	// bit position of the first bit-collision detected on the RF interface
//						  			((PCD_Register)(0x0F))		// reserved for future use
// Page 1: Command
// 						  			((PCD_Register)(0x10))		// reserved for future use
#define PCD_Reg_ModeReg				((PCD_Register)(0x11 << 1))	// defines general modes for transmitting and receiving 
#define PCD_Reg_TxModeReg			((PCD_Register)(0x12 << 1))	// defines transmission data rate and framing
#define PCD_Reg_RxModeReg			((PCD_Register)(0x13 << 1))	// defines reception data rate and framing
#define PCD_Reg_TxControlReg		((PCD_Register)(0x14 << 1))	// controls the logical behavior of the antenna driver pins TX1 and TX2
#define PCD_Reg_TxASKReg			((PCD_Register)(0x15 << 1))	// controls the setting of the transmission modulation
#define PCD_Reg_TxSelReg			((PCD_Register)(0x16 << 1))	// selects the internal sources for the antenna driver
#define PCD_Reg_RxSelReg			((PCD_Register)(0x17 << 1))	// selects internal receiver settings
#define PCD_Reg_RxThresholdReg		((PCD_Register)(0x18 << 1))	// selects thresholds for the bit decoder
#define PCD_Reg_DemodReg			((PCD_Register)(0x19 << 1))	// defines demodulator settings
// 						  			((PCD_Register)(0x1A))		// reserved for future use
// 						  			((PCD_Register)(0x1B))		// reserved for future use
#define PCD_Reg_MfTxReg				((PCD_Register)(0x1C << 1))	// controls some MIFARE communication transmit parameters
#define PCD_Reg_MfRxReg				((PCD_Register)(0x1D << 1))	// controls some MIFARE communication receive parameters
// 						  			((PCD_Register)(0x1E))		// reserved for future use
#define PCD_Reg_SerialSpeedReg		((PCD_Register)(0x1F << 1))	// selects the speed of the serial UART interface
// Page 2: Configuration
// 						  			((PCD_Register)(0x20))		// reserved for future use
#define PCD_Reg_CRCResultRegH		((PCD_Register)(0x21 << 1))	// shows the MSB and LSB values of the CRC calculation
#define PCD_Reg_CRCResultRegL		((PCD_Register)(0x22 << 1))
// 						  			((PCD_Register)(0x23))		// reserved for future use
#define PCD_Reg_ModWidthReg			((PCD_Register)(0x24 << 1))	// controls the ModWidth setting?
// 						  			((PCD_Register)(0x25))		// reserved for future use
#define PCD_Reg_RFCfgReg			((PCD_Register)(0x26 << 1))	// configures the receiver gain
#define PCD_Reg_GsNReg				((PCD_Register)(0x27 << 1))	// selects the conductance of the antenna driver pins TX1 and TX2 for modulation 
#define PCD_Reg_CWGsPReg			((PCD_Register)(0x28 << 1))	// defines the conductance of the p-driver output during periods of no modulation
#define PCD_Reg_ModGsPReg			((PCD_Register)(0x29 << 1))	// defines the conductance of the p-driver output during periods of modulation
#define PCD_Reg_TModeReg			((PCD_Register)(0x2A << 1))	// defines settings for the internal timer
#define PCD_Reg_TPrescalerReg		((PCD_Register)(0x2B << 1))	// the lower 8 bits of the TPrescaler value. The 4 high bits are in TModeReg.
#define PCD_Reg_TReloadRegH			((PCD_Register)(0x2C << 1))	// defines the 16-bit timer reload value
#define PCD_Reg_TReloadRegL			((PCD_Register)(0x2D << 1))
#define PCD_Reg_TCounterValueRegH	((PCD_Register)(0x2E << 1))	// shows the 16-bit timer value
#define PCD_Reg_TCounterValueRegL	((PCD_Register)(0x2F << 1))
// Page 3: Test Registers
// 						  		((PCD_Register)(0x30))		// reserved for future use
#define PCD_Reg_TestSel1Reg		((PCD_Register)(0x31 << 1))	// general test signal configuration
#define PCD_Reg_TestSel2Reg		((PCD_Register)(0x32 << 1))	// general test signal configuration
#define PCD_Reg_TestPinEnReg	((PCD_Register)(0x33 << 1))	// enables pin output driver on pins D1 to D7
#define PCD_Reg_TestPinValueReg	((PCD_Register)(0x34 << 1))	// defines the values for D1 to D7 when it is used as an I/O bus
#define PCD_Reg_TestBusReg		((PCD_Register)(0x35 << 1))	// shows the status of the internal test bus
#define PCD_Reg_AutoTestReg		((PCD_Register)(0x36 << 1))	// controls the digital self-test
#define PCD_Reg_VersionReg		((PCD_Register)(0x37 << 1))	// shows the software version
#define PCD_Reg_AnalogTestReg	((PCD_Register)(0x38 << 1))	// controls the pins AUX1 and AUX2
#define PCD_Reg_TestDAC1Reg		((PCD_Register)(0x39 << 1))	// defines the test value for TestDAC1
#define PCD_Reg_TestDAC2Reg		((PCD_Register)(0x3A << 1))	// defines the test value for TestDAC2
#define PCD_Reg_TestADCReg		((PCD_Register)(0x3B << 1))	// shows the value of ADC I and Q channels
// 						  		((PCD_Register)(0x3C))		// reserved for production tests
// 						  		((PCD_Register)(0x3D))		// reserved for production tests
// 						  		((PCD_Register)(0x3E))		// reserved for production tests
// 						  		((PCD_Register)(0x3F))		// reserved for production tests

// MFRC522 commands. Described in chapter 10 of the datasheet.
typedef uint8_t PCD_Command;
#define PCD_Cmd_Idle				((PCD_Command)(0x00))	// no action, cancels current command execution
#define PCD_Cmd_Mem					((PCD_Command)(0x01))	// stores 25 bytes into the internal buffer
#define PCD_Cmd_GenerateRandomID	((PCD_Command)(0x02))	// generates a 10-byte random ID number
#define PCD_Cmd_CalcCRC				((PCD_Command)(0x03))	// activates the CRC coprocessor or performs a self-test
#define PCD_Cmd_Transmit			((PCD_Command)(0x04))	// transmits data from the FIFO buffer
#define PCD_Cmd_NoCmdChange			((PCD_Command)(0x07))	// no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
#define PCD_Cmd_Receive				((PCD_Command)(0x08))	// activates the receiver circuits
#define PCD_Cmd_Transceive 			((PCD_Command)(0x0C))	// transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
#define PCD_Cmd_MFAuthent 			((PCD_Command)(0x0E))	// performs the MIFARE standard authentication as a reader
#define PCD_Cmd_SoftReset			((PCD_Command)(0x0F))	// resets the MFRC522

// MFRC522 RxGain[2:0] masks, defines the receiver's signal voltage gain factor (on the PCD).
// Described in 9.3.3.6 / table 98 of the datasheet at http://www.nxp.com/documents/data_sheet/MFRC522.pdf
typedef uint8_t PCD_RxGain;
#define PCD_RxGain_18dB			((PCD_RxGain)(0x00 << 4))	// 000b - 18 dB, minimum
#define PCD_RxGain_23dB			((PCD_RxGain)(0x01 << 4))	// 001b - 23 dB
#define PCD_RxGain_18dB_2		((PCD_RxGain)(0x02 << 4))	// 010b - 18 dB, it seems 010b is a duplicate for 000b
#define PCD_RxGain_23dB_2		((PCD_RxGain)(0x03 << 4))	// 011b - 23 dB, it seems 011b is a duplicate for 001b
#define PCD_RxGain_33dB			((PCD_RxGain)(0x04 << 4))	// 100b - 33 dB, average, and typical default
#define PCD_RxGain_38dB			((PCD_RxGain)(0x05 << 4))	// 101b - 38 dB
#define PCD_RxGain_43dB			((PCD_RxGain)(0x06 << 4))	// 110b - 43 dB
#define PCD_RxGain_48dB			((PCD_RxGain)(0x07 << 4))	// 111b - 48 dB, maximum
#define PCD_RxGain_min			((PCD_RxGain)(0x00 << 4))	// 000b - 18 dB, minimum, convenience for RxGain_18dB
#define PCD_RxGain_avg			((PCD_RxGain)(0x04 << 4))	// 100b - 33 dB, average, convenience for RxGain_33dB
#define PCD_RxGain_max			((PCD_RxGain)(0x07 << 4))	// 111b - 48 dB, maximum, convenience for RxGain_48dB

// Commands sent to the PICC.
typedef uint8_t PICC_Command;
	// The commands used by the PCD to manage communication with several PICCs (ISO 14443-3, Type A, section 6.4)
#define PICC_CMD_REQA			((PICC_Command)(0x26))	// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
#define PICC_CMD_WUPA			((PICC_Command)(0x52))	// Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
#define PICC_CMD_CT				((PICC_Command)(0x88))	// Cascade Tag. Not really a command, but used during anti collision.
#define PICC_CMD_SEL_CL1		((PICC_Command)(0x93))	// Anti collision/Select, Cascade Level 1
#define PICC_CMD_SEL_CL2		((PICC_Command)(0x95))	// Anti collision/Select, Cascade Level 2
#define PICC_CMD_SEL_CL3		((PICC_Command)(0x97))	// Anti collision/Select, Cascade Level 3
#define PICC_CMD_HLTA			((PICC_Command)(0x50))	// HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
#define PICC_CMD_RATS           ((PICC_Command)(0xE0))	// Request command for Answer To Reset.
	// The commands used for MIFARE Classic (from http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf, Section 9)
	// Use PCD_MFAuthent to authenticate access to a sector, then use these commands to read/write/modify the blocks on the sector.
	// The read/write commands can also be used for MIFARE Ultralight.
#define PICC_CMD_MF_AUTH_KEY_A	((PICC_Command)(0x60))	// Perform authentication with Key A
#define PICC_CMD_MF_AUTH_KEY_B	((PICC_Command)(0x61))	// Perform authentication with Key B
#define PICC_CMD_MF_READ		((PICC_Command)(0x30))	// Reads one 16 byte block from the authenticated sector of the PICC. Also used for MIFARE Ultralight.
#define PICC_CMD_MF_WRITE		((PICC_Command)(0xA0))	// Writes one 16 byte block to the authenticated sector of the PICC. Called "COMPATIBILITY WRITE" for MIFARE Ultralight.
#define PICC_CMD_MF_DECREMENT	((PICC_Command)(0xC0))	// Decrements the contents of a block and stores the result in the internal data register.
#define PICC_CMD_MF_INCREMENT	((PICC_Command)(0xC1))	// Increments the contents of a block and stores the result in the internal data register.
#define PICC_CMD_MF_RESTORE		((PICC_Command)(0xC2))	// Reads the contents of a block into the internal data register.
#define PICC_CMD_MF_TRANSFER	((PICC_Command)(0xB0))	// Writes the contents of the internal data register to a block.
	// The commands used for MIFARE Ultralight (from http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf, Section 8.6)
	// The PICC_CMD_MF_READ and PICC_CMD_MF_WRITE can also be used for MIFARE Ultralight.
#define PICC_CMD_UL_WRITE		((PICC_Command)(0xA2))	// Writes one 4 byte page to the PICC.

// MIFARE constants that does not fit anywhere else
typedef uint8_t MIFARE_Misc;
#define MF_Misc_ACK				((MIFARE_Misc)(0xA))	// The MIFARE Classic uses a 4 bit ACK/NAK. Any other value than 0xA is NAK.
#define MF_Misc_KEY_SIZE		((MIFARE_Misc)(6))		// A Mifare Crypto1 key is 6 bytes.

// PICC types we can detect. Remember to update PICC_GetTypeName() if you add more.
// last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
typedef uint8_t PICC_Type;
#define PICC_TYPE_UNKNOWN			((PICC_Type)(0))
#define PICC_TYPE_ISO_14443_4		((PICC_Type)(1))	// PICC compliant with ISO/IEC 14443-4 
#define PICC_TYPE_ISO_18092			((PICC_Type)(2)) 	// PICC compliant with ISO/IEC 18092 (NFC)
#define PICC_TYPE_MIFARE_MINI		((PICC_Type)(3))	// MIFARE Classic protocol, 320 bytes
#define PICC_TYPE_MIFARE_1K			((PICC_Type)(4))	// MIFARE Classic protocol, 1KB
#define PICC_TYPE_MIFARE_4K			((PICC_Type)(5))	// MIFARE Classic protocol, 4KB
#define PICC_TYPE_MIFARE_UL			((PICC_Type)(6))	// MIFARE Ultralight or Ultralight C
#define PICC_TYPE_MIFARE_PLUS		((PICC_Type)(7))	// MIFARE Plus
#define PICC_TYPE_MIFARE_DESFIRE	((PICC_Type)(8))	// MIFARE DESFire
#define PICC_TYPE_TNP3XXX			((PICC_Type)(9))	// Only mentioned in NXP AN 10833 MIFARE Type Identification Procedure
#define PICC_TYPE_NOT_COMPLETE		((PICC_Type)(0xff))	// SAK indicates UID is not complete.

// Return codes from the functions in this class. Remember to update GetStatusCodeName() if you add more.
// last value set to 0xff, then compiler uses less ram, it seems some optimisations are triggered
typedef uint8_t RC522Status;
#define STATUS_Code_OK				((RC522Status)(0))		// Success
#define STATUS_Code_ERROR			((RC522Status)(1))		// Error in communication
#define STATUS_Code_COLLISION		((RC522Status)(2))		// Collission detected
#define STATUS_Code_TIMEOUT			((RC522Status)(3))		// Timeout in communication.
#define STATUS_Code_NO_ROOM			((RC522Status)(4))		// A buffer is not big enough.
#define STATUS_Code_INTERNAL_ERROR	((RC522Status)(5))		// Internal error in the code. Should not happen ;-)
#define STATUS_Code_INVALID			((RC522Status)(6))		// Invalid argument.
#define STATUS_Code_CRC_WRONG		((RC522Status)(7))		// The CRC_A does not match
#define STATUS_Code_MIFARE_NACK		((RC522Status)(0xff))	// A MIFARE PICC responded with NAK.

// A struct used for passing the UID of a PICC.
typedef struct RC522Uid {
	uint8_t		size;			// Number of bytes in the UID. 4, 7 or 10.
	uint8_t		uidByte[10];
	uint8_t		sak;			// The SAK (Select acknowledge) byte returned from the PICC after successful selection.
} RC522Uid;

// A struct used for passing a MIFARE Crypto1 key
typedef struct RC522MIFARE_Key {
	uint8_t		keyByte[MF_Misc_KEY_SIZE];
} RC522MIFARE_Key;

// Member variables
extern RC522Uid rc522_uid;			// Used by RC522_PICC_ReadCardSerial().

// Custom code Start
#include "spi.h"
#include "main/config.h"
typedef struct RC522Const
{
    SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* SDA_GPIOx;
    uint16_t      SDA_GPIO_PIN_x;
    GPIO_TypeDef* IRQ_GPIOx;
    uint16_t      IRQ_GPIO_PIN_x;
    GPIO_TypeDef* RST_GPIOx;
    uint16_t      RST_GPIO_PIN_x;
} RC522Const;
extern const RC522MIFARE_Key rc522_default_key;
extern uint8_t rc522_version;
// Custom code End

/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the MFRC522
/////////////////////////////////////////////////////////////////////////////////////
void RC522_PCD_WriteRegister(const RC522Const* rc522_const, PCD_Register reg, uint8_t value);
void RC522_PCD_WriteRegister_full(const RC522Const* rc522_const, PCD_Register reg, uint8_t count, uint8_t *values);
uint8_t RC522_PCD_ReadRegister(const RC522Const* rc522_const, PCD_Register reg);
void RC522_PCD_ReadRegister_full(const RC522Const* rc522_const, PCD_Register reg, uint8_t count, uint8_t *values, uint8_t rxAlign); // defalt: rxAlign = 0
void RC522_PCD_SetRegisterBitMask(const RC522Const* rc522_const, PCD_Register reg, uint8_t mask);
void RC522_PCD_ClearRegisterBitMask(const RC522Const* rc522_const, PCD_Register reg, uint8_t mask);
RC522Status RC522_PCD_CalculateCRC(const RC522Const* rc522_const, uint8_t *data, uint8_t length, uint8_t *result);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for manipulating the MFRC522
/////////////////////////////////////////////////////////////////////////////////////
void RC522_PCD_Init(const RC522Const* rc522_const);
// void RC522_PCD_Init_rs(const RC522Const* rc522_const,uint8_t resetPowerDownPin);
// void RC522_PCD_Init_rs_p(const RC522Const* rc522_const,uint8_t chipSelectPin, uint8_t resetPowerDownPin);
void RC522_PCD_Reset(const RC522Const* rc522_const);
void RC522_PCD_AntennaOn(const RC522Const* rc522_const);
void RC522_PCD_AntennaOff(const RC522Const* rc522_const);
uint8_t RC522_PCD_GetAntennaGain(const RC522Const* rc522_const);
void RC522_PCD_SetAntennaGain(const RC522Const* rc522_const, uint8_t mask);
bool RC522_PCD_PerformSelfTest(const RC522Const* rc522_const);

/////////////////////////////////////////////////////////////////////////////////////
// Power control functions
/////////////////////////////////////////////////////////////////////////////////////
void RC522_PCD_SoftPowerDown(const RC522Const* rc522_const);
void RC522_PCD_SoftPowerUp(const RC522Const* rc522_const);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with PICCs
/////////////////////////////////////////////////////////////////////////////////////
RC522Status RC522_PCD_TransceiveData(
	const RC522Const* rc522_const, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen,
	uint8_t *validBits, uint8_t rxAlign, bool checkCRC
); // defalt: *validBits = nullptr, rxAlign = 0, checkCRC = false
RC522Status RC522_PCD_CommunicateWithPICC(
	const RC522Const* rc522_const, uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen,
	uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC
); // defalt: *backData = nullptr, *backLen = nullptr, *validBits = nullptr, rxAlign = 0, checkCRC = false
RC522Status RC522_PICC_RequestA(const RC522Const* rc522_const, uint8_t *bufferATQA, uint8_t *bufferSize);
RC522Status RC522_PICC_WakeupA(const RC522Const* rc522_const, uint8_t *bufferATQA, uint8_t *bufferSize);
RC522Status RC522_PICC_REQA_or_WUPA(const RC522Const* rc522_const, uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize);
RC522Status RC522_PICC_Select(const RC522Const* rc522_const, RC522Uid *uid, uint8_t validBits); // defalt: validBits = 0
RC522Status RC522_PICC_HaltA(const RC522Const* rc522_const);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with MIFARE PICCs
/////////////////////////////////////////////////////////////////////////////////////
RC522Status RC522_PCD_Authenticate(const RC522Const* rc522_const, uint8_t command, uint8_t blockAddr, RC522MIFARE_Key *key, RC522Uid *uid);
void RC522_PCD_StopCrypto1(const RC522Const* rc522_const);
RC522Status RC522_MIFARE_Read(const RC522Const* rc522_const, uint8_t blockAddr, uint8_t *buffer, uint8_t *bufferSize);
RC522Status RC522_MIFARE_Write(const RC522Const* rc522_const, uint8_t blockAddr, uint8_t *buffer, uint8_t bufferSize);
RC522Status RC522_MIFARE_Ultralight_Write(const RC522Const* rc522_const, uint8_t page, uint8_t *buffer, uint8_t bufferSize);
RC522Status RC522_MIFARE_Decrement(const RC522Const* rc522_const, uint8_t blockAddr, int32_t delta);
RC522Status RC522_MIFARE_Increment(const RC522Const* rc522_const, uint8_t blockAddr, int32_t delta);
RC522Status RC522_MIFARE_Restore(const RC522Const* rc522_const, uint8_t blockAddr);
RC522Status RC522_MIFARE_TwoStepHelper(const RC522Const* rc522_const, uint8_t command, uint8_t blockAddr, int32_t data);
RC522Status RC522_MIFARE_Transfer(const RC522Const* rc522_const, uint8_t blockAddr);
RC522Status RC522_MIFARE_GetValue(const RC522Const* rc522_const, uint8_t blockAddr, int32_t *value);
RC522Status RC522_MIFARE_SetValue(const RC522Const* rc522_const, uint8_t blockAddr, int32_t value);
RC522Status RC522_PCD_NTAG216_AUTH(const RC522Const* rc522_const, uint8_t *passWord, uint8_t pACK[]);

/////////////////////////////////////////////////////////////////////////////////////
// Support functions
/////////////////////////////////////////////////////////////////////////////////////
RC522Status RC522_PCD_MIFARE_Transceive(const RC522Const* rc522_const, uint8_t *sendData, uint8_t sendLen, bool acceptTimeout); // defalt: acceptTimeout = false
// old function used too much memory, now name moved to flash; if you need char, copy from flash to memory
// const char *GetStatusCodeName(byte code);
// const char *RC522_GetStatusCodeName(const RC522Const* rc522_const, RC522Status code);
// PICC_Type RC522_PICC_GetType(const RC522Const* rc522_const, uint8_t sak);
// old function used too much memory, now name moved to flash; if you need char, copy from flash to memory
// const char *PICC_GetTypeName(byte type);
// const char *RC522_PICC_GetTypeName(const RC522Const* rc522_const, PICC_Type type);

// Support functions for debuging
// void RC522_PCD_DumpVersionToSerial(const RC522Const* rc522_const);
// void RC522_PICC_DumpToSerial(const RC522Const* rc522_const, RC522Uid *uid);
// void RC522_PICC_DumpDetailsToSerial(const RC522Const* rc522_const, RC522Uid *uid);
// void RC522_PICC_DumpMifareClassicToSerial(const RC522Const* rc522_const, RC522Uid *uid, PICC_Type piccType, RC522MIFARE_Key *key);
// void RC522_PICC_DumpMifareClassicSectorToSerial(const RC522Const* rc522_const, RC522Uid *uid, RC522MIFARE_Key *key, uint8_t sector);
// void RC522_PICC_DumpMifareUltralightToSerial(const RC522Const* rc522_const);

// Advanced functions for MIFARE
void RC522_MIFARE_SetAccessBits(uint8_t *accessBitBuffer, uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3);
bool RC522_MIFARE_OpenUidBackdoor(const RC522Const* rc522_const);
bool RC522_MIFARE_SetUid(const RC522Const* rc522_const, uint8_t *newUid, uint8_t uidSize);
bool RC522_MIFARE_UnbrickUidSector(const RC522Const* rc522_const);

/////////////////////////////////////////////////////////////////////////////////////
// Convenience functions - does not add extra functionality
/////////////////////////////////////////////////////////////////////////////////////
bool RC522_PICC_IsNewCardPresent(const RC522Const* rc522_const);
bool RC522_PICC_ReadCardSerial(const RC522Const* rc522_const);

#endif