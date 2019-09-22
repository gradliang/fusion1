//-----------------------------------------------------------------------------------
// Descriptions :
//
// This file is used to put GUI drawing sprites functions.
//
// If you want to write any new drawing sprite method,
// you should write it in this file.
//
// Then put your function prototype in Xpgfunc.h.
// Other part of code could call your new function.
//-----------------------------------------------------------------------------------


// define this module show debug message or not,  0 : disable, 1 : enable
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "xpgDrawSprite.h"
#include "mpTrace.h"
#include "xpgGPRSFunc.h"
#include "ui_timer.h"
#include "mpapi.h"
#include "xpgCamFunc.h"
#include "xpgString.h"
#include "charset.h"
#include "SPI_Ex.h"
#include "Setup.h"
#include "uiTouchCtrller.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif


#if MAKE_XPG_PLAYER

//---------------------------------------------------------------------------
//Global variable region
//---------------------------------------------------------------------------
DWORD dwDialogType = 0;                     // current dialog type
BYTE  strCloseDialogBackPage[32] = {0};     // close the dialog, will go back to this page
MODEPARAM tempModeParam;

const BYTE * FModeStrList[] = {
    "SM",
    "MM",
    "DS",
    "NZ",
    "BIF",
    "CZ1",
    "CZ2",
    "AUTO",
    NULL
};

//BYTE xpgStringBuffer[254];
extern DWORD g_dwCurIndex,g_dwModeIconStatus;
#if  (PRODUCT_UI==UI_SURFACE)
extern DWORD g_dwPassNum,g_dwFailNum;
#endif
//---------------------------------------------------------------------------
// xpg draw sprite functions
//---------------------------------------------------------------------------
#pragma alignvar(4)
SWORD(*drawSpriteFunctions[]) (ST_IMGWIN *, STXPGSPRITE *, BOOL) =
{
	xpgDrawSprite,
	xpgDrawSprite_TextWhiteLeft,//type1 
	xpgDrawSprite_TextBlackLeft,	        //type2
	xpgDrawSprite,			          //type3
	xpgDrawSprite_TextBlackCenter,			          //type4
	xpgDrawSprite_HilightFrame,		        //type5
	xpgDrawSprite_ModeIcon,		        //type6
	xpgDrawSprite_Null,		        //type7
	xpgDrawSprite_Null,		        //type8
	xpgDrawSprite_Null,		        //type9
	xpgDrawSprite_FileView,		        //type10
    xpgDrawSprite_Background,           // type11
    xpgDrawSprite_Icon,                 // type12
    xpgDrawSprite_LightIcon,            // type13
    xpgDrawSprite_DarkIcon,             // type14
    xpgDrawSprite_Mask,                 // type15
    xpgDrawSprite_Title,                // type16
    xpgDrawSprite_Aside,                // type17
    xpgDrawSprite_BackIcon,             // type18
    xpgDrawSprite_CloseIcon,            // type19
    xpgDrawSprite_Text,                 // type20
    xpgDrawSprite_Selector,             // type21
    xpgDrawSprite_List,                 // type22
    xpgDrawSprite_Dialog,               // type23
    xpgDrawSprite_Status,               // type24
    xpgDrawSprite_Radio,                // type25
    xpgDrawSprite_Scroll,               // type26
    xpgDrawSprite_Frame,                // type27
};

//---------------------------------------------------------------------------
///
///@ingroup xpgDrawSprite
///@brief   Draw Sprite type Null
///
///@param   pWin - selected image Win
///@param   pstSprite - pointer of input Sprite
///@param   boClip - booloean of Clip - true or not
///
///@return  PASS
///
SWORD xpgDrawSprite_Null(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    MP_DEBUG("xpgDrawSprite_Null");
	return PASS;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgDrawSprite
///@brief   Direct Draw Sprite
///
///@param   pWin - selected image Win
///@param   pstSprite - pointer of input Sprite
///@param   boClip - booloean of Clip - true or not
///
///@return  PASS
///
SWORD xpgDrawSprite(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
	return PASS;
}
///
DWORD GetDigitPerTen(DWORD dwData)
{
	DWORD dwBit=1;

	dwData/=10;
	while (dwData>0)
	{
		dwBit++;
		dwData/=10;
	}
	return dwBit;
}
SWORD xpgDrawSprite_TextWhiteLeft(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	STXPGROLE * pstRole;
	SWORD swData=-1,i;
	DWORD dwBit;
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

#if  (PRODUCT_UI==UI_SURFACE)
    if (dwHashKey == xpgHash("Preview",0))
    {
			switch (pstSprite->m_dwTypeIndex)
			{
				case 0:
					swData=g_psSetupMenu->bReferPhoto;
					break;

				case 1:
					swData=g_psSetupMenu->bBackUp;
					break;

				case 2:
					swData=g_psSetupMenu->bBackDown;
					break;

				case 3:
					swData=g_psSetupMenu->bPhotoUp;
					break;

				case 4:
					swData=g_psSetupMenu->bPhotoDown;
					break;

				case 5:
					swData=g_dwPassNum;
					break;

				case 6:
					swData=g_dwFailNum;
					break;

				case 7:
					swData=g_dwPassNum+g_dwFailNum;
					break;


				default:
					break;
			}
    }
	else if (dwHashKey == xpgHash("PhotoView",0))
    {
			swData=g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
			
    }
	else if (dwHashKey == xpgHash("Setup",0))
    {
			swData=g_dwPassNum+g_dwFailNum;
			
    }
#endif

	if (swData>=0)
	{
		dwBit=GetDigitPerTen(swData);
		//mpDebugPrint("swData=%d dwBit=%d",swData,dwBit);
		for (i=0;i<dwBit;i++)
		{
				pstRole=g_pstXpgMovie->m_pstObjRole[XPG_ROLE_TEXT_WHITE_0+swData%10 ];
				xpgDirectDrawRoleOnWin(pWin,pstRole , pstSprite->m_wPx+pstRole->m_wWidth*(dwBit-i-1), pstSprite->m_wPy, pstSprite, 0);
				swData/=10;
		}
	}
	return PASS;
}

SWORD xpgDrawSprite_TextBlackLeft(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	STXPGROLE * pstRole;
	SWORD swData=-1,i;
	DWORD dwBit;

	if (swData>=0)
	{
		dwBit=GetDigitPerTen(swData);
		for (i=0;i<dwBit;i++)
		{
				pstRole=g_pstXpgMovie->m_pstObjRole[XPG_ROLE_TEXT_BLACK_0+swData%10 ];
				xpgDirectDrawRoleOnWin(pWin,pstRole , pstSprite->m_wPx+pstRole->m_wWidth*(dwBit-i-1), pstSprite->m_wPy, pstSprite, 0);
				swData/=10;
		}
	}
	return PASS;
}


SWORD xpgDrawSprite_TextBlackCenter(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	STXPGROLE * pstRole;
	SWORD swData=-1,i;
	WORD wX;
	DWORD dwBit;

#if  (PRODUCT_UI==UI_SURFACE)
	switch (pstSprite->m_dwTypeIndex)
	{
		case 0:
			swData=g_psSetupMenu->bBackUp;
			break;

		case 1:
			swData=g_psSetupMenu->bBackDown;
			break;

		case 2:
			swData=g_psSetupMenu->bPhotoUp;
			break;

		case 3:
			swData=g_psSetupMenu->bPhotoDown;
			break;

		default:
			break;
	}
#endif
	if (swData>=0)
	{
		dwBit=GetDigitPerTen(swData);
		pstRole=g_pstXpgMovie->m_pstObjRole[XPG_ROLE_TEXT_BLACK_0];
		wX=pstSprite->m_wPx+pstSprite->m_wWidth/2-pstRole->m_wWidth*dwBit/2;
		for (i=0;i<dwBit;i++)
		{
				pstRole=g_pstXpgMovie->m_pstObjRole[XPG_ROLE_TEXT_BLACK_0+swData%10 ];
				xpgDirectDrawRoleOnWin(pWin,pstRole , wX+pstRole->m_wWidth*(dwBit-i-1), pstSprite->m_wPy, pstSprite, 0);
				swData/=10;
		}
	}
	return PASS;
}



#define		HiLightBoxThickness								2
void DrawHiLightBox(ST_IMGWIN * pWin,DWORD x0,DWORD y0,DWORD w,DWORD h,DWORD LineWidth,DWORD dwColor)
{
	DWORD wDrawW,wDrawH;

	x0=ALIGN_2(x0);
	wDrawW=ALIGN_2(w>>2);
	wDrawH=ALIGN_2(h>>2);
	mpPaintWinArea(pWin, x0, y0, wDrawW, LineWidth, dwColor);
	mpPaintWinArea(pWin, x0+w-wDrawW, y0, wDrawW, LineWidth, dwColor);
	mpPaintWinArea(pWin, x0, y0+h-LineWidth, wDrawW, LineWidth, dwColor);
	mpPaintWinArea(pWin, x0+w-wDrawW, y0+h-LineWidth, wDrawW, LineWidth, dwColor);

	mpPaintWinArea(pWin, x0, y0, LineWidth, wDrawH, dwColor);
	mpPaintWinArea(pWin, x0, y0+h-wDrawH, LineWidth, wDrawH, dwColor);
	mpPaintWinArea(pWin, x0+w-LineWidth, y0, LineWidth, wDrawH, dwColor);
	mpPaintWinArea(pWin, x0+w-LineWidth, y0+h-wDrawH, LineWidth, wDrawH, dwColor);
}
SWORD xpgDrawSprite_HilightFrame(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	DWORD x = pstSprite->m_wPx;
	DWORD y = pstSprite->m_wPy;
	DWORD w = pstSprite->m_wWidth;
	DWORD h = pstSprite->m_wHeight;
	DWORD dwPage = g_pstXpgMovie->m_wCurPage;

	if (((g_dwCurIndex>>16)==dwPage)&&((g_dwCurIndex&0x0000ffff)==pstSprite->m_dwTypeIndex))
	{
		mpDebugPrint("g_dwCurIndex=%p",g_dwCurIndex);
		DrawHiLightBox(pWin,x-HiLightBoxThickness*3,y-HiLightBoxThickness*3,w+HiLightBoxThickness*6,h+HiLightBoxThickness*6,HiLightBoxThickness,RGB2YUV(200,200,10));
	}
}

SWORD xpgDrawSprite_ModeIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	DWORD x = pstSprite->m_wPx;
	DWORD y = pstSprite->m_wPy;
	DWORD w = pstSprite->m_wWidth;
	DWORD h = pstSprite->m_wHeight;
	DWORD dwPage = g_pstXpgMovie->m_wCurPage;

	if (((g_dwModeIconStatus>>16)==dwPage)&&(g_dwModeIconStatus&(1<<pstSprite->m_dwTypeIndex)))
	{
		DrawHiLightBox(pWin,x-HiLightBoxThickness*3,y-HiLightBoxThickness*3,w+HiLightBoxThickness*6,h+HiLightBoxThickness*6,HiLightBoxThickness,RGB2YUV(200,200,10));
	}
}

