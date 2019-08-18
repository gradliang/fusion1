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
* Filename      : usbotg_host_msdc.c
* Programmer(s) : Joe Luo    
* Created		: 2008/04/30 
* Description	: 
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

#include "usbotg_ctrl.h"
#include "usbotg_std.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_msdc.h"
#if USBOTG_HOST_CDC
#include "usbotg_host_cdc.h"
#endif

#include "taskid.h"
#include "os.h"

#if (SC_USBHOST==ENABLE)
/*
// Constant declarations
*/
///	The device's function executes commands without any problem. 
#define PASS                        0
///	Unknown or general error encounter. 
#define FAIL                        -1

#define USBOTG_HOST_TRY_TUR  10    // 100ms
//#define READ_WRITE_DEBUG

//#define CBI_CMD_BYTE_CNT    12

// Inquiry Removable Bit Mask
enum
{
	kInqRemovableMask 						= 0x80,
	kInqANSIVersionMask						= 0x07
};
enum
{
	INQ_PERIPHERAL_CONNECTED					= 0x00,
	INQ_PERIPHERAL_SUPPORTED_BUT_NOT_CONNECTED	= 0x20,
	INQ_PERIPHERAL_NOT_SUPPORTED				= 0x60,
	INQ_PERIPHERAL_QUALIFIER_MASK				= 0xE0
};

// Inquiry Peripheral Device types
enum
{
	INQ_DIRECT_ACCESS_SBC_DEVICE				= 0x00,
	INQ_SEQUENTIAL_ACCESS_SSC_DEVICE			= 0x01,
	INQ_PRINTER_SSC_DEVICE					= 0x02,
	INQ_PROCESSOR_SPC_DEVICE					= 0x03,
	INQ_WRITE_ONCE_SBC_DEVICE					= 0x04,
	INQ_CDROM_MMC_DEVICE						= 0x05,
	INQ_SCANNER_SCSI2_DEVICE					= 0x06,
	INQ_OPTICAL_MEMORY_SBC_DEVICE				= 0x07,
	INQ_MEDIUM_CHANGER_SMC_DEVICE				= 0x08,
	INQ_COMMUNICATIONS_SSC_DEVICE				= 0x09,
	/* 0x0A - 0x0B ASC IT8 Graphic Arts Prepress Devices */
	INQ_STORAGE_ARRAY_CONTROLLER_SCC2_DEVICE	= 0x0C,
	INQ_ENCLOSURE_SERVICES_SES_DEVICE			= 0x0D,
	INQ_SIMPLIFIED_DIRECT_ACCESS_RBC_DEVICE		= 0x0E,
	INQ_OPTICAL_CARD_READER_OCRW_DEVICE			= 0x0F,
	/* 0x10 - 0x1E Reserved Device Types */
	INQ_UNKNOWN_OR_NO_DEVICE_TYPE				= 0x1F,
	
	INQ_PERIPHERAL_DEVICE_TYPE_MASK			= 0x1F
};



/*
// Structure declarations
*/


    
/*
// Variable declarations
*/
//static BYTE bDescriptor[5][5] = {"LUN_0", "LUN_1", "LUN_2", "LUN_3", "PTP"};
//static DWORD   gCbwTag = 0;  // Delete
//static BOOL gIsModeSense6TryAgain = FALSE; // Delete

/*
// Extern Variable declarations
*/
//extern PST_USBH_DEVICE_DESCRIPTOR  gpUsbhDeviceDescriptor;
//extern PST_APP_CLASS_DESCRIPTOR gpAppClassDescriptor;
//extern qHD_Structure     *psHost20_qHD_List_Bulk[]; 
//extern qHD_Structure     *psHost20_qHD_List_Control[];
//extern DWORD gHost20_STRUCTURE_qHD_BASE_ADDRESS;

/*
// Macro declarations
*/

/*
// Static function prototype
*/
static void UsbOtgBulkOnlyDataInOutProcess(BYTE dir_flag, WHICH_OTG eWhichOtg);
static SWORD MsdcBulkOnlyExecuteCommand (BYTE opCode, WHICH_OTG eWhichOtg);
static SWORD MsdcCbiExecuteCommand(BYTE opCode, WHICH_OTG eWhichOtg);
static SWORD MsdcCbiExecuteCommand_interrupt(BYTE opCode, WHICH_OTG eWhichOtg);
static DWORD BulkCommand (WHICH_OTG eWhichOtg);
static DWORD CbiCommand (PUSB_CTRL_REQUEST pAdsc, BYTE *pScsiCmd, WHICH_OTG eWhichOtg);
static BOOL CheckIfUse128MaxSectorCount (WORD wVid, WORD wPid);
static void CbwBuilder( PCBW    pCbw,
                        BYTE    opCode,
                        BYTE    devLun,
                        DWORD*  pData_in_length,
                        DWORD*  pData_out_length,
                        DWORD*  pData_addr,
                        DWORD   dLBA,
                        DWORD   bSectorExp,
                        WHICH_OTG eWhichOtg);
static void CbiCmdBuilder(PUSB_CTRL_REQUEST pAdsc, 
                          BYTE * pScsiCmd,
                          BYTE opCode,
                          BYTE devLun,
                          DWORD * pData_in_length,
                          DWORD * pData_out_length,
                          DWORD * pData_addr,
                          DWORD dLBA,
                          DWORD bSectorExp,
                          WHICH_OTG eWhichOtg);
static SWORD UsbhMsdcWrite10(BYTE   bLun,
                             DWORD  dwLba,
                             DWORD  dwCount,
                             DWORD  dwBuffer,
                             WHICH_OTG  eWhichOtg);
static void UsbOtgHostTurTryAgain(void *arg);
/*
// Definition of internal functions
*/
//====================================================================
// * Function Name: OTGH_PT_Bulk_Init                          
// * Description: Bulk Test Init
// * Input: none
// * OutPut: none
//====================================================================
void OTGH_PT_Bulk_Init(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    flib_Host20_Asynchronous_Setting(HOST20_Enable, eWhichOtg);
#if (USBOTG_HOST_DESC == DISABLE) // Don't assign value
    pUsbhDevDes->psAppClass->bBulkInQHDArrayNum=0;//Array 0
    pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum=0;//Array 0
#endif
//    if (((pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress)&BIT7)>0)
//    { 
//        pUsbhDevDes->psAppClass->bBulkInQHDArrayNum=0;//Array 0
//        pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum=1;//Array 0
//    }
//    else
//    {
//        pUsbhDevDes->psAppClass->bBulkInQHDArrayNum=1;//Array 0
//        pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum=0;//Array 0
//    }
}

//====================================================================
// * Function Name: OTGH_PT_Bulk_Close                          
// * Description: Close the Bulk Path Test
// * Input: none
// * OutPut: none
//====================================================================
void OTGH_PT_Bulk_Close(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);      
    //BYTE i;	

    if (pUsbhDevDes->bDeviceStatus != USB_STATE_CONFIGURED)
        return;

    //flib_Host20_ReleaseStructure(Host20_MEM_TYPE_4K_BUFFER,(DWORD)sOTGH_PT_BLK);
//__asm("break 100");
    //Release Memory	sOTGH_PT_BLK->pbDataPage_Out
    //for (i=0;i<5;i++)
    //    flib_Host20_ReleaseStructure(Host20_MEM_TYPE_4K_BUFFER,(DWORD)pUsbhDevDes->psAppClass->pbDataPage_Out[i]);

    //Release Memory	sOTGH_PT_BLK->pbDataPage_In
    //for (i=0;i<5;i++)
    //    flib_Host20_ReleaseStructure(Host20_MEM_TYPE_4K_BUFFER,(DWORD)pUsbhDevDes->psAppClass->pbDataPage_In[i]);	

    //<5>.Hang the qHD
    //if (pUsbhDevDes->psAppClass->sBulkInDescriptor.bmAttributes == OTGH_ED_BULK)    
    {//<5.1>.stop Asynchronous Schedule
        //flib_Host20_Asynchronous_Setting(HOST20_Disable);
        //if (psHost20_qHD_List_Bulk[0]->bMaxPacketSize> 128)
        //{//Support only 2 QHD
        //    mwHost20_CurrentAsynchronousAddr_Set(gHost20_STRUCTURE_qHD_BASE_ADDRESS);
        //}
        //else           
        //{
            //<5.2>.Hang the qHD for ED0~ED3
        //    psHost20_qHD_List_Control[1]->bNextQHDPointer=(((DWORD)psHost20_qHD_List_Control[0])>>5);
        //}

        //<5.2>.Enable Asynchronous Schedule
        //flib_Host20_Asynchronous_Setting(HOST20_Enable);
    }      

}


SWORD RequestSenseDataProcess (WHICH_OTG eWhichOtg)
{
    SWORD err = USB_NO_ERROR;
    err = UsbhMsdcCommand(SCSI_REQUEST_SENSE, eWhichOtg);
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("RequestSenseDataProcess faile err = %d OTG %d", err, eWhichOtg);
    }

    return err;
}



 SWORD UsbhMsdcCommand( BYTE opCode, WHICH_OTG eWhichOtg)
{
    SWORD   err = USB_NO_ERROR;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR pAppMsdc = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PUSB_HOST_MSDC psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);

