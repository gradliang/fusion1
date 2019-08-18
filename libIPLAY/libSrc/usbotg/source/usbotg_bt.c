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
* Filename      : usbotg_bt.c
* Programmer(s) : Rick Chen
* Created		: 2008/06/27 
* Description	: 
*******************************************************************************
*/
#define LOCAL_DEBUG_ENABLE 0
/*
// Include section 
*/


#include "global612.h"
#include "mpTrace.h"

#include "usbotg_ctrl.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_sm.h"
#include "usbotg_host_msdc.h"
//#include "usbotg_api.h"
//rick BT
//#include "usbotg_ctrl.h"
#include "usbotg_bt.h"

#include "taskid.h"
#include "os.h"
#include "devio.h"
#include "ui.h"


#if (SC_USBHOST==ENABLE)
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
#include "BtApi.h"
//rick BT
#define BULK_IN_LENGTH 4096
#define BULK_OUT_LENGTH 4096
#if USBOTG_HOST_ISOC == 0
BYTE gBT_Buffer[BULK_IN_LENGTH*4];
#else
static PST_USB_OTG_HOST_ISOC_DES UsbOtgHostBtGetIsocInDes (WHICH_OTG eWhichOtg);
static void UsbOtgHostBtIsocInDataProcess(WHICH_OTG eWhichOtg);
static PST_USB_OTG_HOST_ISOC_DES UsbOtgHostBtGetIsocOutDes (WHICH_OTG eWhichOtg);
static void UsbOtgHostBtIsocDataIn2(WHICH_OTG eWhichOtg);
#endif
BYTE *gpBT_Buffer;
BYTE *gpBT_Buffer1;


BYTE *gpBT_Buffer_Bulk_In;
BYTE *gpBT_Buffer_Bulk_Out;
BYTE *gpBT_Buffer_Int_In;
volatile static DWORD g_BT_USB_EVENT = 0;


void RegisterBpiCallback(BtPlugInCallback callback)
{
    BpiCallback = callback;
}
void RegisterBpoCallback(BtPlugOutCallback callback)
{
    BpoCallback = callback;
}
void DeRegisterBpiCallback(void)
{
    BpiCallback = 0;
}
void DeRegisterBpoCallback(void)
{
    BpoCallback = 0;
}


static void UsbOtgBTActiveInterrupt(WHICH_OTG eWhichOtg123)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    Host20_BufferPointerArray_Structure aTemp;

    pUsbhDevDes->psAppClass->bIntDataInLen = \
        pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum]->bMaxPacketSize;

    memset(&gpBT_Buffer_Int_In[0],0x00,16);
    pUsbhDevDes->psAppClass->dIntDataBuffer = (DWORD)&gpBT_Buffer_Int_In[0];
    aTemp.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dIntDataBuffer);
    aTemp.BufferPointerArray[1] = 0;	
    aTemp.BufferPointerArray[2] = 0;	
    aTemp.BufferPointerArray[3] = 0;
    aTemp.BufferPointerArray[4] = 0;
    flib_Host20_Issue_Interrupt_Active( pUsbhDevDes->psAppClass->bIntInQHDArrayNum,
                                        pUsbhDevDes->psAppClass->bIntDataInLen, 
                                        (&aTemp.BufferPointerArray[0]), 
                                        0, 
                                        OTGH_DIR_IN,
                                        eWhichOtg);

}


static SWORD BTPipe0Out(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes,BYTE *pbBuffer,DWORD dwLength)
{
    DWORD   data_addr;
    SDWORD   err = 0;
    WORD    val=0;
    DWORD   bufferRounding = 1;
    USB_CTRL_REQUEST	ctrl_request;
    BYTE    bLen ;
	bLen = dwLength;

    WHICH_OTG eWhichOtg = BT_USB_PORT;
        
    SetupBuilder (  &ctrl_request,
                    (BYTE) (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_DEVICE),
                    (BYTE) 0x00,
                    (WORD) 0x00,
                    (WORD) 0,
                    (WORD) bLen);
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        bLen,
                        pbBuffer,
                        &ctrl_request,
                        eWhichOtg);
    if (err != 0)
    {
        MP_DEBUG("ERROR:BTPipe0Out() fail");
        MP_DEBUG1("err = %x", err);
    }
    return err;
}

static void UsbOtgHostBTActive (WHICH_OTG eWhichOtgArg)
{

    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	BYTE *pbuffer;
	BYTE index;
	BYTE length;
    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState)
    {
        case BT_BULK_IN_START_STATE:
        {
            Host20_BufferPointerArray_Structure aTemp;
            aTemp.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dDataBuffer);
            aTemp.BufferPointerArray[1] = 0;	
            aTemp.BufferPointerArray[2] = 0;	
            aTemp.BufferPointerArray[3] = 0;
            aTemp.BufferPointerArray[4] = 0;		
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkInQHDArrayNum,
											BULK_IN_LENGTH,
                                            (&aTemp.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_IN,
                                            eWhichOtg);
                                           
        }
        break;

        case BT_BULK_OUT_START_STATE:
        {
			length = pUsbhDevDes->psAppClass->dDataOutLen;
			pbuffer = pUsbhDevDes->psAppClass->dDataOutBuffer; 	
#if 0//def MPX_BULK_OUT_MESSAGE

            MP_DEBUG("BULK OUT Active");                                            
			for(index=0;index<length;index++)
			{
				UartOutValue(pbuffer[index],2);
			}
			UartOutText("\r\n");
	
#endif			

            Host20_BufferPointerArray_Structure aTemp;
		
            aTemp.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dDataOutBuffer);
            aTemp.BufferPointerArray[1] = 0;	
            aTemp.BufferPointerArray[2] = 0;	
            aTemp.BufferPointerArray[3] = 0;
            aTemp.BufferPointerArray[4] = 0;		
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum,
                                            pUsbhDevDes->psAppClass->dDataOutLen, 
                                            (&aTemp.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_OUT,
                                            eWhichOtg);

        }
        break;

        case BT_ISO_IN_START_STATE:

        break;		

        case BT_ISO_OUT_START_STATE:

        break;		


        default:
        {
        }
        break;
    }
}

static void UsbOtgHostBTIoc (WHICH_OTG eWhichOtgArg)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
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

    ST_MCARD_MAIL   *pSendMailDrv;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
    DWORD index,count;
    BYTE * pbuffer;
    DWORD dwBulkOnlyState =0;
    BYTE i,j;

    if (pUsbhDevDes->dwWhichBulkPipeDone&0xffff)
        dwBulkOnlyState = BT_BULK_OUT_DONE_STATE;
    else if (pUsbhDevDes->dwWhichBulkPipeDone&0xffff0000)
        dwBulkOnlyState = BT_BULK_IN_DONE_STATE;
    else
        MP_DEBUG("Bulk IOC error!!");
        
