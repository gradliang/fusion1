/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      soft_ulaw.c
*
* Programmer:    Rebecca Hsieh
*                MP E320 division
*
* Created:   08/15/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     08/15/2006    Rebecca Hsieh    first file
****************************************************************
*/
///@mainpage Libmad API Use Guaid
///
///This control audio playing. Mp61X contains wrapper for every codecs, some of them include the
///codec function implementation, some calls functions from other files, some calls optional external libraries.
///
///AVSync model doesn't call them directly, but through the decoder_audio.c and video_decoder.c files, 
///so the AVSync doesn't have to know anything about the codecs.


/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mptrace.h"
#include "avTypeDef.h"
#include "debug.h"
#include "amu.h"
#include "../../../audio/include/audio_hal.h"

static MPXWave *mpxw;
static int fending;
static unsigned char *bs_buffer;
static unsigned char *bs_buffer_buffer;

static int buflen,wave_format;

static int BlockAlign;
static int bufptr;
extern DWORD audiostream_finish;
static ad_info_t info = {
	"Magic Pixel wave audio decoder",
	"alaw",
	"ulaw",
	"dvi_adpcm",
	"based on E360...."
};
BYTE buflen_softaviwave;

extern enum AO_AV_TYPE AO_AV;
extern Audio_dec       Media_data;
extern DWORD backward_flag;
extern DWORD forward_flag;
extern int left_channel;
extern int right_channel;

#define WAVE_TYPE 0

static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static void uninit(sh_audio_t * sh);
static int _decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

///@defgroup    uLAW Decode API
///This control audio playing

///@ingroup Magic Pixel
///@defgroup INIT   Initialization

///@ingroup INIT
///@brief   Init the audio system. Allocate space for audio decoder and 
///         set the minimize size of buffers for both input and output audio data. 
///
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.             
///
///@return  static int                  Return 0 for fail and 1 for success.
static int preinit(sh_audio_t * sh)
{
	sh->audio_out_minsize = 8192;//uncompress packet size
	sh->audio_in_minsize = 1024;
	return 1;
}

