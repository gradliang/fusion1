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
* Filename		: usbotg_host.c
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)
* Created Date	: 2007/07/31 
* Description:  This program is for USB Host lib function call
******************************************************************************** 
*/
/*
 * Some common USB routines for WLAN and LAN drivers.
 */

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
/*
// Include section 
*/
#include <stdbool.h>
#include "global612.h"
#include "mpTrace.h"
#include "usbotg_host.h"
#include "usbotg_host_sm.h"
#include "usbotg_host_setup.h"
#include "usbotg_ctrl.h"
#include "taskid.h"
#include "os.h"
#include "devio.h"
#include "ui.h"
#include "corelib.h"

#if (NETWARE_ENABLE == ENABLE)
#include "usbotg_wifi.h"
#endif    

#if SC_USBHOST

BYTE Wlan_flib_Host20_Send_qTD_Done ( qHD_Structure *spTempqHD,WHICH_OTG eWhichOtg);
void flib_Host20_CheckingForResult_QHD(qHD_Structure *spTempqHD,WHICH_OTG eWhichOtg);
DWORD flib_Host20_GetStructure(BYTE Type, WHICH_OTG eWhichOtg);
void flib_Host20_ReleaseStructure(BYTE Type,DWORD pwAddress,WHICH_OTG eWhichOtg);


void Wlan_flib_Host20_Send_qTD_Active (  struct ehci_qtd *spHeadqTD,
                                    struct ehci_qhd *spTempqHD,
                                    WHICH_OTG eWhichOtg)
{
	qTD_Structure           *spNewDumyqTD; 
	struct ehci_qtd  *spOldDumyqTD; 
	qTD_Structure  *spLastqTD;
    DWORD j;
    bool found;
	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	
   	//mpDebugPrint("Wlan_flib_Host20_Send_qTD_Active");
		
    MP_ASSERT(sizeof(qHD_Structure) <= 64);
	//<1>.Copy Head-qTD to OldDumyqTD
	spOldDumyqTD=spTempqHD->dummy;
    MP_ASSERT(spOldDumyqTD);
	memcpy((BYTE*)spOldDumyqTD,(BYTE*)spHeadqTD,Host20_qTD_SIZE);

#ifdef LINUX
	INIT_LIST_HEAD(&spOldDumyqTD->qtd_list);

    list_del (&spHeadqTD->qtd_list);
    list_add (&spOldDumyqTD->qtd_list, qtd_list);
#endif

    spTempqHD->dummy = spHeadqTD;
    MP_ASSERT(spTempqHD->dummy);

#ifdef LINUX
    found = false;
    do
    {
        for (j = 0; j< psUsbhDevDesc->bMaxBulkInEpNumber; j++)
        {
            if (psUsbhDevDesc->hstBulkInqHD[j] == spTempqHD)
            {
                if (psUsbhDevDesc->hstBulkInSendLastqTD[j] == NULL)
                    psUsbhDevDesc->hstBulkInSendLastqTD[j]  = spOldDumyqTD;
                found = true;
                break;
            }
        }
        if (found)
            break;

        for (j = 0; j< psUsbhDevDesc->bMaxBulkOutEpNumber; j++)
        {
            if (psUsbhDevDesc->hstBulkOutqHD[j] == spTempqHD)
            {
                if (psUsbhDevDesc->hstBulkOutSendLastqTD[j] == NULL)
                    psUsbhDevDesc->hstBulkOutSendLastqTD[j] = spOldDumyqTD;
                found = true;
                break;
            }
        }
    } while (0); 

    MP_ASSERT(found);
#else
    if (spTempqHD->head == NULL)
        spTempqHD->head = spOldDumyqTD;
#endif

    
	//<2>.Prepare new dummy qTD      
	spNewDumyqTD=spHeadqTD;
	spNewDumyqTD->bTerminate=1;

	//<3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1
	spLastqTD=spOldDumyqTD;

	while(spLastqTD->bTerminate==0) {
		spLastqTD=(qTD_Structure*)(((DWORD)(spLastqTD->bNextQTDPointer))<<5);
	}

//    if (((uint32_t)spLastqTD & 0xff000000) != 0x80000000)
//            __asm("break 100");

	spLastqTD->bNextQTDPointer=((DWORD)spNewDumyqTD)>>5;
	spLastqTD->bTerminate=0; 

	//Link Alternate qTD pointer
	spLastqTD->bAlternateQTDPointer=((DWORD)spNewDumyqTD)>>5;
	spLastqTD->bAlternateTerminate=0;   

	psUsbhDevDesc->bSendStatusError=0;
    
#ifdef LINUX
    for (j = 0; j < MAX_NUM_OF_ENDPOINT; j++)
    {
        if (psUsbhDevDesc->dwActivedQHD[j] == 0)     /* find a free entry */
        {
            psUsbhDevDesc->dwActivedQHD[j] = (DWORD)spTempqHD;
        }
    }
#endif
    //UartOutText(" S ");
	spOldDumyqTD->qTD.bStatus_Active=1;
//    __asm("break 100");
}