_BT_TD_DONE_:    

    switch (dwBulkOnlyState)
    {
        case BT_BULK_IN_DONE_STATE:

            ResetBtUsbEvent(MPX_BT_USB_BULK_IN_EN);
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[0], eWhichOtg);            
            if(GetBtOn()==1)
    		EventSet(USB_BT_EVENT, MPX_BT_USB_BULK_IN_DONE);     		
            else
            {
                mpDebugPrint("\r\n bt is off , ignore this usb bulk in event");
        }

        break;
		
        case BT_BULK_OUT_DONE_STATE:

            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[0], eWhichOtg);            
            if(GetBtOn()==1)
    		EventSet(USB_BT_EVENT, MPX_BT_USB_BULK_OUT_DONE);     		
            else
            {
                mpDebugPrint("\r\n bt is off , ignore this usb bulk out event");
        }
            break;

        case BT_ISO_IN_DONE_STATE:
            if(GetBtOn()==1)
    		EventSet(USB_BT_EVENT, MPX_BT_USB_ISO_IN_DONE);     		
            else
            {
                mpDebugPrint("\r\n bt is off , ignore this usb iso in event");
            }
            break;

        case BT_ISO_OUT_DONE_STATE:

            if(GetBtOn()==1)
    		EventSet(USB_BT_EVENT, MPX_BT_USB_ISO_OUT_DONE);     		
            else
            {
                mpDebugPrint("\r\n bt is off , ignore this usb iso out event");
            }
            
            break;
		
        default:
        {
			MP_DEBUG("! error !");
        }
        break;
    }

}



static void UsbOtgHostBTSetupIoc(WHICH_OTG eWhichOtgArg)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
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

   	if((GetBtUsbEvent()& MPX_BT_USB_INIT_EN) == MPX_BT_USB_INIT_EN)
   	{        
        if(GetBtOn()==1) 
   		EventSet(USB_BT_EVENT, MPX_BT_USB_CONTROL_DONE);  				
        else
        {
            mpDebugPrint("\r\n bt is off , ignore this usb control done event");
        }
        
	}
	else
	{
        SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg),eWhichOtg); // for next setup command
    }

}

static void UsbOtgHostBTInterruptIoc(WHICH_OTG eWhichOtgArg)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
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
    
   	BYTE *pbuf;
   	WORD len;
   	WORD count=1;
   	pbuf = pUsbhDevDes->psAppClass->dIntDataBuffer;

   	len = 16;
    ResetBtUsbEvent(MPX_BT_USB_INTERRUPT_EN);
    BYTE i=0;	

    flib_Host20_Send_qTD_Done(pUsbhDevDes->hstInterruptInqHD[0], eWhichOtg);    

	if((GetBtUsbEvent()& MPX_BT_USB_INIT_EN)== 0)
	{
		MP_DEBUG("Int Ioc Mail Back");        
    	SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next setup command
	}
	else
	{
        if(GetBtOn()==1)
        {
    	EventSet(USB_BT_EVENT, MPX_BT_USB_INTERRUPT_GET); 
        }
        else
        {
            mpDebugPrint("\r\n bt is off , ignore this usb interrupt event");
            EnBTInterruptRead();
        }
//        TaskYield();
	}
}

static SDWORD UsbBTTaskWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent)
{
	SDWORD ret;
	if (dwNextEvent)
	{
		*pdwEvent = dwNextEvent;
		ret = OS_STATUS_OK;
	}
	else
	{
		ret = EventWait(USB_BT_EVENT, 0x7fffffff, OS_EVENT_OR, pdwEvent);
	}
	return ret;
}

static BYTE bNext_Flow = 0;

static void Next_Flow(void)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD  osSts;
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    MP_DEBUG1("No interrupt data coming in , bNext_Flow = %d",bNext_Flow);

    if(bNext_Flow == 1)
    {
        MP_DEBUG("Send mail to class task BT_CLEAR_FEATURE_BULK_IN_START");
        pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
        pSendMailDrv->wCurrentExecutionState   = BT_CLEAR_FEATURE_BULK_IN_START;
        pSendMailDrv->wStateMachine            = BT_INIT_SM;        
        osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
        
    }
        
}
static void Add_Next_Flow(void)
{
    MP_DEBUG(" Add Next Flow");
    bNext_Flow = 1;
    SysTimerProcAdd(1000,Next_Flow,1);

}
static void UsbBTTask(void)
{
	DWORD dwEvent, dwNextEvent;
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

	dwNextEvent = 0;
	while (1)
	{
		if (UsbBTTaskWaitEvent(&dwEvent, dwNextEvent) == OS_STATUS_OK)
		{
			USBDriverProcess(dwEvent);
		}
	}

	TaskTerminate(USB_BT_TASK); // this will never be excuted
}
static BYTE IntCount = 0;
static WORD IntLen = 0;
static BYTE IntState = 0;
#define IntIdel 0
#define IntData 1

#define ABANDON_INTERRUPT   DISABLE
#define ABANDON_INTERRUPT_COUNT   1


void BTInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtgArg)
{
    static BYTE try_times = 0;
    SWORD       err = 0;
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR     pMsdcDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    //ST_MCARD_MAIL   *pReceiveMailDrv;
    ST_MCARD_MAIL   receiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    BYTE            mail_id;
    WORD            mcard_id = 0;

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    mcard_id = pSendMailDrv->wMCardId;
    MP_DEBUG1("BTSM - sts is %d" , receiveMailDrv.wCurrentExecutionState);
    pMsdcDes->bDevLun = bMcardTransferID; 
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case BT_INIT_START_STATE :
        {

//rick 0319
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))      
//            BtUsbTaskInit();
#endif  


            MP_DEBUG("BT_INIT_START_STATE");
            if(gpBT_Buffer_Int_In == 0)
                gpBT_Buffer_Int_In = allo_mem(16,eWhichOtg);            
            MP_DEBUG1("gpBT_Buffer_Bulk_Out = 0x%x",gpBT_Buffer_Bulk_Out);
            MP_DEBUG1("gpBT_Buffer_Int_In = 0x%x",gpBT_Buffer_Int_In);	
            pUsbhDevDes->psAppClass = pMsdcDes;     
            pUsbhDevDes->fpAppClassBulkActive       = UsbOtgHostBTActive;
            pUsbhDevDes->fpAppClassBulkIoc          = UsbOtgHostBTIoc;	
            pUsbhDevDes->fpAppClassSetupIoc         = UsbOtgHostBTSetupIoc;
            pUsbhDevDes->fpAppClassInterruptIoc     = UsbOtgHostBTInterruptIoc ;
            pUsbhDevDes->fpAppClassInterruptActive  = UsbOtgBTActiveInterrupt;            
#if USBOTG_HOST_ISOC == 1
            pUsbhDevDes->fpGetIsocInDesForIsoInActive   = UsbOtgHostBtGetIsocInDes;
            pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = UsbOtgHostBtIsocInDataProcess;
            pUsbhDevDes->fpGetIsocOutDesForIsoOutActive = UsbOtgHostBtGetIsocOutDes;
            pUsbhDevDes->fpSendIsocInDataTo             = UsbOtgHostBtIsocDataIn2;
