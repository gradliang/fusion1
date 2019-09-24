/*
// Include section
*/
#define LOCAL_DEBUG_ENABLE 1
#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "devio.h"
#include "ui.h"
#include "setup.h"
#include "display.h"
#include "xpg.h"
//#include "RingBuffer.h"

#if (USBOTG_WEB_CAM)
//#define		WEB_CAM_BUF_SIZE												524288 // 128*0x1000   in webCamStateMachine
#define EN_JPGSIZE     (1024 * 90 * 6)
#define		AUTO_CHANGE_MODE_TIMER												8000//11000

extern int count_num,sce1;
#if USBCAM_TMP_SHARE_BUFFER
BYTE *g_pbTmpPicBuffer=NULL;// for cache decode and encode
DWORD g_dwPicBufferSize=0;
static ST_IMGWIN g_stUsbCamCacheWin = {0};
#endif
static BYTE st_bWebCamWaitCapture=0;
extern BYTE g_bFreezeUsbCamDisplay;

//------------------For other page get value--------------------------------//
#if USBCAM_TMP_SHARE_BUFFER
ST_IMGWIN *Idu_GetUsbCamCacheWin()
{
	return &g_stUsbCamCacheWin;
}
#endif
//------------------Repacket----------------------------------------------------//
void WebCamStart()
{
	ST_IMGWIN  *pWin = Idu_GetNextWin();
    ST_SCREEN_TABLE *pstScreenTable,*pstPreScreenTable;
	struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(WEBCAM_USB_PORT);
    PUSB_HOST_UVC psUsbhUvc = (PUSB_HOST_UVC)UsbOtgHostUvcDsGet(WEBCAM_USB_PORT);
	WORD wWidth=psUsbHostAVdc->dwVideoFrameSize>>16;
	WORD wHeight=psUsbHostAVdc->dwVideoFrameSize&0x0000ffff;
	DWORD dwSize;
	BYTE bNewScreenIndex=0;

    mpDebugPrint("%s: total=%d", __FUNCTION__,mem_get_free_space_total());
	if (!SystemCardPresentCheck(USB_WEBCAM_DEVICE))
		return;
#if MAKE_XPG_PLAYER
	xpgSearchAndGotoPage("Preview");
#endif
	mpClearWin(Idu_GetNextWin());
	mpClearWin(Idu_GetCurrWin());
	//mpDebugPrint("%s:cur->%p  offset->%d", __func__, ((ST_IMGWIN *)Idu_GetCurrWin())->pdwStart,((ST_IMGWIN *)Idu_GetCurrWin())->dwOffset);
	pWin = Idu_GetNextWin();
#if SET_DECODE_OFFSET_ENALBE
	if (wWidth < pWin->wWidth)
	{
		SetDecodeWinOffset(pWin->dwOffset);
	}
	else
	{
		SetDecodeWinOffset(0);
	}
#endif
#if CUT_IMAGE_HEIGHT_ENALBE
	if (wHeight>pWin->wHeight)
		SetDecodeWinHeight(pWin->wHeight);
	else
		SetDecodeWinHeight(0);
#endif
	g_bFreezeUsbCamDisplay=0;
	//webCamResetParam(WEBCAM_USB_PORT);
	psUsbhUvc->bNewOneFrame=0;
	Api_UsbhWebCamStart(WEBCAM_USB_PORT);
#ifdef OSDICON_WIFI
	ShowWifiPowerStatus();
#endif

}

/////////////////////////////Web cam capture////////////////
void WebCamSetCapture()
{
	st_bWebCamWaitCapture=1;
}

BYTE WebCamGetCaptureFlag(void)
{
	return st_bWebCamWaitCapture;
}

void WebCamCapture(BYTE *image_buf,DWORD image_size)
{
	if (!st_bWebCamWaitCapture)
		return;
	
	mpDebugPrint("Webcam Capture ...");
	STREAM *handle=(STREAM *)GetNewCaptureHandle();
	if (handle)
	{
#ifdef OSDICON_DC
		BYTE bStr[16];
		sprintf(bStr,"%d", GetNeedCaptureNumber());
		ShowActionIconWithString(OSDICON_DC,bStr,OSD_NEW_WHITE,500);
#endif
		#if SCALER_UP_FOR_CAPTURE
		BYTE *JpegBuf = NULL;
		DWORD JpegBufSize,IMG_size;
		ST_IMGWIN stTmpWin;

		JpegBufSize=1280 * 720 * 2;
		JpegBuf = (BYTE*)ext_mem_malloc(JpegBufSize);
		ImgWinInit(&stTmpWin, NULL, 720, 1280);
		stTmpWin.pdwStart = ext_mem_malloc(JpegBufSize);
		if (stTmpWin.pdwStart==NULL)
		{
			ext_mem_free(JpegBuf);
		}
		if (stTmpWin.pdwStart)
		{
			mpCopyWin(&stTmpWin, Idu_GetCurrWin());
			#if CAPTURE_TIME_STAMP_ON
			WriteTimeStampStringToPhoto(&stTmpWin);
			#endif
			IMG_size = ImageFile_Encode_Img2Jpeg(JpegBuf, &stTmpWin);

			if (JpegBufSize < IMG_size)
			{
				mpDebugPrint("--E-- %s: memory overflow", __FUNCTION__);
				//free memory
				DeleteFile(handle);
			}
			else
			{
				FileWrite(handle, (BYTE *) JpegBuf, IMG_size);
				FileClose(handle);
				mpDebugPrint("-- %s:ok!", __FUNCTION__);
			}

			ext_mem_free(JpegBuf);
			ext_mem_free(stTmpWin.pdwStart);
		}
		else if (image_buf!=NULL && image_size)
		{
			FileWrite(handle, image_buf, image_size);
			FileClose(handle);
			mpDebugPrint("Webcam Capture OK!");
		}
		#else
		FileWrite(handle, image_buf, image_size);
		FileClose(handle);
		mpDebugPrint("Webcam Capture OK!");
		#endif
	}
	st_bWebCamWaitCapture=0;
	//SetUsbCamPreStop();
}

#if 0
void CaptureQueueImageToFile(void)
{
    STREAM *	handle;
	BYTE *pbDataBuffer;
	DWORD dwSize;
	//ST_IMGWIN  *pWin = Idu_GetCurrWin();

#if ISOC_QUEUE_DYNAMIC
	GetQueueDataHead(&pbDataBuffer,&dwSize);
	//pbDataBuffer=pWin->pdwStart;
	//dwSize=pWin->dwOffset*pWin->wHeight;
	if (dwSize &&  pbDataBuffer != NULL)
	{
		handle=(STREAM *)GetNewCaptureHandle();
		if (handle)
		{
			BYTE bStr[16];
			sprintf(bStr,"%d", GetNeedCaptureNumber());
			ShowActionIconWithString(OSDICON_DC,bStr,OSD_NEW_WHITE,500);
			FileWrite(handle, pbDataBuffer, dwSize);
			FileClose(handle);
			mpDebugPrint("Webcam Capture OK!");
		}
	}
#endif
}
#endif
/////////////////////////////Web cam record////////////////
int RecordWebCamStop()
{
	MP_DEBUG("%s", __func__);
	if (!bRecordMode())
		return PASS;
	SetRecordState(Preview_state);
	return AVIEN_closefile();
}

void RecordWebCamStopAndRestart(void)
{
	if (RecordWebCamStop()==PASS)
	{
		RecordWebCamStart();
	}
}

void KeyToWebCamRecordStartOrStop(void)
{
	if (bRecordMode())
	{
		Ui_TimerProcRemove(RecordWebCamStop);
		RecordWebCamStop();
	}
	else
	{
		RecordWebCamStart();
	}
}