SWORD xpgDrawSprite_FileView(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	ST_IMGWIN stViewWin;

	ImgWinInit(&stViewWin,pWin->pdwStart+(pstSprite->m_wPy*pWin->dwOffset+pstSprite->m_wPx*2)/4,pWin->wHeight-pstSprite->m_wPy,pWin->wWidth-pstSprite->m_wPx);
	stViewWin.dwOffset=pWin->dwOffset;
	if (FileBrowserGetTotalFile())
		ImageDraw(&stViewWin,1);
	//else
	//	mpClearWin(&stViewWin);
}


#if SPRITE_TYPE_FLASHICON
void xpgDrawFlashIcon()  // Michael Kao
{
    ST_IMGWIN *pWin = Idu_GetCurrWin();
    STXPGSPRITE *pstSprite;

    static int TypeIndex =0 ;

    int i,j;
    i = 5;

    while(i)
    {
	  pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_FLASHICON, TypeIndex);

	if (pstSprite)
	{
	       xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, 0);
	}

	TypeIndex ++;
	if (TypeIndex >= 5)
		TypeIndex = 0;

	if (pstSprite)
		break;

	i--;
    }
    //if(i || g_boFlashIconInit)
    {
	//g_boFlashIconInit = FALSE;
	  AddTimerProc(300, xpgDrawFlashIcon);
    }
}
SWORD xpgDrawSprite_FlashIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    if (pstSprite->m_dwTypeIndex == 0)
    {
	  xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
	  RemoveTimerProc(xpgDrawFlashIcon);
	  //g_boFlashIconInit = TRUE;
	  AddTimerProc(240,xpgDrawFlashIcon);
    }
}
#endif

SWORD xpgDrawSprite_Background(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("User", strlen("User")) || 
        dwHashKey == xpgHash("ToolBox", strlen("ToolBox")) )
    {
        mpCopyEqualWin(pWin, Idu_GetCacheWin());
    }
    else
        xpgDrawSprite(pWin, pstSprite, boClip);
    return PASS;
}

