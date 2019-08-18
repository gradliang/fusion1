/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : usbotg_host_web_cam.c
* Programmer(s) : Morse Chen
* Created       : 2008-06-25
* Descriptions  :
*******************************************************************************
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

#include "usbotg_api.h"
#include "usbotg_ctrl.h"
#include "Usbotg_host_sidc.h"
#include "Usbotg_host_setup.h"

#include "taskid.h"
#include "os.h"
#include "ui.h"

#if ((SC_USBHOST && USBOTG_WEB_CAM) && (USB_HOST_ISO_TEST==DISABLE))
//#define WEBCAM_SAVE 0
//#define WEBCAM_PLAY 1
#define ENDOSCOPE_WEBCAM 1

static void SetWebcamState(WEBCAM_STATE eState, WHICH_OTG eWhichOtg);
WEBCAM_STATE GetWebcamState(WHICH_OTG eWhichOtg);

/*
//////////////////// WebCam API for FAE ////////////////////////
void Api_UsbhWebCamSetBrightness(WORD brightness, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	BYTE cmdData[] = {0x00, 0x00};

	brightness %= 8;
	mpDebugPrint("brightness %x", brightness);
	cmdData[1] = ((BYTE *) &brightness)[0];
	cmdData[0] = ((BYTE *) &brightness)[1];
	//webCamSetupSetCmd(cmdData, USB_RECIP_INTERFACE, 1, 0x200, 0x300, 2, eWhichOtg);
    webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                        SET_CUR,\
                        VS_COMMIT_CONTROL,\
                        pUsbhDevDes->bAppInterfaceNumber,\
                        2,\
                        eWhichOtg);
	TaskSleep(200);
}
void Api_UsbhWebCamSetContrast(WORD contrast, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	BYTE cmdData[] = {0x00, 0x00};

	contrast %= 256;
	mpDebugPrint("contrast %x", contrast);
	cmdData[1] = ((BYTE *) &contrast)[0];
	cmdData[0] = ((BYTE *) &contrast)[1];
	//webCamSetupSetCmd(cmdData, USB_RECIP_INTERFACE, 1, 0x300, 0x300, 2, eWhichOtg);
    webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                        SET_CUR,\
                        VS_STILL_PROBE_CONTROL,\
                        pUsbhDevDes->bAppInterfaceNumber,\
                        2,\
                        eWhichOtg);
	TaskSleep(200);
}

void Api_UsbhWebCamSetGamma(WORD gamma, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	BYTE cmdData[] = {0x00, 0x00};

	gamma %= 256;
	mpDebugPrint("gamma %x", gamma);
	cmdData[1] = ((BYTE *) &gamma)[0];
	cmdData[0] = ((BYTE *) &gamma)[1];
	//webCamSetupSetCmd(cmdData, 1, 0x900, 0x300, 2, eWhichOtg);
    webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                        SET_CUR,\
                        VS_SYNCH_DELAY_CONTROL,\
                        pUsbhDevDes->bAppInterfaceNumber,\
                        2,\
                        eWhichOtg);
	TaskSleep(200);
}
*/
/////////////////////////// ISO ////////////////////////////////
#define OTGH_MAX_PACKET_SIZE			0x400
#define HOST20_iTD_Status_Active		0x08
#define OTGH_Dir_IN						0x01
#define OTGH_Dir_Out					0x00

#if 0
typedef struct{
	BYTE *FramBuffer;	// Filled by AP task
	DWORD size;			// filled by USBOTG task
} WebCamAudioFram;
//static void OTGH_PT_ISO_IN(UINT32 wEndPt,UINT32 wSize, UINT32 wMaxPacketSize,WHICH_OTG eWhichOtg);
#endif

void webCamiTDAlloc(WHICH_OTG eWhichOtg)
{
	DWORD index;
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) != 0)
	{
		for (index = 0; index < PERIODIC_FRAME_SIZE; ++index)
			psUsbHostAVdc->dwAudioFrameList[index]=flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
	}
	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) != 0)
	{
		for (index = 0; index < PERIODIC_FRAME_SIZE; ++index)
		{
			psUsbHostAVdc->dwVideoFrameList[index]=flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
			//mpDebugPrint("psUsbHostAVdc->dwVideoFrameList[index] %x", psUsbHostAVdc->dwVideoFrameList[index]);
			//mpDebugPrint("vfl %x", psUsbHostAVdc->dwVideoFrameList[index]);
			//((iTD_Structure *)psUsbHostAVdc->dwAudioFrameList[index])->bNextLinkPointer = psUsbHostAVdc->dwVideoFrameList[index]>>5;
			//((iTD_Structure *)psUsbHostAVdc->dwAudioFrameList[index])->bType            = HOST20_HD_Type_iTD; 
			//((iTD_Structure *)psUsbHostAVdc->dwAudioFrameList[index])->bTerminate       = 0;
		}
	}
}

void webCamiTDFree(WHICH_OTG eWhichOtg)
{
	DWORD index;
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) != 0)
	{
		for (index = 0; index < PERIODIC_FRAME_SIZE; ++index)
			flib_Host20_ReleaseStructure(Host20_MEM_TYPE_iTD,psUsbHostAVdc->dwVideoFrameList[index], eWhichOtg);
	}
	if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) != 0)
	{
		for (index = 0; index < PERIODIC_FRAME_SIZE; ++index)
			flib_Host20_ReleaseStructure(Host20_MEM_TYPE_iTD,psUsbHostAVdc->dwAudioFrameList[index], eWhichOtg);
	}
}


#if 0
BOOL WebCamGetAudioBuffer(WebCamAudioBuffer *pAudioBuffer, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	if (psUsbHostAVdc->boVideoStreamInit == TRUE)
	{
		pAudioBuffer->BufferBeginAdd 	= (DWORD)psUsbHostAVdc->sAudioStream.pbAudioBuffer[0];
		pAudioBuffer->BufferSize		= psUsbHostAVdc->dwAudioBufferSize;
		pAudioBuffer->BufferNumber		= AUDIO_NUM_BUF;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL WebCamGetAudioStatus(BYTE index, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->sAudioStream.dwBufferActive[index] ? TRUE : FALSE;	
}

void WebCamClearAudioStatus(BYTE index, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	psUsbHostAVdc->sAudioStream.dwBufferActive[index] = FALSE;	
}

DWORD WebCamGetAudioStreamLength(BYTE index, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	//MP_DEBUG("WebCamGetAudioStreamLength");
	return psUsbHostAVdc->sAudioStream.dwBufferLength[index];
}
#endif

#if USBCAM_TECH_MODE
#define FRAME_RATE_TEST					1
#if FRAME_RATE_TEST
static DWORD g_dwPlayStartTime=0,g_dwFramCnt=0;
#endif
#if USBCAM_DEBUG_ISR_LOST
static DWORD st_dwDecodeCnt=0,st_dwDecordStartTime,st_dwDataOutTime[2], st_dwIsocDataOut=0;
//extern DWORD g_dwQueueUsed;
#endif
#if USBCAM_TMP_SHARE_BUFFER
extern BYTE *g_pbTmpPicBuffer;
extern DWORD g_dwPicBufferSize;
#endif
int g_print_led = 0;


#if CHASE_IN_DECODE
BYTE g_bNeedChase=0;
#endif
BYTE g_bFreezeUsbCamDisplay=0;
static BYTE st_bUsbCamPreStop=0;

#define SWAP_UY(a)	((a & 0xff0000ff)|((a & 0xff00)<<8)|((a & 0xff0000)>>8))

void SetUsbCamPreStop()
{
	st_bUsbCamPreStop=1;
}

#if CHASE_IN_DECODE
BYTE GetNeedChase()
{
	return g_bNeedChase;
}
#endif
void CopyYuvData(DWORD *dst, DWORD *src, DWORD len)
{
	len>>2;
	while(len--)
	{
		*dst = SWAP_UY(*src);//*src++;
		dst++;
		src++;
	}
}

void bCopyYuvData(BYTE *dst, BYTE *src, DWORD len)
{
	DWORD i;

	for (i=0;i<len;i+=4)
	{
		dst[0]=src[0];
		dst[1]=src[2];
		dst[2]=src[1];
		dst[3]=src[3];
		dst+=4;
		src+=4;
	}
}

void YUYVWinVirtualScaleDown( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin,WORD wDx,WORD wDy,WORD wDw,WORD wDh)
{
	register DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	register DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;
	register DWORD dwSrcOffset, dwDstOffset,i,k;
	WORD wWidth,wHeight;

	dwSrcOffset=pSrcWin->dwOffset>>2;
	dwDstOffset=pDstWin->dwOffset>>2;
	wHeight=wDh;
	wWidth=wDw>>1;

	pdwDstBuffer+=(wDx>>1);
	pdwDstBuffer+=dwDstOffset*wDy;
	for (i=0;i<wHeight;i++)
	{
		for (k=0;k<wWidth;k++)
		{
			pdwDstBuffer[k]=SWAP_UY(pdwSrcBuffer[k*(pSrcWin->wWidth/wDw)]);
		}
		pdwDstBuffer+=dwDstOffset;
		pdwSrcBuffer+=(dwSrcOffset*(pSrcWin->wHeight/wDh));
	}
}

void YUYVWinCopy( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin,WORD wDx,WORD wDy,WORD wDw,WORD wDh)
{
	WORD w,h;
	DWORD dwSrcOffset,dwTrgOffset;
	DWORD *pdwSrcBuffer,*pdwTrgBuffer;

	dwSrcOffset=pSrcWin->dwOffset>>2;
	pdwSrcBuffer=pSrcWin->pdwStart;
	dwTrgOffset=pDstWin->dwOffset>>2;
	pdwTrgBuffer=pDstWin->pdwStart+(wDx>>1)+wDy*dwTrgOffset;
	wDw>>=1;
	for (h=0;h<wDh;h++)
	{
		for (w=0;w<wDw;w++)
		{
			pdwTrgBuffer[w]=SWAP_UY(pdwSrcBuffer[w]);
		}
		pdwSrcBuffer+=dwSrcOffset;
		pdwTrgBuffer+=dwTrgOffset;
	}

}

static int ImageAdhocDraw_Decode(BYTE *image_buf,DWORD image_size)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(WEBCAM_USB_PORT);
	ST_IMGWIN  *pTrgWin = (ST_IMGWIN  *)Idu_GetNextWin(),stSrcWin,stDstWin;
	SWORD swRet=PASS;
	DWORD *pdwImageBuf,dwImageOffset,dwImageHeight,i,w,h,*pdwWinBuf,dwWinOffset,dwWinHeight,dwOffset;
	BYTE *pbImageBuf,*pbWinBuf;
	register BYTE bTmp;

	//UartOutText("-D-");

#if USBCAM_DEBUG_ISR_LOST
	st_dwDecodeCnt++;
	if ((st_dwDecodeCnt%100)==0)
		st_dwDecordStartTime=GetSysTime();
#endif

	if (psUsbHostAVdc->eVideoFormat == USING_YUV)
	{
		if (!g_bFreezeUsbCamDisplay)
		{
			if (image_size != (psUsbHostAVdc->dwVideoFrameSize>>16)*(psUsbHostAVdc->dwVideoFrameSize&0x0000ffff)<<1)
			{
				UartOutText("x");
				return PASS;
			}
		#if (USBCAM_IN_ENABLE==2)
			UartOutText(".");
		return PASS;
		#if 0
			pdwWinBuf=pTrgWin->pdwStart;//(BYTE *) ((DWORD) pTrgWin->pdwStart | 0xa0000000);//
			dwWinHeight=pTrgWin->wHeight;
			DWORD dwWidth=psUsbHostAVdc->dwVideoFrameSize>>17;
			dwWinOffset=pTrgWin->dwOffset>>2;
			for (h=0;h<dwWinHeight;h++)
			{
				for (w=0;w<dwWidth;w++)
				{
					//bTmp=pbWinBuf[w+1];
					//pbWinBuf[w+1]=pbWinBuf[w+2];
					//pbWinBuf[w+2]=bTmp;
					pdwWinBuf[w]=SWAP_UY(pdwWinBuf[w]);
				}
				pbWinBuf+=dwWinOffset;
			}
		#endif
		#elif (USBCAM_IN_ENABLE==1)
			/*
			stSrcWin.pdwStart=(DWORD *)image_buf;
			stSrcWin.wWidth=psUsbHostAVdc->dwVideoFrameSize>>16;
			stSrcWin.wHeight=psUsbHostAVdc->dwVideoFrameSize&0x0000ffff;
			stSrcWin.dwOffset=stSrcWin.wWidth<<1;
			w=MIN(stSrcWin.wWidth,pTrgWin->wWidth);
			h=MIN(stSrcWin.wHeight,pTrgWin->wHeight);
			YUYVWinCopy(&stSrcWin,pTrgWin,0,0,w,h);
			*/

			#if 0
			dwImageOffset=psUsbHostAVdc->dwVideoFrameSize>>16<<1;
			pbImageBuf=image_buf;//(BYTE *) ((DWORD) image_buf | 0xa0000000);//
			pbWinBuf=(BYTE *)pTrgWin->pdwStart;//(BYTE *) ((DWORD) pTrgWin->pdwStart | 0xa0000000);//
			dwWinOffset=pTrgWin->dwOffset;
			dwWinHeight=MIN(pTrgWin->wHeight,psUsbHostAVdc->dwVideoFrameSize&0x0000ffff);
			dwOffset=MIN(dwWinOffset,dwImageOffset);
    		//mmcp_block_polling(pbImageBuf, pbWinBuf,dwWinHeight, dwOffset,dwImageOffset, dwWinOffset);
			for (h=0;h<dwWinHeight;h++)
			{
				//mmcp_memcpy_polling(pbWinBuf,pbImageBuf,dwOffset);
				for (w=0;w<dwOffset;w+=4)
				{
					pbWinBuf[w]=pbImageBuf[w];
					pbWinBuf[w+1]=pbImageBuf[w+2];
					pbWinBuf[w+2]=pbImageBuf[w+1];
					pbWinBuf[w+3]=pbImageBuf[w+3];
				}
				pbWinBuf+=dwWinOffset;
				pbImageBuf+=dwImageOffset;
			}
			#else
			pbImageBuf=(BYTE *) ((DWORD) image_buf | 0xa0000000);
			for (w=0;w<image_size;w+=4,pbImageBuf+=4)
			{
				bTmp=pbImageBuf[1];
				pbImageBuf[1]=pbImageBuf[2];
				pbImageBuf[2]=bTmp;
			}
			stSrcWin.pdwStart=(DWORD *)((DWORD) image_buf | 0xa0000000);
			stSrcWin.wWidth=psUsbHostAVdc->dwVideoFrameSize>>16;
			stSrcWin.wHeight=psUsbHostAVdc->dwVideoFrameSize&0x0000ffff;
			stSrcWin.dwOffset=stSrcWin.wWidth<<1;
			w=MIN(stSrcWin.wWidth,pTrgWin->wWidth);
			h=MIN(stSrcWin.wHeight,pTrgWin->wHeight);
			//YUYVWinCopy(&stSrcWin,pTrgWin,0,0,w,h);
			//mpCopyWin(pTrgWin, &stSrcWin);
			Ipu_ImageScaling(&stSrcWin, pTrgWin, 0, 0, w, h, 0, 0, w, h, 0);
			//mpCopyWinAreaSameSize(ST_IMGWIN * pSrcWin, ST_IMGWIN * pDstWin, WORD wSx, WORD wSy, WORD wDx, WORD wDy, WORD wW, WORD wH)
			#endif
		#else
			pdwImageBuf= (DWORD *)image_buf;
			//dwImageSize=image_size>>2;
			dwImageOffset=psUsbHostAVdc->dwVideoFrameSize>>16>>1;
			dwImageHeight=psUsbHostAVdc->dwVideoFrameSize&0x0000ffff;
			//mpDebugPrint("image_size=%d dwImageOffset=%d dwImageHeight=%d",image_size,dwImageOffset,dwImageHeight);

			pbImageBuf=image_buf;
			for (w=0;w<image_size;w+=4,pbImageBuf+=4)
			{
				bTmp=pbImageBuf[1];
				pbImageBuf[1]=pbImageBuf[2];
				pbImageBuf[2]=bTmp;
			}
			pdwWinBuf=pTrgWin->pdwStart;
			dwWinOffset=pTrgWin->dwOffset>>2;
			dwWinHeight=pTrgWin->wHeight;
			for (h=0;h<dwWinHeight&&h<dwImageHeight;h++)
			{
				mmcp_memcpy_polling(pdwWinBuf,pdwImageBuf,dwImageOffset<<2);
				//memcpy((BYTE *)pdwWinBuf,(BYTE *)pdwImageBuf,dwImageOffset<<2);
				/*
				for (w=0;w<dwWinOffset&&w<dwImageOffset;w++)
				{
					pdwWinBuf[w]=SWAP_UY(pdwImageBuf[w]);
				}
				*/
				pdwWinBuf+=dwWinOffset;
				pdwImageBuf+=dwImageOffset;
			}
		#endif
			Idu_ChgWin(Idu_GetNextWin());
			//mpDebugPrint("%d,%d",image_size,g_dwFramCnt);
		}
	}
	else
	{
		if (!g_bFreezeUsbCamDisplay)
		{
			if (g_pbTmpPicBuffer)
			{
				ST_IMGWIN  *pDecodeWin=(ST_IMGWIN  *)Idu_GetUsbCamCacheWin();
				swRet = Img_Jpeg2ImgBuf(image_buf, (BYTE *)pDecodeWin->pdwStart, 0, image_size, pDecodeWin->dwOffset*pDecodeWin->wHeight);
			#if USBCAM_DEBUG_ISR_LOST
				if ((st_dwDecodeCnt%100)==0)
				{
					UartOutText("\r\n");
					mpDebugPrintN("%d->",SystemGetElapsedTime(st_dwDecordStartTime));
				}
			#endif
				//mpCopyWinAreaSameSize(pDecodeWin, pTrgWin, 0, 0,0,0,pTrgWin->wWidth, pTrgWin->wHeight);
				//WinVirtualScaleDown(pDecodeWin, pTrgWin, 0,0,pTrgWin->wWidth, pTrgWin->wHeight);
				myCopyWinPartSameArea(pDecodeWin, pTrgWin);
				//mpCopyWin(pDecodeWin, pTrgWin);
				//Ipu_ImageScaling(pDecodeWin, pTrgWin, 0, 0, pTrgWin->wWidth, pTrgWin->wHeight, 0, 0, pTrgWin->wWidth, pTrgWin->wHeight, 0);
			}
			else
				swRet = Img_Jpeg2ImgBuf(image_buf, (BYTE *)pTrgWin->pdwStart, 0, image_size, pTrgWin->dwOffset*pTrgWin->wHeight);
		}
#if 0
		else
		{
		// decode jpeg role
			swRet = Img_Jpeg2ImgBuf(image_buf, (BYTE *)g_pbTmpPicBuffer, 0, image_size, g_dwPicBufferSize);
			if (swRet==PASS)
			{
				ST_IMGWIN sSrcWin;
				sSrcWin.pdwStart = (DWORD*)g_pbTmpPicBuffer;
				sSrcWin.dwOffset = Img_JpegGetCurWidth() << 1;
				sSrcWin.wHeight = Img_JpegGetCurHeight();
				sSrcWin.wWidth = Img_JpegGetCurWidth();

#if USBCAM_DEBUG_ISR_LOST
	if ((st_dwDecodeCnt%100)==0)
	{
		mpDebugPrintN(">%d>",SystemGetElapsedTime(st_dwDecordStartTime));
	}
#endif
				//mpCopyWinAreaSameSize(&sSrcWin, pTrgWin, 0, 0,0,0,pTrgWin->wWidth, pTrgWin->wHeight);
				Ipu_ImageScaling(&sSrcWin, pTrgWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, 0, 0, pTrgWin->wWidth, pTrgWin->wHeight, 0);
				//Ipu_Video_Bypass_Scaling(&sSrcWin, pTrgWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, 0, 0, pTrgWin->wWidth, pTrgWin->wHeight, 0);
			}
			else
			{
				swRet = Img_Jpeg2ImgBuf(image_buf, (BYTE *)pTrgWin->pdwStart, 0, image_size, pTrgWin->dwOffset*pTrgWin->wHeight);
			}
		}
#endif

		if (g_print_led == 1)
		{
			mpDebugPrint("%s:%d:buf 0x%X size %d", __func__, __LINE__, image_buf, image_size);
			mpDebugPrint("%d X %d,swRet=%d",Img_JpegGetCurWidth(),Img_JpegGetCurHeight(),swRet);
			g_print_led = 0;
		}
		//mpDebugPrint("%d X %d,%d,%d",Img_JpegGetCurWidth(),Img_JpegGetCurHeight(),image_size,g_dwFramCnt);

		if (swRet != PASS)
		{
			MP_ALERT("%s:%d:Jpeg decode fail %d", __func__, __LINE__, swRet);
			return PASS;
		}
#if USBCAM_DEBUG_ISR_LOST
	if ((st_dwDecodeCnt%100)==0)
	{
		//UartOutText("\r\n");
		mpDebugPrintN("decode %d:%d",st_dwDecodeCnt,SystemGetElapsedTime(st_dwDecordStartTime));
	}
#endif

		if (!g_bFreezeUsbCamDisplay)
			Idu_ChgWin(Idu_GetNextWin());
		//mpDebugPrint("CNT%d",g_dwFramCnt);
		WebCamCapture(image_buf,image_size);
#if ONE_WINDOW_REC_VER
		RecordWebCamFillBuffer(image_buf,image_size);
		//RecordWebCamProcess(image_buf,Idu_GetCurrWin());
#endif
	}
	
#if FRAME_RATE_TEST
	if (!g_dwPlayStartTime)
	{
		g_dwPlayStartTime=GetSysTime();
		g_dwFramCnt=1;
	}
	else
	{
		
		if (SystemGetElapsedTime(g_dwPlayStartTime)>=10000)
		{
			//mpDebugPrint("%d X %d,%d",Img_JpegGetCurWidth(),Img_JpegGetCurHeight(),g_dwFramCnt);
			g_dwPlayStartTime=GetSysTime();
			g_dwFramCnt=0;
		}
		g_dwFramCnt++;
	}
#endif
#if USBCAM_DEBUG_ISR_LOST
	if ((st_dwDecodeCnt%100)==0)
	{
		mpDebugPrintN("->%d",SystemGetElapsedTime(st_dwDecordStartTime));
		if (st_dwDecodeCnt==st_dwDataOutTime[0])
			mpDebugPrint("-->%d",SystemGetElapsedTime(st_dwDataOutTime[1]));
		else
		{
			st_dwIsocDataOut=0;
			st_dwDecodeCnt=0;
		}
		UartOutText("\r\n");
	}
#endif

	return PASS;

}

#else

//extern BYTE *g_pTemp;
//DWORD g_index = 0;
//int g_ttt = 0;
//#define TEMP_BUFFER_CNT 3
#define TEMP_BUFFER_SIZE (640*480*2)
#define NETSTREAM_MAX_BUFSIZE TEMP_BUFFER_SIZE // 512*1024
#define WEBCAM_SAVE_MJPEG 0
int g_test_res = 0;
int g_print_led = 0;
static int ImageAdhocDraw_Decode(BYTE *image_buf,DWORD image_size)
{
	ST_IMGWIN * pWin = Idu_GetCurrWin();
	BYTE *pbAllocBuf = NULL;
	BYTE *pImgBuf = NULL;
	int swRet = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(WEBCAM_USB_PORT);
    //pUsbhDev->dwQueueElementByteCount = dwLength;

	if (pWin==NULL)
	{
		mpDebugPrint("pWin==NULL");
		return NULL;
	}

    if (g_test_res == 1 || g_print_led == 1)
    {
        mpDebugPrint("%s:%d:buf 0x%X size %d", __func__, __LINE__, image_buf, image_size);
        DumpBuffer(image_buf ,256);
        DumpBuffer(image_buf+(image_size-256) ,256);
        g_print_led = 0;
    
        mpDebugPrint("%s:%d:save file with the buf 0x%X and size %d", __func__, __LINE__, image_buf, image_size);
#if 0//WEBCAM_SAVE_MJPEG
    {
        DRIVE  *sDrv = NULL;
        STREAM *shandle = NULL;
        DWORD ret;
        
        sDrv = DriveChange(SD_MMC);
        ret  = CreateFile(sDrv, "test", "jpg");
        if (!ret)
            shandle = FileOpen(sDrv);

        if (shandle != NULL)
        {
            ret = FileWrite(shandle, image_buf, image_size);
            FileClose(shandle);
            if (ret > 0)
                mpDebugPrint("%s:%d:saving file ready. sDrv 0x%x shandle 0x%x ret %d!!", __func__, __LINE__, \
                             sDrv, shandle, ret);
            else
                mpDebugPrint("%s:%d:saving file failed with FileWrite!!", __func__, __LINE__);
        }
        else
        {
            mpDebugPrint("%s:%d:open file failed!!", __func__, __LINE__);
        }
    }
#else
        mpDebugPrint("%s:%d:after saving file then return here!!", __func__, __LINE__);
#endif

//__asm("break 100");
     //   return PASS;
    }
// allocate buffer to decode image
	pbAllocBuf = (BYTE *)ext_mem_malloc(pUsbhDev->dwQueueElementByteCount);
	if (pbAllocBuf == NULL)
	{
		MP_ALERT("pbAllocBuf size %d > buffer alloc fail", pUsbhDev->dwQueueElementByteCount);
		return NULL;
	}

	(DWORD *)pImgBuf = (DWORD *) ((DWORD) pbAllocBuf | 0x20000000);

// decode jpeg role
	swRet = Img_Jpeg2ImgBuf(image_buf, (BYTE *)pImgBuf, 0, image_size, pUsbhDev->dwQueueElementByteCount);
	if (swRet != 0)
	{
		MP_ALERT("%s:%d:Jpeg decode fail %d", __func__, __LINE__, swRet);
		if (pbAllocBuf) ext_mem_freeEx(pbAllocBuf, __FILE__, __LINE__);
		return FAIL;
	}
#if 0
    if (0)
    {   if (g_ttt)
        __asm("break 100");
        
        //UartOutText("<");
        //mmcp_memcpy_polling((BYTE*)((DWORD)g_pTemp), (BYTE *)pImgBuf, TEMP_BUFFER_SIZE);
        //memcpy((BYTE*)((DWORD)g_pTemp), (BYTE *)pImgBuf, TEMP_BUFFER_SIZE);
        //memcpy((BYTE*)((DWORD)g_pTemp+g_index), (BYTE *)pImgBuf, TEMP_BUFFER_SIZE);
        //g_index = ((g_index/TEMP_BUFFER_SIZE) >= TEMP_BUFFER_CNT)? 0 : (g_index+TEMP_BUFFER_SIZE);
        //UartOutValue(g_index, 5);
    }
#endif

	WORD wRawWidth  = Img_JpegGetCurWidth();
	WORD wRawHeight = Img_JpegGetCurHeight();
	WORD x = 0;
	WORD y = 0;
	WORD w = 640;//320; // TODO - why the data get wrong ?!
	WORD h = 480;//240;// TODO - why the data get wrong ?!
	ST_IMGWIN sSrcWin;
	sSrcWin.pdwStart = (DWORD*)pImgBuf;
	sSrcWin.dwOffset = wRawWidth << 1;
	sSrcWin.wHeight = wRawHeight;  // simulate pstRole->m_wRawHeight
	sSrcWin.wWidth = wRawWidth;

    //UartOutValue(wRawWidth, 3);
    //UartOutValue(wRawHeight, 3);
    //UartOutText(">");
// Do Scaling & Show to verify
    //Ipu_Video_Bypass_Scaling(&sSrcWin, pWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, x, y, x+w, y+h, 0);

    Ipu_ImageScaling(&sSrcWin, pWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, x, y, x+w, y+h, 0);
	if (pbAllocBuf) ext_mem_freeEx(pbAllocBuf, __FILE__, __LINE__);
    MP_DEBUG("%s:%d:End", __func__, __LINE__);
	return PASS;

}
#endif

#if 0
static void webCamUsbOtgHostIsoActive (WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD div = psUsbHostAVdc->psDeviceAp->bDeviceSpeed == 0 ? 32 : 8;

	MP_DEBUG("webCamUsbOtgHostIsoActive");

	if (psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[0] == FALSE && webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) != 0)
		OTGH_PT_ISO_IN (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg),
		                psUsbHostAVdc->dwMjpgBufferSize/div,
		                webCamVideoGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg),
		                eWhichOtg);
	if (psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[1] == FALSE && webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) != 0)
		OTGH_PT_ISO_IN (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg),
		                psUsbHostAVdc->dwMjpgBufferSize/div,
		                webCamVideoGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg),
		                eWhichOtg);

	if (psUsbHostAVdc->sAudioStream.dwIsoInActive[0] == FALSE && webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg))
		OTGH_PT_ISO_IN (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg),
		                psUsbHostAVdc->dwAudioBufferSize,
		                webCamAudioGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg),
		                eWhichOtg);
	if (psUsbHostAVdc->sAudioStream.dwIsoInActive[1] == FALSE && webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg))
		OTGH_PT_ISO_IN (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg),
		                psUsbHostAVdc->dwAudioBufferSize,
		                webCamAudioGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg),
		                eWhichOtg);
}

