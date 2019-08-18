#include "global612.h"
#include "display.h"
#include "Setup.h"
#include "xpgIconFunc.h"


#if THE_OSD_ICON
typedef struct {
	WORD x;
	WORD y;
	WORD w;
	WORD h;
}ICON_POSITION;

ICON_POSITION iconframe_position[]=
{
	{17,100,82,82},	//smallframe
	{17,256,82,82},

	{226,94,124,124},//gImage_IconFrame
	{400,94,124,124},	
	{585,94,124,124},	
	{222,261,124,124},	
	{409,261,124,124},	
	{585,261,124,124},	

};
void ICONDraw(BYTE index)
{
	Idu_OsdChangeBlending(0xf);

	switch(index)
	{
		case 0:
		case 1:
				Idu_OsdPaintAreaImage(iconframe_position[index].x, iconframe_position[index].y,
				14,14, gImage_SmallFrame0,13,0);
				
				Idu_OsdPaintAreaImage(iconframe_position[index].x+68, iconframe_position[index].y,
				14,14, gImage_SmallFrame1,13,0);

				Idu_OsdPaintAreaImage(iconframe_position[index].x, iconframe_position[index].y+68,
				14,14, gImage_SmallFrame2,13,0);

				Idu_OsdPaintAreaImage(iconframe_position[index].x+68, iconframe_position[index].y+68,
				14,14, gImage_SmallFrame3,13,0);

				break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		default:
				Idu_OsdPaintAreaImage(iconframe_position[index].x, iconframe_position[index].y,
				20,20, gImage_IconFrame0,13,0);
				
				Idu_OsdPaintAreaImage(iconframe_position[index].x+104, iconframe_position[index].y,
				20,20,gImage_IconFrame1,13,0);

				Idu_OsdPaintAreaImage(iconframe_position[index].x, iconframe_position[index].y+104,
				20,20, gImage_IconFrame2,13,0);

				Idu_OsdPaintAreaImage(iconframe_position[index].x+104, iconframe_position[index].y+104,
				20,20, gImage_IconFrame3,13,0);
				break;
	}
}
void ICONErase(BYTE index)
{
	Idu_OsdPaintArea(iconframe_position[index].x,iconframe_position[index].y, 
		iconframe_position[index].w+4,iconframe_position[index].h+4,0);
}

ICON_POSITION Thumbframe_position[]=
{
	{150,117,207,142},	
	{354,117,207,142},	
	{560,117,207,142},	
	{150,277,207,142},	
	{354,277,207,142},	
	{560,277,207,142},	

};

void ThumbDraw(BYTE index)
{
	Idu_OsdChangeBlending(0xf);
	Idu_OsdPaintAreaImage(Thumbframe_position[index].x, Thumbframe_position[index].y,
	30,24, gImage_ThumbFrame0,13,0);
	Idu_OsdPaintAreaImage(Thumbframe_position[index].x+177, Thumbframe_position[index].y,
	30,24, gImage_ThumbFrame1,13,0);
	Idu_OsdPaintAreaImage(Thumbframe_position[index].x, Thumbframe_position[index].y+118,
	30,24, gImage_ThumbFrame2,13,0);
	Idu_OsdPaintAreaImage(Thumbframe_position[index].x+177, Thumbframe_position[index].y+118,
	30,24, gImage_ThumbFrame3,13,0);

}
void ThumbErase(BYTE index)
{
	Idu_OsdPaintArea(Thumbframe_position[index].x,Thumbframe_position[index].y, 
		Thumbframe_position[index].w+4,Thumbframe_position[index].h+4,0);

}
void ThumbEraseAll()
{
	Idu_OsdPaintArea(150,117,620,300,0);

}
#define MessageBackgroundColor 0x3
void MessageShow()
{
	Idu_OsdChangeBlending(0x9);
	Idu_OsdPaintAreaLine(280,140,360,204,MessageBackgroundColor);
	Idu_OsdPaintAreaImage(280, 140,	16,16, gImage_Message0,MessageBackgroundColor,1);
	Idu_OsdPaintAreaImage(624, 140,	16,16, gImage_Message1,MessageBackgroundColor,1);
	Idu_OsdPaintAreaImage(280, 328,	16,16, gImage_Message2,MessageBackgroundColor,1);
	Idu_OsdPaintAreaImage(624, 328,	16,16, gImage_Message3,MessageBackgroundColor,1);


}
void MessageClear()
{
	Idu_OsdChangeBlending(0xf);
	Idu_OsdPaintAreaLine(280,140,360,204,0);


}
ICON_POSITION del_oneAndall_position[]=
{
	{505,185,75,40},	//smallframe
	{505,247,75,40},


};
static BYTE Oldindex;

