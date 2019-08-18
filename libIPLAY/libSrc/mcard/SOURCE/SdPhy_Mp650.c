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
* Filename      : SdPhy_Mp650.c
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#if SD_MMC_ENABLE
#include "mpTrace.h"
#include "Mcard.h"
#include "Uti.h"
#include "sd.h"
#include "taskid.h"

#undef TRIGGER_BY_INT
#define TRIGGER_BY_INT			0
#define WAIT_TREND_WORK_ARROUND         1

#define IC_RSPDONE              0x00000001
#define IC_TREND                0x00000002
#define IC_RSPF                 0x00000004
#define IC_CRC16F               0x00000008
#define IC_CRC7F                0x00000010
#define IC_PGMF                 0x00000020
#define IC_SDTO                 0x00000040
#define IC_WXF                  0x00000080
#define IC_CMDFAIL              (IC_RSPF|IC_CRC7F|IC_SDTO)
#define IC_DATAFAIL             (IC_CRC16F|IC_PGMF|IC_WXF|IC_SDTO)
#define IM_RSPDONE              0x00000100
#define IM_TREND                0x00000200
#define IM_RSPF                 0x00000400
#define IM_CRC16F               0x00000800
#define IM_CRC7F                0x00001000
#define IM_PGMF                 0x00002000
#define IM_SDTO                 0x00004000
#define IM_WXF                  0x00008000
#if TRIGGER_BY_INT
#define IM_CMD					(IM_RSPDONE|IM_RSPF|IM_CRC7F|IM_SDTO)
#define IM_DATA					(IM_TREND|IM_CRC16F|IM_PGMF|IM_WXF|IM_SDTO)
#else
#define IM_CMD					0
#define IM_DATA					0
#endif
#define R1B						BIT11
#define FSMBUSY                 BIT12

#define WATCHDOG_TIMER			0x003fffff

static DWORD McSdC = 0;
static MCARD *sMcard = (MCARD *)MCARD_BASE;
static void DumpMcarRegister(BYTE n)
{
	register MCARD *sMcard;
    register CHANNEL *sChannel;
    
    sChannel = (CHANNEL *) (DMA_MC_BASE);
	sMcard = (MCARD *) (MCARD_BASE);
    mpDebugPrint("<<<<<<<<<<<<<<<<s%d>", n);
    mpDebugPrint("DMA Cur addr   = 0x%x", sChannel->Current);
    mpDebugPrint("DMA Start addr = 0x%x", sChannel->StartA);
    mpDebugPrint("DMA End addr   = 0x%x", sChannel->EndA);
    mpDebugPrint("DMA Enable bit = 0x%x", sChannel->Control);
    mpDebugPrint("2_1000 = 0x%x", sMcard->McardC);
    mpDebugPrint("2_1004 = 0x%x", sMcard->McDmarl);
    mpDebugPrint("2_1008 = 0x%x", sMcard->McRtm);
    mpDebugPrint("2_100c = 0x%x", sMcard->McWtm);
    mpDebugPrint("2_1010 = 0x%x", sMcard->McWdt);
    mpDebugPrint("2_1070 = 0x%x", sMcard->McSdOp);
    mpDebugPrint("2_1074 = 0x%x", sMcard->McSdIc);
    mpDebugPrint("2_1078 = 0x%x", sMcard->McSdC);
    mpDebugPrint("2_107c = 0x%x", sMcard->McSdArg);
    mpDebugPrint("2_1080 = 0x%x", sMcard->McSdRspA);
    mpDebugPrint("2_1084 = 0x%x", sMcard->McSdRspB);
    mpDebugPrint("2_1088 = 0x%x", sMcard->McSdRspC);
    mpDebugPrint("2_108c = 0x%x", sMcard->McSdRspD);
    mpDebugPrint("2_1090 = 0x%x", sMcard->McSdSc);
    mpDebugPrint("2_1094 = 0x%x", sMcard->McSdBl);
    mpDebugPrint("<s%d>>>>>>>>>>>>>>>", n);
}

static SWORD SdHost_WaitHostBusy()
{
	DWORD tmr = GetSysTime();
	SWORD ret = SD_PASS;

	do
	{
		if (!(sMcard->McSdC & FSMBUSY))
			break;
	}
	while (SystemGetElapsedTime(tmr) < 500);	// 500ms
	
	if (sMcard->McSdC & FSMBUSY)
	{
		mpDebugPrint("\tSdHost_WaitHostBusy timeout : %x", sMcard->McSdC & (FSMBUSY|R1B));
		ret = GENERAL_FAIL;
	}

	return ret;
}

SWORD SdHost_WaitCardResponse(DWORD *Resp)
{
	DWORD dwCause = 0;
	SWORD ret = SD_PASS;

#if TRIGGER_BY_INT
	sMcard->McSdIc |= IM_CMD;

	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &dwCause, 1400) != PASS)
	{
		mpDebugPrint("\tSdHost_WaitCardResponse INT timeout %x", dwCause);
		sMcard->McSdIc = 0;
		dwCause = 0;
	}