static void webCamVideoStreamParseFrame(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	iTD_Structure  *spTempiTD;
	BYTE  *pbData;
	DWORD *pwData;
	DWORD iTDInt = ((psUsbHostAVdc->sVideoMjpegStream.dwiTdInt+1) == 2) ? 0 : 1;
#if WEBCAM_SAVE
	static DRIVE  *sDrv;
	static STREAM *shandle;
	static DWORD cntWrite = 0;
#endif

	MP_DEBUG("webCamVideoStreamParseFrame");

	if (psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx == PERIODIC_FRAME_SIZE)
	{
		psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex = 0;
		psUsbHostAVdc->pbVideoMjpegBuffer = \
            psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex];
		psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = 0;
		psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx = 0;
	}

	while (psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx != (iTDInt * 4 * OTGH_ISO_VIDEO_DATA_BUFFER_NUM))
	{
		pbData = (BYTE *)(psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[0] 
						+ psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx * webCamVideoGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg));

		pwData = (DWORD*)&pbData[4];

		if ((psUsbHostAVdc->dwDoor == 0) && (*pwData > 13) && (pbData[12] == 0xff) && (pbData[13] == 0xd8))
		{
			mpDebugPrint("mb");
			psUsbHostAVdc->dwDoor = 1;
		}

		if ((psUsbHostAVdc->dwDoor == 1) && (*pwData > 12) &&\
            ((psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] + (*pwData-12)) <\
            psUsbHostAVdc->dwMjpgBufferSize))
		{
			memcpy(
            &psUsbHostAVdc->pbVideoMjpegBuffer[psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex]],\
            &pbData[12],\
            (*pwData - 12));
			psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] += (*pwData - 12);
		}
		else if ((psUsbHostAVdc->dwDoor == 1) && (*pwData > 12))
		{
			mpDebugPrint("mjpeg too large");
			psUsbHostAVdc->dwDoor = 2;
			psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = 0;
		}
		if ((psUsbHostAVdc->dwDoor == 1) && (pbData[1] & 0x02)) // end of mjpeg
		{
			if (!(pbData[1] & 0x40)) // check error
			{ // no error

#if WEBCAM_SAVE
		if (cntWrite == 0)
		{
			DWORD ret;
			sDrv=DriveChange(SD_MMC);
			ret=CreateFile(sDrv, "test", "jpg");
			if (!ret)
				shandle = FileOpen(sDrv);
		}
		if (cntWrite > 30)
			FileClose(shandle);
		else if (cntWrite == 26) 
			FileWrite(shandle, 
			          psUsbHostAVdc->pbVideoMjpegBuffer, 
			          psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex]);
		cntWrite++;
#endif


#if WEBCAM_PLAY

      IntDisable();
	  ImageAdhocDraw_Decode(psUsbHostAVdc->pbVideoMjpegBuffer,\
                            psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex]);
	  IntEnable();

#endif

				mpDebugPrint("me");
				psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = TRUE;
				psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex = \
                    (++psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex ==  \
                    MJPEG_NUM_BUF) ?  0 : psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex;
				psUsbHostAVdc->pbVideoMjpegBuffer = \
                    psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex];
				psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = 0;
				psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = FALSE;
				psUsbHostAVdc->dwDoor = 0;

			}
			else
			{ // error occurs
				psUsbHostAVdc->pbVideoMjpegBuffer = \
                    psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex];
				psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = 0;
				psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = FALSE;
				psUsbHostAVdc->dwDoor = 0;
			}
		}
		if ((psUsbHostAVdc->dwDoor == 2) && (pbData[1] & 0x02)) // end of mjpeg
		{
			mpDebugPrint("drop large frame");
			psUsbHostAVdc->pbVideoMjpegBuffer = psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex];
			psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = 0;
			psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex] = FALSE;
			psUsbHostAVdc->dwDoor = 0;
		}
		psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx =\
            (++psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx ==\
            (2*4*OTGH_ISO_VIDEO_DATA_BUFFER_NUM)) ? 0 : psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx;
	}
}

static void webCamVideoUsbOtgHostIsoIoc (WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	iTD_Structure  *spTempiTD;
	BYTE  *pbData;
	DWORD *pwData;
	DWORD wFrameNumber,wiTDNum,i,j;
	DWORD indexOffset = psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdInt];
	DWORD pageOffset = 0;

	MP_DEBUG("webCamVideoUsbOtgHostIsoIoc");
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }

	wiTDNum = psUsbHostAVdc->sVideoMjpegStream.dwiTdNum[psUsbHostAVdc->sVideoMjpegStream.dwiTdInt];
//mpDebugPrint("wiTDNum %d indexOffset %d", wiTDNum, indexOffset);

	for (i = 0; i < wiTDNum; ++i)
	{
		spTempiTD = (iTD_Structure  *)psUsbHostAVdc->dwVideoFrameList[indexOffset++];
		indexOffset = (indexOffset == PERIODIC_FRAME_SIZE) ? 0 : indexOffset;
//mpDebugPrint("spTempiTD %x", spTempiTD);

		for (j = 0; j < (psUsbHostAVdc->psDeviceAp->bDeviceSpeed == 0 ? 1 : 8); ++j)
		{
			pbData = (BYTE *)(psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[psUsbHostAVdc->sVideoMjpegStream.dwiTdInt*OTGH_ISO_VIDEO_DATA_BUFFER_NUM] +\
                pageOffset * webCamVideoGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg));
			pageOffset++;
			pwData = (DWORD *)&pbData[4]; 

			if (spTempiTD->ArrayStatus_Word[j].bStatus)
				mpDebugPrint("webCamVideoUsbOtgHostIsoIoc: error status %x j %x", spTempiTD->ArrayStatus_Word[j].bStatus, j);

            // fill the data length that will over-write the PTS
            *pwData = (webCamVideoGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg) - spTempiTD->ArrayStatus_Word[j].bLength);
		}
	}
	webCamVideoStreamParseFrame(eWhichOtg);
}

static void webCamAudioUsbOtgHostIsoIoc(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	iTD_Structure *spTempiTD;
	BYTE  *pbData;
	DWORD wFrameNumber,wiTDNum,i,j;
	DWORD pageOffset = 0;
	DWORD indexOffset = psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdInt];

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }
    
	MP_DEBUG("webCamAudioUsbOtgHostIsoIoc");
	wiTDNum = psUsbHostAVdc->sAudioStream.dwiTdNum[psUsbHostAVdc->sAudioStream.dwiTdInt];
	psUsbHostAVdc->pbAudioBuffer = psUsbHostAVdc->sAudioStream.pbAudioBuffer[psUsbHostAVdc->sAudioStream.dwBufferCurIndex];
	psUsbHostAVdc->sAudioStream.dwBufferLength[psUsbHostAVdc->sAudioStream.dwBufferCurIndex] = 0;
	for (i = 0; i < wiTDNum; ++i)
	{
		spTempiTD = (iTD_Structure  *)psUsbHostAVdc->dwAudioFrameList[indexOffset++];
		indexOffset = (indexOffset == PERIODIC_FRAME_SIZE) ? 0 : indexOffset;
		for (j = 0; j < (psUsbHostAVdc->psDeviceAp->bDeviceSpeed == 0 ? 1 : 8); ++j)
		{
			pbData = (BYTE *)psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[2*OTGH_ISO_VIDEO_DATA_BUFFER_NUM+psUsbHostAVdc->sAudioStream.dwiTdInt*OTGH_ISO_AUDIO_DATA_BUFFER_NUM] + pageOffset * webCamAudioGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg);
			pageOffset++;
			if (spTempiTD->ArrayStatus_Word[j].bStatus)
				mpDebugPrint("webCamAudioUsbOtgHostIsoIoc: error status %x j %x", spTempiTD->ArrayStatus_Word[j].bStatus, j);
			else if ((psUsbHostAVdc->pbAudioBuffer != NULL) && (spTempiTD->ArrayStatus_Word[j].bLength != webCamAudioGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg)))
			{
				memcpy(&psUsbHostAVdc->pbAudioBuffer[psUsbHostAVdc->sAudioStream.dwBufferLength[psUsbHostAVdc->sAudioStream.dwBufferCurIndex]], 
								&pbData[0], 
								(webCamAudioGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg) - spTempiTD->ArrayStatus_Word[j].bLength));
				psUsbHostAVdc->sAudioStream.dwBufferLength[psUsbHostAVdc->sAudioStream.dwBufferCurIndex] += (webCamAudioGetStreamInterfaceEndPointMaxPacketSize(eWhichOtg) - spTempiTD->ArrayStatus_Word[j].bLength);
			}
		}
	}

	psUsbHostAVdc->dwAudioDataCnt += psUsbHostAVdc->sAudioStream.dwBufferLength[psUsbHostAVdc->sAudioStream.dwBufferCurIndex];
	if (psUsbHostAVdc->sAudioStream.pbAudioBuffer[psUsbHostAVdc->sAudioStream.dwBufferCurIndex] != NULL)
		psUsbHostAVdc->sAudioStream.dwBufferActive[psUsbHostAVdc->sAudioStream.dwBufferCurIndex] = TRUE;

	psUsbHostAVdc->sAudioStream.dwBufferCurIndex = (++psUsbHostAVdc->sAudioStream.dwBufferCurIndex ==  AUDIO_NUM_BUF) ?  0 : psUsbHostAVdc->sAudioStream.dwBufferCurIndex;
	if (psUsbHostAVdc->dwTimerTick1 == 0)
		psUsbHostAVdc->dwTimerTick1 = GetSysTime();
	if (SystemGetElapsedTime(psUsbHostAVdc->dwTimerTick1) > 1000)
	{
		psUsbHostAVdc->dwAudioDataCnt = 0;
		psUsbHostAVdc->dwTimerTick1 = 0;
	}
}

static void webCamUsbOtgHostIsoIoc(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	iTD_Structure  *spTempiTD;
	DWORD wiTDNum,j;

	MP_DEBUG("webCamUsbOtgHostIsoIoc");
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }

	if (psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[psUsbHostAVdc->sVideoMjpegStream.dwiTdInt] == TRUE)
	{
		spTempiTD = (iTD_Structure *)psUsbHostAVdc->dwVideoFrameList[psUsbHostAVdc->sVideoMjpegStream.dwLastFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdInt]-1];
		for (j = 0; j < (psUsbHostAVdc->psDeviceAp->bDeviceSpeed == 0 ? 1 : 8); ++j)
		{
			if (spTempiTD->ArrayStatus_Word[j].bInterruptOnComplete == 0)
				continue;

			if ((spTempiTD->ArrayStatus_Word[j].bStatus & HOST20_iTD_Status_Active) == 0)
			{
				webCamVideoUsbOtgHostIsoIoc(eWhichOtg);
				psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[psUsbHostAVdc->sVideoMjpegStream.dwiTdInt] = FALSE;
				psUsbHostAVdc->sVideoMjpegStream.dwiTdInt = (++psUsbHostAVdc->sVideoMjpegStream.dwiTdInt == 2) ? 0 : 1;
				if (psUsbHostAVdc->sVideoMjpegStream.dwStreamActive)
				{
					ST_MCARD_MAIL	*pSendMailDrv;
					DWORD			osSts;
					pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
					pSendMailDrv->wCurrentExecutionState   = WEB_CAM_IN_STATE;
					pSendMailDrv->wStateMachine            = WEB_CAM_SM;
					osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
					if (osSts != OS_STATUS_OK)
						mpDebugPrint("webCamUsbOtgHostIoc: SendMailToUsbOtgHostClassTask failed!!");
				}
			}
		}
	}

	if (psUsbHostAVdc->sAudioStream.dwIsoInActive[psUsbHostAVdc->sAudioStream.dwiTdInt] == TRUE)
	{
		spTempiTD = (iTD_Structure *)psUsbHostAVdc->dwAudioFrameList[psUsbHostAVdc->sAudioStream.dwLastFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdInt]-1];
		for (j = 0; j < (psUsbHostAVdc->psDeviceAp->bDeviceSpeed == 0 ? 1 : 8); ++j)
		{
			if (spTempiTD->ArrayStatus_Word[j].bInterruptOnComplete == 0)
				continue;

			if ((spTempiTD->ArrayStatus_Word[j].bStatus & HOST20_iTD_Status_Active) == 0)
			{
				webCamAudioUsbOtgHostIsoIoc(eWhichOtg);
				psUsbHostAVdc->sAudioStream.dwIsoInActive[psUsbHostAVdc->sAudioStream.dwiTdInt] = FALSE;
				psUsbHostAVdc->sAudioStream.dwiTdInt = (++psUsbHostAVdc->sAudioStream.dwiTdInt == 2) ? 0 : 1;
				if (psUsbHostAVdc->sAudioStream.dwStreamActive)
				{
					ST_MCARD_MAIL	*pSendMailDrv;
					DWORD			osSts;
					pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
					pSendMailDrv->wCurrentExecutionState   = WEB_CAM_IN_STATE;
					pSendMailDrv->wStateMachine            = WEB_CAM_SM;
					osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
					if (osSts != OS_STATUS_OK)
						mpDebugPrint("webCamUsbOtgHostIoc: SendMailToUsbOtgHostClassTask failed!!");
				}
			}
		}
	}
}

int WebCamGetVideoStatus(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD index;

	index = (psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex == 0) ? (MJPEG_NUM_BUF - 1) : (psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex - 1);
	//mpDebugPrint("WebCamGetVideoStatus %i inx %x", videoMjpegStream.bufferActive[index] ? 0 : 1, videoMjpegStream.bufferCurIndex);

	return psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[index] ? 0 : 1;	
}

void webCamStart(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    ST_MCARD_MAIL   *pSendMailDrv;
	SDWORD	osSts;

	MP_DEBUG("webCamStart");
	psUsbHostAVdc->sVideoMjpegStream.dwStreamActive = TRUE;
	psUsbHostAVdc->sAudioStream.dwStreamActive   = TRUE;

    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
	pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_PROBE;//WEB_CAM_VIDEO_CLASS_INIT_STATE0;
	pSendMailDrv->wStateMachine          = WEB_CAM_SM;

	osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
	if (osSts != OS_STATUS_OK)
	{
		mpDebugPrint("webCamStart: SendMailToUsbOtgHostClassTask failed!!");
	}
}

void webCamStop(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD i;
    ST_MCARD_MAIL   *pSendMailDrv;
	SDWORD	osSts;

	MP_DEBUG("webCamStop");
	psUsbHostAVdc->sVideoMjpegStream.dwStreamActive   = FALSE;
	psUsbHostAVdc->sAudioStream.dwStreamActive   = FALSE;
	webCamSWDeInit(eWhichOtg);

	if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
		return;

    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
	pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_STOP; // WEB_CAM_VIDEO_CLASS_DEINIT_STATE;
	pSendMailDrv->wStateMachine          = WEB_CAM_SM;

	osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
	if (osSts != OS_STATUS_OK)
	{
		mpDebugPrint("webCamStop: SendMailToUsbOtgHostClassTask failed!!");
	}
}

DWORD webCamVideoGetStreamBuffer( DWORD dwBuffer, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	DWORD rtn = PASS;
	DWORD index;

	MP_DEBUG("webCamVideoGetStreamBuffer");
	index = (psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex == 0) ? (MJPEG_NUM_BUF - 1) : (psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex - 1);

	if (psUsbHostAVdc->sVideoMjpegStream.dwStreamActive == FALSE)
		return FAIL;

	if (psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[index] && psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[index] != NULL)
	{
		memcpy(	(BYTE *)dwBuffer, &psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[index][0], psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[index]);
		psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[index] = FALSE;
	}
	else return FAIL;

	return rtn;
}

DWORD WebCamGetAudioType(WHICH_OTG eWhichOtg)
{
	DWORD type = webCamAudioGetType(eWhichOtg);

	switch (type)
	{
		case 1: // pcm
			MP_DEBUG("pcm");
			type = 0;
		break;
		default:
		break;
	}
	return type;
}

DWORD WebCamGetAudioSampleSize(WHICH_OTG eWhichOtg)
{
	MP_DEBUG("sample size %i bits", webCamAudioGetSampleSize(eWhichOtg));
	return webCamAudioGetSampleSize(eWhichOtg);
}

DWORD WebCamGetAudioFreqRate(WHICH_OTG eWhichOtg)
{
	MP_DEBUG("FreqRate %i", webCamAudioGetFreqRate(eWhichOtg));
	return webCamAudioGetFreqRate(eWhichOtg);
}

DWORD webCamAudioGetStreamBuffer(DWORD dwBuffer, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgDevDescGet(eWhichOtg);
	DWORD rtn = PASS;
	DWORD index;
	WebCamAudioFram *pWebCamAudioFram = (WebCamAudioFram *)dwBuffer;
    
	MP_DEBUG("webCamAudioGetStreamBuffer");
	index = (psUsbHostAVdc->sAudioStream.dwBufferCurIndex == 0) ? (AUDIO_NUM_BUF - 1) : (psUsbHostAVdc->sAudioStream.dwBufferCurIndex - 1);

	if (psUsbHostAVdc->sAudioStream.dwStreamActive == FALSE)
		return FAIL;
	if (psUsbHostAVdc->sAudioStream.dwBufferActive[index] && psUsbHostAVdc->sAudioStream.pbAudioBuffer[psUsbHostAVdc->sAudioStream.dwBufferCurIndex] != NULL)
	{
		memcpy(	(BYTE *)pWebCamAudioFram->FramBuffer, &psUsbHostAVdc->sAudioStream.pbAudioBuffer[index][0], psUsbHostAVdc->sAudioStream.dwBufferLength[index]);
		pWebCamAudioFram->size = psUsbHostAVdc->sAudioStream.dwBufferLength[index];

		psUsbHostAVdc->sAudioStream.dwBufferActive[index] = FALSE;
	}
	else return FAIL;
	MP_DEBUG("webCamAudioGetStreamBuffer");

	return rtn;
}
#endif //#if 0

//====================================================================
// * Function Name: webCamPeriodicFrameListInit                          
// * Description: 
//   <1>.Init FrameList
//   <2>.Enable Periotic schedule
//
// * Input: 
// * OutPut: 
//====================================================================
void  webCamPeriodicFrameListInit (WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
	PST_USB_OTG_HOST psUsbOtgHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
	UINT32 i;
    PST_USB_OTG_DES psUsbOtg;

	flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

	MP_DEBUG("webCamInit begin");
	//Variable init - ISO
	psUsbHostAVdc->psDeviceAp = psUsbOtgHost->psUsbhDeviceDescriptor;
	//psUsbHostAVdc->psDeviceAp->bDeviceOnHub	= 0;
	//psUsbHostAVdc->psDeviceAp->bAdd			= GetDeviceAddress(eWhichOtg);
	psUsbHostAVdc->psDeviceAp->bDeviceSpeed = mwOTG20_Control_HOST_SPD_TYP_Rd();
	psUsbHostAVdc->dwDoor = 0;

	psUsbHostAVdc->psDeviceAp->bMaxIsoInEpNumber = 0;
	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) || webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg))
		psUsbHostAVdc->psDeviceAp->bMaxIsoInEpNumber++;
	psUsbHostAVdc->psDeviceAp->bMaxIsoOutEpNumber = 0;

	MP_DEBUG("frame Size W %d H %d", (Api_UsbhWebCamVideoGetCurResolution(eWhichOtg)>>16), (Api_UsbhWebCamVideoGetCurResolution(eWhichOtg)&0x0000ffff));

//	psUsbHostAVdc->dwMjpgBufferSize= OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000 * 8;

#if 0
	// init video buffer stream
	psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex = 0;
	psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx = PERIODIC_FRAME_SIZE;
 	for (i = 0; i < MJPEG_NUM_BUF; ++i)
	{
		psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[i] = 0;
		psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[i] = 0;
	}
