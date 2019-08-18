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
* Filename		: netusb.c
* Created		: billwang
* Created Date	: 2010/05/14 
* Description 	: Net USB; This implementation supports mulit-qTDs.
******************************************************************************** 
*/

/*
 * NETUSB API:
 *
 * void NetUsb_Hotplug(BOOL plug);
 * int NetUsb_Init(void);
 *
 */
/*
 * define this module show debug message or not,  0 : disable, 1 : enable
 */
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include <linux/usb.h>

#include "global612.h"
#include "mpTrace.h"

#include "usbotg_ctrl.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_cdc.h"
#include "sio.h"
#include "netusb.h"
#include "atcmd_usb.h"

#include "taskid.h"
#include "os.h"

#define MAX_DATA_PAGE           5

#if HAVE_USB_MODEM > 0

#define NETUSB_PLUG_IN			1
#define NETUSB_PLUG_OUT			2
#define RegisterState_Test		3
#define PPP_CONNECT			    4
#define PPP_DISCONNECT			5
#define PPP_CONNECTED			6
#define PPP_DISCONNECTED		7
#define SIGNAL_READ				8

#define WWAN_ERR_ATD_FAILED		-101

#define USB_MAX_RX_SIZE			512
#define NETUSB_RX_URBS_COUNT 	16
#define NETUSB_INT_URBS_COUNT 	1

static BYTE u08NetUsbMessageId;
static bool is_plugin;
static BYTE u08NetUsbTaskId;
static struct usb_device *pnetusb_devices[MAX_USB_DEVICES];
extern struct usb_device   dev_global[MAX_USB_DEVICES];

#define buffer_size 32768
static char atcmd_buf[buffer_size];
char *writePtr, *readPtr;
extern BYTE * const startPtr, * const endPtr;

char operator[10+1];
int AcT;

extern struct qd_usb h21_usb;
extern struct usb_host_endpoint ep_in_buf[];
extern struct usb_host_endpoint ep_out_buf[];

static void flib_Host20_Send_qTD_Active2(  qTD_Structure *spHeadqTD, struct ehci_qhd *spTempqHD, WHICH_OTG eWhichOtg);
static qTD_Structure *flib_Host20_Issue_Bulk_Active2(   BYTE    bArrayListNum, WORD    hwSize, DWORD   *pwBufferArray, DWORD   wOffset, BYTE    bDirection, WHICH_OTG eWhichOtg);
static void netusb_task(void);
static int netusb_enable_rx(struct usb_device *udev);
static int netusb_enable_int(struct usb_device *udev);
static int netusb_enable(struct usb_device *udev, WHICH_OTG eWhichOtg);
int threeg_init(void);
struct usb_device *usb_data_setup2(WHICH_OTG eWhichOtg);

int pppOpen(sio_fd_t fd, void (*linkStatusCB)(void *ctx, int errCode, void *arg), void *linkStatusCtx);
void send_ppp_event(void *ctx, int errCode, void *arg);
int H21_Init(struct usb_device *udev, WHICH_OTG eWhichOtg);
void netusb_cleanup();
void xpg3G_ConnectCb(void *ctx, int error);
bool SignalTest(bool show);
void Xpg3G_SignalCb(void *ctx, WORD rssi, WORD ber);
bool isDataMode();

extern void *usb_buffer_alloc( struct usb_device *dev, size_t size, gfp_t mem_flags, dma_addr_t *dma);
extern PLM_LIST_ENTRY ListPopHead(PLM_LIST_CONTAINER pList);

