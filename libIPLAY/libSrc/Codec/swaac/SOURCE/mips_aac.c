#include "global612.h"
#include "avtypedef.h"


#if AAC_SW_AUDIO
extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;
extern void * BitStream_Source;
extern int BS_len;
#endif

//---------------------------------------------------------

int MagicPixel_MP4_getposition_callback(void)
{
#if( AAC_SW_AUDIO )
	// return ftell(hos.dec_fp);
	if (AO_AV == AO_TYPE)
		return FilePosGet(Media_data.dec_fp);

#endif
   
}

//---------------------------------------------------------

void MagicPixel_MP4_fileseek_callback(int file_ptr)
{
#if( AAC_SW_AUDIO )
	//   fseek(wd.dec_fp,file_ptr,SEEK_SET);
	//  SeekSet(hos.dec_fp);
	if (AO_AV == AO_TYPE)
		Seek(Media_data.dec_fp, file_ptr);

	Media_data.fending = 0;
#endif
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------

//AO use
int MagicPixel_MP4_bitstream_callback(unsigned char *buf, int len,int *fend_flag) 
{
   int numByteRead;
      
#if (AAC_SW_AUDIO )
	if (AO_AV!=AO_TYPE) 
		UartOutText("AV but called MP4 bitstream call back\r\n");

	if(Media_data.fending==1)
	{
		memset(buf,0,len);
		*fend_flag = 1;
		return len;
	}
	else
	{
		numByteRead = FileRead(Media_data.dec_fp,buf,len);

		if(numByteRead != len)
		{
			memset(buf+numByteRead,0,len-numByteRead);
			Media_data.fending=1;
			*fend_flag=1;
		}
		else
			*fend_flag = 0;
	}
   		
#endif

	return numByteRead ;
}

//------------------------------------------------------------------------

int MagicPixel_avAAC_bitstream_callback(unsigned char *buf, int len)
{
	int bytes=0;
#if( AAC_SW_AUDIO )

	if (AO_AV != AV_TYPE)
		UartOutText("AO but called avAAC bitstream callback\r\n");

	if(Media_data.fending == 1)
	{
		memset(buf, 0, len);
		return 0;
	}
	else
	{
		bytes = 0;

		if(BS_len >= len)
		{
			memcpy(buf + bytes, ((unsigned char *)BitStream_Source), len);
			BS_len -= len;
			bytes = len;
		}
		else
		{
			memcpy(buf + bytes, ((unsigned char *)BitStream_Source), BS_len);
			len -= BS_len;
			bytes = BS_len;
			BS_len = 0;
			memset(buf + bytes, 0, len);	//Fill the 0 data which length are "Len - BS_LEN", because there is no enough data in BitStream_Source
		}

		BitStream_Source += len;
	}
#endif

	return bytes;
} 

unsigned char *MagicPixel_MP4_malloc(unsigned int size)
{
   unsigned char *ptr;

   ptr = (unsigned char *)mem_malloc(size);
   
   return ptr;
}

void MagicPixel_MP4_free(unsigned char *buf)
{
   mem_free(buf);
}
