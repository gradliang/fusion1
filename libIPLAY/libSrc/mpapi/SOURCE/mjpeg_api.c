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
* Filename      : mjpeg_api.c
* Programmer(s) :
* Created       :
* Descriptions  :
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
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
#include "mpTrace.h"
#include "mjpeg_api.h"
#include "av.h"
#include "fs.h"
#include "taskid.h"
#include "display.h"
#include "os.h"
#include "mjpeg_define.h"
#include "ui.h"

//#if MJPEG_ENABLE

//#if MP3_MAD_AUDIO
audio_decoder_t *MJ_audio_decoder = 0x0;
sh_audio_t *MJ_Sh;
extern BYTE video_flag;
//#endif

int BT_len;
extern BYTE MJ;
extern void *BitStream_Source;

//#endif

/*
// Constant declarations
*/
BOOL boMovieTotalTime=0;
/*
// Variable declarations
*/
ST_MJPEG_CONTROL_PARAMETER stMjpegCtrl;
const ST_MJPEG_CONTROL_FUNC stMjpegCtrlFuncTable[MJPEG_MAX_COUNT] =
{
    { Avi_TypeCheck, Avi_DisplayFirstFrame, Avi_playInit, Avi_AudioChunkConnect, Avi_ScreenRefresh, Avi_DataLoad, Avi_Play, Avi_Stop, Avi_Pause, Avi_Resume, Avi_Shift, Avi_VideoDecode, Avi_ReleaseBuf},
    { Mov_TypeCheck, Mov_DisplayFirstFrame, Mov_playInit, Mov_AudioChunkConnect, Mov_ScreenRefresh, Mov_DataLoad, Mov_Play, Mov_Stop, Mov_Pause, Mov_Resume, Mov_Shift, Mov_VideoDecode, Mov_ReleaseBuf}
};

BOOL g_mjpeg_audio_enable;

/*
// Static function prototype
*/
static void MjpegDataIoTask (void);

/*
// Definition of internal functions
*/
SDWORD MjpegTaskInit (void)
{
    SDWORD sdwRetVal;

    sdwRetVal = MSG_NO_ERR;
    sdwRetVal = EventCreate(MJPEG_DATAIO_EVENT, (OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR), 0);
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- event create fail");
        return sdwRetVal;
    }
    sdwRetVal = TaskCreate(MJPEG_DATAIO_TASK, MjpegDataIoTask, CONTROL_PRIORITY, 0x4000);
    if (sdwRetVal != 0)
    {
        MP_DEBUG("-E- TaskCreate fail");
        return sdwRetVal;
    }
    sdwRetVal = TaskStartup(MJPEG_DATAIO_TASK);
    if (sdwRetVal != MSG_NO_ERR)
    {
        MP_DEBUG("-E- TaskStartup fail");
        return sdwRetVal;
    }
    return sdwRetVal;
}

SWORD MjpegPlayerAudioFillBlock (BYTE bType, DWORD* DwReadsize,DWORD *bwordalign)
{
    SWORD ret = 0;
    if(bType == MJPEG_AVI)
       ret = Avi_AudioFillBlock(DwReadsize,bwordalign);
	else
		mpDebugPrint("We should implement a MOV callback function @_@");

    return ret;
}

static BYTE *g_pbSource2;
static STREAM Mjpeg_AHandle;

