/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : sd2.c
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#if SD2_ENABLE
#include "mpTrace.h"

#include "Mcard.h"
#include "uti.h"
#include "sd.h"

/*
// Constant declarations
*/
#define HW_SUPPORT_MMC_4_0
#define SDHC_FLAG	

#ifdef  SDHC_FLAG
#define SDHC_CHECK_PATTERN		0xaa
#define NORMAL					1 // 2.7~3.6
#define LOW_VOL					2
#define SDHC_VOLTAGE_SUPPLIED	NORMAL
#define HCS						0x40000000
#define S18R                    0x01000000
#define S18A                    S18R
#endif

#define MMC_DEFAULT_ADDR  0x00100000
#define HOST_OCR_VALUE    0x00ff8000
#define MMC_VER_4_0       0x4

#define MMC_BUS_WIDTH	0x1	// 0: 1 bit, 1: 4bit, 2: 8bit

#define RETRY_TIME		3

#define SD_INIT_CLK_KHZ			400     // 400 kHz
#define SD_NORMAL_CLK_KHZ		24000  // 24000 kHz
#define SD_HISPEED_CLK_KHZ		50000  // 50000 kHz

enum
{
	SD_STATE_UNKNOWN,
	SD_STATE_STANDBY,	// the state is after CMD3 sent successful
	SD_STATE_TRANSFER,
};

/*
// Structure declarations
*/
typedef struct
{
	BYTE bType;
	BYTE SpecVer;
	WORD wSectorSize;
	WORD wSectorSizeExp;
	DWORD dwBusWidth;
	DWORD dwCardTag;
	DWORD dwClock;
	DWORD dwClockAdj;
	DWORD dwRelativeCardAddr;
	DWORD dwCapacity;
} ST_SD_INFO;

/*
// Type declarations
*/

/*
// Variable declarations
*/
static ST_SD_INFO SdInfo = {SD_TYPE, 0, MCARD_SECTOR_SIZE, MCARD_SECTOR_SIZE_EXP, DBUS_1BIT, MCARD_DMA_SD, SD_INIT_CLK_KHZ, NORMAL_SPEED, 0, 0};
static BYTE bDescriptor[] = "SD2";
static BYTE SdState = SD_STATE_UNKNOWN;

/*
// Macro declarations
*/


/*
// Static function prototype
*/
static void CommandProcess(void *pMcardDev);
static void SetClockSpeed(DWORD dwSpeed);
static SWORD GoTransferMode();
static SWORD WaitTranState();
static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD LogicalRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD LogicalWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD LowLevelFormat();

/*
// Definition of internal functions
*/

/*
// Definition of local functions 
*/
static void SetClockSpeed(DWORD dwSpeed)
{
	SdInfo.dwClock = dwSpeed;	
	SetSD2Clock(SdInfo.dwClock);
}

static void Power_On(void)
{
	McardPowerOn(DEV_SD2);
}

static void Power_Off(void)
{
	McardPowerOff(DEV_SD2);
}

static SWORD SdHost_SendAppCmd(DWORD cmd, DWORD StdArg, DWORD *resp, DWORD Wait)
{
	SWORD ret = SD_PASS;
	
	SdHost2_SendCmd(CMD55, SdInfo.dwRelativeCardAddr);
	if (SdHost2_WaitCardResponse(NULL) != SD_PASS)
		MP_DEBUG("Card has no response to CMD_APP_CMD");
	SdHost2_SendCmd(cmd, StdArg);
	if (Wait)
	{
		if ((ret = SdHost2_WaitCardResponse(resp)) != SD_PASS)
			MP_DEBUG("Card has no response to ACMD%d", cmd&0x3f);
	}

	return ret;
}

static SWORD SdHost_SendStdCmd(DWORD cmd, DWORD arguement, DWORD *resp, DWORD Wait)
{
	SWORD ret = SD_PASS;
	
	SdHost2_SendCmd(cmd, arguement);
	if (Wait)
	{
		if ((ret = SdHost2_WaitCardResponse(resp)) != SD_PASS)
			MP_DEBUG("Card has no response to CMD%d", cmd&0x3f);
	}

	return ret;
}

