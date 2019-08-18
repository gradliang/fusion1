/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_h264.c
*
* Programmer:    Kevin
*                
*
* Created: 	 x/x/2011
*
* Description: h264 stream data file demux implementation 
*        
****************************************************************
*/

#include "global612.h"
#if VIDEO_ENABLE

#include "demux_types.h"
#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux_packet.h"
#include "demux.h"
#include "stheader.h"
#include "filter_graph.h"


extern demuxer_t *new_base_demux();



static demuxer_t *demux = 0;
static int seek_next=0;
static int value_inital=0;
static BYTE *tBuf1;
static BYTE *Buf_64k;


demux_stream_t *demux_h264_select_stream(demuxer_t * demux, unsigned int id)
{
	return NULL;
}

static int valid_fourcc(unsigned int id)
{
	unsigned char *fcc = (unsigned char *) (&id);

//#define FCC_CHR_CHECK(x) (x<48 || x>=96)
#define FCC_CHR_CHECK(x) (x<48 || x>122)
	if (FCC_CHR_CHECK(fcc[0]))
		return 0;
	if (FCC_CHR_CHECK(fcc[1]))
		return 0;
	if (FCC_CHR_CHECK(fcc[2]))
		return 0;
	if (FCC_CHR_CHECK(fcc[3]))
		return 0;
	return 1;
#undef FCC_CHR_CHECK
}

//======================================
int dts, pts ;

static int h264_read_header(demuxer_t *demuxer)
{
    int offset;
	
    unsigned char flags;
      demux_stream_t *d_audio = demuxer->audio;
      demux_stream_t *d_video = demuxer->video;
      sh_audio_t *sh_audio = NULL;
      sh_video_t *sh_video = NULL;
	  seek_next=0;
      value_inital=0;
	pts=0;  
    stream_reset(demuxer->stream);
    stream_seek(demuxer->stream, 0);
	demux->movi_start = stream_tell(demuxer->stream);
	sh_video = new_sh_video(demuxer, 0);
    ds_fill_buffer(demuxer->video);
	demuxer->video->sh = sh_video;
	sh_video->ds = demuxer->video;
	demuxer->sh_video = sh_video;    
	
    return 0;
}
//*********************************************************************************
#if  !USBOTG_WEB_CAM