#if USBOTG_HOST_DESC
    if (BULK_ONLY_PROTOCOL==GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
        err = MsdcBulkOnlyExecuteCommand(opCode, eWhichOtg);

    else if (CBI_WITHOUT_INT_PROTOCOL==GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
        err = MsdcCbiExecuteCommand(opCode, eWhichOtg);

    else if (CBI_PROTOCOL==GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
        err = MsdcCbiExecuteCommand_interrupt(opCode, eWhichOtg);
#else
     if (BULK_ONLY_PROTOCOL==pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
         err = MsdcBulkOnlyExecuteCommand(opCode, eWhichOtg);
    
     else if (CBI_WITHOUT_INT_PROTOCOL==pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
         err = MsdcCbiExecuteCommand(opCode, eWhichOtg);
    
    else if (CBI_PROTOCOL==pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
         err = MsdcCbiExecuteCommand_interrupt(opCode, eWhichOtg);
#endif

    DWORD  dwEvent;
    SDWORD sdwRtVal = 0;
    BYTE is_okay = 1;
    //EventWait(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), 0x7fffffff, OS_EVENT_OR, &dwEvent);
    sdwRtVal = EventWaitWithTO (UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg),
                                0x7fffffff,
                                OS_EVENT_OR,
                                &dwEvent,
                                USBOTG_TRANSACTION_TIME_OUT_CNT);
    
    if (sdwRtVal != OS_STATUS_OK)
    {
        MP_DEBUG("EventWaitWithTO timeout");
        UsbOtgHostInactiveAllqTD(eWhichOtg);
        UsbHostFinal(USB_HOST_DEVICE_RESET, eWhichOtg);
        err = USB_HOST_DEVICE_RESET;
    }
    else
    {
        if (dwEvent & EVENT_MSDC_DEVICE_PLUG_OUT)
        {
            MP_DEBUG("device plug out");
            UsbOtgHostInactiveAllqTD(eWhichOtg);
            UsbHostFinal(USB_HOST_DEVICE_PLUG_OUT, eWhichOtg);
            err = USB_HOST_DEVICE_PLUG_OUT;            
        }
    }

#if USBOTG_HOST_DESC
    if (BULK_ONLY_PROTOCOL==GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
    {
        free1(pUsbhDevDes->psAppClass->psCbw, eWhichOtg);
        free1(pUsbhDevDes->psAppClass->psCsw, eWhichOtg);       
    }
    else if (CBI_WITHOUT_INT_PROTOCOL==GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
    {;
    }
    else if (CBI_PROTOCOL==GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx))
    {;
    }
#else
    if (BULK_ONLY_PROTOCOL==pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
    {
        free1(pUsbhDevDes->psAppClass->psCbw, eWhichOtg);
        free1(pUsbhDevDes->psAppClass->psCsw, eWhichOtg);
    }
    else if (CBI_WITHOUT_INT_PROTOCOL==pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
    {;
    }
    else if (CBI_PROTOCOL==pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol)
    {;
    }
#endif    

    if (pAppMsdc->psCsw->u8Status != USB_NO_ERROR &&
        pAppMsdc->psCbw->u8CB[0] == SCSI_MODE_SENSE_6)
    {
        MP_DEBUG("gIsModeSense6TryAgain = TRUE");
        psMsdc->boIsModeSense6TryAgain = TRUE;
    }
    return err;
}
 
void UsbOtgHostBulkOnlyActive (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState)
    {
        case BULKONLY_CBW_STATE:
        {
            //MP_DEBUG("CBW");
            //UartOutText("1");
            Host20_BufferPointerArray_Structure aTemp;
            aTemp.BufferPointerArray[0] = (DWORD)(&(pUsbhDevDes->psAppClass->psCbw->u32Signature));
            aTemp.BufferPointerArray[1] = 0;	
            aTemp.BufferPointerArray[2] = 0;	
            aTemp.BufferPointerArray[3] = 0;
            aTemp.BufferPointerArray[4] = 0;		
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum,
                                            BYTE_COUNT_OF_CBW, 
                                            (&aTemp.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_OUT,
                                            eWhichOtg);
        }
        break;
        
        case BULKONLY_DATA_IN_STATE:
        {
            UsbOtgBulkOnlyDataInOutProcess(OTGH_DIR_IN, eWhichOtg);
        }
        break;
        
        case BULKONLY_DATA_OUT_STATE:
        {
            UsbOtgBulkOnlyDataInOutProcess(OTGH_DIR_OUT, eWhichOtg);
        }
        break;

        case BULKONLY_CSW_STATE:
        case BULKONLY_SECOND_CSW_STATE:
        {
            Host20_BufferPointerArray_Structure aTemp;
            //MP_DEBUG("CSW");
            //UartOutText("3");
            if (pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_SECOND_CSW_STATE)
            {
                pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum]->bDataToggleControl = 1;
            }
            
            aTemp.BufferPointerArray[0] = (DWORD)(&(pUsbhDevDes->psAppClass->psCsw->u32Signature));
            aTemp.BufferPointerArray[1] = 0;	
            aTemp.BufferPointerArray[2] = 0;	
            aTemp.BufferPointerArray[3] = 0;
            aTemp.BufferPointerArray[4] = 0;		
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkInQHDArrayNum,
                                            BYTE_COUNT_OF_CSW, 
                                            (&aTemp.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_IN,
                                            eWhichOtg);
        }
        break;

        case BULKONLY_DONE_STATE:
        {
        }
        break;

        default:
        {
        }
        break;
    }
}

void UsbOtgHostBulkOnlyIoc (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_DEVICE_PLUG_OUT);
        return;
    }

    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }
    
    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState)
    {
        case BULKONLY_CBW_STATE:
        {
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum], eWhichOtg);
            //MP_DEBUG("CBW Done");
            //UartOutText("a");
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CSW_STATE;
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    //__asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature ( pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                                              eWhichOtg);
                    if (pUsbhDevDes->psAppClass->psCbw->u8CB[0] == SCSI_TEST_UNIT_READY)
                    {
                        pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CBW_STATE;
                    }

                        return;
                }
                else
                {
                    MP_DEBUG1("BULKONLY_CBW_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }
            else
            {
                if (pUsbhDevDes->psAppClass->dDataInLen > 0)
                {
                    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_DATA_IN_STATE;
                }
                else if(pUsbhDevDes->psAppClass->dDataOutLen > 0)
                {
                    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_DATA_OUT_STATE;
                }
                else
                {
                    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CSW_STATE;
                }
            }

            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }
        break;

        case BULKONLY_DATA_IN_STATE:
        {
            //MP_DEBUG("DATA IN Done");
            //UartOutText("b");
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
            //MP_DEBUG("DATA IN Done");
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CSW_STATE;
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    //__asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                                            eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                        free1((void*)pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);                        
                        return;
                    //}
                }
                else
                {
                    MP_DEBUG1("BULKONLY_DATA_IN_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }
            
            pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CSW_STATE;
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }
        break;
        
        case BULKONLY_DATA_OUT_STATE:
        {
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum], eWhichOtg);
            //MP_DEBUG("DATA OUT Done");
            //UartOutText("e");
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CSW_STATE;
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    //__asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                                            eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                        return;
                    //}
                }
                else
                {
                    MP_DEBUG1("BULKONLY_DATA_OUT_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }           
            
            pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CSW_STATE;
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }
        break;

        case BULKONLY_CSW_STATE:
        case BULKONLY_SECOND_CSW_STATE:
        {
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
            //MP_DEBUG("CSW Done");
            //UartOutText("c");
            if (pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_SECOND_CSW_STATE)
            {
                pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum]->bDataToggleControl = 0;
            }
            
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                   // __asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                                            eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                    //    return;
                    //}
                    if (pUsbhDevDes->psAppClass->psCbw->u32DataTransferLength > 0)
                    {
                        free1((void*)pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);                        
                    }
                    return;
                }
                else
                {
                    MP_DEBUG1("BULKONLY_CSW_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }

            ST_MCARD_DEVS *pUsbh;

            pUsbh = (ST_MCARD_DEVS*)GetMcardDevTag4Usb();//psMcardDev;
            
            if (pUsbhDevDes->psAppClass->psCbw->u8CB[0] == SCSI_READ_10)
            {
                if (pUsbhDevDes->psAppClass->psCsw->u8Status == COMMAND_PASS)
                {
                    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
                }
                else
                {
                    pUsbh->sMDevice[pUsbhDevDes->psAppClass->bDevLun].swStatus = USB_CSW_FAILED;
                    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_FAILED);
                }
            }
            else if (pUsbhDevDes->psAppClass->psCbw->u8CB[0] == SCSI_WRITE_10)
            {
                if (pUsbhDevDes->psAppClass->psCsw->u8Status == COMMAND_PASS)
                {
                    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
                }
                else
                {
                    pUsbh->sMDevice[pUsbhDevDes->psAppClass->bDevLun].swStatus = USB_CSW_FAILED;
                    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_FAILED);
                }
            }
            else
            {
                EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
                TaskYield();
                SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg),eWhichOtg); // for next command
            }
        }
        break;

        case BULKONLY_DONE_STATE:
        {
        }
        break;

        default:
        {
        }
        break;
    }
}

void UsbOtgHostCbNoIntActive (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    //MP_DEBUG("UsbOtgHostCbNoIntActive");
    switch ( pUsbhDevDes->psAppClass->dwBulkOnlyState ) 
    {
	  case CBI_DATA_IN_STATE :
	  {
		UsbOtgBulkOnlyDataInOutProcess(OTGH_DIR_IN, eWhichOtg);
	  }
	  break;
	  
	  case CBI_DATA_OUT_STATE:
    	 {
		UsbOtgBulkOnlyDataInOutProcess(OTGH_DIR_OUT, eWhichOtg);
	  }
	  break;
	  
	 default:
        {
        }
        break;
    }
}

void UsbOtgHostCbNoIntIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    //MP_DEBUG("UsbOtgHostCbNoIntIoc");
            if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
            {
                EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_DEVICE_PLUG_OUT);
                return;
            }
            
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }
    
    switch ( pUsbhDevDes->psAppClass->dwBulkOnlyState) 
    {         
	  case CBI_DATA_IN_STATE :
	  {
	  	
	  	flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
		if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
		{
                  if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                  {
                      SWORD err = 0;
                      //__asm("break 100");
                      pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                      err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                                              eWhichOtg);
                      //if (err != USB_NO_ERROR)
                      //{ 
                          return;
                      //}
                  }
                  else
                  {
                      MP_DEBUG("CBI_DATA_IN_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                  }
              } 
		
	    if(  *(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_READ_10 )
           {
	        EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
           }
	    else
	    {
	    	 EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
               TaskYield();
	        SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next command
	    }
		
	  }
	  break;

	  case CBI_DATA_OUT_STATE:
	  {
	      flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum], eWhichOtg);

	     if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
	     {
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    //__asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                                            eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                        return;
                    //}
                }
                else
                {
                    MP_DEBUG("CBI_DATA_OUT_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }

	       //???   mail  ??
           if( *(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_WRITE_10 )
           {
               EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
           }
	    else
	    {
	    	 EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
               TaskYield();
	        SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next command
	    }		   
	  }
	  break;
	  
	  default :
	  {
	  }
	  break;
    }
}

void UsbOtgHostCbWithIntActive (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    //MP_DEBUG("UsbOtgHostCbWithIntActive");
    switch ( pUsbhDevDes->psAppClass->dwBulkOnlyState ) 
    {
	  case CBI_DATA_IN_STATE :
	  {
		UsbOtgBulkOnlyDataInOutProcess(OTGH_DIR_IN, eWhichOtg);
	  }
	  break;
	  
	  case CBI_DATA_OUT_STATE:
    	 {
		UsbOtgBulkOnlyDataInOutProcess(OTGH_DIR_OUT, eWhichOtg);
	  }
	  break;

	 case CBI_INTERRUPT_STATE:
	 {
          pUsbhDevDes->psAppClass->bIntDataInLen  = CBI_INTERRUPT_DATA_BLOCK_LEN;
	      pUsbhDevDes->psAppClass->dIntDataBuffer= (DWORD)allo_mem((DWORD)pUsbhDevDes->psAppClass->bIntDataInLen, eWhichOtg);
	      memset((BYTE*)pUsbhDevDes->psAppClass->dIntDataBuffer, 0, pUsbhDevDes->psAppClass->bIntDataInLen);
	      flib_Host20_Issue_Interrupt_Active( pUsbhDevDes->psAppClass->bIntInQHDArrayNum ,
		  	                                                     CBI_INTERRUPT_DATA_BLOCK_LEN   ,
		  	                                                     pUsbhDevDes->psAppClass->dIntDataBuffer   ,
		  	                                                     0  ,
		  	                                                     OTGH_DIR_IN  ,
		  	                                                     eWhichOtg);
	
	 }
        break;
	 
	 default:
        {
        }
        break;
    }
}

void UsbOtgHostCbWithIntIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    //MP_DEBUG("UsbOtgHostCbWithIntIoc");
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_DEVICE_PLUG_OUT);
        return;
    }
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }
    
    switch ( pUsbhDevDes->psAppClass->dwBulkOnlyState) 
    {         
	  case CBI_DATA_IN_STATE :
	  {
	  	
	  	flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
		if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
		{
                  if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                  {
                      SWORD err = 0;
                      //__asm("break 100");
                      pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                      err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                                              eWhichOtg);
                      //if (err != USB_NO_ERROR)
                      //{ 
                          return;
                      //}
                  }
                  else
                  {
                      MP_DEBUG("CBI_DATA_IN_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                  }
              } 

              if( (*(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_INQUIRY) ||
		     (*(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_REQUEST_SENSE))
              {
                  pUsbhDevDes->psAppClass->dwBulkOnlyState = CBI_INTERRUPT_STATE;
		    EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
              }
		else if( *(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_READ_10 )
		{
		    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
		}
		else
		{
		    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
                  TaskYield();
	           SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next command
		}
	  }
	  break;

	  case CBI_DATA_OUT_STATE:
	  {
	      flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum], eWhichOtg);

	     if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
	     {
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    //__asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                                            eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                        return;
                    //}
                }
                else
                {
                    MP_DEBUG("CBI_DATA_OUT_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }

              if( (*(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_INQUIRY) ||
		     (*(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_REQUEST_SENSE) )
              {
                  pUsbhDevDes->psAppClass->dwBulkOnlyState = CBI_INTERRUPT_STATE;
		    EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
              }
		else if( *(BYTE *)pUsbhDevDes->sSetupPB.dwDataAddress == SCSI_WRITE_10 )
		{
		    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
		}
		else
		{
		    EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
                  TaskYield();
	           SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next command
		}
	  }
	  break;

         case CBI_INTERRUPT_STATE :
	  {
	  	flib_Host20_Send_qTD_Done(pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum], eWhichOtg);

		if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
	       {
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    //__asm("break 100");
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                                            eWhichOtg);
                    //if (err != USB_NO_ERROR)
                    //{ 
                        return;
                    //}
                }
                else
                {
                    MP_DEBUG("CBI_INTERRUPT_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
                }
            }

	     if( (*(SWORD*)pUsbhDevDes->psAppClass->dIntDataBuffer == USB_NO_ERROR))
	     { 
	           EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);                  
	     }
	     else
	     {
	           EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_FAILED);       
	     }

	     free1((void*)pUsbhDevDes->psAppClass->dIntDataBuffer, eWhichOtg);
	     pUsbhDevDes->psAppClass->bIntDataInLen = 0;
	     TaskYield();
	     SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next command
          }
          break;
	  
	  default :
	  {
	  }
	  break;
    }
}

static void UsbOtgHostTurTryAgain(void *arg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    ST_MCARD_MAIL   *pSendMailDrv;
    WHICH_OTG eWhichOtg = ((BYTE *) arg)[1];

    //mpDebugPrint("UsbOtgHostTurTryAgain:eWhichOtg = %d", eWhichOtg);
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_CBW_STATE;
    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_NO_ERROR;
    EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
}


void UsbOtgHostBulkOnlySetupIoc (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }
    
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }

    if (pUsbhDevDes->psAppClass->swBulkOnlyError == USB_STALL_ERROR)
    {
        if (pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_CSW_STATE)
        {
            //mpDebugPrint("CSW try again!");
            pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_SECOND_CSW_STATE;
            pUsbhDevDes->psAppClass->swBulkOnlyError = USB_NO_ERROR;
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }    
        else if (pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_CBW_STATE)
        {
            //mpDebugPrint("CBW TUR try again!");
            UsbOtgSetSysTimerProc(USBOTG_HOST_TRY_TUR, UsbOtgHostTurTryAgain,TRUE, eWhichOtg);
        }
    }
    else
    {
        SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next setup command
    }
}

void UsbOtgHostCbNoIntSetupIoc (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }
        
#if USBOTG_HOST_DESC
    if ((pUsbhDevDes->psAppClass->dwBulkOnlyState == CBI_ADSC_STATE) && 
        (CBI_WITHOUT_INT_PROTOCOL == \
        GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)))
#else        
    if ((pUsbhDevDes->psAppClass->dwBulkOnlyState == CBI_ADSC_STATE )&&
        (CBI_WITHOUT_INT_PROTOCOL == \
         pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol))
#endif
    {
        if (pUsbhDevDes->psAppClass->dDataInLen > 0)
        {
            pUsbhDevDes->psAppClass->dwBulkOnlyState  = CBI_DATA_IN_STATE;
            //MP_DEBUG("CBI_BulkOnlyState = %d",pUsbhDevDes->psAppClass->dwBulkOnlyState);
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        } 
        else if ( pUsbhDevDes->psAppClass->dDataOutLen > 0)
        {
            pUsbhDevDes->psAppClass->dwBulkOnlyState  = CBI_DATA_OUT_STATE;
            //MP_DEBUG("CBI_BulkOnlyState = %d",pUsbhDevDes->psAppClass->dwBulkOnlyState);
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }  
        else
        {
            EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
            SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg);   //for next CB command
        }
    }
}

void UsbOtgHostCbWithIntSetupIoc (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }

#if USBOTG_HOST_DESC
    if ((pUsbhDevDes->psAppClass->dwBulkOnlyState == CBI_ADSC_STATE) && 
        (CBI_PROTOCOL == \
        GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)))
#else
    if ((pUsbhDevDes->psAppClass->dwBulkOnlyState == CBI_ADSC_STATE) && 
    	(CBI_PROTOCOL == \
    	 pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol))
#endif       
    {
        if(pUsbhDevDes->bQHStatus !=USB_NO_ERROR)
        {
            EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_FAILED);	
            SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg);   //for next CB command
        }
        else if (pUsbhDevDes->psAppClass->dDataInLen > 0)
        {    
            pUsbhDevDes->psAppClass->dwBulkOnlyState  = CBI_DATA_IN_STATE;
            //MP_DEBUG("CBI_BulkOnlyState = %d",pUsbhDevDes->psAppClass->dwBulkOnlyState);
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        } 
        else if ( pUsbhDevDes->psAppClass->dDataOutLen > 0)
        {
            pUsbhDevDes->psAppClass->dwBulkOnlyState  = CBI_DATA_OUT_STATE;
            //MP_DEBUG("CBI_BulkOnlyState = %d",pUsbhDevDes->psAppClass->dwBulkOnlyState);
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }  
        else
        {
            EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
            SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg);   //for next CB command
        }
    }
}