int NetUsb_Init(WHICH_OTG eWhichOtg)
{
    int sdwRetVal;
	struct usb_device *udev;
	static int netusb_initialized;

	udev = usb_data_setup2(eWhichOtg);
	MP_ASSERT(udev);

	if (netusb_initialized == 0)
	{
		init_netpool();
	}
	
	urb_queue_init(udev);
    urb_cleanup_init2(udev);
	
	if(!u08NetUsbMessageId) {
		sdwRetVal = mpx_MessageCreate(OS_ATTR_FIFO, 640);
		if(sdwRetVal < 0){
			DPrintf("NetCtrlInit: mailbox create fail");
			BREAK_POINT();
		}
		else
			u08NetUsbMessageId = (U08)sdwRetVal;
	}

	if(!u08NetUsbTaskId){
	    sdwRetVal = mpx_TaskCreate(netusb_task, CONTROL_PRIORITY, 0x1000 * 3);    
	    if (sdwRetVal < 0)
	    {
	        MP_ALERT("-E- netusb_task created failed");
	        return sdwRetVal;
	    }
		
		u08NetUsbTaskId = (BYTE)sdwRetVal;
		
	    sdwRetVal = mpx_TaskStartup(u08NetUsbTaskId);
	    if (sdwRetVal < 0)
	    {
	        MP_ALERT("-E- netusb_task startup failed");
	        return sdwRetVal;
	    }
	}

	netusb_initialized++;

	return 0;
error:
	return -1;
}

static void netusb_task(void)
{
	DWORD dwEvent;
    S32 status;
	int r;
	WHICH_OTG eWhichOtg;
	uint32_t u32Message[8];
	struct usb_device *udev;

	while (1)
	{
        status = mpx_MessageReceive(u08NetUsbMessageId, (U08*)u32Message);
        if(status > 0)
		{
            switch(u32Message[0])
			{
                case NETUSB_PLUG_IN:
                    eWhichOtg = u32Message[1];

                   	h21_usb.new_plug_type=1;
			h21_usb.old_plug_type = h21_usb.new_plug_type;
			
                    if (eWhichOtg >= MAX_USB_DEVICES)
                    {
						MP_ALERT("[NETUSB] Plugin: invalid usb device index(%d)", eWhichOtg);
                        break;
                    }

					if(!is_plugin)
					{
						MP_DEBUG("\r\n$$$ NETUSB_PLUG_IN\r\n");

						is_plugin = true;

						usbModemPlugin();

						udev = &dev_global[eWhichOtg];

						netusb_enable(udev, eWhichOtg);

						init_ppp();

						if ((le16_to_cpu(udev->descriptor.idVendor) == HSUPA_USB_DEVICE_VID &&
							le16_to_cpu(udev->descriptor.idProduct) == HSUPA_USB_CDC_PID) ||
							(le16_to_cpu(udev->descriptor.idVendor) == TDSCDMA_USB_DEVICE_VID &&
							le16_to_cpu(udev->descriptor.idProduct) == TDSCDMA_USB_PID))
							H21_Init(udev, eWhichOtg);              /* modem setup */

                        if (h21_usb.to_connect)
                            if (H21_Connect() == 0)                     /* dial a data call */
                            {
                                if (h21_usb.to_connect)
                                    pppOpen(h21_usb.ppp_unit, send_ppp_event, &h21_usb);
                            }
					}
					break;
                case NETUSB_PLUG_OUT:
					MP_DEBUG("\r\n$$$ NETUSB_PLUG_OUT\r\n");
					if(is_plugin)
					{
						is_plugin = false;
						//release usb 

						//pppClose(0);		//marked by Joyce
						//netusb_cleanup();	//marked by Joyce

						//usbModemUnplug();	//marked by Joyce
					}

					break;
                case RegisterState_Test:
					r = threeg_GetRegisterState();
					netusb_complete_t fn = (netusb_complete_t)u32Message[1];
					if (fn)
						(*fn)((void *)u32Message[2], r);
					break;
                case SIGNAL_READ:
                    if (!isDataMode())
                        if (SignalTest(false))
                        {
                            WORD rssi, ber;
                            EdgeGetCurSignalValue(&rssi, &ber);
#if USBOTG_HOST_DATANG
				if(rssi != 199)		//rssi is 199, it means the signal is unknown.
					
#endif
                            Xpg3G_SignalCb(h21_usb.app_context, rssi, ber);
                        }
                    break;
                case PPP_CONNECT:
                    if (h21_usb.to_connect && !isPPPup())
                    {
                        if (H21_Connect() == 0)                     /* dial a data call */
                        {
                            if (h21_usb.to_connect)
                                pppOpen(h21_usb.ppp_unit, send_ppp_event, &h21_usb);
                        }
                        else
                            xpg3G_ConnectCb(h21_usb.app_context, WWAN_ERR_ATD_FAILED);
                    }

					break;
                case PPP_DISCONNECT:
                    if (!h21_usb.to_connect && isPPPup())
                    {
                        pppClose(u32Message[1]);
                    }

					break;
                case PPP_CONNECTED:
                    xpg3G_ConnectCb(h21_usb.app_context, 0);
					break;
                case PPP_DISCONNECTED:
					break;
			}

		}
	}

}

