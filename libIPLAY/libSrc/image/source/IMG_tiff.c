/*
=======================TIFF CODEC =============================

	This TIFF CODEC refers to TIFF 6.0 SPEC--Baseline TIFF;
	It can support following Format:
		1. byte order
			※II§ (4949.H)			little-endian		L
			※MM§ (4D4D.H)		big-endian		X
		2.  PlanarConfiguration
			Chunky format.			RGBRGB			L
			Planar format.				RRGGBB			X
		3. Compression
			No compression							L
			compresssion								X
		4. MultiStrip									L
		5. Supporting Color Format
			Bilevel Images
			Grayscale Images
			Palette-color Images
				the Color Map is configed as 
					RR *256		when RR is not same, the Later is the Standard.
					GG * 256
					BB *256
					
			RGB Full Color Images
//------------------------------------------------------
	Programer:	Neil
	Date:	20071216
	History:	
			1. 20071216	Neil creat this module;
			2.20071219	Neil Modify 8-bit grayscale & Palette Image for FullScreen
			3.20080317  	Neil add function:TIFF_Readword() and TIFF_ReadDWord() for support BIG endian TIFF file
			4.200912  TYChen Fix BUG for bilevel image
			5.201001 TYChen add Packbit compression decoder(only gray image)
//------------------------------------------------------
=======================TIFF CODEC =============================	
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "jpeg.h"
#include "mpapi.h"
//#include "image.h"
#include "TIFF.h"
#include "taskid.h"


#if TIFF

#pragma alignvar(4)
ST_TIFF_INFO g_stTiffTag;
ST_TIFF_INFO *g_pstTiffTag;

WORD TIFF_Readword(ST_MPX_STREAM *s) 
{
	WORD ii;
	if (g_pstTiffTag->dwByteOrder==IMAGE_TAG_TIFF_BIG)
		ii=mpxStreamReadWord(s);
	else
		ii=mpxStreamReadWord_le(s);
	return ii;
		
}
DWORD TIFF_ReadDWord(ST_MPX_STREAM *s)
{
	DWORD ii;
	if (g_pstTiffTag->dwByteOrder==IMAGE_TAG_TIFF_BIG)
		ii=mpxStreamReadDWord(s);
	else
		ii=mpxStreamReadDWord_le(s);
	return ii;
	
}

//-------------------------------------------------------------------------------

//*********Packbits Compression data
#define BufferSize 1024//256//128//1024
typedef struct{
	unsigned char *Buf;
        int index;//buffer?index,?Buf[index]?Buf[DataLength-1]????
        int AllSize;//buffer??
        int DataLength;//????
}DecoderBuffer;

typedef struct{
    DWORD ImagefFileRemainderByte;
    long ImagePtrRemainderByte;
    int DecoderState;
    DWORD *Strip_Remainder_ByteCounts;
    int StripIndex;
		DWORD ImageCount;//集滿6個就寫進去
		WORD Curr_X_Width;//記錄目前要寫入的x,y
		WORD Curr_Y_Height;
		WORD Scale;
		//DWORD PtrNextLine;
		BYTE TaskYieldInfo;
		BYTE WriteToCbCrBuf[6];
}DecoderInfo;

DecoderBuffer Buf1;
DecoderBuffer Buf2;
DecoderInfo DecoderInfo_temp;
DWORD *ImageYYCbCrPtr;

int ReadImageFileToBuffer(ST_MPX_STREAM *psStream,ST_TIFF_INFO *temp_TIFF_INFO,DWORD *temp_StripOffset,DWORD *temp_StripByteCounts)
{
    int Original_StripIndex=DecoderInfo_temp.StripIndex;   
		int iReadSize;

    if(temp_TIFF_INFO->dwStripCount==1)  //只有一個strip
    {
            if(DecoderInfo_temp.ImagefFileRemainderByte>=BufferSize) //資料夠就讀1k，不夠就全部讀進來
            {
                //fread(Buf1.Buf,1,BufferSize,input);
								iReadSize = mpxStreamRead(Buf1.Buf, 1, BufferSize , psStream);		
								if (iReadSize == 0)
								{
										mpDebugPrint("-------------mpxStreamRead error----------------------");											
										return 1;
								}											
                Buf1.DataLength=BufferSize;
                DecoderInfo_temp.ImagefFileRemainderByte-=BufferSize;
            }else{
                //fread(Buf1.Buf,1,DecoderInfo_temp.ImagefFileRemainderByte,input);
								iReadSize = mpxStreamRead(Buf1.Buf, 1, DecoderInfo_temp.ImagefFileRemainderByte , psStream);		
								if (iReadSize == 0)
								{
										mpDebugPrint("-------------mpxStreamRead error----------------------");											
										return 1;
								}													
                Buf1.DataLength=DecoderInfo_temp.ImagefFileRemainderByte;
                DecoderInfo_temp.ImagefFileRemainderByte=0;
            }
    }else if(temp_TIFF_INFO->dwStripCount>1)    //超過一個strip
    {

            //判斷目前的strip夠不夠
            if(DecoderInfo_temp.Strip_Remainder_ByteCounts[DecoderInfo_temp.StripIndex]>=BufferSize)//夠
            {
                //fread(Buf1.Buf,1,BufferSize,input);
								iReadSize = mpxStreamRead(Buf1.Buf, 1, BufferSize , psStream);		
								if (iReadSize == 0)
								{//__asm("break 100");
										mpDebugPrint("-------------mpxStreamRead error----------------------");											
										return 1;
								}	                
                Buf1.DataLength=BufferSize;
                DecoderInfo_temp.Strip_Remainder_ByteCounts[DecoderInfo_temp.StripIndex]-=BufferSize;

            }else{
                //看目前strip剩多少就讀多少
                //fread(Buf1.Buf,1,DecoderInfo_temp.Strip_Remainder_ByteCounts[DecoderInfo_temp.StripIndex],input);
								iReadSize = mpxStreamRead(Buf1.Buf, 1, DecoderInfo_temp.Strip_Remainder_ByteCounts[DecoderInfo_temp.StripIndex] , psStream);		
								if (iReadSize == 0)
								{//__asm("break 100");
										mpDebugPrint("-------------mpxStreamRead error----------------------");											
										return 1;
								}	                
                Buf1.DataLength=DecoderInfo_temp.Strip_Remainder_ByteCounts[DecoderInfo_temp.StripIndex];
                DecoderInfo_temp.Strip_Remainder_ByteCounts[DecoderInfo_temp.StripIndex]=0;

                //if( DecoderInfo_temp.StripIndex<(int)(temp_TIFF_INFO->dwStripOffsets_Count-1) )//如果是最後一個strip，就不做
                if( DecoderInfo_temp.StripIndex<(temp_TIFF_INFO->dwStripCount-1) )//如果是最後一個strip，就不做
                {
                    DecoderInfo_temp.StripIndex++;
                    //fseek(input, (int)temp_StripOffset[DecoderInfo_temp.StripIndex],SEEK_SET);//移到下一個strip
                    mpxStreamSeek(psStream, temp_StripOffset[DecoderInfo_temp.StripIndex], SEEK_SET);
                }
            }

    }

    Buf1.index=0;//代表buf1開始有資料

    if(Buf2.index<0) //判斷buf2是否有剩下沒用完的資料
      DecoderInfo_temp.DecoderState=1;//轉換到next state
    else
      DecoderInfo_temp.DecoderState=2;//轉換到next state

		return 0;
}
void WriteToYYCbCrBuffer(BYTE ValueTemp,IMAGEFILE *psImage)
{
		DecoderInfo_temp.WriteToCbCrBuf[DecoderInfo_temp.ImageCount++]=ValueTemp;
		ST_TIFF_INFO  *pstTiffTag;
		pstTiffTag = g_pstTiffTag;	
		
		if(pstTiffTag->wBitsPerSample==8)
		{
		
				if(DecoderInfo_temp.ImageCount>=2)
				{
						DecoderInfo_temp.ImageCount=0;

						if(pstTiffTag->PhotoMetric==0)
						{
								DecoderInfo_temp.WriteToCbCrBuf[0]=255-DecoderInfo_temp.WriteToCbCrBuf[0];
								DecoderInfo_temp.WriteToCbCrBuf[1]=255-DecoderInfo_temp.WriteToCbCrBuf[1];
						}
						
						if(DecoderInfo_temp.Scale<=1)//沒有縮小的圖
						{
												*ImageYYCbCrPtr++=RGBRGB_2_YUV(	DecoderInfo_temp.WriteToCbCrBuf[0],
																												DecoderInfo_temp.WriteToCbCrBuf[0],
																												DecoderInfo_temp.WriteToCbCrBuf[0],
																												DecoderInfo_temp.WriteToCbCrBuf[1],
																												DecoderInfo_temp.WriteToCbCrBuf[1],
																												DecoderInfo_temp.WriteToCbCrBuf[1]	);
						}else
						{//有縮小的圖

								if(
									  ((DecoderInfo_temp.Curr_X_Width%(DecoderInfo_temp.Scale<<1))==0)&&
									  ((DecoderInfo_temp.Curr_Y_Height%DecoderInfo_temp.Scale)==0)
										)
								{
											//只有在該範圍才做寫入的動作

														*ImageYYCbCrPtr++=RGBRGB_2_YUV(	DecoderInfo_temp.WriteToCbCrBuf[0],
																														DecoderInfo_temp.WriteToCbCrBuf[0],
																														DecoderInfo_temp.WriteToCbCrBuf[0],
																														DecoderInfo_temp.WriteToCbCrBuf[1],
																														DecoderInfo_temp.WriteToCbCrBuf[1],
																														DecoderInfo_temp.WriteToCbCrBuf[1]	);

								}



						}


						DecoderInfo_temp.Curr_X_Width+=2;

						
						if(DecoderInfo_temp.Curr_X_Width>=psImage->wImageWidth) 
						{
							DecoderInfo_temp.Curr_X_Width=0;
							DecoderInfo_temp.Curr_Y_Height++;//必須要不受影像大小限制，height只能一次移動一行
							ImageYYCbCrPtr = (DWORD *)psImage->pbTarget + (psImage->wTargetWidth >> 1)
																*(DecoderInfo_temp.Curr_Y_Height >> psImage->bScaleDown);//這裡移動到下個line，要考慮縮小之後line會變得比較少
							if (DecoderInfo_temp.TaskYieldInfo) TaskYield();
						}

						

				}
				
		}else if(pstTiffTag->wSampleCount==3){
		
				if(DecoderInfo_temp.ImageCount>=6)
				{
						DecoderInfo_temp.ImageCount=0;



						
						*ImageYYCbCrPtr++=RGBRGB_2_YUV(	DecoderInfo_temp.WriteToCbCrBuf[0],
																						DecoderInfo_temp.WriteToCbCrBuf[1],
																						DecoderInfo_temp.WriteToCbCrBuf[2],
																						DecoderInfo_temp.WriteToCbCrBuf[3],
																						DecoderInfo_temp.WriteToCbCrBuf[4],
																						DecoderInfo_temp.WriteToCbCrBuf[5]	);



						DecoderInfo_temp.Curr_X_Width+=2;
						if(DecoderInfo_temp.Curr_X_Width>=psImage->wImageWidth)
						{
							DecoderInfo_temp.Curr_X_Width=0;
							DecoderInfo_temp.Curr_Y_Height++;
						}						
				}
				
		}





}
/*
去buf1讀資料
*/
void PackbitsDecoderKernel(IMAGEFILE *psImage)
{


    long count,original_count;//代表該數重複幾次
    unsigned char ValueTemp;
		int i;
    //while((n>0)&&(DecoderInfo_temp.ImagePtrRemainderByte>0))
    while( (Buf1.index<=(Buf1.DataLength-1))&&
           (DecoderInfo_temp.ImagePtrRemainderByte>0)
         )
    {
          //if(Buf1.index>189)
         // int aa=0;

          count=(long)Buf1.Buf[Buf1.index++];
          original_count=count;



          if (count >= 128)
              count -= 256;
          if (count < 0) //有壓縮
          {
               //要預防只剩一筆資料
               if(Buf1.index>=Buf1.DataLength)//假如解一半資料空了，就必須再去讀file，並且把剩下的搬去buf2
               {


                   Buf2.Buf[0]=original_count;//Buf1.Buf[Buf1.DataLength-1];//bufi搬到buf2  只有一筆資料  把最後一筆搬過去

                   Buf1.index=-1;//bufi搬到buf2
                   Buf1.DataLength=0;
                   Buf2.index=0;
                   Buf2.DataLength=1;
                   DecoderInfo_temp.DecoderState=0;//轉換state，要去file讀資料
                   return;
               }


           // replicate next byte -n+1 times
               if (count == -128)    // nop
                   continue;
               count = -count + 1;
               //temp[1]=*pCompressionData++; n--;
               ValueTemp=Buf1.Buf[Buf1.index++];


               for(i=0;i<count;i++)
               {
                   //*(ImagePtr++)=ValueTemp;
                   //debug(ValueTemp);
                   WriteToYYCbCrBuffer(ValueTemp,psImage);
               }
               DecoderInfo_temp.ImagePtrRemainderByte-=count; //這個值要重複count次

          }else{       //沒壓縮

               //要預防剩下的資料不夠解
               if( (Buf1.DataLength-Buf1.index) < (count+1) )
               {
                   //判斷是否再read一次file就確保可以把這段解完，否則就看要怎麼改
                   if((Buf1.DataLength-Buf1.index+BufferSize)<(count+1) )
                   {
                       //ShowMessage("error error error");
                       mpDebugPrint("-------------error error error----------------------");
                       return;
                   }


                   //剛剛已經讀到的也要放回去，給下次使用
                   Buf2.Buf[0]=original_count;//Buf1.Buf[Buf1.index-1];
                   for(i=0;i<(Buf1.DataLength-Buf1.index);i++)
                   {
                      Buf2.Buf[i+1]=Buf1.Buf[i+Buf1.index];
                   }


                   Buf2.index=0;
                   Buf2.DataLength=Buf1.DataLength-Buf1.index+1;//加一個是一開始讀的那個要加進去

                   Buf1.index=-1;//bufi搬到buf2
                   Buf1.DataLength=0;

                   DecoderInfo_temp.DecoderState=0;//轉換state，要去file讀資料
                   return;
               }


               for(i=0;i<(count+1);i++)
               {
                  //*(ImagePtr++)=Buf1.Buf[Buf1.index++];
                  ValueTemp=Buf1.Buf[Buf1.index++];
                  //*(ImagePtr++)=ValueTemp;
                   //debug(ValueTemp);
                   WriteToYYCbCrBuffer(ValueTemp,psImage);
               }
               DecoderInfo_temp.ImagePtrRemainderByte-=(count+1); //這個值要重複count次
          }
    }
    if(DecoderInfo_temp.ImagePtrRemainderByte<=0)
      DecoderInfo_temp.DecoderState=10;  //decoder end
    else if(Buf1.index>(Buf1.DataLength-1))
      DecoderInfo_temp.DecoderState=0;//buf1 empty

}
//buf2應該只有一段資料，只需解一次
void PackbitsDecoderKernel_ext(IMAGEFILE *psImage)
{


    long count;//代表該數重複幾次
    unsigned char ValueTemp;
		int i;

    count=(long)Buf2.Buf[Buf2.index++];


        if (count >= 128)
            count -= 256;
        if (count < 0) //有壓縮 剩下的都從buf1去讀
        {
         // replicate next byte -n+1 times
             if (count == -128)    // nop
             {
                 //ShowMessage("error");//不可能發生
                       mpDebugPrint("-------------error error error----------------------");
                       return;                 
						 } 

             count = -count + 1;
             //temp[1]=*pCompressionData++; n--;
             ValueTemp=Buf1.Buf[Buf1.index++];


             for(i=0;i<count;i++)
             {
                 //*(ImagePtr++)=ValueTemp;
                 //debug(ValueTemp);
                 WriteToYYCbCrBuffer(ValueTemp,psImage);                 
             }
             DecoderInfo_temp.ImagePtrRemainderByte-=count; //這個值要重複count次

             Buf2.index=-1;
             Buf2.DataLength=0;
             
        }else{       //沒壓縮  一部分是buf1，另一部分是buf2
             //for(int i=0;i<(Buf2.DataLength-Buf2.index);i++)
             while(Buf2.index<=(Buf2.DataLength-1))
             {
                //*(ImagePtr++)=Buf2.Buf[i];
                ValueTemp=Buf2.Buf[Buf2.index++];
								//*(ImagePtr++)=ValueTemp;
								//debug(ValueTemp);
								WriteToYYCbCrBuffer(ValueTemp,psImage);
             }


             for(i=0;i<(  (count+1)-(Buf2.DataLength-1) );i++)
             {
                //*(ImagePtr++)=Buf1.Buf[Buf1.index++];
                ValueTemp=Buf1.Buf[Buf1.index++];
								//*(ImagePtr++)=ValueTemp;
								//debug(ValueTemp);
								WriteToYYCbCrBuffer(ValueTemp,psImage);
             }
             DecoderInfo_temp.ImagePtrRemainderByte-=(count+1); //這個值要重複count次

             Buf2.index=-1;
             Buf2.DataLength=0;
        }      
}