static int _fill_buffer(demux_stream_t * ds)
{
	value_inital++;
	 //=====
    int ret, i, type, size, flags, is_audio,befor_point;
    static int next, pos,ui_1,ui_2;
   
   
    stream_t * stream;
    sh_audio_t * sh_audio = (sh_audio_t *)demux->sh_audio;
    sh_video_t * sh_video = (sh_video_t *)demux->sh_video;
	sh_video->format=826496577;//0x31435641=1cva=avc1
	sh_video->fps=24.0;
	sh_video->frametime =  1.0/sh_video->fps;

	 stream = demux->stream;
	
	pts=pts+(sh_video->frametime*1000);
//	mpDebugPrint("value_inital=%d",value_inital);
	if(1==value_inital)
	{
	tBuf1 = (BYTE *)ext_mem_malloc(1024*200);
	Buf_64k = (BYTE *)ext_mem_malloc(1024*64);
		size=0;
		pos=0;
		next=0;
		pts=0;
		stream_read_char(stream);
		stream_read_char(stream);
		stream_read_char(stream);
		stream_read_char(stream);
	}
		mmcp_memcpy_polling(Buf_64k, stream->buffer, 1024*64);
		befor_point=0;
	if(1!=value_inital)
		{stream_seek(stream, next); 
		if(stream_eof(stream))return 0;
	    }
	 	pos=stream_tell(stream);

	if(stream_eof(stream))return 0;
	

	 for(;;)
	 {
	
	 if(0==stream_read_char(stream))
	   {
	   	if(stream_eof(stream))return 0;
	

	 	if(0==stream_read_char(stream))
	 	  {
	
		  if(0==stream_read_char(stream))
	 	    {
			if(1==stream_read_char(stream))
	 	      {
			  	size=stream_tell(stream)-pos-4;
				if(1==value_inital)next=next+size+4+4;
                else next=next+size+4;
				if((size+4)>stream->buf_pos)befor_point=1;			
				//stream_seek(stream, pos);
				//stream_seek(stream, pos);
			    break;
	 	      }
	 	    }
	 	  }

	 	}
	  if (stream_eof(stream))
        return 0;
	 }
/*
mpDebugPrint("_____________");
mpDebugPrint("stream->buf_pos=%d",stream->buf_pos);
mpDebugPrint("size=%d",size);
mpDebugPrint("pos=%d",pos);
mpDebugPrint("next=%d",next);
mpDebugPrint("_____________");
*/
	    tBuf1[0]=0;
		tBuf1[1]=0;
		tBuf1[2]=0;
		tBuf1[3]=1;

	if(befor_point>0)
	{
		//openwrite=1;
		mpDebugPrint("befor_point");
		ui_1=size+4-stream->buf_pos;
	ui_2=stream->buf_pos-4;
		if(stream->buf_pos<=4)
		{
		memcpy(tBuf1+4 ,Buf_64k+(1024*64)-ui_1, ui_1);
		}
		else
		{
		memcpy(tBuf1+4,Buf_64k+(1024*64)-ui_1, ui_1);	
		stream_seek(stream, pos+ui_1);
		mpDebugPrint("seek_read=%d",ui_1);
		stream_read(stream,tBuf1+4+ui_1, ui_2);
		}
	}
	else
	{
		mpDebugPrint("==okokok==");
		stream_seek(stream, pos);
		mpDebugPrint("=*");
	 stream_read(stream,tBuf1+4, size);
	}
	 //+++++++++++++++++++++++
	 ds_read_packet_pureh264(ds, tBuf1, size+4, pts, pos, flags);  
	
		  return 1;
}
#else
static int _fill_buffer(demux_stream_t * ds)
{
WORD YieldTime;
WORD  NoNetDatacounter = 0;

//	mpDebugPrint("_fill_buffer=");
	value_inital++;
	 //=====
    int ret, i, type, size, flags, is_audio,JumpFrameNum;
    static int next, pos, ui_1, ui_2;
    //AVStream *st = NULL;
    stream_t * stream;
    sh_audio_t * sh_audio = (sh_audio_t *)demux->sh_audio;
    sh_video_t * sh_video = (sh_video_t *)demux->sh_video;
	sh_video->format=826496577;//0x31435641=1cva=avc1
	sh_video->fps= 22.0;
	sh_video->frametime =  1.0/sh_video->fps;


	 stream = demux->stream;

	pts=pts+(sh_video->frametime*1000);
//	mpDebugPrint("value_inital=%d",value_inital);
	if(1 == value_inital)
	{
		size=0;
		pos=0;
		next=0;
		pts=0;
	}


		 stream_seek(stream, next);

			while(1)
			{
				size = VideoQueue_ReadVideoFrameSize();
				//mpDebugPrint("Y=%d",GetSysTime());
				if(size >0)
				{
					NoNetDatacounter = 0;
				break;
				}
			//	UartOutText("Y");
				TaskYield();
			}


	 
	 next=next+size;
	 	
	 ds_read_packet2(ds, stream, size, pts, pos, flags);  
	  return 1;
}


#endif


extern void resync_audio_stream(sh_audio_t * sh_audio);
static int _seek(float rel_seek_secs, int flags)
{
    
	return 1;
}


static int _close()
{
	value_inital=0;
	mem_free(tBuf1);
	mem_free(Buf_64k);

	return 1;
}


static int _control(int cmd, void *arg)
{
	
		return 1;
	
}


static int _check_type(stream_t * stream, DEMUX_T demux_type)
{
	int id;

	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_h264))
	{



		stream_reset(stream);
		stream_seek(stream, stream->start_pos);
		demux->stream = stream;

		//---- RIFF header:
		id = stream_read_dword_le(demux->stream);	// "RIFF"
		if (id == mmioFOURCC(0x00, 0x00, 0x00, 0x01))
		{
			return 1;
		}
		//MP_DEBUG ( "not avi: unknown   \r\n" );
		return (demux_type == DEMUX_UNKNOWN) ? 0 : -1;
	}
	return 0;
}