SWORD xpgDrawSprite_Icon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE * pstMask;
    WORD wX = pstSprite->m_wPx;
    WORD wY = pstSprite->m_wPy;
    WORD wW = pstSprite->m_wWidth;
    WORD wH = pstSprite->m_wHeight;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;
    char * text = "";
    
    if (dwHashKey == xpgHash("Main", strlen("Main")))
        xpgDrawSprite(pWin, pstSprite, boClip);
    else if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, (dwSpriteId < 6) ? 1:0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("ToolBox", strlen("ToolBox")))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("Record", strlen("Record")) )
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, dwSpriteId);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("Manual_work", strlen("Manual_work")))
    {
        if (dwSpriteId <= 11)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 12)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
    }
    else if (dwHashKey == xpgHash("Auto_work", strlen("Auto_work")))
    {
        if (dwSpriteId <= 5)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 6 || dwSpriteId == 7 || dwSpriteId == 8 )
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
    }
    else if (dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
    {
        if (dwSpriteId == 0 && g_psSetupMenu->bHotUpMode == SETUP_MENU_HOT_UP_MODE_MANUAL)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, getstr(Str_ZiDong), pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth);            
        }
        else if (dwSpriteId == 1 && g_psSetupMenu->bHotUpMode == SETUP_MENU_HOT_UP_MODE_AUTO)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, getstr(Str_ShouDong), pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth);    
        }
        else if (dwSpriteId == 2 && !g_psSetupMenu->bPreHotEnable)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        if (dwSpriteId == 0)
            text = getstr(Str_YiBan);
        else if (dwSpriteId == 1)
            text = getstr(Str_BiaoZhun);
        else if (dwSpriteId == 2)
            text = getstr(Str_JingXi);
        else if (dwSpriteId == 3)
            text = getstr(Str_XianXin);
        else if (dwSpriteId == 4)
            text = getstr(Str_BaoCeng);
        else if (dwSpriteId == 5)
            text = getstr(Str_XPing);
        else if (dwSpriteId == 6)
            text = getstr(Str_YPing);
        else if (dwSpriteId == 7)
            text = getstr(Str_XYPing);
        else if (dwSpriteId == 8)
            text = getstr(Str_XYJiaoTi);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);   
    }
    else if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")))
    {
        if (dwSpriteId <= 7)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            text = (char*) FModeStrList[dwSpriteId];
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
        else if (dwSpriteId == 8)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            text = (char*) getstr(Str_ShuZhiXiuGai);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
    }
    else if (dwHashKey == xpgHash("FusionModeSet", strlen("FusionModeSet")))
    {
        if (dwSpriteId <= 10)
        {
            char tmpText[128];
            WORD startX, width;
            WORD height = 26;
            int AreaYOffset = -2;
            int LineYOffset = 20;
            MODEPARAM * pstModeParam = &tempModeParam;
            DWORD lineColor = RGB2YUV(75,75,75); //RGB2YUV(255,0,0); //RGB2YUV(75,75,75);
            
            Idu_FontColorSet(22,160,255);
            if (dwSpriteId == 0)
            {
                startX = pstSprite->m_wPx + 20;
                width = 92;
                sprintf(tmpText, "%d", pstModeParam->fangDianZhongXin);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 1)
            {
                startX = pstSprite->m_wPx + 20;
                width = 76;
                sprintf(tmpText, "%d", pstModeParam->rongJieDianYa);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 2)
            {
                startX = pstSprite->m_wPx + 20;
                width = 76;
                sprintf(tmpText, "%d", pstModeParam->yuRongDianYa);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 3)
            {
                startX = pstSprite->m_wPx + 20;
                width = 76;
                sprintf(tmpText, "%d", pstModeParam->chuChenDianYa);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 4)
            {
                startX = pstSprite->m_wPx + 36;
                width = 60;
                sprintf(tmpText, "%d", pstModeParam->rongJieChongDieLiang);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 5)
            {
                startX = pstSprite->m_wPx + 36;
                width = 80;
                sprintf(tmpText, "%d", pstModeParam->duiJiaoMuBiaoZhi);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 6)
            {
                startX = pstSprite->m_wPx + 20;
                width = 80;
                sprintf(tmpText, "%d", pstModeParam->rongJieShiJian);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 7)
            {
                startX = pstSprite->m_wPx + 20;
                width = 80;
                sprintf(tmpText, "%d", pstModeParam->yuRongShiJian);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 8)
            {
                startX = pstSprite->m_wPx + 20;
                width = 80;
                sprintf(tmpText, "%d", pstModeParam->chuChenShiJian);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 9)
            {
                startX = pstSprite->m_wPx + 54;
                width = 48;
                sprintf(tmpText, "%d.%d", pstModeParam->qieGeJiaoDuShangXian >> 6, pstModeParam->qieGeJiaoDuShangXian & 0x3f);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            else if (dwSpriteId == 10)
            {
                startX = pstSprite->m_wPx + 62;
                width = 60;
                sprintf(tmpText, "%d", pstModeParam->fangDianJiaoZhengMuBiaoZhi);
                //Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + AreaYOffset, width, height, RGB2YUV(255,255,0));
                xpgSpriteSetTouchArea(pstSprite, startX, pstSprite->m_wPy + AreaYOffset, width, height);
                Idu_PrintStringCenter(pWin, tmpText, startX, pstSprite->m_wPy, 0, width);
                Idu_PaintWinArea(pWin, startX, pstSprite->m_wPy + LineYOffset, width, 2, lineColor);
            }
            Idu_FontColorSet(0xff,0xff,0xff);
            return PASS;
        }
        else if (dwSpriteId >= 11 && dwSpriteId <= 13)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            if (dwSpriteId == 11)
                text = (char*) getstr(Str_HuiFuMoRenZhi);
            else if (dwSpriteId == 12)
                text = (char*) getstr(Str_BaoCun);
            else if (dwSpriteId == 13)
                text = (char*) getstr(Str_LingCunWei);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
    }
    
    xpgSpriteEnableTouch(pstSprite);
}

SWORD xpgDrawSprite_LightIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE * pstMask;
    WORD wX = pstSprite->m_wPx;
    WORD wY = pstSprite->m_wPy;
    WORD wW = pstSprite->m_wWidth;
    WORD wH = pstSprite->m_wHeight;
    char * text;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;

    if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")))
    {
        BOOL enable[8];
        enable[0] = g_psSetupMenu->bEnableIcon_LaLiCeShi;
        enable[1] = g_psSetupMenu->bEnableIcon_DuanMianJianCe;
        enable[2] = g_psSetupMenu->bEnableIcon_ZiDongDuiJiao;
        enable[3] = g_psSetupMenu->bEnableIcon_JiaoDuJianCe;
        enable[4] = g_psSetupMenu->bEnableIcon_BaoCunTuXiang;
        enable[5] = g_psSetupMenu->bEnableIcon_HuiChenJianCe;
        enable[6] = g_psSetupMenu->bEnableIcon_RongJieZanTing;
        enable[7] = g_psSetupMenu->bEnableIcon_YunDuanCeLiang;
        if (enable[dwSpriteId])
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
    {
        if (dwSpriteId >= 6)
            return PASS;
        int lightIdx = g_psSetupMenu->bCustomizeIcon[dwSpriteId];
        if (lightIdx < 0)
            return PASS;
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        STXPGSPRITE * lightSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_ICON, 6 + lightIdx);
        if (pstMask && lightSprite)
            xpgRoleDrawMask(lightSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("Auto_work", strlen("Auto_work")))
    {
        if (dwSpriteId >= 100)
            return PASS;
        if (dwSpriteId <= 5)
        {
            int lightIdx = g_psSetupMenu->bCustomizeIcon[dwSpriteId];
            if (lightIdx < 0)
                return PASS;
            if (g_psSetupMenu->bCustomizeIconEnable[dwSpriteId] == 0)
                return PASS;
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            STXPGSPRITE * lightSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_LIGHT_ICON, 100 + lightIdx);
            if (pstMask && lightSprite)
                xpgRoleDrawMask(lightSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        
    }
    else if (dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
    {
        if (dwSpriteId == 0 && g_psSetupMenu->bHotUpMode == SETUP_MENU_HOT_UP_MODE_AUTO)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, getstr(Str_ZiDong), pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth);
        }
        else if (dwSpriteId == 1 && g_psSetupMenu->bHotUpMode == SETUP_MENU_HOT_UP_MODE_MANUAL)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, getstr(Str_ShouDong), pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth);
        }
        else if (dwSpriteId == 2 && g_psSetupMenu->bPreHotEnable)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        
        if (dwSpriteId == 0)
            text = getstr(Str_YiBan);
        else if (dwSpriteId == 1)
            text = getstr(Str_BiaoZhun);
        else if (dwSpriteId == 2)
            text = getstr(Str_JingXi);
        else if (dwSpriteId == 3)
            text = getstr(Str_XianXin);
        else if (dwSpriteId == 4)
            text = getstr(Str_BaoCeng);
        else if (dwSpriteId == 5)
            text = getstr(Str_XPing);
        else if (dwSpriteId == 6)
            text = getstr(Str_YPing);
        else if (dwSpriteId == 7)
            text = getstr(Str_XYPing);
        else if (dwSpriteId == 8)
            text = getstr(Str_XYJiaoTi);

        if (dwSpriteId <= 2)
        {
            if (g_psSetupMenu->bRongJieZhiLiang == dwSpriteId)
            {
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);
            }
        }
        else if (dwSpriteId >= 3 && dwSpriteId <= 4)
        {
            if (g_psSetupMenu->bDuiXianFangShi == dwSpriteId - 3)
            {
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);
            }
        }
        else if (dwSpriteId >= 5)
        {
            if (g_psSetupMenu->bPingXianFangShi == dwSpriteId - 5)
            {
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);
            }
        }
    }
    else if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")))
    {
        if (g_psSetupMenu->bCurrFusionMode != dwSpriteId)
            return PASS;
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        text = (char*) FModeStrList[dwSpriteId];
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);   
    }
    
    return PASS;
}

SWORD xpgDrawSprite_DarkIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE *pstMask;
    WORD wX = pstSprite->m_wPx;
    WORD wY = pstSprite->m_wPy;
    WORD wW = pstSprite->m_wWidth;
    WORD wH = pstSprite->m_wHeight;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;
    
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("SetYun", strlen("SetYun")))
    {
        if (!g_psSetupMenu->bCloudMode)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("Auto_work", strlen("Auto_work")))
    {
        if (dwSpriteId >= 100)
            return PASS;
        if (dwSpriteId <= 5)
        {
            int darkIdx = g_psSetupMenu->bCustomizeIcon[dwSpriteId];
            if (darkIdx < 0)
                return PASS;
            if (g_psSetupMenu->bCustomizeIconEnable[dwSpriteId] != 0)       // disable
                return PASS;
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            STXPGSPRITE * darkSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_DARK_ICON, 100 + darkIdx);
            if (pstMask && darkSprite)
                xpgRoleDrawMask(darkSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("Manual_work", strlen("Manual_work")))
    {
        if (dwSpriteId == 0)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 1)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 3);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    
    return PASS;
}