//copy底下以前人的function，去file header內讀strip資料
void GetStripData(ST_MPX_STREAM *psStream,DWORD *dwstripOffset,DWORD *dwStripByteCount,BYTE boYield)
{

	int ii;
	ST_TIFF_INFO  *pstTiffTag;
	pstTiffTag = g_pstTiffTag;

	mpxStreamSeek(psStream, pstTiffTag->dwStripOffsets, SEEK_SET);
	for (ii=0;ii<pstTiffTag->dwStripCount;ii++)
	{
		dwstripOffset[ii]=TIFF_ReadDWord(psStream);

		//MP_DEBUG1("dwstripOffset[ii]=%8x", dwstripOffset[ii]);
	}

	if (boYield) TaskYield();

	mpxStreamSeek(psStream, pstTiffTag->dwStripByteCountOffset, SEEK_SET);
	for (ii=0;ii<pstTiffTag->dwStripCount;ii++)
	{
		dwStripByteCount[ii]=TIFF_ReadDWord(psStream);
	}
	
}

void PackbitsDecoder(WORD wBitCount,IMAGEFILE *psImage,BYTE boYield,WORD wScale)
{
		int i;
		DWORD *dwstripOffset;
		DWORD *dwStripByteCount;				
		ST_TIFF_INFO  *pstTiffTag;
		pstTiffTag = g_pstTiffTag;		
		ST_MPX_STREAM *psStream = psImage->psStream;
		int ReadBufState=0;

		ImageYYCbCrPtr=(DWORD *)psImage->pbTarget;

		DecoderInfo_temp.ImageCount=0;
    DecoderInfo_temp.Curr_X_Width=0;
		DecoderInfo_temp.Curr_Y_Height=0;
		DecoderInfo_temp.Scale=wScale;

    //initial buf
    Buf1.AllSize=BufferSize;
    Buf2.AllSize=BufferSize;
    //Buf1.Buf = new unsigned char[BufferSize];//放從file讀到的壓縮影像資料
    BYTE *pbBuf1 = (BYTE *)ext_mem_malloc(BufferSize);
		if (pbBuf1 == NULL) 
		{
      mpDebugPrint("-------------ext_mem_malloc error----------------------");
      return;
  	}	
		memset(pbBuf1, 0, BufferSize);
		Buf1.Buf = (BYTE *)((DWORD)pbBuf1 | 0xA0000000);

		
    //Buf2.Buf = new unsigned char[BufferSize];
    BYTE *pbBuf2 = (BYTE *)ext_mem_malloc(BufferSize);
		if (pbBuf2 == NULL) 
		{
      mpDebugPrint("-------------ext_mem_malloc error----------------------");
      return;
  	}	
		memset(pbBuf2, 0, BufferSize);
		Buf2.Buf = (BYTE *)((DWORD)pbBuf2 | 0xA0000000);


		
    Buf1.index=-1;//代表裡面沒資料
    Buf2.index=-1;//代表裡面沒資料



    //if(temp_TIFF_INFO->dwStripOffsets_Count==1)  //只有一個strip
    if (pstTiffTag->dwStripCount==1)
    {

        //DecoderInfo_temp.ImagefFileRemainderByte=temp_TIFF_INFO->dwStripByteCounts;
        DecoderInfo_temp.ImagefFileRemainderByte=pstTiffTag->dwStripByteCountOffset;

        //fseek(input, temp_TIFF_INFO->dwStripOffsets,SEEK_SET);
        mpxStreamSeek(psStream, pstTiffTag->dwStripOffsets, SEEK_SET);

    //}else if(temp_TIFF_INFO->dwStripOffsets_Count>1)    //超過一個strip
    }else if(pstTiffTag->dwStripCount>1)
    {

		
	      dwstripOffset = (DWORD *)ext_mem_malloc(pstTiffTag->dwStripCount*4);
				dwStripByteCount = (DWORD *)ext_mem_malloc(pstTiffTag->dwStripCount*4);
				GetStripData(psStream,dwstripOffset,dwStripByteCount,boYield);

		
        DecoderInfo_temp.StripIndex=0;

        //DecoderInfo_temp.Strip_Remainder_ByteCounts = new long[(int)temp_TIFF_INFO->dwStripOffsets_Count];
        DecoderInfo_temp.Strip_Remainder_ByteCounts = (DWORD *)ext_mem_malloc(pstTiffTag->dwStripCount*4);

        //for (int i = 0; i < (int)temp_TIFF_INFO->dwStripOffsets_Count; i++)  
        for (i = 0; i < pstTiffTag->dwStripCount; i++)
        {
            //DecoderInfo_temp.Strip_Remainder_ByteCounts[i]=(int)temp_StripByteCounts[i]; 
            DecoderInfo_temp.Strip_Remainder_ByteCounts[i]=dwStripByteCount[i];
        }

        //fseek(input, (int)temp_StripOffset[0],SEEK_SET);
        mpxStreamSeek(psStream, dwstripOffset[0], SEEK_SET);
    }else{
        mpDebugPrint("-------------0 strip error----------------------");
        return;
    }






    //initial decoder info

    switch (wBitCount)
    {
      case 8://gray
      {
        DecoderInfo_temp.ImagePtrRemainderByte=psImage->wImageHeight*psImage->wImageWidth;
      }
      break;
      case 24://RGB
      {
        DecoderInfo_temp.ImagePtrRemainderByte=psImage->wImageHeight*psImage->wImageWidth*3;
      }
      break;
			default:
			{
        mpDebugPrint("-------------no support----------------------");
        return;
			}
				
    }
	//__asm("break 100");

//mpDebugPrint("-********---ImageGetTargetSize=%d----------------------",ImageGetTargetSize());
	
		DecoderInfo_temp.DecoderState=0;
		DecoderInfo_temp.TaskYieldInfo=boYield;
		//DecoderInfo_temp.PtrNextLine=ImageYYCbCrPtr+(psImage->wTargetWidth >> 1);
    while(DecoderInfo_temp.DecoderState<10)
    {
        switch (DecoderInfo_temp.DecoderState)
        {
          case 0:   //??file???????
          {
             ReadBufState=ReadImageFileToBuffer(psStream,pstTiffTag,dwstripOffset,dwStripByteCount);
						 if(ReadBufState>0)
						 		DecoderInfo_temp.DecoderState=10;
          }
          break;
          case 1://buf1???,buf2???,??buf1???
          {
              PackbitsDecoderKernel(psImage);
          }
          break;
          case 2://buf1???,buf2???,??buf1???
          {
              PackbitsDecoderKernel_ext(psImage);
              PackbitsDecoderKernel(psImage);
          }
          break;

        }
    }



    //delete [] Buf1.Buf;
    if (pbBuf1) ext_mem_free(pbBuf1);
    //delete [] Buf2.Buf;
    if (pbBuf2) ext_mem_free(pbBuf2);
    //delete [] DecoderInfo_temp.Strip_Remainder_ByteCounts;
    if (DecoderInfo_temp.Strip_Remainder_ByteCounts) ext_mem_free(DecoderInfo_temp.Strip_Remainder_ByteCounts);



}

