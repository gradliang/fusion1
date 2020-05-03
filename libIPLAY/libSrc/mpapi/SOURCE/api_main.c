/**
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
* @file:         api_main.c
* Descriptions  :
*******************************************************************************
*/


/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "os.h"
#include "av.h"
#include "fs.h"
#include "display.h"
#include "mpapi.h"
#include "api_main.h"
#include "flagdefine.h"
#include "Taskid.h"
#include "Mpapi_inter.h"

#include "../../audio/include/audio_hal.h"
#include "../../audio/INCLUDE/wavUtil.h"

//#include "sensor.h"

#if DISPLAY_VIDEO_SUBTITLE
#ifndef _SUB_COMMON_H_
#include "sub_common.h"
#endif
#endif

/*
// Variable declarations
*/

#if (VIDEO_ON || AUDIO_ON)
BYTE MJ;
//#endif

int left_channel  = 1;
int right_channel = 1;
//STREAM *audio_stream;
BYTE bFileType = FILE_EXT_NULL;

extern DWORD  g_dwTotalSeconds;
extern Audio_dec Media_data;
extern DWORD g_bAniFlag;
extern float StepSeconds;
extern DWORD PcmChunkCounter;
extern DWORD Audio_FB_Seconds;
XPG_FUNC_PTR* g_pXpgFuncPtr=NULL;
BYTE g_bPlayMode;
STREAM *g_sHandle;
BYTE g_bAudioCodecInit = 0;

BYTE* AUDIO_EQ_INFO = NULL;
enum AO_AV_TYPE  AO_AV = NULL_TYPE;
int resoultion_1=DISPLAY_VIDEO_RATIO;
extern int decoder_initialized;
int displayratio_open=0;
extern int audio_samplerate;
float set_video_fps=1;//use API_SetVideoSpeed 

//=========
//extern int rec_stop;
//extern int rec_fileopen;
#endif

//=========

/*
// Structure declarations
*/

struct EXTENSION_TAG
{
	BYTE *pbExt;
	BYTE bExtLength;
	BYTE bFileType;
	BYTE bReserved[2];
};

/*
// Type declarations
*/
typedef struct EXTENSION_TAG ST_EXTENSION;

/*
// Static function prototype
*/
static BYTE GetFileType (BYTE *bExt);

/*
// Definition of internal functions
*/
void Api_MovieVideoFullScreen (void)
{
	MP_DEBUG("Api_MovieVideoFullScreen");

#if (MJPEG_ENABLE || VIDEO_ENABLE)
	switch (bFileType)
	{
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
		MjpegSwapDispSize();
#endif
		break;

	case FILE_EXT_WMA:
		break;

	default:
	case FILE_EXT_NULL:
		MovieVideoFullScreen();
#if ((CHIP_VER & 0xffff0000) != CHIP_VER_615)
		//H.264 might use display buffer as decode picture buffer in bypass mode,
		//thus we have to cover it at the begininng,
		//this will be recovered at	ipu_bypass_scale_650() or Api_ExitMoviePlayer()
		//by calling IDU_Show_Original_Color()
		if (MOVIE_STATUS_FULL_SCREEN & g_psSystemConfig->sVideoPlayer.dwPlayerStatus)
			IDU_Show_BG_Color_Black();
#endif
		break;
	}
#endif
}

SWORD Api_MovieForward (DWORD sec)
{
	//mpDebugPrint("%s(%d)", __FUNCTION__, sec);
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER))
		return FAIL;

	//mpDebugPrint("sec = %d", sec);
	StepSeconds = (float) sec;
	switch (bFileType)
	{
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerForward();
		break;
#endif
	case FILE_EXT_WMA:
		break;

	default:
	case FILE_EXT_NULL:
		MovieForward(1);
		break;
	}

	return PASS;
#endif
	return FAIL;
}

SWORD Api_MovieBackward (DWORD sec)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER))
		return FAIL;
	SemaphoreWait(VIDEO_SEMA_ID);
	//mpDebugPrint("Api_MovieBackward sec = %d", sec);
	StepSeconds = (float) -1.0f * sec;
	switch (bFileType)
	{
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerBackward();
		break;
#endif
	case FILE_EXT_WMA:
		break;

	default:
	case FILE_EXT_NULL:
		MovieBackward(1);
		break;
	}
	SemaphoreRelease(VIDEO_SEMA_ID);
	return PASS;
#endif
	return FAIL;
}

SWORD Api_AudioBackward (DWORD sec)
{
#if AUDIO_ON
	Audio_FB_Seconds = -sec;
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)) return FAIL;
	switch (bFileType)
	{
	default:
	case FILE_EXT_NULL:
		MovieBackward(0);
		break;
	}

	return PASS;
#endif
    return FAIL;
}

SWORD Api_AudioForward (DWORD sec)
{
#if AUDIO_ON
	Audio_FB_Seconds = sec;
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)) return FAIL;

	switch (bFileType)
	{
	default:
	case FILE_EXT_NULL:
		MovieForward(0);
		break;
	}
	return PASS;
#endif
    return FAIL;
}

BYTE *Api_GetEQInfo()
{
#if AUDIO_ON
#ifdef EQUALIZER_EN
	if (AUDIO_EQ_INFO)
		return AUDIO_EQ_INFO;
	else
		return NULL;
#else
	return NULL;
#endif
#endif
    return NULL;
}
#if AUDIO_ON
inline SWORD Api_AudioHWEnable()
{
	return MX6xx_AudioHW_On();
}

inline SWORD Api_AudioHWDisable()
{
	return MX6xx_AudioHW_Off();
}

inline VOID Api_AudioPause()
{
	MX6xx_AudioPause();
}

#else
inline SWORD Api_AudioHWEnable()
{
	return NULL;
}

inline SWORD Api_AudioHWDisable()
{
	return NULL;
}

inline VOID Api_AudioPause()
{
	
}

#endif
extern ST_IMGWIN *g_pMovieScreenWin;
SWORD Api_MoviePlay (void)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	//if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)) return FAIL;
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus &= ~MOVIE_STATUS_PAUSE;

	switch (bFileType)
	{
		//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerPlay();
		break;
#endif
	default:
	case FILE_EXT_NULL:
		MoviePlay();
		break;
	}

	g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_PLAY;
	return PASS;