SWORD xpgDrawSprite_Mask(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    return PASS;
}

SWORD xpgDrawSprite_Title(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")) || 
        dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_GongNengPeiZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("SetYun", strlen("SetYun")) ||
             dwHashKey == xpgHash("SetSleep", strlen("SetSleep")) ||
             dwHashKey == xpgHash("SetSound", strlen("SetSound")) ||
             dwHashKey == xpgHash("SetTime", strlen("SetTime")) ||
             dwHashKey == xpgHash("SetPassword", strlen("SetPassword")) ||
             dwHashKey == xpgHash("SetUi", strlen("SetUi")) ||
             dwHashKey == xpgHash("SetInfo", strlen("SetInfo")) )
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_XiTongSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")) ||
             dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")) ||
             dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_RongJieSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionModeSet", strlen("FusionModeSet")) )
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_XiTongSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("Record", strlen("Record")) )
    {
        if (dwSpriteId == 0)
        {
            Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_RongJieJiLu), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwSpriteId == 1)
        {
            Idu_PaintWinArea(pWin, 14, 100, 770, 30, RGB2YUV(0x36, 0x36, 0x36));
        }
    }

    return PASS;
}

SWORD xpgDrawSprite_Aside(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")) || 
        dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")) ||
        dwHashKey == xpgHash("SetYun", strlen("SetYun")) ||
        dwHashKey == xpgHash("SetSleep", strlen("SetSleep")) ||
        dwHashKey == xpgHash("SetSound", strlen("SetSound")) ||
        dwHashKey == xpgHash("SetTime", strlen("SetTime")) ||
        dwHashKey == xpgHash("SetPassword", strlen("SetPassword")) ||
        dwHashKey == xpgHash("SetUi", strlen("SetUi")) ||
        dwHashKey == xpgHash("SetInfo", strlen("SetInfo")) ||
        dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")) ||
        dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")) ||
        dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
    {
        Idu_PaintWinArea(pWin, 248, 0, 2, pWin->wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
    }    
    return PASS;
}

SWORD xpgDrawSprite_BackIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")) || 
        dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")) ||
        dwHashKey == xpgHash("SetYun", strlen("SetYun")) ||
        dwHashKey == xpgHash("SetSleep", strlen("SetSleep")) ||
        dwHashKey == xpgHash("SetSound", strlen("SetSound")) ||
        dwHashKey == xpgHash("SetTime", strlen("SetTime")) ||
        dwHashKey == xpgHash("SetPassword", strlen("SetPassword")) ||
        dwHashKey == xpgHash("SetUi", strlen("SetUi")) ||
        dwHashKey == xpgHash("SetInfo", strlen("SetInfo")) ||
        dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")) ||
        dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")) ||
        dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")) ||
        dwHashKey == xpgHash("Record", strlen("Record")))
    {
        char tmpbuf[64];
        SetCurrIduFontID(FONT_ID_HeiTi19);
        sprintf(tmpbuf, " < %s", getstr(Str_FanHui));
        Idu_PrintString(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        xpgSpriteSetTouchArea(pstSprite, 0, 0, 120, 40);
    }
    else if (dwHashKey == xpgHash("FusionModeSet", strlen("FusionModeSet")))
    {
        char tmpbuf[64];
        SetCurrIduFontID(FONT_ID_HeiTi19);
        sprintf(tmpbuf, " < %s", getstr(Str_RongJieSheZhi));
        Idu_PrintString(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        xpgSpriteSetTouchArea(pstSprite, 0, 0, 200, 40);
    }
    
    return PASS;
}

SWORD xpgDrawSprite_CloseIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    xpgDrawSprite(pWin, pstSprite, boClip);
    xpgSpriteEnableTouch(pstSprite);
    return PASS;
}