BYTE Wlan_flib_Host20_Send_qTD_Done(qHD_Structure *spTempqHD,WHICH_OTG eWhichOtg)
{
	volatile qTD_Structure           *spNewDumyqTD; 
	volatile qTD_Structure           *spReleaseqTD;    
	volatile qTD_Structure           *spReleaseqTDNext;    
    short i;
    short j;
	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

// force update cache
    SetDataCacheInvalid();
    //UartOutText(" 4 ");

    spReleaseqTD = ((struct ehci_qhd *)spTempqHD)->head;

	//<6>.Checking the Result
	//if (gwLastqTDSendOK==0)
    psUsbhDevDesc->bQHStatus = spTempqHD->bOverlay_Status;
	flib_Host20_CheckingForResult_QHD(spTempqHD,eWhichOtg); 	
    if (psUsbhDevDesc->bQHStatus & HOST20_qTD_STATUS_Halted)
    {
        if ((psUsbhDevDesc->bQHStatus & 0x7F) != HOST20_qTD_STATUS_Halted)
        {
            //__asm("break 100");
            MP_DEBUG("bQHStatus = 0x%x", psUsbhDevDesc->bQHStatus);
            psUsbhDevDesc->bQHStatus = 0;
        }
        else
        {
            psUsbhDevDesc->sSetupPB.dwSetupState = SETUP_DONE_STATE;
        }
    }
    else
    {
        psUsbhDevDesc->bQHStatus = 0;
    }
	//<5>.Release the all the qTD (Not include spNewDumyqTD)
    spNewDumyqTD = ((struct ehci_qhd *)spTempqHD)->dummy;
	do {
        if (spReleaseqTD->bStatus_Active)
        {
            break;
        }
#if USB_WIFI_ENABLE
        struct ehci_qtd *qtd;
        qtd = (struct ehci_qtd *)spReleaseqTD;
        if (qtd->urb)
        {
            usb_urb_complete(qtd->urb, spReleaseqTD);
        }
#endif
		spReleaseqTDNext = (qTD_Structure*)(((DWORD)(spReleaseqTD->bNextQTDPointer))<<5);

        //mpDebugPrint("spReleaseqTD %x",spReleaseqTD);
		flib_Host20_ReleaseStructure(Host20_MEM_TYPE_qTD,(DWORD)spReleaseqTD,eWhichOtg);
		spReleaseqTD = spReleaseqTDNext;
        if (spReleaseqTD == 0)
        {
            MP_DEBUG("flib_Host20_Send_qTD_Done:no TD released!!");
            MP_DEBUG("spTempqHD = 0x%x", spTempqHD);
            __asm("break 100");
            break;
        }
	} while(((DWORD)spReleaseqTD)!=((DWORD)spNewDumyqTD));

    if (((DWORD)spReleaseqTD) == ((DWORD)spNewDumyqTD))
    {
        if (((DWORD)((struct ehci_qhd *)spTempqHD)->dummy) == ((DWORD)spNewDumyqTD))
        {
            ((struct ehci_qhd *)spTempqHD)->head = NULL;
            return 0;
        }
    }

    ((struct ehci_qhd *)spTempqHD)->head = spReleaseqTD;
    return 0;
}

