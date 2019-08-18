/** 
	This function registers the card as HOST/USER(WLAN)
	\param type of the card, function pointer
	\return returns pointer to the type requested for
*/
#define LOCAL_DEBUG_ENABLE 0

#include	"linux/compat-2.6.h"
#include	"linux/types.h"

#if (SC_USBHOST==ENABLE)
#if NETWARE_ENABLE

#include	"UtilTypeDef.h"
#include    "linux/usb.h"
#include 	"devio.h"
//#include 	"lwip_incl.h"
//#include    "osdep_mpixel_service.h"
#include    "taskid.h"
#include    "wlan_sys.h"
#include    "os_mp52x.h"
#include    "mpTrace.h"

#include	"net_device.h"
#include	"usbotg_wifi.h"
#include	"usbotg_ethernet.h"
#include	"list_mpx.h"
#include	"linux/netdevice.h"
#include	"linux/completion.h"

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
#include	"zd_usb.h"
#endif
#include	"ndebug.h"

#include "usbotg_host_setup.h"
#include "net_nic.h"



int urb_sema;
int usb_sema;
#define MAX_DATA_PAGE           5
#define ONE_PAGE_BYTE_CNT       4096
#define ONE_TD_MAX_BYTE_CNT     16384//20480 // 4096 * 5
/*
#define AR2524_MAX_NETPOOL_BUFFER_SIZE 4800
#define AR2524_MAX_NETPOOL_ALLOC_SIZE (AR2524_MAX_NETPOOL_BUFFER_SIZE+32)
#define AR2524_MAX_NUM_BUFFERS (NETPOOL_BUF_SIZE/ AR2524_MAX_NETPOOL_ALLOC_SIZE)

#define RALINK_MAX_NETPOOL_ALLOC_SIZE 2560
#define RALINK_MAX_NETPOOL_BUFFER_SIZE (RALINK_MAX_NETPOOL_ALLOC_SIZE-32)
#define RALINK_MAX_NUM_BUFFERS 500*/

//<4>.0x010(USBCMD - USB Command Register)
//#define mwHost20_USBCMD_IntThreshold_Rd()		          ((gp_UsbOtg->HcUsbCommand>>16)&0x0000FFFF)	//Bit 16~23
//#define mbHost20_USBCMD_IntThreshold_Set(bValue)		  (gp_UsbOtg->HcUsbCommand =((gp_UsbOtg->HcUsbCommand&0xFF00FFFF)|(((DWORD)(bValue))<<16)))	//Bit 16~23
//#define mbHost20_USBCMD_FrameListSize_Rd()				  ((gp_UsbOtg->HcUsbCommand>>2)&0x00000003)    //Bit 2~3
//#define mbHost20_USBCMD_FrameListSize_Set(bValue)		  ((gp_UsbOtg->HcUsbCommand=((gp_UsbOtg->HcUsbCommand&0xFFFFFFF3)|(((DWORD)(bValue))<<2))))	//Bit 2~3

#define mwHost20_Control_ForceFullSpeed_Rd()			  (psUsbOtg->psUsbReg->OtgControlStatus& BIT12)
#define QTDNUM  4

extern void *net_buf_start;
extern void *net_buf_end;


//static _timer   usb_timer_head;
_timer   usb_timer_head;  // testing
//static int      usb_timer_semaphore;
int      usb_timer_semaphore;   // for testing

const static BYTE bDescriptor[8] = "USB_WIFI";//"AR2524";

#define MAX_USB_DEVICES USBOTG_NONE

#define MAX_BULK_OUT_ENDPOINTS  4

LM_LIST_CONTAINER pUrbOutList[MAX_USB_DEVICES][MAX_BULK_OUT_ENDPOINTS];
LM_LIST_CONTAINER pUrbInList[MAX_USB_DEVICES];
LM_LIST_CONTAINER pUrbOutList_Int[MAX_USB_DEVICES];       /* USB Interrupt pipe */
LM_LIST_CONTAINER pUrbInList_Int[MAX_USB_DEVICES];       /* USB Interrupt pipe */
LM_LIST_CONTAINER pUrbInCompleteList[MAX_USB_DEVICES];       
	
/*
 * keep track of tasks waiting for urb completion.
 */
struct urb_wait_taskqueue_s {
	struct list_head task_list;
    struct mutex task_list_mutex;
};

BYTE wifi_device_type = WIFI_USB_DEVICE_NONE;
//For wpa supplicant check device unplug to change interface
BYTE wifi_device_unplug = FALSE;
BYTE wifi_device_plugin = FALSE;
BYTE wifi_device_init = FALSE;
static struct urb_wait_taskqueue_s urb_wait_taskqueue[MAX_USB_DEVICES];

struct usb_device *usbnet_wifi_dev[MAX_USB_DEVICES];
struct usb_device   dev_global[MAX_USB_DEVICES];

static WHICH_OTG ethernet_usbotg;

#pragma alignvar(4)

static void CommandProcess (void *pMcardDev);

void UsbOtgBulkProcess(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes ,BYTE	bDirection, PLM_LIST_CONTAINER pUrbList, WHICH_OTG eWhichOtg);

BYTE flib_Host20_Issue_Control (BYTE bEdNum,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData);
static int flib_Host20_Issue_Control_Vendor (WHICH_OTG eWhichOtg,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData);

	
void (*NetDeviceAttachEvent_Fn)(WORD idVendor, WORD idProduct, int attached);
	
static void usb_api_blocking_completion(struct urb *urb);
static int usb_start_wait_urb(struct urb *urb, int timeout, int *actual_length);
struct usb_device *usb_data_setup(WHICH_OTG eWhichOtg);
void usb_data_cleanup(WHICH_OTG eWhichOtg);
extern struct usb_driver * usb_get_driver(u16 vid, u16 pid);
qTD_Structure *  Wlan_flib_Host20_Issue_Interrupt_Active (  BYTE    bArrayListNum, WORD    hwSize, DWORD   *pwBufferArray, DWORD   wOffset, BYTE    bDirection, WHICH_OTG eWhichOtg);

// TRUE:PlugOut   FALSE:PlugIn
BOOL UsbOtgCheckPlugOut(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    #if Make_USB
    if (wifi_device_unplug || pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// device plug out
    #else
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// device plug out
    #endif
        return TRUE;
    else
        return FALSE;
}

#if Make_USB == RALINK_WIFI||Make_USB == RALINK_AR2524_WIFI
/***************************************************************
//For RT73
***************************************************************/
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

#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
    if(NetConfiged())
    TaskSleep(1);
#else
    TaskYield();
#endif
 	}


	//TaskTerminate();
	
}
/*=============================================================
//For RT73 end
===============================================*/
#endif
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
  if(wifi_device_type == WIFI_USB_DEVICE_RT73)
	SemaphoreWait(urb_sema);	
  else
    IntDisable();
    pEntry->BLink = (PLM_LIST_ENTRY) 0;
    pEntry->FLink = pList->Link.FLink;

    if(pList->Link.FLink == (PLM_LIST_ENTRY) 0) {
        pList->Link.BLink = (PLM_LIST_ENTRY) pEntry;
    } else {
        pList->Link.FLink->BLink = pEntry;
    }

    pList->Link.FLink = (PLM_LIST_ENTRY) pEntry;

    pList->EntryCount++;
	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
  	SemaphoreRelease(urb_sema);
	else
    IntEnable();
} /* ListPushHead */

