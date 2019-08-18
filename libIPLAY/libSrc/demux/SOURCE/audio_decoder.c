/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      audio_decoder.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: audio decoder ,use this for creating new codec
*              all audio decoder APIs' accesses should be invocation by this layer
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mptrace.h"

#include "demux.h"
#include "demux_types.h"
#include "demux_stream.h"

#include "stream_type.h"

#include "audio_decoder.h"

#include "filter_graph.h"

#include "config_codec.h"
#include "taskid.h"

#if BLUETOOTH == ENABLE
#include "BtApi.h"
#endif

///
///@ingroup group2_FilterG
///@defgroup group_Adecoder         Audio Decoder
///
#define SRAM_ADR 0x98000000


#if (VIDEO_ON || AUDIO_ON)
static audio_decoder_t _audio_decoder;
Audio_dec Media_data;
extern DWORD pre_parse;
extern enum AO_AV_TYPE AO_AV;
extern sh_audio_t sh_audio_wrapper;
extern int left_channel;
extern int right_channel;
//*******************************************************************//
#define MAX_AUDIO_BUFFER_SIZE	(12 * 1024)
#if AUDIO_ON
static char __attribute__ ((aligned(4))) a_out_buffer[MAX_AUDIO_BUFFER_SIZE];
#else
static char __attribute__ ((aligned(4))) a_out_buffer[4];
#endif


//*******************************************************************//

inline BYTE set_AudioDecoder_MP3()
{
	BYTE decoder_found = 0;
#if AUDIO_ON
#if MP3_MAD_AUDIO
		_audio_decoder.audio_decoder = mpcodecs_ad_libmad;	//Alex version Codec
		UartOutText("MAD MP3 Decoder...\r\n");
		decoder_found = 1;
#elif MP3_SW_AUDIO
		_audio_decoder.audio_decoder = software_mp3;	//Honda version Codec
		UartOutText("Soft Ware MP3 Decoder...\r\n");
		decoder_found = 1;
#elif MP3_HW_CODEC
		_audio_decoder.audio_decoder = hw_mpcodecs_ad_libmad;
		UartOutText("Hardware MP3 Decoder...\r\n");
		decoder_found = 1;
#else
		//_audio_decoder.audio_decoder = NULL;
		UartOutText("MP3 Decoder search fail...\r\n");
		decoder_found = 0;
#endif
#endif
	return decoder_found;
}



//If return 0 when initialization sucesses, there are some errors in audio media INIT function
int AudioDecoderDS_init(STREAM *sHandle)
{
	int swRetVal = 0;

	AiuDmaClearCallBackFunc();
	// Reset SRAM
#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)
	memset((char *) 0x98000000, 0, 16 * 1024);
	memset((char *) 0xafc00000, 0, 16 * 1024);	
#else
	memset((char *) 0x98000000, 0, 24 * 1024);
#endif	
	memset(&Media_data, 0, sizeof(Audio_dec));

	if (AO_AV == AO_TYPE)
	{
		SeekSet(sHandle);
		Media_data.file_size = FileSizeGet(sHandle);
		Media_data.dec_fp  = sHandle;
	}

	//	AO_AV = AO_TYPE;
	//mpDebugPrint("audio_stream=0x%x, file size: %d", audio_stream, Media_data.file_size);
	//mpDebugPrint("	--->decFp: %x", Media_data.dec_fp);
	
	return swRetVal;
}