SWORD xpgDrawSprite_Text(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char * text = "";
    DWORD dwTextId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("Main", strlen("Main")))
    {
        if (dwTextId == 0)
            text = getstr(Str_YongHuWeiHu);
        else if (dwTextId == 1)
            text = getstr(Str_RongJieJiLu);
        else if (dwTextId == 2)
            text = getstr(Str_RongJieSheZhi);
        else if (dwTextId == 3)
            text = getstr(Str_ShouDongMoShi);
        else if (dwTextId == 4)
            text = getstr(Str_GongJuXiang);
        else if (dwTextId == 5)
            text = getstr(Str_XiTongSheZhi);
        else if (dwTextId == 6)
            text = getstr(Str_GongNengPeiZhi);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")))
    {
        if (dwTextId == 0)
            text = getstr(Str_LaLiCeShi);
        else if (dwTextId == 1)
            text = getstr(Str_DuanMianJianCe);
        else if (dwTextId == 2)
            text = getstr(Str_ZiDongDuiJiao);
        else if (dwTextId == 3)
            text = getstr(Str_JiaoDuJianCe);
        else if (dwTextId == 4)
            text = getstr(Str_BaoCunTuXiang);
        else if (dwTextId == 5)
            text = getstr(Str_HuiChenJianCe);
        else if (dwTextId == 6)
            text = getstr(Str_RongJieZanTing);
        else if (dwTextId == 7)
            text = getstr(Str_YunDuanCeLiang);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
    {
        if (dwTextId <= 5)
        {
            BYTE value = g_psSetupMenu->bCustomizeIcon[dwTextId];
            if (value == 0)
                text = getstr(Str_YuJiaRe);
            else if (value == 1)
                text = getstr(Str_LaLiCeShi);
            else if (value == 2)
                text = getstr(Str_DuanMianJianCe);
            else if (value == 3)
                text = getstr(Str_ZiDongGuanJi);
            else if (value == 4)
                text = getstr(Str_JiaoDuJianCe);
            else if (value == 5)
                text = getstr(Str_FangDianJiaoZheng);
            else if (value == 6)
                text = getstr(Str_HuiChenJianCe);
            else if (value == 7)
                text = getstr(Str_RongJieZanTing);
            else if (value == 8)
                text = getstr(Str_HongGuangBi);
            else if (value == 9)
                text = getstr(Str_GuangGongLvJi);
            else if (value == 10)
                text = getstr(Str_BaoCunTuXiang);
            else if (value == 11)
                text = getstr(Str_YunDuanCeLiang);
            else 
                return PASS;
        }
        else if (dwTextId >= 6 && dwTextId <= 17)
        {
            if (dwTextId == 6)
                text = getstr(Str_YuJiaRe);
            else if (dwTextId == 7)
                text = getstr(Str_LaLiCeShi);
            else if (dwTextId == 8)
                text = getstr(Str_DuanMianJianCe);
            else if (dwTextId == 9)
                text = getstr(Str_ZiDongGuanJi);
            else if (dwTextId == 10)
                text = getstr(Str_JiaoDuJianCe);
            else if (dwTextId == 11)
                text = getstr(Str_FangDianJiaoZheng);
            else if (dwTextId == 12)
                text = getstr(Str_HuiChenJianCe);
            else if (dwTextId == 13)
                text = getstr(Str_RongJieZanTing);
            else if (dwTextId == 14)
                text = getstr(Str_HongGuangBi);
            else if (dwTextId == 15)
                text = getstr(Str_GuangGongLvJi);
            else if (dwTextId == 16)
                text = getstr(Str_BaoCunTuXiang);
            else if (dwTextId == 17)
                text = getstr(Str_YunDuanCeLiang);
        }
        else if (dwTextId == 18){
            text = getstr(Str_XianShi);
        }
        else if (dwTextId == 19){
            text = getstr(Str_TianJiaKongZhi);
        }

        if (dwTextId <= 17)
            SetCurrIduFontID(FONT_ID_HeiTi16);
        else 
            SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("ToolBox", strlen("ToolBox")))
    {
        if (dwTextId == 0)
            text = getstr(Str_HongGuangBi);
        else if (dwTextId == 1)
            text = getstr(Str_GuangGongLvJi);
        else if (dwTextId == 2)
            text = getstr(Str_Str_DuanMianJianCeYi);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 66);
    }
    else if (dwHashKey == xpgHash("Manual_work", strlen("Manual_work")))
    {
        char tempStr[64];
        if (dwTextId == 0)
        {
            text = getstr(Str_ZuoDianJi);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 1)
        {
            text = getstr(Str_SuDuMaoHao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 2)
        {
            sprintf(tempStr, "%d", 10);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_FontColorSet(0,0,0);
            Idu_PrintStringCenter(pWin, tempStr, 4, 139, 0, 74);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dwTextId == 3)
        {
            text = getstr(Str_BuShuMaoHao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 66);
        }
        else if (dwTextId == 4)
        {
            sprintf(tempStr, "%d", 100);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_FontColorSet(0,0,0);
            Idu_PrintStringCenter(pWin, tempStr, 4, 190, 0, 74);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dwTextId == 5)
        {
            text = getstr(Str_YouDianJi);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 6)
        {
            text = getstr(Str_SuDuMaoHao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 7)
        {
            sprintf(tempStr, "%d", 10);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_FontColorSet(0,0,0);
            Idu_PrintStringCenter(pWin, tempStr, 722, 139, 0, 74);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dwTextId == 8)
        {
            text = getstr(Str_BuShuMaoHao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 9)
        {
            sprintf(tempStr, "%d", 100);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_FontColorSet(0,0,0);
            Idu_PrintStringCenter(pWin, tempStr, 722, 190, 0, 74);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dwTextId == 10)
        {
            text = getstr(Str_SuDuMaoHao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 11)
        {
            sprintf(tempStr, "%d", 10);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_FontColorSet(0,0,0);
            Idu_PrintStringCenter(pWin, tempStr, 212, 380, 0, 70);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dwTextId == 12)
        {
            text = getstr(Str_BuShuMaoHao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 13)
        {
            sprintf(tempStr, "%d", 100);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_FontColorSet(0,0,0);
            Idu_PrintStringCenter(pWin, tempStr, 212, 406, 0, 70);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dwTextId == 14)
        {
            sprintf(tempStr, "%ds|%ds", 20, 19);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, tempStr, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 15)
        {
            sprintf(tempStr, "%d%s", 50, getstr(Str_DuC));
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, tempStr, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
    }
    else if (dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
    {
        BYTE tmpbuf[128];
        if (dwTextId == 0)
            sprintf(tmpbuf, "%s >", getstr(Str_ZiDingYi));
        else if (dwTextId == 1)
            sprintf(tmpbuf, "50%s", getstr(Str_DuC));
        else if (dwTextId == 2)
            sprintf(tmpbuf, "%dS", 12);
        else
            return PASS;
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
    }
    else if (dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")))
    {
        if (dwTextId == 0)
            text = getstr(Str_RongJieZhiLiang);
        else if (dwTextId == 1)
            text = getstr(Str_DuiXianFangShi);
        else if (dwTextId == 2)
            text = getstr(Str_PingXianFangShi);
        else
            return PASS;
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")))
    {
        int mode;
        MODEPARAM * pstModeParam;
        char * titleText = "";
        char tmpText[256];
        const BYTE JIANGE = 24;
        
        if (dwTextId != 0)
            return PASS;
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, 468, 250, RGB2YUV(0x24, 0x24, 0x24));
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 36, 468, 2, RGB2YUV(0x31, 0x31, 0x31));

        mode = g_psSetupMenu->bCurrFusionMode;
        if (mode == 0)
            pstModeParam = &(g_psSetupMenu->SM);
        else if (mode == 1)
            pstModeParam = &(g_psSetupMenu->MM);
        else if (mode == 2)
            pstModeParam = &(g_psSetupMenu->DS);
        else if (mode == 3)
            pstModeParam = &(g_psSetupMenu->NZ);
        else if (mode == 4)
            pstModeParam = &(g_psSetupMenu->BIF);
        else if (mode == 5)
            pstModeParam = &(g_psSetupMenu->CZ1);
        else if (mode == 6)
            pstModeParam = &(g_psSetupMenu->CZ2);
        else if (mode == 7)
            pstModeParam = &(g_psSetupMenu->AUTO);
        else
            return PASS;

        text = (char*) FModeStrList[mode];
        sprintf(tmpText, "%s%s", text, getstr(Str_MoShiCanShu));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, tmpText, 304, 174, 0, 0);

        SetCurrIduFontID(FONT_ID_HeiTi16);
        sprintf(tmpText, "%s:", getstr(Str_FangDianZhongXin));
        Idu_PrintString(pWin, tmpText, 306, 222 + JIANGE * 0, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_RongJieDianYa));
        Idu_PrintString(pWin, tmpText, 306, 222 + JIANGE * 1, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_YuRongDianYa));
        Idu_PrintString(pWin, tmpText, 306, 222 + JIANGE * 2, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_ChuChenDianYa));
        Idu_PrintString(pWin, tmpText, 306, 222 + JIANGE * 3, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_RongJieChongDieLiang));
        Idu_PrintString(pWin, tmpText, 306, 222 + JIANGE * 4, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_DuiJiaoMuBiaoZhi));
        Idu_PrintString(pWin, tmpText, 306, 222 + JIANGE * 5, 0, 0);

        sprintf(tmpText, "%s:", getstr(Str_RongJieShiJian));
        Idu_PrintString(pWin, tmpText, 536, 222 + JIANGE * 1, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_YuRongShiJian));
        Idu_PrintString(pWin, tmpText, 536, 222 + JIANGE * 2, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_ChuChenShiJian));
        Idu_PrintString(pWin, tmpText, 536, 222 + JIANGE * 3, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_QieGeJiaoDuShangXian));
        Idu_PrintString(pWin, tmpText, 536, 222 + JIANGE * 4, 0, 0);
        sprintf(tmpText, "%s:", getstr(Str_FangDianJiaoDuShangXian));
        Idu_PrintString(pWin, tmpText, 536, 222 + JIANGE * 5, 0, 0);

        Idu_PrintString(pWin, "mV", 468, 222 + JIANGE * 1, 0, 0);
        Idu_PrintString(pWin, "mV", 468, 222 + JIANGE * 2, 0, 0);
        Idu_PrintString(pWin, "mV", 468, 222 + JIANGE * 3, 0, 0);
        Idu_PrintString(pWin, "um", 468, 222 + JIANGE * 4, 0, 0);

        Idu_PrintString(pWin, "ms", 724, 222 + JIANGE * 1, 0, 0);
        Idu_PrintString(pWin, "ms", 724, 222 + JIANGE * 2, 0, 0);
        Idu_PrintString(pWin, "mV", 724, 222 + JIANGE * 3, 0, 0);
        Idu_PrintString(pWin, getstr(Str_Du), 724, 222 + JIANGE * 4, 0, 0);

        Idu_FontColorSet(22,160,255);
        sprintf(tmpText, "%d", pstModeParam->fangDianZhongXin);
        Idu_PrintStringCenter(pWin, tmpText, 394, 222 + JIANGE * 0, 0, 74);
        sprintf(tmpText, "%d", pstModeParam->rongJieDianYa);
        Idu_PrintStringCenter(pWin, tmpText, 394, 222 + JIANGE * 1, 0, 74);
        sprintf(tmpText, "%d", pstModeParam->yuRongDianYa);
        Idu_PrintStringCenter(pWin, tmpText, 394, 222 + JIANGE * 2, 0, 74);
        sprintf(tmpText, "%d", pstModeParam->chuChenDianYa);
        Idu_PrintStringCenter(pWin, tmpText, 394, 222 + JIANGE * 3, 0, 74);
        sprintf(tmpText, "%d", pstModeParam->rongJieChongDieLiang);
        Idu_PrintStringCenter(pWin, tmpText, 394, 222 + JIANGE * 4, 0, 74);
        sprintf(tmpText, "%d", pstModeParam->duiJiaoMuBiaoZhi);
        Idu_PrintStringCenter(pWin, tmpText, 394, 222 + JIANGE * 5, 0, 74);

        sprintf(tmpText, "%d", pstModeParam->rongJieShiJian);
        Idu_PrintStringCenter(pWin, tmpText, 644, 222 + JIANGE * 1, 0, 76);
        sprintf(tmpText, "%d", pstModeParam->yuRongShiJian);
        Idu_PrintStringCenter(pWin, tmpText, 644, 222 + JIANGE * 2, 0, 76);
        sprintf(tmpText, "%d", pstModeParam->chuChenShiJian);
        Idu_PrintStringCenter(pWin, tmpText, 644, 222 + JIANGE * 3, 0, 76);
        sprintf(tmpText, "%d.%d", pstModeParam->qieGeJiaoDuShangXian >> 6, pstModeParam->qieGeJiaoDuShangXian & 0x3f);
        Idu_PrintStringCenter(pWin, tmpText, 644, 222 + JIANGE * 4, 0, 76);
        sprintf(tmpText, "%d", pstModeParam->fangDianJiaoZhengMuBiaoZhi);
        Idu_PrintStringCenter(pWin, tmpText, 644, 222 + JIANGE * 5, 0, 76);
        
        Idu_FontColorSet(0xff,0xff,0xff);
    }
    else if (dwHashKey == xpgHash("FusionModeSet", strlen("FusionModeSet")))
    {
        char tmpText[256];

        SetCurrIduFontID(FONT_ID_HeiTi16);
        if (dwTextId <= 10)
        {
            switch(dwTextId)
            {
            case 0:
                sprintf(tmpText, "%s:", getstr(Str_FangDianZhongXin));
                break;
            case 1:
                sprintf(tmpText, "%s:", getstr(Str_RongJieDianYa));
                break;
            case 2:
                sprintf(tmpText, "%s:", getstr(Str_YuRongDianYa));
                break;
            case 3:
                sprintf(tmpText, "%s:", getstr(Str_ChuChenDianYa));
                break;
            case 4:
                sprintf(tmpText, "%s:", getstr(Str_RongJieChongDieLiang));
                break;
            case 5:
                sprintf(tmpText, "%s:", getstr(Str_DuiJiaoMuBiaoZhi));
                break;
            case 6:
                sprintf(tmpText, "%s:", getstr(Str_RongJieShiJian));
                break;
            case 7:
                sprintf(tmpText, "%s:", getstr(Str_YuRongShiJian));
                break;
            case 8:
                sprintf(tmpText, "%s:", getstr(Str_ChuChenShiJian));
                break;
            case 9:
                sprintf(tmpText, "%s:", getstr(Str_QieGeJiaoDuShangXian));
                break;
            case 10:
                sprintf(tmpText, "%s:", getstr(Str_FangDianJiaoDuShangXian));
                break;
            }
            Idu_PrintString(pWin, tmpText, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId >= 11 && dwTextId <= 18)
        {
            switch(dwTextId)
            {
            case 11:
                text = "mV";
                break;
            case 12:
                text = "mV";
                break;
            case 13:
                text = "mV";
                break;
            case 14:
                text = "um";
                break;
            case 15:
                text = "ms";
                break;
            case 16:
                text = "ms";
                break;
            case 17:
                text = "mV";
                break;
            case 18:
                text = getstr(Str_Du);
                break;
            }
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
    }
    
    return PASS;
}


SWORD xpgDrawSprite_Selector(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char * text = "";
    DWORD dwSelectorId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")))
    {
        if (dwSelectorId == 0)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_GongNengPeiZhi), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
        }
        else if (dwSelectorId == 1)
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, RGB2YUV(0x0B, 0x0B, 0x0B));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_ZiDingYiTuBiao), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            xpgSpriteEnableTouch(pstSprite);
        }
    }
    else if (dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
    {
        if (dwSelectorId == 0)
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, RGB2YUV(0x0B, 0x0B, 0x0B));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_GongNengPeiZhi), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            xpgSpriteEnableTouch(pstSprite);
        }
        else if (dwSelectorId == 1)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_ZiDingYiTuBiao), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
        }
    }
    else if (dwHashKey == xpgHash("SetYun", strlen("SetYun")) ||
            dwHashKey == xpgHash("SetSleep", strlen("SetSleep")) ||
            dwHashKey == xpgHash("SetSound", strlen("SetSound")) ||
            dwHashKey == xpgHash("SetTime", strlen("SetTime")) ||
            dwHashKey == xpgHash("SetPassword", strlen("SetPassword")) ||
            dwHashKey == xpgHash("SetUi", strlen("SetUi")) ||
            dwHashKey == xpgHash("SetInfo", strlen("SetInfo")))
    {
        DWORD curPageType;
        if(dwSelectorId == 0)
            text = getstr(Str_YunDuanSheZhi);
        else if (dwSelectorId == 1)
            text = getstr(Str_PingMuYuDaiJi);
        else if (dwSelectorId == 2)
            text = getstr(Str_ShengYinSheZhi);
        else if (dwSelectorId == 3)
            text = getstr(Str_ShiJianYuYuYan);
        else if (dwSelectorId == 4)
            text = getstr(Str_MiMaSheZhi);
        else if (dwSelectorId == 5)
            text = getstr(Str_JieMianFengGe);
        else //if (dwSelectorId == 6)
            text = getstr(Str_XiTongXinXi);

        if (dwHashKey == xpgHash("SetYun", strlen("SetYun")))
            curPageType = 0;
        else if (dwHashKey == xpgHash("SetSleep", strlen("SetSleep")))
            curPageType = 1;
        else if (dwHashKey == xpgHash("SetSound", strlen("SetSound")))
            curPageType = 2;
        else if (dwHashKey == xpgHash("SetTime", strlen("SetTime")))
            curPageType = 3;
        else if (dwHashKey == xpgHash("SetPassword", strlen("SetPassword")))
            curPageType = 4;
        else if (dwHashKey == xpgHash("SetUi", strlen("SetUi")))
            curPageType = 5;
        else //if (dwHashKey == xpgHash("SetInfo", strlen("SetInfo")))
            curPageType = 6;
        
        if (dwSelectorId == curPageType)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, text, pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
        }
        else
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, RGB2YUV(0x0B, 0x0B, 0x0B));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, text, pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            xpgSpriteEnableTouch(pstSprite);
        }
    }
    else if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")) ||
             dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")) ||
             dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")) )
    {
        DWORD curPageType;
        if(dwSelectorId == 0)
            text = getstr(Str_RongJieMoShi);
        else if (dwSelectorId == 1)
            text = getstr(Str_RongJieSheZhi);
        else //if (dwSelectorId == 2)
            text = getstr(Str_JiaReSheZhi);
        
        if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")))
            curPageType = 0;
        else if (dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")))
            curPageType = 1;
        else //if (dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
            curPageType = 2;
        
        if (dwSelectorId == curPageType)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, text, pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
        }
        else
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, RGB2YUV(0x0B, 0x0B, 0x0B));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, text, pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            xpgSpriteEnableTouch(pstSprite);
        }
    }
    






}