static SWORD WaitTranState()
{
	SWORD ret = SD_PASS;
	DWORD dwCurState;
	DWORD tmr = GetSysTime();

	while (SystemGetElapsedTime(tmr) < 1000)	// wait 1 sec for ready
	{
		if (SdHost_SendStdCmd(CMD13, SdInfo.dwRelativeCardAddr, &dwCurState, 1) != SD_PASS)
		{
			mpDebugPrint("CMD13 Wait Response fail");
			ret = RES_TIMEOUT;
			break;
		}
		dwCurState = (dwCurState >> 9) & 0xf;
		if (dwCurState == TRANSFER_STATE)
		{
			//MP_DEBUG("Transfer state");
			ret = SD_PASS;
			break;
		}
		else if (dwCurState == RECEIVE_STATE)
		{
			mpDebugPrint("Receive state!!!");
			TaskYield();
		}
		else if (dwCurState == STANDBY_STATE)
		{
			TaskYield();
		}
		else if (dwCurState == PROGRAM_STATE)
		{
			MP_DEBUG("Program state! so slow card?");
			TaskYield();
		}
		else if (dwCurState == NULL_STATE)
		{
			mpDebugPrint("Null state??");
			TaskYield();
		}		
	}
	if (dwCurState == STANDBY_STATE)
	{
		mpDebugPrint("Standby state? why??");
		if (SdHost_SendStdCmd(CMD7, SdInfo.dwRelativeCardAddr, NULL, 1) != SD_PASS)
		{
			MP_DEBUG("Select Card to Transfer State Wait Response fail");
			ret = RES_TIMEOUT;
		}
	}

	return ret;
}

static SWORD GoHiSpeedMode()
{
	MCARD *sMcard = (MCARD *)MCARD_BASE;
	SWORD ret = FAIL;
	DWORD sb[16] = {0x11223344, 0x55667788};
	BYTE *tmpBuf = (BYTE *)((DWORD)sb | 0xA0000000);
	DWORD s;

	// get SCR
	SdHost2_SetHostBlkLen(8);	// 64bit of SCR
	SdHost2_PhyTransfer((DWORD)tmpBuf, 8, MCARD_DMA_DIR_CM);
	if (SdHost_SendAppCmd(ACMD51, 0, NULL, 1) != PASS)
		mpDebugPrint("ACMD51 is not supported by this card!");
	SdHost2_WaitTransfer();
	//SdHost_SendStdCmd(CMD12, 0, NULL, 1);
	//MemDump(tmpBuf, 8);
	if ((tmpBuf[0] & 0x0f) >= 1)	// version 1.10 and higher
	{	// try switch function
		MP_DEBUG("Switch function...");
		SdHost2_SetHostBlkLen(64);	// 512bit of CMD6
		SdHost2_PhyTransfer((DWORD)tmpBuf, 64, MCARD_DMA_DIR_CM);
		if (SdHost_SendStdCmd(SD_CMD6, 0x00ffff01, NULL, 1) != SD_PASS)	// inquire high-speed function
			mpDebugPrint("-E- CMD6 is not supported by this card!");
		if (SdHost2_WaitTransfer() != PASS)
			mpDebugPrint("SD2 DMA for CMD6 read failed!");
		//SdHost_SendStdCmd(CMD12, 0, NULL, 1);
		//MemDump(tmpBuf, 64);
		if ((tmpBuf[12] == 0x80) && (tmpBuf[13] == 0x03) && ((tmpBuf[16] & 0xf) == 0x01))
		{
			// High speed mode is available.
			SdHost2_PhyTransfer((DWORD)tmpBuf, 64, MCARD_DMA_DIR_CM);
			if (SdHost_SendStdCmd(SD_CMD6, 0x80ffff01, NULL, 1) != SD_PASS)	// switch to high-speed function
				mpDebugPrint("CMD6 switch to high speed failed!");
			if (SdHost2_WaitTransfer() != PASS)
				mpDebugPrint("SD2 DMA for CMD6 read failed!");
			//SdHost_SendStdCmd(CMD12, 0, NULL, 1);
			if ((tmpBuf[12] == 0x80) && (tmpBuf[13] == 0x03) && ((tmpBuf[16] & 0xf) == 0x01))
			{
				MP_DEBUG("High Speed Mode!");
				ret = PASS;
			}
			//MemDump(tmpBuf, 64);
		}
	}

	return ret;
}


