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
* Filename      : ata.c
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
#if HD_ENABLE
#include "mpTrace.h"
#include "Mcard.h"
#include "uti.h"
#include "ata.h"

/*
// Constant declarations
*/
#define ATA_TIMEOUT             0x400000

// define the HD command code
#define ATA_READ_SECTOR			0x2000
#define ATA_READ_SECTOR_EX		0x2400
#define ATA_WRITE_SECTOR		0x3000
#define ATA_WRITE_SECTOR_EXT		0x3400
#define ATA_READ_VERIFY			0x4000
#define ATA_READ_MULTIPLE		0xc400
#define ATA_WRITE_MULTIPLE		0xc500
#define ATA_SET_MULTIPLE		0xc600
#define ATA_IDENTIFY_DRIVE		0xec00
#define ATA_FEATURE_SET         0xef00
#define ATA_READ_DMA            0xc800
#define ATA_WRITE_DMA           0xca00
#define ATA_WRITE_DMA_EXT           0x3500
#define ATA_SETMAX_ADDRESS_EX	0x3700
#define ATA_READ_DMA_EX	0x2500

// define ATA tramsfer mode
#define ATA_PIO_MODE            0x00
#define ATA_MULTI_DMA_MODE      0x20
#define ATA_ULTRA_DMA_MODE      0x40

// ATA register definition
// field define for ATA DMA control register
#define ATA_DMA_IN              0x00000000
#define ATA_DMA_OUT             0x00020000

#define ATA_MULTI_DMA           0x00010000
#define ATA_ULTRA_DMA           0x00050000
#define ATA_NONE_DMA            0x00000000


#define ATA_PIO_TIMING          	0x00298321
#define ATA_MDMA0_TIMING 		0x00000d23
#define ATA_MDMA1_TIMING 		0x00000952
#define ATA_MDMA2_TIMING 		0x00000101
#define ATA_UDMA0_TIMING 		0x0000422b
#define ATA_UDMA1_TIMING 		0x000021ab
#define ATA_UDMA2_TIMING 		0x0000192b
#define ATA_UDMA3_TIMING 		0x000010ab
#define ATA_UDMA4_TIMING 		0x000008ab


#define ATA_DELAY  0

#define HD_ICAUSE           BIT2

///
///@defgroup ATA HardDisk
///@ingroup CONSTANT
///@{

///@}
/*
// Structure declarations
*/
struct ST_ATAINFO_TAG
{
	DWORD dwAtaDmaMode;
	WORD wDeviceNum;
	WORD wMultiSector;
	DWORD dwCapacity;
};

/*
// Type declarations
*/
typedef struct ST_ATAINFO_TAG ST_ATAINFO;

/*
// Variable declarations
*/
static ST_ATAINFO sInfo;
static BYTE bDescriptor[] = "HD";
#pragma alignvar(4)
/*
// Static function prototype
*/
static void CommandProcess(void *pMcardDev);
static void WaitDelay(WORD wCount);
static void Select(void);
static void DeSelect(void);
static SWORD DmaWaitDataRdy(void);
static SWORD PioWaitDataRdy(void);
static SWORD WaitReady(void);
static DWORD SetParameters(DWORD dwLba, DWORD dwCount);
static void SetCommand(WORD wCommand);
static SWORD SetTransMode(BYTE bMode);
static SWORD PIORead(DWORD dwBuffer, WORD wSectorCnt);
static SWORD PIOWrite(DWORD dwBuffer, WORD wSectorCnt);
static SWORD DMARead(DWORD dwBuffer, WORD wSectorCnt);
static SWORD DMAWrite(DWORD dwBuffer, WORD wSectorCnt);
static SWORD DataRead(DWORD dwBuffer, DWORD dwLba, DWORD dwCount);
static SWORD DataWrite(DWORD dwBuffer, DWORD dwLba, DWORD dwCount);
static BYTE CalBit1(BYTE bValue);
static SWORD Identify(void);
static SWORD PowerOnInit(void);