#endif            
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_SET_INTERFACE_STATE;
            OTGH_PT_Bulk_Init(eWhichOtg);             
    }
    break;
    case BT_SET_INTERFACE_STATE :
    {   
//MP_DEBUG("SETUP_SET_INTERFACE_STATE");
        pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
        pSendMailDrv->wStateMachine             = BT_INIT_SM;
        pSendMailDrv->wCurrentExecutionState    = BT_SET_INTERFACE_DONE_STATE;
#if USBOTG_HOST_ISOC == 1
        PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
        //pIsocInEpDes = GetBestIsocDes(USB_CLASS_WIRELESS_BT, USB_DIR_IN, eWhichOtg);
        pIsocInEpDes   = GetIsocEpDes(USB_CLASS_WIRELESS_BT, USB_DIR_IN, 2, eWhichOtg); // the max packet size of isoc in ep3 is 17
        err = SetupSetInterface((WORD)pIsocInEpDes->bInterfaceNumber,
                                (WORD)pIsocInEpDes->bAlternativeSetting,
                                eWhichOtg);
#else            
        err = SetupSetInterface((WORD)1,
                                (WORD)0,
                                eWhichOtg);
#endif                                    
    }
    break;
    case BT_SET_INTERFACE_DONE_STATE :
    {
//MP_DEBUG("SETUP_SET_INTERFACE_DONE_STATE");	
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_HCI_COMMAND_STATUS_EVENT_STATE;			
    }
    break;
    case BT_HCI_COMMAND_STATUS_EVENT_STATE :
    {
    MP_DEBUG("SETUP_HCI_COMMAND_STATUS_EVENT_STATE");	    

        //******************
        // enable interrupt pipe
        //******************

            flib_Host20_Interrupt_Init(eWhichOtg);
            ClearBtUsbEvent();
            SetBtUsbEvent(MPX_BT_USB_PLUG_IN);
ComEventState:
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_HCI_COMMAND_STATUS_EVENT_DONE_STATE;	

 #if ABANDON_INTERRUPT == 1
            if ((GetBtUsbEvent()& MPX_BT_USB_INTERRUPT_EN) == 0)
            {
                SetBtUsbEvent(MPX_BT_USB_INTERRUPT_EN);
                EventSet(UsbOtgHostDriverEventIdGet(BT_USB_PORT), EVENT_EHCI_ACTIVE_INTERRUPT);
            }
            if((IntState == IntIdel)&&(bNext_Flow == 0))
            Add_Next_Flow();
            else
            {
                //do not send mail to next state machine , Ioc will send.
                return ;
            }
#else
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_GET_STATUS_STATE;	
#endif

            
        }
        break;
        case BT_HCI_COMMAND_STATUS_EVENT_DONE_STATE :
        {
//MP_DEBUG("SETUP_HCI_COMMAND_STATUS_EVENT_DONE_STATE");	
            WORD i;
            BYTE *pibuf;
            pibuf = pUsbhDevDes->psAppClass->dIntDataBuffer;

            ResetBtUsbEvent(MPX_BT_USB_INTERRUPT_EN);
            if(IntState == IntIdel)
            {
                IntLen = *(pibuf+1);
            #if LOCAL_DEBUG_ENABLE == 1
                mpDebugPrint("");
                for(i=0;i<pUsbhDevDes->psAppClass->bIntDataInLen;i++)
                {
                    UartOutValue(*(pibuf+i),2);
                    UartOutText(" ");
                }
                mpDebugPrint("");                    
            #endif                

                if(IntLen >= 15)
                {
                    IntLen -= 14;
                    IntState = IntData;
                    goto ComEventState;
                }
            }
            else
            {
            #if LOCAL_DEBUG_ENABLE == 1
                mpDebugPrint("");
                for(i=0;i<pUsbhDevDes->psAppClass->bIntDataInLen;i++)
                {
                    UartOutValue(*(pibuf+i),2);
                    UartOutText(" ");
                }
                mpDebugPrint("");                    
            #endif
                if(IntLen > 16)
                {
                    IntLen -= 16;
                    goto ComEventState;                    
                }
            }
            IntState = IntIdel;
            IntLen = 0;
            IntCount ++;
            if(IntCount < ABANDON_INTERRUPT_COUNT)
                goto ComEventState;
            
            IntCount = 0;
            bNext_Flow = 0;
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_GET_STATUS_STATE;
        }
        break;
        case BT_GET_STATUS_STATE :
        {
//MP_DEBUG("SETUP_GET_STATUS_STATE");	
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_GET_STATUS_DONE_STATE;
            err = SetupGetSatus(eWhichOtg);
        }
        break;
        case BT_GET_STATUS_DONE_STATE :
        {
            pSendMailDrv->wCurrentExecutionState   = BT_IDENTIFY_START_STATE;
            pSendMailDrv->wStateMachine            = BT_INIT_SM;
            free1(pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
        }
        break;

        case BT_CLEAR_FEATURE_BULK_IN_START :
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_CLEAR_FEATURE_BULK_IN_DONE;
            err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress, eWhichOtg);

        }
        break;

        case BT_CLEAR_FEATURE_BULK_IN_DONE :
        {
            pSendMailDrv->wCurrentExecutionState   = BT_CLEAR_FEATURE_BULK_OUT_START;
            pSendMailDrv->wStateMachine            = BT_INIT_SM;
        }
        break;

        case BT_CLEAR_FEATURE_BULK_OUT_START :
        {
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = BT_INIT_SM;
            pSendMailDrv->wCurrentExecutionState    = BT_CLEAR_FEATURE_BULK_OUT_DONE;
            err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress, eWhichOtg);
        }
        break;

        case BT_CLEAR_FEATURE_BULK_OUT_DONE :
        {
            pSendMailDrv->wCurrentExecutionState   = BT_IDENTIFY_START_STATE;
            pSendMailDrv->wStateMachine            = BT_INIT_SM;
        }
        break;

        
        case BT_IDENTIFY_START_STATE:
        {
            SetBtUsbEvent(MPX_BT_USB_INIT_EN);
#if USBOTG_HOST_ISOC == 1
            // for isoc
            gpBT_Buffer =  UsbOtgHostGetDataPage(eWhichOtg);
            if (gpBT_Buffer == 0)
                MP_ALERT("%s:%d:NO Page Buffer", __FUNCTION__, __LINE__);

            gpBT_Buffer1 =  UsbOtgHostGetDataPage(eWhichOtg);        
            if (gpBT_Buffer1 == 0)
                MP_ALERT("%s:%d:NO Page Buffer", __FUNCTION__, __LINE__);
#else
            DWORD offset;
            gpBT_Buffer = ((DWORD)gBT_Buffer | 0xa0000000);
            offset = (0x1000-((DWORD)gpBT_Buffer & 0xfff));
            gpBT_Buffer =  ((DWORD)gpBT_Buffer +offset);
            gpBT_Buffer1 =  ((DWORD)gpBT_Buffer +BULK_IN_LENGTH);        
            mOTG20_Wrapper_SwapBufferStart2_Set((DWORD)&gpBT_Buffer[0]);
            mOTG20_Wrapper_SwapBufferEnd2_Set((DWORD)&gpBT_Buffer[BULK_IN_LENGTH*2]);
            mwOTG20_Wrapper_SwapBUffer2Enable_Set();
#endif //#if USBOTG_HOST_ISOC == 1
            MP_DEBUG("indentify gpBT_Buffer = %x",gpBT_Buffer);
            MP_DEBUG("indentify gpBT_Buffer1 = %x",gpBT_Buffer1);
            mpDebugPrint("Send BT dongle plug in event to UI");

#if ((BLUETOOTH == ENABLE) && (Make_TESTCONSOLE == 0))
            EventSet(UI_EVENT,EVENT_BT_DONGLE_IN);
#else
            if(BpiCallback != 0)
                BpiCallback();
#endif
        }
        break;

    }
    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    if (err == USB_NO_ERROR)
    {
         if (
            (receiveMailDrv.wCurrentExecutionState == BT_INIT_START_STATE)                      ||
            (receiveMailDrv.wCurrentExecutionState == BT_SET_INTERFACE_DONE_STATE)              ||
#if ABANDON_INTERRUPT == 0
            (receiveMailDrv.wCurrentExecutionState == BT_HCI_COMMAND_STATUS_EVENT_STATE)        ||
#endif
            (receiveMailDrv.wCurrentExecutionState == BT_HCI_COMMAND_STATUS_EVENT_DONE_STATE)   ||
            (receiveMailDrv.wCurrentExecutionState == BT_CLEAR_FEATURE_BULK_IN_DONE)            ||            
            (receiveMailDrv.wCurrentExecutionState == BT_CLEAR_FEATURE_BULK_OUT_DONE)           ||                        
            (receiveMailDrv.wCurrentExecutionState == BT_GET_STATUS_DONE_STATE))
        {
            SDWORD  osSts;
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("BTSM err = %d", err);
    }
}

