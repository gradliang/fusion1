/** 
	This function registers the card as HOST/USER(WLAN)
	\param type of the card, function pointer
	\return returns pointer to the type requested for
*/
#define LOCAL_DEBUG_ENABLE 0

#include	"rt_config.h"
#include	"UtilTypeDef.h"
#include 	"devio.h"
#include 	"lwip_incl.h"
#include 	"rtmp_def.h"
#include    "usbotg_host.h"
#include    "usbotg_ctrl.h"
#include    "osdep_mpixel_service.h"
#include    "taskid.h"
#include    "wlan_sys.h"
#include    "os_mp52x.h"
#include    "usb.h"
#include    "rtmp.h"
//Kevin ADD
#include	"taskid.h"
#include	"net_device.h"
#include	"usbotg_wifi.h"

int urb_sema;
#define MAX_DATA_PAGE           5
#define ONE_PAGE_BYTE_CNT       4096
#define ONE_TD_MAX_BYTE_CNT     16384//20480 // 4096 * 5
//<4>.0x010(USBCMD - USB Command Register)
//#define mwHost20_USBCMD_IntThreshold_Rd()		          ((gp_UsbOtg->HcUsbCommand>>16)&0x0000FFFF)	//Bit 16~23
//#define mbHost20_USBCMD_IntThreshold_Set(bValue)		  (gp_UsbOtg->HcUsbCommand =((gp_UsbOtg->HcUsbCommand&0xFF00FFFF)|(((DWORD)(bValue))<<16)))	//Bit 16~23
//#define mbHost20_USBCMD_FrameListSize_Rd()				  ((gp_UsbOtg->HcUsbCommand>>2)&0x00000003)    //Bit 2~3
//#define mbHost20_USBCMD_FrameListSize_Set(bValue)		  ((gp_UsbOtg->HcUsbCommand=((gp_UsbOtg->HcUsbCommand&0xFFFFFFF3)|(((DWORD)(bValue))<<2))))	//Bit 2~3

#define mwHost20_Control_ForceFullSpeed_Rd()			  (gp_UsbOtg->OtgControlStatus& BIT12)
#define QTDNUM  4
extern PST_USBH_DEVICE_DESCRIPTOR gpUsbhDeviceDescriptor;
extern PRTMP_ADAPTER RTpAT;

extern void *net_buf_start;
extern void *net_buf_end;


_timer   usb_timer_head;

static BYTE bDescriptor[8] = "USB_RT73";

#pragma alignvar(4)

static void CommandProcess (void *pMcardDev);

int rtusb_control_msg(unsigned char request, unsigned char requesttype, unsigned short value, unsigned short index, unsigned char* data, unsigned short size, int timeout);
void UsbOtgBulkProcess(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes ,BYTE	bDirection);

BYTE flib_Host20_Issue_Control (BYTE bEdNum,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData);
int flib_Host20_Issue_Control_Vendor (BYTE bEdNum,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData);

SDWORD SetupCommand( PST_USBOTG_SETUP_PB pSetupPB, WORD data_in_len,
    WORD data_out_len, DWORD data_addr, PUSB_CTRL_REQUEST pCtrl_request);
	
/*
	========================================================================
	
	Routine Description: NIC initialization complete

	Arguments:

	Return Value:

	Note:
	
	========================================================================
*/
void
ListInitList(
PLM_LIST_CONTAINER pList) 
{
	pList->Link.FLink = pList->Link.BLink = (PLM_LIST_ENTRY) 0;
	pList->EntryCount = 0;
} /* ListInitList */

/******************************************************************************/
/* ListGetHead -- returns the head of the list, but does not remove it.       */
/******************************************************************************/
PLM_LIST_ENTRY 
ListGetHead(
PLM_LIST_CONTAINER pList) 
{
    return pList->Link.FLink;
} /* ListGetHead */



/******************************************************************************/
/* ListGetTail -- returns the tail of the queue, but does not remove it.      */
/******************************************************************************/
PLM_LIST_ENTRY 
ListGetTail(
PLM_LIST_CONTAINER pList) 
{
    return pList->Link.BLink;
} /* ListGetTail */



/******************************************************************************/
/* ListPushHead -- puts an element at the head of the queue.                  */
/******************************************************************************/
void
ListPushHead(
PLM_LIST_CONTAINER pList, 
PLM_LIST_ENTRY pEntry) 
{
	SemaphoreWait(urb_sema);	
    pEntry->BLink = (PLM_LIST_ENTRY) 0;
    pEntry->FLink = pList->Link.FLink;

    if(pList->Link.FLink == (PLM_LIST_ENTRY) 0) {
        pList->Link.BLink = (PLM_LIST_ENTRY) pEntry;
    } else {
        pList->Link.FLink->BLink = pEntry;
    }

    pList->Link.FLink = (PLM_LIST_ENTRY) pEntry;

    pList->EntryCount++;
  	SemaphoreRelease(urb_sema);
} /* ListPushHead */

/******************************************************************************/
/* ListPushTail -- puts an element at the tail of the list.                   */
/******************************************************************************/
void
ListPushTail(
PLM_LIST_CONTAINER pList, 
PLM_LIST_ENTRY pEntry) 
{
	SemaphoreWait(urb_sema);	

    pEntry->BLink = pList->Link.BLink;

    if(pList->Link.BLink) {
        pList->Link.BLink->FLink = pEntry;
    } else {
        pList->Link.FLink = pEntry;
		
    }

    pList->Link.BLink = pEntry;
    pEntry->FLink = (PLM_LIST_ENTRY) 0;

    pList->EntryCount++;
  	SemaphoreRelease(urb_sema);
	
} /* ListPushTail */
/******************************************************************************/
/* ListEmpty -- checks to see if a list is empty.                             */
/******************************************************************************/
char 
ListEmpty(
PLM_LIST_CONTAINER pList) 
{
    return(pList->Link.FLink == (PLM_LIST_ENTRY) 0);
} /* ListEmpty */
/******************************************************************************/
/* ListPopHead -- pops the head off the list.                                 */
/******************************************************************************/
PLM_LIST_ENTRY 
ListPopHead(
PLM_LIST_CONTAINER pList) 
{
    PLM_LIST_ENTRY pEntry;

	SemaphoreWait(urb_sema);	

    pEntry = pList->Link.FLink;

    if(pEntry) {
        pList->Link.FLink = pEntry->FLink;
        if(pList->Link.FLink == (PLM_LIST_ENTRY) 0) {
            pList->Link.BLink = (PLM_LIST_ENTRY) 0;
        } else {
            pList->Link.FLink->BLink = (PLM_LIST_ENTRY) 0;
        }

        pList->EntryCount--;
    } /* if(pEntry) */
    
  	SemaphoreRelease(urb_sema);
    return pEntry;
} /* ListPopHead */



/******************************************************************************/
/* ListPopTail -- pops the tail off a list.                                   */
/******************************************************************************/
PLM_LIST_ENTRY 
ListPopTail(
PLM_LIST_CONTAINER pList) {
    PLM_LIST_ENTRY pEntry;

    pEntry = pList->Link.BLink;

    if(pEntry) {
        pList->Link.BLink = pEntry->BLink;
        if(pList->Link.BLink == (PLM_LIST_ENTRY) 0) {
            pList->Link.FLink = (PLM_LIST_ENTRY) 0;
        } else {
            pList->Link.BLink->FLink = (PLM_LIST_ENTRY) 0;
        }

        pList->EntryCount--;
    } /* if(pEntry) */
    
    return pEntry;
} /* ListPopTail */
/******************************************************************************/
/* ListNextEntry -- gets the next entry.                                      */
/******************************************************************************/
PLM_LIST_ENTRY
ListNextEntry(
PLM_LIST_ENTRY pEntry) 
{
    return pEntry->FLink;
} /* ListNextEntry */



/******************************************************************************/
/* ListPrevEntry -- gets the previous entry.                                  */
/******************************************************************************/
PLM_LIST_ENTRY
ListPrevEntry(
PLM_LIST_ENTRY pEntry) 
{
    return pEntry->BLink;
} /* ListPrevEntry */