static SWORD LogicalRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	SWORD swStatus = PASS;
	
	if (!dwSectorCount)
	{
		return SD_PASS;
	}

	if (WaitTranState() != SD_PASS)
	{
		return GENERAL_FAIL;
	}
#if (CHIP_VER_MSB == CHIP_VER_615)
	SdHost_SendStdCmd(CMD16, SdInfo.wSectorSize, NULL, 1);	// set block size to card
#endif
	SdHost2_PhyTransfer(dwBufferAddress, (dwSectorCount << SdInfo.wSectorSizeExp),MCARD_DMA_DIR_CM);
	if(SdInfo.bType != SDHC_TYPE)
		dwLogAddr <<= SdInfo.wSectorSizeExp;
	if (dwSectorCount > 1)
		SdHost_SendStdCmd(CMD18, dwLogAddr, NULL, 1);
	else
		SdHost_SendStdCmd(CMD17, dwLogAddr, NULL, 1);
	swStatus = SdHost2_WaitTransfer();
	if (swStatus != SD_PASS)
	{
		if (dwSectorCount > 1)
		{
			SdHost_SendStdCmd(CMD12, 0, NULL, 1);
		}
		if (swStatus != GENERAL_FAIL)
		{
			SdHost_SendStdCmd(CMD7, 0, NULL, 1);
			SdHost_SendStdCmd(CMD7, SdInfo.dwRelativeCardAddr, NULL, 1);
		}
	}
	else
	{
		if (dwSectorCount > 1)
		{
			SdHost_SendStdCmd(CMD12, 0, NULL, 1);
		}
	}
	return swStatus;
}

static SWORD LogicalWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	SWORD swStatus = PASS;
	
	if (dwSectorCount == 0)
	{
		return SD_PASS;
	}
	if (WaitTranState() != SD_PASS)
	{
		return GENERAL_FAIL;
	}
#if (CHIP_VER_MSB == CHIP_VER_615)
	SdHost_SendStdCmd(CMD16, SdInfo.wSectorSize, NULL, 1);	// set block size to card
#endif
	SdHost2_PhyTransfer(dwBufferAddress, (dwSectorCount << SdInfo.wSectorSizeExp), MCARD_DMA_DIR_MC);
	if(SdInfo.bType != SDHC_TYPE)
		dwLogAddr <<= SdInfo.wSectorSizeExp;
	if (dwSectorCount > 1)
		SdHost2_SendCmd(CMD25, dwLogAddr );
	else
		SdHost2_SendCmd(CMD24, dwLogAddr);

	swStatus = SdHost2_WaitTransfer();
	if (swStatus != SD_PASS)
	{
		if (dwSectorCount > 1)
		{
			SdHost_SendStdCmd(CMD12, 0, NULL, 1);
		}
		if (swStatus == GENERAL_FAIL)
		{
			MP_DEBUG("-I- re init SdHost_WaitTransfer");
			GoTransferMode();
		}
		SdHost_SendStdCmd(CMD7, 0, NULL, 1);
		SdHost_SendStdCmd(CMD7, SdInfo.dwRelativeCardAddr, NULL, 1);
	}
	else
	{
		if (dwSectorCount > 1)
		{
			SdHost_SendStdCmd(CMD12, 0, NULL, 1);
		}
	}

	return swStatus;
}

