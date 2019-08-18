/*****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2009, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_flv.c
*
* Programmer:    Mingfen lin
*                MPX E360 division
*
* Created: 	 04/28/2009
*
* Description: FLV format file demux implementation   
*              
*        
* Change History (most recent first):
*     <1>     04/28/2009    Mingfen Lin    first file
*****************************************************************/

#include "global612.h"
#if VIDEO_ON

#define LOCAL_DEBUG_ENABLE 0
#define MP_PERFORMANCE_TRACE 0

#include "global612.h"
#include "mpTrace.h"

#include "mpeg4audio.h"
#include "avformat.h"
#include "flv.h"

#include "errno.h"
#include "demux.h"
#include "stream.h"
#include "video_decoder.h"
#include "audio.h"


typedef struct {
    int wrong_dts; ///< wrong dts due to negative cts
} FLVContext;

// FLV demuxer
static demuxer_t *demux = 0;

typedef struct _flv_priv {
    uint8_t *extradata;
    int extradata_size;
    int duration;
    int is_streamed;  /**< true if streamed (no seek possible), default = false */
} flv_priv_t;

extern DWORD  g_dwTotalSeconds;

static int flv_probe(stream_t *s)
{
    const uint8_t *d;

    d = s->buffer;
    if (d[0] == 'F' && d[1] == 'L' && d[2] == 'V' && d[3] < 5 && d[5]==0) {
        return AVPROBE_SCORE_MAX;
    }
    return 0;
}

//static void flv_set_audio_codec(AVFormatContext *s, AVStream *astream, int flv_codecid) {
static void flv_set_audio_codec(sh_audio_t * sh_audio, int flv_codecid) {

//    AVCodecContext *acodec = astream->codec;
    switch(flv_codecid) {
        //no distinction between S16 and S8 PCM codec flags
        case FLV_CODECID_PCM:
//            sh_audio->format = sh_audio->samplesize == 1 ? CODEC_ID_PCM_S8 :
//#ifdef WORDS_BIGENDIAN
//                                CODEC_ID_PCM_S16BE;
//#else
//                                CODEC_ID_PCM_S16LE;
//#endif            
//            break;
        case FLV_CODECID_PCM_LE:
            sh_audio->format = PCM_1; 
//            sh_audio->format = sh_audio->samplesize == 1 ? CODEC_ID_PCM_S8 : CODEC_ID_PCM_S16LE; 
            break;
        case FLV_CODECID_AAC  : 
            sh_audio->format = mp4a;                                    
//            sh_audio->format = CODEC_ID_AAC;                                    
            break;
        case FLV_CODECID_ADPCM: 
            sh_audio->format = DVI_ADPCM;                                      
//            sh_audio->format = CODEC_ID_ADPCM_SWF;                              
            break;
//        case FLV_CODECID_SPEEX:
//            acodec->codec_id = CODEC_ID_SPEEX;
//            acodec->sample_rate = 16000;
//            break;
        case FLV_CODECID_MP3  : 
            sh_audio->format = MP3_FCC;              
//            sh_audio->format = CODEC_ID_MP3;      
            break;
//        case FLV_CODECID_NELLYMOSER_8KHZ_MONO:
//            acodec->sample_rate = 8000; //in case metadata does not otherwise declare samplerate
//        case FLV_CODECID_NELLYMOSER:
//            acodec->codec_id = CODEC_ID_NELLYMOSER;
//            break;
//        default:
//            av_log(s, AV_LOG_INFO, "Unsupported audio codec (%x)\n", flv_codecid >> FLV_AUDIO_CODECID_OFFSET);
//            acodec->codec_tag = flv_codecid >> FLV_AUDIO_CODECID_OFFSET;
    }
}

