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
* Filename      : cf.c
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
#include "mpTrace.h"
#if CF_ENABLE

#include "devio.h"
#include "Mcard.h"
#include "uti.h"
#include "cf.h"

#if (DM9KS_ETHERNET_ENABLE == 1 )
#include "taskid.h"
#include "..\..\Peripheral\include\hal_gpio.h"
#endif
/*
// Constant declarations
*/

#define COMMAND_IDENTIFY_DRIVE  0xec
#define COMMAND_READ_SECTOR     0x20
#define COMMAND_WRITE_SECTOR    0x30
#define COMMAND_TOTAL           0x03

#define IDENTIFY_ID     0x848a


#ifdef DEMO_PID
#define CF_CLOCK_KHZ    153000	// Never bigger than CPU frequency
                                // Try to keep PLL frequnecy as the multiple of this value
#else
#define CF_CLOCK_KHZ    48000	// Never bigger than CPU frequency
                                // Try to keep PLL frequnecy as the multiple of this value
#endif


/*
// Variable declarations
*/
static BYTE bDescriptor[] = "CF";
static volatile BOOL blTimeOutFlag;

/*
// Macro declarations
*/
//#define CF_TIMEOUT_HANDLE

//#define CF_TASK_YIELD
#ifdef CF_TASK_YIELD
#define TASK_YIELD() TaskYield()
#else
#define TASK_YIELD()
#endif

/*
// Static function prototype
*/
static void CommandProcess(void *McardDev);
static SWORD LogicalRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress);
static SWORD LogicalWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress);
static SWORD SelfAdjustingIdent(DWORD *pdwTotalSector);
static SWORD Identify(DWORD * pdwTotalSector);
static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress);
static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress);
static SWORD LowLevelFormat();

