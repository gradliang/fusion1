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
* Filename      : sample.c
* Programmer(s) : Logan
* Created       : 
* Descriptions  : A skeleton of a device driver
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
#if SAMPLE_ENABLE
#include "mpTrace.h"

#include "Mcard.h"
#include "McardApi.h"
#include "uti.h"
#include "sample.h"

/*
// Constant declarations
*/

/*
// Structure declarations
*/

/*
// Type declarations
*/

/*
// Variable declarations
*/
static BYTE bDescriptor[] = "Sample";

/*
// Macro declarations
*/


/*
// Static function prototype
*/
static void CommandProcess(void *pMcardDev);
static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr);
static SWORD RawFormat(void);

/*
// Definition of internal functions
*/

/*
// Definition of local functions 
*/
static SWORD SampleIdentify()
{
	return PASS;
}

static SWORD FlatRead(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	return PASS;
}

static SWORD FlatWrite(DWORD dwBufferAddress, DWORD dwSectorCount, DWORD dwLogAddr)
{
	return PASS;
}

static SWORD LowLevelFormat(void)
{
	return CARD_NOT_SUPPORT;
}

static McardSampleActive()
{
	// Let IC, Hardware to be active
}

static McardSampleInactive()
{
	// Let IC, Hardware to be inactive
}

static void CommandProcess(void *pMcardDev)
{
	register ST_MCARD_DEV *pDev = ((ST_MCARD_DEV *) pMcardDev);
	ST_MCARD_MAIL *mail = pDev->sMcardRMail;
	
	//mpDebugPrint("sample(%x) Command: %x(%x,%d)", pDev->wMcardType, mail->wCmd, mail->dwBlockAddr, mail->dwBlockCount);
	McardSampleActive();
	switch (mail->wCmd)
	{
		case INIT_CARD_CMD:
			if ((mail->swStatus = SampleIdentify()) == PASS)
			{
				pDev->Flag.Present = 1;
				pDev->dwCapacity = 999;			// calculate available sector numbers
				pDev->Flag.ReadOnly = FALSE;	// check write protection
				pDev->wSectorSize = MCARD_SECTOR_SIZE;
				pDev->wSectorSizeExp = MCARD_SECTOR_SIZE_EXP;
				pDev->wRenewCounter++;
			}
			break;
		case REMOVE_CARD_CMD:
			pDev->Flag.Present = 0;
			pDev->Flag.ReadOnly = TRUE;
			pDev->Flag.PipeEnable = 0;
			mail->swStatus = 0;
			pDev->dwCapacity = 0;
			pDev->wSectorSize = 0;
			pDev->wSectorSizeExp = 0;
			break;
		case SECTOR_READ_CMD:
			if (mail->dwBlockAddr < pDev->dwCapacity)
				mail->swStatus = FlatRead(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr);
			else
				mail->swStatus = FAIL;
			break;

		case SECTOR_WRITE_CMD:
			if (mail->dwBlockAddr < pDev->dwCapacity)
			{
				if (pDev->Flag.ReadOnly != TRUE)
					mail->swStatus = FlatWrite(mail->dwBuffer, mail->dwBlockCount, mail->dwBlockAddr);
			}
			else
				mail->swStatus = FAIL;
			break;
		case RAW_FORMAT_CMD:
			if (pDev->Flag.ReadOnly != TRUE)
			{
				mail->swStatus = LowLevelFormat();
			}
			else
				mail->swStatus = FAIL;
			break;
		default:
			mail->swStatus = FAIL;
			break;
	}
	McardSampleInactive();
}

void SampleInit(ST_MCARD_DEV *sDev)
{
	mpDebugPrint(__FUNCTION__);
	MP_ASSERT(sDev != NULL);
	
	sDev->pbDescriptor = bDescriptor;
	sDev->wMcardType = DEV_SAMPLE;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}

#endif