static int flv_set_video_codec(sh_video_t * sh_video, int flv_codecid) {
//    AVCodecContext *vcodec = vstream->codec;
    switch(flv_codecid) {
        case FLV_CODECID_H263  : 
            sh_video->format = FLV1; 
            break;
        case FLV_CODECID_SCREEN: 
            sh_video->format = CODEC_ID_FLASHSV; 
            break;
        case FLV_CODECID_VP6   : 
            sh_video->format = CODEC_ID_VP6F   ;
        case FLV_CODECID_VP6A  :
            if(flv_codecid == FLV_CODECID_VP6A)
                sh_video->format = CODEC_ID_VP6A;
//            if(vcodec->extradata_size != 1) {
//                vcodec->extradata_size = 1;
//                vcodec->extradata = av_malloc(1);
//            }
//            vcodec->extradata[0] = get_byte(s->pb);
            stream_read_char(demux->stream);
            return 1; // 1 byte body size adjustment for flv_read_packet()
        case FLV_CODECID_H264:
            sh_video->format = AVC1;
            return 3; // not 4, reading packet type will consume one byte
//        default:
//            av_log(s, AV_LOG_INFO, "Unsupported video codec (%x)\n", flv_codecid);
//            vcodec->codec_tag = flv_codecid;
    }

    return 0;
}

static int amf_get_string(stream_t * stream, char *buffer, int buffsize) {
    int length = stream_read_word(stream);
    if(length >= buffsize) {
        stream_skip(stream, length);
        return -1;
    }

    stream_read(stream, buffer, length);

    buffer[length] = '\0';

    return length;
}

double _flv_ldexp(double x, int exp)
{
    double res;
	res = x;
	int i;
   if (exp > 0)
   {
	    for (i=0; i<exp; i++)
	    {
	       res *= 2;
       }
	}
   else if (exp < 0)
   {
       for (i=0; i>exp; i--)
	    {
	       res /= 2;
       }
   }
	return res;
}

