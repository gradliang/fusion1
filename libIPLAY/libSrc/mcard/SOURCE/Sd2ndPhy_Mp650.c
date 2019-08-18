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
* Filename      : SdPhy2_Mp650.c
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
#if (CHIP_VER_MSB == CHIP_VER_650)
#if SD2_ENABLE
#include "mpTrace.h"
#include "Mcard.h"
#include "Uti.h"
#include "sd.h"
#include "taskid.h"

#undef TRIGGER_BY_INT
#define TRIGGER_BY_INT			0

#define IC_RSPDONE 			    0x00000001
#define IC_TREND   			    0x00000002
#define IC_RSPF    			    0x00000004
#define IC_CRC16F  			    0x00000008
#define IC_CRC7F   			    0x00000010
#define IC_PGMF    			    0x00000020
#define IC_SDTO    			    0x00000040
#define IC_WXF     			    0x00000080
#define IC_CMDFAIL				(IC_RSPF|IC_CRC7F|IC_SDTO)
#define IC_DATAFAIL				(IC_CRC16F|IC_PGMF|IC_WXF|IC_SDTO)
#define IM_RSPDONE 			    0x00000100
#define IM_TREND   			    0x00000200
#define IM_RSPF    			    0x00000400
#define IM_CRC16F  			    0x00000800
#define IM_CRC7F   			    0x00001000
#define IM_PGMF    			    0x00002000
#define IM_SDTO    			    0x00004000
#define IM_WXF     			    0x00008000
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

static SWORD SdHost2_WaitHostBusy()
{
	MCARD *sMcard = (MCARD *)MCSDIO_BASE;
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
		mpDebugPrint("\tSdHost2_WaitHostBusy timeout : %x", sMcard->McSdC & (FSMBUSY|R1B));
		ret = GENERAL_FAIL;
	}

	return ret;
}

SWORD SdHost2_WaitCardResponse(DWORD *Resp)
{
	register MCARD *sMcard = (MCARD *)MCSDIO_BASE;
	DWORD dwCause = 0;
	SWORD ret = SD_PASS;

#if TRIGGER_BY_INT
	sMcard->McSdIc |= IM_CMD;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &dwCause, 1400) != PASS)
	{
		mpDebugPrint("\tSdHost2_WaitCardResponse INT timeout %x", dwCause);
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
	dwCause = sMcard->McSdIc;
	sMcard->McSdIc &= ~(IC_RSPDONE | IC_CMDFAIL);
#endif
	
	if (dwCause & IC_CMDFAIL)
	{
		MP_DEBUG("\t-E- SdHost2_WaitCardResponse fail : %x", dwCause);
		if (dwCause & IC_CRC7F)
			ret =  BAD_CRC7;
		else if (dwCause & IC_SDTO)
			ret =  RES_TIMEOUT;
		else
			ret =  GENERAL_FAIL;
	}
	else if (!(dwCause & IC_RSPDONE))
	{
		MP_DEBUG("\t-E- SdHost2_WaitCardResponse error : %x", dwCause);
		ret = RES_TIMEOUT;
	}
	if (ret != SD_PASS)
		MP_DEBUG("\t\tMcardC=%x, McSdC=%x, McSdArg=%x, McSdIc=%x, McSdOp=%x", sMcard->McardC, sMcard->McSdC, sMcard->McSdArg, sMcard->McSdIc, sMcard->McSdOp);

	if (Resp)
		*Resp = sMcard->McSdRspA;
	
	return ret;
}

SWORD SdHost2_WaitTransfer()
{
	register MCARD *sMcard = (MCARD *)MCSDIO_BASE;
	DWORD dwCause;
	DWORD tmr;
	SWORD ret = SD_PASS;

#if TRIGGER_BY_INT
	sMcard->McSdIc |= IM_DATA;
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &dwCause, 3000) != PASS)
	{
		mpDebugPrint("\tSdHost2_WaitTransfer INT timeout %x", sMcard->McSdIc);
		sMcard->McSdIc = 0;
		dwCause = 0;
	}
#else
	tmr = GetSysTime();
	do
	{
		if (sMcard->McSdIc & IC_TREND)
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1500);	// 1.5sec
	dwCause = sMcard->McSdIc;
	sMcard->McSdIc &= ~(IC_TREND | IC_DATAFAIL);
#endif

	if (dwCause & IC_DATAFAIL)
	{
		mpDebugPrint("\t-E- SdHost2_WaitTransfer FAIL : %x", dwCause);
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
		mpDebugPrint("\t-E- SdHost2_WaitTransfer timeout : %x", dwCause);
		ret = RES_TIMEOUT;
	}
	if (ret != SD_PASS)
	{
		mpDebugPrint("\t\tMcardC=%x, McSdC=%x, McSdArg=%x, McSdIc=%x, McSdOp=%x", sMcard->McardC, sMcard->McSdC, sMcard->McSdArg, sMcard->McSdIc, sMcard->McSdOp);
	}
	else
	{
		ret = FAIL;
		tmr = GetSysTime();
		do
		{
			if (!(((CHANNEL *)DMA_INTSRAM1_BASE)->Control & MCARD_DMA_ENABLE))
			{
				ret = PASS;
				break;
			}
		}
		while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
		if (ret == FAIL)
		{
			mpDebugPrint("SD: DMA/FIFO timeout, DMA_C=%x,MCARD_C=%x", ((CHANNEL *)DMA_INTSRAM1_BASE)->Control, sMcard->McardC);
			//while(1);
		}
	}
	
	return ret;
}