/******************************************************************************/
/* ListOk -- makes sure the links and counters are consistent.                */
/******************************************************************************/
char
ListOk(
PLM_LIST_CONTAINER pList) 
{
    PLM_LIST_ENTRY pEntry;
    unsigned long Count;

    Count = 0;

    pEntry = ListGetHead(pList);
    while(pEntry) {
        Count++;
        pEntry = ListNextEntry(pEntry);
    }

    if(Count != pList->EntryCount) {
        return 0;
    }

    Count = 0;

    pEntry = ListGetTail(pList);
    while(pEntry) {
        Count++;
        pEntry = ListPrevEntry(pEntry);
    }

    if(Count != pList->EntryCount) {
        return 0;
    }

    return 1;
} /* ListOk */

/******************************************************************************/
/* ListGetSize -- gets the number of elements in the list.                    */
/******************************************************************************/
unsigned long 
ListGetSize(
PLM_LIST_CONTAINER pList) 
{
    return pList->EntryCount;
} /* ListGetSize */

void RTUSB_IOInit(ST_MCARD_DEV * sDev)
{
	mpDebugPrint("\n RTUSB_IOInit!! \n");

	sDev->pbDescriptor = bDescriptor;
	sDev->dwLunNum = LUN_NUM_0;
	sDev->wMcardType = USB_RT73;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}
void NetDriverUpEventSet();
static void CommandProcess (void *pMcardDev)
{
	ST_MCARD_MAIL	*psMcardRMail;
	ST_MCARD_DEV	*pDev = pMcardDev;
	DWORD usbcmd;
	
	psMcardRMail = pDev->sMcardRMail;
	
	//mpDebugPrint("\nCommandProcess\n");

	switch (psMcardRMail->wCmd)
	{
		case INIT_CARD_CMD:
			
			mpDebugPrint("\nRT73 INIT_CARD_CMD\n");
			UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end);
			//usbcmd = mwHost20_USBCMD_IntThreshold_Rd();
			//mpDebugPrint("usbcmd %x",usbcmd);
			//usbcmd = 0x04;
			//mbHost20_USBCMD_IntThreshold_Set(usbcmd);
			//usbcmd = mbHost20_USBCMD_FrameListSize_Rd();
			//mpDebugPrint("usbcmd FrameListSize %x",usbcmd);
			//usbcmd = 0x01;
			//mbHost20_USBCMD_FrameListSize_Set(usbcmd);
			usb_rtusb_probe();
			//NetDriverUpEventSet();
			break;
		case REMOVE_CARD_CMD:
			mpDebugPrint("\nRT73 REMOVE_CARD_CMD\n");
			UsbOtgHostSetSwapBuffer2RangeDisable();
			usb_rtusb_disconnect();
			mpDebugPrint("REMOVE_CARD_CMD END\n");
			break;
		default:
			break;
	}

}
/**
 *	usb_control_msg - Builds a control urb, sends it off and waits for completion
 *	@dev: pointer to the usb device to send the message to
 *	@pipe: endpoint "pipe" to send the message to
 *	@request: USB message request value
 *	@requesttype: USB message request type value
 *	@value: USB message value
 *	@index: USB message index value
 *	@data: pointer to the data to send
 *	@size: length in bytes of the data to send
 *	@timeout: time in msecs to wait for the message to complete before
 *		timing out (if 0 the wait is forever)
 *	Context: !in_interrupt ()
 *
 *	This function sends a simple control message to a specified endpoint
 *	and waits for the message to complete, or timeout.
 *	
 *	If successful, it returns the number of bytes transferred, otherwise a negative error number.
 *
 *	Don't use this function from within an interrupt context, like a
 *	bottom half handler.  If you need an asynchronous message, or need to send
 *	a message from within interrupt context, use usb_submit_urb()
 *      If a thread in your driver uses this call, make sure your disconnect()
 *      method can wait for it to complete.  Since you don't have a handle on
 *      the URB used, you can't cancel the request.
 */
int rtusb_control_msg(unsigned char request, unsigned char requesttype,
			 unsigned short value, unsigned short index, unsigned char* data, unsigned short size, int timeout)
{
	USB_CTRL_REQUEST ctrl_request;
    int   err;
	
//    mpDebugPrint("rtusb_control_msg %x,%x", requesttype, request);
	SetupBuilder(&ctrl_request,
				 requesttype,
				 request,
				 value,
				 index,
				 size);
	//SemaphoreWait(WIFI_IO_SEMA_ID);
//	err = flib_Host20_Issue_Control(1,&ctrl_request,size,data);
	err = flib_Host20_Issue_Control_Vendor(1,(BYTE*)&ctrl_request,size,data);
	//SemaphoreRelease(WIFI_IO_SEMA_ID);
    if (err)
        mpDebugPrint("flib_Host20_Issue_Control_Vendor returns error");
	return err;

}


//====================================================================
// * Function Name: flib_Host20_Send_qTD                          
// * Description: A syncronous call
//   Case-1:1qTD
//   Case-2:2qTD
//   Case-3:3qTD above
// * Input:Type
// * OutPut: 
//====================================================================
BYTE flib_Host20_Send_qTD(qTD_Structure *spHeadqTD ,qHD_Structure *spTempqHD,DWORD wTimeOutSec)
{
	BYTE bExitLoop;
	qTD_Structure *spNewDumyqTD; 
	volatile qTD_Structure *spOldDumyqTD; 
	qTD_Structure *spReleaseqTD;    
	qTD_Structure *spReleaseqTDNext;    
	volatile qTD_Structure *spLastqTD;
	DWORD send_timer = 0;

//    mpDebugPrint("%s", __FUNCTION__);
	//<1>.Copy Head-qTD to OldDumyqTD
	spOldDumyqTD=(qTD_Structure*)(((DWORD)(spTempqHD->bOverlay_NextqTD))<<5);
	memcpy((BYTE*)spOldDumyqTD,(BYTE*)spHeadqTD,Host20_qTD_SIZE);

	//<2>.Prepare new dumy qTD      
	spNewDumyqTD=spHeadqTD;
	spNewDumyqTD->bTerminate=1;

	//<3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1
	spLastqTD=spOldDumyqTD;
	while(spLastqTD->bTerminate==0) {
		spLastqTD=(qTD_Structure*)(((DWORD)(spLastqTD->bNextQTDPointer))<<5);
	};
	spLastqTD->bNextQTDPointer=((DWORD)spNewDumyqTD)>>5;
	spLastqTD->bTerminate=0;   

	//Link Alternate qTD pointer
	spLastqTD->bAlternateQTDPointer=((DWORD)spNewDumyqTD)>>5;
	spLastqTD->bAlternateTerminate=0;   

	//<4>.Enable Timer
	//flib_Host20_TimerEnable_UnLock(1); //1sec
	//WaitMs(1);
	
	bExitLoop=0;

	//<4>.Set OldDumyqTD->Active=1 
	//gwLastqTDSendOK=0;
	//sAttachDevice.psSendLastqTD=spLastqTD;
	//sAttachDevice.bSendStatusError=0;

	// for wrapper control begin
	if (spOldDumyqTD->bTotalBytes != 0)
	{
        UsbOtgHostSetSwapBuffer1RangeEnable(spOldDumyqTD->ArrayBufferPointer_Word[0],
                                            spOldDumyqTD->ArrayBufferPointer_Word[0] + spOldDumyqTD->bTotalBytes - 1,
                                            WIFI_USB_PORT);
	}
	// for wrapper control end

	spOldDumyqTD->bStatus_Active=1;          

	//gwOTG_Timer_Counter=0;
	wTimeOutSec=100000;

	//<5>.Waiting for result
	DWORD fillme = 0;
	//DWORD *dwpt=(DWORD*)0xa0000000;
	DWORD *dwpt=(DWORD*)0x80000000;
    DWORD i = 0;
    DWORD tm = GetSysTime();
	do {
		//fillme=*dwpt;// read dummy from sdram before get statusa
		if (mwHost20Port(0x1dc)!=0) 
		{
			if (mwHost20BitRd(0x1dc,BIT0))
				mpDebugPrint("crc5 error");
			if (mwHost20BitRd(0x1dc,BIT1))
				mpDebugPrint("crc16 error");
			if (mwHost20BitRd(0x1dc,BIT2))
				mpDebugPrint("seg error");
			if (mwHost20BitRd(0x1dc,BIT3))
				mpDebugPrint("tout error");
			if (mwHost20BitRd(0x1dc,BIT4))
				mpDebugPrint("pid error");
			if (mwHost20BitRd(0x1dc,BIT5))
				mpDebugPrint("utmi error");
		}
		mwHost20Port(0x1dc)=0;
// for MP620 A version uncache cannot work
// work arround begin:
#if 0
        dwpt = (DWORD*)((DWORD)&spLastqTD + 0x100000);
        for (i = 0; i <512; i++)
        {
            fillme=*(dwpt++);
        }
#endif
// work arround end
		if (spLastqTD->bStatus_Active==0) 
		{ // spOldDumyqTD //john, should be spLastqTD
			bExitLoop=1;

			if (spLastqTD->bStatus_Halted==1)
				mpDebugPrint("Halted!!");
			//if (spLastqTD->bStatus_Buffer_Err==1)
			//	DpString("bStatus_Buffer_Err!!");
			//if (spLastqTD->bStatus_Babble==1)
			//	DpString("bStatus_Babble!!");
			//if (spLastqTD->bStatus_Transaction_Err==1)
			//	DpString("bStatus_Transaction_Err!!");
			//if (spLastqTD->bStatus_MissMicroFrame==1)
			//	DpString("bStatus_MissMicroFrame!!");
			//if (spLastqTD->bStatus_SplitState==1)
			//	DpString("bStatus_SplitState!!");
			//if (spLastqTD->bStatus_PingState==1)
			//	DpString("bStatus_PingState!!");
		}
		if (send_timer++ > wTimeOutSec)
		{
			bExitLoop=1;               
            tm = (long)GetSysTime() - (long)tm;
			mpDebugPrint(">>> Fail => Time Out for Send qTD...%d\n", tm);	
		}   
	} while(bExitLoop==0);


	if (mwOTG20_Wrapper_SwapBUffer1Enable_Rd() != 0)
		mwOTG20_Wrapper_SwapBUffer1Enable_Clr();

	//flib_Host20_TimerDisable_UnLock();

	//<6>.Checking the Result
	//if (gwLastqTDSendOK==0)
	//flib_Host20_CheckingForResult_QHD(spTempqHD);        

	//<5>.Release the all the qTD (Not include spNewDumyqTD)
	spReleaseqTD=spOldDumyqTD;
	do {
		spReleaseqTDNext=(qTD_Structure*)(((DWORD)(spReleaseqTD->bNextQTDPointer))<<5);
		flib_Host20_ReleaseStructure(Host20_MEM_TYPE_qTD,(DWORD)spReleaseqTD);
		spReleaseqTD=spReleaseqTDNext;  	
	} while(((DWORD)spReleaseqTD)!=((DWORD)spNewDumyqTD));

	//if (gwOTG_Timer_Counter>=wTimeOutSec)  
	//	return(1);
	//else 
		return (0);
}