void MessageDel_OneAndAll()
{
	MessageShow();
	Idu_OsdPutStr(Idu_GetOsdWin(), "ONE", 524, 194, 13);
	Idu_OsdPutStr(Idu_GetOsdWin(), "ALL", 524, 254, 13);
	Idu_OsdPaintAreaImage(322, 174,	94,120, gImage_DelIcon,13,0);

	


}
void MessageDel_Change(BYTE Newindex)
{

	if (!Oldindex)
	{
		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x, del_oneAndall_position[Oldindex].y,
		14,14, gImage_SmallFrame0,MessageBackgroundColor,0);
		
		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x+61, del_oneAndall_position[Oldindex].y,
		14,14, gImage_SmallFrame1,MessageBackgroundColor,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x, del_oneAndall_position[Oldindex].y+26,
		14,14, gImage_SmallFrame2,MessageBackgroundColor,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x+61, del_oneAndall_position[Oldindex].y+26,
		14,14, gImage_SmallFrame3,MessageBackgroundColor,0);

	}
	else
	{
		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x, del_oneAndall_position[Oldindex].y,
		14,14, gImage_SmallFrame0,MessageBackgroundColor,0);
		
		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x+61, del_oneAndall_position[Oldindex].y,
		14,14, gImage_SmallFrame1,MessageBackgroundColor,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x, del_oneAndall_position[Oldindex].y+26,
		14,14, gImage_SmallFrame2,MessageBackgroundColor,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Oldindex].x+61, del_oneAndall_position[Oldindex].y+26,
		14,14, gImage_SmallFrame3,MessageBackgroundColor,0);

	}
	if (!Newindex)
	{
		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x, del_oneAndall_position[Newindex].y,
		14,14, gImage_SmallFrame0,13,0);
		
		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x+61, del_oneAndall_position[Newindex].y,
		14,14, gImage_SmallFrame1,13,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x, del_oneAndall_position[Newindex].y+26,
		14,14, gImage_SmallFrame2,13,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x+61, del_oneAndall_position[Newindex].y+26,
		14,14, gImage_SmallFrame3,13,0);

	}
	else
	{
		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x, del_oneAndall_position[Newindex].y,
		14,14, gImage_SmallFrame0,13,0);
		
		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x+61, del_oneAndall_position[Newindex].y,
		14,14, gImage_SmallFrame1,13,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x, del_oneAndall_position[Newindex].y+26,
		14,14, gImage_SmallFrame2,13,0);

		Idu_OsdPaintAreaImage(del_oneAndall_position[Newindex].x+61, del_oneAndall_position[Newindex].y+26,
		14,14, gImage_SmallFrame3,13,0);

	}
	Oldindex=Newindex;
}
void MessageDel_YesOrNo()
{
	MessageShow();
	Idu_OsdPaintAreaImage(523, 186,	36,34, gImage_NoIcon,6,0);
	Idu_OsdPaintAreaImage(523, 248,	36,34, gImage_YesIcon,13,0);
	Idu_OsdPaintAreaImage(322, 174,	94,120, gImage_DelIcon,13,0);

}
void MessageDel_OneAndAllShow(BYTE index)
{
	if (!index)
		Idu_OsdPutStr(Idu_GetOsdWin(), "ONE FILE", 415, 300, 13);
	else
		Idu_OsdPutStr(Idu_GetOsdWin(), "ALL FILE", 415, 300, 13);

}
ICON_POSITION ScreenSelect_position[]=
{
	{246,152,426,82},	//smallframe
	{246,238,426,82},


};
void ScreenSelect(BYTE Newindex)
{
	Idu_OsdPaintArea(246,152,426,168,0);

		if (!Newindex)
		{
			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x, ScreenSelect_position[Newindex].y,
			14,14, gImage_SmallFrame0,13,0);
			
			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x+412, ScreenSelect_position[Newindex].y,
			14,14, gImage_SmallFrame1,13,0);

			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x, ScreenSelect_position[Newindex].y+68,
			14,14, gImage_SmallFrame2,13,0);

			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x+412, ScreenSelect_position[Newindex].y+68,
			14,14, gImage_SmallFrame3,13,0);

		}
		else
		{
			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x, ScreenSelect_position[Newindex].y,
			14,14, gImage_SmallFrame0,13,0);
			
			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x+412, ScreenSelect_position[Newindex].y,
			14,14, gImage_SmallFrame1,13,0);

			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x, ScreenSelect_position[Newindex].y+68,
			14,14, gImage_SmallFrame2,13,0);

			Idu_OsdPaintAreaImage(ScreenSelect_position[Newindex].x+412, ScreenSelect_position[Newindex].y+68,
			14,14, gImage_SmallFrame3,13,0);

		}
}
void ShowCaptureIcon()
{
	Idu_OsdPaintAreaImage(650, 20,41,35, gImage_Capture,13,0);

}
#endif