//	psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[0] = (BYTE*)ext_mem_malloc(2*psUsbHostAVdc->dwMjpgBufferSize);
//	psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[1] = psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[0] + psUsbHostAVdc->dwMjpgBufferSize;

	psUsbHostAVdc->sVideoMjpegStream.dwiTdInt = 0;
	psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx = 0;
	for (i = 0; i < 2; ++i)
	{
	    psUsbHostAVdc->sVideoMjpegStream.dwiTdNum[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sVideoMjpegStream.dwLastFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[i] = FALSE;
	}
#endif
	// init audio buffer stream
	psUsbHostAVdc->sAudioStream.dwBufferCurIndex = 0;
	for (i = 0; i < AUDIO_NUM_BUF; ++i)
	{
		psUsbHostAVdc->sAudioStream.dwBufferActive[i] = 0;
		psUsbHostAVdc->sAudioStream.dwBufferLength[i] = 0;
	}

	psUsbHostAVdc->sAudioStream.dwiTdInt = 0;
	psUsbHostAVdc->sAudioStream.dwiTdIdx = 0;
	for (i = 0; i < 2; ++i)
	{
	    psUsbHostAVdc->sAudioStream.dwiTdNum[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sAudioStream.dwLastFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sAudioStream.dwIsoInActive[i] = FALSE;
	}
	psUsbHostAVdc->dwAudioBufferSize = (OTGH_ISO_AUDIO_DATA_BUFFER_NUM * 0x1000);

//	psUsbHostAVdc->sAudioStream.pbAudioBuffer[0] = (BYTE*)ext_mem_malloc(AUDIO_NUM_BUF*psUsbHostAVdc->dwAudioBufferSize);
 	for (i = 1; i < AUDIO_NUM_BUF; ++i)
 	{
		psUsbHostAVdc->sAudioStream.pbAudioBuffer[i] = psUsbHostAVdc->sAudioStream.pbAudioBuffer[i-1] + psUsbHostAVdc->dwAudioBufferSize;
		psUsbHostAVdc->sAudioStream.pbAudioBuffer[i] = psUsbHostAVdc->sAudioStream.pbAudioBuffer[i-1] + psUsbHostAVdc->dwAudioBufferSize;
		psUsbHostAVdc->sAudioStream.pbAudioBuffer[i] = psUsbHostAVdc->sAudioStream.pbAudioBuffer[i-1] + psUsbHostAVdc->dwAudioBufferSize;
 	}

	for (i=0;i<Host20_iTD_MAX;i++)
		psUsbHostEhci->pbHostItdManage[i]=Host20_MEM_FREE;

	for (i=0;i<Host20_Page_MAX;i++)
		psUsbHostEhci->pbHostDataPageManage[i]=Host20_MEM_FREE;

//	for(i = 0; i < Host20_Page_MAX; i++)
//		psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[i]=flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER, eWhichOtg);       

//	UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[0],
//                                        (DWORD)psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[0] + 2*OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000 - 1,
//                                        eWhichOtg);

//	for(i = 0; i < 2*OTGH_ISO_AUDIO_DATA_BUFFER_NUM; i++)
//		psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[2*OTGH_ISO_VIDEO_DATA_BUFFER_NUM+i]=flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER, eWhichOtg);          

//#if 1
	webCamiTDAlloc(eWhichOtg);
	// <1>.Init FrameList
	for (i=0;i<PERIODIC_FRAME_SIZE;i++)      
	{
		psUsbHostEhci->psHostFramList->sCell[i].bLinkPointer = psUsbHostAVdc->dwVideoFrameList[i]>>5;
		psUsbHostEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_iTD;     
		psUsbHostEhci->psHostFramList->sCell[i].bTerminal    = 0;
	}
//#endif
//	mbHost20_USBCMD_FrameListSize_Set(FRAME_LIST_SIZE_256);
	//<2>.Set Periodic Base Address	
//	mwHost20_PeriodicBaseAddr_Set(psUsbHostEhci->dwHostStructurePflBaseAddress);	

	//<2>.Enable the periodic 
	flib_Host20_Periodic_Setting(HOST20_Enable, eWhichOtg);
	psUsbHostAVdc->boVideoStreamInit = TRUE;

	MP_DEBUG("webCamPeriodicFrameListInit end");
}

//====================================================================
// * Function Name: webCamDeInit                          
// * Description: 
//   <1>.Init FrameList
//   <2>.Enable Periotic schedule
//
// * Input: 
// * OutPut: 
//====================================================================
void  webCamDeInit(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

    MP_DEBUG("webCamDeInit begin");
    webCamSWDeInit(eWhichOtg);

    SetWebcamState(WEB_CAM_STATE_NOT_EXIST, eWhichOtg);
    psUsbHostAVdc->boVideoStreamInit = FALSE;
    webCamResetParam(eWhichOtg);
    UsbOtgHostIsocDisable(eWhichOtg);

    EventSet(UI_EVENT, EVENT_WEB_CAM_OUT);
    MP_DEBUG("webCamDeInit end");
}

//====================================================================
// * Function Name: webCamSWDeInit                          
// * Description: 
//   <1>.Init FrameList
//   <2>.Enable Periotic schedule
//
// * Input: 
// * OutPut: 
//====================================================================
void  webCamSWDeInit (WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
	DWORD i;

	MP_DEBUG("%s begin", __FUNCTION__);

	// <1>.Init FrameList
	for (i=0;i<PERIODIC_FRAME_SIZE;i++)      
	{
		//if (psUsbHostAVdc->psDeviceAp->bDeviceOnHub==0)
			psUsbHostEhci->psHostFramList->sCell[i].bType=HOST20_HD_Type_iTD;     
		//else
		//	psUsbHostEhci->psHostFramList->sCell[i].bType=HOST20_HD_Type_siTD;     

		psUsbHostEhci->psHostFramList->sCell[i].bLinkPointer=0;
		psUsbHostEhci->psHostFramList->sCell[i].bTerminal=1;             
	}
	webCamiTDFree(eWhichOtg);

	for (i=0;i<Host20_iTD_MAX;i++)
		psUsbHostEhci->pbHostItdManage[i]=Host20_MEM_FREE;

	for (i=0;i<Host20_Page_MAX;i++)
		psUsbHostEhci->pbHostDataPageManage[i]=Host20_MEM_FREE;
#if 0
	// init video buffer stream
	psUsbHostAVdc->sVideoMjpegStream.dwBufferCurIndex = 0;
	psUsbHostAVdc->sVideoMjpegStream.dwScaniTDIdx = PERIODIC_FRAME_SIZE;
 	for (i = 0; i < MJPEG_NUM_BUF; ++i)
	{
		psUsbHostAVdc->sVideoMjpegStream.dwBufferActive[i] = 0;
		psUsbHostAVdc->sVideoMjpegStream.dwBufferLength[i] = 0;
	}
	psUsbHostAVdc->sVideoMjpegStream.dwiTdInt = 0;
	psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx = 0;
	for (i = 0; i < 2; ++i)
	{
	    psUsbHostAVdc->sVideoMjpegStream.dwiTdNum[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sVideoMjpegStream.dwLastFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[i] = FALSE;
	}
#endif
//	ext_mem_free(psUsbHostAVdc->sVideoMjpegStream.pbVideoMjpegBuffer[0]);

	// init audio buffer stream
	psUsbHostAVdc->sAudioStream.dwBufferCurIndex = 0;
	for (i = 0; i < AUDIO_NUM_BUF; ++i)
	{
		psUsbHostAVdc->sAudioStream.dwBufferActive[i] = 0;
		psUsbHostAVdc->sAudioStream.dwBufferLength[i] = 0;
	}
	psUsbHostAVdc->sAudioStream.dwiTdInt = 0;
	psUsbHostAVdc->sAudioStream.dwiTdIdx = 0;
	for (i = 0; i < 2; ++i)
	{
	    psUsbHostAVdc->sAudioStream.dwiTdNum[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sAudioStream.dwLastFrameNumber[i] = PERIODIC_FRAME_SIZE;
		psUsbHostAVdc->sAudioStream.dwIsoInActive[i] = FALSE;
	}

//	ext_mem_free(psUsbHostAVdc->sAudioStream.pbAudioBuffer[0]);

	//UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);

	psUsbHostAVdc->psDeviceAp->bMaxIsoInEpNumber = 0;
	psUsbHostAVdc->psDeviceAp->bMaxIsoOutEpNumber = 0;

	MP_DEBUG("%s end", __FUNCTION__);
}

void dumpiTD(volatile iTD_Structure *piTD)
{
	if (piTD == 0)
	{
		mpDebugPrint("iTD %x", piTD);
		return;
	}
	mpDebugPrint("----------");
	do {
		mpDebugPrint("iTD %x", piTD);
		mpDebugPrint("iTD->bNextLinkPointer = %x", piTD->bNextLinkPointer<<5);
		mpDebugPrint("iTD->bType            = %x", piTD->bType);
		mpDebugPrint("iTD->bTerminate       = %x", piTD->bTerminate);

		mpDebugPrint("iTD->bStatus              = %x", piTD->ArrayStatus_Word[0].bStatus);
		mpDebugPrint("iTD->bLength              = %x", piTD->ArrayStatus_Word[0].bLength);
		mpDebugPrint("iTD->bInterruptOnComplete = %x", piTD->ArrayStatus_Word[0].bInterruptOnComplete);
		mpDebugPrint("iTD->bPageSelect          = %x", piTD->ArrayStatus_Word[0].bPageSelect);
		mpDebugPrint("iTD->bOffset              = %x", piTD->ArrayStatus_Word[0].bOffset);

		mpDebugPrint("iTD->EndPt                = %x", (piTD->ArrayBufferPointer_Word[0].bMultiFunction>>8)&0xf);
		mpDebugPrint("iTD->DeviceAddr           = %x", piTD->ArrayBufferPointer_Word[0].bMultiFunction&0x7f);
		mpDebugPrint("iTD->io                   = %s", (piTD->ArrayBufferPointer_Word[1].bMultiFunction&BIT11)?"in":"out");
		mpDebugPrint("iTD->MPS                  = %x", (piTD->ArrayBufferPointer_Word[1].bMultiFunction&0x7ff));
		mpDebugPrint("iTD->mult                 = %x", (piTD->ArrayBufferPointer_Word[1].bMultiFunction&3));

		if (piTD->bTerminate==0)
			piTD = (iTD_Structure *)(piTD->bNextLinkPointer<<5);
		else piTD=(iTD_Structure *)0;
	} while (piTD != 0);

}

//extern SDWORD SysTimerProcAdd(DWORD dwOffsetValue, void *actionPtr, BOOL isOneShot);
static DWORD *videoFrameListAssr = 0;
static PST_USB_OTG_DES psUsbOtg_tmp = 0;
static DWORD eOtg = 0;
static void webCamTimer(DWORD tmp)
{
	iTD_Structure  *spTempiTD;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eOtg);
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eOtg);
	DWORD wFrameNumber = 0;

		//mpDebugPrint("command %x frameIndex %x isoAddr %x asynAddr %x", 
		//	psUsbOtg_tmp->psUsbReg->HcUsbCommand,
		//	psUsbOtg_tmp->psUsbReg->HcFrameIndex,
		//	psUsbOtg_tmp->psUsbReg->HcPeriodicFrameListBaseAddress,
		//	psUsbOtg_tmp->psUsbReg->HcCurrentAsynListAddress);
#if 0
		mpDebugPrint("cqHD0 %x addr %x ep %x",
	        pUsbhDev->pstControlqHD[0],
	        pUsbhDev->pstControlqHD[0]->bDeviceAddress,
	        pUsbhDev->pstControlqHD[0]->bEdNumber);


		mpDebugPrint("CqTDP %x NqTDP %x T %x D %x tb %d ioc %d ec %x sts %x aqTD %x T %x",
	        pUsbhDev->pstControlqHD[0]->bOverlay_CurrentqTD,
	        pUsbhDev->pstControlqHD[0]->bOverlay_NextqTD,
	        pUsbhDev->pstControlqHD[0]->bOverlay_NextTerminate,
	        pUsbhDev->pstControlqHD[0]->bOverlay_Direction,
	        pUsbhDev->pstControlqHD[0]->bOverlay_TotalBytes,
	        pUsbhDev->pstControlqHD[0]->bOverlay_InterruptOnComplete,
	        pUsbhDev->pstControlqHD[0]->bOverlay_ErrorCounter,
	        pUsbhDev->pstControlqHD[0]->bOverlay_Status, 
	        pUsbhDev->pstControlqHD[0]->bOverlay_AlternateqTD,
	        pUsbhDev->pstControlqHD[0]->bOverlay_AlternateNextTerminate);
#endif

if (0)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eOtg);
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eOtg);
	DWORD wFrameNumber = 0;

	while(wFrameNumber < PERIODIC_FRAME_SIZE)
	{
		spTempiTD=(iTD_Structure*)(iTD_Structure*)videoFrameListAssr[wFrameNumber];

		mpDebugPrint("lp %x type %x T %x", 
			psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bLinkPointer << 5,
			psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bType,     
			psUsbHostEhci->psHostFramList->sCell[wFrameNumber++].bTerminal);
	}
}
if (0)
{
	int i,index;
#if 0
	mpDebugPrint("scell %x", psUsbHostEhci->psHostFramList->sCell);
	for (i=0;i<PERIODIC_FRAME_SIZE;i++)      
	{
		mpDebugPrint("bLinkPointer %x", psUsbHostEhci->psHostFramList->sCell[i].bLinkPointer << 5);
	}
#endif

	while(wFrameNumber < PERIODIC_FRAME_SIZE)
	{
		spTempiTD=(iTD_Structure*)(iTD_Structure*)videoFrameListAssr[wFrameNumber++];
		mpDebugPrint("%d spTempiTD %x bTerminate %x", 
			wFrameNumber,
			spTempiTD,
			spTempiTD->bTerminate);


		for (index = 0; index < 8; ++index)
			mpDebugPrint("bf %x mtf %x",
				spTempiTD->ArrayBufferPointer_Word[index].bBufferPointer,
				spTempiTD->ArrayBufferPointer_Word[index].bMultiFunction);

		for (index = 0; index < 8; ++index)
			mpDebugPrint("len %x sts %x ioc %x ps %x os %x",
				spTempiTD->ArrayStatus_Word[index].bLength,
				spTempiTD->ArrayStatus_Word[index].bStatus,
				spTempiTD->ArrayStatus_Word[index].bInterruptOnComplete,
				spTempiTD->ArrayStatus_Word[index].bPageSelect,
				spTempiTD->ArrayStatus_Word[index].bOffset);
	}
}
#if 0
		mpDebugPrint("command %x frameIndex %x isoAddr %x asynAddr %x", 
			psUsbOtg_tmp->psUsbReg->HcUsbCommand,
			psUsbOtg_tmp->psUsbReg->HcFrameIndex,
			psUsbOtg_tmp->psUsbReg->HcPeriodicFrameListBaseAddress,
			psUsbOtg_tmp->psUsbReg->HcCurrentAsynListAddress);

		mpDebugPrint("cqHD0 %x addr %x ep %x",
	        pUsbhDev->pstControlqHD[0],
	        pUsbhDev->pstControlqHD[0]->bDeviceAddress,
	        pUsbhDev->pstControlqHD[0]->bEdNumber);

		mpDebugPrint("cqHD1 %x addr %x ep %x",
	        pUsbhDev->pstControlqHD[1],
	        pUsbhDev->pstControlqHD[1]->bDeviceAddress,
	        pUsbhDev->pstControlqHD[1]->bEdNumber);
#endif
}
#if 0
void webCamUsbOtgHostIssueIso(
                UINT32 wEndPt,
                UINT32 wMaxPacketSize,
                UINT32 wSize,
                UINT32 *pwBufferArray,
                UINT32 wOffset,
                UINT8 bDirection,
                UINT8 bMult, 
                WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	DWORD wRemainSize,wCurrentOffset,wCurrentBufferNum,wCurrentLength,wiTDNum,wFrameNumber,i,wOriginalFrameNumber,wDummyTemp;
	volatile iTD_Structure  *spTempiTD;
	DWORD *pwLastTransaction;
	DWORD wCurrentBufferNum1,wCurrentBufferNum3,j,wCurrentBufferNum2,wCurrentBufferNum4;
	DWORD *wIsoiTDAddress;
	BYTE  bCurrentTransactionNum,bCurrentPageNum,bTransactionNumMax,bExitFlag;

	if (videoFrameListAssr == 0)
	{	
		eOtg = eWhichOtg;
		SysTimerProcAdd(100, webCamTimer, 1);
		psUsbOtg_tmp = psUsbOtg;
	}

	//Critical Time Period Start ---------------------------------------------------        
	wOriginalFrameNumber=(mwHost20_FrameIndex_Rd()>>3) & 0xff;

	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
	{
		if ((psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx == 0) && (psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx] == PERIODIC_FRAME_SIZE))
			psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[0] = wOriginalFrameNumber + 0x20;
		else 
		{
			if (psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx == 0)
				psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[0] = psUsbHostAVdc->sVideoMjpegStream.dwLastFrameNumber[1];
			else
				psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[1] = psUsbHostAVdc->sVideoMjpegStream.dwLastFrameNumber[0];
		}

		if (psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx] >= PERIODIC_FRAME_SIZE)
			psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx] -= PERIODIC_FRAME_SIZE;

		wFrameNumber=psUsbHostAVdc->sVideoMjpegStream.dwOriginalFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx];
		wIsoiTDAddress = &psUsbHostAVdc->dwVideoFrameList[0];
		videoFrameListAssr = wIsoiTDAddress;
	}
	else if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
	{
		if ((psUsbHostAVdc->sAudioStream.dwiTdIdx == 0) && (psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdIdx] == PERIODIC_FRAME_SIZE))
			psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[0] = wOriginalFrameNumber + 0x40;
		else 
		{
			if (psUsbHostAVdc->sAudioStream.dwiTdIdx == 0)
				psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[0] = psUsbHostAVdc->sAudioStream.dwLastFrameNumber[1];
			else
				psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[1] = psUsbHostAVdc->sAudioStream.dwLastFrameNumber[0];
		}

		if (psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdIdx] >= PERIODIC_FRAME_SIZE)
			psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdIdx] -= PERIODIC_FRAME_SIZE;

		wFrameNumber=psUsbHostAVdc->sAudioStream.dwOriginalFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdIdx];
		wIsoiTDAddress = &psUsbHostAVdc->dwAudioFrameList[0];
	}
	else
	{
		mpDebugPrint("webCamUsbOtgHostIssueIso: undefined end point %x", wEndPt);
		while(1);
	}
	//<2>.Allocate the iTD for the Data Buffer
	wRemainSize=wSize;
	wCurrentBufferNum=0;
	wiTDNum=0;
	wCurrentOffset=wOffset;
//mpDebugPrint("wFN %d", wFrameNumber);

	while(wRemainSize)
	{
		//<2.1>.Allocate iTD
		spTempiTD=(iTD_Structure*)wIsoiTDAddress[wFrameNumber++];
//mpDebugPrint("spTempiTD %x", spTempiTD);
		wFrameNumber = (wFrameNumber == PERIODIC_FRAME_SIZE) ? 0 : wFrameNumber;
		spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction|=(wEndPt<<8);
		spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction|=GetDeviceAddress(eWhichOtg);
		spTempiTD->ArrayBufferPointer_Word[1].bMultiFunction=(bDirection<<11);
		spTempiTD->ArrayBufferPointer_Word[1].bMultiFunction|=wMaxPacketSize;
		spTempiTD->ArrayBufferPointer_Word[2].bMultiFunction=bMult;
		bCurrentTransactionNum=0;
		bCurrentPageNum=0;
		spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer = ((UINT32)*(pwBufferArray+wCurrentBufferNum)>>12); 
//mpDebugPrint("-%x %x %x %x %x", 
//	spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer<<12,
//	pwBufferArray,
//	wCurrentBufferNum,
//	pwBufferArray+wCurrentBufferNum, ((UINT32)*(pwBufferArray+wCurrentBufferNum)>>12));
//mpDebugPrint("psUsbHostAVdc->psDeviceAp->bDeviceSpeed %x", psUsbHostAVdc->psDeviceAp->bDeviceSpeed);

         if (psUsbHostAVdc->psDeviceAp->bDeviceSpeed==0)
            bTransactionNumMax=1;//For Full Speed
         else
         	bTransactionNumMax=8;//For High Speed

		//<2.2>.Fill iTD
		while ((wRemainSize) && (bCurrentTransactionNum < bTransactionNumMax))
		{
//mpDebugPrint("-%x %x", wMaxPacketSize, bMult);
//mpDebugPrint("wRemainSize %x", wRemainSize);
			//Fill iTD
			//if (wRemainSize<(wMaxPacketSize*bMult))
			//{
			//	mpDebugPrint(">>> Length wRemainSize %x must be the X*wMaxPacketSize*bMult\n", wRemainSize);
			//	while(1);
			//}
			//else 
				wCurrentLength=wMaxPacketSize;//*bMult;

//mpDebugPrint("%x %x", wCurrentLength, wCurrentOffset);

//mpDebugPrint("bCurrentTransactionNum %x bTransactionNumMax %x", bCurrentTransactionNum, bTransactionNumMax);
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bLength=wCurrentLength;
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus=HOST20_iTD_Status_Active;
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete=0;	
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bPageSelect=bCurrentPageNum;	
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bOffset=wCurrentOffset;

//mpDebugPrint("%x %x %x %x %x", 
//	spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bLength,
//	spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus,
//	spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete,
//	spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bPageSelect,
//	spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bOffset);

			wRemainSize=wRemainSize-wCurrentLength;
			wCurrentOffset=wCurrentOffset+wCurrentLength;

			if (wCurrentOffset >= 4096)
			{
				bCurrentPageNum++;	
				wCurrentBufferNum++;

				spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer
							=((UINT32)*(pwBufferArray+wCurrentBufferNum)>>12);  
//mpDebugPrint("=%x %x %x %x %x", 
//	spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer<<12,
//	pwBufferArray,
//	wCurrentBufferNum,
//	pwBufferArray+wCurrentBufferNum, ((UINT32)*(pwBufferArray+wCurrentBufferNum)>>12));

				wCurrentOffset=wCurrentOffset - 4096;	
			} 

			//Set the finish Complete-Interrupt
			if (wRemainSize==0)
			{
				spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete=1;	
				//pwLastTransaction=(DWORD*)&(spTempiTD->ArrayStatus_Word[bCurrentTransactionNum]);
			}

			bCurrentTransactionNum++;    
		}//while ((wRemainSize)&&(bCurrentTransactionNum<8))

		//<2.3>.Maintain Variable
//mpDebugPrint("wiTDNum %d", wiTDNum);

		wiTDNum++;
		if (wiTDNum>512)
		{
			mpDebugPrint(">>> Waring...iTD Number >512..."); 	
			while(1);
		}
	} //while(wRemainSize)

	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
	{
		psUsbHostAVdc->sVideoMjpegStream.dwiTdNum[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx] = wiTDNum;
		psUsbHostAVdc->sVideoMjpegStream.dwIsoInActive[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx] = TRUE;
	}
	else if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
	{
		psUsbHostAVdc->sAudioStream.dwiTdNum[psUsbHostAVdc->sAudioStream.dwiTdIdx] = wiTDNum;
		psUsbHostAVdc->sAudioStream.dwIsoInActive[psUsbHostAVdc->sAudioStream.dwiTdIdx] = TRUE;
	}
//mpDebugPrint("-wFN %d", wFrameNumber);
	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
		psUsbHostAVdc->sVideoMjpegStream.dwLastFrameNumber[psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx] = wFrameNumber;
	else if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
		psUsbHostAVdc->sAudioStream.dwLastFrameNumber[psUsbHostAVdc->sAudioStream.dwiTdIdx] = wFrameNumber;

	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
		psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx = (++psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx == 2) ? 0 : 1;
	else if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
		psUsbHostAVdc->sAudioStream.dwiTdIdx = (++psUsbHostAVdc->sAudioStream.dwiTdIdx == 2) ? 0 : 1;

	MP_DEBUG("webCamUsbOtgHostIssueIso: end");
}

static void OTGH_PT_ISO_IN(
            UINT32 wEndPt,
            UINT32 wSize, 
            UINT32 wMaxPacketSize, 
            WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD bufferIdx;

	MP_DEBUG("OTGH_PT_ISO_IN");

	if (webCamVideoGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
		bufferIdx = psUsbHostAVdc->sVideoMjpegStream.dwiTdIdx*OTGH_ISO_VIDEO_DATA_BUFFER_NUM;
	else if (webCamAudioGetStreamInterfaceEndPointNumber(eWhichOtg) == wEndPt)
		bufferIdx = 2*OTGH_ISO_VIDEO_DATA_BUFFER_NUM+psUsbHostAVdc->sAudioStream.dwiTdIdx*OTGH_ISO_AUDIO_DATA_BUFFER_NUM;
	else return;

	webCamUsbOtgHostIssueIso(wEndPt, 
				wMaxPacketSize, 
				wSize, 
				&(psUsbHostAVdc->sOtgHostIso.dwDataBufferArray[bufferIdx]), 
				0,		// offset 
				OTGH_Dir_IN, 
				webCamVideoGetStreamInterfaceEndPointMult(eWhichOtg),
				eWhichOtg);
}
#endif

/*
#define USING_MPEG2_TS 1
#define USING_MJPEG    0

#if USING_MPEG2_TS
#define SAVE_50FRAMES_USING_INSIGHT 0
#if SAVE_50FRAMES_USING_INSIGHT
#define H264_BUFF_LEN (50*40*1024)
BYTE *g_pH264Data;
DWORD g_H264BufIdx = 0;
#endif // SAVE_50FRAMES_USING_INSIGHT
#endif // USING_MPEG2_TS
*/
#define SAVE_50FRAMES_USING_INSIGHT 0
BYTE *g_pH264Data;
DWORD g_H264BufIdx = 0;
//int g_test_res = 0;
static void UsbOtgHostWebcamIsocDataIn(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;
    SWORD sts = FAIL;
        
    //mpDebugPrint("%s", __FUNCTION__);
	#if USBCAM_IN_ENABLE
	  if (st_bUsbCamPreStop)
			return;
	#endif
    if (pUsbhDevDes->bIsoInEnable == FALSE)
    {
        MP_ALERT("%s:Isoc IN is disable!!", __FUNCTION__);
        return;
    }
    

#if ISOC_QUEUE_DYNAMIC
	BYTE *pbDataBuffer;
	DWORD dwSize;

	GetQueueDataHead(&pbDataBuffer,&dwSize);
	if (dwSize== 0 || pbDataBuffer == NULL)
	{
		  //MP_ALERT("dequeue null!!!");
		  //EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
		  return;
	}
    //mpDebugPrint("%s: decode %d!", __FUNCTION__,dwSize);

	if (pUsbhDevDes->fpParseIsocInData != NULL)
	{
		sts = pUsbhDevDes->fpParseIsocInData(pbDataBuffer, dwSize);
		if (sts == FAIL)
		{
			mpDebugPrint("%s:decode fail!!", __FUNCTION__);
		}

	}
	UsbOtgHostIsocInDataDequeueReady(eWhichOtg);

	GetQueueDataHead(&pbDataBuffer,&dwSize);
	if (dwSize && pbDataBuffer!=NULL)
		EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);

#else
    pIsocBuff = UsbOtgHostIsocInDataDequeueGo(eWhichOtg);
    if (pIsocBuff == NULL)
    {
        MP_ALERT("Dequeued buffer is NULL!!");
        return;
    }
    else
    {
        MP_DEBUG("%d:IP: 0x%x; %d", __LINE__, pIsocBuff->pbDataBuffer, pIsocBuff->dwLength);
        if (pIsocBuff->dwLength == 0 || pIsocBuff->pbDataBuffer == NULL)
        {
            MP_ALERT("dequeue null!!!");
            return;
        }


        //UartOutText("<");
        //UartOutValue(pIsocBuff->dwLength, 5);
        //UartOutText(">");
        //mpDebugPrint("%s:0x%x", __FUNCTION__, pUsbhDevDes->fpParseIsocInData);
        if (pUsbhDevDes->fpParseIsocInData != NULL)
        {
            sts = pUsbhDevDes->fpParseIsocInData(pIsocBuff->pbDataBuffer, pIsocBuff->dwLength);
            if (sts == FAIL)
            {
                //mpDebugPrint("Write to SD");
                //DumpVideoQueue();
                mpDebugPrint("%s:decode fail!!", __FUNCTION__);
                //IODelay(200);
                //__asm("break 100");
            }

#if SAVE_50FRAMES_USING_INSIGHT
            static int framecnt = 0;
            framecnt++;
            UartOutText("<");
            UartOutValue(pIsocBuff->dwLength, 5);
            UartOutText(">");
            if (framecnt == 50)
            {
            IODelay(200);
            __asm("break 100");
            }

            mmcp_memcpy_polling((BYTE*)((DWORD)g_pH264Data + g_H264BufIdx), (BYTE *)pIsocBuff->pbDataBuffer, pIsocBuff->dwLength);
            g_H264BufIdx += pIsocBuff->dwLength;
#endif // #if SAVE_50FRAMES_USING_INSIGHT
        }
        else
        {
            mpDebugPrint("%s:fpParseIsocInData is NULL", __FUNCTION__);
        }
        
       // if (g_test_res==1)
       //     g_test_res = 2;
        UsbOtgHostIsocInDataDequeueReady(eWhichOtg);
    } // if (pIsocBuff == NULL)
    
    TaskYield();

#endif
}

