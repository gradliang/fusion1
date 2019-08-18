/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      video_decoder.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:
*
*
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#define DOUBLE_IDU_WIN 0

/*
// Include section
*/

#include "mpTrace.h"

#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "stheader.h"
#include "filter_graph.h"
#include "video_decoder.h"

#include "config_codec.h"

#include "display.h"
#include "taskid.h"

#include "pp.h"

static video_decoder_t _video_decoder;

extern DWORD TickCount0;
extern DWORD TickCount;
#if HANTRO_MPEG12_CODEC
extern vd_functions_t mpcodecs_vd_hantro_mpeg12;
#else
extern vd_functions_t mpcodecs_vd_mpeg2;
#endif
#if HANTRO_MPEG4_CODEC
extern vd_functions_t mpcodecs_vd_hantro_mpeg4;
#endif
#if HANTRO_H264_CODEC
extern vd_functions_t mpcodecs_vd_hantro_h264;
#endif
extern vd_functions_t mpcodecs_vd_xvid;

extern vd_functions_t mpcodecs_vd_ffmpeg;
extern vd_functions_t mpcodecs_vd_mjpg;
extern BYTE full_screen_mode;
#if ANTI_TEARING_ENABLE
extern BYTE frame_buffer_index;
extern BOOL buffer_switch, video_stop;
#endif
extern int resoultion_1;
extern int displayratio_open;
extern unsigned int HD720p;

int frame_num = 0;

TIMER *timer = (TIMER *) TIMER0_BASE;

#define GET_TIMER_START_VALUE   register int value13= TickCount0;\
							register int value12= timer->Tpc &0xff;\
							register int value11= timer->TmV&0xffff;

#define GET_TIMER_END_VALUE 	  register int value11= timer->TmV&0xffff;\
							  register int value12= timer->Tpc &0xff;\
							  register int value13= TickCount0;

//#define TIMER_CURRENT value1 + value2*0xc6c0 + value3*TickCount_PERIOD
//#define TIMER_RESULT  value1 + value2*0xc6c0 + value3*TickCount_PERIOD - temp_start
#define TIMER_CURRENT	value11 + value12*40000 + value13*7200000
#define TIMER_RESULT	value11 + value12*40000 + value13*7200000 - temp_start2

#define VIDEO_FRAME_COUNT 10
int decoder_initialized = 1;


//extern ST_IMGWIN g_extra_win;

#if MJPEG_ENABLE
BYTE g_bScalingFinish;
extern BYTE MJ_INIT;
extern DWORD g_dwVideoUpdateAddr;
#endif
void TurnOffMPVClk()
{
	register CLOCK *clock = (CLOCK *) (CLOCK_BASE);

	clock->MdClken &= ~BIT1;	// disable MPV clock
}

///
///@ingroup         group2_FilterG
///@defgroup        group_Vdecoder  Video Decoder
///
int MJPEG = 0;
int isH263 = 0;

//test code, do NOT remove
#if 0
static void Pixel_420Semi_Planer_2_420Planer(unsigned char * target, const unsigned char * source, unsigned int PixelDimension)
{
	unsigned char *tCb = target + PixelDimension;
	unsigned char *tCr = tCb + PixelDimension/4;
	const unsigned char *sCbCr = source + PixelDimension;
	int i=0;
	while (PixelDimension)
	{
		for (i=0; i<4; i++)	*target++ = *source++;
		*tCb++ = *sCbCr++;
		*tCr++ = *sCbCr++;
		PixelDimension-=4;
	}
}

/*!
@note	mpv decoder output may present bit swap format, need to disable to write out readable image
		Wrapper_reg0 &= ~0x1 in MPV_HW_Init
*/
int WriteOutFrameTest(const BYTE card, const unsigned char * frame, const int width, const int height)
{
	DRIVE * const sDrv = DriveGet(card);
	DirReset(sDrv);
	STREAM *shandle=NULL;
	char fname[30];
	char extname[]="yuv";
	const unsigned int len = width * height *1.5;
	sprintf(fname, "f_%dx%d", width, height);
	if (CreateFile(sDrv, fname, extname)==FS_SUCCEED)
		shandle=FileOpen(sDrv);
	if (shandle)
	{
		unsigned char * planer = (unsigned char*)mem_malloc(len);
		Pixel_420Semi_Planer_2_420Planer(planer, frame, width * height);

		if (FileWrite(shandle, planer, len)!=len)
			MP_ALERT("FileWrite failed");
		FileClose(shandle);
		mpDebugPrint("writing %s.%s completed", fname, extname);
		mem_free(planer);
		return 1;
	}
	else	MP_ALERT("FileOpen Failed");
	return 0;
}
#endif