double _flv_int2dbl(int64_t v){
    if(v+v > 0xFFEULL<<52)
        return 0.0/0.0;
    return _flv_ldexp(((v&((1LL<<52)-1)) + (1LL<<52)) * (v>>63|1), (v>>52&0x7FF)-1075);
}
//static int amf_parse_object(AVFormatContext *s, AVStream *astream, AVStream *vstream, const char *key, int64_t max_pos, int depth) {
static int amf_parse_object(stream_t * stream, const char *key, int64_t max_pos, int depth) {
//    AVCodecContext *acodec, *vcodec;
//    ByteIOContext *ioc;
    AMFDataType amf_type;
    char str_val[256];
    double num_val;
    sh_audio_t * sh_audio = (sh_audio_t *)demux->sh_audio;
    sh_video_t * sh_video = (sh_video_t *)demux->sh_video;
    flv_priv_t * flv_priv = (flv_priv_t *)demux->priv;

    num_val = 0;
//    ioc = s->pb;

    amf_type = stream_read_char(stream);

    switch(amf_type) {
        case AMF_DATA_TYPE_NUMBER:
			{
				
			uint64_t num_64;
			num_64 = stream_read_qword(stream); 
            num_val = _flv_int2dbl(num_64);
			
        	}
             break;

        case AMF_DATA_TYPE_BOOL:
            num_val = stream_read_char(stream); 
            break;
        case AMF_DATA_TYPE_STRING:
            if(amf_get_string(stream, str_val, sizeof(str_val)) < 0)
                return -1;
            break;
        case AMF_DATA_TYPE_OBJECT: {
            unsigned int keylen;

            while(stream_tell(stream) < max_pos - 2 && (keylen = stream_read_word(stream))) {
                stream_skip(stream, keylen); //skip key string
                if(amf_parse_object(stream, NULL, max_pos, depth + 1) < 0)
                    return -1; //if we couldn't skip, bomb out.
            }
            if(stream_read_char(stream) != AMF_END_OF_OBJECT)
                return -1;
        }
            break;
        case AMF_DATA_TYPE_NULL:
        case AMF_DATA_TYPE_UNDEFINED:
        case AMF_DATA_TYPE_UNSUPPORTED:
            break; //these take up no additional space
        case AMF_DATA_TYPE_MIXEDARRAY:
            stream_skip(stream, 4); //skip 32-bit max array index
            while(stream_tell(stream) < max_pos - 2 && amf_get_string(stream, str_val, sizeof(str_val)) > 0) {
                //this is the only case in which we would want a nested parse to not skip over the object
                if(amf_parse_object(stream, str_val, max_pos, depth + 1) < 0)
                    return -1;
            }
            if(stream_read_char(stream) != AMF_END_OF_OBJECT)
                return -1;
            break;
        case AMF_DATA_TYPE_ARRAY: {
            unsigned int arraylen, i;

            arraylen = stream_read_dword(stream);
            for(i = 0; i < arraylen && stream_tell(stream) < max_pos - 1; i++) {
                if(amf_parse_object(stream, NULL, max_pos, depth + 1) < 0)
                    return -1; //if we couldn't skip, bomb out.
            }
        }
            break;
        case AMF_DATA_TYPE_DATE:
            stream_skip(stream, 8 + 2); //timestamp (double) and UTC offset (int16)
            break;
        default: //unsupported type, we couldn't skip
            return -1;
    }

    if(depth == 1 && key) { //only look for metadata values when we are not nested and key != NULL
//        acodec = astream ? astream->codec : NULL;
//        vcodec = vstream ? vstream->codec : NULL;

        if(amf_type == AMF_DATA_TYPE_BOOL) {
            if(!strcmp(key, "stereo") && sh_audio) sh_audio->channels = num_val > 0 ? 2 : 1;
        } else if(amf_type == AMF_DATA_TYPE_NUMBER) {
            if(!strcmp(key, "duration")) 
            {
               flv_priv->duration = num_val;
			   g_dwTotalSeconds = (DWORD) (flv_priv->duration + 0.5);
            }
            else if(!strcmp(key, "width")  && sh_video && num_val > 0) sh_video->disp_w  = num_val;
            else if(!strcmp(key, "height") && sh_video && num_val > 0) sh_video->disp_h = num_val;
            else if(!strcmp(key, "videodatarate") && sh_video && 0 <= (int)(num_val * 1024.0))
                sh_video->i_bps = num_val * 1024.0;
			else if(!strcmp(key, "framerate") && sh_video && num_val > 0)
			{
			    sh_video->fps = num_val;
				sh_video->frametime =  1.0/sh_video->fps;
			}    
            else if(!strcmp(key, "audiocodecid") && sh_audio && 0 <= (int)num_val)
                flv_set_audio_codec(sh_audio, (int)num_val << FLV_AUDIO_CODECID_OFFSET);
            else if(!strcmp(key, "videocodecid") && sh_video && 0 <= (int)num_val)
                flv_set_video_codec(sh_video, (int)num_val);
            else if(!strcmp(key, "audiosamplesize") && sh_audio && 0 < (int)num_val) {
                sh_audio->samplesize = (num_val==8)? 1:2 ;
                //we may have to rewrite a previously read codecid because FLV only marks PCM endianness.
                if(num_val == 8 && (sh_audio->format == CODEC_ID_PCM_S16BE || sh_audio->format == CODEC_ID_PCM_S16LE))
                    sh_audio->format = CODEC_ID_PCM_S8;
            }
            else if(!strcmp(key, "audiosamplerate") && sh_audio && num_val >= 0) {
                //some tools, like FLVTool2, write consistently approximate metadata sample rates
                if (!sh_audio->samplerate) {
                    switch((int)num_val) {
                        case 44000: sh_audio->samplerate = 44100  ; break;
                        case 22000: sh_audio->samplerate = 22050  ; break;
                        case 11000: sh_audio->samplerate = 11025  ; break;
                        case 5000 : sh_audio->samplerate = 5512   ; break;
                        default   : sh_audio->samplerate = num_val;
                    }
                }
            }
        }
    }

    return 0;
}