static SWORD SdIdentify()
{
	DWORD dwResp;
	BYTE bHCS = 0, bCCS = 0;
	SWORD swRetValue = FAIL;
	
	// reset host
	SdInfo.bType = SD_TYPE;
	SdInfo.SpecVer = 0;
	SdInfo.wSectorSize = MCARD_SECTOR_SIZE;
	SdInfo.wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
	SdInfo.dwBusWidth = DBUS_1BIT;
	SdInfo.dwCardTag = MCARD_DMA_SD;
	SdInfo.dwClock = SD_INIT_CLK_KHZ;	
	SdInfo.dwClockAdj = NORMAL_SPEED;
	SdInfo.dwRelativeCardAddr = 0;
	SdInfo.dwCapacity = 0;
	SetSD2Clock(SdInfo.dwClock);
	McardSd2Active(SdInfo.dwBusWidth, NORMAL_SPEED);

	SdHost2_SendCmd(CMD0, 0);
	Mcard_TaskYield(100);
	SdHost2_SendCmd(CMD0, 0);
	Mcard_TaskYield(300);
	
#ifdef SDHC_FLAG // SDHC
	SdHost_SendStdCmd(CMD8, 0x000001aa, &dwResp, 1);
	if((dwResp & 0xff) == SDHC_CHECK_PATTERN)
		bHCS = 1;

	//DWORD ocr = (bHCS == 1) ? (HOST_OCR_VALUE|HCS) : HOST_OCR_VALUE;
	DWORD ocr = (bHCS == 1) ? (HOST_OCR_VALUE|HCS|S18R) : HOST_OCR_VALUE; // SDHC & SDXC
#else
	DWORD ocr = HOST_OCR_VALUE;
#endif

	/*
	   while card is still busy, keep sending OP_COND command
	   send OP_COND command for getting into READY state
	 */
	DWORD tmr = GetSysTime();
	do
	{			    
		dwResp = 0;
		if ((swRetValue = SdHost_SendAppCmd(ACMD41, ocr, &dwResp, 1)) == SD_PASS)
		{
			if (dwResp >> 31)	// ready
				break;
		}
		Mcard_TaskYield(90);
	} while (SystemGetElapsedTime(tmr) < 2000);	// wait 2 sec for ready
	MP_DEBUG1("SD2 ACMD41 dwResp = %x", dwResp);

	if ((dwResp >> 31) && (swRetValue == SD_PASS))
	{
		MP_DEBUG("Mcard acmd41 ok");
		#ifdef SDHC_FLAG
		if(dwResp & HCS)
		{
			bCCS = 1;
			if(dwResp & S18A)
				mpDebugPrint("SDXC card");
			else
				mpDebugPrint("SDHC card");
		}
		#endif
		if (bCCS == 0)
			MP_DEBUG("SD card");

		if ((swRetValue = SdHost_SendStdCmd(CMD2, 0, NULL, 1)) != SD_PASS)
		{
			mpDebugPrint("-E- CMD2 FAIL (swRetValue: %d)", swRetValue);
		}
		else if ((swRetValue = SdHost_SendStdCmd(CMD3, 0, &SdInfo.dwRelativeCardAddr, 1)) != SD_PASS)
		{	// send the host designating RCA to card and change to STANDBY state
			mpDebugPrint("-E- CMD3 FAIL (swRetValue: %d)", swRetValue);
		}
		else
		{
			SdInfo.dwRelativeCardAddr &= 0xffff0000;
			MP_DEBUG1("dwRelativeCardAddr = %x", SdInfo.dwRelativeCardAddr);
		}
	}
	else
	{
		MP_DEBUG("-E- ACMD41 FAIL, try to switch to MMC card");
		SdInfo.bType = MMC_TYPE;
		MP_DEBUG("mmc card");

		SdInfo.dwCardTag = MCARD_DMA_MMC;
		McardSd2Active(SdInfo.dwBusWidth, NORMAL_SPEED);
		DWORD tmr = GetSysTime();
		do
		{
			dwResp = 0;
			if ((swRetValue = SdHost_SendStdCmd(CMD1, HOST_OCR_VALUE, &dwResp, 1)) == SD_PASS)
			{
				if (dwResp >> 31)
					break;
			}
			Mcard_TaskYield(90);
		} while (SystemGetElapsedTime(tmr) < 1000);	// wait 1 sec for ready
		MP_DEBUG("mmc op condition = %x", dwResp);

		if (swRetValue != SD_PASS)
		{
			mpDebugPrint("-E- CMD1 FAIL");
		}
		else if ((swRetValue = SdHost_SendStdCmd(CMD2, 0, NULL, 1)) != SD_PASS)
		{
			mpDebugPrint("-E- CMD2 FAIL (swRetValue: %d)", swRetValue);
		}
		else if ((swRetValue = SdHost_SendStdCmd(CMD3, MMC_DEFAULT_ADDR, NULL, 1)) != SD_PASS)
		{	// send the host designating RCA to card and change to STANDBY state
			mpDebugPrint("-E- CMD3 FAIL (swRetValue: %d)", swRetValue);
		}
		else
		{
			SdInfo.dwRelativeCardAddr = MMC_DEFAULT_ADDR;
			MP_DEBUG1("dwRelativeCardAddr = %x", SdInfo.dwRelativeCardAddr);
		}
	}
	
	if (swRetValue == SD_PASS)
	{
		// change frequency to 24MHz after identified
		SetClockSpeed(SD_NORMAL_CLK_KHZ);
		McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change card type
		// get the Card Specific Register
		if ((swRetValue = SdHost_SendStdCmd(CMD9, SdInfo.dwRelativeCardAddr, NULL, 1)) != SD_PASS)
		{
			mpDebugPrint("-E- CMD9 FAIL (swRetValue: %d)", swRetValue);
		}
		else
		{
			DWORD Resp[4];

			SdHost2_GetResponse(Resp);
			if(bCCS == 0)  // SDSC
			{
				// calculate the card capacity
				DWORD dwReadBlLen = (Resp[1] & 0x000f0000) >> 16;
				DWORD dwCSize = (Resp[1] & 0x000003ff) << 2;
				DWORD dwCSizeMult = ((Resp[2] & 0x00038000) >> 15) + 2;
				dwCSize += ((Resp[2] >> 30) + 1);
				SdInfo.dwCapacity =
					((1 << dwReadBlLen) * ((1 << dwCSizeMult) * dwCSize)) / SdInfo.wSectorSize;
				MP_DEBUG1("Card Capacity = %d", SdInfo.dwCapacity);
			}
			else  // SDHC & SDXC
			{
				//DWORD bVersion = (Resp[0] &0xc0000000)>>30;
				DWORD dwCSize = 0;
				DWORD dwReadBlLen = (Resp[1] & 0x000f0000)>>16;

				// SD3.0 C_SIZE use total 22 bits
				dwCSize = ((Resp[1] & 0x0000003f) << 16) | ((Resp[2] & 0xffff0000) >> 16);
				MP_DEBUG1("C_SIZE: %x", dwCSize);
				dwCSize += 1;
				SdInfo.dwCapacity = (dwCSize) * (1<<(dwReadBlLen+1));
				SdInfo.bType = SDHC_TYPE;
				MP_DEBUG1("Card Capacity = %d",SdInfo.dwCapacity);
			}

			// if mmc card get spec version.
			if (SdInfo.dwCardTag == MCARD_DMA_MMC)
			{
				SdInfo.SpecVer = (Resp[0] & 0x3c000000) >> 26;
				MP_DEBUG1("MMC Version = %x", SdInfo.SpecVer);
			}
			else
			{
				SdInfo.SpecVer = 0;
			}
		}
	}

	return swRetValue;
}