///
///@ingroup group_Vdecoder
///@brief   This function will init video_decoder_t structure _video_decoder according the type of video stream.
///         Then call its init function _video_decoder.video_decoder.init(sh_video) to init the codec.
///
///@param   sh_video_t* sh_video        Video stream header structure including the parameters of  video stream.
///
///@return  video_decoder_t*        This pointer will be recorded in filter_graph_t structure.
///
int NotDecodeVideo =1;
video_decoder_t *init_best_video_codec(sh_video_t * sh_video)
{
	const unsigned int four_cc = sh_video->format;
	//Waiting for first time of video decode finish.
	//It prevents that the system crashes in Audio task(for *.mpg(MPEG-1 stream))
	//XianWen Chang note it 2010/10/04
	NotDecodeVideo = 1;

	frame_num = 0;
	MJPEG = 0;
	isH263=0;
	reset_video_decoder();
	switch (four_cc)
	{
#if VIDEO_ENABLE
	case MPEG1:
	case MPEG2:
	case DVR:  //not sure, robert
	case hdv2:
	case mpg1:
	case PIM1:
	case VCR2:
	case mpg2:
	case MPG2:
	case MPEG:
	case hdv3:
	case mx5p:
	case MMES:
	case mmes:
	{

#if HANTRO_MPEG12_CODEC
		//call hardware codec
		_video_decoder.video_decoder = mpcodecs_vd_hantro_mpeg12;
		break;

#endif

#ifdef SW_MPEG1_CODEC
//			if(sh_video->disp_w > 352 || sh_video->disp_h > 288) return NULL;
		_video_decoder.video_decoder = mpcodecs_vd_mpeg2;
//          _video_decoder.video_decoder=mpcodecs_vd_ffmpeg;
		break;
#endif

		//MP_DEBUG("-----------BugCheck!---------\r\n");
		//MP_DEBUG("We cann't find an appropriate MPEG1/2 codec for decoding\r\n");
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
	}
	case DIV3:
	case div3:
	case DIV4:
	case div4:
	case MP43:
	case mp43:
	{
#if (CHIP_VER_MSB == CHIP_VER_650)
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
#endif

#ifdef HW_DIVX3_CODEC
		//call hardware codec
		//MP_DEBUG("-----------BugCheck!---------\r\n");
		//MP_DEBUG("DIVX3 HW codec not ready yet!\r\n");
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
#endif

#ifdef SW_DIVX3_CODEC
//			if(sh_video->disp_w > 352 || sh_video->disp_h > 288) return NULL;
		_video_decoder.video_decoder = mpcodecs_vd_ffmpeg;
		break;
#endif

		//MP_DEBUG("-----------BugCheck!---------\r\n");
		//MP_DEBUG("We cann't find an appropriate DIVX3 codec for decoding\r\n");
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
	}
	case MPEG4_DX50:
	{
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
	}
	case MPEG4_XVID:
	case MPEG4_xvid:
	case MPEG4_DIVX:
	case MPEG4_divx:
	case mp4v:
	case MP4V:
	case M4S2:
	case s263:
	case h263:
	{
		if ((four_cc==h263)||(four_cc==s263))
			isH263=1;

#if HANTRO_MPEG4_CODEC
		_video_decoder.video_decoder = mpcodecs_vd_hantro_mpeg4;
		break;
#endif

#ifdef SW_XVID_CODEC
//       _video_decoder.video_decoder=mpcodecs_vd_ffmpeg;
//			if(sh_video->disp_w > 720 || sh_video->disp_h > 576) return NULL;
//			if(sh_video->mp4_profile > MPEG4_CODEC_PROFILE) return NULL;
		_video_decoder.video_decoder = mpcodecs_vd_xvid;
		break;
#endif

		//MP_DEBUG("-----------BugCheck!---------\r\n");
		//MP_DEBUG("We cann't find an appropriate XVID codec for decoding\r\n");
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
	}

	case h264:
	case H264:
	case x264:
	case X264:
	case avc1:
	case AVC1:
	case davc:
	case DAVC:
	case VCODEC_H264:
	{
#if HANTRO_H264_CODEC
		_video_decoder.video_decoder = mpcodecs_vd_hantro_h264;
		break;
#endif
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
	}

	case FLV1:
	{
#if HANTRO_MPEG4_CODEC
		_video_decoder.video_decoder = mpcodecs_vd_hantro_mpeg4;
		break;
#endif
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;
	}

#endif
#if MJPEG_ENABLE
	case JPEG:					// motion jpeg
	case jpeg:
	case MJPG:
	case mjpg:
	case dmb1:
	{
#if (CHIP_VER_MSB == CHIP_VER_615)
		TurnOffMPVClk();
#endif
		//UartOutText("init_best_video_codec , JPEG\r\n");
#if MJPEG_TOGGLE
#ifdef HW_MJPG_CODEC
		MJPEG = 1;
		break;
#endif
		SystemSetErrMsg(ERR_VIDEO_CODEC);
		return NULL;

//#elif VIDEO_ENABLE
#else
        extern BYTE g_mjpeg_decode_buf_num; 
        #if DISPLAY_VIDEO_FILLWALL_ENABLE
		g_mjpeg_decode_buf_num = 5;
		#else
		g_mjpeg_decode_buf_num = 2;
		#endif
		_video_decoder.video_decoder = mpcodecs_vd_mjpg;
		break;
#endif
	}
#endif
	default:
		MP_ALERT("Unknow FourCC 0x%x!", four_cc);
		return NULL;
	}

	if (_video_decoder.video_decoder.init)
	{
		if (sh_video->disp_w > MAX_VIDEO_DECODE_WIDTH || sh_video->disp_h > MAX_VIDEO_DECODE_HEIGHT || !_video_decoder.video_decoder.init(sh_video, VIDEO_FRAME_COUNT))
		{
			MP_ALERT("init codec failed! %s @ %d", __FILE__, __LINE__);
			return NULL;
		}
		else
			sh_video->inited = 1;
	}
	else
		return NULL;

	return &_video_decoder;
}

///
///@ingroup group_Vdecoder
///@brief   This function will check whether there is a video stream and its decoder, then call the decoder's change_mode function
///         to change display resolution and IPU mode.
///
///@param   filter_graph_t* graph   This structure is used to manage all the filers includeing demux, deocde, out filters.
///
///@return  No
///
void change_video_mode(filter_graph_t * graph)
{
	MP_DEBUG("change_video_mode");
	sh_video_t * const sh_video = (sh_video_t *) graph->demux->sh_video;


	if (sh_video)
	{
		if (graph->v_decoder->video_decoder.change_mode)
			graph->v_decoder->video_decoder.change_mode(sh_video);
	}

}
///@brief   This function will check whether there is a video stream and its decoder, then call the decoder's uninit function
///         to release the resources occupied by decoder.
///
///@param   filter_graph_t* graph   This structure is used to manage all the filers includeing demux, deocde, out filters.
///
///@return  No
///
void uninit_video_decode(filter_graph_t * graph)
{
	//mpDebugPrint("uninit_video");
	sh_video_t * const sh_video = (sh_video_t *) graph->demux->sh_video;

	DWORD release;
	EventWait(AV_DECODER_EVENT, EVENT_V_NOT_DECODING|EVENT_A_NOT_DECODING, OS_EVENT_AND, &release);

	if (sh_video)
	{
		if (graph->v_decoder->video_decoder.uninit)
			graph->v_decoder->video_decoder.uninit(sh_video);
		sh_video->inited = 0;
	}

	//Video_TurnOffMPVClk();
	//Video_TurnOffIPUClk();
	//mpDebugPrint("uninit_video completed");
}