//====================================================================
 //For Control-Single qTD// * Function Name: flib_Host20_Issue_Control                          
// * Description: 
//   <1>.Analysis the Controller Command => 3 type
//   <2>.Case-1:"Setup/In/Out' Format..."
//       (get status/get descriptor/get configuration/get interface)
//   <3>.Case-2:'Setup/In' Format...      => Faraday Driver will not need 
//       (clear feature/set feature/set address/set Configuration/set interface  ) 
//   <4>.Case-3:'Setup/Out/In'
//       (set descriptor)
// * Input: 
// * OutPut: 0: OK
//           X:>0 => Fail
//====================================================================

#if 0
BYTE flib_Host20_Issue_Control (BYTE bEdNum,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData)
{
   PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = gpUsbhDeviceDescriptor;
   qTD_Structure *spTempqTD;
   BYTE bReturnValue;
   volatile BYTE bpDataout[8];
   volatile BYTE bpDataIN[8];
   
//    mpDebugPrint("%s (%d)",__FUNCTION__, *(pbCmd+1));
   //<0>.Allocate qTD & Data Buffer
   spTempqTD    = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
   //bpDataPage   = usbbuf;//allo_mem(8);//(BYTE*)((DWORD)sAttachDevice.bDataBuffer);// | 0xa0000000);
  
   //<2.1>.Setup packet
   //<A>.Fill qTD
   spTempqTD->bPID=HOST20_qTD_PID_SETUP;                   //Bit8~9   
   spTempqTD->bTotalBytes=8;           //Bit16~30   
   spTempqTD->bDataToggle=0;            //Bit31 
   spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)bpDataout; 
   memcpy(bpDataout,pbCmd,8);
   
   //<B>.Send qTD
   bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
   if (bReturnValue>0)
      return (bReturnValue);

   //<1>.Analysis the Controller Command
   
   //mpDebugPrint("*(pbCmd+1) = %x",*(pbCmd+1));
   switch (*(pbCmd+1)) { // by Standard Request codes
      // <2>.Case-1:"Setup/In/Out' Format..."
   	  case 0:		// get status
	  case 3:		 // set feature
	  case 4:		 // set feature
//	  case 7:		// set descriptor
	  case 6:		// set descriptor
	  case 8:		// get configuration
	  case 9:		 // read eeprom		 
          mpDebugPrint("%s 2.2",__FUNCTION__);
         //<2.2>.In packet
         //<A>.Fill qTD
         spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD              
         spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9   
         spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30   
         spTempqTD->bDataToggle=1;            //Bit31 
         spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)bpDataIN; // joeluo : need to revise
         
         //<B>.Send qTD
         bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
         if (bReturnValue>0)
            return (bReturnValue);
 
		 //<C>.Waiting for result
		 dma_invalid_dcache();
		 
         memcpy(pbData,bpDataIN,hwDataSize);

          mpDebugPrint("%s 2.3",__FUNCTION__);
         //<2.3>.Out packet
         //<A>.Fill qTD
         spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD              
         spTempqTD->bPID=HOST20_qTD_PID_OUT;                   //Bit8~9   
         spTempqTD->bTotalBytes=0;           //Bit16~30   
         spTempqTD->bDataToggle=1;            //Bit31 

         //<B>.Send qTD
         bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
         if (bReturnValue>0)
            return (bReturnValue);
         break;
     
      //<3>.Case-2:'Setup/In' Format...      => Faraday Driver will not need 
         case 1:		
         case 2:		
   		 case 5:		// set address
		 case 10:  // Set LED
      	 case 11:	// set interface   		
      	 
            //<3.2>.In packet
            //<A>.Fill qTD
            spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD              
            spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9   
            spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30   
            spTempqTD->bDataToggle=1;            //Bit31 
            spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)bpDataIN; 

            //<B>.Send qTD
            bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
            if (bReturnValue>0)
               return (bReturnValue);
                
            //<C>.Copy Result
            memcpy(pbData,bpDataIN,hwDataSize);
            break;

      //<4>.Case-3:'Setup/Out/In'
	  case 7:	// get descriptor
            //<4.2>.Out packet
            //<A>.Fill qTD
            spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD 
            spTempqTD->bPID=HOST20_qTD_PID_OUT;                   //Bit8~9   
            spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30   
            spTempqTD->bDataToggle=1;            //Bit31 
            spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)bpDataout; 
            memcpy(bpDataout,pbData,hwDataSize);

			//<B>.Send qTD
            bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
            if (bReturnValue>0)
               return (bReturnValue);
                         
            //<4.3>.In packet
            //<A>.Fill qTD
            spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD 
            spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9   
            spTempqTD->bTotalBytes=0;           //Bit16~30   
            spTempqTD->bDataToggle=1;            //Bit31 

            //<B>.Send qTD
            bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
            if (bReturnValue>0)
               return (bReturnValue);
            break;

		 default:
			break;
   	}
    return (0);
}
#endif