void BtUsbTaskInit(void)
{
    SDWORD sdwRetVal;
    sdwRetVal = OS_STATUS_OK;
    sdwRetVal = EventCreate(USB_BT_EVENT, (OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR), 0);
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- OTGHOST usb bt event create fail");
        return sdwRetVal;
    }
    sdwRetVal = TaskCreate(USB_BT_TASK, UsbBTTask, CONTROL_PRIORITY, 0x1000);    
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- OTGHOST Usb BT Task create fail");
        return sdwRetVal;
    }
    sdwRetVal = TaskStartup(USB_BT_TASK);
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- OTGHOST Usb BT Task startup fail");
        return sdwRetVal;
    }
}

//rick 0319
void BtUsbTaskDeInit(void)
{
    SDWORD sdwRetVal;
    sdwRetVal = OS_STATUS_OK;
    sdwRetVal = EventDestroy(USB_BT_EVENT);
    if (sdwRetVal != 0)
    {
        mpDebugPrint("-E- OTGHOST usb bt event destroy fail");
//        return sdwRetVal;
    }
    TaskTerminate(USB_BT_TASK);    

}
void EnBTBulkInRead(void)
{
    if((GetBtUsbEvent() & MPX_BT_USB_BULK_IN_EN) != MPX_BT_USB_BULK_IN_EN)
    {
        WHICH_OTG eWhichOtg = BT_USB_PORT;
        PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
        PST_APP_CLASS_DESCRIPTOR    pAppDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);	
        pAppDes->dwBulkOnlyState   = BT_BULK_IN_START_STATE;
        pUsbhDevDes->psAppClass     = pAppDes;
        memset(&gpBT_Buffer[0],0,BULK_IN_LENGTH);
        pUsbhDevDes->psAppClass->dDataBuffer = (DWORD)&gpBT_Buffer[0];
        SetBtUsbEvent(MPX_BT_USB_BULK_IN_EN);
        EventSet(UsbOtgHostDriverEventIdGet(BT_USB_PORT), EVENT_EHCI_ACTIVE_BULK);	
    }
    else
    {
        mpDebugPrint("Warning : Can not enable bulk in 0x%x",GetBtUsbEvent());
    }
    
}

DWORD EnBTBulkOutWrite(BYTE *pbBuffer,DWORD dwlength)
{
    WORD	bi;
    BYTE *pbuf;
    pbuf = pbBuffer;

#if 0//def BULK_OUT_MESSAGE
    UartOutText("%%%%%%%%%%%%\r\n");
    UartOutText("%    BT    %\r\n");		
    UartOutText("% Bulk Out % \r\n");
    UartOutText("%%%%%%%%%%%%\r\n");
    for(bi=0;bi<(dwlength);bi++)
    {
        if((bi == 4) ||
        (bi == 8) ||
        (bi == 64) ||
        (bi == 128) ||
        (bi == 192) ||
        (bi == 256) ||           
        (bi == 320) ||           
        (bi == 384) ||           
        (bi == 448) ||           
        (bi == 512) ||                      
        (bi == 576)                                  
        )
        {
            UartOutText("\r\n");
            UartOutText("\r\n");            
        }
        UartOutValue(*(pbuf+bi),2);
        UartOutText(" ");            
    }
    UartOutText("\r\n");
#endif

    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);	
    pAppDes->dwBulkOnlyState   = BT_BULK_OUT_START_STATE;
    pUsbhDevDes->psAppClass     = pAppDes;
    pUsbhDevDes->psAppClass->dDataOutLen= dwlength;
    memcpy(&gpBT_Buffer1[0], pbBuffer, dwlength);
    pUsbhDevDes->psAppClass->dDataOutBuffer= (DWORD)&gpBT_Buffer1[0];
    EventSet(UsbOtgHostDriverEventIdGet(BT_USB_PORT), EVENT_EHCI_ACTIVE_BULK);	
    if(dwlength < pUsbhDevDes->psAppClass->sBulkInDescriptor.wMaxPacketSize)
    {
        return dwlength;
    }
    else
        return pUsbhDevDes->psAppClass->sBulkInDescriptor.wMaxPacketSize;
}

BOOL BTControlWrite(BYTE * pbBuffer,DWORD dwLength)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    SWORD err = 0;
    err = BTPipe0Out(pUsbhDevDes,pbBuffer,dwLength);

    if(err == 0)
    {
        return TRUE;
    }
    else
    {
        MP_DEBUG("control write not ready");
        return FALSE;
    }
}
void EnBTInterruptRead(void)
{
    if((GetBtUsbEvent() & MPX_BT_USB_INTERRUPT_EN) != MPX_BT_USB_INTERRUPT_EN)
    {
        
        SetBtUsbEvent(MPX_BT_USB_INTERRUPT_EN);
        EventSet(UsbOtgHostDriverEventIdGet(BT_USB_PORT), EVENT_EHCI_ACTIVE_INTERRUPT);
    }
    else
    {
        mpDebugPrint("Warning : Can not enable interrupt 0x%x 0x%x",&g_BT_USB_EVENT,GetBtUsbEvent());
    }
}
BYTE CopyInterruptToBuffer(BYTE *pbBuffer,WORD *leftcount)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE *pbBufferS;
    WORD i,j,maxlen,copybyte;
    maxlen = pUsbhDevDes->psAppClass->bIntDataInLen;
    pbBufferS = pUsbhDevDes->psAppClass->dIntDataBuffer;

    if(*leftcount!=0)
        j = ((*leftcount > maxlen)?(maxlen):(*leftcount));
    else
        j=2;
    for(i=0;i<j;i++)
    {
        pbBuffer[i] = pbBufferS[i];
    }
    if(*leftcount !=0 )
    {

        *leftcount -= j; 
        return j;
    }
    j = pbBuffer[1];
    copybyte = (((j+2)> maxlen)?(maxlen):(j+2));
    *leftcount = (j+2) - copybyte;
    for(i=0;i<(copybyte-2);i++)
    {
        pbBuffer[i+2] = pbBufferS[i+2];
    }

    return copybyte;
}
WORD CopyBulkReadToBuffer(BYTE *pbBuffer,WORD *leftcount)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    BYTE *pbBufferS;
    WORD i,j,maxlen,copybyte;
    BYTE index,count;
    BYTE * pbuffer;
    maxlen = BULK_IN_LENGTH;
    pbBufferS = pUsbhDevDes->psAppClass->dDataBuffer;
    if(*leftcount!=0)
        j = ((*leftcount > maxlen)?(maxlen):(*leftcount));
    else
        j=4;
    for(i=0;i<j;i++)
    {
        pbBuffer[i] = pbBufferS[i];
    }
    if(*leftcount !=0 )
    {
        *leftcount -= j;
        return j;
    }
    j = ((pbBuffer[2])|(pbBuffer[3]<<8));
    copybyte = (((j+4)> maxlen)?(maxlen):(j+4));
    *leftcount = (j+4) - copybyte;
    for(i=0;i<(copybyte-4);i++)
    {
        pbBuffer[i+4] = pbBufferS[i+4];
    }
    return copybyte;
}
void BT_USB_Off(void)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
//    UartOutText("BT_USB_Off\r\n");
    free1(gpBT_Buffer_Int_In, eWhichOtg);
    gpBT_Buffer_Int_In = 0;
        
}
DWORD GetBtUsbEvent(void)
{
    return g_BT_USB_EVENT;
}
void SetBtUsbEvent(DWORD flag)
{
    IntDisable();    
    g_BT_USB_EVENT |= flag;
    IntEnable();
}
void ResetBtUsbEvent(DWORD flag)
{
    IntDisable();    
    g_BT_USB_EVENT &= ~flag;
    IntEnable();
}
void ClearBtUsbEvent(void)
{
    IntDisable();    
    g_BT_USB_EVENT = 0;
    IntEnable();
}

