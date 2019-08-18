/*
 * Qisda H21 HSUPA USB dongle
 */

/*
 * define this module show debug message or not,  0 : disable, 1 : enable
 */
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include <stdio.h>
#include <linux/usb.h>

#include "global612.h"
#include "mpTrace.h"
#include "netusb.h"
#include "atcmd_usb.h"

#include "usbotg_ctrl.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_cdc.h"

#include "taskid.h"
#include "os.h"

cops_format_ cops_format;

struct qd_usb h21_usb;
int dongle_type=0;
extern char operator[10+1];

extern LM_LIST_CONTAINER pUrbOutList[];
extern LM_LIST_CONTAINER pUrbInList[];

BYTE flib_Host20_Send_qTD_Done2(qHD_Structure *spTempqHD, WHICH_OTG eWhichOtg);
void UsbOtgBulkProcess2(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes ,BYTE	bDirection, PLM_LIST_CONTAINER pUrbList, WHICH_OTG eWhichOtg);
bool threeg_get_ccid(void);
bool threeg_get_imei(void);
char * getcops1(char *buf, int size);
bool at_cops_get(char *operator, int size);


int H21_Init(struct usb_device *udev, WHICH_OTG eWhichOtg)
{
	struct qd_usb *usb = &h21_usb;
	int RetVal;
	
	usb->udevice = udev;
	usb->idx = eWhichOtg;
    	usb->ppp_unit = 0;
//    usb->to_connect = true;
    	usb->to_connect = false;

	TaskSleep(10000);
#if USBOTG_HOST_CDC
	if((udev->descriptor.idVendor==cpu_to_le16(HSUPA_USB_DEVICE_VID))&&
		(udev->descriptor.idProduct==cpu_to_le16(HSUPA_USB_CDC_PID)))
	{
		dongle_type = 0;	//for H21_Connect and at_d functions use
	//	AtCmdStartUp();
		RetVal = Attention();
		if(RetVal < 0)
			return -1;
	//    getcmee();
		RetVal = setcmee(2);
		if(RetVal < 0)
			return -1;
	//    setwind(); not supported
	    	RetVal = threeg_get_imei();
		if(RetVal < 0)
			return -1;
		if (isPINready(true))
			MP_DEBUG("PIN READY");
	    	else
	    	{
	        	MP_DEBUG("set PIN");
	        	RetVal = setPINcode("0000");
	        	if(RetVal < 0)
	        		return -1;
	//        	setPINcode(user_config.pin_number);
	    	}
		RetVal = at_bqpvrf();
		if(RetVal < 0)
			return -1;
		cops_format = SHORT_AN_STRING;
		RetVal = at_cops(SHORT_AN_STRING);
		if(RetVal < 0)
			return -1;

		RetVal = at_cops_get(operator, sizeof operator);
		if(RetVal < 0)
			return -1;
		RetVal = SignalTest(false);
		if(RetVal < 0)
			return -1;
		RetVal = at_clck();
		if(RetVal < 0)
			return -1;
		RetVal = at_bqpvrf();
		if(RetVal < 0)
			return -1;
	}// USBOTG_HOST_CDC
#endif
#if USBOTG_HOST_DATANG
	//for Datang 3G USB modem
	if((udev->descriptor.idVendor==cpu_to_le16(TDSCDMA_USB_DEVICE_VID))&&
		(udev->descriptor.idProduct==cpu_to_le16(TDSCDMA_USB_PID)))
	{
		mpDebugPrint("[H21_Init]Start AT cmmand for Datang LC-6311");
		dongle_type = 1;	//for H21_Connect and at_d functions use
		RetVal = sendAtcmd("atz");	/* Reset to default configuration */
		if(RetVal < 0)
			return -1;
		RetVal = sendAtcmd("ate1");	/* Enable echo */
		if(RetVal < 0)
			return -1;
		RetVal = setcmee(2);			/* Report mobile termination error; 0:disable, 1:enable and use numeric values, 2: enable and use verbose */
		if(RetVal < 0)
			return -1;
		RetVal = threeg_get_imei();	/* Request product serial number identification (IMEI) identical */
		if(RetVal < 0)
			return -1;
		if (isPINready(true))
			MP_DEBUG("PIN READY");
	    	else
	    	{
	        	MP_DEBUG("set PIN");
	        	RetVal = setPINcode("0000");
	        	if(RetVal < 0)
	        		return -1;
	//        	setPINcode(user_config.pin_number);
	    	}
		RetVal = setreset();			/* set AT cmmand: at+cfun=1\r, that mean is to setting the full functionality mode, also triggers network registration */
		if(RetVal < 0)
			return -1;
		RetVal = sendAtcmd("at+cops=0");	/* choose or register the 0:GSM/2:UMTS network operator in automatically or manually */
		if(RetVal < 0)
			return -1;
		RetVal = sendAtcmd("at+copn");	/* Read Operator Name */
		if(RetVal < 0)
			return -1;
		RetVal = SignalTest(false);
		if(RetVal < 0)
			return RetVal;
		RetVal = at_clck();
		if(RetVal < 0)
			return RetVal;
	}
#endif
	setServiceName("internet");

    return 0;
}

int H21_Connect()
{
	return at_d(dongle_type) ? 0 : -1;
}

/*
 * Disconnect a call
 */
int H21_Disconnect()
{
//	return at_hangup() ? 0 : -1;
	return 0;
}

