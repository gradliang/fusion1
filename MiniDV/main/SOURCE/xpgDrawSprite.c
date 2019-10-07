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
MODEPARAM tempModeParam;
DWORD dwDialogTempValue = 0;
BOOL  boDialogValueIsFloat = 0;
char strEditPassword[8] = {0};                  // editing password
char strEditValue[32] = {0};                  // editing value
char * strDialogTitle = NULL;
DWORD * pdwEditingFusionValue = NULL;

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
// Dialog
//---------------------------------------------------------------------------
int popupDialog(int dialogType, char * backToPage)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    
    xpgAddDialog(dialogType, backToPage, Idu_GetCurrWin());
    //mpDebugPrint("dialogType = %d, backToPage = %s", dialogType, backToPage);
    
    if (dialogType == Dialog_ReSuGuan)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        dwDialogTempValue = g_psSetupMenu->bReSuGuanSheZhi;
    }
    else if (dialogType == Dialog_ModifyNumber)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
    }
    else if (dialogType == Dialog_ShutdownTime)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
    }
    else if (dialogType == Dialog_SetTime)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
    }
    else if (dialogType == Dialog_SetDate)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
    }
    else if (dialogType == Dialog_SetDateFormat)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 1, 0);
    }
    else if (dialogType == Dialog_About)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 3, 0);
    }
    else if (dialogType == Dialog_Times)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 3, 0);
    }
    else if (dialogType == Dialog_TempInfo)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 3, 0);
    }
    else if (dialogType == Dialog_BatInfo)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 3, 0);
    }
    else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 8, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 9, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 10, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 11, 0);
    }
    else if(dialogType == Dialog_Value)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
    }
    else if(dialogType == Dialog_EditValue)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 8, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 9, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 10, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 11, 0);
    }
    
    xpgSearchAndGotoPage(DIALOG_PAGE_NAME);
    return;
}