SWORD xpgDrawSprite_List(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char * text = "";
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    DWORD dwListId = pstSprite->m_dwTypeIndex;
    
    if (dwHashKey == xpgHash("SetYun", strlen("SetYun")))
    {
        if (dwListId == 0)
            text = getstr(Str_YunDuanMoShi);
        else if (dwListId == 1)
            text = getstr(Str_MAD);
        else if (dwListId == 2)
            text = getstr(Str_YunDuanOPMZhuangTai);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetSleep", strlen("SetSleep")))
    {
        if (dwListId == 0)
            text = getstr(Str_ZhiNengBeiGuang);
        else if (dwListId == 1)
            text = getstr(Str_LiangDuTiaoJie);
        else if (dwListId == 2)
            text = getstr(Str_ZiDongGuanJi);
        else if (dwListId == 3)
            text = getstr(Str_GuanJiShiJian);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetSound", strlen("SetSound")))
    {
        if (dwListId == 0)
            text = getstr(Str_ChuPingShengYin);
        else if (dwListId == 1)
            text = getstr(Str_YinLiangTiaoJie);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetTime", strlen("SetTime")))
    {
        if (dwListId == 0)
            text = getstr(Str_24XiaoShiZhi);
        else if (dwListId == 1)
            text = getstr(Str_ShiJianSheZhi);
        else if (dwListId == 2)
            text = getstr(Str_RiQiSheZhi);
        else if (dwListId == 3)
            text = getstr(Str_RiQiGeShi);
        else if (dwListId == 4)
            text = getstr(Str_XiTongYuYan);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetPassword", strlen("SetPassword")))
    {
        if (dwListId == 0)
            text = getstr(Str_KaiJiMiMa);
        else if (dwListId == 1)
            text = getstr(Str_SheZhiKaiJiMiMa);
        else if (dwListId == 2)
            text = getstr(Str_ZuJieMiMa);
        else if (dwListId == 3)
            text = getstr(Str_SheZhiZuJieMiMa);
        else if (dwListId == 4)
            text = getstr(Str_ZuJieRiQi);
        else if (dwListId == 5)
            text = getstr(Str_SuoDingRongJieCiShu);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetUi", strlen("SetUi")))
    {
        if (dwListId == 0)
            text = getstr(Str_ZhuJieMianFengGe);
        else if (dwListId == 1)
            text = getstr(Str_YangShiYanSe);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetInfo", strlen("SetInfo")))
    {
        if (dwListId == 0)
            text = getstr(Str_GuanYuBenJi);
        else if (dwListId == 1)
            text = getstr(Str_DianJiBangXinXi);
        else if (dwListId == 2)
            text = getstr(Str_WenDuXinXi);
        else if (dwListId == 3)
            text = getstr(Str_DianChiXinXi);
        else if (dwListId == 4)
            text = getstr(Str_RongJieZongCiShu);
        else if (dwListId == 5)
            text = getstr(Str_GuJianBanBenHao);
        else if (dwListId == 6)
            text = getstr(Str_XiTongBanBenHao);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("User", strlen("User")))
    {
        if (dwListId == 0)
            text = getstr(Str_ZiDongRongJieMoShi);
        else if (dwListId == 1)
            text = getstr(Str_GongChangTiaoXinMoShi);
        else if (dwListId == 2)
            text = getstr(Str_FangDianJiaoZhengMoShi);
        else if (dwListId == 3)
            text = getstr(Str_PingMuHuiChenJianCe);
        else if (dwListId == 4)
            text = getstr(Str_GongChangMoShi);
        else if (dwListId == 5)
            text = getstr(Str_DianJiBangJiHuo);
        
        xpgDrawSprite(pWin, pstSprite, boClip);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 3, 0, 300);
        xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("Record", strlen("Record")))
    {
        char tmpbuff[128];
        STRECORD * pr;
        int iCurIndex;
        DWORD dwCurPageStart = 0;
        DWORD dwTotal = GetRecordTotal();
        DWORD dwPageTotal = dwTotal / PAGE_RECORD_SIZE;
        if (dwTotal % PAGE_RECORD_SIZE)
            dwPageTotal++;
        
        if (g_dwRecordListCurrPage >= dwPageTotal)
            g_dwRecordListCurrPage = dwPageTotal - 1;
        
        
        dwCurPageStart = PAGE_RECORD_SIZE * g_dwRecordListCurrPage;
        dwCurPageStart = dwTotal - 1 - dwCurPageStart;                  // Fan Guo Lai

        iCurIndex =  dwCurPageStart - dwListId;
        if (iCurIndex < 0)
            return PASS;

        pr = GetRecord((DWORD)iCurIndex);
        if (pr == NULL)
            return PASS;

        sprintf(tmpbuff, "%03d", iCurIndex);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, tmpbuff, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);

        sprintf(tmpbuff, "[%04d/%d/%d %02d:%02d:%02d]", pr->wYear, pr->bMonth, pr->bDay, pr->bHour, pr->bMinute, pr->bSecond);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, tmpbuff, pstSprite->m_wPx + 40, pstSprite->m_wPy, 0, 0);

        sprintf(tmpbuff, "%s", pr->bRecordName);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, tmpbuff, pstSprite->m_wPx + 270, pstSprite->m_wPy, 0, 0);

        sprintf(tmpbuff, "%d.%ddB", pr->dwPowerWaste >> 6, pr->dwPowerWaste & 0x3F);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, tmpbuff, pstSprite->m_wPx + 510, pstSprite->m_wPy, 0, 0);

        sprintf(tmpbuff, "ChaKan" );
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, tmpbuff, pstSprite->m_wPx + 638, pstSprite->m_wPy, 0, 0);

        Idu_PaintWinArea(pWin, 14, pstSprite->m_wPy + 35, 744, 2, RGB2YUV(0x20, 0x20, 0x20));
    }
    else if (dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3")))
    {
        if (dwListId == 0)
            text = getstr(Str_YuReMoShi);
        else if (dwListId == 1)
            text = getstr(Str_JiaReFangShi);
        else if (dwListId == 2)
            text = getstr(Str_ReSuGuanSheZhi);
        else if (dwListId == 3)
            text = getstr(Str_JiaReWenDu);
        else if (dwListId == 4)
            text = getstr(Str_JiaReShiJian);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 42, 472, 2, RGB2YUV(0x37, 0x37, 0x37));
    }
    
    return PASS;
}