void RecordWebCamStart(void)
{
	STREAM *handle;
	record_argument currPara;
	int  iRet;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(WEBCAM_USB_PORT);
	WORD wWidth=psUsbHostAVdc->dwVideoFrameSize>>16;
	WORD wHeight=psUsbHostAVdc->dwVideoFrameSize&0x0000ffff;
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

	if (bRecordMode())
		return ;
	if (dwHashKey != xpgHash("Preview"))
		return;
 	handle =(STREAM *)GetRecordFileHandle();
	if (handle==NULL)
		return ;
     currPara = RecordArgumentGet();
	if (wWidth==800 && wHeight==600)
	{
		currPara.resolution = RESOLUTION_800x600_15FPS;
		currPara.fps = 15;
		currPara.image_size = SIZE_SVGA_800x600;
	}
	else if (wWidth==1280 && wHeight==720)
	{
		currPara.resolution = RESOLUTION_720P_16;
		currPara.fps = 16;
		currPara.image_size = SIZE_720P_1280x720;
	}
	else // 640*480
	{
		currPara.resolution = RESOLUTION_VGA_24;//RESOLUTION_VGA_24;RESOLUTION_VGA_16
		currPara.fps = 24;
		currPara.image_size = SIZE_VGA_640x480;
	}

    currPara.handle = handle;
    //currPara.movie_length = 100;
    currPara.quantification = r_midlle;
    currPara.RecordingAudio = r_close;//
    //currPara.sensor_buffer_num = 0; // 0 or 1

    currPara.driveIndex = DriveCurIdGet();
    RecordArgument(&currPara);
	SetResolution();
	//st_pbRecBuffer = (BYTE *)ext_mem_malloc(EN_JPGSIZE);

	iRet=AVIEN_createfile(handle,EN_JPGSIZE, 1024, 16000, 8);
	if (iRet==PASS)
	{
		SetRecordState(Recording_state);
#if OSDICON_Rec
		Timer_Recording_Icon();
#endif
		//Ui_TimerProcAdd(GetRecordLenth(), RecordWebCamStopAndRestart);
	}
	else
		DeleteFile(handle);
	mpDebugPrint("%s:%d", __func__, iRet);
	//return iRet;
}

#if ONE_WINDOW_REC_VER
int RecordWebCamFillBuffer(BYTE *image_buf,DWORD image_size)
{
	if (!bRecordMode())
		return PASS;

	if (GetRemainFreeSize()<0)
		return FAIL;
	if (SetRemainFreeSize(GetRemainFreeSize()-(image_size>>4))<0)
	{
		EventSet(UI_EVENT, EVENT_DISK_FULL);
		return FAIL;
	}

	//mpDebugPrint("-%d",image_size);//640*480->15000
	//UartOutText("R");
	if(sce1>=count_num)
	{
		sce1=0;
		EventSet(AUDIO_ID, BIT0);
	}
	sce1++;
	return ( AVIEN_fillJpeg(image_buf, ALIGN_2(image_size)));
}
/*
int RecordWebCamProcess(BYTE *image_buf,ST_IMGWIN *pWin)
{
	
	if (!bRecordMode())
		return PASS;
	//UartOutText("R");

	if(sce1>=count_num)
	{
		sce1=0;
		EventSet(AUDIO_ID, BIT0);
	}
	sce1++;

	DWORD iSize;
	BYTE *pImgBuf;
	#if USBCAM_TMP_SHARE_BUFFER
	if (g_pbTmpPicBuffer)
		(DWORD *)pImgBuf = (DWORD *) ((DWORD) g_pbTmpPicBuffer | 0x20000000);
	else
	#endif
	(DWORD *)pImgBuf = (DWORD *) ((DWORD) image_buf | 0x20000000);
	iSize = ImageFile_Encode_Img2Jpeg_WithQT(pImgBuf, pWin,8);

	return AVIEN_fillJpeg(pImgBuf, ALIGN_2(iSize));
}
*/
#endif
void Set_WebCam_Brightness(void)
{
	//UvcTestFunction6(WEBCAM_USB_PORT);
	UvcSetContrast(WEBCAM_USB_PORT);
	Ui_TimerProcAdd(3000, Set_WebCam_Brightness);
}

void Print_WebCam_Config(void)
{
	UvcTestFunction5(WEBCAM_USB_PORT);
}

void UvcIn_MJPG_640x480();

void UvcIn_YUV_1600x1200()
{
	WHICH_OTG eWhichOtg=WEBCAM_USB_PORT;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    
    mpDebugPrint("%s:YUV Begin", __func__);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 1600;
    sFrameRes.wHigh  = 1200;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_YUV, sFrameRes);
    WebCamStart();
    mpDebugPrint("%s:YUV End", __func__);
	Ui_TimerProcAdd(AUTO_CHANGE_MODE_TIMER, UvcIn_MJPG_640x480);
}

void UvcIn_MJPG_1280x720() //max 15fps
{
	WHICH_OTG eWhichOtg=WEBCAM_USB_PORT;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    
    mpDebugPrint("%s:MJPEG Begin", __func__);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 1280;
    sFrameRes.wHigh  = 720;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    WebCamStart();
    mpDebugPrint("%s:MJPEG End", __func__);
	//Ui_TimerProcAdd(21000, UvcIn_YUV_1600x1200);
	Ui_TimerProcAdd(AUTO_CHANGE_MODE_TIMER, UvcIn_MJPG_640x480);
}

void UvcIn_MJPG_1024x768()
{
	WHICH_OTG eWhichOtg=WEBCAM_USB_PORT;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    
    mpDebugPrint("%s:MJPEG Begin", __func__);
    //UvcTestFunction0(eWhichOtg);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 1024;
    sFrameRes.wHigh  = 768;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    WebCamStart();
    mpDebugPrint("%s:MJPEG End", __func__);
	Ui_TimerProcAdd(AUTO_CHANGE_MODE_TIMER, UvcIn_MJPG_1280x720);
}

void UvcIn_MJPG_800x600() //max 25 fps
{
	WHICH_OTG eWhichOtg=WEBCAM_USB_PORT;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    
    mpDebugPrint("%s:MJPEG Begin", __func__);
    //UvcTestFunction0(eWhichOtg);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 800;
    sFrameRes.wHigh  = 600;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    WebCamStart();
    mpDebugPrint("%s:MJPEG End", __func__);
	Ui_TimerProcAdd(AUTO_CHANGE_MODE_TIMER, UvcIn_MJPG_1024x768);
}

void UvcIn_MJPG_640x480()
{
	WHICH_OTG eWhichOtg=WEBCAM_USB_PORT;
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    
    mpDebugPrint("%s:MJPEG Begin", __func__);
    //UvcTestFunction0(eWhichOtg);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 640;
    sFrameRes.wHigh  = 480;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    WebCamStart();
    mpDebugPrint("%s:MJPEG End", __func__);
	Ui_TimerProcAdd(AUTO_CHANGE_MODE_TIMER, UvcIn_MJPG_1280x720);
}

void StopAllWebCamAction(void)
{
	//Ui_TimerProcRemove(CaptureQueueImageToFile);
	Ui_TimerProcRemove(RecordWebCamStart);
	Ui_TimerProcRemove(RecordWebCamStopAndRestart);
	if (bRecordMode())
	{
		Ui_TimerProcRemove(RecordWebCamStop);
		RecordWebCamStop();
	}


}

void StopWebCam(void)
{
	StopAllWebCamAction();
	Api_UsbhWebCamStop(WEBCAM_USB_PORT);
}

int WebCamEventProcess(DWORD dwEvent)
{
    WORD wDevId;

    wDevId = DEV_USB_WEBCAM;
    if(dwEvent == EVENT_WEB_CAM_IN)
    {
        /////////////////////////////////////////////////////
        //      Add Code for UI Operation for Webcam In   //
        /////////////////////////////////////////////////////
        Api_UsbhStorageDeviceInit(wDevId);
#if 0
        if (!SystemCardPresentCheck(USB_WEBCAM_DEVICE))
        {
            Api_UsbhStorageDeviceRemove(wDevId);
        }
        else
        {
            DriveAdd(USB_WEBCAM_DEVICE);
            if(DriveChange(USB_WEBCAM_DEVICE) == 0)
            {
                mpDebugPrint("DriveChange Error");
            }
            else
            {
                MP_ALERT("EVENT_WEB_CAM_IN");    
				WebCamStart(); // for test, JL, 01082016
				TurnOnBackLight();
            }
        }
#else
			if (SystemCardPresentCheck(USB_WEBCAM_DEVICE))
			{
#if (T530_AV_IN&&USBCAM_IN_ENABLE)
			CheckToStopPreview();
#endif
				USBH_UVC_FRAME_RESOLUTION sFrameRes;
				ST_IMGWIN  *pWin = Idu_GetNextWin();
	struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

#if 1
				if ((pstScreen->wInnerWidth>=800 && pstScreen->wInnerHeight>=600))
				{
					sFrameRes.wWidth = 800;
					sFrameRes.wHigh  = 600;
				}
				else if (pstScreen->wInnerWidth>=640 && pstScreen->wInnerHeight>=480)
				{
					sFrameRes.wWidth = 640;
					sFrameRes.wHigh  = 480;
				}
				else
				{
					sFrameRes.wWidth = 320;
					sFrameRes.wHigh  = 240;
				}
#else
				sFrameRes.wWidth = 640;
				sFrameRes.wHigh  = 480;
#endif
				Api_UsbhWebCamVedioFormat(WEBCAM_USB_PORT, USING_MJPEG, sFrameRes);// USING_MJPEG  USING_YUV
#if SENSOR_THREE_MODE
				RunSensorCurrentColorSet();
#endif
#if SENSOR_ADJUST_ALL_COLOR
				DoSensorAdjust(0);
#endif
				WebCamStart(); // for test, JL, 01082016
				TurnOnBackLight();
				//Ui_TimerProcAdd(5000,CaptureQueueImageToFile);
				//Ui_TimerProcAdd(5000, RecordWebCamStart);
				//Ui_TimerProcAdd(AUTO_CHANGE_MODE_TIMER, UvcIn_MJPG_640x480);
				//Ui_TimerProcAdd(3000, Print_WebCam_Config);
				//Ui_TimerProcAdd(8000, Set_WebCam_Brightness);
				//TimerToCapture();

#if 0//USBCAM_TMP_SHARE_BUFFER
				if (g_pbTmpPicBuffer)
				{
					ext_mem_free(g_pbTmpPicBuffer);
					g_pbTmpPicBuffer=NULL;
				}
				g_dwPicBufferSize=sFrameRes.wWidth*sFrameRes.wHigh*2;
				g_pbTmpPicBuffer = (BYTE *)ext_mem_malloc(g_dwPicBufferSize);
				if (g_pbTmpPicBuffer)
				{
					ImgWinInit(&g_stUsbCamCacheWin, NULL, sFrameRes.wHigh, sFrameRes.wWidth);
					g_stUsbCamCacheWin.pdwStart = ext_mem_malloc(pWin->wWidth * pWin->wHeight * 2);
				}
#endif

			}
#endif
    }
    else if(dwEvent == EVENT_WEB_CAM_OUT)
    {
        /////////////////////////////////////////////////////
        //      Add Code for UI Operation for Webcam Out   //
        /////////////////////////////////////////////////////
		Api_UsbhStorageDeviceRemove(wDevId);
		StopAllWebCamAction();
#if USBCAM_TMP_SHARE_BUFFER
		if (g_pbTmpPicBuffer)
		{
			ext_mem_free(g_pbTmpPicBuffer);
			g_pbTmpPicBuffer=NULL;
		}
#endif
		SetRecordState(Rec_StandBy_state);

#if 0
        DriveDelete(USB_WEBCAM_DEVICE);
        MP_ALERT("EVENT_WEB_CAM_OUT");    
#else
		xpgCb_EnterCamcoderPreview();

#endif

    }

    return PASS;
}

#if ISOC_QUEUE_DYNAMIC
#define QUEUE_DEBUG_ENABLE								0

#define REMAIN_BUFFER_SIZE												3500*1024

BYTE *g_pbUsbIsocQueueBuffer=NULL;
DWORD g_dwQueueBufferSize=0;

#if ISOC_EVERY_QUEUE_SIZE_FIXED
static DWORD dwTotalQueNum=0,dwOneQueSize=0, head,tail, st_dwQueueDataSize[NUMBER_OF_ISOC_Q_ELEMENTS]={0};
#else
#define QueueBufferFlag   										0x59 // morthan 0x50
#define MIN_SPACE           										1024 // 0 is ok too
//Make sure the sizeof(struct mem) is 16
#define SIZEOF_STRUCT_MEM   								16
typedef struct
{
	WORD tag;
	BYTE used;
	BYTE TaskId; // ->  0->free  1->has decode,wait release    QueueBufferFlag-1->has alloc wait write end  QueueBufferFlag->write ok
	DWORD size;
	DWORD next, prev;
} ST_HEAP_MEM;

#define WAIT_RELEASE_NUMBER											4
static WORD wQueueBufferTag;
static ST_HEAP_MEM *QueueBufStart;
static ST_HEAP_MEM *QueueFreeBufStart,*QueueHeadBuffer,*QueueTailBuffer; // tail is last be used buffer
SWORD g_swWaitWrite;
DWORD g_dwWaitRelease[WAIT_RELEASE_NUMBER];
//for debug
SWORD g_swQueueUsedNum=0;
#endif
#if 0//USBCAM_DEBUG_ISR_LOST
DWORD g_dwQueueUsed=0;
#endif


BYTE *GetIsocWholeQueueBufferStart()
{
	return g_pbUsbIsocQueueBuffer;
}

void IsocWholeQueueBufferReset(void)
{
	DWORD i;
	
	if (!g_dwQueueBufferSize || g_pbUsbIsocQueueBuffer==NULL)
		return;
	memset(g_pbUsbIsocQueueBuffer,0,g_dwQueueBufferSize);
#if ISOC_EVERY_QUEUE_SIZE_FIXED
	head=0;
	tail=0;
	for (i=0;i<NUMBER_OF_ISOC_Q_ELEMENTS;i++)
		st_dwQueueDataSize[i]=0;
#else
	wQueueBufferTag++;
	QueueBufStart=(ST_HEAP_MEM *)g_pbUsbIsocQueueBuffer;
	QueueBufStart->tag=wQueueBufferTag;
	QueueBufStart->used=0;
	QueueBufStart->TaskId=QueueBufferFlag;
	QueueBufStart->size=g_dwQueueBufferSize-SIZEOF_STRUCT_MEM;
	QueueBufStart->prev=0;
	QueueBufStart->next=0;
	QueueFreeBufStart=QueueBufStart;
	QueueHeadBuffer=QueueBufStart;
	g_swQueueUsedNum=0;
	g_swWaitWrite=0;
	for (i=0;i<WAIT_RELEASE_NUMBER;i++)
	g_dwWaitRelease[i]=0;
#if QUEUE_DEBUG_ENABLE
    UartOutText("-r-");
	UartOutValue((DWORD)QueueBufStart, 8);
#endif
#endif
}

void IsocWholeQueueBufferAlloc()
{
    DWORD dwQueueElementByteCount;

	IntDisable();
    //UartOutText("-WA-");
	if (g_pbUsbIsocQueueBuffer&&g_dwQueueBufferSize)
		ext_mem_free(g_pbUsbIsocQueueBuffer);
	if (mem_get_free_space_total()<REMAIN_BUFFER_SIZE)
	{
	    IntEnable();
		return;
	}
	g_dwQueueBufferSize=ALIGN_32(mem_get_free_space_total()-REMAIN_BUFFER_SIZE);
	dwQueueElementByteCount=GetQueueElementByteCount(WEBCAM_USB_PORT);
	#if ISOC_EVERY_QUEUE_SIZE_FIXED
	dwOneQueSize=dwQueueElementByteCount;
	dwTotalQueNum=g_dwQueueBufferSize/dwOneQueSize;
	if (dwTotalQueNum>NUMBER_OF_ISOC_Q_ELEMENTS)
		dwTotalQueNum=NUMBER_OF_ISOC_Q_ELEMENTS;
	g_dwQueueBufferSize=dwTotalQueNum*dwOneQueSize;
    mpDebugPrint("%s: %dx%d=%d", __FUNCTION__,dwOneQueSize,(g_dwQueueBufferSize/dwOneQueSize),g_dwQueueBufferSize);
	#else
	if (dwQueueElementByteCount&&(dwQueueElementByteCount+SIZEOF_STRUCT_MEM)*NUMBER_OF_ISOC_Q_ELEMENTS<g_dwQueueBufferSize)
	{
		dwQueueElementByteCount += SIZEOF_STRUCT_MEM;
		g_dwQueueBufferSize=dwQueueElementByteCount*NUMBER_OF_ISOC_Q_ELEMENTS;
	}
    mpDebugPrint("%s: g_dwQueueBufferSize=%p  %p", __FUNCTION__,g_dwQueueBufferSize,dwQueueElementByteCount*NUMBER_OF_ISOC_Q_ELEMENTS);
	#endif
	g_pbUsbIsocQueueBuffer=(BYTE *)ext_mem_malloc(g_dwQueueBufferSize);
	IsocWholeQueueBufferReset();
	IntEnable();

}

void IsocWholeQueueBufferRelease()
{

	if (g_pbUsbIsocQueueBuffer&&g_dwQueueBufferSize)
	{
		IntDisable();
		ext_mem_free(g_pbUsbIsocQueueBuffer);
		g_pbUsbIsocQueueBuffer=NULL;
		g_dwQueueBufferSize=0;
		#if ISOC_EVERY_QUEUE_SIZE_FIXED
		dwTotalQueNum=0;
		dwOneQueSize=0;
		#endif
	    IntEnable();
	}
    mpDebugPrint("%s: total=%d", __FUNCTION__,mem_get_free_space_total());

}

#if !ISOC_EVERY_QUEUE_SIZE_FIXED
#if QUEUE_DEBUG_ENABLE
static void PrintWholeBufferStatus()
{
	ST_HEAP_MEM  *mem;

	mpDebugPrint("%s:",__FUNCTION__);
	mpDebugPrint("-wQueueBufferTag=0x%x QueueBufStart=0x%x  ,QueueFreeBufStart=0x%x,QueueHeadBuffer=0x%x  QueueTailBuffer=0x%x",wQueueBufferTag,QueueBufStart,QueueFreeBufStart,QueueHeadBuffer,QueueTailBuffer);
    for (mem = QueueBufStart; (mem != NULL); mem = (ST_HEAP_MEM *) mem->next)
    {
		mpDebugPrint("0x%x:tag=0x%x used=0x%x ,Flag=0x%x  size=0x%x",(DWORD)mem,mem->tag,mem->used,mem->TaskId,mem->size);
    }
	__asm("break 300");

}
#endif

static void IsocOneQueueBufferRelease(void *rmem)
{
	DWORD i;
	BYTE bWaitRelease=0;
    ST_HEAP_MEM *mem,*nmem;

    //rmem = (void *)(((DWORD) rmem) & ~BIT29);
    mem = (ST_HEAP_MEM *) (((DWORD) rmem) & ~BIT29);
#if QUEUE_DEBUG_ENABLE
	UartOutText("-R-");
	UartOutValue((DWORD)mem, 8);
#endif

	if (g_swWaitWrite)
	{
		for  (i=0;i<WAIT_RELEASE_NUMBER;i++)
		{
				if (!g_dwWaitRelease[i])
				{
					IntDisable();
					g_dwWaitRelease[i]=(DWORD)mem;
					mem->TaskId=1;
					/* Data head move to next data */
					nmem =  (ST_HEAP_MEM *)mem->next;
					if (nmem == NULL)
						QueueHeadBuffer=QueueBufStart;
					else
					{
						//if (mem->used)
							QueueHeadBuffer=nmem;
					}
				    IntEnable();
					return;
				}
		}
		mpDebugPrint("Wait Release over!!");
	}
#if QUEUE_DEBUG_ENABLE
	UartOutValue((DWORD)mem, 8);
	//UartOutValue((DWORD)QueueFreeBufStart, 8);
#endif
    if(mem->used == 0)
    {
        MP_ALERT("--E-- %s: Wrong address to be free !!!", __FUNCTION__);
        return;
    }

    if (mem->tag != wQueueBufferTag)
    {
        MP_ALERT("--E-- %s: Wrong memory tag !!!", __FUNCTION__);
        return;
    }

	if (((DWORD) mem < (DWORD) QueueBufStart) ||	((DWORD) mem >= (DWORD) QueueBufStart+g_dwQueueBufferSize))
	{
		MP_ALERT("--E-- %s: Wrong MAIN_HEAP address to be free !!!", __FUNCTION__);
		return;
	}


    // check chain link
    nmem = (ST_HEAP_MEM *)mem->next;
    if ((nmem != NULL) && nmem->prev != (DWORD) mem)
    {
        MP_ALERT("--E-- %s: Next link error !!!%p %p", __FUNCTION__,nmem,nmem->prev);
        return;
    }

    nmem = (ST_HEAP_MEM *) mem->prev;
    if((nmem != NULL) && nmem->next != (DWORD) mem)
    {
        MP_ALERT("--E-- %s: Prev link error !!!%p %p", __FUNCTION__,nmem,nmem->next);
#if QUEUE_DEBUG_ENABLE
	    for (mem = QueueBufStart; (mem != NULL); mem = (ST_HEAP_MEM *) mem->next)
	    {
			mpDebugPrint("%p:%p %p",(DWORD)mem,mem->prev,mem->next);
	    }
		__asm("break 300");
#endif
		return;
    }

	IntDisable();
	if (mem->TaskId==1)
		bWaitRelease=1;
	mem->used = 0;
	mem->TaskId = 0;

    // merge memory space
    /* plug hole forward - if next mem is empty, merge together */
    nmem = (ST_HEAP_MEM *) mem->next;
#if QUEUE_DEBUG_ENABLE
	UartOutValue((DWORD)nmem, 8);
#endif

    if ((nmem != NULL) && (mem != nmem) && (nmem->used == 0))
    {
        mem->next = nmem->next;
        mem->size += nmem->size + SIZEOF_STRUCT_MEM;
        ((ST_HEAP_MEM *)nmem->next)->prev = (DWORD)mem;
		if (QueueFreeBufStart==nmem)
		{
			QueueFreeBufStart=mem;
#if QUEUE_DEBUG_ENABLE
			UartOutText(" n1 ");
#endif
		}
		if (QueueHeadBuffer==nmem)
		{
			QueueHeadBuffer=mem;
#if QUEUE_DEBUG_ENABLE
			UartOutText(" n2 ");
#endif
		}
#if QUEUE_DEBUG_ENABLE
		mpDebugPrintN(" %p %p ",(DWORD)mem,mem->size);
#endif
    }

	/* Data head move to next data */
	if (!bWaitRelease)
	{
		nmem =  (ST_HEAP_MEM *)mem->next;
#if QUEUE_DEBUG_ENABLE
		mpDebugPrintN(" h %p %p ",(DWORD)nmem,nmem->size);
#endif
		if (nmem == NULL)
		{
			//if (QueueBufStart->used)
				QueueHeadBuffer=QueueBufStart;
		}
		else
		{
			//if (nmem->used)
				QueueHeadBuffer=nmem;
		}
	}

    /* plug hole backward - if previous mem is empty, merge together */
    nmem = (ST_HEAP_MEM *) mem->prev;
#if QUEUE_DEBUG_ENABLE
	UartOutValue((DWORD)nmem, 8);
#endif

    if ((nmem != NULL) && (nmem != mem) && (nmem->used == 0))
    {
        nmem->next = mem->next;
        nmem->size += mem->size + SIZEOF_STRUCT_MEM;
        ((ST_HEAP_MEM *) mem->next)->prev = (DWORD) nmem;
		if (QueueFreeBufStart==mem)
		{
			QueueFreeBufStart=nmem;
#if QUEUE_DEBUG_ENABLE
			UartOutText(" p1 ");
#endif
		}
		if (QueueHeadBuffer==mem)
		{
			QueueHeadBuffer=nmem;
#if QUEUE_DEBUG_ENABLE
			UartOutText(" p2 ");
#endif
		}
#if QUEUE_DEBUG_ENABLE
		mpDebugPrintN(" %p %p ",(DWORD)nmem,nmem->size);
#endif
    }
	g_swQueueUsedNum--;
#if QUEUE_DEBUG_ENABLE
	mpDebugPrintN(" F %p ",(DWORD)QueueFreeBufStart);
#endif
    IntEnable();
}

static void *IsocOneQueueBufferAlloc(DWORD size )
{
	DWORD i;
	ST_HEAP_MEM  *tmpmem;

	for  (i=0;i<WAIT_RELEASE_NUMBER;i++)
	{
		if (g_dwWaitRelease[i])
		{
			IsocOneQueueBufferRelease((void *)g_dwWaitRelease[i]);
			g_dwWaitRelease[i]=0;
		}
	}
#if QUEUE_DEBUG_ENABLE
	mpDebugPrint("");
	UartOutText("-A-");
#endif
	if (size == 0)
	{
		MP_ALERT("--E-- %s size = 0", __FUNCTION__);
		return NULL;
	}
	if (!g_dwQueueBufferSize)
		IsocWholeQueueBufferAlloc();

	IntDisable();
	if ((DWORD)QueueFreeBufStart+size > (DWORD)QueueBufStart+g_dwQueueBufferSize)
		QueueFreeBufStart=QueueBufStart;
	tmpmem = QueueFreeBufStart;
	if (tmpmem->used || tmpmem->size<size)
	{
#if QUEUE_DEBUG_ENABLE
		mpDebugPrint("-!-0x%x  ,%d %d<%d Total use:%d ",tmpmem,tmpmem->used,tmpmem->size,size,g_swQueueUsedNum);
		//PrintWholeBufferStatus();
#endif
	    IntEnable();
		return NULL;
	}

	tmpmem->used = 1;
	tmpmem->TaskId = QueueBufferFlag-1;
	QueueTailBuffer=tmpmem;

#if QUEUE_DEBUG_ENABLE
 	mpDebugPrintN(" %p %p %p ",(DWORD)tmpmem,tmpmem->size,QueueBufStart->size);
#endif
	g_swWaitWrite++;

	IntEnable();
	return (void *)((DWORD)tmpmem + SIZEOF_STRUCT_MEM);
}
/*
 void DropOneQueueBuffer()
{
	IntDisable();
	QueueTailBuffer->used = 0;
	QueueTailBuffer->TaskId = 0;
	g_swWaitWrite--;
	IntEnable();
}
*/
static void *IsocOneQueueBufferWriteEnd(DWORD size)
{
	DWORD i;
	//ST_HEAP_MEM  *tmpmem;

#if QUEUE_DEBUG_ENABLE
	UartOutText("-W-");
#endif
	if (size == 0)
	{
		MP_ALERT("--E-- %s size = 0", __FUNCTION__);
		return ;
	}
#if 0
	tmpmem = QueueTailBuffer;//(ST_HEAP_MEM  *)(pdwQueueBuffer-SIZEOF_STRUCT_MEM);
	if (tmpmem->tag!=wQueueBufferTag || tmpmem->TaskId<QueueBufferFlag)
	{
		mpDebugPrint("--E--buffer error,reset");
		IsocWholeQueueBufferReset();
		return ;
	}
#endif
	IntDisable();
	QueueTailBuffer->used = 1;
	QueueTailBuffer->tag = wQueueBufferTag;
	QueueTailBuffer->TaskId = QueueBufferFlag;
#if QUEUE_DEBUG_ENABLE
	UartOutValue((DWORD)QueueTailBuffer, 8);
#endif
	// Create new entry for rest of space more than MIN_SPACE, after alloc size from mem
	if (QueueTailBuffer->size > (size + SIZEOF_STRUCT_MEM + MIN_SPACE))
	{
		QueueFreeBufStart = (ST_HEAP_MEM *)((DWORD)QueueTailBuffer + SIZEOF_STRUCT_MEM + size);
		// Insert to list between mem and mem->next
		QueueFreeBufStart->prev = (DWORD)QueueTailBuffer;
		QueueFreeBufStart->next = QueueTailBuffer->next;
		QueueFreeBufStart->size = QueueTailBuffer->size - (size + SIZEOF_STRUCT_MEM);
		QueueFreeBufStart->used = 0;

		QueueTailBuffer->next = (DWORD)QueueFreeBufStart;
		QueueTailBuffer->size = size;

		if (QueueFreeBufStart->next != 0)
		{
			((ST_HEAP_MEM *) (QueueFreeBufStart->next))->prev = (DWORD) QueueFreeBufStart;
		}
#if QUEUE_DEBUG_ENABLE
		mpDebugPrintN(" %p %p ",(DWORD)QueueFreeBufStart,QueueFreeBufStart->size);
#endif

	}
	else if (QueueTailBuffer->next) //(((DWORD)QueueTailBuffer+size + SIZEOF_STRUCT_MEM+MIN_SPACE) <((DWORD)QueueBufStart+g_dwQueueBufferSize)))
	{
		QueueFreeBufStart=(ST_HEAP_MEM *)QueueTailBuffer->next;
	}
	else
	{
		QueueFreeBufStart=QueueBufStart;
	}
	if (!QueueHeadBuffer->used)
		QueueHeadBuffer=QueueTailBuffer;
	g_swQueueUsedNum++;
	g_swWaitWrite--;
	for  (i=0;i<WAIT_RELEASE_NUMBER;i++)
	{
		if (g_dwWaitRelease[i])
		{
			IsocOneQueueBufferRelease((void *)g_dwWaitRelease[i]);
			g_dwWaitRelease[i]=0;
		}
	}
	IntEnable();
}
#endif

/////////////////
/*
PUSBH_ISOC_BUFFER UsbOtgHostIsocInDataDequeueGo(WHICH_OTG eWhichOtg) // get queue data head
{
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;

    if (QueueHeadBuffer->used && QueueHeadBuffer->size)
    {
		pIsocBuff = (PUSBH_ISOC_BUFFER)UsbOtgHostIsocInGetQueueElement(0, eWhichOtg);
		pIsocBuff->dwLength=QueueHeadBuffer->size;
		pIsocBuff->pbDataBuffer=(BYTE *)QueueHeadBuffer+SIZEOF_STRUCT_MEM;
    }
    
    return pIsocBuff;
}
*/
void GetQueueDataHead(BYTE **pbData,DWORD *dwSize) // get head used queue
{
	IntDisable();
#if ISOC_EVERY_QUEUE_SIZE_FIXED
    if (head == tail) // null 
    {
			*dwSize=0;
			*pbData=NULL;
    }
    else
    {
			*dwSize=st_dwQueueDataSize[head];
			*pbData=g_pbUsbIsocQueueBuffer+ dwOneQueSize*head;
    }
#else
	if (g_dwWaitRelease[WAIT_RELEASE_NUMBER-1])
	{
			*dwSize=0;
			*pbData=NULL;
#if QUEUE_DEBUG_ENABLE
			UartOutText("-gv-");
#endif
	}
    else if (QueueHeadBuffer->used)
    {
		if (QueueHeadBuffer->TaskId == QueueBufferFlag)
		{
			*dwSize=QueueHeadBuffer->size;
			*pbData=(BYTE *)QueueHeadBuffer+SIZEOF_STRUCT_MEM;
#if QUEUE_DEBUG_ENABLE
			UartOutText("-D-");
			UartOutValue((DWORD)QueueHeadBuffer, 8);
#endif
		}
		else //QueueHeadBuffer->TaskId==1
		{
			*dwSize=0;
			*pbData=NULL;
#if QUEUE_DEBUG_ENABLE
			UartOutText("-g-");
			UartOutValue((DWORD)QueueHeadBuffer, 8);
#endif
		}
    }
	else
    {
		*dwSize=0;
		*pbData=NULL;
#if QUEUE_DEBUG_ENABLE
		UartOutText("-G-");
#endif
    }
#endif
	IntEnable();
}

void UsbOtgHostIsocInDataDequeueReady(WHICH_OTG eWhichOtg) // release one head used queue
{
#if ISOC_EVERY_QUEUE_SIZE_FIXED
    if (head >= (dwTotalQueNum-1))
    {
        head = 0;
    }
    else
    {
        head = head+1;
    }
#else
	IsocOneQueueBufferRelease((BYTE *)QueueHeadBuffer);
#endif
#if 0//USBCAM_DEBUG_ISR_LOST
	g_dwQueueUsed--;
#endif
}
SDWORD UsbOtgHostIsocInDataEnqueueDataBuffer(BYTE **hData, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg) // get first free queue
{
#if ISOC_EVERY_QUEUE_SIZE_FIXED
    DWORD queue_len = 0;// be used

    if (tail >= head)
    {
        queue_len = tail - head;
    }
    else
    {
        queue_len = tail + (dwTotalQueNum - head);
    }
    
    if (queue_len >= dwTotalQueNum)
    {
		*hData = NULL;
#if 0//USBCAM_DEBUG_ISR_LOST
		mpDebugPrint("used=%d head=%d tail=%d",g_dwQueueUsed,head,tail);
#endif
		return USBOTG_ISOC_QUEUE_FULL;
    }
    else
    {
		#if (USBCAM_IN_ENABLE==2)
		*hData = (BYTE *)((ST_IMGWIN  *)Idu_GetNextWin()->pdwStart);
		#else
		*hData = g_pbUsbIsocQueueBuffer+ dwOneQueSize*tail;
		#endif
    }
		
#else
	*hData = IsocOneQueueBufferAlloc(GetQueueElementByteCount(WEBCAM_USB_PORT));
	if (*hData == NULL)
	{
		EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
		return USBOTG_ISOC_QUEUE_FULL;
	}
#endif

#if 0//USBCAM_DEBUG_ISR_LOST
	g_dwQueueUsed++;
#endif

	return USBOTG_NO_ERROR;
}

void UsbOtgHostIsocInDataEnqueueDataLength(DWORD dwDataLength, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg) // set tail mark be used
{
#if ISOC_EVERY_QUEUE_SIZE_FIXED
	st_dwQueueDataSize[tail]=dwDataLength;
    if (tail == (dwTotalQueNum-1))
    {
        if (head != 0)
            tail = 0;
    }
    else
    {
        if (head != (tail + 1))
            tail = tail + 1;
    }

#else
	IsocOneQueueBufferWriteEnd(dwDataLength);
#endif
}


#endif


#elif (USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG)
// define state value for WEB_CAM_INFO
#define WEB_CAM_STATE_NOT_EXIST	0
#define WEB_CAM_STATE_IDLE		1
#define WEB_CAM_STATE_PLAY		2

// define webcam status
#define WEB_CAM_READY		0
#define WEB_CAM_BUSY		1
#define WEB_CAM_IDLE		2
#define WEB_CAM_NOT_EXIST	3
#define WEB_CAM_TIMEOUT		4

#define WEB_CAM_BUF_SIZE	0x50000 // as same as UsbMjpgBufferSize (Side Monitor)
#define WEB_CAM_DIMENSION	640*480 // 320*240

#define WEB_CAM_AUDIO_BUF_NUM_MAX	4


/*
// Structure declarations
*/

/* Define audio buffer structure */
typedef struct
{
	// for video
	BYTE 	*pFramBuffer[2];
	DWORD 	state;
	BYTE 	CurFramID;				// The fram displayed on panel
	BYTE    BufferReady[2];

	// for audio
	BYTE 	*pAudioBuffer[WEB_CAM_AUDIO_BUF_NUM_MAX];
	BYTE	AudioBufferNumber;
	DWORD 	AudioState;
	BYTE 	sample_size;
	BYTE 	channels;
	WORD 	sample_rate;
	BYTE 	CurAudioIndex;

} WEB_CAM_INFO;


#if CHIP_VER == CHIP_VER_620
typedef struct
{
	DWORD	BufferBeginAdd;
	DWORD	BufferSize;
	BYTE	BufferNumber;

} WEB_CAM_AUDIO_BUFFER;
#endif

static ST_MCARD_MAIL sWebCamMail;
static BYTE bWebCamMailId;
static WEB_CAM_INFO WebCamInfo;
static BYTE gbBoxId;  // check WebCam or SideMonitor


BYTE *WebCamGetCurFram()
{
	if(WebCamInfo.CurFramID == 0)
		return WebCamInfo.pFramBuffer[0];
	else
		return WebCamInfo.pFramBuffer[1];
}



BYTE *WebCamGetNextFram()
{
	if(WebCamInfo.CurFramID == 0)
		return WebCamInfo.pFramBuffer[1];
	else
		return WebCamInfo.pFramBuffer[0];
}

BYTE NextFramReady()
{
	if(WebCamInfo.CurFramID == 0)
		return WebCamInfo.BufferReady[1];
	else
		return WebCamInfo.BufferReady[0];
}


BYTE CurFramReady()
{
	if(WebCamInfo.CurFramID == 0)
		return WebCamInfo.BufferReady[0];
	else
		return WebCamInfo.BufferReady[1];
}


int WebCamFramSwap()
{
	if(WebCamInfo.CurFramID == 0)
		WebCamInfo.CurFramID = 1;
	else
		WebCamInfo.CurFramID = 0;
}


#if 1
// The function will access WebCam Plug IN / OUT event
int WebCamEventProcess(DWORD dwEvent)
{
    WORD wDevId;

    if(dwEvent == EVENT_WEB_CAM_IN)
    {
        if (Api_UsbdCheckIfConnectedSideMonitor(Api_UsbdGetWhichOtgConnectedSideMonitor()))
        {
            if(gbBoxId == NULL_OBJECT)
            {   // Side Monitor Init
                gbBoxId = USBOTG_DEVICE_EXTERN_MAIL_ID;
                WebCamInfo.state = WEB_CAM_STATE_IDLE;                    
                MP_ALERT("EVENT_SIDE_MONITOR_IN");
            }
        }
    }
    else if(dwEvent == EVENT_WEB_CAM_OUT)
    {
        if( gbBoxId == USBOTG_DEVICE_EXTERN_MAIL_ID)
        {
            MP_ALERT("EVENT_SIDE_MONITOR_OUT");
        }
    }

    return PASS;
}
#else
// The function will access WebCam Plug IN / OUT event
int WebCamEventProcess(DWORD dwEvent)
{
        WORD wDevId;

	if(dwEvent == EVENT_WEB_CAM_IN)
	{

            #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
            // Device First
            if (Api_UsbdCheckIfConnectedSideMonitor(Api_UsbdGetWhichOtgConnectedSideMonitor()))
            {
                if(gbBoxId == NULL_OBJECT)
                {   // Side Monitor Init
                    gbBoxId = USBOTG_DEVICE_EXTERN_MAIL_ID;
                    WebCamInfo.state = WEB_CAM_STATE_IDLE;                    
                    MP_ALERT("EVENT_SIDE_MONITOR_IN");
                }
            }
            #endif            

            #if USBOTG_WEB_CAM
            // Host Second
            if(Api_UsbhCheckIfConnectedWebCam(WEBCAM_USB_PORT)
                #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
                && gbBoxId != USBOTG_DEVICE_EXTERN_MAIL_ID 
                #endif
                )  
            {
                if(gbBoxId == NULL_OBJECT)
                {   // WebCame Init
                    gbBoxId = UsbOtgHostMailBoxIdGet(WEBCAM_USB_PORT);
            /* // JL, 11212011   
                    wDevId = WEBCAM_USB_PORT ==  USBOTG0 ? DEV_USB_HOST_ID1 : DEV_USBOTG1_HOST_ID1 ;
        		Api_UsbhStorageDeviceInit(wDevId);
        		if (!SystemCardPresentCheck(wDevId))
        			Api_UsbhStorageDeviceRemove(wDevId);
*/
                    WebCamInfo.state = WEB_CAM_STATE_IDLE;
                    MP_ALERT("EVENT_WEB_CAM_IN");    
                }     
            }
            #endif            
	}
	else if(dwEvent == EVENT_WEB_CAM_OUT)
	{
            if (WebCamInfo.state == WEB_CAM_STATE_PLAY)
    	        WebCamStopPlay();

            #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
            if( gbBoxId == USBOTG_DEVICE_EXTERN_MAIL_ID)
            {
                MP_ALERT("EVENT_SIDE_MONITOR_OUT");
            }
            #endif

            #if USBOTG_WEB_CAM
            if(gbBoxId == UsbOtgHostMailBoxIdGet(WEBCAM_USB_PORT))	
            {
                MP_ALERT("EVENT_WEB_CAM_OUT");
                Api_UsbhStorageDeviceRemove(WEBCAM_USB_PORT ==  USBOTG0 ? DEV_USB_HOST_ID1 : DEV_USBOTG1_HOST_ID1);
            }
            #endif
            WebCamInfo.state = WEB_CAM_STATE_NOT_EXIST;
            gbBoxId = NULL_OBJECT;
	}

	return PASS;

}
#endif


// Send mail to USB Host to start webcam image catch
SWORD _WebCamStart()
{
	sWebCamMail.wMCardId = DEV_USB_HOST_ID1;
	sWebCamMail.wCmd = WEB_CAM_START_CMD;

        if(gbBoxId != NULL_OBJECT)
        {
            MailboxSend(gbBoxId, (BYTE *) (&sWebCamMail.dwBlockAddr), sizeof(ST_MCARD_MAIL), &bWebCamMailId);
            MailTrack(bWebCamMailId);
            return PASS;
        }
        else
        {
            MP_ALERT("--E-- %s Box ID is NULL", __FUNCTION__);
            return FAIL;
        }   
}


// Send mail to USB Host to stop webcam image catch
SWORD WebCamStop()
{	
	sWebCamMail.wMCardId = DEV_USB_HOST_ID1;
	sWebCamMail.wCmd = WEB_CAM_STOP_CMD;

        if(gbBoxId != NULL_OBJECT)
        {
            MailboxSend(gbBoxId, (BYTE *) (&sWebCamMail.dwBlockAddr), sizeof(ST_MCARD_MAIL), &bWebCamMailId);
            MailTrack(bWebCamMailId);
            return PASS;
        }
        else
        {
            MP_ALERT("--E-- %s Box ID is NULL", __FUNCTION__);
            return FAIL;
        }
}



// Send mail to USB Host to get image data
SWORD WebCamGetFramData()
{
	sWebCamMail.wMCardId = DEV_USB_HOST_ID1;
	sWebCamMail.wCmd = WEB_CAM_GET_VFRAM_CMD;
	sWebCamMail.dwBuffer = (DWORD)WebCamGetNextFram();

    
        if(gbBoxId != NULL_OBJECT)
        {
            MailboxSend(gbBoxId, (BYTE *) (&sWebCamMail.dwBlockAddr), sizeof(ST_MCARD_MAIL), &bWebCamMailId);
            MailTrack(bWebCamMailId);
            return PASS;
        }
        else
        {
            MP_ALERT("--E-- %s Box ID is NULL", __FUNCTION__);
            return FAIL;
        }
}

int WebCamTrackFramData()
{
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;
	int ret;

	MailTrack(bWebCamMailId);

        ret = GetVideoStatus();

	//if(ret == OS_STATUS_OK)
	{
		if(WebCamInfo.CurFramID == 0)
			WebCamInfo.BufferReady[1] = 1;
		else
			WebCamInfo.BufferReady[0] = 1;
	}

	return ret;
}


int WebCamDecodeFramData()
{
	BYTE *pTarget;
	DWORD FreeSpace;
	ST_IMGWIN * ImgWin, *TrgWin;
	WORD wWidth = 0, wHeight = 0;	
	DWORD	dwWebCamDim = WEB_CAM_DIMENSION;

	if(CurFramReady() == 0)
		return FAIL;
#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
	if(gbBoxId == USBOTG_DEVICE_EXTERN_MAIL_ID)
	{
		SystemPanelSizeGet(&wWidth, &wHeight);
		dwWebCamDim = wWidth * wHeight;
	}
#endif
	FreeSpace = ext_mem_get_free_space();
	if(FreeSpace < dwWebCamDim*2)
		return FAIL;

        pTarget = (BYTE *)ext_mem_malloc(dwWebCamDim * 2);

	Img_Jpeg2ImgBuf(WebCamGetCurFram(), pTarget, IMG_DECODE_PHOTO, WEB_CAM_BUF_SIZE, ALIGN_16(dwWebCamDim * 2)); // ALIGN_16 for 420

	WebCamInfo.BufferReady[WebCamInfo.CurFramID] = 0;
	ImgWin = Idu_GetNextWin();

	TrgWin = (ST_IMGWIN*)ImageGetDecodeWin();
	TrgWin->pdwStart = (DWORD*)pTarget;
	TrgWin->wWidth = Jpg_GetTargetWidth();
	TrgWin->wHeight = Jpg_GetTargetHeight();
	TrgWin->dwOffset = (TrgWin->wWidth << 1);
#if IMAGE_ROTATE_MEMO_ENABLE
	ImageScaleFromJPEGTarget(ImgWin, pTarget, 0);
#else
	ImageScaleFromJPEGTarget(ImgWin, pTarget);
#endif
	Idu_ChgWin(ImgWin);

	ext_mem_free(pTarget);

	return PASS;

}


int WebCamStartPlay()
{
	BYTE *ptr;
	DWORD FreeSpace;
	
	#if USBOTG_WEB_CAM
	if (!SystemCardPresentCheck(DEV_USB_HOST_ID1))		// web cam init fail
		return FAIL;
	#endif

	xpgStopAllAction ();

    FreeSpace = ext_mem_get_free_space();
    if(FreeSpace < WEB_CAM_BUF_SIZE*2)
    {
        MP_ALERT("--E-- %s Ext Memory is not enough", __FUNCTION__);
        return FAIL;    
    }

	if(WebCamInfo.pFramBuffer[0] == NULL)
		ptr = (BYTE*)ext_mem_malloc(WEB_CAM_BUF_SIZE*2);
	else
		ptr = WebCamInfo.pFramBuffer[0];

	if(ptr == NULL)
		return FAIL;

	if( WebCamInfo.state == WEB_CAM_STATE_PLAY )
		return PASS;

	WebCamInfo.pFramBuffer[0] = ptr;
	WebCamInfo.pFramBuffer[1] = (BYTE *)((DWORD)ptr + WEB_CAM_BUF_SIZE);
	WebCamInfo.BufferReady[0] = 0;
	WebCamInfo.BufferReady[1] = 0;
	WebCamInfo.CurFramID = 1;
	WebCamInfo.state = WEB_CAM_STATE_PLAY;

#if CHIP_VER == CHIP_VER_620
	WebCamInfo.AudioState = WEB_CAM_STATE_IDLE;

	TaskStartup(WEB_CAM_AUDIO_TASK);
#endif

	TaskStartup(WEB_CAM_VIDEO_TASK);

	return PASS;
}


int WebCamStopPlay()
{
	WebCamStop();
	TaskTerminate(WEB_CAM_VIDEO_TASK);
#if CHIP_VER == CHIP_VER_620
	TaskTerminate(WEB_CAM_AUDIO_TASK);
#endif

	if(WebCamInfo.pFramBuffer[0])
		ext_mem_free(WebCamInfo.pFramBuffer[0]);

	WebCamInfo.pFramBuffer[0] = NULL;
	WebCamInfo.pFramBuffer[1] = NULL;
	WebCamInfo.state = WEB_CAM_STATE_IDLE;
#if CHIP_VER == CHIP_VER_620
	WebCamInfo.AudioState = WEB_CAM_STATE_IDLE;
#endif
	g_psSystemConfig->dwCurrentOpMode = OP_IDLE_MODE;
	RenewAllDrv();
	xpgClearCatch();
	FileBrowserResetFileList();

	return PASS;
}


int WebCamPlayTask()
{
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;
    DWORD frameCounter = 0, startTime, countTime;
    int ret;
    BYTE bStr[256];

    ret = GetVideoStatus();  
    if(ret == WEB_CAM_NOT_EXIST)
        TaskTerminate(WEB_CAM_VIDEO_TASK);

    WebCamStart();

    //for(ret=0; ret<0x1000; ret++) // wait 1st for frame data
    while(GetVideoStatus() != WEB_CAM_READY)
        TaskYield();

    // get first fram data to buffer 0
    WebCamGetFramData();

    WebCamDecodeFramData();

    WebCamTrackFramData();
    WebCamFramSwap();

    for(ret=0; ret<0x1000; ret++)   // wait 2nd for fram data
        TaskYield();

    startTime = GetSysTime();
    while(1)
    {
        while(GetVideoStatus() != WEB_CAM_READY)
            TaskYield();

        WebCamGetFramData();
        WebCamDecodeFramData();
        frameCounter++;
        countTime = SystemGetElapsedTime(startTime);
        if (countTime > 1000)
        {
            startTime = GetSysTime();
            mpDebugPrint("%i f/s", frameCounter/(countTime/1000));
            
            Idu_OsdPaintArea(400, 400, 100, 50,0);
            sprintf(bStr,"%i f/s", frameCounter/(countTime/1000));
            Idu_OsdPutStr(Idu_GetOsdWin(), bStr, 400, 400, 6);
            
            frameCounter = 0;
        }

        WebCamTrackFramData();
        WebCamFramSwap();
        TaskYield();
    }

    return PASS;
}


#if CHIP_VER == CHIP_VER_620

BYTE *GetAudioNextReadBuffer()
{
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;

	WebCamInfo.CurAudioIndex++;
	WebCamInfo.CurAudioIndex %= WebCamInfo.AudioBufferNumber;

	if( !WebCamGetAudioStatus( WebCamInfo.CurAudioIndex, eWhichOtg ) )
	{
		WebCamInfo.CurAudioIndex += WebCamInfo.AudioBufferNumber - 1;
		WebCamInfo.CurAudioIndex %= WebCamInfo.AudioBufferNumber;
		return NULL;
	}

	return WebCamInfo.pAudioBuffer[WebCamInfo.CurAudioIndex];
}

void WebCamAudioIsr()
{
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;
	DWORD dwStreamLength;

	g_psDmaAiu->Control &= ~0x00010000;	/* clear buffer end interrupt */

	WebCamClearAudioStatus( WebCamInfo.CurAudioIndex, eWhichOtg );

	if( GetAudioNextReadBuffer() && ( dwStreamLength = WebCamGetAudioStreamLength( WebCamInfo.CurAudioIndex, eWhichOtg ) ) ) 
	{
		DmaIntEna(IM_AIUDM);
		g_psDmaAiu->StartA = (DWORD)WebCamInfo.pAudioBuffer[WebCamInfo.CurAudioIndex];
		g_psDmaAiu->EndA = g_psDmaAiu->StartA + dwStreamLength - 1;
		if( WebCamInfo.AudioState == WEB_CAM_STATE_IDLE )
			MX6xx_AudioResume();
		g_psDmaAiu->Control |= 0x01000001;
		WebCamInfo.AudioState = WEB_CAM_STATE_PLAY;
	}
	else
	{
		WebCamInfo.AudioState = WEB_CAM_STATE_IDLE;
		MX6xx_AudioPause();
	}
}

int WebCamAudioTask()
{
	WEB_CAM_AUDIO_BUFFER WebCamAudioBuffer;
	DWORD dwStreamLength;
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;
	BYTE i;

	if (MX6xx_AudioOpen(0) < 0)
		return FAIL;

	WebCamInfo.channels = 1;
	WebCamInfo.sample_size = WebCamGetAudioSampleSize(eWhichOtg);
	WebCamInfo.sample_rate = WebCamGetAudioFreqRate(eWhichOtg);

	MX6xx_AudioConfig(WebCamInfo.sample_rate, WebCamInfo.channels, WebCamInfo.sample_size);

	AiuDmaRegCallBackFunc(WebCamAudioIsr);

	g_psDmaAiu->Control &= ~0x0000000A;

	while( !WebCamGetAudioBuffer(&WebCamAudioBuffer, eWhichOtg) )
		TaskYield();

	if( WebCamAudioBuffer.BufferNumber > WEB_CAM_AUDIO_BUF_NUM_MAX )
		return FAIL;

	WebCamInfo.AudioBufferNumber = WebCamAudioBuffer.BufferNumber;

	for( i=0; i<WebCamInfo.AudioBufferNumber; i++ )
	{
		WebCamInfo.pAudioBuffer[i] = (BYTE*)WebCamAudioBuffer.BufferBeginAdd + WebCamAudioBuffer.BufferSize*i;
	}
	WebCamInfo.CurAudioIndex = WebCamInfo.AudioBufferNumber - 1;

	while(1)
	{
		if( WebCamInfo.AudioState == WEB_CAM_STATE_IDLE && GetAudioNextReadBuffer() )
		{
			dwStreamLength = WebCamGetAudioStreamLength( WebCamInfo.CurAudioIndex, eWhichOtg );

			if( dwStreamLength )
			{
				DmaIntEna(IM_AIUDM);
				g_psDmaAiu->StartA = (DWORD)WebCamInfo.pAudioBuffer[WebCamInfo.CurAudioIndex];
				g_psDmaAiu->EndA = g_psDmaAiu->StartA + dwStreamLength - 1;
				MX6xx_AudioResume();
				g_psDmaAiu->Control |= 0x01000001;
				WebCamInfo.AudioState = WEB_CAM_STATE_PLAY;
			}
		}

		TaskYield();
	}
	return PASS;
}
#endif

int BabyMonitorInit()
{
#if CHIP_VER == CHIP_VER_620
	TaskCreate (WEB_CAM_AUDIO_TASK, WebCamAudioTask, CONTROL_PRIORITY, 0x4000);
#endif
	TaskCreate (WEB_CAM_VIDEO_TASK, WebCamPlayTask, CONTROL_PRIORITY, 0x4000);

	WebCamInfo.pFramBuffer[0] = NULL;
	WebCamInfo.pFramBuffer[1] = NULL;
	WebCamInfo.BufferReady[0] = 0;
	WebCamInfo.BufferReady[1] = 0;
	WebCamInfo.CurFramID = 1;

	WebCamInfo.state = WEB_CAM_STATE_NOT_EXIST;

	gbBoxId = NULL_OBJECT;

	return PASS;
}

extern BYTE g_boNeedRepaint;
void xpgCb_WebCamOnOff()
{
	if(WebCamInfo.state == WEB_CAM_STATE_PLAY)
		WebCamStopPlay();
	else
		WebCamStartPlay();

	g_boNeedRepaint = FALSE;
}


DWORD IsWebCamPlay()
{
	if(WebCamInfo.state == WEB_CAM_STATE_PLAY)
		return 1;
	else
		return 0;

}

DWORD IsWebCamExist()
{
	if(WebCamInfo.state == WEB_CAM_STATE_NOT_EXIST)
		return 0;
	else
		return 1;
}

int GetVideoStatus(void)
{
    if(gbBoxId == NULL_OBJECT)
        return WEB_CAM_NOT_EXIST;
    #if 0 //USBOTG_WEB_CAM    
    else if(gbBoxId == UsbOtgHostMailBoxIdGet(WEBCAM_USB_PORT))           
	return WebCamGetVideoStatus(WEBCAM_USB_PORT);
    #endif

    #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
    else if(gbBoxId == USBOTG_DEVICE_EXTERN_MAIL_ID)
        return SideMonitorGetVideoStatus();
    #endif    

}

#endif