#if 1
//--if (bOverlay==0)   &&  (imgbuffer==0 )  ->Pixel=0
//--if (b_PaletteIndex==0) && (imgbuffer>0 ) ->Pixel=imgbuffer
void Idu_OsdPaint4BitIcon(WORD startX, WORD startY, WORD SizeX, WORD SizeY,
					  const BYTE  *pImgBuffer,  BYTE b_PaletteIndex,BYTE bOverlay)
{
#if OSD_ENABLE
	WORD w_Width, w_Height, w_Offset, i = 0;
	register BYTE *Pixel;
	register ST_OSDWIN *OSDWin;
	BYTE j,k,bImgBuffer,backcolor,newcolor;
	DWORD dwoffset;

       if (pImgBuffer == NULL)
      {
        return;
      }
	OSDWin = Idu_GetOsdWin();
	if (OSDWin == NULL)
		return;
	if(startX & 1) startX--;
	OSDWin->boPainted = TRUE;
	OSDWin->wX = startX;
	OSDWin->wY = startY;
	if ((OSDWin->pdwStart == NULL) || (OSDWin->wX >= OSDWin->wWidth)
		|| (OSDWin->wY >= OSDWin->wHeight))
		return;

	if ((OSDWin->wX + SizeX) > OSDWin->wWidth)
		SizeX = OSDWin->wWidth - OSDWin->wX;

	if ((OSDWin->wY + SizeY) > OSDWin->wHeight)
		SizeY = OSDWin->wHeight - OSDWin->wY;

	w_Height = SizeY;
	//if(SizeX & 1) SizeX--;

	Pixel = (BYTE *) OSDWin->pdwStart + (OSDWin->wY * (OSDWin->wWidth >> OSD_BIT_OFFSET)) +
		(OSDWin->wX >> OSD_BIT_OFFSET);
	dwoffset = ((OSDWin->wWidth - SizeX) >> OSD_BIT_OFFSET);
	j=8;
	//mpDebugPrint(" Idu_OsdPaint4BitIcon startX=%d startY=%d SizeX=%d SizeY=%d  winW=%d",startX,startY,SizeX,SizeY,OSDWin->wWidth);
	while (w_Height)
	{
		w_Width = SizeX;
		while (w_Width)
		{
			bImgBuffer = (*pImgBuffer);
			j-=4;
			bImgBuffer = bImgBuffer >> j;
			if ((startX + i) & 0x1)
			{
				if (bImgBuffer & 0x0f)
				{
					if (b_PaletteIndex)
						*Pixel = (*Pixel & 0xf0) + (b_PaletteIndex & 0x0f);
					else
						*Pixel = (*Pixel & 0xf0) + ((bImgBuffer) & 0x0f);
				}
				else if (!bOverlay)
					*Pixel = (*Pixel & 0xf0);
				Pixel++;
			}
			else
			{
				if (bImgBuffer & 0x0f)
				{
					if (b_PaletteIndex)
						*Pixel = (*Pixel & 0x0f) + ((b_PaletteIndex & 0x0f) << 4);
					else
						*Pixel = (*Pixel & 0x0f) + (((bImgBuffer) & 0x0f) << 4);
				}
				else if (!bOverlay)
					*Pixel = (*Pixel & 0x0f);
			}
			
			if(j==0)
			{
				j=8;
				pImgBuffer++;
			}
			
			i++;
			w_Width--;
		}
		if (j==4)
		{
			j=8;
			pImgBuffer++;
		}
		if ((startX + i) & 0x1)
			Pixel++;
       Pixel += dwoffset;
		w_Height--;
		i = 0;
	}
#endif
}