#if 0
BYTE flib_Host20_Issue_Control_Vendor (BYTE bEdNum,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData)
{
   PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = gpUsbhDeviceDescriptor;
   qTD_Structure *spTempqTD;
   BYTE bReturnValue;
   volatile BYTE buf1[8], *bpDataout = (DWORD)buf1 | 0xa0000000;
   volatile BYTE buf2[128], *bpDataIN =(DWORD)buf2 | 0xa0000000;
   USB_CTRL_REQUEST *ctrl_request = (USB_CTRL_REQUEST *)pbCmd;
   
//    mpDebugPrint("%s (%d)",__FUNCTION__, *(pbCmd+1));
   //<0>.Allocate qTD & Data Buffer
   spTempqTD    = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
  
   //<2.1>.Setup packet
   //<A>.Fill qTD
   spTempqTD->bPID=HOST20_qTD_PID_SETUP;                   //Bit8~9   
   spTempqTD->bTotalBytes=8;           //Bit16~30   
   spTempqTD->bDataToggle=0;            //Bit31 
   //<1>.Analysis the Controller Command
   
//   if (hwDataSize > 0)
   {
       if (ctrl_request->bRequestType & USB_DIR_IN)
       {
         if (hwDataSize > sizeof buf2)
         {
             MP_ASSERT(0);
         }
         //<2.2>.In packet
         //<A>.Fill qTD
         spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD              
         if (pbData)
             spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30   
         else
         {
             MP_ASSERT(0);
             spTempqTD->bTotalBytes=byte_swap_of_word(ctrl_request->wLength);           //Bit16~30   
         }
         spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9   
         spTempqTD->bDataToggle=1;            //Bit31 
         spTempqTD->bInterruptOnComplete=0;   //Bit15   
         spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)bpDataIN;
         
         //<B>.Send qTD
         bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
         if (bReturnValue>0)
         {
            mpDebugPrint("flib_Host20_Send_qTD returns error");
            return (bReturnValue);
         }
 
         memcpy(pbData,bpDataIN,hwDataSize);

         //<2.2>.In packet
         //<A>.Fill qTD
         spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD              
         spTempqTD->bPID=HOST20_qTD_PID_OUT;                   //Bit8~9   
         spTempqTD->bTotalBytes=0;           //Bit16~30   
         spTempqTD->bDataToggle=1;            //Bit31 
         spTempqTD->bInterruptOnComplete=0;   //Bit15   
         
         //<B>.Send qTD
         bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
         if (bReturnValue>0)
            return (bReturnValue);
       }
       else
       {
           if (pbData)
           {
               //<4.2>.Out packet
               //<A>.Fill qTD
               spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);
               spTempqTD->bPID=HOST20_qTD_PID_OUT;                   //Bit8~9   
               spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30   
               spTempqTD->bDataToggle=1;            //Bit31 
               spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)bpDataout; 
               spTempqTD->bInterruptOnComplete=0;   //Bit15   
               memcpy(bpDataout,pbData,hwDataSize);

               //<B>.Send qTD
               bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
               if (bReturnValue>0)
               {
                   mpDebugPrint("flib_Host20_Send_qTD returns error");
                   return (bReturnValue);
               }
           }

         //<2.2>.In packet
         //<A>.Fill qTD
         spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD              
         spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9   
         spTempqTD->bTotalBytes=0;           //Bit16~30   
         spTempqTD->bDataToggle=1;            //Bit31 
         spTempqTD->bInterruptOnComplete=0;   //Bit15   
         
         //<B>.Send qTD
         bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->pstControlqHD[bEdNum],5);
         if (bReturnValue>0)
            return (bReturnValue);
       }
   }

    return (0);
}
#else
int flib_Host20_Issue_Control_Vendor (BYTE bEdNum,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = gpUsbhDeviceDescriptor;
    DWORD   data_addr;
    int   err;
    WORD   in_len, out_len;
    USB_CTRL_REQUEST ctrl_request;
//    MP_DEBUG("flib_Host20_Issue_Control_Vendor");

    if (pUsbhDevDes->sSetupPB.dwSetupState != SETUP_IDLE_STATE)
    {
        return -2;
    }

    memcpy(&ctrl_request, pbCmd, sizeof(ctrl_request));

    if (ctrl_request.bRequestType & USB_DIR_IN)
    {
//        MP_DEBUG("USB_DIR_IN len=%d", hwDataSize);
        if (pbData)
        {
            in_len=hwDataSize;
            data_addr   = (DWORD)pbData;
            MP_ASSERT(data_addr);
        }
        else
        {
            MP_ASSERT(0);
            in_len=byte_swap_of_word(ctrl_request.wLength);
            data_addr = 0;
        }
        out_len = 0;
    }
    else
    {
//        MP_DEBUG("USB_DIR_OUT len=%d", hwDataSize);
        in_len = 0;
        out_len=hwDataSize;
        if (out_len > 0)
        {
//            data_addr   = allo_mem(out_len);
            data_addr   = (DWORD)pbData;
            MP_ASSERT(data_addr);
//            memcpy(data_addr,pbData,hwDataSize);
        }
        else
            data_addr = 0;
    }
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        in_len,
                        out_len,
                        data_addr,
                        &ctrl_request);
    
    if (err != USB_NO_ERROR)
    {
        MP_ALERT("flib_Host20_Issue_Control_Vendor(): SetupCommand failed");
        err = -1;
    }
    else
    {
        SemaphoreWait(USB_CONTROL_SEMA);
    }

    return err;
}
#endif

#if 0
//====================================================================
// * Function Name: flib_Host20_Issue_Bulk                          
// * Description: Input data must be 4K-Alignment 
//               <1>.MaxSize=20 K
//               <2>.Support Only 1-TD
// * Input: 
// * OutPut: 
//====================================================================
UINT8  flib_Host20_Issue_Bulk (UINT8 bArrayListNum,UINT16 hwSize,UINT32 *pwBufferArray,UINT32 wOffset,UINT8 bDirection)
{
   PST_USBH_DEVICE_DESCRIPTOR	pUsbhDevDes = gpUsbhDeviceDescriptor;
   qTD_Structure *spTempqTD;
   BYTE        bReturnValue;
    mpDebugPrint("%s",__FUNCTION__);
   mpDebugPrint("hwSize %x",hwSize);

   //<1>.Fill TD
   spTempqTD =flib_Host20_GetStructure(Host20_MEM_TYPE_qTD); //The qTD will be release in the function "Send"
   spTempqTD->bTotalBytes=hwSize;           
   spTempqTD->ArrayBufferPointer_Word[0]=(UINT32)((*pwBufferArray++)+wOffset); 
   spTempqTD->ArrayBufferPointer_Word[1]=(UINT32)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[2]=(UINT32)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[3]=(UINT32)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[4]=(UINT32)*pwBufferArray++; 

   //<2>.Analysis the Direction 
   if (bDirection)          
   	{
      spTempqTD->bPID=HOST20_qTD_PID_IN;             
	  bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->hstBulkInqHD[bArrayListNum],1000);
   	}
   
   else
   	{
      spTempqTD->bPID=HOST20_qTD_PID_OUT;
	  bReturnValue=flib_Host20_Send_qTD(spTempqTD ,pUsbhDevDes->hstBulkOutqHD[bArrayListNum],1000);
   	}
}
#endif

/*
// Definition of internal functions
*/
//====================================================================
// * Function Name: OTGH_PT_Bulk_Init                          
// * Description: Bulk Test Init
// * Input: none
// * OutPut: none
//====================================================================
void UR_OTGH_PT_Bulk_Init(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes)
{
    WORD i,wMaxPacketSize;
    BYTE *wTemp;
    BYTE isHead = 0;
	
	if(mwHost20_Control_ForceFullSpeed_Rd())
		wMaxPacketSize = 0x40;//FS 0x40,  HS 0x200
	else
		wMaxPacketSize = 0x200;//FS 0x40,  HS 0x200

	mpDebugPrint("bulk wMaxPacketSize = 0x%x", wMaxPacketSize);
    //<5>.Hang the qHD
    {//<5.1>.stop Asynchronous Schedule
        flib_Host20_Asynchronous_Setting(HOST20_Disable);
        //if (wMaxPacketSize>128)
        //{//Support only 2 QHD
        //    psHost20_qHD_List_Bulk[0]->bNextQHDPointer=(((DWORD)psHost20_qHD_List_Bulk[1])>>5);
        //    psHost20_qHD_List_Bulk[1]->bNextQHDPointer=(((DWORD)psHost20_qHD_List_Bulk[0])>>5);
        //    mwHost20_CurrentAsynchronousAddr_Set(((DWORD)psHost20_qHD_List_Bulk[0]));
        //}
        //else
        {
            //<5.2>.Hang the qHD for ED0~ED3
            //pUsbhDevDes->pstControlqHD[1]->bNextQHDPointer=(((DWORD)pUsbhDevDes->hstBulkInqHD[0])>>5);
            //pUsbhDevDes->hstBulkInqHD[0]->bNextQHDPointer=(((DWORD)pUsbhDevDes->hstBulkInqHD[1])>>5);
            //pUsbhDevDes->hstBulkInqHD[1]->bNextQHDPointer=(((DWORD)pUsbhDevDes->hstBulkInqHD[0])>>5);
			
			//mwHost20_CurrentAsynchronousAddr_Set(((DWORD)psHost20_qHD_List_Bulk[1]));
        }    

        //<5.2>.Enable Asynchronous Schedule
        flib_Host20_Asynchronous_Setting(HOST20_Enable);
    }      
#if 0
    //Setting Max Packet Size and print message
    pUsbhDevDes->hstBulkInqHD[0]->bMaxPacketSize = wMaxPacketSize;  
    pUsbhDevDes->hstBulkInqHD[1]->bMaxPacketSize = wMaxPacketSize; //Suppose In/Out have the same Max packet Size        
    pUsbhDevDes->hstBulkInqHD[0]->bDeviceAddress = 1;
    pUsbhDevDes->hstBulkInqHD[1]->bDeviceAddress = 1;
    pUsbhDevDes->hstBulkInqHD[0]->bEdNumber = 1;
    pUsbhDevDes->hstBulkInqHD[1]->bEdNumber = 1;

    isHead = 0;
    flib_Host20_Allocate_QHD(pUsbhDevDes->hstBulkInqHD[0],
                            HOST20_HD_Type_QH,
                            1,
                            isHead,
                            1,
                            wMaxPacketSize);
	
    flib_Host20_Allocate_QHD(pUsbhDevDes->hstBulkInqHD[1],
                            HOST20_HD_Type_QH,
                            1,
                            isHead,
                            1,
                            wMaxPacketSize);
#endif

}