void DumpBuffer(BYTE *pData, DWORD length)
{
    int i = 0;
    mpDebugPrint("%s:0x%x, %d", __FUNCTION__, pData, length);
    for (i = 0; i < length; i+=8)
    {
        mpDebugPrint("    %02X %02X %02X %02X %02X %02X %02X %02X",\
            pData[i+0],\
            pData[i+1],\
            pData[i+2],\
            pData[i+3],\
            pData[i+4],\
            pData[i+5],\
            pData[i+6],\
            pData[i+7]);
    }
    //IODelay(200);
    //__asm("break 100");
}


static PST_USB_OTG_HOST_ISOC_DES UsbOtgHostWebcamGetIsocInDes (BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_USB_OTG_HOST_ISOC_DES pIsocEp = NULL;

    if (pUsbhDevDes->bIsoInEnable == TRUE)
    pIsocEp = GetBestIsocDes(bInterfaceClass, bInteraceNumber, USB_DIR_IN, eWhichOtg);
    
    return pIsocEp;
}

/*
void CHECK_T(void); 
int g_check_t = 0;
void CHECK_T(void) 
{
    WORD *pCheck_t = 0;
    
    if(g_check_t)
    {
        pCheck_t = (WORD*)0xa0328c10;
        UartOutText("t");
        if(pCheck_t[0]!= 0xffd8)
        {
            __asm("break 100");
        }
    }
}
#define LIMITED_FRAME_COUNT 100
BOOL CheckFrameCount (DWORD dwOriginalFrameNumber, DWORD dwLastFrameNumber)
{
    DWORD frameCount  = 0;
    BOOL ret = FALSE;
    
    if (dwLastFrameNumber > dwOriginalFrameNumber)
    {
        frameCount = dwLastFrameNumber - dwOriginalFrameNumber;
    }
    else
    {
        frameCount = ((PERIODIC_FRAME_SIZE-dwOriginalFrameNumber)+(dwLastFrameNumber+1));
    }

    UartOutText("<");
    UartOutValue(frameCount, 3);
    UartOutText(">");
    if (frameCount > LIMITED_FRAME_COUNT)
    {
        ret = FALSE;
        //mpDebugPrint("%d:PERIODIC_FRAME_SIZE = %d, (PERIODIC_FRAME_SIZE-%d)=%d, dwOriginalFrameNumber=%d",\
        //    __LINE__,
        //    PERIODIC_FRAME_SIZE,
        //    dwLastFrameNumber,
        //    (PERIODIC_FRAME_SIZE-dwLastFrameNumber),
        //    dwOriginalFrameNumber);
        //IODelay(200);
        //__asm("break 100");
    }
    else
    {
        ret = TRUE;
    }

    return ret;
        
}
*/

#define UVC_HEADER_SIZE 12
#define UVC_HEADER_FRAME_ID     BIT0
#define UVC_HEADER_END_OF_FRAME BIT1
#define UVC_HEADER_ERROR        BIT6
    
#define CHECK_QUEUE_COUNT 200
SDWORD GetIsocInBuffer(BYTE **hData, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    SWORD sts = USBOTG_NO_ERROR;
    DWORD bTryCnt = 0;
    
    do {
        if (pUsbhDevDes->bIsoInEnable == FALSE)
            return FALSE;
        
        sts = UsbOtgHostIsocInDataEnqueueDataBuffer(&pUsbhDevDes->pbIsocInBuffer, dwFrameNumber, dwItd, eWhichOtg);
        if (sts != USBOTG_NO_ERROR)
        {
            if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
            {
                MP_ALERT("%s:Device plug-out!!", __FUNCTION__);
                return FALSE;
            }
            else if (sts == USBOTG_UNKNOW_ERROR)
            {
            MP_ALERT("%s:sts = %d ", __FUNCTION__, sts);
                return FALSE;
            }
            
            bTryCnt++;
            if (bTryCnt >= CHECK_QUEUE_COUNT)
            {
                MP_ALERT("%s:sts = %d; bTryCnt = %d", __FUNCTION__, sts, bTryCnt);
                //IODelay(200);
                //__asm("break 100");
                return FALSE;
            }
        }
        
        TaskYield();
    } while (sts != USBOTG_NO_ERROR);

    return TRUE;
}


extern int g_ff;
#if USBCAM_DEBUG_ISR_LOST
static DWORD st_dwIsocDataIn=0;
#endif
#if ISOC_EVERY_QUEUE_SIZE_FIXED
static BYTE st_bQueueDataInvalid=0;
#endif

static SDWORD UsbOtgHostWebcamIsocInDataProcess (WHICH_OTG eWhichOtg)
{
    //PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PUSB_HOST_UVC psUsbhUvc = (PUSB_HOST_UVC)UsbOtgHostUvcDsGet(eWhichOtg);
    iTD_Structure *spTempiTD;
    BYTE  *pbData;
    DWORD wiTDNum,i,j;
    DWORD pageOffset = 0;
    DWORD indexOffset = pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdInt];
    PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
    DWORD dwMaxPacketSize = 0;
    SDWORD sts = 0;
    SWORD wLength = 0;
    BYTE bMult = 0;
    DWORD *pdwDataOffset = 0;
    DWORD dwRemainDataLength = 0;
    //static int dCnt = 1;

    //if ((dCnt++)%50)
#if USBCAM_DEBUG_ISR_LOST
    st_dwIsocDataIn++;
	if ((st_dwIsocDataIn%1000)==0)
		mpDebugPrint("DataIn %d",st_dwIsocDataIn);
#endif
	#if USBCAM_IN_ENABLE
	  if (st_bUsbCamPreStop)
			return;
	#endif

    if (pUsbhDevDes->bIsoInEnable == FALSE)
        return USBOTG_UNKNOW_ERROR;
    
    pIsocInEpDes = GetBestIsocDes(pUsbhDevDes->bAppInterfaceClass, pUsbhDevDes->bAppInterfaceNumber, USB_DIR_IN, eWhichOtg);
    if (pIsocInEpDes == NULL)
    {
        MP_ALERT("%s:%d:pIsocInEpDes is NULL!", __FUNCTION__, __LINE__);
        return USBOTG_UNKNOW_ERROR;
    }
    
    dwMaxPacketSize  = (pIsocInEpDes->wMaxPacketSize) & ~(BIT12|BIT11);
    bMult = (pIsocInEpDes->wMaxPacketSize) >> 11;
    dwMaxPacketSize *= (bMult+1);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        return USBOTG_UNKNOW_ERROR;
    }

    wiTDNum = pUsbhDevDes->stIsocDataStream.dwiTdNum[pUsbhDevDes->stIsocDataStream.dwiTdInt];
    MP_DEBUG("%s:%d:wiTDNum = %d; indexOffset = %d", __FUNCTION__, __LINE__, wiTDNum, indexOffset);

    //pUsbhDevDes->stIsocDataStream.dwBufferLength[pUsbhDevDes->stIsocDataStream.dwBufferCurIndex] = 0;
    pdwDataOffset = &pUsbhDevDes->stIsocDataStream.dwBufferLength[0];
    MP_DEBUG("%s:%d:*pdwDataOffset = %d", __FUNCTION__, __LINE__, *pdwDataOffset);
    for (i = 0; i < wiTDNum; ++i)
    {
        psUsbhUvc->dwFrameNumber = indexOffset;
        spTempiTD = (iTD_Structure  *)pUsbhDevDes->pstPeriodicFrameList[0].dwFrameList[indexOffset++];
        MP_DEBUG("%s:%d:indexOffset = %d; spTempiTD = 0x%x", __FUNCTION__, __LINE__, (indexOffset-1), spTempiTD);
        indexOffset = (indexOffset == PERIODIC_FRAME_SIZE) ? 0 : indexOffset;
        for (j = 0; j < (pUsbhDevDes->bDeviceSpeed == 0 ? 1 : 8); ++j)
        {
            SDWORD index = 0;

            if (pUsbhDevDes->bIsoInEnable == FALSE)
            {
                MP_ALERT("%s:%d:Stop Processing", __FUNCTION__, __LINE__);
                psUsbhUvc->eNewFrame = HAS_NO_FRAME;
                psUsbhUvc->bNewOneFrame = 0;
                pUsbhDevDes->dwIsocInDataCount = 0;
                pUsbhDevDes->pbIsocInBuffer = NULL;
                mpDebugPrint("%s:%d", __func__, __LINE__);
                __asm("break 100");
                *pdwDataOffset = 0;
                return USBOTG_UNKNOW_ERROR;
            }

            index = pUsbhDevDes->stIsocDataStream.dwiTdInt*(OTGH_ISO_VIDEO_DATA_BUFFER_NUM);
            if (index > (ISOC_DATA_NUM_BUF-1)*OTGH_ISO_VIDEO_DATA_BUFFER_NUM)
            {
                mpDebugPrint("%d:index > %d!!", __LINE__, (ISOC_DATA_NUM_BUF-1)*OTGH_ISO_VIDEO_DATA_BUFFER_NUM);
                IODelay(200);
                __asm("break 100");
            }
            
            pbData = (BYTE *)psEhci->stOtgHostIsoc.dwDataBufferArray[index] + pageOffset * dwMaxPacketSize;
            pageOffset++;            
            //MP_DEBUG("%d:j = %d; index = %d; pageOffset = %d; pbData = 0x%x", __LINE__, j, index, pageOffset-1, pbData);
            if (spTempiTD->ArrayStatus_Word[j].bStatus & 0x7)
            {
                ;//UartOutText("x");
                //mpDebugPrint("%s: error status %x j %x; do not care for isoc tx", __FUNCTION__, spTempiTD->ArrayStatus_Word[j].bStatus, j);
                //IODelay(200);
                //__asm("break 100");
            }
            else if (spTempiTD->ArrayStatus_Word[j].bLength != dwMaxPacketSize)
            {
                if ((pageOffset*dwMaxPacketSize) > pUsbhDevDes->dwIsocInBufferSize)
                {
                    dwRemainDataLength = pUsbhDevDes->dwIsocInBufferSize - ((pageOffset-1)*dwMaxPacketSize);
                    mpDebugPrint("%d:dwRemainDataLength = %d",__LINE__, dwRemainDataLength);
                }
                
                if (spTempiTD->ArrayStatus_Word[j].bInterruptOnComplete == 1)
                {
                    if (dwRemainDataLength > spTempiTD->ArrayStatus_Word[j].bLength)
                    {
                        wLength = dwRemainDataLength - spTempiTD->ArrayStatus_Word[j].bLength;
                        mpDebugPrint("%d:dwRemainDataLength = %d",__LINE__, dwRemainDataLength);
                    }
                    else
                        wLength = dwMaxPacketSize - spTempiTD->ArrayStatus_Word[j].bLength;
                    dwRemainDataLength = 0;
                }
                else
                {
                    wLength = dwMaxPacketSize - spTempiTD->ArrayStatus_Word[j].bLength;
                }

                if (wLength < 0)
                {
                    MP_ALERT("%d:wLength is negtive!! %d", __LINE__, wLength);
                    MP_ALERT("%d:dwMaxPacketSize = %d", __LINE__, dwMaxPacketSize);
                    MP_ALERT("%d:spTempiTD->ArrayStatus_Word[j].bLength = %d", __LINE__, spTempiTD->ArrayStatus_Word[j].bLength);
                    MP_ALERT("%d:<j = %d; s = %d; wLength = %d; P = %d; offset = %d>", __LINE__,\
                        j, ((pageOffset-1)*dwMaxPacketSize), wLength, spTempiTD->ArrayStatus_Word[j].bPageSelect,\
                        spTempiTD->ArrayStatus_Word[j].bOffset);
                    IODelay(200);
                    __asm("break 100");
                }
                
                MP_DEBUG("%d:<j = %d; s = %d; wLength = %d; P = %d; offset = %d>", __LINE__,\
                    j, ((pageOffset-1)*dwMaxPacketSize), wLength, spTempiTD->ArrayStatus_Word[j].bPageSelect,\
                    spTempiTD->ArrayStatus_Word[j].bOffset);
                ///////////////////////////////////////////////////
                // collect data
                //int c_tt = 0;

                MP_DEBUG("%d:pbData[1] 0x%x eNewFrame %d", __LINE__, pbData[1], psUsbhUvc->eNewFrame);
                if (pbData[1] & UVC_HEADER_ERROR)
                {
                    MP_DEBUG("%d:UVC_HEADER_ERROR!!", __LINE__);
                    continue;
                }
                
                //MP_DEBUG("%d:dCnt = %d; T = 0x%x%x", __LINE__, dCnt++, pbData[6], pbData[7]);

              /*
                c_tt = pbData[6]<<8|pbData[7];
                if (0)//sg_tt > 0)
                {
                    UartOutText("<");
                    UartOutValue(c_tt, 4);
                    UartOutText(">");
                }
              */  
         //////////////////////////////////////////////////////////////////////////////
         //
         // Check if New From Coming with frame ID (D0) and get a buffer from Queue
         //
         //////////////////////////////////////////////////////////////////////////////
                psUsbhUvc->bFrameID = (pbData[1] & UVC_HEADER_FRAME_ID);
                if (psUsbhUvc->bFrameID != psUsbhUvc->bStartOfFrame)
                {
                    psUsbhUvc->bStartOfFrame = (psUsbhUvc->bStartOfFrame == 0)?1:0; // frame ID is toggled
                    if ((psUsbhUvc->bNewOneFrame == 0) && (wLength > UVC_HEADER_SIZE))
                    { // here is the first time the data in length > UVC_HEADER_SIZE.
                        psUsbhUvc->bNewOneFrame = 1;
                        psUsbhUvc->eNewFrame = NEW_FRAME_BEGIN;
                        psUsbhUvc->dwOriginalFrameNumber = psUsbhUvc->dwFrameNumber;
                        MP_DEBUG("%d:FRAME_STATE:UVC_HEADER_FRAME_ID:eNewFrame %d:call GetIsocInBuffer here", __LINE__, psUsbhUvc->eNewFrame);
                        #if ISOC_EVERY_QUEUE_SIZE_FIXED
								st_bQueueDataInvalid=0;
							#endif
                        //////////////////////////////////////////////////////////////////////////////
                        //
                        // Get a buffer from queue
                        //
                        //////////////////////////////////////////////////////////////////////////////
                        sts = GetIsocInBuffer(&pUsbhDevDes->pbIsocInBuffer, psUsbhUvc->dwFrameNumber, (DWORD)spTempiTD, eWhichOtg);
                        if (sts != TRUE)
                        {
                            MP_ALERT("No Buffer in Queue!!");
                            return USBOTG_UNKNOW_ERROR;
                        }
                        else
                        {
                            MP_DEBUG("%d:pUsbhDevDes->pbIsocInBuffer 0x%x", __LINE__, pUsbhDevDes->pbIsocInBuffer);
                        }

                        if (pUsbhDevDes->pbIsocInBuffer == NULL)
                            MP_ALERT("%s:%d", __func__, __LINE__);
                    }
                    else
                    {
                        // need to do something for no EOF case
                        MP_DEBUG("%d:no EOF case: maybe reset *pdwDataOffset from %d to zero", __LINE__, *pdwDataOffset);
                        MP_DEBUG("%d:FRAME_STATE:eNewFrame %d", __LINE__, psUsbhUvc->eNewFrame);
                        *pdwDataOffset = 0;
                    }
                }

                //////////////////////////////////////////////////////////////////////////////
                //
                // Process data while size > header 12 bytes
                //
                //////////////////////////////////////////////////////////////////////////////
                if (psUsbhUvc->bNewOneFrame && wLength >= UVC_HEADER_SIZE)
                {
                    MP_DEBUG("%d:<j = %d; s = %d; wLength = %d; P = %d; offset = %d>",__LINE__,\
                        j, \
                        ((pageOffset-1)*dwMaxPacketSize), \
                        wLength, \
                        spTempiTD->ArrayStatus_Word[j].bPageSelect,\
                        spTempiTD->ArrayStatus_Word[j].bOffset);
                    MP_DEBUG("%d:0x%x <0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x>",\
                        __LINE__,\
                        pbData,\
                        pbData[UVC_HEADER_SIZE],\
                        pbData[UVC_HEADER_SIZE+1],\
                        pbData[UVC_HEADER_SIZE+2],\
                        pbData[UVC_HEADER_SIZE+3],\
                        pbData[UVC_HEADER_SIZE+4],\
                        pbData[UVC_HEADER_SIZE+5],\
                        pbData[UVC_HEADER_SIZE+6],\
                        pbData[UVC_HEADER_SIZE+7]);

                    //////////////////////////////////////////////////////////////////////////////
                    //
                    // Copy packet data into the buffer without header 12 bytes
                    //
                    //////////////////////////////////////////////////////////////////////////////
                    if (pUsbhDevDes->pbIsocInBuffer  != NULL)
                    {
                        MP_DEBUG("%d:memcpy:pUsbhDevDes->pbIsocInBuffer 0x%x", __LINE__, pUsbhDevDes->pbIsocInBuffer);
							#if ISOC_EVERY_QUEUE_SIZE_FIXED
							if (st_bQueueDataInvalid)
							{
								mpDebugPrintN("-v-");
							}
							else	if (*pdwDataOffset+(wLength-UVC_HEADER_SIZE)>GetQueueElementByteCount(WEBCAM_USB_PORT))
							{
								st_bQueueDataInvalid=1;
								UartOutText("-V-");
								//mpDebugPrint("--Data in overload!!!throw it! %d",*pdwDataOffset+(wLength-UVC_HEADER_SIZE));
							}
							else
							#endif
							{
							#if (USBCAM_IN_ENABLE==2)
							#if 1
							 PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(WEBCAM_USB_PORT);
							DWORD dwFrameOffset=psUsbHostAVdc->dwVideoFrameSize>>16<<1,dwLenth,dwHead,dwLine,dwTail,i;
							DWORD dwWinoffset=((ST_IMGWIN  *)Idu_GetNextWin())->dwOffset;
							//BYTE *pbSrcData=(BYTE*)&pbData[UVC_HEADER_SIZE];//(((DWORD)(&pbData[UVC_HEADER_SIZE])) | 0xa0000000);
							//BYTE *pbTrgData=pUsbhDevDes->pbIsocInBuffer;//(BYTE*)(((DWORD)pUsbhDevDes->pbIsocInBuffer) | 0xa0000000);

							dwLenth=wLength-UVC_HEADER_SIZE;
							dwHead=dwFrameOffset-(*pdwDataOffset)%dwFrameOffset;
							if (dwHead>dwLenth)
								dwHead=dwLenth;
							dwLine=(dwLenth-dwHead)/dwFrameOffset;
							dwTail=dwLenth-dwHead-dwLine*dwFrameOffset;
							//memset memcpy
							if (dwHead)
                        		memcpy((BYTE*)(pUsbhDevDes->pbIsocInBuffer + (*pdwDataOffset)/dwFrameOffset*dwWinoffset+(*pdwDataOffset)%dwFrameOffset),&pbData[UVC_HEADER_SIZE],dwHead);
							for (i=0;i<dwLine;i++)
							{
	                        memcpy((BYTE*)(pUsbhDevDes->pbIsocInBuffer + ((*pdwDataOffset)/dwFrameOffset+1+i)*dwWinoffset), 
	                               &pbData[UVC_HEADER_SIZE]+dwHead+i*dwFrameOffset,
	                               dwFrameOffset);
							}
							if (dwTail)
                        		memcpy((BYTE*)(pUsbhDevDes->pbIsocInBuffer + ((*pdwDataOffset)/dwFrameOffset+1+i)*dwWinoffset),&pbData[UVC_HEADER_SIZE]+dwHead+i*dwFrameOffset,dwTail);
							//TaskYield();
							#endif
							#else
                        memcpy((BYTE*)(pUsbhDevDes->pbIsocInBuffer + *pdwDataOffset), 
                               &pbData[UVC_HEADER_SIZE],
                               (wLength-UVC_HEADER_SIZE));
                        //CopyYuvData((DWORD*)(pUsbhDevDes->pbIsocInBuffer + *pdwDataOffset), 
                        //       (DWORD*)&pbData[UVC_HEADER_SIZE],
                        //       (wLength-UVC_HEADER_SIZE));
                        //bCopyYuvData((BYTE*)(pUsbhDevDes->pbIsocInBuffer + *pdwDataOffset), 
                         //      &pbData[UVC_HEADER_SIZE],
                        //       (wLength-UVC_HEADER_SIZE));
                        #endif
							}
                        if (*pdwDataOffset == 0)
                        {
                            MP_DEBUG("%d:0x%x <0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x>",\
                                __LINE__,\
                                pUsbhDevDes->pbIsocInBuffer,\
                                pUsbhDevDes->pbIsocInBuffer[0],\
                                pUsbhDevDes->pbIsocInBuffer[1],\
                                pUsbhDevDes->pbIsocInBuffer[2],\
                                pUsbhDevDes->pbIsocInBuffer[3],\
                                pUsbhDevDes->pbIsocInBuffer[4],\
                                pUsbhDevDes->pbIsocInBuffer[5],\
                                pUsbhDevDes->pbIsocInBuffer[6],\
                                pUsbhDevDes->pbIsocInBuffer[7]);
                            
                        }
                    }
                    else
                    {
                        //MP_ALERT("%s:%d:isocE %d:buffer is NULL!!", __func__, __LINE__, pUsbhDevDes->bIsoInEnable);
                        //////////////////////////////////////////////////////////////////////////////
                        //
                        // something wrong in processing data and it needs to drop frame but not complete yet!!
                        //
                        //////////////////////////////////////////////////////////////////////////////
                        if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
                        {
                            MP_ALERT("%s:%d:Device plug-out!!", __FUNCTION__, __LINE__);
                            return USBOTG_UNKNOW_ERROR;
                        }

                        //MP_ALERT("%s:%d:isocE %d", __FUNCTION__, __LINE__, pUsbhDevDes->bIsoInEnable);
                        psUsbhUvc->eNewFrame = HAS_NO_FRAME;
                        psUsbhUvc->bNewOneFrame = 0;
                        pUsbhDevDes->dwIsocInDataCount = 0;
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        //__asm("break 100");
                        *pdwDataOffset = 0;
                        MP_ALERT("%s:%d:isocE %d:buffer is NULL!!", __func__, __LINE__, pUsbhDevDes->bIsoInEnable);
                        return USBOTG_UNKNOW_ERROR;
                    }
                    
                    *pdwDataOffset += (wLength-UVC_HEADER_SIZE);
                    MP_DEBUG("%d:wLength = %d; *pdwDataOffset = %d", __LINE__, wLength, *pdwDataOffset);
                    wLength = 0;

                    //////////////////////////////////////////////////////////////////////////////
                    //
                    // Check End of Frame bit (D1) and change frame state to NEW_FRAME_END from NEW_FRAME_BEGIN
                    //
                    //////////////////////////////////////////////////////////////////////////////
                    if (pbData[1] & UVC_HEADER_END_OF_FRAME)
                    { // end of frame
                        MP_DEBUG("%d:FRAME_STATE:UVC_HEADER_END_OF_FRAME", __LINE__, psUsbhUvc->eNewFrame);
                        MP_DEBUG("%s:%d:UVC_HEADER_END_OF_FRAME",__func__, __LINE__);
                        if (psUsbhUvc->eNewFrame == NEW_FRAME_BEGIN)
                        {
                            if (*pdwDataOffset > GetQueueElementByteCount(eWhichOtg))
                            {
                                UartOutText("V");
                                //UartOutText("<.V.>");
                                //mpDebugPrint("%s:%d:%d",__FUNCTION__, __LINE__, *pdwDataOffset);
                                //IODelay(200);
                                //__asm("break 100");
                            }
                            psUsbhUvc->eNewFrame = NEW_FRAME_END;
                        }
                        else
                        {
                            //UartOutText("<WOW>");
                            MP_ALERT("%s:%d:may drop this frame:eNewFrame %d", __FUNCTION__, __LINE__, psUsbhUvc->eNewFrame);
                            psUsbhUvc->eNewFrame = HAS_NO_FRAME;
                            psUsbhUvc->bNewOneFrame = 0;
                            pUsbhDevDes->dwIsocInDataCount = 0;
                            pUsbhDevDes->pbIsocInBuffer = NULL;
                            *pdwDataOffset = 0;
                        #if ISOC_EVERY_QUEUE_SIZE_FIXED
								st_bQueueDataInvalid=1;
							#endif
                            return USBOTG_UNKNOW_ERROR;
                            //IODelay(200);
                            //__asm("break 100");
                        }
                    }
                    
                    //////////////////////////////////////////////////////////////////////////////
                    //
                    // Process End Frame
                    //
                    //////////////////////////////////////////////////////////////////////////////
                    if (psUsbhUvc->eNewFrame == NEW_FRAME_END)
                    { // the data count is the amount of isoc IN data located in the buffer
                        MP_DEBUG("%d:FRAME_STATE:psUsbhUvc->eNewFrame %d", __LINE__, psUsbhUvc->eNewFrame);
                        MP_DEBUG("%d:enqueue a frame",__LINE__);
                        psUsbhUvc->eNewFrame = HAS_NO_FRAME;
                        psUsbhUvc->bNewOneFrame = 0;
                        pUsbhDevDes->dwIsocInDataCount += *pdwDataOffset;
                        *pdwDataOffset = 0;
                        if (pUsbhDevDes->dwIsocInDataCount == 0)
                        {
                            MP_ALERT("%s:dwIsocInDataCount = 0 so do nothing", __FUNCTION__);
                        #if ISOC_EVERY_QUEUE_SIZE_FIXED
								st_bQueueDataInvalid=1;
							#endif
                            return USBOTG_UNKNOW_ERROR;
                        }
#if 0//USING_MJPEG
                    /*
                        if (g_ff) // it's for checking decode timeout for MJPEG
                        {
                            UartOutText("X");
                            IODelay(200);
                            __asm("break 100");
                        }
                        
                        // for MJPEG debugging
                        if ((pUsbhDevDes->pbIsocInBuffer[0]!=0xff) && (pUsbhDevDes->pbIsocInBuffer[1]!=0xd8))
                        {
                            IODelay(200);
                            __asm("break 100");
                        }
                      */
#endif
                        psUsbhUvc->dwLastFrameNumber = psUsbhUvc->dwFrameNumber;
                        //////////////////////////////////////////////////////////////////////////////
                        //
                        // set data length in the element of Queue
                        //
                        //////////////////////////////////////////////////////////////////////////////
                        #if ISOC_EVERY_QUEUE_SIZE_FIXED
							if (!st_bQueueDataInvalid)
							#endif
							{
                        UsbOtgHostIsocInDataEnqueueDataLength(\
                                    pUsbhDevDes->dwIsocInDataCount,\
                                    psUsbhUvc->dwFrameNumber,\
                                    (DWORD)spTempiTD,\
                                    eWhichOtg);
                        EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
						#if USBCAM_DEBUG_ISR_LOST
						st_dwIsocDataOut++;
						if ((st_dwIsocDataOut%100)==0)
						{
							st_dwDataOutTime[0]=st_dwIsocDataOut;
							st_dwDataOutTime[1]=GetSysTime();
							//mpDebugPrint("DataOut %d",st_dwIsocDataOut);
						}
						#endif
							}
                        #if ISOC_EVERY_QUEUE_SIZE_FIXED
								st_bQueueDataInvalid=1;
							#endif
                        pUsbhDevDes->dwIsocInDataCount = 0;
                        pUsbhDevDes->pbIsocInBuffer = NULL;
                    } // if (eNewFrame == NEW_FRAME_END)
                } // if (wLength > UVC_HEADER_SIZE)
                else
                {
                    MP_DEBUG("%d:wLength = %d", __LINE__, wLength);
                }

                if ((pUsbhDevDes->dwIsocInBufferSize <= ((pageOffset)*dwMaxPacketSize)))
                { // has 1K or less (zero) bytes data need to processing and then change to next iTD 
                    MP_DEBUG("%s:pageOffset = %d; dwRemainDataLength = %d; need to change to next iTD",__FUNCTION__,\
                        pageOffset, dwRemainDataLength);
                    break;
                }               
            } // if (spTempiTD->ArrayStatus_Word[j].bLength != dwMaxPacketSize)
        } // for (j = 0; j < (pUsbhDevDes->bDeviceSpeed == 0 ? 1 : 8); ++j, pageOffset++)
    } // for (i = 0; i < wiTDNum; ++i)

    return USBOTG_NO_ERROR;
}