int exitDialog()
{
    char backPage[32];
    char * pageName = xpgGetCurrDialogBackPage();
    strncpy(backPage, pageName, sizeof(backPage) - 1);
    backPage[sizeof(backPage) - 1] = 0;
    xpgDeleteDialog();
    if (backPage[0] != 0)
        xpgSearchAndGotoPage(backPage);
    xpgUpdateStage();
}


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
    if (dwHashKey == xpgHash("Preview"))
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
	else if (dwHashKey == xpgHash("PhotoView"))
    {
			swData=g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
			
    }
	else if (dwHashKey == xpgHash("Setup"))
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

    if (dwHashKey == xpgHash("User") || 
        dwHashKey == xpgHash("ToolBox") )
    {
        mpCopyEqualWin(pWin, Idu_GetCacheWin());
    }
    else if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        ST_IMGWIN * pCacheWin = xpgGetCurrDialogCacheWin();
        if (pCacheWin != NULL && pCacheWin->pdwStart != NULL)
            mpCopyEqualWin(pWin, pCacheWin);
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

    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        STXPGROLE * pstRole = NULL;
        STXPGROLE * pstMask = NULL;
        if (dialogType == Dialog_ReSuGuan)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_SMALL_BUTTON_ICON];
            pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_SMALL_BUTTON_MASK];
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 246;
                wY = pstSprite->m_wPy = 224;
                text = "40mm";
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 356;
                wY = pstSprite->m_wPy = 224;
                text = "45mm";
            }
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = 462;
                wY = pstSprite->m_wPy = 224;
                text = "50mm";
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = 246;
                wY = pstSprite->m_wPy = 264;
                text = "55mm";
            }
            else if (dwSpriteId == 4)
            {
                wX = pstSprite->m_wPx = 356;
                wY = pstSprite->m_wPy = 264;
                text = "60mm";
            }
            else if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = 462;
                wY = pstSprite->m_wPy = 264;
                text = getstr(Str_ZiDingYi);
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 70;
            wH = pstSprite->m_wHeight = 25;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth); 
        }
        else if (dialogType == Dialog_ModifyNumber)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD_MINUS];
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 300;
                wY = pstSprite->m_wPy = 238;
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD_MASK];
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 460;
                wY = pstSprite->m_wPy = 238;
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_MINUS_MASK];
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 40;
            wH = pstSprite->m_wHeight = 40;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_ShutdownTime)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD_MINUS];
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 300;
                wY = pstSprite->m_wPy = 238;
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD_MASK];
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 460;
                wY = pstSprite->m_wPy = 238;
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_MINUS_MASK];
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 40;
            wH = pstSprite->m_wHeight = 40;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_SetTime)
        {
            if (g_psSetupMenu->b24HourFormat)
            {
                if (dwSpriteId == 0)
                {
                    xpgSpriteSetTouchArea(pstSprite, 304, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 1)
                {
                    xpgSpriteSetTouchArea(pstSprite, 304, 258, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 2)
                {
                    xpgSpriteSetTouchArea(pstSprite, 426, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 3)
                {
                    xpgSpriteSetTouchArea(pstSprite, 426, 258, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 4 || dwSpriteId == 5)
                {
                    return PASS;
                }
                else if (dwSpriteId == 6)
                {
                    wX = pstSprite->m_wPx = 205;
                    wY = pstSprite->m_wPy = 350;
                    pstSprite->m_wWidth = 195;
                    pstSprite->m_wHeight = 50;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 7)
                {
                    wX = pstSprite->m_wPx = 400;
                    wY = pstSprite->m_wPy = 350;
                    pstSprite->m_wWidth = 195;
                    pstSprite->m_wHeight = 50;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
            }
            else    // 12 hour
            {
                if (dwSpriteId == 6)
                {
                    wX = pstSprite->m_wPx = 205;
                    wY = pstSprite->m_wPy = 350;
                    pstSprite->m_wWidth = 195;
                    pstSprite->m_wHeight = 50;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 7)
                {
                    wX = pstSprite->m_wPx = 400;
                    wY = pstSprite->m_wPy = 350;
                    pstSprite->m_wWidth = 195;
                    pstSprite->m_wHeight = 50;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 0)
                {
                    xpgSpriteSetTouchArea(pstSprite, 224, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 1)
                {
                    xpgSpriteSetTouchArea(pstSprite, 224, 258, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 2)
                {
                    xpgSpriteSetTouchArea(pstSprite, 366, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 3)
                {
                    xpgSpriteSetTouchArea(pstSprite, 366, 258, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 4)
                {
                    xpgSpriteSetTouchArea(pstSprite, 488, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 5)
                {
                    xpgSpriteSetTouchArea(pstSprite, 488, 258, 70, 70);
                    return PASS;
                }
            }
        }
        else if (dialogType == Dialog_SetDate)
        {
            if (1)
            {
                if (dwSpriteId == 6)
                {
                    wX = pstSprite->m_wPx = 205;
                    wY = pstSprite->m_wPy = 350;
                    pstSprite->m_wWidth = 195;
                    pstSprite->m_wHeight = 50;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 7)
                {
                    wX = pstSprite->m_wPx = 400;
                    wY = pstSprite->m_wPy = 350;
                    pstSprite->m_wWidth = 195;
                    pstSprite->m_wHeight = 50;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 0)
                {
                    xpgSpriteSetTouchArea(pstSprite, 224, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 1)
                {
                    xpgSpriteSetTouchArea(pstSprite, 224, 258, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 2)
                {
                    xpgSpriteSetTouchArea(pstSprite, 366, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 3)
                {
                    xpgSpriteSetTouchArea(pstSprite, 366, 258, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 4)
                {
                    xpgSpriteSetTouchArea(pstSprite, 488, 134, 70, 70);
                    return PASS;
                }
                else if (dwSpriteId == 5)
                {
                    xpgSpriteSetTouchArea(pstSprite, 488, 258, 70, 70);
                    return PASS;
                }
            }
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG_LIST_DARK];
            pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG_LIST_MASK];
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 250;
                wY = pstSprite->m_wPy = 208;
                text = "YYYY/MM/DD";
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 250;
                wY = pstSprite->m_wPy = 260;
                text = "MM/DD/YYYY";
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 300;
            wH = pstSprite->m_wHeight = 40;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 6, 0, pstSprite->m_wWidth);
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword || dialogType == Dialog_EditValue)
        {
            const WORD x0 = 252, x1 = 352, x2 = 452;
            const WORD y0 = 210, y1 = 260, y2 = 310, y3 = 360;
            pstSprite->m_wWidth = 100;
            pstSprite->m_wHeight = 50;
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = x1;
                wY = pstSprite->m_wPy = y3;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_0], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = x0;
                wY = pstSprite->m_wPy = y0;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_1], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = x1;
                wY = pstSprite->m_wPy = y0;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_2], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = x2;
                wY = pstSprite->m_wPy = y0;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_3], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 4)
            {
                wX = pstSprite->m_wPx = x0;
                wY = pstSprite->m_wPy = y1;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_4], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = x1;
                wY = pstSprite->m_wPy = y1;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_5], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 6)
            {
                wX = pstSprite->m_wPx = x2;
                wY = pstSprite->m_wPy = y1;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_6], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 7)
            {
                wX = pstSprite->m_wPx = x0;
                wY = pstSprite->m_wPy = y2;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_7], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 8)
            {
                wX = pstSprite->m_wPx = x1;
                wY = pstSprite->m_wPy = y2;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_8], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 9)
            {
                wX = pstSprite->m_wPx = x2;
                wY = pstSprite->m_wPy = y2;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_9], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 10)
            {
                wX = pstSprite->m_wPx = x0;
                wY = pstSprite->m_wPy = y3;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_NULL];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_LEFT_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            }
            else if (dwSpriteId == 11)
            {
                wX = pstSprite->m_wPx = x2;
                wY = pstSprite->m_wPy = y3;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_BACKSPACE];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_RIGHT_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            }
        }
        else if(dialogType == Dialog_Value)
        {
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 278;
                wY = pstSprite->m_wPy = 274;
                text = getstr(Str_QueRen);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_BUTTON_OK_ICON];
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 438;
                wY = pstSprite->m_wPy = 274;
                text = getstr(Str_QuXiao);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_BUTTON_CANCEL_ICON];
            }
            else 
                return PASS;
            pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_BUTTON_MASK];
            
            wW = pstSprite->m_wWidth = 90;
            wH = pstSprite->m_wHeight = 30;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth);
            
        }
        
    }
    else if (dwHashKey == xpgHash("Main"))
    {
        xpgDrawSprite(pWin, pstSprite, boClip);
    }
    else if (dwHashKey == xpgHash("FuncSet"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, (dwSpriteId < 6) ? 1:0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("ToolBox"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("Record") )
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, dwSpriteId);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("Manual_work"))
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
    else if (dwHashKey == xpgHash("Auto_work"))
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
    else if (dwHashKey == xpgHash("FusionSet3"))
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
    else if (dwHashKey == xpgHash("FusionSet2"))
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
    else if (dwHashKey == xpgHash("FusionSet1"))
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
    else if (dwHashKey == xpgHash("FusionModeSet"))
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
    else if (dwHashKey == xpgHash("SetTime"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        if (dwSpriteId == 0)
            text = getstr(Str_JianTiZhongWen);
        else if (dwSpriteId == 1)
            text = getstr(Str_YingWen);
        else if (dwSpriteId == 2)
            text = getstr(Str_PuTaoYaWen);
        else if (dwSpriteId == 3)
            text = getstr(Str_XiBanYaWen);
        else if (dwSpriteId == 4)
            text = getstr(Str_EWen);
        else if (dwSpriteId == 5)
            text = getstr(Str_FaWen);
        else if (dwSpriteId == 6)
            text = getstr(Str_TaiWen);
        else if (dwSpriteId == 7)
            text = getstr(Str_ALaBoWen);
        else
            return PASS;
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
    }
    else if (dwHashKey == xpgHash("SetYun"))
    {
        xpgDrawSprite(pWin, pstSprite, boClip);
    }
    else if(dwHashKey == xpgHash("RedLight"))
    {
        if (dwSpriteId == 0)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 1)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 2)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 3)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 4)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 5)
        {
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

    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        STXPGROLE * pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_SMALL_BUTTON_LIGHT];
        STXPGROLE * pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_SMALL_BUTTON_MASK];
        if (dialogType == Dialog_ReSuGuan)
        {
            if (dwDialogTempValue != dwSpriteId)
                return PASS;
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 246;
                wY = pstSprite->m_wPy = 224;
                text = "40mm";
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 356;
                wY = pstSprite->m_wPy = 224;
                text = "45mm";
            }
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = 462;
                wY = pstSprite->m_wPy = 224;
                text = "50mm";
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = 246;
                wY = pstSprite->m_wPy = 264;
                text = "55mm";
            }
            else if (dwSpriteId == 4)
            {
                wX = pstSprite->m_wPx = 356;
                wY = pstSprite->m_wPy = 264;
                text = "60mm";
            }
            else if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = 462;
                wY = pstSprite->m_wPy = 264;
                text = getstr(Str_ZiDingYi);
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 70;
            wH = pstSprite->m_wHeight = 25;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth); 
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG_LIST_LIGHT];
            pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG_LIST_MASK];
            
            if (dwSpriteId == 0)
            {
                if (g_psSetupMenu->bDataFormatMMDDYYYY)
                    return PASS;
                wX = pstSprite->m_wPx = 250;
                wY = pstSprite->m_wPy = 208;
                text = "YYYY/MM/DD";
            }
            else if (dwSpriteId == 1)
            {
                if (!g_psSetupMenu->bDataFormatMMDDYYYY)
                    return PASS;
                wX = pstSprite->m_wPx = 250;
                wY = pstSprite->m_wPy = 260;
                text = "MM/DD/YYYY";
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 300;
            wH = pstSprite->m_wHeight = 40;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 6, 0, pstSprite->m_wWidth);
        }
    }
    else if (dwHashKey == xpgHash("FuncSet"))
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
    else if (dwHashKey == xpgHash("FuncSet2"))
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
    else if (dwHashKey == xpgHash("Auto_work"))
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
    else if (dwHashKey == xpgHash("FusionSet3"))
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
    else if (dwHashKey == xpgHash("FusionSet2"))
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
    else if (dwHashKey == xpgHash("FusionSet1"))
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
    else if (dwHashKey == xpgHash("SetTime"))
    {
        if (dwSpriteId == g_psSetupMenu->bLanguage)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            if (dwSpriteId == 0)
                text = getstr(Str_JianTiZhongWen);
            else if (dwSpriteId == 1)
                text = getstr(Str_YingWen);
            else if (dwSpriteId == 2)
                text = getstr(Str_PuTaoYaWen);
            else if (dwSpriteId == 3)
                text = getstr(Str_XiBanYaWen);
            else if (dwSpriteId == 4)
                text = getstr(Str_EWen);
            else if (dwSpriteId == 5)
                text = getstr(Str_FaWen);
            else if (dwSpriteId == 6)
                text = getstr(Str_TaiWen);
            else if (dwSpriteId == 7)
                text = getstr(Str_ALaBoWen);
            else
                return PASS;
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth); 
        }
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
    if (dwHashKey == xpgHash("SetYun"))
    {
        if (!g_psSetupMenu->bCloudMode)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("Auto_work"))
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
    else if (dwHashKey == xpgHash("Manual_work"))
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
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
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
    if (dwHashKey == xpgHash("FuncSet") || 
        dwHashKey == xpgHash("FuncSet2"))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_GongNengPeiZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("SetYun") ||
             dwHashKey == xpgHash("SetSleep") ||
             dwHashKey == xpgHash("SetSound") ||
             dwHashKey == xpgHash("SetTime") ||
             dwHashKey == xpgHash("SetPassword") ||
             dwHashKey == xpgHash("SetUi") ||
             dwHashKey == xpgHash("SetInfo") )
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_XiTongSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionSet1") ||
             dwHashKey == xpgHash("FusionSet2") ||
             dwHashKey == xpgHash("FusionSet3"))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_RongJieSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionModeSet") )
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_XiTongSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("Record") )
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
    else if(dwHashKey == xpgHash("RedLight"))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_HongGuangBi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }

    return PASS;
}