static SWORD GoTransferMode(void)
{
	SWORD swRetValue;
	// change to TRANSFER state
	if ((swRetValue = SdHost_SendStdCmd(CMD7, SdInfo.dwRelativeCardAddr, NULL, 1)) != SD_PASS)
	{
		mpDebugPrint("-E- CMD7 FAIL (swRetValue: %d)", swRetValue);
	}
	else
	{
		if (SdInfo.dwCardTag == MCARD_DMA_SD)
		{
			if ((swRetValue = SdHost_SendAppCmd(ACMD6, 2, NULL, 1)) != SD_PASS)
			{
				MP_DEBUG1("-E- ACMD6 FAIL (swRetValue: %d)", swRetValue);
			}
			else
			{
				SdInfo.dwBusWidth = DBUS_4BIT;
				McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change bus width
				#if ((STD_BOARD_VER == MP650_FPGA) || (CHIP_VER_MSB == CHIP_VER_615))
				#else
				// after card selected, try to switch to high speed mode
				if (GoHiSpeedMode() == PASS)
				{
					SetClockSpeed(SD_HISPEED_CLK_KHZ);
					SdInfo.dwClockAdj = HIGH_SPEED;
					McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change high speed clock mode
				}
				else
				{
					MP_DEBUG("low speed card!");
					SdInfo.dwClockAdj = NORMAL_SPEED;
				}
				#endif
			}
		}
		else
		{
#ifdef HW_SUPPORT_MMC_4_0
			if (SdInfo.SpecVer == MMC_VER_4_0)
			{
				MP_DEBUG1("MMC_BUS_WIDTH = %d", MMC_BUS_WIDTH);
				//access | index | value | command set
				DWORD dwParam = (0x03 << 24) | (0xB7 << 16) | (MMC_BUS_WIDTH << 8) | 0x00;
				if ((swRetValue = SdHost_SendStdCmd(MMC_CMD6, dwParam, NULL, 1)) != SD_PASS)
				{
					MP_DEBUG1("-E- MMC_CMD6 FAIL (swRetValue: %d)", swRetValue);
				}
				else
				{
#if (MMC_BUS_WIDTH == 1)
					SdInfo.dwBusWidth = DBUS_4BIT;
					MP_DEBUG("-I- MMC transfer bus width to 4 bit");
#elif (MMC_BUS_WIDTH == 2)
					SdInfo.dwBusWidth = DBUS_8BIT;
					MP_DEBUG("-I- MMC transfer bus width to 8 bit");
#else
					SdInfo.dwBusWidth = DBUS_1BIT;
					MP_DEBUG("-I- MMC transfer bus width to 1 bit");
#endif
				}
			}
			else
			{
				MP_DEBUG("-I- Only Support MMC 1 bit");
			}
#else
			MP_DEBUG("-I- Only Support MMC 1 bit");
#endif
		}
	}

	if (swRetValue == SD_PASS)
	{		
		// set BLOCK length
		SdHost_SendStdCmd(CMD16, SdInfo.wSectorSize, NULL, 1);	// set block size to card
		SdHost_SetHostBlkLen(SdInfo.wSectorSize);	// set block size to host
	}
	else
	{
		mpDebugPrint("SD2/MMC: switch to transfer mode failed!");
	}
	
	return swRetValue;
}