SWORD MjpegPlayerOpen (
	STREAM *stHandle,
	BYTE bType,
	ST_IMGWIN *stDispWin,
	DWORD dwDispX,
	DWORD dwDispY,
	DWORD dwDispWidth,
	DWORD dwDispHeight,
	DWORD dwCmd)
{
    MP_DEBUG("MjpegPlayerOpen");
    if ((stHandle == NULL) || (bType >= MJPEG_MAX_COUNT) || (stDispWin == NULL))
    {
        MP_DEBUG("-E- input parameter fail");
        return MSG_INPUT_ERROR;
    }

#if MJPEG_AUDIO_ENABLE
    g_mjpeg_audio_enable = TRUE;
#else
    g_mjpeg_audio_enable = FALSE;
#endif
    MP_DEBUG("stHandle=0x%x,shandle=0x%x",stHandle,stHandle);
    MP_TRACE_LINE();
    stMjpegCtrl.dwCurrentMode = MJPEG_IDEL_MODE;
    stMjpegCtrl.swStatus = MSG_NO_ERR;
    stMjpegCtrl.dwCommand = dwCmd;
    stMjpegCtrl.dwDispalyX = dwDispX;
    stMjpegCtrl.dwDisplayY = dwDispY;
    stMjpegCtrl.dwDisplayWidth = dwDispWidth;
    stMjpegCtrl.dwDisplayHeight = dwDispHeight;
    stMjpegCtrl.pdwDispalyBuffer = stDispWin->pdwStart;
    stMjpegCtrl.dwOffsetValue = stDispWin->dwOffset;
    stMjpegCtrl.stFileHandler = stHandle;
    stMjpegCtrl.dwParameter = 0;
    MjpegSwapDispSize ();
    stMjpegCtrl.stMJpegCtrlFunc = (ST_MJPEG_CONTROL_FUNC *)&stMjpegCtrlFuncTable[bType];
    MP_TRACE_LINE();
//    ext_mem_init();


	DWORD dwSourceSize;
	BYTE  *pbSource;
	int   iErrorCode;
	dwSourceSize = 128 * 1024; // try it by change from 64KB to 128KB

    //For Video stream
    pbSource = (BYTE *)ImageAllocSourceBuffer(dwSourceSize+32);

	stMjpegCtrl.MPX_Stream= (ST_MPX_STREAM *)ext_mem_malloc(sizeof(ST_MPX_STREAM));
	iErrorCode = mpxStreamOpen(stMjpegCtrl.MPX_Stream, stHandle, pbSource,dwSourceSize, 0,STREAM_SOURCE_FILE);
    MP_DEBUG2("psStream=0x%x, iErrorCode=%d",stMjpegCtrl.MPX_Stream, iErrorCode);

	stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MjpegTypeCheck();

	if (stMjpegCtrl.swStatus != MSG_NO_ERR)
	{
		mpDebugPrint("-I- not support format stMjpegCtrl.swStatus=%d", stMjpegCtrl.swStatus);
		return stMjpegCtrl.swStatus;
	}

    DWORD dwTargetSize = 848 * 480 * 2 * 2;  //SystemGetMemSize(DISPLAY_BUF1_MEM_ID);
    ST_JPEG  *psJpeg ;
    psJpeg = (ST_JPEG *)Mjpeg_Decode_Init(stMjpegCtrl.MPX_Stream,stDispWin->pdwStart, dwTargetSize);
    stMjpegCtrl.psJpeg=psJpeg;


    //For Audio stream
	memcpy(&Mjpeg_AHandle, stHandle, sizeof(STREAM));
	DriveHandleCopy(DriveGet(MAX_DRIVE_NUM), DriveGet(DriveCurIdGet()));
	Mjpeg_AHandle.Drv = DriveGet(MAX_DRIVE_NUM);

	g_pbSource2 = (BYTE *)ext_mem_malloc(dwSourceSize + 32);//ImageAllocSourceBuffer(dwSourceSize+32);

	stMjpegCtrl.MPX_Stream_a= (ST_MPX_STREAM *)ext_mem_malloc(sizeof(ST_MPX_STREAM));
    iErrorCode = mpxStreamOpen(stMjpegCtrl.MPX_Stream_a, &Mjpeg_AHandle, g_pbSource2, dwSourceSize, 0, STREAM_SOURCE_FILE);
	MP_DEBUG2("psStream=0x%x, iErrorCode=%d",stMjpegCtrl.MPX_Stream_a, iErrorCode);


    switch(stMjpegCtrl.dwCommand)
    {
        case CMD_DECODE_FIRST_FRAME:
            stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MjpegDisplayFirst();
            //MjpegPlayerClose();
            break;

        case CMD_NORMAL_FORWARD:
            stMjpegCtrl.dwCurrentMode = MJPEG_PLAY_MODE;
            stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MjpegInitial();
            SWORD swStatus = stMjpegCtrl.swStatus;
            MP_TRACE_LINE();
            switch(stMjpegCtrl.swStatus)
            {
                case MSG_NO_ERR:
                    Tm2RegisterCallBackFunc(stMjpegCtrl.stMJpegCtrlFunc->MJpegVideoPlay);
#if MJPEG_AUDIO_ENABLE
                    AiuDmaRegCallBackFunc(stMjpegCtrl.stMJpegCtrlFunc->MJpegAudioPlay);
#endif
                    MP_TRACE_LINE();
                    break;

                case MSG_NOT_SUPPORT_FORMAT:
                    MP_TRACE_LINE();
                    break;

                default:
                    MjpegPlayerClose();
                    MP_TRACE_LINE();
					return swStatus;
            }
            break;

        default:
            MP_DEBUG("-E- command fail");
            MjpegPlayerClose();

			return MSG_CMD_ERROR;
    }

#if (!MP3_SW_AUDIO) && (!MP3_MAD_AUDIO)
	video_flag = 1;
#endif

	TaskWakeup(MJPEG_DATAIO_TASK);
    MP_TRACE_LINE();
    return stMjpegCtrl.swStatus;
}