static int TIFF_Decode_Bits(IMAGEFILE *psImage, DWORD *pdwPal, WORD wBitCount, BYTE boYield)
{
	int n, iRowBytes;
	ST_TIFF_INFO  *pstTiffTag;
	pstTiffTag = g_pstTiffTag;
	WORD wScale =  1 << psImage->bScaleDown;
	MP_DEBUG2("TIFF_Decode_Bits %d %d", wBitCount, wScale);
	if (wBitCount <= 8) 
	{
		n = 8 / wBitCount;    // 1bit : n=8, 4bit : n=2, 8bit : n=1
		iRowBytes = psImage->wImageWidth / n;
		if (psImage->wImageWidth & (n - 1)) iRowBytes++;
	}
	else
	{
		n = wBitCount >> 3;    // 16bit : n=2, 24bit : n=3, 32bit : n=4
		iRowBytes = psImage->wImageWidth * n;
	}

	

	
	int i, j, y, iReadSize;
	BYTE bData0, bData1;	 
	BYTE *pbLinePtr;
	DWORD dwData;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;
	
	pdwTarget = (DWORD *)psImage->pbTarget;
	n = psImage->wRealTargetWidth;
	
  if(pstTiffTag->dwCompression==32773)//TYChen Add Packbits Compression
  {
  mpDebugPrint("11111111111111111111111155555555555555");
//__asm("break 100");
			PackbitsDecoder(wBitCount,psImage,boYield,wScale);//TYChen Add Packbits Compression
	}else{
	
  		BYTE *pbLineBuffer = (BYTE *)mem_malloc(iRowBytes + 256);
			if (pbLineBuffer == NULL) return FAIL;

			memset(pbLineBuffer, 0, iRowBytes);

	
	for (y = 0; y < psImage->wTargetHeight; y++)
	{			
		pdwNextLine = pdwTarget + (psImage->wTargetWidth >> 1);
		pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);		
		if (iReadSize == 0) break;
		
		mpxStreamSkip(psStream, iRowBytes * (wScale - 1));
		switch (wBitCount)
		{
			case 1 :  // 1 byte = 8 pixels, 2 colors
				for (i = 0; i < iRowBytes; i++)
				{
					bData0 = *pbLinePtr;
					for (j = 6; j >= 0; j -= 2)
					{
						bData1 = bData0 >> j;
						*pdwTarget++ = YCbCrYCbCr_2_YYCbCr(pdwPal[(bData1 >> 1) & 1], pdwPal[bData1 & 1]);	
					}
					pbLinePtr += 1 * wScale;
				}					
				break;
							
			case 8:  // 1 byte = 1 pixels, 256 colors
				if (pstTiffTag->dwColorMap>0)		/*Palette Image*/
				{
					for (i = 0; i < n; i+=2)
					{
						bData0 = *pbLinePtr;
						if (i < n)
							bData1 = *(pbLinePtr+1);
						else
							bData1 = bData0;
						
						*pdwTarget++ = YCbCrYCbCr_2_YYCbCr(pdwPal[bData0], pdwPal[bData1]);
						pbLinePtr += 2 * wScale;
					}	
				}
				else				/*Grayscale Image*/
				{
					for (i = 0; i < n; i+=2)
					{
						bData0 = *pbLinePtr;
						if (i < n)
							bData1 = *(pbLinePtr+1);
						else
							bData1 = bData0;
						if(pstTiffTag->PhotoMetric==0)	//TYChen Fix
						{
								bData0=255-bData0;
								bData1=255-bData1;
						}							
						*pdwTarget++ = RGBRGB_2_YUV(bData0,bData0,bData0,bData1,bData1,bData1);
						pbLinePtr += 2 * wScale;
					}					
				}
				break;	
				
			case 24:  						/*RGB-Full Image*/				
				for (i = 0; i < n; i+=2)
				{	
					*pdwTarget++ = RGBRGB_2_YUV(	pbLinePtr[0],pbLinePtr[1],pbLinePtr[2],
												pbLinePtr[3],pbLinePtr[4],pbLinePtr[5]);
					pbLinePtr += 6 * wScale;
				}
				break;
			default:
				break;
		}
		
		pdwTarget = pdwNextLine;
		
		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();
	}
	
	if (pbLineBuffer) mem_free(pbLineBuffer);
}
	return PASS;
}

//------------------------------------------------------------------------------


static int TIFF_YCbCr_Decode_Bits(IMAGEFILE *psImage, DWORD *pdwPal, WORD wBitCount, BYTE boYield)
{
    int n, iRowBytes;
	ST_TIFF_INFO  *pstTiffTag;
	pstTiffTag = g_pstTiffTag;
	WORD wScale =  1 << psImage->bScaleDown;
    int  iErrorCode=FAIL;
	DWORD YCbCr_sampling;

    YCbCr_sampling=pstTiffTag->dwYCbCrSubsampleHorizVert;

    if(YCbCr_sampling==0x10002) /*YCbCr 422 0x10002*/
	{
			   iRowBytes = psImage->wImageWidth * 2; /*n=2*/
			   MP_DEBUG1("iRowBytes=%d",iRowBytes);
			   iErrorCode=PASS;
	}
	else if (YCbCr_sampling==0x20002)/*YCbCr 420 0x20002*/
	{
	   MP_DEBUG2("##########   n=%d,iRowBytes=%d",n,iRowBytes);
			   iRowBytes = psImage->wImageWidth *3;/*n=3*/
			   MP_DEBUG1("iRowBytes=%d",iRowBytes);
			   iErrorCode=PASS;
	}

	int i, j, y, iReadSize;
	BYTE bData0, bData1;	 
	BYTE *pbLinePtr;
	DWORD dwData;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;
	DWORD *pdwTarget2;
	
	pdwTarget = (DWORD *)psImage->pbTarget;
	n = psImage->wRealTargetWidth;
	
  	BYTE *pbLineBuffer = (BYTE *)mem_malloc(iRowBytes + 256);
	if (pbLineBuffer == NULL) return FAIL;

	memset(pbLineBuffer, 0, iRowBytes);


    mpDebugPrint("iRowBytes=%d",iRowBytes);

    if(YCbCr_sampling==0x10002)/*-------------------------------------------*/
    {
	    for (y = 0; y < psImage->wTargetHeight; y++)
	    {			
		  pdwNextLine = pdwTarget + (psImage->wTargetWidth >> 1);
		  pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		  iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);		
		  if (iReadSize == 0) break;
		
		  mpxStreamSkip(psStream, iRowBytes * (wScale - 1));

           for (i = 0; i < n; i+=2)
           {
            *pdwTarget++= (pbLinePtr[0] << 24)| (pbLinePtr[1]<< 16) | (pbLinePtr[2]<<8) |pbLinePtr[3] /*pbLinePtr[3]*/;
             pbLinePtr += 4  * wScale;
           }
		
		  pdwTarget = pdwNextLine;
		
		  if ((boYield) && ((y & 0x3f) == 0))
			  TaskYield();
	    }
    }
	else if (YCbCr_sampling==0x20002)/*---------------------------------------*/
	{

       MP_DEBUG2("22222222 psImage->wTargetWidth=%d,psImage->wTargetHeight=%d",psImage->wTargetWidth,psImage->wTargetHeight);
	   		  MP_DEBUG1("wScale=%d",wScale);
	   	    for (y = 0; y < psImage->wTargetHeight; y+=2)
	    {			
		  pdwNextLine = pdwTarget + (psImage->wTargetWidth /*>> 1*/);

		  
          pdwTarget2=pdwTarget+(psImage->wTargetWidth >> 1);
           
		  
		  pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		  iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);		
		  if (iReadSize == 0) break; 

		  mpxStreamSkip(psStream, iRowBytes * (wScale - 1));

           for (i = 0; i < n; i+=2)
           {
            *pdwTarget++  = (pbLinePtr[0] << 24)| (pbLinePtr[1]<< 16) | (pbLinePtr[4]<<8) |pbLinePtr[5];
            *pdwTarget2++ = (pbLinePtr[2] << 24)| (pbLinePtr[3]<< 16) | (pbLinePtr[4]<<8) |pbLinePtr[5];
			 pbLinePtr += 6  * wScale;
           }
		
		  pdwTarget = pdwNextLine;
		
		  if ((boYield) && ((y & 0x3f) == 0))
			  TaskYield();
	    }
	}
	else/*-------------------------------------------------------------------*/
	{
	    	MP_ALERT("YCbCr not supported. YCbCr_sampling=0x%x",YCbCr_sampling);
			iErrorCode=FAIL;
	}
	
	
	if (pbLineBuffer) mem_free(pbLineBuffer);


	if(iErrorCode!=PASS)
		MP_ALERT("YCbCr not support this format. func:TIFF_YCbCr_Decode_Bits");
    
	return iErrorCode;
}


//-------------------------------------------------------------------------------
static int TIFF_Decode_MultiStrip_Bits(IMAGEFILE* psImage, DWORD *dwstripOffset,DWORD dwRowPerStrip, DWORD *dwStripByteCount, BOOL boYield)
{
	DWORD  n, iRowBytes;
	MP_DEBUG("$$$$TIFF_Decode_MultiStrip_Bits\n");
	
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	WORD wScale =  1 << psImage->bScaleDown;
	MP_DEBUG1("^^^^^^wScale=%d", wScale);

	  n = 3;    // 16bit : n=2, 24bit : n=3, 32bit : n=4
	  iRowBytes = psImage->wImageWidth * n;
    
    BYTE *pbLineBuffer = (BYTE *)ext_mem_malloc(iRowBytes + 256);
	if (pbLineBuffer == NULL) return FAIL;
	
	memset(pbLineBuffer, 0, iRowBytes);

	DWORD i, j, y, iReadSize;
	BYTE bData0, bData1;	 
	BYTE *pbLinePtr;
	DWORD dwData;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;
	DWORD dwRowcount;
	DWORD *dwStripOffsetCount;
	WORD winStripCount=0;
	
	pdwTarget = (DWORD *)psImage->pbTarget;
	n = psImage->wRealTargetWidth;
	
	dwStripOffsetCount = dwstripOffset;

MP_DEBUG1("-----dwRowPerStrip=%d",dwRowPerStrip);
MP_DEBUG1("=====*dwStripOffsetCount=0x%x",*dwStripOffsetCount);
	
	mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
	dwStripOffsetCount++;
	MP_DEBUG1("$$$$$$$dwStripOffsetCount=%d",*dwStripOffsetCount);

	MP_DEBUG1("iRowBytes =%d",iRowBytes);

	for (y = 0; y < psImage->wTargetHeight; y++)
	{	
		pdwNextLine = pdwTarget + (psImage->wTargetWidth >> 1);
		
		pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);		
		if (iReadSize == 0) break;
		
		dwRowcount=winStripCount*wScale;
		winStripCount++;
		MP_DEBUG1("$$$$$$$dwRowcount=%d",dwRowcount);
		
		if ((dwRowcount+wScale)>=dwRowPerStrip)	/*when Use the next Strip*/
		{
			WORD dwLeftRow;
			if (wScale>dwRowPerStrip)				//when ROWPerStrip is Little Scale, seek MUlti Strip
			{
				dwStripOffsetCount+=wScale/dwRowPerStrip;
				mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
			}
			else
			{
				mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
				dwStripOffsetCount++;
				MP_DEBUG1("$$$$$$$dwStripOffsetCount=%d",*dwStripOffsetCount);

				dwLeftRow=dwRowPerStrip-dwRowcount;
				mpxStreamSkip(psStream, iRowBytes * (wScale-dwLeftRow));		//line skip		
				winStripCount=0;
			}


		}
		else
		{
			mpxStreamSkip(psStream, iRowBytes * (wScale - 1));		//line skip		
		}
		
		
		for (i = 0; i < n; i+=2)
		{	
			*pdwTarget++ = RGBRGB_2_YUV(	pbLinePtr[0],pbLinePtr[1],pbLinePtr[2],
										pbLinePtr[3],pbLinePtr[4],pbLinePtr[5]);
			pbLinePtr += 6 * wScale;				//row skip
		}
		pdwTarget = pdwNextLine;
		
		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();

	}

	
	if (pbLineBuffer) ext_mem_free(pbLineBuffer);
	return PASS;
	
}