//====================================================================
// * Function Name: flib_Host20_Issue_Bulk                          
// * Description: Input data must be 4K-Alignment 
//               <1>.MaxSize=20 K
//               <2>.Support Only 1-TD
// * Input: 
// * OutPut: 
//====================================================================
qTD_Structure *  Wlan_flib_Host20_Issue_Bulk_Active_Multi_TD (  BYTE    bArrayListNum,
                                                           WORD    hwSize,
                                                           DWORD   *pwBufferArray,
                                                           DWORD   wOffset,
                                                           BYTE    bDirection,
                                                           BOOL    fActive,
                                                           WHICH_OTG eWhichOtg)
{
    qHD_Structure *spBulkqHD;
    qTD_Structure *spCurrentTempqTD;
    static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;
    static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;
	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	
	//UartOutText(" M ");
	//mpDebugPrint("Wlan_flib_Host20_Issue_Bulk_Active_Multi_TD hwSize %d",hwSize);
    // Fill TD
    spCurrentTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD,eWhichOtg); //The qTD will be release in the function "Send"
    spCurrentTempqTD->bTotalBytes=hwSize; 
    spCurrentTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); 
    spCurrentTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++; 
    spCurrentTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
    spCurrentTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
    spCurrentTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
    ((struct ehci_qtd *)spCurrentTempqTD)->urb  = psUsbhDevDesc->urb;

    // Analysis the Direction 
    if (bDirection) 
    {
        spCurrentTempqTD->bPID = HOST20_qTD_PID_IN;
        spBulkqHD              = psUsbhDevDesc->hstBulkInqHD[bArrayListNum];
    }
    else
    {
        spCurrentTempqTD->bPID = HOST20_qTD_PID_OUT;
        spBulkqHD              = psUsbhDevDesc->hstBulkOutqHD[bArrayListNum];
    }

    if (fActive)
    { // Send TD
        st_spPreviousTempqTD->bNextQTDPointer       = (((DWORD)spCurrentTempqTD)>>5);
		//mpDebugPrint("st_spPreviousTempqTD->bNextQTDPointer %x",st_spPreviousTempqTD->bNextQTDPointer);
        st_spPreviousTempqTD->bAlternateQTDPointer  = (((DWORD)spCurrentTempqTD)>>5);             
        spCurrentTempqTD->bTerminate                = 1; // now, spCurrentTempqTD is the last TD
        spCurrentTempqTD->bAlternateTerminate       = 1;             
        spCurrentTempqTD->bStatus_Active            = 1;
        spCurrentTempqTD->bInterruptOnComplete      = 1;             
        Wlan_flib_Host20_Send_qTD_Active(st_spFirstTempqTD, (struct ehci_qhd *)spBulkqHD, eWhichOtg);
        st_spPreviousTempqTD = OTGH_NULL;
        st_spFirstTempqTD    = OTGH_NULL;

    }
    else
    { // link TD
        spCurrentTempqTD->bTerminate            = 0;
        spCurrentTempqTD->bAlternateTerminate   = 0;             
        spCurrentTempqTD->bInterruptOnComplete  = 1;             
        if (st_spFirstTempqTD == OTGH_NULL)
        {
            st_spFirstTempqTD = spCurrentTempqTD;
        }
        else
        {
            st_spPreviousTempqTD->bNextQTDPointer       = (((DWORD)spCurrentTempqTD)>>5);
			//mpDebugPrint("st_spPreviousTempqTD->bNextQTDPointer %x",st_spPreviousTempqTD->bNextQTDPointer);
            st_spPreviousTempqTD->bAlternateQTDPointer  = (((DWORD)spCurrentTempqTD)>>5);             
            spCurrentTempqTD->bStatus_Active            = 1;
        }
        
        st_spPreviousTempqTD = spCurrentTempqTD;
    }

    return spCurrentTempqTD;
}


//====================================================================
// * Function Name: flib_Host20_Issue_Bulk                          
// * Description: Input data must be 4K-Alignment 
//               <1>.MaxSize=20 K
//               <2>.Support Only 1-TD
// * Input: 
// * OutPut: 
//====================================================================
qTD_Structure *  Wlan_flib_Host20_Issue_Bulk_Active (   BYTE    bArrayListNum,
                                                   WORD    hwSize,
                                                   DWORD   *pwBufferArray,
                                                   DWORD   wOffset,
                                                   BYTE    bDirection,
                                                   WHICH_OTG eWhichOtg)
{
    qHD_Structure *spBulkqHD;
    qTD_Structure *spTempqTD;  
	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    //<1>.Fill TD
    spTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD,eWhichOtg); //The qTD will be release in the function "Send"
    spTempqTD->bTotalBytes=hwSize;  
    spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); 
    spTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
   ((struct ehci_qtd *)spTempqTD)->urb = psUsbhDevDesc->urb;

    //<2>.Analysis the Direction 
    if (bDirection)
    {
        spTempqTD->bPID = HOST20_qTD_PID_IN;
        spBulkqHD       = psUsbhDevDesc->hstBulkInqHD[0];
    }
    else
    {
        spTempqTD->bPID = HOST20_qTD_PID_OUT;
        spBulkqHD       = psUsbhDevDesc->hstBulkOutqHD[0];
		//mpDebugPrint("bulk spTempqTD %x ",spTempqTD);
    }

    //<3>.Send TD
    Wlan_flib_Host20_Send_qTD_Active(spTempqTD, (struct ehci_qhd *)spBulkqHD, eWhichOtg);
    return spTempqTD;
}