///
///@ingroup group_Adecoder
///@brief   This function will init audio_decoder_t structure _audio_decoder according the type of audio stream.
///         Then call function init_audio_codec(sh_audio) to init the audio decoder.
///
///@param   sh_audio_t* sh_audio    Audio stream header structure including the parameters of  audio stream.
///@param   STREAM* sHandle         Input file's file handler
///
///@return  audio_decoder_t*        This pointer will be recorded in filter_graph_t structure.
///
audio_decoder_t *init_best_audio_codec(register sh_audio_t * sh_audio, STREAM *sHandle)
{
	//mpDebugPrint("Init best audio codec!!");
	unsigned int four_cc = sh_audio->format;

	AudioDecoderDS_init(sHandle);
	memset(&_audio_decoder, 0, sizeof(audio_decoder_t));
	MP_DEBUG1("four_cc = 0x%x", four_cc);
	switch (four_cc)
	{
		case MP3_50:
		case MP3_55:
		case MP3_CVBR:
		case MP3_FCC:
		case 0x56 :				//Video status case
		case 0x58 :				//Video status case
			if(!set_AudioDecoder_MP3())
			{
				UartOutText("Error MP3 Decoder...\r\n");
				return NULL;
			}
			break;

#if (WMA_ENABLE && AUDIO_ON)
		case WMA_T :
		case 0x161 :
			_audio_decoder.audio_decoder = software_wma;
			UartOutText("Soft Ware WMA Decoder...\r\n");
			break;
#endif

#if (AC3_AUDIO_ENABLE && AUDIO_ON)
		case AC3_T :
			_audio_decoder.audio_decoder = software_ac3;
			UartOutText("Soft Ware AC3 Decoder...\r\n");
			break;
#endif

		case mp4a:
		case MP4A:
		{
#if (AAC_SW_AUDIO && AUDIO_ON)
			_audio_decoder.audio_decoder = software_aac;
			UartOutText("Soft Ware AAC Decoder...\r\n");
			break;
#elif (AAC_FAAD_AUDIO && AUDIO_ON)
			_audio_decoder.audio_decoder = mpcodecs_ad_faad;
			UartOutText("FAAD AAC Decoder...\r\n");
			break;
#endif
			MP_ALERT("We cann't find an appropriate AAC codec for decoding\r\n");
			return NULL;
			break;
		}

#if (OGG_ENABLE && AUDIO_ON)
		case OGG_T :
			_audio_decoder.audio_decoder = software_ogg;
			break;
#endif

#if (RAM_AUDIO_ENABLE && AUDIO_ON)
		case RAM_T :
		case ram_T:
			_audio_decoder.audio_decoder = software_ram;
			break;
#endif
#if (AMR_ENABLE && AUDIO_ON)

		case AMR_NB:
		{
			//If you want to support AMR, you should link libffmpeg.a in your makefile.     
			//And you should open the macro AMR_NB_ENABLE in config.mak.
#ifdef AMR_NB_ENABLE
#if (!AMR_ENCODE_ENABLE)
			_audio_decoder.audio_decoder = mpcodecs_ad_ffmpeg;
			UartOutText("Soft Ware AMR Decoder...\r\n");
#endif
#endif
			break;
		}
		case AMR:
			_audio_decoder.audio_decoder = software_amr;
			break;
#endif

#if (WAV_ENABLE && AUDIO_ON)
		case PCM_RAW:
		case PCM_TWOS:
		case PCM_SOWT:
		case PCM_1:
		case 0x7:
		case ulaw:
		case ULAW:
		case 0x6:
		case ALAW:
		case alaw:
		case 0x11:
		case DVI_ADPCM:
		case 0x1100736d:
		case 0x809DDF48:
		case 0x2:
			_audio_decoder.audio_decoder = software_aviwave;
			break;
#endif
		case 0x57 :	//AAC case
		case 0x59 :
#if (AAC_SW_AUDIO && AUDIO_ON)
			_audio_decoder.audio_decoder = software_aac;
			UartOutText("Soft Ware AAC Decoder...\r\n");
#elif (AAC_FAAD_AUDIO && AUDIO_ON)
			_audio_decoder.audio_decoder = mpcodecs_ad_faad;
			UartOutText("FAAD AAC Decoder...\r\n");
#else
			UartOutText("Can not find AAC codec...\r\n");
			return NULL;
#endif
			break;
		default:
			mpDebugPrint("Unknow audio format!");
			return NULL;
	}

#ifdef EQUALIZER_EN
	//Init EQ info pointer
	AUDIO_EQ_INFO = NULL;
#endif

	if (pre_parse == 0 || pre_parse == 2){
		if(!init_audio_codec(sh_audio)){				//Init audio codec stage
			MP_ALERT("Init audio codec fail...");
			return NULL;
		}
	}
	return &_audio_decoder;
}

