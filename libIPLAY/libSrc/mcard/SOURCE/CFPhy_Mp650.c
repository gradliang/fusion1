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
* Filename      : CFPhy_Mp615.c
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

//#undef TRIGGER_BY_INT
#define TRIGGER_BY_INT			1

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#if (CF_ENABLE && ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660)))

#include "devio.h"
#include "Mcard.h"
#include "uti.h"
#include "cf.h"
#include "taskid.h"

/*
// Constant declarations
*/
#define MEMORY_MODE                 0x00000000
#define IO_MODE                     0x00000004
#define TRUE_IDE_MODE               0x00000008
#define ADR_NORMAL                  0x00000000
#define ADR_PRIMARY                 0x00000010
#define ADR_SECONDARY               0x00000020
#define WIDTH_8BIT                  0x00000040
#define WIDTH_16BIT                 0x00000000
#define SINGLE_SECTOR               0x00000000
#define MULTI_SECTOR                0x00000100
#define MULTI_SECTOR_WAIT_EDGE      0x00000000
#define MULTI_SECTOR_WAIT_TIME      0x00000200
#define RESET_ENABLE                0x00010000

#define MEMORY_SETTING   (MEMORY_MODE | ADR_NORMAL | WIDTH_8BIT | SINGLE_SECTOR)
#define TRUE_IDE_SETTING (TRUE_IDE_MODE | ADR_NORMAL | WIDTH_16BIT | SINGLE_SECTOR | RESET_ENABLE)
#define IO_SETTING       (IO_MODE | ADR_NORMAL | WIDTH_16BIT | SINGLE_SECTOR & (~RESET_ENABLE))

#define IC_CD1                      0x00000001
#define IC_CD2                      0x00000002
#define IC_FRDY                     0x00000004
#define IC_FVS1                     0x00000008
#define IC_FVS2                     0x00000010
#define IC_ACSER                    0x00000020
#define IC_CFTCE                    0x00000040
#define IC_CFTO                     0x00000080
#define IC_CD_ALL                   (IC_CD1 | IC_CD2)
#define IM_CD1                      0x00000100
#define IM_CD2                      0x00000200
#define IM_FRDY                     0x00000400
#define IM_FVS1                     0x00000800
#define IM_FVS2                     0x00001000
#define IM_ACSER                    0x00002000
#define IM_CFTCE                    0x00004000
#define IM_CFTO                     0x00008000
#define IM_ALL                      0	/*(IM_FRDY | IM_ACSER | IM_CFTCE | IM_CFTO) */
#define PL_HIGH_CD1                 0x00010000
#define PL_HIGH_CD2                 0x00020000
#define PL_HIGH_FRDY                0x00040000
#define PL_HIGH_FVS1                0x00080000
#define PL_HIGH_FVS2                0x00100000
#define PL_HIGH_ACSER               0x00200000
#define PL_HIGH_CFTCE               0x00400000
#define PL_HIGH_CFTO                0x00800000
#define MD_EDGE_CD1                 0x01000000
#define MD_EDGE_CD2                 0x02000000
#define MD_EDGE_FRDY                0x04000000
#define MD_EDGE_FVS1                0x08000000
#define MD_EDGE_FVS2                0x10000000
#define MD_EDGE_ACSER               0x20000000
#define MD_EDGE_CFTCE               0x40000000
#define MD_EDGE_CFTO                0x80000000

#define GPIO_DEFINE_0               0x0000fffe//Fpgio[15~1] : reset,a3,a2,a1,a0,we,d7~0,oe
#define GPIO_DEFINE_1               0x000078c0//Fgpio[30~27] :cd2,cd1,ce2,ce1 ,, Fgpio[23,22] : rb,wait 
#define GPIO_DEFINE_2               0x00000030//Fgpio[37,36] : reg,wp
#define H_GPIO_DEFINE_0             0xfffe0000
#define H_GPIO_DEFINE_1             0x78c00000 
#define H_GPIO_DEFINE_2             0x00300000
/*
//only test for cf
#define GPIO_DEFINE_0               0x0000ffff
#define GPIO_DEFINE_1               0x0000ffff
#define GPIO_DEFINE_2               0x0000ffff
#define H_GPIO_DEFINE_0             0xffff0000
#define H_GPIO_DEFINE_1             0xffff0000 
#define H_GPIO_DEFINE_2             0xffff0000
*/