static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwRetryTime = RETRY_TIME;
	SWORD swRetValue = PASS;

	MP_DEBUG2("-I- SD2 FlatRead dwLogAddr %d,dwSectorCount %d",dwLogAddr,dwSectorCount);
	while (dwRetryTime)
	{
		if ((swRetValue = LogicalRead(dwBufferAddress, dwSectorCount, dwLogAddr)) == PASS)
		{
			return SD_PASS;
		}
		dwRetryTime--;
		mpDebugPrint("-I- FlatRead failed, retry(%d)", dwRetryTime);
	}
	if (dwRetryTime == 0)
		mpDebugPrint("-E- FlatRead failed(%x), logAddr=%d sectors=%d", swRetValue, dwLogAddr, dwSectorCount);

	return swRetValue;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwRetryTime = RETRY_TIME;
	SWORD swRetValue = PASS;
	
	MP_DEBUG2("-I- SD2 FlatWrite dwLogAddr %d,dwSectorCount %d",dwLogAddr,dwSectorCount);
	while (dwRetryTime)
	{
		if ((swRetValue = LogicalWrite(dwBufferAddress, dwSectorCount, dwLogAddr)) == PASS)
		{
			break;
		}
		dwRetryTime--;
		mpDebugPrint("-I- FlatWrite failed, retry(%d)", dwRetryTime);
	}
	if (dwRetryTime == 0)
		mpDebugPrint("-E- FlatWrite failed(%x), logAddr=%d sectors=%d", swRetValue, dwLogAddr, dwSectorCount);
	
	return swRetValue;
}