void Wlan_SetDummy(WHICH_OTG eWhichOtg)
{
    struct ehci_qhd *eqhd;
	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    register short i;

    for (i=0; i< psUsbhDevDesc->bMaxBulkOutEpNumber; i++)
    {
        if (psUsbhDevDesc->hstBulkOutqHD[i])
        {
            eqhd = (struct ehci_qhd *)psUsbhDevDesc->hstBulkOutqHD[i];
            eqhd->dummy = psUsbhDevDesc->hstBulkOutqHD[i]->bOverlay_NextqTD << 5;
        }
        else
            break;
    }

    for (i=0; i< psUsbhDevDesc->bMaxBulkInEpNumber; i++)
    {
        if (psUsbhDevDesc->hstBulkInqHD[i])
        {
            eqhd = (struct ehci_qhd *)psUsbhDevDesc->hstBulkInqHD[i];
            eqhd->dummy = psUsbhDevDesc->hstBulkInqHD[i]->bOverlay_NextqTD << 5;
        }
        else
            break;
    }
	
    for (i=0; i< psUsbhDevDesc->bMaxInterruptInEpNumber; i++)
    {
        if (psUsbhDevDesc->hstInterruptInqHD[i])
        {
            eqhd = (struct ehci_qhd *)psUsbhDevDesc->hstInterruptInqHD[i];
            eqhd->dummy = psUsbhDevDesc->hstInterruptInqHD[i]->bOverlay_NextqTD << 5;
        }
        else
            break;
    }
	
    for (i=0; i< psUsbhDevDesc->bMaxInterruptOutEpNumber; i++)
    {
        if (psUsbhDevDesc->hstInterruptOutqHD[i])
        {
            eqhd = (struct ehci_qhd *)psUsbhDevDesc->hstInterruptOutqHD[i];
            eqhd->dummy = psUsbhDevDesc->hstInterruptOutqHD[i]->bOverlay_NextqTD << 5;
        }
        else
            break;
    }


}

int Wlan_UsbOtgHostBulkIoc(qHD_Structure *qhd)
{
    //UartOutText(" W ");
    struct ehci_qhd *eqhd = (struct ehci_qhd *)qhd;
    if (eqhd->head && eqhd->head->bStatus_Active == 0)
        return TRUE;
    else
        return FALSE;
}

//====================================================================
// * Function Name: Wlan_flib_Host20_Issue_Interrupt_Active                          
// * Description: 
// * Input: 
// * OutPut: 
//====================================================================
BYTE  Wlan_flib_Host20_Issue_Interrupt_Active (  BYTE    bArrayListNum,
                                            WORD    hwSize,
                                            DWORD   *pwBufferArray,
                                            DWORD   wOffset,
                                            BYTE    bDirection,
                                            WHICH_OTG eWhichOtg)
{
    qHD_Structure *spIntqHD;
    qTD_Structure *spTempqTD;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
   
   //<1>.Fill TD
   // mpDebugPrint("%s", __FUNCTION__);
   spTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg); //The qTD will be release in the function "Send"
   spTempqTD->bTotalBytes=hwSize;           
   spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); // john, allow non-alignment 
   spTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++;  
   spTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
   ((struct ehci_qtd *)spTempqTD)->urb = pUsbhDevDes->urb;

    //<2>.Analysis the Direction 
    if (bDirection)
    {
        spTempqTD->bPID = HOST20_qTD_PID_IN;
        spIntqHD       = pUsbhDevDes->hstInterruptInqHD[bArrayListNum];
    }
    else
    {
        spTempqTD->bPID = HOST20_qTD_PID_OUT;
        spIntqHD       = pUsbhDevDes->hstInterruptOutqHD[bArrayListNum];
		//mpDebugPrint("interrupt spTempqTD %x",spTempqTD);

    }   
      
   //<3>.Send TD
   Wlan_flib_Host20_Send_qTD_Active(spTempqTD,(struct ehci_qhd *)spIntqHD, eWhichOtg);
   //flib_Host20_Send_qTD_Active(spTempqTD,spIntqHD, eWhichOtg);
}

#endif //SC_USBOTG_HOST