struct usb_device *usb_data_setup2(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    struct usb_device_descriptor *desc;
	struct usb_device *udev = &dev_global[eWhichOtg];

	MP_DEBUG("[NETUSB] --> %s(%d)\n", __func__, eWhichOtg);
    pUsbhDevDes = UsbOtgHostDevDescGet(eWhichOtg);
//    struct usb_interface   *intf = &intf_global;
    short i, k, n, ep_num;
    BYTE direction;
	udev->devnum = eWhichOtg;
    memcpy(&udev->descriptor, &pUsbhDevDes->sDeviceDescriptor, sizeof udev->descriptor);

    pUsbhDevDes->bIsNetUsb = TRUE;                 /* TODO cleanup */

//	SET_NETDEV_DEV(intf, udev->dev);
    desc = &udev->descriptor;
    udev->speed = pUsbhDevDes->bDeviceSpeed + 1;
    mpDebugPrint("Device Speed: %d", udev->speed);

    mpDebugPrint("Device Desc: =============");
    mpDebugPrint("bLength = %d",desc->bLength);
    mpDebugPrint("bDescriptorType = %d",desc->bDescriptorType);
    mpDebugPrint("bcdUSB = %x",le16_to_cpu(desc->bcdUSB));
    mpDebugPrint("bDeviceClass = %x",desc->bDeviceClass);
    mpDebugPrint("bDeviceSubClass = %x",desc->bDeviceSubClass);
    mpDebugPrint("bDeviceProtocol = %x",desc->bDeviceProtocol);
    mpDebugPrint("bMaxPacketSize0 = %x",desc->bMaxPacketSize0);
    mpDebugPrint("idVendor = %x",le16_to_cpu(desc->idVendor));
    mpDebugPrint("idProduct = %x",le16_to_cpu(desc->idProduct));
    mpDebugPrint("bcdDevice = %x",desc->bcdDevice);
    mpDebugPrint("iManufacturer = %x",desc->iManufacturer);
    mpDebugPrint("iProduct = %x",desc->iProduct);
    mpDebugPrint("iSerialNumber = %x",desc->iSerialNumber);
    mpDebugPrint("bNumConfigurations = %x",desc->bNumConfigurations);

    for ( k = 0; k < pUsbhDevDes->pConfigDescriptor->bNumInterfaces; k++ ) {
        MP_DEBUG("[NETUSB] interface: %d", k);
        n = pUsbhDevDes->pConfigDescriptor->pInterface[k].altsetting->bNumEndpoints;
        for ( i = 0; i < n; i++ ) {
            PUSB_ENDPOINT_DESCRIPTOR ep_desc;

            ep_desc = &pUsbhDevDes->pConfigDescriptor->pInterface[k].altsetting->pEndpoint[i];
            ep_num = (ep_desc->bEndpointAddress)&USB_ENDPOINT_NUMBER_MASK;
            direction = (ep_desc->bEndpointAddress)&USB_ENDPOINT_DIR_MASK;

            if (direction == USB_DIR_OUT)
            {
                MP_DEBUG("[NETUSB] endpoint(out): %d", ep_num);
                memcpy(&ep_out_buf[ep_num].desc, ep_desc, sizeof *ep_desc);
                udev->ep_out[ep_num] = &ep_out_buf[ep_num];
            }
            else
            {
                MP_DEBUG("[NETUSB] endpoint(in): %d", ep_num);
                memcpy(&ep_in_buf[ep_num].desc, ep_desc, sizeof *ep_desc);
                udev->ep_in[ep_num] = &ep_in_buf[ep_num];
            }
        }
    }

	mpDebugPrint("<-- %s\n", __func__);

	return pnetusb_devices[eWhichOtg] = udev;
}

/*
 * USB hotplug callback
 */