SWORD MjpegSwapDispSize (void)
{
    ST_IMGWIN *pWin;

    if  (g_psSystemConfig->sVideoPlayer.isFullScreen == SETUP_MENU_OFF)  //Check currently setting for video full screen mode
    {
        stMjpegCtrl.dwParameter &= ~MJPEG_FULL_SCREEN;
    }
    else
    {
        stMjpegCtrl.dwParameter |= MJPEG_FULL_SCREEN;
    }
    stMjpegCtrl.dwParameter |= MJPEG_SCREEN_CLEAR;
    return MSG_NO_ERR;
}

SWORD MjpegShowFirstFrame (void)
{
    if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
    {
        EventClear(MJPEG_DATAIO_EVENT, 0);
        stMjpegCtrl.dwCommand = CMD_NORMAL_FORWARD;
        stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MjpegInitial();
    }
    return MSG_NO_ERR;
}

SWORD MjpegPlayerForward (void)
{
    if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
    {
        stMjpegCtrl.dwCommand = CMD_FORWARD;
        EventSet(MJPEG_DATAIO_EVENT, MJPEG_DATAIO_EVENT_SHIFT);
    }
    return MSG_NO_ERR;
}

SWORD MjpegPlayerBackward (void)
{
    if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
    {
        stMjpegCtrl.dwCommand = CMD_BACKWARD;
        EventSet(MJPEG_DATAIO_EVENT, MJPEG_DATAIO_EVENT_SHIFT);
    }
    return MSG_NO_ERR;
}