static int flv_read_metabody(stream_t * stream, int64_t next_pos) {

    AMFDataType type;
//    AVStream *stream, *astream, *vstream;
//    ByteIOContext *ioc;
    int i, keylen;
    char buffer[11]; //only needs to hold the string "onMetaData". Anything longer is something we don't want.

//    astream = NULL;
//    vstream = NULL;
    keylen = 0;
//    ioc = s->pb;

    //first object needs to be "onMetaData" string
    type = stream_read_char(stream);
    if(type != AMF_DATA_TYPE_STRING || amf_get_string(stream, buffer, sizeof(buffer)) < 0 || strcmp(buffer, "onMetaData"))
        return -1;

    //find the streams now so that amf_parse_object doesn't need to do the lookup every time it is called.
//    for(i = 0; i < s->nb_streams; i++) {
//        stream = s->streams[i];
//        if     (stream->codec->codec_type == CODEC_TYPE_AUDIO) astream = stream;
//        else if(stream->codec->codec_type == CODEC_TYPE_VIDEO) vstream = stream;
//    }

    //parse the second object (we want a mixed array)
    if(amf_parse_object(stream, buffer, next_pos, 0) < 0)
        return -1;

    return 0;
    
}
#if 0
static AVStream *create_stream(AVFormatContext *s, int is_audio){
    AVStream *st = av_new_stream(s, is_audio);
    if (!st)
        return NULL;
    st->codec->codec_type = is_audio ? CODEC_TYPE_AUDIO : CODEC_TYPE_VIDEO;
    av_set_pts_info(st, 32, 1, 1000); /* 32 bit pts in ms */
    return st;
}
#endif
static int flv_read_header(demuxer_t *demuxer)
{
    int offset;
    unsigned char flags;
      demux_stream_t *d_audio = demuxer->audio;
      demux_stream_t *d_video = demuxer->video;
      sh_audio_t *sh_audio = NULL;
      sh_video_t *sh_video = NULL;
      flv_priv_t *priv = (flv_priv_t *) mem_malloc(sizeof(flv_priv_t));

      memset(priv, 0, sizeof(flv_priv_t));
      demux->priv = (void *) priv;

    stream_reset(demuxer->stream);
    stream_skip(demuxer->stream, 4);
    flags = stream_read_char(demuxer->stream);
     /* old flvtool cleared this field */
    /* FIXME: better fix needed */
    if (!flags) {
        flags = FLV_HEADER_FLAG_HASVIDEO | FLV_HEADER_FLAG_HASAUDIO;
//        av_log(s, AV_LOG_WARNING, "Broken FLV file, which says no streams present, this might fail\n");
    }

//    if((flags & (FLV_HEADER_FLAG_HASVIDEO|FLV_HEADER_FLAG_HASAUDIO))
//             != (FLV_HEADER_FLAG_HASVIDEO|FLV_HEADER_FLAG_HASAUDIO))
//        s->ctx_flags |= AVFMTCTX_NOHEADER;

    offset = stream_read_dword(demuxer->stream);
    stream_seek(demuxer->stream, offset);
	demux->movi_start = stream_tell(demuxer->stream);
    if(flags & FLV_HEADER_FLAG_HASVIDEO){
					sh_video = new_sh_video(demuxer, 0);
          ds_fill_buffer(demuxer->video);
          
			    demuxer->video->sh = sh_video;
			    sh_video->ds = demuxer->video;
					demuxer->sh_video = sh_video;
//        if(!create_stream(s, 0))
//            return AVERROR(ENOMEM);
    }
    if(flags & FLV_HEADER_FLAG_HASAUDIO){
					sh_audio = new_sh_audio(demuxer, 0);
			    demuxer->stream_a = (stream_t *)av_stream_create(demuxer->stream);
      		stream_reset(demuxer->stream_a);
          stream_seek(demuxer->stream_a, offset);
			    ds_fill_buffer(demuxer->audio);

			    demuxer->audio->sh = sh_audio;
			    sh_audio->ds = demuxer->audio;
			    demuxer->sh_audio = sh_audio;
//        if(!create_stream(s, 1))
//            return AVERROR(ENOMEM);
    }
	mpDebugPrint("stream_tell(demuxer->stream) %d",stream_tell(demuxer->stream_a));
//    s->start_time = 0;

    return 0;
}

static int flv_get_extradata(stream_t * stream, int size)
{
    flv_priv_t * flv_priv = demux->priv;
    if (flv_priv->extradata)    
      mem_free(flv_priv->extradata);
    flv_priv->extradata = (uint8_t *)mem_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (flv_priv->extradata)
      memset(flv_priv->extradata, 0, size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!flv_priv->extradata)
        return AVERROR(ENOMEM);
    flv_priv->extradata_size = size;
    stream_read(demux->stream, flv_priv->extradata, flv_priv->extradata_size);
    return 0;
}