void SdHost2_SendCmd(WORD wCommand, DWORD dwArgument)
{
	register MCARD *sMcard = (MCARD *)MCSDIO_BASE;
	
	if (SdHost2_WaitHostBusy() != SD_PASS)
		mpDebugPrint("\tSdHost2_WaitHostBusy failed before cmd%d, 0x%x", wCommand & 0x3f, dwArgument);
	sMcard->McSdC = McSdC | (wCommand & 0x3f);
	sMcard->McSdArg = dwArgument;
	sMcard->McSdIc = 0;
	sMcard->McSdOp = (wCommand >> 8);
	MP_DEBUG("\tSD2 Cmd%d : %x,%x", wCommand & 0x3f, sMcard->McSdC, sMcard->McSdArg);
	McardIODelay(50);
}

void SdHost2_PhyTransfer(DWORD dwBufferAddress, DWORD dwSize, DWORD dwDirection)
{
	register MCARD *sMcard = (MCARD *)MCSDIO_BASE;
	register CHANNEL *sChannel = (CHANNEL *)DMA_INTSRAM1_BASE;
	DWORD tmr = GetSysTime();

	do
	{
		sMcard->McardC &= ~MCARD_FIFO_ENABLE;	// clean FIFO
		if (sMcard->McardC & MCARD_FIFO_EMPTY)
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
	MP_DEBUG("\tSD2 DMA %s : 0x%x, %d", (dwDirection == MCARD_DMA_DIR_CM)?"Read":"Write", dwBufferAddress, dwSize);
	sChannel->Control = 0;
	if (dwDirection)
		sMcard->McardC |= dwDirection;
	else
		sMcard->McardC &= ~dwDirection;
	sChannel->StartA = dwBufferAddress;
	sChannel->EndA = dwBufferAddress + dwSize - 1;
	sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE | (dwSize >> 2);
	sChannel->Control = MCARD_DMA_ENABLE;
	sMcard->McardC |= MCARD_FIFO_ENABLE;
}

void SdHost2_GetResponse(DWORD *Resp)
{
	MCARD *sMcard = (MCARD *)MCSDIO_BASE;

	Resp[0] = sMcard->McSdRspA;
	Resp[1] = sMcard->McSdRspB;
	Resp[2] = sMcard->McSdRspC;
	Resp[3] = sMcard->McSdRspD;
}

void SdHost2_SetHostBlkLen(DWORD length)
{
	((MCARD *)MCSDIO_BASE)->McSdBl = BIT12 | length;
}

void SdHost2_DmaReset()
{
	register CHANNEL *sChannel = (CHANNEL *)DMA_INTSRAM1_BASE;
	register MCARD *sMcard = (MCARD *)MCSDIO_BASE;

	sChannel->Control = 0;
	sMcard->McDmarl = 0;
	sChannel->StartA = 0;
	sChannel->EndA = 0;
	sMcard->McSdIc = 0;
}

void McardSd2Inactive(DWORD cardtype)
{
	McardDeselect(PIN_DEFINE_FOR_SD2);
	((MCARD *)MCSDIO_BASE)->McSdC = 0;
	((MCARD *)MCSDIO_BASE)->McardC = 0;
	//SdHost_DmaReset();
}

static DWORD SdDataPinNr;
void SdHost2_DataConfig(DWORD data_nr)
{
	SdDataPinNr = data_nr;
}

void McardSd2FreeDataPin()
{
	GPIO *sGpio = (GPIO *)GPIO_BASE;
	DWORD i, tmp = SdDataPinNr;
	
	for (i = 0 ; i < 8 ; i++, tmp++)
	{
		sGpio->Fgpcfg[tmp>>4] &= ~(0x00010001 << (tmp & 0x0f));	// as gpio
		sGpio->Fgpdat[tmp>>4] |= 0x00010001 << (tmp & 0x0f);	// output high
	}
}

void McardSd2Active(DWORD BusWidth, DWORD HiSpeed)
{
	register MCARD *sMcard = (MCARD *)MCSDIO_BASE;
	
    ((CLOCK *)CLOCK_BASE)->Clkss_EXT2 &= ~BIT6;	// disable SPI flash function
	sMcard->McardC = 0;
	sMcard->McSdC = 0;
	McardSelect(PIN_DEFINE_FOR_SD2);
	sMcard->McWdt = WATCHDOG_TIMER;
	sMcard->McWdt &= ~BIT21 ;
	sMcard->McardC = MCARD_SD_ENABLE | BIT9;
	SdHost2_SetHostBlkLen(MCARD_SECTOR_SIZE);
	McSdC = /*BIT17 | (HiSpeed << 16) | */(BusWidth << 14) | BIT8 | BIT7;
	MP_DEBUG("\tSDIO, Bus width:%x, speed:%x", BusWidth, HiSpeed);
}

#endif
#endif