/******************************************************************************/
/* ListPushTail -- puts an element at the tail of the list.                   */
/******************************************************************************/
void
ListPushTail(
PLM_LIST_CONTAINER pList, 
PLM_LIST_ENTRY pEntry) 
{
	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
	SemaphoreWait(urb_sema);	
	else
    IntDisable();

    pEntry->BLink = pList->Link.BLink;

    if(pList->Link.BLink) {
        pList->Link.BLink->FLink = pEntry;
    } else {
        pList->Link.FLink = pEntry;
		
    }

    pList->Link.BLink = pEntry;
    pEntry->FLink = (PLM_LIST_ENTRY) 0;

    pList->EntryCount++;
	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
  	SemaphoreRelease(urb_sema);
	else
    IntEnable();
	
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

	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
	SemaphoreWait(urb_sema);	
	else
    IntDisable();

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
    
	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
  	SemaphoreRelease(urb_sema);
	else
    IntEnable();
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

void UsbWifiInit(ST_MCARD_DEV * sDev)
{
	mpDebugPrint("\nUsbWifiInit\n");

	sDev->pbDescriptor = (BYTE*)bDescriptor;
	//sDev->dwLunNum = LUN_NUM_0;
	sDev->wMcardType = DEV_USB_WIFI_DEVICE;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}
#if DM9621_ETHERNET_ENABLE
void UsbEthernetInit(ST_MCARD_DEV * sDev)
{
	mpDebugPrint("\nUsbEthernetInit\n");

	sDev->pbDescriptor = (BYTE*)bDescriptor;
	//sDev->dwLunNum = LUN_NUM_0;
	sDev->wMcardType = DEV_USB_ETHERNET_DEVICE;
	sDev->Flag.Installed = 1;
	sDev->CommandProcess = CommandProcess;
}
#endif
void NetDriverUpEventSet(int nic);
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
 *
 *	NOTE: <dev> and <pipe> arguments are not used on MP620 systems.
 */
int usb_control_msg(struct usb_device *dev, unsigned int pipe, unsigned char request, unsigned char requesttype,
			 unsigned short value, unsigned short index, void* data, unsigned short size, int timeout)
{
	USB_CTRL_REQUEST ctrl_request;
    int   err;
    int   ret = 0;

    if (UsbOtgCheckPlugOut(dev->devnum)) // device plug out
    {
        wifi_device_unplug = TRUE;
        return -ENODEV; 
    }
	
    MP_FUNCTION_ENTER();
//    mpDebugPrint("usb_control_msg %x,%x", requesttype, request);
	SetupBuilder(&ctrl_request,
				 requesttype,
				 request,
				 value,
				 index,
				 size);
#if 0
    mpDebugPrint("SETUP REQ: =============");
    mpDebugPrint("request = %x",request);
    mpDebugPrint("requesttype = %x",requesttype);
    mpDebugPrint("value = %x",value);
    mpDebugPrint("index = %d",index);
    mpDebugPrint("size = %d",size);
#endif	
	//SemaphoreWait(WIFI_IO_SEMA_ID);
#if 0
    if ((requesttype & USB_TYPE_MASK) == USB_TYPE_VENDOR)
#endif
        err = flib_Host20_Issue_Control_Vendor(dev->devnum,(BYTE*)&ctrl_request,size,data);
#if 0
    else
        err = flib_Host20_Issue_Control(1,&ctrl_request,size,data);
#endif
	//SemaphoreRelease(WIFI_IO_SEMA_ID);
	
    if (err)
    {
        mpDebugPrint("flib_Host20_Issue_Control returns error %d", err);
        ret = -1;
    }
    else
        ret = size;
    MP_FUNCTION_EXIT();
	return ret;
}

/**
 *	usb_bulk_msg - Builds a bulk urb, sends it off and waits for completion
 *	@usb_dev: pointer to the usb device to send the message to
 *	@pipe: endpoint "pipe" to send the message to
 *	@data: pointer to the data to send
 *	@len: length in bytes of the data to send
 *	@actual_length: pointer to a location to put the actual length transferred in bytes
 *	@timeout: time in msecs to wait for the message to complete before
 *		timing out (if 0 the wait is forever)
 *	Context: !in_interrupt ()
 *
 *	This function sends a simple bulk message to a specified endpoint
 *	and waits for the message to complete, or timeout.
 *	
 *	If successful, it returns 0, otherwise a negative error number.
 *	The number of actual bytes transferred will be stored in the 
 *	actual_length paramater.
 *
 *	Don't use this function from within an interrupt context, like a
 *	bottom half handler.  If you need an asynchronous message, or need to
 *	send a message from within interrupt context, use usb_submit_urb()
 *      If a thread in your driver uses this call, make sure your disconnect()
 *      method can wait for it to complete.  Since you don't have a handle on
 *      the URB used, you can't cancel the request.
 *
 *	Because there is no usb_interrupt_msg() and no USBDEVFS_INTERRUPT
 *	ioctl, users are forced to abuse this routine by using it to submit
 *	URBs for interrupt endpoints.  We will take the liberty of creating
 *	an interrupt URB (with the default interval) if the target is an
 *	interrupt endpoint.
 */
int usb_bulk_msg(struct usb_device *usb_dev, unsigned int pipe, 
			void *data, int len, int *actual_length, int timeout)
{
	struct urb *urb;
	struct usb_host_endpoint *ep;

    if(UsbOtgCheckPlugOut(usb_dev->devnum)) // device plug out 
    {
        wifi_device_unplug = TRUE;
        return -ETIMEDOUT; 
    }

	ep = (usb_pipein(pipe) ? usb_dev->ep_in : usb_dev->ep_out)
			[usb_pipeendpoint(pipe)];
	if (!ep || len < 0)
    {
        MP_DEBUG("%s: endpoint doesn't exist. Maybe USB dongle is removed.", __func__);
		return -EINVAL;
    }

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return -ENOMEM;

#ifdef LINUX
	if ((ep->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
			USB_ENDPOINT_XFER_INT) {
		pipe = (pipe & ~(3 << 30)) | (PIPE_INTERRUPT << 30);
		usb_fill_int_urb(urb, usb_dev, pipe, data, len,
				usb_api_blocking_completion, NULL,
				ep->desc.bInterval);
	} else
#else
	if ((ep->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
        MP_DEBUG("%s: intr ep=%d", __func__, usb_pipeendpoint(pipe));
		pipe = (pipe & ~(3 << 30)) | (PIPE_INTERRUPT << 30);
		usb_fill_int_urb(urb, usb_dev, pipe, data, len,
				usb_api_blocking_completion, NULL,
				timeout);
	} else
#endif
		usb_fill_bulk_urb(urb, usb_dev, pipe, data, len,
				usb_api_blocking_completion, NULL);

//    mpDebugPrint("%s: %p", __func__, urb);
    MP_DEBUG("%s: ep=%d", __func__, usb_pipeendpoint(pipe));
	return usb_start_wait_urb(urb, timeout, actual_length);
}

/**
 * usb_kill_urb - cancel a transfer request and wait for it to finish
 * @urb: pointer to URB describing a previously submitted request,
 *	may be NULL
 *
 * This routine cancels an in-progress request.
 */
void usb_kill_urb(struct urb *urb)
{
    // TODO
}

/**
 * usb_buffer_alloc - allocate dma-consistent buffer for URB_NO_xxx_DMA_MAP
 * @dev: device the buffer will be used with
 * @size: requested buffer size
 * @mem_flags: affect whether allocation may block
 * @dma: used to return DMA address of buffer
 *
 * Return value is either null (indicating no buffer could be allocated), or
 * the cpu-space pointer to a buffer that may be used to perform DMA to the
 * specified device.  Such cpu-space buffers are returned along with the DMA
 * address (through the pointer provided).
 *
 * These buffers are used with URB_NO_xxx_DMA_MAP set in urb->transfer_flags
 * to avoid behaviors like using "DMA bounce buffers", or thrashing IOMMU
 * hardware during URB completion/resubmit.  The implementation varies between
 * platforms, depending on details of how DMA will work to this device.
 * Using these buffers also eliminates cacheline sharing problems on
 * architectures where CPU caches are not DMA-coherent.  On systems without
 * bus-snooping caches, these buffers are uncached.
 *
 * When the buffer is no longer used, free it with usb_buffer_free().
 */
void *usb_buffer_alloc(
	struct usb_device *dev,
	size_t size,
	gfp_t mem_flags,
	dma_addr_t *dma
)
{
#ifdef LINUX
	if (!dev)
		return NULL;
	return hcd_buffer_alloc(dev->bus, size, mem_flags, dma);
#else
    return usb_alloc_buffer(size);
#endif
}

/**
 * usb_buffer_free - free memory allocated with usb_buffer_alloc()
 * @dev: device the buffer was used with
 * @size: requested buffer size
 * @addr: CPU address of buffer
 * @dma: DMA address of buffer
 *
 * This reclaims an I/O buffer, letting it be reused.  The memory must have
 * been allocated using usb_buffer_alloc(), and the parameters must match
 * those provided in that allocation request.
 */
void usb_buffer_free(
	struct usb_device *dev,
	size_t size,
	void *addr,
	dma_addr_t dma
)
{
#ifdef LINUX
	if (!dev)
		return;
	if (!addr)
		return;
	hcd_buffer_free(dev->bus, size, addr, dma);
#else
    usb_free_buffer(addr);
#endif
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
   WHICH_OTG eWhichOtg = WIFI_USB_PORT;
   PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(WIFI_USB_PORT);
   DWORD   data_addr;
   int   err;
   WORD   in_len, out_len;
   USB_CTRL_REQUEST ctrl_request;
   
   MP_DEBUG("flib_Host20_Issue_Control");
   
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
           &ctrl_request,
           WIFI_USB_PORT);

   if (err != USB_NO_ERROR)
   {
       MP_ALERT("flib_Host20_Issue_Control: SetupCommand failed err=%d", err);
       err = 1;
   }
   else
   {
       SemaphoreWait(USB_CONTROL_SEMA);
   }

   return (0);
}
#endif

static int flib_Host20_Issue_Control_Vendor (WHICH_OTG eWhichOtg,BYTE* pbCmd,WORD hwDataSize,BYTE* pbData)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    DWORD   data_addr;
    int   err;
    WORD   in_len, out_len;
    USB_CTRL_REQUEST ctrl_request;
//    MP_DEBUG("flib_Host20_Issue_Control_Vendor");

    mpx_SemaphoreWait(usb_sema);

//    MP_FUNCTION_ENTER();
    if (pUsbhDevDes->sSetupPB.dwSetupState != SETUP_IDLE_STATE)
    {
        mpx_SemaphoreRelease(usb_sema);
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
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_ALERT("flib_Host20_Issue_Control_Vendor(): SetupCommand failed err=%d", err);
		switch(wifi_device_type)
		{
		  case WIFI_USB_DEVICE_AR2524:
		  case WIFI_USB_DEVICE_RTL8192U:
		  case WIFI_USB_DEVICE_AR9271:
          case ETHERNET_USB_DEVICE_DM9621:
		  case WIFI_USB_DEVICE_RTL8188C:
		  default:
			err = 1;
			break;
		  case WIFI_USB_DEVICE_RT73:
			err = -1;
			 break;
			
		}
    }
    else
    {
        SemaphoreWait(USB_CONTROL_SEMA);
    }

    mpx_SemaphoreRelease(usb_sema);
//    MP_FUNCTION_EXIT();
    return err;
}
   

/*
// Definition of internal functions
*/
//====================================================================
// * Function Name: OTGH_PT_Bulk_Init                          
// * Description: Bulk Test Init
// * Input: none
// * OutPut: none
//====================================================================
void UR_OTGH_PT_Bulk_Init(WHICH_OTG eWhichOtg)
{
    WORD i,wMaxPacketSize;
    BYTE *wTemp;
    BYTE isHead = 0;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    if(mwHost20_Control_ForceFullSpeed_Rd())
        wMaxPacketSize = 0x40;//FS 0x40,  HS 0x200
    else
        wMaxPacketSize = 0x200;//FS 0x40,  HS 0x200

	mpDebugPrint("bulk wMaxPacketSize = 0x%x", wMaxPacketSize);
    //<5>.Hang the qHD
    {//<5.1>.stop Asynchronous Schedule
        flib_Host20_Asynchronous_Setting(HOST20_Disable, eWhichOtg);
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
        flib_Host20_Asynchronous_Setting(HOST20_Enable, eWhichOtg);
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
	
	if (usb_timer_semaphore == 0)
	{
		int ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(ret > 0);
		if (ret > 0)
			usb_timer_semaphore = ret;
	}
	mpDebugPrint("usb_init_timer %x %x\n",entry->next,entry->prev);
}
static unsigned long wifi_next_expires;
void usb_add_timer(_timer *newtimer)
{
    _timer *entry = &usb_timer_head;
	//mpDebugPrint("usb_add_timer wifi_device_type %d",wifi_device_type);

    if (newtimer->next || newtimer->prev)
    {
        MP_ALERT("usb_add_timer error\n");
		MP_ASSERT(0);
    }
	
    mpx_SemaphoreWait(usb_timer_semaphore);
	//mpDebugPrint("usb_add_timer 2\n");
	//mpDebugPrint("%p %p %p",usb_timer_head.next,&usb_timer_head,entry);
    while (entry->next != &usb_timer_head)
	{
		//mpDebugPrint("%p %p",entry->next,newtimer);
#if 1
        /* support sorted timer list */
        if ((long)newtimer->expires - (long)entry->next->expires < 0)
            break;
#endif
        entry = entry->next;
	}

    newtimer->next = entry->next;
    entry->next->prev = newtimer;
    entry->next = newtimer;
    newtimer->prev = entry;
#if 1
    wifi_next_expires = usb_timer_head.next->expires;
    wifi_timer_set(wifi_next_expires);
#endif
	//mpDebugPrint("usb_add_timer 3\n");
	mpx_SemaphoreRelease(usb_timer_semaphore);
//	MP_DEBUG("<-- usb_add_timer\n");
}

int usb_cancel_timer(_timer *ptimer)
{
    _timer *entry = &usb_timer_head;
	MP_DEBUG("--> usb_cancel_timer\n");

    mpx_SemaphoreWait(usb_timer_semaphore);
	
    while ((unsigned long)entry->next != (unsigned long)ptimer && (unsigned long)entry->next != (unsigned long)&usb_timer_head)
        entry = entry->next;

    if (entry->next == &usb_timer_head)     /* not found */
    {
        MP_DEBUG("usb_cancel_timer not found\n");
        mpx_SemaphoreRelease(usb_timer_semaphore);
        return 0;
    }

    entry->next = ptimer->next;
    ptimer->next->prev = entry;
    ptimer->next = ptimer->prev = NULL;
    mpx_SemaphoreRelease(usb_timer_semaphore);
	MP_DEBUG("<-- usb_cancel_timer\n");
    return 1;
}

int usb_timer_proc_in_progress;
void usb_timer_proc(void)
{
	//DWORD tck;

    _timer *entry = &usb_timer_head;
    _timer *prev_entry;

    mpx_SemaphoreWait(usb_timer_semaphore);
    usb_timer_proc_in_progress = TRUE;
    //mpDebugPrint("usb_timer_proc: got sema,e=%p,n=%p", entry, entry->next);
    while (entry->next != &usb_timer_head)
    {
        prev_entry = entry;
        entry = entry->next;
		
        if ((long)(entry->expires) - (long)(jiffies) < 0)
        {
//            mpDebugPrint("usb_timer_proc: timer fires %x\n", entry->function);
			//tck = mpx_SystemTickerGet();
            //mpDebugPrint("mpx_SystemTickerGet %x\n",tck );
			
            /* ----------  Remove this timer  ---------- */
            prev_entry->next = entry->next;
            entry->next->prev = prev_entry;
            entry->next = entry->prev = NULL;

            mpx_SemaphoreRelease(usb_timer_semaphore);

//            mpDebugPrint("entry->expires %x (%p)\n",entry->expires, entry->function);
            if (entry->function)
                (*entry->function)(entry->data);

            entry = prev_entry;
            mpx_SemaphoreWait(usb_timer_semaphore);

            if (!entry->next)                         /* timer is already cancelled */
                break;
        }
        else
            break;
#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
     if(NetConfiged())
     TaskSleep(1);
#endif

    }
	
#if 1
    unsigned long old = wifi_next_expires;
    if (usb_timer_head.next != &usb_timer_head)
        wifi_next_expires = usb_timer_head.next->expires;
    else
        wifi_next_expires = 0;                  /* list is empty */
    if (old != wifi_next_expires)
        wifi_timer_set(wifi_next_expires);
#endif

    usb_timer_proc_in_progress = FALSE;
    mpx_SemaphoreRelease(usb_timer_semaphore);
//	mpDebugPrint("<-- usb_timer_proc");
}

/* 
 * Similar to usb_add_timer but without semaphore wait/release
 */
void usb_add_timer2(_timer *newtimer)
{
    _timer *entry = &usb_timer_head;
//	mpDebugPrint("--> usb_add_timer2\n");

    if (newtimer->next || newtimer->prev)
    {
        //mpDebugPrint("usb_add_timer error\n");
		MP_ASSERT(0);
    }
    while (entry->next != &usb_timer_head)
	{
		//mpDebugPrint("%p %p",entry->next,newtimer);
        entry = entry->next;
	}

    newtimer->next = entry->next;
    entry->next->prev = newtimer;
    entry->next = newtimer;
    newtimer->prev = entry;
//	mpDebugPrint("<-- usb_add_timer2\n");
}

//************************************
// USB Timer task
//************************************
void usb_timer_task()
{
	DWORD wifiEvent;  	
	//mpDebugPrint("usb_timer_task\n");

	while(1)
	{
        wifiEvent = 0;  	
        EventWait(WIFI_EVENT, EVENT_WIFI_TIMER, OS_EVENT_OR, &wifiEvent);
        if (wifiEvent & EVENT_WIFI_TIMER)
        {
            usb_timer_proc();
        }

#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
    TaskSleep(1);
#else
    TaskYield();
#endif
 	}
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
int usb_submit_urb(struct urb *urb, unsigned int mem_flags)
{
	int			pipe, temp, max;
	struct usb_device	*dev;
	int			is_out;
	//PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
	//pUsbhDevDes = gpUsbhDeviceDescriptor;
	DWORD wOffset,i;
	//DWORD pbDataPage[5]; 
	DWORD buffadd;
    int ep;
    BYTE bEventId;
    short   idx;

	//mpDebugPrint("usb_submit_urb");
	//if (!urb || urb->hcpriv || !urb->complete)
	//	return -EINVAL;
	if (!(dev = urb->dev))
	//    (dev->state < USB_STATE_DEFAULT) ||
	//    (!dev->bus) || (dev->devnum <= 0))
		return -ENODEV;
	//if (dev->bus->controller->power.power_state.event != PM_EVENT_ON
	//		|| dev->state == USB_STATE_SUSPENDED)
	//	return -EHOSTUNREACH;
	urb->status = -EINPROGRESS;
	urb->actual_length = 0;
//	urb->bandwidth = 0;

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

    idx = dev->devnum;
    bEventId = UsbOtgHostDriverEventIdGet(idx);

    ep = usb_pipeendpoint(pipe);
    MP_DEBUG("%s: %s ep(%d)=%d,urb=%p", __func__, is_out ? "out" : "in", temp, ep, urb);
	/*
	 * Force periodic transfer intervals to be legal values that are
	 * a power of two (so HCDs don't need to).
	 *
	 * FIXME want bus->{intr,iso}_sched_horizon values here.  Each HC
	 * supports different values... this uses EHCI/UHCI defaults (and
	 * EHCI can use smaller non-default values).
	 */
		switch (temp) {
		case PIPE_INTERRUPT:
			if(is_out){
                usb_get_urb(urb);
				ListPushTail(&pUrbOutList_Int[idx],&urb->Link);

				EventSet(bEventId, EVENT_EHCI_ACTIVE_INTERRUPT);
			}
			else{
                usb_get_urb(urb);
				ListPushTail(&pUrbInList_Int[idx],&urb->Link);

				EventSet(bEventId, EVENT_EHCI_ACTIVE_INTERRUPT);
			}

			break;
		case PIPE_BULK:
			if(is_out){
                usb_get_urb(urb);
				//if(usb_pipeendpoint(pipe) == 0xa)
				//	pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum = 6;
				//else
				//	pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum = usb_pipeendpoint(pipe) - 1;
	            ep = usb_pipeendpoint(pipe);
                if(wifi_device_type == WIFI_USB_DEVICE_AR2524)
                {
#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
                    if (ep == EP_DATA_OUT)
                        ListPushTail(&pUrbOutList[idx][0],&urb->Link);
                    else
                        ListPushTail(&pUrbOutList[idx][1],&urb->Link);
#endif
                }
				else if(wifi_device_type == WIFI_USB_DEVICE_AR9271)
				{
					switch (ep)
					{
						case 1:
							ListPushTail(&pUrbOutList[idx][0],&urb->Link);
							break;
						case 4:
                            ListPushTail(&pUrbOutList_Int[idx],&urb->Link);
                            EventSet(bEventId, EVENT_EHCI_ACTIVE_INTERRUPT);
							break;
						case 5:
							ListPushTail(&pUrbOutList[idx][1],&urb->Link);
							break;
						case 6:
							ListPushTail(&pUrbOutList[idx][2],&urb->Link);
							break;
					}

                }
				else if(wifi_device_type == WIFI_USB_DEVICE_RTL8188C)
				{
					switch(ep)
					{
						case 2:
							ListPushTail(&pUrbOutList[idx][0], &urb->Link);
							break;
						case 3:
							ListPushTail(&pUrbOutList[idx][1], &urb->Link);
							break;
					}
				}
#if Make_USB == REALTEK_RTL8188E
				else if(wifi_device_type == WIFI_USB_DEVICE_RTL8188E)
				{
                    MP_ASSERT(ep == 2 || ep == 3);
					switch(ep)
					{
						case 2:
							ListPushTail(&pUrbOutList[idx][0], &urb->Link);
							break;
						case 3:
							ListPushTail(&pUrbOutList[idx][1], &urb->Link);
							break;
					}
				}
#endif
                else
                    ListPushTail(&pUrbOutList[idx][0],&urb->Link);

				EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
			}
			else{
                usb_get_urb(urb);
				if(wifi_device_type == WIFI_USB_DEVICE_AR9271)
				{
					switch (ep)
					{
						case 2:                 /* USB_WLAN_RX_PIPE */
							ListPushTail(&pUrbInList[idx],&urb->Link);
							break;
						case 3:                 /* USB_REG_IN_PIPE */
                            /* use right queue */
                            ListPushTail(&pUrbInList_Int[idx],&urb->Link);
                            EventSet(bEventId, EVENT_EHCI_ACTIVE_INTERRUPT);
							break;
                        default:
                            MP_ASSERT(0);
							break;
					}

                }
                else
                    ListPushTail(&pUrbInList[idx],&urb->Link);
				EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
			}

			break;
	    default:
	        return -EINVAL;
		}
#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
      TaskSleep(1);
#else
      TaskYield();
#endif
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
	unsigned long token;
	//mpDebugPrint("usb_urb_complete");
		
   	 token = *(unsigned long *)(((unsigned char *)qtd) + 8); /* token */
		
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
		
        MP_ALERT("usb_urb_complete status=%x,token=%x ",sts, token);
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
            mpDebugPrint("XactErr %d ",urb->status);
        else if (sts != QTD_STS_PING)
        	{
            mpDebugPrint("other err %d,sts=%x",urb->actual_length, sts);
        	}
    }
	
	usb_unanchor_urb(urb);

    MP_ASSERT(urb->complete);
    urb->complete(urb);
	usb_put_urb (urb);
}

#ifdef LINUX
unsigned int
RTUSBBulkInCount(unsigned long data)
{
	return (unsigned int)ListGetSize(&pUrbInList);
}
#endif

void UsbWifiBulkOnlyActive (WHICH_OTG eWhichOtg)
{
    
	int tmpdwBulkOnlyState = 0;
	struct urb *urb_out;
	struct urb *urb_in;
	//mpDebugPrint("UsbWifiBulkOnlyActive");
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	
	if( !(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_OUT_STATE))
    {
		if(!ListEmpty(&pUrbOutList[eWhichOtg][0]))
		{
		
#if 0
		 if((pUsbhDevDes->dwWhichBulkPipeDone&0xFFFF)==0)	// XXX
#endif
         {
			//UartOutText("1");
#if 0
            pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_OUT_STATE;	
#endif
			UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_OUT, &pUrbOutList[eWhichOtg][0], eWhichOtg);
		  }
	    }
		
		if(wifi_device_type == WIFI_USB_DEVICE_AR9271)
		{
			if(!ListEmpty(&pUrbOutList[eWhichOtg][1]))
			{
				{
					UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_OUT, &pUrbOutList[eWhichOtg][1], eWhichOtg);
				}
			}

			if(!ListEmpty(&pUrbOutList[eWhichOtg][2]))
			{
				{
					UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_OUT, &pUrbOutList[eWhichOtg][2], eWhichOtg);
				}
			}
		}
			
		if(wifi_device_type == WIFI_USB_DEVICE_RTL8188C)
		{
			if(!ListEmpty(&pUrbOutList[eWhichOtg][1]))
			{
				{
					UsbOtgBulkProcess(pUsbhDevDes, OTGH_DIR_OUT, &pUrbOutList[eWhichOtg][1], eWhichOtg);
				}
			}
		}

#if Make_USB == REALTEK_RTL8188E
		if(wifi_device_type == WIFI_USB_DEVICE_RTL8188E)
		{
			if(!ListEmpty(&pUrbOutList[eWhichOtg][1]))
			{
				{
					UsbOtgBulkProcess(pUsbhDevDes, OTGH_DIR_OUT, &pUrbOutList[eWhichOtg][1], eWhichOtg);
				}
			}
		}
#endif
	}
	if( wifi_device_type == WIFI_USB_DEVICE_AR2524 )
	{
	if( !(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_OUT_STATE2))
    {
		if(!ListEmpty(&pUrbOutList[eWhichOtg][1]))
		{
		
#if 0
		 if((pUsbhDevDes->dwWhichBulkPipeDone&0xFFFF)==0)	// XXX
#endif
         {
		
//			pUsbhDevDes->psAppClass->bBulkInQHDArrayNum = 0;
		
//			UartOutText("2");
#if 0
            pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_OUT_STATE2;	
#endif
			UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_OUT, &pUrbOutList[eWhichOtg][1], eWhichOtg);
		  }
	    }
		
			
	}
	}
	if(!(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_IN_STATE))
	{
	
        if(!ListEmpty(&pUrbInList[eWhichOtg]))
        {
	  	
#if 0
            if(pUsbhDevDes->dwWhichBulkPipeDone==0)
#endif
			{
#if 0
				pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_IN_STATE;	
#endif
                UsbOtgBulkProcess(pUsbhDevDes,OTGH_DIR_IN, &pUrbInList[eWhichOtg], eWhichOtg);
		 	}
	  	}	
	}
	
	


}