#ifdef USBOTG_HOST_CDC_HUAWEI_TESTING
SWORD ProcessInquiryData(PSTANDARD_INQUIRY pInquiryBuffer)
{

   /*
00 80 02 02 1F 00 00 00 
49 2D 53 74 69 63 6B 32 
49 6E 74 65 6C 6C 69 67 
65 6E 74 53 74 69 63 6B 
32 2E 30 30 
typedef struct {
    BYTE   bQlfrDevtype;                00              
                                       
    BYTE   bRmb;                        80                      
    BYTE   bStandard;                   02                 
    BYTE   bSupDatType;                 02                   
                                       
    BYTE   bAddDataLen;                 1F                
    BYTE   bReserved1;                  00
    BYTE   bReserved2;                  00
    BYTE   bSupport;                    00
    BYTE   bVendorID[VEN_ID_LEN];       49 2D 53 74 69 63 6B 32
    BYTE   bPrdctID[PRDCT_ID_LEN];      49 6E 74 65 6C 6C 69 67 65 6E 74 53 74 69 63 6B
    BYTE   bPrdctRev[PRDCT_REV_LEN];    32 2E 30 30
} STANDARD_INQUIRY, *PSTANDARD_INQUIRY; 

   */
    
    // Check if there is a valid device connected.
    if ( (pInquiryBuffer->bQlfrDevtype & INQ_PERIPHERAL_QUALIFIER_MASK) != INQ_PERIPHERAL_CONNECTED )
    {
        // Check the Peripheral Qualifier to determine if there is a connected device at this LUN
        // Since ATAPI does not support LUNs, this will be always be INQ_PERIPHERAL_CONNECTED for
        // devices with an ATAPI interface.

        // If there is not a valid device, return an error and allow ourselves to be removed
        mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
        return PARSE_ERROR_DEVICE_NOT_SUPPORT;
    }

    // Check if the connected device is of a supported device class.
    switch ( pInquiryBuffer->bQlfrDevtype & INQ_PERIPHERAL_DEVICE_TYPE_MASK )
    {
        case INQ_DIRECT_ACCESS_SBC_DEVICE:
        case INQ_CDROM_MMC_DEVICE:
        {
            // The connected device is of a supported device class
            // break and proceed with the processing
            ;//gTheUnitCharacteristics[unitNumber].peripheralDeviceType = pInqReplyData->bQlfrDevtype & INQ_PERIPHERAL_DEVICE_TYPE_MASK;
        }
        break;

        case INQ_WRITE_ONCE_SBC_DEVICE:
        //case INQ_CDROM_MMC_DEVICE:
        case INQ_OPTICAL_MEMORY_SBC_DEVICE:
        case INQ_SIMPLIFIED_DIRECT_ACCESS_RBC_DEVICE:
        default:
        {
            // The device class is not supported, return an error
            // and allow the driver to be removed.
            mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
            return PARSE_ERROR_DEVICE_NOT_SUPPORT;
        }
        break;
    }
    
    // Check if this is a Removable device
    if ( (pInquiryBuffer->bRmb & kInqRemovableMask) != 0)
    {

        // Is it a CD/DVD Drive?
        if (( pInquiryBuffer->bQlfrDevtype & INQ_PERIPHERAL_DEVICE_TYPE_MASK ) == INQ_CDROM_MMC_DEVICE )
        {
            // It is a CD Drive
            mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
            //return PARSE_ERROR_DEVICE_NOT_SUPPORT;
            return PARSE_NO_ERROR;
        }
        else
        {
            ;// Not a CD Drive
            // Add Removables to the list of supported media types
        }
    }
    else
    {
        ;// Add Fixed Disk to the list of supported media types
    }

    // Check to determine which bStandard of the commands should
    // 0 = ATAPI, 1= SCSI, 2 = SCSI-2, 3 = SCSI-3
    switch(pInquiryBuffer->bStandard & kInqANSIVersionMask)	// be used for the attached device.
    {
        case 0: // ATAPI
        case 1: // SCSI 
        case 2: // SCSI-2
        case 3: // SCSI-3
        {
            ;// ok
        }
        break;
        
        default:
        {
            mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
            return PARSE_ERROR;
        }
        break;
    }

    mpDebugPrint("%s:%d", __FUNCTION__, __LINE__);
    return PARSE_NO_ERROR;
}
#else
SWORD ProcessInquiryData(PSTANDARD_INQUIRY pInquiryBuffer)
{

   /*
00 80 02 02 1F 00 00 00 
49 2D 53 74 69 63 6B 32 
49 6E 74 65 6C 6C 69 67 
65 6E 74 53 74 69 63 6B 
32 2E 30 30 
typedef struct {
    BYTE   bQlfrDevtype;                00              
                                       
    BYTE   bRmb;                        80                      
    BYTE   bStandard;                   02                 
    BYTE   bSupDatType;                 02                   
                                       
    BYTE   bAddDataLen;                 1F                
    BYTE   bReserved1;                  00
    BYTE   bReserved2;                  00
    BYTE   bSupport;                    00
    BYTE   bVendorID[VEN_ID_LEN];       49 2D 53 74 69 63 6B 32
    BYTE   bPrdctID[PRDCT_ID_LEN];      49 6E 74 65 6C 6C 69 67 65 6E 74 53 74 69 63 6B
    BYTE   bPrdctRev[PRDCT_REV_LEN];    32 2E 30 30
} STANDARD_INQUIRY, *PSTANDARD_INQUIRY; 

   */
    
    // Check if there is a valid device connected.
    if ( (pInquiryBuffer->bQlfrDevtype & INQ_PERIPHERAL_QUALIFIER_MASK) != INQ_PERIPHERAL_CONNECTED )
    {
        // Check the Peripheral Qualifier to determine if there is a connected device at this LUN
        // Since ATAPI does not support LUNs, this will be always be INQ_PERIPHERAL_CONNECTED for
        // devices with an ATAPI interface.

        // If there is not a valid device, return an error and allow ourselves to be removed
        return PARSE_ERROR_DEVICE_NOT_SUPPORT;
    }

    // Check if the connected device is of a supported device class.
    switch ( pInquiryBuffer->bQlfrDevtype & INQ_PERIPHERAL_DEVICE_TYPE_MASK )
    {
        case INQ_DIRECT_ACCESS_SBC_DEVICE:
        {
            // The connected device is of a supported device class
            // break and proceed with the processing
            ;//gTheUnitCharacteristics[unitNumber].peripheralDeviceType = pInqReplyData->bQlfrDevtype & INQ_PERIPHERAL_DEVICE_TYPE_MASK;
        }
        break;

        case INQ_WRITE_ONCE_SBC_DEVICE:
        case INQ_CDROM_MMC_DEVICE:
        case INQ_OPTICAL_MEMORY_SBC_DEVICE:
        case INQ_SIMPLIFIED_DIRECT_ACCESS_RBC_DEVICE:
        default:
        {
            // The device class is not supported, return an error
            // and allow the driver to be removed.
            return PARSE_ERROR_DEVICE_NOT_SUPPORT;
        }
        break;
    }
    
    // Check if this is a Removable device
    if ( (pInquiryBuffer->bRmb & kInqRemovableMask) != 0)
    {

        // Is it a CD/DVD Drive?
        if (( pInquiryBuffer->bQlfrDevtype & INQ_PERIPHERAL_DEVICE_TYPE_MASK ) == INQ_CDROM_MMC_DEVICE )
        {
            // It is a CD Drive
            return PARSE_ERROR_DEVICE_NOT_SUPPORT;
        }
        else
        {
            ;// Not a CD Drive
            // Add Removables to the list of supported media types
        }
    }
    else
    {
        ;// Add Fixed Disk to the list of supported media types
    }

    // Check to determine which bStandard of the commands should
    // 0 = ATAPI, 1= SCSI, 2 = SCSI-2, 3 = SCSI-3
    switch(pInquiryBuffer->bStandard & kInqANSIVersionMask)	// be used for the attached device.
    {
        case 0: // ATAPI
        case 1: // SCSI 
        case 2: // SCSI-2
        case 3: // SCSI-3
        {
            ;// ok
        }
        break;
        
        default:
        {
            return PARSE_ERROR;
        }
        break;
    }

    return PARSE_NO_ERROR;
}

#endif // #ifdef USBOTG_HOST_CDC_HUAWEI_TESTING
#if 0 //USBH_MJPEG_DROP_FRAME_ISSUE
#define BOUNDARY_ALIAN 0xff
SWORD UsbhMsdcRead10(   BYTE                        bLun,
                        DWORD                       dwLba,
                        DWORD                       dwCount,
                        DWORD                       dwBuffer,
                        WHICH_OTG                   eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);     
    register SWORD err             = USB_NO_ERROR;
    SWORD          err_req_sense   = USB_NO_ERROR;
    BYTE          *buf_temp = 0;
    
    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
        //MP_DEBUG("CMP_STOP_TIMER");
    if (dwCount > 128)
    {
        ;//gtt = 1;//MP_DEBUG1("Read10 count is %d", dwCount);
    }

     MP_DEBUG("R LBA = %d; CNT = %d; dwBuffer = 0x%x", dwLba, dwCount, dwBuffer);
     //__asm("break 100");
    pAppMsdc->bDevLun       = bLun;
    pAppMsdc->dDataInLen    = dwCount << pAppMsdc->bSectorExp;
    pAppMsdc->dDataOutLen   = 0;
    pAppMsdc->dLba          = dwLba;
    if (dwBuffer & BOUNDARY_ALIAN)
    {
        DWORD len = 0;
        len = pAppMsdc->dDataInLen+0x100;
        MP_DEBUG("free space = %d; alloc len = %d", ker_mem_get_free_space(), len);
        buf_temp = (BYTE*)ker_mem_malloc(len, TaskGetId());
        if (buf_temp == 0)
        {
            MP_ASSERT("alloc fail");
            __asm("break 100");
            return FAIL;
        }

        pAppMsdc->dDataBuffer = (DWORD)buf_temp+0x100;
        MP_DEBUG("alloc buffer = 0x%x", buf_temp);
        pAppMsdc->dDataBuffer &= ~BOUNDARY_ALIAN;
        MP_DEBUG("buffer = 0x%x; len = %d", pAppMsdc->dDataBuffer, pAppMsdc->dDataInLen);
    }
    else
    {
        pAppMsdc->dDataBuffer   = (DWORD)(dwBuffer|0xa0000000);
    }

    UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)pAppMsdc->dDataBuffer,
                                        (DWORD)((DWORD)pAppMsdc->dDataBuffer+pAppMsdc->dDataInLen));
    err = UsbhMsdcCommand(pUsbhDevDes, pAppMsdc, SCSI_READ_10);    
    if (err == USB_NO_ERROR)
    {
        err = PASS;
    }
    else
    {
        MP_DEBUG1("ERROR:UsbhMsdcRead10:err = %x", err);
        err = FAIL;
        if (pAppMsdc->dEventSet[bLun] == 1)
        {
            // set event to main to un-mount
            Ui_SetCardOutCodeByLun(bLun, eWhichOtg);
            //EventSet(UI_EVENT, EVENT_CARD_OUT);
            pAppMsdc->dEventSet[bLun] = 0;
            pAppMsdc->dPresent[bLun] = 0;
        }
    }

    if (dwBuffer & BOUNDARY_ALIAN)
    {
        memcpy((BYTE*)dwBuffer, (BYTE*)pAppMsdc->dDataBuffer, pAppMsdc->dDataInLen);
        ker_mem_free(buf_temp);
        MP_DEBUG("free buffer = 0x%x", buf_temp);
    }
    
    UsbOtgHostSetSwapBuffer2RangeDisable();
    SetCheckMediaPresentTimerFlag(CMP_START_TIMER, eWhichOtg);
    return err;
}
#else
SWORD UsbhMsdcRead10(   BYTE                        bLun,
                        DWORD                       dwLba,
                        DWORD                       dwCount,
                        DWORD                       dwBuffer,
                        WHICH_OTG                   eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);    
    register SWORD                       err             = USB_NO_ERROR;
    SWORD                       err_req_sense   = USB_NO_ERROR;
    
    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
        //MP_DEBUG("CMP_STOP_TIMER");
    if (dwCount > 128)
    {
        ;//gtt = 1;//MP_DEBUG1("Read10 count is %d", dwCount);
    }

    MP_DEBUG("-USBOTG%d- Lun = %d; R LBA = %d; CNT = %d", eWhichOtg, bLun, dwLba, dwCount);
     //__asm("break 100");
    pAppMsdc->bDevLun       = bLun;
    pAppMsdc->dDataInLen    = dwCount << pAppMsdc->bSectorExp;
    pAppMsdc->dDataOutLen   = 0;
    pAppMsdc->dDataBuffer   = (DWORD)(dwBuffer|0xa0000000);
    pAppMsdc->dLba          = dwLba;

    UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)pAppMsdc->dDataBuffer,
                                        (DWORD)((DWORD)pAppMsdc->dDataBuffer+pAppMsdc->dDataInLen),
                                        eWhichOtg);
    err = UsbhMsdcCommand(SCSI_READ_10, eWhichOtg);    
    if (err == USB_NO_ERROR)
    {
        err = PASS;
    }
    else
    {
        MP_ALERT("-USBOTG%d- ERROR:UsbhMsdcRead10:err = %x", eWhichOtg, err);
        err = FAIL;
        if (pAppMsdc->dEventSet[bLun] == 1)
        {
            // set event to main to un-mount
            Ui_SetCardOutCodeByLun(bLun, eWhichOtg);
            //EventSet(UI_EVENT, EVENT_CARD_OUT);
            pAppMsdc->dEventSet[bLun] = 0;
            pAppMsdc->dPresent[bLun] = 0;
        }
        /*
        err_req_sense = RequestSenseDataProcess(pUsbhDevDes, pAppMsdc);
        if (err_req_sense == USB_NO_ERROR)
        {
            if (pAppMsdc->bEventSet[pAppMsdc->bDevLun] == 1 ||
                pAppMsdc->dRequestSenseCode[pAppMsdc->bDevLun] != USBH_NO_SENSE)
            {
                // set event to main to un-mount
                Ui_SetCardCodeByLun(pAppMsdc->bDevLun);
                EventSet(UI_EVENT, EVENT_CARD_OUT);
                pAppMsdc->bEventSet[pAppMsdc->bDevLun] = 0;
                pAppMsdc->dRequestSenseCode[pAppMsdc->bDevLun] = USBH_NO_SENSE;
            }
        }
        else
        {
            MP_DEBUG("UsbhMsdcRead10:RequestSense failed");
            SetHostToResetDeviceFlag(eWhichOtg);
        }
        */
    }

    UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
    SetCheckMediaPresentTimerFlag(CMP_START_TIMER, eWhichOtg);
    return err;
}
#endif