int DisplayRatio(ST_IMGWIN * srcWin, ST_IMGWIN * dispWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2, WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE mode,int resolution)
{
	mpDebugPrint("*******************************");
	mpDebugPrint("DisplayRatio@@_resolution=%d",resolution);
	struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

	int ix=0,i=1;
	DWORD ratio=0,ratio_add=0;;
	WORD wSrcWidth, wSrcHeight;
	WORD wTrgWidth, wTrgHeight;
	WORD panle_width,panle_height;
	wSrcWidth=srcWin->wWidth;
	wSrcHeight=srcWin->wHeight;
	panle_width=pstScreen->wInnerWidth;
	panle_height=pstScreen->wInnerHeight;

	ix=0,i=10;

	switch (resolution)
	{
	case 0:
		dispWin->wWidth=pstScreen->wInnerWidth;
		dispWin->wHeight=pstScreen->wInnerHeight;
		break;
	case 1:		//origne size
		if ((wSrcWidth <= pstScreen->wInnerWidth) && (wSrcHeight <= pstScreen->wInnerHeight))
		{
			dispWin->wWidth=wSrcWidth;
			dispWin->wHeight=wSrcHeight;
			break;
		}
		if (wSrcWidth>=wSrcHeight)
		{
			ratio = FIX_POINT_D(wSrcWidth) / wSrcHeight;
			ratio_add=ratio/100;
			if ((ratio-(ratio_add*100))>=50)ratio_add++;
			for (i=20;; i=i+20)
			{
				if (ratio_add*i<pstScreen->wInnerWidth)
				{
					if (10*i<pstScreen->wInnerHeight)
					{
					}
					else
					{
						ix=i-20;
						break;
					}
				}
				else
				{
					ix=i-20;
					break;
				}
			}
			for (i=ix;; i++)
			{
				if (ratio_add*i<=pstScreen->wInnerWidth)
				{
					if (10*i<=pstScreen->wInnerHeight)
					{
					}
					else
					{
						ix=i-1;
						break;
					}
				}
				else
				{
					ix=i-1;
					break;
				}
			}
			dispWin->wWidth=ix*ratio_add;
			dispWin->wHeight=ix*10;
			break;
		}
		else
		{
			ratio = FIX_POINT_D(wSrcHeight) / wSrcWidth;
			ratio_add=ratio/100;
			if ((ratio-(ratio_add*100))>=50)ratio_add++;
			for (i=20;; i=i+20)
			{
				if (ratio_add*i<pstScreen->wInnerHeight)
				{
					if (10*i<pstScreen->wInnerWidth)
					{
					}
					else
					{
						ix=i-20;
						break;
					}
				}
				else
				{
					ix=i-20;
					break;
				}
			}
			for (i=ix;; i++)
			{
				if (ratio_add*i<=pstScreen->wInnerHeight)
				{
					if (10*i<=pstScreen->wInnerWidth)
					{
					}
					else
					{
						ix=i-1;
						break;
					}
				}
				else
				{
					ix=i-1;
					break;
				}
			}
			dispWin->wWidth=ix*10;
			dispWin->wHeight=ix*ratio_add;
			break;
		}
	case 2:
		for (i=10;; i=i+10)
		{
			if (16*i<=pstScreen->wInnerWidth)
			{
				if (9*i<=pstScreen->wInnerHeight)
				{
				}
				else
				{
					ix=(i-10);
					break;
				}
			}
			else
			{
				ix=(i-10);
				break;
			}
		}
		for (i=ix;; i++)
		{
			if (16*i<=pstScreen->wInnerWidth)
			{
				if (9*i<=pstScreen->wInnerHeight) {}
				else
				{
					ix=(i-1);
					break;
				}
			}
			else
			{
				ix=(i-1);
				break;
			}
		}
		dispWin->wWidth=ix*16;
		dispWin->wHeight=ix*9;
		break;
	case 3:
		for (i=40;; i=i+40)
		{
			if (4*i<=pstScreen->wInnerWidth)
			{
				if (3*i<=pstScreen->wInnerHeight)
				{
				}
				else
				{
					ix=(i-40);
					break;
				}
			}
			else
			{
				ix=(i-40);
				break;
			}
		}
		for (i=ix;; i++)
		{
			if (4*i<=pstScreen->wInnerWidth)
			{
				if (3*i<=pstScreen->wInnerHeight)
				{
				}
				else
				{
					ix=(i-1);
					break;
				}
			}
			else
			{
				ix=(i-1);
				break;
			}
		}
		dispWin->wWidth=ix*4;
		dispWin->wHeight=ix*3;
		break;
				case 4:
		
		if (wSrcWidth>=wSrcHeight)
				{
					ratio = FIX_POINT_D(wSrcWidth) / wSrcHeight;
					ratio_add=ratio/100;
					if ((ratio-(ratio_add*100))>=50)ratio_add++;
					for (i=20;; i=i+20)
					{
						if (ratio_add*i<pstScreen->wInnerWidth)
						{
							if (10*i<pstScreen->wInnerHeight)
							{
							}
							else
							{
								ix=i-20;
								break;
							}
						}
						else
						{
							ix=i-20;
							break;
						}
					}
					for (i=ix;; i++)
					{
						if (ratio_add*i<=pstScreen->wInnerWidth)
						{
							if (10*i<=pstScreen->wInnerHeight)
							{
							}
							else
							{
								ix=i-1;
								break;
							}
						}
						else
						{
							ix=i-1;
							break;
						}
					}
					dispWin->wWidth=ix*ratio_add;
					dispWin->wHeight=ix*10;
					break;
				}
				else
				{
					ratio = FIX_POINT_D(wSrcHeight) / wSrcWidth;
					ratio_add=ratio/100;
					if ((ratio-(ratio_add*100))>=50)ratio_add++;
					for (i=20;; i=i+20)
					{
						if (ratio_add*i<pstScreen->wInnerHeight)
						{
							if (10*i<pstScreen->wInnerWidth)
							{
							}
							else
							{
								ix=i-20;
								break;
							}
						}
						else
						{
							ix=i-20;
							break;
						}
					}
					for (i=ix;; i++)
					{
						if (ratio_add*i<=pstScreen->wInnerHeight)
						{
							if (10*i<=pstScreen->wInnerWidth)
							{
							}
							else
							{
								ix=i-1;
								break;
							}
						}
						else
						{
							ix=i-1;
							break;
						}
					}
					dispWin->wWidth=ix*10;
					dispWin->wHeight=ix*ratio_add;
				}
		
						break;

	}
	if (dispWin->wWidth%2)dispWin->wWidth--;
	if (dispWin->wHeight%2)dispWin->wHeight--;
	tx2=dispWin->wWidth;
	ty2=dispWin->wHeight;

	if (444 == srcWin->wType)
	{
		srcWin->dwOffset = (DWORD) srcWin->wWidth * 3;
		dispWin->wType = 444;
		dispWin->dwOffset = (DWORD) dispWin->wWidth * 3;
	}

	Ipu_Video_Bypass_Scaling(srcWin, dispWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2,ty2, mode);
	return 1;
}