static int init(sh_audio_t * sh)
{
	int format;
	mpDebugPrint("Init Soft AVI_WAVE");
	mpxw = (MPXWave *) mem_malloc(sizeof(MPXWave) );
	Init_BitstreamCache(WAVE_TYPE);

	bs_buffer_buffer = NULL;
	if(AO_AV == AV_TYPE){
		audiostream_finish = 0;
		sh->channels	= ((WAVEFORMATEX*)sh->wf)->nChannels;
		sh->samplerate	= ((WAVEFORMATEX*)sh->wf)->nSamplesPerSec;
		sh->i_bps		= ((WAVEFORMATEX*)sh->wf)->nAvgBytesPerSec * 8 ;
		sh->samplesize	= 2;//((WAVEFORMATEX*)sh->wf)->wBitsPerSample / 8;//2;
		
		mpxw->ch		= ((WAVEFORMATEX*)sh->wf)->nChannels;
		BlockAlign		= ((WAVEFORMATEX*)sh->wf)->nBlockAlign;//avi has value
		switch(sh->format)
		{
		    case 0xFFFE:
				mpxw->tag =0xFFFE;
			    break;
			case PCM_1:
				mpxw->tag = 0x01;

				if(sh->channels < 1){
					MP_ALERT("Unsupport one channel mode at PCM_1 format");
					uninit(sh);
					return -1;
				}	
				break;
			case 0x7  :
			case ulaw :
			case ULAW :
				mpxw->tag = 0x7;
			break;
			case 0x6  :
			case alaw :
			case ALAW : 
				mpxw->tag = 0x6;
			break;
			case 17   :
			case DVI_ADPCM :
			case 0x1100736d:
				mpxw->tag = 0x11;
			break;
			case PCM_RAW   :
			case PCM_SOWT :
			case PCM_TWOS:
				sh->samplesize	= 2;//((WAVEFORMATEX*)sh->wf)->wBitsPerSample / 8;
				mpxw->tag = 0x0;
				BlockAlign = 0;				
			break;
			case 2 :
			case 0x809DDF48:
				mpxw->tag =0x2;
				break;
				//MP_ALERT("This wav format %d is not supported now we will add ASAP");
				//return -1;
			default :
				MP_ALERT("This wav format %d is not supported", sh->format);
				uninit(sh);
				return -1;
			}
		}

	else{	// AO case
		unsigned int format;
		//Init_BitstreamCache(WAVE_TYPE);
		MagicPixel_wav_init(Media_data.file_size, &Media_data.ch,
							&Media_data.srate, &Media_data.bitrate,
							&Media_data.total_time,&Media_data.frame,&format,&Media_data.blockalign,&Media_data.waveoffset);
		
		sh->channels	= Media_data.ch;
		sh->samplerate	= Media_data.srate;
		sh->i_bps		= Media_data.bitrate;
		if (Media_data.total_time == 0)
			Media_data.total_time = 1;
		//sh->samplesize	= Media_data.frame;
		sh->samplesize	= 2;
		//if channel >2 {channel =2} in wave-lib
		mpxw->ch 		= sh->channels;
		BlockAlign		= 0;
		mpDebugPrint("Mad Decode audio, sh: %x", sh);
		mpDebugPrint("total_time:%d",Media_data.total_time);
		mpxw->tag = format;	
		if(format==0xFFFE)
		{
		    wave_format=format;
		    mpxw->tag =1;
		    if(sh->channels>2) {mpxw->ch = sh->channels=2;}
		}
		if(format != 1 && format != 7 && format != 6 && format != 0x11 && format !=0x2 && format != 0xFFFE)
		{
			MP_ALERT("This wav format %d is not supported", sh->format);
			uninit(sh);
			return -1;
		}
		bs_buffer	= InitBitStreamBufferTo();
	}

	fending		= 0;
	bufptr		= 0;
	buflen		= 0;
	bs_buffer	= InitBitStreamBufferTo();
	
	return 0;
}

///@ingroup Magic Pixel
///@defgroup DECODE   Decode