static int flv_read_packet(demux_stream_t * ds)
{
//    FLVContext *flv = s->priv_data;
    int ret, i, type, size, flags, is_audio;
//    int64_t next, pos;
    int next, pos;
//    int64_t dts, pts = AV_NOPTS_VALUE;
    int dts, pts = AV_NOPTS_VALUE;
    AVStream *st = NULL;
    stream_t * stream;
    sh_audio_t * sh_audio = (sh_audio_t *)demux->sh_audio;
    sh_video_t * sh_video = (sh_video_t *)demux->sh_video;
    flv_priv_t * flv_priv = demux->priv;
 
	if ( DEV_USB_WIFI_DEVICE == demux->stream->fd->Drv->DevID || DEV_CF_ETHERNET_DEVICE == demux->stream->fd->Drv->DevID)
	{		    
		flv_priv->is_streamed = 1;
	}	   

    if (ds == demux->video)
      stream = demux->stream;
		else  // (ds == demux->audio)  
      stream = demux->stream_a;

 for(;;){
		pos = stream_tell(stream);
		stream_skip(stream, 4);     /* size of previous packet */
		type = stream_read_char(stream);
    size = stream_read_int24(stream);
    dts = stream_read_int24(stream);
    dts |= stream_read_char(stream) << 24;
//    av_log(s, AV_LOG_DEBUG, "type:%d, size:%d, dts:%d\n", type, size, dts);

    if (stream_eof(stream))
        return AVERROR_EOF;
    stream_skip(stream, 3); /* stream id, always 0 */
    flags = 0;

    if(size == 0)
        continue;

    next= size + stream_tell(stream);

    if (type == FLV_TAG_TYPE_AUDIO) {
      if(ds == demux->audio)
      {
        is_audio=1;
        flags = stream_read_char(stream);
        size--;
      }
      else
        goto skip;        
    } else if (type == FLV_TAG_TYPE_VIDEO) {
      if (ds == demux->video)
      {
        is_audio=0;
        flags = stream_read_char(stream);
        size--;
        if ((flags & 0xf0) == 0x50) /* video info / command frame */
            goto skip;
      }
      else
        goto skip;
    } else {
        if (type == FLV_TAG_TYPE_META && size > 13+1+4)
            flv_read_metabody(stream, next);
//        else /* skip packet */
//            av_log(s, AV_LOG_ERROR, "skipping flv packet: type %d, size %d, flags %d\n", type, size, flags);
    skip:
        stream_seek(stream, next);
        continue;
    }

    /* skip empty data packets */
    if (!size)
        continue;
    /* now find stream */
//    for(i=0;i<s->nb_streams;i++) {
//        st = s->streams[i];
//        if (st->id == is_audio)
//            break;
//    }
//    if(i == s->nb_streams){
//        av_log(s, AV_LOG_ERROR, "invalid stream\n");
//        st= create_stream(s, is_audio);
//        s->ctx_flags &= ~AVFMTCTX_NOHEADER;
//    }
//    av_log(s, AV_LOG_DEBUG, "%d %X %d \n", is_audio, flags, st->discard);
//    if(  (st->discard >= AVDISCARD_NONKEY && !((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_KEY ||         is_audio))
//       ||(st->discard >= AVDISCARD_BIDIR  &&  ((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_DISP_INTER && !is_audio))
//       || st->discard >= AVDISCARD_ALL
//       ){
//        url_fseek(s->pb, next, SEEK_SET);
//        continue;
//    }
//    if ((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_KEY)
//        av_add_index_entry(st, pos, dts, size, 0, AVINDEX_KEYFRAME);
    break;
 }

    // if not streamed and no duration from metadata then seek to end to find the duration from the timestamps
//    if(!url_is_streamed(s->pb) && s->duration==AV_NOPTS_VALUE){
    if(!flv_priv->is_streamed  && flv_priv->duration == 0)
    {
        DWORD size;
        const int64_t pos = (int64_t)stream_tell(stream);
        const int64_t fsize = (int64_t)stream_get_file_size(stream);
		
        stream_seek(stream, fsize-4);
			
        size = stream_read_dword(stream);
        if((fsize-3-size) >= 0)
        {
          
              stream_seek(stream, fsize-3-size);
          
		  
          if(size == stream_read_int24(stream)+11)
          {
            flv_priv->duration = stream_read_int24(stream) /1000.0f;
			g_dwTotalSeconds = (DWORD) (flv_priv->duration + 0.5);
          }
        }

        stream_seek(stream, pos);
    }

    if(is_audio){
//        if(!st->codec->channels || !st->codec->sample_rate || !st->codec->bits_per_coded_sample) {
        if (!sh_audio->channels || !sh_audio->samplerate || !sh_audio->samplesize)
        {
            sh_audio->channels = (flags & FLV_AUDIO_CHANNEL_MASK) == FLV_STEREO ? 2 : 1;
            sh_audio->samplerate = (44100 << ((flags & FLV_AUDIO_SAMPLERATE_MASK) >> FLV_AUDIO_SAMPLERATE_OFFSET) >> 3);
            sh_audio->samplesize = (flags & FLV_AUDIO_SAMPLESIZE_MASK) ? 2 : 1;   // 16:8

            sh_audio->wf = (WAVEFORMATEX *) mem_malloc(sizeof(WAVEFORMATEX));
            memset(sh_audio->wf, 0, sizeof(WAVEFORMATEX));
            sh_audio->wf->nChannels = sh_audio->channels;
            sh_audio->wf->wBitsPerSample = sh_audio->samplesize<<3;
            sh_audio->wf->nSamplesPerSec =sh_audio->samplerate;
       }
        if(!sh_audio->format){
            flv_set_audio_codec(sh_audio, flags & FLV_AUDIO_CODECID_MASK);
        }
    }else{
        size -= flv_set_video_codec(sh_video, flags & FLV_VIDEO_CODECID_MASK);
    }

    if ( ((sh_audio->format== CODEC_ID_AAC) && (ds==demux->audio)) ||
        ((sh_video->format == CODEC_ID_H264) && (ds==demux->video)) )
    {
        int type = stream_read_char(stream);
        size--;
        if ((sh_video->format == CODEC_ID_H264) && (ds==demux->video)) {
            int32_t cts = (stream_read_int24(stream)+0xff800000)^0xff800000; // sign extension
            pts = dts + cts;
//            if (cts < 0) { // dts are wrong
//                flv->wrong_dts = 1;
//                av_log(s, AV_LOG_WARNING, "negative cts, previous timestamps might be wrong\n");
//            }
//            if (flv->wrong_dts)
//                dts = AV_NOPTS_VALUE;
        }
        if (type == 0) {
            if ((ret = flv_get_extradata(stream, size)) < 0)
                return ret;
            if ((sh_audio->format== CODEC_ID_AAC) && (ds==demux->audio)) {
                MPEG4AudioConfig cfg;
//                ff_mpeg4audio_get_config(&cfg, st->codec->extradata,
//                                         st->codec->extradata_size);
                if (cfg.chan_config > 7)
                    return -1;
//                sh_audio->channels = ff_mpeg4audio_channels[cfg.chan_config];
                sh_audio->samplerate = cfg.sample_rate;
//                dprintf(s, "mp4a config channels %d sample rate %d\n",
//                        st->codec->channels, st->codec->sample_rate);
            }

            return AVERROR(EAGAIN);
        }
    }

    /* skip empty data packets */
    if (!size)
        return AVERROR(EAGAIN);
#if 0
    ret= av_get_packet(s->pb, pkt, size);
    if (ret < 0) {
        return AVERROR(EIO);
    }
    /* note: we need to modify the packet size here to handle the last
       packet */
    pkt->size = ret;
    pkt->dts = dts;
    pkt->pts = pts == AV_NOPTS_VALUE ? dts : pts;
    pkt->stream_index = st->index;

    if (is_audio || ((flags & FLV_VIDEO_FRAMETYPE_MASK) == FLV_FRAME_KEY))
        pkt->flags |= PKT_FLAG_KEY;
#endif
        pts = (pts == AV_NOPTS_VALUE) ? dts : pts;
//        pts = pts/1000;
        pos = stream_tell(stream);

        ds_read_packet2(ds, stream, size, pts, pos, flags);  // ds_read_packet2:temporary solution for passing floating pts parameter error bug
//        ds_read_packet(ds, stream, size, (float)pts, pos, flags);
	      return ds ? 1 : 0;

//    return ret;
}


