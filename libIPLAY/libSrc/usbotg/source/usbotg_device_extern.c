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
* Filename      : usbotg_device_extern.c
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
#include "usbotg_device_extern.h"
#include "ui.h"
#include "display.h"
#include "usbotg_api.h"


 #if (SC_USBDEVICE && USBOTG_DEVICE_EXTERN)
/*
// Constant declarations
*/
#define SAVE_JPEG_TO_SDCARD       0  // Save JPEG File From PC into SD Card
#define VGA_RAW_DATA_DIRECTLY  0  // Get VGA(640*480) 444 raw data from PC Directly (Enlarge USB Device Task Stack)
#define YUV_444_BYPASSMODE_FOR_TEST  0  //  0 is DMA mode, 1 is Bypass mode
#define DECODE_JPEG_DIRECTLY      0  // Decode JPEG 444 Directly

#define MJPEG_NUM_BUF	2

#if (VGA_RAW_DATA_DIRECTLY == ENABLE)
#define UsbMjpgBufferSize  (640*480*3)  // VGA Raw Data
#else
#define UsbMjpgBufferSize  0x50000  // as same as WEB_CAM_BUF_SIZE
#endif

#define EXT_MSDC_MODE  0x08  // Side Monitor for Change MSDC/SideMonitor Mode
#define EXT_SIDE_MODE   0xFF  // Side Monitor for Change MSDC/SideMonitor Mode

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
    #if ( VGA_RAW_DATA_DIRECTLY || DECODE_JPEG_DIRECTLY)
    return 3;  // WEB_CAM_NOT_EXIST
    #endif
    
    DWORD dwCurIndex;

    dwCurIndex = (gVideoMjpegStream.dwBufferCurIndex == 0) ? (MJPEG_NUM_BUF - 1) : (gVideoMjpegStream.dwBufferCurIndex - 1);
    return gVideoMjpegStream.dwBufferActive[dwCurIndex] ? 0 : 1;	
}

BOOL SideMonitorChangeMode(PEXT_CHANGE_MODE pbModeStr,BYTE bCurMode, BYTE bNextMode, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE  psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);  

    MP_DEBUG("%s PC  CurMode:%s NextMode:%s",__FUNCTION__, ((bCurMode == EXT_MSDC_MODE) ? "MSDC" : "SIDE"), ((bNextMode == EXT_MSDC_MODE) ? "MSDC" : "SIDE"));
    MP_DEBUG("%s DPF CurMode:%s",__FUNCTION__, ((psUsbDev->sDesc.bUsbApMode == USB_AP_MSDC_MODE) ? "MSDC" : "SIDE"));

    pbModeStr->bExtLen = sizeof(EXT_CHANGE_MODE);
    
    if(psUsbDev->sDesc.bUsbApMode == ((bCurMode == EXT_MSDC_MODE) ? USB_AP_MSDC_MODE : USB_AP_EXTERN_MODE)) // Check current mode of DPF
    {
        Api_UsbdFinal(eWhichOtg);   //mUsbOtgUnPLGSet();

        #if USBOTG_DEVICE_MSDC_TECO
        if(Idu_GetIs444Mode())
        	Idu_Chg_422_Mode();
        #endif        

        switch(bCurMode)
        {
            case EXT_MSDC_MODE:
                SetupExternMode();   //Api_UsbdSetMode((bNextMode == EXT_MSDC_MODE ? USB_AP_MSDC_MODE : USB_AP_EXTERN_MODE) , eWhichOtg);
                //WebCamStartPlay();  // Mask:SideMonitorStartStop get from AP
                break;

            case EXT_SIDE_MODE:
                SetupMSDCMode();    //Api_UsbdSetMode((bNextMode == EXT_MSDC_MODE ? USB_AP_MSDC_MODE : USB_AP_EXTERN_MODE) , eWhichOtg);
                //WebCamStopPlay();  // Mask:SideMonitorStartStop get from AP
                break;

            default:
                break;
        }
        
        pbModeStr->bExtCurClass = bCurMode;
        pbModeStr->bExtNextClass = bNextMode;
        
        Api_UsbdInit(eWhichOtg);    //mUsbOtgUnPLGClr();
        
        return TRUE;
    }
    else
    {
        pbModeStr->bExtCurClass = bNextMode;
        pbModeStr->bExtNextClass = bCurMode;    
        MP_ALERT("--W-- %s -Not Change- DPF Current Mode(%s) is the same as PC Next Mode!", __FUNCTION__, ((bNextMode == EXT_MSDC_MODE) ? "MSDC" : "SIDE"));
        return FALSE;
    }
}


