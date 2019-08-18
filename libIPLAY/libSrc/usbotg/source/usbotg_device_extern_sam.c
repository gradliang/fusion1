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
* Filename      : usbotg_device_extern_sam.c
* Programmer(s) : Calvin Liao
* Created       : 
* Descriptions  : For Side Monitor
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
#include "taskid.h"
#include "Usbotg_device.h"
#include "usbotg_device_extern_sam.h"
#include "ui.h"
#include "display.h"
#include "udtp.h"   // Samsung


 #if (SC_USBDEVICE && USBOTG_DEVICE_EXTERN_SAMSUNG)
/*
// Constant declarations
*/
#define SAVE_JPEG_TO_SDCARD  0  // Save JPEG File From PC into SD Card

#define MJPEG_NUM_BUF	2
#define UsbMjpgBufferSize  0x50000  // as same as WEB_CAM_BUF_SIZE
#define USB_EXTERN_MAX_DATA   512

// USB COMMAND
#define USB_CMD_TYPE        0x00
#define USB_CMD_REQ          0x06
#define USB_CMD_VALUE      0xFE
#define USB_CMD_INDEX      0xFE
// UDTP REQUEST
#define UDTP_REQ_TYPE        0x40
#define UDTP_REQ_VALUE      0x00
#define UDTP_REQ_INDEX      0x00

static DWORD dwSizeOfFrame = 0;  /* Maintains size of a frame */
static BYTE gCtrlCmd = 0; /* Maintains type of USB data (Update/Jpeg/Brightness)*/
/*
// Structure declarations
*/
typedef struct _EXTERN_VIDEO_STREAM_
{
	DWORD dwStreamActive;//DWORD streamActive;
	DWORD dwBufferCurIndex;//DWORD bufferCurIndex;
	DWORD dwBufferActive[MJPEG_NUM_BUF];//DWORD bufferActive[MJPEG_NUM_BUF];
	DWORD dwBufferLength[MJPEG_NUM_BUF];//DWORD bufferLength[MJPEG_NUM_BUF];
	BYTE  *pbVideoMjpegBuffer[MJPEG_NUM_BUF];//BYTE  *videoMjpegBuffer[MJPEG_NUM_BUF];
} EXTERN_VIDEO_STREAM, *PEXTERN_VIDEO_STREAM;

/*
// Variable declarations
*/
EXTERN_VIDEO_STREAM gVideoMjpegStream;


/*
// External Variable declarations
*/


/*
// Macro declarations
*/

/*
// Static function prototype
*/
static void UsbOtgDevExternTask(void);
static void UsbExternInit(void);
static void UsbExternDeInit(void);



/*
// Definition of internal functions
*/

SDWORD UsbOtgDevExternTaskInit(void)
{
    SDWORD sdwRetVal = 0;

    sdwRetVal = MailboxCreate(USBOTG_DEVICE_EXTERN_MAIL_ID, OS_ATTR_PRIORITY);
    if (sdwRetVal != OS_STATUS_OK)
        MP_ALERT("--E-- %s MailBox ID %d Create Fail!! Ret:%d",  __FUNCTION__, USBOTG_DEVICE_EXTERN_MAIL_ID, sdwRetVal);

    sdwRetVal = TaskCreate(USBOTG_DEVICE_EXTERN_TASK, UsbOtgDevExternTask, CONTROL_PRIORITY, 0x4000);
    if (sdwRetVal != OS_STATUS_OK)
        MP_ALERT("--E-- %s Task ID %d Create Fail!! Ret:%d",  __FUNCTION__, USBOTG_DEVICE_EXTERN_TASK, sdwRetVal);
            
    sdwRetVal = TaskStartup(USBOTG_DEVICE_EXTERN_TASK);
    if (sdwRetVal != OS_STATUS_OK)
        MP_ALERT("--E-- %s Task ID %d Startup Fail!! Ret:%d",  __FUNCTION__, USBOTG_DEVICE_EXTERN_TASK, sdwRetVal);

    return sdwRetVal;
}

void SideMonitorIn(void)
{
    UsbExternInit();
    EventSet(UI_EVENT, EVENT_WEB_CAM_IN);

}

void SideMonitorOut(void)
{
    EventSet(UI_EVENT, EVENT_WEB_CAM_OUT);
    UsbExternDeInit();
}

int SideMonitorGetVideoStatus(void)
{
	DWORD dwCurIndex;

	dwCurIndex = (gVideoMjpegStream.dwBufferCurIndex == 0) ? (MJPEG_NUM_BUF - 1) : (gVideoMjpegStream.dwBufferCurIndex - 1);
	return gVideoMjpegStream.dwBufferActive[dwCurIndex] ? 0 : 1;	
}