/*
#if USING_MPEG2_TS  
#define FORMAT_INDEX        1 // 1: MPEG2-TS
#define FRAME_INDEX         1 // 1: MPEG2-TS
#elif USING_MJPEG
#define FORMAT_INDEX        2 // 2: MJPEG
#define FRAME_INDEX         2 // 2: MJPEG(352x288)
#define COMPRESSION_INDEX   1 
#define MAX_VIDEO_FRAME_SIZE (352*288*2)
#endif
*/
void PrepareDataSetCurProbe (BYTE *pData, 
                             BYTE length,
                             USBH_WEBCAM_VIDEO_FORMAT eVideoFormat,
                             DWORD dwFrameSize,
                             DWORD dwFormatIndex,
                             DWORD dwFrameIndex)
{
    UVC_PROBE_COMMIT_CONTROLS videoProbeControls;
    DWORD   dwSize       = 0;

    dwSize = (dwFrameSize >> 16) * (dwFrameSize & 0x0000FFFF) * 2;
    MP_DEBUG("%s:%d:dwSize %d", __func__, __LINE__, dwSize);

    videoProbeControls.bmHint.dwFrameInterval = 0;
    videoProbeControls.bmHint.wKeyFrameRate   = 0;//1;
    videoProbeControls.bmHint.wPFrameRate     = 0;
    videoProbeControls.bmHint.wCompQuality    = 0;
    videoProbeControls.bmHint.wCompWindowSize = 0;
    videoProbeControls.bmHint.Reserved1       = 0;//4;
    videoProbeControls.bmHint.Reserved        = 0;//0xF3;
    
    ///videoProbeControls.bFormatIndex           = FORMAT_INDEX;
    //videoProbeControls.bFrameIndex            = FRAME_INDEX;
    if (eVideoFormat == USING_MPEG2_TS)
    {
        videoProbeControls.bFormatIndex           = 1;
        videoProbeControls.bFrameIndex            = 1;
        videoProbeControls.dwFrameInterval        = byte_swap_of_dword(0);//1111111;
        videoProbeControls.dwMaxVideoFrameSize    = byte_swap_of_dword(1843200);
    }
    else if (eVideoFormat == USING_MJPEG)
    {
#if ENDOSCOPE_WEBCAM        
        videoProbeControls.bFormatIndex           = dwFormatIndex;
        videoProbeControls.bFrameIndex            = dwFrameIndex;
        videoProbeControls.dwFrameInterval        = byte_swap_of_dword(333333);//1111111;
        videoProbeControls.dwMaxVideoFrameSize    = byte_swap_of_dword(dwSize);
        videoProbeControls.wDelay                 = byte_swap_of_word(50);//6273;
#else        
        videoProbeControls.bFormatIndex           = 2;
        videoProbeControls.bFrameIndex            = 2;
        videoProbeControls.dwFrameInterval        = byte_swap_of_dword(667111);//1111111;
        videoProbeControls.dwMaxVideoFrameSize    = byte_swap_of_dword(0);
        videoProbeControls.wDelay                 = byte_swap_of_word(0);//6273;
#endif        
    }
    else if (eVideoFormat == USING_YUV)
    {
#if ENDOSCOPE_WEBCAM        
        videoProbeControls.bFormatIndex           = dwFormatIndex;
        videoProbeControls.bFrameIndex            = dwFrameIndex;
        videoProbeControls.dwFrameInterval        = byte_swap_of_dword(333333);//1111111;
        videoProbeControls.dwMaxVideoFrameSize    = byte_swap_of_dword(dwSize);
        videoProbeControls.wDelay                 = byte_swap_of_word(50);//6273;
#else        
        MP_ALERT("%s:%d:need to consider bFormatIndex and bFrameIndex", __func__, __LINE__, eVideoFormat);
#endif        
    }
    else
    {
        MP_ALERT("-E- %s:%d:unknow eVideoFormat %d!!", __func__, __LINE__, eVideoFormat);
    }

    videoProbeControls.wKeyFrameRate          = byte_swap_of_word(0);
    videoProbeControls.wPFrameRate            = byte_swap_of_word(0);//35100;
    videoProbeControls.wCompQuality           = byte_swap_of_word(0);
    videoProbeControls.wCompWindowSize        = byte_swap_of_word(0);//23486;
    
    videoProbeControls.dwMaxPayloadTransferSize = byte_swap_of_dword(0);
    
    memcpy ( pData, (BYTE*)&videoProbeControls.bmHint, 2);
    memcpy ( pData+2, (BYTE*)&videoProbeControls.bFormatIndex, length-2-(2*sizeof(DWORD)));
    memcpy ( pData+2+(length-2-(2*sizeof(DWORD))),\
             (BYTE*)&videoProbeControls.dwMaxVideoFrameSize, \
             2*sizeof(DWORD));
    
    MP_DEBUG("%s:%d:bFormatIndex %d bFrameIndex %d", \
        __func__, __LINE__, videoProbeControls.bFormatIndex, videoProbeControls.bFrameIndex );
}

void PrepareDataSetCurCommit (BYTE *pData,
                              BYTE length,
                              USBH_WEBCAM_VIDEO_FORMAT eVideoFormat,
                              DWORD dwFrameSize,
                              DWORD dwFormatIndex,
                              DWORD dwFrameIndex)
{
    UVC_PROBE_COMMIT_CONTROLS videoCommitControls;
    DWORD   dwSize       = 0;

    dwSize = (dwFrameSize >> 16) * (dwFrameSize & 0x0000FFFF) * 2;
    MP_DEBUG("%s:%d:dwSize %d", __func__, __LINE__, dwSize);

    videoCommitControls.bmHint.dwFrameInterval = 0;
    videoCommitControls.bmHint.wKeyFrameRate   = 0;//1;
    videoCommitControls.bmHint.wPFrameRate     = 0;
    videoCommitControls.bmHint.wCompQuality    = 0;
    videoCommitControls.bmHint.wCompWindowSize = 0;
    videoCommitControls.bmHint.Reserved1       = 0;//4;
    videoCommitControls.bmHint.Reserved        = 0;//0xF3;
    
    //videoCommitControls.bFormatIndex           = FORMAT_INDEX;
    //videoCommitControls.bFrameIndex            = FRAME_INDEX;

    if (eVideoFormat == USING_MPEG2_TS)
    {
        videoCommitControls.bFormatIndex           = 1;
        videoCommitControls.bFrameIndex            = 1;
        videoCommitControls.dwFrameInterval        = 0;//byte_swap_of_dword(1111111);//1111111;
        videoCommitControls.wDelay                 = 0;//byte_swap_of_word(6273);//6273;
        videoCommitControls.dwMaxVideoFrameSize    = byte_swap_of_dword(1843200);
        videoCommitControls.dwMaxPayloadTransferSize = byte_swap_of_dword(2048);//3072;
    }
    else if (eVideoFormat == USING_MJPEG)
    {
#if ENDOSCOPE_WEBCAM        
        videoCommitControls.bFormatIndex           = dwFormatIndex;
        videoCommitControls.bFrameIndex            = dwFrameIndex;
        videoCommitControls.dwFrameInterval        = byte_swap_of_dword(333333);//1111111;
        videoCommitControls.wDelay                 = byte_swap_of_word(50);//6273;
        videoCommitControls.dwMaxVideoFrameSize    = byte_swap_of_dword(dwSize);
        videoCommitControls.dwMaxPayloadTransferSize = byte_swap_of_dword(3072);//3072;
#else        
        videoCommitControls.bFormatIndex           = 2;
        videoCommitControls.bFrameIndex            = 2;
        videoCommitControls.dwFrameInterval        = byte_swap_of_dword(667111);//1111111;
        videoCommitControls.wDelay                 = byte_swap_of_word(6273);//6273;
        videoCommitControls.dwMaxVideoFrameSize    = byte_swap_of_dword(203341);
        videoCommitControls.dwMaxPayloadTransferSize = byte_swap_of_dword(3072);//3072;
#endif
    }
    else if (eVideoFormat == USING_YUV)
    {
#if ENDOSCOPE_WEBCAM        
        videoCommitControls.bFormatIndex           = dwFormatIndex;
        videoCommitControls.bFrameIndex            = dwFrameIndex;
        videoCommitControls.dwFrameInterval        = byte_swap_of_dword(333333);//1111111;
        videoCommitControls.wDelay                 = byte_swap_of_word(50);//6273;
        videoCommitControls.dwMaxVideoFrameSize    = byte_swap_of_dword(dwSize);
        videoCommitControls.dwMaxPayloadTransferSize = byte_swap_of_dword(3072);//3072;
#else        
        MP_ALERT("%s:%d:need to consider bFormatIndex and bFrameIndex", __func__, __LINE__, eVideoFormat);
#endif
    }
    else
    {
        MP_ALERT("-E- %s:%d:unknow eVideoFormat %d!!", __func__, __LINE__, eVideoFormat);
    }

    videoCommitControls.wKeyFrameRate          = 0;//byte_swap_of_word(0);
    videoCommitControls.wPFrameRate            = 0;//byte_swap_of_word(35100);//35100;
    videoCommitControls.wCompQuality           = 0;//byte_swap_of_word(0);
    videoCommitControls.wCompWindowSize        = 0;//byte_swap_of_word(23486);//23486;
    
    memcpy ( pData, (BYTE*)&videoCommitControls.bmHint, 2);
    memcpy ( pData+2, (BYTE*)&videoCommitControls.bFormatIndex, length-2-(2*sizeof(DWORD)));
    memcpy ( pData+2+(length-2-(2*sizeof(DWORD))),\
             (BYTE*)&videoCommitControls.dwMaxVideoFrameSize, \
             2*sizeof(DWORD));

    MP_DEBUG("%s:%d:bFormatIndex %d bFrameIndex %d", \
        __func__, __LINE__, videoCommitControls.bFormatIndex, videoCommitControls.bFrameIndex );
}

void UvcTestFunction (WHICH_OTG eWhichOtg);
void UvcTestFunction2 (WHICH_OTG eWhichOtg);
void UvcTestFunction3 (WHICH_OTG eWhichOtg);
void UvcTestFunction4 (WHICH_OTG eWhichOtg);
void UvcTestFunction5 (WHICH_OTG eWhichOtg);
void UvcTestFunction6 (WHICH_OTG eWhichOtg);

typedef enum _PROCESSING_UNIT_CONTROL_STATUS_ 
{
    PU_NOT_READY    = 0x00,
    CMD_GET_INFO,
    CMD_GET_INFO_DONE,
    CMD_GET_MIN,
    CMD_GET_MIN_DONE,
    CMD_GET_MAX,
    CMD_GET_MAX_DONE,
    CMD_GET_RES,
    CMD_GET_RES_DONE,
    CMD_GET_DEF,
    CMD_GET_DEF_DONE,
    PU_READY_STS,
    CMD_GET_CUR,
    CMD_GET_CUR_DONE,
    CMD_SET_CUR,
    CMD_SET_CUR_DONE,
} UVC_PU_CONTROL_STATUS, *PUVC_PU_CONTROL_STATUS;

void UvcProcessUnitSm (UVC_PROCESSING_CONTROLS controls, WHICH_OTG eWhichOtg)
{ 
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    WORD        wUnitId = 0;
    WORD        wControlSelectors = 0;
    SWORD       err;
    static BYTE len = 0;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    wUnitId = pUsbhDevDes->sUvcProcessUnit.bUnitID;
    wControlSelectors = (WORD)pUsbhDevDes->psUvcReq[controls].bControlSelector;
    MP_DEBUG("%s:%d:wUnitId %d wControlSelectors %d wStatus %d", \
        __func__, __LINE__, wUnitId, wControlSelectors, pUsbhDevDes->psUvcReq[controls].wStatus);
/////
    switch (pUsbhDevDes->psUvcReq[controls].wStatus)
    {
        case PU_NOT_READY:
        {
            len = 1;
            err = webCamSetupGetCmd (  (BYTE) GET_INFO,\
                                       (WORD) byte_swap_of_word(wControlSelectors),\
                                       (WORD) byte_swap_of_word(wUnitId),\
                                       len,\
                                       eWhichOtg);
            if(err == USB_NO_ERROR)
                pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_INFO;
        }
        break;

        case CMD_GET_INFO:
        {
            memcpy ( (BYTE*)&pUsbhDevDes->psUvcReq[controls].sInfo, (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, len);
            MP_DEBUG("%s:%d:sInfo %d:len %d:mem 0x%x [%d]", \
                __func__, __LINE__, pUsbhDevDes->psUvcReq[controls].sInfo,\
                len, ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress),\
                *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress));
            MP_DEBUG("sInfo: %d %d %d %d %d", pUsbhDevDes->psUvcReq[controls].sInfo.bmGetValue,\
                                               pUsbhDevDes->psUvcReq[controls].sInfo.bmSetValue,\
                                               pUsbhDevDes->psUvcReq[controls].sInfo.bmDisable,\
                                               pUsbhDevDes->psUvcReq[controls].sInfo.bmAutoupdate,\
                                               pUsbhDevDes->psUvcReq[controls].sInfo.bmAsynchronous);
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
            len = 0;
            pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_INFO_DONE;
        }
        break;
    
        case CMD_GET_INFO_DONE:
        {
            len = 2;
            err = webCamSetupGetCmd (  (BYTE) GET_MIN,\
                                       (WORD) byte_swap_of_word(wControlSelectors),\
                                       (WORD) byte_swap_of_word(wUnitId),\
                                       len,\
                                       eWhichOtg);
            if(err == USB_NO_ERROR)
                pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_MIN;
        }
        break;
        
        case CMD_GET_MIN:
        {
            memcpy ( (BYTE*)&pUsbhDevDes->psUvcReq[controls].wMin, (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, len);
            pUsbhDevDes->psUvcReq[controls].wMin = byte_swap_of_word(pUsbhDevDes->psUvcReq[controls].wMin);
            MP_DEBUG("%s:%d:wMin %d:len %d:mem 0x%x [%d]",\
                __func__, __LINE__, pUsbhDevDes->psUvcReq[controls].wMin,\
                len, ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress),\
                *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress));
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
            len = 0;
            pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_MIN_DONE;
        }
        break;

        case CMD_GET_MIN_DONE:
        {
            len = 2;
            err = webCamSetupGetCmd (  (BYTE) GET_MAX,\
                                       (WORD) byte_swap_of_word(wControlSelectors),\
                                       (WORD) byte_swap_of_word(wUnitId),\
                                       len,\
                                       eWhichOtg);
            if(err == USB_NO_ERROR)
                pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_MAX;
        }
        break;
        
        case CMD_GET_MAX:
        {
            memcpy ( (BYTE*)&pUsbhDevDes->psUvcReq[controls].wMax, (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, len);
            pUsbhDevDes->psUvcReq[controls].wMax = byte_swap_of_word(pUsbhDevDes->psUvcReq[controls].wMax);
            MP_DEBUG("%s:%d:wMax %d:len %d:mem 0x%x [%d]", \
                __func__, __LINE__, pUsbhDevDes->psUvcReq[controls].wMax, \
                len, ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress),\
                *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress));
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
            len = 0;
            pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_MAX_DONE;
        }
        break;

        case CMD_GET_MAX_DONE:
        {
            len = 2;
            err = webCamSetupGetCmd (  (BYTE) GET_RES,\
                                       (WORD) byte_swap_of_word(wControlSelectors),\
                                       (WORD) byte_swap_of_word(wUnitId),\
                                       len,\
                                       eWhichOtg);
            if(err == USB_NO_ERROR)
                pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_RES;
        }
        break;
        
        case CMD_GET_RES:
        {
            memcpy ( (BYTE*)&pUsbhDevDes->psUvcReq[controls].wRes, (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, len);
            pUsbhDevDes->psUvcReq[controls].wRes = byte_swap_of_word(pUsbhDevDes->psUvcReq[controls].wRes);
            MP_DEBUG("%s:%d:wRes %d:len %d:mem 0x%x [%d]", \
                __func__, __LINE__, pUsbhDevDes->psUvcReq[controls].wRes, \
                len, ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress),\
                *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress));
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
            len = 0;
            pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_RES_DONE;
        }
        break;

        case CMD_GET_RES_DONE:
        {
            len = 2;
            err = webCamSetupGetCmd (  (BYTE) GET_DEF,\
                                       (WORD) byte_swap_of_word(wControlSelectors),\
                                       (WORD) byte_swap_of_word(wUnitId),\
                                       len,\
                                       eWhichOtg);
            if(err == USB_NO_ERROR)
                pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_DEF;
        }
        break;
        
        case CMD_GET_DEF:
        {
            memcpy ( (BYTE*)&pUsbhDevDes->psUvcReq[controls].wDef, (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress, len);
            pUsbhDevDes->psUvcReq[controls].wDef = byte_swap_of_word(pUsbhDevDes->psUvcReq[controls].wDef);
            MP_DEBUG("%s:%d:wDef %d:len %d:mem 0x%x [%d]",\
                __func__, __LINE__, pUsbhDevDes->psUvcReq[controls].wDef, \
                len, ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress),\
                *((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress));
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
            len = 0;
            pUsbhDevDes->psUvcReq[controls].wStatus = CMD_GET_DEF_DONE;
        }
        break;
    
        case CMD_GET_DEF_DONE:
        {
            pUsbhDevDes->psUvcReq[controls].wStatus = PU_READY_STS;
        }
        break;
        
        case PU_READY_STS:
        {
            ;//*pStatus = PU_READY_STS;
        }
        break;
      
        default:
        break;
    }                
/////  
}    