int NetUsb_Hotplug(WHICH_OTG eWhichOtg, BOOL plug)
{
	uint32_t message[2];
    int r;
    if(plug)
	{
		message[0] = NETUSB_PLUG_IN;
	}
    else
	{
		h21_usb.new_plug_type=0;
		message[0] = NETUSB_PLUG_OUT;
	}
	message[1] = eWhichOtg;
	r = mpx_MessageSend(u08NetUsbMessageId, (BYTE *)message, sizeof(message));
    return (r == OS_STATUS_OK) ? 0 : -1;
}

/*
 * USB PPP callback
 */
int NetUsb_PppStatusChange(void *ctx, bool ppp_connected)
{
    struct qd_usb *usb = ctx;
	uint32_t message[2];
    int r;
    if(ppp_connected)
		message[0] = PPP_CONNECTED;
    else
		message[0] = PPP_DISCONNECTED;
	message[1] = usb->idx;
	r = mpx_MessageSend(u08NetUsbMessageId, (BYTE *)message, sizeof(message));
    return (r == OS_STATUS_OK) ? 0 : -1;
}

/*
 * Read signal strength 
 */
int NetUsb_SignalRead()
{
	uint32_t message[2];
    int r;
    message[0] = SIGNAL_READ;
	r = mpx_MessageSend(u08NetUsbMessageId, (BYTE *)message, sizeof(message));
    return (r == OS_STATUS_OK) ? 0 : -1;
}

/*
 * USB connect
 */
int NetUsb_Connect(void *ctx)
{
	uint32_t message[2];
    int r;
    message[0] = PPP_CONNECT;

	h21_usb.to_connect = true;
	h21_usb.app_context = ctx;

	r = mpx_MessageSend(u08NetUsbMessageId, (BYTE *)message, sizeof(message));
    return (r == OS_STATUS_OK) ? 0 : -1;
}

/*
 * USB disconnect
 */
int NetUsb_Disconnect(void *ctx)
{
	uint32_t message[2];
    int r;
    message[0] = PPP_DISCONNECT;
	message[1] = h21_usb.ppp_unit;

	h21_usb.to_connect = false;
	r = mpx_MessageSend(u08NetUsbMessageId, (BYTE *)message, sizeof(message));
    return (r == OS_STATUS_OK) ? 0 : -1;
}

struct atcmd_request_t {
	bool get_register;
};

static struct atcmd_request_t atcmd_req;

static void GetRegisterStateCb(void *ctx, int status)
{
	struct atcmd_request_t *req = ctx;

	if (!req)
		return;

	req->get_register = false;
}

/*
 * async call
 */
int GetRegisterState(void)
{
	struct atcmd_request_t * const req = &atcmd_req;
	uint32_t message[3];
    int r = -1;
	mpDebugPrint("Edge Get Register State ...");
	if (!req->get_register)
	{
		req->get_register = true;
		message[0] = RegisterState_Test;
		message[1] = (uint32_t)GetRegisterStateCb;
		message[2] = NULL;
		r = mpx_MessageSend(u08NetUsbMessageId, (BYTE *)message, sizeof(message));
	}

    return (r == OS_STATUS_OK) ? 0 : -1;
}

static int netusb_enable(struct usb_device *udev, WHICH_OTG eWhichOtg)
{
    extern void *net_buf_start;
    extern void *net_buf_end;

	init_Pointer();
    threeg_init();
    UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)net_buf_start,(DWORD)net_buf_end, eWhichOtg);
    netusb_enable_rx(udev);
//Datang 3G modem doesn't need interrupt(means netusb_enable_int(udev);)
#if USBOTG_HOST_DATANG
	if(!((udev->descriptor.idVendor==cpu_to_le16(TDSCDMA_USB_DEVICE_VID))&&
		(udev->descriptor.idProduct==cpu_to_le16(TDSCDMA_USB_PID))))
#endif
    netusb_enable_int(udev);
    EnableNetWareTask();                        /* enable network tick */
    return 0;
}