BYTE eUsbExternDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    DWORD dwCurIndex; 
    BYTE *pbUsbBuff;

    pbUsbBuff = (ext_mem_malloc(USB_EXTERN_MAX_DATA) | 0xA0000000);
    dwCurIndex = gVideoMjpegStream.dwBufferCurIndex;

    memset(pbUsbBuff, 0, USB_EXTERN_MAX_DATA);
    
    bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT, pbUsbBuff, u16FIFOByteCount, eWhichOtg);    


    if((*(pbUsbBuff) == SYNC0) && (*(pbUsbBuff+1) == SYNC1))
    {
        // Check Protocol Cmd
        dwSizeOfFrame = GetLenghtOFFrame(pbUsbBuff) - FRAME_HEADER_LENTH; // including FRAME_HEADER_LENTH
        MP_DEBUG("SYNC0/SYNC1 File Len:%d", dwSizeOfFrame);

        gVideoMjpegStream.dwBufferLength[dwCurIndex]  = u16FIFOByteCount - FRAME_HEADER_LENTH;

        gCtrlCmd = *(pbUsbBuff + CONTROL_CODE_POSITION); 

        switch(gCtrlCmd)
        {
            case 	UDTP_PROTOCOL_JPEG:
                MP_DEBUG("UDTP_PROTOCOL_JPEG");
                //Copy the update firmware data from USB to gpDispBuff
                memcpy((BYTE *)(gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex]), 
                            (BYTE*)(pbUsbBuff + FRAME_HEADER_LENTH), 
                            (u16FIFOByteCount - FRAME_HEADER_LENTH));
                break;
                
            case 	UDTP_PROTOCOL_CONTROL_BRIGHTNESS:
                MP_DEBUG("UDTP_PROTOCOL_CONTROL_BRIGHTNESS");                
                break;        
                
            case 	UDTP_PROTOCOL_CONTROL_UPDATE:
                MP_DEBUG("UDTP_PROTOCOL_CONTROL_UPDATE");                
                break;     
                
            default:
                MP_ALERT("--E-- %s Not Protocol Cmd %d", __FUNCTION__, gCtrlCmd);
                break;
        }     

        ext_mem_free(pbUsbBuff);
        return STATE_EXTERN_OUT;        
    }
    

    if(gCtrlCmd == UDTP_PROTOCOL_JPEG)
    {
        memcpy((gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex] + gVideoMjpegStream.dwBufferLength[dwCurIndex] ), pbUsbBuff, u16FIFOByteCount);

        if((dwSizeOfFrame - gVideoMjpegStream.dwBufferLength[dwCurIndex] ) <= USB_EXTERN_MAX_DATA)
        {        
            MP_DEBUG("Len Form PC : %d  To Device %d", dwSizeOfFrame, gVideoMjpegStream.dwBufferLength[dwCurIndex]);

            gVideoMjpegStream.dwBufferLength[dwCurIndex] = dwSizeOfFrame;
        
            #if SAVE_JPEG_TO_SDCARD  // Save JPEG File into SD Card
            MP_ALERT("Save photo from PC into SD card!! Size %d", gVideoMjpegStream.dwBufferLength[dwCurIndex]);
            Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "Side", "JPG", &gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][0], gVideoMjpegStream.dwBufferLength[dwCurIndex]);
            #endif
            
            gVideoMjpegStream.dwBufferActive[dwCurIndex] = TRUE;
            MP_DEBUG("Current Video Stream Index %d Len %d",dwCurIndex , gVideoMjpegStream.dwBufferLength[dwCurIndex]);

            // Check Buffer Length less than UsbMjpgBufferSize
            if(gVideoMjpegStream.dwBufferLength[dwCurIndex] > UsbMjpgBufferSize)
                MP_ALERT("--E-- %s Mpjpeg is over buffer size!! Mjpeg 0x%x > Buffer 0x%x", __FUNCTION__, gVideoMjpegStream.dwBufferLength[dwCurIndex], UsbMjpgBufferSize);

            #if 0  // The End is not FF D9 
            // Check the EOF of MJPEG File.
            if( gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -2] != 0xFF ||
                gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -1] != 0xD9)
                MP_ALERT("--E-- %s is not MJPEG file!! Last Char 0x%x 0x%x", __FUNCTION__,
                                    gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -2],
                                    gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -1]);        
            #endif

            // Next Buffer
            gVideoMjpegStream.dwBufferCurIndex = (dwCurIndex+1 == MJPEG_NUM_BUF) ? 0 : dwCurIndex+1;
            gVideoMjpegStream.dwBufferActive[gVideoMjpegStream.dwBufferCurIndex] = FALSE;        
            gVideoMjpegStream.dwBufferLength[gVideoMjpegStream.dwBufferCurIndex]= 0;
            dwSizeOfFrame = 0;

            gCtrlCmd = CONTROL_CODE_DEFAULT_VALUE;
            
            ext_mem_free(pbUsbBuff);
            return STATE_EXTERN_OUT;            
        }

        gVideoMjpegStream.dwBufferLength[dwCurIndex] += u16FIFOByteCount;  

        ext_mem_free(pbUsbBuff);
        return STATE_EXTERN_OUT;
    }

    ext_mem_free(pbUsbBuff);
    return STATE_EXTERN_OUT;
}