BOOL SideMonitorGetResolution(PEXT_CUR_RESOLUTION pbResolutionStr)
{
    WORD wWidth = 0, wHeight = 0;
    
    SystemPanelSizeGet(&wWidth, &wHeight);  

    #if ( VGA_RAW_DATA_DIRECTLY || DECODE_JPEG_DIRECTLY)
    wWidth = 640;
    wHeight = 480;
    #endif
    
    pbResolutionStr->bExtLen = sizeof(EXT_CUR_RESOLUTION);
    pbResolutionStr->bExtWidthH = (wWidth >> 8) & 0xff ;
    pbResolutionStr->bExtWidthL =   wWidth & 0xff;
    pbResolutionStr->bExtHeightH =  (wHeight >> 8) & 0xff ;
    pbResolutionStr->bExtHeightL =    wHeight & 0xff;

    MP_DEBUG("%s DPF Current Resolution to PC Width(%d): 0x%x%x Height(%d): 0x%x%x", __FUNCTION__, 
                            wWidth, pbResolutionStr->bExtWidthH, pbResolutionStr->bExtWidthL, 
                            wHeight,  pbResolutionStr->bExtHeightH, pbResolutionStr->bExtHeightL);
}

BOOL SideMonitorStartStop(PEXT_START_STOP pbActionStr, BYTE bAction)
{
    MP_DEBUG("%s SideMonitor  --%s--", __FUNCTION__, (bAction == SMA_START) ? "Start" : "Stop" );
        
    switch(bAction)
    {
        case SMA_STOP:     
            WebCamStopPlay();  // Should send event to system
            break;
            
        case SMA_START:
            WebCamStartPlay();  // Should send event to system
            break;
    }

    pbActionStr->bExtLen = sizeof(EXT_START_STOP);
    pbActionStr->bExtAct = bAction;
    pbActionStr->bReserved = 0;  

}
    
