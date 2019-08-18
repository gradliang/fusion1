
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
unsigned int MagicPixel_AAC_FAAD_getposition_callback(void)
{
	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);

	return NULL;
}

//---------------------------------------------------------
//---------------------------------------------------------
void MagicPixel_AAC_FAAD_fileseek_callback(int file_ptr)
{
	dma_invalid_dcache();

	if (AO_AV == AO_TYPE)
		Seek(Media_data.dec_fp, file_ptr);

	Media_data.fending = 0;
}

//------------------------------------------------------------------------

//In AV case, just call ds_fill_buffer, demuxer will fill audio data into ds->buffer
static inline DWORD AV_FillBuffer(int *fend_flag)
{
	int x;
	demux_stream_t *ds;
	sh_audio_t *BT_S;

	BT_S = (sh_audio_t *)BitStream_Source;

	//mpDebugPrint("AAC AV fill buffer -	sh_audio address: %x", BT_S);

	if(Media_data.fending == 1)
	{
		*fend_flag = 1;
		//memset(buf, 0, len);
		return 0;
	}
	else
	{
		ds = BT_S->ds;

		//mpDebugPrint("------ AAC AV fill buffer 2 ------");

		if (!ds_fill_buffer(ds))
		{
			mpDebugPrint("Request AAC audio buffer fail...");
			Media_data.fending = 1;
			*fend_flag = 1;
			return ds->buffer_size;
		}

		*fend_flag = 0;

		//mpDebugPrint("Aac AV fill buffer End, ds addr: %x,
		//				buffer Addr: %x, Size: %d", ds, ds->buffer, ds->buffer_size);
		return ds->buffer_size;
	}

}

static inline DWORD AO_FillBuffer(unsigned char *buf, int len,int *fend_flag)
{
	int numByteRead;

	MP_TRACE_LINE();

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

int MagicPixel_AAC_FAAD_bitstream_callback(unsigned char *buf, int len,int *fend_flag) 
{
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

int MagicPixel_AAC_FAAD_songinfo_callback(void *ptr, int len, int bad_frame)
{
	return NULL;
}