/*
// Definition of local functions 
*/
static void SideMonitorVideoGetStreamBuffer( DWORD dwBuffer)
{
	DWORD dwCurIndex;

	dwCurIndex = (gVideoMjpegStream.dwBufferCurIndex == 0) ? (MJPEG_NUM_BUF - 1) : (gVideoMjpegStream.dwBufferCurIndex - 1);

	if (gVideoMjpegStream.dwStreamActive == FALSE)
            MP_ALERT("--E-- %s Mjpeg Stream does not support!!", __FUNCTION__);

	if (gVideoMjpegStream.dwBufferActive[dwCurIndex] && gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex] != NULL)
	{
		memcpy((BYTE *)dwBuffer, &gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][0], gVideoMjpegStream.dwBufferLength[dwCurIndex]);
		gVideoMjpegStream.dwBufferActive[dwCurIndex] = FALSE;
	}
	else
	{
            MP_ALERT("--E-- %s Mjpeg Stream is not Ready!!", __FUNCTION__);
	}
}


static void UsbOtgDevExternTask(void)
{
    volatile ST_MCARD_MAIL *psMcardRMail;
    BYTE bMcardMailId;

    while(1)
    {
        MailboxReceive(USBOTG_DEVICE_EXTERN_MAIL_ID, &bMcardMailId);
        
        if(MailGetBufferStart(bMcardMailId, (DWORD*)(&psMcardRMail)) == OS_STATUS_OK)
        {
            switch(psMcardRMail->wCmd)
            {
                case WEB_CAM_START_CMD:
                    MP_DEBUG("--%s WEB_CAM_START_CMD", __FUNCTION__);
                    break;
                    
                case WEB_CAM_STOP_CMD:
                    MP_DEBUG("--%s WEB_CAM_STOP_CMD", __FUNCTION__);
                    break;
                    
                case WEB_CAM_GET_VFRAM_CMD:
                    MP_DEBUG("--%s WEB_CAM_GET_VFRAM_CMD -BuffAddr %x-", __FUNCTION__, psMcardRMail->dwBuffer);
                    SideMonitorVideoGetStreamBuffer(psMcardRMail->dwBuffer);
                    break;
                      
                case WEB_CAM_GET_AFRAM_CMD:  // Not support
                    MP_DEBUG("--%s WEB_CAM_GET_AFRAM_CMD", __FUNCTION__);                    
                    break;
            }

            MailRelease(bMcardMailId);
        }
    }
}


static void UsbExternInit(void)
{
    BYTE bCnt;

    if(ext_mem_get_free_space() < UsbMjpgBufferSize*2)
    {
        MP_ALERT("--E-- %s UsbExternInit Not Enough", __FUNCTION__);
        return ;    
    }

    gVideoMjpegStream.dwStreamActive = TRUE;
    gVideoMjpegStream.dwBufferCurIndex = 0;
    
    for(bCnt = 0; bCnt < MJPEG_NUM_BUF; bCnt++)
    {
        gVideoMjpegStream.dwBufferActive[bCnt] = FALSE;
        gVideoMjpegStream.dwBufferLength[bCnt] = FALSE;
    }

     gVideoMjpegStream.pbVideoMjpegBuffer[0] = (BYTE*)ext_mem_malloc(UsbMjpgBufferSize*MJPEG_NUM_BUF);
     for(bCnt = 1; bCnt < MJPEG_NUM_BUF; bCnt++)
        gVideoMjpegStream.pbVideoMjpegBuffer[bCnt] = gVideoMjpegStream.pbVideoMjpegBuffer[bCnt-1] + UsbMjpgBufferSize;

}

static void UsbExternDeInit(void)
{
    return;
}