void H21_BulkActive(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

	MP_DEBUG("H21_BulkActive");
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	
//	if( !(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_OUT_STATE))
    {
		if(!ListEmpty(&pUrbOutList[eWhichOtg]))
		{
		
		 if((pUsbhDevDes->dwWhichBulkPipeDone&0xFFFF)==0)	// XXX
         {
			//UartOutText("1");
//            pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_OUT_STATE;	
			UsbOtgBulkProcess2(pUsbhDevDes,OTGH_DIR_OUT, &pUrbOutList[eWhichOtg], eWhichOtg);
		  }
	    }
		
			
	}
//	if(!(pUsbhDevDes->psAppClass->dwBulkOnlyState&WIFI_BULK_DATA_IN_STATE))
	{
	
        if(!ListEmpty(&pUrbInList[eWhichOtg]))
        {
	  	
            if(pUsbhDevDes->dwWhichBulkPipeDone==0)
			{
//				UartOutText("3");
//				pUsbhDevDes->psAppClass->dwBulkOnlyState|= WIFI_BULK_DATA_IN_STATE;	
                UsbOtgBulkProcess2(pUsbhDevDes,OTGH_DIR_IN, &pUrbInList[eWhichOtg], eWhichOtg);
		 	}
	  	}	
	}
	
	


}

void H21_BulkIoc(WHICH_OTG eWhichOtg)
{
    short  qHD_index;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    BYTE bEventId = UsbOtgHostDriverEventIdGet(eWhichOtg);

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	//mpDebugPrint("pUsbhDevDes->psAppClass->dwBulkOnlyState %x ",pUsbhDevDes->psAppClass->dwBulkOnlyState);
	MP_DEBUG("pUsbhDevDes->dwWhichBulkPipeDone %x ",pUsbhDevDes->dwWhichBulkPipeDone);

    /* ----------  Bulk out  ---------- */
    //if (pUsbhDevDes->dwWhichBulkPipeDone & 0x03)
	if (pUsbhDevDes->dwWhichBulkPipeDone & 0x3F)	//0x3f mask for Datang and Qisda both
    	{
        //UartOutText("4");
		//__asm("break 100");
#if USBOTG_HOST_CDC
	//for Qisda 3.5G USB modem
	if((pUsbhDevDes->sDeviceDescriptor.idVendor==cpu_to_le16(HSUPA_USB_DEVICE_VID))&&
		(pUsbhDevDes->sDeviceDescriptor.idProduct==cpu_to_le16(HSUPA_USB_CDC_PID)))
	{
		if(pUsbhDevDes->dwWhichBulkPipeDone & 0x01)
			qHD_index = 0;
		else if(pUsbhDevDes->dwWhichBulkPipeDone & 0x02)
			qHD_index = 1;
	}
#endif
#if USBOTG_HOST_DATANG
	//for Datang 3G USB modem
	if(pUsbhDevDes->sDeviceDescriptor.idVendor == cpu_to_le16(TDSCDMA_USB_DEVICE_VID) && 
          pUsbhDevDes->sDeviceDescriptor.idProduct == cpu_to_le16(TDSCDMA_USB_PID))
	{
		qHD_index=pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum;
	}
#endif
		flib_Host20_Send_qTD_Done2(pUsbhDevDes->hstBulkOutqHD[qHD_index], eWhichOtg);
        pUsbhDevDes->dwWhichBulkPipeDone &= ~(1<<qHD_index);
		
        if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
        {
            MP_ALERT("error=0x%x", pUsbhDevDes->bQHStatus);
//            pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_OUT_STATE;
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

            if (ListGetSize(&pUrbOutList[eWhichOtg]) > 0)
                EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);

        }
    }
        
    /* ----------  Bulk in  ---------- */
    //if(pUsbhDevDes->dwWhichBulkPipeDone & 0x30000)
	if(pUsbhDevDes->dwWhichBulkPipeDone & 0x3FFFFF)	//0x3fffff mask for Datang and Qisda both
    	{
        UartOutText("5");
#if USBOTG_HOST_CDC
	//for Qisda 3.5G USB modem
	if((pUsbhDevDes->sDeviceDescriptor.idVendor==cpu_to_le16(HSUPA_USB_DEVICE_VID))&&
		(pUsbhDevDes->sDeviceDescriptor.idProduct==cpu_to_le16(HSUPA_USB_CDC_PID)))
	{
		if(pUsbhDevDes->dwWhichBulkPipeDone & 0x10000)
			qHD_index = 0;
		else if(pUsbhDevDes->dwWhichBulkPipeDone & 0x20000)
			qHD_index = 1;
	}
#endif
#if USBOTG_HOST_DATANG
	//for Datang 3G USB modem
	if((pUsbhDevDes->sDeviceDescriptor.idVendor==cpu_to_le16(TDSCDMA_USB_DEVICE_VID))&&
		(pUsbhDevDes->sDeviceDescriptor.idProduct==cpu_to_le16(TDSCDMA_USB_PID)))
	{
		qHD_index=pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum;
	}
#endif
        flib_Host20_Send_qTD_Done2(pUsbhDevDes->hstBulkInqHD[qHD_index], eWhichOtg);
        pUsbhDevDes->dwWhichBulkPipeDone &= ~(0x10000<<qHD_index);

        if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
        {
//            pUsbhDevDes->psAppClass->dwBulkOnlyState |= WIFI_BULK_DATA_IN_STATE;
            if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
            {
		
                mpDebugPrint("HOST20_qTD_STATUS_Halted");
                SWORD err = 0;
                pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                        eWhichOtg);
                if (err != USB_NO_ERROR)
                { 
                    free1(pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);						
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
//            pUsbhDevDes->psAppClass->dwBulkOnlyState &= ~WIFI_BULK_DATA_IN_STATE;
			
            if (ListGetSize(&pUrbInList[eWhichOtg]) > 0)
                EventSet(bEventId, EVENT_EHCI_ACTIVE_BULK);
        }
			//net_buf_mem_free(pUsbhDevDes->psAppClass->dDataBuffer);
			//EventSet(USB_BULK_COMPLETE_EVENT, 0x2);	

    }

}