BOOL BtPlugIn(void)
{
    return(g_BT_USB_EVENT &(MPX_BT_USB_PLUG_IN|MPX_BT_USB_INIT_EN)); 
}
void UsbOtgPlugOut(WHICH_OTG eWhichOtgArg)
{
//    mpDebugPrint("UsbOtgPlugOut");
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);//(PST_USBH_DEVICE_DESCRIPTOR)pUsbhDev;
    pUsbhDevDes->fpAppClassBulkActive   = OTGH_NULL;
    pUsbhDevDes->fpAppClassBulkIoc      = OTGH_NULL;
    pUsbhDevDes->fpAppClassSetupIoc     = OTGH_NULL;
    pUsbhDevDes->fpAppClassInterruptIoc = OTGH_NULL ;  
#if USBOTG_HOST_ISOC == 1
    pUsbhDevDes->fpGetIsocInDesForIsoInActive   = OTGH_NULL;
    pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = OTGH_NULL;
    pUsbhDevDes->fpGetIsocOutDesForIsoOutActive = OTGH_NULL;
#endif            
    ClearBtUsbEvent();
    bNext_Flow =1;
    BT_USB_Off();    
    //rick 0319
//    BtUsbTaskDeInit();   
    mpDebugPrint("Send BT dongle plug out event");
#if (Make_TESTCONSOLE == 0)
    EventSet(UI_EVENT,EVENT_BT_DONGLE_OUT);    
#else
    if(BpoCallback != 0)
        BpoCallback();
#endif  

}

#if USBOTG_HOST_ISOC == 1
#define USE_BT_HCI_FLOW 0
#define BT_HCI_HEAD_LENGTH 3
#define ISOC_OUT_DATA_LENGTH 1088  // 17 x 64
#define ONE_PACKET_LENGTH_16K_DATA_RATE 17
typedef enum {
    HCI_INIT = 0,
    HCI_CH_SEG,
    HCI_BODY1_SEG,
    HCI_BODY2_SEG
}HF_HCI_SEGMENT;
// for isoc in
static BYTE bSkipUp = 0;
static SBYTE bSkipDown = 0;
static HF_HCI_SEGMENT sHciSegment = HCI_INIT;
static DWORD gdwHciData = 0;
static BYTE bIso_Start = 0;
static BYTE bIso_Count = 0;
// for isoc out
static BYTE *pTargetData = NULL;
static BYTE bRemainDataLength = 0;
static DWORD dwTargetLen = 0;
static DWORD dwSrcIndex = 0;
static BYTE bGetBuffer = 0;

static void UsbOtgHostBtIsocInDataProcess(WHICH_OTG eWhichOtg)
{
    //PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    iTD_Structure *spTempiTD;
    BYTE  *pbData;
    DWORD wiTDNum,i,j;
    DWORD pageOffset = 0;
    DWORD indexOffset = pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdInt];
    PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
    DWORD dwMaxPacketSize = 0;
    SDWORD sts = 0;
    static BYTE *pbTxData = NULL;
    static BYTE bDataTxLength = 0;
    static BYTE bSkipUpByteCnt = 0;
    static BYTE bBackupDataLength = 0;
    static BYTE bBackupData[32];
    
    DWORD dwHciTemp = 0;
    BYTE bDataOffset = 0;

    if (pUsbhDevDes->bIsoInEnable == FALSE)
        return;
    
    pIsocInEpDes    = GetIsocEpDes(USB_CLASS_WIRELESS_BT, USB_DIR_IN, 2, eWhichOtg); // the max packet size of isoc in ep3 is 17
    dwMaxPacketSize = GetIsocEndPointMaxPacketSize((DWORD)pIsocInEpDes->wMaxPacketSize);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }

    wiTDNum = pUsbhDevDes->stIsocDataStream.dwiTdNum[pUsbhDevDes->stIsocDataStream.dwiTdInt];
