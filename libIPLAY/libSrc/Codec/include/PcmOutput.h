/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      ad_internal.h
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 07/11/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     07/11/2005    Brenda Li   first file
****************************************************************
*/
#if 0
static inline void* PcmOutput(int n_channels,int n_samples,int *ch0,int *ch1,void *buf)
{
#if AUDIO_ENDIAN//Big Endian Audio Output
	int *left_ch,*right_ch;
	int x;
     	left_ch = ch0;
     	right_ch = ch1;
	unsigned short *pPlayBuffer;
	pPlayBuffer = (unsigned short*)buf;
      	if (n_channels == 2) 
	     	for(x = 0; x < n_samples/2; x++) 
	     	{
	   		*pPlayBuffer++ = left_ch[x] & 0xffff;
	     		*pPlayBuffer++ = right_ch[x] & 0xffff;
	   		*pPlayBuffer++ = (left_ch[x]>>16) & 0xffff;
	     		*pPlayBuffer++ = (right_ch[x]>>16) & 0xffff;
	      	}
	else	
     		for(x = 0; x < n_samples/2; x++) 
     		{
   		*pPlayBuffer++ = left_ch[x] & 0xffff;
         	*pPlayBuffer++ = (left_ch[x]>>16) & 0xffff;
       	}
	return buf;
#else //little endian audio output
	int *left_ch,*right_ch;
	int x;
     	left_ch = ch0;
     	right_ch = ch1;
	unsigned char *pPlayBuffer;
	pPlayBuffer = (unsigned char*)buf;
	
     	for(x = 0; x < n_samples/2; x++) 
     	{
    		*pPlayBuffer++ = left_ch[x] & 0xff;
   		*pPlayBuffer++ = (left_ch[x]>>8)&0xff;
       	if (n_channels == 2) 
         	{
	     		*pPlayBuffer++ = right_ch[x] & 0xff;
			*pPlayBuffer++ = (right_ch[x]>>8)&0xff;
         	}
    		*pPlayBuffer++ = (left_ch[x]>>16) & 0xff;
 		*pPlayBuffer++ = (left_ch[x]>>24)&0xff;
         	if (n_channels == 2) 
         	{
	     		*pPlayBuffer++ = (right_ch[x]>>16) & 0xff;
			*pPlayBuffer++ = (right_ch[x]>>24) & 0xff;
         	}
   	}
	return buf;
#endif
}
#else
static inline void* PcmOutput(int n_channels,int n_samples,int *ch0,int *ch1,void *buf)
{
//#if AUDIO_ENDIAN//Big Endian Audio Output
#if 0
	int *left_ch,*right_ch;
	int x;
     	left_ch = ch0;
     	right_ch = ch1;
	unsigned short *pPlayBuffer;
	pPlayBuffer = (unsigned short*)buf;
      	if (n_channels == 2) 
	     	for(x = 0; x < n_samples/2; x++) 
	     	{
	   		*pPlayBuffer++ = left_ch[x] & 0xffff;
	     		*pPlayBuffer++ = right_ch[x] & 0xffff;
	   		*pPlayBuffer++ = (left_ch[x]>>16) & 0xffff;
	     		*pPlayBuffer++ = (right_ch[x]>>16) & 0xffff;
	      	}
	else	
     		for(x = 0; x < n_samples/2; x++) 
     		{
   		*pPlayBuffer++ = left_ch[x] & 0xffff;
         	*pPlayBuffer++ = (left_ch[x]>>16) & 0xffff;
       	}
	return buf;
#else
	unsigned short *left_ch,*right_ch;
	int x;
     	left_ch = (unsigned short *)ch0;
     	right_ch = (unsigned short *)ch1;
	unsigned int *pPlayBuffer;
	pPlayBuffer = (unsigned int*)buf;
      	if (n_channels == 2) 
	     	for(x = 0; x < n_samples; x++) 
	     	{
	     		*pPlayBuffer++ = (right_ch[x]<<16) + left_ch[x];
	     		
	   		//*pPlayBuffer++ = left_ch[x] & 0xffff;
	     		//*pPlayBuffer++ = right_ch[x] & 0xffff;
	   		//*pPlayBuffer++ = (left_ch[x]>>16) & 0xffff;
	     		//*pPlayBuffer++ = (right_ch[x]>>16) & 0xffff;
	      	}
	else	
		memcpy(pPlayBuffer, left_ch, 2*n_samples);
	/*
     		for(x = 0; x < n_samples/2; x++) 
     		{
			*pPlayBuffer++ = ((unsigned int *)left_ch)[x];//(left_ch[x+1]<<16) + left_ch[x];

      		}
      	*/	
	return buf;
#endif
}

#endif