DWORD frameCounter = 0, startTime = 0 , countTime = 0;
BYTE eUsbExternDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    DWORD dwCurIndex; 

    #if ( VGA_RAW_DATA_DIRECTLY)
    if(startTime == 0)  
    {
        startTime = GetSysTime();
        Idu_Chg_444_Mode();
    }
    #endif

    dwCurIndex = gVideoMjpegStream.dwBufferCurIndex;
    
    bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,
        (gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex] + gVideoMjpegStream.dwBufferLength[dwCurIndex] ),
        u16FIFOByteCount, eWhichOtg);

    gVideoMjpegStream.dwBufferLength[dwCurIndex] += u16FIFOByteCount;  

    if((u16FIFOByteCount < 512) ||
        #if ( VGA_RAW_DATA_DIRECTLY == ENABLE)
	 ( gVideoMjpegStream.dwBufferLength[dwCurIndex]  >= 640*480*3) ||	
	 #endif
        ( gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -2] == 0xFF &&
          gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -1] == 0xD9)) // USB Short Package or Eof of JPEG
    {        
        #if SAVE_JPEG_TO_SDCARD  // Save JPEG File into SD Card
        MP_ALERT("Save photo from PC into SD card!! Size %d", gVideoMjpegStream.dwBufferLength[dwCurIndex]);
        Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "Side", "JPG", &gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][0], gVideoMjpegStream.dwBufferLength[dwCurIndex]);
        #endif
        
        gVideoMjpegStream.dwBufferActive[dwCurIndex] = TRUE;
        MP_DEBUG("Current Video Stream Index %d Len %d",dwCurIndex , gVideoMjpegStream.dwBufferLength[dwCurIndex]);

        // Check Buffer Length less than UsbMjpgBufferSize
        if(gVideoMjpegStream.dwBufferLength[dwCurIndex] > UsbMjpgBufferSize)
            MP_ALERT("--E-- %s Mpjpeg is over buffer size!! Mjpeg 0x%x > Buffer 0x%x", __FUNCTION__, gVideoMjpegStream.dwBufferLength[dwCurIndex], UsbMjpgBufferSize);

        #if ( VGA_RAW_DATA_DIRECTLY == DISABLE)
        // Check the EOF of MJPEG File.
        if( gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -2] != 0xFF ||
            gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -1] != 0xD9)
            MP_ALERT("--E-- %s is not MJPEG file!! Last Char 0x%x 0x%x", __FUNCTION__,
                                gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -2],
                                gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][gVideoMjpegStream.dwBufferLength[dwCurIndex] -1]);        
        #endif

	#if ( VGA_RAW_DATA_DIRECTLY == ENABLE)
	ST_IMGWIN * ImgWin;
	DWORD dwCnt = 0;
	ImgWin = Idu_GetNextWin();

	#if (YUV_444_BYPASSMODE_FOR_TEST == ENABLE)
		ST_IMGWIN * srcwin;
		ST_IMGWIN srcwin_content;
		srcwin = &srcwin_content;
		srcwin->pdwStart = (DWORD)(&gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][0])|(0xa0000000);
		srcwin->wWidth = 640;
		srcwin->wHeight = 480;
		srcwin->wType = 444;
		srcwin->dwOffset = 640*3;
		//Idu_Chg_444_Mode();
		//Idu_ChgWin(Idu_GetNextWin());
		//Ipu_ImageScaling(srcwin, ImgWin, 0, 0, srcwin->wWidth, srcwin->wHeight, 0, 0, ImgWin->wWidth, ImgWin->wHeight, 0);
		register IDU *idu = (IDU *) IDU_BASE;
		if(idu->RST_Ctrl_Bypass & 0x1)
		{
			Video_Update_Frame(srcwin, 0);
		}
		else
		{
			Ipu_Video_Bypass_Scaling(srcwin, ImgWin, 0, 0, srcwin->wWidth, srcwin->wHeight, 0, 0, ImgWin->wWidth, ImgWin->wHeight, 0);
		}
	#else
		//for(dwCnt = 0; dwCnt < 480; dwCnt++)
		//	mmcp_memcpy(((BYTE*)(ImgWin->pdwStart)+(800*3*dwCnt)), &gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][640*3*dwCnt], 640*3);
		mmcp_block(&gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][0], (BYTE*)(ImgWin->pdwStart),  480, 640*3, 640*3, 800*3);
		Idu_ChgWin(ImgWin);
	#endif


        frameCounter ++;
        #endif

        #if DECODE_JPEG_DIRECTLY
        BYTE *pTarget;
        DWORD FreeSpace;
        ST_IMGWIN * ImgWin, *TrgWin;

        pTarget = (BYTE *)ext_mem_malloc(640*480*3);

        Img_Jpeg2ImgBuf(&gVideoMjpegStream.pbVideoMjpegBuffer[dwCurIndex][0], (BYTE *)pTarget, IMG_DECODE_PHOTO, gVideoMjpegStream.dwBufferLength[dwCurIndex], 640*480*3);

    	ImgWin = Idu_GetNextWin();

    	TrgWin = (ST_IMGWIN*)ImageGetDecodeWin();
    	TrgWin->pdwStart = (DWORD*)pTarget;
    	TrgWin->wWidth = Jpg_GetTargetWidth();
    	TrgWin->wHeight = Jpg_GetTargetHeight();
    	TrgWin->dwOffset = (TrgWin->wWidth << 1);
        //#if 0 // IMAGE_ROTATE_MEMO_ENABLE
        //	ImageScaleFromJPEGTarget(ImgWin, pTarget, 0);
        //#else
        	ImageScaleFromJPEGTarget(ImgWin, pTarget);
        //#endif
        Idu_ChgWin(ImgWin);
        ext_mem_free(pTarget);

        frameCounter ++;        
        #endif

        // Next Buffer
        gVideoMjpegStream.dwBufferCurIndex = (dwCurIndex+1 == MJPEG_NUM_BUF) ? 0 : dwCurIndex+1;
        gVideoMjpegStream.dwBufferActive[gVideoMjpegStream.dwBufferCurIndex] = FALSE;        
        gVideoMjpegStream.dwBufferLength[gVideoMjpegStream.dwBufferCurIndex]= 0;
    }

    #if ( VGA_RAW_DATA_DIRECTLY || DECODE_JPEG_DIRECTLY)
    countTime = SystemGetElapsedTime(startTime);
    if (countTime > 1000)
    {
        startTime = GetSysTime();
        mpDebugPrint("\r\n%i f/s", frameCounter/(countTime/1000));

        frameCounter = 0;
    }
    #endif

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
    ext_mem_free(gVideoMjpegStream.pbVideoMjpegBuffer[0]);
    return;
}