void MakeDialogRole(STXPGROLE * pstRole, WORD width, WORD height)
{
    DWORD dwSize;
    ST_IMGWIN win;
    ST_IMGWIN * pWin;
    
    xpgRoleInit(pstRole);
    pstRole->m_bBitDepth = 24;
    pstRole->m_wIndex = 0;
    pstRole->m_wWidth = width;
    if (pstRole->m_wWidth % 2)
        pstRole->m_wWidth ++;
    pstRole->m_wHeight = height;
    if (pstRole->m_wHeight < 41)
        pstRole->m_wHeight = 41;
    
    dwSize = ALIGN_16(pstRole->m_wWidth) * ALIGN_16(pstRole->m_wHeight) * 2 + 256;

    pstRole->m_pRawImage = (BYTE *) ext_mem_malloc(dwSize);
    pstRole->m_wRawWidth = pstRole->m_wWidth;

	MP_ASSERT(pstRole->m_pRawImage != NULL);
    MP_DEBUG4("malloc role %d, w%d h%d raw_w%d", pstRole->m_wIndex, pstRole->m_wWidth, pstRole->m_wHeight, pstRole->m_wRawWidth);

    mpWinInit(&win, pstRole->m_pRawImage, pstRole->m_wHeight, pstRole->m_wWidth);
    pWin = &win;

    Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YVU(0x28, 0x29, 0x2D));
    Idu_PaintWinArea(pWin, 0, 40, pWin->wWidth, pstRole->m_wHeight - 40, RGB2YVU(0x1C, 0x1D, 0x21));
    
    return;
}

