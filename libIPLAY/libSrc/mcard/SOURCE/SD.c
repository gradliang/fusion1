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
* Filename      : sd.c
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE	0
#define SD_DATA_DIV_ENABLE	1
#ifdef SD_DATA_DIV_ENABLE
#define SD_DATA_DIV_SIZE	0x800
#endif
/*
// Include section 
*/
#include "global612.h"
#if SD_MMC_ENABLE
#include "mpTrace.h"

#include "Mcard.h"
#include "uti.h"
#include "sd.h"

#if (DM9KS_ETHERNET_ENABLE == 1 )
#include "taskid.h"
#include "..\..\Peripheral\include\hal_gpio.h"
#endif

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
	BYTE cntErr;
	BYTE rev;
	WORD wSectorSize;
	WORD wSectorSizeExp;
	DWORD dwBusWidth;
	DWORD dwCardTag;
	DWORD dwClock;
	DWORD dwClockAdj;
	DWORD dwRelativeCardAddr;
	DWORD dwCapacity;
	BYTE bWriteProtect;	
} ST_SD_INFO;

/*
// Type declarations
*/

/*
// Variable declarations
*/
static ST_SD_INFO SdInfo = {SD_TYPE, 0, 0, 0, MCARD_SECTOR_SIZE, MCARD_SECTOR_SIZE_EXP, DBUS_1BIT, MCARD_DMA_SD, SD_INIT_CLK_KHZ, NORMAL_SPEED, 0, 0, 0};
static BYTE bDescriptor[] = "SD_MMC";
static BYTE SdState = SD_STATE_UNKNOWN;
static BYTE WriteMultiSector = FALSE;

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
	SetMcardClock(SdInfo.dwClock);
}

static void Power_On(void)
{
	McardPowerOn(DEV_SD_MMC);
}

static void Power_Off(void)
{
	McardPowerOff(DEV_SD_MMC);
}

static SWORD SdHost_SendAppCmd(DWORD cmd, DWORD StdArg, DWORD *resp, DWORD Wait)
{
	SWORD ret = SD_PASS;

	MP_DEBUG("~ACMD%d~", (cmd & 0x3f));

	SdHost_SendCmd(CMD55, SdInfo.dwRelativeCardAddr);
	if (SdHost_WaitCardResponse(NULL) != SD_PASS)
		MP_DEBUG("Card has no response to CMD_APP_CMD");
	SdHost_SendCmd(cmd, StdArg);
	if (Wait)
	{
		if ((ret = SdHost_WaitCardResponse(resp)) != SD_PASS)
			MP_DEBUG("Card has no response to ACMD%d", cmd&0x3f);
	}

	return ret;
}