void UsbWifiBulkOnlyIoc (WHICH_OTG eWhichOtg)
{
    short  qHD_index = 0;
	MP_DEBUG("UsbWifiBulkOnlyIoc(%d)", eWhichOtg);

    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    BYTE bEventId = UsbOtgHostDriverEventIdGet(eWhichOtg);

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	//mpDebugPrint("pUsbhDevDes->psAppClass->dwBulkOnlyState %x ",pUsbhDevDes->psAppClass->dwBulkOnlyState);
	//mpDebugPrint("pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

    /* ----------  Bulk out  ---------- */
	if(pUsbhDevDes->dwWhichBulkPipeDone & 0x07)
    {
        //UartOutText("4");
		//__asm("break 100");
//#if USBOTG_HOST_DESC
//        qHD_index = pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum;
//#else
        if(pUsbhDevDes->dwWhichBulkPipeDone & 0x01)
        {
            qHD_index = 0;
            pUsbhDevDes->dwWhichBulkPipeDone &= ~1UL;
        }
        else if(pUsbhDevDes->dwWhichBulkPipeDone & 0x02)
        {
            qHD_index = 1;
            pUsbhDevDes->dwWhichBulkPipeDone &= ~2UL;
        }
        else if(pUsbhDevDes->dwWhichBulkPipeDone & 0x04)
        {
            qHD_index = 2;
            pUsbhDevDes->dwWhichBulkPipeDone &= ~4UL;
        }
		else
			MP_ALERT("pUsbhDevDes->dwWhichBulkPipeDone %x",pUsbhDevDes->dwWhichBulkPipeDone);
//#endif
		Wlan_flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[qHD_index], eWhichOtg);
		
        if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
        {
            MP_ALERT("error=0x%x", pUsbhDevDes->bQHStatus);
#if 0
            pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_OUT_STATE;
#endif
            if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
            {
                SWORD err = 0;
                MP_ASSERT(0);
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
                mpDebugPrint("WIFI_BULK_DATA_OUT_STATE:error = 0x%x", pUsbhDevDes->bQHStatus);
            }
        }
        else
        {
			pUsbhDevDes->psAppClass->dDataOutLen	 = 0;
//#if USBOTG_HOST_DESC
//            if(qHD_index == pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum) // Always TRUE
//#else
            if (!qHD_index)
//#endif    
            {

#if 0
                pUsbhDevDes->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_OUT_STATE;
#endif
                if (ListGetSize(&pUrbOutList[eWhichOtg][0]) > 0)
                    EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
            }
            else if (qHD_index == 1)
            {
                if (ListGetSize(&pUrbOutList[eWhichOtg][1]) > 0)
                    EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
            }
            else
            {
                pUsbhDevDes->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_OUT_STATE2;
                if (ListGetSize(&pUrbOutList[eWhichOtg][2]) > 0)
                    EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
            }

        }
			//EventSet(USB_BULK_COMPLETE_EVENT, 0x1);	
    }
        
    /* ----------  Bulk in  ---------- */
    if(pUsbhDevDes->dwWhichBulkPipeDone & 0x10000)
    {
        //UartOutText("5");

        pUsbhDevDes->dwWhichBulkPipeDone &= ~0x10000UL;
#if USBOTG_HOST_DESC
        Wlan_flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
#else
        Wlan_flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[0], eWhichOtg);