SWORD MjpegPlayerClose (void)
{
    MP_DEBUG("MjpegPlayerClose");
	MP_DEBUG1("mem_get_free_space() = %d", mem_get_free_space());
	MP_DEBUG1("ext_mem_get_free_space() = %d", ext_mem_get_free_space());
	register ST_TASK *Task;

	EventClear(MJPEG_DATAIO_EVENT, 0);

	stMjpegCtrl.stMJpegCtrlFunc->MJpegRleaseBuf();

    MP_TRACE_LINE();
    AiuDmaClearCallBackFunc();
    Tm2ClearCallBackFunc();

#if (SC_USBDEVICE)
    if (SystemCheckUsbdPlugIn())
        TaskYield();
    else
#endif
    TimerDelay(1);

    Mjpeg_Decode_Close((ST_MPX_STREAM *)MjpegGetMPXStream());
    mpxStreamClose((ST_MPX_STREAM *)MjpegGetMPXStream());
    mpxStreamClose((ST_MPX_STREAM *)MjpegGetMPXStream_a());

    ImageReleaseSourceBuffer();
    ext_mem_free(g_pbSource2);


    FileClose(MjpegGetFileHandle());
    //Mem_Init_with_Audio_Buff();
//    ext_mem_init();
#if (SC_USBDEVICE)
    if (SystemCheckUsbdPlugIn())
        TaskYield();
    else
#endif
    TimerDelay(1);

    memset(&stMjpegCtrl, 0x0, sizeof(ST_MJPEG_CONTROL_PARAMETER));

#if (AUDIO_DAC == DAC_AK4387)
	AudioClose_AK4387();
#endif

//    #if  MJPEG_MP3

    #if (MP3_HW_CODEC)  //#if (! MP3_SW_VIDEO) && (! MP3_MAD_VIDEO)
    video_flag = 0;
    //if (MJ_audio_decoder->audio_decoder.uninit)
	if (MJ_audio_decoder)
	{
		MJ_audio_decoder->audio_decoder.uninit(MJ_Sh);

		MPA			*mpa;
		CHANNEL		*mpardma, *mpawdma;
		mpa = (MPA *) MPA_BASE;
		mpardma = (CHANNEL *) DMA_MPAR_BASE;
		mpawdma = (CHANNEL *) DMA_MPAW_BASE;
		mpardma->Control = 0;
		mpawdma->Control = 0;
		mpa->MpaCtrl = 0x100;
		mpa->MpaCtrl = 0x10;

		extern int n_frames;
		extern int Layer3_Inited;
		extern int Faad_Inited;
		extern int Layer1_Inited;
		extern int Layer2_Inited;

		n_frames = 0;
		Layer3_Inited = 0;
//		Faad_Inited = 0;
		Layer1_Inited = 0;
		Layer2_Inited = 0;
		mSetMpaCks(MPACKS_PLL1_DIV_2);
		mClrIntsram0Cks ();
		mClrIntsram1Cks ();
		mSetIntsram0Cks (INTSRAM2CKS_CPU_CLK);
		mSetIntsram1Cks(INTSRAM2CKS_CPU_CLK);
		mem_free(MJ_audio_decoder);
		mem_free(MJ_Sh);
	}

	MJ_audio_decoder = 0x0;
	#endif

    MJ = 0;
//    #endif

    MP_DEBUG1("mem_get_free_space() = %d", mem_get_free_space());
	MP_DEBUG1("ext_mem_get_free_space() = %d", ext_mem_get_free_space());
    return MSG_NO_ERR;
}

SWORD MjpegPlayerPlay (void)
{
    if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
    {
        EventClear(MJPEG_DATAIO_EVENT, 0);
        stMjpegCtrl.stMJpegCtrlFunc->MJpegPlay();
    }
    return MSG_NO_ERR;
}

SWORD MjpegPlayerPause (void)
{
    if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
    {
        stMjpegCtrl.dwCommand = CMD_PAUSE;
        stMjpegCtrl.stMJpegCtrlFunc->MJpegPause();
    }
    return MSG_NO_ERR;
}

SWORD MjpegPlayerResume (void)
{
	if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
	{
		stMjpegCtrl.dwCommand = CMD_NORMAL_FORWARD;
		stMjpegCtrl.stMJpegCtrlFunc->MJpegResume();
	}

	return MSG_NO_ERR;
}

SWORD MjpegPlayerStop (void)
{
    if (stMjpegCtrl.dwCurrentMode == MJPEG_PLAY_MODE)
    {
        stMjpegCtrl.dwCommand = CMD_PAUSE;
        stMjpegCtrl.stMJpegCtrlFunc->MJpegStop();
    }
    return MSG_NO_ERR;
}