static SWORD LowLevelFormat(void)
{
	return CARD_NOT_SUPPORT;
}

static SWORD AccessCmdProc(MCARD_DEVCE_CMD_SET DeviceCmd, DWORD Buffer, DWORD Lba, DWORD SectorNr)
{
	SWORD ret = SD_PASS;
	
	switch (DeviceCmd)
	{
		case SECTOR_READ_CMD:
			MP_DEBUG("SD2 Read cmd: %x,%x,%x", Lba, SectorNr, Buffer);
			SetSD2Clock(SdInfo.dwClock);
			McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);
			if (Lba >= SdInfo.dwCapacity)
				mpDebugPrint("SD2 : read() invalid blk %x, max=%x", Lba, SdInfo.dwCapacity);
			else
				ret = FlatRead(Buffer, SectorNr, Lba);
			McardSd2Inactive(SdInfo.dwCardTag);
			break;
		case SECTOR_WRITE_CMD:
			MP_DEBUG("SD2 Write cmd: %x,%x,%x", Lba, SectorNr, Buffer);
			SetSD2Clock(SdInfo.dwClock);
			McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);
			if (Lba >= SdInfo.dwCapacity)
				mpDebugPrint("SD2 : write() invalid blk %x, max=%x", Lba, SdInfo.dwCapacity);
			else if (SdHost2_GetCardWP() != TRUE)
				ret = FlatWrite(Buffer, SectorNr, Lba);
			McardSd2Inactive(SdInfo.dwCardTag);
			break;
		case RAW_FORMAT_CMD:
			MP_DEBUG("SD2 Format cmd!");
			SetSD2Clock(SdInfo.dwClock);
			McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);
			ret = LowLevelFormat();
			McardSd2Inactive(SdInfo.dwCardTag);
			break;
		default:
			break;
	}

	return ret;
}