#define MEMORY_READ_TIMMING1		0x02020701 // for video play lag but command may fail when mmc plug out
#define MEMORY_READ_TIMMING         0x04020702
#define MEMORY_WRITE_TIMMING        0x04030d02

// This timing setting looks fine for all CF cards in our company
#define MEMORY_READ_SLOWTIMMING     ((4<<24)|(2<<16)|(8<<8) |4)
#define MEMORY_WRITE_SLOWTIMMING    ((4<<24)|(3<<16)|(13<<8)|4)


#define IDE_READ_TIMMING            0x02010804
#define IDE_WRITE_TIMMING           0x02010804

#define I_O_READ_TIMMING            0x00010201
#define I_O_WRITE_TIMMING           0x00010201

#define DRIVE_PROPERTY              0xe0

#define ERR         0x01
#define CORR        0x04
#define DRQ         0x08
#define DSC         0x10
#define DWF         0x20
#define RDY         0x40
#define BUSY        0x80

#define TIMEOUT_COUNT       2400000
///
///@defgroup CF CompactFlash
///@ingroup CONSTANT
///@{


/// Wait for CompactFlash's ready signal fail.
#define TIMEOUT                     -2
/// Wait for FIFO transaction end Fail.
#define IC_SFTCE_TIMEOUT            -3
/// Wait for DMA end Fail.
#define DMA_TIMEOUT                 -4
///@}

/*
// Variable declarations
*/

/*
// Macro declarations
*/

/*
// Static function prototype
*/
/*
// Definition of internal functions
*/
//==========================================================================
BOOL Polling_CF_Status()
{
	MCARD *mcard = (MCARD *)MCARD_BASE;
	DWORD tmp;
	BOOL  ret = 0;

	tmp = mcard->McCfIc;
	if(tmp & 0x1 != 0x1){
		mpDebugPrint("polling fail...%x", tmp);
		ret = 1;
	}
	return ret;
//	return ((((MCARD *)MCARD_BASE)->McCfIc & 0x1) != 0x1);
}

SWORD WaitReady()
{
	CFATA *sCfata = (CFATA *)MC_CFCOM_BASE;
#if TRIGGER_BY_INT
	MCARD *mcard = (MCARD *)MCARD_BASE;
	DWORD dwCause = 0;

	if (Polling_CF_Status())
		return FAIL;

	mcard->McCfIc |= (IM_FRDY + PL_HIGH_FRDY);// + MD_EDGE_FRDY);
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &dwCause, 3000) != PASS)
	{
		mpDebugPrint("-E- WaitReady FAIL (status: %x)", sCfata->StatusCommand);
		mcard->McCfIc = 0;
		dwCause = 0;
		return TIMEOUT;
	}
	return PASS;
#else

	DWORD tmr = GetSysTime();
	do
	{
		if (Polling_CF_Status())
			return FAIL;

		if (sCfata->StatusCommand == (DSC | RDY))
			return PASS;
	}
	while (SystemGetElapsedTime(tmr) < 3000);	// 3sec

	mpDebugPrint("-E- WaitReady FAIL (status: %x)", sCfata->StatusCommand);
	return TIMEOUT;
#endif
}

SWORD CommandReady()
{
	CFATA *sCfata = (CFATA *)MC_CFCOM_BASE;
#if 0//TRIGGER_BY_INT
	MCARD *mcard = (MCARD *)MCARD_BASE;
	DWORD dwCause = 0;

	if (Polling_CF_Status())
		return FAIL;

	mcard->McCfIc |= 0x0000E000;//(IM_FRDY);// + PL_HIGH_FRDY + MD_EDGE_FRDY);
	if (EventWaitWithTO(MCARD_EVENT_ID, 0xffffffff, OS_EVENT_OR, &dwCause, 3000) != PASS)
	{
		mpDebugPrint("-E- command FAIL (status: 0x%x)", sCfata->StatusCommand);
		mpDebugPrint("				   (Error : 0x%x)", sCfata->ErrorFeature);
		mcard->McCfIc = 0;
		dwCause = 0;
		return TIMEOUT;
	}
	return PASS;
#else
	DWORD tmr = GetSysTime();

	do
	{
		if (Polling_CF_Status())
			return FAIL;

		if ((sCfata->StatusCommand & (BUSY | DRQ)) == DRQ)
			return PASS;
	}
	while (SystemGetElapsedTime(tmr) < 2000);	// 2sec

	mpDebugPrint("-E- command FAIL (status: 0x%x)", sCfata->StatusCommand);
	mpDebugPrint("                 (Error : 0x%x)", sCfata->ErrorFeature);
	return TIMEOUT;
#endif
}