#if (USBOTG_HOST_TEST || Make_TESTCONSOLE)
#define TEST_SECTOR_CNT 512
#define TEST_BYTE_CNT   512 * TEST_SECTOR_CNT
SWORD UsbOtgHostTestFunction (  BYTE                        bLun,
                                DWORD                       dwLba,
                                DWORD                       dwCount,
                                DWORD                       dwBuffer,
                                WHICH_OTG                   eWhichOtg)
{
    DWORD   len = 0, i;
    DWORD   idx = 0;
    DWORD   buf = 0;
    BYTE    *pbuf_tmp_w = 0;
    BYTE    *pbuf_tmp_r = 0;
    BYTE    *pbuf_tmp_w1 = 0;
    BYTE    *pbuf_tmp_r1 = 0;
	DWORD   OKcnt = 0;
	DWORD   FAILCnt = 0;
	DWORD	addrOffset=0x0;
	DWORD   sectorCnt = 0;
	BOOL    error = FALSE;
    SWORD   sts = FAIL;
    DWORD   dwStartTime = 0, dwEndTime = 0;

    PUSB_HOST_MSDC pMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);

    if (CheckIfUsbhReadyToReadWrite(eWhichOtg) == FALSE ||
        GetUsbhCardPresentFlag(bLun, eWhichOtg) == FALSE)
    {
        mpDebugPrint("-USBOTG%d- Not Present", eWhichOtg);
        return FAIL;
    }
    
    idx = 0x2000;
	pbuf_tmp_r = (BYTE *)ext_mem_malloc((TEST_BYTE_CNT+0x6000));
	pbuf_tmp_w = (BYTE *)ext_mem_malloc((TEST_BYTE_CNT+0x6000));
	pbuf_tmp_w = (BYTE *)(DWORD)(((DWORD)pbuf_tmp_w | 0xa0000000));
	pbuf_tmp_r = (BYTE *)(DWORD)(((DWORD)pbuf_tmp_r | 0xa0000000));
	pbuf_tmp_w = (BYTE *)(DWORD)((((DWORD)pbuf_tmp_w)&0xfffff000)+0x00001000);
	pbuf_tmp_r = (BYTE *)(DWORD)((((DWORD)pbuf_tmp_r)&0xfffff000)+0x00001000);

	pbuf_tmp_w1 = pbuf_tmp_w;
	pbuf_tmp_r1 = pbuf_tmp_r;

_TEST_LOOP_:    
	pbuf_tmp_w = (BYTE *)(DWORD)(((DWORD)pbuf_tmp_w1)+addrOffset);
	pbuf_tmp_r = (BYTE *)(DWORD)(((DWORD)pbuf_tmp_r1)+addrOffset);

	SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
	mpDebugPrint("pbuf_tmp_w = %x", pbuf_tmp_w);
	mpDebugPrint("pbuf_tmp_r = %x", pbuf_tmp_r);

#if 1
	if (sectorCnt > TEST_SECTOR_CNT)
	{
		sectorCnt = 1;

		if (addrOffset <= 0x1000)
			addrOffset+=0x4;
		else 
		{
			addrOffset = 0;
		}
	}
	else sectorCnt++;
#elif 0
	sectorCnt+=17;
	if (sectorCnt > 512)
	{
		sectorCnt -= 512;

		if (addrOffset <= 0x1000)
		{
			addrOffset+=0x80;
		}
		else 
		{
			addrOffset -= 0x1000;
		}
	}
#elif 0
	if (sectorCnt >= 0x1f)
	{
		sectorCnt = 1;

		if (addrOffset <= 0x1000)
			addrOffset+=0x4;
		else 
		{
			addrOffset = 0;
		}
	}
	else sectorCnt++;


#elif 0
		if (addrOffset <= 0x1000)
			addrOffset+=0x4;
		else 
		{
			addrOffset = 0x0;
			sectorCnt++;
		}
#elif 0
		if (addrOffset <= 0x1000)
			addrOffset+=0x4;
		else 
		{
			addrOffset = 0;
			//sectorCnt++;
		}
#endif
    len = sectorCnt;
#if 1
{
	DWORD doCnt = (sectorCnt*512)/4;
	DWORD *pdw, *pdr;

	pdw = (DWORD *)pbuf_tmp_w;
	pdr = (DWORD *)pbuf_tmp_r;

	for(i=0; i<doCnt; i++)
	{
		pdw[i] = (BYTE)pMsdc->dwCbwTag;//gCbwTag;
		pdr[i] = 0;
	}
}
#endif

#if 1 // Write USB

	//mpDebugPrint("UsbhMsdcWrite10");
    dwStartTime = GetSysTime();
    sts = UsbhMsdcWrite10(  bLun,
                            idx,
                            len,
                            (DWORD)pbuf_tmp_w,
                            eWhichOtg);
    if(sts == PASS)
    {
        dwEndTime = SystemGetElapsedTime(dwStartTime);
        if(dwEndTime == 0) dwEndTime = 1;
        mpDebugPrint("-USBOTG%d- Write: %dSector %dByte %dms %dByte/s", eWhichOtg, len, len<<9, dwEndTime, (len<<9)*1000/dwEndTime);
    }
    else
    {
        mpDebugPrint("-USBOTG%d- Write Fail");
    }
    
#endif

#if 1 // Read USB

	//mpDebugPrint("UsbhMsdcRead10");
    dwStartTime = GetSysTime();
    sts = UsbhMsdcRead10(  bLun,
                           idx,
                           len,
                           (DWORD)pbuf_tmp_r,
                            eWhichOtg);
    if(sts == PASS)
    {
        dwEndTime = SystemGetElapsedTime(dwStartTime);
        if(dwEndTime == 0) dwEndTime = 1;
        mpDebugPrint("-USBOTG%d- Read: %dSector %dByte %dms %dByte/s", eWhichOtg, len, len<<9, dwEndTime, (len<<9)*1000/dwEndTime);
    }
    else
    {
        mpDebugPrint("-USBOTG%d- Read Fail");
    }
    
#endif

#if 1
	error = FALSE;
	// Compare
	{
		DWORD *pdw, *pdr;
		DWORD cmpCnt = (sectorCnt*512)/4;
		pdw = (DWORD *)pbuf_tmp_w;
		pdr = (DWORD *)pbuf_tmp_r;
    	for (i=0;i<cmpCnt;i++)
	    { 
    	    if (pdw[i] != pdr[i])
        	{
        		error = TRUE;
	            mpDebugPrint("index = %d", i);
    	        mpDebugPrint("Expected data : 0x%x 0x%x", (DWORD)(&pdw[i]), pdw[i]);
        	    mpDebugPrint("Received Data : 0x%x 0x%x", (DWORD)(&pdr[i]), pdr[i]);
                mpDebugPrint("%s compared data NOT the same!!", __FUNCTION__);
                IODelay(200);
                __asm("break 100");
				break;
        	}
	    }
	}
	if (!error)
		OKcnt++;
	else
		FAILCnt++;

	mpDebugPrint("sectorCnt %x OK %x Fail %x", sectorCnt, OKcnt, FAILCnt);
#endif

goto _TEST_LOOP_;  
}
#endif


SWORD UsbhMsdcReadData (    BYTE                        bLun,
                            DWORD                       dwLba,
                            DWORD                       dwCount,
                            DWORD                       dwBuffer,
                            WHICH_OTG                   eWhichOtg)
{
    SWORD   sts = FAIL;
    DWORD   len = 0;
    DWORD   idx = 0;
    DWORD   buf = 0;
    BOOL    use128SectorCnt;
	WORD	wMAX_READ_SECTORS = 0;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);

    use128SectorCnt = CheckIfUse128MaxSectorCount(pUsbhDevDes->sDeviceDescriptor.idVendor, pUsbhDevDes->sDeviceDescriptor.idProduct);
//__asm("break 100");

#if 0 //USBH_MJPEG_DROP_FRAME_ISSUE
//__asm("break 100");
    WORD free_space_by_sector = 0;
    free_space_by_sector = ker_mem_get_free_space() >> pAppMsdc->bSectorExp >> 1;
    free_space_by_sector = 4*(free_space_by_sector/4+(free_space_by_sector%4?1:0));
    wMAX_READ_SECTORS = free_space_by_sector;//16; // 16 x 512 = 8KB
    MP_DEBUG("free_space_by_sector = %d", free_space_by_sector);
#else
	if (use128SectorCnt)
	{
		wMAX_READ_SECTORS = 128;
	}
	else
	{
		wMAX_READ_SECTORS = USB_OTG_BUF_SIZE>>pAppMsdc->bSectorExp;
	}
#endif

    if (CheckIfUsbhReadyToReadWrite(eWhichOtg) == FALSE ||
        GetUsbhCardPresentFlag(bLun, eWhichOtg) == FALSE)
    {
        return FAIL;
    }
    
    len = dwCount;
    idx = dwLba;
    buf = dwBuffer;

    while(len > 0)
    {
        if (len >= wMAX_READ_SECTORS)
        {
            sts = UsbhMsdcRead10 (  bLun,
                                    idx,
                                    wMAX_READ_SECTORS,
                                    buf,
                                    eWhichOtg);
            if (sts == FAIL)
            {
        		MP_DEBUG("UsbhMsdcReadData failed");
                break;
            }
            else
            {
                len -= wMAX_READ_SECTORS;
                idx += (wMAX_READ_SECTORS);
                buf += (wMAX_READ_SECTORS << pAppMsdc->bSectorExp);
            }
        }
        else
        {
            sts = UsbhMsdcRead10 (  bLun,
                                    idx,
                                    len,
                                    buf,
                                    eWhichOtg);
            if (sts == FAIL)
            {
        		MP_DEBUG("UsbhMsdcReadData failed");
                break;
            }
            else
            {
                len = 0;
            }
        }
        
        TaskYield();
    }

    //SetCheckMediaPresentTimer(cmp_val);
    return sts;
}