/*
    sts = UsbOtgHostIsocInDataEnqueueDataBuffer(&pUsbhDevDes->pbIsocInBuffer, eWhichOtg);
    if (sts != USBOTG_NO_ERROR)
    {
        MP_ALERT("%s:sts = %d pUsbhDevDes->pbIsocInBuffer is NULL!!", __FUNCTION__, sts);
        IODelay(200);
        HandlerEnable(TRUE);
        __asm("break 100");
        return;
    }
*/
    BYTE bTryCnt = 0;
    do {
        sts = UsbOtgHostIsocInDataEnqueueDataBuffer(&pUsbhDevDes->pbIsocInBuffer, eWhichOtg);
        if (sts != USBOTG_NO_ERROR)
        {
            MP_ALERT("%s:sts = %d ", __FUNCTION__, sts);
            bTryCnt++;
            if (bTryCnt >= 200)
            {
                MP_ALERT("%s:sts = %d; bTryCnt = %d", __FUNCTION__, sts, bTryCnt);
                IODelay(200);
                HandlerEnable(TRUE);
                __asm("break 100");
                return;
            }
        }
        
        TaskYield();
    } while (sts != USBOTG_NO_ERROR);



    
    pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex] = 0;
    for (i = 0; i < wiTDNum; ++i)
    {
        spTempiTD = (iTD_Structure  *)pUsbhDevDes->pstPeriodicFrameList[0].dwFrameList[indexOffset++];
        indexOffset = (indexOffset == PERIODIC_FRAME_SIZE) ? 0 : indexOffset;
        for (j = 0; j < (pUsbhDevDes->bDeviceSpeed == 0 ? 1 : 8); ++j)
        {
            SDWORD index = 0;
            index = pUsbhDevDes->stIsocDataStream.dwiTdInt*(OTGH_PT_ISO_DATABUFFER_NUM-1);
            pbData = (BYTE *)psEhci->stOtgHostIsoc.dwDataBufferArray[index] + pageOffset * dwMaxPacketSize;
            pageOffset++;            
            if (spTempiTD->ArrayStatus_Word[j].bStatus & 0x7)
            {
                //MP_DEBUG("%s: error status %x j %x; do not care for isoc tx", __FUNCTION__, spTempiTD->ArrayStatus_Word[j].bStatus, j);
                if (sHciSegment == HCI_CH_SEG)
                {
                    bSkipUp   = 1;
                    bSkipDown = 1;
                    //MP_DEBUG("%s:%d P1 %d %d", __FUNCTION__,bDataTxLength , bSkipUp, bSkipDown);
                }
                else if (sHciSegment == HCI_BODY1_SEG)
                {
                    bSkipUp   = 2;
                    bSkipDown = 0;
                    //MP_DEBUG("%s:%d P2 %d %d", __FUNCTION__,bDataTxLength , bSkipUp, bSkipDown);
                }
                else if (sHciSegment == HCI_BODY2_SEG)
                {
                    bSkipUp   = 0;
                    bSkipDown = 2;
                    //MP_DEBUG("%s:%d P3 %d %d", __FUNCTION__,bDataTxLength , bSkipUp, bSkipDown);
                }
                else
                {
                    bSkipUp   = 0;
                    bSkipDown = 0;
                    MP_ALERT("%s:1 something wrong!? sHciSegment = %d", __FUNCTION__, sHciSegment);
                    IODelay(200);
                    HandlerEnable(TRUE);
                    __asm("break 100");
                }
            }
            else if (bSkipDown > 0)
            {
                //MP_DEBUG("%s: data skip down %d", __FUNCTION__, bSkipDown);
                bSkipDown--;
            }
            else if ((pUsbhDevDes->pbIsocInBuffer != NULL) &&\
                     (spTempiTD->ArrayStatus_Word[j].bLength != dwMaxPacketSize))
            {
                dwHciTemp = pbData[0]<<16|pbData[1]<<8|pbData[2];
                if (gdwHciData == dwHciTemp)
                {
                    bDataOffset = BT_HCI_HEAD_LENGTH;
                    sHciSegment = HCI_CH_SEG;
                }
                else
                {
                    bDataOffset = 0;
                    if (sHciSegment == HCI_CH_SEG)
                    {
                        sHciSegment = HCI_BODY1_SEG;
                    }
                    else if (sHciSegment == HCI_BODY1_SEG)
                    {
                        sHciSegment = HCI_BODY2_SEG;
                    }
                    else if (sHciSegment == HCI_BODY2_SEG)
                    {
                        MP_ALERT("%s:2 something wrong!? sHciSegment = %d", __FUNCTION__, sHciSegment);
                        IODelay(200);
                        HandlerEnable(TRUE);
                        __asm("break 100");
                    }
                }

                bDataTxLength = (dwMaxPacketSize - spTempiTD->ArrayStatus_Word[j].bLength);
                if (bSkipUp == 1)
                    bSkipUpByteCnt = bDataTxLength-BT_HCI_HEAD_LENGTH; // 17 - 3 = 14
                else if (bSkipUp == 2)
                    bSkipUpByteCnt = (bDataTxLength-BT_HCI_HEAD_LENGTH)+(bDataTxLength); // 14 + 17
                else
                    bSkipUpByteCnt = 0;
                
                index = pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex];
                if (index == 0 && bSkipUp > 0)
                {
                    //MP_DEBUG("%s:a index is 0 data skip up %d", __FUNCTION__, bSkipUp);
                    bBackupDataLength = 0; // it's the biginning of the buffer, so no need to copy the data in backup.
                }
                else if (bSkipUp > 0)
                {
                    //MP_DEBUG("%s:b index %d data skip up %d", __FUNCTION__, index, bSkipUp);
                    pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex] -=\
                        bSkipUpByteCnt;
                    index -= bSkipUpByteCnt;
                    if (index < 0 || bBackupDataLength != 0)
                    {
                        MP_ALERT("%d:%s:something wrong", __LINE__, __FUNCTION__);
                        IODelay(200);
                        HandlerEnable(TRUE);
                        __asm("break 100");
                    }
                }

                pbTxData = &pUsbhDevDes->pbIsocInBuffer[index];
                if (index == 0 && bSkipUp == 0 && bBackupDataLength > 0)
                {
                    //MP_DEBUG("%s:index is 0 and no data skip up, but need to copy %d from backup buffer",\
                    //          __FUNCTION__, bBackupDataLength);
                    if (i > 0)
                    {
                        MP_ALERT("%d:%s:something wrong", __LINE__, __FUNCTION__);
                        IODelay(200);
                        HandlerEnable(TRUE);
                        __asm("break 100");
                    }

                    memcpy( pbTxData, &bBackupData[0], bBackupDataLength);
                    pbTxData += bBackupDataLength;
                    pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex] +=\
                        bBackupDataLength;
                    bBackupDataLength = 0;
                }
                
                if (bDataOffset > 0)
                {
                    pbData        += bDataOffset;
                    bDataTxLength -= bDataOffset;
                }
                
                memcpy( pbTxData, pbData, bDataTxLength);                
                bSkipUp = 0;
                pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex] +=\
                    bDataTxLength;
            }
        }
    }
    
    if ((sHciSegment != HCI_BODY2_SEG) && (sHciSegment != HCI_INIT))
    {
        //MP_DEBUG("2 sHciSegment = %d", sHciSegment);
        if (sHciSegment == HCI_CH_SEG)
        {
            bBackupDataLength = (bDataTxLength); // should be 14            
        }
        else if (sHciSegment == HCI_BODY1_SEG)
        {
            bBackupDataLength = (bDataTxLength-BT_HCI_HEAD_LENGTH)+(bDataTxLength); // 14+17
        }
        else
        {
            bBackupDataLength = 0;
            MP_ALERT("%s:3 something wrong!? sHciSegment = %d", __FUNCTION__, sHciSegment);
            IODelay(200);
            HandlerEnable(TRUE);
            __asm("break 100");
        }
    }
    else if (sHciSegment == HCI_INIT)
    {
        MP_ALERT("%s:no data", __FUNCTION__);
    }

    if (bBackupDataLength > 0)
    {
        //MP_DEBUG("copy %d bytes to backup.", bBackupDataLength);
        memcpy(&bBackupData[0], (BYTE*)((DWORD)pbTxData+bDataTxLength-bBackupDataLength), bBackupDataLength);
        pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex] -=\
            bBackupDataLength;
    }
    
// the data count is the amount of isoc IN data located in the buffer    
    pUsbhDevDes->dwIsocInDataCount += pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex];
    if (pUsbhDevDes->dwIsocInDataCount == 0)
    {
        static DWORD cnt = 0;
        cnt++;
        if (cnt == 200)
        {
            MP_ALERT("%s:dwIsocInDataCount = 0 so do nothing", __FUNCTION__);
            cnt = 0;
        }
        return;
    }

    UsbOtgHostIsocInDataEnqueueDataLength(pUsbhDevDes->dwIsocInDataCount, eWhichOtg);
    EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
    pUsbhDevDes->dwIsocInDataCount = 0;
    pUsbhDevDes->stIsocDataStream.dwBufferCurIndex =\
    (++pUsbhDevDes->stIsocDataStream.dwBufferCurIndex ==  ISOC_DATA_NUM_BUF) ?  0 : pUsbhDevDes->stIsocDataStream.dwBufferCurIndex;
}