#endif
	return FAIL;
}

#if (VIDEO_ON || AUDIO_ON)
extern int NotDecodeVideo;
#endif

SWORD Api_MoviePause (void)
{
#if AUDIO_ON||VIDEO_ON
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_PAUSE;
	if (g_bAniFlag & ANI_VIDEO)
	{
		while(NotDecodeVideo)//Waiting for first time of video decode finish.
			TaskYield();	 //It prevents that the system crashes in Audio task(for *.mpg(MPEG-1 stream))
							 //XianWen Chang note it 2010/10/04
	}
	MP_DEBUG("Api_MoviePause");
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)) return FAIL;
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)) return FAIL;
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_STOP) return FAIL;

	#if DISPLAY_PAUSE_BYPASS2DMA
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_FULL_SCREEN)
	{
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))      
		MP650_FPGA_Change_DMA_Priority_IDU();
#endif
	}
    #endif
	
	switch (bFileType)
	{
		//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerPause();
		break;
#endif
	default:
	case FILE_EXT_NULL:
		MoviePause();
		break;
	}

	return PASS;
#endif
	return FAIL;
}

SWORD Api_MovieResume (void)
{    
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	MP_DEBUG("Api_MovieResume");
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)) return FAIL;
	if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PAUSE)) return FAIL;
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_STOP)
	{
		TaskYield();
		return FAIL;
	}

	g_psSystemConfig->sVideoPlayer.dwPlayerStatus &= ~MOVIE_STATUS_PAUSE;

    #if DISPLAY_PAUSE_BYPASS2DMA
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_FULL_SCREEN)
	{
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		DMA *dma = (DMA *)DMA_BASE;
		dma->FDMACTL_EXT2 = 0x00044002;
		dma->FDMACTL_EXT3 = 0xfffbbffd;
		dma->FDMACTL_EXT1 |= 0x00000001 ;
#endif
	}
	#endif

	switch (bFileType)
	{
		//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerResume();
		break;
#endif
	default:
	case FILE_EXT_NULL:
		MovieResume();
		break;
	}
	return PASS;
#endif
	return FAIL;
}

#if ANTI_TEARING_ENABLE
extern BYTE frame_buffer_index;
extern BOOL buffer_switch, video_stop;
#endif
SWORD Api_MovieStop (void)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s", __FUNCTION__);
	#endif
	MP_DEBUG("dwPlayerStatus |= MOVIE_STATUS_STOP");
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_STOP;

#if ANTI_TEARING_ENABLE
	CHANNEL *dma = (CHANNEL *) (DMA_IDU_BASE);
	ST_IMGWIN *curwin = Idu_GetCurrWin();
#endif

	switch (bFileType)
	{
		//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerStop();
		break;
#endif
	default:
	case FILE_EXT_NULL:
#if ANTI_TEARING_ENABLE
		while ((dma->StartB - dma->StartA) > (curwin->wWidth << 1));
		frame_buffer_index = buffer_switch = 0;
		video_stop = TRUE;
#endif
		MovieStop();
		break;
	}

	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s end", __FUNCTION__);
	#endif
	return PASS;
#endif
	return FAIL;
}

void Api_ExitMoviePlayer (void)
{
	
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	displayratio_open=0;
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
	#endif

	// The USB I/O can not be canceled.
	// Daylay to make sure that the USB I/O can be finished.
	//	TimerDelay(50);  // Mark out temporarily after USB timing is confirmed on MP650 later
#if 0
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
	{
		Api_MovieStop();
		MP_DEBUG("Api_MovieStop");
	}
#endif
	BOOL b_display_video_drop_info = FALSE;
#if DISPLAY_VIDEO_DROP_INFO_FULL
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_FULL_SCREEN)
	{
		b_display_video_drop_info = TRUE;
	}
#endif

#if DISPLAY_VIDEO_DROP_INFO_THUMB
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_DRAW_THUMB)
	{
		b_display_video_drop_info = TRUE;
	}
#endif

#if (DISPLAY_VIDEO_DROP_INFO_FULL || DISPLAY_VIDEO_DROP_INFO_THUMB)
	if (b_display_video_drop_info)
	{
		S_VIDEO_DROP_INFO_T sDropInfo;
		GetVideoCodecCategory(sDropInfo.cVidoeCodec);
		GetAudioCodecCategory(sDropInfo.cAudioCodec);
		sDropInfo.iTotalFrames = GetTotalVideoFrames();
		sDropInfo.iMissedFrame = GetMissedVideoFrames();
		sDropInfo.iPlayedFrame = sDropInfo.iTotalFrames - sDropInfo.iMissedFrame;
		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgDispVideoDropInfo)
  		    g_pXpgFuncPtr->xpgDispVideoDropInfo(sDropInfo);
	}
#endif

	switch (bFileType)
	{
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_MOV:
	case FILE_EXT_AVI:
		MjpegPlayerClose();
		break;
#endif
	default:
	case FILE_EXT_NULL:
		ExitMoviePlayer();
		break;
	}

#if (((CHIP_VER & 0xFFFF0000) != CHIP_VER_615))
	if (g_bAniFlag & ANI_VIDEO)
	{
		if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
		{
			MP650_FPGA_Change_DMA_Priority_IDU();
			#if YUV444_ENABLE
	        Idu_Chg_422_Mode();
	        #endif
		}

		if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_FULL_SCREEN)
		{
			Ipu_Video_Bypass_Show_Main_Win();
		#if 0
			ST_IMGWIN * CurrWin = Idu_GetCurrWin();
			struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
			CurrWin->wWidth = pstScreen->wInnerWidth;
			CurrWin->wHeight = pstScreen->wInnerHeight;
		#else
			IODelay(1500);
		#endif
			//Idu_OsdErase();			
			Idu_PaintWin(Idu_GetCurrWin(), RGB2YUV(0, 0, 0));
			Idu_Bypass2DMA(Idu_GetCurrWin());
#if ((CHIP_VER & 0xffff0000) != CHIP_VER_615)
			IDU_Show_Original_Color(); //recovering IDU_Show_BG_Color_Black();
#endif
		}
	}
#endif    
	
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_NULL;
	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s @ %d done", __FUNCTION__, __LINE__);
	#endif
	mwTerm();
#endif
}