void VideoAdjustDisplaySize(ST_IMGWIN * pTargetWin, ST_IMGWIN * pSourceWin)
{
	WORD wRealWidth, wRealHeight;

	if ((pTargetWin->wWidth * pSourceWin->wHeight) >= (pTargetWin->wHeight * pSourceWin->wWidth))
	{
		if (pSourceWin->wHeight < (pTargetWin->wHeight >> 2))
		{
			wRealHeight = pSourceWin->wHeight << 2;
			wRealWidth = ((pSourceWin->wWidth * wRealHeight) / pSourceWin->wHeight) & 0xfffe;
			pTargetWin->wX = ((pTargetWin->wWidth - wRealWidth) >> 1) & 0xfffe;
			pTargetWin->wY = ((pTargetWin->wHeight - wRealHeight) >> 1);
		}
		else
		{
			wRealHeight = pTargetWin->wHeight;
			wRealWidth = ((pSourceWin->wWidth * wRealHeight) / pSourceWin->wHeight) & 0xfffe;
			pTargetWin->wX = ((pTargetWin->wWidth - wRealWidth) >> 1) & 0xfffe;
			pTargetWin->wY = 0;
		}
	}
	else
	{
		if (pSourceWin->wWidth < (pTargetWin->wWidth >> 2))
		{
			wRealWidth = pSourceWin->wWidth << 2;
			wRealHeight = (pSourceWin->wHeight * wRealWidth) / pSourceWin->wWidth;
			pTargetWin->wX = ((pTargetWin->wWidth - wRealWidth) >> 1) & 0xfffe;
			pTargetWin->wY = ((pTargetWin->wHeight - wRealHeight) >> 1);
		}
		else
		{
			wRealWidth = pTargetWin->wWidth;
			wRealHeight = (pSourceWin->wHeight * wRealWidth) / pSourceWin->wWidth;
			pTargetWin->wX = 0;
			pTargetWin->wY = (pTargetWin->wHeight - wRealHeight) >> 1;
		}
	}
	pTargetWin->dwOffset = (pTargetWin->wWidth - wRealWidth) << 1;
	pTargetWin->wWidth = wRealWidth;
	pTargetWin->wHeight = wRealHeight;
}


///
///@ingroup group_Vdecoder
///@brief   This function will call the video decode function, _video_decoder.video_decoder.decode(), then change yv12 to yyuv.
///
///@param   mp_image_t *video_mpi, the image display window
///@param   int direct_render
///@param   sh_video_t* sh_video, Video stream header structure including the parameters of  video stream.
///@param   unsigned char *start
///@param   int in_size
///@param   int drop_video_frame
///
///@return  video_decoder_t*, this pointer will be recorded in filter_graph_t structure.
///

int decode_video(mp_image_t * video_mpi, int frame_no, sh_video_t * sh_video, unsigned char *start,
                 int in_size, int drop_video_frame)
{
#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s", __FUNCTION__);
#endif
	int result;
	register CLOCK *clock = (CLOCK *) (CLOCK_BASE);

	if (!MJPG_FOURCC(sh_video->format))
	{
		//enable MPV clock
#if (CHIP_VER_MSB == CHIP_VER_615)
		switch (Clock_CpuFreqGet())
		{
		case CLOCK_108M_PLL2CFG:
			mSetMpvCks(MPVCKS_PLL2);
			break;
		case CLOCK_96M_PLL1CFG:
		case CLOCK_108M_PLL1CFG:
		default:
			mSetMpvCks(MPVCKS_PLL1);
			break;
		}
#endif
		clock->MdClken |= 0x00000002L;	// MPV clock enable
	}

	//memset(video_mpi, 0, sizeof(mp_image_t));

	result =
	    _video_decoder.video_decoder.decode(video_mpi, frame_no, sh_video, start, in_size,
	                                        drop_video_frame);
	//Waiting for first time of video decode finish.
	//It prevents that the system crashes in Audio task(for *.mpg(MPEG-1 stream))
	//XianWen Chang note it 2010/10/04
	if (NotDecodeVideo)
		NotDecodeVideo = 0;

	if (result == 1)
		frame_num++;
#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
	mpDebugPrint("%s end", __FUNCTION__);
#endif
	return result;
}