BOOL SetupVendorCommand(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_DESC psDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    BOOL bRetVal = FALSE;
    BYTE bUsbData[10] = {0};
    BYTE bUsbDataLen = 0;    

    if(!(psDev->psControlCmd->Type == 0x0  && psDev->psControlCmd->Request == 0x6)) // Not (standard command && GET_DESCRIPTOR)
        return FALSE;

    if((((BYTE)(psDev->psControlCmd->Value >> 8)) & EXTERN_CHANGE_MODE) == EXTERN_CHANGE_MODE)
        MP_DEBUG("Side Monitor Cmd(wValue)0x%x  Value(wIndex)0x%x 0x%x", (BYTE)(psDev->psControlCmd->Value >> 8), (psDev->psControlCmd->Index&0xFF), ((psDev->psControlCmd->Index>>8)&0xFF));

    // Side Monitor Setup Vendor Command
    switch((BYTE)(psDev->psControlCmd->Value >> 8))
    {
        case EXTERN_CHANGE_MODE: // Side Monitor for Change MSDC/SideMonitor Mode
        {
            MP_DEBUG("Side Monitor for Change MSDC/SideMonitor Mode 0xC0");
            EXT_CHANGE_MODE sExtChangeMode;  
            SideMonitorChangeMode(&sExtChangeMode, (psDev->psControlCmd->Index&0xFF), ((psDev->psControlCmd->Index>>8)&0xFF), eWhichOtg);            
            memcpy(bUsbData , &sExtChangeMode, sizeof(EXT_CHANGE_MODE));
            bUsbDataLen = sizeof(EXT_CHANGE_MODE);
            bRetVal = TRUE;
        }  break;

        case EXTERN_CUR_RESOLUTION: // Side Monitor for Current Resolution
        {
            MP_DEBUG("Side Monitor for Current Resolution 0xC1");
            EXT_CUR_RESOLUTION sExtCruResolution;  
            SideMonitorGetResolution(&sExtCruResolution);                
            memcpy(bUsbData , &sExtCruResolution, sizeof(EXT_CUR_RESOLUTION));
            bUsbDataLen = sizeof(EXT_CUR_RESOLUTION);
            bRetVal = TRUE;
        }  break;
        
        case EXTERN_START_STOP: // Side Monitor for Start/Stop SideMonitor
        {
            MP_DEBUG("Side Monitor for Start/Stop SideMonitor 0xC2"); 
            EXT_START_STOP sExtStartStop;  
            SideMonitorStartStop(&sExtStartStop, psDev->psControlCmd->Index & 0xFF); 
            memcpy(bUsbData , &sExtStartStop, sizeof(EXT_START_STOP));
            bUsbDataLen = sizeof(EXT_START_STOP);
            bRetVal = TRUE;
        }  break;
            
        default:
            break;
    }

    if(bRetVal)
    {        
        psDev->eUsbCxFinishAction  = ACT_DONE;
        bOTGCxFxWrRd(FOTG200_DMA2CxFIFO, DIRECTION_IN, bUsbData, bUsbDataLen, eWhichOtg);
    }
    
    return bRetVal;    
}


#endif  // (SC_USBDEVICE && USBOTG_DEVICE_EXTERN)