void uninit_audio(sh_audio_t * sh_audio)
{
	DWORD release;
	//mpDebugPrint("uninit_audio");

	EventWait(AV_DECODER_EVENT, EVENT_A_NOT_DECODING, OS_EVENT_OR, &release);

	if (sh_audio->inited)
	{
		_audio_decoder.audio_decoder.uninit(sh_audio);
		sh_audio->inited = 0;
	}

	//	Reset channel config
	left_channel = 1;
	right_channel = 1;
}


int init_audio_codec(register sh_audio_t * sh_audio)
{
	// reset in/out buffer size/pointer:
	sh_audio->a_buffer_size = 0;
	sh_audio->a_buffer = NULL;
	sh_audio->a_in_buffer_size = 0;
	sh_audio->a_in_buffer = NULL;
	sh_audio->a_in_buffer_len = 0;
	// Set up some common usefull defaults. ad->preinit() can override these:

	sh_audio->samplesize = 2;

	/*	#ifdef WORDS_BIGENDIAN
		sh_audio->sample_format=AFMT_S16_BE;
		#else
		sh_audio->sample_format=AFMT_S16_LE;
		#endif
	*/

	//when codec is not pcm,we set sample format to 16bits(hard coding).
	//if it is pcm,it is set by demux.
	sh_audio->samplesize = 16 >> 3;
	sh_audio->samplerate = 0;
	sh_audio->channels = 0;
	sh_audio->i_bps = 0;		// input rate (bytes/sec)
	sh_audio->o_bps = 0;		// output rate (bytes/sec)

	sh_audio->audio_out_minsize = 8192;	/* default size, maybe not enough for Win32/ACM */
	sh_audio->audio_in_minsize = 0;
	if (_audio_decoder.audio_decoder.preinit && !_audio_decoder.audio_decoder.preinit(sh_audio))
	{
		MP_ALERT("Audio decoder preinit fail");
		return 0;
	}

	sh_audio->a_in_buffer_size = sh_audio->audio_in_minsize;

	if (_audio_decoder.audio_decoder.init && _audio_decoder.audio_decoder.init(sh_audio))
	{
		MP_ALERT("Audio decoder init fail");
		uninit_audio(sh_audio);	// free buffers
		return 0;
	}

	sh_audio->inited = 1;
	mpDebugPrint("Channel: %d, Samplerate: %d", sh_audio->channels, sh_audio->samplerate);



#if ((BLUETOOTH == ENABLE) && (BT_PROFILE_TYPE & BT_A2DP))

//        if(MpxGetA2dpRecordingFlag()&&GetA2dpStreamStart())
    if(GetA2dpConnect() && MpxGetA2dpRecordingFlag() \
        && GetA2dpStreamStart()&&(GetAniFlag()&ANI_AUDIO))
        {
            BYTE temp;
            temp = Get_MP3_Srate(sh_audio->samplerate);
            if(BtA2dpGetA2dpSamplingRate() != temp)
            {
                BtA2dpSetA2dpSamplingRate(temp);
//                BTA2DP(A2dp_Reconfig_Srate)();
                Ui_TimerProcAdd(10,BTA2DP(A2dp_Reconfig_Srate));
                return 0;
            }
        }
#endif



	if (!sh_audio->channels || !sh_audio->samplerate)
	{
		uninit_audio(sh_audio);	// free buffers
		MP_ALERT("Audio decoder -- channel and sample rate checking fail");
		MP_ALERT("Channel %d, Sample rate %d", sh_audio->channels, sh_audio->samplerate);
		return 0;
	}

	if (!sh_audio->o_bps)
#ifdef LAYERII_FORCE_MONO
//	#if Libmad_FAAD
//		if (Layer2_Inited)
//		{
//			sh_audio->o_bps = sh_audio->samplerate * sh_audio->samplesize;
//		}
//		else
//	#endif
		{
			sh_audio->o_bps = sh_audio->channels * sh_audio->samplerate * sh_audio->samplesize;
		}

#else
		sh_audio->o_bps = sh_audio->channels * sh_audio->samplerate * sh_audio->samplesize;
#endif

	sh_audio->a_out_buffer      = sh_audio->a_buffer;
	sh_audio->a_out_buffer_size = sh_audio->a_buffer_size;
	sh_audio->a_out_buffer_len  = sh_audio->a_buffer_len;


	sh_audio->afilter = 0;

	return 1;
}