//-------------------------------------------------------------------------------

//for SEPARATE 
static int TIFF_Decode_SEPARATE_MultiStrip_Bits(IMAGEFILE* psImage, DWORD *dwstripOffset,DWORD dwRowPerStrip, DWORD *dwStripByteCount, BOOL boYield, ST_TIFF_INFO  *pstTiffTag)
{

	int   iErrorCode = FAIL;
	
	MP_DEBUG("$$$$TIFF_Decode_SEPARATE_MultiStrip_Bits\n");
	
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	WORD wScale =  1 << psImage->bScaleDown;
	MP_DEBUG1("^^^^^^wScale=%d", wScale);

 

	MP_DEBUG1("ff 0901 pstTiffTag->dwStripCount=%d",pstTiffTag->dwStripCount);

   
#if 1
	if(pstTiffTag->dwStripCount==3)
	{
		/*for RGB Separate , single strip*/
        /* _____*/
		/*|  R   |*/
		/*|       |*/
		/*|____|*/
		/*|  G   |*/
		/*|       |*/
		/*|____|*/
		/*|  B   |*/
		/*|       |*/
		/*|____|*/
		/*          */
        
      iErrorCode=TIFF_Decode_RGB_SEPARATE_SingleStrip_Bits(psImage,dwstripOffset,dwRowPerStrip,dwStripByteCount,boYield,pstTiffTag);
  
      if(iErrorCode!=PASS)
	  	MP_ALERT("TIFF_Decode_SEPARATE_SingleStrip_Bits FAIL!!");
		
	}
	else
	{
	  iErrorCode=TIFF_Decode_RGB_SEPARATE_MultiStrip_Bits(psImage,dwstripOffset,dwRowPerStrip,dwStripByteCount,boYield,pstTiffTag);

	  if(iErrorCode!=PASS)
	  	MP_ALERT("TIFF_Decode_RGB_SEPARATE_MultiStrip_Bits FAIL!!");
	}

#else
  iErrorCode=TIFF_Decode_RGB_SEPARATE_SingleStrip_Bits(psImage,dwstripOffset,dwRowPerStrip,dwStripByteCount,boYield,pstTiffTag);
#endif


   if(iErrorCode==FAIL)
   	 MP_ALERT("TIFF_Decode_SEPARATE_MultiStrip_Bits FAIL!!");
	
   return iErrorCode;
}


//-------------------------------------------------------------------------------


static int TIFF_Decode_RGB_SEPARATE_SingleStrip_Bits(IMAGEFILE* psImage, DWORD *dwstripOffset,DWORD dwRowPerStrip, DWORD *dwStripByteCount, BOOL boYield, ST_TIFF_INFO  *pstTiffTag)
{
    BYTE *pbLineBufferR=NULL;
	BYTE *pbLineBufferG=NULL;
	BYTE *pbLineBufferB=NULL;

    DWORD R_pos;
	DWORD G_pos;
	DWORD B_pos;
	
	DWORD  n, iRowBytes;
	MP_DEBUG("$$$$TIFF_Decode_MultiStrip_Bits\n");
	
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	WORD wScale =  1 << psImage->bScaleDown;
	MP_DEBUG1("^^^^^^wScale=%d", wScale);



    MP_DEBUG("TIFF_Decode_MultiStrip_Bits : n=1");
	/*n=1 RGB separate,the row of each part =psImage->wImageWidth*/
    iRowBytes = psImage->wImageWidth;

	pbLineBufferR = (BYTE *)ext_mem_malloc(iRowBytes*wScale*MEM_BASE_SIZE + 256);
	if (pbLineBufferR == NULL) return FAIL;
	memset(pbLineBufferR, 0, iRowBytes);

	pbLineBufferG = (BYTE *)ext_mem_malloc(iRowBytes*wScale*MEM_BASE_SIZE + 256);
    if (pbLineBufferG == NULL) return FAIL;
	memset(pbLineBufferG, 0, iRowBytes);

	pbLineBufferB = (BYTE *)ext_mem_malloc(iRowBytes*wScale*MEM_BASE_SIZE + 256);
	if (pbLineBufferB == NULL) return FAIL;
	memset(pbLineBufferB, 0, iRowBytes);
 

	DWORD i, j, y, iReadSize;
	BYTE bData0, bData1;	 

	BYTE *pbLinePtrR;
    BYTE *pbLinePtrG;
	BYTE *pbLinePtrB;
    DWORD StripCountPerRGB;
	DWORD ii=0;/*for Multi-Strip*/
	
	DWORD dwData;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;
	DWORD dwRowcount;
	DWORD *dwStripOffsetCount;
	WORD winStripCount=0;

	DWORD f_temp;

	DWORD get_time;
	BYTE *testaddr;


    StripCountPerRGB= (pstTiffTag->dwStripCount)/3; /*for RGB multi-strip*/
	MP_DEBUG2("pstTiffTag->dwStripCount=%d,StripCountPerRGB=%d",pstTiffTag->dwStripCount,StripCountPerRGB);

	
	pdwTarget = (DWORD *)psImage->pbTarget;
	n = psImage->wRealTargetWidth;

	//Get R: Frank Lin add
	dwStripOffsetCount = dwstripOffset;
	R_pos=dwStripOffsetCount[0];
	MP_DEBUG1("R_pos=%d",R_pos);

    //Get G: Frank Lin add
    G_pos=dwStripOffsetCount[StripCountPerRGB];
	MP_DEBUG1("G_pos=%d",G_pos);

	//Get B: Frank Lin add	
    B_pos=dwStripOffsetCount[StripCountPerRGB<<1];
    MP_DEBUG1("B_pos=%d",B_pos);

	MP_DEBUG1("iRowBytes =%d",iRowBytes);

	for (y = 0; y <psImage->wTargetHeight; y+=MEM_BASE_SIZE)
	{	        
		pbLinePtrR = (BYTE *)((DWORD)pbLineBufferR | 0xA0000000);
		pbLinePtrG = (BYTE *)((DWORD)pbLineBufferG | 0xA0000000);
		pbLinePtrB = (BYTE *)((DWORD)pbLineBufferB | 0xA0000000);


   //R-Seek to R_pos,and read R data 
   mpxStreamSeek(psStream, R_pos, SEEK_SET);
   iReadSize = mpxStreamRead(pbLinePtrR, 1, iRowBytes*wScale*MEM_BASE_SIZE , psStream);

   //G-Seek to G_pos,and read G data
   mpxStreamSeek(psStream, G_pos, SEEK_SET);
   iReadSize = mpxStreamRead(pbLinePtrG, 1, iRowBytes*wScale*MEM_BASE_SIZE , psStream);

   //B-Seek to B_pos,and read B data 
   mpxStreamSeek(psStream, B_pos, SEEK_SET);
   iReadSize = mpxStreamRead(pbLinePtrB, 1, iRowBytes*wScale*MEM_BASE_SIZE , psStream);
	
   dwRowcount=winStripCount*wScale*MEM_BASE_SIZE;
   winStripCount++;
   MP_DEBUG1("=============$$$$$$$dwRowcount=%d",dwRowcount);


	MP_DEBUG3("fff dwRowcount=%d,wScale=%d,dwRowPerStrip=%d",dwRowcount,wScale,dwRowPerStrip);
		
		if ((dwRowcount+wScale)>=dwRowPerStrip)	/*when Use the next Strip*/
		{
		
			WORD dwLeftRow;
			ii++;
			if (wScale>dwRowPerStrip)				//when ROWPerStrip is Little Scale, seek MUlti Strip
			{			
			    dwStripOffsetCount = dwstripOffset;
				R_pos=dwStripOffsetCount[ii+(wScale/dwRowPerStrip)];
				G_pos=dwStripOffsetCount[ii+StripCountPerRGB+(wScale/dwRowPerStrip)];
				B_pos=dwStripOffsetCount[ii+(StripCountPerRGB<<1)+(wScale/dwRowPerStrip)];
			}
			else
			{
				
				dwStripOffsetCount = dwstripOffset;
				R_pos=dwStripOffsetCount[ii];
				G_pos=dwStripOffsetCount[ii+StripCountPerRGB];
				B_pos=dwStripOffsetCount[ii+(StripCountPerRGB<<1)];
				winStripCount=0;		    
			}

           MP_DEBUG3("((((( Multi-stripe R_pos=0x%x,G_pos=0x%x,B_pos=0x%x",R_pos,G_pos,B_pos);

		}
		else
		{
			R_pos+=iRowBytes*wScale*MEM_BASE_SIZE;
			G_pos+=iRowBytes*wScale*MEM_BASE_SIZE;
			B_pos+=iRowBytes*wScale*MEM_BASE_SIZE;
						
		}

		

      int yi=0;
	  for(yi=0;yi<MEM_BASE_SIZE;yi++)
	  {

        MP_DEBUG4("yi=%d pbLinePtrR=0x%x,pbLinePtrG=0x%x,pbLinePtrB=0x%x",yi,pbLinePtrR,pbLinePtrG,pbLinePtrB);
	    pdwNextLine = pdwTarget + (psImage->wTargetWidth >> 1);
		
		for (i = 0; i < n; i+=2)
		{	

			*pdwTarget++ = RGBRGB_2_YUV(pbLinePtrR[0],pbLinePtrG[0],pbLinePtrB[0],
										pbLinePtrR[1],pbLinePtrG[1],pbLinePtrB[1]);
			pbLinePtrR += 2   * wScale;				//row skip
			pbLinePtrG += 2   * wScale;
			pbLinePtrB += 2   * wScale;

		}

		pbLinePtrR+= iRowBytes*(wScale-1);
		pbLinePtrG+= iRowBytes*(wScale-1);
		pbLinePtrB+= iRowBytes*(wScale-1);
		
		pdwTarget = pdwNextLine;

	  }

		
		
		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();

	}

	
	if (pbLineBufferR) ext_mem_free(pbLineBufferR);
	if (pbLineBufferG) ext_mem_free(pbLineBufferG);
	if (pbLineBufferB) ext_mem_free(pbLineBufferB);
	return PASS;
	
}




//--------------------------------------------------------------------------------


#define F_TEMP_SIZE MEM_BASE_SIZE