#else
	DWORD tmr = GetSysTime();
	do
	{
		if (sMcard->McSdIc & IC_RSPDONE)
			break;
	}
	while (SystemGetElapsedTime(tmr) < 700);	// 700ms

	// SD2.0 LOG for 1.2-4
	if((sMcard->McSdC & 0x3f) == 8) // Check CRC Error when CMD8
	{
		tmr = GetSysTime();
		do
		{
			if (sMcard->McSdIc & (IC_CRC16F|IC_CRC7F))
			{
				MP_DEBUG("CMD8 sMcard->McSdIc IC_CRC16F 0x%x IC_CRC7F 0x%x Tmr %d", sMcard->McSdIc & IC_CRC16F, sMcard->McSdIc & IC_CRC7F, SystemGetElapsedTime(tmr));
				break;
			}
		}
		while (SystemGetElapsedTime(tmr) < 100);	// 100ms
	}

	
	dwCause = sMcard->McSdIc;
	sMcard->McSdIc &= ~(IC_RSPDONE | IC_CMDFAIL);
#endif
	
	if (dwCause & IC_CMDFAIL)
	{
		MP_DEBUG("\t-E- SdHost_WaitCardResponse fail : %x", dwCause);
		if (dwCause & IC_CRC7F)
			ret =  BAD_CRC7;
		else if (dwCause & IC_SDTO)
			ret =  RES_TIMEOUT;
		else
			ret =  GENERAL_FAIL;
	}
	else if (!(dwCause & IC_RSPDONE))
	{
		MP_DEBUG("\t-E- SdHost_WaitCardResponse error : %x", dwCause);
		ret = RES_TIMEOUT;
	}
	if (ret != SD_PASS)
		MP_DEBUG("\t\tMcardC=%x, McSdC=%x, McSdArg=%x, McSdIc=%x, McSdOp=%x", sMcard->McardC, sMcard->McSdC, sMcard->McSdArg, sMcard->McSdIc, sMcard->McSdOp);

	if (Resp)
		*Resp = sMcard->McSdRspA;
	
	return ret;
}

SWORD SdHost_WaitTransfer()
{
	DWORD dwCause;
	DWORD tmr;
	SWORD ret = SD_PASS;

#if TRIGGER_BY_INT
	sMcard->McSdIc |= IM_DATA;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &dwCause, 3000) != PASS)
	{
		mpDebugPrint("\tSdHost_WaitTransfer INT timeout %x", sMcard->McSdIc);
		sMcard->McSdIc = 0;
		dwCause = 0;
	}
#else
	tmr = GetSysTime();
	do
	{
#if WAIT_TREND_WORK_ARROUND
            register CHANNEL *sChannel;
        
            sChannel = (CHANNEL *) (DMA_MC_BASE);
            if (sMcard->McSdIc & IC_TREND)
            {
                break;
            }
            else if ((!(sMcard->McSdC & FSMBUSY))& (!(sChannel->Control & BIT0)))
            {
                break;
            }
#else
	    if (sMcard->McSdIc & IC_TREND)
		break;
#endif
	}
	while (SystemGetElapsedTime(tmr) < 1500);	// 1.5sec
	dwCause = sMcard->McSdIc;
	sMcard->McSdIc &= ~(IC_TREND | IC_DATAFAIL);
#endif

#if WAIT_TREND_WORK_ARROUND
    register CHANNEL *sChannel;    
    sChannel = (CHANNEL *) (DMA_MC_BASE);
    if(!(dwCause & IC_TREND) && (sChannel->Current >= sChannel->EndA)) // not in while
    {
        mpDebugPrint("\t-W- SdHost DMA finish, but no ISR\n"); // Workaround for IC.        
        dwCause |= IC_TREND;
    }    
#endif
    

	if (dwCause & IC_DATAFAIL)
	{
		mpDebugPrint("\t-E- SdHost_WaitTransfer FAIL : %x", dwCause);
		if (dwCause & IC_CRC16F)
			ret = BAD_CRC16;
		else if ((dwCause & IC_RSPF) || (dwCause & IC_WXF))
			ret = PROGRAM_FAIL;
		else if (dwCause & IC_SDTO)
			ret = WRITE_TIMEOUT;
		else
			ret = GENERAL_FAIL;
	}
	else if (!(dwCause & IC_TREND))
	{
		mpDebugPrint("\t-E- SdHost_WaitTransfer TIMEOUT : %x", dwCause);
		ret = RES_TIMEOUT;
	}
	if (ret != SD_PASS)
	{
		mpDebugPrint("\t\tMcardC=%x, McSdC=%x, McSdArg=%x, McSdIc=%x, McSdOp=%x", sMcard->McardC, sMcard->McSdC, sMcard->McSdArg, sMcard->McSdIc, sMcard->McSdOp);
                DumpMcarRegister(1);
	}
	else
	{
		ret = FAIL;
		tmr = GetSysTime();
		do
		{
			if (!(((CHANNEL *)DMA_MC_BASE)->Control & MCARD_DMA_ENABLE))
			{
				ret = PASS;
				break;
			}
		}
		while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
		if (ret == FAIL)
		{
			mpDebugPrint("SD: DMA/FIFO timeout, DMA_C=%x,MCARD_C=%x", ((CHANNEL *)DMA_MC_BASE)->Control, sMcard->McardC);
			//while(1);
		}
	}
	
	return ret;
}