BOOL boVideoPreview;
extern int MJPEG;
 int CHANGMODE=1;
int Api_EnterMoviePlayer (STREAM *sHandle, BYTE *bExt, STREAM *sLyricHandle, ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                          XPG_FUNC_PTR* xpg_func_ptr, BYTE play_mode, int subtitle_sync_align)
{
	
#if	DISPLAY_CHANGMODE
CHANGMODE=0;
#endif
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	MP_DEBUG("Api_EnterMoviePlayer");
	mwInit();
	SWORD swRetVal = 0;
	g_bAudioCodecInit = 0;

	g_pXpgFuncPtr = xpg_func_ptr;
	g_bPlayMode   = play_mode;
	g_sHandle     = sHandle;

	while (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PARSING)
	{
		MP_DEBUG("play movie while preparsing");
		TaskYield();
	}
	mpDebugPrint("g_psSystemConfig->sVideoPlayer.dwPlayerStatus movie player %d",g_psSystemConfig->sVideoPlayer.dwPlayerStatus);
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)
	{
		Api_ExitMoviePlayer();
		TimerDelay(1);
	}

	MJPEG = 0;
	bFileType = GetFileType(bExt);
	MP_DEBUG2("-I- get file type = %d %s", bFileType, bExt);
	if (! SystemCardPresentCheck(sHandle->Drv->DrvIndex))
	{
	    MP_ALERT("Card not present(video play) !");
	    return FAIL;
	}

	g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_PLAYER;
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_FULL_SCREEN;
	//Api_MovieVideoFullScreen();
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650)|((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	DMA *dma = (DMA *)DMA_BASE;
	if (( sHandle->Drv->DevID ==DEV_SD_MMC)|( sHandle->Drv->DevID ==DEV_SD2)|( sHandle->Drv->DevID ==DEV_CF)|( sHandle->Drv->DevID ==DEV_MS)|(sHandle->Drv->DevID ==DEV_CF_ETHERNET_DEVICE))
	{
	    dma->FDMACTL_EXT2 = 0x00044002;
	    dma->FDMACTL_EXT3 = 0xfffbbffd;
	    dma->FDMACTL_EXT1 |= 0x00000001 ;
	}
	mpDebugPrint("sHandle->Drv->DevID %d",sHandle->Drv->DevID);

	if (( sHandle->Drv->DevID ==DEV_USB_WIFI_DEVICE)|( sHandle->Drv->DevID ==DEV_USBOTG1_HOST_PTP)|( sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID1)|
        ( sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID2)|( sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID3)|( sHandle->Drv->DevID ==DEV_USBOTG1_HOST_ID4))
	{
		dma->FDMACTL_EXT2 = 0x00000602;
	    dma->FDMACTL_EXT3 = 0xfffff9fd;
	    dma->FDMACTL_EXT1 |= 0x00000001 ;
	}    
	
#endif 
	switch (bFileType)
	{
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_AVI :
	case FILE_EXT_MOV :
		AO_AV = AV_TYPE;
		MJ = 1;

		if (bFileType==FILE_EXT_AVI)
			swRetVal = MjpegPlayerOpen(sHandle, MJPEG_AVI, stScreenWin,
			                           0, 0, stScreenWin->wWidth, stScreenWin->wHeight, CMD_NORMAL_FORWARD);
		else
			swRetVal = MjpegPlayerOpen(sHandle, MJPEG_MOV, stScreenWin,
			                           0, 0, stScreenWin->wWidth, stScreenWin->wHeight, CMD_NORMAL_FORWARD);

		if (swRetVal == MSG_NOT_SUPPORT_FORMAT)
		{
			MP_DEBUG("-I- not support format");
			AO_AV = NULL_TYPE;
			//#if  MJPEG_ENABLE
			MJ = 0;
			//#endif

			goto UNKNOWN_EXT;
		}
		MJPEG = 1;
		if (swRetVal != MSG_NO_ERR)
			return FAIL;

		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();
		break;
#endif 
		//--- Audio file case ---
	case FILE_EXT_MP3 :
	case FILE_EXT_WMA :
	case FILE_EXT_AAC:
	case FILE_EXT_M4A:
	case FILE_EXT_OGG:
	case FILE_EXT_RA:
	case FILE_EXT_RAM:
	case FILE_EXT_RM:
	case FILE_EXT_AC3:
	case FILE_EXT_WAV:
	case FILE_EXT_AMR:
		swRetVal = EnterMoviePlayer(sHandle, sLyricHandle, bExt, stScreenWin, stMovieWin, subtitle_sync_align);
		if (swRetVal != PASS)//init fail
		{
			MP_ALERT("EnterMoviePlayer fail...may cause some memory leakage!!!!", swRetVal);
			AO_AV = NULL_TYPE;
			g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_NULL;
			MovieTask_Close();
			return FAIL;//init fail
		}

		//if (sLyricHandle != NULL)
			//FileClose(sLyricHandle); //temp for lyric

		if (swRetVal)//init fail
		{
			MP_DEBUG1("EnterMoviePlayer fail=%d",swRetVal);
			AO_AV = NULL_TYPE;
			return swRetVal;
		}

		g_dwTotalSeconds = Media_data.total_time;

		bFileType = FILE_EXT_NULL;
		break;

	default:
	case FILE_EXT_NULL:
UNKNOWN_EXT:
		bFileType = FILE_EXT_NULL;
		AO_AV = AV_TYPE;
		AiuDmaClearCallBackFunc();
		MP_DEBUG("FILE_EXT_NULL & EnterMoviePlayer");
		swRetVal = EnterMoviePlayer(sHandle, sLyricHandle, bExt, stScreenWin, stMovieWin, subtitle_sync_align);

		if (sLyricHandle != NULL)
			FileClose(sLyricHandle); //temp for lyric
		
		if (swRetVal) 
		{
			MovieTask_Close();
			g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_NULL;
			return swRetVal;
		}
		if (stScreenWin != NULL)
		{
			WinInit4VideoPlay(stScreenWin, stMovieWin);
	    }
        if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();

		if (g_bAniFlag & ANI_VIDEO)
		{
			MoviePlayerDispTime(TRUE);
		}
		else
		{
			MoviePlayerDispTotalTime(0);
			MoviePlayerDispTime(TRUE);
		}

		break;
	}

    #if YUV444_ENABLE
	SetJpegDecoder444();
	#endif
	

	return PASS;
#endif
	return FAIL;
}

int Api_EnterMoviePreview (STREAM *sHandle, BYTE *bExt, ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                           DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, XPG_FUNC_PTR* xpg_func_ptr)
{
     
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	mwInit();
	SWORD swRetVal = PASS;

	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PARSING)
		return FAIL;

	SemaphoreWait(VIDEO_SEMA_ID);
	g_pXpgFuncPtr = xpg_func_ptr;
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)
	{
		Api_ExitMoviePlayer();
		TimerDelay(1);
	}

	g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_PREVIEW;

	MJPEG = 0;
	bFileType = GetFileType(bExt);
	MP_DEBUG2("-I- get file type = %d %s", bFileType, bExt);
	if (! SystemCardPresentCheck(sHandle->Drv->DrvIndex))
			{
			MP_ALERT("Card not present(videopreview) !");
			SemaphoreRelease(VIDEO_SEMA_ID);
			return FAIL;
			}

	switch (bFileType)
	{
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_AVI:
		swRetVal = MjpegPlayerOpen(sHandle, MJPEG_AVI, stScreenWin,
		                           dwX, dwY, dwWidth, dwHeight, CMD_DECODE_FIRST_FRAME); // CMD_NORMAL_FORWARD); // Jonny 20090401 for VideoPreview
		if (swRetVal == MSG_NOT_SUPPORT_FORMAT)
		{
			MP_DEBUG("-I- not support format");
			goto UNKNOWN_EXT;
		}
		if (swRetVal != MSG_NO_ERR)
		{
			SemaphoreRelease(VIDEO_SEMA_ID);
			return FAIL;
		}

		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();
		MJPEG = 1;
		break;

	case FILE_EXT_MOV:
		swRetVal = MjpegPlayerOpen(sHandle, MJPEG_MOV, stScreenWin,
		                           dwX, dwY, dwWidth, dwHeight, CMD_DECODE_FIRST_FRAME); // CMD_NORMAL_FORWARD); // Jonny 20090401 for VideoPreview
		if (swRetVal == MSG_NOT_SUPPORT_FORMAT)
		{
			MP_DEBUG("-I- not support format");
			goto UNKNOWN_EXT;
		}

		if (swRetVal != MSG_NO_ERR)
		{
			SemaphoreRelease(VIDEO_SEMA_ID);
			return FAIL;
		}

        if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();
		MJPEG = 1;
		break;
#endif

	default:
	case FILE_EXT_NULL:
UNKNOWN_EXT:
		bFileType = FILE_EXT_NULL;
		#if AUDIO_ON
		AiuDmaRegCallBackFunc(AudioISR);
		#endif
		MP_DEBUG("FILE_EXT_NULL & EnterMoviePreview");
		stMovieWin->wX = dwX;
		stMovieWin->wY = dwY;
		stMovieWin->wWidth = dwWidth;
		stMovieWin->wHeight = dwHeight;
		swRetVal = EnterMoviePreview(sHandle, bExt, stScreenWin, stMovieWin);

		MJPEG = 1;
		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();

		if (swRetVal == PASS && stScreenWin != NULL)
		{
			//dwX &= 0xfffffffe;
			//stMovieWin->dwOffset = (stScreenWin->wWidth - dwWidth) << 1;
			//stMovieWin->pdwStart = (DWORD *)((BYTE *)stScreenWin->pdwStart + ((dwY * stScreenWin->wWidth + dwX) << 1));
			//stMovieWin->wWidth = dwWidth;
			//stMovieWin->wHeight = dwHeight;
			#if YUV444_ENABLE
			SetJpegDecoder422();
			#endif
			if (MovieShowFirstFrame() != PASS)
			{
                MovieTask_Close();
				g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_NULL;
				SemaphoreRelease(VIDEO_SEMA_ID);
				return FAIL;
			}
		}

		MoviePlayerDispTime(TRUE);
		break;
	}
	//if (swRetVal == PASS)
	; //g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_PLAYER | MOVIE_STATUS_PREVIEW; // Jonny 20090401 for VideoPreview
	//else
	if (swRetVal != PASS)
	{
	    MovieTask_Close();
		g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_NULL;
	}

	SemaphoreRelease(VIDEO_SEMA_ID);
	return swRetVal;