#define SET_CUR_LEN 26
#define SET_CUR_EXT_LEN 11
#define STILL_IMAGE_CUR_LEN 11
void webCamStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    ST_MCARD_MAIL   *pReceiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD	osSts;
    BYTE bNeedToReArrangePfl = 0;
    WORD wAltInf4Stop = AlterSettingDefaultIdx;
    static BYTE cmdData[SET_CUR_LEN];
    SWORD err = 0;
    BYTE len = 0;
    static BYTE controls     = 0xFF;
    static BYTE cur_controls = 0;
    static BYTE isSupported  = 0;
    static BYTE isSetCurStillImgProbeDone = FALSE;    

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }
    
    pReceiveMailDrv = pUsbh->sMDevice[bMcardTransferID].sMcardRMail;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
    MP_DEBUG("%s:State = %d", __FUNCTION__, pReceiveMailDrv->wCurrentExecutionState);
    switch ( pReceiveMailDrv->wCurrentExecutionState )
    {
        case WEB_CAM_INIT: // WEB_CAM_BEGIN_STATE0:
        {
            webCamResetParam(eWhichOtg);
            SetWebcamState(WEB_CAM_STATE_NOT_READY, eWhichOtg);
#if SAVE_50FRAMES_USING_INSIGHT
            g_H264BufIdx = 0;
            g_pH264Data = (BYTE*)ext_mem_malloc(H264_BUFF_LEN, UsbOtgHostDriverTaskIdGet(eWhichOtg));
            if (g_pH264Data == 0)
            {
                MP_ALERT("-E-:-USBOTG%d-:%s:%d:Host not enough memory 1:%d", eWhichOtg, __FUNCTION__, __LINE__, ker_mem_get_free_space());
                __asm("break 100");
            }
            else
            {
                g_pH264Data = (BYTE*)((DWORD)g_pH264Data|0x20000000);
                memset(g_pH264Data, 0, H264_BUFF_LEN);
                mpDebugPrint("buffer is ready for test");
            }
#endif
            if (pUsbhDevDes->fpParseIsocInData == NULL)
            { // if did not regist the fp for image parsing, then use MJPEG by default
                MP_ALERT("%s:fpParseIsocInData is ImageAdhocDraw_Decode for MJPEG.", __FUNCTION__);
                pUsbhDevDes->fpParseIsocInData = ImageAdhocDraw_Decode; // for MJPEG
            }
            else
                MP_ALERT("%s:fpParseIsocInData is not asigned.", __FUNCTION__);

            if (pUsbhDevDes->bAppInterfaceClass != USB_CLASS_VIDEO)
            {
                bNeedToReArrangePfl = 1;
                pUsbhDevDes->bAppInterfaceClass = USB_CLASS_VIDEO;
            }

            if (psUsbHostAVdc->eVideoFormat == USING_MPEG2_TS)
            {
                //for Sonix, inf 2 is for VS_FORMAT_MPEG2TS
                if (pUsbhDevDes->bAppInterfaceNumber != 2)
                {
                    bNeedToReArrangePfl = 1;
                    pUsbhDevDes->bAppInterfaceNumber = 2;
                }
            }
            else if (psUsbHostAVdc->eVideoFormat == USING_MJPEG)
            {
                 //for Sonix, inf 1 is for VS_FORMAT_UNCOMPRESSED and VS_FORMAT_MJPEG
                 if (pUsbhDevDes->bAppInterfaceNumber != 1)
                 {
                     bNeedToReArrangePfl = 1;
                     pUsbhDevDes->bAppInterfaceNumber = 1;
                 }
            }
            else
            {
                MP_ALERT("%s:1.Video Format not defined! Use the default is MJPEG.", __FUNCTION__);
                bNeedToReArrangePfl = 1;
                psUsbHostAVdc->eVideoFormat = USING_MJPEG;
                pUsbhDevDes->bAppInterfaceNumber = 1;
            }

            if (bNeedToReArrangePfl == 1)
            {
                bNeedToReArrangePfl = 0;
                UsbOtgHostItdFree(eWhichOtg); // will allocate again with different interface number
                UsbOtgHostPrepareFrameListForIsocEp(pUsbhDevDes->bAppInterfaceClass, pUsbhDevDes->bAppInterfaceNumber, eWhichOtg);
            }

            webCamGetAudioVideoInfo(pUsbhDevDes->bAppInterfaceClass, pUsbhDevDes->bAppInterfaceNumber, eWhichOtg);
            pUsbhDevDes->fpGetIsocInDesForIsoInActive   = UsbOtgHostWebcamGetIsocInDes;
            //pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = UsbOtgHostebcamIsocInDataProcess;
            pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = NULL;
            pUsbhDevDes->fpSendIsocInDataTo             = UsbOtgHostWebcamIsocDataIn;

            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = WEB_CAM_SM;
            //if (psUsbHostAVdc->dwAudioStreamInterfaceNumber != 0)
            //	pSendMailDrv->wCurrentExecutionState    = WEB_CAM_BEGIN_STATE1;
            //else
            //pSendMailDrv->wCurrentExecutionState    = WEB_CAM_INIT_DONE;// WEB_CAM_UVC_SETCUR_PROBE; // WEB_CAM_VIDEO_CLASS_INIT_STATE0;
            pSendMailDrv->wCurrentExecutionState    = WEB_CAM_INIT_GET_PU;

            webCamVideoSetupSetInterface(pUsbhDevDes->bAppInterfaceNumber, wAltInf4Stop, eWhichOtg);
        }
        return;

        case WEB_CAM_INIT_GET_PU:
        { 
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_INIT_GET_PU;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            if (cur_controls >=  pUsbhDevDes->sUvcProcessUnit.bControlUnits)
            {
                cur_controls = PROCESSING_RESERVED;
            }
            else
            {
                MP_DEBUG("cur_controls %d controls %d status %d", \
                    cur_controls, controls, pUsbhDevDes->psUvcReq[cur_controls].wStatus);
            }
            
            if (cur_controls != controls)
            {
                switch (cur_controls)
                {
                    case PROCESSING_BRIGHTNESS:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmBrightness;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_BRIGHTNESS_CONTROL;
                        break;
                    case PROCESSING_CONTRAST:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmContrast;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_CONTRAST_CONTROL;
                        break;
                    case PROCESSING_HUE:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmHue;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_HUE_CONTROL;
                        break;
                    case PROCESSING_SATURATION:
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmSaturation;
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_SATURATION_CONTROL;
                        break;
                    case PROCESSING_SHARPNESS:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmSharpness;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_SHARPNESS_CONTROL;
                        break;
                    case PROCESSING_GAMMA:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmGamma;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_GAMMA_CONTROL;
                        break;
                    case PROCESSING_WHITE_BALANCE_TEMPERATURE:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbt;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_WHITE_BALANCE_TEMPERATURE_CONTROL;
                        break;
                    case PROCESSING_WHITE_BALANCE_COMPONENT:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbc;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_WHITE_BALANCE_COMPONENT_CONTROL;
                        break;
                    case PROCESSING_BACKLIGHT_COMPENSATON:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmBacklightCompensation;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_BACKLIGHT_COMPENSATION_CONTROL;
                        break;
                    case PROCESSING_GAIN:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmGain;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_GAIN_CONTROL;
                        break;
//                    case PROCESSING_POWER_LINE_FREQUENCY: 
//                        mpDebugPrint("%s:%d", __func__, __LINE__);
//                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmPowerLineFreq;
//                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_POWER_LINE_FREQUENCY_CONTROL;
//                        break;
//                    case PROCESSING_HUE_AUTO:
//                        mpDebugPrint("%s:%d", __func__, __LINE__);
//                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmHueAuto;
//                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_HUE_AUTO_CONTROL;
//                        break;
//                    case PROCESSING_WHITE_BALANCE_TEMPERATURE_AUTO: 
//                        mpDebugPrint("%s:%d", __func__, __LINE__);
//                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbtAuto;
//                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL;
//                        break;
//                    case PROCESSING_WHITE_BALANCE_COMPONENT_AUTO:
//                        mpDebugPrint("%s:%d", __func__, __LINE__);
//                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbcAuto;
//                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL;
//                        break;
                    case PROCESSING_DIGITAL_MULTIPLIER: 
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmDigitalMultiplier;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_DIGITAL_MULTIPLIER_CONTROL;
                        break;
                    case PROCESSING_DIGITAL_MULTIPLIER_LIMIT:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = pUsbhDevDes->sUvcProcessUnit.sUnit.bmDigitalMultiplierLimit;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL;
                        break;
                    case PROCESSING_RESERVED:
                        //mpDebugPrint("Have process all controls, jump to the state WEB_CAM_INIT_DONE", cur_controls);
                        pSendMailDrv->wCurrentExecutionState = WEB_CAM_INIT_DONE;
                        pSendMailDrv->wStateMachine          = WEB_CAM_SM;
                        break;
                    case PROCESSING_WHITE_BALANCE_TEMPERATURE_AUTO: 
                    case PROCESSING_WHITE_BALANCE_COMPONENT_AUTO:
                    case PROCESSING_POWER_LINE_FREQUENCY:
                    case PROCESSING_HUE_AUTO:
                    default:
                        //mpDebugPrint("%s:%d", __func__, __LINE__);
                        isSupported = 0;
                        pUsbhDevDes->psUvcReq[cur_controls].bControlSelector = 0;
                        break;
                }
            }

            if (cur_controls == PROCESSING_RESERVED)
            {
                controls     = 0xFF;
                cur_controls = 0;
                isSupported  = 0;
                goto _SEND_MAIL_; // send mail directly cause of pSendMailDrv be equal to pReceiveMailDrv
            }
            
            controls = cur_controls;
            if (isSupported == 1)
            {
               if (pUsbhDevDes->psUvcReq[cur_controls].wStatus != PU_READY_STS)
               {
                   UvcProcessUnitSm(cur_controls, eWhichOtg);
               }
               else
               {
                   MP_DEBUG("PU %d ready. next", cur_controls);
                   cur_controls ++;
                   isSupported = 0;
               }
            }
            else
            {
                MP_DEBUG("PU %d Not support!!", cur_controls);
                cur_controls ++;
            }

            if ((pUsbhDevDes->psUvcReq[cur_controls].wStatus == CMD_GET_INFO) ||\
                (pUsbhDevDes->psUvcReq[cur_controls].wStatus == CMD_GET_MIN)  ||\
                (pUsbhDevDes->psUvcReq[cur_controls].wStatus == CMD_GET_MAX)  ||\
                (pUsbhDevDes->psUvcReq[cur_controls].wStatus == CMD_GET_RES)  ||\
                (pUsbhDevDes->psUvcReq[cur_controls].wStatus == CMD_GET_DEF))
            {
                //mpDebugPrint("wait for cmd %d ready", pUsbhDevDes->psUvcReq[cur_controls].wStatus);
                return;
            }
        }
        //mpDebugPrint("issue cmd %d", pUsbhDevDes->psUvcReq[cur_controls].wStatus);
        break;;
        
        case WEB_CAM_INIT_DONE:
        { // send event to main
            //mpDebugPrint("%s:%d", __func__, __LINE__);
            webCamPeriodicFrameListInit(eWhichOtg);
            SetWebcamState(WEB_CAM_STATE_INIT_READY, eWhichOtg);
            EventSet(UI_EVENT, EVENT_WEB_CAM_IN);
////
////     test code       
////
            //UvcTestFunction0(eWhichOtg);          
//
// Please note:
// the application layer Main needs to call Api_UsbhWebCamStart to let webcam work
//
        }
        break;

        case WEB_CAM_UVC_SET_VIDEO_FORMAT:
        { 
            //SetWebcamState(WEB_CAM_STATE_NOT_READY, eWhichOtg);
            if (psUsbHostAVdc->eVideoFormat == USING_MPEG2_TS)
            {
                pUsbhDevDes->bAppInterfaceNumber = 2;
            }
            else if (psUsbHostAVdc->eVideoFormat == USING_MJPEG)
            {
                pUsbhDevDes->bAppInterfaceNumber = 1;
            }
            else if (psUsbHostAVdc->eVideoFormat == USING_YUV)
            {
                pUsbhDevDes->bAppInterfaceNumber = 1;
            }
            else
            {
                MP_ALERT("%s:2.Video Format not defined! Use the default is MJPEG.", __FUNCTION__);
                pUsbhDevDes->bAppInterfaceNumber = 1;
            }

            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = WEB_CAM_SM;
            pSendMailDrv->wCurrentExecutionState    = WEB_CAM_UVC_SET_VIDEO_FORMAT_DONE;
            //webCamVideoSetupSetInterface(pUsbhDevDes->bAppInterfaceNumber, wAltInf4Stop, eWhichOtg);
        }
        break;

        case WEB_CAM_UVC_SET_VIDEO_FORMAT_DONE:
        { 
            webCamPeriodicFrameListInit(eWhichOtg);
            SetWebcamState(WEB_CAM_STATE_INIT_READY, eWhichOtg);
        }
        break;
        
        case WEB_CAM_UVC_SETCUR_EXTENSION:
        {
            BYTE bEntity = 3; // bit rate 1024kbps
            static BYTE cmdDataExt[SET_CUR_EXT_LEN] = {0, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0};

            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_PROBE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;

            webCamSetupSetCmd( (DWORD)cmdDataExt|0xa0000000,\
                                SET_CUR,\
                                byte_swap_of_word(VS_UPDATE_FRAME_SEGMENT_CONTROL),\
                                uchar_to_ushort(bEntity, 0),\
                                SET_CUR_EXT_LEN,\
                                eWhichOtg);
        }
        return;

        case WEB_CAM_UVC_SETCUR_PROBE: // WEB_CAM_VIDEO_CLASS_INIT_STATE0:
        { // SetCur(Probe)
        
            //UvcTestFunction0(eWhichOtg);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = WEB_CAM_SM;
            //pSendMailDrv->wCurrentExecutionState    = WEB_CAM_UVC_SETCUR_COMMIT; // WEB_CAM_VIDEO_CLASS_INIT_STATE1;
            pSendMailDrv->wCurrentExecutionState    = WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE;

            //psUsbHostAVdc->sVideoMjpegStream.dwStreamActive = TRUE;
            //psUsbHostAVdc->sAudioStream.dwStreamActive   = TRUE;
            memset(cmdData, 0, SET_CUR_LEN);
            PrepareDataSetCurProbe (cmdData,
                                    SET_CUR_LEN,
                                    psUsbHostAVdc->eVideoFormat,
                                    psUsbHostAVdc->dwVideoFrameSize,
                                    psUsbHostAVdc->dwFormatIndex,
                                    psUsbHostAVdc->dwFrameIndex);
            webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                                SET_CUR,\
                                byte_swap_of_word(VS_PROBE_CONTROL),\
                                pUsbhDevDes->bAppInterfaceNumber,\
                                SET_CUR_LEN,\
                                eWhichOtg);
        }
        return;

        case WEB_CAM_UVC_SETCUR_COMMIT: // WEB_CAM_VIDEO_CLASS_INIT_STATE1:
        { // SetCur(Commit)
            //UvcTestFunction0(eWhichOtg);

            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = WEB_CAM_SM;

            //if (psUsbHostAVdc->dwAudioStreamInterfaceNumber != 0)
            //	pSendMailDrv->wCurrentExecutionState    = WEB_CAM_AUDIO_CLASS_INIT_STATE0;
            //else
            pSendMailDrv->wCurrentExecutionState    = WEB_CAM_UVC_SET_INTERFACE; // WEB_CAM_IN_DONE_STATE

            memset(cmdData, 0, SET_CUR_LEN);
            PrepareDataSetCurCommit(cmdData,
                                    SET_CUR_LEN,
                                    psUsbHostAVdc->eVideoFormat,
                                    psUsbHostAVdc->dwVideoFrameSize,
                                    psUsbHostAVdc->dwFormatIndex,
                                    psUsbHostAVdc->dwFrameIndex);
            webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                                SET_CUR,\
                                byte_swap_of_word(VS_COMMIT_CONTROL),\
                                pUsbhDevDes->bAppInterfaceNumber,\
                                SET_CUR_LEN,\
                                eWhichOtg);
        }
        return;

        case WEB_CAM_UVC_SET_INTERFACE: // WEB_CAM_VIDEO_CLASS_INIT_STATE2
        {
            //UvcTestFunction0(eWhichOtg);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wStateMachine             = WEB_CAM_SM;
            //pSendMailDrv->wCurrentExecutionState    = WEB_CAM_UVC_ACTIVE_ISOC;
            pSendMailDrv->wCurrentExecutionState    = WEB_CAM_UVC_SETCUR_STILL_IMAGE_COMMIT;
            webCamVideoSetupSetInterface(pUsbhDevDes->bAppInterfaceNumber, pUsbhDevDes->bIsocAlternateSetting, eWhichOtg);
        }
        return;

        case WEB_CAM_UVC_ACTIVE_ISOC: // WEB_CAM_IN_DONE_STATE:
        {
            DWORD dwQueueElementByteCount = 0;
            MP_DEBUG("%s:%d:WEB_CAM_UVC_ACTIVE_ISOC", __func__, __LINE__);
            if (psUsbHostAVdc->eVideoFormat == USING_MPEG2_TS)
            {
                dwQueueElementByteCount = 128*0x1000;  // 512KB
                MP_ALERT("%s:%d:USING_MPEG2_TS:dwQueueElementByteCount %d", __FUNCTION__, __LINE__, dwQueueElementByteCount);
            }
            else if (psUsbHostAVdc->eVideoFormat == USING_MJPEG)
            {
                dwQueueElementByteCount = \   
                    (psUsbHostAVdc->dwVideoFrameSize >> 16) * (psUsbHostAVdc->dwVideoFrameSize & 0x0000FFFF) * 2;
					#if ISOC_EVERY_QUEUE_SIZE_FIXED
					dwQueueElementByteCount=ALIGN_32(dwQueueElementByteCount/5); //8->first cam  5->sony yang
					#endif
                MP_ALERT("%s:%d:USING_MJPEG %dx%d:dwQueueElementByteCount %d", __FUNCTION__, __LINE__, (psUsbHostAVdc->dwVideoFrameSize >> 16),(psUsbHostAVdc->dwVideoFrameSize & 0x0000FFFF),dwQueueElementByteCount);
            }
            else if (psUsbHostAVdc->eVideoFormat == USING_YUV)
            {
				#if ISOC_QUEUE_DYNAMIC
                dwQueueElementByteCount = \
                    (psUsbHostAVdc->dwVideoFrameSize >> 16) * (psUsbHostAVdc->dwVideoFrameSize & 0x0000FFFF) * 2;
				#else
                dwQueueElementByteCount = 0x60000; //384k
              #endif
                MP_ALERT("%s:%d:USING_YUV:dwQueueElementByteCount %d", __FUNCTION__, __LINE__, dwQueueElementByteCount);
            }
            else // USING_MJPEG by default
            {
                dwQueueElementByteCount = \
                    (psUsbHostAVdc->dwVideoFrameSize >> 16) * (psUsbHostAVdc->dwVideoFrameSize & 0x0000FFFF) * 2;
                MP_ALERT("%s:%d:USING_MJPEG:dwQueueElementByteCount %d", __FUNCTION__, __LINE__, dwQueueElementByteCount);
            }
		#if CHASE_IN_DECODE
				ST_IMGWIN  *pWin = Idu_GetNextWin();
				DWORD dwWidth=psUsbHostAVdc->dwVideoFrameSize>>16,dwHeight=psUsbHostAVdc->dwVideoFrameSize&0x0000ffff;

				#if USBCAM_TMP_SHARE_BUFFER
				if (g_pbTmpPicBuffer && g_dwPicBufferSize)
					pWin=Idu_GetUsbCamCacheWin();
				#endif
				if ((dwWidth<=pWin->wWidth)&&(dwHeight<=pWin->wHeight))
				{
					g_bNeedChase=0;
					SetDecodeWinOffset(pWin->dwOffset);
				}
				else //WebCamGetCaptureFlag
				{
					g_bNeedChase=1;
					SetDecodeWinOffset(0);
				}

              MP_ALERT("%s:g_bNeedChase=%d.", __FUNCTION__,g_bNeedChase);
		#endif
            
            SetQueueElementByteCount(dwQueueElementByteCount, eWhichOtg);
            pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = UsbOtgHostWebcamIsocInDataProcess;
            UsbOtgHostIsocEnable(eWhichOtg);
            SetWebcamState(WEB_CAM_STATE_BUSY, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_DONE_STATE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
        }
        break;

        case WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE:
        {
            MP_DEBUG("%s:%d:WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE", __func__, __LINE__);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE_DONE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            err = webCamSetupGetCmd (  (BYTE) GET_CUR,\
                                        byte_swap_of_word(VS_STILL_PROBE_CONTROL),\
                                        pUsbhDevDes->bAppInterfaceNumber,\
                                        STILL_IMAGE_CUR_LEN,\
                                        eWhichOtg);
            if(err != USB_NO_ERROR)
                MP_ALERT("%s:%d:err %d", __func__, __LINE__, err);
        }
        return;

        case WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE_DONE:
        {
            BYTE *pBuf;
            pBuf = (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;

            pUsbhDevDes->sUvcStillImageInfo.bFormatIndex       = pBuf[0];
            pUsbhDevDes->sUvcStillImageInfo.bFrameIndex        = pBuf[1];
            pUsbhDevDes->sUvcStillImageInfo.bCompressionIndex  = pBuf[2];
            pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize = \
                BYTE_TO_DWORD (pBuf[6], pBuf[5], pBuf[4], pBuf[3]);
            pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize = \
                BYTE_TO_DWORD (pBuf[10], pBuf[9], pBuf[8], pBuf[7]);

            MP_DEBUG("%d:stillImgInfo: %x %x %x %x %x",\ 
                __LINE__,\
                pUsbhDevDes->sUvcStillImageInfo.bFormatIndex,\
                pUsbhDevDes->sUvcStillImageInfo.bFrameIndex,\
                pUsbhDevDes->sUvcStillImageInfo.bCompressionIndex,\
                (pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize),\
                (pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize));
            
            free1((void*)pUsbhDevDes->sSetupPB.dwDataAddress, eWhichOtg);
            if (isSetCurStillImgProbeDone)
            {
                isSetCurStillImgProbeDone = FALSE;
                //pSendMailDrv->wCurrentExecutionState = WEB_CAM_END_STATE;
                pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_COMMIT;
            }
            else
            {
                pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_STILL_IMAGE_PROBE;
            }
            
            pSendMailDrv->wStateMachine              = WEB_CAM_SM;
        }
        break;

        case WEB_CAM_UVC_SETCUR_STILL_IMAGE_PROBE:
        {
            MP_DEBUG("%s:%d:WEB_CAM_UVC_SETCUR_STILL_IMAGE_PROBE", __func__, __LINE__);
            isSetCurStillImgProbeDone = TRUE;
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            memset(cmdData, 0, STILL_IMAGE_CUR_LEN);
            cmdData[0] = pUsbhDevDes->sUvcStillImageInfo.bFormatIndex;
            cmdData[1] = pUsbhDevDes->sUvcStillImageInfo.bFrameIndex;//3
            cmdData[2] = pUsbhDevDes->sUvcStillImageInfo.bCompressionIndex;
            cmdData[3] = lo_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[4] = midlo_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[5] = midhi_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[6] = hi_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            //cmdData[7] = hi_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            //cmdData[8] = midhi_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            //cmdData[9] = midlo_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            //cmdData[10]= lo_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            err = webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                                     SET_CUR,\
                                     byte_swap_of_word(VS_STILL_PROBE_CONTROL),\
                                     pUsbhDevDes->bAppInterfaceNumber,\
                                     STILL_IMAGE_CUR_LEN,\
                                     eWhichOtg);
            if(err != USB_NO_ERROR)
                MP_ALERT("%s:%d:err %d", __func__, __LINE__, err);
        }
        return;
        
        case WEB_CAM_UVC_SETCUR_STILL_IMAGE_COMMIT:
        {
            MP_DEBUG("%s:%d:WEB_CAM_UVC_SETCUR_STILL_IMAGE_COMMIT", __func__, __LINE__);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_ACTIVE_ISOC;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            memset(cmdData, 0, STILL_IMAGE_CUR_LEN);
				//1->640*360  2->800*600 3->800*600   4->512*384  5->640*480
            //mpDebugPrint("%s:%d:WEB_CAM_UVC_SETCUR_STILL_IMAGE_COMMIT bFrameIndex=%d", __func__, __LINE__,pUsbhDevDes->sUvcStillImageInfo.bFrameIndex);
			//	pUsbhDevDes->sUvcStillImageInfo.bFrameIndex=2;
						pUsbhDevDes->sUvcStillImageInfo.bFrameIndex=4;
            cmdData[0] = pUsbhDevDes->sUvcStillImageInfo.bFormatIndex;
            cmdData[1] = pUsbhDevDes->sUvcStillImageInfo.bFrameIndex;//3
            cmdData[2] = pUsbhDevDes->sUvcStillImageInfo.bCompressionIndex;
            cmdData[3] = lo_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[4] = midlo_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[5] = midhi_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[6] = hi_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxVideoFrameSize);
            cmdData[7] = lo_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            cmdData[8] = midlo_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            cmdData[9] = midhi_byte_of_dword(pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            cmdData[10]= hi_byte_of_dword   (pUsbhDevDes->sUvcStillImageInfo.dwMaxPayloadTransferSize);
            err = webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                                     SET_CUR,\
                                     byte_swap_of_word(VS_STILL_COMMIT_CONTROL),\
                                     pUsbhDevDes->bAppInterfaceNumber,\
                                     STILL_IMAGE_CUR_LEN,\
                                     eWhichOtg);
            if(err != USB_NO_ERROR)
                MP_ALERT("%s:%d:err %d", __func__, __LINE__, err);
        }
        return;

        case WEB_CAM_UVC_SETCUR_STILL_IMAGE_TRIGGER:
        {
            g_print_led = 1;
            MP_ALERT("%s:%d:g_print_led %d", __func__, __LINE__, g_print_led);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_END_STATE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
        }
        break;
 /*       
        case WEB_CAM_UVC_SETCUR_STILL_IMAGE_TRIGGER:
        {
            mpDebugPrint("%s:%d:WEB_CAM_UVC_SETCUR_STILL_IMAGE_TRIGGER", __func__, __LINE__);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_DONE_STATE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            memset(cmdData, 0, 1);
            cmdData[0] = 1;
            err = webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                                     SET_CUR,\
                                     byte_swap_of_word(VS_STILL_IMAGE_TRIGGER_CONTROL),\
                                     pUsbhDevDes->bAppInterfaceNumber,\
                                     1,\
                                     eWhichOtg);
            if(err != USB_NO_ERROR)
                MP_ALERT("%s:%d:err %d", __func__, __LINE__, err);
        }
        return;
*/        
        case WEB_CAM_UVC_STOP: // WEB_CAM_VIDEO_CLASS_DEINIT_STATE
        {
            //UsbOtgHostIsocDisable(eWhichOtg);
            //pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = NULL;;
            webCamVideoSetupSetInterface(pUsbhDevDes->bAppInterfaceNumber, wAltInf4Stop, eWhichOtg);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_STOP_DONE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            //UvcTestFunction0(eWhichOtg);
        }
        return;

        case WEB_CAM_UVC_STOP_DONE:
        {
            mpDebugPrint("%s:%d:WEB_CAM_UVC_STOP_DONE", __func__, __LINE__);
            UsbOtgHostIsocDisable(eWhichOtg);
            pUsbhDevDes->fpIsocInDataProcessForIsoInIoc = NULL;;
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            SetWebcamState(WEB_CAM_STATE_INIT_READY, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_DONE_STATE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
        }
        break;

        case WEB_CAM_DONE_STATE:
        {
            //UvcTestFunction0(eWhichOtg);
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_END_STATE;
            //pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE; // test still cap
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
        }
        break;

        case WEB_CAM_UVC_SETCUR_VAL_DONE:
        {
            MP_DEBUG("%s:%d:WEB_CAM_UVC_SETCUR_VAL_DONE", __func__, __LINE__);
            SetUvcRequestStatus(GetUvcPuControl(eWhichOtg), CMD_SET_CUR_DONE, eWhichOtg);
        }
        break;
        
        case WEB_CAM_UVC_GETCUR_VAL_DONE:
        {
            MP_DEBUG("%s:%d:WEB_CAM_UVC_GETCUR_VAL_DONE", __func__, __LINE__);
            SetUvcRequestStatus(GetUvcPuControl(eWhichOtg), CMD_GET_CUR_DONE, eWhichOtg);
        }
        break;
        
        case WEB_CAM_END_STATE:
        {
            MP_DEBUG("%s:%d:WEB_CAM_END_STATE", __func__, __LINE__);
        }
        break;

        default:
            mpDebugPrint("webCamStateMachine: state error %x!!", pReceiveMailDrv->wCurrentExecutionState);
        break;
    }

    if ((pReceiveMailDrv->wCurrentExecutionState == WEB_CAM_UVC_ACTIVE_ISOC) || \ 
        (pReceiveMailDrv->wCurrentExecutionState == WEB_CAM_UVC_STOP)        || \
        (pReceiveMailDrv->wCurrentExecutionState == WEB_CAM_UVC_STOP_DONE)   || \
        (pReceiveMailDrv->wCurrentExecutionState == WEB_CAM_INIT_GET_PU)   || \
        (pReceiveMailDrv->wCurrentExecutionState == WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE_DONE)   || \
        (pReceiveMailDrv->wCurrentExecutionState == WEB_CAM_DONE_STATE))// check next state
    {
_SEND_MAIL_:        
        //UartOutText("<s>");
        osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
        if (osSts != OS_STATUS_OK)
        {
            mpDebugPrint("webCamStateMachine: SendMailToUsbOtgHostClassTask failed!!");
        }
    }
}

void SetWebcamState(WEBCAM_STATE eState, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pWebcamDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    pWebcamDes->eWebcamState = eState;
}

WEBCAM_STATE GetWebcamState(WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pWebcamDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    return pWebcamDes->eWebcamState;
}

void SetUvcRequestStatus (UVC_PROCESSING_CONTROLS controls, UVC_PU_CONTROL_STATUS status, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psUsbhDevDes->psUvcReq[controls].wStatus = (WORD) status;
//    mpDebugPrint("%s:%d:%d", __func__, __LINE__, psUsbhDevDes->psUvcReq[controls].wStatus);
}

UVC_PU_CONTROL_STATUS GetUvcRequestStatus (UVC_PROCESSING_CONTROLS controls, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
//    mpDebugPrint("%s:%d:%d", __func__, __LINE__, psUsbhDevDes->psUvcReq[controls].wStatus);
    return psUsbhDevDes->psUvcReq[controls].wStatus;
}

void SetUvcPuControl (UVC_PROCESSING_CONTROLS controls, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psUsbhDevDes->eCurPuControl = controls;
}

UVC_PROCESSING_CONTROLS GetUvcPuControl (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return psUsbhDevDes->eCurPuControl;
}


WORD GetFormatIndexByFormat(USBH_WEBCAM_VIDEO_FORMAT eVideoFormat, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    BYTE i = 0;

    for (i = 0; i < MAX_NUM_VIDEO_FORMAT_INDEX; i++)
    {
        if (eVideoFormat == psUsbHostAVdc->sUvcFormatInfo[i].eFormat)
        {
            MP_DEBUG("%s:FormatIndex=%d", __func__,psUsbHostAVdc->sUvcFormatInfo[i].dwFormatIndex);
            return psUsbHostAVdc->sUvcFormatInfo[i].dwFormatIndex;
        }
    }

#if USBCAM_CHECK_INPUT_SET
	return MAX_NUM_VIDEO_FORMAT_INDEX;
#endif

    return 0;
}