//static _timer   rtl8711_timer_head;
void usb_init_timer(void)
{
    _timer *entry = &usb_timer_head;
//	mpDebugPrint("usb_init_timer %x\n",&usb_timer_head);

    //entry->next = &usb_timer_head;
	//entry->prev = &usb_timer_head;
	usb_timer_head.next = &usb_timer_head;
	usb_timer_head.prev = &usb_timer_head;
//	mpDebugPrint("usb_init_timer %x %x\n",entry->next,entry->prev);
}
void usb_add_timer(_timer *newtimer)
{
    _timer *entry = &usb_timer_head;
	//mpDebugPrint("usb_add_timer 1\n");

    if (newtimer->next || newtimer->prev)
    {
        mpDebugPrint("usb_add_timer error\n");
		MP_ASSERT(0);
    }
	//mpDebugPrint("usb_add_timer 2\n");
	//mpDebugPrint("%p %p %p",usb_timer_head.next,&usb_timer_head,entry);
    while (entry->next != &usb_timer_head)
	{
		//mpDebugPrint("%p %p",entry->next,newtimer);
        entry = entry->next;
	}

    newtimer->next = entry->next;
    entry->next->prev = newtimer;
    entry->next = newtimer;
    newtimer->prev = entry;
	//mpDebugPrint("usb_add_timer 3\n");
}

int usb_cancel_timer(_timer *ptimer)
{
    _timer *entry = &usb_timer_head;
	//mpDebugPrint("rtl8711_cancel_timer\n");

    while (entry->next != ptimer && entry->next != &usb_timer_head)
        entry = entry->next;

    if (entry->next == &usb_timer_head)     /* not found */
    {
        mpDebugPrint("usb_cancel_timer error\n");
        return 0;
    }

    entry->next = ptimer->next;
    ptimer->next->prev = entry;
    ptimer->next = ptimer->prev = NULL;
    return 1;
}

void usb_timer_proc(void)
{
	//mpDebugPrint("usb_timer_proc");
	DWORD tck;

    _timer *entry = &usb_timer_head;
    _timer *prev_entry;

    while (entry->next != &usb_timer_head)
    {
        prev_entry = entry;
        entry = entry->next;
        if ((long)(entry->expires) - (long)(mpx_SystemTickerGet()) < 0)
        {
            //mpDebugPrint("usb_timer_proc: timer fires %x\n", entry->function);
			tck = mpx_SystemTickerGet();
            //mpDebugPrint("mpx_SystemTickerGet %x\n",tck );
			
            /* ----------  Remove this timer  ---------- */
            prev_entry->next = entry->next;
            entry->next->prev = prev_entry;
            entry->next = entry->prev = NULL;

            //mpDebugPrint("entry->expires %x\n",entry->expires );
            if (entry->function)
                (*entry->function)(entry->data);

            entry = prev_entry;
        }
    }
}

//************************************
//HISR Polling Function
//************************************
void usb_hisr_poll_handler ()

{
	//_adapter *padapter = (_adapter *)FunctionContext;
	SDWORD ret, *pdwEvent;
	DWORD wifiEvent;  	
	//mpDebugPrint("hisr_poll_handler\n");

	while(1)
	{
			//ReleaseIO();
			//if(( padapter->IntCounter == 0 ))
			//{
			
				//TaskSleep();
			
			wifiEvent = 0;  	
			//EventWait(WIFI_EVENT, EVENT_WIFI_INTERRUPT | EVENT_WIFI_TIMER, OS_EVENT_OR, &wifiEvent);
			EventWait(WIFI_EVENT, EVENT_WIFI_TIMER, OS_EVENT_OR, &wifiEvent);
			//UartOutText("EW");
			if (wifiEvent & EVENT_WIFI_TIMER)
			{
                usb_timer_proc();
			}
			//}

			TaskYield();
 	}


	//TaskTerminate();
	
}
		
		
/**
 * usb_fill_bulk_urb - macro to help initialize a bulk urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete_fn: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 *
 * Initializes a bulk urb with the proper information needed to submit it
 * to a device.
 */
inline void usb_fill_bulk_urb (struct urb *urb,
        struct usb_device *dev,
        unsigned int pipe,
        void *transfer_buffer,
        int buffer_length,
        usb_complete_t complete_fn,
        void *context)
{
    //spin_lock_init(&urb->lock);
    urb->dev = dev;
    urb->pipe = pipe;
    urb->transfer_buffer = transfer_buffer;
    urb->transfer_buffer_length = buffer_length;
    urb->complete = complete_fn;
    urb->context = context;

#if	0
    mpDebugPrint("usb_fill_bulk_urb");
    mpDebugPrint("urb->dev %x",urb->dev);
    mpDebugPrint("urb->pipe %x",urb->pipe);
    mpDebugPrint("urb->transfer_buffer %x",urb->transfer_buffer);
    mpDebugPrint("urb->transfer_buffer_length %x",urb->transfer_buffer_length);
    mpDebugPrint("urb->complete %x",urb->complete);
    mpDebugPrint("urb->context %x",urb->context);
#endif	
    //UR_OTGH_PT_Bulk_Init();
}
		
/*-------------------------------------------------------------------*/