void uninit_audio_decode(filter_graph_t *graph)
{
	//mpDebugPrint("uninit_audio_decode");
	//condition will be true when codec is not pcm.
	if (/*filter_graph.*/graph->a_decoder->audio_decoder.decode_audio)
	{
		sh_audio_t *sh_audio;
		if (AO_AV == AO_TYPE)
		{
			//mpDebugPrint("AO");
			sh_audio = &sh_audio_wrapper;
			sh_audio->ds1.eof = 0;
			//sh_audio->ds->eof = 0;
		}
		else if(AO_AV == AV_TYPE)
		{
			//mpDebugPrint("AV");
			sh_audio = (sh_audio_t *) graph->demux->sh_audio;
		}
		else{
			//mpDebugPrint("NULL");
			sh_audio = NULL;
		}
		if (sh_audio)
			uninit_audio(sh_audio);
	}

	//Audio decoder cleans temporal buffer case
	if (Media_data.sdram_buf) {
		mem_free(Media_data.sdram_buf);
		Media_data.sdram_buf = NULL;
	}

	if (Media_data.sdram_buf2) {
		mem_free(Media_data.sdram_buf2);
		Media_data.sdram_buf2 = NULL;
	}
}

///
///@ingroup group_Adecoder
///@brief   This function will call the audio decoder's decode_audio() function to decode audio.
///
///@param   sh_audio_t* sh_audio        Audio stream header structure including the parameters of  audio stream.
///@param   unsigned char *buf
///@param   int minlen
///@param   int maxlen
///
///@return  audio_decoder_t*        This pointer will be recorded in filter_graph_t structure.
///
int Decode_audio(sh_audio_t * sh_audio, unsigned char *buf, int minlen, int maxlen)
{
#if AUDIO_SW_VOLUME_DOWN	

    int ret,i=0,temp_sample;
	short *buf_temp,sample,*buf_temp2;
	WORD sample_PCM;
    if (filter_graph.a_decoder->audio_decoder.decode_audio)
	{
		ret = _audio_decoder.audio_decoder.decode_audio(sh_audio, buf, minlen, maxlen);
		buf_temp = (short *)buf;
		buf_temp2 = (short *)buf;
		/////////Add soft volume control 2010/07/28 by xianwen chang/////                
		if (ret > 0)
		{     
			if (sh_audio->format == 0x1)
			{
					for(i=0;i<ret;i+=2)
					{
						if (g_bVolumeIndex>0 && g_bVolumeIndex<=16)
						{                                        
							sample_PCM = buf[i] + ((WORD)buf[i+1]<<8);
							sample_PCM = (sample_PCM*g_bVolumeIndex) / 16;
							buf[i] = (sample_PCM & 0xff);
							buf[i+1] = ((sample_PCM>>8) & 0xff);
						}
						else
						{
							buf[i] = 0x0;
							buf[i+1] = 0x0;
						}
					}
			}
			else
			{
				for(i=0;i<ret/2;i++)
				{
					if (g_bVolumeIndex>0 && g_bVolumeIndex<=16)
					{
						sample = buf_temp[i]*g_bVolumeIndex / 16;
						buf_temp2[i] = sample;
					}
					else 
						buf_temp2[i] = 0;
				}
			}
		}
		////////////////////////////////////////////////////////////////                    
		return ret;
	}
	else if (sh_audio && sh_audio->ds)
	{
		//Fix.2004.12.08: remove 4096 threshold.  joshua,bruce
		//return ds_read_data(sh_audio->ds,buf,maxlen>4096?4096:maxlen);
		return ds_read_data(sh_audio->ds, buf, maxlen);
	}
#else
 	if (filter_graph.a_decoder->audio_decoder.decode_audio)
    {
  		return _audio_decoder.audio_decoder.decode_audio(sh_audio, buf, minlen, maxlen);
    }
  	else if (sh_audio && sh_audio->ds)
  	{
  		//Fix.2004.12.08: remove 4096 threshold.  joshua,bruce
  		//return ds_read_data(sh_audio->ds,buf,maxlen>4096?4096:maxlen); 
		return ds_read_data(sh_audio->ds, buf, maxlen);
  	}
#endif
}