static int TIFF_Decode_RGB_SEPARATE_MultiStrip_Bits(IMAGEFILE* psImage, DWORD *dwstripOffset,DWORD dwRowPerStrip, DWORD *dwStripByteCount, BOOL boYield, ST_TIFF_INFO  *pstTiffTag)
{
    BYTE *pbLineBufferR=NULL;
	BYTE *pbLineBufferG=NULL;
	BYTE *pbLineBufferB=NULL;

    DWORD R_pos;
	DWORD G_pos;
	DWORD B_pos;
	
	DWORD  n, iRowBytes;
	MP_DEBUG("$$$$TIFF_Decode_MultiStrip_Bits\n");

	DWORD Read_Row_Size;
	
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	WORD wScale =  1 << psImage->bScaleDown;
	MP_DEBUG1("^^^^^^wScale=%d", wScale);


    Read_Row_Size=F_TEMP_SIZE;

    MP_DEBUG("TIFF_Decode_MultiStrip_Bits : n=1");
	/*n=1 RGB separate,the row of each part =psImage->wImageWidth*/
    iRowBytes = psImage->wImageWidth;

	pbLineBufferR = (BYTE *)ext_mem_malloc(iRowBytes*wScale*Read_Row_Size + 256);
	if (pbLineBufferR == NULL) return FAIL;
	memset(pbLineBufferR, 0, iRowBytes);

	pbLineBufferG = (BYTE *)ext_mem_malloc(iRowBytes*wScale*Read_Row_Size + 256);
    if (pbLineBufferG == NULL) return FAIL;
	memset(pbLineBufferG, 0, iRowBytes);

	pbLineBufferB = (BYTE *)ext_mem_malloc(iRowBytes*wScale*Read_Row_Size + 256);
	if (pbLineBufferB == NULL) return FAIL;
	memset(pbLineBufferB, 0, iRowBytes);
 

	DWORD i, j, y, iReadSize;
	BYTE bData0, bData1;	 

	BYTE *pbLinePtrR;
    BYTE *pbLinePtrG;
	BYTE *pbLinePtrB;
    DWORD StripCountPerRGB;
	DWORD ii=0;/*for Multi-Strip*/
	
	DWORD dwData;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;
	DWORD dwRowcount;
	DWORD *dwStripOffsetCount;
	WORD winStripCount=0;

	DWORD f_temp;

	DWORD get_time;
	BYTE *testaddr;


    StripCountPerRGB= (pstTiffTag->dwStripCount)/3; /*for RGB multi-strip*/
	MP_DEBUG2("pstTiffTag->dwStripCount=%d,StripCountPerRGB=%d",pstTiffTag->dwStripCount,StripCountPerRGB);

	
	pdwTarget = (DWORD *)psImage->pbTarget;
	n = psImage->wRealTargetWidth;

	//Get R: Frank Lin add
	dwStripOffsetCount = dwstripOffset;
	R_pos=dwStripOffsetCount[0];
	MP_DEBUG1("R_pos=%d",R_pos);

    //Get G: Frank Lin add
    G_pos=dwStripOffsetCount[StripCountPerRGB];
	MP_DEBUG1("G_pos=%d",G_pos);

	//Get B: Frank Lin add	
    B_pos=dwStripOffsetCount[StripCountPerRGB<<1];
    MP_DEBUG1("B_pos=%d",B_pos);

	MP_DEBUG1("iRowBytes =%d",iRowBytes);

	for (y = 0; y <psImage->wTargetHeight; y+=Read_Row_Size)
	{	        
		pbLinePtrR = (BYTE *)((DWORD)pbLineBufferR | 0xA0000000);
		pbLinePtrG = (BYTE *)((DWORD)pbLineBufferG | 0xA0000000);
		pbLinePtrB = (BYTE *)((DWORD)pbLineBufferB | 0xA0000000);


   //R-Seek to R_pos,and read R data 
   mpxStreamSeek(psStream, R_pos, SEEK_SET);
   iReadSize = mpxStreamRead(pbLinePtrR, 1, iRowBytes*wScale*Read_Row_Size , psStream);

   //G-Seek to G_pos,and read G data
   mpxStreamSeek(psStream, G_pos, SEEK_SET);
   iReadSize = mpxStreamRead(pbLinePtrG, 1, iRowBytes*wScale*Read_Row_Size , psStream);

   //B-Seek to B_pos,and read B data 
   mpxStreamSeek(psStream, B_pos, SEEK_SET);
   iReadSize = mpxStreamRead(pbLinePtrB, 1, iRowBytes*wScale*Read_Row_Size , psStream);
	
   dwRowcount=winStripCount*wScale*Read_Row_Size;
   winStripCount++;
   MP_DEBUG2("y=%d =============$$$$$$$dwRowcount=%d",y,dwRowcount);
		

    	if ((dwRowcount+wScale*F_TEMP_SIZE)>=dwRowPerStrip)	/*when Use the next Strip*/
		{
		
			WORD dwLeftRow;
			ii++;
			if (wScale>dwRowPerStrip)				//when ROWPerStrip is Little Scale, seek MUlti Strip
			{			
			    dwStripOffsetCount = dwstripOffset;
				R_pos=dwStripOffsetCount[ii+(wScale/dwRowPerStrip)];
				G_pos=dwStripOffsetCount[ii+StripCountPerRGB+(wScale/dwRowPerStrip)];
				B_pos=dwStripOffsetCount[ii+(StripCountPerRGB<<1)+(wScale/dwRowPerStrip)];
					
			}
			else
			{


				dwStripOffsetCount = dwstripOffset;
				R_pos=dwStripOffsetCount[ii];
				G_pos=dwStripOffsetCount[ii+StripCountPerRGB];
				B_pos=dwStripOffsetCount[ii+(StripCountPerRGB<<1)];
				winStripCount=0;				
			}

           MP_DEBUG3("((((( Multi-stripe R_pos=0x%x,G_pos=0x%x,B_pos=0x%x",R_pos,G_pos,B_pos);

		}
		else
		{
			R_pos+=iRowBytes*wScale*Read_Row_Size;
			G_pos+=iRowBytes*wScale*Read_Row_Size;
			B_pos+=iRowBytes*wScale*Read_Row_Size;
						
		}
		

      int yi=0;
	  for(yi=0;yi<Read_Row_Size;yi++)
	  {

        MP_DEBUG4("yi=%d pbLinePtrR=0x%x,pbLinePtrG=0x%x,pbLinePtrB=0x%x",yi,pbLinePtrR,pbLinePtrG,pbLinePtrB);
	    pdwNextLine = pdwTarget + (psImage->wTargetWidth >> 1);
		
		for (i = 0; i < n; i+=2)
		{	

			*pdwTarget++ = RGBRGB_2_YUV(pbLinePtrR[0],pbLinePtrG[0],pbLinePtrB[0],
										pbLinePtrR[1],pbLinePtrG[1],pbLinePtrB[1]);
			pbLinePtrR += 2   * wScale;				//row skip
			pbLinePtrG += 2   * wScale;
			pbLinePtrB += 2   * wScale;

		}

		pbLinePtrR+= iRowBytes*(wScale-1);
		pbLinePtrG+= iRowBytes*(wScale-1);
		pbLinePtrB+= iRowBytes*(wScale-1);
		
		pdwTarget = pdwNextLine;

	  }

		
		
		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();

	}

	
	if (pbLineBufferR) ext_mem_free(pbLineBufferR);
	if (pbLineBufferG) ext_mem_free(pbLineBufferG);
	if (pbLineBufferB) ext_mem_free(pbLineBufferB);
	return PASS;
	
}



//-------------------------------------------------------------------------------
static int TIFF_YCbCr_Decode_MultiStrip_Bits(IMAGEFILE* psImage, DWORD *dwstripOffset,DWORD dwRowPerStrip, DWORD *dwStripByteCount, BOOL boYield,ST_TIFF_INFO  *pstTiffTag)
{
	DWORD  n, iRowBytes;
	ST_TIFF_INFO  *pstTiffTag;
	pstTiffTag = g_pstTiffTag;
    DWORD iErrorCode;
	
	
	MP_DEBUG("$$$$TIFF_Decode_MultiStrip_Bits\n");

	iErrorCode=FAIL;
	
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	WORD wScale =  1 << psImage->bScaleDown;
	MP_DEBUG1("^^^^^^wScale=%d", wScale);
	
	//n = 2;    // 16bit : n=2, 24bit : n=3, 32bit : n=4
	//iRowBytes = psImage->wImageWidth * n;

	DWORD YCbCr_sampling;
    YCbCr_sampling=pstTiffTag->dwYCbCrSubsampleHorizVert;
    if(YCbCr_sampling==0x10002) /*YCbCr 422 0x10002*/
	{
			   //MP_DEBUG1("wBitCount=%d",wBitCount);
			   iRowBytes = psImage->wImageWidth * 2; /*n=2*/
			   MP_DEBUG1("iRowBytes=%d",iRowBytes);
			   iErrorCode=PASS;
	}
	else if (YCbCr_sampling==0x20002)/*YCbCr 420 0x20002*/
	{
	   MP_DEBUG2("##########   n=%d,iRowBytes=%d",n,iRowBytes);
			   iRowBytes = psImage->wImageWidth *3;/*n=3*/
			   MP_DEBUG1("iRowBytes=%d",iRowBytes);
			   iErrorCode=PASS;
	}


    
    BYTE *pbLineBuffer = (BYTE *)ext_mem_malloc(iRowBytes + 256);
	if (pbLineBuffer == NULL) return FAIL;
	
	memset(pbLineBuffer, 0, iRowBytes);

	DWORD i, j, y, iReadSize;
	BYTE bData0, bData1;	 
	BYTE *pbLinePtr;

    BYTE *pbLinePtr2;
	
	DWORD dwData;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;
	DWORD *pdwTarget2;
	DWORD dwRowcount;
	DWORD *dwStripOffsetCount;
	WORD winStripCount=0;

	DWORD ii=0;/*for Multi-Strip YCbCr 420*/
	
	pdwTarget = (DWORD *)psImage->pbTarget;
	n = psImage->wRealTargetWidth;
	

/*------------------422---------------*/


     dwStripOffsetCount = dwstripOffset;
	
	mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
	dwStripOffsetCount++;
	MP_DEBUG1("$$$$$$$dwStripOffsetCount=%d",*dwStripOffsetCount);
	
	if(YCbCr_sampling==0x10002)
	{
	  for (y = 0; y < psImage->wTargetHeight; y++)
	  {	
		pdwNextLine = pdwTarget + (psImage->wTargetWidth >> 1);
		
		pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);		
		if (iReadSize == 0) break;
		
		dwRowcount=winStripCount*wScale;
		winStripCount++;
		MP_DEBUG1("$$$$$$$dwRowcount=%d",dwRowcount);
		
		if ((dwRowcount+wScale)>=dwRowPerStrip)	/*when Use the next Strip*/
		{
			WORD dwLeftRow;
			if (wScale>dwRowPerStrip)				//when ROWPerStrip is Little Scale, seek MUlti Strip
			{
				dwStripOffsetCount+=wScale/dwRowPerStrip;
				mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
			}
			else
			{
				mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
				dwStripOffsetCount++;
				MP_DEBUG1("$$$$$$$dwStripOffsetCount=%d",*dwStripOffsetCount);

				dwLeftRow=dwRowPerStrip-dwRowcount;
				mpxStreamSkip(psStream, iRowBytes * (wScale-dwLeftRow));		//line skip		
				winStripCount=0;
			}


		}
		else
		{
			mpxStreamSkip(psStream, iRowBytes * (wScale - 1));		//line skip		
		}
		
		
		for (i = 0; i < n; i+=2)
		{	
          *pdwTarget++= (pbLinePtr[0] << 24)| (pbLinePtr[1]<< 16) | (pbLinePtr[2]<<8) |pbLinePtr[3] /*pbLinePtr[3]*/;
           pbLinePtr += 4  * wScale;				//row skip
		}
		pdwTarget = pdwNextLine;
		
		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();

	  }
	}
	else if (YCbCr_sampling==0x20002)/*--------- 420------------------------------*/
	{

 
     // dwRowPerStrip=dwRowPerStrip>>1;  //frank add

	 DWORD BytePerRow= (*dwStripByteCount)/pstTiffTag->dwRowPerStrip;
	 DWORD ImageWidth= (DWORD) pstTiffTag->dwImageWidth;
	 BYTE  CheckY2Y3Zero=NULL;
	 BYTE  Y2Y3Result=NULL;
	 DWORD YIncRow;
	 MP_DEBUG3("F===== BytePerRow=%d,pstTiffTag->dwImageWidth=%d,pstTiffTag->dwRowPerStrip=%d",BytePerRow,pstTiffTag->dwImageWidth,pstTiffTag->dwRowPerStrip);

    
	 YIncRow=2; /*for YCbCr 420*/
		

    
    /*Check Y0 Y1 Y2 Y3 U0 V0 ,Y2==0 and  Y3==0 */
	if( (ImageWidth * 3/2) != BytePerRow)
	{
	  
	  MP_DEBUG("===========check y2 and y3 =================");
	  dwStripOffsetCount = dwstripOffset;
      mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
      pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
      iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);	

	  for (i = 0; i < n; i+=2)
		{	
		   	
            Y2Y3Result |= (pbLinePtr[2] << 8)| (pbLinePtr[3]); /*check y2 and y3 are zero or not.*/
		}

	  if(Y2Y3Result==NULL)
	  	{
	  	  YIncRow=1;
		  CheckY2Y3Zero=1;
		  wScale=wScale>>1;
	  	}
	  else
	  	{
	  	 MP_DEBUG("Y2Y3Result is not zero. ");
	  	}
	}

	
     /*-----------------------------------------------------------------*/

    dwStripOffsetCount = dwstripOffset;
    mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);

    MP_DEBUG2("fff 0903-pstTiffTag->dwRowPerStrip=%d,pstTiffTag->dwStripByteCountOffset=%d",pstTiffTag->dwRowPerStrip,pstTiffTag->dwStripByteCountOffset);//dwStripByteCountOffset

	
	MP_DEBUG1("$$$$$$$dwStripOffsetCount=%d",*dwStripOffsetCount);
    MP_DEBUG1("hhhhhhhhhh iRowBytes=%d",iRowBytes);

	  for (y = 0; y < psImage->wTargetHeight; y+=YIncRow)
	  {	
		
		
		pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);		
		if (iReadSize == 0) break;
		
		dwRowcount=(winStripCount*wScale)<<1;
		winStripCount++;//frank lin modify

        if ((dwRowcount+wScale*2)>=dwRowPerStrip)	/*when Use the next Strip*/
        {
			WORD dwLeftRow;
			ii++;
			if (wScale*2>dwRowPerStrip)				//when ROWPerStrip is Little Scale, seek MUlti Strip
			{
			   dwStripOffsetCount = dwstripOffset;
    		   dwStripOffsetCount+=ii*(wScale/dwRowPerStrip)*2;
			   mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
			}
			else
			{   

				dwStripOffsetCount = dwstripOffset;
				dwStripOffsetCount+=ii;
				mpxStreamSeek(psStream, *dwStripOffsetCount, SEEK_SET);
				MP_DEBUG1("$$$$$$$dwStripOffsetCount=0x%x",*dwStripOffsetCount);
			    winStripCount=0;
			}
			
		}
		else
		{
			mpxStreamSkip(psStream, iRowBytes * (wScale - 1));		//line skip	ok			
		}

		if( (Y2Y3Result==NULL) && (CheckY2Y3Zero==1))
		{
		  pdwNextLine = pdwTarget + (psImage->wTargetWidth>>1 );
		  for (i = 0; i < n; i+=2)
		  {	
		   	
            *pdwTarget++  = (pbLinePtr[0] << 24)| (pbLinePtr[1]<< 16) | (pbLinePtr[4]<<8) |pbLinePtr[5];
		     pbLinePtr += 6  * (wScale<<1);
		  }
		}
		else
		{
	      pdwNextLine = pdwTarget + (psImage->wTargetWidth );
		  pdwTarget2  = pdwTarget+(psImage->wTargetWidth >> 1);
		  for (i = 0; i < n; i+=2)
		  {	
		   	
            *pdwTarget++  = (pbLinePtr[0] << 24)| (pbLinePtr[1]<< 16) | (pbLinePtr[4]<<8) |pbLinePtr[5];
		    *pdwTarget2++ = (pbLinePtr[2] << 24)| (pbLinePtr[3]<< 16) | (pbLinePtr[4]<<8) |pbLinePtr[5];
          	 pbLinePtr += 6  * wScale; 
		  }
		}		
		pdwTarget = pdwNextLine;

		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();

	  }
	}
	else/*-------------------------not supported--------------------------------*/
	{
	    	MP_ALERT("YCbCr MultiStrip not supported. YCbCr_sampling=0x%x",YCbCr_sampling);
			iErrorCode=FAIL;
	}
