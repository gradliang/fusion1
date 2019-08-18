/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#define MP_PERFORMANCE_TRACE 0


/*
// Include section 
*/
#include "global612.h"
#include "avtypedef.h"
#include "mptrace.h"

#if (MJPEG_ENABLE && MJPEG_TOGGLE)
#include "mjpeg_define.h"
extern int BT_len;
static DWORD bwordalign;
static BYTE  *BS = NULL;
extern BYTE MJ;
#endif

extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;

extern void * BitStream_Source;
#if ( HAVE_NETSTREAM || NET_UPNP )
extern unsigned char irready;
#endif

//---------------------------------------------------------
int MagicPixel_MP3_getposition_callback(void)
{
#if MP3_SW_AUDIO
	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);
#endif
}

//---------------------------------------------------------

void MagicPixel_MP3_fileseek_callback(int file_ptr)
{
#if( MP3_SW_AUDIO)

	if (AO_AV == AO_TYPE)
		Seek(Media_data.dec_fp, file_ptr);

	Media_data.fending = 0;
#endif
}

//------------------------------------------------------------------------

int MagicPixel_MP3_bitstream_callback(unsigned char *buf, int len,int *fend_flag) 
{
#if (MP3_SW_AUDIO)

	int numByteRead;
#if MP_PERFORMANCE_TRACE
	static unsigned int count=0;
	unsigned int ttime;
#endif

#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	DWORD dwReadSize;
	SWORD ret;

	if(!BT_len)
		BS = (BYTE *)BitStream_Source;
#endif

	int x;
	int bytes;   
	demux_stream_t * ds;
	sh_audio_t  * BT_S;
	BT_S = (sh_audio_t *)BitStream_Source;

	switch (AO_AV)
	{
		case AV_TYPE :
//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
			if(MJ)	//Motion jpeg Audio playing case...
			{
				numByteRead=0;

				if(Media_data.fending == 1)
				{
					memset(buf,0,len);
					*fend_flag = 1;

					return len;
				}
				else
				{
					while(numByteRead < len)
					{
						MP_DEBUG3("numByteRead=%d,len=%d,BT_len=%d",numByteRead,len,BT_len);
						if (BT_len==0)
						{
							ret = MjpegPlayerAudioFillBlock (MJPEG_AVI, &dwReadSize,&bwordalign);

							if(ret != MSG_NO_ERR)
							{
								Media_data.fending = 1;
								*fend_flag = 1;
								memset(buf+numByteRead, 0, len - numByteRead);
								break;
							}

							BS = (BYTE *)(BitStream_Source + bwordalign);
							BT_len = dwReadSize-bwordalign;
						}
						else
							dwReadSize = BT_len; 

						MP_DEBUG1("dwReadSize=%d",dwReadSize);

						if(numByteRead + BT_len > len)
						{
							memcpy(buf+numByteRead,BS,len-numByteRead);
							dwReadSize-=len-numByteRead;
							BT_len-=len-numByteRead;
							BS+=len-numByteRead;
							numByteRead+=len-numByteRead;
						}
						else
						{
							memcpy(buf+numByteRead,BS,BT_len);
							numByteRead+=BT_len;
							BT_len=0;
							BS=BitStream_Source;
						}

						*fend_flag = 0;
					}
				}

				MP_DEBUG1("BT_len=%d",BT_len);
			}
			else
#endif
			{	//General video playing case
				MP_TRACE_LINE();

				if(Media_data.fending == 1)
				{
					MP_TRACE_LINE();
					*fend_flag = 1;
					memset(buf, 0, len);            

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
							#if MP_PERFORMANCE_TRACE
							count++;
							ttime = GetSysTime();
							#endif
							MP_PERFORMANCE_TRC("b:mp3_fill[%d](t:%d)",count,ELAPSE_TIME);
							if (!ds_fill_buffer(ds))
							{
								Media_data.fending = 1;
								*fend_flag=1;
								return bytes;
							}
							MP_PERFORMANCE_TRC("a:mp3_fill[%d](t:%d)\t(tt:%d)",count, ELAPSE_TIME, NowSysTime - ttime);
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
					MP_PERFORMANCE_TRC("mips_mp3 = %d (t:%d)", bytes, ELAPSE_TIME);
					return bytes;           
				}
			}
			break;

		case AO_TYPE :
			if(Media_data.fending == 1)
			{
				memset(buf, 0, len);
				*fend_flag = 1;

				return len;
			}

			else
			{
			
#if ( HAVE_NETSTREAM || NET_UPNP )
				if( irready == 0 )
#endif
				{
					numByteRead = FileRead(Media_data.dec_fp, buf, len);
					if(numByteRead != len)
					{
						memset(buf+numByteRead,0,len-numByteRead);
						Media_data.fending = 1;
						*fend_flag = 1;
					}
					else
						*fend_flag = 0;
				}
#if NET_UPNP
				else if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
				{
					numByteRead = UPNP_Stream_File_Read(buf,len);
					if( numByteRead != len )
					{
						memset(buf+numByteRead,0,len-numByteRead);
						Media_data.fending = 1;
						*fend_flag = 1;
					}
					else
						*fend_flag = 0;
				}
#endif

#if ( HAVE_NETSTREAM || NET_UPNP )
                else
                {
					len = shoutcast_get_data(buf,len);
					*fend_flag = 0;
                }
#endif

			}
	}

	return len;
#else
	return 0;
#endif	
} 