BYTE* Get_Osd_Icon_Buffer(BYTE IconIndex)
{
	BYTE *pImgBuffer=NULL,index;

	switch (IconIndex)
	{
#ifdef OSDICON_BatteryBox
			case OSDICON_BatteryBox:
				pImgBuffer = Osd_BatteryBox;
				break;
#endif
#if OSD_ICON_BATTERY_ENABLE
			case OSDICON_Battery_0:
				pImgBuffer = Battery_0;
				break;

			case OSDICON_Battery_1:
				pImgBuffer = Battery_1;
				break;

			case OSDICON_Battery_2:
				pImgBuffer = Battery_2;
				break;

			case OSDICON_Battery_3:
				pImgBuffer = Battery_3;
				break;

			case OSDICON_Battery_4:
				pImgBuffer = Battery_4;
				break;
#endif

#if OSDICON_Rec
			case OSDICON_Rec:
				pImgBuffer = Rec;
				break;
#endif

#if OSDICON_DC
			case OSDICON_DC: //capture
				pImgBuffer = DC;
				break;
#endif

#if OSDICON_NoSD
			case OSDICON_NoSD:
				pImgBuffer = NoSD;
				break;
#elif OSDICON_NoSD_ForRec
			case OSDICON_NoSD_ForRec:
				pImgBuffer = NoSD;
				break;
#endif

#if SHOW_USB_STATUS
			case OSDICON_UsbOn:
				pImgBuffer = UsbOn;
				break;
			case OSDICON_UsbOff:
				pImgBuffer = UsbOff;
				break;
#endif
#ifdef OSDICON_Freeze
			case OSDICON_Freeze:
				pImgBuffer = Osd_Freeze;
				break;
#endif
#ifdef OSDICON_WIFI
			case OSDICON_WIFI:
				pImgBuffer = Osd_WIFI;
				break;
#endif
#ifdef OSDICON_BRIGHTNESS
			case OSDICON_BRIGHTNESS:
				pImgBuffer = Osd_brightness;
				break;
#endif
#ifdef OSDICON_Contrast
			case OSDICON_Contrast:
				pImgBuffer = Osd_Contrast;
				break;
#endif
#ifdef OSDICON_Hue
			case OSDICON_Hue:
				pImgBuffer = OSD_Hue;
				break;
#endif
#ifdef OSDICON_LIGHT_LEVEL
			case OSDICON_LIGHT_LEVEL:
				index=GetLightLevel();
				if (index>=sizeof(OSD_LightLevel)/sizeof(DWORD))
					index=sizeof(OSD_LightLevel)/sizeof(DWORD)-1;
				pImgBuffer = OSD_LightLevel[index];
				break;
#endif
#ifdef OSDICON_PLAY
			case OSDICON_PLAY:
				pImgBuffer = OSD_Play;
				break;
#endif
#ifdef OSDICON_PAUSE
			case OSDICON_PAUSE:
				pImgBuffer = OSD_Pause;
				break;
#endif
#ifdef OSDICON_FF
			case OSDICON_FF:
				pImgBuffer = OSD_FF;
				break;
#endif
#ifdef OSDICON_FB
			case OSDICON_FB:
				pImgBuffer = OSD_FB;
				break;
#endif
#ifdef OSDICON_CARD_FULL
			case OSDICON_CARD_FULL:
				pImgBuffer = OSD_card_full;
				break;
#endif

			default:
				pImgBuffer=NULL;
				break;
		}
	return pImgBuffer;
}