void SdHost_SendCmd(WORD wCommand, DWORD dwArgument)
{
	if (SdHost_WaitHostBusy() != SD_PASS)
		mpDebugPrint("\tSdHost_WaitHostBusy failed before cmd%d, 0x%x", wCommand & 0x3f, dwArgument);
	sMcard->McSdC = McSdC | (wCommand & 0x3f);
	sMcard->McSdArg = dwArgument;
	sMcard->McSdIc = 0;
	sMcard->McSdOp = (wCommand >> 8);
	MP_DEBUG("\tSD Cmd%d : %x,%x", wCommand & 0x3f, sMcard->McSdC, sMcard->McSdArg);
	McardIODelay(50);
}

void SdHost_PhyTransfer(DWORD dwBufferAddress, DWORD dwSize, DWORD dwDirection)
{
	register CHANNEL *sChannel = (CHANNEL *)DMA_MC_BASE;
	DWORD tmr = GetSysTime();

	do
	{
		sMcard->McardC &= ~MCARD_FIFO_ENABLE;	// clean FIFO
		if (sMcard->McardC & MCARD_FIFO_EMPTY)
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
	MP_DEBUG("\tSD DMA %s : 0x%x, %d", (dwDirection == MCARD_DMA_DIR_CM) ? "Read" : "Write", dwBufferAddress, dwSize);
	sChannel->Control = 0;
	if (dwDirection)
		sMcard->McardC |= MCARD_DMA_DIR_CM;   // set MCDMADR to 1    mem <- card
	else
		sMcard->McardC &= ~MCARD_DMA_DIR_CM;  // clean MCDMADR to 0  mem -> card
	sChannel->StartA = dwBufferAddress;
	sChannel->EndA = dwBufferAddress + dwSize - 1;
	sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE | (dwSize >> 2);
	sChannel->Control = MCARD_DMA_ENABLE;
	sMcard->McardC |= MCARD_FIFO_ENABLE;
}

void SdHost_GetResponse(DWORD *Resp)
{
	Resp[0] = sMcard->McSdRspA;
	Resp[1] = sMcard->McSdRspB;
	Resp[2] = sMcard->McSdRspC;
	Resp[3] = sMcard->McSdRspD;
}

void SdHost_SetHostBlkLen(DWORD length)
{
	((MCARD *)MCARD_BASE)->McSdBl = BIT12 | length;
}

void SdHost_DmaReset()
{
	register CHANNEL *sChannel = (CHANNEL *)DMA_MC_BASE;

	sChannel->Control = 0;
	sMcard->McDmarl = 0;
	sChannel->StartA = 0;
	sChannel->EndA = 0;
	sMcard->McSdIc = 0;
}

void McardSdInactive(DWORD cardtype)
{
	McardDeselect((cardtype == MCARD_DMA_MMC) ? PIN_DEFINE_FOR_MMC : PIN_DEFINE_FOR_SD);
	sMcard->McSdC = 0;
	sMcard->McardC = 0;
//	SdHost_DmaReset();
}

static DWORD SdDataPinNr;
void SdHost_DataConfig(DWORD data_nr)
{
	SdDataPinNr = data_nr;
}

void McardSdFreeDataPin()
{
	GPIO *sGpio = (GPIO *)GPIO_BASE;
	DWORD i, tmp = SdDataPinNr;
	
	for (i = 0 ; i < 8 ; i++, tmp++)
	{
		sGpio->Fgpcfg[tmp>>4] &= ~(0x00010001 << (tmp & 0x0f));	// as gpio
		sGpio->Fgpdat[tmp>>4] |= 0x00010001 << (tmp & 0x0f);	// output high
	}
}

void McardSdActive(DWORD CardType, DWORD BusWidth, DWORD HiSpeed)
{
	((CLOCK *)CLOCK_BASE)->Clkss_EXT2 &= ~BIT6;	// disable SPI flash function
	sMcard->McardC = 0;
	sMcard->McSdC = 0;
	if (CardType == 0)
		CardType = MCARD_DMA_SD;
	McardSelect((CardType == MCARD_DMA_MMC) ? PIN_DEFINE_FOR_MMC : PIN_DEFINE_FOR_SD);
	sMcard->McWdt = WATCHDOG_TIMER;
	sMcard->McWdt &= ~BIT25;	// disable watch dog timer
	sMcard->McardC = MCARD_SD_ENABLE | BIT3;
	SdHost_SetHostBlkLen(MCARD_SECTOR_SIZE);
	McSdC = /*BIT17 | (HiSpeed << 16) | */(BusWidth << 14) | BIT8 | BIT7;
	MP_DEBUG("\tCard:%x, Bus width:%x, speed:%x", CardType, BusWidth, HiSpeed);
}

#endif
#endif