#endif        
        if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
        {
            pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_IN_STATE;
            if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
            {
		
                mpDebugPrint("HOST20_qTD_STATUS_Halted");
                SWORD err = 0;
                pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                        eWhichOtg);
                if (err != USB_NO_ERROR)
                { 
                    free1((void *)pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);						
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
#if 0
            pUsbhDevDes->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_IN_STATE;
#endif
			
            if ((ListGetSize(&pUrbInList[eWhichOtg]) > 0)&&(wifi_device_type != WIFI_USB_DEVICE_RT73))
                EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
        }
			//net_buf_mem_free(pUsbhDevDes->psAppClass->dDataBuffer);
			//EventSet(USB_BULK_COMPLETE_EVENT, 0x2);	

    }
//mpDebugPrint("-->pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

//pUsbhDevDes->dwWhichBulkPipeDone = 0;

//mpDebugPrint("pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

}

void UsbWifiInterruptActive(WHICH_OTG eWhichOtg)
{
    Host20_BufferPointerArray_Structure aTemp;
	struct urb *urb_out, *urb_in;
	short ep;
    WORD  hwSize = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    struct ehci_qtd *qtd;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	MP_DEBUG("UsbWifiInterruptActive %d",eWhichOtg);

	aTemp.BufferPointerArray[0] = 
	aTemp.BufferPointerArray[1] = 
	aTemp.BufferPointerArray[2] =
	aTemp.BufferPointerArray[3] =
	aTemp.BufferPointerArray[4] = 0;

	while (urb_in = (struct urb *)ListPopHead(&pUrbInList_Int[eWhichOtg])) 
	{
		pUsbhDev->urb = urb_in;
		ep = usb_pipeendpoint(urb_in->pipe);
		MP_DEBUG("UsbWifiInterruptActive: In ep=%d", ep); 
		pUsbhDev->psAppClass->dIntDataBuffer = (DWORD)urb_in->transfer_buffer;
		pUsbhDev->psAppClass->bIntDataInLen = urb_in->transfer_buffer_length;
	    aTemp.BufferPointerArray[0] = (DWORD)(urb_in->transfer_buffer);
        hwSize              = urb_in->transfer_buffer_length;
        if ((((DWORD)urb_in->transfer_buffer & 0xfffUL) + hwSize) > 0x1000)
        {
            /* ----------  over a 4KB page boundary  ---------- */
            aTemp.BufferPointerArray[1]       = ((DWORD)urb_in->transfer_buffer & ~0xfffUL) + 0x1000;
            //                MP_ALERT("UsbWifiInterruptActive: over 4KB=%p", aTemp.BufferPointerArray[1]); 
        }
        else
            aTemp.BufferPointerArray[1]       = 0;
//		MP_ALERT("UsbWifiInterruptActive: buf=%p, len=%d", aTemp.BufferPointerArray[0],
//				pUsbhDev->psAppClass->bIntDataInLen); 

		pUsbhDev->psAppClass->bIntInQHDArrayNum = 0;   // INT-IN

		qtd = (struct ehci_qtd *) Wlan_flib_Host20_Issue_Interrupt_Active( pUsbhDev->psAppClass->bIntInQHDArrayNum,
				pUsbhDev->psAppClass->bIntDataInLen, 
				(&aTemp.BufferPointerArray[0]), 
				0, 
				OTGH_DIR_IN,
				eWhichOtg);

		pUsbhDev->urb = NULL;
#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
      if(NetConfiged())
      	TaskSleep(1);
#endif	  
		
	}
	
	while (urb_out = (struct urb *)ListPopHead(&pUrbOutList_Int[eWhichOtg]))
	{
		pUsbhDev->urb = urb_out;
		ep = usb_pipeendpoint(urb_out->pipe);
		MP_DEBUG("UsbWifiInterruptActive: Out ep=%d", ep); 
#if 0
		pUsbhDev->psAppClass->dIntDataBuffer = urb_out->transfer_buffer;
		pUsbhDev->psAppClass->bIntDataInLen = urb_out->transfer_buffer_length;
#endif
	    aTemp.BufferPointerArray[0] = (DWORD)(urb_out->transfer_buffer);
        hwSize              = urb_out->transfer_buffer_length;
        if ((((DWORD)urb_out->transfer_buffer & 0xfffUL) + hwSize) > 0x1000)
        {
            /* ----------  over a 4KB page boundary  ---------- */
            aTemp.BufferPointerArray[1]       = ((DWORD)urb_out->transfer_buffer & ~0xfffUL) + 0x1000;
        }
        else
            aTemp.BufferPointerArray[1]       = 0;
//		MP_ALERT("UsbWifiInterruptActive: buf=%p, len=%d", aTemp.BufferPointerArray[0],
//				pUsbhDev->psAppClass->bIntDataInLen); 
		//NetPacketDump(aTemp.BufferPointerArray[0], hwSize);
		pUsbhDev->psAppClass->bIntOutQHDArrayNum = 0;
		Wlan_flib_Host20_Issue_Interrupt_Active( pUsbhDev->psAppClass->bIntOutQHDArrayNum,
				hwSize, 
				(&aTemp.BufferPointerArray[0]), 
				0, 
				OTGH_DIR_OUT,
				eWhichOtg);

		pUsbhDev->urb = NULL;
#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
     //if(NetConfiged())
     TaskSleep(1);
#endif
		
	}
}

void UsbWifiInterruptIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
//    MP_FUNCTION_ENTER();
    if (pUsbhDev->bCmdTimeoutError == 1)
    {
        pUsbhDev->bCmdTimeoutError = 0;
		MP_ASSERT(0);
        return;
    }
	MP_DEBUG("UsbWifiInterruptIoc %x", pUsbhDev->dwWhichInterruptPipeDone);

    if(pUsbhDev->dwWhichInterruptPipeDone & 0x1)
    {
		pUsbhDev->dwWhichInterruptPipeDone&=~(0x01);
        Wlan_flib_Host20_Send_qTD_Done(pUsbhDev->hstInterruptOutqHD[0], eWhichOtg);
    }
    if(pUsbhDev->dwWhichInterruptPipeDone & (1<<16))
    {
		pUsbhDev->dwWhichInterruptPipeDone&=~(1<<16);

        Wlan_flib_Host20_Send_qTD_Done(pUsbhDev->hstInterruptInqHD[0], eWhichOtg);
    }