/*
// Definition of internal functions
*/
void CfInit(ST_MCARD_DEV * sDev)
{
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_CF;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

static void Power_On()
{
	McardPowerOn(DEV_CF);
}

static void Power_Off()
{
	McardPowerOff(DEV_CF);
}

void debugCFtiming()
{
	register MCARD *sMcard;
	sMcard = (MCARD *) (MCARD_BASE);

	mpDebugPrint("--- debug CF timing!! ---");
	mpDebugPrint("sMcard->McRtm: %x", sMcard->McRtm);
	mpDebugPrint("sMcard->McWtm: %x", sMcard->McWtm);
}

/*
// Definition of local functions 
*/
static void CommandProcess(void *pMcardDev)
{
	static BYTE inited = FALSE;
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;

#if SD_MMC_ENABLE
    SdProgChk();
#endif

#if (DM9KS_ETHERNET_ENABLE == 1 )
	//IntDisable();
	SemaphoreWait(CFETHERNET_MCARD_SEMA);
	Gpio_IntDisable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
#endif
	SetMcardClock(CF_CLOCK_KHZ);
	//mpDebugPrint("CF(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	switch (mail->wCmd)
	{
	case INIT_CARD_CMD:
		if (inited == FALSE)
		{
			Power_On();
			if (pDev->Flag.Detected)
			{
				CFCardReset();
#if 0
				CalculatePulseTiming(TIMING_250ns);
				McardCFActive(INITIAL_MEMORY);
				if ((pDev->swStatus = Identify(&pDev->dwCapacity)))
				{
					MP_ALERT("CF initial fail, try again with wide pulse timg!!");
					CalculatePulseTiming(TIMING_SLOWEST);
					McardCFActive(INITIAL_MEMORY);
					if ((pDev->swStatus = Identify(&pDev->dwCapacity)))
					{
						pDev->Flag.Present = 0;
						pDev->wSectorSize = 0;
						pDev->wSectorSizeExp = 0;
						pDev->dwCapacity = 0;
					}
					else
					{
						pDev->wRenewCounter++;
						pDev->Flag.ReadOnly = 0;
						pDev->Flag.Present = 1;
						pDev->wSectorSize = MCARD_SECTOR_SIZE;
						pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
						inited = TRUE;
					}
				}
#else			// [Experiment] using SelfAdjustingIdent() to do Identify action
				if ((pDev->swStatus = SelfAdjustingIdent(&pDev->dwCapacity)))
				{
					pDev->Flag.Present = 0;
					pDev->wSectorSize = 0;
					pDev->wSectorSizeExp = 0;
					pDev->dwCapacity = 0;
				}
#endif
				else
				{
					pDev->wRenewCounter++;
					pDev->Flag.ReadOnly = 0;
					pDev->Flag.Present = 1;
					pDev->wSectorSize = MCARD_SECTOR_SIZE;
					pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
					//EventSet(UI_EVENT, EVENT_CARD_INIT);
					//mpDebugPrint("CF card size: %dMB", (MCARD_SECTOR_SIZE * pDev->dwCapacity) >> 20);
					inited = TRUE;
				}
				McardCFInactive();
			}
			else
			{
				//card out
				pDev->Flag.Present = 0;
				pDev->Flag.ReadOnly = 0;
				pDev->Flag.PipeEnable = 0;
				pDev->swStatus = 0;
				pDev->dwCapacity = 0;
				pDev->wSectorSize = 0;
				pDev->wSectorSizeExp = 0;
				//EventSet(UI_EVENT, EVENT_CARD_INIT);  
				Power_Off();
			}
		}
		break;
	case REMOVE_CARD_CMD:		//Athena 03.11.2006 seperate card in & out
		//card out
		pDev->Flag.Present = 0;
		pDev->Flag.ReadOnly = 0;
		pDev->Flag.PipeEnable = 0;
		pDev->swStatus = 0;
		pDev->dwCapacity = 0;
		pDev->wSectorSize = 0;
		pDev->wSectorSizeExp = 0;
		inited = FALSE;
		Power_Off();
		//EventSet(UI_EVENT, EVENT_CARD_INIT);
		break;
	case SECTOR_READ_CMD:
		if (mail->dwBlockAddr >= pDev->dwCapacity)
		{
			mpDebugPrint("CF: read addr exceed capacity(%d/%d)", mail->dwBlockAddr, pDev->dwCapacity);
			pDev->swStatus=FAIL;
		}
		else
		{
			McardCFActive(INITIAL_MEMORY);
			pDev->swStatus = FlatRead(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr, &mail->dwProgress);
			McardCFInactive();
		}
		break;

	case SECTOR_WRITE_CMD:
		if (mail->dwBlockAddr >= pDev->dwCapacity)
		{
			mpDebugPrint("CF: read addr exceed capacity(%d/%d)", mail->dwBlockAddr, pDev->dwCapacity);
			pDev->swStatus=FAIL;
		}
		else
		{
			McardCFActive(INITIAL_MEMORY);
			pDev->swStatus = FlatWrite(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr, &mail->dwProgress);
			McardCFInactive();
		}
		break;

	case RAW_FORMAT_CMD:
		McardCFActive(INITIAL_MEMORY);
		pDev->swStatus = LowLevelFormat();
		McardCFInactive();
		break;

	default:
		MP_DEBUG1("-E- INVALID CMD %d", mail->wCmd);
		break;
	}
	mail->swStatus = pDev->swStatus;
#if (DM9KS_ETHERNET_ENABLE == 1 )
	//IntEnable();	
	Gpio_IntEnable(GetGpioIntIndexByGpioPinNum(GPIO_GPIO_3));
	SemaphoreRelease(CFETHERNET_MCARD_SEMA);
#endif
}

static SWORD PhysicalSpeedTest()
{
	#define TESTSZ  (512 * 2)
	#define RSIZE   100
	SWORD swRetValue = 0;
	DWORD progress = 0;
	DWORD *buf = ((DWORD)ext_mem_malloc(TESTSZ * 1024) | 0xA0000000);

	int i, j;
	DWORD tmr = GetSysTime();
	for(j=0; j<RSIZE; j++)
		LogicalRead(buf, 256 * 8, 0, &progress);
	mpDebugPrint("Average speed: %d MB/ms", (RSIZE * 1000000 / SystemGetElapsedTime(tmr)));
	while(1);
}

static SWORD LogicalRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	register DWORD dwCount, dwTempSectorCount;
	register SWORD swRetValue;
	register BYTE *pbBuffer;
	BYTE readCnt = 0;

	pbBuffer = (BYTE *) dwBufferAddress;
	while (dwSectorCount)
	{
		if (dwSectorCount >= 256)
		{
			dwTempSectorCount = 256;
			dwCount = 0;
		}
		else
		{
			dwTempSectorCount = dwSectorCount;
			dwCount = dwSectorCount;
		}
		if (Polling_CF_Status())
			return FAIL;

		SetParameter(dwCount, dwLogAddr);
		SetCommand(COMMAND_READ_SECTOR);

		dwCount = dwTempSectorCount;
		while (dwCount)
		{
			if (Polling_CF_Status())
				return FAIL;
			//IODelay(40);	// This delay looks unnecessary...
			if ((swRetValue = CommandReady()))
			{
				MP_DEBUG1("-E- Read command FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
#if 0
			if ((swRetValue = ReadSector((DWORD) pbBuffer, MCARD_SECTOR_SIZE)))
			{
				MP_DEBUG1("-E- DMA Read FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			pbBuffer += MCARD_SECTOR_SIZE;
			dwCount--;
			(*progress)++;

#else       // Multiple Sectors reading version - read 32 sectors at one time
			if(dwCount >= 32)
				readCnt = 32;
			else
				readCnt = dwCount;

			if ((swRetValue = ReadSector((DWORD) pbBuffer, MCARD_SECTOR_SIZE * readCnt)))
			{
				MP_DEBUG1("-E- DMA Read FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			pbBuffer += (MCARD_SECTOR_SIZE * readCnt);
			dwCount -= readCnt;
			(*progress)+= readCnt;
#endif
		}

		dwSectorCount = dwSectorCount - dwTempSectorCount;
		dwLogAddr = dwLogAddr + dwTempSectorCount;

		if ((swRetValue = WaitReady()))
		{
			MP_DEBUG1("-E- Ready FAIL(swRetValue: %d)", swRetValue);
			return swRetValue;
		}
	}

	return PASS;
}

static SWORD LogicalWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	DWORD dwCount, dwTempSectorCount;
	SWORD swRetValue;
	BYTE *pbBuffer;

	pbBuffer = (BYTE *) dwBufferAddress;
	while (dwSectorCount)
	{
		if (Polling_CF_Status())
			return FAIL;
		if (dwSectorCount >= 256)
		{
			dwTempSectorCount = 256;
			dwCount = 0;
		}
		else
		{
			dwTempSectorCount = dwSectorCount;
			dwCount = dwSectorCount;
		}

		SetParameter(dwCount, dwLogAddr);
		SetCommand(COMMAND_WRITE_SECTOR);

		dwCount = dwTempSectorCount;
		while (dwCount)
		{
			if ((swRetValue = CommandReady()))
			{
				MP_DEBUG1("-E- Write command FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}
			if ((swRetValue = WriteSector((DWORD) pbBuffer, MCARD_SECTOR_SIZE)))
			{
				MP_DEBUG1("-E- DMA Write FAIL (swRetValue: %d)", swRetValue);
				return swRetValue;
			}

			pbBuffer += MCARD_SECTOR_SIZE;
			dwCount--;
			(*progress)++;
		}

		dwSectorCount = dwSectorCount - dwTempSectorCount;
		dwLogAddr = dwLogAddr + dwTempSectorCount;

		if ((swRetValue = WaitReady()))
		{
			MP_DEBUG1("-E- Ready FAIL(swRetValue: %d)", swRetValue);
			return swRetValue;
		}
	}

	return PASS;
}

static SWORD SelfAdjustingIdent(DWORD *pdwTotalSector)
{
	SWORD swRetValue;

	CalculatePulseTiming(TIMING_80ns);
	McardCFActive(INITIAL_MEMORY);
	if ((swRetValue = Identify(pdwTotalSector)))
	{
		CalculatePulseTiming(TIMING_250ns);
		McardCFActive(INITIAL_MEMORY);
		if ((swRetValue = Identify(pdwTotalSector)))
		{
			CalculatePulseTiming(TIMING_SLOWEST);
			McardCFActive(INITIAL_MEMORY);
			swRetValue = Identify(pdwTotalSector);
		}
	}

	return swRetValue;
}

static SWORD Identify(DWORD * pdwTotalSector)
{
	SWORD swRetValue;
	WORD wSign;

	DWORD bReadBuffer[MCARD_SECTOR_SIZE>>2];
	BYTE *pbReadBuffer = (BYTE *) (((DWORD) bReadBuffer | 0xa0000000));

	//mpDebugPrint("--- CF Identify ---");
	//debugCFtiming();

	SetCommand(COMMAND_IDENTIFY_DRIVE);
	if ((swRetValue = CommandReady()))
	{
		mpDebugPrint("-E- Identify command(0xEC) FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	if ((swRetValue = ReadSector((DWORD) pbReadBuffer, MCARD_SECTOR_SIZE)))
	{
		mpDebugPrint("-E- Indetify DMA Read FAIL (swRetValue: %d)", swRetValue);
		return swRetValue;
	}

	wSign = ((pbReadBuffer[1] << 8) | pbReadBuffer[0]);
	MP_DEBUG1("-I- Sign %x", wSign);
	if ((wSign == IDENTIFY_ID)||(wSign == 0x0000) ||(wSign == 0x8c8a)||(wSign == 0x44a) ||(wSign == 0x45a))
	{
		*pdwTotalSector =
			((pbReadBuffer[123] << 24) | (pbReadBuffer[122] << 16) | (pbReadBuffer[121] << 8) |
			 pbReadBuffer[120]);
		mpDebugPrint("CF Driver, %dKHz", GetMcardClock());
		MP_DEBUG3("-I- Total %d(%x) sectors, %d KB", *pdwTotalSector, *pdwTotalSector,
				  (*pdwTotalSector >> 1));

		// Even the card information shows that 80ns timing mode is supported.
		// We still fail to idetify the card. Give up check WORD164...
		#if 0		// Get timing support
		mpDebugPrint("Dumpping CIS buffer: ");
		MemDump(pbReadBuffer, 512);
		mpDebugPrint("Support Speed: %x", ((pbReadBuffer[329] << 8) | pbReadBuffer[328]));
		#endif

		return PASS;
	}
	else
	{
		mpDebugPrint("CF identified, but unknown type!%x)", wSign);
	}

	return FAIL;
}


static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	SWORD swRetValue = FAIL;
	BYTE bRetry;

	MP_DEBUG2("-I- CF FlatRead dwLogAddr %d,dwSectorCount %d", dwLogAddr,dwSectorCount);
	bRetry = MCARD_RETRY_TIME;
	while (bRetry)
	{
		if (Polling_CF_Status())
			return FAIL;
		*progress = 0;
		if (!(swRetValue = LogicalRead(dwBufferAddress, dwSectorCount, dwLogAddr, progress)))
		{
			return PASS;
		}
		bRetry--;
		MP_DEBUG1("-I- remain retry times %d", bRetry);
	}

	// First time fail, change to slower CF pulse timing and retry again.
	if((swRetValue != PASS) && (bRetry == 0)){
		if(CFSlowPulseTimingCheck() == PASS){
			CalculatePulseTiming(TIMING_SLOWEST);
			McardCFActive(INITIAL_MEMORY);
			return FlatRead(dwBufferAddress, dwSectorCount, dwLogAddr, progress);
		}
	}

	return swRetValue;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr, DWORD *progress)
{
	SWORD swRetValue;
	BYTE bRetry;
		MP_DEBUG2("-I- CF FlatWrite dwLogAddr %d,dwSectorCount %d", dwLogAddr,dwSectorCount);
	bRetry = MCARD_RETRY_TIME;
	while (bRetry)
	{
		if (Polling_CF_Status())
			return FAIL;
		*progress = 0;
		if (!(swRetValue = LogicalWrite(dwBufferAddress, dwSectorCount, dwLogAddr, progress)))
		{
			return PASS;
		}
		bRetry--;
		MP_DEBUG1("-I- remain retry times %d", bRetry);
	}
	return swRetValue;
}

static SWORD LowLevelFormat(void)
{
	return FAIL;
}

#endif
