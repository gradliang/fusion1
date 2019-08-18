/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_h263.c
*
* Programmer:    Kevin
*                
*
* Created: 	 x/x/2011
*
* Description: h263 stream data file demux implementation 
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


demux_stream_t *demux_h263_select_stream(demuxer_t * demux, unsigned int id)
{
	return NULL;
}

static int valid_fourcc(unsigned int id)
{
	unsigned char *fcc = (unsigned char *) (&id);


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

int dts, pts ;

static int h263_read_header(demuxer_t *demuxer)
{
    int offset;
    unsigned char flags;
      demux_stream_t *d_audio = demuxer->audio;
      demux_stream_t *d_video = demuxer->video;
      sh_audio_t *sh_audio = NULL;
      sh_video_t *sh_video = NULL;
	  seek_next=0;
      value_inital=0;
	  dts=0;
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

static int _fill_buffer(demux_stream_t * ds)
{
	unsigned char mask_bit = 0x80;
	unsigned char bit_one;
	value_inital++;
	 //=====
    int ret, i, type, size, flags, is_audio;
    static int next, pos;
    stream_t * stream;
    sh_audio_t * sh_audio = (sh_audio_t *)demux->sh_audio;
    sh_video_t * sh_video = (sh_video_t *)demux->sh_video;
	sh_video->format=859189864;//0x33363268=362h=h263
	sh_video->fps=20.0;
	sh_video->frametime =  1.0/sh_video->fps;
	 stream = demux->stream;
	pts=pts+(sh_video->frametime*1000);
	if(1==value_inital)
	{
		size=0;
		pos=0;
		next=0;
		pts=0;
	}

	
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
		  	bit_one=stream_read_char(demux->stream);
		  if(bit_one & mask_bit)
	 	    {
			  	if(0!=seek_next)
			  	{
			  	size=stream_tell(stream)-pos-3;
				next=next+size;
				stream_seek(stream, pos);
				
			    break;
			  	}
				seek_next++; 
	 	    }
	 	  }

	 	}
	  if (stream_eof(stream))
        return 0;
	 }
	seek_next--; 

	 ds_read_packet2(ds, stream, size, pts, pos, flags);  

		  return 1;
}


extern void resync_audio_stream(sh_audio_t * sh_audio);
static int _seek(float rel_seek_secs, int flags)
{
    
	return 1;
}


static int _close()
{
	return 1;
}


static int _control(int cmd, void *arg)
{
		return 1;
}


static int _check_type(stream_t * stream, DEMUX_T demux_type)
{
	int id;
	unsigned char mask9 = 0x80;
	unsigned char k1;

	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_h263))
	{



		stream_reset(stream);
		stream_seek(stream, stream->start_pos);
		demux->stream = stream;

		//====================
		if(0==stream_read_char(demux->stream))
	 	{
		   if(0==stream_read_char(demux->stream))
		   {
		   	k1=stream_read_char(demux->stream);
		   	  if(k1 & mask9)
		   	  {                               
				return 1;
		   	  }
		   }
		}   	
		  	
		return (demux_type == DEMUX_UNKNOWN) ? 0 : -1;
	}

	//MP_DEBUG ( "not avi:  \r\n" );
	return 0;
}

static int _open()
{

		 mpDebugPrint("_open_open_open_open_");
	demux->seekable = 0;

  if(h263_read_header(demux))  // if flv read header failed, return 0
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

			// convert time value to string
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

				if ((pInfo->dwTotalTime =100 /*(DWORD) flv_priv->duration*/) >= 0)
					pInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
			}

			return 1;
		}	

    default:
      break;
  }          

	return 0;
}


demuxer_t *new_h263_demux()
{
	demux = new_base_demux();
	demux->check_type = _check_type;
	demux->open = _open;
	demux->fill_buffer = _fill_buffer;
	demux->seek = _seek;
	demux->control = _control;
	demux->close = _close;
	demux->get_pts_by_relframe =NULL ;
	demux->get_movie_info =_get_movie_info;
	demux->type = DEMUXER_TYPE_h263;
	demux->file_format = DEMUXER_TYPE_h263;
	return demux;
}
#endif