#define PHISON_VID		0x13FE
#define TOSHIBA_4G_PID	0x1A30
SWORD UsbhMsdcWriteData(    BYTE                        bLun,
                            DWORD                       dwLba,
                            DWORD                       dwCount,
                            DWORD                       dwBuffer,
                            WHICH_OTG                   eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);    
    SWORD   sts = FAIL;
    DWORD   len = 0;
    DWORD   idx = 0;
    DWORD   buf = 0;
    BOOL    cmp_val;
	WORD 	wPID = 0;
	WORD	wVID = 0;
	WORD	wMAX_READ_SECTORS = 0;

	wVID = (WORD)byte_swap_of_word(PHISON_VID);
	wPID = (WORD)byte_swap_of_word(TOSHIBA_4G_PID);
	if (pUsbhDevDes->sDeviceDescriptor.idProduct == wPID &&\
		pUsbhDevDes->sDeviceDescriptor.idVendor == wVID)
	{
		wMAX_READ_SECTORS = 128;
	}
	else
	{
		wMAX_READ_SECTORS = USB_OTG_BUF_SIZE>>pAppMsdc->bSectorExp;
	}
    
    if (CheckIfUsbhReadyToReadWrite(eWhichOtg) == FALSE ||
        GetUsbhCardPresentFlag(bLun, eWhichOtg) == FALSE)
    {
        return FAIL;
    }

    /*cmp_val = GetCheckMediaPresentTimer();
    if (CMP_START_TIMER == cmp_val)
        SetCheckMediaPresentTimer(CMP_STOP_TIMER);
    */
    len = dwCount;
    idx = dwLba;
    buf = dwBuffer;
    while(len > 0)
    {
        if (len > wMAX_READ_SECTORS)
        {
            sts = UsbhMsdcWrite10(  bLun,
                                    idx,
                                    wMAX_READ_SECTORS,
                                    buf,
                                    eWhichOtg);
            if (sts == FAIL)
            {
                break;
            }
            else
            {
                len -= wMAX_READ_SECTORS;
                idx += (wMAX_READ_SECTORS);
                buf += (wMAX_READ_SECTORS << pAppMsdc->bSectorExp);
            }
        }
        else
        {
            sts = UsbhMsdcWrite10(  bLun,
                                    idx,
                                    len,
                                    buf,
                                    eWhichOtg);
            if (sts == FAIL)
            {
                break;
            }
            else
            {
                len = 0;
            }
        }
        
        TaskYield();
    }

    //SetCheckMediaPresentTimer(cmp_val);
    return sts;
}
    
DWORD UsbhCheckReqSenseData(BYTE senseKey, BYTE asc, BYTE ascq)
{
    DWORD    code = ((senseKey << 16) | (asc << 8) | ascq);

    MP_DEBUG("CheckReqSenseData");
    MP_DEBUG1("code = %x", code);
    switch(code)
    {
        case RECOVERED_DATA_WITH_RETRIES:
	        MP_DEBUG("RECOVERED_DATA_WITH_RETRIES");
            break;
        case RECOVERED_DATA_WITH_ECC:
	        MP_DEBUG("RECOVERED_DATA_WITH_ECC");
            break;
        case MEDIUM_NOT_PRESENT:
	        MP_DEBUG("MEDIUM_NOT_PRESENT");
            break;
        case LOGICAL_DRIVE_NOT_READY_BECOMING_READY:
	        MP_DEBUG("LOGICAL_DRIVE_NOT_READY_BECOMING_READY");
            break;
        case LOGICAL_DRIVE_NOT_READY_FORMAT_IN_PROGRESS:
	        MP_DEBUG("LOGICAL_DRIVE_NOT_READY_FORMAT_IN_PROGRESS");
            break;
        case NO_REFERENCE_POSITION_FOUND:
	        MP_DEBUG("NO_REFERENCE_POSITION_FOUND");
            break;
        case NO_SEEK_COMPLETE:
	        MP_DEBUG("NO_SEEK_COMPLETE");
            break;
        case WRITE_FAULT:
	        MP_DEBUG("WRITE_FAULT");
            break;
        case ID_CRC_ERROR:
	        MP_DEBUG("ID_CRC_ERROR");
            break;
        case UNRECOVERED_READ_ERROR:
	        MP_DEBUG("UNRECOVERED_READ_ERROR");
            break;
        case ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD:
	        MP_DEBUG("ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD");
            break;
        case RECORDED_ENTITY_NOT_FOUND:
	        MP_DEBUG("RECORDED_ENTITY_NOT_FOUND");
            break;
        case INCOMPATIBLE_MEDIUM_INSTALLED:
	        MP_DEBUG("INCOMPATIBLE_MEDIUM_INSTALLED");
            break;
        case CANNOT_READ_MEDIUM_INCOMPATIBLE_FORMAT:
	        MP_DEBUG("CANNOT_READ_MEDIUM_INCOMPATIBLE_FORMAT");
            break;
        case CANNOT_READ_MEDIUM_UNKNOWN_FORMAT:
	        MP_DEBUG("CANNOT_READ_MEDIUM_UNKNOWN_FORMAT");
            break;
        case FORMAT_COMMAND_FAILED:
	        MP_DEBUG("FORMAT_COMMAND_FAILED");
            break;
        case INVALID_COMMAND_OPERATION_CODE:
	        MP_DEBUG("INVALID_COMMAND_OPERATION_CODE");
            break;
        case LOGICAL_BLOCK_DDRESS_OUT_OF_RANGE:
	        MP_DEBUG("LOGICAL_BLOCK_DDRESS_OUT_OF_RANGE");
            break;
        case INVALID_FIELD_IN_COMMAND_PACKET:
	        MP_DEBUG("INVALID_FIELD_IN_COMMAND_PACKET");
            break;
        case LOGICAL_UNIT_NOT_SUPPORTED:
	        MP_DEBUG("LOGICAL_UNIT_NOT_SUPPORTED");
            break;
        case INVALID_FIELD_IN_PARAMETER_LIST:
	        MP_DEBUG("INVALID_FIELD_IN_PARAMETER_LIST");
            break;
        case MEDIUM_REMOVAL_PREVENTED:
 	        MP_DEBUG("MEDIUM_REMOVAL_PREVENTED");
           break;
        case NOT_READY_TO_READY_TRANSITION_MEDIA_CHANGED:
	        MP_DEBUG("NOT_READY_TO_READY_TRANSITION_MEDIA_CHANGED");
            break;
        case POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED:
	        MP_DEBUG("POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED");
            break;
        case WRITE_PROTECTED_MEDIA:
	        MP_DEBUG("WRITE_PROTECTED_MEDIA");
            break;
        case OVERLAPPED_COMMAND_ATTEMPTED:
	        MP_DEBUG("OVERLAPPED_COMMAND_ATTEMPTED");
            break;
        default:
	        MP_DEBUG("??SENSE NOT DEFINED??");
            break;
    }
    return code;
}

/*
// Definition of local functions 
*/
#define MAX_DATA_PAGE           5
#define ONE_PAGE_BYTE_CNT       4096
#define ONE_TD_MAX_BYTE_CNT     16384//20480 // 4096 * 5
static void UsbOtgBulkOnlyDataInOutProcess(BYTE dir_flag, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);     
    DWORD pbDataPage[MAX_DATA_PAGE]; 
    DWORD wTotalLengthRemain = 0;
    DWORD wOneTdTotalLengthRemain = 0;
    DWORD w4kRemain;
    DWORD wOffset;
    WORD  hwSize = 0;
    DWORD i;
    BOOL  fActive = FALSE;
    BOOL  fUseMultiTDs = FALSE;
    //BYTE  dir_flag = 0;
    BYTE  qHD_array_number = 0;
    DWORD dwMaxTxLength = 0;
    DWORD dwBufferOffset = 0;
    //MP_DEBUG("DATA");
    //UartOutText("2");
    for (i = 0; i < MAX_DATA_PAGE; i++)
        pbDataPage[i] = 0;

    if (OTGH_DIR_IN == dir_flag)
    {
        qHD_array_number = pUsbhDevDes->psAppClass->bBulkInQHDArrayNum;
        wTotalLengthRemain = pUsbhDevDes->psAppClass->dDataInLen;
    }
    else if (OTGH_DIR_OUT == dir_flag)
    {
        qHD_array_number = pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum;
        wTotalLengthRemain = pUsbhDevDes->psAppClass->dDataOutLen;
    }
    else
    {
        MP_DEBUG("Wrong direction");
    }
//if (wTotalLengthRemain == 2048)
//__asm("break 100");
    pbDataPage[0]       = (DWORD)((pUsbhDevDes->psAppClass->dDataBuffer) & 0xfffff000);
    wOffset             = (pUsbhDevDes->psAppClass->dDataBuffer) & 0x0fff;
    w4kRemain           = ONE_PAGE_BYTE_CNT - wOffset;            
    hwSize              = 0;
    i                   = 1;
    if (wOffset == 0)
    { // if wOffset is 0, then max tx lenght is 20kbyte
        dwMaxTxLength = ONE_TD_MAX_BYTE_CNT + ONE_PAGE_BYTE_CNT;
        if (wTotalLengthRemain > ONE_TD_MAX_BYTE_CNT)
            hwSize += ONE_PAGE_BYTE_CNT;
    }
    else
    {
        dwMaxTxLength = ONE_TD_MAX_BYTE_CNT;
    }
    
    if (wTotalLengthRemain > dwMaxTxLength)
    { // use more then one TD
        fUseMultiTDs = TRUE;
    }
    
    fActive = FALSE;
    while (wTotalLengthRemain > 0)
    {
        if (wTotalLengthRemain > dwMaxTxLength)
        {
            wOneTdTotalLengthRemain = dwMaxTxLength;
        }
        else
        { // the last TD
            if (wTotalLengthRemain <= w4kRemain)
            { // not over one page boundary
                if (wTotalLengthRemain < dwMaxTxLength)
                {
                    hwSize = wTotalLengthRemain;
                    fActive = TRUE; // for sending multi-TD
                }
                
                wTotalLengthRemain -= hwSize;
            }
            else
            { // over one page
                hwSize = wTotalLengthRemain;
                wOneTdTotalLengthRemain = wTotalLengthRemain;
                fActive = TRUE; // for sending multi-TD
            }
        }
        
        while (wOneTdTotalLengthRemain > w4kRemain)
        { // process one page each loop
            if (wOneTdTotalLengthRemain > w4kRemain)
            {
                if (wOneTdTotalLengthRemain > ONE_PAGE_BYTE_CNT)
                {
                    if (fActive == FALSE)
                    { 
                        hwSize          += ONE_PAGE_BYTE_CNT;
                        dwBufferOffset  += ONE_PAGE_BYTE_CNT;
                    }
                    else
                    {
                        ;// hwSize is assigned already
                    }
                }
                else
                {
                    if (fActive == FALSE)
                    {
                        hwSize += wOneTdTotalLengthRemain;
                    }
                    else
                    {
                        ;// hwSize is assigned already
                    }
                }
                
                if (i < MAX_DATA_PAGE)
                    pbDataPage[i] = pbDataPage[i-1] + ONE_PAGE_BYTE_CNT;

                if (wTotalLengthRemain > w4kRemain)
                wTotalLengthRemain      -= w4kRemain;
                
                wOneTdTotalLengthRemain -= w4kRemain;
                if (wTotalLengthRemain <= ONE_PAGE_BYTE_CNT)
                { // the last page
                    if (fActive)
                    {
                        wTotalLengthRemain = 0;
                        break;
                    }
                    else
                    {
                        i++;
                    }
                }
                else
                {
                    w4kRemain = ONE_PAGE_BYTE_CNT;
                    i++;
                }
            }
            
            if (i == MAX_DATA_PAGE)
            {
                if (fUseMultiTDs)
                {
                    if (fActive == FALSE)
                    { // prepare next TD
                        flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                                 hwSize, 
                                                                 &(pbDataPage[0]), 
                                                                 wOffset, 
                                                                 dir_flag,
                                                                 fActive,
                                                                 eWhichOtg);
                
                        for (i = 0; i < MAX_DATA_PAGE; i++)
                            pbDataPage[i] = 0;
                
                        if (wOffset == 0)
                        {
                            dwBufferOffset += ONE_PAGE_BYTE_CNT;
                        }
                        
                        wTotalLengthRemain -= wOneTdTotalLengthRemain;
                        pbDataPage[0]       = (DWORD)(((DWORD)pUsbhDevDes->psAppClass->dDataBuffer+dwBufferOffset) & 0xfffff000);
                        w4kRemain           = ONE_PAGE_BYTE_CNT - wOffset;
                        hwSize              = 0;
                        i                   = 1;
                        if (wOffset == 0)
                        {
                            if (wTotalLengthRemain > ONE_TD_MAX_BYTE_CNT)
                                hwSize += ONE_PAGE_BYTE_CNT;
                        }
                    }
                }
                
                break;
            }                      
        }
    }

    if (fUseMultiTDs)
    { // for multi-TD
        flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                 hwSize, 
                                                 &(pbDataPage[0]), 
                                                 wOffset, 
                                                 dir_flag,
                                                 fActive,
                                                 eWhichOtg);
    }
    else
    { // for single-TD 
        flib_Host20_Issue_Bulk_Active ( qHD_array_number,
                                        hwSize, 
                                        &(pbDataPage[0]), 
                                        wOffset, 
                                        dir_flag,
                                        eWhichOtg);
    }
}