static int _check_type(stream_t * stream, DEMUX_T demux_type)
{

	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_FLV))
	{
	
		stream_reset(stream);

		stream_seek(stream, stream->start_pos);
		demux->stream = stream;

		if (flv_probe(demux->stream))
		{
			return 1;
		}	
		else
		{
			return (demux_type == DEMUX_UNKNOWN) ? 0 : -1;
		}	
	}

	return 0;
}

static int _open()
{

    demux->seekable = 0;

  if(flv_read_header(demux))  // if flv read header failed, return 0
    return 0;  

  return 1;
}

static int _fill_buffer(demux_stream_t * ds)
{
  int ret;
  
  ret=flv_read_packet(ds);
  if (ret == AVERROR_EOF)
    return 0;                  // EOF
  else
    return ret;
}

static int _seek(float rel_seek_secs, int flags)
{
  
    //if (current_avi_format == _AVI_NI_NI)
    //if (rel_seek_secs != 0)
	//	__asm("break 100");
	
	demux_stream_t *d_audio = demux->audio;
	demux_stream_t *d_video = demux->video;
	demux_stream_t *d_video_3D = demux->video_3D;
	sh_audio_t *sh_audio = d_audio->sh;
	sh_video_t *sh_video = d_video->sh;
	sh_video_t *sh_video_3D = d_video_3D->sh;
	float skip_audio_secs = 0;

	//================= seek in FLV ==========================
    unsigned int newpos_v,newpos_a,total_off=0;
	int rel_seek_frames = rel_seek_secs * sh_video->fps;
	int video_chunk_pos = d_video->pos;
	int i;
	int frame_rate,four_cc;
	int next, pos,size,type,dts,dts_audio;
	unsigned char cmp;
	stream_t *stream,*stream_a;
	stream = demux->stream;	
	stream_a = demux->stream_a;	
    if (flags & 1)
    {
		
		demux->movi_end = stream->file_len;
		demux->filepos = stream_tell(stream);
    }
	else
	{
		demux->filepos = stream_tell(stream);
	}
	
	newpos_v = (flags & 1) ? demux->movi_start : demux->filepos;
	if (!sh_video->i_bps)	
		newpos_v += 2324 * 75 * rel_seek_secs;	// 174.3 kbyte/sec
	else
	{
		newpos_v += (unsigned int ) (rel_seek_secs * (sh_video->i_bps+sh_audio->i_bps))>>3; //using byte rate to caculate correct time. 2010/02/23
	}
	
	if (newpos_v < demux->movi_start)
		newpos_v = demux->movi_start;
	else if (newpos_v > demux->movi_end)	
		newpos_v = demux->movi_end;    
	four_cc = sh_video->format;
// ------------ STEP 1: find nearest video keyframe chunk ------------
	// find nearest video keyframe chunk pos:
	// seek forward/backward					
	stream_seek(stream,newpos_v);
	while(1)
	{
		cmp =stream_read_char(stream);
		if (cmp == 0x00)
		{
			if (stream_read_char(stream)==0x00)
			{	cmp = stream_read_char(stream);
				if (cmp >=0x84 )
				{	
					stream_read_char(stream);
					if (stream_read_char(stream)==0x80)
					{
						stream_read_char(stream);
						//if (stream_read_char(stream)==0xC8)
							if (stream_read_char(stream)==0x00)
							{
								stream_read_char(stream);//every chunk start offset 0x18 2010/02/23
								//newpos_v = stream_tell(stream);
								//stream_seek(stream,newpos_v-0x18);
								break;
								//if (stream_read_char(stream)==0x71)
								//{
									//break;
								//}			
							}
					}
				}
			}
		}
		if (stream_eof(stream))
			break;
	}
	newpos_v = stream_tell(stream);
	stream_seek(stream,newpos_v-16);
	dts = stream_read_int24(stream);
	dts |= stream_read_char(stream) << 24;
	d_video->pts = (float)(dts/1000.0f);
	newpos_a = newpos_v;
    // ------------ STEP 2: seek audio, find the right chunk & pos ------------	
	stream_seek(stream_a,newpos_a);
	while(1)
	{
		cmp =stream_read_char(stream_a);
		if (cmp == 0x08)
		{
			if (stream_read_char(stream_a)== 0x00)
			{
				cmp =stream_read_char(stream_a);	
				cmp =stream_read_char(stream_a);
				
				dts_audio =stream_read_char(stream_a)<<16;	
				dts_audio |=stream_read_char(stream_a)<<8;	
				dts_audio |=stream_read_char(stream_a);	
				dts_audio |=stream_read_char(stream_a)<<24;	
				cmp =stream_read_char(stream_a);	
				cmp =stream_read_char(stream_a);	
				cmp =stream_read_char(stream_a);
				cmp =stream_read_char(stream_a);
				if (0xff == stream_read_char(stream_a))
				{
					if (0xf3 == stream_read_char(stream_a))//every chunk start offset 0x12 2010/02/23
						break;
				}
			}
		}
		if (stream_eof(stream_a))
			break;
	}
	newpos_a = stream_tell(stream_a);
	if (dts_audio - dts > 0x8)
		d_video->pts = (float)(dts_audio/1000.0f);
#if 0	
	mpDebugPrint("newpos_v-0x18 0x%x",newpos_v-0x18);
	mpDebugPrint("newpos_a-18 0x%x",newpos_a-18);
	mpDebugPrint("dts 0x%x",dts);
#endif	
	stream_seek(stream,newpos_v-0x18);
	flv_read_packet(d_video);
	stream_seek(stream_a,newpos_a-18);
	return 1;
}

