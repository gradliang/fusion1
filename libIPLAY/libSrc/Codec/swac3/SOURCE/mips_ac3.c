
#define LOCAL_DEBUG_ENABLE 0
#define MP_PERFORMANCE_TRACE 0


#include "global612.h"
#include "mptrace.h"
#include "avtypedef.h"

#if AC3_AUDIO_ENABLE
extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;

#define SD_BUFFER_SIZE 4096
extern int sdbuf_left;
unsigned char *sdbuf_ptr;
extern void * BitStream_Source;
#endif

//---------------------------------------------------------
/*int MagicPixel_ac3_getposition_callback(void)
{
   #if( AC3_AUDIO_ENABLE|| AC3_VIDEO_ENABLE)
           if (AO_AV == AO_TYPE)
                return FFilePosGet(ac3_data.dec_fp);
   #endif
}*/
//---------------------------------------------------------
void MagicPixel_ac3_fileseek_callback(DWORD file_ptr)
{
#if AC3_AUDIO_ENABLE
	if (AO_AV == AO_TYPE)
		Seek(Media_data.dec_fp, file_ptr);
	Media_data.fending = 0;
#endif   
}

void MagicPixel_ac3_resync_post_function()
{
#if AC3_AUDIO_ENABLE
	sdbuf_left = 0;
#endif
}

int MagicPixel_ac3_getposition_callback(void)
{
#if AC3_AUDIO_ENABLE
	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);
#endif
}

int MagicPixel_ac3_bitstream_callback(unsigned char *buf, int len)
{
#if (AC3_AUDIO_ENABLE)
	sh_audio_t *BT_S = NULL;
	int numByteRead;
	int x;
	int bytes;   
	demux_stream_t *ds;
	int total_len = 0;

	BT_S = (sh_audio_t *)BitStream_Source;

	switch (AO_AV)
	{
	case AV_TYPE :
		if(Media_data.fending==1)
		{
			//*fend_flag = 1;
			memset(buf,0,len);            
			return 0;
		}
		else
		{
			bytes = 0;
			ds = BT_S->ds;
			while (len > 0)
			{
				x = ds->buffer_size - ds->buffer_pos;
				if (x == 0)
				{
					MP_PERFORMANCE_TRCN("");
					if (!ds_fill_buffer(ds))
					{
						Media_data.fending=1;
						//	 *fend_flag=1;
						return bytes;
					}
					MP_PERFORMANCE_TRCN("%d ", ELAPSE_TIME);
				}
				else
				{
					if (x > len)
						x = len;
					memcpy(buf + bytes, &ds->buffer[ds->buffer_pos], x);
					bytes += x;
					len -= x;
					ds->buffer_pos += x;
				}
			}

			BT_S->a_in_buffer_len = bytes;
			//*fend_flag = 0;
			return bytes;           
		}
	break;

	case AO_TYPE :
		if(sdbuf_left < len)
		{
			memcpy(buf,sdbuf_ptr,sdbuf_left);
			len -= sdbuf_left;
			buf += sdbuf_left;
			total_len = sdbuf_left;
			sdbuf_left = FileRead(Media_data.dec_fp,(BYTE*)Media_data.sdram_buf,SD_BUFFER_SIZE);
			// 	fread(sd_buffer,sizeof(char),SD_BUFFER_SIZE,wd.dec_fp);
			//don't think the (numByteRead < len && fending==0) case,
			//because max bit-stream syncframe size=3840 and the sd buffer size=4096
			sdbuf_ptr = (unsigned char*)Media_data.sdram_buf;
			len = (sdbuf_left >= len)? len : sdbuf_left;
		}
		memcpy(buf,sdbuf_ptr,len);
		sdbuf_left -= len;
		sdbuf_ptr += len;
		total_len += len;
		return total_len;
	}

	return len;
#else
	return 0;
#endif
} 