#endif
	return FAIL;
}

SWORD Api_VideoDrawThumb(STREAM *sHandle, BYTE *bExt, ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                         DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, XPG_FUNC_PTR* xpg_func_ptr, BYTE play_mode)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	SWORD swRetVal = 0;
	g_bAudioCodecInit = 0;

	MP_DEBUG("Api_VideoDrawThumb");
	MP_DEBUG1("sHandle = 0x%x", sHandle);
	MP_DEBUG1("dwX = %d", dwX);
	MP_DEBUG1("dwY = %d", dwY);
	MP_DEBUG1("dwWidth = %d", dwWidth);
	MP_DEBUG1("dwHeight = %d", dwHeight);
	MP_DEBUG1("stScreenWin->wX = %d", stScreenWin->wX);
	MP_DEBUG1("stScreenWin->wY = %d", stScreenWin->wY);
	MP_DEBUG1("stScreenWin->dwWidth = %d", stScreenWin->wWidth);
	MP_DEBUG1("stScreenWin->wHeight = %d", stScreenWin->wHeight);

	g_sHandle     = sHandle;
	g_pXpgFuncPtr = xpg_func_ptr;
	g_bPlayMode   = play_mode;
	while (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PARSING)
	{
		MP_DEBUG("play movie while preparsing");
		TaskYield();
	}

	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)
	{
		Api_ExitMoviePlayer();
		TimerDelay(1);
	}

	g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_DRAW_THUMB;

	MJPEG = 0;

	bFileType = GetFileType(bExt);
	MP_DEBUG2("-I- get file type = %d %s", bFileType, bExt);
	//Api_MovieVideoFullScreen();

	switch (bFileType)
	{
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	case FILE_EXT_AVI :
	case FILE_EXT_MOV :
		AO_AV = AV_TYPE;

		//#if  MJPEG_ENABLE
		MJ=1;
		//#endif

		if (bFileType==FILE_EXT_AVI)
			swRetVal = MjpegPlayerOpen(sHandle, MJPEG_AVI, stScreenWin,
			                           dwX, dwY, dwWidth, dwHeight, CMD_NORMAL_FORWARD);
		else
			swRetVal = MjpegPlayerOpen(sHandle, MJPEG_MOV, stScreenWin,
			                           dwX, dwY, dwWidth, dwHeight, CMD_NORMAL_FORWARD);

		if (swRetVal == MSG_NOT_SUPPORT_FORMAT)
		{
			MP_DEBUG("-I- not support format");
			AO_AV = NULL_TYPE;
			//#if  MJPEG_ENABLE
			MJ=0;
			//#endif

			goto UNKNOWN_EXT;
		}
		MJPEG =1;
		if (swRetVal != MSG_NO_ERR)
			return FAIL;

		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();
		break;
#endif

	default:
	case FILE_EXT_NULL:
UNKNOWN_EXT:
		bFileType = FILE_EXT_NULL;
		AO_AV = AO_TYPE;
		AiuDmaClearCallBackFunc();
		MP_DEBUG("FILE_EXT_NULL & EnterMoviePlayer");

		STREAM *sLyricHandle = NULL;
		stMovieWin->wX = dwX;
		stMovieWin->wY = dwY;
		stMovieWin->wWidth = dwWidth;
		stMovieWin->wHeight = dwHeight;
		//swRetVal = EnterMoviePlayer(sHandle, sLyricHandle, bExt, stMovieWin);
		swRetVal = EnterMoviePlayer(sHandle, NULL, bExt, stScreenWin, stMovieWin, 0);


		if (sLyricHandle != NULL)
			FileClose(sLyricHandle); //temp for lyric

		if (swRetVal) return swRetVal;

		if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();

		if (stScreenWin != NULL)
		{
			WinInit4VideoPlay(stScreenWin, stMovieWin);
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
			if (MovieShowFirstFrame() != PASS)
			{
				if(!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW))
				if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgOsdPopupError)
				    g_pXpgFuncPtr->xpgOsdPopupError(ERR_VIDEO_ERROR);
				return FAIL;
			}