static void rx_urb_complete(struct urb *urb)
{
	struct qd_usb *usb;
	const u8 *buffer;
	unsigned int length,len;
    uint32_t wp, rp;
    int idx;

	buffer = urb->transfer_buffer;
	length = urb->actual_length;
	usb = urb->context;

#if 0
	NetDumpData(0, urb->transfer_buffer, urb->actual_length);
#endif

    idx = 0;
    while (length > 0)
    {
        wp = (uint32_t)writePtr;
		rp = (uint32_t)readPtr;

        if (wp < rp)
        {
            len = MIN(length, rp-wp-1);
        }
        else
        {
            len = MIN(length, (uint32_t)endPtr-wp);
            if (rp == 0)
            {
                if ((uint32_t)endPtr-wp == len)
                    len--;
            }
        }
        if (len == 0)
            break;                              /* buffer full */
		MP_ASSERT(wp);
#if DEMO_PID       
       mmcp_memcpy((BYTE *)wp, (BYTE *)&buffer[idx], len):
#else
        MEMCOPY((BYTE *)wp, (BYTE *)&buffer[idx], len);
#endif
        length -= len;
        idx += len;
        wp += len;
        if (wp >= (uint32_t)endPtr)
			wp = (uint32_t)startPtr;
        writePtr = (char *)wp;
    }

	SetUartRx();

error:
	usb_submit_urb(urb, GFP_ATOMIC);
}

static struct urb *alloc_rx_urb(struct usb_device *udev)
{
	struct qd_usb *usb = &h21_usb;
//	struct usb_device *udev = zd_usb_to_usbdev(usb);
	struct urb *urb;
	void *buffer;

    MP_ASSERT(udev);
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return NULL;
	buffer = usb_buffer_alloc(udev, USB_MAX_RX_SIZE, GFP_KERNEL,
		                  &urb->transfer_dma);
	if (!buffer) {
		usb_free_urb(urb);
		return NULL;
	}
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
	//Qisda 3.5G USB modem
	if (le16_to_cpu(udev->descriptor.idVendor) == HSUPA_USB_DEVICE_VID &&
		le16_to_cpu(udev->descriptor.idProduct) == HSUPA_USB_CDC_PID)
	{
#endif
	usb_fill_bulk_urb(urb, udev, usb_rcvbulkpipe(udev, EP_DATA_IN),
		          buffer, USB_MAX_RX_SIZE,
			  rx_urb_complete, usb);
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
	}
	//Datang 3G USB modem
	else if(le16_to_cpu(udev->descriptor.idVendor) == TDSCDMA_USB_DEVICE_VID &&
			le16_to_cpu(udev->descriptor.idProduct) == TDSCDMA_USB_PID)
	{
		usb_fill_bulk_urb(urb, udev, usb_rcvbulkpipe(udev, EP_DATA_IN_DATANG),
				buffer, USB_MAX_RX_SIZE,
				rx_urb_complete, usb);
	}
#endif
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	return urb;
}

static void free_rx_urb(struct urb *urb)
{
	if (!urb)
		return;
	usb_buffer_free(urb->dev, urb->transfer_buffer_length,
		        urb->transfer_buffer, urb->transfer_dma);
	usb_free_urb(urb);
}

static int netusb_enable_rx(struct usb_device *udev)
{
	int i, r;
	struct urb **urbs;

	urbs = mm_malloc(NETUSB_RX_URBS_COUNT * sizeof(struct urb *));
	if (!urbs)
		goto error;
	for (i = 0; i < NETUSB_RX_URBS_COUNT; i++) {
		urbs[i] = alloc_rx_urb(udev);
		if (!urbs[i])
			goto error;
	}

	for (i = 0; i < NETUSB_RX_URBS_COUNT; i++) {
		r = usb_submit_urb(urbs[i], GFP_KERNEL);
		if (r)
        {
            __asm("break 100");
			goto error_submit;
        }
	}

	return 0;

error_submit:
	for (i = 0; i < NETUSB_RX_URBS_COUNT; i++) {
		usb_kill_urb(urbs[i]);
	}
error:
	__asm("break 100");
	if (urbs) {
		for (i = 0; i < NETUSB_RX_URBS_COUNT; i++)
			free_rx_urb(urbs[i]);
	}
	return r;
}

static void int_urb_complete(struct urb *urb)
{
	int r;

//    MP_FUNCTION_ENTER();
    MP_ASSERT(!urb->status);
	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
	case -EINVAL:
	case -ENODEV:
	case -ENOENT:
	case -ECONNRESET:
	case -EPIPE:
		goto kfree;
	default:
		goto resubmit;
	}