int display_video_thumb(ST_IMGWIN * screen_win, ST_IMGWIN * trgwin, mp_image_t * video_mpi, int direct_render)
{
	MP_DEBUG("%s", __FUNCTION__);
#if (VIDEO_ENABLE || MJPEG_ENABLE)
	static ST_SCA_PARM sScaParm;
	static ST_IMAGEINFO src, trg;
	ST_IMGWIN srcwin, tmpwin;
	int full_window;
	sh_video_t *sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	const unsigned int four_cc = sh_video->format;
	if ((0 == video_mpi->display_width) || (0 == video_mpi->display_height))
	{
		return 0;
	}
	//video_mpi->display_width = ALIGN_16(video_mpi->display_width);	//temporarily avoid the problem of IPU timeout
	//video_mpi->display_height = ALIGN_16(video_mpi->display_height);

	//if (!MovieTask_CheckPlaying()) return FAIL;
#if (CHIP_VER_MSB == CHIP_VER_615)
	if (g_psCDU->CduOp && !g_psCDU->CduIc) return PASS;
	if (g_psIpu->IpIpw10 && !g_psIpu->IpIc) return PASS;
#endif

	if (direct_render)
	{
#if (CHIP_VER_MSB != CHIP_VER_615)

		WORD wScaleFactorWidth;
		WORD wScaleFactorHeight;
		WORD wScaledWidth;
		WORD wScaledHeight;
		WORD wStartX;
		WORD wStartY;
		ST_IMGWIN *p_cur_win;
		p_cur_win = screen_win;

		//mpDebugPrint("ScaleFactor: width = %d, height = %d", wScaleFactorWidth, wScaleFactorHeight);


		wScaleFactorWidth = ( (DWORD) trgwin->wWidth << 10 ) / video_mpi->display_width;
		wScaleFactorHeight = ( (DWORD) trgwin->wHeight << 10 ) / video_mpi->display_height;

		if (wScaleFactorWidth < wScaleFactorHeight)
		{
			wScaledWidth = trgwin->wWidth;
			wScaledHeight = (video_mpi->display_height * wScaleFactorWidth) >> 10;
			wStartX = 0;
			wStartY = (trgwin->wHeight - wScaledHeight) >> 1;
		}
		else
		{
			wScaledWidth = (video_mpi->display_width * wScaleFactorHeight) >> 10;
			wScaledHeight = trgwin->wHeight;
			wStartX = (trgwin->wWidth - wScaledWidth) >> 1;
			wStartY = 0;
		}

		//mpDebugPrint("ScaleFactor: width = %d, height = %d", wScaleFactorWidth, wScaleFactorHeight);
		//mpDebugPrint("video_mpi->display_width = %d, video_mpi->display_height = %d", video_mpi->display_width, video_mpi->display_height);

		srcwin.pdwStart = (DWORD*)video_mpi->planes[0];
		srcwin.dwOffset = video_mpi->display_width << 1;
		srcwin.wX = 0;
		srcwin.wY = 0;
		srcwin.wWidth = video_mpi->display_width;
		srcwin.wHeight = video_mpi->display_height;

		if (MJPG_FOURCC(four_cc))
		{
#if MJPEG_ENABLE
			BYTE *pbTempRotateBuffer = NULL;
			ST_IMGWIN sTempWin;
			g_bScalingFinish = 0;
			extern DWORD g_dwScalingStartAddr;
			g_dwScalingStartAddr = (DWORD) video_mpi->planes[0];
			ST_IMGWIN *p_next_win;

#if (CHIP_VER_MSB == CHIP_VER_650)
			const unsigned int pp_rotate = Video_rotation_display_get();
#else
			const unsigned int pp_rotate = PP_ROTATION_NONE;
#endif

			//mpDebugPrint("p_cur_win->wX = %d, p_cur_win->wY = %d, p_cur_win->wWidth = %d, p_cur_win->wHeight = %d", p_cur_win->wX, p_cur_win->wY, p_cur_win->wWidth, p_cur_win->wHeight);
			//mpDebugPrint("srcwin.wX = %d, srcwin.wY = %d, srcwin.wWidth = %d, srcwin.wHeight = %d", srcwin.wX, srcwin.wY, srcwin.wWidth, srcwin.wHeight);
			//mpDebugPrint("trgwin->wX = %d, trgwin->wY = %d, trgwin->wWidth = %d, trgwin->wHeight = %d", trgwin->wX, trgwin->wY, trgwin->wWidth, trgwin->wHeight);
#if DOUBLE_IDU_WIN
			p_next_win = Idu_GetNextWin();
#endif
			if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW)
			{
				srcwin.wType = 422;
				//Idu_Chg_Bypass422_Mode();
				if (srcwin.wWidth % 16 != 0)
				{
					srcwin.wWidth = ((srcwin.wWidth / 16) + 1) * 16;
					srcwin.dwOffset = srcwin.wWidth << 1;
				}

				if (srcwin.wHeight % 2 != 0)
				{
					srcwin.wHeight = ((srcwin.wHeight / 2) + 1) * 2;
				}

				Ipu_ImageScaling(&srcwin, p_cur_win, 0, 0, srcwin.wWidth, srcwin.wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
#if DOUBLE_IDU_WIN
				WinCopy(p_next_win, 0, 0, p_cur_win->wWidth, p_cur_win->wHeight, p_cur_win->pdwStart, p_cur_win->wWidth);
#endif
			}
			else
			{
#if YUV444_ENABLE
				if (GetCurImgOutputType() == 444)
				{
					srcwin.wType = 444;
					Idu_Chg_444_Mode();
				}
				else
				{
					srcwin.wType = 422;
					Idu_Chg_422_Mode();
				}
				MP_DEBUG("srcwin.wType = %d", srcwin.wType);
#else
				srcwin.wType = 422;
				Idu_Chg_422_Mode();
#endif
				ST_IMGWIN * pScaleSrcWin;
				if (0 != MJ_INIT)
				{
					WORD img_h = Jpg_GetImageHeight();
					srcwin.wHeight = img_h;

					if (pp_rotate != PP_ROTATION_NONE)
					{
						pbTempRotateBuffer = (BYTE *)mem_malloc((srcwin.wWidth * 2) * (srcwin.wHeight));
						if (!pbTempRotateBuffer)
						{
							MP_ALERT("%s: pbTempRotateBuffer alloc fail", __FUNCTION__);
							return FAIL;
						}

						ImgWinInit(&sTempWin, (DWORD *)pbTempRotateBuffer, srcwin.wWidth, srcwin.wHeight);
						srcwin.wWidth = ALIGN_CUT_16(srcwin.wWidth);
						srcwin.wHeight = ALIGN_CUT_16(srcwin.wHeight);//maybe risky
						#if VIDEO_ENABLE
						const PPResult ppRet = Img_Rotate_PP(&sTempWin, &srcwin, pp_rotate, FALSE);
						#else
						const PPResult ppRet = PP_OK;
						#endif
						pScaleSrcWin = &sTempWin;
					}
					else
						pScaleSrcWin = &srcwin;


					if (pScaleSrcWin->wWidth % 16 != 0)
						pScaleSrcWin->wWidth = ((pScaleSrcWin->wWidth / 16) + 1) * 16;

					if (pScaleSrcWin->wHeight % 2 != 0)
						pScaleSrcWin->wHeight = ((pScaleSrcWin->wHeight / 2) + 1) * 2;

					if ((pScaleSrcWin->wWidth >= trgwin->wWidth<<3) && (img_h >= trgwin->wHeight<<3))
					{
						pScaleSrcWin->wWidth >>= 3;
						pScaleSrcWin->wHeight >>= 3;
					}
					else if ((pScaleSrcWin->wWidth >= trgwin->wWidth<<2) && (img_h >= trgwin->wHeight<<2))
					{
						pScaleSrcWin->wWidth >>= 2;
						pScaleSrcWin->wHeight >>= 2;
					}
					else if ((pScaleSrcWin->wWidth >= trgwin->wWidth<<1) && (img_h >= trgwin->wHeight<<1))
					{
						pScaleSrcWin->wWidth >>= 1;
						pScaleSrcWin->wHeight >>= 1;
					}
					pScaleSrcWin->dwOffset = pScaleSrcWin->wWidth << 1;

					if (pScaleSrcWin->wWidth % 16 != 0)
						pScaleSrcWin->dwOffset =  (((pScaleSrcWin->wWidth / 16) + 1) * 16) << 1;

					if (pScaleSrcWin->wHeight % 2 != 0)
						pScaleSrcWin->wHeight = ((pScaleSrcWin->wHeight / 2) + 1) * 2;
				}

				if (0 == MJ_INIT)
				{
					MJ_INIT++;
					return 1;
				}
				else if (1 == MJ_INIT)
				{
					MJ_INIT++;
#if (DISPLAY_VIDEO_RATIO==1)
					ImageDraw_FitToOrigin(&srcwin, p_cur_win);
#else
					Ipu_ImageScaling(pScaleSrcWin, p_cur_win, 0, 0, pScaleSrcWin->wWidth, pScaleSrcWin->wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
#endif
#if DOUBLE_IDU_WIN
					WinCopy(p_next_win, 0, 0, p_cur_win->wWidth, p_cur_win->wHeight, p_cur_win->pdwStart, p_cur_win->wWidth);
#endif
				}
				else
				{
#if DOUBLE_IDU_WIN
#if (DISPLAY_VIDEO_RATIO==1)
					ImageDraw_FitToOrigin(&srcwin, p_next_win);
#else
					Ipu_ImageScaling(pScaleSrcWin, p_next_win, 0, 0, pScaleSrcWin->wWidth, pScaleSrcWin->wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
#endif
					Idu_ChgWin(p_next_win);
#else
#if DISPLAY_ORIGIANL_VIDEO_RATIO
					ImageDraw_FitToOrigin(&srcwin, p_cur_win);
#else
					Ipu_ImageScaling(pScaleSrcWin, p_cur_win, 0, 0, pScaleSrcWin->wWidth, pScaleSrcWin->wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
#endif
#endif
				}
			}
			if (pbTempRotateBuffer)
			{
				mem_free(pbTempRotateBuffer);
				pbTempRotateBuffer = NULL;
			}
			g_bScalingFinish = 1;
#endif
		}
#if VIDEO_ENABLE
		else
		{
			srcwin.wType = 411;
#if	DOUBLE_IDU_WIN_VIDEO_PID
			image_scale_mpv2mem(&srcwin, Idu_GetNextWin(), 0, 0, video_mpi->display_width, video_mpi->display_height, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight);
			Idu_ChgWin(Idu_GetNextWin());
#else
if(HD720p){
			DMA * const dmabase = (DMA *)DMA_BASE;
					dmabase->res |= BIT7;
}
			image_scale_mpv2mem(&srcwin, p_cur_win, 0, 0, video_mpi->display_width, video_mpi->display_height, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight);
#endif
		}
#endif
#else
		memset(&srcwin, 0, sizeof(ST_IMGWIN));
		srcwin.pdwStart = (DWORD*)video_mpi->planes[0];
		srcwin.wX = 0;
		srcwin.wY = 0;
		srcwin.wWidth = video_mpi->display_width;
		srcwin.wHeight = video_mpi->display_height;
		srcwin.dwOffset = srcwin.wWidth << 1;

		if (MJPG_FOURCC(four_cc))
		{
#if MJPEG_ENABLE
			g_bScalingFinish = 0;
#endif
			ST_IMGWIN *p_cur_win;
			ST_IMGWIN *p_next_win;
			p_cur_win = screen_win;
			p_next_win = Idu_GetNextWin();
			if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PREVIEW)
			{
				Ipu_ImageScaling(&srcwin, p_cur_win, 0, 0, srcwin.wWidth, srcwin.wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
				WinCopy(p_next_win, 0, 0, p_cur_win->wWidth, p_cur_win->wHeight, p_cur_win->pdwStart, p_cur_win->wWidth);
			}
			else
			{
				if (0 != MJ_INIT)
				{
					if (srcwin.wHeight % 16 != 0)
					{
						srcwin.wHeight = ((srcwin.wHeight / 16) + 1) * 16;
					}

					if (srcwin.wWidth % 16 != 0)
					{
						srcwin.wWidth = ((srcwin.wWidth / 16) + 1) * 16;
					}

					if ((srcwin.wWidth >= trgwin->wWidth<<2) && (srcwin.wHeight >= trgwin->wHeight<<2))
					{
						srcwin.wWidth >>= 2;
						srcwin.wHeight >>= 2;
						srcwin.dwOffset = srcwin.wWidth << 1;
					}
					else if ((srcwin.wWidth >= trgwin->wWidth<<1) && (srcwin.wHeight >= trgwin->wHeight<<1))
					{
						srcwin.wWidth >>= 1;
						srcwin.wHeight >>= 1;
						srcwin.dwOffset = srcwin.wWidth << 1;
					}

					if (srcwin.wWidth % 16 != 0)
					{
						srcwin.dwOffset =  (((srcwin.wWidth / 16) + 1) * 16) << 1;
					}

					if (srcwin.wHeight % 2 != 0)
					{
						srcwin.wHeight = ((srcwin.wHeight / 2) + 1) * 2;
					}
					//mpDebugPrint("srcwin.wWidth = %d, srcwin.wHeight = %d", srcwin.wWidth, srcwin.wHeight);
				}

				if (0 == MJ_INIT)
				{
					MJ_INIT++;
					return 1;
				}
				else if (1 == MJ_INIT)
				{
					MJ_INIT++;
					Ipu_ImageScaling(&srcwin, p_cur_win, 0, 0, srcwin.wWidth, srcwin.wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
					WinCopy(p_next_win, 0, 0, p_cur_win->wWidth, p_cur_win->wHeight, p_cur_win->pdwStart, p_cur_win->wWidth);
				}
				else
				{
					Ipu_ImageScaling(&srcwin, p_next_win, 0, 0, srcwin.wWidth, srcwin.wHeight, trgwin->wX, trgwin->wY, trgwin->wX+trgwin->wWidth, trgwin->wY+trgwin->wHeight, 0);
					Idu_ChgWin(p_next_win);
				}
			}
#if MJPEG_ENABLE
			g_bScalingFinish = 1;
#endif
		}
#endif
		return 1;
	}
#endif
}

int display_video(mp_image_t * const video_mpi, const int direct_render)
{
	int resolution=0;
	resolution= resoultion_1;

	static ST_SCA_PARM sScaParm;
	static ST_IMAGEINFO src, trg;
	ST_IMGWIN srcwin, tmpwin, dispwin;
	int full_window;

	ST_IMGWIN * const trgwin = Idu_GetCurrWin();

	//src.wType = TYPE_420;
	sh_video_t * const sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	const unsigned int four_cc = sh_video->format;
	memcpy(&dispwin, trgwin, sizeof(dispwin));
#if MJPEG_ENABLE
	if (MJPG_FOURCC(four_cc))
	{
		if (!MJ_INIT)
		{
			MJ_INIT = 1;
			//mpPaintWin(Idu_GetNextWin(), 0x00008080);
			//Idu_ChgWin(Idu_GetNextWin());
			return 1;
		}
	}
#endif

#if ANTI_TEARING_ENABLE
	if (frame_buffer_index == 0)    trgwin = Idu_GetCurrWin();
	else if (frame_buffer_index == 1)    trgwin = Idu_GetExtraWin(0);
	else    trgwin = Idu_GetExtraWin(1);
#endif

	if (video_mpi->display_width == 0 || video_mpi->display_height == 0) return 0;
	if (direct_render)
	{
#if (CHIP_VER_MSB != CHIP_VER_615)

		WORD wScaleFactorWidth;
		WORD wScaleFactorHeight;
		WORD wScaledWidth;
		WORD wScaledHeight;
		WORD wStartX;
		WORD wStartY;

		wScaleFactorWidth = ( (DWORD) trgwin->wWidth << 10 ) / video_mpi->display_width;
		wScaleFactorHeight = ( (DWORD) trgwin->wHeight << 10 ) / video_mpi->display_height;

		if (wScaleFactorWidth < wScaleFactorHeight)
		{
			wScaledWidth = trgwin->wWidth;
			wScaledHeight = (video_mpi->display_height * wScaleFactorWidth) >> 10;
			wStartX = 0;
			wStartY = (trgwin->wHeight - wScaledHeight) >> 1;
		}
		else
		{
			wScaledWidth = (video_mpi->display_width * wScaleFactorHeight) >> 10;
			wScaledHeight = trgwin->wHeight;
			wStartX = (trgwin->wWidth - wScaledWidth) >> 1;
			wStartY = 0;
		}

		//mpDebugPrint("ScaleFactor: width = %d, height = %d", wScaleFactorWidth, wScaleFactorHeight);
		//mpDebugPrint("video_mpi->display_width = %d, video_mpi->display_height = %d", video_mpi->display_width, video_mpi->display_height);

		srcwin.pdwStart = (DWORD*)video_mpi->planes[0];
		srcwin.dwOffset = video_mpi->display_width << 1;
		srcwin.wX = 0;
		srcwin.wY = 0;
		srcwin.wWidth = video_mpi->display_width;
		srcwin.wHeight = video_mpi->display_height;


		if (MJPG_FOURCC(four_cc))
		{
#if YUV444_ENABLE
			if (GetCurImgOutputType() == 444)
			{
				srcwin.wType = 444;
				srcwin.dwOffset = srcwin.wWidth * 3;
				//Idu_Chg_Bypass444_Mode();
			}
			else
			{
				srcwin.wType = 422;
				//Idu_Chg_Bypass422_Mode();
			}
			MP_DEBUG("srcwin.wType = %d", srcwin.wType);
#else
			srcwin.wType = 422;
			//Idu_Chg_Bypass422_Mode();
#endif
			if (!(g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PAUSE))
			{
#if MJPEG_ENABLE
				g_bScalingFinish = 0;
				g_dwVideoUpdateAddr = (DWORD) video_mpi->planes[0];
				//mpDebugPrint("video update 0x%x", g_dwVideoUpdateAddr);
#endif
				//DWORD start_time;
				//	start_time = GetSysTime();
				if (decoder_initialized)
				{
					WORD img_h = Jpg_GetImageHeight();
					srcwin.wHeight = img_h;
					//mpDebugPrint("display_video_thumb:img_h = %d", img_h);

					if (srcwin.wWidth % 16 != 0)
					{
						srcwin.wWidth = ((srcwin.wWidth / 16) + 1) * 16;
					}

					if (srcwin.wHeight % 2 != 0)
					{
						srcwin.wHeight = ((srcwin.wHeight / 2) + 1) * 2;
					}

					if ((srcwin.wWidth >= trgwin->wWidth<<3) && (img_h >= trgwin->wHeight<<3))
					{
						srcwin.wWidth >>= 3;
						srcwin.wHeight >>= 3;
					}
					else if ((srcwin.wWidth >= trgwin->wWidth<<2) && (img_h >= trgwin->wHeight<<2))
					{
						srcwin.wWidth >>= 2;
						srcwin.wHeight >>= 2;
					}
					else if ((srcwin.wWidth >= trgwin->wWidth<<1) && (img_h >= trgwin->wHeight<<1))
					{
						srcwin.wWidth >>= 1;
						srcwin.wHeight >>= 1;
					}
					srcwin.dwOffset = srcwin.wWidth << 1;

					if (srcwin.wWidth % 16 != 0)
					{
						srcwin.dwOffset =  (((srcwin.wWidth / 16) + 1) * 16) << 1;
					}

					if (srcwin.wHeight % 2 != 0)
					{
						srcwin.wHeight = ((srcwin.wHeight / 2) + 1) * 2;
					}
					if (displayratio_open==1)display_video_bypass2dma(video_mpi);

					DisplayRatio(&srcwin, &dispwin, 0, 0, video_mpi->display_width, video_mpi->display_height, 0, 0, trgwin->wWidth, trgwin->wHeight, 0, resolution);
					decoder_initialized = 0;

				}
				else
				{
					Video_Update_Frame(&srcwin, 0);
					//Add a mechanism to prevent that it always is in while loop.
					//It is because IPU interrupt fails.
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
					DWORD IPU_ISR_time;
					DWORD IPU_ISR_timeout=0;
					IPU_ISR_time=GetSysTime();
					while (!Get_Video_Frame_Cnt()&& IPU_ISR_timeout<100)
					{
						IPU_ISR_timeout=SystemGetElapsedTime(IPU_ISR_time);
						TaskYield();
					}
					if ((IPU_ISR_timeout>=100)&&!Get_Video_Frame_Cnt())
					{
						MP_ALERT("IPU_ISR_timeout %d",IPU_ISR_timeout);
						display_video_bypass2dma(video_mpi);
						decoder_initialized = 1;
					}
#else
					while (!Get_Video_Frame_Cnt())
					{
						TaskYield();
					}
#endif
				}
#if MJPEG_ENABLE
				extern DWORD g_dwScalingStartAddr;
				//Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "display", "yuv", (BYTE *)video_mpi->planes[0], 640*480*2);
				g_dwScalingStartAddr = (DWORD) video_mpi->planes[0];
				// mpDebugPrint("set g_dwScalingStartAddr = 0x%x", g_dwScalingStartAddr);
				g_bScalingFinish = 1;
				//DWORD end_time = GetSysTime();
				//DWORD elap_time = end_time - start_time;
				//mpDebugPrint("elap_time = %d", elap_time);
#endif
			}

		}
#if VIDEO_ENABLE
		else
		{
			src.wType = 420;
			if (decoder_initialized)
			{
				decoder_initialized = 0;

				if (displayratio_open==1)display_video_bypass2dma(video_mpi);
				DisplayRatio(&srcwin, &dispwin, 0, 0,video_mpi->display_width,video_mpi->display_height,0, 0, trgwin->wWidth, trgwin->wHeight, 1,resolution);

			}
			else
			{
				Video_Update_Frame(&srcwin, 1);
				while (!Get_Video_Frame_Cnt())
					TaskYield();
			}
		}
#endif


#else	// (CHIP_VER & 0xffff0000) == CHIP_VER_615

		src.dwPointer = (DWORD *) video_mpi->planes[0];
		if (MJPG_FOURCC(four_cc))
		{
			trgwin = Idu_GetNextWin();
		}

		if ((src.wWidth != video_mpi->display_width) || (src.wHeight != video_mpi->display_height) || (trg.dwPointer != trgwin->pdwStart)
		        || (trg.wWidth != trgwin->wWidth) || (trg.wHeight != trgwin->wHeight))
		{
			if (video_mpi->display_width < video_mpi->display_height)
				full_window = 0;
			else if ((video_mpi->display_width < (trgwin->wWidth >> 2))) // || (video_mpi->display_height < (trgwin->wHeight >> 2)))
				full_window = 0;
			else
				full_window = 1;
			if (full_window)
			{
				trg.dwPointer = trgwin->pdwStart;
				trg.wWidth = trgwin->wWidth;
				trg.wHeight = trgwin->wHeight;
				trg.dwOffset = 0;
			}
			else
			{
				tmpwin.wWidth = trgwin->wWidth;
				tmpwin.wHeight = trgwin->wHeight;
				srcwin.wWidth = video_mpi->display_width;
				srcwin.wHeight = video_mpi->display_height;
				VideoAdjustDisplaySize(&tmpwin, &srcwin);

				trg.dwPointer = trgwin->pdwStart + ((tmpwin.wY * (trgwin->dwOffset>>1) + tmpwin.wX)>>1);
				trg.wWidth = tmpwin.wWidth;
				trg.wHeight = tmpwin.wHeight;
				trg.dwOffset = tmpwin.dwOffset;
			}

			src.wWidth = video_mpi->display_width;
			src.wHeight = video_mpi->display_height;

			if (MJPG_FOURCC(four_cc))
			{
				src.dwOffset = 0;
			}
			else
			{
				src.dwOffset = get_align_mpv_width((video_mpi->display_width + 15) >> 4) << 4;
			}

			ImgGetResizeParam(&src, &trg, &sScaParm);
		}
		MP_TRACE_TIME_START();
#if MJPEG_ENABLE
		g_bScalingFinish = 0;
#endif

		ImgIpuScaling(&sScaParm, &src, &trg);
		if (MJPG_FOURCC(four_cc))
		{
			Idu_ChgWin(Idu_GetNextWin());
		}
#if MJPEG_ENABLE
		g_bScalingFinish = 1;
#endif
		MP_TRACE_TIME_PRINT("IPU", 0);

#endif	// (CHIP_VER & 0xffff0000) == CHIP_VER_615
	}

#if ANTI_TEARING_ENABLE
	frame_buffer_index++;
	if (frame_buffer_index == 3)    frame_buffer_index = 0;
	if (!video_stop)    buffer_switch = TRUE;
#endif
	return 1;
}


int display_video_bypass2dma(mp_image_t * const video_mpi)
{
	MP_DEBUG("%s : %d", __FUNCTION__, __LINE__);
#if (VIDEO_ENABLE || MJPEG_ENABLE)
#if (CHIP_VER_MSB != CHIP_VER_615)
	if ((video_mpi->display_width > MAX_VIDEO_DECODE_WIDTH) || (video_mpi->display_height > MAX_VIDEO_DECODE_HEIGHT))
	{
		MP_ALERT("%s : %d", __FUNCTION__, __LINE__);
		return 1;
	}

	ST_IMGWIN srcwin;

	ST_IMGWIN * const trgwin = Idu_GetCurrWin();

	srcwin.pdwStart = (DWORD*)video_mpi->planes[0];
	srcwin.dwOffset = video_mpi->display_width << 1;
	srcwin.wX = 0;
	srcwin.wY = 0;
	srcwin.wWidth = video_mpi->display_width;
	srcwin.wHeight = video_mpi->display_height;
	srcwin.wType = 411;

	sh_video_t * const sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	const unsigned int four_cc = sh_video->format;

	if (MJPG_FOURCC(four_cc))
	{
		if (srcwin.wWidth % 16 != 0)
		{
			srcwin.wWidth = ((srcwin.wWidth / 16) + 1) * 16;
		}

		if (srcwin.wHeight % 2 != 0)
		{
			srcwin.wHeight = ((srcwin.wHeight / 2) + 1) * 2;
		}

		if ((srcwin.wWidth >= trgwin->wWidth<<3) && (srcwin.wHeight >= trgwin->wHeight<<3))
		{
			srcwin.wWidth >>= 3;
			srcwin.wHeight >>= 3;
		}
		else if ((srcwin.wWidth >= trgwin->wWidth<<2) && (srcwin.wHeight >= trgwin->wHeight<<2))
		{
			srcwin.wWidth >>= 2;
			srcwin.wHeight >>= 2;
		}
		else if ((srcwin.wWidth >= trgwin->wWidth<<1) && (srcwin.wHeight >= trgwin->wHeight<<1))
		{
			srcwin.wWidth >>= 1;
			srcwin.wHeight >>= 1;
		}
		srcwin.dwOffset = srcwin.wWidth << 1;

		if (srcwin.wWidth % 16 != 0)
		{
			srcwin.dwOffset =  (((srcwin.wWidth / 16) + 1) * 16) << 1;
		}

		if (srcwin.wHeight % 2 != 0)
		{
			srcwin.wHeight = ((srcwin.wHeight / 2) + 1) * 2;
		}

		//mpDebugPrint("enter Ipu_ImageScaling, trgwin->pdwStart = 0x%x", trgwin->pdwStart);
		Ipu_ImageScaling(&srcwin, trgwin, 0, 0, video_mpi->display_width, video_mpi->display_height, 0, 0, trgwin->wWidth, trgwin->wHeight, 0);
		//mpDebugPrint("leave Ipu_ImageScaling");
	}
#if VIDEO_ENABLE
	else
	{
		//WriteOutFrameTest(SD_MMC, (unsigned char*)srcwin.pdwStart, srcwin.wWidth, srcwin.wHeight);
		image_scale_mpv2mem(&srcwin, trgwin, 0, 0, video_mpi->display_width, video_mpi->display_height, 0, 0, trgwin->wWidth, trgwin->wHeight);
	}
#endif
	Idu_Bypass2DMA(trgwin);
#if VIDEO_ENABLE
	if (!MJPG_FOURCC(four_cc) && HD720p)
	{
		DMA * const dmabase = (DMA *)DMA_BASE;
		dmabase->res |= BIT7;
	}
#endif
	//decoder_initialized = 1;
#endif
#endif
	MP_DEBUG("display_video_bypass2dma return");
	return 1;
}
const unsigned char* next_start_pattern(const unsigned char *start, int len, const unsigned int pattern)
{
	if (len<=0) return NULL;
	unsigned int head = 0xffffff00;
	while (head!=pattern && len--)
	{
		head <<= 8;
		head |= *start++;
	}

	if (head==pattern)	return start-4;
	else				return NULL;
}

int control_video(sh_video_t * const sh_video, const int cmd, void *arg, const unsigned char * const start, const unsigned int len)
{
	if (!_video_decoder.video_decoder.control)	return -1;
	return _video_decoder.video_decoder.control(sh_video, cmd, arg, start, len);
}

void reset_video_decoder()
{
	memset(&_video_decoder, 0, sizeof(video_decoder_t));
}
#endif