/**
 * usb_submit_urb - issue an asynchronous transfer request for an endpoint
 * @urb: pointer to the urb describing the request
 * @mem_flags: the type of memory to allocate, see kmalloc() for a list
 *	of valid options for this.
 *
 * This submits a transfer request, and transfers control of the URB
 * describing that request to the USB subsystem.  Request completion will
 * be indicated later, asynchronously, by calling the completion handler.
 * The three types of completion are success, error, and unlink
 * (a software-induced fault, also called "request cancellation").  
 *
 * URBs may be submitted in interrupt context.
 *
 * The caller must have correctly initialized the URB before submitting
 * it.  Functions such as usb_fill_bulk_urb() and usb_fill_control_urb() are
 * available to ensure that most fields are correctly initialized, for
 * the particular kind of transfer, although they will not initialize
 * any transfer flags.
 *
 * Successful submissions return 0; otherwise this routine returns a
 * negative error number.  If the submission is successful, the complete()
 * callback from the URB will be called exactly once, when the USB core and
 * Host Controller Driver (HCD) are finished with the URB.  When the completion
 * function is called, control of the URB is returned to the device
 * driver which issued the request.  The completion handler may then
 * immediately free or reuse that URB.
 *
 * With few exceptions, USB device drivers should never access URB fields
 * provided by usbcore or the HCD until its complete() is called.
 * The exceptions relate to periodic transfer scheduling.  For both
 * interrupt and isochronous urbs, as part of successful URB submission
 * urb->interval is modified to reflect the actual transfer period used
 * (normally some power of two units).  And for isochronous urbs,
 * urb->start_frame is modified to reflect when the URB's transfers were
 * scheduled to start.  Not all isochronous transfer scheduling policies
 * will work, but most host controller drivers should easily handle ISO
 * queues going from now until 10-200 msec into the future.
 *
 * For control endpoints, the synchronous usb_control_msg() call is
 * often used (in non-interrupt context) instead of this call.
 * That is often used through convenience wrappers, for the requests
 * that are standardized in the USB 2.0 specification.  For bulk
 * endpoints, a synchronous usb_bulk_msg() call is available.
 *
 * Request Queuing:
 *
 * URBs may be submitted to endpoints before previous ones complete, to
 * minimize the impact of interrupt latencies and system overhead on data
 * throughput.  With that queuing policy, an endpoint's queue would never
 * be empty.  This is required for continuous isochronous data streams,
 * and may also be required for some kinds of interrupt transfers. Such
 * queuing also maximizes bandwidth utilization by letting USB controllers
 * start work on later requests before driver software has finished the
 * completion processing for earlier (successful) requests.
 *
 * As of Linux 2.6, all USB endpoint transfer queues support depths greater
 * than one.  This was previously a HCD-specific behavior, except for ISO
 * transfers.  Non-isochronous endpoint queues are inactive during cleanup
 * after faults (transfer errors or cancellation).
 *
 * Reserved Bandwidth Transfers:
 *
 * Periodic transfers (interrupt or isochronous) are performed repeatedly,
 * using the interval specified in the urb.  Submitting the first urb to
 * the endpoint reserves the bandwidth necessary to make those transfers.
 * If the USB subsystem can't allocate sufficient bandwidth to perform
 * the periodic request, submitting such a periodic request should fail.
 *
 * Device drivers must explicitly request that repetition, by ensuring that
 * some URB is always on the endpoint's queue (except possibly for short
 * periods during completion callacks).  When there is no longer an urb
 * queued, the endpoint's bandwidth reservation is canceled.  This means
 * drivers can use their completion handlers to ensure they keep bandwidth
 * they need, by reinitializing and resubmitting the just-completed urb
 * until the driver longer needs that periodic bandwidth.
 *
 * Memory Flags:
 *
 * The general rules for how to decide which mem_flags to use
 * are the same as for kmalloc.  There are four
 * different possible values; GFP_KERNEL, GFP_NOFS, GFP_NOIO and
 * GFP_ATOMIC.
 *
 * GFP_NOFS is not ever used, as it has not been implemented yet.
 *
 * GFP_ATOMIC is used when
 *   (a) you are inside a completion handler, an interrupt, bottom half,
 *       tasklet or timer, or
 *   (b) you are holding a spinlock or rwlock (does not apply to
 *       semaphores), or
 *   (c) current->state != TASK_RUNNING, this is the case only after
 *       you've changed it.
 * 
 * GFP_NOIO is used in the block io path and error handling of storage
 * devices.
 *
 * All other situations use GFP_KERNEL.
 *
 * Some more specific rules for mem_flags can be inferred, such as
 *  (1) start_xmit, timeout, and receive methods of network drivers must
 *      use GFP_ATOMIC (they are called with a spinlock held);
 *  (2) queuecommand methods of scsi drivers must use GFP_ATOMIC (also
 *      called with a spinlock held);
 *  (3) If you use a kernel thread with a network driver you must use
 *      GFP_NOIO, unless (b) or (c) apply;
 *  (4) after you have done a down() you can use GFP_KERNEL, unless (b) or (c)
 *      apply or your are in a storage driver's block io path;
 *  (5) USB probe and disconnect can use GFP_KERNEL unless (b) or (c) apply; and
 *  (6) changing firmware on a running storage or net device uses
 *      GFP_NOIO, unless b) or c) apply
 *
 */
void UsbOtgBulkActiveEventSend();
int usb_submit_urb(struct urb *urb)
{
	PRTMP_ADAPTER       pAd = (PRTMP_ADAPTER)RTpAT;
	int			pipe, temp, max;
	struct usb_device	*dev;
	int			is_out;
	PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
	pUsbhDevDes = gpUsbhDeviceDescriptor;
	DWORD wOffset,i;
	DWORD pbDataPage[5]; 
	DWORD buffadd;

	//if (!urb || urb->hcpriv || !urb->complete)
	//	return -EINVAL;
	//if (!(dev = urb->dev) ||
	//    (dev->state < USB_STATE_DEFAULT) ||
	//    (!dev->bus) || (dev->devnum <= 0))
	//	return -ENODEV;
	//if (dev->bus->controller->power.power_state.event != PM_EVENT_ON
	//		|| dev->state == USB_STATE_SUSPENDED)
	//	return -EHOSTUNREACH;
	urb->status = -EINPROGRESS;
	urb->actual_length = 0;
	urb->bandwidth = 0;

	/* Lots of sanity checks, so HCDs can rely on clean data
	 * and don't need to duplicate tests
	 */
	pipe = urb->pipe;
	temp = usb_pipetype (pipe);
	is_out = usb_pipeout (pipe);
	
	//mpDebugPrint("temp %x",temp);
	//mpDebugPrint("is_out %x",is_out);

	//if (!usb_pipecontrol (pipe) && dev->state < USB_STATE_CONFIGURED)
	//	return -ENODEV;
	

	/* FIXME there should be a sharable lock protecting us against
	 * config/altsetting changes and disconnects, kicking in here.
	 * (here == before maxpacket, and eventually endpoint type,
	 * checks get made.)
	 */

	//max = usb_maxpacket (dev, pipe, is_out);
	//if (max <= 0) {
	//	dev_dbg(&dev->dev,
	//		"bogus endpoint ep%d%s in %s (bad maxpacket %d)\n",
	//		usb_pipeendpoint (pipe), is_out ? "out" : "in",
	//		__FUNCTION__, max);
	//	return -EMSGSIZE;
	//}

	/* periodic transfers limit size per frame/uframe,
	 * but drivers only control those sizes for ISO.
	 * while we're checking, initialize return status.
	 */
#if	 0
	if (temp == PIPE_ISOCHRONOUS) {
		int	n, len;

		/* "high bandwidth" mode, 1-3 packets/uframe? */
		if (dev->speed == USB_SPEED_HIGH) {
			int	mult = 1 + ((max >> 11) & 0x03);
			max &= 0x07ff;
			max *= mult;
		}

		if (urb->number_of_packets <= 0)		    
			return -EINVAL;
		for (n = 0; n < urb->number_of_packets; n++) {
			len = urb->iso_frame_desc [n].length;
			if (len < 0 || len > max) 
				return -EMSGSIZE;
			urb->iso_frame_desc [n].status = -EXDEV;
			urb->iso_frame_desc [n].actual_length = 0;
		}
	}
#endif
	/* the I/O buffer must be mapped/unmapped, except when length=0 */
	if (urb->transfer_buffer_length < 0)
	{
		mpDebugPrint("urb->transfer_buffer_length %x",urb->transfer_buffer_length);
		return -EMSGSIZE;
	}

	/*
	 * Force periodic transfer intervals to be legal values that are
	 * a power of two (so HCDs don't need to).
	 *
	 * FIXME want bus->{intr,iso}_sched_horizon values here.  Each HC
	 * supports different values... this uses EHCI/UHCI defaults (and
	 * EHCI can use smaller non-default values).
	 */
#if	 0
	 
	switch (temp) {
	case PIPE_ISOCHRONOUS:
	case PIPE_INTERRUPT:
		/* too small? */
		if (urb->interval <= 0)
			return -EINVAL;
		/* too big? */
		switch (dev->speed) {
		case USB_SPEED_HIGH:	/* units are microframes */
			// NOTE usb handles 2^15
			if (urb->interval > (1024 * 8))
				urb->interval = 1024 * 8;
			temp = 1024 * 8;
			break;
		case USB_SPEED_FULL:	/* units are frames/msec */
		case USB_SPEED_LOW:
			if (temp == PIPE_INTERRUPT) {
				if (urb->interval > 255)
					return -EINVAL;
				// NOTE ohci only handles up to 32
				temp = 128;
			} else {
				if (urb->interval > 1024)
					urb->interval = 1024;
				// NOTE usb and ohci handle up to 2^15
				temp = 1024;
			}
			break;
		default:
			return -EINVAL;
		}
		/* power of two? */
		while (temp > urb->interval)
			temp >>= 1;
		urb->interval = temp;
	}
#endif	
	
#if	0
			mpDebugPrint("urb->bandwidth %x",urb->bandwidth);
			mpDebugPrint("urb->use_count %x",urb->use_count);
			mpDebugPrint("urb->reject %x",urb->reject);
			mpDebugPrint("urb->urb_list %x",urb->urb_list);
			mpDebugPrint("urb->dev %x",urb->dev);
			mpDebugPrint("urb->pipe %x",urb->pipe);
			mpDebugPrint("urb->status %x",urb->status);
			mpDebugPrint("urb->transfer_flags %x",urb->transfer_flags);
			mpDebugPrint("urb->transfer_buffer %x",urb->transfer_buffer);
			mpDebugPrint("urb->transfer_buffer_length %x",urb->transfer_buffer_length);
			mpDebugPrint("urb->actual_length %x",urb->actual_length);
			mpDebugPrint("urb->setup_packet %x",urb->setup_packet);
			mpDebugPrint("urb->start_frame %x",urb->start_frame);
			mpDebugPrint("urb->number_of_packets %x",urb->number_of_packets);
			mpDebugPrint("urb->interval %x",urb->interval);
			mpDebugPrint("urb->error_count %x",urb->error_count);
			mpDebugPrint("urb->context %x",urb->context);
			mpDebugPrint("urb->complete %x",urb->complete);
			mpDebugPrint("urb->iso_frame_desc[0] %x",urb->iso_frame_desc[0]);
#endif
	if(is_out)
	{
		ListPushTail(&pAd->pUrbOutList,&urb->Link);
	
//		pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_OUT_STATE;
		EventSet(USBOTG_HOST_DRIVER_EVENT, EVENT_EHCI_ACTIVE_BULK);
	}
	else
	{
	
		ListPushTail(&pAd->pUrbInList,&urb->Link);
//		pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_IN_STATE;
#ifndef MQTD_BULK_IN
		EventSet(USBOTG_HOST_DRIVER_EVENT, EVENT_EHCI_ACTIVE_BULK);
#endif

	}
    

	return 0;
}