#endif
		}

		MoviePlayerDispTime(TRUE);
		break;
	}

    #if YUV444_ENABLE
	SetJpegDecoder444();
	#endif
	g_psSystemConfig->sVideoPlayer.dwPlayerStatus |= MOVIE_STATUS_PLAYER;

	return PASS;
#endif
	return FAIL;
}

int Api_MovieGetMediaInfo(STREAM *sHandle, BYTE *bExt, Media_Info* p_media_info)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	MP_DEBUG("Api_MovieGetMediaInfo");
	MP_DEBUG1("sHandle = 0x%x", sHandle);

	int ret;

	ret = MovieGetMediaInfo (FILE_TYPE_UNKNOWN, (BYTE *) p_media_info);
	if (!ret)
		return FAIL;

	return PASS;
#endif
	return FAIL;
}

#if DISPLAY_CHANGMODE
int Api_MovieChangeToThumbMode(ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                                           DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, XPG_FUNC_PTR* xpg_func_ptr, BYTE play_mode)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
    MP_DEBUG("Api_MovieChangeToThumbMode");
    if (MovieChangeToThumbMode(stScreenWin, 
		                       stMovieWin,
                               dwX, 
                               dwY, 
                               dwWidth, 
                               dwHeight, 
                               xpg_func_ptr, 
                               play_mode) != PASS)
		return FAIL;
	
	return PASS;
#endif
	return FAIL;
}

int Api_MovieChangeToFullScreenMode(void)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)    
    MP_DEBUG("Api_MovieChangeToFullScreenMode");	
    if (MovieChangeToFullScreenMode() != PASS)
		return FAIL;
	
	return PASS;
#endif
	return FAIL;
}
#endif

int Api_AlarmSoundPlayer (STREAM *sHandle, BYTE *bExt)
{
#if AUDIO_ON
	DWORD AlarmStartTime;
	STREAM *sLyricHandle = NULL;
	ST_IMGWIN *stScreenWin = NULL;
	ST_IMGWIN *stMovieWin = NULL;
	XPG_FUNC_PTR* xpg_func_ptr = NULL;
	BYTE play_mode = MUSIC_PLAY_MODE;
	int subtitle_sync_align = 0;
	
//#if(MJPEG_ENABLE || VIDEO_ENABLE)
	MP_DEBUG("Api_AlarmSoundPlayer");
	mwInit();
	SWORD swRetVal = 0;
	g_bAudioCodecInit = 0;

	g_pXpgFuncPtr = xpg_func_ptr;
	g_bPlayMode   = play_mode;

	while (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PARSING)
	{
		MP_DEBUG("play movie while preparsing");
		TaskYield();
	}
	mpDebugPrint("g_psSystemConfig->sVideoPlayer.dwPlayerStatus movie player %d",g_psSystemConfig->sVideoPlayer.dwPlayerStatus);
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)
	{
		Api_ExitMoviePlayer();
		TimerDelay(1);
	}

	MJPEG = 0;

	bFileType = GetFileType(bExt);
	MP_DEBUG2("-I- get file type = %d %s", bFileType, bExt);

	switch (bFileType)
	{
		//--- Audio file case ---
	case FILE_EXT_MP3:
	case FILE_EXT_WMA:
	case FILE_EXT_AAC:
	case FILE_EXT_M4A:
	case FILE_EXT_OGG:
	case FILE_EXT_RA:
	case FILE_EXT_RAM:
	case FILE_EXT_RM:
	case FILE_EXT_AC3:
	case FILE_EXT_WAV:
	case FILE_EXT_AMR:

		swRetVal = EnterMoviePlayer(sHandle, sLyricHandle, bExt, stScreenWin, stMovieWin, subtitle_sync_align);
		if (swRetVal != PASS)//init fail
		{
			MP_ALERT("EnterMoviePlayer fail...may cause some memory leakage!!!!", swRetVal);
//			AO_AV = NULL_TYPE;
			return FAIL;//init fail
		}

//		if (sLyricHandle != NULL)
	//		FileClose(sLyricHandle); //temp for lyric

		if (swRetVal)//init fail
		{
			MP_DEBUG1("EnterMoviePlayer fail=%d",swRetVal);
	//		AO_AV = NULL_TYPE;
			return swRetVal;
		}

		g_dwTotalSeconds = Media_data.total_time;

		bFileType = FILE_EXT_NULL;
		break;

	default:
	case FILE_EXT_NULL:
UNKNOWN_EXT:
		bFileType = FILE_EXT_NULL;
//		AO_AV = AV_TYPE;
		AiuDmaClearCallBackFunc();
		MP_DEBUG("FILE_EXT_NULL & EnterMoviePlayer");
/*		swRetVal = EnterMoviePlayer(sHandle, sLyricHandle, bExt, stScreenWin, stMovieWin, subtitle_sync_align);

		//if (sLyricHandle != NULL)
			//FileClose(sLyricHandle); //temp for lyric

		if (swRetVal) return swRetVal;

		if (stScreenWin != NULL)
		{
			WinInit4VideoPlay(stScreenWin, stMovieWin);
	    }
        if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgInitOsdTimeBar)
		    g_pXpgFuncPtr->xpgInitOsdTimeBar();

		if (g_bAniFlag & ANI_VIDEO)
		{
			MoviePlayerDispTime(TRUE);
		}
		else
		{
			MoviePlayerDispTotalTime(0);
			MoviePlayerDispTime(TRUE);
		}
*/
		break;
	}

	g_psSystemConfig->sVideoPlayer.dwPlayerStatus = MOVIE_STATUS_PLAYER;
	

	return PASS;
	