///@ingroup DECODE
///@brief       Decode the compress data from demux into audio_out buffer at a certain length
///@param   sh_audio_t *sh  Pointer to the stream header structure used
///@param   unsigned char *buf  Pointer to the audio_out buffer 
///@param   int minlen      The max length of decoded audio out data
///@param   int maxlen      The max length of decoded audio out data ??what's diff???
///@return  The length of actual decoded audio data, if success
///         -1, if failed
static int _decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
	//mpDebugPrint("soft avi wave decoding...len: %d / %d", minlen, maxlen);
	int pcm_len=0;//word number
	int len = 0,i,j;
	unsigned char * temp_buf,*buf_flag,accumulate=0;
	short *buffer = (short *)buf;
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);
	if(Media_data.fending == 0)
	fending = 0;

	switch (mpxw->tag) //move from mpapi_inter.c for endian
	{	//PCM data is little endian
        case 0xFFFE:
		case 1:
			AIU_SET_LITTLEENDIAN();
			break;

		case 6:
		case 7:
			AIU_SET_BIGENDIAN();
			break;

		default:
			break;
	}
	//It should not use the following code if adding a condition above to distinguish Media_data.fending equals to zero
	/*
	if (!audiostream_finish && buflen == -1)
	{
		fending = 0;
		Media_data.fending = 0;
		buflen = 0;
	}*/
	if (fending)
		return len ? len : -1;
	j = 0;

	if (sh->samplerate < 16000)
	{
		if (sh->format == PCM_RAW)
			minlen = 2048;
		else
			minlen = 4096;
	}

	while (len < minlen )  // && len + 4608 <= maxlen)
	{
		if (buflen==0)
		{		
			if(AO_AV == AV_TYPE)	
			{
				buflen = ds_get_packet(sh->ds, &bs_buffer);
			}	
			else
			{
				if (fending == 0)
				{ 
					if (mpxw->tag == 0x11 || mpxw->tag == 0x2)
						buflen = MagicPixel_WAV_bitstream_callback(bs_buffer, Media_data.blockalign, &fending);
					else
						buflen = MagicPixel_WAV_bitstream_callback(bs_buffer, 4096, &fending);
				}
				else 
					break;
			}
			if (buflen <= 0) 
			{
				audiostream_finish = 1;
				fending = 1;
				Media_data.fending = 1;
				buflen = 0;
				break;
			}
			buflen_softaviwave = 0;
			bufptr = 0;

		}

		temp_buf = bs_buffer + bufptr;	
		if (buflen_softaviwave == 1)
		{
			buflen = 0;
			continue;
		}
		if(BlockAlign)
		{
			if (mpxw->tag != 0x11 && mpxw->tag!= 0x2)
			{
				if (buflen < BlockAlign<<10)
				{
					//   For 1 channel and samplesize = 8bits
					if (((WAVEFORMATEX*)sh->wf)->wBitsPerSample  == 8 && mpxw->tag == 0x1)
					{
						j = 0;
						for (i=0; i<buflen; i++)
						{
							//accumulate += temp_buf[i];
							buf[j] =0x0;
							buf[j+1] = temp_buf[i] - 0x80;
							j+=2;
						}
						if (accumulate == 0)
						{
							//memset(buf, 0x00, buflen<<1);
						}			
						accumulate = 0;
						pcm_len = buflen<<1; 
					}
					else
						{
						pcm_len = MPXWaveDecoder(mpxw, temp_buf,(short *) buf,buflen);
						}
					bufptr += buflen;
					buflen -= buflen;
				}
				else
				{
					//   For 1 channel and samplesize = 8bits
					if (((WAVEFORMATEX*)sh->wf)->wBitsPerSample  == 8 && mpxw->tag == 0x1)
					{
						j = 0;
						for (i=0; i<BlockAlign<<10; i++)
						{
							//accumulate += temp_buf[i];
							buf[j] =0x0;
							buf[j+1] = temp_buf[i]- 0x80;
							j+=2;
						}
						// Prevent high frequency noise
						if (accumulate == 0)
						{
							//memset(buf, 0x00, BlockAlign<<11);
						}														
						accumulate = 0;
						pcm_len = BlockAlign<<11;						
					}	
					else
					{	
						pcm_len = MPXWaveDecoder(mpxw, temp_buf,(short *) buf, BlockAlign<<10);
					}
					buflen -= (BlockAlign<<10);
					bufptr += (BlockAlign<<10);	
					
				}
			}
			else
			{
				if (buflen<BlockAlign)
				{   
					pcm_len = MPXWaveDecoder(mpxw, temp_buf,(short *) buf,buflen);
					bufptr += buflen;
					buflen -= buflen;		
				}
				else
				{
					pcm_len = MPXWaveDecoder(mpxw, temp_buf,(short *) buf, BlockAlign);
					buflen -= (BlockAlign);
					bufptr += (BlockAlign);					
				}
			}
		}
		else
		{	
			//   For 1 channel and samplesize = 8bits
			if ((((WAVEFORMATEX*)sh->wf)->wBitsPerSample  == 8 && mpxw->tag == 0x1)||(Media_data.frame == 1 && AO_AV == AO_TYPE &&mpxw->tag == 0x1))
			{  
			    if(wave_format == 0xFFFE) //For 4 channel
			    {
					j = 0;
					for (i=0; i<buflen; i+=2)
					{
                        buf[j] =0x0;
						buf[j+1] = ((temp_buf[i]/2) +(temp_buf[i+1]/2)) - 0x80;
						j+=2;                   
					}
					pcm_len = buflen;
					bufptr += buflen;
					buflen -= buflen;
			    }
				else 
				{
					j = 0;
					for (i=0; i<buflen; i++)
					{
						buf[j] =0x0;
						buf[j+1] = temp_buf[i] - 0x80;
						j+=2;
					}
					pcm_len = buflen<<1;
					bufptr += buflen;
					buflen -= buflen;
			    }
			}
			else if(mpxw->tag == 0x11)
			{
				pcm_len = MPXWaveDecoder(mpxw, temp_buf,(short *) buf,buflen);
				bufptr += buflen;
				buflen -= buflen;	
			}				
			else if (sh->format ==PCM_RAW)
			{
				if (((WAVEFORMATEX*)sh->wf)->wBitsPerSample / 8 == 2 )
				{
					if (buflen < minlen)
					{				
						for(i = 0; i < buflen; i++)
						{
							*buf = *(temp_buf + i);
							buf++;
						}
						len += buflen;
						buflen = 0;
#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)						
						break;
#endif						
					}
					else
					{
						for(i = 0; i < minlen; i++)
						{
							*buf = *(temp_buf + i);
							buf++;
						}
						len += minlen;
						buflen -= minlen;
						bufptr += minlen;
					}
				}
				else
				{ 
					if (buflen < minlen)
					{

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650 || (CHIP_VER & 0xFFFF0000) == CHIP_VER_660)
						#if 0
						mmcp_memcpy_polling(buf,temp_buf,buflen);
						buf+=buflen;	
						#endif
						#if 1
						for (i=0; i<buflen; i++)
						{
							*buf = ((WORD)(temp_buf[i]<<6)&0xff00)>>8;
							buf++;
							*buf = temp_buf[i]<<6;
							buf++;	
						}
						#endif
#endif
#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)	
						for (i=0; i<buflen; i++)
						{
							*buf = temp_buf[i]-0x80;
							buf++;	
						}
#endif	
						len +=2*buflen;
						bufptr += buflen;
						buflen = 0;
						TaskYield();
#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)
						//Add a break 09/08/18
						break;
#endif
					}
					else
					{
#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650 || (CHIP_VER & 0xFFFF0000) == CHIP_VER_660)	
						#if 0
							mmcp_memcpy_polling(buf,temp_buf,minlen);
							buf+=minlen;
						#endif	
						#if 1	
						for (i=0; i<minlen; i++)
						{
							*buf = ((WORD)(temp_buf[i]<<6)&0xff00)>>8;
							buf++;
							*buf = temp_buf[i]<<6;
							buf++;	
						}
						#endif
#endif	

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)	
						for (i=0; i<minlen; i++)
						{		

							*buf = temp_buf[i]-0x80;
							buf++;						
						}
#endif								
						len +=2*minlen;
						buflen -= minlen;
						bufptr += minlen;			
					}
					TaskYield();
				}			
			}
			else if (sh->format == PCM_TWOS)
			{
				if (buflen < minlen)
				{
					for(i = 0; i < buflen; i++)
					{	
						*buf = (*(temp_buf + i));
						buf++;
					}
					len += buflen;
					buflen = 0;
//#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)						
					break;
//#endif
				}
				else
				{
					for(i = 0; i < minlen; i++)
					{	
						*buf = (*(temp_buf + i));
						buf++;
					}
					len += minlen;
					buflen -= minlen;
					bufptr += minlen;
					TaskYield();
				}
			}
			else if (sh->format == PCM_SOWT)
			{
				if (buflen < minlen)
				{
					for(i = 0; i < buflen; i+=2)
					{	
						*buf = (*(temp_buf + i+1));
						buf++;
						*buf = (*(temp_buf +i));
						buf++;					
					}
					len += buflen;
					buflen = 0;
//#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)						
					break;
//#endif
				}
				else
				{
					for(i = 0; i < minlen; i+=2)
					{		
						*buf = (*(temp_buf + i+1));
						buf++;
						*buf = (*(temp_buf +i));
						buf++;					
					}
					len += minlen;
					buflen -= minlen;
					bufptr += minlen;
					TaskYield();
				}							
			}
			else
			{   
				pcm_len = MPXWaveDecoder(mpxw, temp_buf,(short *) buf,buflen);
				bufptr += buflen;
				buflen -= buflen;	
			}
		}
		if((mpxw->tag == 1 || mpxw->tag == 0xFFFE) && pcm_len > 0 )	//PCM_1 case: caculate method is different(pcm_len means number of bytes)
		{
			if (left_channel && !right_channel)
			{
				for(i= 0; i<pcm_len;i+=4)
				{
					buf[i+2] = buf[i];
					buf[i+3] = buf[i+1];
				}
			}
			else if (!left_channel && right_channel)
			{
				for(i= 0; i<pcm_len;i+=4)
				{
					buf[i] = buf[i+2];
					buf[i+1] = buf[i+3];
				}
			}

			len += pcm_len;
			buf += pcm_len;
		}
		else if(pcm_len > 0)
		{
			if (left_channel && !right_channel)
			{
				for(i= 0; i<2*pcm_len;i+=4)
				{
					buf[i+2] = buf[i];
					buf[i+3] = buf[i+1];
				}
			}
			else if (!left_channel && right_channel)
			{
				for(i= 0; i<2*pcm_len;i+=4)
				{
					buf[i] = buf[i+2];
					buf[i+1] = buf[i+3];
				}
			}
		
			len +=  2*pcm_len;// short * frame_size(always 2 channe) 
			buf +=  2*pcm_len;
		}
		else
		{
			//UartOutValue(pcm_len, 5);
			//UartOutText("<--pcm_len,Decode error in ADPCM\r\n");
		}
		if (AO_AV == AO_TYPE)
		{  //4
			Media_data.play_time = (MagicPixel_WAV_getposition_callback(Media_data.dec_fp)-44)/(Media_data.bitrate)*1000;
		}
		
		TaskYield();
		
		if (minlen - len <2048)
			break;
		
	}
	
	return len ? len : -1;
}