static int _open()
{

		 mpDebugPrint("_open_open_open_open_");
	demux->seekable = 0;

  if(h264_read_header(demux))  // if flv read header failed, return 0
	 return 0;
	return 1;
}


static int _get_movie_info(BYTE * buffer, int command)
{
	BYTE *string;
	unsigned int  t1, t2;
	int bits;
return 0;
	switch (command)
	{
	  case 0:
		{

			t1 = t2 = demux->stream->file_len;
			t1 = t1 / 1024;
			t2 = t2 / 1024;

			bits = 0;
			while (t2)
			{
				t2 /= 10;
				bits++;
			}

			string = buffer;
			t1 = Bin2Bcd(t1);
			string = HexString(string, t1, bits);

			*string = 'k';
			string++;
			*string = 0;
			return 1;
			//break;
		}

	  case 1:
		{
			t1 = t2 = demux->stream->pos;
			// convert time value to string
			t1 = t1 / 1024;
			t2 = t2 / 1024;

			bits = 0;
			while (t2)
			{
				t2 /= 10;
				bits++;
			}

			string = buffer;
			t1 = Bin2Bcd(t1);
			string = HexString(string, t1, bits);

			*string = 'k';
			string++;

			*string = 0;

			return 1;

		}
		
	  case 2:
		{
			t1 = t2 = demux->stream->pos * 100 / demux->stream->file_len;
			bits = 0;
			while (t2)
			{
				t2 /= 10;
				bits++;
			}
			if (bits == 0)
				bits = 1;

			string = buffer;
			t1 = Bin2Bcd(t1);
			string = HexString(string, t1, bits);

			*string = '%';
			string++;

			*string = 0;

			return 1;
		}

	  case 3:
		{
			PMedia_Info pInfo = (PMedia_Info) buffer;
			sh_video_t *sh_video = (sh_video_t *) demux->sh_video;
			sh_audio_t *sh_audio = (sh_audio_t *) demux->sh_audio;		
			if (sh_audio)
			{
                GetAudioCodecCategory(pInfo->cAudioCodec);
				
				pInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_AUDIO;

				if ((pInfo->dwSampleRate = sh_audio->samplerate) > 0)
					pInfo->dwFlags |= (DWORD) MOVIE_SampleRate_USEFUL;

				if ((pInfo->dwSampleSize = sh_audio->samplesize) > 0)
					pInfo->dwFlags |= (DWORD) MOVIE_SampleSize_USEFUL;

				if ((pInfo->dwBitrate = sh_audio->i_bps) > 0)
					pInfo->dwFlags |= (DWORD) MOVIE_Bitrate_USEFUL;
			}

			if (sh_video)
			{
			    GetVideoCodecCategory(pInfo->cVidoeCodec);
				
				pInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_VIDEO;

				if ((pInfo->dwFrameRate = sh_video->fps) > 0)
					pInfo->dwFlags |= (DWORD) MOVIE_FrameRate_USEFUL;

				if ((pInfo->dwImageHeight = sh_video->disp_h) > 0)
					pInfo->dwFlags |= (DWORD) MOVIE_ImageHeight_USEFUL;

				if ((pInfo->dwImageWidth = sh_video->disp_w) > 0)
					pInfo->dwFlags |= (DWORD) MOVIE_ImageWidth_USEFUL;

				if ((pInfo->dwTotalTime = 100/*(DWORD) flv_priv->duration*/) >= 0)
					pInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
			}

			return 1;
		}	

    default:
      break;
  }          

	return 0;
}


demuxer_t *new_h264_demux()
{
	demux = new_base_demux();
	// Assign all of the function pointers and type specific data
	demux->check_type = _check_type;
	demux->open = _open;
	demux->fill_buffer = _fill_buffer;
	demux->seek = _seek;
	demux->control = _control;
	demux->close = _close;
	demux->get_pts_by_relframe =NULL ;
	demux->get_movie_info =_get_movie_info;
	demux->type = DEMUXER_TYPE_h264;
	demux->file_format = DEMUXER_TYPE_h264;
	return demux;
}
#endif