static int _control(int cmd, void *arg)
{
	switch (cmd)
	{

	case DEMUXER_CTRL_IF_PTS_FROM_STREAM:
		*((int*)arg) = 1;
		return DEMUXER_CTRL_OK;
		
	default:
	return DEMUXER_CTRL_NOTIMPL;
	}
}
static int _close()
{
	flv_priv_t * flv_priv = demux->priv;

	if (flv_priv)
    {
		if (flv_priv->extradata)
			mem_free(flv_priv->extradata);
		mem_free(flv_priv);
    }
  
	return 1;
}

//function GetPTSByRelFrame
//flags=0 seek to begin
//flags=1 seek to end
//flags=2 seek to "+/-" rel_seek_samples
static float _get_pts_by_relframe(int rel_seek_samples, int flags)
{
	float pts;

	switch (flags)
	{
	case 0:
    break;
  case 1:
    pts = 0;
    break;
  case 2:
    break;
  default:
    break;
  }
  
  return pts;
}

//command=0: get total time/file length
//command=1: get current time/file point
//command=2: get percentage
//command=3: get ts information
//return=0:  not fill the buffer
//return=1:  fill the buffer
static int _get_movie_info(BYTE * buffer, int command)
{
	BYTE *string;
	unsigned int  t1, t2;
	int bits;

	switch (command)
	{
	  case 0:
		{

			t1 = t2 = demux->stream->file_len;
			////MP_DEBUG1("_get_movie_info:case0 t=", t, 8);

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
			////MP_DEBUG1("_get_movie_info:case2 t=", t, 8);
			// convert time value to string

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
			flv_priv_t * flv_priv = (flv_priv_t *)demux->priv;
			
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

				//if((pAVIInfo->dwTotalTime = (demux->stream->file_len/(sh_audio->i_bps+sh_video->i_bps) - 3))>=0)  //need fix   
				//  pAVIInfo->dwFlags |=(DWORD)MOVIE_TotalTime_USEFUL;
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

				if ((pInfo->dwTotalTime = (DWORD) flv_priv->duration) >= 0)
					pInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
			}

			return 1;
		}	

    default:
      break;
  }          

	return 0;
}


#if 0
AVInputFormat flv_demuxer = {
    "flv",
    NULL_IF_CONFIG_SMALL("FLV format"),
    sizeof(FLVContext),
    flv_probe,
    flv_read_header,
    flv_read_packet,
    .extensions = "flv",
    .value = CODEC_ID_FLV1,
};
#endif

demuxer_t *new_flv_demux()
{
	demux = new_base_demux();
	// Assign all of the function pointers and type specific data	
	demux->check_type = _check_type;
	demux->open = _open;
	demux->fill_buffer = _fill_buffer;
	demux->seek = NULL;
	demux->control = _control;
	demux->close = _close;
	demux->get_pts_by_relframe = _get_pts_by_relframe;
	demux->get_movie_info = _get_movie_info;
	demux->type = DEMUXER_TYPE_FLV;
	demux->file_format = DEMUXER_TYPE_FLV;
	return demux;
}

#endif