static SWORD MsdcBulkOnlyExecuteCommand (BYTE opCode, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);    
    SWORD   err = USB_NO_ERROR;

    pUsbhDevDes->psAppClass->psCbw = (CBW*)allo_mem(sizeof(CBW),eWhichOtg);
    pUsbhDevDes->psAppClass->psCsw = (CSW*)allo_mem(sizeof(CSW),eWhichOtg);
    CbwBuilder( pAppMsdc->psCbw,
                opCode,
                pAppMsdc->bDevLun,
                &pAppMsdc->dDataInLen,
                &pAppMsdc->dDataOutLen,
                &pAppMsdc->dDataBuffer,
                pAppMsdc->dLba,
                pAppMsdc->bSectorExp,
                eWhichOtg);
                
    err = BulkCommand(eWhichOtg); 

//    if (pAppMsdc->psCsw->u8Status != USB_NO_ERROR &&
//        pAppMsdc->psCbw->u8CB[0] == SCSI_MODE_SENSE_6)
//    {
//        gIsModeSense6TryAgain = TRUE;
//    }
    
    return err;
}

// static BYTE scsi_cmd[CBI_CMD_BYTE_CNT]; // Delete
static SWORD MsdcCbiExecuteCommand(BYTE opCode, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST    psUsbOtgHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);   
    PUSB_HOST_MSDC      psMsdc       = (PUSB_HOST_MSDC)&psUsbOtgHost->sMsdc;  	
	DWORD   i;
	SWORD  err = USB_NO_ERROR;
	USB_CTRL_REQUEST       adsc;        //  accept device specific command
    

	memset( &(psMsdc->bScsiCmd[0]), 0, sizeof(psMsdc->bScsiCmd));
	CbiCmdBuilder( &adsc,
		          &(psMsdc->bScsiCmd[0]), 
		          opCode, 
		          psUsbOtgHost->psAppClassDescriptor->bDevLun, 
		          &psUsbOtgHost->psAppClassDescriptor->dDataInLen, 
		          &psUsbOtgHost->psAppClassDescriptor->dDataOutLen, 
		          &psUsbOtgHost->psAppClassDescriptor->dDataBuffer, 
		          psUsbOtgHost->psAppClassDescriptor->dLba,
                  psUsbOtgHost->psAppClassDescriptor->bSectorExp,
                  eWhichOtg);

	err = CbiCommand(&adsc, &(psMsdc->bScsiCmd[0]), eWhichOtg);

      return err;
}

static SWORD MsdcCbiExecuteCommand_interrupt(BYTE opCode, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST    psUsbOtgHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);   
    PUSB_HOST_MSDC      psMsdc       = (PUSB_HOST_MSDC)&psUsbOtgHost->sMsdc;    
	DWORD  i;
	SWORD  err = USB_NO_ERROR;
	USB_CTRL_REQUEST       adsc;        //  accept device specific command

	memset( &(psMsdc->bScsiCmd[0]), 0, sizeof(psMsdc->bScsiCmd));
	CbiCmdBuilder( &adsc,
		          &(psMsdc->bScsiCmd[0]), 
		          opCode, 
		          psUsbOtgHost->psAppClassDescriptor->bDevLun, 
		          &psUsbOtgHost->psAppClassDescriptor->dDataInLen, 
		          &psUsbOtgHost->psAppClassDescriptor->dDataOutLen, 
		          &psUsbOtgHost->psAppClassDescriptor->dDataBuffer, 
		          psUsbOtgHost->psAppClassDescriptor->dLba,
                  psUsbOtgHost->psAppClassDescriptor->bSectorExp,
                  eWhichOtg);

	err = CbiCommand(&adsc, &(psMsdc->bScsiCmd[0]), eWhichOtg);

      return err;
}

static DWORD BulkCommand (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); 
                            
    pAppMsdc->dwBulkOnlyState   = BULKONLY_CBW_STATE;
    pUsbhDevDes->psAppClass     = pAppMsdc;
    return EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
}

static DWORD CbiCommand (PUSB_CTRL_REQUEST pAdsc, BYTE *pScsiCmd, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
        
	pUsbhDevDes->sSetupPB.stCtrlRequest.bRequestType    = pAdsc->bRequestType;
	pUsbhDevDes->sSetupPB.stCtrlRequest.bRequest        = pAdsc->bRequest;
	pUsbhDevDes->sSetupPB.stCtrlRequest.wIndex          = pAdsc->wIndex;
	pUsbhDevDes->sSetupPB.stCtrlRequest.wValue          = pAdsc->wValue;
	pUsbhDevDes->sSetupPB.stCtrlRequest.wLength         = pAdsc->wLength;

	pUsbhDevDes->sSetupPB.dwDataAddress                 = (DWORD)((DWORD *)(pScsiCmd));
	pUsbhDevDes->sSetupPB.wDataInLength                 = 0;
	pUsbhDevDes->sSetupPB.wDataOutLength                = (WORD) CBI_CMD_BYTE_CNT;

	pUsbhDevDes->sSetupPB.dwSetupState                  = SETUP_COMMAND_STATE;

	pAppMsdc->dwBulkOnlyState                           = CBI_ADSC_STATE;
	pUsbhDevDes->psAppClass                             = pAppMsdc;

	return EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_SETUP);
}



#define NETAC_VID       0x0DD8
#define USAFE_1G_PID    0x1400
#define U288_2G_PID     0x1408
static BOOL CheckIfUse128MaxSectorCount (WORD wVid, WORD wPid)
{

    if ((wPid == byte_swap_of_word(TOSHIBA_4G_PID)  && wVid == byte_swap_of_word(PHISON_VID))||
        (wPid == byte_swap_of_word(USAFE_1G_PID)    && wVid == byte_swap_of_word(NETAC_VID)) ||
        (wPid == byte_swap_of_word(U288_2G_PID)     && wVid == byte_swap_of_word(NETAC_VID)))
    {

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#define PANASONIC_VID			0x04DA
#define DMC_FZ10_CAMERA_PID		0x2372
BOOL CheckIfPanasonicDMC_FX8 (WORD wVid, WORD wPid)
{
    if ((wPid == byte_swap_of_word(DMC_FZ10_CAMERA_PID) && wVid == byte_swap_of_word(PANASONIC_VID)))
    {

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void CbwBuilder( PCBW    pCbw,
                        BYTE    opCode,
                        BYTE    devLun,
                        DWORD*  pData_in_length,
                        DWORD*  pData_out_length,
                        DWORD*  pData_addr,
                        DWORD   dLBA,
                        DWORD   bSectorExp,
                        WHICH_OTG eWhichOtg)
{
    WORD    transfer_length = 0;
    PUSB_HOST_MSDC psMsdc;

    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);

    // clear_mem((DWORD)pCbw, sizeof(CBW));
    memset((BYTE*)pCbw, 0, sizeof(CBW));
    pCbw->u32Signature = (DWORD)COMMAND_BLOCK_WRAPPER_SIGNATURE;
    pCbw->u32Tag       = psMsdc->dwCbwTag++;
    pCbw->u8LUN       = devLun;
    pCbw->u8CB[0]    = opCode;
    pCbw->u8CB[1]    = (devLun << 5);
    
    switch (opCode)
    {
#ifdef USBOTG_HOST_CDC_HUAWEI_TESTING
        case 0x11: // HUAWEI MSDC vendor command
        {
            pCbw->u32Tag        = 0x12345678;
            pCbw->u32DataTransferLength    = 0;
            pCbw->u8Flags     = 0;
            pCbw->u8LUN       = 0;
            pCbw->u8CBLength  = 0;
            pCbw->u8CB[0]    = opCode;
            pCbw->u8CB[1]    = 0x06;
            pCbw->u8CB[2]    = 0x20;
            pCbw->u8CB[4]    = 0;
            pCbw->u8CB[5]    = 1;
        }
#endif
        case SCSI_INQUIRY:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_INQUIRY_DATA;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = (BYTE)0x6;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            //pCbw->u8CB[1]    = (devLun << 5);
            pCbw->u8CB[4]    = *pData_in_length;
        }
        break;

        case SCSI_TEST_UNIT_READY:
        {
            //__asm("break 100");            
            *pData_out_length   = 0;
            *pData_in_length    = 0;
            *pData_addr         = 0;
            pCbw->u8Flags     = CBW_FLAGS_NO_DATA;
            pCbw->u8CBLength  = 0x6;
            pCbw->u32DataTransferLength   = 0;
            //pCbw->u8CB[1]    = (devLun << 5);
        }
        break;

        case SCSI_REQUEST_SENSE:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_SENSE_DATA;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = 0xC;
            pCbw->u32DataTransferLength    = byte_swap_of_dword(*pData_in_length);
            //pCbw->u8CB[1]    = (devLun << 5);
            pCbw->u8CB[4]    = *pData_in_length;
        }
        break;

        case SCSI_READ_FORMAT_CAPACITIES:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_FORMAT_CAPACITY_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = 0xA;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            //pCbw->u8CB[1]    = (devLun << 5);
            pCbw->u8CB[7]    = midlo_byte_of_dword(*pData_in_length);
            pCbw->u8CB[8]    = lo_byte_of_dword(*pData_in_length);
        }
        break;

        case SCSI_READ_CAPACITY:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_READ_CAPACITY_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = 0xA;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            //pCbw->u8CB[1]    = (devLun << 5);
        }
        break;

        case SCSI_MODE_SENSE_6:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_MODE_SENSE_6_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = 0x6;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            //pCbw->u8CB[1]    = (devLun << 5);
            if (psMsdc->boIsModeSense6TryAgain == TRUE)
            {
                MP_DEBUG("SCSI_MODE_SENSE_6 len = RETURN_ALL_PAGES 0x3F");
                pCbw->u8CB[2]    = RETURN_ALL_PAGES;
                psMsdc->boIsModeSense6TryAgain = FALSE;
            }
            else
            {
                MP_DEBUG("SCSI_MODE_SENSE_6 len = TIMER_AND_PROTECT_PAGE 0x1C");
                pCbw->u8CB[2]    = TIMER_AND_PROTECT_PAGE;
            }
            
            pCbw->u8CB[4]    = *pData_in_length;
        }
        break;

        case SCSI_MODE_SENSE_10:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_MODE_SENSE_10_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = 0xC;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            //pCbw->u8CB[1]    = (devLun << 5);
            pCbw->u8CB[2]    = TIMER_AND_PROTECT_PAGE;
            pCbw->u8CB[7]    = hi_byte_of_word(*pData_in_length);
            pCbw->u8CB[8]    = lo_byte_of_word(*pData_in_length);
        }
        break;

        case SCSI_READ_10:
        {
            transfer_length = *pData_in_length >> bSectorExp;
            pCbw->u8Flags = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength = 0xA;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            pCbw->u8CB[2] = hi_byte_of_dword(dLBA);
            pCbw->u8CB[3] = midhi_byte_of_dword(dLBA);
            pCbw->u8CB[4] = midlo_byte_of_dword(dLBA);
            pCbw->u8CB[5] = lo_byte_of_dword(dLBA);
            pCbw->u8CB[7] = hi_byte_of_word(transfer_length);
            pCbw->u8CB[8] = lo_byte_of_word(transfer_length);
        }
        break;

        case SCSI_WRITE_10:
        {
            transfer_length     = *pData_out_length >> bSectorExp;
            pCbw->u8Flags     = CBW_FLAGS_DATA_OUT;
            pCbw->u8CBLength  = 0xA;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_out_length);
            //pCbw->u8CB[1] = (devLun << 5);
            pCbw->u8CB[2] = hi_byte_of_dword(dLBA);
            pCbw->u8CB[3] = midhi_byte_of_dword(dLBA);
            pCbw->u8CB[4] = midlo_byte_of_dword(dLBA);
            pCbw->u8CB[5] = lo_byte_of_dword(dLBA);
            pCbw->u8CB[7] = hi_byte_of_word(transfer_length);
            pCbw->u8CB[8] = lo_byte_of_word(transfer_length);
        }
        break;

        case 0xff: // test
        {
            if (*pData_in_length == 0)
            *pData_in_length = 12;

            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            pCbw->u8Flags     = CBW_FLAGS_DATA_IN;
            pCbw->u8CBLength  = 0xA;
            pCbw->u32DataTransferLength   = byte_swap_of_dword(*pData_in_length);
            pCbw->u8CB[7]    = midlo_byte_of_dword(*pData_in_length);
            pCbw->u8CB[8]    = lo_byte_of_dword(*pData_in_length);
        }
        break;
    }
}