/*
// Definition of internal functions
*/
void AtaInit(ST_MCARD_DEV * sDev)
{
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_HD;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

/*
// Definition of local functions 
*/
static void CommandProcess(void *pMcardDev)
{
	register UART *sUart;
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	mail->swStatus = FAIL;

	Select();
	switch (mail->wCmd)
	{
	case INIT_CARD_CMD:
		
		if (!((ST_MCARD_DEV *) pMcardDev)->Flag.Detected)
		{
			//card in
			if ((mail->swStatus = PowerOnInit()))
			{
					
				((ST_MCARD_DEV *) pMcardDev)->Flag.Present = 0;
				((ST_MCARD_DEV *) pMcardDev)->dwCapacity = sInfo.dwCapacity;
				((ST_MCARD_DEV *) pMcardDev)->wSectorSize = MCARD_SECTOR_SIZE;
			}
			else
			{
				((ST_MCARD_DEV *) pMcardDev)->wRenewCounter++;
				((ST_MCARD_DEV *) pMcardDev)->Flag.Present = 1;
				((ST_MCARD_DEV *) pMcardDev)->dwCapacity = sInfo.dwCapacity;
				((ST_MCARD_DEV *) pMcardDev)->wSectorSize = MCARD_SECTOR_SIZE;
				//EventSet(UI_EVENT, EVENT_CARD_INIT);
				
			}
		}
		else
		{
			//card out
			((ST_MCARD_DEV *) pMcardDev)->Flag.Present = 0;
			((ST_MCARD_DEV *) pMcardDev)->Flag.ReadOnly = 0;
			((ST_MCARD_DEV *) pMcardDev)->Flag.PipeEnable = 0;
			mail->swStatus = 0;
			((ST_MCARD_DEV *) pMcardDev)->dwCapacity = 0;
			((ST_MCARD_DEV *) pMcardDev)->wSectorSize = 0;
		}
		break;
	case REMOVE_CARD_CMD:		//Athena 03.11.2006 seperate card in & out
		//card out
		((ST_MCARD_DEV *) pMcardDev)->Flag.Present = 0;
		((ST_MCARD_DEV *) pMcardDev)->Flag.ReadOnly = 0;
		((ST_MCARD_DEV *) pMcardDev)->Flag.PipeEnable = 0;
		mail->swStatus = 0;
		((ST_MCARD_DEV *) pMcardDev)->dwCapacity = 0;
		((ST_MCARD_DEV *) pMcardDev)->wSectorSize = 0;
		break;
	case SECTOR_READ_CMD:
		mail->swStatus = DataRead(mail->dwBuffer, mail->dwBlockAddr, mail->dwBlockCount);
		break;

	case SECTOR_WRITE_CMD:
		mail->swStatus = DataWrite(mail->dwBuffer, mail->dwBlockAddr, mail->dwBlockCount);
		break;

	case RAW_FORMAT_CMD:
		
	default:
		MP_DEBUG("-E- ATA INVALID CMD");
		break;
	}
	DeSelect();
}

static void WaitDelay(WORD wCount)
{
	WORD i, j;

	for (i = 0; i < wCount; i++)
	{
		for (j = 0; j < 0x80; j++);
	}
}

static void Select(void)
{
	register MCARD *sMcard;
	register GPIO *sGpio;
	register UART *sUart;
	register ATA *sAta;

	sAta = (ATA *) (ATA_BASE);
	McardSetClock(1);//set ata clock as PLL1/4
	sGpio = (GPIO *) (GPIO_BASE);
	///////////////////////////////
	sGpio->Fgpcfg[0] = 0x0000ffff;
	//??sGpio->Fgpcfg[1] &= ~0x06070000;	// config Fgpio[16] as outpt for FPGA bug (control by software)
	//??sGpio->Fgpcfg[1] |= 0x0000607;	// config Fgpio[16] as outpt for FPGA bug (control by software)
	sGpio->Fgpcfg[2] &= ~0x00260000;
	sGpio->Fgpcfg[2] |= 0x00000026;
	sGpio->Fgpcfg[3] &= ~0x00070000;
	sGpio->Fgpcfg[3] |= 0x00000007;

	sGpio->Fgpcfg[0] = 0x0000FFFF;
	sGpio->Fgpcfg[1] = 0x00000607;
	sGpio->Fgpcfg[2] = 0x00004026;
	sGpio->Fgpcfg[3] = 0x00000187;
		

	sMcard = (MCARD *) MCARD_BASE;
	sMcard->McardC = 0x00000100;	// enable ATA module   

	sAta->AtaSta = 0x00000001;
	sAta->AtaSta = 0x00000000;
	sAta->AtaPioTim = 0x00000108;	//12109
	sAta->AtaMdmaTim = 0x00000088;

	sAta->AtaDevice = sInfo.wDeviceNum;
	WaitDelay(5);
}

static void DeSelect(void)
{
	MCARD *sMcard;

	sMcard = (MCARD *) MCARD_BASE;
	sMcard->McardC = 0;
}

static SWORD DmaWaitDataRdy(void)
{
	register ATA *sAta;
	DWORD dwTimeOut;

	sAta = (ATA *) (ATA_BASE);
	dwTimeOut = ATA_TIMEOUT;
	while (dwTimeOut)
	{
		if ((sAta->AtaSta & 0x00000020))	// wait DMARQ
		{
			return PASS;
		}
		WaitDelay(1);
		dwTimeOut--;
	}
	return FAIL;
}

static SWORD PioWaitDataRdy(void)
{
	register ATA *sAta;
	DWORD dwTimeOut;

	sAta = (ATA *) (ATA_BASE);

	dwTimeOut = ATA_TIMEOUT;

	while (dwTimeOut)
	{
		if ((sAta->AtaStatusCommand & 0x8800) == 0x0800)	// wait BSY=0 and DRQ=1
		{
			//MP_DEBUG("PioWaitDataRdy success!!!!!!!!!!!!");
			return PASS;
		}
		WaitDelay(1);
		dwTimeOut--;
	}
	return FAIL;
}


static SWORD WaitReady(void)
{
	register ATA *sAta;
	DWORD dwTimeOut;

	sAta = (ATA *) (ATA_BASE);
	dwTimeOut = ATA_TIMEOUT;
	while (dwTimeOut)
	{
		if (!(sAta->AtaStatusCommand & 0x8000))	// wait BSY=0 and DRQ=0
		{
			return PASS;
		}
		WaitDelay(1);
		dwTimeOut--;
	}
	return FAIL;
}


// Set ATA command register, and return the number of sector read
#if 1
static DWORD SetParameters(DWORD dwLba, DWORD dwCount)
{
	register ATA *sAta;
	DWORD dwTemp;
	BYTE i,j;
	j=1;
	sAta = (ATA *) (ATA_BASE);
	if(dwLba & 0xf0000000)
		j=2;
	for(i=0;i<j;i++)
	{
		if((i==0) && (j==1))
			dwTemp = dwLba;
		else if((i==0) && (j==2))
			dwTemp = (dwLba & 0xff000000)>>24;
		else if((i==1) && (j==2))
			dwTemp = dwLba & 0x00ffffff;	
		
		sAta->AtaLbaL = (dwTemp & 0x000000ff) << 8;
		WaitDelay(1);
		sAta->AtaLbaM = dwTemp & 0x0000ff00;
		WaitDelay(1);
		dwTemp >>= 8;
		sAta->AtaLbaH = dwTemp & 0x0000ff00;
		WaitDelay(1);
		if(!((i==0) && (j==2)))
		{
			dwTemp >>= 8;
			sAta->AtaDevice = (dwTemp & 0x0000ff00) | (sInfo.wDeviceNum);
		
			WaitDelay(1);
			if (dwCount >= 256)
			{
				sAta->AtaSectorCnt = 0;
				return 256;
			}
			else
			{
				sAta->AtaSectorCnt = (BYTE) dwCount << 8;
				return dwCount;
			}
		}
		if((i==0) &&(j==2))
		{
			if(dwCount>=256)	
				sAta->AtaSectorCnt = (0x01<<8);
			else
			sAta->AtaSectorCnt = 0x00;				
		}
	}
}
#else
static DWORD SetParameters(DWORD dwLba, DWORD dwCount)
{
	register ATA *sAta;
	DWORD dwTemp;

	sAta = (ATA *) (ATA_BASE);

	dwTemp = dwLba;
	sAta->AtaLbaL = (dwTemp & 0x000000ff) << 8;
	WaitDelay(1);
	sAta->AtaLbaM = dwTemp & 0x0000ff00;
	WaitDelay(1);
	dwTemp >>= 8;
	sAta->AtaLbaH = dwTemp & 0x0000ff00;
	WaitDelay(1);
	dwTemp >>= 8;
	sAta->AtaDevice = (dwTemp & 0x0000ff00) | (sInfo.wDeviceNum);

	WaitDelay(1);
	if (dwCount >= 256)
	{
		sAta->AtaSectorCnt = 0;
		return 256;
	}
	else
	{
		sAta->AtaSectorCnt = (BYTE) dwCount << 8;
		return dwCount;
	}
}

#endif
static void SetCommand(WORD wCommand)
{
	register ATA *sAta;

	sAta = (ATA *) (ATA_BASE);
	sAta->AtaStatusCommand = wCommand;
	WaitDelay(1);
}


/*
    ATA set transfer mode

*/
static SWORD SetTransMode(BYTE bMode)
{
	register ATA *sAta;
	DWORD dwTemp;

	sAta = (ATA *) ATA_BASE;

	sAta->AtaDevice = sInfo.wDeviceNum;
	WaitDelay(1);
	sAta->AtaErrorFeature = 0x0300;	// set transfer mode subcommand
	WaitDelay(1);
	sAta->AtaSectorCnt = bMode << 8;
	WaitDelay(1);
	SetCommand(ATA_FEATURE_SET);

	WaitDelay(1);

	do
	{
		dwTemp = sAta->AtaStatusCommand;
		WaitDelay(1);
		if (dwTemp & 0x0100)
		{
			MP_DEBUG("Set Trans Mode fail");
			return FAIL;
		}
	}
	while (dwTemp & 0x8000);
	return PASS;
}


static SWORD PIORead(DWORD dwBuffer, WORD wSectorCnt)
{
	register ATA *sAta;
	WORD wTmpSectorCnt;
	WORD *pwTmpBufferC, *pwTmpBufferE;

	sAta = (ATA *) (ATA_BASE);
	pwTmpBufferC = (WORD *) dwBuffer;
	pwTmpBufferE = pwTmpBufferC + 256;
	MP_DEBUG1("PIO read begin %d!!!!!!!",wSectorCnt);
	//TimerDelay(ATA_DELAY);
	for (wTmpSectorCnt = 0; wTmpSectorCnt < wSectorCnt; wTmpSectorCnt++)
	{
	
		if (PioWaitDataRdy() != PASS)
		{
			return FAIL;
		}

		while (pwTmpBufferC < pwTmpBufferE)
		{
			*pwTmpBufferC = (WORD) sAta->AtaData;
			WaitDelay(1);
			pwTmpBufferC++;
		}
		pwTmpBufferE += 256;
	}
	return PASS;
}

static SWORD PIOWrite(DWORD dwBuffer, WORD wSectorCnt)
{
	register ATA *sAta;
	WORD wTmpSectorCnt;
	WORD *pwTmpBufferC, *pwTmpBufferE;

	sAta = (ATA *) (ATA_BASE);
	pwTmpBufferC = (WORD *) dwBuffer;
	pwTmpBufferE = pwTmpBufferC + 256;
	//TimerDelay(ATA_DELAY);
	for (wTmpSectorCnt = 0; wTmpSectorCnt < wSectorCnt; wTmpSectorCnt++)
	{
		if (PioWaitDataRdy() != PASS)
		{
			return FAIL;
		}

		while (pwTmpBufferC < pwTmpBufferE)
		{
			sAta->AtaData = (DWORD) * pwTmpBufferC;
			WaitDelay(1);
			pwTmpBufferC++;
		}
		pwTmpBufferE += 256;
	}
	return PASS;
}


static SWORD DMARead(DWORD dwBuffer, WORD wSectorCnt)
{
	register CHANNEL *sChannel;
	register ATA *sAta;
	register GPIO *sGpio;
	DWORD dwcount = ATA_TIMEOUT;

	sGpio = (GPIO *) (GPIO_BASE);
	//__asm("break 100");
	if (!wSectorCnt)
	{
		UartOutText("DMA¡@Read 1!!!");
		return  FAIL;
	}
	MP_DEBUG1("DMA READ =%d!!!!!!!!",wSectorCnt);
	sAta = (ATA *) ATA_BASE;
	sChannel = (CHANNEL *) (DMA_MC_BASE);
	sChannel->Control = 0;
	sAta->AtaDmaCtl = 0;

	if (dwBuffer & 3)
	{
		UartOutText("DMA¡@Read 2!!!");
		return FAIL;
	}
	sChannel->StartA = dwBuffer;	// set DMA transfer bank on SDRAM side
	sChannel->EndA = dwBuffer + (wSectorCnt << 9) - 1;
	sAta->AtaDmaCtl = wSectorCnt + sInfo.dwAtaDmaMode;
	sChannel->Control = 0x00000001;	// enable DMA operation
//	MP_DEBUG1("INTRQ1 %08X", sGpio->Gpint[2]);
	WaitDelay(1);
	while(!(sGpio->Gpint[2] & HD_ICAUSE))
	{

//		MP_DEBUG1("INTRQ1 %08X", sGpio->Gpint[2]);
		if(!(dwcount--))
		{
			UartOutText("DMA¡@Read 3!!!");
			return FAIL;
		}
	}
	sGpio->Gpint[2] &=~HD_ICAUSE;
	MP_DEBUG("DMA READ FINISH!!!!!!!!");
	return PASS;
}

static SWORD DMAWrite(DWORD dwBuffer, WORD wSectorCnt)
{
	register CHANNEL *sChannel;
	register ATA *sAta;

	register GPIO *sGpio;
	DWORD dwcount = ATA_TIMEOUT;

	sGpio = (GPIO *) (GPIO_BASE);
	MP_DEBUG("DMA WRITE!!!!!!!!");
	//
	if (!wSectorCnt)
	{
		MP_DEBUG("DMAWrite wSectorCnt = 0");	
		return FAIL;
	}

	sAta = (ATA *) ATA_BASE;
	sChannel = (CHANNEL *) (DMA_MC_BASE);
	sChannel->Control = 0;
	sAta->AtaDmaCtl = 0;

	if (dwBuffer & 3)
	{
		MP_DEBUG("DMAWrite dwBuffer not Aligned");	
		return FAIL;
	}

	sChannel->StartA = dwBuffer;	// set DMA transfer bank on SDRAM side
	sChannel->EndA = dwBuffer + (wSectorCnt << 9) - 1;
	sAta->AtaDmaCtl = 0x00020000 + wSectorCnt + sInfo.dwAtaDmaMode;
	sChannel->Control = 0x00000001;	// enable DMA operation
	WaitDelay(1);
	MP_DEBUG1("INTRQ1 %08X", sGpio->Gpint[2]);
	while(!(sGpio->Gpint[2] & HD_ICAUSE))
	{
		MP_DEBUG1("INTRQ1 %08X", sGpio->Gpint[2]);
		if(!(dwcount--))
		{
			MP_DEBUG("DMAWrite fail");
			return FAIL;
	}
	}
	sGpio->Gpint[2] &=~HD_ICAUSE;
	MP_DEBUG("DMA WRITE FINISH!!!!!!!!");
	return PASS;
	
}

static SWORD DataRead(DWORD dwBuffer, DWORD dwLba, DWORD dwCount)
{
	register ATA *sAta;
	DWORD dwCount0, dwTemp;

	//////////////////////////
	PARTITION *par;
	DWORD buffer;
	
	sAta = (ATA *) ATA_BASE;
	//if (sInfo.dwAtaDmaMode == ATA_NONE_DMA)
	MP_DEBUG2("Data Read Lba= %d,Count=%d",dwLba,dwCount);
	TimerDelay(ATA_DELAY);
	
	//MP_DEBUG("DataRead!!!!!!!!!!!");
	while (dwCount)
	{
		//TimerDelay(ATA_DELAY);
	MP_DEBUG2("Fragment Read Lba= %d,Count=%d",dwLba,dwCount);		
		dwCount0 = SetParameters(dwLba, dwCount);
		//TimerDelay(ATA_DELAY);

		if (sInfo.dwAtaDmaMode == ATA_NONE_DMA)
		{
			if(dwLba & 0xf0000000)
			{
				SetCommand(ATA_READ_SECTOR_EX);			
			}
			else			
			SetCommand(ATA_READ_SECTOR);//dean
			WaitDelay(1);
			if(PIORead(dwBuffer, dwCount0) != PASS)
			{
				MP_DEBUG("PIO Read fail");
				return FAIL;
			}


		}
		else
		{
			if(dwLba & 0xf0000000)
			{
				SetCommand(ATA_READ_DMA_EX);			
			}
			else			
			SetCommand(ATA_READ_DMA);
			WaitDelay(1);			

//			SetCommand(ATA_READ_DMA);
			MP_DEBUG("wait dmarq...");
			if (DmaWaitDataRdy() == FAIL)
			{
				MP_DEBUG("Dma Wait Data Ready fail");			
				return FAIL;
			}
			MP_DEBUG("wait dmarq ok");			
			if(DMARead(dwBuffer, dwCount0) != PASS)
			{
				MP_DEBUG("DMA Read fail");			
				return FAIL;
			}
			//dwTemp = sAta->AtaSta & 0x00000010;
			//while (!(sAta->AtaSta & 0x00000010));	// wait Intreq high
			WaitDelay(3);
			dwTemp = sAta->AtaStatusCommand;	// read out status
			WaitDelay(1);
			sAta->AtaDmaCtl = 0;
		}

		dwLba += dwCount0;
		dwCount -= dwCount0;
		dwBuffer += (dwCount0 << 9);
	}
	return PASS;
}


static SWORD DataWrite(DWORD dwBuffer, DWORD dwLba, DWORD dwCount)
{
	register ATA *sAta;
	DWORD dwCount0, dwTemp;
	DWORD	dwTimeOut = ATA_TIMEOUT;


	MP_DEBUG2("Data Write Lba= %d,Count=%d",dwLba,dwCount);
	sAta = (ATA *) ATA_BASE;
	while (dwCount)
	{
	MP_DEBUG2("Fragment write Lba= %d,Count=%d",dwLba,dwCount);			
		dwCount0 = SetParameters(dwLba, dwCount);

		if (sInfo.dwAtaDmaMode == ATA_NONE_DMA)
		{
			if(dwLba & 0xf0000000)
			{
				SetCommand(ATA_WRITE_SECTOR_EXT);			
			}
			else		
			SetCommand(ATA_WRITE_SECTOR);
			WaitDelay(1);
			PIOWrite(dwBuffer, dwCount0);
		}
		else if (sInfo.dwAtaDmaMode == ATA_MULTI_DMA)
		{
			if(dwLba & 0xf0000000)
			{
				SetCommand(ATA_WRITE_DMA_EXT);			
			}
			else				
			SetCommand(ATA_WRITE_DMA);
			if (DmaWaitDataRdy() == FAIL)
			{
				return FAIL;
			}

			DMAWrite(dwBuffer, dwCount0);
			WaitDelay(1);
			dwTemp = sAta->AtaStatusCommand;	// read out status
			WaitDelay(1);
			sAta->AtaDmaCtl = 0;
		}
		else
		{
			if(dwLba & 0xf0000000)
			{
				SetCommand(ATA_WRITE_DMA_EXT);			
			}
			else				
			SetCommand(ATA_WRITE_DMA);
			if (DmaWaitDataRdy() == FAIL)
			{
				return FAIL;
			}

			if(DMAWrite(dwBuffer, dwCount0) != PASS)
				return FAIL;
			dwTemp = sAta->AtaSta & 0x00000010;
			WaitDelay(1);
			dwTemp = sAta->AtaStatusCommand;	// read out status
			WaitDelay(1);
			sAta->AtaDmaCtl = 0;
		}
		dwLba += dwCount0;
		dwCount -= dwCount0;
		dwBuffer += (dwCount0 << 9);
	}
	return PASS;
}

static BYTE CalBit1(BYTE bValue)
{
	BYTE i, bTemp;

	bTemp = bValue >> 1;
	i = 0;
	while (bTemp)
	{
		i++;
		bTemp = bTemp >> 1;
	}
	return i;
}


static SWORD Identify(void)
{
	register ATA *sAta;
	DWORD dwTemp;

//#pragma alignvar(4)
	WORD IdBuffer[0x100];
	WORD *buffer;

	sAta = (ATA *) (ATA_BASE);
	sInfo.dwAtaDmaMode = ATA_NONE_DMA;

	sAta->AtaDevice = sInfo.wDeviceNum;
	WaitDelay(1);

	// do identify drive command
	SetCommand(ATA_IDENTIFY_DRIVE);
	WaitDelay(1);
	buffer = (WORD *) (((DWORD) & IdBuffer[0]) + 0x20000000);
	if (PIORead((DWORD) (buffer), 1) == FAIL)
	{
		return FAIL;
	}

	// get the maximal number of sector that shall be transfer per interrupt on multiple read or write
	sInfo.wMultiSector = (buffer[47] & 0xff00) >> 8;

	// get the total sector count in this device
	sInfo.dwCapacity = ((buffer[63] << 24) | (buffer[62] << 16) | (buffer[61] << 8) | buffer[60]);

	// get DMA mode 
	dwTemp = (buffer[88] & 0x3f00) >> 8;
	
	if (dwTemp)
	{
	
		dwTemp = CalBit1(dwTemp);
		MP_DEBUG1("This HDD support to UDMA mode %d!!!!!!!!!", dwTemp);
		if (dwTemp >= 4)
		{
			dwTemp = 3;			// support UDMA mode 4
			
			switch(dwTemp)
			{
				case 0:
					sAta->AtaUdmaTim =  ATA_UDMA0_TIMING;
				break;

				case 1:
					sAta->AtaUdmaTim =  ATA_UDMA1_TIMING;
				break;	

				case 2:
					sAta->AtaUdmaTim =  ATA_UDMA2_TIMING;
				break;	

				case 3:
					sAta->AtaUdmaTim =  ATA_UDMA3_TIMING;
				break;	
			}
			
			sAta->AtaUdmaTim =  ATA_UDMA3_TIMING;
			MP_DEBUG("But we only support up to mode 3!!!!!!!");
		}

		dwTemp = dwTemp+ ATA_ULTRA_DMA_MODE;//terst up to Ultra dma mode 3,need to short the cable
		if (SetTransMode((BYTE) dwTemp) == PASS)
		{
			sInfo.dwAtaDmaMode = ATA_ULTRA_DMA;
		}
	}
	else
	{
		dwTemp = (buffer[63] & 0x0700) >> 8;
		if (dwTemp)
		{
			dwTemp = CalBit1(dwTemp);
			if (dwTemp > 2)
			{
				dwTemp = 2;		// support MDMA mode 2
			}
			dwTemp = dwTemp + ATA_MULTI_DMA_MODE;
			if (SetTransMode((BYTE) dwTemp) == PASS)
			{
				sInfo.dwAtaDmaMode = ATA_MULTI_DMA;
			}
		}
		else
		{
			dwTemp = 4 + ATA_PIO_MODE;//PIO mode 4
			if (SetTransMode((BYTE) dwTemp) == PASS)
			{
				sInfo.dwAtaDmaMode = ATA_NONE_DMA;
			}
		}
	}
	
	
	return PASS;
}

#define dwDataStartSector  0x0100
static SWORD PowerOnInit(void)
{
	register ATA *sAta;
	DWORD dwTemp;
	

	sAta = (ATA *) (ATA_BASE);

	// Wait HD ready
	dwTemp = sAta->AtaStatusCommand;
	while (dwTemp & 0x8000)
	{
		WaitDelay(1);
		dwTemp = sAta->AtaStatusCommand;
	}

	// determine which drive should be used
	sAta->AtaDevice = 0xe000;
	MP_DEBUG("power on init  begin!!!!!!!!!!!");

	WaitDelay(1);
	

	
	sAta->AtaLbaL = 0x5a00;

	WaitDelay(1);
	if ((sAta->AtaLbaL & 0xff00) == 0x5a00)
	{
		sInfo.wDeviceNum = 0xe000;
	}
	else
	{
		sAta->AtaDevice = 0xf000;
		WaitDelay(1);
		sAta->AtaLbaL = 0xa500;
		WaitDelay(1);
		if ((sAta->AtaLbaL & 0xff00) == 0xa500)
		{
			sInfo.wDeviceNum = 0xf000;
		}
		else
		{
			return FAIL;
		}
	}
	#if 1
	// perform software reset once
	sAta->AtaAstsDctl = 0x0400;	// set SRST and enable interrupt
	WaitDelay(0x20);			// wait at least 5us
	sAta->AtaAstsDctl = 0x0000;	// Clear SRST bit
	WaitDelay(0x800);			// wait at least 2ms
	while ((sAta->AtaStatusCommand & 0xc000) != 0x4000);	// wait ready
#endif
	// read device capacity, max sector count
	if (Identify() == FAIL)
	{
		return FAIL;
	}


	////////////////////////////////////////////////////
	#if 0//for test
	    BYTE bReadBuffer[0x10000], bWriteBuffer[0x10000];

	    BYTE *pbWriteBuffer, *pbReadBuffer;
	    DWORD i, j ,k, l;
	    GPIO * gpio = (GPIO *)GPIO_BASE;
	
	pbWriteBuffer = (BYTE *)(((DWORD)(&bWriteBuffer) | 0xa0000000));
    	pbReadBuffer = (BYTE *)(((DWORD)(&bReadBuffer) | 0xa0000000));
        //SetTransMode(0x08);  
	     for (i = 0; i < 0x10000; i++)
	    {
	        pbWriteBuffer[i] = i;
	    } 
	     for (i = 0; i < 0x10000; i++)
	    {
	        pbReadBuffer[i] = 0;
	    }      
   
    
   
    for (k = 1; k < 2; k++)
    {
       MP_DEBUG1("-I- %d Number of Sector Test", k);
        for (i = 0; i < 10; i++)
        {
            MP_DEBUG1("-I- Test Count %d", i);
            DataWrite((DWORD)pbWriteBuffer, dwDataStartSector, k);
		//MP_DEBUG("DataWrite finish!!!!!!!!!!");
            DataRead((DWORD)pbReadBuffer, dwDataStartSector, k);
		//MP_DEBUG("Dataread finish!!!!!!!!!!");	
           
            for (j = 0; j < (k << 9); j++)
            {
                if (pbWriteBuffer[j] != pbReadBuffer[j])
                {
                    //MP_DEBUG("PIO fail!!!!!!!!!!!!!");
                }
                pbReadBuffer[j] = 0x0;
            }
            
           
        }
    } 
    MP_DEBUG("PIO test ok");
	BreakPoint();
	#endif
   

	///////////////////////////////////////////////////

	

	// do set multiple command
#if 0
	sAta->AtaDevice = sInfo.wDeviceNum;
	WaitDelay(1);
	//sAta->AtaSectorCnt = sInfo.MultiSector << 8;
	WaitDelay(1);
	sAta->AtaStatusCommand = ATA_SET_MULTIPLE;
	WaitReady();
#endif
	return PASS;
}

#endif