#if 0
	NetDumpData(0, urb->transfer_buffer, urb->actual_length);
#endif

resubmit:
	r = usb_submit_urb(urb, GFP_ATOMIC);
	if (r) {
		goto kfree;
	}
//    MP_FUNCTION_EXIT();
	return;
kfree:
    MP_ASSERT(0);
    MP_FUNCTION_EXIT();
#ifdef LINUX
	kfree(urb->transfer_buffer);
#else
	usb_free_buffer(urb->transfer_buffer);
#endif
}

static int netusb_enable_int(struct usb_device *udev)
{
	struct urb **urbs;
	char *buf;                                  /* usb transfer buffer */
	short i;
	int r;

	urbs = mpx_Zalloc(sizeof(struct urb*) * NETUSB_INT_URBS_COUNT);
	for(i = 0; i < NETUSB_INT_URBS_COUNT; i++){
		urbs[i] = usb_alloc_urb(0, GFP_KERNEL);

		buf = usb_alloc_buffer(64);
		if (!buf)
			goto error;

		usb_fill_bulk_urb(urbs[i], udev, usb_rcvintpipe(udev, EP_COMM_IN),
						  	buf, 64, int_urb_complete, buf);

		r = usb_submit_urb(urbs[i], GFP_KERNEL);
	}
	return 0;

error:
	if (urbs) {
		for (i = 0; i < NETUSB_INT_URBS_COUNT; i++)
			free_rx_urb(urbs[i]);
	}
	return -1;
}

struct usb_device * netusb_usbdevice(WHICH_OTG eWhichOtg)
{
	return pnetusb_devices[eWhichOtg];
}

void UsbOtgBulkProcess2(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes
	,BYTE	bDirection, PLM_LIST_CONTAINER pUrbList, WHICH_OTG eWhichOtg)
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
	struct urb *urb_out;
	struct urb *urb;
	struct urb *urb_in;
    struct ehci_qtd *qtd;
    short ep, cnt = 0;
    struct usb_device *udev =  netusb_usbdevice(eWhichOtg);

	//mpDebugPrint("UsbOtgBulkProcess %x",pUsbhDevDes->psAppClass->bBulkInQHDArrayNum);
    for (i = 0; i < MAX_DATA_PAGE; i++)
        pbDataPage[i] = 0;
	
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
			    mpDebugPrint("multi %d", i++);
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

            hwSize              = urb_out->transfer_buffer_length;
            pbDataPage[0]       = ((DWORD)(urb_out->transfer_buffer));
//            MP_ALERT("UsbOtgBulkProcess: OUT s=%p", pbDataPage[0]); 
//            MP_ALERT("UsbOtgBulkProcess: OUT e=%p", pbDataPage[0]+hwSize); 
            if ((((DWORD)urb_out->transfer_buffer & 0xfffUL) + hwSize) > 0x1000)
            {
                /* ----------  over a 4KB page boundary  ---------- */
                pbDataPage[1]       = ((DWORD)urb_out->transfer_buffer & ~0xfffUL) + 0x1000;
//                MP_ALERT("UsbOtgBulkProcess: over 4KB=%p", pbDataPage[1]); 
            }
            else
                pbDataPage[1]       = 0;
#if 0
            if (((unsigned long)(pbDataPage[0]) & 3))
                MP_ALERT("UsbOtgBulkProcess: unaligned=%p", pbDataPage[0]); 
            MP_VALIDATE_POINTER(pbDataPage[0]);
#endif
            wOffset             = 0;
            urb                 = urb_out;
//#if RX_URBS_COUNT == 1
            break;                              /* disable multi for now TODO */
