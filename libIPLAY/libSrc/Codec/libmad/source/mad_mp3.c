
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "avtypedef.h"
#include "mptrace.h"


#include "dma_cache.h"
extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;

#if 1
extern void * BitStream_Source;
#endif
//---------------------------------------------------------
//---------------------------------------------------------
int MagicPixel_MP3_MAD_getposition_callback(void)
{
	MP_DEBUG("mad mp3 call back");

//mpDebugPrint("----- Call back 1 - pos %d -----", FilePosGet(Media_data.dec_fp));

   #if MP3_MAD_AUDIO 

	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);

   #endif
}
//---------------------------------------------------------
//---------------------------------------------------------

void MagicPixel_MP3_MAD_fileseek_callback(int file_ptr)
{
	MP_DEBUG("mad mp3 call back2");
#if MP3_MAD_AUDIO
	dma_invalid_dcache();

	if (AO_AV == AO_TYPE)
		Seek(Media_data.dec_fp, file_ptr);

	Media_data.fending = 0;
#endif
}

//------------------------------------------------------------------------

//In AV case, just call ds_fill_buffer, demuxer will fill audio data into ds->buffer
static inline DWORD AV_FillBuffer(int *fend_flag)
{
	int x;
	demux_stream_t *ds;
	sh_audio_t *BT_S;

	BT_S = (sh_audio_t *)BitStream_Source;
	//mpDebugPrint("	sh_audio address: %x", BT_S);

	if(Media_data.fending==1)
	{
		*fend_flag = 1;
		//memset(buf, 0, len);
		return 0;
	}
	else
	{
		ds = BT_S->ds;

		if (!ds_fill_buffer(ds))
		{
			Media_data.fending = 1;
			*fend_flag = 1;
			return ds->buffer_size;
		}

		*fend_flag = 0;

		return ds->buffer_size;
	}

}

static inline DWORD AO_FillBuffer(unsigned char *buf, int len,int *fend_flag)
{
	int numByteRead;

	if(Media_data.fending==1)
	{
		MP_TRACE_LINE();
		memset(buf,0, len);
		*fend_flag = 1;

		return len;
	}
	else
	{
		dma_invalid_dcache();
		numByteRead = FileRead(Media_data.dec_fp, buf, len);
		MP_DEBUG2("Media_data.dec_fp=0x%x len=%d", Media_data.dec_fp, len);

		if(numByteRead != len)
		{
			MP_TRACE_LINE();
			memset(buf + numByteRead, 0, len - numByteRead);
			Media_data.fending = 1;
			*fend_flag = 1;
		}
		else
			*fend_flag = 0;

		return numByteRead;
	}
}

//In AV case, parameters of "buf" and "len" are useless, please reference AV_FillBuffer()
int MagicPixel_MP3_MAD_bitstream_callback(unsigned char *buf, int len,int *fend_flag) 
{
	MP_DEBUG("mad mp3 call back3");

	int numByteRead;

	if(AO_AV == AV_TYPE){
		numByteRead = AV_FillBuffer(fend_flag);
	}
	else if(AO_AV == AO_TYPE){
		numByteRead = AO_FillBuffer(buf, len, fend_flag);
	}
	else{
		numByteRead = -1;
	}

	return numByteRead;
} 

int MagicPixel_MP3_MAD_songinfo_callback(void *ptr, int len, int bad_frame)
{
	mpDebugPrint("MagicPixel_MP3_MAD_songinfo_callback NULL");
}