//    MP_FUNCTION_EXIT();
	//mpDebugPrint("IOC done");
}
void UsbWifiSetupIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    //#if Make_USB
    if(pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)
        wifi_device_unplug = TRUE;
    //#endif
    SemaphoreRelease(USB_CONTROL_SEMA);
}
void UsbOtgBulkProcess(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes
	,BYTE	bDirection, PLM_LIST_CONTAINER pUrbList, WHICH_OTG eWhichOtg)
{
    DWORD pbDataPage[MAX_DATA_PAGE]; 
    DWORD wOneTdTotalLengthRemain = 0;
    DWORD wOffset = 0;
    WORD  hwSize = 0;
    DWORD i;
    BOOL  fUseMultiTDs = FALSE;
    BYTE  qHD_array_number;
    DWORD dwMaxTxLength = 0;
    DWORD dwBufferOffset = 0;
	struct urb *urb_out;
	struct urb *urb;
	struct urb *urb_in;
    struct ehci_qtd *qtd;
    short ep, cnt = 0;
    struct usb_device *udev =  &dev_global[eWhichOtg];
	int sz;
	short j;
    
	//mpDebugPrint("UsbOtgBulkProcess %x",pUsbhDevDes->psAppClass->bBulkInQHDArrayNum);
    for (i = 0; i < MAX_DATA_PAGE; i++)
        pbDataPage[i] = 0;

	
	if(wifi_device_type == WIFI_USB_DEVICE_RT73)
	{
		qHD_array_number = pUsbhDevDes->psAppClass->bBulkInQHDArrayNum;
		
		if(bDirection == OTGH_DIR_OUT)
		{
			while (urb_out = (struct urb *)ListPopHead(pUrbList))
			{
			    //mpDebugPrint("urb_out %x",urb_out);
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
													 eWhichOtg);
					pUsbhDevDes->urb = NULL;
					fUseMultiTDs = TRUE;
				}
		
				hwSize				= urb_out->transfer_buffer_length;
				pbDataPage[0]		= ((DWORD)(urb_out->transfer_buffer));
				if ((((DWORD)urb_out->transfer_buffer & 0xfffUL) + hwSize) > 0x1000)
				{
					/* ----------  over a 4KB page boundary  ---------- */
					//mpDebugPrint("URB IN over a 4KB page boundary");
					pbDataPage[1]		= ((DWORD)urb_out->transfer_buffer & ~0xfffUL) + 0x1000;
					//	MP_ALERT("UsbOtgBulkProcess: over 4KB=%p", pbDataPage[1]); 
					MP_ASSERT(qHD_array_number != 1);
				}
				else
					pbDataPage[1]		= 0;
				wOffset 			= 0;
				urb 				= urb_out;
				break;								/* disable multi for now */
			}
		}
		else
		{
			i = 0;
			while (urb_in = (struct urb *)ListPopHead(pUrbList))
			{
				if (pbDataPage[0])
				{
	//				mpDebugPrint("multi b %d", i++);
					pUsbhDevDes->urb = urb;
					qtd = (struct ehci_qtd *)flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
													 hwSize, 
													 &(pbDataPage[0]), 
													 wOffset, 
													 bDirection,
													 0,
													 eWhichOtg);
					pUsbhDevDes->urb = NULL;
					fUseMultiTDs = TRUE;
				}
	
	
				hwSize				= urb_in->transfer_buffer_length;
				pbDataPage[0]		= ((DWORD)(urb_in->transfer_buffer));
				
				if ((((DWORD)urb_in->transfer_buffer & 0xfffUL) + hwSize) > 0x1000)
				{
					/* ----------  over a 4KB page boundary  ---------- */
					//mpDebugPrint("URB IN over a 4KB page boundary");
					pbDataPage[1]		= ((DWORD)urb_in->transfer_buffer & ~0xfffUL) + 0x1000;
					//  MP_ALERT("UsbOtgBulkProcess: over 4KB=%p", pbDataPage[1]); 
					MP_ASSERT(qHD_array_number != 1);
				}
				else
					pbDataPage[1]		= 0;
				wOffset 			= 0;
				urb 				= urb_in;
			}
	
		}
	}
	else
	{
		i = 0;
        while (urb_out = (struct urb *)ListPopHead(pUrbList))
        {
            ep = usb_pipeendpoint(urb_out->pipe);
            cnt++;

            if (bDirection == OTGH_DIR_OUT)
                qHD_array_number = udev->ep_out[ep]->desc.bRefresh;
            else
                qHD_array_number = udev->ep_in[ep]->desc.bRefresh;

//            MP_ALERT("UsbOtgBulkProcess: ep=%d,idx=%d", ep, qHD_array_number); 
//            MP_ALERT("UsbOtgBulkProcess: ep=%d,urb=%p", ep, urb_out); 
            if (pbDataPage[0])
            {
			    //mpDebugPrint("multi %d", i++);
			    //UartOutText(" * ");

                pUsbhDevDes->urb = urb;
            	//hwSize = urb_out->transfer_buffer_length;
                qtd = (struct ehci_qtd *)Wlan_flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                 hwSize, 
                                                 &(pbDataPage[0]), 
                                                 wOffset, 
                                                 bDirection,
                                                 0,
                                                 eWhichOtg);
                pUsbhDevDes->urb = NULL;
                fUseMultiTDs = TRUE;
            }

            hwSize              = urb_out->transfer_buffer_length;
            pbDataPage[0]       = ((DWORD)(urb_out->transfer_buffer));
//            MP_ALERT("UsbOtgBulkProcess: OUT s=%p", pbDataPage[0]); 
//            MP_ALERT("UsbOtgBulkProcess: OUT e=%p", pbDataPage[0]+hwSize); 
            if ((((DWORD)urb_out->transfer_buffer & 0xfffUL) + hwSize) > 0x1000)
            {
                /* ----------  over a 4KB page boundary  ---------- */
                pbDataPage[1]       = ((DWORD)urb_out->transfer_buffer & ~0xfffUL) + 0x1000;
//                MP_ALERT("UsbOtgBulkProcess: over 4KB=%p", pbDataPage[1]); 
                sz = (int)hwSize - (pbDataPage[1] - pbDataPage[0]) - 0x1000;
                pbDataPage[2]       = 
                pbDataPage[3]       = 
                pbDataPage[4]       = 0;
                for (j=2; j<5 && sz >0; j++, sz -= 0x1000)
                {
                    pbDataPage[j]   = pbDataPage[j-1] + 0x1000;
                    MP_DEBUG("UsbOtgBulkProcess: pbDataPage[j]=%p", pbDataPage[j]); 
                }
            }
            else
			{
                pbDataPage[1]       = 
                pbDataPage[2]       = 
                pbDataPage[3]       = 
                pbDataPage[4]       = 0;
			}
#if 0
            if (((unsigned long)(pbDataPage[0]) & 3))
                MP_ALERT("UsbOtgBulkProcess: unaligned=%p", pbDataPage[0]); 
            MP_VALIDATE_POINTER(pbDataPage[0]);
#endif
            wOffset             = 0;
            urb                 = urb_out;
#if RX_URBS_COUNT == 1
            break;                              /* disable multi for now */
#endif
#if (DEMO_PID&&(DM9621_ETHERNET_ENABLE==0))
     if(NetConfiged())
     TaskSleep(1);
#endif
        }
    }

    MP_ASSERT(pbDataPage[0]);
	
//	if(wifi_device_type == WIFI_USB_DEVICE_AR2524)
	{
	    if (cnt == 0)               /* no URBs */
	    {
	        return;
	}
	}

    if (fUseMultiTDs)
    { // for multi-TD
//    mpDebugPrint("multi c");
        pUsbhDevDes->urb = urb;
        //hwSize = urb_out->transfer_buffer_length;
		qtd = (struct ehci_qtd *)Wlan_flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
                                                 hwSize, 
                                                 &(pbDataPage[0]), 
                                                 wOffset, 
                                                 bDirection,
                                                 1,
                                                 eWhichOtg);
        pUsbhDevDes->urb = NULL;
    }
    else
    { // for single-TD 
	        pUsbhDevDes->urb = urb;
	//        MP_ALERT("UsbOtgBulkProcess: idx=%d", qHD_array_number); 
	//        NetPacketDump(pbDataPage[0], hwSize);
	        qtd = (struct ehci_qtd *)Wlan_flib_Host20_Issue_Bulk_Active ( qHD_array_number,
	                                        hwSize, 
	                                        &(pbDataPage[0]), 
	                                        wOffset, 
	                                        bDirection,
	                                        eWhichOtg);
	        pUsbhDevDes->urb = NULL;

    }
}

static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;

static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;

void WifiStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
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


    if (eWhichOtg != WIFI_USB_PORT)
    {
        MP_ALERT("-USBOTG- %s() WiFi dongle does not support on USBOTG%d!!", __FUNCTION__, eWhichOtg);
        __asm("break 100");
        return;
    }
    
    pUsbhDevDes     = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(WIFI_USB_PORT);//gpUsbhDeviceDescriptor;
	mpDebugPrint("WifiStateMachine");
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
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
#if Make_USB == REALTEK_RTL8188CU || Make_USB == REALTEK_RTL8188E
            /* rtl8188 series don't use interrupt pipe: fixed by joeluo */
#else
			flib_Host20_Interrupt_Init(eWhichOtg);  // For enable Interrupt EP 