/**
 * usb_urb_complete - execute URB 'complete' callback
 * @urb: pointer to the urb describing the request
 * @qtd: 
 *
 *
 */
void usb_urb_complete(struct urb *urb, qTD_Structure *qtd)
{
    unsigned char sts = *(((unsigned char *)qtd) + 11); /* Status */
    unsigned long token = *(((unsigned char *)qtd) + 8); /* token */
    int p = 0;
    if (sts == 0)
        urb->status = 0;
    else if (sts & QTD_STS_HALT)
    {
        // TODO
        if (sts & QTD_STS_BABBLE)
            urb->status = -EOVERFLOW;
		else if (sts & QTD_STS_MMF) {
			/* fs/ls interrupt xfer missed the complete-split */
			urb->status = -EPROTO;
		} else if (sts & QTD_STS_DBE) {
			urb->status = (QTD_PID (token) == 1) /* IN ? */
				? -ENOSR  /* hc couldn't read data */
				: -ECOMM; /* hc couldn't write data */
		} else if (sts & QTD_STS_XACT) {
			/* timeout, bad crc, wrong PID, etc; retried */
			if (QTD_CERR (token))
				urb->status = -EPIPE;
			else {
				urb->status = -EPROTO;
			}
		/* CERR nonzero + no errors + halt --> stall */
		} else if (QTD_CERR (token))
			urb->status = -EPIPE;
		else	/* unknown */
			urb->status = -EPROTO;
        MP_ASSERT(0);
    }
    else
    {
        p = 1;
        urb->status = 0;
    }
    urb->actual_length = urb->transfer_buffer_length - qtd->bTotalBytes;           
    if (p)
    {
        if (sts & QTD_STS_XACT)
            mpDebugPrint("xact error %d ",urb->actual_length);
        else
            mpDebugPrint("other err %d ",urb->actual_length);
    }
    urb->complete(urb);
}

unsigned int
RTUSBBulkInCount(unsigned long data)
{
	PRTMP_ADAPTER       pAd = (PRTMP_ADAPTER)data;
	return (unsigned int)ListGetSize(&pAd->pUrbInList);
}

void UsbWifiBulkOnlyActive (DWORD *pUsbhDev)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)RTpAT;
	int tmpdwBulkOnlyState = 0;
	struct urb *urb_out;
	struct urb *urb_in;
	//mpDebugPrint("UsbWifiBulkOnlyActive");

    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)pUsbhDev;
	
	if( !(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_OUT_STATE))
    {
		if(!ListEmpty(&pAd->pUrbOutList))
		{
		
		 if((pUsbhDevDes->dwWhichBulkPipeDone&0xFFFF)==0)	// XXX
         {
		
			pUsbhDevDes->psAppClass->bBulkInQHDArrayNum = 0;
		
//			mpDebugPrint("1");
            pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_OUT_STATE;	
			UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_OUT);
		  }
	    }
		
			
	}

	if(!(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_IN_STATE))
	{
	
        if(!ListEmpty(&pAd->pUrbInList))
        {
	  	
            if(pUsbhDevDes->dwWhichBulkPipeDone==0)
			{
                pUsbhDevDes->psAppClass->bBulkInQHDArrayNum  = 0;
//				mpDebugPrint("2");
				pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_IN_STATE;	
                UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_IN);
		 	}
	  	}	
	}
	
	


}

void UsbWifiBulkOnlyIoc (DWORD *pUsbhDev)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)RTpAT;

//	mpDebugPrint("UsbWifiBulkOnlyIoc");

    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)pUsbhDev;
	//mpDebugPrint("pUsbhDevDes->psAppClass->dwBulkOnlyState %x ",pUsbhDevDes->psAppClass->dwBulkOnlyState);
	//mpDebugPrint("<--pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

    if(pUsbhDevDes->dwWhichBulkPipeDone & 0x01)
    {
        
//        mpDebugPrint("3");
        flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[0], pUsbhDevDes);
        if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
        {
            mpDebugPrint("error=0x%x", pUsbhDevDes->bQHStatus);
            pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_OUT_STATE;
            if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
            {
                SWORD err = 0;
                MP_ASSERT(0);
                pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                        &pUsbhDevDes->sSetupPB);
			//if (err != USB_NO_ERROR)
			//{ 
                return;
			//}
            }
            else
            {
                mpDebugPrint("WIFI_BULK_DATA_OUT_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
            }
        }
        else
        {
			pUsbhDevDes->psAppClass->dDataOutLen	 = 0;
			pUsbhDevDes->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_OUT_STATE;

			if (ListGetSize(&pAd->pUrbOutList) > 0)
				EventSet(USBOTG_HOST_DRIVER_EVENT, EVENT_EHCI_ACTIVE_BULK);
        }
			//EventSet(USB_BULK_COMPLETE_EVENT, 0x1);	
    }
        
    if(pUsbhDevDes->dwWhichBulkPipeDone & 0x10000)
    {
//        mpDebugPrint("4");

        flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[0], pUsbhDevDes);
        if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
        {
            pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_IN_STATE;
            if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
            {
		
                mpDebugPrint("HOST20_qTD_STATUS_Halted");
                SWORD err = 0;
                pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                        &pUsbhDevDes->sSetupPB);
                if (err != USB_NO_ERROR)
                { 
                    free1(pUsbhDevDes->psAppClass->dDataBuffer);						
                    return;
                }
            }
            else
            {
                mpDebugPrint("WIFI_BULK_DATA_IN_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
            }
        }
        else
        {
            pUsbhDevDes->psAppClass->dDataInLen 	 = 0;
            pUsbhDevDes->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_IN_STATE;
        }
			//net_buf_mem_free(pUsbhDevDes->psAppClass->dDataBuffer);
			//EventSet(USB_BULK_COMPLETE_EVENT, 0x2);	

    }
//mpDebugPrint("-->pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

//pUsbhDevDes->dwWhichBulkPipeDone = 0;

//mpDebugPrint("pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

}

void UsbWifiSetupIoc(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes)
{
    SemaphoreRelease(USB_CONTROL_SEMA);
}