SWORD xpgDrawSprite_Aside(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("FuncSet") || 
        dwHashKey == xpgHash("FuncSet2") ||
        dwHashKey == xpgHash("SetYun") ||
        dwHashKey == xpgHash("SetSleep") ||
        dwHashKey == xpgHash("SetSound") ||
        dwHashKey == xpgHash("SetTime") ||
        dwHashKey == xpgHash("SetPassword") ||
        dwHashKey == xpgHash("SetUi") ||
        dwHashKey == xpgHash("SetInfo") ||
        dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3"))
    {
        Idu_PaintWinArea(pWin, 248, 0, 2, pWin->wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
    }    
    return PASS;
}

SWORD xpgDrawSprite_BackIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("FuncSet") || 
        dwHashKey == xpgHash("FuncSet2") ||
        dwHashKey == xpgHash("SetYun") ||
        dwHashKey == xpgHash("SetSleep") ||
        dwHashKey == xpgHash("SetSound") ||
        dwHashKey == xpgHash("SetTime") ||
        dwHashKey == xpgHash("SetPassword") ||
        dwHashKey == xpgHash("SetUi") ||
        dwHashKey == xpgHash("SetInfo") ||
        dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3") ||
        dwHashKey == xpgHash("Record") ||
        dwHashKey == xpgHash("RedLight"))
    {
        char tmpbuf[64];
        SetCurrIduFontID(FONT_ID_HeiTi19);
        sprintf(tmpbuf, " < %s", getstr(Str_FanHui));
        Idu_PrintString(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        xpgSpriteSetTouchArea(pstSprite, 0, 0, 120, 40);
    }
    else if (dwHashKey == xpgHash("FusionModeSet"))
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
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        STXPGROLE * pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON];
        STXPGROLE * pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON_MASK];
        if (dialogType == Dialog_ReSuGuan)
        {
            pstSprite->m_wPx = 526;
            pstSprite->m_wPy = 152;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_ModifyNumber)
        {
            pstSprite->m_wPx = 526;
            pstSprite->m_wPy = 166;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_ShutdownTime)
        {
            pstSprite->m_wPx = 526;
            pstSprite->m_wPy = 166;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_SetTime || dialogType == Dialog_SetDate)
        {
            pstSprite->m_wPx = 548;
            pstSprite->m_wPy = 84;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            pstSprite->m_wPx = 548;
            pstSprite->m_wPy = 142;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_About || dialogType == Dialog_Times || dialogType == Dialog_TempInfo || dialogType == Dialog_BatInfo)
        {
            pstSprite->m_wPx = 548;
            pstSprite->m_wPy = 138;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword || dialogType == Dialog_EditValue)
        {
            pstSprite->m_wPx = 502;
            pstSprite->m_wPy = 72;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_Value)
        {
            pstSprite->m_wPx = 528;
            pstSprite->m_wPy = 152;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else
            return PASS;
        pstSprite->m_wWidth = 40;
        pstSprite->m_wHeight = 40;
        xpgSpriteEnableTouch(pstSprite);
    }
    else
    {
        xpgDrawSprite(pWin, pstSprite, boClip);
        xpgSpriteEnableTouch(pstSprite);
    }
    return PASS;
}