DWORD GetFrameIndexByResolution(USBH_WEBCAM_VIDEO_FORMAT eVideoFormat, WORD wWidth, WORD wHigh, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    BYTE i = 0;
    BYTE j = 0;
    BYTE k = 0;

    psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

//    UvcTestFunction0(eWhichOtg);
    MP_DEBUG("%s:%d:psUsbHostAVdc 0x%x sUvcFormatInfo 0x%x eVideoFormat %d wWidth %d wHigh %d", __func__, __LINE__,\
            psUsbHostAVdc, psUsbHostAVdc->sUvcFormatInfo, eVideoFormat, wWidth, wHigh);
    for (i = 0; i < MAX_NUM_VIDEO_FORMAT_INDEX; i++)
    {
        MP_DEBUG("%s:%d:sUvcFormatInfo[%d].eFormat %d", __func__, __LINE__, i, psUsbHostAVdc->sUvcFormatInfo[i].eFormat);
        if (eVideoFormat == psUsbHostAVdc->sUvcFormatInfo[i].eFormat)
        {
            k = psUsbHostAVdc->sUvcFormatInfo[i].dwFormatIndex;
            MP_DEBUG("%s:%d:k %d", __func__, __LINE__, k);
            break;
        }
    }
#if USBCAM_TECH_MODE
	if (i>=MAX_NUM_VIDEO_FORMAT_INDEX)
	{
		mpDebugPrint("%s:%d:Error!!", __func__, __LINE__);
		return psUsbHostAVdc->sUvcFormatInfo[0].sFrameResolution[0].wFrameIndex;
	}
#endif

    for (j = 0; j < psUsbHostAVdc->sUvcFormatInfo[k-1].dwTotalFrameNumber; j++)
    {
        MP_DEBUG("bTotalFrameNumber %d wWidth %d wHigh %d",\
                    j,
                    psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wWidth,
                    psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wHigh);
        if ((wWidth == psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wWidth) &&\
            (wHigh == psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wHigh))
        {
            mpDebugPrint("%s:%d:FormatIndex=%d wFrameIndex %d", __func__, __LINE__, k,psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wFrameIndex);
            return psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wFrameIndex;
        }
    }

/*
    for (j = 0; j < MAX_NUM_FRAME_INDEX; j++)
    {
        mpDebugPrint("%s:%d:sFrameResolution[%d].wWidth %d sFrameResolution[%d].wHigh %d", __func__, __LINE__,\
            j, psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wWidth, j, psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wHigh);
        if ((wWidth == psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wWidth) &&\
            (wHigh == psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wHigh))
        {
            mpDebugPrint("%s:%d:wFrameIndex %d", __func__, __LINE__, psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wFrameIndex);
            return psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[j].wFrameIndex;
        }
    }
*/

#if USBCAM_TECH_MODE
mpDebugPrint("%s:%d:wFrameIndex FORCE to %d!!", __func__, __LINE__,psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[0].wFrameIndex);
	return psUsbHostAVdc->sUvcFormatInfo[k-1].sFrameResolution[0].wFrameIndex;
#else
__asm("break 100");
    return 0;
#endif
}

BOOL Api_UsbhCheckIfConnectedWebCam(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if((psUsbhDevDes->bConnectStatus == USB_STATUS_CONNECT) && 
        (psUsbhDevDes->sInterfaceDescriptor[psUsbhDevDes->bDeviceInterfaceIdx].bInterfaceClass == USB_CLASS_VIDEO))
        return TRUE;
    else
        return FALSE;
}

WEBCAM_STATE Api_UsbhGetWebCamState(WHICH_OTG eWhichOtg)
{
    return GetWebcamState(eWhichOtg);
}


void Api_UsbhRegistForIsoc(void *pFunctionPointer)
{
    WHICH_OTG eWhichOtg = GetUsbhPortSupportIsoc();
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psUsbhDevDes->fpParseIsocInData = pFunctionPointer;
    mpDebugPrint("%s:0x%x", __FUNCTION__, pFunctionPointer);
}


SDWORD Api_UsbhWebCamStart(WHICH_OTG eWhichOtg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD	osSts = USBOTG_NO_ERROR;

    mpDebugPrint("%s :%d", __FUNCTION__, GetWebcamState(eWhichOtg));
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
        return USBOTG_DEVICE_NOT_EXIST;

    switch (GetWebcamState(eWhichOtg)) 
    {
        case WEB_CAM_STATE_NOT_EXIST:
        {
            return USBOTG_DEVICE_NOT_EXIST;
        }
        break;
        
        case WEB_CAM_STATE_NOT_READY:
        {
            return USBOTG_DEVICE_NOT_READY;
        }
        break;
        
        case WEB_CAM_STATE_INIT_READY:
        {
			#if USBCAM_IN_ENABLE
			  st_bUsbCamPreStop=0;
			#endif
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
            //pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_EXTENSION;//WEB_CAM_VIDEO_CLASS_INIT_STATE0;
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_PROBE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;
            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
            if (osSts != OS_STATUS_OK)
            {
                mpDebugPrint("%s: SendMailToUsbOtgHostClassTask failed!!", __FUNCTION__);
                return USBOTG_MAILBOX_SEND_ERROR;
            }
			#if USBCAM_IN_ENABLE
			DWORD dwStartTime=GetSysTime();

			while (WEB_CAM_STATE_BUSY != GetWebcamState(eWhichOtg))
			{
				TaskYield();
				TaskSleep(10);
				if (SystemGetElapsedTime(dwStartTime)>8000)
					break;
			}
			#else
            while (WEB_CAM_STATE_BUSY != GetWebcamState(eWhichOtg))
            {
                TaskYield();
            }
			#endif
        }
        break;
        
        case WEB_CAM_STATE_BUSY:
        {
            return USBOTG_DEVICE_IS_BUSY;
        }
        break;
        default:
        {
            return USBOTG_UNKNOW_ERROR;
        }
        break;
    }
    
    MP_DEBUG("%s End:%d", __FUNCTION__, GetWebcamState(eWhichOtg));
    return USBOTG_NO_ERROR;
}

SDWORD Api_UsbhWebCamStop(WHICH_OTG eWhichOtg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    SDWORD	osSts = USBOTG_NO_ERROR;

#if CHASE_IN_DECODE
		g_bNeedChase=0;
#endif
#if SET_DECODE_OFFSET_ENALBE
		SetDecodeWinOffset(0);
#endif
#if CUT_IMAGE_HEIGHT_ENALBE
		SetDecodeWinHeight(0);
#endif
    mpDebugPrint("%s :%d", __FUNCTION__, GetWebcamState(eWhichOtg));

//    UvcTestFunction0(eWhichOtg);
    if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
    return USBOTG_DEVICE_NOT_EXIST;

    switch (GetWebcamState(eWhichOtg))
    {
        case WEB_CAM_STATE_NOT_EXIST:
        {
            return USBOTG_DEVICE_NOT_EXIST;
        }
        break;
        
        case WEB_CAM_STATE_NOT_READY:
        {
            return USBOTG_DEVICE_NOT_READY;
        }
        break;
        
        case WEB_CAM_STATE_INIT_READY:
        {
            return USBOTG_DEVICE_NOT_START_YET;
        }
        break;
        
        case WEB_CAM_STATE_BUSY:
        {
			#if USBCAM_IN_ENABLE
			  st_bUsbCamPreStop=1;
			#endif
            // it's able to call SendMailToUsbOtgHostClassTask
            pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
            pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_STOP; // WEB_CAM_VIDEO_CLASS_DEINIT_STATE;
            pSendMailDrv->wStateMachine          = WEB_CAM_SM;

            osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
            if (osSts != OS_STATUS_OK)
            {
                mpDebugPrint("%s: SendMailToUsbOtgHostClassTask failed!!", __FUNCTION__);
                return USBOTG_MAILBOX_SEND_ERROR;
            }

            while (WEB_CAM_STATE_INIT_READY != GetWebcamState(eWhichOtg))
            {
                TaskYield();
            };
        }
        break;
        
        default:
        {
            return USBOTG_UNKNOW_ERROR;
        }
        break;
    }

    MP_DEBUG("%s End:%d", __FUNCTION__, GetWebcamState(eWhichOtg));
    return USBOTG_NO_ERROR;
}

SDWORD Api_UsbhWebCamVedioFormat(WHICH_OTG eWhichOtg, 
                                 USBH_WEBCAM_VIDEO_FORMAT  eVideoFormat,
                                 USBH_UVC_FRAME_RESOLUTION sFrameResolution
                                 )
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    SDWORD sdwRet = USBOTG_NO_ERROR;
    WEBCAM_STATE eState;

    eState = GetWebcamState(eWhichOtg);
    if (WEB_CAM_STATE_NOT_EXIST == eState)
    {
        MP_ALERT("%s: WEB_CAM_STATE_NOT_EXIST!", __FUNCTION__);
        return WEB_CAM_STATE_NOT_EXIST;
    }
    
    if (WEB_CAM_STATE_NOT_READY == eState)
    {
        MP_ALERT("%s: USBOTG_DEVICE_NOT_READY!", __FUNCTION__);
        return USBOTG_DEVICE_NOT_READY;
    }

    MP_DEBUG("%s:%d:", __func__, __LINE__);
    //if (psUsbHostAVdc->eVideoFormat != eVideoFormat)
    {
        SDWORD	osSts = USBOTG_NO_ERROR;
        ST_MCARD_MAIL   *pSendMailDrv;

        MP_DEBUG("%s:%d:wWidth %d wHigh %d", __func__, __LINE__, sFrameResolution.wWidth, sFrameResolution.wHigh);
        psUsbHostAVdc->eVideoFormat = eVideoFormat;
        psUsbHostAVdc->dwVideoFrameSize = ((DWORD)sFrameResolution.wWidth << 16) | ((DWORD)sFrameResolution.wHigh);
        pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_WEB_CAM_SENDER, eWhichOtg);
        pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SET_VIDEO_FORMAT;
        pSendMailDrv->wStateMachine          = WEB_CAM_SM;
        MP_DEBUG("%s:%d:dwVideoFrameSize = 0x%x", __func__, __LINE__, psUsbHostAVdc->dwVideoFrameSize);
        
        osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
        if (osSts != OS_STATUS_OK)
        {
            MP_ALERT("%s: SendMailToUsbOtgHostClassTask failed!!", __FUNCTION__);
            return USBOTG_MAILBOX_SEND_ERROR;
        }

        while (WEB_CAM_STATE_INIT_READY != GetWebcamState(eWhichOtg))
        {
            TaskYield();
        };
    }
    //else
    //{
    ///    MP_ALERT("%s:the setting is the same as original.", __FUNCTION__);
    //}
//
// here need to get Format and frame inex from a function with the two paremeters eVideoFormat and sFrameResolution
// write this functio later on 2/15, 2016
//

#if USBCAM_CHECK_INPUT_SET
	if (GetFormatIndexByFormat(psUsbHostAVdc->eVideoFormat, eWhichOtg)>=MAX_NUM_VIDEO_FORMAT_INDEX)
	{
       MP_ALERT("%s: Force eVideoFormat from %d to %d!!!", __FUNCTION__,psUsbHostAVdc->eVideoFormat,psUsbHostAVdc->sUvcFormatInfo[0].eFormat);
		psUsbHostAVdc->eVideoFormat=psUsbHostAVdc->sUvcFormatInfo[0].eFormat;
	}
#endif

    
    if (psUsbHostAVdc->eVideoFormat == USING_MPEG2_TS)
    {
        psUsbHostAVdc->dwFormatIndex = 1;
        psUsbHostAVdc->dwFrameIndex  = 1;
    }
    else if (psUsbHostAVdc->eVideoFormat == USING_MJPEG)
    {
        psUsbHostAVdc->dwFormatIndex = GetFormatIndexByFormat(psUsbHostAVdc->eVideoFormat, eWhichOtg);
        psUsbHostAVdc->dwFrameIndex  = GetFrameIndexByResolution(psUsbHostAVdc->eVideoFormat, sFrameResolution.wWidth, sFrameResolution.wHigh, eWhichOtg);
    }
    else if (psUsbHostAVdc->eVideoFormat == USING_YUV)
    {
        psUsbHostAVdc->dwFormatIndex = GetFormatIndexByFormat(psUsbHostAVdc->eVideoFormat, eWhichOtg);
        psUsbHostAVdc->dwFrameIndex  = GetFrameIndexByResolution(psUsbHostAVdc->eVideoFormat, sFrameResolution.wWidth, sFrameResolution.wHigh, eWhichOtg);
    }
    else
    {
        sdwRet = USBOTG_PARAMETER_ERROR;
    }

    if ((psUsbHostAVdc->dwFormatIndex == 0) || (psUsbHostAVdc->dwFrameIndex == 0))
    {
        sdwRet = USBOTG_PARAMETER_ERROR;
    }
#if USBCAM_CHECK_INPUT_SET
	if (sdwRet == USBOTG_NO_ERROR)
	{
        psUsbHostAVdc->dwVideoFrameSize = ((DWORD)psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwFormatIndex-1].sFrameResolution[psUsbHostAVdc->dwFrameIndex-1].wWidth << 16) | ((DWORD)psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwFormatIndex-1].sFrameResolution[psUsbHostAVdc->dwFrameIndex-1].wHigh);
	}
#endif

    return sdwRet;
}

PUVC_BM_PROCESSING_UNIT Api_UsbhWebCamProcessUnitCap(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;
    WORD        wUnitId = 0;
    WORD        wControlSelectors;

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    wControlSelectors = (WORD) psUsbhDevDes->psUvcReq[0].bControlSelector;
    wUnitId           = (WORD) psUsbhDevDes->sUvcProcessUnit.bUnitID;
    MP_DEBUG("%s:%d:psUsbhDevDes 0x%x psUvcReq[0] 0x%x sUvcProcessUnit 0x%x wUnitId %d wControlSelectors %d wStatus %d", \
        __func__, __LINE__, psUsbhDevDes, &psUsbhDevDes->psUvcReq[0], &psUsbhDevDes->sUvcProcessUnit, wUnitId, wControlSelectors,\
        psUsbhDevDes->psUvcReq[0].wStatus);
    return &(psUsbhDevDes->sUvcProcessUnit.sUnit);
}

PUVC_GET_REQUEST_OF_PROCESSING Api_UsbhWebCamProcessUnitControls(UVC_PROCESSING_CONTROLS controls, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    return &(psUsbhDevDes->psUvcReq[controls]);
}

SWORD Api_UsbhWebCamPuControlsGetVal(WORD* pVal, UVC_PROCESSING_CONTROLS controls, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;
    ST_MCARD_MAIL   *pSendMailDrv;
    WORD        wUnitId = 0;
    WORD        wControlSelectors;
    WORD        len = 0;
    SWORD       err = USB_NO_ERROR;

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_GETCUR_VAL_DONE;
    pSendMailDrv->wStateMachine          = WEB_CAM_SM;

    len = 2;
    wControlSelectors = (WORD) psUsbhDevDes->psUvcReq[controls].bControlSelector;
    wUnitId           = (WORD) psUsbhDevDes->sUvcProcessUnit.bUnitID;
    MP_DEBUG("%s:%d:psUsbhDevDes 0x%x psUvcReq[%d] 0x%x sUvcProcessUnit 0x%x wUnitId %d wControlSelectors %d wStatus %d", \
        __func__, __LINE__, psUsbhDevDes, controls, &psUsbhDevDes->psUvcReq[controls], &psUsbhDevDes->sUvcProcessUnit, wUnitId, wControlSelectors,\
        psUsbhDevDes->psUvcReq[controls].wStatus);
    SetUvcRequestStatus(controls, CMD_GET_CUR, eWhichOtg);
    SetUvcPuControl(controls, eWhichOtg);
    err = webCamSetupGetCmd (  (BYTE) GET_CUR,\
                               (WORD) byte_swap_of_word(wControlSelectors),\
                               (WORD) byte_swap_of_word(wUnitId),\
                               len,\
                               eWhichOtg);
    if(err != USB_NO_ERROR)
    {
        MP_ALERT("-E- %s:%d", __func__, __LINE__);
        return err;
    }

    while (CMD_GET_CUR_DONE != GetUvcRequestStatus(controls, eWhichOtg))
    {
        TaskYield();
    };

    memcpy ( (BYTE*)pVal, (BYTE*)psUsbhDevDes->sSetupPB.dwDataAddress, len);
//    mpDebugPrint("%s:%d:%d", __func__, __LINE__, byte_swap_of_word(*pVal));

    *pVal = byte_swap_of_word(*pVal);
    
    return err;
}

void Api_UsbhWebCamPuControlsSetVal(WORD val,  UVC_PROCESSING_CONTROLS controls, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;
    ST_MCARD_MAIL   *pSendMailDrv;
    WORD        wUnitId = 0;
    WORD        wControlSelectors;
    WORD        len = 0;
	static BYTE cmdData[] = {0x00, 0x00};

    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_VAL_DONE;
    pSendMailDrv->wStateMachine          = WEB_CAM_SM;

    SetUvcRequestStatus(controls, CMD_SET_CUR, eWhichOtg);
    SetUvcPuControl(controls, eWhichOtg);
    len = 2;
    wControlSelectors = (WORD) psUsbhDevDes->psUvcReq[controls].bControlSelector;
    wUnitId           = (WORD) psUsbhDevDes->sUvcProcessUnit.bUnitID;
    MP_DEBUG("%s:%d:psUsbhDevDes 0x%x psUvcReq[%d] 0x%x sUvcProcessUnit 0x%x wUnitId %d wControlSelectors %d wStatus %d", \
        __func__, __LINE__, psUsbhDevDes, controls, &psUsbhDevDes->psUvcReq[controls], &psUsbhDevDes->sUvcProcessUnit, wUnitId, wControlSelectors,\
        psUsbhDevDes->psUvcReq[controls].wStatus);
	cmdData[1]        = ((BYTE *) &val)[0];
	cmdData[0]        = ((BYTE *) &val)[1];
    webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                        SET_CUR,\
                        byte_swap_of_word(wControlSelectors),\
                        byte_swap_of_word(wUnitId),\
                        len,\
                        eWhichOtg);
}

SWORD Api_UsbhWebCamStillImageTrigger(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes = NULL;
    ST_MCARD_MAIL   *pSendMailDrv;
    WORD            len = 0;
	static BYTE            cmdData[] = {0x00, 0x00};   
    SWORD           err;

//    g_print_led = 1;
    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wCurrentExecutionState = WEB_CAM_UVC_SETCUR_STILL_IMAGE_TRIGGER;//WEB_CAM_DONE_STATE;
    pSendMailDrv->wStateMachine          = WEB_CAM_SM;

    len = 1;
	cmdData[0] = 1;
    mpDebugPrint("%s:%d:cmdData 0x%x cmdData[0] 0x%x ", __func__, __LINE__, cmdData, cmdData[0]);
    err = webCamSetupSetCmd( (DWORD)cmdData|0xa0000000,\
                             SET_CUR,\
                             byte_swap_of_word(VS_STILL_IMAGE_TRIGGER_CONTROL),\
                             psUsbhDevDes->bAppInterfaceNumber,\
                             len,\
                             eWhichOtg);
    if(err != USB_NO_ERROR)
        MP_ALERT("%s:%d:err %d", __func__, __LINE__, err);

    return err;
}

void UvcTestFunction7(void)
{
	SWORD err;
	WHICH_OTG eWhichOtg=WEBCAM_USB_PORT;
    err = Api_UsbhWebCamStillImageTrigger(eWhichOtg);
    if(err != USBOTG_NO_ERROR)
        MP_ALERT("%s:%d:err %d", __func__, __LINE__, err);
    
}

void TimerToUvcCapture(void)
{
    MP_ALERT("%s:%d:add timer function", __func__, __LINE__);
	Ui_TimerProcAdd(10000, UvcTestFunction7);
}

void UvcTestFunction6 (WHICH_OTG eWhichOtg)
{
    UVC_PROCESSING_CONTROLS controls;
    WORD val;
    SWORD err = USB_NO_ERROR;

    controls = PROCESSING_BRIGHTNESS;
    err = Api_UsbhWebCamPuControlsGetVal(&val, controls, eWhichOtg);
    if (err != USB_NO_ERROR )
    {
        return;
    }

	if (val<10)
		return;
    mpDebugPrint("Brightness GetValue = %d", val);

    val -= 10;
    Api_UsbhWebCamPuControlsSetVal(val, controls, eWhichOtg);
    mpDebugPrint("Brightness SetValue = %d", val);
}

void UvcSetContrast(WHICH_OTG eWhichOtg)
{
    UVC_PROCESSING_CONTROLS controls;
    WORD val;
    SWORD err = USB_NO_ERROR;

    controls = PROCESSING_HUE;
    err = Api_UsbhWebCamPuControlsGetVal(&val, controls, eWhichOtg);
    if (err != USB_NO_ERROR )
    {
        return;
    }

	if (val>250)
		return;
    mpDebugPrint("Contrast GetValue = %d", val);

    val += 5;
    Api_UsbhWebCamPuControlsSetVal(val, controls, eWhichOtg);
    mpDebugPrint("Contrast SetValue = %d", val);
}

WORD UvcGetSensorColor(UVC_PROCESSING_CONTROLS controls)
{
	WORD val;
	SWORD err = USB_NO_ERROR;

	err = Api_UsbhWebCamPuControlsGetVal(&val, controls, WEBCAM_USB_PORT);
	if (err != USB_NO_ERROR )
	{
		val=120;
	}

	return val;
}

void UvcSetSensorColor(UVC_PROCESSING_CONTROLS controls,WORD val)
{
    Api_UsbhWebCamPuControlsSetVal(val, controls, WEBCAM_USB_PORT);
}

