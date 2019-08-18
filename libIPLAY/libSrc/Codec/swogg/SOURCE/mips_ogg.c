#include "global612.h"
#include "avtypedef.h"

#if OGG_ENABLE
extern Audio_dec  Media_data;
extern enum AO_AV_TYPE AO_AV;
#endif
//---------------------------------------------------------
//---------------------------------------------------------

int MagicPixel_OGG_filepos_callback(void)
{
   #if OGG_ENABLE
   //return ftell(ogg.dec_fp);
   if (AO_AV == AO_TYPE)
            return FilePosGet(Media_data.dec_fp);
   #endif
   
}
//---------------------------------------------------------
//---------------------------------------------------------

void MagicPixel_OGG_fileseek_callback(int file_ptr)
{
   #if OGG_ENABLE
   Seek(Media_data.dec_fp,file_ptr);
   Media_data.fending = 0;
   #endif
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
int MagicPixel_OGG_bitstream_callback(unsigned char *buf, int len, int *fending)
{

   int numByteRead;
   #if OGG_ENABLE
   if(Media_data.fending==1)
   {
      *fending = 1;   
      memset(buf,0,len);
      return len;
   }
   else
   {
      numByteRead = FileRead(Media_data.dec_fp,buf,len);
      
      if(numByteRead != len)
      {
         *fending = 1;
         Media_data.fending = 1;
      }
      else
         *fending = 0;   
   }
   #endif
   return numByteRead;
} 


//------------------------------------------------------------------------
//------------------------------------------------------------------------