void MjpegPlayerDispTotalTime (DWORD dwTotalSeconds)
{
#if 0// OSD_ENABLE
    BYTE bPrintBuf[10], bDispSec, bDispMin, bDispHour;

    bDispSec = dwTotalSeconds % 60;
    bDispMin = ((dwTotalSeconds - bDispSec) / 60) % 60;
    bDispHour = ((dwTotalSeconds - bDispSec) / 60) / 60;

    mp_sprintf(bPrintBuf, "%d:%02d:%02d", bDispHour, bDispMin, bDispSec);
    Idu_OsdPaintArea(MOVIE_DISPLAY_TOTAL_TIME_START_X, MOVIE_DISPLAY_TOTAL_TIME_START_Y, MOVIE_DISPLAY_TOTAL_TIME_WIDTH, MOVIE_DISPLAY_TOTAL_TIME_HEIGHT, 15);
	Idu_OSDPrint(Idu_GetOsdWin(),bPrintBuf, MOVIE_DISPLAY_TOTAL_TIME_START_X, MOVIE_DISPLAY_TOTAL_TIME_START_Y, OSD_COLOR_GREEN);
#endif
}

void MjpegPlayerDispTime (DWORD dwFrameCount, DWORD dwFrameRate, BYTE blRedraw)
{
#if 0
    if ((dwFrameCount % dwFrameRate == 0) || (blRedraw == TRUE))
    {
        Idu_OsdPaintArea(MOVIE_DISPLAY_TIME_START_X, MOVIE_DISPLAY_TIME_START_Y, MOVIE_DISPLAY_TIME_WIDTH, MOVIE_DISPLAY_TIME_HEIGHT, 15);
        Idu_PrintMovieTime((dwFrameCount / dwFrameRate));
    }
#endif
}

/*
// Definition of local functions
*/
static void MjpegDataIoTask (void)
{
    DWORD dwEvent, dwTempCommand, dwStartTime;
    DWORD i;

    MP_DEBUG("-I- MjpegDataIoTask init");
    // initial all parameter
    memset(&stMjpegCtrl, 0x0, sizeof(ST_MJPEG_CONTROL_PARAMETER));
//    TaskSleep();
	MP_TRACE_LINE();
    while(1)
    {

        EventWait(MJPEG_DATAIO_EVENT, 0xffffffff, OS_EVENT_OR, &dwEvent);
        MP_DEBUG1("dwEvent=0x%x",dwEvent);
        if (dwEvent & MJPEG_DATAIO_EVENT_STOP)
        {
        	g_psDmaAiu->Control = 0;
            EventClear(MJPEG_DATAIO_EVENT, 0);
            stMjpegCtrl.dwCurrentMode = MJPEG_PLAY_MODE;
            EventSet(UI_EVENT, EVENT_VIDEO_END);
            //MjpegPlayerClose();
            MP_DEBUG("Video End");
            continue;
        }

        if (dwEvent & MJPEG_DATAIO_EVENT_SHIFT)
        {
            MP_DEBUG("*************************shift event");
            MP_TRACE_LINE();
			g_psDmaAiu->Control = 0;

			//clear current deque event(pop up from  task_list)
			//though perform well, withdraw all MJPEG_DATAIO_EVENT is suggested
			EventClear(MJPEG_DATAIO_EVENT, 0);
			stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MJpegShift();
            stMjpegCtrl.dwCommand = CMD_NORMAL_FORWARD;

            stMjpegCtrl.stMJpegCtrlFunc->MJpegPlay();
            continue;
        }

		if (dwEvent & MJPEG_DATAIO_EVENT_LOAD_DATA)
		{
			if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY)
			{
				MP_DEBUG("******** DataLoad ********");
				MP_TRACE_LINE();
				stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MJpegDataLoad();
				TaskYield();
			}
		}

        if(dwEvent & MJPEG_DATAIO_EVENT_PLAY_VIDEO)
		{
			MP_DEBUG("****************VideoDecode****************");
			MP_TRACE_LINE();
			stMjpegCtrl.swStatus = stMjpegCtrl.stMJpegCtrlFunc->MJpegVideoDecode();
		}

		TaskYield();
    }
}