//#endif
        }
    }

    MP_ASSERT(pbDataPage[0]);
	
	if(wifi_device_type == WIFI_USB_DEVICE_AR2524)
	{
	    if (cnt == 0)               /* no URBs */
	        return;
	}

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
                                                 eWhichOtg);
        pUsbhDevDes->urb = NULL;
    }
    else
    { // for single-TD 
	        pUsbhDevDes->urb = urb;
	//        MP_ALERT("UsbOtgBulkProcess: idx=%d", qHD_array_number); 
	//        NetPacketDump(pbDataPage[0], hwSize);
	        qtd = (struct ehci_qtd *)flib_Host20_Issue_Bulk_Active2( qHD_array_number,
	                                        hwSize, 
	                                        &(pbDataPage[0]), 
	                                        wOffset, 
	                                        bDirection,
	                                        eWhichOtg);
	        pUsbhDevDes->urb = NULL;

    }
}

static qTD_Structure *flib_Host20_Issue_Bulk_Active2(   BYTE    bArrayListNum,
                                                   WORD    hwSize,
                                                   DWORD   *pwBufferArray,
                                                   DWORD   wOffset,
                                                   BYTE    bDirection,
                                                   WHICH_OTG eWhichOtg)
{
    qHD_Structure *spBulkqHD;
    qTD_Structure *spTempqTD;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    //<1>.Fill TD
    spTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg); //The qTD will be release in the function "Send"
    spTempqTD->bTotalBytes=hwSize;           
    spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); 
    spTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
   ((struct ehci_qtd *)spTempqTD)->urb = pUsbhDevDes->urb;

    //<2>.Analysis the Direction 
    if (bDirection)
    {
        spTempqTD->bPID = HOST20_qTD_PID_IN;
        spBulkqHD       = pUsbhDevDes->hstBulkInqHD[bArrayListNum];
    }
    else
    {
        spTempqTD->bPID = HOST20_qTD_PID_OUT;
        spBulkqHD       = pUsbhDevDes->hstBulkOutqHD[bArrayListNum];
    }

    //<3>.Send TD
    flib_Host20_Send_qTD_Active2(spTempqTD, (struct ehci_qhd *)spBulkqHD, eWhichOtg);
    return spTempqTD;
}

static void flib_Host20_Send_qTD_Active2(  qTD_Structure *spHeadqTD,
                                    struct ehci_qhd *spTempqHD,
                                    WHICH_OTG eWhichOtg)
{
	qTD_Structure           *spNewDumyqTD; 
	qTD_Structure  *spOldDumyqTD; 
	qTD_Structure  *spLastqTD;
    DWORD dwSwapStartBuffer = 0;
    DWORD dwSwapStopBuffer = 0;
    //DWORD dwSwapTotalByteLength = 0;
    DWORD i = 0;
    DWORD j = 0;
    DWORD cnt_1;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    MP_ASSERT(sizeof(qHD_Structure) <= 64);

    MP_DEBUG("spHeadqTD = 0x%x", spHeadqTD);
    MP_DEBUG("spOldDumyqTD = 0x%x", spTempqHD->dummy);
	//<1>.Copy Head-qTD to OldDumyqTD
	spOldDumyqTD=spTempqHD->dummy;
    MP_ASSERT(spOldDumyqTD);
	memcpy((BYTE*)spOldDumyqTD,(BYTE*)spHeadqTD,Host20_qTD_SIZE);

    spTempqHD->dummy = spHeadqTD;

    if (spTempqHD->head == NULL)
        spTempqHD->head = spOldDumyqTD;

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

	pUsbhDev->bSendStatusError=0;
    
//__asm("break 100");

_TD_AVTIVE_:
	spOldDumyqTD->bStatus_Active=1;
}

