#if 0

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
//#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "avtypedef.h"
//#include "mptrace.h"


#include "dma_cache.h"
extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;

#if 0
extern void * BitStream_Source;
#endif
//---------------------------------------------------------
//---------------------------------------------------------

int MagicPixel_MP3_MAD_getposition_callback(void)
{
mpDebugPrint("mad mp3 call back");

	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);   
}

//---------------------------------------------------------
//---------------------------------------------------------

void MagicPixel_MP3_MAD_fileseek_callback(int file_ptr)
{
mpDebugPrint("mad mp3 call back2");

	dma_invalid_dcache();

	if (AO_AV == AO_TYPE)
		Seek(Media_data.dec_fp, file_ptr);

	Media_data.fending = 0;
}


//------------------------------------------------------------------------


int MagicPixel_MP3_MAD_bitstream_callback(unsigned char *buf, int len,int *fend_flag) 
{
mpDebugPrint("mad mp3 call back3");

	int numByteRead;

#if 0
	sh_audio_t  * BT_S=NULL;
	int x;
	int bytes;   
	demux_stream_t * ds;

	BT_S=(sh_audio_t *)BitStream_Source;
#endif

	switch (AO_AV)
	{
#if 0
     	case AV_TYPE :
			if(hos.fending==1)
			{
				*fend_flag = 1;
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
						if (!ds_fill_buffer(ds))
						{
							hos.fending=1;
							*fend_flag=1;
							return bytes;
						}
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
			*fend_flag = 0;
			return bytes;           
		}
		break;
#endif

        case AO_TYPE :
     		if(Media_data.fending == 1)
     		{
         		 memset(buf, 0, len);
         		*fend_flag = 1;
                		return len;
      		}
			else
			{
				dma_invalid_dcache();
				numByteRead = FileRead(Media_data.dec_fp,buf,len);
				if(numByteRead != len)
				{
					memset(buf+numByteRead,0,len-numByteRead);
					Media_data.fending = 1;
					*fend_flag = 1;
				}
				else
				*fend_flag = 0;
			}
  	}
   		
     return numByteRead;

} 

//------------------------------------------------------------------------
//------------------------------------------------------------------------

int MagicPixel_MP3_MAD_songinfo_callback(void *ptr, int len, int bad_frame)
{
mpDebugPrint("mad mp3 call back4");
}

//------------------------------------------------------------------------

#endif