#endif
			pUsbhDevDes->psAppClass->sBulkInDescriptor.wMaxPacketSize = 0x940;
			pUsbhDevDes->psAppClass->dDataInLen = 0x40;
			pUsbhDevDes->bDeviceAddress = 1;
			pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress = 1;
			pUsbhDevDes->psAppClass->sBulkInDescriptor.bmAttributes = OTGH_ED_BULK;
            pUsbhDevDes->psAppClass->dwBulkOnlyState = 0; /* reset the state */
			UR_OTGH_PT_Bulk_Init(eWhichOtg);
			pSendMailDrv->wCurrentExecutionState	= WIFI_INIT_STATE;
			pSendMailDrv->wStateMachine 			= WIFI_SM;
	        break;
		
        case WIFI_INIT_STATE:

            if (UsbOtgHostConnectStatusGet(eWhichOtg))
            { // plug in
                wifi_device_plugin = TRUE;
                mpDebugPrint("WIFI Plug-in\r\n");
                    if (NetDeviceAttachEvent_Fn)
                        NetDeviceAttachEvent_Fn(
                                le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
                                le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
                                TRUE);
				SystemDeviceInit(USB_WIFI_DEVICE);
            }
            else
            { // plug out
                mpDebugPrint("WIFI init:Plug-out\r\n");
				wifi_device_plugin = FALSE;

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
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
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

static struct usb_interface   intf_global;
static struct usb_host_interface _altsetting;
static struct usb_host_endpoint _pEP[4];

struct usb_host_endpoint ep_out_buf[6+1];
struct usb_host_endpoint ep_in_buf[6+1];

struct usb_interface *getUsbInterface()
{
    return &intf_global;
}
extern PST_USBH_DEVICE_DESCRIPTOR g_pUsbhDevDes;
struct usb_device *usb_data_setup(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    struct usb_device_descriptor *desc;    
	mpDebugPrint("--> %s\n", __func__);
    pUsbhDevDes     = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);//gpUsbhDeviceDescriptor;
	struct usb_device *udev = &dev_global[eWhichOtg];
    struct usb_interface   *intf = &intf_global;
    short i, n, ep_num;
    BYTE direction;
	
    memcpy(&dev_global[eWhichOtg].descriptor, &pUsbhDevDes->sDeviceDescriptor, sizeof dev_global[eWhichOtg].descriptor);
	SET_NETDEV_DEV(intf, &dev_global[eWhichOtg].dev);
    desc = &dev_global[eWhichOtg].descriptor;
    udev->devnum = eWhichOtg;
    udev->speed = pUsbhDevDes->bDeviceSpeed + 1;
    udev->intf = intf;
    mpDebugPrint("Device Speed: %d", udev->speed);

    mpDebugPrint("Device Desc: =============");
    mpDebugPrint("bLength = %d",desc->bLength);
    mpDebugPrint("bDescriptorType = %d",desc->bDescriptorType);
    mpDebugPrint("bcdUSB = %x",desc->bcdUSB);
    mpDebugPrint("bDeviceClass = %x",desc->bDeviceClass);
    mpDebugPrint("bDeviceSubClass = %x",desc->bDeviceSubClass);
    mpDebugPrint("bDeviceProtocol = %x",desc->bDeviceProtocol);
    mpDebugPrint("bMaxPacketSize0 = %x",desc->bMaxPacketSize0);
    mpDebugPrint("bcdDevice = %x",desc->bcdDevice);
    mpDebugPrint("iManufacturer = %x",desc->iManufacturer);
    mpDebugPrint("iProduct = %x",desc->iProduct);
    mpDebugPrint("iSerialNumber = %x",desc->iSerialNumber);
    mpDebugPrint("bNumConfigurations = %x",desc->bNumConfigurations);

#if USBOTG_HOST_DESC
    n = GetInterfaceEPs(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);
#else    
    n = pUsbhDevDes->sInterfaceDescriptor[0].bNumEndpoints;
#endif
    mpDebugPrint("bNumEndpoints = %x",n);

    for (i = 0; i < n; i++)
    {
        PUSB_ENDPOINT_DESCRIPTOR ep_desc;
        
#if USBOTG_HOST_DESC
        ep_desc = GetEndpointStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx, i);
        ep_num = ep_desc->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
        direction = ep_desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK;
#else 
        ep_desc = &(pUsbhDevDes->sEndpointDescriptor[i]);
        ep_num = (pUsbhDevDes->sEndpointDescriptor[i].bEndpointAddress)&USB_ENDPOINT_NUMBER_MASK,
        direction = ((pUsbhDevDes->sEndpointDescriptor[i].bEndpointAddress)&USB_ENDPOINT_DIR_MASK);
#endif    

        if (direction == USB_DIR_OUT)
        {
            memcpy(&ep_out_buf[ep_num].desc, ep_desc, sizeof *ep_desc);
            udev->ep_out[ep_num] = &ep_out_buf[ep_num];
        }
        else
        {
            memcpy(&ep_in_buf[ep_num].desc, ep_desc, sizeof *ep_desc);
            udev->ep_in[ep_num] = &ep_in_buf[ep_num];
        }
    }

#if 1
	USB_INTERFACE_DESCRIPTOR *pUsbIFDesc;
	pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);

    if (pUsbIFDesc)
    {
        USB_ENDPOINT_DESCRIPTOR *pUsbEPDesc;

        intf->altsetting = &_altsetting;
        intf->altsetting->desc.bLength = pUsbIFDesc->bLength;
        intf->altsetting->desc.bDescriptorType = pUsbIFDesc->bDescriptorType;
        intf->altsetting->desc.bInterfaceNumber = pUsbIFDesc->bInterfaceNumber;
        intf->altsetting->desc.bAlternateSetting = pUsbIFDesc->bAlternateSetting;
        intf->altsetting->desc.bNumEndpoints = pUsbIFDesc->bNumEndpoints;
        intf->altsetting->desc.bInterfaceClass = pUsbIFDesc->bInterfaceClass;
        intf->altsetting->desc.bInterfaceSubClass = pUsbIFDesc->bInterfaceSubClass;
        intf->altsetting->desc.bInterfaceProtocol = pUsbIFDesc->bInterfaceProtocol;
        intf->altsetting->desc.iInterface = pUsbIFDesc->iInterface;

        intf->num_altsetting = 1;               /* always one ? XXX */

#if Make_USB == REALTEK_RTL8188E
        struct usb_host_endpoint *pEP = &_pEP[3];
        short bep_count;
        /* fill parameter value of Endpoint Configure to pusb_intf->altsetting->endpoint->desc */
        for (bep_count = n ; bep_count > 0; bep_count--)
        {
            pUsbEPDesc = (USB_ENDPOINT_DESCRIPTOR *)(pUsbIFDesc->pEndpoint + bep_count - 1);
            pEP->desc.bLength = pUsbEPDesc->bLength;
            pEP->desc.bDescriptorType = pUsbEPDesc->bDescriptorType;
            pEP->desc.bEndpointAddress = pUsbEPDesc->bEndpointAddress;
            pEP->desc.bmAttributes = pUsbEPDesc->bmAttributes;
            pEP->desc.wMaxPacketSize = 0xFFFF & byte_swap_of_word(pUsbEPDesc->wMaxPacketSize);
            pEP->desc.bInterval = pUsbEPDesc->bInterval;
            pEP->desc.bRefresh = 0;
            pEP->desc.bSynchAddress = 0;
            pEP--;
        }

        intf->altsetting->endpoint= &_pEP[0];
#endif
    }
#endif

	mpDebugPrint("<-- %s\n", __func__);

    g_pUsbhDevDes = pUsbhDevDes;
    return udev;
}

void usb_data_cleanup(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
	mpDebugPrint("--> %s\n", __func__);
    pUsbhDevDes     = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);//gpUsbhDeviceDescriptor;
	struct usb_device *udev = &dev_global[eWhichOtg];
    struct usb_interface   *intf = &intf_global;

    memset(udev->ep_out, 0, sizeof udev->ep_out);
    memset(udev->ep_in, 0, sizeof udev->ep_in);

	mpDebugPrint("<-- %s\n", __func__);
}

/* ----------  kernel/sched.c  ---------- */

void wait_for_completion(struct completion *x)
{
#if 0
    SemaphoreWait(x->wait);
#else
    if (!x->done)
    {
        mutex_lock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);
        list_add_tail(&x->cleanup, &urb_wait_taskqueue[x->udev_index].task_list);
        mutex_unlock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);

        x->wait = TaskGetId();
        TaskSleep(0);

        mutex_lock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);
		list_del_init(&x->cleanup);
        mutex_unlock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);
    }
#endif
//    mpDebugPrint("%s: x=%p,s=%d", __func__, x, x->wait);
    MP_DEBUG("%s: done", __func__);
}

unsigned long 
wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
#if 0
	MP_ASSERT(x->wait);
    SemaphoreWait(x->wait);            /* TODO */
#else
    if (!x->done)
    {
        mutex_lock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);
        list_add_tail(&x->cleanup, &urb_wait_taskqueue[x->udev_index].task_list);
        mutex_unlock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);

        x->wait = TaskGetId();
#if 0
        TaskSleep(jiffies_to_msecs(timeout));
#else
    if(UsbOtgCheckPlugOut(x->udev_index)) // device plug out
        wifi_device_unplug = TRUE;
    else
        TaskSleep(10 * 1000);              /* 10 sec */
    x->wait = 0;
#endif

        mutex_lock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);
		list_del_init(&x->cleanup);
        mutex_unlock(&urb_wait_taskqueue[x->udev_index].task_list_mutex);
    }
#endif
//    mpDebugPrint("%s: x=%p,s=%d", __func__, x, x->wait);
    MP_DEBUG("%s: done=%d", __func__, x->done);
	if (!x->done)
    {
        return 0;                     /* 0 indicates timeout */
    }
    x->done--;
	return 1;
}

void complete(struct completion *x)
{
	unsigned long flags;

//    MP_FUNCTION_ENTER();
//    mpDebugPrint("%s: x=%p", __func__, x);
#ifdef LINUX
	spin_lock_irqsave(&x->wait.lock, flags);    // TODO
#endif
	x->done++;
//	MP_ASSERT(x->wait);
#if 0
    SemaphoreRelease(x->wait);
#else
    if (x->wait)
        TaskWakeup(x->wait);
#endif
#ifdef LINUX
	spin_unlock_irqrestore(&x->wait.lock, flags);
#endif
//    MP_FUNCTION_EXIT();
}

/* ----------  usb/core/message.c  ---------- */

struct api_context {
	struct completion	done;
	int			status;
};

static void usb_api_blocking_completion(struct urb *urb)
{
	struct api_context *ctx = urb->context;

	ctx->status = urb->status;
	complete(&ctx->done);
}

/*
 * Starts urb and waits for completion or timeout. Note that this call
 * is NOT interruptible. Many device driver i/o requests should be
 * interruptible and therefore these drivers should implement their
 * own interruptible routines.
 */
static int usb_start_wait_urb(struct urb *urb, int timeout, int *actual_length)
{ 
	struct api_context ctx;
	unsigned long expire;
	int retval;
    struct usb_device *udev = urb->dev;

//    MP_FUNCTION_ENTER();
	init_completion(&ctx.done);
	ctx.done.udev_index = udev->devnum;
    ctx.status = -ENOENT;
	urb->context = &ctx;
	urb->actual_length = 0;
	retval = usb_submit_urb(urb, GFP_NOIO);
	if (unlikely(retval))
		goto out;

//	MP_TRACE_LINE();
	expire = timeout ? msecs_to_jiffies(timeout) : 0;
	if (!wait_for_completion_timeout(&ctx.done, expire)) {
        
        if(UsbOtgCheckPlugOut(udev->devnum)) // device plug out
            wifi_device_unplug = TRUE;
        
		usb_kill_urb(urb);
		retval = (ctx.status == -ENOENT ? -ETIMEDOUT : ctx.status);
        if (retval)
            mpDebugPrint("%s: retval=%d,sts=%d\n", __func__, retval, ctx.status);

		dev_dbg(&urb->dev->dev,
			"%s timed out on ep%d%s len=%d/%d\n",
			current->comm,
			usb_pipeendpoint(urb->pipe),
			usb_pipein(urb->pipe) ? "in" : "out",
			urb->actual_length,
			urb->transfer_buffer_length);
		MP_ALERT("%s timed out on ep%d%s len=%d/%d,tm=%d\n",
			__func__,
			usb_pipeendpoint(urb->pipe),
			usb_pipein(urb->pipe) ? "in" : "out",
			urb->actual_length,
			urb->transfer_buffer_length,
            expire);
	} else
	{
		retval = ctx.status;
		MP_ASSERT(retval == 0);
	}
//    MP_FUNCTION_EXIT();
out:
//    MP_FUNCTION_EXIT();
	if (actual_length)
		*actual_length = urb->actual_length;

	usb_free_urb(urb);
	return retval;
}