/*--------------------------------*/
#if 0  /*dump data for debug*/
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		char s[256];

        DWORD *F_out_buf;
		DWORD *F_out_buf_next;

        DWORD *f_memptr;
		DWORD *f_memptr2;

		f_memptr=malloc(60*60*4);
		f_memptr2=f_memptr;
        int fi;
		int fj;

		F_out_buf=(DWORD *)psImage->pbTarget;
		
		for( fi=0;fi<60;fi++)
		{
		  F_out_buf_next=F_out_buf+(psImage->wTargetWidth >> 1);
		  for( fj=0;fj<60;fj++)
		  	{
		  	  *f_memptr++=*F_out_buf++;
		  	}
		  F_out_buf=F_out_buf_next;
		}
		


			strcpy(s,"source");
			
			//MP_DEBUG("dump source pack_no %d",graph->demux->video->pack_no);
			sDrv = DriveGet(SD_MMC);

			ret = CreateFile(sDrv, s, "raw");
			if (ret) UartOutText("create file fail\r\n");

			shandle = FileOpen(sDrv);
			if (!shandle) UartOutText("open file fail\r\n");

			ret = FileWrite(shandle, f_memptr2, 60*60*4);
			if (!ret) UartOutText("write file fail\r\n");

			FileClose(shandle);
			UartOutText("\n\rfile close\n\r");

			free(f_memptr2);
		
#endif

/*--------------------------------*/

	
	if (pbLineBuffer) ext_mem_free(pbLineBuffer);

    if(iErrorCode!=PASS)
		MP_ALERT("YCbCr not support this format. func:TIFF_YCbCr_Decode_Bits");
  
	
	return iErrorCode;
	
}

/*Frank Lin add 20100923*/
void TIFF_2ndIFD(ST_MPX_STREAM *psStream, ST_TIFF_INFO  *pstTiffTag, DWORD IFDOffset)
{
    BYTE JpegParseEnable=0;
	DWORD NoOfIFDDE;
	DWORD i;
	DWORD ValueOffset;//Frank Lin
	DWORD SeekNextTag;//Frank lin

    ST_TIFF_IFD_TAG stTempTag;
	
	
    MP_DEBUG("--TIFF_2ndIFD--");

	mpxStreamSeek(psStream,IFDOffset, SEEK_SET);
	NoOfIFDDE=TIFF_Readword(psStream);
	MP_DEBUG1("NoOfIFDDE=%d",NoOfIFDDE);


   for(i=0;i<NoOfIFDDE;i++)
   {
        stTempTag.wTempTAG	= TIFF_Readword(psStream);
		stTempTag.wValueType= TIFF_Readword(psStream);
		stTempTag.dwValueCount= TIFF_ReadDWord(psStream);
		stTempTag.dwValue= TIFF_ReadDWord(psStream);
		
	
   	switch(stTempTag.wTempTAG)
   	{
	        case  TIFFTAG_IMAGEWIDTH:
				pstTiffTag->dwImageWidth=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwImageWidth=%d ", pstTiffTag->dwImageWidth);
				break;
			case TIFFTAG_IMAGELENGTH:
				pstTiffTag->dwImageHeight=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwImageheight=%d ", pstTiffTag->dwImageHeight);
				break;
			case TIFFTAG_BITSPERSAMPLE:
				pstTiffTag->wSampleCount=stTempTag.dwValueCount;
				MP_DEBUG1("$$TIFF wSampleCount=%d ", pstTiffTag->wSampleCount);

                //Frank Lin add
				ValueOffset=stTempTag.dwValue;
				SeekNextTag=mpxStreamTell(psStream);
				
                MP_DEBUG1("ValueOffset=0x%x",ValueOffset);
                mpxStreamSeek(psStream, ValueOffset, SEEK_SET);
				pstTiffTag->wBitsPerSample=TIFF_Readword(psStream);
                MP_DEBUG1("wBitsPerSample=%d",pstTiffTag->wBitsPerSample);
				mpxStreamSeek(psStream, SeekNextTag, SEEK_SET);/*Seek to the next tag*/
								
				pstTiffTag->wBitsPerSample=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF wBitsPerSample=%d ", pstTiffTag->wBitsPerSample);
				break;			
			case TIFFTAG_COMPRESSION:
				pstTiffTag->dwCompression=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwCompression=%d ", pstTiffTag->dwCompression);
				if((pstTiffTag->dwCompression)==6)
					JpegParseEnable=1;
				break;			
			case TIFFTAG_PHOTOMETRIC:
				pstTiffTag->PhotoMetric=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF PhotoMetric=%d ", pstTiffTag->PhotoMetric);
				break;			
			case TIFFTAG_STRIPOFFSETS:
				pstTiffTag->dwStripCount=stTempTag.dwValueCount;		
				pstTiffTag->dwStripOffsets=stTempTag.dwValue;
				
				MP_DEBUG1("$$TIFF dwStripCount=%d ", pstTiffTag->dwStripCount);
				MP_DEBUG1("$$TIFF dwStripOffsets=%d ", pstTiffTag->dwStripOffsets);
				break;
			case TIFFTAG_ROWSPERSTRIP:
				pstTiffTag->dwRowPerStrip=stTempTag.dwValue;			
				MP_DEBUG1("$$TIFF dwRowPerStrip=%d ", pstTiffTag->dwRowPerStrip);
				break;
			case TIFFTAG_STRIPBYTECOUNTS:
				pstTiffTag->dwStripByteCountOffset=stTempTag.dwValue;			
				MP_DEBUG1("$$TIFF dwStripByteCountOffset=%d ", pstTiffTag->dwStripByteCountOffset);
				break;			
			case TIFFTAG_ORIENTATION:
				pstTiffTag->dwOrientation=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwOrientation=%d ", pstTiffTag->dwOrientation);
				break;			
			case TIFFTAG_SAMPLESPERPIXEL:
				pstTiffTag->dwSamplesPerPixel=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwSamplesPerPixel=%d ", pstTiffTag->dwSamplesPerPixel);
				break;
			case TIFFTAG_XRESOLUTION:
				pstTiffTag->dwXResolution=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwXResolution=%d ", pstTiffTag->dwXResolution);
				break;
			case TIFFTAG_YRESOLUTION:
				pstTiffTag->dwYResolution=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwYResolution=%d ", pstTiffTag->dwYResolution);
				break;
			case TIFFTAG_PLANARCONFIG:
				pstTiffTag->dwPlanarConfiguration=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwPlanarConfiguration=%d ", pstTiffTag->dwPlanarConfiguration);
				break;
			case TIFFTAG_RESOLUTIONUNIT:
				pstTiffTag->dwResolutionUnit=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwResolutionUnit=%d ", pstTiffTag->dwResolutionUnit);
				break;
			case TIFFTAG_COLORMAP:
				pstTiffTag->dwColorMap=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwColorMap=%d ", pstTiffTag->dwColorMap);
				break;	
			case TIFFTAG_SubIFDs:
				pstTiffTag->dwSubIFDsOffset=stTempTag.dwValue;
                MP_DEBUG1("$$TIFF dwSubIFDsOffset=0x%x",pstTiffTag->dwSubIFDsOffset);
				break;
			case TIFFTAG_JPEGIFOFFSET:
                pstTiffTag->dwJPEGIFOffset=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwJPEGIFOffset=0x%x",pstTiffTag->dwJPEGIFOffset);
				break;
			case TIFFTAG_JPEGIFBYTECOUNT:
                pstTiffTag->dwJPEGIFByteCount=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwJPEGIFByteCount=0x%x",pstTiffTag->dwJPEGIFByteCount);
				break;
	        default:
			    MP_DEBUG1("$Not used. Skip IFD TAG=0x%4x",stTempTag.wTempTAG);
   	}
   	
   }
   MP_DEBUG1("$IFD Total TAG=%d",NoOfIFDDE);
   MP_DEBUG1("TIFF_2ndIFD IFDOffset=0x%x",IFDOffset);

	//return JpegParseEnable;
}