SWORD WriteSector(DWORD dwBufferAdress, WORD wSize)
{
	register CHANNEL *sChannel = (CHANNEL *) (DMA_MC_BASE);
	register MCARD *sMcard = (MCARD *) (MCARD_BASE);
	DWORD tmr;

	if (dwBufferAdress & 0x3)
	{
		mpDebugPrint("-E- source buffer must align to 4 bytes boundary !");
		return FAIL;
	}

	sChannel->Control = 0x0;
	sChannel->StartA = dwBufferAdress;
	sChannel->EndA = dwBufferAdress + wSize - 1;
	sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (wSize >> 2);
	sMcard->McardC = ((sMcard->McardC & 0xffffffef) | MCARD_DMA_DIR_MC | MCARD_FIFO_ENABLE | MCARD_CF_ENABLE);
	
	sChannel->Control = MCARD_DMA_ENABLE;

	/*
	 *  Because we re-enable CF module, card detection bit will change into zero for a while.
	 *  So we put a delay here
	 */
	IODelay(30);	// Delay 30 * 30 * 6ns at CPU clock 120 MHZ case

	tmr = GetSysTime();
	do
	{
		if (Polling_CF_Status())
			return FAIL;
		if ((sMcard->McCfIc & IC_CFTCE))
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
	
	if (!(sMcard->McCfIc & IC_CFTCE))
	{
		mpDebugPrint("-E- FIFO transaction count FAIL (status: %x)", sMcard->McCfIc);
		return IC_SFTCE_TIMEOUT;
	}
	sMcard->McCfIc &= ~IC_CFTCE;

	tmr = GetSysTime();
	do
	{
		if (Polling_CF_Status())
			return FAIL;
		if (!(sChannel->Control & MCARD_DMA_ENABLE))
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
	
	if (sChannel->Control & MCARD_DMA_ENABLE)
	{
		mpDebugPrint("-E- DMA end FAIL (status: %x)", sChannel->Control);
		return DMA_TIMEOUT;
	}

	return PASS;
}

SWORD ReadSector(DWORD dwBufferAdress, WORD wSize)
{
	register CHANNEL *sChannel = (CHANNEL *) (DMA_MC_BASE);
	register MCARD *sMcard = (MCARD *) (MCARD_BASE);
	DWORD tmr;

	if (dwBufferAdress & 0x3)
	{
		mpDebugPrint("-E- target buffer must align to 4 bytes boundary !");
		return FAIL;
	}


	//mpDebugPrint("McCfC1: %x", sMcard->McCfC);
	if(wSize == 512)
		sMcard->McCfC &= ~BIT8;
	else  // When reading over one sector, enable multiple read function
		sMcard->McCfC |= BIT8;
	//mpDebugPrint("McCfC2: %x", sMcard->McCfC);


	#ifdef DEMO_PID
	//*****************************************************
		//**************  For IDU testing *********************
		register IDU *idu = (IDU *)IDU_BASE;
		char waitCnt;
		waitCnt = 64;
		if(idu->IduSts & BIT0)
			mpDebugPrint("ID FIFO Underflow!!!!!");
		
		while(waitCnt && (idu->IduSts & BIT0))
			waitCnt--;
		
		if(idu->IduSts & BIT0)
			mpDebugPrint("count is too short!!!!!");
		//*****************************************************
	#endif

 	sChannel->Control = 0x0;
	sChannel->StartA = dwBufferAdress;
	sChannel->EndA = dwBufferAdress + wSize - 1;
	sMcard->McDmarl = MCARD_DMA_LIMIT_ENABLE + (wSize >> 2);
	sMcard->McardC = ((sMcard->McardC & 0xffffffef) | MCARD_DMA_DIR_CM | MCARD_FIFO_ENABLE | MCARD_CF_ENABLE);

	sChannel->Control = MCARD_DMA_ENABLE;

	/*
	 *  Because we re-enable CF module, card detection bit will change into zero for a while.
	 *  So we put a delay here
	 */
	//IODelay(30);	// Delay 30 * 30 * 6ns at CPU clock 120 MHZ case

	tmr = GetSysTime();
	do
	{
		if (Polling_CF_Status()){
				mpDebugPrint("		polling cf status fail...");
				return FAIL;
		}
		if (sMcard->McCfIc & IC_CFTCE)
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec
	
	if (!(sMcard->McCfIc & IC_CFTCE))
	{
		mpDebugPrint("-E- FIFO transaction count FAIL (status: %x)", sMcard->McCfIc);
		return IC_SFTCE_TIMEOUT;
	}

	tmr = GetSysTime();
	
	do
	{
		if (Polling_CF_Status()){
			mpDebugPrint("		polling cf status fail...");
			return FAIL;
		}
		if (!(sChannel->Control & MCARD_DMA_ENABLE))
			break;
	}
	while (SystemGetElapsedTime(tmr) < 1000);	// 1sec

	if (sChannel->Control & MCARD_DMA_ENABLE)
	{
		mpDebugPrint("-E- DMA end FAIL (status: %x)", sChannel->Control);
		return DMA_TIMEOUT;
	}

	return PASS;
}

void SetCommand(BYTE bCommand)
{
	register CFATA *sCfata;

	sCfata = (CFATA *) (MC_CFCOM_BASE);
	sCfata->StatusCommand = bCommand;
}

void SetParameter(DWORD dwSectorCount, DWORD dwLogAddr)
{
	register CFATA *sCfata;

	sCfata = (CFATA *) (MC_CFCOM_BASE);
	sCfata->Lba0 = dwLogAddr & 0xff;
	sCfata->Lba1 = (dwLogAddr >> 8) & 0xff;
	sCfata->Lba2 = (dwLogAddr >> 16) & 0xff;
	sCfata->Lba3 = ((dwLogAddr >> 24) & 0xff) | DRIVE_PROPERTY;
	sCfata->SectorCount = dwSectorCount;
}

static DWORD McRtm = MEMORY_READ_TIMMING, McWtm = MEMORY_WRITE_TIMMING;
void CalculatePulseTiming(BYTE speedLevel)
{
	DWORD T = 1000000 / GetMcardClock();	// Time unit : ns

	switch(speedLevel)
	{
	case TIMING_250ns:
		#if 0      // High performance version from Logan
		McWtm = (((30+T-1)/T) << 16) | ((( 125+T-1) / T) << 8) | 1;
		McRtm = (((20+T-1)/T) << 16) | (( (124 / T) > 5 ? 5 : (124/T)) << 8) | 1;

		#elif 1    // 250 ns mode, More stable version(Please refence CF specification rev.4.1 Table 18 and Table 19)
		McWtm = (1 << 24) | (((35 + T - 1) / T) << 16) | (( ((150 + T - 1)/ T)) << 8) | 2;
		McRtm = (1 << 24) | (((24 + T - 1) / T) << 16) | (( ((90 + T - 1)/ T)) << 8) | 1;
		#endif
		break;

	case TIMING_80ns:
		McWtm = (1 << 24) | (((15 + T - 1) / T) << 16) | (( ((60 + T - 1)/ T)) << 8) | 2;
		McRtm = (((14 + T - 1) / T) << 16) | (( ((25 + T - 1)/ T)) << 8);	
		//McRtm = 0x00020400;	// The Fastest handmade speed value
		break;

	case TIMING_SLOWEST:    // This timing setting looks fine for all CF cards in our company
		McRtm = MEMORY_READ_SLOWTIMMING;
		McWtm = MEMORY_WRITE_SLOWTIMMING;
		break;
	}

	mpDebugPrint("CF pluse register : R%08x, W%08x", McRtm, McWtm);
}


// Check if current CF pulse timing is the same as our safe CF pulse timing
SBYTE CFSlowPulseTimingCheck()
{
	if((McRtm != MEMORY_READ_SLOWTIMMING) && (McWtm != MEMORY_WRITE_SLOWTIMMING))
		return PASS;
	else
		return FAIL;
}

void McardCFActive(BYTE bMode)
{
	register GPIO *sGpio;
	register MCARD *sMcard;
	register BYTE *pbAttr;

	sGpio = (GPIO *) (GPIO_BASE);
	sMcard = (MCARD *) (MCARD_BASE);
	pbAttr = (BYTE *) (MC_CFATR_BASE);

	sMcard->McardC = MCARD_DMA_DIR_CM | MCARD_CF_ENABLE;
	sMcard->McWdt &= ~BIT25;	// disable watch dog timer

//	MP_DEBUG1("McardC =%x",sMcard->McardC);
	sMcard->McCfIc = IM_ALL;
	switch (bMode)
	{
	case INITIAL_MEMORY:
		McardSelect(PIN_DEFINE_FOR_CFM);
		sMcard->McRtm = McRtm;
		sMcard->McWtm = McWtm;
		sMcard->McCfC = MEMORY_SETTING;
		//MP_DEBUG("-I- Memory Mode Select");
		break;

	case INITIAL_TRUE_IDE:
		sGpio->Fgpcfg[0] &= 0x0000ffff;
		sGpio->Fgpcfg[0] |= GPIO_DEFINE_0;
		sGpio->Fgpcfg[1] |= GPIO_DEFINE_1;
		sGpio->Fgpcfg[2] |= GPIO_DEFINE_2;
		sMcard->McCfC = TRUE_IDE_SETTING;
		sMcard->McRtm = IDE_READ_TIMMING;
		sMcard->McWtm = IDE_WRITE_TIMMING;
		//MP_DEBUG("-I- True IDE Mode Select");
		break;

	case INITIAL_IO:

		sGpio->Fgpcfg[0] |= GPIO_DEFINE_0;

		//////////////////////////////////////////////////////////////
		//simulate A9 
		//gpio->Fgpcfg[1] = gpio->Fgpcfg[1] | GPIO_DEFINE_1;
		sGpio->Fgpcfg[1] |= 0x0000fdff;
		sGpio->Fgpdat[1] |= 0x02000200;
		//////////////////////////////////////////////////////////////

		sGpio->Fgpcfg[2] |= GPIO_DEFINE_2;
		sMcard->McCfC = MEMORY_SETTING;
		sMcard->McRtm = MEMORY_READ_TIMMING;
		sMcard->McWtm = MEMORY_WRITE_TIMMING;
		*pbAttr = 0x1;
		sGpio->Fgpcfg[0] |= GPIO_DEFINE_0;
		sGpio->Fgpcfg[1] |= GPIO_DEFINE_1;
		sGpio->Fgpcfg[2] |= GPIO_DEFINE_2;
		sMcard->McCfC = IO_SETTING;
		sMcard->McRtm = I_O_READ_TIMMING;
		sMcard->McWtm = I_O_WRITE_TIMMING;
		//MP_DEBUG("-I- IO Mode Select");
		break;
	}
	WaitReady();
}

void McardCFInactive()
{
	register MCARD *sMcard;

	sMcard = (MCARD *) (MCARD_BASE);
	sMcard->McardC = 0;
	McardDeselect(PIN_DEFINE_FOR_CFM);
}

void CFCardReset()
{
	MCARD *sMcard = (MCARD *) (MCARD_BASE);

	sMcard->McardC = MCARD_DMA_DIR_CM | MCARD_CF_ENABLE;
	sMcard->McWdt &= ~BIT25;	// disable watch dog timer
	sMcard->McCfIc = IM_ALL;
	McardSelect(PIN_DEFINE_FOR_CFM);
	sMcard->McRtm = McRtm;
	sMcard->McWtm = McWtm;
	
	sMcard->McCfC = MEMORY_SETTING | RESET_ENABLE;
	McardIODelay(0x500);
	sMcard->McCfC &= ~RESET_ENABLE;
	Mcard_TaskYield(800);
}

#endif