//----------------------------------  Motion Jpeg Audio Api -----------------------------------------------------
#if MP3_SW_AUDIO
extern Audio_dec Media_data;
static BYTE fending;					//Is this due to any problem? checking!!!		C.W 080930
#elif MP3_MAD_AUDIO
extern ad_functions_t mpcodecs_ad_libmad;
extern audio_decoder_t *MJ_audio_decoder;
extern sh_audio_t *MJ_Sh;
#endif

void MotionJpeg_AudioDevInit(WORD FormatTag)
{
#if MP3_SW_AUDIO
    fending =0;
#elif MP3_MAD_AUDIO
//    mem_init();

	if(FormatTag == 0x55)	//MP3 case
	{
		MJ_audio_decoder = (audio_decoder_t *)mem_malloc(sizeof(audio_decoder_t));
		MJ_Sh = (sh_audio_t *)mem_malloc(sizeof(sh_audio_t));

		memset(MJ_audio_decoder, 0, sizeof(audio_decoder_t));
		memset(MJ_Sh, 0, sizeof(sh_audio_t));

		MJ_audio_decoder->audio_decoder = mpcodecs_ad_libmad;
		MJ_audio_decoder->audio_decoder.preinit(MJ_Sh);

		if (MJ_Sh->audio_in_minsize > 0)
		{
			MJ_Sh->a_in_buffer_size = MJ_Sh->audio_in_minsize;
			MJ_Sh->a_in_buffer = (BYTE *)((DWORD)( mem_malloc(MJ_Sh->a_in_buffer_size)) | 0xa0000000);
			MP_DEBUG1("MJ_Sh.a_in_buffer_size=%d", MJ_Sh->a_in_buffer_size);
			memset(MJ_Sh->a_in_buffer, 0, MJ_Sh->a_in_buffer_size);
			MJ_Sh->a_in_buffer_len = 0;
		}

		MJ_Sh->a_buffer_size = MJ_Sh->audio_out_minsize + 65536;	/* worst case calc. */
		MJ_Sh->a_buffer_len = 0;
	}
#endif

	BT_len=0;
}