int usb_reset_configuration(struct usb_device *dev)
{
	int			i, retval;
#ifdef LINUX
	struct usb_host_config	*config;

	if (dev->state == USB_STATE_SUSPENDED)
		return -EHOSTUNREACH;

	/* caller must have locked the device and must own
	 * the usb bus readlock (so driver bindings are stable);
	 * calls during probe() are fine
	 */

	for (i = 1; i < 16; ++i) {
		usb_disable_endpoint(dev, i);
		usb_disable_endpoint(dev, i + USB_DIR_IN);
	}

	config = dev->actconfig;
#endif
	retval = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			USB_REQ_SET_CONFIGURATION, 0,
#ifdef LINUX
			config->desc.bConfigurationValue, 0,
#else
			1, 0,
#endif
			NULL, 0, USB_CTRL_SET_TIMEOUT);
	if (retval < 0)
		return retval;

#ifdef LINUX
	dev->toggle[0] = dev->toggle[1] = 0;

	/* re-init hc/hcd interface/endpoint state */
	for (i = 0; i < config->desc.bNumInterfaces; i++) {
		struct usb_interface *intf = config->interface[i];
		struct usb_host_interface *alt;

		usb_remove_sysfs_intf_files(intf);
		alt = usb_altnum_to_altsetting(intf, 0);

		/* No altsetting 0?  We'll assume the first altsetting.
		 * We could use a GetInterface call, but if a device is
		 * so non-compliant that it doesn't have altsetting 0
		 * then I wouldn't trust its reply anyway.
		 */
		if (!alt)
			alt = &intf->altsetting[0];

		intf->cur_altsetting = alt;
		usb_enable_interface(dev, intf);
		if (device_is_registered(&intf->dev))
			usb_create_sysfs_intf_files(intf);
	}
#endif
	return 0;
}

void urb_queue_init(struct usb_device *udev)
{
    short i = 0, k;
    if (udev)
        i = udev->devnum;
    for (k=0; k<MAX_BULK_OUT_ENDPOINTS; k++)
        ListInitList(&pUrbOutList[i][k]);

    ListInitList(&pUrbInList[i]);
#if USBOTG_HOST_DATANG
	if (!(le16_to_cpu(udev->descriptor.idVendor) == TDSCDMA_USB_DEVICE_VID &&
		le16_to_cpu(udev->descriptor.idProduct) == TDSCDMA_USB_PID))
#endif
	{
		ListInitList(&pUrbOutList_Int[i]);
		ListInitList(&pUrbInList_Int[i]);
	}
}

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI||Make_USB == AR9271_WIFI || Make_USB == REALTEK_RTL8188CU || Make_USB == REALTEK_RTL8188E
void urb_cleanup_init(struct usb_device *udev)
{
    short i = 0;
    if (udev)
        i = udev->devnum;
    /* TODO */
	mutex_init(&urb_wait_taskqueue[i].task_list_mutex);
	INIT_LIST_HEAD(&urb_wait_taskqueue[i].task_list);
}
#endif

/*
 * Don't use mutex_init
 */
void urb_cleanup_init2(struct usb_device *udev)
{
    struct mutex *m;

    m = &urb_wait_taskqueue[udev->devnum].task_list_mutex;
    if (m->semaphore == 0)
    {
        m->semaphore = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        atomic_set(&(m)->count, 1);
    }
	INIT_LIST_HEAD(&urb_wait_taskqueue[udev->devnum].task_list);
}

/*
 * Unfortunately, our USB driver doesn't return pending URBs after USB dongle
 * is removed.  So we have to clean the mess ourselves.
 */
void urb_cleanup_run(struct usb_device *udev)
{
    struct completion *x;
	int timeout = 40;
    PLM_LIST_CONTAINER pList;

    mutex_lock(&urb_wait_taskqueue[udev->devnum].task_list_mutex);
	while (!list_empty(&urb_wait_taskqueue[udev->devnum].task_list)) {
		x = list_first_entry(&urb_wait_taskqueue[udev->devnum].task_list, struct completion, cleanup);
		list_del_init(&x->cleanup);
        mutex_unlock(&urb_wait_taskqueue[udev->devnum].task_list_mutex);

		if(timeout<0)
			break;

        if (x->wait > 0)
        {
            mpDebugPrint("%s: task ID=%d\n", x->wait);
            TaskWakeup(x->wait);
        }
        mutex_lock(&urb_wait_taskqueue[udev->devnum].task_list_mutex);
		UartOutText(" T ");
		timeout--;
    }
    mutex_unlock(&urb_wait_taskqueue[udev->devnum].task_list_mutex);


    /* ----------  clean up urb queues  ---------- */
    urb_queue_init(udev);
}
#if Make_USB
void usb_wifi_unplug(void)
{
  wifi_device_unplug = TRUE;
  switch(wifi_device_type)
  {
#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
   	 case WIFI_USB_DEVICE_AR2524:
	 	zd_usb_wifi_unplug();
	 	break;
#endif		
#if Make_USB == RALINK_WIFI||Make_USB == RALINK_AR2524_WIFI
	case WIFI_USB_DEVICE_RT73:
		usb_rtusb_disconnect();
	   break;
#endif	   
#if Make_USB == AR9271_WIFI
   	 case WIFI_USB_DEVICE_AR9271:
        __asm("break 100");
		usb_ar9271_unplug();
	 	break;
#endif		
     default:
		MP_ASSERT(0);
	 	break;
  }
}
#endif
static void CommandProcess (void *pMcardDev)
{
	ST_MCARD_MAIL	*psMcardRMail;
	ST_MCARD_DEV	*pDev = pMcardDev;
	DWORD usbcmd;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    WHICH_OTG eWhichOtg;
	
	psMcardRMail = pDev->sMcardRMail;
    if (pDev->wMcardType == DEV_USB_ETHERNET_DEVICE)
        eWhichOtg = ethernet_usbotg;            /* Ethernet dongle can be in either USBOTG0 or USBOTG1 */
    else
        eWhichOtg = WIFI_USB_PORT;
    mpDebugPrint("\nCommandProcess usb=%d\n", eWhichOtg);
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);	
	//mpDebugPrint("\nCommandProcess\n");
	switch (psMcardRMail->wCmd)
	{
		case INIT_CARD_CMD:
			/* note: File System had always set drv->Flag.Present as 1 for non-storage or non-FAT devices (ex: WiFi, USB_PTP) for always passing card present check */
			//pDev->Flag.Present	= 1;
 			
			mpDebugPrint("\nWIFI/Ethernet INIT_CARD_CMD\n");
			//usbcmd = mwHost20_USBCMD_IntThreshold_Rd();
			//mpDebugPrint("usbcmd %x",usbcmd);
			//usbcmd = 0x04;
			//mbHost20_USBCMD_IntThreshold_Set(usbcmd);
			//usbcmd = mbHost20_USBCMD_FrameListSize_Rd();
			//mpDebugPrint("usbcmd FrameListSize %x",usbcmd);
			//usbcmd = 0x01;
			//mbHost20_USBCMD_FrameListSize_Set(usbcmd);
#if DM9621_ETHERNET_ENABLE
			if( (pUsbhDevDes->sDeviceDescriptor.idVendor==(WORD)byte_swap_of_word(DM_USB_DEVICE_VID))&&((pUsbhDevDes->sDeviceDescriptor.idProduct==(WORD)byte_swap_of_word(DM9621_USB_PID))||
				(pUsbhDevDes->sDeviceDescriptor.idProduct==(WORD)byte_swap_of_word(DM9621_USB_OTHER_PID))))
			{
				mpDebugPrint("DM 9621 Device");
			    dm9621_netpool_init((int)eWhichOtg);
				wifi_device_type = ETHERNET_USB_DEVICE_DM9621;
				pUsbhDevDes->isWlan = TRUE;
				Wlan_SetDummy(eWhichOtg);
				pUsbhDevDes->fpAppClassBulkActive	= UsbWifiBulkOnlyActive;
				pUsbhDevDes->fpAppClassBulkIoc		= UsbWifiBulkOnlyIoc;
				pUsbhDevDes->fpAppClassSetupIoc     = UsbWifiSetupIoc; /* for control transfer */
				pUsbhDevDes->fpAppClassInterruptActive = UsbWifiInterruptActive;
				pUsbhDevDes->fpAppClassInterruptIoc    = UsbWifiInterruptIoc;
				//pUsbhDevDes->psAppClass->dwBulkOnlyState = 0; /* reset the state */
				usbnet_wifi_dev[eWhichOtg] = usb_data_setup(eWhichOtg);
#if 0
				if (NetDeviceAttachEvent_Fn)
					NetDeviceAttachEvent_Fn(
							le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
							le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
							TRUE);
#endif
				dm9620_bind(eWhichOtg);
				break;
			}
#endif //#if DM9621_ETHERNET_ENABLE
#if Make_USB == RALINK_WIFI||Make_USB == RALINK_AR2524_WIFI

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
			rt73_netpool_init();
			    //UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end, eWhichOtg);
				wifi_device_type = WIFI_USB_DEVICE_RT73;
				pUsbhDevDes->isWlan = TRUE;
                Wlan_SetDummy(eWhichOtg);
				pUsbhDevDes->fpAppClassBulkActive	= UsbWifiBulkOnlyActive;
				pUsbhDevDes->fpAppClassBulkIoc		= UsbWifiBulkOnlyIoc;
				pUsbhDevDes->fpAppClassSetupIoc     = UsbWifiSetupIoc; /* for control transfer */
					
					if (NetDeviceAttachEvent_Fn)
						NetDeviceAttachEvent_Fn(
								le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
								le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
								TRUE);
			    usb_rtusb_probe();
                break;
			}
#endif

#if Make_USB == AR2524_WIFI||Make_USB == RALINK_AR2524_WIFI
			if(((pUsbhDevDes->sDeviceDescriptor.idVendor==cpu_to_le16(AR2524_USB_DEVICE1_VID))&&
				(pUsbhDevDes->sDeviceDescriptor.idProduct==cpu_to_le16(AR2524_USB_DEVICE1_PID))))
				{
				    ar2524_netpool_init();
					//UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end);
					wifi_device_type = WIFI_USB_DEVICE_AR2524;
					pUsbhDevDes->isWlan = TRUE;
                    Wlan_SetDummy(eWhichOtg);
					pUsbhDevDes->fpAppClassBulkActive	= UsbWifiBulkOnlyActive;
					pUsbhDevDes->fpAppClassBulkIoc		= UsbWifiBulkOnlyIoc;
					pUsbhDevDes->fpAppClassSetupIoc     = UsbWifiSetupIoc; /* for control transfer */
					pUsbhDevDes->fpAppClassInterruptActive = UsbWifiInterruptActive;
					pUsbhDevDes->fpAppClassInterruptIoc    = UsbWifiInterruptIoc;
					
					usbnet_wifi_dev[eWhichOtg] = usb_data_setup(eWhichOtg);
					if (NetDeviceAttachEvent_Fn)
						NetDeviceAttachEvent_Fn(
								le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
								le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
								TRUE);
					if(usb_ar2524_init(eWhichOtg)>= 0)
					NetDriverUpEventSet(NIC_INDEX_WIFI);
                    break;
				}
#endif		