BYTE flib_Host20_Send_qTD_Done2(qHD_Structure *spTempqHD, WHICH_OTG eWhichOtg)
{
	qTD_Structure           *spNewDumyqTD; 
	qTD_Structure           *spReleaseqTD;    
	qTD_Structure           *spReleaseqTDNext;    
    struct ehci_qhd *qhd = (struct ehci_qhd *)spTempqHD;
    BYTE i = 0;
    BYTE j = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

// force update cache
    SetDataCacheInvalid();

    spReleaseqTD = qhd->head;

_TD_DONE_:
	//<6>.Checking the Result
	//if (gwLastqTDSendOK==0)
    pUsbhDev->bQHStatus = spTempqHD->bOverlay_Status;
//	flib_Host20_CheckingForResult_QHD(spTempqHD, eWhichOtg); 	TODO
    if (pUsbhDev->bQHStatus & HOST20_qTD_STATUS_Halted)
    {
        if ((pUsbhDev->bQHStatus & 0x7F) != HOST20_qTD_STATUS_Halted)
        {
            //__asm("break 100");
            MP_DEBUG("bQHStatus = 0x%x", pUsbhDev->bQHStatus);
            pUsbhDev->bQHStatus = 0;
        }
        else
        {
            pUsbhDev->sSetupPB.dwSetupState = SETUP_DONE_STATE;
        }
    }
    else
    {
        pUsbhDev->bQHStatus = 0;
    }
	//<5>.Release the all the qTD (Not include spNewDumyqTD)
    spNewDumyqTD = qhd->dummy;
	do {
        if (spReleaseqTD->bStatus_Active)
        {
            break;
        }
        struct ehci_qtd *qtd;
        qtd = (struct ehci_qtd *)spReleaseqTD;
        if (qtd->urb)
        {
            usb_urb_complete(qtd->urb, spReleaseqTD);
        }
		spReleaseqTDNext = (qTD_Structure*)(((DWORD)(spReleaseqTD->bNextQTDPointer))<<5);
		flib_Host20_ReleaseStructure(Host20_MEM_TYPE_qTD,(DWORD)spReleaseqTD, eWhichOtg);
        if (spReleaseqTD == 0)
        {
            MP_DEBUG("flib_Host20_Send_qTD_Done:no TD released!!");
            MP_DEBUG("spTempqHD = 0x%x,td=%x", spTempqHD, spReleaseqTD);
            break;
        }
		spReleaseqTD = spReleaseqTDNext;
	} while(((DWORD)spReleaseqTD)!=((DWORD)spNewDumyqTD));

    if (((DWORD)spReleaseqTD) == ((DWORD)spNewDumyqTD))
    {
        if (((DWORD)qhd->dummy) == ((DWORD)spNewDumyqTD))
        {
            qhd->head = NULL;
            return 0;
        }
    }

    qhd->head = spReleaseqTD;
    return 0;
}
    
static void UsbOtgHostEventIocBulk (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDev->fpAppClassBulkIoc != OTGH_NULL)
        pUsbhDev->fpAppClassBulkIoc(eWhichOtg);
    else
        MP_DEBUG("pUsbhDev->fpAppClassBulkIoc is NULL!!");
}


void NetUsb_EventIoc(WHICH_OTG eWhichOtg, int max_endpoints)
{
    short j;
    struct ehci_qhd *qhd;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    for (j = 0; j< pUsbhDev->bMaxBulkOutEpNumber; j++)
    {
        qhd = (struct ehci_qhd *)pUsbhDev->hstBulkOutqHD[j];
        if (qhd->head &&
                qhd->head->bStatus_Active == 0)
        {
            pUsbhDev->dwWhichBulkPipeDone |= (1<<j);
            UsbOtgHostEventIocBulk(eWhichOtg);
        }
    }

    for (j = 0; j< pUsbhDev->bMaxBulkInEpNumber; j++)
    {
        qhd = (struct ehci_qhd *)pUsbhDev->hstBulkInqHD[j];
        if (qhd->head &&
                qhd->head->bStatus_Active == 0)
        {
            pUsbhDev->dwWhichBulkPipeDone |= (1<<(j+16));
            UsbOtgHostEventIocBulk(eWhichOtg);
        }
    }
}

void netusb_cleanup()
{
	int i;


#ifdef LINUX
	//
	for(i = 0; i < RX_URB; i++){
		net_buf_mem_free(rx_urbs[i]->transfer_buffer);
		usb_free_urb(rx_urbs[i]);
	}
	mpx_Free(rx_urbs);

	for(i = 0; i < TX_URB; i++){
		net_buf_mem_free(tx_urbs[i]->transfer_buffer);
		usb_free_urb(tx_urbs[i]);
	}
	mpx_Free(tx_urbs);
	
#endif
	
}

char *NetUsb_NetworkName()
{
    return operator;
}

char *NetUsb_AccessTech()
{
	char AccessTech[3];
	if(AcT == 0)
		strcpy(AccessTech, "2G");
	else if(AcT == 2)
		strcpy(AccessTech, "3G");
	
	return AccessTech;
}

#endif