//-------------------------------------------------------------------------------
static DWORD TIFF_GetImageInfo(IMAGEFILE *psImage)
{
	MP_DEBUG("TIFF_GetImageInfo");
	
	ST_TIFF_IFD_TAG stTempTag;
	ST_TIFF_INFO  *pstTiffTag;

	extern ST_JPEG *g_psCurJpeg;
	ST_JPEG *psJpeg;
	int iErrorCode;

	DWORD ValueOffset;//Frank Lin
	DWORD SeekNextTag;//Frank lin
	DWORD dwNextIFDOffset;//Frank Lin


	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;
	
	ST_MPX_STREAM *psStream = psImage->psStream;
	psImage->wImageWidth = 0;
	psImage->wImageHeight = 0;

	pstTiffTag = g_pstTiffTag;
	
	//image header
	mpxStreamSeek(psStream, 0, SEEK_SET);

	//type "II", 0x4949//type "MM", 0x4D4D
	WORD tmp;
	tmp=mpxStreamReadWord(psStream);
	if (tmp==IMAGE_TAG_TIFF_LITTLE)	//"II"
		pstTiffTag->dwByteOrder=IMAGE_TAG_TIFF_LITTLE;
	else if (tmp==IMAGE_TAG_TIFF_BIG)		//"MM"
		pstTiffTag->dwByteOrder=IMAGE_TAG_TIFF_BIG;

	
	MP_DEBUG("identifies the TIFF file");
	if (TIFF_Readword(psStream) != TIFF_IDENTIFICATION)	//must
		return 0;
	
	//first IFD
	DWORD dwFitrstIFDOffset=TIFF_ReadDWord(psStream);
	if (dwFitrstIFDOffset==0)
		return 0;
	MP_DEBUG1("dwFitrstIFDOffset= %d", dwFitrstIFDOffset);

	mpxStreamSeek(psStream, dwFitrstIFDOffset, SEEK_SET);
	pstTiffTag->dwDirectoryEntries = TIFF_Readword(psStream);
	MP_DEBUG1("dwDirectoryEntries= %d", pstTiffTag->dwDirectoryEntries );

	WORD ii;
	for (ii=0;ii<pstTiffTag->dwDirectoryEntries ;ii++)
	{
		stTempTag.wTempTAG	= TIFF_Readword(psStream);
		stTempTag.wValueType= TIFF_Readword(psStream);
		stTempTag.dwValueCount= TIFF_ReadDWord(psStream);
		stTempTag.dwValue= TIFF_ReadDWord(psStream);
		

		if ((pstTiffTag->dwByteOrder==IMAGE_TAG_TIFF_BIG)&&stTempTag.wValueType==3)		//3 3= SHORT 16-bit (2-byte) unsigned integer.//neil add for big endian
		{
			stTempTag.dwValue=(DWORD)(stTempTag.dwValue)>>16;
		}
		
		switch(stTempTag.wTempTAG)
		{
			case  TIFFTAG_IMAGEWIDTH:
				pstTiffTag->dwImageWidth=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwImageWidth=%d ", pstTiffTag->dwImageWidth);
				break;
			case TIFFTAG_IMAGELENGTH:
				pstTiffTag->dwImageHeight=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwImageheight=%d ", pstTiffTag->dwImageHeight);
				break;
			case TIFFTAG_BITSPERSAMPLE:
				pstTiffTag->wSampleCount=stTempTag.dwValueCount;
				MP_DEBUG1("$$TIFF wSampleCount=%d ", pstTiffTag->wSampleCount);

                 //Frank Lin add
				ValueOffset=stTempTag.dwValue;
				SeekNextTag=mpxStreamTell(psStream);
				
                MP_DEBUG1("ValueOffset=0x%x",ValueOffset);
                mpxStreamSeek(psStream, ValueOffset, SEEK_SET);
				pstTiffTag->wBitsPerSample=TIFF_Readword(psStream);
                MP_DEBUG1("wBitsPerSample=%d",pstTiffTag->wBitsPerSample);
				mpxStreamSeek(psStream, SeekNextTag, SEEK_SET);/*Seek to the next tag*/
				
				pstTiffTag->wBitsPerSample=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF wBitsPerSample=%d ", pstTiffTag->wBitsPerSample);
				break;			
			case TIFFTAG_COMPRESSION:
				pstTiffTag->dwCompression=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwCompression=%d ", pstTiffTag->dwCompression);
				break;			
			case TIFFTAG_PHOTOMETRIC:
				pstTiffTag->PhotoMetric=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF PhotoMetric=%d ", pstTiffTag->PhotoMetric);
				break;			
			case TIFFTAG_STRIPOFFSETS:
				pstTiffTag->dwStripCount=stTempTag.dwValueCount;		
				pstTiffTag->dwStripOffsets=stTempTag.dwValue;
				
				MP_DEBUG1("$$TIFF dwStripCount=%d ", pstTiffTag->dwStripCount);
				MP_DEBUG1("$$TIFF dwStripOffsets=%d ", pstTiffTag->dwStripOffsets);
				break;
			case TIFFTAG_ROWSPERSTRIP:
				pstTiffTag->dwRowPerStrip=stTempTag.dwValue;			
				MP_DEBUG1("$$TIFF dwRowPerStrip=%d ", pstTiffTag->dwRowPerStrip);
				break;
			case TIFFTAG_STRIPBYTECOUNTS:
				pstTiffTag->dwStripByteCountOffset=stTempTag.dwValue;			
				MP_DEBUG1("$$TIFF dwStripByteCountOffset=%d ", pstTiffTag->dwStripByteCountOffset);
				break;			
			case TIFFTAG_ORIENTATION:
				pstTiffTag->dwOrientation=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwOrientation=%d ", pstTiffTag->dwOrientation);
				break;			
			case TIFFTAG_SAMPLESPERPIXEL:
				pstTiffTag->dwSamplesPerPixel=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwSamplesPerPixel=%d ", pstTiffTag->dwSamplesPerPixel);
				break;
			case TIFFTAG_XRESOLUTION:
				pstTiffTag->dwXResolution=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwXResolution=%d ", pstTiffTag->dwXResolution);
				break;
			case TIFFTAG_YRESOLUTION:
				pstTiffTag->dwYResolution=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwYResolution=%d ", pstTiffTag->dwYResolution);
				break;
			case TIFFTAG_PLANARCONFIG:
				pstTiffTag->dwPlanarConfiguration=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwPlanarConfiguration=%d ", pstTiffTag->dwPlanarConfiguration);
				break;
			case TIFFTAG_RESOLUTIONUNIT:
				pstTiffTag->dwResolutionUnit=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwResolutionUnit=%d ", pstTiffTag->dwResolutionUnit);
				break;
			case TIFFTAG_COLORMAP:
				pstTiffTag->dwColorMap=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwColorMap=%d ", pstTiffTag->dwColorMap);
				break;	
			case TIFFTAG_SubIFDs:
				pstTiffTag->dwSubIFDsOffset=stTempTag.dwValue;
                MP_DEBUG1("$$TIFF dwSubIFDsOffset=0x%x",pstTiffTag->dwSubIFDsOffset);
				break;
			case TIFFTAG_JPEGIFOFFSET:
                pstTiffTag->dwJPEGIFOffset=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwJPEGIFOffset=0x%x",pstTiffTag->dwJPEGIFOffset);
				break;
			case TIFFTAG_JPEGIFBYTECOUNT:
                pstTiffTag->dwJPEGIFByteCount=stTempTag.dwValue;
				MP_DEBUG1("$$TIFF dwJPEGIFByteCount=0x%x",pstTiffTag->dwJPEGIFByteCount);
				break;
				
            case TIFFTAG_YCBCRSUBSAMPLING:
				/*stTempTag.wValueType Type=3:the size is SHORT*/
				/*stTempTag.dwValueCount count=2*/
				/*only ycbcr 422*/				
				if((stTempTag.wValueType==3) && (stTempTag.dwValueCount==2) ) 
				{
				  pstTiffTag->dwYCbCrSubsampleHorizVert= stTempTag.dwValue ;  //Frank Lin add for TIFFTAG_YCBCRSUBSAMPLING 
				  MP_DEBUG1("stTempTag.dwValue=0x%x",stTempTag.dwValue );
	           	}
				else
				{
				  MP_ALERT("TIFFTAG_YCBCRSUBSAMPLING: not support this format.");
				}
				MP_DEBUG1("$$TIFF dwYCbCrSubsampleHorizVert=0x%x",pstTiffTag->dwYCbCrSubsampleHorizVert);

				break;
				
			default:
				MP_DEBUG1("$$TIFF not use Tag dwValue=0x %x ", stTempTag.wTempTAG);
				break;	
		}

	}


   //Frank Lin add 20100923
	dwNextIFDOffset=TIFF_ReadDWord(psStream);
    MP_DEBUG1("dwNextIFDOffset=0x%x",dwNextIFDOffset);

    if(dwNextIFDOffset!=NULL)
    {
		TIFF_2ndIFD(psStream,pstTiffTag,dwNextIFDOffset);
    }



    //Frank Lin add for Tiff with jpeg 20100921
    if(pstTiffTag->dwCompression==6)/*Frank add for TIFF with JPEG decode*/
    {

	  psJpeg=g_psCurJpeg;
	  memset(psJpeg, 0, sizeof(ST_JPEG));
	  mpxStreamSeek(psStream,pstTiffTag->dwJPEGIFOffset, SEEK_SET);
      iErrorCode = Jpg_Parse_Marker(psStream, psJpeg);

	  MP_DEBUG2("-w=%d,w=%d",psJpeg->wImageWidth,psJpeg->wImageHeight);
	  
	  pstTiffTag->dwImageWidth=psJpeg->wImageWidth;//Frank Lin add
	  pstTiffTag->dwImageHeight=psJpeg->wImageHeight;//Frank Lin add


	  if(iErrorCode !=PASS)
		return 0;/*0=FAIL in this API*/
    }
 

    
	psImage->wImageWidth = pstTiffTag->dwImageWidth;
	psImage->wImageHeight =  pstTiffTag->dwImageHeight;
	MP_DEBUG2("w %d, h %d", psImage->wImageWidth, psImage->wImageHeight);
	
	return (psImage->wImageWidth << 16) | psImage->wImageHeight;
}