static SWORD FsmProc(MCARD_DEVCE_CMD_SET event, ST_MCARD_DEV *pDev)
{
	SWORD ret = SD_PASS;

	switch (SdState)
	{
		case SD_STATE_UNKNOWN:
			MP_DEBUG("SD2 FSM: UNKNOWN, event is %x", event);
			if ((event == INIT_CARD_CMD) && pDev->Flag.Detected)
			{
				Power_On();
				Mcard_TaskYield(251);	// According to SD spec V2.00, max time to power up is 250+1ms.
				if ((ret = SdIdentify()))
				{
				    mpDebugPrint("-I- SD2 identification failed!");
					SdInfo.dwClock = SD_INIT_CLK_KHZ;	
				}
				else
				{
				    MP_DEBUG("-I- SD2 identification ok.");
					SdState = SD_STATE_STANDBY;
				}
				McardSd2Inactive(SdInfo.dwCardTag);
			}
			else{
				ret = GENERAL_FAIL;
			}
			break;

		case SD_STATE_STANDBY:
			MP_DEBUG("SD2 FSM: STANDBY, event is %x", event);
			if (event == REMOVE_CARD_CMD)
			{
				if (pDev->Flag.Detected)
				{
					SetSD2Clock(SdInfo.dwClock);
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
					SdHost_SendCmd(CMD0, 0);
					McardSd2Inactive(SdInfo.dwCardTag);
				}
				SdState = SD_STATE_UNKNOWN;
			}
			else
			{
				// switch to transfer mode
				SetSD2Clock(SdInfo.dwClock);
				McardSd2Active(SdInfo.dwBusWidth, SdInfo.dwClockAdj);
				if ((ret = GoTransferMode()) == SD_PASS)
					SdState = SD_STATE_TRANSFER;
				mpDebugPrint("%s card driver (%s speed, %d bit mode)"
								, (SdInfo.dwCardTag == MCARD_DMA_SD) ? "SD2" : "MMC2"
								, (SdInfo.dwClockAdj != NORMAL_SPEED) ? "High" : "Low"
								, (SdInfo.dwBusWidth == DBUS_8BIT) ? 8 : ((SdInfo.dwBusWidth == DBUS_4BIT) ? 4 : 1));
				McardSd2Inactive(SdInfo.dwCardTag);
			}
			break;
		case SD_STATE_TRANSFER:
			MP_DEBUG("SD2 FSM: TRANSFER, event is %x", event);
			if (event == REMOVE_CARD_CMD)
			{
				if (pDev->Flag.Detected)
				{
					SetSD2Clock(SdInfo.dwClock);
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
					SdHost_SendCmd(CMD0, 0);
					McardSd2Inactive(SdInfo.dwCardTag);
				}
				SdState = SD_STATE_UNKNOWN;
			}
			break;
		default:
			MP_DEBUG("SD2 FSM: ERROR state, event is %x", event);
			SdState = SD_STATE_UNKNOWN;
			break;
	}

	return ret;
}

static void CommandProcess(void *pMcardDev)
{
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	
	//mpDebugPrint("sd(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	if ((mail->swStatus = FsmProc(mail->wCmd, pDev)) == SD_PASS)
	{
		MP_DEBUG("Mcard2 fsm pass");
		if (mail->wCmd == INIT_CARD_CMD)
		{
			pDev->wRenewCounter++;
			pDev->Flag.ReadOnly = SdHost2_GetCardWP();
			pDev->Flag.Present = 1;
			pDev->dwCapacity = SdInfo.dwCapacity;
			pDev->wSectorSize = SdInfo.wSectorSize;
			pDev->wSectorSizeExp = SdInfo.wSectorSizeExp;
		}
		else if (mail->wCmd == REMOVE_CARD_CMD)
		{
			pDev->Flag.Present = 0;
			pDev->Flag.ReadOnly = 0;
			pDev->Flag.PipeEnable = 0;
			pDev->dwCapacity = 0;
			pDev->wSectorSize = 0;
			pDev->wSectorSizeExp = 0;
			mail->swStatus = PASS;
			Power_Off();
		}
		if (pDev->Flag.Detected && (SdState == SD_STATE_TRANSFER))
			mail->swStatus = AccessCmdProc(mail->wCmd
											, mail->dwBuffer
											, mail->dwBlockAddr
											, mail->dwBlockCount);
	}
	else
	{
		MP_DEBUG("Mcard2 fsm fail...");
		pDev->Flag.Present    = 0;
		pDev->Flag.ReadOnly   = 0;
		pDev->Flag.PipeEnable = 0;
		pDev->dwCapacity      = 0;
		pDev->wSectorSize     = 0;
		pDev->wSectorSizeExp  = 0;
		mail->swStatus = PASS;
	}
}

void Sd2Init(ST_MCARD_DEV *sDev)
{
	MP_DEBUG(__FUNCTION__);
	MP_ASSERT(sDev != NULL);
	
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_SD2;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
	SdInfo.dwClock = SD_INIT_CLK_KHZ;
}

#endif