void UsbOtgBulkProcess(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes
	,BYTE	bDirection)
{
    DWORD pbDataPage[MAX_DATA_PAGE]; 
    DWORD wOneTdTotalLengthRemain = 0;
    DWORD wOffset;
    WORD  hwSize = 0;
    DWORD i;
    BOOL  fUseMultiTDs = FALSE;
    BYTE  qHD_array_number;
    DWORD dwMaxTxLength = 0;
    DWORD dwBufferOffset = 0;
	PRTMP_ADAPTER	pAd =(PRTMP_ADAPTER)RTpAT;
	struct urb *urb_out;
	struct urb *urb_in, *urb;
    struct ehci_qtd *qtd;
	//mpDebugPrint("UsbOtgBulkProcess %x",pUsbhDevDes->psAppClass->bBulkInQHDArrayNum);
    for (i = 0; i < MAX_DATA_PAGE; i++)
        pbDataPage[i] = 0;

    qHD_array_number = pUsbhDevDes->psAppClass->bBulkInQHDArrayNum;
	
	if(bDirection == OTGH_DIR_OUT)
    {
        while (urb_out = ListPopHead(&pAd->pUrbOutList))
        {
            if (pbDataPage[0])
            {
			    mpDebugPrint("multi a");
                pUsbhDevDes->urb = urb;
                qtd = (struct ehci_qtd *)flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                 hwSize, 
                                                 &(pbDataPage[0]), 
                                                 wOffset, 
                                                 bDirection,
                                                 0,
                                                 pUsbhDevDes);
                pUsbhDevDes->urb = NULL;
                fUseMultiTDs = TRUE;
            }

            hwSize              = urb_out->transfer_buffer_length;
            pbDataPage[0]       = ((DWORD)(urb_out->transfer_buffer));
            wOffset             = 0;
            urb                 = urb_out;
            break;                              /* disable multi for now */
        }
    }
    else
    {
		i = 0;
        while (urb_in = ListPopHead(&pAd->pUrbInList))
        {
            if (pbDataPage[0])
            {
//			    mpDebugPrint("multi b %d", i++);
                pUsbhDevDes->urb = urb;
                qtd = (struct ehci_qtd *)flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                 hwSize, 
                                                 &(pbDataPage[0]), 
                                                 wOffset, 
                                                 bDirection,
                                                 0,
                                                 pUsbhDevDes);
                pUsbhDevDes->urb = NULL;
                fUseMultiTDs = TRUE;
            }

            hwSize              = urb_in->transfer_buffer_length;
            pbDataPage[0]       = ((DWORD)(urb_in->transfer_buffer));
            wOffset             = 0;
            urb                 = urb_in;
        }

    }

    MP_ASSERT(pbDataPage[0]);

    if (fUseMultiTDs)
    { // for multi-TD
//    mpDebugPrint("multi c");
        pUsbhDevDes->urb = urb;
		qtd = (struct ehci_qtd *)flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                 hwSize, 
                                                 &(pbDataPage[0]), 
                                                 wOffset, 
                                                 bDirection,
                                                 1,
                                                 pUsbhDevDes);
        pUsbhDevDes->urb = NULL;
    }
    else
    { // for single-TD 
        pUsbhDevDes->urb = urb;
        qtd = (struct ehci_qtd *)flib_Host20_Issue_Bulk_Active ( qHD_array_number,
                                        hwSize, 
                                        &(pbDataPage[0]), 
                                        wOffset, 
                                        bDirection,
                                        pUsbhDevDes);
        pUsbhDevDes->urb = NULL;
    }
}


static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;

static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;

void UsbOtgBulkActiveEventSend()
{
	EventSet(USBOTG_HOST_DRIVER_EVENT, EVENT_EHCI_ACTIVE_BULK);
}

void WifiStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID)
{
    static DWORD                frame_idx = 0;
    static BYTE                 config_idx = 0;
    static BYTE                 interface_idx = 0;
    //static BYTE                 try_cnt = 0;
    SWORD                       err = USB_NO_ERROR;

    err = USB_NO_ERROR;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    ST_MCARD_MAIL               receiveMailDrv;
    ST_MCARD_MAIL               *pSendMailDrv;
    DWORD   i = 0;
	BYTE tmp8[4];

    static BOOL isProcessedBiuReset = FALSE;
    static BOOL isHostToResetDevice = FALSE;

    pUsbhDevDes     = gpUsbhDeviceDescriptor;
	mpDebugPrint("WifiStateMachine");

    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER);
    if (mwHost20_PORTSC_ConnectStatus_Rd() == 0)// device plug out
    {
        if (pUsbhDevDes->bDeviceStatus == USB_STATE_ATTACHED)
        {
            mpDebugPrint("1Device Plug Out");
        }
        else
        {
            mpDebugPrint("2Device Plug Out");
            if ( receiveMailDrv.wCurrentExecutionState != SETUP_INIT_STOP_STATE)
            {
                mpDebugPrint("3Device Plug Out");
                //pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
                //if (mwHost20_USBINTR_Rd() == 0)
                //    mwHost20_USBINTR_Set(HOST20_USBINTR_ENABLE);
                return;
            }
        }
    }

    mpDebugPrint("WSSM-stat %d" , receiveMailDrv.wCurrentExecutionState);
	
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case WIFI_INIT_START_STATE:
			
			pUsbhDevDes->psAppClass->sBulkInDescriptor.wMaxPacketSize = 0x940;
			pUsbhDevDes->psAppClass->dDataInLen = 0x40;
			pUsbhDevDes->bDeviceAddress = 1;
			pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress = 1;
			pUsbhDevDes->psAppClass->sBulkInDescriptor.bmAttributes = OTGH_ED_BULK;
			UR_OTGH_PT_Bulk_Init(pUsbhDevDes);
			pSendMailDrv->wCurrentExecutionState	= WIFI_INIT_STATE;
			pSendMailDrv->wStateMachine 			= WIFI_SM;
	        break;
		
        case WIFI_INIT_STATE:
			
            if (mwHost20_PORTSC_ConnectStatus_Rd())
            { // plug in
                mpDebugPrint("WIFI Plug-in\r\n");
				
				mpDebugPrint("idVendor = %x",pUsbhDevDes->sDeviceDescriptor.idVendor);
				mpDebugPrint("idProduct = %x",pUsbhDevDes->sDeviceDescriptor.idProduct);
				 
				//if((pUsbhDevDes->sDeviceDescriptor.idVendor==RTL8711_USB_DEVICES_VID)&&
				// (pUsbhDevDes->sDeviceDescriptor.idProduct==RTL8711_USB_DEVICES_PID))
				//{
				//	SystemDeviceInit(USB_RTL8711);
						
				//}
				//else 
				if(((pUsbhDevDes->sDeviceDescriptor.idVendor==RT73_USB_DEVICES_VID)&&
				 (pUsbhDevDes->sDeviceDescriptor.idProduct==RT73_USB_DEVICES_PID))||
				 ((pUsbhDevDes->sDeviceDescriptor.idVendor==RT73_USB_DEVICES1_VID)&&
				 (pUsbhDevDes->sDeviceDescriptor.idProduct==RT73_USB_DEVICES1_PID))||
				 ((pUsbhDevDes->sDeviceDescriptor.idVendor==RT73_USB_DEVICES2_VID)&&
				 (pUsbhDevDes->sDeviceDescriptor.idProduct==RT73_USB_DEVICES2_PID))||
				 ((pUsbhDevDes->sDeviceDescriptor.idVendor==RT73_USB_DEVICES_ASUS_VID)&&
				 (pUsbhDevDes->sDeviceDescriptor.idProduct==RT73_USB_DEVICES_ASUS_PID))||
				 ((pUsbhDevDes->sDeviceDescriptor.idVendor==RT73_USB_DEVICES_LANTECH_VID)&&
				 (pUsbhDevDes->sDeviceDescriptor.idProduct==RT73_USB_DEVICES_LANTECH_PID))	
				)
				{
					
					pUsbhDevDes->fpAppClassBulkActive	= &UsbWifiBulkOnlyActive;
					pUsbhDevDes->fpAppClassBulkIoc		= &UsbWifiBulkOnlyIoc;
					pUsbhDevDes->fpAppClassSetupIoc = &UsbWifiSetupIoc; /* for control transfer */
					SystemDeviceInit(USB_RT73);
				}
            }
            else
            { // plug out
                mpDebugPrint("WIFI init:Plug-out\r\n");
                pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
                return;
            }
			break;
		
        default:
        	{
	            MP_ALERT("Wi-Fi SETUP STATE is invalid!!");
        	}
        	break;
    }

    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if (receiveMailDrv.wCurrentExecutionState == WIFI_INIT_START_STATE)
        {
            SDWORD  osSts;
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv);
            if (osSts != OS_STATUS_OK)
            {
                MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
            }
        }
    }
    else if (err != USB_NO_ERROR)
    {
    	MP_DEBUG1("SetupSM err = %d", err);
    }
}

void usb_wifi_unplug(void)
{
    /* TODO */
}