/*---Frank Lin add 20100920-------------------------------------------------------*/
static int TIFF_DecodeWithJPEG_Decode_Bits(IMAGEFILE *psImage, ST_TIFF_INFO  *pstTiffTag, BYTE boYield)
{
   ST_TIFF_INFO  *TiffTag;
   BYTE *pbSource;
   DWORD dwJpegBitSrcPos;
   DWORD dwJpegHeaderInfoPos;
   DWORD dwJpegByteCount;
   DWORD dwSourceSize;
   DWORD dwTargetSize;

   DWORD dwJohn;
   BYTE bMode;
   WORD w, h;

   extern ST_JPEG *g_psCurJpeg;
   ST_MPX_STREAM *psStream;
   ST_JPEG *psJpeg;

   int iErrorCode;
   iErrorCode=PASS;

   TiffTag=pstTiffTag;
   psStream=psImage->psStream;

   dwJpegHeaderInfoPos=TiffTag->dwJPEGIFOffset;
   mpxStreamSeek(psStream,dwJpegHeaderInfoPos, SEEK_SET);
   dwJohn = mpxStreamReadWord(psStream);
   MP_DEBUG1("JPEG TAG=0x%x",dwJohn);
   if(dwJohn!=0xffd8)/*JPEG Header*/
   {
      MP_ALERT("%s:Not JPEG Header Start!!", __FUNCTION__);
  	  return FAIL;
   }

   MP_DEBUG1("TiffTag->dwSubIFDsOffset=0x0%x",TiffTag->dwSubIFDsOffset);/*Maybe bitstream data*/
   MP_DEBUG1("TiffTag->dwJPEGIFOffset=0x%x",TiffTag->dwJPEGIFOffset);
   MP_DEBUG1("TiffTag->dwJPEGIFByteCount=0x%x",TiffTag->dwJPEGIFByteCount);

   psJpeg=g_psCurJpeg;
   dwJpegBitSrcPos=TiffTag->dwSubIFDsOffset;
   dwJpegByteCount=TiffTag->dwJPEGIFByteCount;
     
   psJpeg->pbTarget = psImage->pbTarget;
   psJpeg->pbSource = psStream->buffer;//dwJpegBitSrcPos;/*Frank Lin*//*psStream->buffer;*/
   psJpeg->psStream = psStream;
   psJpeg->dwSourceSize = psStream->buf_max_size;//dwJpegByteCount;/*Frank Lin*//*psStream->buf_max_size*/;
   psJpeg->dwTargetSize = psImage->dwTargetSize;
   bMode=psImage->bDecodeMode;
   psJpeg->bDecodeMode = bMode;
   psJpeg->dwOffset = psImage->dwDecodeOffset;
   psJpeg->dwCduControl = 0;

   MP_DEBUG2("W=%d,H=%d",psJpeg->wImageWidth,psJpeg->wImageHeight);
   MP_DEBUG2("img-w=%d,H=%d",psJpeg->wImageWidth,psJpeg->wImageHeight);
   MP_DEBUG2("imgRealTarge-w=%d,H=%d",psImage->wRealTargetWidth,psImage->wRealTargetHeight);


   /*----------------------------------------------*/
   pstTiffTag->dwImageWidth=psJpeg->wImageWidth;
   MP_DEBUG1("psJpeg->wImageWidth=%d",psJpeg->wImageWidth);
   pstTiffTag->dwImageHeight=psJpeg->wImageHeight;
   MP_DEBUG1("psJpeg->wImageHeight=%d",psJpeg->wImageHeight);


   /*JPEG decoder*/
   psJpeg->bDecodeMode = psImage->bDecodeMode;
   psJpeg->pbTarget = psImage->pbTarget;
   psJpeg->dwTargetSize = psImage->dwTargetSize;

   MP_DEBUG1("before cdu-- psJpeg->dwEcsStart=0x%x",psJpeg->dwEcsStart);
   MP_DEBUG1("before cdu--psJpeg->dwEoiPoint=0x%x",psJpeg->dwEoiPoint);

   SemaphoreWait(JPEG_CDU_SEMA_ID);
   iErrorCode = Jpg_Decoder_CduDecode(psJpeg);
   SemaphoreRelease(JPEG_CDU_SEMA_ID);

   MP_DEBUG1("jpg iErrorCode=%d ",iErrorCode );
   MP_DEBUG1("dwSourceSize=0x%x",psJpeg->dwSourceSize);
   MP_DEBUG1("dwTargetSize=0x%x",psJpeg->dwTargetSize);
   MP_DEBUG1("psJpeg->psStream=0x%x",psJpeg->psStream);
   MP_DEBUG1("psJpeg->bJResizeRatio=%d",psJpeg->bJResizeRatio);

   return iErrorCode;	
}

//-------------------------------------------------------------------------------
int TIFF_Decoder_DecodeImage(IMAGEFILE *psImage)
{
	ST_TIFF_INFO  *pstTiffTag;

	MP_DEBUG("TIFF_Decoder_DecodeImage");
	DWORD *pdwPal = NULL;
	int iPalSize = 0;
	ST_MPX_STREAM *psStream = psImage->psStream;
	
	BYTE *bpTarget = psImage->pbTarget;	
	BYTE boYield = ((psImage->bDecodeMode & 0x3f) == IMG_DECODE_SLIDE);
	int iErrorCode = PASS;
	WORD wBitCount;
	pstTiffTag = g_pstTiffTag;

	psImage->wImageWidth = pstTiffTag->dwImageWidth;
	psImage->wImageHeight = pstTiffTag->dwImageHeight;
	
	psImage->wRealTargetWidth = pstTiffTag->dwImageWidth>> psImage->bScaleDown;
	MP_DEBUG1("psImage->bScaleDown=%d",psImage->bScaleDown);
	psImage->wRealTargetHeight =pstTiffTag->dwImageHeight >> psImage->bScaleDown;	
	
	psImage->wTargetWidth = ALIGN_16(psImage->wRealTargetWidth);	
	psImage->wTargetHeight = (psImage->wRealTargetHeight);
	
	psImage->wThumbWidth = psImage->wRealTargetWidth;
	psImage->wThumbHeight = psImage->wRealTargetHeight;
	
	if ((psImage->wImageWidth < IMAGE_MIN_WIDTH) || (psImage->wImageHeight< IMAGE_MIN_HEIGHT))
		return NOT_SUPPORTED_TIFF_SIZE;
	
	if ((psImage->wTargetWidth * psImage->wRealTargetHeight * 2) > psImage->dwTargetSize)
		return NOT_SUPPORTED_TIFF_SIZE;

	if(pstTiffTag->dwCompression==32773)//TYChen Add Packbits Compression
	{
          ;	
	/*		
		if(wScale>1)
			return NOT_SUPPROTED_TIFF;*/
	} 
	else if (pstTiffTag->dwCompression >1 ||pstTiffTag->dwPlanarConfiguration==PLANARCONFIG_SEPARATE )
		{
		      
              if(pstTiffTag->PhotoMetric==PHOTOMETRIC_RGB && pstTiffTag->dwCompression == 1)
              {
               /*Frank Lin add: PhotoMetric=RGB format ,PlanarConfiguration= Planar format(2)*/
			   /*later do it*/ 
			   ;
			   MP_DEBUG("PHOTOMETRIC_RGB ,PLANARCONFIG_SEPARATE and Uncompressed "); 
              }
              else if(pstTiffTag->dwCompression==6)/*Frank add for JPEG decode*/
			  {
			  	
				iErrorCode=TIFF_DecodeWithJPEG_Decode_Bits(psImage, pstTiffTag, boYield);
                MP_DEBUG1("fff-iErrorCode=%d",iErrorCode);
				return iErrorCode;
			  }
			  else
			  {
              	MP_DEBUG("TIFF not supported.");
		        return NOT_SUPPROTED_TIFF;
			  }
		}

	if  (pstTiffTag->wSampleCount==1)		//Bileve Image ,Grayscale image ,Palette Image
	{
		iPalSize = 1 << pstTiffTag->wBitsPerSample;
		iPalSize <<= 2;
		
        pdwPal = (DWORD *)ext_mem_malloc(iPalSize + 256);
		if (pdwPal == NULL) return NULL;
		
		if (pstTiffTag->wBitsPerSample==1)
		{
			wBitCount=1;
			
			if(pstTiffTag->PhotoMetric==1)		//BlackIsZero. For bilevel and grayscale images: 0 is imaged as black.
			{
				pdwPal[0]= RGB_2_YUV(0, 0, 0);
				pdwPal[1]= RGB_2_YUV(0xff, 0xff, 0xff);
			}
			else				//WhiteIsZero. For bilevel and grayscale images: 0 is imaged as white.
			{
				pdwPal[0]= RGB_2_YUV(0xff, 0xff, 0xff);
				pdwPal[1]= RGB_2_YUV(0, 0, 0);	
			}
			
		}
		else if(pstTiffTag->wBitsPerSample==8)
		{
			wBitCount=8;
			
			if (pstTiffTag->dwColorMap>0)
			{
				int i;
				BYTE bred[256],bGreen[256],bBlue[256];
				
				iPalSize >>= 2;
				mpxStreamSeek(psStream, pstTiffTag->dwColorMap, SEEK_SET);

				for (i = 0; i < iPalSize; i++)
				{
					bred[i]= mpxStreamReadWord(psStream)&0x00ff;
					MP_DEBUG1("$$$red = %x", bred[i]);
				}
				for (i = 0; i < iPalSize; i++)
				{
					bGreen[i]= mpxStreamReadWord(psStream) &0x00FF;
					MP_DEBUG1("$$$red = %x", bred[i]);
				}
				for (i = 0; i < iPalSize; i++)
				{
					bBlue[i]= mpxStreamReadWord(psStream) &0x00FF;
					MP_DEBUG1("$$$red = %x", bred[i]);
				}
				
				for (i = 0; i < iPalSize; i++)
				{
					pdwPal[i] = RGB_2_YUV(bred[i],bGreen[i],bBlue[i]);
				}
			
			}

		}

	}
	else if (pstTiffTag->wSampleCount==3)		//RGB
	{

		WORD wsampleLenght;
		mpxStreamSeek(psStream, pstTiffTag->wBitsPerSample, SEEK_SET);
		wsampleLenght = TIFF_Readword(psStream);
	
		if (wsampleLenght==0x0010)		//16bit
			return NOT_SUPPROTED_TIFF;
	
		if (pstTiffTag->dwStripCount==1)		//only one strip, later do it 
		{
			wBitCount=24;			
		}
		else								//Multi-Strip
		{
			int ii;
			DWORD *dwstripOffset;
			DWORD *dwStripByteCount;
			
			
            dwstripOffset = (DWORD *)ext_mem_malloc(pstTiffTag->dwStripCount*4 + 256);
			dwStripByteCount = (DWORD *)ext_mem_malloc(pstTiffTag->dwStripCount*4 + 256);
						
			mpxStreamSeek(psStream, pstTiffTag->dwStripOffsets, SEEK_SET);
			for (ii=0;ii<pstTiffTag->dwStripCount;ii++)
			{
				dwstripOffset[ii]=TIFF_ReadDWord(psStream);

				MP_DEBUG1("dwstripOffset[ii]=%8x", dwstripOffset[ii]);
			}
			
			if (boYield) TaskYield();
			
			mpxStreamSeek(psStream, pstTiffTag->dwStripByteCountOffset, SEEK_SET);
			for (ii=0;ii<pstTiffTag->dwStripCount;ii++)
			{
				dwStripByteCount[ii]=TIFF_ReadDWord(psStream);
			}
			
			if (boYield) TaskYield();

            if(pstTiffTag->PhotoMetric==6) /*YCbCr*/
            {         
			  iErrorCode =TIFF_YCbCr_Decode_MultiStrip_Bits(psImage, dwstripOffset,pstTiffTag->dwRowPerStrip, dwStripByteCount, boYield,pstTiffTag);
            }
			else
			{
			  if(pstTiffTag->dwPlanarConfiguration==PLANARCONFIG_SEPARATE)
			  {
			    iErrorCode =TIFF_Decode_SEPARATE_MultiStrip_Bits(psImage, dwstripOffset,pstTiffTag->dwRowPerStrip, dwStripByteCount, boYield,pstTiffTag);
			  }
			  else
			  {
		        iErrorCode =TIFF_Decode_MultiStrip_Bits(psImage, dwstripOffset,pstTiffTag->dwRowPerStrip, dwStripByteCount, boYield);
			  }
			}
  
			
			if (dwstripOffset) 		ext_mem_free(dwstripOffset);
			if (dwStripByteCount) 	ext_mem_free(dwStripByteCount);
			return iErrorCode;	
		
		}
			
	}
	else
	{
		return NOT_SUPPROTED_TIFF;
	}
		
	mpxStreamSeek(psStream, pstTiffTag->dwStripOffsets, SEEK_SET);
    if(pstTiffTag->PhotoMetric==6) /*YCbCr*/
	  iErrorCode=TIFF_YCbCr_Decode_Bits(psImage, pdwPal, wBitCount, boYield);
    else
	  iErrorCode =TIFF_Decode_Bits(psImage, pdwPal, wBitCount, boYield);

   
	
	if (pdwPal) 		ext_mem_free(pdwPal);
	return iErrorCode;	
}

//-------------------------------------------------------------------------------
int TIFF_Decoder_Init(IMAGEFILE *psImage)
{
	MP_ASSERT(psImage != NULL);
	
	MP_DEBUG("TIFF_Decoder_Init");
	g_pstTiffTag = &g_stTiffTag;
	
	memset(g_pstTiffTag,0,sizeof(ST_TIFF_INFO));
	
	psImage->ImageDecodeThumb =TIFF_Decoder_DecodeImage;
	psImage->ImageDecodeImage = TIFF_Decoder_DecodeImage;
			
	if (TIFF_GetImageInfo(psImage) != 0)
		return PASS;
	else
		return FAIL;
}
#endif