static void CbiCmdBuilder(PUSB_CTRL_REQUEST pAdsc, 
					BYTE * pScsiCmd,
					BYTE opCode,
					BYTE devLun,
					DWORD * pData_in_length,
					DWORD * pData_out_length,
					DWORD * pData_addr,
					DWORD dLBA,
					DWORD bSectorExp,
					WHICH_OTG eWhichOtg)
{
    WORD transfer_length = 0;

    pAdsc->bRequestType = (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE);
    pAdsc->bRequest	= 0;
    pAdsc->wIndex	= 0;
    pAdsc->wValue       = 0;
    pAdsc->wLength     = byte_swap_of_word(CBI_CMD_BYTE_CNT);

    pScsiCmd[0]     = opCode;
    pScsiCmd[1]     = devLun<<5;

    switch (opCode) 
    {
          case SCSI_INQUIRY:
	   {
	          *pData_out_length   = 0;
	          *pData_in_length   =BYTE_COUNT_OF_INQUIRY_DATA;
	          *pData_addr = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
	          memset((BYTE*)(*pData_addr),0,*pData_in_length);
	          pScsiCmd[4]             =*pData_in_length;
          }
         break;
		 
         case SCSI_TEST_UNIT_READY:
	  {
	        *pData_out_length    = 0;
		 *pData_in_length      = 0;
		 *pData_addr             = 0;
		
         }
	  break;
	  
	  case  SCSI_REQUEST_SENSE:
	  {
	  	*pData_out_length   = 0;
		*pData_in_length     = BYTE_COUNT_OF_SENSE_DATA;
		*pData_addr    = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
		memset((BYTE*)(*pData_addr), 0, *pData_in_length);
		pScsiCmd[4]     = *pData_in_length;
	  }
	  break;

	 case SCSI_READ_FORMAT_CAPACITIES:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_FORMAT_CAPACITY_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pScsiCmd[7]         = midlo_byte_of_dword(*pData_in_length);
            pScsiCmd[8]         = lo_byte_of_dword(*pData_in_length);
        }
        break;

        case SCSI_READ_CAPACITY:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_READ_CAPACITY_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            //pScsiCmd[4]         = *pData_in_length;
        }
        break;

        case SCSI_MODE_SENSE_6:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_MODE_SENSE_6_LEN;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pScsiCmd[2]         = TIMER_AND_PROTECT_PAGE;
            pScsiCmd[4]         = BYTE_COUNT_OF_MODE_SENSE_6_LEN;
        }
        break;

        case SCSI_MODE_SENSE_10:
        {
            *pData_out_length   = 0;
            *pData_in_length    = BYTE_COUNT_OF_MODE_SENSE_10_LEN_CBI;
            *pData_addr         = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            pScsiCmd[2]         = TIMER_AND_PROTECT_PAGE;
            pScsiCmd[7]         = hi_byte_of_word(*pData_in_length);
            pScsiCmd[8]         = lo_byte_of_word(*pData_in_length);
        }
        break;

        case SCSI_READ_10:
        {
            transfer_length = *pData_in_length >> bSectorExp;
            pScsiCmd[2]     = hi_byte_of_dword(dLBA);
            pScsiCmd[3]     = midhi_byte_of_dword(dLBA);
            pScsiCmd[4]     = midlo_byte_of_dword(dLBA);
            pScsiCmd[5]     = lo_byte_of_dword(dLBA);
            pScsiCmd[7]     = hi_byte_of_word(transfer_length);
            pScsiCmd[8]     = lo_byte_of_word(transfer_length);
        }
        break;

        case SCSI_WRITE_10:
        {
            transfer_length = *pData_out_length >> bSectorExp;
            pScsiCmd[2]     = hi_byte_of_dword(dLBA);
            pScsiCmd[3]     = midhi_byte_of_dword(dLBA);
            pScsiCmd[4]     = midlo_byte_of_dword(dLBA);
            pScsiCmd[5]     = lo_byte_of_dword(dLBA);
            pScsiCmd[7]     = hi_byte_of_word(transfer_length);
            pScsiCmd[8]     = lo_byte_of_word(transfer_length);
        }
        break;
		
        case 0xff: // test
        {
            if (*pData_in_length == 0)
            *pData_in_length = 12;

            *pData_addr = (DWORD)allo_mem((DWORD)*pData_in_length, eWhichOtg);
            pScsiCmd[7] = midlo_byte_of_dword(*pData_in_length);
            pScsiCmd[8] = lo_byte_of_dword(*pData_in_length);
        }
        break;	

    }
}

static SWORD UsbhMsdcWrite10(  BYTE                        bLun,
                        DWORD                       dwLba,
                        DWORD                       dwCount,
                        DWORD                       dwBuffer,
                        WHICH_OTG                   eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pAppMsdc    = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);    
    SWORD                       err             = USB_NO_ERROR;
    SWORD                       err_req_sense   = USB_NO_ERROR;
    
    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
    pAppMsdc->bDevLun       = bLun;
    pAppMsdc->dDataInLen    = 0;
    pAppMsdc->dDataOutLen   = dwCount << pAppMsdc->bSectorExp;
    pAppMsdc->dDataBuffer   = (DWORD)(dwBuffer|0xa0000000);
    pAppMsdc->dLba          = dwLba;
    
    UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)pAppMsdc->dDataBuffer,
                                        (DWORD)((DWORD)pAppMsdc->dDataBuffer+pAppMsdc->dDataOutLen),
                                        eWhichOtg);
    err = UsbhMsdcCommand(SCSI_WRITE_10, eWhichOtg);
    if (err == USB_NO_ERROR)
    {
        err = PASS;
    }
    else
    {
        MP_DEBUG1("ERROR:UsbhMsdcWrite10:err = %x", err);
        //UsbHostFinal();
        err = FAIL;
        if (pAppMsdc->dEventSet[bLun] == 1)
        {
            // set event to main to un-mount
            Ui_SetCardOutCodeByLun(pAppMsdc->bDevLun, eWhichOtg);
            //EventSet(UI_EVENT, EVENT_CARD_OUT);
            pAppMsdc->dEventSet[bLun] = 0;
            pAppMsdc->dPresent[bLun] = 0;
        }
    /*
        if (err_req_sense == USB_NO_ERROR)
        {
            if (pAppMsdc->bEventSet[pAppMsdc->bDevLun] == 1 ||
                pAppMsdc->dRequestSenseCode[pAppMsdc->bDevLun] != USBH_NO_SENSE)
            {
                if (pAppMsdc->dRequestSenseCode[pAppMsdc->bDevLun] != WRITE_PROTECTED_MEDIA)
                {
                    // set event to main to un-mount
                    Ui_SetCardCodeByLun(pAppMsdc->bDevLun);
                    EventSet(UI_EVENT, EVENT_CARD_OUT);
                    pAppMsdc->bEventSet[pAppMsdc->bDevLun] = 0;
                    pAppMsdc->bPresent[pAppMsdc->bDevLun] = 0;
                }
                
                pAppMsdc->dRequestSenseCode[pAppMsdc->bDevLun] = USBH_NO_SENSE;
            }
        }
        else
        {
            MP_DEBUG("UsbhMsdcWrite10:RequestSense failed");
            SetHostToResetDeviceFlag(eWhichOtg);
        }
    */
    }

    UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
    SetCheckMediaPresentTimerFlag(CMP_START_TIMER, eWhichOtg);
    return err;
}

#endif //SC_USBHOST