#if Make_USB == AR9271_WIFI
			struct usb_driver *udriver;
			if (udriver = usb_get_driver(le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
						le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct)))
				{
					ar9271_netpool_init(eWhichOtg);
					wifi_device_type = WIFI_USB_DEVICE_AR9271;
					pUsbhDevDes->isWlan = TRUE;
                    Wlan_SetDummy(eWhichOtg);
					pUsbhDevDes->fpAppClassBulkActive	= &UsbWifiBulkOnlyActive;
					pUsbhDevDes->fpAppClassBulkIoc		= &UsbWifiBulkOnlyIoc;
					pUsbhDevDes->fpAppClassSetupIoc = &UsbWifiSetupIoc; /* for control transfer */
					pUsbhDevDes->fpAppClassInterruptActive = &UsbWifiInterruptActive;
					pUsbhDevDes->fpAppClassInterruptIoc    = &UsbWifiInterruptIoc;
					
					usbnet_wifi_dev[eWhichOtg] = usb_data_setup(eWhichOtg);
					if (NetDeviceAttachEvent_Fn)
						NetDeviceAttachEvent_Fn(
								le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
								le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
								TRUE);
					if(usb_ar9271_init(udriver, &intf_global, usbnet_wifi_dev[eWhichOtg])>= 0)
					NetDriverUpEventSet(NIC_INDEX_WIFI);
					break;
				}
#endif	
#if Make_USB == REALTEK_RTL8188CU
			struct usb_driver *udriver;
			if (udriver = usb_get_driver(le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
						le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct)))
			{
				rtl8188c_netpool_init(eWhichOtg);
				wifi_device_type = WIFI_USB_DEVICE_RTL8188C;
				pUsbhDevDes->isWlan = TRUE;
				Wlan_SetDummy(eWhichOtg);
				pUsbhDevDes->fpAppClassBulkActive	= &UsbWifiBulkOnlyActive;
				pUsbhDevDes->fpAppClassBulkIoc	= &UsbWifiBulkOnlyIoc;
				pUsbhDevDes->fpAppClassSetupIoc 	= &UsbWifiSetupIoc; /* for control transfer */
				pUsbhDevDes->fpAppClassInterruptActive 	= &UsbWifiInterruptActive;
				pUsbhDevDes->fpAppClassInterruptIoc    	= &UsbWifiInterruptIoc;
					
				usbnet_wifi_dev[eWhichOtg] = usb_data_setup(eWhichOtg);					
				if (NetDeviceAttachEvent_Fn)
					NetDeviceAttachEvent_Fn(
							le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
							le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
							TRUE);
				if(usb_r8188c_init(udriver, &intf_global, eWhichOtg) >= 0)
					NetDriverUpEventSet(NIC_INDEX_WIFI);
				else
					BreakPoint();
				//test_scan();
				break;
			}
#endif	//REALTEK_RTL8188CU

#if Make_USB == REALTEK_RTL8188E
			struct usb_driver *udriver;
			if (udriver = usb_get_driver(le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
						le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct)))
			{
				rtl8188e_netpool_init(eWhichOtg);
				wifi_device_type = WIFI_USB_DEVICE_RTL8188E;
				pUsbhDevDes->isWlan = TRUE;
				Wlan_SetDummy(eWhichOtg);
				pUsbhDevDes->fpAppClassBulkActive	= &UsbWifiBulkOnlyActive;
				pUsbhDevDes->fpAppClassBulkIoc	= &UsbWifiBulkOnlyIoc;
				pUsbhDevDes->fpAppClassSetupIoc 	= &UsbWifiSetupIoc; /* for control transfer */
				pUsbhDevDes->fpAppClassInterruptActive 	= &UsbWifiInterruptActive;
				pUsbhDevDes->fpAppClassInterruptIoc    	= &UsbWifiInterruptIoc;
					
				usbnet_wifi_dev[eWhichOtg] = usb_data_setup(eWhichOtg);					
				if (NetDeviceAttachEvent_Fn)
					NetDeviceAttachEvent_Fn(
							le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
							le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
							TRUE);
				if(usb_r8188e_init(udriver, &intf_global, eWhichOtg) >= 0)
					NetDriverUpEventSet(NIC_INDEX_WIFI);
				else
					BreakPoint();
				break;
			}
#endif	//REALTEK_RTL8188E

            #if USBOTG_HOST_CDC
            // Qisda 3.5G modem   CDC_PID:0x4522
             if((pUsbhDevDes->sDeviceDescriptor.idVendor==cpu_to_le16(HSUPA_USB_DEVICE_VID))&&
				(pUsbhDevDes->sDeviceDescriptor.idProduct==cpu_to_le16(HSUPA_USB_CDC_PID)))
            {
					//netpool_mem_init(AR2524_MAX_NUM_BUFFERS,AR2524_MAX_NETPOOL_ALLOC_SIZE);
					//UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end);
					//wifi_device_type = WIFI_USB_DEVICE_AR2524;
					pUsbhDevDes->isWlan = TRUE;
					Wlan_SetDummy(eWhichOtg);
					pUsbhDevDes->fpAppClassBulkActive	= &UsbWifiBulkOnlyActive;
					pUsbhDevDes->fpAppClassBulkIoc		= &UsbWifiBulkOnlyIoc;
					pUsbhDevDes->fpAppClassSetupIoc = &UsbWifiSetupIoc; /* for control transfer */
					pUsbhDevDes->fpAppClassInterruptActive = &UsbWifiInterruptActive;
					pUsbhDevDes->fpAppClassInterruptIoc    = &UsbWifiInterruptIoc;

					//add
					pUsbhDevDes->psAppClass->dwBulkOnlyState = 0; /* reset the state */
				}
            #endif
            #if USBOTG_HOST_DATANG
			// Datang 3G modem
		if((pUsbhDevDes->sDeviceDescriptor.idVendor==cpu_to_le16(TDSCDMA_USB_DEVICE_VID))&&
			(pUsbhDevDes->sDeviceDescriptor.idProduct==cpu_to_le16(TDSCDMA_USB_PID)))
            	{
            		pUsbhDevDes->isWlan = TRUE;
			Wlan_SetDummy(eWhichOtg);
			pUsbhDevDes->fpAppClassBulkActive	= &UsbWifiBulkOnlyActive;
			pUsbhDevDes->fpAppClassBulkIoc	= &UsbWifiBulkOnlyIoc;
			pUsbhDevDes->fpAppClassSetupIoc    = &UsbWifiSetupIoc; /* for control transfer */
			
			//add
			pUsbhDevDes->psAppClass->dwBulkOnlyState = 0; /* reset the state */
		}
		#endif	//USBOTG_HOST_DATANG
			break;
		case REMOVE_CARD_CMD:
			mpDebugPrint("\nWIFI REMOVE_CARD_CMD\n");
			
			/* note: File System had always set drv->Flag.Present as 1 for non-storage or non-FAT devices (ex: WiFi, USB_PTP) for always passing card present check */
			//pDev->Flag.Present	= 0;

			UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
			// usb_rtusb_disconnect(); TODO
			mpDebugPrint("REMOVE_CARD_CMD END\n");
			break;
		default:
			break;
	}

}

/*
 * Returns true if a real Wi-Fi or Ethernet device.
 * Returns false if no device.
 */
int isWIFIDeviceType(){
	if(wifi_device_type == WIFI_USB_DEVICE_NONE)
		return 0;
	return 1;
}

/*
 * Returns true if a real Wi-Fi device.
 * Returns false if no device or Ethernet device.
 */
int isWIFIDeviceType2()
{
	if(wifi_device_type == WIFI_USB_DEVICE_NONE || wifi_device_type == ETHERNET_USB_DEVICE_DM9621)
		return 0;
	return 1;
}

int UsbOtgWifiPlugin(void){
	return wifi_device_plugin;
}

void UsbOtgWifiSetInitFlag(BYTE flag){
	 wifi_device_init = flag;
}

int UsbOtgWifiGetInitFlag(void){
	return wifi_device_init;
}

void UsbOtgWifiPlugout(void){
	 wifi_device_plugin = FALSE;
	 UsbOtgWifiSetInitFlag(FALSE);
}
void urb_disable(void)
{
    WHICH_OTG eWhichOtg;
    eWhichOtg = WIFI_USB_PORT;

	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	psUsbhDevDesc->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_IN_STATE;
}
void urb_enable(void)
{
    WHICH_OTG eWhichOtg;
    eWhichOtg = WIFI_USB_PORT;

	PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

	psUsbhDevDesc->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_IN_STATE;
}

#if DM9621_ETHERNET_ENABLE
void EthernetStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
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

    ethernet_usbotg = eWhichOtg;                /* save the USB port number */

    pUsbhDevDes     = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);//gpUsbhDeviceDescriptor;
	mpDebugPrint("EthernetStateMachine==>");
    memcpy((BYTE*)&receiveMailDrv, (BYTE*)pUsbh->sMDevice[bMcardTransferID].sMcardRMail, sizeof(ST_MCARD_MAIL));
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
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

    MP_DEBUG("WSSM-stat %d" , receiveMailDrv.wCurrentExecutionState);
	
    switch( receiveMailDrv.wCurrentExecutionState)
    {
        case ETHERNET_INIT_START_STATE:
			flib_Host20_Interrupt_Init(eWhichOtg);  // For enable Interrupt EP 
			pUsbhDevDes->psAppClass->sBulkInDescriptor.wMaxPacketSize = 0x940;
			pUsbhDevDes->psAppClass->dDataInLen = 0x40;
			pUsbhDevDes->bDeviceAddress = 1;
			pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress = 1;
			pUsbhDevDes->psAppClass->sBulkInDescriptor.bmAttributes = OTGH_ED_BULK;
            pUsbhDevDes->psAppClass->dwBulkOnlyState = 0; /* reset the state */
			UR_OTGH_PT_Bulk_Init(eWhichOtg);
			pSendMailDrv->wCurrentExecutionState	= ETHERNET_INIT_STATE;
			pSendMailDrv->wStateMachine 			= USB_ETHERNET_SM;
	        break;
		
        case ETHERNET_INIT_STATE:

            if (UsbOtgHostConnectStatusGet(eWhichOtg))
            { // plug in
                wifi_device_plugin = TRUE;
                mpDebugPrint("USB ETHERNET DEIVCE Plug-in\r\n");
                    if (NetDeviceAttachEvent_Fn)
                        NetDeviceAttachEvent_Fn(
                                le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idVendor),
                                le16_to_cpu(pUsbhDevDes->sDeviceDescriptor.idProduct),
                                TRUE);
				mpDebugPrint("Will call SystemDeviceInit");
				SystemDeviceInit(USB_ETHERNET_DEVICE);
            }
            else
            { // plug out
                mpDebugPrint("WIFI init:Plug-out\r\n");
				wifi_device_plugin = FALSE;
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

	mpDebugPrint("OPOOOOOOOOO");
    pUsbh->sMDevice[bMcardTransferID].swStatus = err;
    
    if (err == USB_NO_ERROR)
    {
        if ( receiveMailDrv.wCurrentExecutionState == ETHERNET_INIT_START_STATE )
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
    	MP_DEBUG1("SetupSM err = %d", err);
    }
}
#endif //#if DM9621_ETHERNET_ENABLE
#endif //#if NETWARE_ENABLE
#endif //#if (SC_USBHOST==ENABLE)