SWORD Mjpeg_MP3Format_DataLoad(BYTE *pbWavPointer, BYTE *pbOutWavPointer,
										 DWORD AUDIO_BUF_SIZE, BYTE *tmp_pcm, int* pcm_len_total,
										 char *song_info, char *song_track, int *mpeg2, int *layer)
{
	short intRet = 0;

#if MP3_SW_AUDIO
	if (fending)
		return MSG_AVI_TAG_SEARCH_FAIL;

	if(Media_data.frame == 0)
	{
		BitStream_Source = (BYTE *)((DWORD)(pbWavPointer) | 0xa0000000);
		MP_DEBUG1("Media_data.file_size=%d",Media_data.file_size);

		intRet = MagicPixel_mp3_init(Media_data.file_size, 0, &Media_data.ch, &Media_data.srate, &Media_data.frame_size, &Media_data.bitrate, &Media_data.total_time,
									 &song_info, &song_info, &song_info,&song_info, &song_info, &song_info, &song_track, &mpeg2, &layer);

		setWaveFormatInfo(Media_data.ch, Media_data.srate, Media_data.srate);
		MP_DEBUG3("Media_data.ch=%d,Media_data.srate=%d,Media_data.bitrate=%d",Media_data.ch,Media_data.srate,Media_data.bitrate);
	}

	tmp_pcm = pbOutWavPointer;
	*pcm_len_total = 0;

	while( *pcm_len_total + 4608 <= AUDIO_BUF_SIZE)
	{
		MP_DEBUG1("pcm_len_total%d", *pcm_len_total);
		intRet = MagicPixel_mp3_decode(1, 0, Media_data.pcm4_buf, &Media_data.play_time, &song_info, &Media_data.ch);
		MP_TRACE_LINE();

		if(intRet)//eof
		{
			MP_TRACE_LINE();
			if(intRet == 0xe)
			{
				MP_TRACE_LINE();
				fending = 1;
				break;
			}

			//?? continue;
			//return MSG_AVI_TAG_SEARCH_FAIL;
		}

		Media_data.frame++;
		MP_TRACE_LINE();
		MagicPixel_mp3_post_process(Media_data.ch-1, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf, tmp_pcm);
		MP_TRACE_LINE();
		*pcm_len_total	+= 2 * Media_data.ch * Media_data.frame_size;	// short * channel * frame_size
		tmp_pcm			+= 2 * Media_data.ch * Media_data.frame_size;
	}

#elif MP3_MAD_AUDIO		//LIBMAD case

	if(MJ_Sh->inited==0)
	{
		MJ_Sh->inited = 1;
		MJ_audio_decoder->audio_decoder.init(MJ_Sh);

		MJ_Sh->channels		= getWaveFormat_Channel();
		MJ_Sh->samplerate	= getWaveFormat_SamplesPerSec();
		MJ_Sh->i_bps		= getWaveFormat_AvgBytesPerSec();

		BitStream_Source = (BYTE *)((DWORD)(pbWavPointer) | 0xa0000000);
		//MP_DEBUG3("stWaveFormat.wnChannels=%d,stWaveFormat.dwnSamplesPerSec=%d,stWaveFormat.dwnAvgBytesPerSec=%d", stWaveFormat.wnChannels, stWaveFormat.dwnSamplesPerSec, stWaveFormat.dwnAvgBytesPerSec);

		if(!MJ_Sh->channels || !MJ_Sh->samplerate)
		{
			if (MJ_Sh->a_in_buffer)
			mem_free(MJ_Sh->a_in_buffer);
			MJ_Sh->a_in_buffer = NULL;
			return MSG_INPUT_ERROR;
		}
	}

	tmp_pcm = pbOutWavPointer;

	intRet = MJ_audio_decoder->audio_decoder.decode_audio(MJ_Sh, tmp_pcm, AUDIO_BUF_SIZE, AUDIO_BUF_SIZE);

	if(intRet == -1)
	{
		MP_DEBUG("MJP libmad decode audio fail....");
		return MSG_AVI_TAG_SEARCH_FAIL;
	}

	*pcm_len_total = intRet;
#endif

	return intRet;
}

void Mjpeg_initAudioDec()
{
#if MP3_SW_AUDIO
	memset(&Media_data, 0, sizeof(Audio_dec));
#endif
}

SWORD MjpegGetMediaInfo(STREAM *sHandle, BYTE bType, ST_MEDIA_INFO* p_media_info)
{
    MP_DEBUG("MjpegGetMediaInfo");
	BYTE  *pbSource;
	int   iErrorCode;
	SWORD swStatus;

    // set stMjpegCtrl parameters
	stMjpegCtrl.stFileHandler = sHandle;
	stMjpegCtrl.stMJpegCtrlFunc = (ST_MJPEG_CONTROL_FUNC *)&stMjpegCtrlFuncTable[bType];

    // allocate video stream
    DWORD dwSourceSize = 128 * 1024;
	pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
	stMjpegCtrl.MPX_Stream= (ST_MPX_STREAM *)ext_mem_malloc(sizeof(ST_MPX_STREAM));
	iErrorCode = mpxStreamOpen(stMjpegCtrl.MPX_Stream, sHandle, pbSource, dwSourceSize, 0, STREAM_SOURCE_FILE);

    // type check
    swStatus = stMjpegCtrl.stMJpegCtrlFunc->MjpegTypeCheck();
	if (swStatus != MSG_NO_ERR)
	{
		mpDebugPrint("-I- not support format swStatus=%d", swStatus);
		return swStatus;
	}

	// get media information
	swStatus = stMjpegCtrl.stMJpegCtrlFunc->MJpegGetMediaInfo(sHandle, p_media_info);

	// release video stream
	ImageReleaseSourceBuffer();
    mpxStreamClose((ST_MPX_STREAM *)MjpegGetMPXStream());
	return swStatus;
}


#endif


