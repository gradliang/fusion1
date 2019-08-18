#include "global612.h"
#include "avtypedef.h"


#if RAM_AUDIO_ENABLE
extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;
#endif
//---------------------------------------------------------
//---------------------------------------------------------
int MagicPixel_ram_getposition_callback()
{
   #if RAM_AUDIO_ENABLE
   //return ftell(ogg.dec_fp);
   if (AO_AV == AO_TYPE)
            return FilePosGet(Media_data.dec_fp);
   #endif
   
}

void MagicPixel_ram_fileseek_callback(int file_ptr)
{
     #if RAM_AUDIO_ENABLE
      // fseek(wd.dec_fp,file_ptr,SEEK_SET);
      if (AO_AV == AO_TYPE)
            Seek(Media_data.dec_fp,file_ptr);
           
      Media_data.fending = 0;
     #endif
  }

//------------------------------------------------------------------------
//------------------------------------------------------------------------
int MagicPixel_ram_bitstream_callback(unsigned char *buf, int len)
{
  #if RAM_AUDIO_ENABLE
   int numByteRead;
  
   if(Media_data.fending==1)
   {
      memset(buf,0,len);
      return 0;
   }
   else
   {
      numByteRead = FileRead(Media_data.dec_fp,buf,len);
      if(numByteRead != len)
      {
         memset(buf+numByteRead,0,len-numByteRead);
         Media_data.fending = 1;
      }
   }
    
  return numByteRead;
  #endif
  
  return len;

} 

   
//------------------------------------------------------------------------
//------------------------------------------------------------------------