BOOL SetupVendorCommand(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);  
    BOOL bRetVal = FALSE;
    BYTE bUsbData[10] = {0};
    BYTE bUsbDataLen = 0;
    

    // UDTP REQUEST from UDTP_DEVICE_REQUEST for SideMonitor Mode
    if(psDev->psControlCmd->Type == UDTP_REQ_TYPE &&
        psDev->psControlCmd->Value == UDTP_REQ_VALUE &&
        psDev->psControlCmd->Index == UDTP_REQ_INDEX )              
    {    
        MP_DEBUG("%s UDTP REQUEST : %d", __FUNCTION__, psDev->psControlCmd->Request);
        switch (psDev->psControlCmd->Request)
        {
            case UDTP_DEVICE_REQUEST_GET_DEVICE_INFO:
            {
                UDTP_DEVICE_INFO UdtpDeviceInfo;  
                UdtpDeviceInfo.DeviceProtocol = UDTP_PROTOCOL_JPEG;
                UdtpDeviceInfo.DeviceType = 0x1B; //UDTP_DEVICE_MAGICPIXEL_800x480x24; // MP652 ; check by system
                memcpy(bUsbData , &UdtpDeviceInfo, sizeof(UDTP_DEVICE_INFO));
                bUsbDataLen = sizeof(UDTP_DEVICE_INFO);
                bRetVal = TRUE;
            }  break;
                
            case UDTP_DEVICE_REQUEST_GET_BRIGHRNESS:
                bUsbData[0] = 0x28; // check by system
                bUsbDataLen = 1;
                bRetVal = TRUE;
                break;
                
            case UDTP_DEVICE_REQUEST_SET_BRIGHRNESS:
                return TRUE;
                break;
                
            case UDTP_DEVICE_REQUEST_GET_STATE:
                bUsbData[0] =DeviceStateMiniMonitor; // check by system
                bUsbDataLen = 1;
                bRetVal = TRUE;
                break;
                
            case UDTP_DEVICE_REQUEST_MINIMO_TO_MASS:
                mUsbOtgUnPLGSet();
                Api_UsbdSetMode(USB_AP_MSDC_MODE, eWhichOtg);
                mUsbOtgUnPLGClr();                   
                return TRUE;
                break;
                
            case UDTP_DEVICE_REQUEST_GET_SIDEBAR_STATE:
            {
                UDTP_DEVICE_SIDEBAR_STATUS UdtpDeviceSideBarStatus;
                UdtpDeviceSideBarStatus.SideBarState = 0x01;  //  check by system
                UdtpDeviceSideBarStatus.Reserved = 0;
                memcpy(bUsbData , &UdtpDeviceSideBarStatus, sizeof(UDTP_DEVICE_SIDEBAR_STATUS));
                bUsbDataLen = 0;//sizeof(UDTP_DEVICE_SIDEBAR_STATUS);
                bRetVal = TRUE;
            }  break;
                
            case UDTP_DEVICE_REQUEST_SET_SIDEBAR_STATE:
                return TRUE;
                break;

            default:
                MP_ALERT("--W-- %s Error UDTP REQUEST : %d", __FUNCTION__, psDev->psControlCmd->Request);
                break;
        }

        if(bRetVal)
        {
            psDev->eUsbCxFinishAction  = ACT_DONE;
            bOTGCxFxWrRd(FOTG200_DMA2CxFIFO, DIRECTION_IN, bUsbData, bUsbDataLen, eWhichOtg);
        }
        
        return bRetVal;
    }

    

    // USB COMMAND from DEVICE_USB_COMMAND for MSDC Mode
    if(psDev->psControlCmd->Type == USB_CMD_TYPE &&
        psDev->psControlCmd->Request == USB_CMD_REQ &&
        psDev->psControlCmd->Value == USB_CMD_VALUE &&
        psDev->psControlCmd->Index == USB_CMD_INDEX )
    {
        MP_DEBUG("%s USB CMD : 0x%x", __FUNCTION__, psDev->psControlCmd->Length);
        switch(psDev->psControlCmd->Length)
        {
            case COMMAND_FRAME_MANAGER_START:
                bRetVal = TRUE;
                break;
                
            case COMMAND_FRAME_MANAGER_EXIT:
                bRetVal = TRUE;
                break;
                
            case COMMAND_DEVICE_STATE_SELECTOR:
                bRetVal = TRUE;
                break;
                
            case COMMAND_MASS_STORAGE:
                bRetVal = TRUE;
                break;
                
            case COMMAND_BRIGHTNESS:
                bRetVal = TRUE;
                break;
                
            case COMMAND_UPDATE_FIRMWARE:
                bRetVal = TRUE;
                break;
                
            case COMMAND_MINIMONITOR:
                
                mUsbOtgUnPLGSet();
                Api_UsbdSetMode(USB_AP_EXTERN_MODE, eWhichOtg);
                mUsbOtgUnPLGClr();                  
                bRetVal = TRUE;
                break;
                
            case COMMAND_FIRSTTIME_PLUG:
                bRetVal = TRUE;
                break;   
                
            default:
                MP_ALERT("--W-- %s Error USB CMD : 0x%x", __FUNCTION__, psDev->psControlCmd->Length);
                break;
        }

        return bRetVal;  // Samsung Vendor Cmd.
    }    

    return FALSE; // Not Samsung Vendor Cmd.
}

#endif  // (SC_USBDEVICE && USBOTG_DEVICE_EXTERN_SAMSUNG)