#endif
	return FAIL;
}


#define KEY_SOUND_TAG			0x574B4254
void Api_KeyToneSoundPlayer() //this api is based on fuji's 'void Boot_Sound_Play()'
{
#if AUDIO_ON
//	DWORD WavSize;
//	DWORD *pSrcBuf;
	WAV_CONFIG WavCfg;
	DWORD StartTime;
	PCM_DATA_BLOCK WavPcmData;

//	WavSize = IspFunc_GetRESOURCESize(KEY_SOUND_TAG);
//	pSrcBuf = (DWORD *)ext_mem_malloc(WavSize + 32);
	WavPcmData.size = IspFunc_GetRESOURCESize(KEY_SOUND_TAG);
	WavPcmData.start = (BYTE *)ext_mem_malloc(WavPcmData.size + 32);

//	if (IspFunc_ReadRESOURCE(KEY_SOUND_TAG, (BYTE *)pSrcBuf, WavSize) == 0)
	if (IspFunc_ReadRESOURCE(KEY_SOUND_TAG, (BYTE *)WavPcmData.start, WavPcmData.size) == 0)
	{
		MP_DEBUG(" -E- open Key Tone Sound error");
		return;
	}

//	if(DecodeWavHader(&WavCfg, (BYTE *)pSrcBuf, WavSize))
	if(DecodeWavHader1(&WavCfg, &WavPcmData))
	{
		MP_DEBUG(" -E- Key Tone Sound header error");
		return;
	}
	
	MP_DEBUG("Sample rate: %d, SampleSize: %d, Channels: %d",
		WavCfg.SampleRate, WavCfg.BitsPerSample, WavCfg.Channels);
		
	//while(1)
	//{
		Audio_OpenOutputDevice(WavCfg.SampleRate, WavCfg.Channels, WavCfg.BitsPerSample, 0);
		//MX6xx_printAIUstatus();
		WaveOutProc(WavPcmData.start + 44, WavPcmData.size - 44);
		
		StartTime = GetSysTime();
		while(WaveOutWriteDone()<0)
		{
			if(SystemGetElapsedTime(StartTime) > 5000)
				break;
		}
			
		//TaskYield();
		/*Audio_CloseOutputDevice();*/
	//}

//	ext_mem_free(pSrcBuf);
	ext_mem_free(WavPcmData.start);
#endif		
}	



#if DISPLAY_VIDEO_SUBTITLE
int Api_MovieGetSubtitleInfo(S_SUBTITLE_INFO_T* p_subtitle_info)
{
#if (MJPEG_ENABLE || VIDEO_ENABLE)
	MP_DEBUG("Api_MovieGetSubtitleInfo");

	int ret;

	ret = MovieGetSubtitleInfo(p_subtitle_info);

	return ret;
#endif
	return FAIL;
}
#endif

int Api_ChangeDisplayRatio(int resolution)
{
#if AUDIO_ON //change to video_on play video crack ·Ö²ã
    mpDebugPrint("Api_ChangeDisplayRatio");
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
	{
		displayratio_open=1;
    	if (Api_MoviePause() != PASS)
		return FAIL;
	}
	decoder_initialized=1;
    resoultion_1=resolution;
      
	if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
	{
		if (Api_MovieResume() != PASS)
		return FAIL;
	}
	return PASS;
#endif
    return FAIL;
}


/*
// Definition of local functions
*/
static BYTE GetFileType (BYTE *bExt)
{
	const ST_EXTENSION stExtensionTable[FILE_EXT_MAX_COUNT] =
	{
	#if WAV_ENABLE
		{"WAV", 3, FILE_EXT_WAV},
	#endif
		{"AC3", 3, FILE_EXT_AC3},
		{"RAM", 3, FILE_EXT_RAM},
		{"RA ", 3, FILE_EXT_RA},
		{"RM ", 3, FILE_EXT_RM},
		{"OGG", 3, FILE_EXT_OGG},
		{"WMA", 3, FILE_EXT_WMA},
		{"wma", 3, FILE_EXT_WMA},
		{"MP3", 3, FILE_EXT_MP3},
		{"AAC", 3, FILE_EXT_AAC},
		{"M4A", 3, FILE_EXT_M4A},
		{"AMR", 3, FILE_EXT_AMR},
		{"MOV", 3, FILE_EXT_MOV},
		{"AVI", 3, FILE_EXT_AVI},
	};
	BYTE i, j, c, bFlag;
	const ST_EXTENSION * stExtension = stExtensionTable;
	for (i = 0; i < FILE_EXT_MAX_COUNT; i ++)
	{
		bFlag = TRUE;
		for (j = 0; j < stExtension->bExtLength; j ++)
		{
			c = bExt[j];
			if (c >= 0x61 && c <= 0x7a)
			{
				c -= 0x20;	// To upper case
			}
			if (c != stExtension->pbExt[j])
			{
				bFlag = FALSE;
				break;
			}
		}

		if (bFlag == TRUE)
		{
			return stExtension->bFileType;
		}
		stExtension ++;
	}

	return FILE_EXT_NULL;
}