void MakeMaskRole(STXPGROLE * pstMaskRole, int maskRoleIndex, WORD width, WORD height)
{
    ST_IMGWIN win1, win2;
    ST_IMGWIN * pWin1, * pWin2;
    DWORD * pdwStart1, * pdwStart2;
    WORD iconWidth;
    WORD iconHeight;
    WORD corner;
    
    if (maskRoleIndex == XPG_ROLE_ICON_MASK_0)
    {
        iconWidth = 66;
        iconHeight = 66;
        corner = 12;
    }
    else 
    {
        return;
    }
    
    pdwStart1 = (DWORD*) ext_mem_malloc((iconHeight) * (iconWidth*2) * 2);
    mpWinInit(&win1, pdwStart1, iconHeight, iconWidth * 2);
    pWin1 = &win1;
    xpgDirectDrawRoleOnWin(pWin1, g_pstXpgMovie->m_pstObjRole[maskRoleIndex], 0, 0, NULL, 0);

    if (width < corner * 2)
        width = corner * 2;
    if (height < corner * 2)
        height = corner * 2;
    if (width % 2)
        width ++;
    
    pdwStart2 = (DWORD*) ext_mem_malloc(width * height * 2);
    mpWinInit(&win2, pdwStart2, height, width);
    pWin2 = &win2;
    mpPaintWin(pWin2, 0xffff8080);

    mpCopyWinAreaSameSize(pWin1, pWin2, 0, 0, 0, 0, corner, corner);
    mpCopyWinAreaSameSize(pWin1, pWin2, iconWidth - corner, 0, width - corner, 0, corner, corner);
    mpCopyWinAreaSameSize(pWin1, pWin2, 0, iconHeight - corner, 0, height - corner, corner, corner);
    mpCopyWinAreaSameSize(pWin1, pWin2, iconWidth - corner, iconHeight - corner, width - corner, height - corner, corner, corner);

    ext_mem_free(pdwStart1);
    
    xpgRoleInit(pstMaskRole);
    pstMaskRole->m_bBitDepth = 24;
    pstMaskRole->m_wIndex = 0;
    pstMaskRole->m_wWidth = width;
    pstMaskRole->m_wHeight = height;
    pstMaskRole->m_pRawImage = pdwStart2;
    pstMaskRole->m_wRawWidth = pstMaskRole->m_wWidth;

    return;
}


SWORD xpgDrawSprite_Dialog(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGROLE  stRole, stMaskRole;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    xpgRoleInit(&stRole);
    xpgRoleInit(&stMaskRole);
    
    if (dwHashKey == xpgHash("User", strlen("User")))
    {
        MakeDialogRole(&stRole, 400, 440);
        MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, 400, 440);
        xpgRoleDrawMask(&stRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, getstr(Str_YongHuWeiHu), pstSprite->m_wPx, pstSprite->m_wPy + 5, 0, pstSprite->m_wWidth);
    }
    else if (dwHashKey == xpgHash("ToolBox", strlen("ToolBox")))
    {
        MakeDialogRole(&stRole, 500, 250);
        MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, 500, 250);
        xpgRoleDrawMask(&stRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);

        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, getstr(Str_GongJuXiang), pstSprite->m_wPx, pstSprite->m_wPy + 5, 0, pstSprite->m_wWidth);
    }
    else
        xpgDrawSprite(pWin, pstSprite, boClip);

    xpgRoleRelease(&stRole);
    xpgRoleRelease(&stMaskRole);
        
    return PASS;
}

SWORD xpgDrawSprite_Status(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)         // type24
{
    return PASS;
}

SWORD xpgDrawSprite_Radio(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)          // type25
{
    STXPGSPRITE *pstMask;
    
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("SetYun", strlen("SetYun")))
    {
        if (g_psSetupMenu->bCloudMode)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("SetSleep", strlen("SetSleep")))
    {
    }
    else if (dwHashKey == xpgHash("SetSound", strlen("SetSound")))
    {
    }
    return PASS;
}

SWORD xpgDrawSprite_Scroll(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)          // type26
{
    STXPGSPRITE *pstMask;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;

    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("SetSleep", strlen("SetSleep")))
    {
        if (dwSpriteId == 0)
        {
            STXPGSPRITE *pstRound;
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            pstRound = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_SCROLL, 1); 
            if (pstMask && pstRound){

                DWORD xadd = (g_psSetupMenu->bBrightness) * 250 / 100;
                DWORD xpoint = pstSprite->m_wPx + (32-8) + xadd;
                
                xpgRoleDrawMask(pstRound->m_pstRole, pWin->pdwStart, xpoint, pstSprite->m_wPy + 6, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                Idu_PaintWinArea(pWin, pstSprite->m_wPx + 24, pstSprite->m_wPy + 12, xadd, 2, RGB2YUV(0x14, 0xb6, 0xff));
            }
            
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy-8, pstSprite->m_wWidth, pstSprite->m_wHeight+8);
        }
    }
    else if (dwHashKey == xpgHash("SetSound", strlen("SetSound")))
    {
        if (dwSpriteId == 0)
        {
            STXPGSPRITE *pstRound;
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            pstRound = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_SCROLL, 1); 
            if (pstMask && pstRound){

                DWORD xadd = (g_psSetupMenu->bVolume) * 250 / 100;
                DWORD xpoint = pstSprite->m_wPx + (32-8) + xadd;
                
                xpgRoleDrawMask(pstRound->m_pstRole, pWin->pdwStart, xpoint, pstSprite->m_wPy + 6, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                Idu_PaintWinArea(pWin, pstSprite->m_wPx + 24, pstSprite->m_wPy + 11, xadd, 2, RGB2YUV(0x14, 0xb6, 0xff));
            }
            
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy-8, pstSprite->m_wWidth, pstSprite->m_wHeight+8);
        }
    }
    return PASS;
}

SWORD xpgDrawSprite_Frame(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE *pstMask = NULL;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("Manual_work", strlen("Manual_work")))
    {
        if (dwSpriteId == 0)
        {
            Idu_PaintWin(pWin, RGB2YUV(0xC9, 0xCE, 0xE0));
            Idu_PaintWinArea(pWin, 0, 0,  pWin->wWidth, 40, RGB2YUV(0x18, 0x19, 0x1D));
            Idu_PaintWinArea(pWin, 0, 40, pWin->wWidth, 30, RGB2YUV(0x00, 0x4E, 0xFF));
            Idu_PaintWinArea(pWin, 0, 70, 82, 410, RGB2YUV(0x2F, 0x2F, 0x2F));
            Idu_PaintWinArea(pWin, 718, 70, 82, 410, RGB2YUV(0x2F, 0x2F, 0x2F));
            Idu_PaintWinArea(pWin, 4, 139, 74, 18, RGB2YUV(0xFF, 0xFF, 0xFF));
            Idu_PaintWinArea(pWin, 4, 190, 74, 18, RGB2YUV(0xFF, 0xFF, 0xFF));
            Idu_PaintWinArea(pWin, 722, 139, 74, 18, RGB2YUV(0xFF, 0xFF, 0xFF));
            Idu_PaintWinArea(pWin, 722, 190, 74, 18, RGB2YUV(0xFF, 0xFF, 0xFF));
        }
        else if (dwSpriteId == 1)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            Idu_PaintWinArea(pWin, 212, 380, 70, 18, RGB2YUV(0xFF, 0xFF, 0xFF));
            Idu_PaintWinArea(pWin, 212, 406, 70, 18, RGB2YUV(0xFF, 0xFF, 0xFF));
            Idu_PaintWinArea(pWin, 312, 368, 02, 70, RGB2YUV(0x47, 0x47, 0x47));
            Idu_PaintWinArea(pWin, 486, 368, 02, 70, RGB2YUV(0x47, 0x47, 0x47));
        }
    }
    if (dwHashKey == xpgHash("Auto_work", strlen("Auto_work")))
    {
        if (dwSpriteId == 0)
        {
            Idu_PaintWin(pWin, RGB2YUV(0xC9, 0xCE, 0xE0));
            Idu_PaintWinArea(pWin, 0, 0,  pWin->wWidth, 40, RGB2YUV(0x18, 0x19, 0x1D));
            Idu_PaintWinArea(pWin, 0, 40, pWin->wWidth, 30, RGB2YUV(0x00, 0x4E, 0xFF));
            Idu_PaintWinArea(pWin, 0, 370, pWin->wWidth, 30, RGB2YUV(0x00, 0x4E, 0xFF));
            Idu_PaintWinArea(pWin, 0, 400, pWin->wWidth, 80, RGB2YUV(0x1F, 0x21, 0x26));
        }
    }
    
    return PASS;
}



#endif