void Draw_Osd_Icon(WORD x, WORD y, BYTE *pImgBuffer, BYTE IconIndex,BYTE  b_PaletteIndex, BYTE bOverlay)
{
	WORD w,h;

    //mpDebugPrint("### %s ### pImgBuffer=0x%x IconIndex=%d,x=%d y=%d", __FUNCTION__,pImgBuffer,IconIndex,x,y);

	if (pImgBuffer==NULL)
	{
		pImgBuffer=Get_Osd_Icon_Buffer(IconIndex);
	}

	if (pImgBuffer==NULL)
		return;
	
	if(*(pImgBuffer+1) == 4)
	{
		w=(*(pImgBuffer+3)<<8)|(*(pImgBuffer+2));
		h=(*(pImgBuffer+5)<<8)|(*(pImgBuffer+4));
		pImgBuffer+=6;
		Idu_OsdPaint4BitIcon(x,  y, w, h, pImgBuffer, b_PaletteIndex,bOverlay);
	}
}

void ClearOsdIcon(BYTE IconIndex,WORD x, WORD y)
{
	WORD w,h;
	BYTE *pImgBuffer=NULL;

	pImgBuffer=Get_Osd_Icon_Buffer(IconIndex);
    //mpDebugPrint("### %s ### pImgBuffer=0x%x IconIndex=%d", __FUNCTION__,pImgBuffer,IconIndex);
	if (pImgBuffer==NULL)
		return;
	
	if(*(pImgBuffer+1) == 4)
	{
		w=(*(pImgBuffer+3)<<8)|(*(pImgBuffer+2));
		h=(*(pImgBuffer+5)<<8)|(*(pImgBuffer+4));
		Idu_OsdPaintArea(x,  y, w, h,0);
	}
}

void DrawOsdIcon(BYTE IconIndex,WORD x, WORD y)
{
	WORD w,h;
	BYTE *pImgBuffer=NULL;

	pImgBuffer=Get_Osd_Icon_Buffer(IconIndex);
    //mpDebugPrint("### %s ###  IconIndex=%d,x=%d y=%d", __FUNCTION__,IconIndex,x,y);
	if (pImgBuffer==NULL)
		return;
	
	if(*(pImgBuffer+1) == 4)
	{
		w=(*(pImgBuffer+3)<<8)|(*(pImgBuffer+2));
		h=(*(pImgBuffer+5)<<8)|(*(pImgBuffer+4));
		pImgBuffer+=6;
		Idu_OsdPaint4BitIcon(x,  y, w, h, pImgBuffer, 0,0);
	}
}

void DrawOsdIconWithString(BYTE IconIndex,WORD x, WORD y,BYTE *pbStr,BYTE bStringColorIndex)
{
	WORD w,h;
	BYTE *pImgBuffer=NULL;

	pImgBuffer=Get_Osd_Icon_Buffer(IconIndex);
    //mpDebugPrint("### %s ###  IconIndex=%d,x=%d y=%d", __FUNCTION__,IconIndex,x,y);
	if (pImgBuffer==NULL)
		return;
	
	if(*(pImgBuffer+1) == 4)
	{
		w=(*(pImgBuffer+3)<<8)|(*(pImgBuffer+2));
		h=(*(pImgBuffer+5)<<8)|(*(pImgBuffer+4));
		pImgBuffer+=6;
		Idu_OsdPaint4BitIcon(x,  y, w, h, pImgBuffer, 0,0);
		if (pbStr)
			Idu_OsdPutStr(Idu_GetOsdWin (), pbStr, x+w+16, y+h/2-14/2, bStringColorIndex);
	}
}

//ACTION && STATUS
void Clear_Action_Icon()
{
    //mpDebugPrint("### %s ### ", __FUNCTION__);
	Idu_OsdPaintArea(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,OSD_ICON_ACTION_POS_W,OSD_ICON_ACTION_POS_H,0);
}

void Timer_Clear_Action_Icon(DWORD dwTime)
{
	Ui_TimerProcAdd(dwTime, Clear_Action_Icon);
}

void Show_Status_Icon(BYTE IconIndex)
{
	Ui_TimerProcRemove(Clear_Action_Icon);
	Clear_Action_Icon();
	Draw_Osd_Icon(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,NULL,IconIndex,0,0);
}