/*
 *  @ingroup  AUDIOAPI
 *  @brief    Enable sound of left channel or right channel
 *  @param    left_temp  Enable left channel
 *  @param    right_temp Enable right channel
 *  If it only has one channel, please set left_temp = 0 and right_temp =0; 
 */
void Api_audio_channel_select(int left_temp,int right_temp)
{
#if AUDIO_ON
	left_channel = left_temp;
	right_channel = right_temp;
#endif	
}

void Api_Audio_getID3_pic(int *pic_size, int *offset, STREAM *handle)
{
#if AUDIO_ON
	char buf[64]={0};
	int file_size;
	unsigned char *header;
	unsigned char *id3buf;

	Media_data.dec_fp = handle;
	file_size = FileSizeGet(handle);

	header = (unsigned char*)mem_malloc(10);
	header = (char *)((DWORD)header | 0xa0000000);
	id3buf = (unsigned char*)mem_malloc(4096);
	id3buf = (char *)((DWORD)id3buf | 0xa0000000);
	AO_AV = AO_TYPE;
#if (MP3_SW_AUDIO && AUDIO_ON)
	*offset = MagicPixel_mp3_id3info(8, &buf[0], pic_size, file_size, (char*) header, (char*) id3buf);
#endif
	mem_free(id3buf);
	mem_free(header);
#endif	
}

void MagicPixel_audio_channel_process(int left_temp,int right_temp)
{
#if AUDIO_ON
	int i,val;
	int *pcm_ptr0,*pcm_ptr1;
	if (Media_data.frame_size== 0)
	{
//     mpDebugPrint("Frame_size is 0\n");
		return;
	}
	pcm_ptr0 = Media_data.pcm4_buf[0];
	pcm_ptr1 = Media_data.pcm4_buf[1];

	if (left_channel && !right_channel)
	{

		for (i = 0; i < Media_data.frame_size; i++)
		{
			val = *pcm_ptr0++;
			*pcm_ptr1 = val;
			*pcm_ptr1 ++;
		}

	}

	if (right_channel && !left_channel)
	{

		for (i = 0; i < Media_data.frame_size; i++)
		{
			val = *pcm_ptr1++;
			*pcm_ptr0 = val;
			*pcm_ptr0 ++;
		}
	}
	if (!right_channel && !left_channel)
	{
		for (i = 0; i < Media_data.frame_size; i++)
		{
			val = *pcm_ptr1/2 + *pcm_ptr0/2;
			*pcm_ptr0 = val;
			*pcm_ptr1 = val;
			*pcm_ptr0 ++;
			*pcm_ptr1 ++;
		}		
	}
#endif	
}


///////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////
static OS_INTERRUPT_CALLBACK_FUNC aiuDmaCallBackFunction = NULL;

static void aiuDmaIsr(void)
{
	if (aiuDmaCallBackFunction)
	{
		aiuDmaCallBackFunction();
#if 0	// Mark this feature temporarily by C.W
		if (doAudioBufRecycle())
		{
			EventSet(MOVIE_EVENT, EVENT_START_AUDIODECODE);
		}
#endif
	}
	else
		MP_ALERT("aiuDmaISR, callback function null!");
}

void AiuDmaRegCallBackFunc(void (*AiuDmaCallBack_function) (void))
{
	DmaIntHandleRegister(IM_AIUDM, aiuDmaIsr);
	aiuDmaCallBackFunction = AiuDmaCallBack_function;
}

void AiuDmaClearCallBackFunc(void)
{
	DmaIntDis(IM_AIUDM);
	aiuDmaCallBackFunction = NULL;
}

DWORD Api_MovieStatusGet(void)
{
	return g_psSystemConfig->sVideoPlayer.dwPlayerStatus;
}

/*=========================================*/
/* recording                                                                                */
/*=========================================*/
#if RECORD_AUDIO
int Api_AudioRecording(record_argument *p)
{
    BYTE a = AUDIO_REC_OP_RECORDING_START;
    record_argument currPara;
    DWORD RecErrEvent;
    currPara = RecordArgumentGet();
    currPara.handle = p->handle;
    currPara.driveIndex = p->driveIndex;
    RecordArgument(&currPara);

    MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
    EventWait(REC_ERR_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &RecErrEvent);

    if (RecErrEvent & BIT0)
    {
        mpDebugPrint("Recording-ok");

        return PASS;
    }

    mpDebugPrint("Recording-fail");

    return -1;
}



int Api_StopAudioRecording(void)
{
    BYTE a = AUDIO_REC_OP_RECORDING_STOP;
    DWORD RecErrEvent;
    //mpDebugPrint("stop_rec");

    MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
    EventWait(REC_ERR_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &RecErrEvent);

    if(RecErrEvent & BIT0)
    {
        mpDebugPrint("StopRecording---BIT0---");
        return PASS;
    }

    mpDebugPrint("StopRecording---BIT1---");
    return -1;
}
#endif

#if RECORD_ENABLE

int Api_VideoRecording(record_argument *p)
{
    BYTE a = VIDEO_REC_OP_RECORDING_START;
    record_argument currPara;
    DWORD RecErrEvent;
    currPara = RecordArgumentGet();
    currPara.handle = p->handle;
    currPara.driveIndex = p->driveIndex;
    currPara.RecordingAudio = p->RecordingAudio;
    RecordArgument(&currPara);

    MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
    EventWait(REC_ERR_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &RecErrEvent);

    if (RecErrEvent & BIT0)
    {
        mpDebugPrint("Recording-ok");

        return PASS;
    }

    mpDebugPrint("Recording-fail");

    return -1;
}



int Api_StopVideoRecording(void)
{
    BYTE a = VIDEO_REC_OP_RECORDING_STOP;
    DWORD RecErrEvent;
    //mpDebugPrint("stop_rec");

    MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
    EventWait(REC_ERR_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &RecErrEvent);

    if(RecErrEvent & BIT0)
    {
        mpDebugPrint("StopRecording---BIT0---");
        return PASS;
    }

    mpDebugPrint("StopRecording---BIT1---");
    return -1;
}

