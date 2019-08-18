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
* Filename		: usb_at.c
* Programmer(s)	: Calvin
* Created Date	: 2008/02/18 
* Description:  1. AT Command base on USB OTG CDC Class
*                   2. 3.5G Modem
******************************************************************************** 
*/

#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "usbotg_host_cdc.h"
#include "usbotg_at.h"

#if USBOTG_HOST_CDC


/***********************************************************/
/***                  AT Command Data In/Out/INT  Flow                             ***/ 
/***********************************************************/

/* Fake Data to pre-test */
BYTE FakeAtDataOut[4] = {0x0A, 0x0D, 0x54, 0x41}; // AT.. (AT Command)
BYTE FakeAtDataIn[10] = {0x00}; 
BYTE FakeAtIntIn[10] = {0x00}; 
void AtCmdStartUp(void)
{
    CdcDataOut(FakeAtDataOut, 0x04, WIFI_USB_PORT);
}

void AtCmdDataOutIoc (void)
{
    MP_DEBUG("AtCmdDataOutIoc");
    
    CdcDataIn(FakeAtDataIn, 1024, WIFI_USB_PORT);
}

BYTE bFlow = 0; /* Fake Data to pre-test */
void AtCmdDataInIoc (void)
{
    BYTE bCnt = 0;
    
    
    MP_DEBUG("AtCmdDataInIoc");

    for(bCnt = 0; bCnt <= 4; bCnt += 4)
        MP_DEBUG("%x", byte_swap_of_dword(*(DWORD*)(FakeAtDataIn+bCnt)));

    if(bFlow == 0)
    {
        bFlow = 1;
        CdcDataIn(FakeAtDataIn, 1024, WIFI_USB_PORT);
    }
    else
    {
        CdcInterruptIn(FakeAtIntIn, WIFI_USB_PORT);
    }
}

void AtCmdInterruptIoc(void)
{
    BYTE bCnt = 0;
    
    MP_DEBUG("AtCmdInterruptIoc");
    
    for(bCnt = 0; bCnt <= 4; bCnt += 4)
        MP_DEBUG("%x", byte_swap_of_dword(*(DWORD*)(FakeAtIntIn+bCnt)));
}

/***********************************************************/
/***                  AT Command Data In/Out/INT  Flow End                       ***/
/***********************************************************/


SDWORD AtCmdTaskWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent)
{
	SDWORD ret;
	if (dwNextEvent)
	{
		*pdwEvent = dwNextEvent;
		ret = OS_STATUS_OK;
	}
	else
	{
		ret = EventWait(AT_CMD_EVENT, 0x7fffffff, OS_EVENT_OR, pdwEvent);
	}
	return ret;
}

void AtCmdTask(void)
{
    DWORD dwEvent, dwNextEvent;

    dwNextEvent = 0;

    while (1)
    {
        if (AtCmdTaskWaitEvent(&dwEvent, dwNextEvent) == OS_STATUS_OK)
        {
            if (dwEvent & AT_CMD_PLUG_IN)
            {
                AtCmdStartUp();
            } 
            
            if (dwEvent & AT_CMD_PLUG_OUT)
            {
                MP_ALERT("%s Terminate!", __FUNCTION__);
                EventDestroy(AT_CMD_EVENT);
                TaskTerminate(AT_CMD_TASK);
            }            

            if (dwEvent & AT_CMD_DATA_OUT_IOC)
            {
                AtCmdDataOutIoc();
            }

            if (dwEvent & AT_CMD_DATA_IN_IOC)
            {
                AtCmdDataInIoc();
            }    

            if (dwEvent & AT_CMD_INT_IN_IOC)
            {
                AtCmdInterruptIoc();
            }    
        }
    }
}

SDWORD AtCmdTaskInit(void)
{
    SDWORD sdwRetVal = 0;
    
    sdwRetVal = EventCreate(AT_CMD_EVENT, (OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR), 0);
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- %s AT_CMD_EVENT create fail", __FUNCTION__);
        return sdwRetVal;
    }
    
    sdwRetVal = TaskCreate(AT_CMD_TASK, AtCmdTask, CONTROL_PRIORITY, 0x1000);    
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- %s AT_CMD_TASK create fail", __FUNCTION__);
        return sdwRetVal;
    }
    
    sdwRetVal = TaskStartup(AT_CMD_TASK);
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- %s AT_CMD_TASK startup fail", __FUNCTION__);
        return sdwRetVal;
    }

}

#endif //USBOTG_HOST_CDC