///@ingroup Magic Pixel
///@defgroup CON   Control

///@ingroup CON
///@brief           Decode the compress data from demux into play out buffer at a certain length
///@param   sh_audio_t *sh  Pointer to the stream header structure used
///@param   int cmd         Type of command
///@param   void *arg           Pointer to argument, not yet used
///@return  CONTROL_OK for success
///     CONTROL_UNKNOW for fail
///@remark  This is for reading/setting special parameters and can be used for keyboard input for example.  
///         control codes are found in audio_decoder.h.
static int control(sh_audio_t * sh, int cmd, void *arg, ...)
{
	return CONTROL_UNKNOWN;
}

static int resync(unsigned int second)
{
	return MagicPixel_WAV_resync(second);
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{

	///if(AO_AV == AO_TYPE)
	Free_BitstreamCache();
	//if (bs_buffer_buffer != NULL)
		//mem_free(bs_buffer_buffer);	
	mem_free(mpxw);


	bs_buffer_buffer = NULL;
}

ad_functions_t software_aviwave = {
	&info,
	preinit,
	init,
	uninit,
	control,
	_decode_audio,
	resync
};

int MagicPixel_WAV_resync(int Next_Second)
{
#if WAV_ENABLE
	DWORD ret, numberpacket;
	mpDebugPrint("Next_Second %d", Next_Second);

	ret = (Next_Second / 1000) * (Media_data.bitrate) + 44;

	if (ret >= Media_data.file_size)
		ret = Media_data.file_size;

	ClearBitstreamCache();
	numberpacket = ret / Media_data.blockalign;
	ret = Media_data.waveoffset + Media_data.blockalign * numberpacket;
	MagicPixel_WAV_fileseek_callback(ret);
#endif

    return 0;
}