void UvcTestFunction5 (WHICH_OTG eWhichOtg)
{
    PUVC_GET_REQUEST_OF_PROCESSING  psGetRequest; 
    PUVC_BM_PROCESSING_UNIT         psUnitCap;

    psUnitCap = Api_UsbhWebCamProcessUnitCap(eWhichOtg);
    
    mpDebugPrint("========================================================");
    mpDebugPrint("-- Brightness --");
    if (psUnitCap->bmBrightness)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_BRIGHTNESS, eWhichOtg);
        
        mpDebugPrint("Brightness SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Brightness GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Brightness Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Brightness bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Brightness Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Brightness Min = %d", psGetRequest->wMin);
        mpDebugPrint("Brightness Max = %d", psGetRequest->wMax);
        mpDebugPrint("Brightness Res = %d", psGetRequest->wRes);
        mpDebugPrint("Brightness Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Brightness Not support!");
    }

    mpDebugPrint("========================================================");
    mpDebugPrint("-- Contrast --");
    if (psUnitCap->bmContrast)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_CONTRAST, eWhichOtg);
        
        mpDebugPrint("Contrast SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Contrast GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Contrast Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Contrast bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Contrast Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Contrast Min = %d", psGetRequest->wMin);
        mpDebugPrint("Contrast Max = %d", psGetRequest->wMax);
        mpDebugPrint("Contrast Res = %d", psGetRequest->wRes);
        mpDebugPrint("Contrast Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Contrast Not support!");
    }

    mpDebugPrint("========================================================");
    mpDebugPrint("-- Hue --");
    if (psUnitCap->bmHue)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_HUE, eWhichOtg);
        
        mpDebugPrint("Hue SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Hue GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Hue Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Hue bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Hue Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Hue Min = %d", psGetRequest->wMin);
        mpDebugPrint("Hue Max = %d", psGetRequest->wMax);
        mpDebugPrint("Hue Res = %d", psGetRequest->wRes);
        mpDebugPrint("Hue Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Hue Not support!");
    }
    
    mpDebugPrint("========================================================");
    mpDebugPrint("-- Saturation --");
    if (psUnitCap->bmSaturation)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_SATURATION, eWhichOtg);
        
        mpDebugPrint("Saturation SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Saturation GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Saturation Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Saturation bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Saturation Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Saturation Min = %d", psGetRequest->wMin);
        mpDebugPrint("Saturation Max = %d", psGetRequest->wMax);
        mpDebugPrint("Saturation Res = %d", psGetRequest->wRes);
        mpDebugPrint("Saturation Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Saturation Not support!");
    }

    mpDebugPrint("========================================================");
    mpDebugPrint("-- Sharpness --");
    if (psUnitCap->bmSharpness)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_SHARPNESS, eWhichOtg);
        
        mpDebugPrint("Sharpness SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Sharpness GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Sharpness Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Sharpness bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Sharpness Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Sharpness Min = %d", psGetRequest->wMin);
        mpDebugPrint("Sharpness Max = %d", psGetRequest->wMax);
        mpDebugPrint("Sharpness Res = %d", psGetRequest->wRes);
        mpDebugPrint("Sharpness Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Saturation Not support!");
    }

    mpDebugPrint("========================================================");
    mpDebugPrint("-- Gamma --");
    if (psUnitCap->bmGamma)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_GAMMA, eWhichOtg);
        
        mpDebugPrint("Gamma SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Gamma GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Gamma Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Gamma bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Gamma Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Gamma Min = %d", psGetRequest->wMin);
        mpDebugPrint("Gamma Max = %d", psGetRequest->wMax);
        mpDebugPrint("Gamma Res = %d", psGetRequest->wRes);
        mpDebugPrint("Gamma Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Gamma Not support!");
    }

    mpDebugPrint("========================================================");
    mpDebugPrint("-- White Balance Temperature --");
    if (psUnitCap->bmWbt)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_WHITE_BALANCE_TEMPERATURE, eWhichOtg);
        
        mpDebugPrint("Wbt SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Wbt GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Wbt Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Wbt bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Wbt Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Wbt Min = %d", psGetRequest->wMin);
        mpDebugPrint("Wbt Max = %d", psGetRequest->wMax);
        mpDebugPrint("Wbt Res = %d", psGetRequest->wRes);
        mpDebugPrint("Wbt Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("White Balance Temperature Not support!");
    }
    
    mpDebugPrint("========================================================");
    mpDebugPrint("-- White Balance Component --");
    if (psUnitCap->bmWbc)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_WHITE_BALANCE_COMPONENT, eWhichOtg);
        
        mpDebugPrint("Wbc SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Wbc GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Wbc Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Wbc bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Wbc Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Wbc Min = %d", psGetRequest->wMin);
        mpDebugPrint("Wbc Max = %d", psGetRequest->wMax);
        mpDebugPrint("Wbc Res = %d", psGetRequest->wRes);
        mpDebugPrint("Wbc Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Wbc Not support!");
    }
        
    mpDebugPrint("========================================================");
    mpDebugPrint("-- BacklightCompensation --");
    if (psUnitCap->bmBacklightCompensation)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_BACKLIGHT_COMPENSATON, eWhichOtg);
        
        mpDebugPrint("BacklightCompensation SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("BacklightCompensation GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("BacklightCompensation Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("BacklightCompensation bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("BacklightCompensation Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("BacklightCompensation Min = %d", psGetRequest->wMin);
        mpDebugPrint("BacklightCompensation Max = %d", psGetRequest->wMax);
        mpDebugPrint("BacklightCompensation Res = %d", psGetRequest->wRes);
        mpDebugPrint("BacklightCompensation Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("BacklightCompensation Not support!");
    }
        
    mpDebugPrint("========================================================");
    mpDebugPrint("-- Gain --");
    if (psUnitCap->bmGain)
    {
        psGetRequest = Api_UsbhWebCamProcessUnitControls(PROCESSING_GAIN, eWhichOtg);
        
        mpDebugPrint("Gain SetValue = %d", psGetRequest->sInfo.bmSetValue);
        mpDebugPrint("Gain GetValue = %d", psGetRequest->sInfo.bmGetValue);
        mpDebugPrint("Gain Disable(Auto mode) = %d", psGetRequest->sInfo.bmDisable);
        mpDebugPrint("Gain bmAutoupdate = %d", psGetRequest->sInfo.bmAutoupdate);
        mpDebugPrint("Gain Asynchronous = %d", psGetRequest->sInfo.bmAsynchronous);
        
        mpDebugPrint("Gain Min = %d", psGetRequest->wMin);
        mpDebugPrint("Gain Max = %d", psGetRequest->wMax);
        mpDebugPrint("Gain Res = %d", psGetRequest->wRes);
        mpDebugPrint("Gain Def = %d", psGetRequest->wDef);
    }
    else
    {
        mpDebugPrint("Gain Not support!");
    }
    
    mpDebugPrint("========================================================");
    
}


void UvcTestFunction (WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    
    pUvcFormatInfo = Api_UsbhWebCamVideoGetForamtInfo(eWhichOtg);
/// check sUvcFormatInfo
    int i = 0;
    int j = 0;
    
    mpDebugPrint("%s:%d:Check sUvcFormatInfo Begin", __func__, __LINE__);
    while (WEB_CAM_STATE_INIT_READY != GetWebcamState(eWhichOtg))
    {
        TaskYield();
    };
    
    for (i = 0; i < 3; i++)
    {
        mpDebugPrint("sUvcFormatInfo[%d]:bFormat %d bTotalFrameNumber %d",\
                    i,
                    psUsbHostAVdc->sUvcFormatInfo[i].eFormat,
                    psUsbHostAVdc->sUvcFormatInfo[i].dwTotalFrameNumber);
        for (j = 0; j < psUsbHostAVdc->sUvcFormatInfo[i].dwTotalFrameNumber; j++)
        {
            mpDebugPrint("bTotalFrameNumber %d wWidth %d wHigh %d",\
                        j,
                        psUsbHostAVdc->sUvcFormatInfo[i].sFrameResolution[j].wWidth,
                        psUsbHostAVdc->sUvcFormatInfo[i].sFrameResolution[j].wHigh);
        }
    }
    mpDebugPrint("%s:%d:Check sUvcFormatInfo End", __func__, __LINE__);
////
   mpDebugPrint("%s:%d:Test MJPEG 320x240 Begin", __func__, __LINE__);
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    sFrameRes.wWidth = 320;
    sFrameRes.wHigh  = 240;
    //Api_UsbhWebCamVedioFormat(eWhichOtg, USING_YUV, sFrameRes);
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);

    mpDebugPrint("%s:%d:Test MJPEG 320x240 End", __func__, __LINE__);
}

void UvcTestFunction0 (WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    
    pUvcFormatInfo = Api_UsbhWebCamVideoGetForamtInfo(eWhichOtg);
/// check sUvcFormatInfo
    int i = 0;
    int j = 0;
    
    mpDebugPrint("%s:%d:Check sUvcFormatInfo Begin", __func__, __LINE__);
    
    for (i = 0; i < 3; i++)
    {
        mpDebugPrint("sUvcFormatInfo[%d]:bFormat %d bTotalFrameNumber %d",\
                    i,
                    psUsbHostAVdc->sUvcFormatInfo[i].eFormat,
                    psUsbHostAVdc->sUvcFormatInfo[i].dwTotalFrameNumber);
        for (j = 0; j < psUsbHostAVdc->sUvcFormatInfo[i].dwTotalFrameNumber; j++)
        {
            mpDebugPrint("bTotalFrameNumber %d wWidth %d wHigh %d",\
                        j,
                        psUsbHostAVdc->sUvcFormatInfo[i].sFrameResolution[j].wWidth,
                        psUsbHostAVdc->sUvcFormatInfo[i].sFrameResolution[j].wHigh);
        }
    }
    mpDebugPrint("%s:%d:Check sUvcFormatInfo End", __func__, __LINE__);
}

void UvcTestFunction2 (WHICH_OTG eWhichOtg)
{
	/*
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 1280;//640;
    sFrameRes.wHigh  = 720;//480;
 //   Api_UsbhWebCamVedioFormat(eWhichOtg, USING_YUV, sFrameRes);
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    Api_UsbhWebCamStart(eWhichOtg);
//    mpDebugPrint("%s:%d:Test YUV 640x480 End", __func__, __LINE__);
    g_test_res = 1;

    while (!(g_test_res == 2))
    {
        TaskYield();
    };
    
   g_test_res = 0;
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 640;
    sFrameRes.wHigh  = 480;
   Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    Api_UsbhWebCamStart(eWhichOtg);
   mpDebugPrint("%s:%d:Done", __func__, __LINE__);
   */
}

void UvcTestFunction4 (WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

    
    mpDebugPrint("%s:%d:Test MJPEG 640x480 Begin", __func__, __LINE__);
    //UvcTestFunction0(eWhichOtg);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 640;
    sFrameRes.wHigh  = 480;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    Api_UsbhWebCamStart(eWhichOtg);
    mpDebugPrint("%s:%d:Test MJPEG 640x480 End", __func__, __LINE__);
}

void UvcTestFunction3 (WHICH_OTG eWhichOtg)
{
    PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSBH_UVC_FORMAT_INFORMATION pUvcFormatInfo;
    USBH_UVC_FRAME_RESOLUTION sFrameRes;
    int i = 0;

//    for (i = 0; i < 5000; i++)
//    {
//        TaskYield();
//    }
    
    mpDebugPrint("%s:%d:back to MJPEG 320x240 Begin", __func__, __LINE__);
    Api_UsbhWebCamStop(eWhichOtg);
    sFrameRes.wWidth = 320;
    sFrameRes.wHigh  = 240;
    Api_UsbhWebCamVedioFormat(eWhichOtg, USING_MJPEG, sFrameRes);
    Api_UsbhWebCamStart(eWhichOtg);
    mpDebugPrint("%s:%d:back to MJPEG 320x240 End", __func__, __LINE__);
}
// JL, 10032011
#if 0
//#elif (SC_USBHOST && USB_HOST_ISO_TEST)
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "usbotg_host.h"
#include "usbotg_host_sm.h"
#include "usbotg_host_setup.h"
#include "usbotg_ctrl.h"
#include "taskid.h"
#include "os.h"
#include "devio.h"
#include "ui.h"

/////////////////////////// ISO ////////////////////////////////
#define HOST20_iTD_Status_Active              0x08
#define OTGH_PT_ISO_DATABUFFER_NUM                10
#define OTGH_Dir_IN 	                         0x01
#define OTGH_Dir_Out 	                         0x00	   

extern USB_OTG *gp_UsbOtg;

#define mwHost20_FrameIndex14Bit_Rd()                     (psUsbOtg->psUsbReg->HcFrameIndex&0x00003FFF)//(gp_UsbOtg->HcFrameIndex&0x00003FFF) 	//Only Read Bit0~Bit12(Skip Bit 13)     	
#define mwHost20_FrameIndex_Rd()		                  (psUsbOtg->psUsbReg->HcFrameIndex&0x00001FFF) 	//Only Read Bit0~Bit12(Skip Bit 13)     	
#define mwHost20_PeriodicBaseAddr_Set(wValue)		      (psUsbOtg->psUsbReg->HcPeriodicFrameListBaseAddress=wValue) 	

typedef unsigned char		UINT8;
typedef unsigned short      UINT16;
typedef unsigned long       UINT32;

typedef struct
{	
	UINT32      aDataBufferArray[OTGH_PT_ISO_DATABUFFER_NUM];//Max support 600*4K=2.4M
}OTGH_PT_ISO_Struct;

//volatile Host20_Attach_Device_Structure  sAttachDevice;
volatile UINT32 gwLastiTDSendOK;

OTGH_PT_ISO_Struct sOTGH_PT_ISO;
UINT32 wRandomFrameStart = 200;
UINT32 wISOOTGIn_Counter=0;

static ST_USBH_DEVICE_DESCRIPTOR Device_AP;

OTGH_PT_ISO_Struct sOTGH_PT_ISO;


extern BYTE GetDeviceAddress(WHICH_OTG eWhichOtg);

//====================================================================
// * Function Name: flib_Host20_ISO_Init                          
// * Description: 
//   <1>.Init FrameList
//   <2>.Enable Periotic schedule
//
// * Input: 
// * OutPut: 
//====================================================================
void  flib_Host20_ISO_Init (WHICH_OTG eWhichOtg)
{
	UINT32 i;
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

	flib_Host20_Asynchronous_Setting(HOST20_Disable, eWhichOtg);
	Device_AP.bAdd = GetDeviceAddress(eWhichOtg);

	// <1>.Init FrameList
	for (i=0;i<PERIODIC_FRAME_SIZE;i++)      
	{
		psUsbHostEhci->psHostFramList->sCell[i].bLinkPointer = 0;
		psUsbHostEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_iTD;     
		psUsbHostEhci->psHostFramList->sCell[i].bTerminal    = 1;
	}
	mbHost20_USBCMD_FrameListSize_Set(2);

	//<2>.Set Periodic Base Address	
	mwHost20_PeriodicBaseAddr_Set(psUsbHostEhci->dwHostStructurePflBaseAddress);	

	//<2>.Enable the periodic 
	flib_Host20_Periodic_Setting(HOST20_Enable, eWhichOtg);
}


void flib_Host20_Issue_ISO(UINT8 bCheckResult,UINT32 wEndPt,UINT32 wMaxPacketSize,UINT32 wSize
                            ,UINT32 *pwBufferArray,UINT32 wOffset,UINT8 bDirection,UINT8 bMult, WHICH_OTG eWhichOtg)
{
	iTD_Structure  *spTempiTD;
	UINT32 *pwLastTransaction;
	UINT32 wRemainSize,wCurrentOffset,wCurrentBufferNum,wCurrentLength,wiTDNum,wFrameNumber,i,wOriginalFrameNumber,wDummyTemp;
	UINT32 wCurrentBufferNum1,wCurrentBufferNum3,j,wCurrentBufferNum2,wCurrentBufferNum4;
	UINT8  bCurrentTransactionNum,bCurrentPageNum,bTransactionNumMax,bExitFlag;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

	UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)(UINT32)*pwBufferArray, (DWORD)((DWORD)(UINT32)*pwBufferArray) + wSize, eWhichOtg);
	MP_DEBUG("flib_Host20_Issue_ISO");

	//<2>.Allocate the iTD for the Data Buffer
	wRemainSize=wSize;
	wCurrentBufferNum=0;
	wiTDNum=0;
	wCurrentOffset=wOffset;
	while(wRemainSize)
	{
		//<2.1>.Allocate iTD
		Device_AP.wISOiTDAddress[wiTDNum]=flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
		spTempiTD=Device_AP.wISOiTDAddress[wiTDNum];
		spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction|=(wEndPt<<8);
		spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction|=GetDeviceAddress(eWhichOtg);
		spTempiTD->ArrayBufferPointer_Word[1].bMultiFunction=(bDirection<<11);
		spTempiTD->ArrayBufferPointer_Word[1].bMultiFunction|=wMaxPacketSize;
		spTempiTD->ArrayBufferPointer_Word[2].bMultiFunction=bMult;

		bCurrentTransactionNum=0;
		bCurrentPageNum=0;

		spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer = ((UINT32)*(pwBufferArray+wCurrentBufferNum)>>12);   
		bTransactionNumMax = 8;//For High Speed

		//<2.2>.Fill iTD
		while ((wRemainSize)&&(bCurrentTransactionNum<bTransactionNumMax))
		{
			//Fill iTD
			if (wRemainSize<(wMaxPacketSize*bMult))
				wCurrentLength = wRemainSize;
			else 
				wCurrentLength=wMaxPacketSize*bMult;

			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bLength=wCurrentLength;
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus=HOST20_iTD_Status_Active;	
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete=0;	
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bPageSelect=bCurrentPageNum;	
			spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bOffset=wCurrentOffset;
			//Maintain the wRemainSize/bCurrentPageNum/wCurrentOffset/wCurrentBufferNum         
			wRemainSize=wRemainSize-wCurrentLength;
			wCurrentOffset=wCurrentOffset+wCurrentLength;
			if (wCurrentOffset>=4096)
			{
				bCurrentPageNum++;	
				wCurrentBufferNum++;
				spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer
							=((UINT32)*(pwBufferArray+wCurrentBufferNum)>>12);   

				wCurrentOffset=wCurrentOffset-4096;	
			} 

			//Set the finish Complete-Interrupt
			if (wRemainSize==0)
			{
				spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete=1;	
				pwLastTransaction=&(spTempiTD->ArrayStatus_Word[bCurrentTransactionNum]);
			}

			bCurrentTransactionNum++;     
		}//while ((wRemainSize)&&(bCurrentTransactionNum<8))

		//<2.3>.Maintain Variable
		wiTDNum++;
		if (wiTDNum>512)
		{
			mpDebugPrint(">>> Waring...iTD Number >512..."); 	
			while(1);
		}
	} //while(wRemainSize)

	//<3>.Hang the qTD to the FrameList
	//<3.1>.Read the Current Frame List

	//Critical Time Period Start ---------------------------------------------------        

	wCurrentBufferNum1=mwHost20_FrameIndex14Bit_Rd()>>3;
	wOriginalFrameNumber=(mwHost20_FrameIndex_Rd()>>3);

	wOriginalFrameNumber &= 0xff;
	wOriginalFrameNumber=wOriginalFrameNumber+10;//300
	//<3.2>.Hang the qTD to the Current Frame + 100 ms
	wCurrentBufferNum2=mwHost20_FrameIndex14Bit_Rd()>>3;           //wCurrentBufferNum2      
	wFrameNumber=wOriginalFrameNumber;
	Device_AP.wOriginalFrameNumber=wOriginalFrameNumber;
	for (i=0;i<wiTDNum;i++)      
	{
		if (wFrameNumber>255)
			wFrameNumber=wFrameNumber-256;
		psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bLinkPointer=(UINT32)(Device_AP.wISOiTDAddress[i])>>5;
		psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bTerminal=0;             
		wFrameNumber++;
	}
	Device_AP.bMaxIniTDNum = wiTDNum;
}

UINT32 wTxRxLength=0;
UINT32 isoLoopCnt=1;
BYTE    *pbuf_iso_w = 0;
BYTE    *pbuf_iso_r = 0;
#define TEST_SECTOR_CNT 512
#define TEST_BYTE_CNT   512 * TEST_SECTOR_CNT
void OTGH_PT_ISO(UINT8 dir, WHICH_OTG eWhichOtg)
{
	UINT32 wMaxPacketSize,wOffset,i;
	UINT8  bMult,bTemp;

	MP_DEBUG("OTGH_PT_ISO");

	if (pbuf_iso_w == 0)
	{
		pbuf_iso_w = (BYTE *)ext_mem_malloc((TEST_BYTE_CNT+0x6000));
		pbuf_iso_r = (BYTE *)ext_mem_malloc((TEST_BYTE_CNT+0x6000));

		pbuf_iso_w = (BYTE *)(DWORD)(((DWORD)pbuf_iso_w | 0xa0000000));
		pbuf_iso_r = (BYTE *)(DWORD)(((DWORD)pbuf_iso_r | 0xa0000000));
		pbuf_iso_w = (BYTE *)(DWORD)((((DWORD)pbuf_iso_w)&0xfffff000)+0x00001000);
		pbuf_iso_r = (BYTE *)(DWORD)((((DWORD)pbuf_iso_r)&0xfffff000)+0x00001000);
	
		sOTGH_PT_ISO.aDataBufferArray[0]=(UINT32)&pbuf_iso_w[0];//flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER);          
		sOTGH_PT_ISO.aDataBufferArray[1]=(UINT32)&pbuf_iso_r[0];//flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER);          
	}
	for (i = 0; i < 0x1000; ++i)
	{
		((BYTE *)sOTGH_PT_ISO.aDataBufferArray[0])[i] = (BYTE)isoLoopCnt;
		((BYTE *)sOTGH_PT_ISO.aDataBufferArray[1])[i] = (BYTE)0;
	}

	flib_Host20_ISO_Init(eWhichOtg);

	wMaxPacketSize=0x200;
	bTemp=wMaxPacketSize>>11;
	switch(bTemp)
	{
		case 0:
			bMult=1;
			break;
		case 1:
			bMult=2;
			break;
		case 2:
			bMult=3;          
			break;
		default:
			mpDebugPrint("??? MaxPacketSize Parse Error...");
			while(1);
			break;	
	}

	wMaxPacketSize = 0x200;
	wTxRxLength=wMaxPacketSize;
	wOffset = 0;
	if (dir == OTGH_Dir_IN)
	    flib_Host20_Issue_ISO(1, 1, wMaxPacketSize, wTxRxLength, &(sOTGH_PT_ISO.aDataBufferArray[1]), wOffset, OTGH_Dir_IN, bMult, eWhichOtg);
	else
	{
		mpDebugPrint("ISO out len %x", wTxRxLength);
	    flib_Host20_Issue_ISO(1, 2, wMaxPacketSize, wTxRxLength, &(sOTGH_PT_ISO.aDataBufferArray[0]), wOffset, OTGH_Dir_Out, bMult, eWhichOtg);
	}
}

void webCamStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    ST_MCARD_MAIL   *pReceiveMailDrv;
    ST_MCARD_MAIL   *pSendMailDrv;
    SWORD	err = 0;
	SDWORD	osSts;
    BYTE	mail_id;

    if (mwHost20_PORTSC_ConnectStatus_Rd() == 0)// device plug out
    {
        pUsbh->sMDevice[bMcardTransferID].swStatus = USB_HOST_DEVICE_PLUG_OUT;
        return;
    }
    pReceiveMailDrv = pUsbh->sMDevice[bMcardTransferID].sMcardRMail;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);


    switch( pReceiveMailDrv->wCurrentExecutionState )
    {
        case ISO_IN_STATE:
			MP_DEBUG("ISOInitStateMachine: ISO_IN_STATE");
        break;

        case ISO_IN_DONE_STATE:
			MP_DEBUG("ISOInitStateMachine: ISO_IN_DONE_STATE");
        break;
        case ISO_OUT_STATE:
			MP_DEBUG("ISOInitStateMachine: ISO_OUT_STATE");
        break;

        case ISO_OUT_DONE_STATE:
			MP_DEBUG("ISOInitStateMachine: ISO_OUT_DONE_STATE");
        break;
    }

    switch( pReceiveMailDrv->wCurrentExecutionState )
    {
        case ISO_IN_STATE:
        {
			extern void UsbOtgHostISOActive (WHICH_OTG);
			extern void UsbOtgHostISOIoc (WHICH_OTG);

			pUsbhDevDes->fpAppClassIsoActive   = &UsbOtgHostISOActive;
			pUsbhDevDes->fpAppClassIsoIoc      = &UsbOtgHostISOIoc;
			pSendMailDrv->wCurrentExecutionState    = ISO_IN_DONE_STATE;
			pSendMailDrv->wStateMachine             = WEB_CAM_SM;
        }
        break;

        case ISO_IN_DONE_STATE:
        {
			EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_ISO_IN);
			return;
        }
        break;

        case ISO_OUT_STATE:
        {
			extern void UsbOtgHostISOOutActive (WHICH_OTG);
			extern void UsbOtgHostISOOutIoc (WHICH_OTG);

			pUsbhDevDes->fpAppClassIsoActive   = &UsbOtgHostISOOutActive;
			pUsbhDevDes->fpAppClassIsoIoc      = &UsbOtgHostISOOutIoc;
			pSendMailDrv->wCurrentExecutionState    = ISO_OUT_DONE_STATE;
			pSendMailDrv->wStateMachine             = WEB_CAM_SM;
        }
        break;

        case ISO_OUT_DONE_STATE:
        {
			EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_ISO_IN);
			return;
        }
        break;



		default:
			mpDebugPrint("ISOInitStateMachine: state error %x!!", pReceiveMailDrv->wCurrentExecutionState);
		break;
    }
	if ((pSendMailDrv->wCurrentExecutionState == ISO_IN_DONE_STATE)||(pSendMailDrv->wCurrentExecutionState == ISO_OUT_DONE_STATE))
	{
		osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
		if (osSts != OS_STATUS_OK)
		{
			mpDebugPrint("ISOInitStateMachine: SendMailToUsbOtgHostClassTask failed!!");
		}
	}
}

BOOL inISO = FALSE;
BOOL outISO = FALSE;

void UsbOtgHostISOActive (WHICH_OTG eWhichOtg)
{
	MP_DEBUG("UsbOtgHostISOActive");
	inISO = TRUE;
	OTGH_PT_ISO(OTGH_Dir_IN, eWhichOtg);
}

void UsbOtgHostISOOutActive (WHICH_OTG eWhichOtg)
{
	MP_DEBUG("UsbOtgHostISOActive");
	outISO = TRUE;
	OTGH_PT_ISO(OTGH_Dir_Out, eWhichOtg);
}

DWORD okCnt = 0;
void UsbOtgHostISOIoc (WHICH_OTG eWhichOtg)
{
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
	UINT32 wFrameNumber,wiTDNum,wMaxPacketSize,i,j,l;
	UINT8  *pbData;
	iTD_Structure  *spTempiTD;
	DWORD cmp;

	if (!inISO) return;
	MP_DEBUG("UsbOtgHostISOIoc");
	UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
	inISO = FALSE;

	wiTDNum = Device_AP.bMaxIniTDNum;
	pbData=sOTGH_PT_ISO.aDataBufferArray[1];
	for (i = 0; i < wiTDNum; ++i)
	{
		spTempiTD=Device_AP.wISOiTDAddress[i];
		for (j = 0; j < 1; ++j)
		{
			if (spTempiTD->ArrayStatus_Word[j].bLength != 0x200)
			{
				mpDebugPrint("ISO in len %x", (0x200 - spTempiTD->ArrayStatus_Word[j].bLength));
				if ((0x200 - spTempiTD->ArrayStatus_Word[j].bLength) != 0x200)
				{
					mpDebugPrint("recv len error");
					while (1);
				}
				for (l = 0; l < (0x200 - spTempiTD->ArrayStatus_Word[j].bLength); ++l)
				{
					if (*(pbData + l) != ((BYTE)(isoLoopCnt)))
					{
						mpDebugPrint("pbData[%i] %x != %x", l, *(pbData + l), (BYTE)(isoLoopCnt));
						while(1);
					}
				}
				mpDebugPrint("data cmp ok %x", ++okCnt);
			}
			else mpDebugPrint("device return null data");
			if (spTempiTD->ArrayStatus_Word[j].bStatus)
				mpDebugPrint("error status %x j %x", spTempiTD->ArrayStatus_Word[j].bStatus, j);
		}
	}
	isoLoopCnt++;
	//<5>.Retire the iTD
	wFrameNumber=Device_AP.wOriginalFrameNumber;   
	for (i=0;i<wiTDNum;i++)      
	{
		if (wFrameNumber>255)
			wFrameNumber=wFrameNumber-256;
		psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bTerminal=1;             
		psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bLinkPointer=0;               
		wFrameNumber++;
	}   

	//<6>.Free the iTD
	for (i=0;i<wiTDNum;i++)      
	{
		flib_Host20_ReleaseStructure(Host20_MEM_TYPE_iTD,Device_AP.wISOiTDAddress[i], eWhichOtg);
	}

	//<5>.Free the allocated Buffer
	//flib_Host20_ReleaseStructure(Host20_MEM_TYPE_4K_BUFFER,sOTGH_PT_ISO.aDataBufferArray[0]);


	{
    	ST_MCARD_MAIL	*pSendMailDrv;
		SDWORD			osSts;

		pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
		pSendMailDrv->wCurrentExecutionState   = ISO_OUT_STATE;
		pSendMailDrv->wStateMachine            = WEB_CAM_SM;

		osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
		if (osSts != OS_STATUS_OK)
		{
			MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
		}
	}
}

void UsbOtgHostISOOutIoc (WHICH_OTG eWhichOtg)
{
	UINT32 wFrameNumber,wiTDNum,i;
	PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

	if (!outISO) return;
	MP_DEBUG("UsbOtgHostISOOutIoc");
	UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
	outISO = FALSE;

	wiTDNum = Device_AP.bMaxIniTDNum;
	//<5>.Retire the iTD
	wFrameNumber=Device_AP.wOriginalFrameNumber;   
	for (i=0;i<wiTDNum;i++)      
	{
		if (wFrameNumber>255)
			wFrameNumber=wFrameNumber-256;
		psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bTerminal=1;             
		psUsbHostEhci->psHostFramList->sCell[wFrameNumber].bLinkPointer=0;               
		wFrameNumber++;
	}   

	//<6>.Free the iTD
	for (i=0;i<wiTDNum;i++)      
	{
		flib_Host20_ReleaseStructure(Host20_MEM_TYPE_iTD,Device_AP.wISOiTDAddress[i], eWhichOtg);
	}

	//<5>.Free the allocated Buffer
	//flib_Host20_ReleaseStructure(Host20_MEM_TYPE_4K_BUFFER,sOTGH_PT_ISO.aDataBufferArray[0]);


	{
    	ST_MCARD_MAIL	*pSendMailDrv;
		SDWORD			osSts;

		pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
		pSendMailDrv->wCurrentExecutionState   = ISO_IN_STATE;
		pSendMailDrv->wStateMachine            = WEB_CAM_SM;

		osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
		if (osSts != OS_STATUS_OK)
		{
			MP_DEBUG("SendMailToUsbOtgHostClassTask failed!!");
		}
	}
}
#endif // 0 // JL, 10022011

#endif //(SC_USBOTG)