void Show_Action_Icon(BYTE IconIndex)
{
	Show_Status_Icon(IconIndex);
	Ui_TimerProcAdd(700, Clear_Action_Icon);
}

void ShowActionIconWithString(BYTE IconIndex,BYTE *pbStr,BYTE bStringColorIndex,DWORD dwTimer)
{
	Clear_Action_Icon();
	DrawOsdIconWithString(IconIndex,OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,pbStr,bStringColorIndex);
	Ui_TimerProcAdd(dwTimer, Clear_Action_Icon);
}

void Show_Action_Icon_TimerOff(BYTE IconIndex,DWORD dwTime)
{
	Show_Status_Icon(IconIndex);
	Ui_TimerProcAdd(dwTime, Clear_Action_Icon);
}

//PHOTO 
void ShowItemIndex()
{
#if FOOT_56_MODE && (PRODUCT_UI!=UI_HXJ_1)
	ShowCaptureFileName(-1);
#endif
#if 	0
	BYTE bStr[64];
	DWORD dwIndex = FileListAddCurIndex(0);

	Idu_OsdPaintArea(350,400,100,50,0);
	if (FileBrowserGetTotalFile())
		dwIndex++;
    sprintf(bStr, "%d / %d", dwIndex,FileBrowserGetTotalFile());
	Idu_OsdPutStr(Idu_GetOsdWin(), bStr, 350 , 420, OSD_COLOR_WHITE);
#endif
	
}

#if OSDICON_Rec
void Timer_Recording_Icon()
{
    //mpDebugPrint("### %s ###", __FUNCTION__);
	Clear_Action_Icon();
	 if (!bRecordMode())
	 	return;
	Draw_Osd_Icon(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,NULL,OSDICON_Rec,0,0);
	Ui_TimerProcAdd(1000, Clear_Action_Icon);
	Ui_TimerProcAdd(2000, Timer_Recording_Icon);
}
#endif

#if SHOW_VIDEO_STATUS
//VIDEO   pos 300 400
void Clear_Video_Action_Icon()
{
	//Clear_Action_Icon();
	Idu_OsdPaintArea(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,100,OSD_ICON_ACTION_POS_H,0);
}

void Show_Video_Action_Icon(BYTE IconIndex)
{
	Ui_TimerProcRemove(Clear_Video_Action_Icon);
	Clear_Video_Action_Icon();
	Draw_Osd_Icon(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,NULL,IconIndex,0,0);
	if (IconIndex== OSDICON_FF || IconIndex==OSDICON_FB)
	{
    	BYTE bPrintBuf[10];

		mp_sprintf(bPrintBuf, "%d X", abs(GetFastSpeed()));
    	Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, OSD_ICON_ACTION_POS_X+26, OSD_ICON_ACTION_POS_Y, OSD_COLOR_WHITE);
	}
	//Ui_TimerProcAdd(1000, Clear_Video_Action_Icon);
}

void CleanVideoPlayTime()
{
	ST_IMGWIN *pWin= Idu_GetCurrWin();

	Idu_OsdPaintArea(OSD_ICON_ACTION_POS_X+100,OSD_ICON_ACTION_POS_Y,150,50,0);
}

void ShowVideoPalyTime(register DWORD wValue)
{
    register BYTE bDispSec, bDispMin, bDispHour;
    BYTE bPrintBuf[10];
	ST_IMGWIN *pWin= Idu_GetCurrWin();

    bDispHour = wValue / 3600;
    wValue -= bDispHour * 3600;
    bDispMin = wValue / 60;
    bDispSec = wValue + (bDispMin << 2) - (bDispMin << 6);
    mp_sprintf(bPrintBuf, "%d:%02d:%02d", bDispHour, bDispMin, bDispSec);
	CleanVideoPlayTime();
	//Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, OSD_ICON_ACTION_POS_X+360, pWin->wHeight-DISPLAY_POS_DY, OSD_COLOR_WHITE);
	Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, OSD_ICON_ACTION_POS_X+100, OSD_ICON_ACTION_POS_Y, OSD_COLOR_WHITE);
	
}
#endif