SWORD xpgDrawSprite_Text(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char * text = "";
    DWORD dwTextId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("Main"))
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
    else if (dwHashKey == xpgHash("FuncSet"))
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
    else if (dwHashKey == xpgHash("FuncSet2"))
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
    else if (dwHashKey == xpgHash("ToolBox"))
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
    else if (dwHashKey == xpgHash("Manual_work"))
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
    else if (dwHashKey == xpgHash("FusionSet3"))
    {
        BYTE tmpbuf[128];
        if (dwTextId == 0)
            sprintf(tmpbuf, "%s >", getstr(Str_ZiDingYi));
        else if (dwTextId == 1)
            sprintf(tmpbuf, "%d%s", g_psSetupMenu->wJiaReWenDu, getstr(Str_DuC));
        else if (dwTextId == 2)
            sprintf(tmpbuf, "%dS", g_psSetupMenu->wJiaReShiJian);
        else
            return PASS;
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
    }
    else if (dwHashKey == xpgHash("FusionSet2"))
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
    else if (dwHashKey == xpgHash("FusionSet1"))
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
    else if (dwHashKey == xpgHash("FusionModeSet"))
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
    else if (dwHashKey == xpgHash("SetTime"))
    {
        char tmpbuf[256];
        if (dwTextId == 1)
        {
            ST_SYSTEM_TIME curTime;
            SystemTimeGet(&curTime);
            if (g_psSetupMenu->b24HourFormat)
            {
                sprintf(tmpbuf, "%02d:%02d >", curTime.u08Hour, curTime.u08Minute);
            }
            else
            {
                BOOL isPm = FALSE;
                BYTE newHour;
                if (curTime.u08Hour >= 12)
                    isPm = TRUE;
                if (curTime.u08Hour == 0 || curTime.u08Hour == 12)
                    newHour = 12;
                else
                    newHour = curTime.u08Hour > 12 ? curTime.u08Hour - 12 : curTime.u08Hour;
                sprintf(tmpbuf, "%02d:%02d %s >", newHour, curTime.u08Minute, isPm ? "PM" : "AM");
            }
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        else if (dwTextId == 2)
        {
            ST_SYSTEM_TIME curTime;
            SystemTimeGet(&curTime);
            if (g_psSetupMenu->bDataFormatMMDDYYYY)
                sprintf(tmpbuf, "%02d/%02d/%04d >", curTime.u08Month, curTime.u08Day, curTime.u16Year);
            else
                sprintf(tmpbuf, "%04d/%02d/%02d >", curTime.u16Year, curTime.u08Month, curTime.u08Day);
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        else if (dwTextId == 3)
        {
            if (g_psSetupMenu->bDataFormatMMDDYYYY)
                sprintf(tmpbuf, "%s >", "MM/DD/YYYY");
            else
                sprintf(tmpbuf, "%s >", "YYYY/MM/DD");
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
    }
    else if (dwHashKey == xpgHash("SetYun"))
    {
        if (dwTextId == 1)
        {
            SetCurrIduFontID(FONT_ID_HeiTi19);
            text = "13800138000";
            Idu_PrintStringRight(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        else if (dwTextId == 2)
        {
            SetCurrIduFontID(FONT_ID_HeiTi19);
            if (g_psSetupMenu->bCloudMode)
                text = getstr(Str_ZaiXian);
            else
                text = "----";
            Idu_PrintStringRight(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        else if (dwTextId == 3)
        {
            SetCurrIduFontID(FONT_ID_HeiTi16);
            text = getstr(Str_SaoMiaoErWeiMa);
            Idu_PrintStringCenter(pWin, text, 288, pstSprite->m_wPy, 0, 470);
        }
    }
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        char tmpbuf[64];
        if (dwTextId == 3)
        {
            sprintf(tmpbuf, "%dmin >", g_psSetupMenu->wShutdownTime);
            if (tmpbuf[0])
            {
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
            }
        }
    }
    else if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        if (dialogType == Dialog_ModifyNumber)
        {
            WORD width, height;
            char tmpbuf[64];
            pstSprite->m_wPx = 350;
            pstSprite->m_wPy = 238;
            width = 100;
            height = 40;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, width, height, 0xffff8080);
            Idu_FontColorSet(0,0,0);
            sprintf(tmpbuf, "%d", dwDialogTempValue);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy + 6, 0, 100);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dialogType == Dialog_ShutdownTime)
        {
            WORD width, height;
            char tmpbuf[64];
            pstSprite->m_wPx = 350;
            pstSprite->m_wPy = 238;
            width = 100;
            height = 40;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, width, height, 0xffff8080);
            Idu_FontColorSet(0,0,0);
            sprintf(tmpbuf, "%dmin", dwDialogTempValue);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy + 6, 0, 100);
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if (dialogType == Dialog_SetTime)
        {
            WORD hour1 = 0, minute1 = 0;
            WORD hour2 = 0, minute2 = 0;
            WORD hour3 = 0, minute3 = 0;
            hour2 = dwDialogTempValue >> 16;
            minute2 = dwDialogTempValue & 0xffff;
            char tmpbuf[64];
            STXPGROLE * pstRole, *pstMask;
            
            if (g_psSetupMenu->b24HourFormat)
            {
                if (hour2 != 0)
                    hour1 = hour2 - 1;
                if (hour2 != 23)
                    hour3 = hour2 + 1;
                if (minute2 != 0)
                    minute1 = minute2 - 1;
                if (minute2 != 59)
                    minute3 = minute2 + 1;

                if (hour2 != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    sprintf(tmpbuf, "%d", hour1);
                    Idu_PrintStringCenter(pWin, tmpbuf, 304, 174, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 328, 154, pWin->wWidth, pWin->wHeight, pstMask);
                }
                SetCurrIduFontID(FONT_ID_HeiTi19);
                sprintf(tmpbuf, "%d", hour2);
                Idu_PrintStringCenter(pWin, tmpbuf, 304, 220, 0, 70);
                if (hour2 != 23)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    sprintf(tmpbuf, "%d", hour3);
                    Idu_PrintStringCenter(pWin, tmpbuf, 304, 270, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 328, 300, pWin->wWidth, pWin->wHeight, pstMask);
                }

                if (minute2 != 0) 
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    sprintf(tmpbuf, "%d", minute1);
                    Idu_PrintStringCenter(pWin, tmpbuf, 426, 174, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 452, 154, pWin->wWidth, pWin->wHeight, pstMask);
                }
                SetCurrIduFontID(FONT_ID_HeiTi19);
                sprintf(tmpbuf, "%d", minute2);
                Idu_PrintStringCenter(pWin, tmpbuf, 426, 220, 0, 70);
                if (minute2 != 59)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    sprintf(tmpbuf, "%d", minute3);
                    Idu_PrintStringCenter(pWin, tmpbuf, 426, 270, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 452, 300, pWin->wWidth, pWin->wHeight, pstMask);
                }

                Idu_PaintWinArea(pWin, 304, 205, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 304, 255, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 426, 205, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 426, 255, 70, 2, RGB2YUV(140,140,140));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintString(pWin, ":", 400, 220, 0, 0);
            }
            else
            {
                BYTE isPm = 0;
                char hstr1[16], hstr2[16], hstr3[16];
                char mstr1[16], mstr2[16], mstr3[16];
                char apmstr1[64], apmstr2[64], apmstr3[64];
                
                hstr1[0] = hstr2[0] = hstr3[0] = 0;
                mstr1[0] = mstr2[0] = mstr3[0] = 0;
                apmstr1[0] = apmstr2[0] = apmstr3[0] = 0;

                sprintf(mstr2, "%d", minute2);
                if (minute2 != 0)
                {
                    minute1 = minute2 - 1;
                    sprintf(mstr1, "%d", minute1);
                }
                if (minute2 != 59)
                {
                    minute3 = minute2 + 1;
                    sprintf(mstr3, "%d", minute3);
                }
                
                if (hour2 >= 12)
                {
                    isPm = 1;
                }

                if (hour2 == 0 || hour2 == 12)
                    sprintf(hstr2, "%d", 12);
                else
                    sprintf(hstr2, "%d", hour2 > 12 ? hour2 - 12 : hour2);
                
                if (hour2 != 0)
                {
                    hour1 = hour2 - 1;
                    if (hour1 == 0 || hour1 == 12)
                        sprintf(hstr1, "%d", 12);
                    else
                        sprintf(hstr1, "%d", hour1 > 12 ? hour1 - 12 : hour1);
                }
                if (hour2 != 23)
                {
                    hour3 = hour2 + 1;
                    if (hour3 == 0 || hour3 == 12)
                        sprintf(hstr3, "%d", 12);
                    else
                        sprintf(hstr3, "%d", hour3 > 12 ? hour3 - 12 : hour3);
                }

                if (isPm)
                {
                    strcpy(apmstr1, "AM");
                    strcpy(apmstr2, "PM");
                }
                else 
                {
                    strcpy(apmstr2, "AM");
                    strcpy(apmstr3, "PM");
                }
                //////////////////////

                if (hstr1[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_PrintStringCenter(pWin, hstr1, 244, 174, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 268, 154, pWin->wWidth, pWin->wHeight, pstMask);
                }
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringCenter(pWin, hstr2, 244, 220, 0, 70);
                if (hstr3[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_PrintStringCenter(pWin, hstr3, 244, 270, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 268, 300, pWin->wWidth, pWin->wHeight, pstMask);
                }

                if (mstr1[0] != 0) 
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_PrintStringCenter(pWin, mstr1, 366, 174, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 392, 154, pWin->wWidth, pWin->wHeight, pstMask);
                }
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringCenter(pWin, mstr2, 366, 220, 0, 70);
                if (mstr3[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_PrintStringCenter(pWin, mstr3, 366, 270, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 392, 300, pWin->wWidth, pWin->wHeight, pstMask);
                }

                if (apmstr1[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_PrintStringCenter(pWin, apmstr1, 488, 174, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 514, 154, pWin->wWidth, pWin->wHeight, pstMask);
                }
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringCenter(pWin, apmstr2, 488, 220, 0, 70);
                if (apmstr3[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_PrintStringCenter(pWin, apmstr3, 488, 270, 0, 70);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 514, 300, pWin->wWidth, pWin->wHeight, pstMask);
                }

                Idu_PaintWinArea(pWin, 244, 205, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 244, 255, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 366, 205, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 366, 255, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 488, 205, 70, 2, RGB2YUV(140,140,140));
                Idu_PaintWinArea(pWin, 488, 255, 70, 2, RGB2YUV(140,140,140));
                
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintString(pWin, ":", 340, 220, 0, 0);
            }
        }
        else if (dialogType == Dialog_SetDate)
        {
            STXPGROLE *pstRole, *pstMask;
            char ystr1[16], ystr2[16], ystr3[16];
            char mstr1[16], mstr2[16], mstr3[16];
            char dstr1[16], dstr2[16], dstr3[16];

            WORD year1, year2, year3;
            WORD month1, month2, month3;
            WORD day1, day2, day3;

            ystr1[0] = ystr2[0] = ystr3[0] = 0;
            mstr1[0] = mstr2[0] = mstr3[0] = 0;
            dstr1[0] = dstr2[0] = dstr3[0] = 0;

            year2 = dwDialogTempValue >> 16;
            month2 = (dwDialogTempValue >> 8) & 0xff;
            day2 = dwDialogTempValue & 0xff;

            sprintf(ystr2, "%04d", year2);
            sprintf(mstr2, "%d", month2);
            sprintf(dstr2, "%d", day2);

            if (year2 > 2000)
            {
                year1 = year2 - 1;
                sprintf(ystr1, "%04d", year1);
            }
            if (year2 < 2050)
            {
                year3 = year2 + 1;
                sprintf(ystr3, "%04d", year3);
            }

            if (month2 > 1)
            {
                month1 = month2 - 1;
                sprintf(mstr1, "%d", month1);
            }
            if (month2 < 12)
            {
                month3 = month2 + 1;
                sprintf(mstr3, "%d", month3);
            }

            if (day2 > 1)
            {
                day1 = day2 - 1;
                sprintf(dstr1, "%d", day1);
            }
            if (day2 < 31)
            {
                day3 = day2 + 1;
                sprintf(dstr3, "%d", day3);
            }
            //////////////////////
            if (ystr1[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, ystr1, 244, 174, 0, 70);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 268, 154, pWin->wWidth, pWin->wHeight, pstMask);
            }
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, ystr2, 244, 220, 0, 70);
            if (ystr3[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, ystr3, 244, 270, 0, 70);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 268, 300, pWin->wWidth, pWin->wHeight, pstMask);
            }
            
            if (mstr1[0] != 0) 
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, mstr1, 366, 174, 0, 70);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 392, 154, pWin->wWidth, pWin->wHeight, pstMask);
            }
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, mstr2, 366, 220, 0, 70);
            if (mstr3[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, mstr3, 366, 270, 0, 70);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 392, 300, pWin->wWidth, pWin->wHeight, pstMask);
            }

            if (dstr1[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, dstr1, 488, 174, 0, 70);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 514, 154, pWin->wWidth, pWin->wHeight, pstMask);
            }
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, dstr2, 488, 220, 0, 70);
            if (dstr3[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, dstr3, 488, 270, 0, 70);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 514, 300, pWin->wWidth, pWin->wHeight, pstMask);
            }
            //////////////////////
            Idu_PaintWinArea(pWin, 244, 205, 70, 2, RGB2YUV(140,140,140));
            Idu_PaintWinArea(pWin, 244, 255, 70, 2, RGB2YUV(140,140,140));
            Idu_PaintWinArea(pWin, 366, 205, 70, 2, RGB2YUV(140,140,140));
            Idu_PaintWinArea(pWin, 366, 255, 70, 2, RGB2YUV(140,140,140));
            Idu_PaintWinArea(pWin, 488, 205, 70, 2, RGB2YUV(140,140,140));
            Idu_PaintWinArea(pWin, 488, 255, 70, 2, RGB2YUV(140,140,140));
                
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, "/", 340, 220, 0, 0);
            Idu_PrintString(pWin, "/", 450, 220, 0, 0);
        }
        else if (dialogType == Dialog_About)
        {
            char text1[128], text2[128];
            if (dwTextId == 0)
            {
                sprintf(text1, "%s:", getstr(Str_XingHao));
                sprintf(text2, "%s", "K5");
                Idu_PrintString(pWin, text1, 228, 208, 0, 0);
                Idu_PrintStringRight(pWin, text2, 572, 208, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s:", getstr(Str_ZhiZaoShang));
                sprintf(text2, "%s", getstr(Str_GongSiMing));
                Idu_PrintString(pWin, text1, 228, 236, 0, 0);
                Idu_PrintStringRight(pWin, text2, 572, 236, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s:", getstr(Str_BanBen));
                sprintf(text2, "%s", "11.0.01");
                Idu_PrintString(pWin, text1, 228, 264, 0, 0);
                Idu_PrintStringRight(pWin, text2, 572, 264, 0);
            }
            else if (dwTextId == 3)
            {
                sprintf(text1, "%s:", getstr(Str_XuLieHao));
                sprintf(text2, "%s", "matengfx2018");
                Idu_PrintString(pWin, text1, 228, 292, 0, 0);
                Idu_PrintStringRight(pWin, text2, 572, 292, 0);
            }
        }
        else if (dialogType == Dialog_Times)
        {
            char text1[128], text2[128];
            if (dwTextId == 0)
            {
                sprintf(text1, "%s:", getstr(Str_XuLieHao));
                sprintf(text2, "%s", "DJ201809080001");
                Idu_PrintString(pWin, text1, 258, 222, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 222, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s:", getstr(Str_JiHuoRiQi));
                sprintf(text2, "%d%s%d%s%d%s", 2018, getstr(Str_Year), 8, getstr(Str_Month), 8, getstr(Str_Day));
                Idu_PrintString(pWin, text1, 258, 250, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 250, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s:", getstr(Str_ShengYuCiShu));
                sprintf(text2, "%d", 2800);
                Idu_PrintString(pWin, text1, 258, 278, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 278, 0);
            }
        }
        else if (dialogType == Dialog_TempInfo)
        {
            char text1[128], text2[128];
            if (dwTextId == 0)
            {
                sprintf(text1, "%s:", getstr(Str_HuanJingWenDu));
                sprintf(text2, "%d%s", 28, getstr(Str_DuC));
                Idu_PrintString(pWin, text1, 258, 208, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 208, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s:", getstr(Str_NeiBuWenDu));
                sprintf(text2, "%d%s", 30, getstr(Str_DuC));
                Idu_PrintString(pWin, text1, 258, 236, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 236, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s:", getstr(Str_HuanJingShiDu));
                sprintf(text2, "%d%%", 60);
                Idu_PrintString(pWin, text1, 258, 264, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 264, 0);
            }
            else if (dwTextId == 3)
            {
                sprintf(text1, "%s:", getstr(Str_QiYa));
                sprintf(text2, "%dkPa", 101);
                Idu_PrintString(pWin, text1, 258, 292, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 292, 0);
            }
        }
        else if (dialogType == Dialog_BatInfo)
        {
            char text1[128], text2[128];
            if (dwTextId == 0)
            {
                sprintf(text1, "%s:", getstr(Str_DianChiRongLiang));
                sprintf(text2, "%d%%", 100);
                Idu_PrintString(pWin, text1, 258, 208, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 208, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s:", getstr(Str_FangDianCiShu));
                sprintf(text2, "%d%s", 15, getstr(Str_Ci));
                Idu_PrintString(pWin, text1, 258, 236, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 236, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s:", getstr(Str_ShiYongShiJian));
                sprintf(text2, "%d%s", 30, getstr(Str_FenZhong));
                Idu_PrintString(pWin, text1, 258, 264, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 264, 0);
            }
            else if (dwTextId == 3)
            {
                sprintf(text1, "%s:", getstr(Str_DaiJiShiJian));
                sprintf(text2, "%d%s%d%s", 3, getstr(Str_XiaoShi), 45, getstr(Str_FenZhong));
                Idu_PrintString(pWin, text1, 258, 292, 0, 0);
                Idu_PrintStringRight(pWin, text2, 542, 292, 0);
            }
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword)
        {
            WORD x = 0, y = 138, w = 40, h = 40;
            char str[2];
            str[1] = 0;
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_FontColorSet(0,0,0);
            if (dwTextId == 0)
            {
                x = 298;
                //mpDebugPrint("cur-input password = %s", strEditPassword);
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                str[0] = strEditPassword[0];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+4, 0, w);
                x = 352;
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                str[0] = strEditPassword[1];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+4, 0, w);
                x = 406;
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                str[0] = strEditPassword[2];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+4, 0, w);
                x = 460;
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                str[0] = strEditPassword[3];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+4, 0, w);
            }
            Idu_FontColorSet(0xff,0xff,0xff);
        }
        else if(dialogType == Dialog_Value)
        {
            WORD x = 260, y = 210, w = 278, h = 42;
            char tmpbuf[128];
            if (dwTextId == 0)
            {
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                Idu_FontColorSet(0,0,0);
                if (boDialogValueIsFloat)
                    sprintf(tmpbuf, "%d.%d", dwDialogTempValue>>6, dwDialogTempValue&0x3F);
                else
                    sprintf(tmpbuf, "%d", dwDialogTempValue);
                Idu_PrintString(pWin, tmpbuf, x + 6, y + 4, 0, 0);
                Idu_FontColorSet(0xff,0xff,0xff);
                xpgSpriteSetTouchArea(pstSprite, x, y, w, h);
            }
        }
        else if (dialogType == Dialog_EditValue)
        {
            WORD x = 298, y = 138, w = 202, h = 40;
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_FontColorSet(0,0,0);
            if (dwTextId == 0)
            {
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                if (strEditValue[0])
                    Idu_PrintString(pWin, strEditValue, x+4, y+4, 0, 0);
            }
            Idu_FontColorSet(0xff,0xff,0xff);
        }
    }
    
    return PASS;
}


SWORD xpgDrawSprite_Selector(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char * text = "";
    DWORD dwSelectorId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("FuncSet"))
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
    else if (dwHashKey == xpgHash("FuncSet2"))
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
    else if (dwHashKey == xpgHash("SetYun") ||
            dwHashKey == xpgHash("SetSleep") ||
            dwHashKey == xpgHash("SetSound") ||
            dwHashKey == xpgHash("SetTime") ||
            dwHashKey == xpgHash("SetPassword") ||
            dwHashKey == xpgHash("SetUi") ||
            dwHashKey == xpgHash("SetInfo"))
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

        if (dwHashKey == xpgHash("SetYun"))
            curPageType = 0;
        else if (dwHashKey == xpgHash("SetSleep"))
            curPageType = 1;
        else if (dwHashKey == xpgHash("SetSound"))
            curPageType = 2;
        else if (dwHashKey == xpgHash("SetTime"))
            curPageType = 3;
        else if (dwHashKey == xpgHash("SetPassword"))
            curPageType = 4;
        else if (dwHashKey == xpgHash("SetUi"))
            curPageType = 5;
        else //if (dwHashKey == xpgHash("SetInfo"))
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
    else if (dwHashKey == xpgHash("FusionSet1") ||
             dwHashKey == xpgHash("FusionSet2") ||
             dwHashKey == xpgHash("FusionSet3") )
    {
        DWORD curPageType;
        if(dwSelectorId == 0)
            text = getstr(Str_RongJieMoShi);
        else if (dwSelectorId == 1)
            text = getstr(Str_RongJieSheZhi);
        else //if (dwSelectorId == 2)
            text = getstr(Str_JiaReSheZhi);
        
        if (dwHashKey == xpgHash("FusionSet1"))
            curPageType = 0;
        else if (dwHashKey == xpgHash("FusionSet2"))
            curPageType = 1;
        else //if (dwHashKey == xpgHash("FusionSet3"))
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
    
    if (dwHashKey == xpgHash("SetYun"))
    {
        if (dwListId == 0)
            text = getstr(Str_YunDuanMoShi);
        else if (dwListId == 1)
            text = getstr(Str_MAD);
        else if (dwListId == 2)
            text = getstr(Str_YunDuanOPMZhuangTai);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetSleep"))
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
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));

        if (dwListId == 3)
        {
            //Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy - 20, 470, 56, RGB2YUV(0xff, 0xff, 0x37));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 20, 470, 56);
        }
        
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        if (dwListId == 0)
            text = getstr(Str_ChuPingShengYin);
        else if (dwListId == 1)
            text = getstr(Str_YinLiangTiaoJie);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetTime"))
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
        if (dwListId < 4)
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));

        if (dwListId >= 1 && dwListId <= 3)
        {
            //Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy - 20, 470, 56, RGB2YUV(0xff, 0xff, 0x37));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 20, 470, 56);
        }
    }
    else if (dwHashKey == xpgHash("SetPassword"))
    {
        WORD lineWidth = 470; 
        Idu_FontColorSet(0xff,0xff,0xff);
        if (dwListId == 0)
            text = getstr(Str_KaiJiMiMa);
        else if (dwListId == 1)
        {
            if (g_psSetupMenu->bEnableOpenPassword)
                text = getstr(Str_GuanBiKaiJiMiMa);
            else
                text = getstr(Str_SheZhiKaiJiMiMa);
            lineWidth = 210;
        }
        else if (dwListId == 2)
            text = getstr(Str_ZuJieMiMa);
        else if (dwListId == 3)
        {
            if (g_psSetupMenu->bEnableHirePassword)
                text = getstr(Str_GuanBiZuJieMiMa);
            else
                text = getstr(Str_SheZhiZuJieMiMa);
            lineWidth = 210;
        }
        else if (dwListId == 4)
        {
            text = getstr(Str_ZuJieRiQi);
            if (g_psSetupMenu->bEnableHirePassword)
                Idu_FontColorSet(130,130,130);
        }
        else if (dwListId == 5)
        {
            text = getstr(Str_SuoDingRongJieCiShu);
            if (g_psSetupMenu->bEnableHirePassword)
                Idu_FontColorSet(130,130,130);
        }
        else if (dwListId == 6)
        {
            if (g_psSetupMenu->bEnableOpenPassword == 0)
                return PASS;
            lineWidth = 210;
            text = getstr(Str_GengGaiKaiJiMiMa);
        }
        else if (dwListId == 7)
        {
            if (g_psSetupMenu->bEnableHirePassword == 0)
                return PASS;
            lineWidth = 210;
            text = getstr(Str_GengGaiZuJieMiMa);
        }

        //Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy - 14, lineWidth, 50, RGB2YUV(0x4F, 0x4F, 0x00));
        xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 14, lineWidth, 50);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, lineWidth, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
        Idu_FontColorSet(0xff,0xff,0xff);

    }
    else if (dwHashKey == xpgHash("SetUi"))
    {
        if (dwListId == 0)
            text = getstr(Str_ZhuJieMianFengGe);
        else if (dwListId == 1)
            text = getstr(Str_YangShiYanSe);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 400, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetInfo"))
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

        //Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy - 14, 470, 50, RGB2YUV(0x4F, 0x4F, 0x00));
        xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 14, 470, 50);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));

        char text1[128];
        if (dwListId == 0)
        {
            sprintf(text1, "%s:%s", getstr(Str_XingHao), "K5");
        }
        else if (dwListId == 1)
        {
            sprintf(text1, "%s:%d", getstr(Str_ShengYuCiShu), 2800);
        }
        else if (dwListId == 2)
        {
            sprintf(text1, "%s:%d%s", getstr(Str_NeiBuWenDu), 30, getstr(Str_DuC));
        }
        else if (dwListId == 3)
        {
            sprintf(text1, "%s:%d%%", getstr(Str_DianChiRongLiang), 100);
        }
        else if (dwListId == 4)
        {
            sprintf(text1, "%d%s", 200, getstr(Str_Ci));
        }
        else if (dwListId == 5)
        {
            sprintf(text1, "110.00.01");
        }
        else if (dwListId == 6)
        {
            sprintf(text1, "MT.000.01");
        }
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringRight(pWin, text1, pstSprite->m_wPx + 470, pstSprite->m_wPy, 0);
    }
    else if (dwHashKey == xpgHash("User"))
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
    else if (dwHashKey == xpgHash("Record"))
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
    else if (dwHashKey == xpgHash("FusionSet3"))
    {
        if (dwListId == 0)
            text = getstr(Str_YuReMoShi);
        else if (dwListId == 1)
            text = getstr(Str_JiaReFangShi);
        else if (dwListId == 2)
        {
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 18, 472, 60);
            text = getstr(Str_ReSuGuanSheZhi);
        }
        else if (dwListId == 3)
        {
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 18, 472, 60);
            text = getstr(Str_JiaReWenDu);
        }
        else if (dwListId == 4)
        {
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 18, 472, 60);
            text = getstr(Str_JiaReShiJian);
        }
        
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
    char *text = "";
    STXPGROLE  stRole, stMaskRole;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    xpgRoleInit(&stRole);
    xpgRoleInit(&stMaskRole);
    
    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogW, dialogH, dailogX, dialogY;
        int dialogId = xpgGetCurrDialogTypeId();
        if (dialogId == Dialog_ReSuGuan)
        {
            dialogW = 350;
            dialogH = 182;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, getstr(Str_ReSuGuanSheZhi), dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_ModifyNumber)
        {
            dialogW = 350;
            dialogH = 150;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, getstr(Str_ReSuGuanSheZhi), dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_ShutdownTime)
        {
            dialogW = 350;
            dialogH = 150;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, getstr(Str_GuanJiShiJian), dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_SetTime || dialogId == Dialog_SetDate)
        {
            dialogW = 390;
            dialogH = 320;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            if (dialogId == Dialog_SetTime)
                text = getstr(Str_ShiJianSheZhi);
            else
                text = getstr(Str_RiQiSheZhi);
            Idu_PrintStringCenter(pWin, text, dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_SetDateFormat)
        {
            dialogW = 400;
            dialogH = 200;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, getstr(Str_RiQiGeShi), dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_About || dialogId == Dialog_Times || dialogId == Dialog_TempInfo || dialogId == Dialog_BatInfo)
        {
            dialogW = 400;
            dialogH = 210;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            if (dialogId == Dialog_About)
                text = getstr(Str_GuanYuBenJi);
            else if (dialogId == Dialog_Times)
                text = getstr(Str_DianJiBangXinXi);
            else if (dialogId == Dialog_TempInfo)
                text = getstr(Str_WenDuXinXi);
            else if (dialogId == Dialog_BatInfo)
                text = getstr(Str_DianChiXinXi);
            Idu_PrintStringCenter(pWin, text, dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_SetPassword1 || dialogId == Dialog_SetPassword2 || dialogId == Dialog_CheckPassword || dialogId == Dialog_EditValue)
        {
            dialogW = 300;
            dialogH = 340;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogId == Dialog_Value)
        {
            dialogW = 350;
            dialogH = 180;
            dailogX = (pWin->wWidth - dialogW) / 2;
            dialogY = (pWin->wHeight - dialogH) / 2;
            MakeDialogRole(&stRole, dialogW, dialogH);
            MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH);
            xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 5, 0, dialogW);
        }
        
    }
    else if (dwHashKey == xpgHash("User"))
    {
        MakeDialogRole(&stRole, 400, 440);
        MakeMaskRole(&stMaskRole, XPG_ROLE_ICON_MASK_0, 400, 440);
        xpgRoleDrawMask(&stRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, getstr(Str_YongHuWeiHu), pstSprite->m_wPx, pstSprite->m_wPy + 5, 0, pstSprite->m_wWidth);
    }
    else if (dwHashKey == xpgHash("ToolBox"))
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
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;
    
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("SetYun"))
    {
        if (g_psSetupMenu->bCloudMode)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        if (dwSpriteId == 0)
        {
            if (g_psSetupMenu->bSmartBacklight)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
            xpgSpriteEnableTouch(pstSprite);
        }
        else if (dwSpriteId == 1)
        {
            if (g_psSetupMenu->bAutoShutdown)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
            xpgSpriteEnableTouch(pstSprite);
        }
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        if (dwSpriteId == 0)
        {
            if (g_psSetupMenu->bToundSoundEnable)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
            xpgSpriteEnableTouch(pstSprite);
        }
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        if (dwSpriteId == 0)
        {
            if (g_psSetupMenu->b24HourFormat)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
            xpgSpriteEnableTouch(pstSprite);
        }
    }
    
    return PASS;
}

SWORD xpgDrawSprite_Scroll(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)          // type26
{
    STXPGSPRITE *pstMask;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;

    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("SetSleep"))
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
    else if (dwHashKey == xpgHash("SetSound"))
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
    if (dwHashKey == xpgHash("Manual_work"))
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
    if (dwHashKey == xpgHash("Auto_work"))
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