int Api_VideoRecordingPause(void)
{
	BYTE a = VIDEO_REC_OP_RECORDING_PAUSE;
		DWORD RecErrEvent;
		MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
		EventWait(REC_ERR_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &RecErrEvent);

   if(RecErrEvent & BIT0)
    {
        mpDebugPrint("RecordingPause-ok---");
        return PASS;
    }

    mpDebugPrint("RecordingPause-Fail---");
    return -1;
}
int Api_VideoRecordingResume(void)
{
	BYTE a = VIDEO_REC_OP_RECORDING_RESUME;
	DWORD RecErrEvent;
	MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
	EventWait(REC_ERR_EVENT, 0xFFFFFFFF, OS_EVENT_OR, &RecErrEvent);
   if(RecErrEvent & BIT0)
    {
        mpDebugPrint("RecordingResume-ok---");
        return PASS;
    }

    mpDebugPrint("RecordingResume-Fail---");
    return -1;
}
BYTE Api_SetRecordVolume(WORD ivol)
{
#if AUDIO_ON
	MX6xx_AudioSetVolume(ivol);
#endif
	return PASS;
}
#endif

#if RECORD_ENABLE||SENSOR_WITH_DISPLAY
SDWORD Api_VideoRecordingPreviewStart(record_argument *p)//ST_IMGWIN * trgWin,int PathSelect)
{
    BYTE a = VIDEO_REC_OP_PREVIEW_START;
    // mpDebugPrint("start_sen");
    DWORD RecErrEvent;

    RecordArgument(p);
    MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
    EventWait(REC_ERR_EVENT, BIT0 | BIT1, OS_EVENT_OR, &RecErrEvent);
    if(RecErrEvent & BIT0)
    {
        mpDebugPrint("SensorPreview---BIT0---");
        return PASS;
    }

    mpDebugPrint("SensorPreview---BIT1---");
    return -1;
}



SDWORD Api_VideoRecordingPreviewStop(void)
{
    BYTE a = VIDEO_REC_OP_PREVIEW_STOP;
    //mpDebugPrint("stop_sen");
    DWORD RecErrEvent;

    MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
    EventWait(REC_ERR_EVENT, BIT0 | BIT1, OS_EVENT_OR, &RecErrEvent);

    if(RecErrEvent & BIT0)
    {
        mpDebugPrint("StopSensor---BIT0---");
        return PASS;
    }

    mpDebugPrint("StopSensor---BIT1---");
    return -1;
}
#endif


typedef DWORD (*AMRRECORD_CALLBACK_FUNC)(BYTE *record_buf,DWORD buf_len);

SBYTE Api_AmrRecordEnable(DWORD (*AMRRECORD_CALLBACK_FUNC)(BYTE *record_buf,DWORD buf_len))
{
#if AUDIO_ON
	return Audio_OpenInputDevice(8000,1,2,AMRRECORD_CALLBACK_FUNC,1024,1);
#else
	return NULL;
#endif	
}
SBYTE Api_AmrRecordDisable()
{
#if AUDIO_ON
	return Audio_CloseInputDevice();
#else
	return NULL;
#endif
}


void Api_Wav_Sound_play(BYTE *src,DWORD len)
{
#if AUDIO_ON
	WAV_CONFIG WavCfg;
	DWORD StartTime,EndTime;
	PCM_DATA_BLOCK PcmBufer1;
	PcmBufer1.start = src;
	PcmBufer1.size = len;

	if(DecodeWavHader1(&WavCfg, &PcmBufer1))
	{
		mpDebugPrint(" -E-  Sound header error");
		return;
	}

	mpDebugPrint("Sample rate: %d, SampleSize: %d, Channels: %d",
		WavCfg.SampleRate, WavCfg.BitsPerSample, WavCfg.Channels);

#if (AUDIO_DAC != DAC_INTERNAL) //reset Audio Internal Unit (AIU) to default settings
		BIU 	  *biu;
		biu = (BIU *) (BIU_BASE);
		biu->BiuArst &= 0xfffffdff;
		biu->BiuArst |= 0x00000200;
#else // (AUDIO_DAC == DAC_INTERNAL)
	PcmChunkCounter = 1;
#endif

	Audio_OpenOutputDevice(WavCfg.SampleRate, WavCfg.Channels, WavCfg.BitsPerSample/8,0);
	EndTime = (len-44)/(WavCfg.Channels*WavCfg.BitsPerSample/8)*1000/WavCfg.SampleRate;
	mpDebugPrint("EndTime %d",EndTime);
	WaveOutProc(src + 44, len - 44);
		
	StartTime = GetSysTime();
	while(WaveOutWriteDone()<0)
	{
		if(SystemGetElapsedTime(StartTime) > EndTime-32)
		{
			MX6xx_AudioPause();
			break;
		}
		TaskYield();
	}
	PcmChunkCounter = 0;
	Audio_CloseOutputDevice();
#endif	
}

extern av_player_data_t av_player_data;
void API_SetAudioTimeStamp (int32_t av_time)
{
#if (AUDIO_ON || VIDEO_ON)

		av_player_data.av_timer_pts = av_time;

#endif
}

int API_SetVideoSpeed (int speedrate)
{
#if PURE_VIDEO
	if(speedrate>10)
	{
		mpDebugPrint("##ERROR parameter speedrate can be not over 10 in API_SetVideoSpeed");
		return -1;
	}

	if(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
	{
		
		set_video_fps=speedrate/10;
		mpDebugPrint("SetVideoSpeed=%d/10",speedrate);
		return 0;
	}

#endif
#if (AUDIO_ON )//|| VIDEO_ON
int rate=0;
	if(speedrate>10)
	{
		mpDebugPrint("##ERROR parameter speedrate can be not over 10 in API_SetVideoSpeed");
		return -1;
	}
	if(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
	{
		if(audio_samplerate<8000)
		{
			mpDebugPrint("##ERROR not support audio samplerate less 8kmhz in API_SetVideoSpeed");
			return -1;
		}
		rate=(audio_samplerate *speedrate)/10;
		MX6xx_AudioConfigSampleRate(rate);
		mpDebugPrint("SetVideoSpeed=%d/10",speedrate);

	return 0;
	}
	else
	{
		mpDebugPrint("##ERROR API_SetVideoSpeed_function can be used in only play video state");
		return -1;
	}

#endif
}