// This stupid function is created by C.W
// It break the original structure a little.
int resync_audio_stream_ext(DWORD sec)
{
	return _audio_decoder.audio_decoder.resync(sec);
}

int seek_audio_stream(sh_audio_t * sh_audio, int *target)
{
	if (!sh_audio->inited)
		return;
	
	return _audio_decoder.audio_decoder.control(sh_audio, ADCTRL_SEEK_STREAM, target);
}

// We do not implement this function yet!
void 
resync_audio_stream(sh_audio_t * sh_audio)
{
	sh_audio->a_in_buffer_len = 0;	// clear audio input buffer
	if (!sh_audio->inited)
		return;

	_audio_decoder.audio_decoder.control(sh_audio, ADCTRL_RESYNC_STREAM, NULL);
}

void skip_audio_frame(sh_audio_t * sh_audio)
{
	if (!sh_audio->inited)	return;
//	if (_audio_decoder.audio_decoder.control(sh_audio, ADCTRL_SKIP_FRAME, NULL) == CONTROL_TRUE)	return;

	// default skip code:
	ds_fill_buffer(sh_audio->ds);	// skip block
}

void av_resync(void)
{
	switch(((sh_audio_t *) filter_graph.demux->sh_audio)->format)
	{
		case MP3_50:
		case MP3_55:
		case MP3_CVBR:
		case MP3_FCC:
		case 0x56 :				//Video status case
		case 0x58 :				//Video status case
			#if MP3_SW_AUDIO && AUDIO_ON
			MP_DEBUG("av mp3 resync");
			MagicPixel_mp3_resync(1, (DWORD)(filter_graph.demux->video->pts * 1000.0f));
			#endif
			break;
		case 0x2000:
			#if AC3_AUDIO_ENABLE && AUDIO_ON
			MP_DEBUG("av AC3 resync");
			MagicPixel_ac3_resync(1,(DWORD)(filter_graph.demux->video->pts * 1000.0f));
			MagicPixel_ac3_resync_post_function();
			#endif
			break;
	}
}