static PST_USB_OTG_HOST_ISOC_DES UsbOtgHostBtGetIsocInDes (WHICH_OTG eWhichOtg)
{
    //return GetBestIsocDes(USB_CLASS_WIRELESS_BT, USB_DIR_IN, eWhichOtg);
    return GetIsocEpDes(USB_CLASS_WIRELESS_BT, USB_DIR_IN, 2, eWhichOtg); // the max packet size of isoc in ep3 is 17
}

static PST_USB_OTG_HOST_ISOC_DES UsbOtgHostBtGetIsocOutDes (WHICH_OTG eWhichOtg)
{
    //return GetBestIsocDes(USB_CLASS_WIRELESS_BT, USB_DIR_OUT, eWhichOtg);
    return GetIsocEpDes(USB_CLASS_WIRELESS_BT, USB_DIR_OUT, 2, eWhichOtg); // the max packet size of isoc out ep3 is 17
}

#define TRY_CNT 127
DWORD UsbOtgHostBtIsocDataOut(BYTE *pSrcData, DWORD dwSrcLength, DWORD dwHciDataLength)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
#if USE_BT_HCI_FLOW
return;
#else // USE_BT_HCI_FLOW
// get queue buffer first before processing data
    //static BYTE bOnePacketDataLength = ONE_PACKET_LENGTH_16K_DATA_RATE;
    //DWORD dwHciDataLength = 48;
    SDWORD sts = 0;
    SDWORD dwSrcLen = dwSrcLength;
    DWORD dwHciPktLen = 3+dwHciDataLength;
    BYTE bReTryCount = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(BT_USB_PORT);
    static BYTE bRemainData[ONE_PACKET_LENGTH_16K_DATA_RATE*2];

    if (pUsbhDevDes->bIsoOutEnable == FALSE)
    {
        //mpDebugPrint("<W>");
        return 0;
    }

    if(bGetBuffer == 0)
    {
        do {
            sts = UsbOtgHostIsocOutDataEnqueueGo(&pTargetData, ISOC_OUT_DATA_LENGTH, eWhichOtg);
            if (sts == USBOTG_NO_ERROR)
            {
                MP_DEBUG("%s:1set bGetBuffer to 1", __FUNCTION__);
                MP_DEBUG("0. 0x%x", pTargetData);
                bGetBuffer = 1;
            }
            else
            {
                bReTryCount++;
                //MP_ALERT("%s:1sts = %d, %d times", __FUNCTION__, sts, bReTryCount);
                if (bReTryCount >= TRY_CNT)
                {
                  MP_ALERT("1 %s:have retry %d times failed so return error %d", __FUNCTION__, bReTryCount, sts);
                  return sts;
                }
            }

            if (pUsbhDevDes->bIsoOutEnable == FALSE)
            {
                //mpDebugPrint("<X>");
                return 0;
            }
            TaskYield();
        } while (sts != USBOTG_NO_ERROR);
    }

    dwSrcLen   = dwSrcLength;
    dwSrcIndex = 0;
    while(dwSrcLen > 0)
    {
        MP_DEBUG("T = %d; S = %d", dwTargetLen, dwSrcLen);
        if (dwTargetLen >= ISOC_OUT_DATA_LENGTH)
        {
            BYTE bCopyDataLength = 0;
            bReTryCount = 0;
            if (dwTargetLen > ISOC_OUT_DATA_LENGTH)
            {
                bRemainDataLength = dwTargetLen - ISOC_OUT_DATA_LENGTH;
                bCopyDataLength = ISOC_OUT_DATA_LENGTH - (dwTargetLen - dwHciDataLength);
                MP_DEBUG("<R = %d>", bRemainDataLength);
                pTargetData[0] = GetScoHandler();
                pTargetData[1] = 0x00;
                pTargetData[2] = dwHciDataLength;
                memcpy(&pTargetData[3], pSrcData+dwSrcIndex, bCopyDataLength);
                pTargetData += (3+bCopyDataLength);
                dwSrcIndex += bCopyDataLength;
                memcpy(&bRemainData[0], pSrcData+dwSrcIndex, bRemainDataLength);
                dwSrcIndex += bRemainDataLength;
                dwSrcLen -= dwHciDataLength;
            }
            else // (dwTargetLen == ISOC_OUT_DATA_LENGTH)
            {
                MP_DEBUG("%s:dwTargetLen = %d, so do nothing.", __FUNCTION__, ISOC_OUT_DATA_LENGTH);
            }

            UsbOtgHostIsocOutDataEnqueueReady(eWhichOtg);
            EventSet(UsbOtgHostDriverEventIdGet(BT_USB_PORT), EVENT_EHCI_ACTIVE_ISO_OUT);
            //pTargetData = NULL;
            do {
                sts = UsbOtgHostIsocOutDataEnqueueGo(&pTargetData, ISOC_OUT_DATA_LENGTH, eWhichOtg);
                if (sts == USBOTG_NO_ERROR)
                {
                    MP_DEBUG("0x%x", pTargetData);
                }
                else
                {
                    bReTryCount++;
                    //MP_ALERT("2sts = %d, %d times", sts, bReTryCount);
                    if (bReTryCount >= TRY_CNT)
                    {
                      MP_ALERT("2 %s:have retry %d times failed so return error %d", __FUNCTION__, bReTryCount, sts);
                      return 0;
                    }
                }
                
                if (pUsbhDevDes->bIsoOutEnable == FALSE)
                {
                    //mpDebugPrint("<Y>");
                    return 0;
                }
                TaskYield();
            } while (sts != USBOTG_NO_ERROR);

            dwTargetLen = 0;
            //MP_DEBUG("%s:set bGetBuffer to 0", __FUNCTION__);
            //bGetBuffer  = 0;
        }
        else
        {
            if (bRemainDataLength > 0)
            {
                memcpy(pTargetData, &bRemainData[0], bRemainDataLength);
                pTargetData += bRemainDataLength;
                dwTargetLen += bRemainDataLength;
                bRemainDataLength = 0;
            }
            
            pTargetData[0] = GetScoHandler();
            pTargetData[1] = 0x00;
            pTargetData[2] = dwHciDataLength;
            memcpy(&pTargetData[3], pSrcData+dwSrcIndex, dwHciDataLength);
            pTargetData += dwHciPktLen;
            dwTargetLen += dwHciPktLen;
            dwSrcIndex  += dwHciDataLength;
            dwSrcLen    -= dwHciDataLength;
        }
        
        if (pUsbhDevDes->bIsoOutEnable == FALSE)
        {
            //mpDebugPrint("<Z>");
            return 0;
        }
    };
#endif//USE_BT_HCI_FLOW    
    return 1;
}