//DELETE 
#if FOOT_56_MODE
#define      DeleteDialogDY					216
#else
#define      DeleteDialogDY					296
#endif
void ClearDeleteDialog()
{
		Idu_OsdPaintArea(DeleteDialogDY,36,208,44+64,0);
}

void ShowDeleteDialog(BYTE bFocusIndex)
{

	Idu_OsdPaintArea(DeleteDialogDY,36+60*(bFocusIndex>0 ? 0 : 1),208,44,0);
	Idu_OsdPaintArea(DeleteDialogDY,36+60*bFocusIndex,208,44,OSD_CLR_RED);
	Idu_OsdPaintArea(DeleteDialogDY+4,40,200,36,OSD_CLR_BLUE);
	Idu_OsdPaintArea(DeleteDialogDY+4,100,200,36,OSD_CLR_BLUE);
#if OSDICON_Delete
	if (g_psSetupMenu->language == LANGUAGE_S_CHINESE)
	{
		Draw_Osd_Icon(DeleteDialogDY+4,40,NULL,OSDICON_Cancel,OSD_COLOR_BLACK,1);
		Draw_Osd_Icon(DeleteDialogDY+4,100,NULL,OSDICON_Delete,OSD_COLOR_BLACK,1);
	}
	else
#endif
	{
		Idu_OsdPutStr(Idu_GetOsdWin(), "Cancel", DeleteDialogDY+54 , 42, OSD_COLOR_BLACK);
		Idu_OsdPutStr(Idu_GetOsdWin(), "Delete", DeleteDialogDY+54 , 102, OSD_COLOR_BLACK);
	}

}

//ZOOM
#if (OSDICON_ZOOM||OSD_SHOW_ZOOM)
void ShowZoomAction(SWORD swMode,BYTE ZoomStep)
{
    BYTE bPrintBuf[8];

	Ui_TimerProcRemove(Clear_Action_Icon);
	Clear_Action_Icon();
	if (swMode>0)
	{
#if OSDICON_ZOOM
		Draw_Osd_Icon(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,NULL,OSDICON_ZoomOut,0,0);
#endif
#if OSD_SHOW_ZOOM
    	mp_sprintf(bPrintBuf, "%d.%d X", ZoomStep/10,ZoomStep%10);
#endif
	}
	else
	{
#if OSDICON_ZOOM
		Draw_Osd_Icon(OSD_ICON_ACTION_POS_X,OSD_ICON_ACTION_POS_Y,NULL,OSDICON_ZoomIn,0,0);
#endif
#if OSD_SHOW_ZOOM
    	mp_sprintf(bPrintBuf, "1/%d X", ZoomStep/10,ZoomStep%10);
#endif
	}
#if OSD_SHOW_ZOOM
    Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, OSD_ICON_ACTION_POS_X+36, OSD_ICON_ACTION_POS_Y, OSD_COLOR_WHITE);
#endif
	Ui_TimerProcAdd(3000, Clear_Action_Icon);
}
#endif

#if FOOT_56_MODE
//capture file name
void ClearCaptureFileName(void)
{
	Idu_OsdPaintArea(OSD_ICON_ACTION_POS_X, DISPLAY_POS_Y, 204, DISPLAY_POS_DY, 0);
}

void ShowCaptureFileName(SDWORD sdwIndex)
{
    BYTE pbTempBuffer[8];
	DWORD dwIndex;

	if (sdwIndex>=0)
		dwIndex = sdwIndex;
	else
		dwIndex = FileBrowserGetCurIndex();
	mp_sprintf(pbTempBuffer, "%04d",dwIndex);
	ClearCaptureFileName();
	Idu_OsdPutStr(Idu_GetOsdWin(), pbTempBuffer, OSD_ICON_ACTION_POS_X, DISPLAY_POS_Y, OSD_COLOR_WHITE); 
}
#endif

#ifdef OSDICON_WIFI
void ShowWifiPowerStatus(void)
{
	if (g_psSetupMenu->wifimode)
		DrawOsdIcon(OSDICON_WIFI,OSD_ICON_WIFI_POS_X,OSD_ICON_WIFI_POS_Y);
	else
		ClearOsdIcon(OSDICON_WIFI,OSD_ICON_WIFI_POS_X,OSD_ICON_WIFI_POS_Y);
}
#endif

#endif



