#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "avtypedef.h"
#include "mpTrace.h"

#if WMA_ENABLE
#if ( HAVE_NETSTREAM || NET_UPNP )
#include "..\..\..\..\libIPLAY\libSrc\netstream\INCLUDE\netstream.h"
#endif

extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;
extern void * BitStream_Source;
#if ( HAVE_NETSTREAM || NET_UPNP )
#if (CHIP_VER_MSB == CHIP_VER_650)
extern unsigned char girbuf[];
#else
extern unsigned char *girbuf;
#endif
extern unsigned char irready;
extern int asf_curindx;
extern int kcurindx;
extern int bufindex_end;
#endif //( HAVE_NETSTREAM || NET_UPNP )

#endif
//---------------------------------------------------------
int MagicPixel_ASF_getposition_callback(void)
{
#if WMA_ENABLE
#if NET_UPNP
	if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
	{
		return kcurindx;
	}
	else
#endif		
	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);
#endif
}

int MagicPixel_ASF_bitstream_callback(unsigned char *buf, int len)
{
	int numByteRead;
	int cpylen=0;
#if WMA_ENABLE
#if NET_UPNP
		if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
		{
			numByteRead = UPNP_Stream_File_Read(buf,len);
			if( numByteRead != len )
			{
				memset(buf+numByteRead,0,len-numByteRead);
				Media_data.fending = 1;
			}
			else
				Media_data.fending = 0;

		}
		else
#endif
#if ( HAVE_NETSTREAM || NET_UPNP )
		if( irready == 1 )
		{
			if (AO_AV == AO_TYPE)
		  {
			 //numByteRead need to 4096;
			 numByteRead = len;
			 mpDebugPrint("asf_curindx %d",asf_curindx);
			 if(bufindex_end)
			 {
			    if((asf_curindx+len)>bufindex_end)
			    {
			        cpylen = bufindex_end-asf_curindx;
					mpDebugPrint("cpylen %d",cpylen);
					if(cpylen)
						mmcp_memcpy(buf,girbuf+asf_curindx,cpylen);
					asf_curindx = 0;
					mmcp_memcpy(buf+cpylen,girbuf+asf_curindx,(len-cpylen));
					asf_curindx+=(len-cpylen);
					bufindex_end = 0;
					TaskYield();

			    }
				else
				{
				 	mmcp_memcpy(buf,girbuf+asf_curindx,len);
					asf_curindx+=len;
					TaskYield();

				}
			   	mpDebugPrint("bufindex_end %d",bufindex_end);
			 }
			 else
			 {
			 mmcp_memcpy(buf,girbuf+asf_curindx,len);
			 //memset(girbuf+asf_curindx,0x00,len);
				//len = 4096;ASF_HEADER_SIZE=len*2
			 if((asf_curindx+len) >= NETSTREAM_MAX_BUFSIZE)
			 {
				asf_curindx = 0;
			 }
			 else
				asf_curindx+=len;
	 		  TaskYield();
			 
		  }
			 
		  }
		  else
			  mpDebugPrint("AV but call AO bitstream\r\n");
		  
		  TaskYield();
		}
		else				
#endif

	if (AO_AV == AO_TYPE){
		//mpDebugPrint("pts : %d, len", FilePosGet(Media_data.dec_fp), len);
		numByteRead = FileRead(Media_data.dec_fp, buf, len);
	}	
	else
		UartOutText("AV but call AO bitstream\r\n");
#endif

	return numByteRead;
} 

void MagicPixel_ASF_fileseek_callback(int file_ptr)
{
#if WMA_ENABLE
#if NET_UPNP
	if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
	{
		kcurindx = file_ptr;
		asf_curindx = file_ptr;
		mpDebugPrint("MagicPixel_MP3_fileseek_callback %x %x",kcurindx,file_ptr);
	}
	else
#endif		
#if HAVE_NETSTREAM
	if( irready )
	{
		asf_curindx = file_ptr;
		mpDebugPrint("MagicPixel_ASF_fileseek_callback %x", file_ptr);
	}
	else
#endif //HAVE_NETSTREAM
		if (AO_AV == AO_TYPE){
			Seek(Media_data.dec_fp, file_ptr);
			//mpDebugPrint("Seek position: %d", file_ptr);
		}
	Media_data.fending = 0;
#endif
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

int AV_WMA_bitstream_callback(unsigned char *buf, int len)
{
	sh_audio_t  * BT_S = NULL;
	int numByteRead;
	int x;
	int bytes;   
	demux_stream_t * ds;
	int total_len = 0;
#if WMA_ENABLE
	BT_S=(sh_audio_t *)BitStream_Source;

	if (AO_AV == AV_TYPE)
	{
		if(Media_data.fending == 1)
		{
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
						Media_data.fending = 1;
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
			return bytes;
		}
	}
	else
		UartOutText("AO but call AV bitstream function\r\n");
#endif
	return len;
} 

void AV_WMA_newblock_callback()
{
#if WMA_ENABLE
	//MagicPixel_WMA_CrossPacket_PostProcess(0,0,((WAVEFORMATEX*)sh->wf)->nBlockAlign);
	MagicPixel_WMA_Get_NewBlock();   
#endif
}