#if USE_BT_HCI_FLOW
#define ONE_PACKET_MAX_LENGTH 0xff
// get queue buffer first before processing data
DWORD UsbOtgHostBtIsocDataOut2(BYTE *pSrcData, DWORD dwHciDataLength)
{
    WHICH_OTG eWhichOtg = BT_USB_PORT;
    static BYTE *pTargetData = NULL;
    SDWORD sts = 0;
    static BYTE bRemainDataLength = 0;
    static BYTE bRemainData[ONE_PACKET_MAX_LENGTH];
    static DWORD dwTargetLen = 0;
    static BYTE bGetBuffer = 0;
    BYTE bReTryCount = 0;

    //HandlerEnable(TRUE);
    //__asm("break 100");
    if (dwHciDataLength % 51 != 0)
    {
        MP_ALERT("%s:MUST be the multiple of 51!! Do nothing and return!!", __FUNCTION__);
        return 0;
    }

    if(bGetBuffer == 0)
    {
        do {
            sts = UsbOtgHostIsocOutDataEnqueueGo(&pTargetData, ISOC_OUT_DATA_LENGTH, eWhichOtg);
            if (sts == USBOTG_NO_ERROR)
            {
                //MP_DEBUG("%s:1set bGetBuffer to 1", __FUNCTION__);
                //MP_DEBUG("0. 0x%x", pTargetData);
                bGetBuffer = 1;
            }
            else
            {
                bReTryCount++;
                //MP_ALERT("%s:1sts = %d, %d times", __FUNCTION__, sts, bReTryCount);
                //if (bReTryCount >= 10)
                //{
                //  MP_DEBUG("%s:have retry %d times failed so return error %d", __FUNCTION__, bReTryCount, sts);
                //  return sts;
                //}
            }
            TaskYield();
        } while (sts != USBOTG_NO_ERROR);
    }

    MP_DEBUG("T = %d", dwTargetLen);
    if (dwTargetLen >= ISOC_OUT_DATA_LENGTH)
    {
        BYTE bCopyDataLength = 0;
        if (dwTargetLen > ISOC_OUT_DATA_LENGTH)
        {
            bRemainDataLength = dwTargetLen - ISOC_OUT_DATA_LENGTH;
            bCopyDataLength = ISOC_OUT_DATA_LENGTH - (dwTargetLen - dwHciDataLength);
            MP_DEBUG("<R = %d>", bRemainDataLength);
            memcpy(pTargetData, pSrcData, bCopyDataLength);
            pTargetData += bCopyDataLength;
            memcpy(&bRemainData[0], pSrcData+bCopyDataLength, bRemainDataLength);
        }
        else // (dwTargetLen == ISOC_OUT_DATA_LENGTH)
        {
            MP_DEBUG("%s:dwTargetLen = %d, so do nothing.", __FUNCTION__, ISOC_OUT_DATA_LENGTH);
        }
        
        UsbOtgHostIsocOutDataEnqueueReady(eWhichOtg);
        EventSet(UsbOtgHostDriverEventIdGet(BT_USB_PORT), EVENT_EHCI_ACTIVE_ISO_OUT);
        ///pTargetData = NULL;
        do {
            sts = UsbOtgHostIsocOutDataEnqueueGo(&pTargetData, ISOC_OUT_DATA_LENGTH, eWhichOtg);
            if (sts == USBOTG_NO_ERROR)
            {
                //MP_DEBUG("%s:1set bGetBuffer to 1", __FUNCTION__);
                //MP_DEBUG("0. 0x%x", pTargetData);
                bGetBuffer = 1;
            }
            else
            {
                bReTryCount++;
                //MP_ALERT("%s:1sts = %d, %d times", __FUNCTION__, sts, bReTryCount);
                //if (bReTryCount >= 10)
                //{
                //  MP_DEBUG("%s:have retry %d times failed so return error %d", __FUNCTION__, bReTryCount, sts);
                //  return sts;
                //}
            }
            TaskYield();
        } while (sts != USBOTG_NO_ERROR);
        dwTargetLen = 0;
        bGetBuffer  = 0;
    }
    else
    {
        if (bRemainDataLength > 0)
        {
            memcpy(pTargetData, &bRemainData[0], bRemainDataLength);
            pTargetData += bRemainDataLength;
            dwTargetLen += bRemainDataLength;
            bRemainDataLength = 0;
        }
        
        memcpy(pTargetData, pSrcData, dwHciDataLength);
        pTargetData += dwHciDataLength;
        dwTargetLen += dwHciDataLength;
    }
    
///////////////////////////////////////////////////
//    change to BT event

//    EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
    EventSet(USB_BT_EVENT, MPX_BT_USB_ISO_OUT_DONE); 

    return 1;
}
#endif //USE_BT_HCI_FLOW

static void UsbOtgHostBtIsocDataIn2(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;
    SBYTE bRet = FAIL;
        
    if (pUsbhDevDes->bIsoInEnable == FALSE)
    {
        MP_DEBUG("%s:return", __FUNCTION__);
        return;
    }
    
    pIsocBuff = UsbOtgHostIsocInDataDequeueGo(eWhichOtg);
    if (pIsocBuff == NULL)
    {
        ;//UartOutText("<NULL>");
        //HandlerEnable(TRUE);
        //__asm("break 100");
    }
    else
    {
        //mpDebugPrint("WaveOut");
        if (pIsocBuff->dwLength == 0 || pIsocBuff->pbDataBuffer == NULL)
        {
            //HandlerEnable(TRUE);
            //__asm("break 100");
            MP_ALERT("dequeue null!!!");
            return;
        }

        #if 1 
        WaveOutProc_Stream(pIsocBuff->pbDataBuffer, pIsocBuff->dwLength);
        #else
        WaveOutProc(pIsocBuff->pbDataBuffer, pIsocBuff->dwLength);
        TaskYield();
        while(1)
        {
            if (WaveOutWriteDone() == 0)
                break;
            TaskYield();
        };
        #endif
        //UartOutText("<I>");
        UsbOtgHostIsocInDataDequeueReady(eWhichOtg);
        pIsocBuff->dwLength = 0;
    }
    
    TaskYield();
    EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
}

void UsbOtgHostBtIsocEnable(void)
{
    mpDebugPrint("%s", __FUNCTION__);
    UsbOtgHostIsocEnable(BT_USB_PORT);
    // for isoc out
    *pTargetData = NULL;
    bRemainDataLength = 0;
    dwTargetLen = 0;
    dwSrcIndex = 0;
    bGetBuffer = 0;

    // for isoc in
    gdwHciData = GetScoHandler()<<16 | 0x30;
    bSkipUp = 0;
    bSkipDown = 0;
    sHciSegment = HCI_INIT;
}

void UsbOtgHostBtIsocDisable(void)
{
    mpDebugPrint("%s", __FUNCTION__);
    UsbOtgHostIsocDisable(BT_USB_PORT);
}

void IsoDone(void)
{
//    if(GetBtOn()==1)
//        EventSet(USB_BT_EVENT, BT_USB_ISO_OUT_DONE); 
//    else
//    {
//        mpDebugPrint("\r\n bt is off , ignore this usb iso out event");
//    }
}
#endif //#if USBOTG_HOST_ISOC == 1



#endif // ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	


#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
//MPX_KMODAPI_SET(ResetBtEventWithDonglePlugIn);
//MPX_KMODAPI_SET(SetBtEventWithDonglePlugIn);
#if USBOTG_HOST_ISOC == 1
MPX_KMODAPI_SET(UsbOtgHostBtIsocDataOut);
MPX_KMODAPI_SET(UsbOtgHostBtIsocEnable);
MPX_KMODAPI_SET(UsbOtgHostBtIsocDisable);
#endif //#if USBOTG_HOST_ISOC == 1
MPX_KMODAPI_SET(BtPlugIn);
MPX_KMODAPI_SET(RegisterBpiCallback);
MPX_KMODAPI_SET(RegisterBpoCallback);
MPX_KMODAPI_SET(DeRegisterBpiCallback);
MPX_KMODAPI_SET(DeRegisterBpoCallback);
#endif // #if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))
#endif //#if (SC_USBHOST==ENABLE)