//For getting audio track information by call decoder initialization API
int GetAudioTrackInfo(STREAM *sHandle, FILE_TYPE_T file_type, DWORD * err_code,BYTE * buffer)
{
	//mpDebugPrint("Get Media info sw decode...");
	//mpDebugPrint("File type: %d", file_type);

	int mpeg2, layer, ret, header_type;
	unsigned char * song_singer, *song_name, *song_album, *song_comment, *song_genre, *song_year, *song_track, total_frame;
	unsigned int tmpWMA_imgSize, tmpWMA_imageOffset;	//You should implement those two variables what you want (You can discuss it with Audio engineer)
	int format = 0;							//For wav case
	char g_song_track;

	PMedia_Info pMediaInfo = (PMedia_Info) buffer;
	MP_DEBUG1("pMediaInfo=0x%x", pMediaInfo);

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	memset((char *) SRAM_ADR, 0, 24 * 1024);
#else
	memset((char *) SRAM_ADR, 0, 16 * 1024);
    memset((char *) 0xafc00000, 0, 16 * 1024);
#endif    
    memset(&Media_data, 0, sizeof(Audio_dec));
	
    Media_data.file_size = FileSizeGet(sHandle);
    SeekSet(sHandle);

    Media_data.dec_fp  = sHandle;
    Media_data.fending = 0;
    AO_AV = AO_TYPE;

	song_track = &g_song_track;

    //Media_data.sdram_buf=(char *)((BYTE *) (SystemGetMemAddr(AV_BUF_MEM_ID) ))+1024*1024;//the first 80K will be used
	//memset((char *)Media_data.sdram_buf , 0 , 300*1024);

	MP_DEBUG1("file_type=%d",file_type);

    switch(file_type)
    {
		case FILE_TYPE_MP3 :
#if (MP3_MAD_AUDIO && AUDIO_ON)
			ret = MagicPixel_mp3_mad_init(Media_data.file_size,&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,
										&song_singer,&song_name,&song_album,&song_comment,&song_genre,&song_year,&song_track,&mpeg2,&layer, 1);
			memcpy(&pMediaInfo->id3_tag.Track, song_track, 1);
			Free_BitstreamCache();
#elif (MP3_SW_AUDIO && AUDIO_ON)
			//song_track = &g_song_track;
			ret = MagicPixel_mp3_init(Media_data.file_size,0,&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,
									&song_singer,&song_name,&song_album,&song_comment,&song_genre,&song_year,song_track,&mpeg2,&layer,AO_AV);
			//memcpy(&pMediaInfo->id3_tag.Track, song_track, 1);
#endif
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Track_USEFUL;
			break;

#if (WMA_ENABLE && AUDIO_ON)
		case FILE_TYPE_WMA :
			ret= MagicPixel_ASF_AO_Init(&Media_data.ch, &Media_data.srate, &Media_data.bitrate, &Media_data.total_time,
										&song_singer, &song_name, &song_album, &song_genre, &song_year, &song_track, 0,
										&tmpWMA_imgSize, &tmpWMA_imageOffset);

			memcpy(&pMediaInfo->id3_tag.Track, song_track, 1);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Track_USEFUL;
			MP_DEBUG1("ret_wma=0x%x", ret);
			break;
#endif

#if (WAV_ENABLE && AUDIO_ON)
		case FILE_TYPE_WAV :
			ret = MagicPixel_wav_init(Media_data.file_size,&Media_data.ch,&Media_data.srate,&Media_data.bitrate,&Media_data.total_time,&Media_data.frame, &format,&Media_data.blockalign,&Media_data.waveoffset);

			if (Media_data.total_time == 0)
				Media_data.total_time  = 1;
			//Free_BitstreamCache();	//Looks this command is not neeeded  //C.W 091211
			break;
#endif

#if OGG_ENABLE && AUDIO_ON
		case FILE_TYPE_OGG :
			Media_data.frame_num = 0;
			Media_data.sdram_buf2= (BYTE *) mem_malloc(SDRAM_SIZE_OGG);
			memset((char *)Media_data.sdram_buf2,0,SDRAM_SIZE_OGG);
			ret = MagicPixel_OGG_init(Media_data.file_size,&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,
										&song_singer,&song_name, Media_data.sdram_buf2, SDRAM_SIZE_OGG);
			break;
#endif

#if (RAM_AUDIO_ENABLE && AUDIO_ON)
		case FILE_TYPE_RAM:
		case FILE_TYPE_RA :
		case FILE_TYPE_RM :
			Media_data.sdram_buf = (int *)mem_malloc(SDRAM_SIZE_RAM);

			ret = MagicPixel_ram_init(&Media_data.ch, &Media_data.srate, &Media_data.frame_size, &Media_data.bitrate,
										&Media_data.total_time, &song_singer,&song_name, Media_data.sdram_buf, SDRAM_SIZE_RAM);

			if (ret == 0x80040201) Media_data.total_time = 0;
			break;
#endif

#if (AC3_AUDIO_ENABLE && AUDIO_ON)
		case FILE_TYPE_AC3 :
			sdbuf_left = 0;
			Media_data.sdram_buf = (int *)mem_malloc(SDRAM_SIZE_AC3);
			Media_data.sdram_buf = (int *)((DWORD)Media_data.sdram_buf | 0x20000000);
			ret = MagicPixel_ac3_init(&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,Media_data.file_size);
			MP_DEBUG4("Media_data.ch=%d,Media_data.srate=%d,Media_data.frame_size=%d,Media_data.bitrate=%d",Media_data.ch,Media_data.srate,Media_data.frame_size,Media_data.bitrate);
			break;
#endif

		case FILE_TYPE_AAC :
		case FILE_TYPE_M4A :
#if (AAC_SW_AUDIO && AUDIO_ON)
			Media_data.sdram_buf = (int *) mem_malloc(SDRAM_SIZE_AAC);
			Media_data.sdram_buf = (int *)((DWORD)Media_data.sdram_buf | 0x20000000);
			ret = MagicPixel_mp4_init(Media_data.file_size, &Media_data.srate, &Media_data.ch,
									&Media_data.bitrate, &Media_data.total_time,
									(int *)&total_frame, &header_type, Media_data.sdram_buf,
									SDRAM_SIZE_AAC, (char **)&song_singer, (char **)&song_name,
									(char **)&song_comment, (char **)&song_album,
									(char **)&song_genre, (char **)&song_year,
									(char *)song_track, 1); // 1 is for file browser

			MP_DEBUG1("ret_mp4=%d", ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Track_USEFUL;
			memcpy(&pMediaInfo->id3_tag.Track, song_track, 1);
			break;
#elif AAC_FAAD_AUDIO && AUDIO_ON
			//mpDebugPrint("--AAC faad codec init --");
			ret = MagicPixel_AAC_FAAD_init(Media_data.file_size,&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,
											&song_singer,&song_name,&song_album,&song_comment,&song_genre,&song_year,&song_track,&mpeg2,&layer,1);
			break;
#else
			mpDebugPrint("[Warning] There is no AAC codec installed");
#endif
#if (AMR_ENABLE && AUDIO_ON)
		case FILE_TYPE_AMR :
			ret = MagicPixel_amr_init(Media_data.file_size,&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time);
			mpDebugPrint("AMR bit rate: %d, total time: %d", Media_data.bitrate, Media_data.total_time);
#endif
		default:
			break;
	}


	pMediaInfo->dwSampleRate = Media_data.srate;
	pMediaInfo->dwSampleSize = 2;	
	pMediaInfo->dwBitrate = Media_data.bitrate;
	pMediaInfo->dwTotalTime = Media_data.total_time;

	pMediaInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_AUDIO;

	if (pMediaInfo->dwSampleRate > 0)
		pMediaInfo->dwFlags |= (DWORD) MOVIE_SampleRate_USEFUL;

	if (pMediaInfo->dwSampleSize == 2)  //O_bps will use
		pMediaInfo->dwFlags |= (DWORD) MOVIE_SampleSize_USEFUL;

	if (pMediaInfo->dwBitrate > 0)
		pMediaInfo->dwFlags |= (DWORD) MOVIE_Bitrate_USEFUL;

	if((Media_data.bitrate > 0) && ((pMediaInfo->dwTotalTime) >= 0))
		pMediaInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;

	if(Media_data.sdram_buf){
		mem_free(Media_data.sdram_buf);
		Media_data.sdram_buf = NULL;
	}
	if(Media_data.sdram_buf2){
		mem_free(Media_data.sdram_buf2);
		Media_data.sdram_buf2 = NULL;
	}
	
#if WAV_ENABLE
	if (file_type !=  FILE_TYPE_WAV)
#endif
	{
		ret = strlen(song_name);
		if (ret > 0)
		{
			if (ret >31)
				ret = 31;
			memcpy(pMediaInfo->id3_tag.Title, song_name,ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Title_USEFUL;
		}
	
		ret = strlen(song_singer);
		if (ret > 0)
		{
			if (ret > 31)
				ret =31;
			memcpy(pMediaInfo->id3_tag.Artist, song_singer, ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Artist_USEFUL;
		}
	
		ret = strlen(song_album);
		if (ret > 0)
		{
			if (ret > 31)
					ret = 31;
			memcpy(pMediaInfo->id3_tag.Album, song_album, ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Album_USEFUL;
		}
			  
		ret = strlen(song_comment);
		if (ret > 0)
		{
			if (ret > 31)
				ret =31;
			memcpy(pMediaInfo->id3_tag.Comment, song_comment, ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Comment_USEFUL;
		}
	
		ret = strlen(song_genre);
		if (ret > 0)
		{
			if (ret > 31)
				ret =31;
			memcpy(pMediaInfo->id3_tag.Genre, song_genre, ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Genre_USEFUL;
		}
	
		ret = strlen(song_year);
		if (ret > 0)
		{
			if (ret > 5)
				ret =5;
			memcpy(pMediaInfo->id3_tag.Year, song_year, ret);
			pMediaInfo->dwFlags |= (DWORD) MOVIE_ID3_Year_USEFUL;
		}
	}

	return PASS;
}

#endif

