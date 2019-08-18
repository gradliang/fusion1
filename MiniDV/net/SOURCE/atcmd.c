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
* 
* Filename		: atcmd.c
* Programmer(s)	: billwang
* Created Date	: 2010/05/20 
* Description:  1. AT Command base on USB OTG CDC Class
*                   2. 3.5G Modem
******************************************************************************** 
*/

/*
 * :
 *
 *
 */

#define LOCAL_DEBUG_ENABLE 1
#include "linux/list.h"
#include <linux/kernel.h>
#include <linux/usb.h>

#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "atcmd_usb.h"

#include "..\..\Xpg\include\xpg.h"
#include "..\..\..\..\std_dpf\main\include\xpgGPRSFunc.h"


#if HAVE_USB_MODEM > 0

static format_flag_ format_flag;

extern BYTE g_bXpgStatus;
extern cops_format_ cops_format;

/***********************************************************/
/***                  AT Command Data In/Out/INT  Flow                             ***/ 
/***********************************************************/
static BOOL if_clck;

int AtCmd_Request(char *cmd, char *pattern, char *result, int length, int timeout);

static void StrConverse(BYTE *src, BYTE *dest, int len)
{
	int 	i;
	
	for(i=0; i<len; i++)
		*(dest++) = *(src+len-1-i);	
}

void AtCmdStartUp(void)
{

	if_clck = FALSE;

	Attention();
}

int AtCmdSend(BYTE *cmdStr, int len)
{
	AtCmd_Request(cmdStr, NULL, NULL, 0, 60 /* sec */ );
}

stripCOPS(BYTE *InData, int length)
{
	int i;
	BYTE format;
	BYTE str[32];

	for(i=0; i<length; i++){
		if(*(InData+i) == 0x20){
			length -= (i+1);
			memmove(InData, (InData+i+1), length);
			break;
		}
	}
	
	if((*InData >= '0') && (*InData <= '4')){			//strip cops mode
		length -= 2;
		memmove(InData, (InData+2), length);
	}

	if((*InData >= '0') && (*InData <='2')){
		format = *InData;
		length -= 3;
		memmove(InData, (InData+3), length);
		
		i=0;
		while(*(InData+i) != 0x22){
			str[i] = *(InData+i);
			i++;
		}

		length -= (i+2);
		memmove(InData, (InData+i+2), length);
		
		switch(format)
		{
			case '0':
				break;

			case '1':
				break;

			case '2':
				break;
		}	

//		MP_DEBUG("mode= %X, format = %x, oper = %s, AcT = %x\r\n", modem_feature->cops_mode, format, str, modem_feature->cops_act );
		
	}
		
}


void CopsFmtRenew(void)
{
	if(cops_format == LONG_AN_STRING){
	}
	else if(cops_format == SHORT_AN_STRING){
		if(format_flag.short_1 == FALSE && format_flag.numeric_2 == FALSE)
			cops_format = NUMERIC_ID;
	}
	else if(cops_format == NUMERIC_ID){
		if(format_flag.short_1 == FALSE && format_flag.numeric_2 == TRUE)
			cops_format = SHORT_AN_STRING;
	}	
}


#endif

// vim: :noexpandtab:
