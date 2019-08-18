/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1
#include <stdio.h>
#include <string.h>
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgFunc.h"
#include "taskid.h"
#include "ui.h"
#include "mpapi.h"
#include "filebrowser.h"
#include "os.h"
#include "ui_timer.h"
#include "setup.h"
#include "ispfunc.h"
#if RTC_ENABLE
#include "../../ui/include/rtc.h"
extern ST_IMGWIN m_stClockWin[3];
#endif
#include "mpapi.h"
#include "Camcorder_func.h"
#include "xpgCamFunc.h"
#include "record.h"
#include "Ui_FileSystem.h"
#include "MiniDV_stateMachine.h"

#if (SENSOR_ENABLE == ENABLE)
#define MIN_FREE_DISK_SPACE1             200

int CameraInit = 1;
E_MINI_DV_MODE g_MinidvMode;
//E_CAMCORDER_TIMING recordTiming_i = CAMCORDER_RESOLUTION_VGA;
//E_CAMERA_RESOLUTION resolution_i = CAMERA_RESOLUTION_640x480;

#if CAMCODERRECORD_OLDMODE
DWORD GetRecordLenth(void)
{
	DWORD  dwTime=3*60*1000;
#if 0	
	switch (g_psSetupMenu->RecordMovieLenth)
	{
		case 1:
			dwTime=3*60*1000;
			break;

		case 2:
			dwTime=30*60*1000;
			break;

		case 3:
			dwTime=60*60*1000;
			break;

		case 0:
		default:
			dwTime=0;
			break;
	}
#endif
	return dwTime;
}

void xpgCb_CamCoderRecordStart()
{
    MP_DEBUG("%s", __func__);
    if(g_MinidvMode == CAMERAPREVIEW || g_MinidvMode == CAMERACAPTURE)
	{
        CameraInit = 1;
        xpgCb_CameraPreviewAndCapture();
        return;
    }
    MP_ALERT("- g_MinidvMode = %d -", g_MinidvMode);
    if (g_bXpgStatus != XPG_MODE_PAUSE)
    {
        if((g_MinidvMode != CAMRECORDING) && (g_MinidvMode != CAMCAPTURE))
        {
      	
        	if (SystemGetFlagReadOnly(DriveCurIdGet()))
    		{
				MP_ALERT("Write protected !");
				TaskSleep(4000);
        		return ;
    		}
        	
            g_MinidvMode = CAMRECORDING;
            BOOL init_i = 1;
            g_bAniFlag &= ~ANI_READY;
            g_bAniFlag |= ANI_CAMRECORDING;
            g_bXpgStatus = XPG_MODE_RECORDING;
            g_boNeedRepaint = false;
              Idu_SetFontColor(255, 255, 255); //TimeStamp
            if(Camcorder_RecordStart(init_i) != PASS)
            {
            	g_MinidvMode = CAMRECORDSTOP;
            	g_bAniFlag |= ANI_READY;
            	g_bAniFlag &= ~ANI_CAMRECORDING;
            	g_bXpgStatus = XPG_MODE_PREVIEW;
            	TaskSleep(4000);
           	return;
            }         
            
			if (GetRecordLenth())
				Ui_TimerProcAdd(GetRecordLenth(), xpgCb_CamCoderAutoRecordStop); //xpgCb_CamCoderRecordStop //xpgCb_CamCoderAutoRecordStop
            
            return;
        }
    }

}

void xpgCb_CamCoderRecordStop()
{
    MP_DEBUG("%s", __func__);
    RemoveTimerProc(xpgCb_CamCoderAutoRecordStop);
    
    g_MinidvMode = CAMRECORDSTOP;
    Idu_OsdErase();
    g_boNeedRepaint = false;
    Camcorder_RecordStop();
    g_bAniFlag &= ~ANI_CAMRECORDING;
    g_bAniFlag &= ~ANI_READY;


    
    
}


void xpgCb_CamCoderAutoRecordStop()
{
    MP_DEBUG("%s", __func__);
    Idu_OsdErase();
    g_boNeedRepaint = true;
    if(g_MinidvMode == CAMRECORDING)
    {
        Camcorder_RecordStop();

        g_bAniFlag &= ~ANI_CAMRECORDING;
        g_bAniFlag &= ~ANI_READY;
        g_MinidvMode = CAMRECORDSTOP;

        xpgCb_CamCoderRecordStart();
    }
}

void xpgCb_CameraPreviewAndCapture()
{
    MP_DEBUG("%s : CameraInit = %d", __func__, CameraInit);
    if (g_bXpgStatus != XPG_MODE_PAUSE && g_MinidvMode != CAMRECORDING)
    {
        if(CameraInit == 1)
        {
            g_MinidvMode = CAMERAPREVIEW;
            MP_DEBUG("%s : g_MinidvMode = %d", __func__, g_MinidvMode);
            g_bAniFlag &= ~ANI_VIDEO;
            g_bAniFlag &= ~ANI_READY;
//            xpgSearchAndGotoPage("Preview", sizeof("Preview")-1);
//            xpgUpdateStage();
//            mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());

            Camcorder_PreviewStop();
            Camera_PreviewStart(CAMERA_RESOLUTION_640x480);
            CameraInit = 0;
        }
        else
        {
            g_MinidvMode = CAMERACAPTURE;
            MP_DEBUG("%s : g_MinidvMode = %d", __func__, g_MinidvMode);
            Camera_Capture(CAMERA_RESOLUTION_640x480);
        }
    }
    return;
}
#endif
//------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if VIDEO_ON
struct video_thumbs_t video_thumbs = {0};

int DecodeVideoFirstFrameThumb(ST_SEARCH_INFO * pSearchInfo, ST_IMGWIN* pWin)
{
	int rc;
	STREAM *sHandle;
	XPG_FUNC_PTR *xpgFuncPtr;
	extern BOOL boVideoPreview;
	
	sHandle = FileListOpen(FileBrowserGetCurDrive(), pSearchInfo);
	if (sHandle == NULL)
		return DECODE_FAIL;

	g_bXpgStatus = XPG_MODE_ENTERVIDEOPLAYER;
	boVideoPreview = TRUE;

	g_pMovieScreenWin = Idu_GetNextWin();
	xpgFuncPtr = (XPG_FUNC_PTR *)xpgGetFuncPtr();

	mpPaintWin(g_pMovieScreenWin, RGB2YUV(0, 0, 0));

    mpDebugPrint("start Api_EnterMoviePreview()");
	rc = Api_EnterMoviePreview(sHandle, pSearchInfo->bExt, g_pMovieScreenWin, &g_sMovieWin, 
						0, 0, g_pMovieScreenWin->wWidth, g_pMovieScreenWin->wHeight, xpgFuncPtr);
	
    if(rc >= 0) // 0-PASS
    {
        Api_ExitMoviePlayer();
        g_bAniFlag |= ANI_VIDEO_PREVIEW;
        mpDebugPrint("preview ok!  Api_ExitMoviePlayer()");
    }
	else
	{
        mpDebugPrint("preview fail!");
		mpPaintWin(g_pMovieScreenWin, RGB2YUV(0x55, 0x55, 0x55));
	}

    boVideoPreview = FALSE;
	mpCopyWin(pWin, g_pMovieScreenWin);

	g_bXpgStatus = XPG_MODE_VIDEOMENU;
	
	return PASS;
}

int Free4VideoFilesThumb()
{
    int i;
    for (i = 0; i < THUMB_COUNT; i++) 
    {
        video_thumbs.enable[i] = FALSE;
        if (video_thumbs.buffer[i] != NULL) {
            ext_mem_free(video_thumbs.buffer[i]);
            video_thumbs.buffer[i] = NULL;
        }
    }
}

int Draw4VideoFilesThumb()
{
	int ret;
	DWORD i;
	struct ST_FILE_BROWSER_TAG *psFileBrowser;
	DWORD dwTotalFileCnt, dwCurrFileIndex;
	DWORD dwPageNum, dwDispIndex;
	ST_SEARCH_INFO* pSearchInfo;

	mpDebugPrint("%s()", __FUNCTION__);
	
	psFileBrowser = &g_psSystemConfig->sFileBrowser;
	dwTotalFileCnt = psFileBrowser->dwImgAndMovTotalFile;
	dwCurrFileIndex = FileBrowserGetCurIndex();
	dwPageNum = dwCurrFileIndex / THUMB_COUNT;

    if (dwTotalFileCnt == 0)                // No video file.
    {   
        Free4VideoFilesThumb();
        return 0;
    }

    for (i = 0; i < THUMB_COUNT; i++) 
        video_thumbs.enable[i] = FALSE;
    
	for (i = 0; i < THUMB_COUNT; i++)               // init wins
	{
        if (video_thumbs.buffer[i] == NULL) 
        {
            video_thumbs.buffer[i] = (DWORD *)ext_mem_malloc(THUMB_WIDTH * THUMB_HEIGHT * 2 + 64);
            if (video_thumbs.buffer[i] == NULL)
            {
                mpDebugPrint("-E- : no memory to Draw VIDEO thumbs");
                return FAIL;
            }
        }
		mpWinInit(&video_thumbs.wins[i], video_thumbs.buffer[i], THUMB_HEIGHT, THUMB_WIDTH);
	}
	
	for (i = 0; i < THUMB_COUNT; i++) 
	{
		dwDispIndex = dwPageNum*THUMB_COUNT + i;
		if (dwDispIndex >= dwTotalFileCnt)
			break;
		pSearchInfo = &g_psSystemConfig->sFileBrowser.sImgAndMovFileList[dwDispIndex];
    	if (pSearchInfo == NULL)
			continue;

		ret = DecodeVideoFirstFrameThumb(pSearchInfo, &video_thumbs.wins[i]);
		mpDebugPrint("DecodeVideoFirstFrameThumb ret = %d", ret);
		if (ret == PASS)
			video_thumbs.enable[i] = TRUE;
	}
	
}
#endif

void mpCopyWinAreaToWinArea(ST_IMGWIN * pDstWin, ST_IMGWIN * pSrcWin, SWORD wDx, SWORD wDy, SWORD wSx, SWORD wSy, WORD wW,
					WORD wH)
{
	register DWORD *pdwSrcBuffer = (DWORD *) ((DWORD) pSrcWin->pdwStart | 0xa0000000);
	register DWORD *pdwDstBuffer = (DWORD *) ((DWORD) pDstWin->pdwStart | 0xa0000000);

	if (pDstWin == pSrcWin) {
		return;
	}
	if (wSy < 0) wSy = 0;
	if (wSx < 0) wSx = 0;
	if (wDy < 0) wDy = 0;
	if (wDx < 0) wDx = 0;
	if (wSy+wH > pSrcWin->wHeight) return;
	if (wSx+wW> pSrcWin->wWidth) return;
	if (wDy+wH > pDstWin->wHeight) return;
	if (wDx+wW> pDstWin->wWidth) return;

#if 0//(UI_FLOW_PAN2009||UI_FLOW_FOR_SONY||GIINII_FOR_MAVIS2009_UI_FLOW)
    image_scale(pSrcWin, pDstWin, wSx, wSy, wSx+wW, wSy+wH, wDx, wDy, wDx+wW, wDy+wH);
#else
	register DWORD dwSrcOffset = pSrcWin->dwOffset >> 2;

	pdwSrcBuffer += wSy * dwSrcOffset;
	pdwSrcBuffer += wSx >> 1;

	register DWORD dwDstOffset = pDstWin->dwOffset >> 2;

	pdwDstBuffer += wDy * dwDstOffset;
	pdwDstBuffer += wDx >> 1;

	if (wW >= pDstWin->wWidth)
		wW = pDstWin->wWidth;
	
	if (wH >= pDstWin->wHeight)
		wH = pDstWin->wHeight;

	register DWORD y;
	DWORD dwLineWidth = wW << 1;

	if (dwLineWidth == 0)
		return;

	for (y = 0; y < wH; y++)
	{
		memcpy(pdwDstBuffer, pdwSrcBuffer, dwLineWidth);
		pdwSrcBuffer += dwSrcOffset;
		pdwDstBuffer += dwDstOffset;
	}
#endif
}


void ui_PreviewToMainPage()
{
	MP_DEBUG("%s", __func__);
    Camcorder_PreviewStop();
    g_bAniFlag &= ~ANI_VIDEO;
    g_bAniFlag &= ~ANI_READY;
    g_boNeedRepaint = false;
    g_bXpgStatus = XPG_MODE_MAINMENU;
    //CamCameraPhotoMenu();
    //xpgCb_PhotoMenuThumbExit();
    xpgUpdateStage();
    
}

int FormatCurrStorageCard()
{
    int ret;
    int currdrv;

    mpDebugPrint("%s() ,CurId=%d", __FUNCTION__,g_psSystemConfig->sStorage.dwCurStorageId);
    if (g_psSystemConfig->sStorage.dwCurStorageId==NAND)
        currdrv = NAND;
    else if (g_psSystemConfig->sStorage.dwCurStorageId==SD_MMC)
        currdrv = SD_MMC;
	else
		return FAIL;
    
    if (ret = Fat32_Format(DriveGet(currdrv), USER_DRV_LABEL) != PASS)
        if (ret = Fat16_Format(DriveGet(currdrv), USER_DRV_LABEL) != PASS)
            if (ret = Fat12_Format(DriveGet(currdrv), USER_DRV_LABEL) != PASS)
                MP_ALERT("-E- (DrvId = %d) FAT32/FAT16/FAT12 formatting all failed ! => Pls check it ...", currdrv);
    if (ret != PASS)
        return FAIL;
    DriveDelete(currdrv);
		
    // after drive formatting, DriveAdd() again
    if (DriveAdd(currdrv))		// at least one partition/drive added
    {
        DRIVE * drv;
        drv = DriveChange(currdrv);
        if ((drv->StatusCode == FS_SUCCEED) && (drv->Flag.Present == 1))	//this drive has already been added
        {
            DirReset(drv);
            DirFirst(drv);
        }
        else
        {
            MP_ALERT("-E- The partition drive of (drvId = %d) was not well-added !	Pls check it !!\r\n", currdrv);
            return FAIL;
        }
    }
    else // none of partition/drive added
    {
        MP_ALERT("-E- (drvId = %d) DriveAdd() failed !\r\n", currdrv);
        return FAIL;
    }
}

#if VIDEO_ON
DWORD dwSaveVideoFileIndex=0;

void SetSaveVideoIndex(DWORD dwIndex)
{
	dwSaveVideoFileIndex=dwIndex;
}

void ui_EnterVideoPage()
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    
    g_psSystemConfig->dwCurrentOpMode = OP_MOVIE_MODE;
    g_pstBrowser->bCurDriveId = g_psSystemConfig->sStorage.dwCurStorageId;

    FileBrowserResetFileList();
    g_bXpgStatus = XPG_MODE_VIDEOMENU;
    xpgCb_EnterVideoMenu();
    FileBrowserScanFileList(SEARCH_TYPE);
    if (psFileBrowser->dwImgAndMovTotalFile == 0) {
        mpDebugPrint("No avi file.");
        mpDebugPrint("EXIT VIDEO mode.");
        return;
    }
	if (dwSaveVideoFileIndex)
	{
		xpgMenuListSetIndex(FileListSetCurIndex(dwSaveVideoFileIndex), false);
		dwSaveVideoFileIndex=0;
	}
	else
		FileListSetCurIndex(0);
    g_boNeedRepaint = true;
    Draw4VideoFilesThumb();
}

void ui_SelectPrevVideo()
{
    DWORD dwCurIndex, dwTotalFile;
    DWORD dwCurrPage, dwTotalPage, dwNewPage;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;

    dwCurIndex = psFileBrowser->dwImgAndMovCurIndex;
    dwTotalFile = psFileBrowser->dwImgAndMovTotalFile;
    if (dwTotalFile == 0)
        return;
    if (dwTotalFile == 1)
        return;
    dwCurrPage = dwCurIndex / THUMB_COUNT;
    dwTotalPage = dwTotalFile / THUMB_COUNT;
    if (dwTotalFile % THUMB_COUNT)
        dwTotalPage ++;
    
    xpgCb_VideoMenuListPrev();

    if (dwTotalPage == 1)
        return;

    dwCurIndex = psFileBrowser->dwImgAndMovCurIndex;
    dwNewPage = dwCurIndex / THUMB_COUNT;
    if (dwNewPage != dwCurrPage)
        Draw4VideoFilesThumb();
    return;
}

void ui_SelectNextVideo()
{
    DWORD dwCurIndex, dwTotalFile;
    DWORD dwCurrPage, dwTotalPage, dwNewPage;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;

    dwCurIndex = psFileBrowser->dwImgAndMovCurIndex;
    dwTotalFile = psFileBrowser->dwImgAndMovTotalFile;
    if (dwTotalFile == 0)
        return;
    if (dwTotalFile == 1)
        return;
    dwCurrPage = dwCurIndex / THUMB_COUNT;
    dwTotalPage = dwTotalFile / THUMB_COUNT;
    if (dwTotalFile % THUMB_COUNT)
        dwTotalPage ++;
    
    xpgCb_VideoMenuListNext();

    if (dwTotalPage == 1)
        return;

    dwCurIndex = psFileBrowser->dwImgAndMovCurIndex;
    dwNewPage = dwCurIndex / THUMB_COUNT;
    if (dwNewPage != dwCurrPage)
        Draw4VideoFilesThumb();
    return;
}

#endif

void xpgCb_EnterCamcoderPreview()
{
    //MP_DEBUG("%s", __func__);

    MP_DEBUG("%s: total=%d", __FUNCTION__,mem_get_free_space_total());


#if USBOTG_WEB_CAM
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;
	if (UsbOtgHostConnectStatusGet(eWhichOtg))
	{
    	MP_DEBUG("UsbOtgHostConnectStatusGet  ON");
		return;
	}
	if (SystemCardPresentCheck(USB_WEBCAM_DEVICE))
	{
    	MP_DEBUG("USBOTG_WEB_CAM  ON");
		return;
	}
#endif

    //ES8328_Codec_SetRecordMode();
    //Ui_TimerProcRemove(AutoSleep);
#if RTC_ENABLE
    Ui_TimerProcRemove(UpdateClock);
#endif

#if MAKE_XPG_PLAYER
    xpgSearchAndGotoPage("Preview", 0);
    xpgUpdateStage();
    mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());
#endif
    CamCamCoderEnter();

    g_MinidvMode = CAMPREVIEW;
#if (AUDIO_DAC==DAC_ALC5621)
      Codec_ElecSwitch_RecordMode();
#endif

}

void Timer_FirstEnterCamPreview()
{
    Ui_TimerProcAdd(1000, xpgCb_EnterCamcoderPreview);
	
}




void CamCamCoderEnter()
{
    MP_DEBUG("%s", __func__);
    g_bAniFlag &= ~ANI_READY;
#if 0//((PRODUCT_UI==UI_SURFACE))
    Camcorder_PreviewStart(CAMCORDER_RESOLUTION_720P);
#else
   ////(PRODUCT_UI==UI_WELDING)
    Camcorder_PreviewStart(CAMCORDER_RESOLUTION_VGA);
#endif
}

#if CAPTURE_TIME_STAMP_ON
void WriteTimeStampStringToPhoto(ST_IMGWIN * pWin)
{
	int i;
	char strbuff[64];
	ST_SYSTEM_TIME stCurrTime;
	
	SystemTimeGet(&stCurrTime);

	sprintf(strbuff, "%04d/%02d/%02d %02d:%02d:%02d", stCurrTime.u16Year, stCurrTime.u08Month, stCurrTime.u08Day,
		stCurrTime.u08Hour, stCurrTime.u08Minute, stCurrTime.u08Second);
    //Idu_SetFontColor(255, 255, 0);
	Idu_PrintString(pWin, strbuff, pWin->wWidth-260, pWin->wHeight-30, 0, 0);
}
#endif

SWORD xpgCb_CaptureWinToFile(ST_IMGWIN *pWin)
{
    STREAM *handle;
    MP_DEBUG("%s -%d", __FUNCTION__,DriveCurIdGet());

	if (DriveCurIdGet()==NULL_DRIVE)
    {
        MP_ALERT("%s: NULL drive index!", __FUNCTION__);
        return FAIL;
    }
	if (SystemGetFlagReadOnly(DriveCurIdGet()))
	{
        MP_ALERT("Write protected !");
	      TaskSleep(4000);
	      return FAIL;
	}

	handle=(STREAM *)GetNewCaptureHandle();

	if (handle)
	{
		MP_DEBUG1("## %s ##", __FUNCTION__);		
#ifdef OSDICON_DC
		BYTE bStr[16];
		sprintf(bStr,"%d", GetNeedCaptureNumber());
		ShowActionIconWithString(OSDICON_DC,bStr,OSD_NEW_WHITE,500);
#endif

		SWORD swRet=PASS;
		DWORD JpegBufSize;
		DWORD IMG_size = 0;
		BYTE *JpegBuf = NULL;
		ST_IMGWIN stTmpWin;

		mpWinInit(&stTmpWin,NULL,pWin->wHeight,pWin->wWidth);
		stTmpWin.pdwStart = (DWORD *)ext_mem_malloc(stTmpWin.wWidth*stTmpWin.wHeight*2);
        mpCopyEqualWin(&stTmpWin, pWin);
		JpegBufSize = pWin->wWidth*pWin->wHeight*2;
		JpegBuf = (BYTE*)ext_mem_malloc(JpegBufSize);


		//encode jpeg
#if CAPTURE_TIME_STAMP_ON
		WriteTimeStampStringToPhoto(&stTmpWin);
#endif
		IMG_size = ImageFile_Encode_Img2Jpeg(JpegBuf, &stTmpWin);

		if (JpegBufSize < IMG_size)
		{
			mpDebugPrint("--E-- %s: memory overflow", __FUNCTION__);
			//free memory
			DeleteFile(handle);
			swRet= -3;
		}
		else
		{
			FileWrite(handle, (BYTE *) JpegBuf, IMG_size);
			FileClose(handle);
			mpDebugPrint("-- %s:ok!", __FUNCTION__);
		}

		//free memory
		if(JpegBuf != NULL)
		{
			ext_mem_free(JpegBuf);
			JpegBuf = NULL;
		}
		ext_mem_free(stTmpWin.pdwStart );

		return swRet;

	}

    return FAIL;
}

#endif //#if (SENSOR_ENABLE == ENABLE)