static SWORD SdHost_SendStdCmd(DWORD cmd, DWORD arguement, DWORD *resp, DWORD Wait)
{
	SWORD ret = SD_PASS;

	MP_DEBUG("~CMD%d~", (cmd & 0x3f));	

	SdHost_SendCmd(cmd, arguement);
	if (Wait)
	{
		if ((ret = SdHost_WaitCardResponse(resp)) != SD_PASS)
			MP_DEBUG("Card has no response to CMD%d", cmd & 0x3f);
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
			MP_DEBUG("CMD13 Wait Response fail");
			ret = RES_TIMEOUT;
			break;
		}
		dwCurState = (dwCurState >> 9) & 0xf;
		if (dwCurState == TRANSFER_STATE)
		{
			//MP_DEBUG("Transfer state");
			ret = SD_PASS;
			WriteMultiSector = FALSE;
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
	DWORD scrV = 0;
	BYTE SD_SECURITY = 0;

	// get SCR
	SdHost_SetHostBlkLen(8);	// 64bit of SCR
	SdHost_PhyTransfer((DWORD)tmpBuf, 8, MCARD_DMA_DIR_CM);
	if (SdHost_SendAppCmd(ACMD51, 0, NULL, 1) != PASS)
		mpDebugPrint("ACMD51 is not supported by this card!");
	if (SdHost_WaitTransfer() != SD_PASS)
	{
		mpDebugPrint("Wait acmd51 fail...");
	}
	else
	{
		MP_DEBUG("SCR information");
		scrV = *(DWORD *)tmpBuf;
		SD_SECURITY = (scrV >> 20) & 0X07;
		if(SD_SECURITY == 0)
			MP_DEBUG("No security");
		else if(SD_SECURITY == 2)
			MP_DEBUG("SDSC card");
		else if(SD_SECURITY == 3)
		{
			mpDebugPrint("SDHC card");
			SdInfo.bType = SDHC_TYPE;
		}
		else if(SD_SECURITY == 4)
		{
			mpDebugPrint("SDXC card");
			SdInfo.bType = SDXC_TYPE;
		}
	}
	
	//SdHost_SendStdCmd(CMD12, 0, NULL, 1);
	//MemDump(tmpBuf, 8);
	if ((tmpBuf[0] & 0x0f) >= 1)	// version 1.10 and higher
	{	// try switch function
		MP_DEBUG("Switch function...");
		SdHost_SetHostBlkLen(64);	// 512bit of CMD6
		SdHost_PhyTransfer((DWORD)tmpBuf, 64, MCARD_DMA_DIR_CM);
		// SD2.0 LOG for 1.4-3
		if (SdHost_SendStdCmd(SD_CMD6, 0x00fffff1, NULL, 1) != SD_PASS)	// inquire high-speed function
			mpDebugPrint("-E- CMD6 is not supported by this card!");
		if (SdHost_WaitTransfer() != PASS)
			mpDebugPrint("SD DMA for CMD6 read failed!");
		//SdHost_SendStdCmd(CMD12, 0, NULL, 1);
		//MemDump(tmpBuf, 64);
		if ((tmpBuf[12] == 0x80) && (tmpBuf[13] == 0x03) && ((tmpBuf[16] & 0xf) == 0x01))
		{
			// High speed mode is available.
			SdHost_PhyTransfer((DWORD)tmpBuf, 64, MCARD_DMA_DIR_CM);
			// SD2.0 LOG for 1.4-3
			if (SdHost_SendStdCmd(SD_CMD6, 0x80fffff1, NULL, 1) != SD_PASS)	// switch to high-speed function
				mpDebugPrint("CMD6 switch to high speed failed!");
			if (SdHost_WaitTransfer() != PASS)
				mpDebugPrint("SD DMA for CMD6 read failed!");
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


void ParsingSCR()
{
	SWORD swStatus = PASS;
	BYTE buf[8];
	BYTE *SCR = (BYTE *)((DWORD)buf | 0xa0000000);
	DWORD scrV = 0;

    //|   Field                               | Width | SCR Slice |
	BYTE SCR_STRUCTURE         = 0; //|  4    | [63 : 60] |
	BYTE SD_SPEC               = 0; //|  4    | [59 : 56] |
	BYTE DATA_STAT_AFTER_ERASE = 0; //|  1    | [55 : 55] |
	BYTE SD_SECURITY           = 0; //|  3    | [54 : 52] |
	BYTE SD_BUS_WIDTHS         = 0; //|  4    | [51 : 48] |
	BYTE SD_SPEC3              = 0; //|  1    | [47]      |
	BYTE EX_SECURITY           = 0; //|  4    | [46 : 43] |
	BYTE CMD_SUPPORT           = 0; //|  2    | [33 : 32] |
	
	MP_DEBUG("%s", __FUNCTION__);

	if (WaitTranState() != SD_PASS)
	{
		return GENERAL_FAIL;
	}

	SdHost_SetHostBlkLen(8);
	SdHost_PhyTransfer(SCR, 8, MCARD_DMA_DIR_CM);
	SdHost_SendAppCmd(ACMD51, NULL, NULL, 1);
	swStatus = SdHost_WaitTransfer();
	if (swStatus != SD_PASS)
	{
		mpDebugPrint("Wait acmd51 fail...");
	}
	else
	{
		MP_DEBUG("SCR information");
		//MemDump(SCR, 8);
	}

	scrV = *(DWORD *)SCR;

	MP_DEBUG("SCR: %x", scrV);

	SD_SPEC = (scrV >> 24) & 0xff;
	if(SD_SPEC == 0)
		MP_DEBUG("SD ver1.0");	
	else if(SD_SPEC == 1)
		MP_DEBUG("SD ver1.10");	
	else if(SD_SPEC == 2){
		SD_SPEC3 = (scrV >> 15) & 0X01;
		if(SD_SPEC3)
			MP_DEBUG("SD ver 3.0");
		else
			MP_DEBUG("SD ver 2.0");
	}	
	else
		MP_DEBUG("SD ver? !!!");

	SD_SECURITY = (scrV >> 20) & 0X07;
	if(SD_SECURITY == 0)
		MP_DEBUG("No security");
	else if(SD_SECURITY == 2)
		MP_DEBUG("SDSC card");
	else if(SD_SECURITY == 3)
	{
		mpDebugPrint("SDHC card");
		SdInfo.bType = SDHC_TYPE;
	}
	else if(SD_SECURITY == 4)
	{
		mpDebugPrint("SDXC card");
		SdInfo.bType = SDXC_TYPE;
	}

	SD_BUS_WIDTHS = (scrV >> 16) & 0X0f;
	if(SD_BUS_WIDTHS & 0x1)
		MP_DEBUG("Bus 1 bit support");	
	if(SD_BUS_WIDTHS & 0x4)
		MP_DEBUG("Bus 4 bits support");

	if(scrV & BIT0)
		MP_DEBUG("support CMD20");	
	
	if(scrV & BIT1)
		MP_DEBUG("support CMD23");	


//	mpDebugPrint("End of SCR testing...hanged");
//	while(1);
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
	SdHost_PhyTransfer(dwBufferAddress, (dwSectorCount << SdInfo.wSectorSizeExp),MCARD_DMA_DIR_CM);
	if(SdInfo.bType != SDHC_TYPE)
		dwLogAddr <<= SdInfo.wSectorSizeExp;
	if (dwSectorCount > 1)
		SdHost_SendStdCmd(CMD18, dwLogAddr, NULL, 1);
	else
		SdHost_SendStdCmd(CMD17, dwLogAddr, NULL, 1);
	swStatus = SdHost_WaitTransfer();
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
#ifdef SD_DATA_DIV_ENABLE
	DWORD i = 0;
	DWORD divisor, remainder;
#endif
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
#ifdef SD_DATA_DIV_ENABLE
	if(dwSectorCount<SD_DATA_DIV_SIZE) // <1M bytes
#endif
	{
		SdHost_PhyTransfer(dwBufferAddress, (dwSectorCount << SdInfo.wSectorSizeExp), MCARD_DMA_DIR_MC);
		if(SdInfo.bType != SDHC_TYPE)
			dwLogAddr <<= SdInfo.wSectorSizeExp;
		if (dwSectorCount > 1)
			SdHost_SendCmd(CMD25, dwLogAddr );
		else
			SdHost_SendCmd(CMD24, dwLogAddr);
		
		swStatus = SdHost_WaitTransfer();
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
				WriteMultiSector = TRUE;
			}
		}
		
		return swStatus;
	}
#ifdef SD_DATA_DIV_ENABLE
	else
	{
		divisor = dwSectorCount/SD_DATA_DIV_SIZE;
		remainder = dwSectorCount%SD_DATA_DIV_SIZE;

		for(i=0; i<divisor; i++)
		{
			SdHost_PhyTransfer(dwBufferAddress, (SD_DATA_DIV_SIZE << SdInfo.wSectorSizeExp), MCARD_DMA_DIR_MC);
			if(SdInfo.bType != SDHC_TYPE)
				dwLogAddr <<= SdInfo.wSectorSizeExp;
			
			SdHost_SendCmd(CMD25, (dwLogAddr+(i<<11)));
			
			swStatus = SdHost_WaitTransfer();
			if (swStatus != SD_PASS)
			{
				SdHost_SendStdCmd(CMD12, 0, NULL, 1);
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
				SdHost_SendStdCmd(CMD12, 0, NULL, 1);
				WriteMultiSector = TRUE;
			}
			WaitTranState();
		}
		if(remainder)
		{
			SdHost_PhyTransfer(dwBufferAddress, (remainder << SdInfo.wSectorSizeExp), MCARD_DMA_DIR_MC);
			if(SdInfo.bType != SDHC_TYPE)
				dwLogAddr <<= SdInfo.wSectorSizeExp;
			
			SdHost_SendCmd(CMD25, (dwLogAddr+(i<<11)) );
			
			swStatus = SdHost_WaitTransfer();
			if (swStatus != SD_PASS)
			{
				SdHost_SendStdCmd(CMD12, 0, NULL, 1);
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
				SdHost_SendStdCmd(CMD12, 0, NULL, 1);
				WriteMultiSector = TRUE;
			}
		}
		return swStatus;
	}
#endif
}

static SWORD SdIdentify()
{
	DWORD dwResp;
	BYTE bHCS = 0, bCCS = 0;
	SWORD swRetValue = FAIL;
	
	// reset host
	SdInfo.bType = SD_TYPE;
	SdInfo.SpecVer = 0;
	SdInfo.cntErr  = 0;
	SdInfo.wSectorSize = MCARD_SECTOR_SIZE;
	SdInfo.wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
	SdInfo.dwBusWidth = DBUS_1BIT;
	SdInfo.dwCardTag = MCARD_DMA_SD;
	SdInfo.dwClock = SD_INIT_CLK_KHZ;	
	SdInfo.dwClockAdj = NORMAL_SPEED;
	SdInfo.dwRelativeCardAddr = 0;
	SdInfo.dwCapacity = 0;
	SdInfo.bWriteProtect = 0;
	SetMcardClock(SdInfo.dwClock);
	McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, NORMAL_SPEED);

	SdHost_SendCmd(CMD0, 0);
	Mcard_TaskYield(100);
	SdHost_SendCmd(CMD0, 0);
	Mcard_TaskYield(300);
	
#ifdef SDHC_FLAG // SDHC

	swRetValue = SdHost_SendStdCmd(CMD8, 0x000001aa, &dwResp, 1);
	// SD2.0 LOG for 1.2-4
	if (swRetValue == BAD_CRC7)
	{
		MP_ALERT("-E- CMD8 CRC FAIL");
		return FAIL;
	}

	if (swRetValue == RES_TIMEOUT) // Ver1.x or Not SD
	{
		MP_DEBUG("-I- SD Ver1.x or Not SD");
		swRetValue = SD_PASS;
	}
	else // Ver2.0 or later
	{
		if((dwResp & 0xff) == SDHC_CHECK_PATTERN)
		{
			bHCS = 1;
		}
		else // SD2.0 LOG for 1.2-3
		{
			MP_ALERT("-E- CMD8 FAIL (Err-Pattern: 0x%x)", (dwResp & 0xff));	
			return FAIL;
		}
	}

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
	MP_DEBUG1("SD ACMD41 dwResp = %x", dwResp);

	if ((dwResp >> 31) && (swRetValue == SD_PASS))
	{
		MP_DEBUG("Mcard acmd41 ok");
#ifdef SDHC_FLAG
		if(dwResp & HCS) // HCS : mean SDHC/SDXC
		{
			bCCS = 1;
			#if 0  // Not use to check SDXC // S18A : not all of SDXC supported and some SDHCs supported 
			if(dwResp & S18A)  // First general check 
			{
				//MP_DEBUG("SDXC card");
				SdInfo.bType = SDXC_TYPE;
				//mpDebugPrint("SdInfo.bType = %x", SdInfo.bType);
			}
			else 
			{
				//MP_DEBUG("SDHC card");
				SdInfo.bType = SDHC_TYPE;
			}
			#else 
			//MP_DEBUG("SDHC card");
			SdInfo.bType = SDHC_TYPE;  // SDHC or SDXC from spec. Please check SDXC in SCR
			#endif
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
		McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, NORMAL_SPEED);
		DWORD tmr = GetSysTime();
		do
		{
			dwResp = 0;
#ifdef SDHC_FLAG            
			if ((swRetValue = SdHost_SendStdCmd(CMD1, HOST_OCR_VALUE | HCS, &dwResp, 1)) == SD_PASS)  // Host support HC
#else
			if ((swRetValue = SdHost_SendStdCmd(CMD1, HOST_OCR_VALUE, &dwResp, 1)) == SD_PASS)
#endif
			{
				if (dwResp >> 31)
					break;
			}
			Mcard_TaskYield(90);
		} while (SystemGetElapsedTime(tmr) < 1000);	// wait 1 sec for ready
		MP_DEBUG("mmc op condition = %x", dwResp);

		if ((dwResp >> 31) && (swRetValue == SD_PASS))
		{
#ifdef SDHC_FLAG
        		if(dwResp & HCS) // HCS : mean SDHC/SDXC
        		{
        			bCCS = 1;
        			mpDebugPrint("MMC for HC/XC card\r\n");
        			SdInfo.bType = SDHC_TYPE;  // SDHC or SDXC from spec. Please check SDXC in SCR
        		}
#endif
		}
        

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
		MP_DEBUG("Start cmd9");
		// change frequency to 24MHz after identified
		SetClockSpeed(SD_NORMAL_CLK_KHZ);
		McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change card type
		// get the Card Specific Register
		if ((swRetValue = SdHost_SendStdCmd(CMD9, SdInfo.dwRelativeCardAddr, NULL, 1)) != SD_PASS)
		{
			mpDebugPrint("-E- CMD9 FAIL (swRetValue: %d)", swRetValue);
		}
		else
		{
			DWORD Resp[4];

			SdHost_GetResponse(Resp);
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
				//SdInfo.bType = SDHC_TYPE;
				MP_DEBUG1("Card Capacity = %d",SdInfo.dwCapacity);
			}

			// SD2.0 LOG for 1.1-31 
			// PERM_WRITE_PROTECT : BIT13
			// TMP_WRITE_PROTECT   : BIT12
			if(Resp[3] & (BIT12|BIT13)) // WRITE_PROTECT
			{
				SdInfo.bWriteProtect = 1;
				MP_DEBUG("Card Write Protect");
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
				McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change bus width
				#if ((STD_BOARD_VER == MP650_FPGA) || (CHIP_VER_MSB == CHIP_VER_615))
				#else
				// after card selected, try to switch to high speed mode
				//--SdInfo.dwCapacity unit byte,DIV2  ; 256MB is 475136
				 // 解决小容量卡被识别为高速卡造成读卡失败
				if (SdInfo.dwCapacity>1000000 &&  GoHiSpeedMode() == PASS)
				{
					SetClockSpeed(SD_HISPEED_CLK_KHZ);
					SdInfo.dwClockAdj = HIGH_SPEED;
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change high speed clock mode
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
			if (SdInfo.SpecVer >= MMC_VER_4_0)
			{
				MP_DEBUG1("MMC_BUS_WIDTH = %d", MMC_BUS_WIDTH);
                            DWORD dwParam = (0x03 << 24) | (0xB7 << 16) | (MMC_BUS_WIDTH << 8) | 0x00;
				if ((swRetValue = SdHost_SendStdCmd(MMC_CMD6, dwParam, NULL, 1)) != SD_PASS)
				{
					MP_DEBUG1("-E- MMC_CMD6 BW FAIL (swRetValue: %d)", swRetValue);
				}
				else
				{
					WaitTranState();				
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
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change high speed clock mode                    
				}
                            

				//access | index | value | command set
				dwParam = (0x03 << 24) | (0xB9 << 16) | (HIGH_SPEED << 8) | 0x00;
				if ((swRetValue = SdHost_SendStdCmd(MMC_CMD6, dwParam, NULL, 1)) != SD_PASS)
				{
					MP_DEBUG1("-E- MMC_CMD6 HS FAIL (swRetValue: %d)", swRetValue);
					SdInfo.dwClockAdj = NORMAL_SPEED;
				}
				else
				{
					WaitTranState();
					SetClockSpeed(SD_HISPEED_CLK_KHZ);
					SdInfo.dwClockAdj = HIGH_SPEED;
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);	// change high speed clock mode                    
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
		mpDebugPrint("SD/MMC: switch to transfer mode failed!");
	}

	return swRetValue;
}

static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwRetryTime = RETRY_TIME;
	SWORD swRetValue = PASS;

	MP_DEBUG2("-I- SD FlatRead dwLogAddr %d,dwSectorCount %d",dwLogAddr,dwSectorCount);
	while (dwRetryTime)
	{
		if ((swRetValue = LogicalRead(dwBufferAddress, dwSectorCount, dwLogAddr)) == PASS)
		{
			return SD_PASS;
		}
		dwRetryTime--;
		//mpDebugPrint("-I- FlatRead failed, retry(%d)", dwRetryTime);
	}
	//if (dwRetryTime == 0)
	//	mpDebugPrint("-E- FlatRead failed(%x), logAddr=%d sectors=%d", swRetValue, dwLogAddr, dwSectorCount);

	return swRetValue;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	DWORD dwRetryTime = RETRY_TIME;
	SWORD swRetValue = PASS;

	MP_DEBUG2("-I- SD FlatWrite dwLogAddr %d,dwSectorCount %d",dwLogAddr,dwSectorCount);
	while (dwRetryTime)
	{
		if ((swRetValue = LogicalWrite(dwBufferAddress, dwSectorCount, dwLogAddr)) == PASS)
		{
			break;
		}
		dwRetryTime--;
		//mpDebugPrint("-I- FlatWrite failed, retry(%d)", dwRetryTime);
	}
	//if (dwRetryTime == 0)
	//	mpDebugPrint("-E- FlatWrite failed(%x), logAddr=%d sectors=%d", swRetValue, dwLogAddr, dwSectorCount);

	// [Bug Fixed]MP660-32M-SDRAM Version
	// Transcend 64MB SD will pull some pins after doing writing command.
	// So send a CMD13 to make it relase those pins.
	DWORD dwCurState;
	if (SdHost_SendStdCmd(CMD13, SdInfo.dwRelativeCardAddr, &dwCurState, 1) != SD_PASS)
	{
		MP_DEBUG("CMD13 Wait Response fail");
	}

	return swRetValue;
}

static SWORD LowLevelFormat(void)
{
	return CARD_NOT_SUPPORT;
}

// If there are successive errors over error upper bound, send exception to system layer
static SWORD AccessErrorHandler(SWORD ret)
{
	#define SD_ERROR_BOUNDARY  10
	if(ret != SD_PASS)
		SdInfo.cntErr++;
	else
		SdInfo.cntErr = 0;

	if(SdInfo.cntErr > SD_ERROR_BOUNDARY){
		SdInfo.cntErr = 0;
		CardFatalErrorNotify(DEV_SD_MMC);
	}

	return PASS;
}
static SWORD AccessCmdProc(MCARD_DEVCE_CMD_SET DeviceCmd, DWORD Buffer, DWORD Lba, DWORD SectorNr)
{
	SWORD ret = SD_PASS;
	
	switch (DeviceCmd)
	{
		case SECTOR_READ_CMD:
			MP_DEBUG("SD Read cmd: %x,%x,%x", Lba, SectorNr, Buffer);
			SetMcardClock(SdInfo.dwClock);
			McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
			if (Lba >= SdInfo.dwCapacity)
				mpDebugPrint("SD : read() invalid blk %x, max=%x", Lba, SdInfo.dwCapacity);
			else
				ret = FlatRead(Buffer, SectorNr, Lba);
			McardSdInactive(SdInfo.dwCardTag);
			break;
		case SECTOR_WRITE_CMD:
			MP_DEBUG("SD Write cmd: %x,%x,%x", Lba, SectorNr, Buffer);
			SetMcardClock(SdInfo.dwClock);
			McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
			if (Lba >= SdInfo.dwCapacity)
				mpDebugPrint("SD : write() invalid blk %x, max=%x", Lba, SdInfo.dwCapacity);
			else if (SdHost_GetCardWP() != TRUE)
				ret = FlatWrite(Buffer, SectorNr, Lba);
			McardSdInactive(SdInfo.dwCardTag);
			break;
		case RAW_FORMAT_CMD:
			MP_DEBUG("SD Format cmd!");
			SetMcardClock(SdInfo.dwClock);
			McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
			ret = LowLevelFormat();
			McardSdInactive(SdInfo.dwCardTag);
			break;
		default:
			break;
	}

	if(DeviceCmd == SECTOR_READ_CMD || DeviceCmd == SECTOR_WRITE_CMD)
		AccessErrorHandler(ret);

	return ret;
}

static SWORD FsmProc(MCARD_DEVCE_CMD_SET event, ST_MCARD_DEV *pDev)
{
	SWORD ret = SD_PASS;

	MP_DEBUG("SD card state: %x", SdState);
	switch (SdState)
	{
		case SD_STATE_UNKNOWN:
			MP_DEBUG("SD FSM: UNKNOWN, event is %x", event);
			if ((event == INIT_CARD_CMD) && pDev->Flag.Detected)
			{
				Power_On();
				Mcard_TaskYield(251);	// According to SD spec V2.00, max time to power up is 250+1ms.
				if ((ret = SdIdentify()))
				{
				    mpDebugPrint("-I- SD identification failed!");
					SdInfo.dwClock = SD_INIT_CLK_KHZ;	
				}
				else
				{
				    MP_DEBUG("-I- SD identification ok.");
					SdState = SD_STATE_STANDBY;
				}
				McardSdInactive(SdInfo.dwCardTag);
			}
			else{
				MP_DEBUG("[SD UNKNOW STATE fail]event: %x, sd detect: %x", event, pDev->Flag.Detected);
				ret = GENERAL_FAIL;
			}
			break;

		case SD_STATE_STANDBY:
			MP_DEBUG("SD FSM: STANDBY, event is %x", event);
			if (event == REMOVE_CARD_CMD)
			{
				if (pDev->Flag.Detected)
				{
					SetMcardClock(SdInfo.dwClock);
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
					SdHost_SendCmd(CMD0, 0);
					McardSdInactive(SdInfo.dwCardTag);
				}
				SdState = SD_STATE_UNKNOWN;
			}
			else
			{
				// switch to transfer mode
				SetMcardClock(SdInfo.dwClock);
				McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
				if ((ret = GoTransferMode()) == SD_PASS)
					SdState = SD_STATE_TRANSFER;
				mpDebugPrint("%s card driver (%s speed %dKHz, %d bit mode)"
								, (SdInfo.dwCardTag == MCARD_DMA_SD) ? "SD" : "MMC"
								, (SdInfo.dwClockAdj != NORMAL_SPEED) ? "High" : "Low"
								, GetMcardClock()
								, (SdInfo.dwBusWidth == DBUS_8BIT) ? 8 : ((SdInfo.dwBusWidth == DBUS_4BIT) ? 4 : 1));

				// Read SCR register here if it's necessary
				// ParsingSCR();
				McardSdInactive(SdInfo.dwCardTag);
			}
			break;
		case SD_STATE_TRANSFER:
			MP_DEBUG("SD FSM: TRANSFER, event is %x", event);
			if (event == REMOVE_CARD_CMD)
			{
				if (pDev->Flag.Detected)
				{
					SetMcardClock(SdInfo.dwClock);
					McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
					SdHost_SendCmd(CMD0, 0);
					McardSdInactive(SdInfo.dwCardTag);
				}
				SdState = SD_STATE_UNKNOWN;
			}
			break;
		default:
			MP_DEBUG("SD FSM: ERROR state, event is %x", event);
			SdState = SD_STATE_UNKNOWN;
			break;
	}

	return ret;
}

static void CommandProcess(void *pMcardDev)
{
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	
#if (DM9KS_ETHERNET_ENABLE == 1 )
	//IntDisable();
	SemaphoreWait(CFETHERNET_MCARD_SEMA);
	Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
#endif
	//mpDebugPrint("sd(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	if ((mail->swStatus = FsmProc(mail->wCmd, pDev)) == SD_PASS)
	{
		//mpDebugPrint("Mcard fsm pass");
		if (mail->wCmd == INIT_CARD_CMD)
		{
			MP_DEBUG("INIT card cmd");
			pDev->wRenewCounter++;
			pDev->Flag.ReadOnly  = SdHost_GetCardWP() | SdInfo.bWriteProtect ;
			pDev->Flag.Present   = 1;
			pDev->dwCapacity     = SdInfo.dwCapacity;
			pDev->wSectorSize    = SdInfo.wSectorSize;
			pDev->wSectorSizeExp = SdInfo.wSectorSizeExp;
			pDev->wProperty1     = (WORD)SdInfo.bType;
		}
		else if (mail->wCmd == REMOVE_CARD_CMD)
		{
			pDev->Flag.Present    = 0;
			pDev->Flag.ReadOnly   = 0;
			pDev->Flag.PipeEnable = 0;
			pDev->dwCapacity      = 0;
			pDev->wSectorSize     = 0;
			pDev->wSectorSizeExp  = 0;
			pDev->wProperty1      = 0;
			mail->swStatus        = PASS;
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
		MP_DEBUG("Mcard fsm fail...");
		pDev->Flag.Present    = 0;
		pDev->Flag.ReadOnly   = 0;
		pDev->Flag.PipeEnable = 0;
		pDev->dwCapacity      = 0;
		pDev->wSectorSize     = 0;
		pDev->wSectorSizeExp  = 0;
		pDev->wProperty1      = 0;
		mail->swStatus = PASS;
	}
#if (DM9KS_ETHERNET_ENABLE == 1 )
	//IntEnable();	
	Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
	SemaphoreRelease(CFETHERNET_MCARD_SEMA);
#endif

}

BYTE DetermineSDSlotType(void)
{
	//mpDebugPrint("SdInfo.bType at DeterMineSDSlotType = %x", SdInfo.bType);
	
	return (SdInfo.bType);
}

// SD card programming is handling in background by sd controller.
// At that time, the DATA0 will be low with force.
// Since the xD/Nand/CF/SD/MMC are shared data pins, please call
// SdProgChk() once before R/W to them.
void SdProgChk()
{
	if (WriteMultiSector == TRUE)
	{
		SetMcardClock(SdInfo.dwClock);
		McardSdActive(SdInfo.dwCardTag, SdInfo.dwBusWidth, SdInfo.dwClockAdj);
		WaitTranState();
		McardSdInactive(SdInfo.dwCardTag);
	}
}

void SdInit(ST_MCARD_DEV *sDev)
{
	MP_DEBUG(__FUNCTION__);
	MP_ASSERT(sDev != NULL);
	
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_SD_MMC;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
	SdInfo.dwClock = SD_INIT_CLK_KHZ;
}

#endif
