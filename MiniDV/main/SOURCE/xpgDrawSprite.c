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
#include "xpgProcSensorData.h"

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
DIALOGVALUE dwDialogValue;
BOOL  boDialogValueIsFloat = 0;
char strEditPassword[8] = {0};                  // editing password
char strEditValue[32] = {0};                  // editing value
char * strDialogTitle = NULL;
DWORD * pdwEditingFusionValue = NULL;
BOOL isSelectOnlineOPM = 0;

DWORD opmLocalDataCurrentPage = 0;
DWORD opmCloudDataCurrentPage = 0;

DWORD opmLocalDataTotal = 8;
DWORD opmCloudDataTotal = 3;

OPMDATAITEM  localOpmDataArray[] = {
    {"LocalData #001", 1997, 06, 24, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #002", 1997, 06, 25, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #003", 1997, 06, 26, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #004", 1997, 06, 27, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #005", 1997, 06, 28, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #006", 1997, 06, 29, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #007", 1997, 06, 30, 11, 12, 14, 1310, -70, 0 },
    {"LocalData #008", 1997, 07, 01, 11, 12, 14, 1310, -70, 0 },
};
OPMDATAITEM  cloudOpmDataArray[] = {
    {"CloudData #001", 1997, 06, 24, 11, 12, 14, 1310, -70, 0 },
    {"CloudData #002", 1997, 06, 25, 11, 12, 14, 1310, -70, 0 },
    {"CloudData #003", 1997, 06, 26, 11, 12, 14, 1310, -70, 0 },
};
OPMDATAITEM * localOpmData = localOpmDataArray;
OPMDATAITEM * cloudOpmData = cloudOpmDataArray;

////////////////////////////////////////////////////////////////////////

char keyboardBuffer[KEYBOARD_BUFFER_SIZE];
const char * keyboardTitle = "";
void (*keyboardGoBack)(BOOL boEnter) = NULL;
BOOL boKeyLight = FALSE;
DWORD dwKeyID = 0;
BOOL boCapsLock = FALSE;
WORD g_wElectrodeRandomCode,g_wElectrodeSN;

////////////////////////////////////////////////////////////////////////
static WORD curDialogWidth = 800;
static WORD curDialogHeight = 480;
static WORD curDialogLeft = 0;
static WORD curDialogTop = 0;

////////////////////////////////////////////////////////////////////////

const BYTE * FModeStrList[] = {
    "SM",
    "MM",
    "DS",
    "NZ",
    "BIF",
    "CZ1",
    "CZ2",
    "AUTO",
    "SET1",
    "SET2",
    "SET3",
    "SET4",
    NULL
};

BYTE xpgStringBuffer[64];
extern DWORD g_dwCurIndex,g_dwModeIconStatus;
#if  (PRODUCT_UI==UI_SURFACE)
extern DWORD g_dwPassNum,g_dwFailNum;
#endif

static DWORD asideColor;
//---------------------------------------------------------------------------
// Dialog
//---------------------------------------------------------------------------
int popupDialog(int dialogType, WORD wReturnPageIndex, ST_IMGWIN* pWin_Background)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    
#if  (PRODUCT_UI==UI_WELDING)
    xpgAddDialog(dialogType, wReturnPageIndex,pWin_Background);
    mpDebugPrint("dialogType = %d, wReturnPageIndex = %d", dialogType, wReturnPageIndex);
    
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
    }
    else if (dialogType == Dialog_Hot_Mode)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
    }
    else if (dialogType == Dialog_ModifyNumber)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
    }
    else if (dialogType == Dialog_SetBrightness)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
    }
    else if (dialogType == Dialog_ShutdownTime||dialogType == Dialog_SleepTime)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        //xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
    }
    else if (dialogType == Dialog_SetSound)
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
        //xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
    }
    else if (dialogType == Dialog_SetDate)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        //xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_REPEATICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
    }
    else if (dialogType == Dialog_SetDateFormat)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        //xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 1, 0);
    }
    else if (dialogType == Dialog_SetLang)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        //xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 1, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 3, 0);
        /*
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 7, 0);
        */
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 1, 0);
        /*
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 2, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 3, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 4, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 5, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 6, 0);
        xpgAddDialogSprite(SPRITE_TYPE_LIGHT_ICON, 7, 0);
        */
    }
    else if (dialogType == Dialog_YESNO_Upgrade||dialogType == Dialog_Note_ClearAll)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TextColorBar, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TextColorBar, 1, 0);
    }
    else if (dialogType == Dialog_Record_Detail)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 1, 0);
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
    else if(dialogType == Dialog_MainPageError||dialogType == Dialog_MachineWarning)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
    }
    else if (dialogType == Dialog_PowerOnCheckHirePassword||dialogType == Dialog_PowerOnCheckOpenPassword|| dialogType == Dialog_PowerOnCheckOpenPassword\
			|| dialogType == Dialog_Electrode_SN_Input|| dialogType == Dialog_Electrode_CheckCode_Input)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        //xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
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
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 10, 0);//enter
        xpgAddDialogSprite(SPRITE_TYPE_ICON, 11, 0);//Backspace
    }
    else if (dialogType == Dialog_Note_ForgetHirePassword||dialogType == Dialog_Note_ForgetOpenPassword||dialogType == Dialog_Note_ElectrodeEnable_Path\
			||dialogType == Dialog_Note_ElectrodeEnable_PASS||dialogType == Dialog_Note_ElectrodeEnable_FAIL||dialogType == Dialog_ShutdownRemain\
			||dialogType == Dialog_Note_Electrode_Enable_Process)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_CLOSE_ICON, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
    }
    else if (dialogType == Dialog_Note_Reinput||dialogType == Dialog_Note_PasswordError||dialogType == Dialog_Note_ChangeBootWordPASS||dialogType == Dialog_Note_ChangeHireWordPASS)
    {
        xpgAddDialogSprite(SPRITE_TYPE_DIALOG, 0, 0);
        xpgAddDialogSprite(SPRITE_TYPE_TEXT, 0, 0);
    }
    
    xpgSearchtoPageWithAction(DIALOG_PAGE_NAME);
#endif
}

void exitDialog()
{
    DWORD dwPageIndex = xpgGetCurrDialogBackPage();

    xpgDeleteDialog();
    if (dwPageIndex != 0)
        xpgGotoPageWithAction(dwPageIndex);
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
#if  (PRODUCT_UI==UI_WELDING)
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
	xpgDrawSprite_Null,		        //type28
	xpgDrawSprite_RepeatIcon,		        //type29
	xpgDrawSprite_HomeStatus,           //type30
	xpgDrawSprite_TextColorBar,           //type31
#endif
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

#if  (PRODUCT_UI==UI_WELDING)
SWORD xpgDrawSprite_Background(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("User") || 
        dwHashKey == xpgHash("ToolBox") )
    {
        mpCopyEqualWin(pWin, Idu_GetCacheWin());
    }
    else if (dwHashKey == xpgHash("opm1") || dwHashKey == xpgHash("opm2") || dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2"))
    {
        Idu_PaintWin(pWin, RGB2YUV(32, 33, 38));
    }
    else if (dwHashKey == xpgHash("opmWarn"))
    {
        ST_IMGWIN * pCacheWin = Idu_GetCacheWin();
        if (pCacheWin != NULL && pCacheWin->pdwStart != NULL)
            mpCopyEqualWin(pWin, pCacheWin);
        xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("Keyboard"))
    {
        ST_IMGWIN * pCacheWin = Idu_GetCacheWin();
        if (pCacheWin != NULL && pCacheWin->pdwStart != NULL)
            mpCopyEqualWin(pWin, pCacheWin);
        Idu_PaintWinArea(pWin, 0, 190, 800, 480-190, IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("FuncSet") || 
        dwHashKey == xpgHash("FuncSet2") ||
        dwHashKey == xpgHash("SetYun") ||
        dwHashKey == xpgHash("SetSleep") ||
        dwHashKey == xpgHash("SetSound") ||
        dwHashKey == xpgHash("SetTime") ||
        dwHashKey == xpgHash("SetPassword") ||
        dwHashKey == xpgHash("SetUi") ||
        dwHashKey == xpgHash("Upgrade") ||
        dwHashKey == xpgHash("SetInfo") ||
        dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3"))
    {
        Idu_PaintWin(pWin, IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }    
    //else if (dwHashKey == xpgHash("opm1") || 
    //    dwHashKey == xpgHash("opm2") ||
    //    dwHashKey == xpgHash("opm3") ||
    //    dwHashKey == xpgHash("opm4"))
    //{
    //    Idu_PaintWin(pWin, RGB2YUV(0xff, 0xff, 0xff));
    //}
    else if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        ST_IMGWIN * pCacheWin = xpgGetCurrDialogCacheWin();
        if (pCacheWin != NULL && pCacheWin->pdwStart != NULL)
            mpCopyEqualWin(pWin, pCacheWin);
    }
    else if (dwHashKey == xpgHash("Logo"))
    {
		extern BYTE g_bLogoMix;
		
		mmcp_memset_u32((BYTE *)pWin->pdwStart,0x00008080, pWin->dwOffset*pWin->wHeight);
		xpgRoleMixOnWin(pWin,pstSprite->m_pstRole, pstSprite->m_wPx, pstSprite->m_wPy,g_bLogoMix);
		//xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, pstSprite->m_wPx, pstSprite->m_wPy, NULL, 0);
    }
    else if (dwHashKey == xpgHash("Charge"))
    {
		if ( pstSprite->m_dwTypeIndex==100)
		{
			mmcp_memset_u32((BYTE *)pWin->pdwStart,0x00008080, pWin->dwOffset*pWin->wHeight);
			 xpgDrawSprite(pWin, pstSprite, boClip);
		}
		else if (g_psUnsaveParam->bChargeStatus)
		{
			if (pstSprite->m_dwTypeIndex<=g_psUnsaveParam->bBatteryQuantity/20)
				mpPaintWinArea(pWin,  pstSprite->m_wPx, pstSprite->m_wPy,  ALIGN_CUT_2(pstSprite->m_wWidth), pstSprite->m_wHeight, RGB2YUV(0xff,0xd8,0));
		}
		else
		{
			if (g_psUnsaveParam->bBatteryQuantity<20 && pstSprite->m_dwTypeIndex==0)
				mpPaintWinArea(pWin,  pstSprite->m_wPx, pstSprite->m_wPy,  ALIGN_CUT_2(pstSprite->m_wWidth), pstSprite->m_wHeight, RGB2YUV(0xff,0,0));
			else if (pstSprite->m_dwTypeIndex<=g_psUnsaveParam->bBatteryQuantity/20)
				mpPaintWinArea(pWin,  pstSprite->m_wPx, pstSprite->m_wPy,  ALIGN_CUT_2(pstSprite->m_wWidth), pstSprite->m_wHeight, RGB2YUV(0,0xff,0x29));
		}
    }
    else
        xpgDrawSprite(pWin, pstSprite, boClip);
    return PASS;
}

SWORD xpgDrawSprite_Icon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE * pstMask, stMaskRole;
	  STXPGROLE * pstRole = NULL,* pstRoleMask = NULL;
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
        if (dialogType == Dialog_ReSuGuan)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_SMALL_BUTTON_ICON];
            pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_SMALL_BUTTON_MASK];
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
            else //if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = 462;
                wY = pstSprite->m_wPy = 264;
                text = getstr(Str_ZiDingYi);
            }
            wW = pstSprite->m_wWidth = 70;
            wH = pstSprite->m_wHeight = 25;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth); 
        }
		 else if (dialogType == Dialog_Hot_Mode)
        {
			DWORD	dwYUVcolor=0;
			
			wX = pstSprite->m_wPx = 224;
			text = getstr(Str_JianTiZhongWen);
			wW = pstSprite->m_wWidth = 350;
			wH = pstSprite->m_wHeight = 50;
			SetCurrIduFontID(FONT_ID_HeiTi19);
            if (dwSpriteId == 0)
            {
					wY = pstSprite->m_wPy = 180;
                text = getstr(Str_ZiDong);
            }
            else if (dwSpriteId == 1)
            {
                wY = pstSprite->m_wPy = 262;
                text = getstr(Str_ShouDong);
            }
			if (dwDialogValue.dwValueData==pstSprite->m_dwTypeIndex)
				DrawRoundIcon(pWin,XPG_ROLE_MASK_70x25,wX, wY,wW, wH,RGB2YUV(0x37,0x9d,0xff));
			else
				DrawRoundIcon(pWin,XPG_ROLE_MASK_70x25,wX, wY,wW, wH,0x00008080);
           Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
        }
        else if (dialogType == Dialog_SetBrightness)
        {
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 180;
                wY = pstSprite->m_wPy = 210;
                wW = pstSprite->m_wWidth = 100;
                wW = pstSprite->m_wHeight = 100;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 520;
                wY = pstSprite->m_wPy = 210;
                wW = pstSprite->m_wWidth = 100;
                wW = pstSprite->m_wHeight = 100;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_MINUS], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else 
                return PASS;
        }
        else if (dialogType == Dialog_SetSound)
        {
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 180;
                wY = pstSprite->m_wPy = 210;
                wW = pstSprite->m_wWidth = 100;
                wW = pstSprite->m_wHeight = 100;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 520;
                wY = pstSprite->m_wPy = 210;
                wW = pstSprite->m_wWidth = 100;
                wW = pstSprite->m_wHeight = 100;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_MINUS], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else 
                return PASS;
        }
        else if (dialogType == Dialog_ModifyNumber||dialogType == Dialog_ShutdownTime||dialogType == Dialog_SleepTime)
        {
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 180;
                wY = pstSprite->m_wPy = 180;
                wW = pstSprite->m_wWidth = 100;
                wW = pstSprite->m_wHeight = 100;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ADD], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 520;
                wY = pstSprite->m_wPy = 180;
                wW = pstSprite->m_wWidth = 100;
                wW = pstSprite->m_wHeight = 100;
                xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_MINUS], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
            }
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = 120;
                wY = pstSprite->m_wPy = 330;
                wW = pstSprite->m_wWidth = 280;
                wW = pstSprite->m_wHeight = 70;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = 400;
                wY = pstSprite->m_wPy = 330;
                wW = pstSprite->m_wWidth = 280;
                wW = pstSprite->m_wHeight = 70;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
            }
            else 
                return PASS;
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
                    wX = pstSprite->m_wPx = 120;
                    wY = pstSprite->m_wPy = 378;
                    pstSprite->m_wWidth = 280;
                    pstSprite->m_wHeight = 70;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON];
                    pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                    //SetCurrIduFontID(FONT_ID_HeiTi19);
                    //Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 7)
                {
                    wX = pstSprite->m_wPx = 400;
                    wY = pstSprite->m_wPy = 378;
                    pstSprite->m_wWidth = 280;
                    pstSprite->m_wHeight = 70;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON];
                    pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                    //SetCurrIduFontID(FONT_ID_HeiTi19);
                    //Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
            }
            else    // 12 hour
            {
                if (dwSpriteId == 6)
                {
                    wX = pstSprite->m_wPx = 120;
                    wY = pstSprite->m_wPy = 378;
                    pstSprite->m_wWidth = 280;
                    pstSprite->m_wHeight = 70;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON];
                    pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                    //SetCurrIduFontID(FONT_ID_HeiTi19);
                    //Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 7)
                {
                    wX = pstSprite->m_wPx = 400;
                    wY = pstSprite->m_wPy = 378;
                    pstSprite->m_wWidth = 280;
                    pstSprite->m_wHeight = 70;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON];
                    pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                    //SetCurrIduFontID(FONT_ID_HeiTi19);
                    //Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
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
            {
                if (dwSpriteId == 6)
                {
                    wX = pstSprite->m_wPx = 120;
                    wY = pstSprite->m_wPy = 378;
                    pstSprite->m_wWidth = 280;
                    pstSprite->m_wHeight = 70;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON];
                    pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                    //SetCurrIduFontID(FONT_ID_HeiTi19);
                    //Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
                }
                else if (dwSpriteId == 7)
                {
                    wX = pstSprite->m_wPx = 400;
                    wY = pstSprite->m_wPy = 378;
                    pstSprite->m_wWidth = 280;
                    pstSprite->m_wHeight = 70;
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON];
                    pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                    //SetCurrIduFontID(FONT_ID_HeiTi19);
                    //Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
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
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_DARK];
            pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_MASK];
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 200;
                text = "YYYY/MM/DD";
	            wW = pstSprite->m_wWidth = 350;
	            wH = pstSprite->m_wHeight = 50;
	            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
	            SetCurrIduFontID(FONT_ID_HeiTi19);
	            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 262;
                text = "MM/DD/YYYY";
	            wW = pstSprite->m_wWidth = 350;
	            wH = pstSprite->m_wHeight = 50;
	            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
	            SetCurrIduFontID(FONT_ID_HeiTi19);
	            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
            }
            else if (dwSpriteId == 6)
            {
                wX = pstSprite->m_wPx = 120;
                wY = pstSprite->m_wPy = 378;
                pstSprite->m_wWidth = 280;
                pstSprite->m_wHeight = 70;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                //SetCurrIduFontID(FONT_ID_HeiTi19);
                //Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
            }
            else if (dwSpriteId == 7)
            {
                wX = pstSprite->m_wPx = 400;
                wY = pstSprite->m_wPy = 378;
                pstSprite->m_wWidth = 280;
                pstSprite->m_wHeight = 70;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                //SetCurrIduFontID(FONT_ID_HeiTi19);
                //Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
            }
        }
        else if (dialogType == Dialog_SetLang)
        {
            if (dwSpriteId == 0)
            {
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_DARK];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_MASK];
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 180;
                text = getstr(Str_JianTiZhongWen);
                wW = pstSprite->m_wWidth = 350;
                wH = pstSprite->m_wHeight = 50;
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
            }
            else if (dwSpriteId == 1)
            {
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_DARK];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_MASK];
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 262;
                text = getstr(Str_YingWen);
                wW = pstSprite->m_wWidth = 350;
                wH = pstSprite->m_wHeight = 50;
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
            }
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = 120;
                wY = pstSprite->m_wPy = 378;
                pstSprite->m_wWidth = 280;
                pstSprite->m_wHeight = 70;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_OK_BUTTON_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                //SetCurrIduFontID(FONT_ID_HeiTi19);
                //Idu_PrintStringCenter(pWin, getstr(Str_QueRen), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = 400;
                wY = pstSprite->m_wPy = 378;
                pstSprite->m_wWidth = 280;
                pstSprite->m_wHeight = 70;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CANCEL_BUTTON_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstRoleMask);
                //SetCurrIduFontID(FONT_ID_HeiTi19);
                //Idu_PrintStringCenter(pWin, getstr(Str_QuXiao), pstSprite->m_wPx, pstSprite->m_wPy + 8, 0, pstSprite->m_wWidth);
            }
            else
                return PASS;
            /*
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = 164;
                wY = pstSprite->m_wPy = 202;
                text = getstr(Str_PuTaoYaWen);
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = 404;
                wY = pstSprite->m_wPy = 202;
                text = getstr(Str_XiBanYaWen);
            }
            else if (dwSpriteId == 4)
            {
                wX = pstSprite->m_wPx = 164;
                wY = pstSprite->m_wPy = 266;
                text = getstr(Str_EWen);
            }
            else if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = 404;
                wY = pstSprite->m_wPy = 266;
                text = getstr(Str_FaWen);
            }
            else if (dwSpriteId == 6)
            {
                wX = pstSprite->m_wPx = 164;
                wY = pstSprite->m_wPy = 331;
                text = getstr(Str_TaiWen);
            }
            else if (dwSpriteId == 7)
            {
                wX = pstSprite->m_wPx = 404;
                wY = pstSprite->m_wPy = 331;
                text = getstr(Str_ALaBoWen);
            }
            */
            
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword \
					|| dialogType == Dialog_EditValue || dialogType == Dialog_PowerOnCheckHirePassword|| dialogType == Dialog_PowerOnCheckOpenPassword\
					|| dialogType == Dialog_Electrode_SN_Input|| dialogType == Dialog_Electrode_CheckCode_Input)
        {
            const WORD x0 = 220, x1 = 340, x2 = 460;
            const WORD y0 = 190, y1 = 255, y2 = 320, y3 = 385;
            pstSprite->m_wWidth = 120;
            pstSprite->m_wHeight = 65;
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
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_LEFT_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
                if (dialogType == Dialog_CheckPassword|| dialogType == Dialog_PowerOnCheckHirePassword|| dialogType == Dialog_PowerOnCheckOpenPassword)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi16);
                    Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                    Idu_PrintStringCenter(pWin, getstr(Str_WangJiMiMa), wX, wY + 18, 0, pstSprite->m_wWidth);
                    Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                }
                else if (dialogType == Dialog_EditValue)
                {
                    BYTE * text1 = getstr(Str_QueDing);
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                    Idu_PrintStringCenter(pWin, text1, wX, wY + 15, 0, pstSprite->m_wWidth);
                    Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                }
				  else if (dialogType == Dialog_Electrode_SN_Input|| dialogType == Dialog_Electrode_CheckCode_Input)
				  {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                    Idu_PrintStringCenter(pWin, "EXIT", wX, wY + 15, 0, pstSprite->m_wWidth);//ENTER
                    Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
				  }
            }
            else if (dwSpriteId == 11)
            {
                wX = pstSprite->m_wPx = x2;
                wY = pstSprite->m_wPy = y3;
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_BACKSPACE];
                pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_KEYBOARD_RIGHT_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
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
            pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_BUTTON_MASK];
            
            wW = pstSprite->m_wWidth = 90;
            wH = pstSprite->m_wHeight = 30;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
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
        pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_MASK_0];
        if ( dwSpriteId < 12 )
        {
			if (dwSpriteId==FUNCTION_ID_LAMP && g_psSetupMenu->bFunctionIconEnable[dwSpriteId]==2)
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LAMP_AUTO];
            else if (g_psSetupMenu->bFunctionIconEnable[dwSpriteId])
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_ON + dwSpriteId];
            else
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_OFF + dwSpriteId];
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
        }
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        STXPGSPRITE * tinyIcon = NULL, * tinyMask = NULL;
		char sbIconId=g_psSetupMenu->bCustomizeIcon[dwSpriteId];
		
		pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_MASK_0];
        tinyMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
        if (dwSpriteId < 6)
        {
            if (sbIconId>= 0)
            {
               // pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_ON + g_psSetupMenu->bCustomizeIcon[dwSpriteId]];
				if (sbIconId==FUNCTION_ID_LAMP && g_psSetupMenu->bFunctionIconEnable[sbIconId]==2)
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LAMP_AUTO];
	            else if (g_psSetupMenu->bFunctionIconEnable[sbIconId])
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_ON + sbIconId];
	            else
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_OFF + sbIconId];
                tinyIcon = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 3);
            }
            else
            {
                pstRole = pstSprite->m_pstRole;
            }
        }
        else if (dwSpriteId >= 6 && dwSpriteId < 18)
        {
            //pstRole = pstSprite->m_pstRole;
            sbIconId=dwSpriteId-6;
				if (sbIconId==FUNCTION_ID_LAMP && g_psSetupMenu->bFunctionIconEnable[sbIconId]==2)
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LAMP_AUTO];
	            else if (g_psSetupMenu->bFunctionIconEnable[sbIconId])
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_ON + sbIconId];
	            else
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_OFF + sbIconId];
            tinyIcon = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 4);
        }
        else
            return PASS;
        
        xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
        if (tinyIcon && tinyMask)
        {
            xpgRoleDrawMask(tinyIcon->m_pstRole, pWin->pdwStart, wX - 4, wY - 4, pWin->wWidth, pWin->wHeight, tinyMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("ToolBox"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
            xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
    }
    else if (dwHashKey == xpgHash("Record") )
    {
	    //if (dwSpriteId >=10 && dwSpriteId<20)
	    {
			STRECORD * pr;
			int iCurIndex;
			BYTE bListMain=dwSpriteId/10-1,ListSub=dwSpriteId%10;

			if (!g_WeldRecordPage.dwTotalData)
				return PASS;
			iCurIndex =  PAGE_RECORD_SIZE * g_WeldRecordPage.wCurPage + bListMain;
			if (iCurIndex>g_WeldRecordPage.dwTotalData-1)
				return PASS;
			pr = GetRecord((DWORD)iCurIndex);
			if (pr == NULL)
				return PASS;

          xpgDrawSprite(pWin, pstSprite, boClip);
			if (ListSub==1)
					xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx-pstSprite->m_wWidth, pstSprite->m_wPy-pstSprite->m_wHeight, pstSprite->m_wWidth*3, pstSprite->m_wHeight*3);
	    }
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
        if ( dwSpriteId<6)
        {
				char sbIconId=g_psSetupMenu->bCustomizeIcon[dwSpriteId];

			if (sbIconId<0)
			{
				pstRole=pstSprite->m_pstRole;
			}
			else
			{
				if (sbIconId==FUNCTION_ID_LAMP && g_psSetupMenu->bFunctionIconEnable[sbIconId]==2)
	                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LAMP_AUTO];
				else if (g_psSetupMenu->bFunctionIconEnable[sbIconId] )
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_ON+sbIconId];
				else
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_YJR_OFF+sbIconId];
			}
			pstRoleMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_ICON_MASK_0];
        	xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstRoleMask);
        }
		else
			xpgDrawSprite(pWin, pstSprite, boClip);
    }
    else if (dwHashKey == xpgHash("FusionSet3"))
    {
		#if 0
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
        else 
		#endif
		if (dwSpriteId == 2 && !g_psSetupMenu->bPreHotEnable)
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
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+8, 0, pstSprite->m_wWidth);   
    }
    else if (dwHashKey == xpgHash("FusionSet1"))
    {
        if (dwSpriteId <12)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            text = (char*) FModeStrList[dwSpriteId];
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
        else if (dwSpriteId == 12)
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            return PASS;
        }
        else if (dwSpriteId >= 11 && dwSpriteId <= 13)
        {
			if (dwSpriteId == 11)
			{
				wX-=10;
				DrawRoundIcon(pWin,XPG_ROLE_MASK_70x25,wX-10, pstSprite->m_wPy,pstSprite->m_wWidth+20, pstSprite->m_wHeight,RGB2YUV(0x37,0x9d,0xff));
			}
			else
			{
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
			}
            if (dwSpriteId == 11)
                text = (char*) getstr(Str_HuiFuMoRenZhi);
            else if (dwSpriteId == 12)
                text = (char*) getstr(Str_BaoCun);
            else if (dwSpriteId == 13)
                text = (char*) getstr(Str_QuXiao);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, wX, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
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
		DWORD dwFileSize;
		STREAM* handle = (STREAM *)Test_OpenFileByNameForRead(SYS_DRV_ID,"qrcode","jpg",&dwFileSize);//DriveCurIdGet()
		if (handle)
		{
			ST_IMGWIN stTmpWin;
			mpWinInit(&stTmpWin ,NULL, pstSprite->m_wHeight, pstSprite->m_wWidth); 
			stTmpWin.pdwStart=pWin->pdwStart+pstSprite->m_wPx/2+pstSprite->m_wPy*pWin->dwOffset/4;
			stTmpWin.dwOffset=pWin->dwOffset;
			ImageDecodeByHandle(handle,&stTmpWin);
			FileClose(handle);
		}
		
        //xpgDrawSprite(pWin, pstSprite, boClip);
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
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 5);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 6)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 6);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 7)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 7);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 8)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 8);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);

            char tempstr[32];
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            sprintf(tempstr, "%dmin", g_psUnsaveParam->bRedPenTime);
            Idu_PrintStringCenter(pWin, tempstr, pstSprite->m_wPx, pstSprite->m_wPy+4, 0, pstSprite->m_wWidth);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
    }
    else if(dwHashKey == xpgHash("opm1") || dwHashKey == xpgHash("opm2") /*|| dwHashKey == xpgHash("opm3") || dwHashKey == xpgHash("opm4")*/)
    {
		if (dwHashKey == xpgHash("opm1"))
		{
			if (g_stLocalOpmPagePara.bCal && dwSpriteId>=10 && dwSpriteId<20)
			{
					xpgSpriteDisableTouch(pstSprite);
					return PASS;
			}
			else if (!g_stLocalOpmPagePara.bCal && dwSpriteId>=30 && dwSpriteId<40)
			{
					xpgSpriteDisableTouch(pstSprite);
					return PASS;
			}
		}
        if (dwSpriteId == 20)
        {
			xpgDrawSprite(pWin, pstSprite, boClip);
            if (dwHashKey == xpgHash("opm1"))
                text = getstr(Str_LocalOPMCtrlPanel);
            else 
                text = getstr(Str_RemoteOPMCtrlPanel);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+pstSprite->m_wHeight-IduFontGetMaxHeight()*2, 0, pstSprite->m_wWidth);  
        }
        else if (dwSpriteId == 0 || dwSpriteId == 1)
        {
            //pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, dwSpriteId);
           // if (pstMask)
           //     xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
			xpgDrawSprite(pWin, pstSprite, boClip);
            if (dwSpriteId == 0)
                text = getstr(Str_LocalOPM);
            else 
                text = getstr(Str_CloudOPM);
			if ((dwHashKey == xpgHash("opm1")&&dwSpriteId == 1)||(dwHashKey == xpgHash("opm2")&&dwSpriteId == 0))
	            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx+10, pstSprite->m_wPy+8, 0, pstSprite->m_wWidth);  
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dwSpriteId == 2)
        {
            //pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            //if (pstMask)
           //     xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
           //mpPaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, LEFT_ASIDE_YUV_COLOR);
			xpgDrawSprite(pWin, pstSprite, boClip);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            if (dwHashKey == xpgHash("opm1"))
                text = getstr(Str_LocalOPMCtrlPanel);
            else if (dwHashKey == xpgHash("opm2"))
                text = getstr(Str_RemoteOPMCtrlPanel);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+4, 0, pstSprite->m_wWidth);  
        }
		#if 0
        else if (dwSpriteId == 3)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 3);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
		#endif
        else if (dwSpriteId == 4)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 4);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 5)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
        else if (dwSpriteId == 6)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
        else if (dwSpriteId == 7)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
        else if (dwSpriteId == 8)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
        else if (dwSpriteId == 9)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 9);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        else if (dwSpriteId == 10)
        {
			MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT7);
			xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
			//if (!g_stLocalOpmPagePara.bPowerOnOff)
			//	DrakSprite(pWin,pstSprite);
        }
        else if (dwSpriteId == 11)
        {
				MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT5);
				xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
        }
        else if (dwSpriteId == 12)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
        else if (dwSpriteId == 13)
        {
			MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT6);
			xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
        }
        else if (dwSpriteId == 14)
        {
			MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT4);
			xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
        }
        else if (dwSpriteId == 15)
        {
            if (dwHashKey == xpgHash("opm2"))
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 15);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
        }
		 else if (g_stLocalOpmPagePara.bCal && dwSpriteId>=30 && dwSpriteId<40)//(dwHashKey == xpgHash("opm2"))
		 {
	 			if (dwSpriteId==30)
				{
					MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT7);
					xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
				}
	 			else if (dwSpriteId==32)
				{
					MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT6);
					xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
				}
				else
					xpgDrawSprite(pWin, pstSprite, boClip);
		 }
    }
    else if(dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2") )
    {
        if (dwSpriteId == 0 || dwSpriteId == 1)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, dwSpriteId);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            if (dwSpriteId == 0)
                text = getstr(Str_LocalOPM);
            else 
                text = getstr(Str_CloudOPM);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dwSpriteId == 2)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            text = getstr(Str_QingKong);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
        else if (dwSpriteId >= 3 && dwSpriteId <= 7)
        {
            /////
            DWORD dwListId = dwSpriteId - 3;
            DWORD * pdwPageId;
            DWORD * pdwTotal;
            OPMDATAITEM * pstItems;
            OPMDATAITEM * curItem;
            if (dwHashKey == xpgHash("opmList1"))
            {
                pdwPageId = &opmLocalDataCurrentPage;
                pdwTotal = &opmLocalDataTotal;
                pstItems = localOpmData;
            }
            else
            {
                pdwPageId = &opmCloudDataCurrentPage;
                pdwTotal = &opmCloudDataTotal;
                pstItems = cloudOpmData;
            }
            if ((*pdwPageId) * 5 + dwListId >= *pdwTotal)
                return PASS;
        
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 3);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx - 14, pstSprite->m_wPy - 14, pstSprite->m_wWidth + 28, pstSprite->m_wHeight + 28);
            return PASS;
            
        }
        else if (dwSpriteId == 8)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 8);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            text = getstr(Str_ShuaXin);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
        
    }
    else if (dwHashKey == xpgHash("opmWarn"))
    {
        if (dwSpriteId == 0)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
    }
    else if (dwHashKey == xpgHash("Keyboard"))
    {
        if (dwSpriteId >= 10 && dwSpriteId <= 28 || dwSpriteId >= 30 && dwSpriteId <= 36)
        {
            if (boCapsLock)
            {
                DWORD capsId;
                if (dwSpriteId >= 10 && dwSpriteId <= 28)
                    capsId = dwSpriteId - 10;
                else
                    capsId = dwSpriteId - 30 + 19;
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_DARK_ICON, capsId);
                xpgDirectDrawRoleOnWin(pWin, pstMask->m_pstRole, pstSprite->m_wPx, pstSprite->m_wPy, pstMask, boClip);
            }
            else
            {
                xpgDrawSprite(pWin, pstSprite, boClip);
            }
        }
        else
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
    }
    
    xpgSpriteEnableTouch(pstSprite);
}

SWORD xpgDrawSprite_RepeatIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE * pstMask;
	  STXPGROLE * pstRole = NULL,* pstRoleMask = NULL;
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
        if (dialogType == Dialog_SetTime)
        {
            if (g_psSetupMenu->b24HourFormat)
            {
                if (dwSpriteId == 0)
                {
                    xpgSpriteSetTouchArea(pstSprite, 269, 81, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 1)
                {
                    xpgSpriteSetTouchArea(pstSprite, 269, 242, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 2)
                {
                    xpgSpriteSetTouchArea(pstSprite, 431, 81, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 3)
                {
                    xpgSpriteSetTouchArea(pstSprite, 431, 242, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 4 || dwSpriteId == 5)
                {
                    return PASS;
                }
            }
            else    // 12 hour
            {
                if (dwSpriteId == 0)
                {
                    xpgSpriteSetTouchArea(pstSprite, 189, 80, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 1)
                {
                    xpgSpriteSetTouchArea(pstSprite, 189, 281, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 2)
                {
                    xpgSpriteSetTouchArea(pstSprite, 351, 80, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 3)
                {
                    xpgSpriteSetTouchArea(pstSprite, 351, 281, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 4)
                {
                    xpgSpriteSetTouchArea(pstSprite, 511, 80, 100, 100);
                    return PASS;
                }
                else if (dwSpriteId == 5)
                {
                    xpgSpriteSetTouchArea(pstSprite, 511, 281, 100, 100);
                    return PASS;
                }
            }
        }
        else if (dialogType == Dialog_SetDate)
        {
            if (dwSpriteId == 0)
            {
                xpgSpriteSetTouchArea(pstSprite, 189, 80, 100, 100);
                return PASS;
            }
            else if (dwSpriteId == 1)
            {
                xpgSpriteSetTouchArea(pstSprite, 189, 281, 100, 100);
                return PASS;
            }
            else if (dwSpriteId == 2)
            {
                xpgSpriteSetTouchArea(pstSprite, 351, 80, 100, 100);
                return PASS;
            }
            else if (dwSpriteId == 3)
            {
                xpgSpriteSetTouchArea(pstSprite, 351, 281, 100, 100);
                return PASS;
            }
            else if (dwSpriteId == 4)
            {
                xpgSpriteSetTouchArea(pstSprite, 511, 80, 100, 100);
                return PASS;
            }
            else if (dwSpriteId == 5)
            {
                xpgSpriteSetTouchArea(pstSprite, 511, 281, 100, 100);
                return PASS;
            }
        }
        
    }
    
    xpgSpriteEnableTouch(pstSprite);
}

SWORD xpgDrawSprite_LightIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE * pstMask,stMaskRole;
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
            if (dwDialogValue.dwValueData != dwSpriteId)
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
            else //if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = 462;
                wY = pstSprite->m_wPy = 264;
                text = getstr(Str_ZiDingYi);
            }
            wW = pstSprite->m_wWidth = 70;
            wH = pstSprite->m_wHeight = 25;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth); 
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_LIGHT];
            pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_MASK];
            
            if (dwSpriteId == 0)
            {
                if (dwDialogValue.dwValueData)
                    return PASS;
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 200;
                text = "YYYY/MM/DD";
            }
            else if (dwSpriteId == 1)
            {
                if (!dwDialogValue.dwValueData)
                    return PASS;
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 262;
                text = "MM/DD/YYYY";
            }
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 350;
            wH = pstSprite->m_wHeight = 50;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
        }
        else if (dialogType == Dialog_SetLang)
        {
            WORD langid = g_psSetupMenu->bLanguage;
            //if (langid >= 8)
            //    langid = 0;
            if (langid >= 2)
                langid = 0;
            if (langid != dwSpriteId)
                return PASS;
            pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_LIGHT];
            pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_LONG2_LIST_MASK];
            if (dwSpriteId == 0)
            {
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 180;
                text = getstr(Str_JianTiZhongWen);
            }
            else if (dwSpriteId == 1)
            {
                wX = pstSprite->m_wPx = 224;
                wY = pstSprite->m_wPy = 262;
                text = getstr(Str_YingWen);
            }/*
            else if (dwSpriteId == 2)
            {
                wX = pstSprite->m_wPx = 164;
                wY = pstSprite->m_wPy = 202;
                text = getstr(Str_PuTaoYaWen);
            }
            else if (dwSpriteId == 3)
            {
                wX = pstSprite->m_wPx = 404;
                wY = pstSprite->m_wPy = 202;
                text = getstr(Str_XiBanYaWen);
            }
            else if (dwSpriteId == 4)
            {
                wX = pstSprite->m_wPx = 164;
                wY = pstSprite->m_wPy = 266;
                text = getstr(Str_EWen);
            }
            else if (dwSpriteId == 5)
            {
                wX = pstSprite->m_wPx = 404;
                wY = pstSprite->m_wPy = 266;
                text = getstr(Str_FaWen);
            }
            else if (dwSpriteId == 6)
            {
                wX = pstSprite->m_wPx = 164;
                wY = pstSprite->m_wPy = 331;
                text = getstr(Str_TaiWen);
            }
            else if (dwSpriteId == 7)
            {
                wX = pstSprite->m_wPx = 404;
                wY = pstSprite->m_wPy = 331;
                text = getstr(Str_ALaBoWen);
            }*/
            else 
                return PASS;
            wW = pstSprite->m_wWidth = 350;
            wH = pstSprite->m_wHeight = 50;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 10, 0, pstSprite->m_wWidth);
        }
    }
    else if (dwHashKey == xpgHash("FusionSet3"))
    {
		#if 0
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
        else 
		#endif
		if (dwSpriteId == 2 && g_psSetupMenu->bPreHotEnable)
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
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+8, 0, pstSprite->m_wWidth);
            }
        }
        else if (dwSpriteId >= 3 && dwSpriteId <= 4)
        {
            if (g_psSetupMenu->bDuiXianFangShi == dwSpriteId - 3)
            {
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+8, 0, pstSprite->m_wWidth);
            }
        }
        else if (dwSpriteId >= 5)
        {
            if (g_psSetupMenu->bPingXianFangShi == dwSpriteId - 5)
            {
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+8, 0, pstSprite->m_wWidth);
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
    else if(dwHashKey == xpgHash("opm1") || dwHashKey == xpgHash("opm2") || dwHashKey == xpgHash("opmList1"))
    {
		ST_OPM_PAGE *stOpmPagePara=&g_stLocalOpmPagePara;

		if (dwHashKey == xpgHash("opm1"))
		{
	       if (!stOpmPagePara->bPowerOnOff)
				 	return PASS;
			if (dwSpriteId>=10 && dwSpriteId<20)
			{
				if (stOpmPagePara->bCal)
					return PASS;
			}
		}
		else
		{
			stOpmPagePara=&g_stCloudOpmPagePara;
	       if (!stOpmPagePara->bPowerOnOff && dwSpriteId!=11)
				 	return PASS;
		}
		if (dwSpriteId==13)
		{
			MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT6);
			xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
		}
		else if (dwSpriteId==11)
		{
			MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT5);
			xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
		}
		else if (dwSpriteId==14)
		{
			MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, pstSprite->m_wWidth, pstSprite->m_wHeight,BIT4);
			xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, &stMaskRole);
		}
		else if (dwSpriteId>10)
			xpgDrawSprite(pWin, pstSprite, boClip);
		else if (dwSpriteId>=4 && dwSpriteId<=9)
		{
			//if (((stOpmPagePara->bWaveLenth&0x38)>>3)==dwSpriteId-4)
			if (stOpmPagePara->bWaveIconIndex==dwSpriteId)
				xpgDrawSprite(pWin, pstSprite, boClip);
		}
    }
    else if(dwHashKey == xpgHash("opmList2"))
    {
#if 0
        if (dwSpriteId == 1)
        {
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            if (dwSpriteId == 0)
                text = getstr(Str_LocalOPM);
            else 
                text = getstr(Str_CloudOPM);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy+2, 0, pstSprite->m_wWidth);  
        }
#endif
    }
    else if(dwHashKey == xpgHash("RedLight"))
    {
        if (dwSpriteId == 0)
        {
            if (g_psUnsaveParam->bRedPenEnable)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
        }
        else if (dwSpriteId == 1)
        {
        }
        else if (dwSpriteId == 2)
        {
            if (g_psUnsaveParam->bRedPenEnable)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
        }
        else if (dwSpriteId == 3)
        {
            if (g_psUnsaveParam->bRedPenEnable && g_psUnsaveParam->bRedPenHZ)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
        }
        else if (dwSpriteId == 4)
        {
            if (g_psUnsaveParam->bRedPenEnable && g_psUnsaveParam->bRedPenTimerEnable)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
        }
        else if (dwSpriteId == 5)
        {
            if (g_psUnsaveParam->bRedPenEnable && g_psUnsaveParam->bRedPenTimerEnable)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 5);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
        }
    }
    else if(dwHashKey == xpgHash("Keyboard") )
    {
        if (boKeyLight)
        {
            if (dwSpriteId != 0)
                return PASS;
            
            DWORD iconX, iconY;
            STXPGSPRITE * pstIcon, *pstLight = NULL;
            
            pstIcon = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_ICON, dwKeyID);
            iconX = pstIcon->m_wPx;
            iconY = pstIcon->m_wPy;
            if (boCapsLock)
            {
                if (dwKeyID >= 10 && dwKeyID <= 28 || dwKeyID >= 30 && dwKeyID <= 36)
                {
                    DWORD capsId;
                    if (dwKeyID >= 10 && dwKeyID <= 28)
                        capsId = dwKeyID - 10 + 43;
                    else
                        capsId = dwKeyID - 30 + 19 + 43;
                    pstLight = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_LIGHT_ICON, capsId);
                }
                else
                {
                    pstLight = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_LIGHT_ICON, dwKeyID);
                }
            }
            else
            {
                pstLight = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_LIGHT_ICON, dwKeyID);
            }
            xpgDirectDrawRoleOnWin(pWin, pstLight->m_pstRole, iconX, iconY, pstLight, 0);
         //   boKeyLight = FALSE;
        }
    }
    else if (dwHashKey == xpgHash("User"))
    {
        if (g_psSetupMenu->bUserMode != dwSpriteId)
            return PASS;
        
        if (dwSpriteId == 0)
            text = getstr(Str_ZiDongRongJieMoShi);
        else if (dwSpriteId == 1)
            text = getstr(Str_GongChangTiaoXinMoShi);
        else if (dwSpriteId == 2)
            text = getstr(Str_FangDianJiaoZhengMoShi);
        else if (dwSpriteId == 3)
            text = getstr(Str_PingMuHuiChenJianCe);
        else if (dwSpriteId == 4)
            text = getstr(Str_GongChangMoShi);
        else if (dwSpriteId == 5)
            text = getstr(Str_DianJiBangJiHuo);
        
        xpgDrawSprite(pWin, pstSprite, boClip);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy + 3, 0, 300);
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
    else if (dwHashKey == xpgHash("SetPassword"))
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
    STXPGSPRITE * pstMask;
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
             dwHashKey == xpgHash("Upgrade") ||
             dwHashKey == xpgHash("SetInfo") )
    {
        //Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, LEFT_ASIDE_YUV_COLOR);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_XiTongSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionSet1") ||
             dwHashKey == xpgHash("FusionSet2") ||
             dwHashKey == xpgHash("FusionSet3"))
    {
        //Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, LEFT_ASIDE_YUV_COLOR);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_RongJieSheZhi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FusionModeSet") )
    {
        //Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, LEFT_ASIDE_YUV_COLOR);
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
			WORD XX, YY = pstSprite->m_wPy+3;
						
			pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_TEXT, 10);
			if (pstMask)
			{
				XX=pstMask->m_wPx-8;
				Idu_PaintWinArea(pWin, XX, pstSprite->m_wPy, pWin->wWidth-XX*2, 30, RGB2YUV(0x37,0x9d,0xff));
			}
			SetCurrIduFontID(FONT_ID_HeiTi16);
			Idu_PrintString(pWin, getstr(Str_XuHao), 54, YY, 0, 0);
			Idu_PrintString(pWin, getstr(Str_BiaoTi), 290, YY, 0, 0);
			Idu_PrintString(pWin, getstr(Str_SunHao), 504, YY, 0, 0);
			Idu_PrintString(pWin, getstr(Str_GengDuoXinXi), 614, YY, 0, 0);
        }
    }
    else if(dwHashKey == xpgHash("RedLight"))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_HongGuangBi), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("opm1") ||
             dwHashKey == xpgHash("opm2") ||
             dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2"))
    {
        Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 40, RGB2YUV(0x0B, 0x0C, 0x0E));
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_OPMKongZhiMianBan), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if(dwHashKey == xpgHash("Keyboard") )
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
        xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintStringCenter(pWin, keyboardBuffer, pstSprite->m_wPx + 55, pstSprite->m_wPy + 80, 0, 449);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
        dwHashKey == xpgHash("Upgrade") ||
        dwHashKey == xpgHash("SetInfo") ||
        dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3"))
    {
        Idu_PaintWinArea(pWin, 0, 0, 250, pWin->wHeight, LEFT_ASIDE_YUV_COLOR);
    }    
    else if (dwHashKey == xpgHash("opm1") || 
        dwHashKey == xpgHash("opm2") ||
        dwHashKey == xpgHash("opm3") ||
        dwHashKey == xpgHash("opm4"))
    {
        Idu_PaintWinArea(pWin, 0, PAGE_TITLE_HEIGHT, 200, 480-PAGE_TITLE_HEIGHT, LEFT_ASIDE_YUV_COLOR);
    }
    return PASS;
}

SWORD xpgDrawSprite_BackIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
        STXPGROLE * pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_BACK_ICON];
        STXPGROLE * pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_BACK_ICON_MASK];
        xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);

    return PASS;
}

SWORD xpgDrawSprite_CloseIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        STXPGROLE * pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON_OLD];
        STXPGROLE * pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON_MASK];
        if (dialogType == Dialog_ReSuGuan||dialogType == Dialog_Hot_Mode||dialogType == Dialog_Value)
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 40 - 8;
            pstSprite->m_wPy = curDialogTop + 5;
            //xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
		#if 0
        else if (dialogType == Dialog_ModifyNumber||dialogType == Dialog_ShutdownTime||dialogType == Dialog_SleepTime)//no use
        {
            pstSprite->m_wPx = 526;
            pstSprite->m_wPy = 166;
            //xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
		#endif
        else if (dialogType == Dialog_SetBrightness||dialogType == Dialog_SetSound)
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 40 - 8;
            pstSprite->m_wPy = curDialogTop + 5;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else if (dialogType == Dialog_SetTime || dialogType == Dialog_SetDate)
        {
            pstSprite->m_wPx = 548;
            pstSprite->m_wPy = 84;
            //xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 40 - 8;
            pstSprite->m_wPy = curDialogTop + 5;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else if (dialogType == Dialog_SetLang)
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 40 - 8;
            pstSprite->m_wPy = curDialogTop + 5;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword)
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 40 - 8;
            pstSprite->m_wPy = curDialogTop + 5;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else if (dialogType == Dialog_EditValue)
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 40 - 8;
            pstSprite->m_wPy = curDialogTop + 5;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
				/*
        else if (dialogType == Dialog_Value)
        {
            pstSprite->m_wPx = 528;
            pstSprite->m_wPy = 152;
            xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        */
        else if (dialogType == Dialog_About || dialogType == Dialog_Times || dialogType == Dialog_TempInfo || dialogType == Dialog_BatInfo|| (dialogType == Dialog_Record_Detail))
        {
            pstSprite->m_wPx = curDialogLeft + curDialogWidth - 48;
            pstSprite->m_wPy = curDialogTop + 5;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else if (dialogType == Dialog_MainPageError||dialogType == Dialog_MachineWarning)
        {
			STXPGROLE stMaskRole;
			pstSprite->m_wPx = 640-20;
			pstSprite->m_wPy = 90+12;
			pstRole=g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON];
			pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON_NEW_MASK];
			xpgRoleDrawMask(pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask);
        }
        else if (dialogType == Dialog_Note_ForgetHirePassword||dialogType == Dialog_Note_ForgetOpenPassword||dialogType == Dialog_Note_ElectrodeEnable_Path\
			||dialogType == Dialog_Note_ElectrodeEnable_PASS||dialogType == Dialog_Note_ElectrodeEnable_FAIL||dialogType == Dialog_ShutdownRemain\
			||dialogType == Dialog_Note_Electrode_Enable_Process)
        {
            pstSprite->m_wPx = (pWin->wWidth+pWin->wWidth*2/3)/2-50;//DIALOG_DEFAULT_WIDTH
            pstSprite->m_wPy = pWin->wHeight/6+4;
            xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
        }
        else
            return PASS;
        pstSprite->m_wWidth = 40;
        pstSprite->m_wHeight = 40;
        xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("User"))
    {
        xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_CLOSE_ICON], pstSprite->m_wPx, pstSprite->m_wPy, pstSprite, boClip);
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
    char * text = "",*pStr=NULL;
    DWORD dwTextId = pstSprite->m_dwTypeIndex;
	WORD wX=pstSprite->m_wPx;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
	
    if (dwHashKey == xpgHash("Main"))
    {
        if (dwTextId == 0)
            text = getstr(Str_YongHuWeiHu);
        else if (dwTextId == 1)
            text = getstr(Str_RongJieJiLu);
        else if (dwTextId == 2)
            text = getstr(Str_GongNengPeiZhi);
        else if (dwTextId == 3)
            text = getstr(Str_RongJieSheZhi);
        else if (dwTextId == 4)
            text = getstr(Str_XiTongSheZhi);
        else if (dwTextId == 5)
            text = getstr(Str_OPM_YunDuanOPM);
        else if (dwTextId == 6)
            text = getstr(Str_KeShiHuaHongGuangYuan);
        else if (dwTextId == 7)
            text = getstr(Str_DianJiShengYuCiShu);
        else if (dwTextId == 8)
            text = getstr(Str_RongJieZongCiShu2);
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
    }
    else if (dwHashKey == xpgHash("FuncSet"))
    {
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        if (dwTextId < 12)
        {
            if (dwTextId == 0)
                text = getstr(Str_YuJiaRe);
            else if (dwTextId == 1)
                text = getstr(Str_LaLiCeShi);
            else if (dwTextId == 2)
                text = getstr(Str_DuanMianJianCe);
            else if (dwTextId == 3)
                text = getstr(Str_ZiDongDuiJiao);
            else if (dwTextId == 4)
                text = getstr(Str_JiaoDuJianCe);
            else if (dwTextId == FUNCTION_ID_LAMP)
                text = getstr(Str_ZhaoMingDeng);//Str_FangDianJiaoZheng
            else if (dwTextId == 6)
                text = getstr(Str_HuiChenJianCe);
            else if (dwTextId == 7)
                text = getstr(Str_RongJieZanTing);
            else if (dwTextId == 8)
                text = getstr(Str_HongGuangBi);
            else if (dwTextId == 9)
                text = getstr(Str_GuangGongLvJi);
            else if (dwTextId == 10)
                text = getstr(Str_BaoCunTuXiang);
            else if (dwTextId == 11)
                text = getstr(Str_YunDuanCeLiang);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 12)
        {
            text = getstr(Str_ClickCanSetupFunction);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
            Idu_PaintWinArea(pWin, 290, 84, 470, 2, RGB2YUV(0,0,0));
        }
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
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
                text = getstr(Str_ZiDongDuiJiao);
            else if (value == 4)
                text = getstr(Str_JiaoDuJianCe);
            else if (value == FUNCTION_ID_LAMP)
                text = getstr(Str_ZhaoMingDeng);
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
                text = getstr(Str_ZiDongDuiJiao);
            else if (dwTextId == 10)
                text = getstr(Str_JiaoDuJianCe);
            else if (dwTextId == 6+FUNCTION_ID_LAMP)
                text = getstr(Str_ZhaoMingDeng);
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
            text = getstr(Str_Display_CanChoose6Icons);
            Idu_PaintWinArea(pWin, 290, 82, 470, 2, RGB2YUV(0,0,0));
        }
        else if (dwTextId == 19){
            text = getstr(Str_TianJiaKongZhi);
            Idu_PaintWinArea(pWin, 290, 245, 470, 2, RGB2YUV(0,0,0));
        }

        //if (dwTextId <= 17)
            SetCurrIduFontID(FONT_ID_HeiTi16);
        //else 
        //    SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("ToolBox"))
    {
        if (dwTextId == 0)
            text = getstr(Str_HongGuangBi);
        else if (dwTextId == 1)
            text = getstr(Str_GuangGongLvJi);
        else if (dwTextId == 2)
            text = getstr(Str_Str_DuanMianJianCeYi);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 66);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, tempStr, 4, 139, 0, 74);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, tempStr, 4, 190, 0, 74);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, tempStr, 722, 139, 0, 74);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, tempStr, 722, 190, 0, 74);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, tempStr, 212, 380, 0, 70);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, tempStr, 212, 406, 0, 70);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
        {
            if (g_psSetupMenu->bReSuGuanSheZhi == 0)
            {
                text = "40mm";
            }
            else if (g_psSetupMenu->bReSuGuanSheZhi == 1)
            {
                text = "45mm";
            }
            else if (g_psSetupMenu->bReSuGuanSheZhi == 2)
            {
                text = "50mm";
            }
            else if (g_psSetupMenu->bReSuGuanSheZhi == 3)
            {
                text = "55mm";
            }
            else if (g_psSetupMenu->bReSuGuanSheZhi == 4)
            {
                text = "60mm";
            }
            else //if (dwSpriteId == 5)
            {
                text = getstr(Str_ZiDingYi);
            }
            sprintf(tmpbuf, "%s >", text);
        }
        else if (dwTextId == 1)
            sprintf(tmpbuf, "%d%s >", g_psSetupMenu->wJiaReWenDu, getstr(Str_DuC));
        else if (dwTextId == 2)
            sprintf(tmpbuf, "%dS >", g_psSetupMenu->wJiaReShiJian);
		else if (dwTextId == 3)
		{
			if (g_psSetupMenu->bHotUpMode == SETUP_MENU_HOT_UP_MODE_AUTO)
            sprintf(tmpbuf, "%s >", getstr(Str_ZiDong));
			else
            sprintf(tmpbuf, "%s >", getstr(Str_ShouDong));
		}
        else
            return PASS;
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("FusionSet2"))
    {
        if (dwTextId == 0)
        {
            text = getstr(Str_RongJieZhiLiang);
            Idu_PaintWinArea(pWin, 290, 159, 470, 2, RGB2YUV(0,0,0));
        }
        else if (dwTextId == 1)
        {
            text = getstr(Str_DuiXianFangShi);
            Idu_PaintWinArea(pWin, 290, 279, 470, 2, RGB2YUV(0,0,0));
        }
        else if (dwTextId == 2)
        {
            text = getstr(Str_PingXianFangShi);
            Idu_PaintWinArea(pWin, 290, 399, 470, 2, RGB2YUV(0,0,0));
        }
        else
            return PASS;
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("FusionSet1"))
    {
        MODEPARAM * pstModeParam;
        char * titleText = "";
        char tmpText[256];
        const BYTE JIANGE = 24;
        
        if (dwTextId != 0)
            return PASS;
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, 468, 250, RGB2YUV(0x24, 0x24, 0x24));
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 36, 468, 2, RGB2YUV(0x31, 0x31, 0x31));

		if (g_psSetupMenu->bCurrFusionMode>=WELD_FIBER_PRAR_TOTAL)
			g_psSetupMenu->bCurrFusionMode=0;
		pstModeParam = &(g_psSetupMenu->SM);
		pstModeParam+=g_psSetupMenu->bCurrFusionMode;

        text = (char*) FModeStrList[g_psSetupMenu->bCurrFusionMode];
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
        Idu_PrintString(pWin, getstr(Str_DuChinese), 724, 222 + JIANGE * 4, 0, 0);

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
        
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
                text = getstr(Str_DuChinese);
                break;
            }
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        char tmpbuf[256];
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
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
        else if (dwTextId == 4)
        {
            if (g_psSetupMenu->bLanguage == 1)
                text = getstr(Str_YingWen);
            else if (g_psSetupMenu->bLanguage == 2)
                text = getstr(Str_PuTaoYaWen);
            else if (g_psSetupMenu->bLanguage == 3)
                text = getstr(Str_XiBanYaWen);
            else if (g_psSetupMenu->bLanguage == 4)
                text = getstr(Str_EWen);
            else if (g_psSetupMenu->bLanguage == 5)
                text = getstr(Str_FaWen);
            else if (g_psSetupMenu->bLanguage == 6)
                text = getstr(Str_TaiWen);
            else if (g_psSetupMenu->bLanguage == 7)
                text = getstr(Str_ALaBoWen);
            else
                text = getstr(Str_JianTiZhongWen);
            sprintf(tmpbuf, "%s >", text);
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("SetYun"))
    {
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        if (dwTextId == 1)
        {
        	  char tmpbuf[16];

            sprintf(tmpbuf, "%01x%02x%02x%02x%02x%02x", g_psSetupMenu->bMADarry[5],g_psSetupMenu->bMADarry[4],g_psSetupMenu->bMADarry[3]\
							,g_psSetupMenu->bMADarry[2],g_psSetupMenu->bMADarry[1],g_psSetupMenu->bMADarry[0]);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            //text = "13800138000";
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        else if (dwTextId == 2)
        {
            SetCurrIduFontID(FONT_ID_HeiTi19);
            if (g_psSetupMenu->bCloudMode && g_psUnsaveParam->bCloudOPMonline)
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
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        char tmpbuf[64];
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        if (dwTextId == 1)
        {
            if (!g_psSetupMenu->bLowPowerMode)
            {
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
                strcpy(tmpbuf, "-->");
            }
            else 
            {
                sprintf(tmpbuf, "%dmin >", g_psSetupMenu->bSleepTime);
            }
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
        }
        else if (dwTextId == 3)
        {
            if (!g_psSetupMenu->bAutoShutdown)
            {
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
                sprintf(tmpbuf, "-->");
            }
            else
                sprintf(tmpbuf, "%dmin >", g_psSetupMenu->wShutdownTime);
            if (tmpbuf[0])
            {
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);
            }
        }
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        char tmpbuf[64];
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        
        if (dwTextId == 1)
        {
	        if (!g_psSetupMenu->bToundSoundEnable)
	            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
            if (!g_psSetupMenu->bToundSoundEnable)
            {
                strcpy(tmpbuf, "-- >");
            }
            else 
            {
                int volume;
                if (g_psSetupMenu->bVolume <= 15)
                    volume = g_psSetupMenu->bVolume;
                else
                    volume = 15;
                sprintf(tmpbuf, "%d >", volume);
            }
        }
        else if (dwTextId == 2)
        {
                sprintf(tmpbuf, "%d >", g_psSetupMenu->bBrightness);
        }
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy, 0);

        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if(dwHashKey == xpgHash("opm1") || dwHashKey == xpgHash("opm2") /*|| dwHashKey == xpgHash("opm3") || dwHashKey == xpgHash("opm4")*/)
    {
			ST_OPM_PAGE *stOpmPagePara=&g_stCloudOpmPagePara;
			
			if (dwHashKey == xpgHash("opm1"))
				stOpmPagePara=&g_stLocalOpmPagePara;
	       if (!stOpmPagePara->bPowerOnOff)
				 	return PASS;
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
			switch (dwTextId)
			{
				case 0:
					mpPaintWinArea(pWin, pstSprite->m_wPx+8, pstSprite->m_wPy+8, pstSprite->m_wWidth-16, pstSprite->m_wHeight-16, 0xffff8080);
					if (dwHashKey == xpgHash("opm2"))
					{
						if (stOpmPagePara->bStaus<4)
						{
							SetCurrIduFontID(FONT_ID_ARIAL_36);
							Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
							sprintf(xpgStringBuffer, "LINKING");
							BYTE i;
							for (i=0;i<stOpmPagePara->bStaus;i++)
								strcat(xpgStringBuffer,".");
							Idu_PrintString(pWin, xpgStringBuffer, pstSprite->m_wPx+40, pstSprite->m_wPy+24, 0,0);
							Ui_TimerProcAdd(800, OpmCloudShowLinkingStatus);
						}
					}
					break;
				case 1:
					if (stOpmPagePara->bUnit)
					{
						SDWORD sdwIntData;
						BYTE bPoint,i;
						DWORD dwPointData=1;
						
						sdwIntData=g_stOpmRealData.wdbm;
						if (!(g_stOpmRealData.bWaveLenth&BIT7))
							sdwIntData=-sdwIntData;
						bPoint=g_stOpmRealData.bDbDbmDot>>4;
						for (i=0;i<bPoint;i++)
							dwPointData*=10;
						sprintf(xpgStringBuffer, "%02d.%02d", sdwIntData/dwPointData,sdwIntData%dwPointData);
					}
					else
					{
						sprintf(xpgStringBuffer, "Lo");
					}
					SetCurrIduFontID(FONT_ID_ARIAL_36);
					Idu_PrintStringRight(pWin, xpgStringBuffer, pstSprite->m_wPx, pstSprite->m_wPy, 0);
					break;

				case 2:
					if (stOpmPagePara->bUnit)
					{
						sprintf(xpgStringBuffer, "dBm");
						SetCurrIduFontID(FONT_ID_HeiTi19);
						Idu_PrintString(pWin, xpgStringBuffer, pstSprite->m_wPx, pstSprite->m_wPy,0, 0);
					}
					break;

				case 3:
					if (stOpmPagePara->bCal)
					{
						sprintf(xpgStringBuffer, "CAL");
					}
					else if (stOpmPagePara->bUnit)
					{
						SDWORD sdwIntData;
						BYTE bPoint,i;
						DWORD dwPointData=1;
						
						sdwIntData=g_stOpmRealData.wdb;
						if (!(g_stOpmRealData.bWaveLenth&BIT6))
							sdwIntData=-sdwIntData;
						bPoint=g_stOpmRealData.bDbDbmDot&0x0f;
						for (i=0;i<bPoint;i++)
							dwPointData*=10;
						sprintf(xpgStringBuffer, "%02d.%02d", sdwIntData/dwPointData,sdwIntData%dwPointData);
					}
					else
					{
						SDWORD sdwIntData;
						BYTE bPoint,i;
						DWORD dwPointData=1;
						
						sdwIntData=g_stOpmRealData.wuW;
						bPoint=g_stOpmRealData.bWaveLenth&0x07;
						for (i=0;i<bPoint;i++)
							dwPointData*=10;
						sprintf(xpgStringBuffer, "%02d.%02d", sdwIntData/dwPointData,sdwIntData%dwPointData);
					}
					SetCurrIduFontID(FONT_ID_ARIAL_36);
					Idu_PrintString(pWin, xpgStringBuffer, pstSprite->m_wPx, pstSprite->m_wPy, 0,0);
					break;

				case 4:
					if (!stOpmPagePara->bCal)
					{
						if (stOpmPagePara->bUnit)
						{
							sprintf(xpgStringBuffer, "dB");
						}
						else
						{
							sprintf(xpgStringBuffer, "uW");
						}
						SetCurrIduFontID(FONT_ID_HeiTi19);
						Idu_PrintString(pWin, xpgStringBuffer, pstSprite->m_wPx, pstSprite->m_wPy, 0,0);
					}
					break;

				default:
					break;
			}
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if(dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2"))
    {
        /////
        DWORD * pdwPageId;
        DWORD * pdwTotal;
        OPMDATAITEM * pstItems;
        OPMDATAITEM * curItem;
        char tempbuf[128];
        if (dwHashKey == xpgHash("opmList1"))
        {
            pdwPageId = &opmLocalDataCurrentPage;
            pdwTotal = &opmLocalDataTotal;
            pstItems = localOpmData;
        }
        else
        {
            pdwPageId = &opmCloudDataCurrentPage;
            pdwTotal = &opmCloudDataTotal;
            pstItems = cloudOpmData;
        }
        /////////////////////////
        if (dwTextId == 0)
        {
            SetCurrIduFontID(FONT_ID_HeiTi19);
            if(dwHashKey == xpgHash("opmList1"))
                text = getstr(Str_BenDiShuJuBiao);
            else
                text = getstr(Str_YunDuanShuJuBiao);
            Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 1)
        {
            DWORD pageNum;
            char tmpbuf1[128];
            char tmpbuf2[128];
            sprintf(tmpbuf1, getstr(Str_GongDuoShaoTiaoJiLu), *pdwTotal);
            sprintf(tmpbuf2, "[ %s", tmpbuf1);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, tmpbuf2, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
            pageNum = *pdwTotal / 5;
            if (*pdwTotal % 5)
                pageNum++;
            sprintf(tmpbuf2, "%s %d/%d]", getstr(Str_YeShu), *pdwPageId + 1, pageNum);
            Idu_PrintString(pWin, tmpbuf2, pstSprite->m_wPx + 430, pstSprite->m_wPy, 0, 0);
        }
        else if (dwTextId == 2)
        {
            WORD W = 50, H = 30;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H, RGB2YUV(55, 157, 255));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, getstr(Str_ShouYe), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
            return PASS;
        }
        else if (dwTextId == 3)
        {
            WORD W = 62, H = 30;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H, RGB2YUV(55, 157, 255));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, getstr(Str_PrevPage), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
            return PASS;
        }
        else if (dwTextId == 4)
        {
            WORD W = 62, H = 30;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H, RGB2YUV(55, 157, 255));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, getstr(Str_NextPage), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
            return PASS;
        }
        else if (dwTextId == 5)
        {
            WORD W = 50, H = 30;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H, RGB2YUV(55, 157, 255));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx - 4, pstSprite->m_wPy - 4, W, H);
            SetCurrIduFontID(FONT_ID_HeiTi16);
            Idu_PrintString(pWin, getstr(Str_WeiYe), pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
            return PASS;
        }
    }
    else if (dwHashKey == xpgHash("Record") )
    {
	    if (dwTextId >=10 && dwTextId<70)
	    {
			char tmpbuff[128];
			STRECORD * pr;
			int iCurIndex;
			BYTE bListMain=dwTextId/10-1,ListSub=dwTextId%10;

			//mpDebugPrint("dwTextId= %d bListMain=%d ListSub=%d",dwTextId,bListMain,ListSub);
			if (!g_WeldRecordPage.dwTotalData)
				return PASS;
			iCurIndex =  PAGE_RECORD_SIZE * g_WeldRecordPage.wCurPage + bListMain;
			if (iCurIndex>g_WeldRecordPage.dwTotalData-1)
				return PASS;
			pr = GetRecord((DWORD)iCurIndex);
			if (pr == NULL)
				return PASS;

			tmpbuff[0]=0;
			switch (ListSub)
			{
				case 0:
					sprintf(tmpbuff, "%04d [%04d/%d/%d %02d:%02d:%02d]", iCurIndex+1, 2000+pr->bYear, pr->bMonth, pr->bDay, pr->bHour, pr->bMinute, pr->bSecond);
					wX=pstSprite->m_wPx-8;
					Idu_PaintWinArea(pWin, wX, pstSprite->m_wPy + IduFontGetMaxHeight()*2-4,pWin->wWidth-wX*2, 2, RGB2YUV(0x20, 0x20, 0x20));
					break;

				case 1:
					sprintf(tmpbuff, "%s", pr->bRecordName);
					xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx-pstSprite->m_wWidth, pstSprite->m_wPy-pstSprite->m_wHeight, pstSprite->m_wWidth*3, pstSprite->m_wHeight*3);
					break;

				case 2:
					sprintf(tmpbuff, "%d.%ddB", pr->bFiberLoss%100, pr->bFiberLoss /100);
					break;

				case 3:
					sprintf(tmpbuff, getstr(Str_DianJiChaKan));
					xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx-pstSprite->m_wWidth, pstSprite->m_wPy-pstSprite->m_wHeight, pstSprite->m_wWidth*2, pstSprite->m_wHeight*3);
					break;

				default:
					break;
			}
			if (tmpbuff[0])
			{
					SetCurrIduFontID(FONT_ID_HeiTi16);
					Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
					wX=Idu_PrintString(pWin, tmpbuff, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
					if (ListSub==3 && wX>pstSprite->m_wPx)
						Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + IduFontGetMaxHeight()+1, wX-pstSprite->m_wPx, 1, RGB2YUV(0x20, 0x20, 0x20));
					Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
			}
	    }
		else //if (dwTextId >=70)
		{
			SetCurrIduFontID(FONT_ID_HeiTi16);
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
	        char tmpbuf1[64];
	        if (dwTextId == 70)
	        {
	            char tmpbuf2[64];

	            sprintf(tmpbuf2, getstr(Str_GongDuoShaoTiaoJiLu), g_WeldRecordPage.dwTotalData);
	            sprintf(tmpbuf1, "[  %s", tmpbuf2);
	        }
	        else if (dwTextId == 71)
	        {
	            sprintf(tmpbuf1,"%s : %d/%d  ]" , getstr(Str_YeShu),g_WeldRecordPage.wCurPage+1,g_WeldRecordPage.wTotalPage);
	        }
			Idu_PrintString(pWin, tmpbuf1, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
		}
    }
    else if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        /*if (dialogType == Dialog_ModifyNumber)
        {
            WORD width, height;
            char tmpbuf[64];
            pstSprite->m_wPx = 350;
            pstSprite->m_wPy = 238;
            width = 100;
            height = 40;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, width, height, 0xffff8080);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            sprintf(tmpbuf, "%d", dwDialogValue.dwValueData);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy + 6, 0, 100);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else*/ if (dialogType == Dialog_SetBrightness)
        {
            WORD width, height;
            char tmpbuf[64];
            pstSprite->m_wPx = 290;
            pstSprite->m_wPy = 210;
            width = 220;
            height = 100;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, width, height, RGB2YUV(0, 0, 0));
            //Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            sprintf(tmpbuf, "%d", dwDialogValue.dwValueData);
            SetCurrIduFontID(FONT_ID_ARIAL_36);
            Idu_PrintStringCenter(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy + 20, 0, width);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_ModifyNumber||dialogType == Dialog_ShutdownTime||dialogType == Dialog_SleepTime)
        {
            WORD width, height;
            char tmpbuf[64];
            pstSprite->m_wPx = 290;
            pstSprite->m_wPy = 180;
            width = 220;
            height = 100;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, width, height, 0x00008080);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
			if (dialogType == Dialog_ShutdownTime||dialogType == Dialog_SleepTime)
	            sprintf(tmpbuf, "%dmin", dwDialogValue.dwValueData);
			else
	            sprintf(tmpbuf, "%d", dwDialogValue.dwValueData);
            SetCurrIduFontID(FONT_ID_ARIAL_36);
            Idu_PrintStringCenter(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy + 20, 0, 220);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetSound)
        {
            WORD width, height;
            char tmpbuf[64];
            pstSprite->m_wPx = 290;
            pstSprite->m_wPy = 210;
            width = 220;
            height = 100;
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, width, height, RGB2YUV(0, 0, 0));
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            sprintf(tmpbuf, "%d", dwDialogValue.dwValueData);
            SetCurrIduFontID(FONT_ID_ARIAL_36);
            Idu_PrintStringCenter(pWin, tmpbuf, pstSprite->m_wPx, pstSprite->m_wPy + 20, 0, width);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetTime)
        {
            WORD hour1 = 0, minute1 = 0;
            WORD hour2 = 0, minute2 = 0;
            WORD hour3 = 0, minute3 = 0;
            hour2 = dwDialogValue.dwValueData >> 16;
            minute2 = dwDialogValue.dwValueData & 0xffff;
            char tmpbuf[64];
            STXPGROLE * pstRole, *pstMask;

            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
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
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    sprintf(tmpbuf, "%d", hour1);
                    Idu_PrintStringCenter(pWin, tmpbuf, 270, 163, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 304, 123, pWin->wWidth, pWin->wHeight, pstMask);
                }
                Idu_PaintWinArea(pWin, 270, 200, 100, 60, RGB2YUV(0,0,0));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                sprintf(tmpbuf, "%d", hour2);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                Idu_PrintStringCenter(pWin, tmpbuf, 270, 220, 0, 100);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                if (hour2 != 23)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    sprintf(tmpbuf, "%d", hour3);
                    Idu_PrintStringCenter(pWin, tmpbuf, 270, 288, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 304, 324, pWin->wWidth, pWin->wHeight, pstMask);
                }

                if (minute2 != 0) 
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    sprintf(tmpbuf, "%d", minute1);
                    Idu_PrintStringCenter(pWin, tmpbuf, 430, 163, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 466, 123, pWin->wWidth, pWin->wHeight, pstMask);
                }
                Idu_PaintWinArea(pWin, 430, 200, 100, 60, RGB2YUV(0,0,0));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                sprintf(tmpbuf, "%d", minute2);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                Idu_PrintStringCenter(pWin, tmpbuf, 430, 220, 0, 100);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                if (minute2 != 59)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    sprintf(tmpbuf, "%d", minute3);
                    Idu_PrintStringCenter(pWin, tmpbuf, 430, 288, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 466, 324, pWin->wWidth, pWin->wHeight, pstMask);
                }
                //Idu_PaintWinArea(pWin, 304, 205, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 304, 255, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 426, 205, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 426, 255, 70, 2, RGB2YUV(140,140,140));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintString(pWin, ":", 398, 224, 0, 0);
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
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, hstr1, 190, 162, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 224, 122, pWin->wWidth, pWin->wHeight, pstMask);
                }
                Idu_PaintWinArea(pWin, 190, 199, 100, 60, RGB2YUV(0,0,0));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                Idu_PrintStringCenter(pWin, hstr2, 190, 219, 0, 100);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                if (hstr3[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, hstr3, 190, 287, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 224, 323, pWin->wWidth, pWin->wHeight, pstMask);
                }

                if (mstr1[0] != 0) 
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, mstr1, 350, 162, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 386, 122, pWin->wWidth, pWin->wHeight, pstMask);
                }
                Idu_PaintWinArea(pWin, 350, 199, 100, 60, RGB2YUV(0,0,0));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                Idu_PrintStringCenter(pWin, mstr2, 350, 219, 0, 100);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                if (mstr3[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, mstr3, 350, 287, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 386, 323, pWin->wWidth, pWin->wHeight, pstMask);
                }

                if (apmstr1[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, apmstr1, 510, 162, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 546, 122, pWin->wWidth, pWin->wHeight, pstMask);
                }
                Idu_PaintWinArea(pWin, 510, 199, 100, 60, RGB2YUV(0,0,0));
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                Idu_PrintStringCenter(pWin, apmstr2, 510, 219, 0, 100);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                if (apmstr3[0] != 0)
                {
                    SetCurrIduFontID(FONT_ID_HeiTi19);
                    Idu_PrintStringCenter(pWin, apmstr3, 510, 287, 0, 100);
                    pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                    pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                    xpgRoleDrawMask(pstRole, pWin->pdwStart, 546, 323, pWin->wWidth, pWin->wHeight, pstMask);
                }

                //Idu_PaintWinArea(pWin, 244, 205, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 244, 255, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 366, 205, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 366, 255, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 488, 205, 70, 2, RGB2YUV(140,140,140));
                //Idu_PaintWinArea(pWin, 488, 255, 70, 2, RGB2YUV(140,140,140));
                
                SetCurrIduFontID(FONT_ID_HeiTi19);
                Idu_PrintString(pWin, ":", 318, 223, 0, 0);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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

            year2 = dwDialogValue.dwValueData >> 16;
            month2 = (dwDialogValue.dwValueData >> 8) & 0xff;
            day2 = dwDialogValue.dwValueData & 0xff;

            sprintf(ystr2, "%04d", year2);
            sprintf(mstr2, "%d", month2);
            sprintf(dstr2, "%d", day2);

            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            
            if (year2 > 2000)
            {
                year1 = year2 - 1;
                sprintf(ystr1, "%04d", year1);
            }
            if (year2 < 2556)
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
                Idu_PrintStringCenter(pWin, ystr1, 190, 162, 0, 100);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 224, 122, pWin->wWidth, pWin->wHeight, pstMask);
            }
            Idu_PaintWinArea(pWin, 190, 199, 100, 60, RGB2YUV(0,0,0));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            Idu_PrintStringCenter(pWin, ystr2, 190, 219, 0, 100);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (ystr3[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, ystr3, 190, 287, 0, 100);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 224, 323, pWin->wWidth, pWin->wHeight, pstMask);
            }
            
            if (mstr1[0] != 0) 
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, mstr1, 350, 162, 0, 100);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 386, 122, pWin->wWidth, pWin->wHeight, pstMask);
            }
            Idu_PaintWinArea(pWin, 350, 199, 100, 60, RGB2YUV(0,0,0));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            Idu_PrintStringCenter(pWin, mstr2, 350, 219, 0, 100);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (mstr3[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, mstr3, 350, 287, 0, 100);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 386, 323, pWin->wWidth, pWin->wHeight, pstMask);
            }

            if (dstr1[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, dstr1, 510, 162, 0, 100);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_UP_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 546, 122, pWin->wWidth, pWin->wHeight, pstMask);
            }
            Idu_PaintWinArea(pWin, 510, 199, 100, 60, RGB2YUV(0,0,0));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
            Idu_PrintStringCenter(pWin, dstr2, 510, 219, 0, 100);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dstr3[0] != 0)
            {
                SetCurrIduFontID(FONT_ID_HeiTi16);
                Idu_PrintStringCenter(pWin, dstr3, 510, 287, 0, 100);
                pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_ICON];
                pstMask = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_DOWN_MASK];
                xpgRoleDrawMask(pstRole, pWin->pdwStart, 546, 323, pWin->wWidth, pWin->wHeight, pstMask);
            }
            //////////////////////
            //Idu_PaintWinArea(pWin, 244, 205, 70, 2, RGB2YUV(140,140,140));
            //Idu_PaintWinArea(pWin, 244, 255, 70, 2, RGB2YUV(140,140,140));
            //Idu_PaintWinArea(pWin, 366, 205, 70, 2, RGB2YUV(140,140,140));
            //Idu_PaintWinArea(pWin, 366, 255, 70, 2, RGB2YUV(140,140,140));
            //Idu_PaintWinArea(pWin, 488, 205, 70, 2, RGB2YUV(140,140,140));
            //Idu_PaintWinArea(pWin, 488, 255, 70, 2, RGB2YUV(140,140,140));
                
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, "/", 313, 217, 0, 0);
            Idu_PrintString(pWin, "/", 472, 217, 0, 0);
            
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_About)
        {
            char text1[128];
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dwTextId == 0)
            {
                sprintf(text1, "%s%s", getstr(Str_XingHao),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 198, 0, 0);
                sprintf(text1, "%s", "K5");
                Idu_PrintStringRight(pWin, text1, 590, 198, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s%s", getstr(Str_ZhiZaoShang),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 234, 0, 0);
                sprintf(text1, "%s", getstr(Str_GongSiMing));
                Idu_PrintStringRight(pWin, text1, 590, 234, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s%s", getstr(Str_Note_MakeDate),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 270, 0, 0);
                sprintf(text1, "%s", "2020/8/8");
                Idu_PrintStringRight(pWin, text1, 590, 270, 0);
            }
            else if (dwTextId == 3)
            {
                sprintf(text1, "%s%s", getstr(Str_MAD),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 306, 0, 0);
	            sprintf(text1, "%01x%02x%02x%02x%02x%02x", g_psSetupMenu->bMADarry[5],g_psSetupMenu->bMADarry[4],g_psSetupMenu->bMADarry[3]\
								,g_psSetupMenu->bMADarry[2],g_psSetupMenu->bMADarry[1],g_psSetupMenu->bMADarry[0]);
                Idu_PrintStringRight(pWin, text1, 590, 306, 0);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_Times)
        {
            char text1[128];
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dwTextId == 0)
            {
                sprintf(text1, "%s%s", getstr(Str_XuLieHao),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 198, 0, 0);
                sprintf(text1, "%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d%01d", g_psSetupMenu->bElectrodeInfo[5]>>4,g_psSetupMenu->bElectrodeInfo[5]&0x0f\
									, g_psSetupMenu->bElectrodeInfo[4]>>4,g_psSetupMenu->bElectrodeInfo[4]&0x0f, g_psSetupMenu->bElectrodeInfo[3]>>4,g_psSetupMenu->bElectrodeInfo[3]&0x0f\
									, g_psSetupMenu->bElectrodeInfo[2]>>4,g_psSetupMenu->bElectrodeInfo[2]&0x0f, g_psSetupMenu->bElectrodeInfo[1]>>4,g_psSetupMenu->bElectrodeInfo[1]&0x0f\
									, g_psSetupMenu->bElectrodeInfo[0]>>4,g_psSetupMenu->bElectrodeInfo[0]&0x0f);
                Idu_PrintStringRight(pWin, text1, 590, 198, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s%s", getstr(Str_JiHuoRiQi),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 234, 0, 0);
                sprintf(text1, "%04d/%d/%d", 2000+g_psSetupMenu->bElectrodeInfo[6],g_psSetupMenu->bElectrodeInfo[7],g_psSetupMenu->bElectrodeInfo[8]);
                Idu_PrintStringRight(pWin, text1, 590, 234, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s%s", getstr(Str_ShengYuCiShu),getstr(Str_Note_MaoHao));
                Idu_PrintString(pWin, text1, 210, 270, 0, 0);
                sprintf(text1, "%d", g_psSetupMenu->wElectrodeRemainTimes);
                Idu_PrintStringRight(pWin, text1, 590, 270, 0);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_TempInfo)
        {
            char text1[128], text2[128];
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dwTextId == 0)
            {
                sprintf(text1, "%s%s", getstr(Str_HuanJingWenDu),getstr(Str_Note_MaoHao));
                sprintf(text2, "%d%s", 28, getstr(Str_DuC));
                Idu_PrintString(pWin, text1, 210, 198, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 198, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s%s", getstr(Str_NeiBuWenDu),getstr(Str_Note_MaoHao));
                sprintf(text2, "%d%s", 30, getstr(Str_DuC));
                Idu_PrintString(pWin, text1, 210, 234, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 234, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s%s", getstr(Str_HuanJingShiDu),getstr(Str_Note_MaoHao));
                sprintf(text2, "%d%%", 60);
                Idu_PrintString(pWin, text1, 210, 270, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 270, 0);
            }
            else if (dwTextId == 3)
            {
                sprintf(text1, "%s%s", getstr(Str_QiYa),getstr(Str_Note_MaoHao));
                sprintf(text2, "%dkPa", 101);
                Idu_PrintString(pWin, text1, 210, 306, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 306, 0);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_BatInfo)
        {
            char text1[128], text2[128];
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dwTextId == 0)
            {
                sprintf(text1, "%s:", getstr(Str_DianChiRongLiang));
                sprintf(text2, "%d%%", 100);
                Idu_PrintString(pWin, text1, 210, 198, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 198, 0);
            }
            else if (dwTextId == 1)
            {
                sprintf(text1, "%s:", getstr(Str_FangDianCiShu));
                sprintf(text2, "%d%s", 15, getstr(Str_Ci));
                Idu_PrintString(pWin, text1, 210, 234, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 234, 0);
            }
            else if (dwTextId == 2)
            {
                sprintf(text1, "%s:", getstr(Str_ShiYongShiJian));
                sprintf(text2, "%d%s", 30, getstr(Str_FenZhong));
                Idu_PrintString(pWin, text1, 210, 270, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 270, 0);
            }
            else if (dwTextId == 3)
            {
                sprintf(text1, "%s:", getstr(Str_DaiJiShiJian));
                sprintf(text2, "%d%s%d%s", 3, getstr(Str_XiaoShi), 45, getstr(Str_FenZhong));
                Idu_PrintString(pWin, text1, 210, 306, 0, 0);
                Idu_PrintStringRight(pWin, text2, 590, 306, 0);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword\
					|| dialogType == Dialog_PowerOnCheckHirePassword|| dialogType == Dialog_PowerOnCheckOpenPassword|| dialogType == Dialog_Electrode_SN_Input\
					|| dialogType == Dialog_Electrode_CheckCode_Input)
        {
            WORD x = 0, y = 108, w = 50, h = 50;
            char str[2];
            str[1] = 0;
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dwTextId == 0)
            {
                x = 270;
                //mpDebugPrint("cur-input password = %s", strEditPassword);
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(0xc9,0xc9,0xc9));
                Idu_PaintWinArea(pWin, x+2, y+2, w-4, h-4, RGB2YUV(0xff,0xff,0xff));
                str[0] = strEditPassword[0];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+10, 0, w);
                x = 340;
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(0xc9,0xc9,0xc9));
                Idu_PaintWinArea(pWin, x+2, y+2, w-4, h-4, RGB2YUV(0xff,0xff,0xff));
                str[0] = strEditPassword[1];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+10, 0, w);
                x = 410;
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(0xc9,0xc9,0xc9));
                Idu_PaintWinArea(pWin, x+2, y+2, w-4, h-4, RGB2YUV(0xff,0xff,0xff));
                str[0] = strEditPassword[2];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+10, 0, w);
                x = 480;
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(0xc9,0xc9,0xc9));
                Idu_PaintWinArea(pWin, x+2, y+2, w-4, h-4, RGB2YUV(0xff,0xff,0xff));
                str[0] = strEditPassword[3];
                if (str[0] != 0)
                    Idu_PrintStringCenter(pWin, str, x, y+10, 0, w);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if(dialogType == Dialog_Value)
        {
            WORD x = 260, y = 210, w = 278, h = 42;
            char tmpbuf[128];
            if (dwTextId == 0)
            {
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(255,255,255));
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
                if (boDialogValueIsFloat)
                    sprintf(tmpbuf, "%d.%d", dwDialogValue.dwValueData>>6, dwDialogValue.dwValueData&0x3F);
                else
                    sprintf(tmpbuf, "%d", dwDialogValue.dwValueData);
                Idu_PrintString(pWin, tmpbuf, x + 6, y + 4, 0, 0);
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
                xpgSpriteSetTouchArea(pstSprite, x, y, w, h);
            }
        }
        else if (dialogType == Dialog_EditValue)
        {
            WORD x = 250, y = 108, w = 300, h = 50;
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dwTextId == 0)
            {
                Idu_PaintWinArea(pWin, x, y, w, h, RGB2YUV(0xc9,0xc9,0xc9));
                Idu_PaintWinArea(pWin, x+2, y+2, w-4, h-4, RGB2YUV(0xff,0xff,0xff));
                if (strEditValue[0])
                    Idu_PrintStringCenter(pWin, strEditValue, x+8, y+10, 0, w - 20);
            }
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
		else if (dialogType == Dialog_Note_ForgetHirePassword||dialogType == Dialog_Note_ForgetOpenPassword||dialogType == Dialog_Note_ElectrodeEnable_Path\
			||dialogType == Dialog_Note_ElectrodeEnable_PASS||dialogType == Dialog_Note_ElectrodeEnable_FAIL||dialogType == Dialog_ShutdownRemain\
			||dialogType == Dialog_Note_Reinput||dialogType == Dialog_Note_PasswordError||dialogType == Dialog_Note_ChangeBootWordPASS\
			||dialogType == Dialog_Note_ChangeHireWordPASS||dialogType == Dialog_Note_Electrode_Enable_Process)
		{
			char tempStr[10];
			SetCurrIduFontID(FONT_ID_HeiTi19);
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
			if (dialogType == Dialog_Note_ElectrodeEnable_Path)
			{
              sprintf(tempStr, "%04d%04d", g_wElectrodeSN,g_wElectrodeRandomCode);
              Idu_PrintStringCenter(pWin, tempStr, (pWin->wWidth/3)>>1, pWin->wHeight/2-IduFontGetMaxHeight()*2, 0, pWin->wWidth*2/3);
				pStr=getstr(Str_Note_GetElectrodeEnableCode);
				
              sprintf(tempStr, "(%04d)", ProduceElectrodeCheckCode());
              Idu_PrintStringCenter(pWin, tempStr, (pWin->wWidth/3)>>1, pWin->wHeight/2+IduFontGetMaxHeight()*3, 0, pWin->wWidth*2/3);
			}
			else if (dialogType == Dialog_Note_Electrode_Enable_Process)
				pStr=getstr(Str_EnableProcing);
			else if (dialogType == Dialog_Note_ElectrodeEnable_PASS)
				pStr=getstr(Str_ElectrodeEnablePass);
			else if (dialogType == Dialog_Note_ElectrodeEnable_FAIL)
			{
				if (dwDialogValue.dwValueData==1)
					pStr=getstr(Str_ElectrodeSNnotExist);
				else if (dwDialogValue.dwValueData==2)
					pStr=getstr(Str_ElectrodeEnableFail);
				else
					pStr=getstr(Str_ElectrodeCheckCodeError);
			}
			else if (dialogType == Dialog_ShutdownRemain)
			{
				extern DWORD g_dwPowerOff;

				sprintf(tempStr, "%d", g_dwPowerOff/41);
				SetCurrIduFontID(FONT_ID_ARIAL_36);
				Idu_PrintString(pWin, tempStr, pWin->wWidth/2-20, pWin->wHeight>>1, 0, 0);
			}
			else if (dialogType == Dialog_Note_Reinput)
				pStr=getstr(Str_Note_Reinput);
			else if (dialogType == Dialog_Note_PasswordError)
				pStr=getstr(Str_Password_Error);
			else if (dialogType == Dialog_Note_ChangeBootWordPASS)
				pStr=getstr(Str_Note_ChangeBootWordOk);
			else if (dialogType == Dialog_Note_ChangeHireWordPASS)
				pStr=getstr(Str_Note_ChangeHireWordOk);
			else //if (dialogType == Dialog_Note_ForgetHirePassword||dialogType == Dialog_Note_ForgetOpenPassword)
				pStr=getstr(Str_Info_ForgetPassword);
			if (pStr)
				Idu_PrintStringCenterNewLine(pWin, pStr, (pWin->wWidth/3)>>1, pWin->wHeight/2-IduFontGetMaxHeight(), 0, pWin->wWidth*2/3);
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
		}
		else if (dialogType == Dialog_YESNO_Upgrade||dialogType == Dialog_Note_ClearAll)
		{
			BYTE bString[64];

			if (dialogType == Dialog_Note_ClearAll)
			{
				pStr=getstr(Str_IfClearAll);
			}
			else
			{
				sprintf(bString, "%s(V%d.%d.%d.%d)?",getstr(Str_IfUpgrade),g_psUnsaveParam->wCpuNewVersion>>8,g_psUnsaveParam->wCpuNewVersion&0xff,g_psUnsaveParam->wMcuNewVersion>>8,g_psUnsaveParam->wMcuNewVersion&0xff);
				pStr=&bString[0];
			}

			SetCurrIduFontID(FONT_ID_HeiTi19);
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
			Idu_PrintStringCenterNewLine(pWin, pStr, curDialogLeft+curDialogWidth/8, curDialogTop+curDialogHeight/2-IduFontGetMaxHeight(), 0, curDialogWidth*3/4);
			Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
		}
	    else if (dialogType == Dialog_Record_Detail)
	    {
				BYTE bString[128],*pStr="";
				STRECORD * pr;

				pr = GetRecord(g_WeldRecordPage.dwCurIndex);
				if (pr == NULL)
					return PASS;

				SetCurrIduFontID(FONT_ID_HeiTi19);
				Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
				if (dwTextId)
				{
					if (pr->bResult==1)
						pStr=getstr(Str_WeldOk);
					else if (pr->bResult==2)
						pStr=getstr(Str_WeldFail);
					sprintf(bString, "%s: SM    L:%d%s    R:%d%s   %s:0.%02ddB    %s:%s",getstr(Str_Mode),pr->bFiberL,getstr(Str_Du),pr->bFiberR,getstr(Str_Du),getstr(Str_Loss),pr->bFiberLoss,getstr(Str_Result),pStr);
					Idu_PrintString(pWin, bString, curDialogLeft+20, curDialogTop+curDialogHeight-40, 0, 0);
					//--jpg
					WORD w=ALIGN_16(curDialogWidth-40),h=curDialogHeight-130,x,y;
					ST_IMGWIN stTmpWin;

					x=(pWin->wWidth-w)>>1;
					y=(pWin->wHeight-h)>>1;
					mpWinInit(&stTmpWin ,NULL, h, w); 
					stTmpWin.pdwStart=pWin->pdwStart+x/2+y*pWin->dwOffset/4;
					stTmpWin.dwOffset=pWin->dwOffset;
					FileListSetCurIndex(pr->dwFileIndex);
					if (ImageDraw(&stTmpWin,1)!=PASS)
						Idu_PaintWinArea(pWin, x, y, w, h, 0xcccc8080);
				}
				else
				{
					sprintf(bString, "%03d %s", g_WeldRecordPage.dwCurIndex,pr->bRecordName);
					Idu_PrintString(pWin, bString, curDialogLeft+20, curDialogTop+10, 0, 0);
				}
				Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, LEFT_ASIDE_YUV_COLOR);
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_ZiDingYiTuBiao), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            //xpgSpriteEnableTouch(pstSprite);
        }
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        if (dwSelectorId == 0)
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, LEFT_ASIDE_YUV_COLOR);
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, getstr(Str_GongNengPeiZhi), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            //xpgSpriteEnableTouch(pstSprite);
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
            dwHashKey == xpgHash("Upgrade") ||
            dwHashKey == xpgHash("SetInfo"))
    {
        if (dwSelectorId > 6)
            return PASS;
        
        DWORD curPageType;
        if(dwSelectorId == 0)
            text = getstr(Str_YunDuanSheZhi);
        else if (dwSelectorId == 1)
            text = getstr(Str_ShengYinSheZhi);
        else if (dwSelectorId == 2)
            text = getstr(Str_PingMuYuDaiJi);
        else if (dwSelectorId == 3)
            text = getstr(Str_ShiJianYuYuYan);
        else if (dwSelectorId == 4)
            text = getstr(Str_MiMaSheZhi);
        else if (dwSelectorId == 5)
            text = getstr(Str_XiTongXinXi);
        else //if (dwSelectorId == 6)
            text = getstr(Str_Firmware_upgrade);

        if (dwHashKey == xpgHash("SetYun"))
            curPageType = 0;
        else if (dwHashKey == xpgHash("SetSleep"))
            curPageType = 2;
        else if (dwHashKey == xpgHash("SetSound"))
            curPageType = 1;
        else if (dwHashKey == xpgHash("SetTime"))
            curPageType = 3;
        else if (dwHashKey == xpgHash("SetPassword"))
            curPageType = 4;
        //else if (dwHashKey == xpgHash("SetUi"))
        //    curPageType = 5;
        else if (dwHashKey == xpgHash("SetInfo"))
            curPageType = 5; // 6;
        else if (dwHashKey == xpgHash("Upgrade"))
            curPageType = 6;
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        if (dwSelectorId == curPageType)
        {
            xpgDrawSprite(pWin, pstSprite, boClip);
        }
        else
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, LEFT_ASIDE_YUV_COLOR);
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            //xpgSpriteEnableTouch(pstSprite);
        }
        Idu_PrintString(pWin, text, pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
        Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
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
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, LEFT_ASIDE_YUV_COLOR);
            Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 1, RGB2YUV(0x2E, 0x2E, 0x2E));
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintString(pWin, text, pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
            Idu_PrintString(pWin, ">", pstSprite->m_wPx + 220, pstSprite->m_wPy + 12, 0, 0);
            //xpgSpriteEnableTouch(pstSprite);
        }
    }
    else if (dwHashKey == xpgHash("opm1"))
    {
			/*
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, RGB2YUV(20, 20, 20));
        Idu_PaintWinArea(pWin, pstSprite->m_wPx + pstSprite->m_wWidth - 2, pstSprite->m_wPy, 2, pstSprite->m_wHeight, RGB2YUV(0x2E, 0x2E, 0x2E));
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + pstSprite->m_wHeight - 2, pstSprite->m_wWidth, 2, RGB2YUV(0x2E, 0x2E, 0x2E));
		*/
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_BenDiShuJu), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
        Idu_PrintString(pWin, ">", pstSprite->m_wPx + 164, pstSprite->m_wPy + 12, 0, 0);
        //xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("opm2"))
    {
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_YunDuanShuJu), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
        Idu_PrintString(pWin, ">", pstSprite->m_wPx + 164, pstSprite->m_wPy + 12, 0, 0);
    }
    else if (dwHashKey == xpgHash("opmList1"))
    {
        xpgDrawSprite(pWin, pstSprite, boClip);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_BenDiShuJu), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
        Idu_PrintString(pWin, ">", pstSprite->m_wPx + 168, pstSprite->m_wPy + 12, 0, 0);
        //xpgSpriteEnableTouch(pstSprite);
    }
    else if (dwHashKey == xpgHash("opmList2"))
    {
        xpgDrawSprite(pWin, pstSprite, boClip);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, getstr(Str_YunDuanShuJu), pstSprite->m_wPx + 20, pstSprite->m_wPy + 12, 0, 0);
        Idu_PrintString(pWin, ">", pstSprite->m_wPx + 168, pstSprite->m_wPy + 12, 0, 0);
        //xpgSpriteEnableTouch(pstSprite);
    }




}

SWORD xpgDrawSprite_List(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char * text = "";
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    DWORD dwListId = pstSprite->m_dwTypeIndex;
    WORD wX = pstSprite->m_wPx;
	register STXPGBROWSER *pstBrowser = g_pstBrowser;
	SWORD y;
    
    if (dwHashKey == xpgHash("SetYun"))
    {
        if (dwListId == 0)
            text = getstr(Str_YunDuanMoShi);
        else if (dwListId == 1)
            text = getstr(Str_MAD);
        else if (dwListId == 2)
            text = getstr(Str_YunDuanOPMZhuangTai);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));
    }
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        DWORD lineColor = RGB2YUV(0x2F, 0x2F, 0x2F);
        if (dwListId == 0)
            text = getstr(Str_LowPowerMode);
        else if (dwListId == 1)
            text = getstr(Str_SleepTime);
        else if (dwListId == 2)
            text = getstr(Str_ZiDongGuanJi);
        else if (dwListId == 3)
            text = getstr(Str_GuanJiShiJian);

        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        if (!g_psSetupMenu->bLowPowerMode && dwListId == 1) 
        {
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
            lineColor = RGB2YUV(191, 191, 191);
        }
        if (!g_psSetupMenu->bAutoShutdown && dwListId == 3) 
        {
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
            lineColor = RGB2YUV(191, 191, 191);
        }
        
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
		y=pstSprite->m_wPy+IduFontGetMaxHeight()+8;

        if (dwListId == 0)
        {
            text = getstr(Str_LowPowerMode_DESC);
            SetCurrIduFontID(FONT_ID_HeiTi10);
            Idu_PrintStringLeftNewLine(pWin, text, pstSprite->m_wPx, y, 0, pWin->wWidth-pstSprite->m_wPx-40);
			  y+=IduFontGetMaxHeight()*2;
        }
        if (dwListId == 2)
        {
            text = getstr(Str_ZiDongGuanJi_DESC);
            SetCurrIduFontID(FONT_ID_HeiTi10);
            Idu_PrintStringLeftNewLine(pWin, text, pstSprite->m_wPx, y, 0, pWin->wWidth-pstSprite->m_wPx-40);
			  y+=IduFontGetMaxHeight();
        }
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, y+8, 470, 2, lineColor);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        DWORD dwColor = RGB2YUV(0x2F, 0x2F, 0x2F);
        if (dwListId == 0)
            text = getstr(Str_ChuPingShengYin);
        else if (dwListId == 1)
            text = getstr(Str_YinLiangTiaoJie);
        else if (dwListId == 2)
            text = getstr(Str_LiangDuTiaoJie);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        if (!g_psSetupMenu->bToundSoundEnable && dwListId == 1)
        {
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
            dwColor = RGB2YUV(191, 191, 191);
        }
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, dwColor);
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
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        if (dwListId < 5)
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));

        if (dwListId >= 1 && dwListId <= 4)
        {
            //Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy - 20, 470, 56, RGB2YUV(0xff, 0xff, 0x37));
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 20, 470, 56);
        }
    }
    else if (dwHashKey == xpgHash("SetPassword"))
    {
        STXPGSPRITE * pstMask1;
        STXPGSPRITE * pstMask2;
        WORD lineWidth = 470; 
        DWORD lineColor = IDU_FONT_YUVCOLOR_BLACK;
        DWORD textX = pstSprite->m_wPx;
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        if (dwListId == 0)
            text = getstr(Str_KaiJiMiMa);
        else if (dwListId == 1)
        {
            if (!g_psSetupMenu->bEnableOpenPassword)
            {
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
                lineColor = IDU_FONT_YUVCOLOR_GREY;
            }
            text = getstr(Str_GengGaiKaiJiMiMa);
            Idu_PrintStringRight(pWin, ">", pstSprite->m_wPx + lineWidth, pstSprite->m_wPy, 0);
        }
        else if (dwListId == 2)
            return PASS;
        else if (dwListId == 3)
            text = getstr(Str_SuoDingMiMa);
        else if (dwListId == 4)
        {
            if (!g_psSetupMenu->bEnableHirePassword)
            {
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
                lineColor = IDU_FONT_YUVCOLOR_GREY;
            }
            text = getstr(Str_GengGaiSuoDingMiMa);
            Idu_PrintStringRight(pWin, ">", pstSprite->m_wPx + lineWidth, pstSprite->m_wPy, 0);
        }
        else if (dwListId == 5)
        {
            char tmpbuf[64];
            textX += 30;
            text = getstr(Str_SuoDingRiQi);
            if (g_psSetupMenu->bEnableHirePassword)
            {
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
                lineColor = IDU_FONT_YUVCOLOR_GREY;
            }
            DWORD radioX = pstSprite->m_wPx;
            DWORD radioY = pstSprite->m_wPy + 4;
            pstMask1 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, g_psSetupMenu->bEnableHirePassword ? 5 : 1);
            pstMask2 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask1 && pstMask2)
                xpgRoleDrawMask(pstMask1->m_pstRole, pWin->pdwStart, radioX, radioY, pWin->wWidth, pWin->wHeight, pstMask2->m_pstRole);
            if (g_psSetupMenu->bMachineLockMode&BIT0)
            {
                pstMask1 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, g_psSetupMenu->bEnableHirePassword ? 6 : 3);
                pstMask2 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 4);
                if (pstMask1 && pstMask2)
                    xpgRoleDrawMask(pstMask1->m_pstRole, pWin->pdwStart, radioX + 6, radioY + 5, pWin->wWidth, pWin->wHeight, pstMask2->m_pstRole);
            }

            sprintf(tmpbuf, "%04d/%02d/%02d%s >", g_psSetupMenu->wLockDateYear, g_psSetupMenu->bLockDateMonth, g_psSetupMenu->bLockDateDay, getstr(Str_Zhi));
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx + lineWidth, pstSprite->m_wPy, 0);
        }
        else if (dwListId == 6)
        {
            char tmpbuf[64];
            textX += 30;
            text = getstr(Str_SuoDingRongJieCiShu);
            if (g_psSetupMenu->bEnableHirePassword)
            {
                Idu_SetFontYUV(IDU_FONT_YUVCOLOR_GREY);
                lineColor = IDU_FONT_YUVCOLOR_GREY;
            }
            DWORD radioX = pstSprite->m_wPx;
            DWORD radioY = pstSprite->m_wPy + 4;
            pstMask1 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, g_psSetupMenu->bEnableHirePassword ? 5 : 1);
            pstMask2 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
            if (pstMask1 && pstMask2)
                xpgRoleDrawMask(pstMask1->m_pstRole, pWin->pdwStart, radioX, radioY, pWin->wWidth, pWin->wHeight, pstMask2->m_pstRole);
            if (g_psSetupMenu->bMachineLockMode&BIT1)
            {
                pstMask1 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, g_psSetupMenu->bEnableHirePassword ? 6 : 3);
                pstMask2 = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 4);
                if (pstMask1 && pstMask2)
                    xpgRoleDrawMask(pstMask1->m_pstRole, pWin->pdwStart, radioX + 6, radioY + 5, pWin->wWidth, pWin->wHeight, pstMask2->m_pstRole);
            }

            sprintf(tmpbuf, "%d%s >", g_psSetupMenu->wLockedTimes, getstr(Str_Ci));
            Idu_PrintStringRight(pWin, tmpbuf, pstSprite->m_wPx + lineWidth, pstSprite->m_wPy, 0);
        }

        //Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy - 14, lineWidth, 50, RGB2YUV(0x4F, 0x4F, 0x00));
        xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 14, lineWidth, 50);
        
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintString(pWin, text, textX, pstSprite->m_wPy, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, lineWidth, 2, lineColor);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);

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
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 38, 470, 2, RGB2YUV(0x2F, 0x2F, 0x2F));

        char text1[128];
        if (dwListId == 0)
        {
            sprintf(text1, "%s:%s", getstr(Str_XingHao), "K5 >");
        }
        else if (dwListId == 1)
        {
            sprintf(text1, "%s:%d >", getstr(Str_ShengYuCiShu), g_psSetupMenu->wElectrodeRemainTimes);
        }
        else if (dwListId == 2)
        {		
			int iData;
			if (g_psUnsaveParam->bTemperatureInhome[0]&BIT7)
				iData=-(g_psUnsaveParam->bTemperatureInhome[0]&0x3f);
			else
				iData=(g_psUnsaveParam->bTemperatureInhome[0]&0x3f);
            sprintf(text1, "%s:%d%s >", getstr(Str_NeiBuWenDu), iData, getstr(Str_DuC));
        }
				/*
        else if (dwListId == 3)
        {
            sprintf(text1, "%s:%d%%", getstr(Str_DianChiRongLiang), 100);
        }
        */
        else if (dwListId == 3)
        {
            sprintf(text1, "%d%s", 200, getstr(Str_Ci));
        }
        else if (dwListId == 4)
        {
            sprintf(text1, "%d.%d",g_psSetupMenu->bMcuLocalVer[0],g_psSetupMenu->bMcuLocalVer[1]);
        }
        else if (dwListId == 5)
        {
            sprintf(text1, "%d.%d",PRODUCT_MAIN_VER,PRODUCT_SUB_VER);
        }
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintStringRight(pWin, text1, pstSprite->m_wPx + 470, pstSprite->m_wPy, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
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
    else if (dwHashKey == xpgHash("FusionSet3"))
    {
        if (dwListId == 0)
            text = getstr(Str_YuReMoShi);
        else if (dwListId == 1)
        {
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy - 18, pWin->wWidth-pstSprite->m_wPx, 60);
            text = getstr(Str_JiaReFangShi);
        }
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
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
        Idu_PrintString(pWin, text, pstSprite->m_wPx, pstSprite->m_wPy, 0, 0);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 42, 472, 2, RGB2YUV(0x37, 0x37, 0x37));
    }
    else if (dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2") )
    {
        if (dwListId == 0)
        {
            Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy, 540, 2, RGB2YUV(0x41, 0x41, 0x41));
        }
        /////
        DWORD * pdwPageId;
        DWORD * pdwTotal;
        OPMDATAITEM * pstItems;
        OPMDATAITEM * curItem;
        char tempbuf[128];
        if (dwHashKey == xpgHash("opmList1"))
        {
            pdwPageId = &opmLocalDataCurrentPage;
            pdwTotal = &opmLocalDataTotal;
            pstItems = localOpmData;
        }
        else
        {
            pdwPageId = &opmCloudDataCurrentPage;
            pdwTotal = &opmCloudDataTotal;
            pstItems = cloudOpmData;
        }

        if ((*pdwPageId) * pstBrowser->wListCount + dwListId >= *pdwTotal)
            return PASS;
        
        curItem = & pstItems[(*pdwPageId) * pstBrowser->wListCount + dwListId];
        SetCurrIduFontID(FONT_ID_HeiTi16);
        Idu_PrintString(pWin, curItem->itemName, pstSprite->m_wPx, pstSprite->m_wPy + 20, 0, 0);
        sprintf(tempbuf, "[%04d/%d/%d %02d:%02d:%02d]", curItem->year, curItem->month, curItem->day, curItem->hour, curItem->minute, curItem->second);
        Idu_PrintString(pWin, tempbuf, pstSprite->m_wPx, pstSprite->m_wPy + 40, 0, 0);
        sprintf(tempbuf, "%dnm %dBm %ddB", curItem->nm, curItem->db1, curItem->db2);
        Idu_PrintString(pWin, tempbuf, pstSprite->m_wPx + 278, pstSprite->m_wPy + 30, 0, 0);
        Idu_PaintWinArea(pWin, pstSprite->m_wPx, pstSprite->m_wPy + 66, 540, 2, RGB2YUV(0x41, 0x41, 0x41));
        xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy, 480, 66);
    }
    
    return PASS;
}

void MakeDialogColorRole(STXPGROLE * pstRole, WORD width, WORD height,DWORD dwColor)
{
    DWORD dwSize;
    ST_IMGWIN win;
    
    xpgRoleInit(pstRole);
    pstRole->m_bBitDepth = 24;
    pstRole->m_wIndex = 0;
    pstRole->m_wWidth = ALIGN_2(width);
    pstRole->m_wHeight = height;
    if (pstRole->m_wHeight < 4)
        pstRole->m_wHeight = 4;
    
    dwSize = ALIGN_16(pstRole->m_wWidth) * ALIGN_16(pstRole->m_wHeight) * 2 + 256;

    pstRole->m_pRawImage = (BYTE *) ext_mem_malloc(dwSize);
    pstRole->m_wRawWidth = pstRole->m_wWidth;

	MP_ASSERT(pstRole->m_pRawImage != NULL);
    MP_DEBUG4("malloc role %d, w%d h%d raw_w%d", pstRole->m_wIndex, pstRole->m_wWidth, pstRole->m_wHeight, pstRole->m_wRawWidth);

    mpWinInit(&win, pstRole->m_pRawImage, pstRole->m_wHeight, pstRole->m_wWidth);
    Idu_PaintWinArea(&win, 0, 0, win.wWidth, win.wHeight, dwColor);
}
/*
void MakeDialogRoleNew(STXPGROLE * pstRole, WORD width, WORD height)
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
    if (pstRole->m_wHeight < 51)
        pstRole->m_wHeight = 51;
    
    dwSize = ALIGN_16(pstRole->m_wWidth) * ALIGN_16(pstRole->m_wHeight) * 2 + 256;

    pstRole->m_pRawImage = (BYTE *) ext_mem_malloc(dwSize);
    pstRole->m_wRawWidth = ALIGN_16(pstRole->m_wWidth);

	MP_ASSERT(pstRole->m_pRawImage != NULL);
    MP_DEBUG4("malloc role %d, w%d h%d raw_w%d", pstRole->m_wIndex, pstRole->m_wWidth, pstRole->m_wHeight, pstRole->m_wRawWidth);

    mpWinInit(&win, pstRole->m_pRawImage, pstRole->m_wHeight, pstRole->m_wRawWidth);
    pWin = &win;

    Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, 50, RGB2YUV(255, 192, 0));
    Idu_PaintWinArea(pWin, 0, 50, pWin->wWidth, pstRole->m_wHeight - 50, RGB2YUV(0xff, 0xff, 0xff));
    
}
*/
//bMode: 1->黄TITLE + 白底
void MakeDialogRole(STXPGROLE * pstRole, BYTE bMode,WORD width, WORD height) 
{
	WORD wTitleHeight;
    DWORD dwSize,dwTitleColor,dwBodyColor;
    ST_IMGWIN win;
    ST_IMGWIN * pWin;
    
    xpgRoleInit(pstRole);
    pstRole->m_bBitDepth = 24;
    pstRole->m_wIndex = 0;
    pstRole->m_wWidth = ALIGN_2(width);
    pstRole->m_wHeight = height;
    if (pstRole->m_wHeight < 41)
        pstRole->m_wHeight = 41;
	if (bMode)
	{
		dwTitleColor=0xbcbc15af;//RGB2YUV(255, 192, 0);
		dwBodyColor=0xffff8080;
	}
	else
	{
		dwTitleColor=0x2929827f;//RGB2YUV(0x28, 0x29, 0x2D);
		dwBodyColor=0x1d1d827f;//RGB2YUV(0x1C, 0x1D, 0x21);
	    //mpDebugPrint("dwTitleColor=%p dwBodyColor=%p", dwTitleColor,dwBodyColor);
	}
	if (pstRole->m_wHeight>240)
		wTitleHeight=50;
	else
		wTitleHeight=40;
    dwSize = ALIGN_16(pstRole->m_wWidth) * ALIGN_16(pstRole->m_wHeight) * 2 + 256;

    pstRole->m_pRawImage = (BYTE *) ext_mem_malloc(dwSize);
    pstRole->m_wRawWidth = pstRole->m_wWidth;

	MP_ASSERT(pstRole->m_pRawImage != NULL);
    MP_DEBUG4("malloc role %d, w%d h%d raw_w%d", pstRole->m_wIndex, pstRole->m_wWidth, pstRole->m_wHeight, pstRole->m_wRawWidth);

    mpWinInit(&win, pstRole->m_pRawImage, pstRole->m_wHeight, pstRole->m_wWidth);
    pWin = &win;

    Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, wTitleHeight, dwTitleColor);//RGB2YVU(0x28, 0x29, 0x2D)
    Idu_PaintWinArea(pWin, 0, wTitleHeight, pWin->wWidth, pstRole->m_wHeight - wTitleHeight, dwBodyColor);//RGB2YVU(0x1C, 0x1D, 0x21)
}

//--bMaskConerFlag: BIT7->left top BIT6->right top BIT5->left bottom BIT40->right bottom
void MakeMaskRole(STXPGROLE * pstMaskRole, int maskRoleIndex, WORD width, WORD height,BYTE bMaskConerFlag)
{
    ST_IMGWIN win1, win2;
    ST_IMGWIN * pWin1, * pWin2;
    DWORD * pdwStart1, * pdwStart2;
    WORD iconWidth;
    WORD iconHeight;
    WORD corner;
	STXPGROLE * pstRole = g_pstXpgMovie->m_pstObjRole[maskRoleIndex];

	#if 1
	iconWidth = pstRole->m_wWidth;
	iconHeight = pstRole->m_wHeight;
	corner = iconHeight>>1;
	#else
    if (maskRoleIndex == XPG_ROLE_ICON_MASK_0)
    {
        iconWidth = 70;
        iconHeight = 70;
        corner = 12;
    }
    else 
    {
        return;
    }
	#endif
    
    pdwStart1 = (DWORD*) ext_mem_malloc((iconHeight) * ALIGN_16(iconWidth<<1) <<1);
    mpWinInit(&win1, pdwStart1, iconHeight, ALIGN_16(iconWidth <<1));
    pWin1 = &win1;
    xpgDirectDrawRoleOnWin(pWin1, g_pstXpgMovie->m_pstObjRole[maskRoleIndex], 0, 0, NULL, 0);
/*
    if (width < corner * 2)
        width = corner * 2;
    if (height < corner * 2)
        height = corner * 2;
    if (width % 2)
        width ++;
    */
    pdwStart2 = (DWORD*) ext_mem_malloc(ALIGN_16(width) * height * 2);
    mpWinInit(&win2, pdwStart2, height, width);
    pWin2 = &win2;
    mpPaintWin(pWin2, 0xffff8080);

	if (bMaskConerFlag&BIT7)
	    mpCopyWinAreaSameSize(pWin1, pWin2, 0, 0, 0, 0, corner, corner);
	if (bMaskConerFlag&BIT6)
	    mpCopyWinAreaSameSize(pWin1, pWin2, iconWidth - corner, 0, width - corner, 0, corner, corner);
	if (bMaskConerFlag&BIT5)
	    mpCopyWinAreaSameSize(pWin1, pWin2, 0, iconHeight - corner, 0, height - corner, corner, corner);
	if (bMaskConerFlag&BIT4)
	    mpCopyWinAreaSameSize(pWin1, pWin2, iconWidth - corner, iconHeight - corner, width - corner, height - corner, corner, corner);

    ext_mem_free(pdwStart1);
    
    xpgRoleInit(pstMaskRole);
    pstMaskRole->m_bBitDepth = 24;
    pstMaskRole->m_wIndex = 0;
    pstMaskRole->m_wWidth = width;
    pstMaskRole->m_wHeight = height;
    pstMaskRole->m_pRawImage = (BYTE *)pdwStart2;
    pstMaskRole->m_wRawWidth = pstMaskRole->m_wWidth;

}


void DrawRoundIcon(ST_IMGWIN * pWin,int maskRoleIndex,WORD wX,WORD wY,WORD wW,WORD wH,DWORD dwYUVcolor)
{
	STXPGROLE  stRole, stMaskRole;

	wW=ALIGN_2(wW);
	MakeDialogColorRole(&stRole, wW,wH,dwYUVcolor);
	MakeMaskRole((STXPGROLE *)&stMaskRole, maskRoleIndex, wW,wH,0xf0);
	xpgRoleDrawMask(&stRole, pWin->pdwStart, wX,wY, pWin->wWidth, pWin->wHeight, &stMaskRole);
	if (stRole.m_pRawImage)
	{
		ext_mem_freeEx(stRole.m_pRawImage, __FILE__, __LINE__);
		stRole.m_pRawImage=NULL;
	}
	if (stMaskRole.m_pRawImage)
	{
		ext_mem_freeEx(stMaskRole.m_pRawImage, __FILE__, __LINE__);
		stMaskRole.m_pRawImage=NULL;
	}
}

void DrawRoundDialog(ST_IMGWIN * pWin,BYTE bMode,int maskRoleIndex,WORD wX,WORD wY,WORD wW,WORD wH)
{
	STXPGROLE  stRole, stMaskRole;

	wW=ALIGN_2(wW);
	curDialogWidth = wW;
	curDialogHeight = wH;
	curDialogLeft = wX;
	curDialogTop = wY;
	MakeDialogRole((STXPGROLE *)&stRole,bMode, wW, wH);
	MakeMaskRole((STXPGROLE *)&stMaskRole, maskRoleIndex, wW, wH,0xf0);
	xpgRoleDrawMask(&stRole, pWin->pdwStart, wX, wY, pWin->wWidth, pWin->wHeight, &stMaskRole);
	if (stRole.m_pRawImage)
	{
		ext_mem_freeEx(stRole.m_pRawImage, __FILE__, __LINE__);
		stRole.m_pRawImage=NULL;
	}
	if (stMaskRole.m_pRawImage)
	{
		ext_mem_freeEx(stMaskRole.m_pRawImage, __FILE__, __LINE__);
		stMaskRole.m_pRawImage=NULL;
	}
}

SWORD xpgDrawSprite_Dialog(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    char *text = "",*pStr=NULL;
    STXPGROLE  stRole, stMaskRole;
	STXPGROLE *pstRole, *pstMask;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    xpgRoleInit(&stRole);
    xpgRoleInit(&stMaskRole);
    
    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogW, dialogH, dailogX, dialogY;
        int dialogType = xpgGetCurrDialogTypeId();
        if (dialogType == Dialog_ReSuGuan||dialogType == Dialog_Hot_Mode)
        {
			dialogW = 560;
			dialogH = 300;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
			SetCurrIduFontID(FONT_ID_HeiTi19);
			if (strDialogTitle)
				Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 5, 0, dialogW);
        }
				/*
        else if (dialogType == Dialog_ModifyNumber)
        {
            dialogW = 350;
            dialogH = 300;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, getstr(Str_ReSuGuanSheZhi), dailogX, dialogY + 5, 0, dialogW);
        }
        */
        else if (dialogType == Dialog_SetBrightness)
        {
            dialogW = 560;
            dialogH = 300;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, getstr(Str_LiangDuTiaoJie), dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_ModifyNumber||dialogType == Dialog_ShutdownTime||dialogType == Dialog_SleepTime)
        {
            dialogW = 560;
            dialogH = 320;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetSound)
        {
            dialogW = 560;
            dialogH = 300;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, getstr(Str_YinLiangTiaoJie), dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetTime || dialogType == Dialog_SetDate)
        {
            dialogW = 560;
            dialogH = 415;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            if (dialogType == Dialog_SetTime)
                text = getstr(Str_ShiJianSheZhi);
            else
                text = getstr(Str_RiQiSheZhi);
            Idu_PrintStringCenter(pWin, text, dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            dialogW = 560;
            dialogH = 380;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, getstr(Str_RiQiGeShi), dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_SetLang)
        {
            dialogW = 560;
            dialogH = 380;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, getstr(Str_XiTongYuYan), dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_YESNO_Upgrade||dialogType == Dialog_Note_ClearAll)
        {
				dialogW = 560;
				dialogH = 380;
				dailogX=(pWin->wWidth - dialogW)>>1;
				dialogY=(pWin->wHeight - dialogH)>>1;
				DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
				SetCurrIduFontID(FONT_ID_HeiTi19);
				Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
				Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 10, 0, dialogW);
				Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_Record_Detail)
        {
				dialogW = 600;
				dialogH = 350;
				dailogX=(pWin->wWidth - dialogW)>>1;
				dialogY=(pWin->wHeight - dialogH)>>1;
				DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword || dialogType == Dialog_EditValue\
					|| dialogType == Dialog_PowerOnCheckHirePassword|| dialogType == Dialog_PowerOnCheckOpenPassword|| dialogType == Dialog_Electrode_SN_Input\
					|| dialogType == Dialog_Electrode_CheckCode_Input)
        {
				dialogW = 360;
				dialogH = 420;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
				SetCurrIduFontID(FONT_ID_HeiTi19);
				/*
            if (dialogType == Dialog_About)
                text = getstr(Str_GuanYuBenJi);
            else if (dialogType == Dialog_Times)
                text = getstr(Str_DianJiBangXinXi);
            else if (dialogType == Dialog_TempInfo)
                text = getstr(Str_WenDuXinXi);
            else if (dialogType == Dialog_BatInfo)
                text = getstr(Str_DianChiXinXi);
             */
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_Value)
        {
            dialogW = 350;
            dialogH = 180;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,0,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 5, 0, dialogW);
        }
        else if (dialogType == Dialog_About || dialogType == Dialog_Times || dialogType == Dialog_TempInfo || dialogType == Dialog_BatInfo)
        {
            dialogW = 560;
            dialogH = 300;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
            SetCurrIduFontID(FONT_ID_HeiTi19);
            if (dialogType == Dialog_About)
                text = getstr(Str_GuanYuBenJi);
            else if (dialogType == Dialog_Times)
                text = getstr(Str_DianJiBangXinXi);
            else if (dialogType == Dialog_TempInfo)
                text = getstr(Str_WenDuXinXi);
            else if (dialogType == Dialog_BatInfo)
                text = getstr(Str_DianChiXinXi);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            Idu_PrintStringCenter(pWin, text, dailogX, dialogY + 10, 0, dialogW);
            Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
        else if (dialogType == Dialog_MainPageError||dialogType == Dialog_MachineWarning)
        {
				dialogW = 560;//pstRole->m_wWidth;
				dialogH = 300;//pstRole->m_wHeight;
				dailogX = (pWin->wWidth - dialogW) / 2;
				dialogY = (pWin->wHeight - dialogH) / 2;
		       //MakeDialogColorRole(&stRole, dialogW, dialogH,RGB2YUV(0xf9,0x59,0x5a));
		       // MakeMaskRole((STXPGROLE *)&stMaskRole, XPG_ROLE_ICON_MASK_0, dialogW, dialogH,0xf0);
		       // xpgRoleDrawMask(&stRole, pWin->pdwStart, dailogX, dialogY, pWin->wWidth, pWin->wHeight, &stMaskRole);
				DrawRoundIcon(pWin,XPG_ROLE_ICON_MASK_0,dailogX, dialogY,dialogW, dialogH,RGB2YUV(0xf9,0x59,0x5a));
			 	pstRole = g_pstXpgMovie->m_pstObjRole[XPG_ROLE_MAIN_ERROR_ICON];
				xpgDirectDrawRoleOnWin(pWin, pstRole,dailogX+(dialogW-pstRole->m_wWidth)/2, dialogY+40, NULL, boClip);
				SetCurrIduFontID(FONT_ID_HeiTi19);
				if (g_dwMachineErrorFlag&g_dwMachineErrorShow)
				{
					if (g_dwMachineErrorFlag&MACHINE_ERROR_LOCKED)
						Idu_PrintStringCenter(pWin, getstr(Str_Error_SystemLocked), dailogX, dialogY + dialogH/2+10, 0, dialogW);
					else if (g_dwMachineErrorFlag&MACHINE_ERROR_SENSOR)
						Idu_PrintStringCenter(pWin, getstr(Str_SheXiangTouGuZhang), dailogX, dialogY + dialogH/2+10, 0, dialogW);
				}
				else if (g_dwMachineWarningFlag)
				{
					if (g_dwMachineWarningFlag&WARNING_OUTSIDE_TEMP_HIGH)
						pStr=getstr(Str_Warning_OutTempHigh);
					else if (g_dwMachineWarningFlag&WARNING_OUTSIDE_TEMP_LOW)
						pStr=getstr(Str_Warning_OutTempLow);
					else if (g_dwMachineWarningFlag&WARNING_INSIDE_TEMP_HIGH)
						pStr=getstr(Str_Warning_InsideTempHigh);
					else if (g_dwMachineWarningFlag&WARNING_INSIDE_TEMP_LOW)
						pStr=getstr(Str_Warning_InsideTempLow);
					else if (g_dwMachineWarningFlag&WARNING_HUMIDITY)
						pStr=getstr(Str_Warning_Humity);
					else if (g_dwMachineWarningFlag&WARNING_ATMOS_PRESSURE)
						pStr=getstr(Str_Warning_Pressure);
					else if (g_dwMachineWarningFlag&WARNING_ELECTRODE_LESS)
						pStr=getstr(Str_Warning_ElectrodeLess);
					else if (g_dwMachineWarningFlag&WARNING_BATTERY_LOW)
						pStr=getstr(Str_Warning_LowPower);
					else if (g_dwMachineWarningFlag&WARNING_NETSIGNAL_LOW)
						pStr=getstr(Str_Warning_LowNetsignal);
					if (pStr!=NULL)
						Idu_PrintStringCenterNewLine(pWin,pStr, dailogX, dialogY + dialogH/2+10, 0, dialogW);
				}
        }
        else if (dialogType == Dialog_Note_ForgetHirePassword||dialogType == Dialog_Note_ForgetOpenPassword||dialogType == Dialog_Note_ElectrodeEnable_Path\
			||dialogType == Dialog_Note_ElectrodeEnable_PASS||dialogType == Dialog_Note_ElectrodeEnable_FAIL||dialogType == Dialog_ShutdownRemain\
			||dialogType == Dialog_Note_Reinput||dialogType == Dialog_Note_PasswordError||dialogType == Dialog_Note_ChangeBootWordPASS\
			||dialogType == Dialog_Note_ChangeHireWordPASS||dialogType == Dialog_Note_Electrode_Enable_Process)
        {
				dialogW = pWin->wWidth*2/3;//DIALOG_DEFAULT_WIDTH
				dialogH = pWin->wHeight*2/3;
			dailogX=(pWin->wWidth - dialogW)>>1;
			dialogY=(pWin->wHeight - dialogH)>>1;
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,dailogX,dialogY,dialogW,dialogH);
				SetCurrIduFontID(FONT_ID_HeiTi19);
				Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
				Idu_PrintStringCenter(pWin, strDialogTitle, dailogX, dialogY + 5, 0, dialogW);
				Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        }
    }
    else if (dwHashKey == xpgHash("User"))
    {
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,pstSprite->m_wPx, pstSprite->m_wPy,400,440);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_PrintStringCenter(pWin, getstr(Str_YongHuWeiHu), pstSprite->m_wPx, pstSprite->m_wPy + 5, 0, pstSprite->m_wWidth);
    }
    else if (dwHashKey == xpgHash("ToolBox"))
    {
			DrawRoundDialog(pWin,XPG_ROLE_ICON_MASK_0,1,pstSprite->m_wPx, pstSprite->m_wPy,560,300);
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
    DWORD rightStart = 800 - 108;
    DWORD rightX = rightStart;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("Main"))
    {
        Idu_PaintWinArea(pWin, 0, 0, 800, 40, RGB2YUV(0,0,0));
        
    }

    if (1)          // auto fusion
    {
        xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_STATUS_ICON_AUTOFUSION], rightX-24, 8, pstSprite, boClip);
        rightX -= 30;
    }
	if (g_psSetupMenu->bCloudMode)          // cloud connect
	{
		xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_STATUS_ICON_CLOUD_MODE], rightX-24, 8, pstSprite, boClip);
		rightX -= 30;
	}
	else            // cloud disconnect
	{
		xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_STATUS_ICON_CLOUD_OFF], rightX-24, 8, pstSprite, boClip);
		rightX -= 30;
	}
    if (g_psSetupMenu->bAutoShutdown)
    {
        xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_STATUS_ICON_AUTO_OFF], rightX-24, 8, pstSprite, boClip);
        rightX -= 30;
    }
	/*
    if (g_psSetupMenu->bSmartBacklight)
    {
        xpgDirectDrawRoleOnWin(pWin, g_pstXpgMovie->m_pstObjRole[XPG_ROLE_STATUS_ICON_SMART_BL], rightX-24, 8, pstSprite, boClip);
        rightX -= 30;
    }
    */
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
            if (g_psSetupMenu->bLowPowerMode)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
            //xpgSpriteEnableTouch(pstSprite);
        }
        else if (dwSpriteId == 1)
        {
            if (g_psSetupMenu->bAutoShutdown)
            {
                pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 2);
                if (pstMask)
                    xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            }
            //xpgSpriteEnableTouch(pstSprite);
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
    else if (dwHashKey == xpgHash("SetPassword"))
    {
        pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
        if (pstMask)
        {
            if (dwSpriteId == 0 && g_psSetupMenu->bEnableOpenPassword)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            if (dwSpriteId == 1 && g_psSetupMenu->bEnableHirePassword)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
        }
        xpgSpriteEnableTouch(pstSprite);
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
        /*
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
        */
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        /*
        if (dwSpriteId == 0)
        {
            STXPGSPRITE *pstRound;
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 0);
            if (pstMask)
                xpgRoleDrawMask(pstSprite->m_pstRole, pWin->pdwStart, pstSprite->m_wPx, pstSprite->m_wPy, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
            
            pstMask = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 1);
            pstRound = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_SCROLL, 1); 
            if (pstMask && pstRound){

                DWORD xadd =  (g_psSetupMenu->bVolume)* (pstSprite->m_wWidth-70) / 15;
                DWORD xpoint = pstSprite->m_wPx + 20 + xadd;
                
                xpgRoleDrawMask(pstRound->m_pstRole, pWin->pdwStart, xpoint, pstSprite->m_wPy + 6, pWin->wWidth, pWin->wHeight, pstMask->m_pstRole);
                Idu_PaintWinArea(pWin, pstSprite->m_wPx + 22, pstSprite->m_wPy + 11, xadd, 2, RGB2YUV(0x14, 0xb6, 0xff));
            }
            
            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx-20, pstSprite->m_wPy-20, pstSprite->m_wWidth+40, pstSprite->m_wHeight+40);
        }
        */
    }
    return PASS;
}

#define VALUE_TYPE_TEMPERATURE      1
#define VALUE_TYPE_PERCENT          2
#define VALUE_TYPE_PRESSURE         3
#define VALUE_TYPE_TIMES            4

static void showMainPageValue(int type, ST_IMGWIN * pWin, DWORD X, DWORD Y, int intValue, unsigned int fraction, BOOL isNeedShowRed)
{
    DWORD BIG_NUM_START;
    DWORD SMALL_NUM_START;
    DWORD COLOR;
    DWORD MINUS_SIGN_INDEX;
    STXPGSPRITE * pstSprite;
    BYTE strvalue[64];
    int i = 0;
    DWORD offset = 0;

    if (isNeedShowRed)
    {
        BIG_NUM_START = 10;
        SMALL_NUM_START = 30;
        COLOR = RGB2YUV(255, 0, 0);
        MINUS_SIGN_INDEX = 41;
    }
    else
    {
        BIG_NUM_START = 0;
        SMALL_NUM_START = 20;
        COLOR = IDU_FONT_YUVCOLOR_DEFAULT_WHITE;
        MINUS_SIGN_INDEX = 40;
    }
    if (intValue < 0)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, MINUS_SIGN_INDEX);
        if (pstSprite)
        {
            xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, X, Y, pstSprite, 0);
	        offset += 24;
        }
    }

    sprintf(strvalue, "%d", (intValue >= 0) ? intValue : (-intValue));
    while (strvalue[i]) 
    {
        BYTE numindex = strvalue[i] - '0';
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, BIG_NUM_START + numindex);
        if (pstSprite)
            xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, X + offset, Y, pstSprite, 0);
        offset += 28;
        i++;
    }

    // 小数部分
    if (type == VALUE_TYPE_TEMPERATURE)
    {
        // point
        Idu_PaintWinArea(pWin, X + offset, Y + 34, 6, 6, COLOR);
        offset += 10;

        // fraction
        sprintf(strvalue, "%d", fraction);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, SMALL_NUM_START + (strvalue[0] - '0'));
        if (pstSprite)
            xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, X + offset, Y + 20, pstSprite, 0);
        offset += 16;
    }
    
    // 单位
    if (type == VALUE_TYPE_TEMPERATURE)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 42);
        if (pstSprite)
            xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, X + offset, Y + 20, pstSprite, 0);
    }
    else if (type == VALUE_TYPE_PERCENT)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 43);
        if (pstSprite)
            xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, X + offset, Y + 20, pstSprite, 0);
    }
    else if (type == VALUE_TYPE_PRESSURE)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_MASK, 44);
        if (pstSprite)
            xpgDirectDrawRoleOnWin(pWin, pstSprite->m_pstRole, X + offset, Y + 20, pstSprite, 0);
    }
    else if (type == VALUE_TYPE_TIMES)
    {
        BYTE * text;
        text = getstr(Str_Ci);
        SetCurrIduFontID(FONT_ID_HeiTi19);
        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
        Idu_PrintString(pWin, text, X + offset, Y + 20, 0, 0);
    }
    return;
}

SWORD xpgDrawSprite_HomeStatus(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
	WORD wData;
	int iData;
    
    if (dwHashKey == xpgHash("Main"))
    {
        xpgDrawSprite(pWin, pstSprite, boClip);
        ///// 温度 value 1
		if (g_psUnsaveParam->bTemperatureInhome[0]&BIT7)
			iData=-(g_psUnsaveParam->bTemperatureInhome[0]&0x3f);
		else
			iData=(g_psUnsaveParam->bTemperatureInhome[0]&0x3f);
        showMainPageValue(VALUE_TYPE_TEMPERATURE, pWin, 255, 88, iData, g_psUnsaveParam->bTemperatureInhome[1], (iData<0 || iData>60));
        ///// 温度 value 2
		if (g_psUnsaveParam->bTemperatureOuthome[0]&BIT7)
			iData=-(g_psUnsaveParam->bTemperatureOuthome[0]&0x3f);
		else
			iData=(g_psUnsaveParam->bTemperatureOuthome[0]&0x3f);
        showMainPageValue(VALUE_TYPE_TEMPERATURE, pWin, 454, 88, iData, g_psUnsaveParam->bTemperatureOuthome[1], (iData<-20 || iData>60));
        ////  
        showMainPageValue(VALUE_TYPE_PERCENT, pWin, 270, 178, g_psUnsaveParam->bHumidity, 0, g_psUnsaveParam->bHumidity>60);
        ////  
        showMainPageValue(VALUE_TYPE_PRESSURE, pWin, 453, 178, g_psUnsaveParam->wPressure, 0, (g_psUnsaveParam->wPressure<50) ||(g_psUnsaveParam->wPressure>200));
        ////  
        showMainPageValue(VALUE_TYPE_TIMES, pWin, 240, 270, g_psSetupMenu->wElectrodeRemainTimes, 0, g_psSetupMenu->wElectrodeRemainTimes<5);
        ////  
        showMainPageValue(VALUE_TYPE_TIMES, pWin, 450, 270, g_psSetupMenu->dwWorkTotalTimes, 0, FALSE);
    }
    return PASS;
}
// for osd bar
SWORD xpgDrawSprite_Frame(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
    STXPGSPRITE *pstMask = NULL;
    DWORD dwSpriteId = pstSprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;


	//(dwHashKey == xpgHash("Auto_work"))
	switch (pstSprite->m_dwTypeIndex)
	{
		case 0:
			Idu_OsdPaintArea(0,0, 24, pWin->wHeight, OSD_COLOR_BLACK);
			break;

		case 1:
			Idu_OsdPaintArea(776,0, 24, pWin->wHeight, OSD_COLOR_BLACK);
			break;

		case 2:
			Idu_OsdPaintArea(pstSprite->m_wPx,pstSprite->m_wPy, pWin->wWidth-pstSprite->m_wPx*2, 30, OSD_COLOR_BLUE);
			break;

		case 3:
			Idu_OsdPaintArea(pstSprite->m_wPx,pstSprite->m_wPy, pWin->wWidth-pstSprite->m_wPx*2, 30, OSD_COLOR_BLUE);
			break;

		default:
			break;
	}

#if 0
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
#endif
    return PASS;
}

SWORD xpgDrawSprite_TextColorBar(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip)
{
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey,dwYUVcolor=0;
	BYTE bString[64],*pStr=&bString[0];

	//mpDebugPrint("xpgDrawSprite_TextColorBar w=%d h=%d",pstSprite->m_wWidth,pstSprite->m_wHeight);
	SetCurrIduFontID(FONT_ID_HeiTi19);
	if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
	{
		int dialogType = xpgGetCurrDialogTypeId();

		pstSprite->m_wPx = 0;
		pstSprite->m_wPy = 310;
		pstSprite->m_wWidth = 180;
		pstSprite->m_wHeight = 50;
		dwYUVcolor=0x00008080;
		if (dialogType==Dialog_YESNO_Upgrade||dialogType == Dialog_Note_ClearAll)
		{
			if (pstSprite->m_dwTypeIndex == 0)
			{
				pstSprite->m_wPx = 194;
				pStr=getstr(Str_Yes);
			}
			else if (pstSprite->m_dwTypeIndex == 1)
			{
				pstSprite->m_wPx = 426;
				pStr=getstr(Str_No);
			}
			if (dwDialogValue.dwValueData==pstSprite->m_dwTypeIndex)
				dwYUVcolor=RGB2YUV(0x37,0x9d,0xff);
		}
  		xpgSpriteEnableTouch(pstSprite);
	}
	else if (dwHashKey == xpgHash("Upgrade"))
    {
        if (pstSprite->m_dwTypeIndex == 0)
        {
	        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
            sprintf(bString, "%sV%d.%d.%d.%d", getstr(Str_Cur_Version),PRODUCT_MAIN_VER,PRODUCT_SUB_VER,g_psUnsaveParam->wMcuCurVersion>>8,g_psUnsaveParam->wMcuCurVersion&0xff);
        }
        else if (pstSprite->m_dwTypeIndex == 1)
        {
			switch (g_psUnsaveParam->bDetectNewVersion)
			{
				case 0:
					pStr=getstr(Str_PressToGetNewVersion);
					dwYUVcolor=RGB2YUV(0x37,0x9d,0xff);
					break;

				case 1:
					pStr=getstr(Str_PollingVersion);
					dwYUVcolor=RGB2YUV(0x53,0x54,0x56);
					break;

				case 2:
					pStr=getstr(Str_NeednotUpgrade);
					dwYUVcolor=RGB2YUV(0x53,0x54,0x56);
					break;

				default:
					dwYUVcolor=RGB2YUV(0x37,0x9d,0xff);
					sprintf(bString, "%s(V%d.%d.%d.%d),%s",getstr(Str_HaveNewVersion),g_psUnsaveParam->wCpuNewVersion>>8,g_psUnsaveParam->wCpuNewVersion&0xff,g_psUnsaveParam->wMcuNewVersion>>8,g_psUnsaveParam->wMcuNewVersion&0xff,getstr(Str_HaveNewVersion));
					break;
			}
        }
        
    }
    else if (dwHashKey == xpgHash("Record") )
    {
			if (pstSprite->m_dwTypeIndex==0)
				dwYUVcolor=RGB2YUV(0xff,0,0);
			else if (pstSprite->m_dwTypeIndex>3)
			{
				if (boClip||(pstSprite->m_dwTypeIndex==4 && !g_WeldRecordPage.wCurPage)||(pstSprite->m_dwTypeIndex==7&& g_WeldRecordPage.wCurPage+1==g_WeldRecordPage.wTotalPage))
				{
					dwYUVcolor=RGB2YUV(0x37,0x9d,0xff);
					Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
				}
				else
			        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_BLACK);
			}
			else if ( (pstSprite->m_dwTypeIndex == 1&&g_WeldRecordPage.bMode==3)||(pstSprite->m_dwTypeIndex == 2&&g_WeldRecordPage.bMode==7)||(pstSprite->m_dwTypeIndex == 3&&g_WeldRecordPage.bMode==0) )
				dwYUVcolor=RGB2YUV(0x37,0x9d,0xff);
			else
				dwYUVcolor=0x00008080;
			if (pstSprite->m_dwTypeIndex == 0)
                pStr = getstr(Str_QingKong);
			else if (pstSprite->m_dwTypeIndex == 1)
                pStr = getstr(Str_Near3Days);
            else if (pstSprite->m_dwTypeIndex == 2)
                pStr = getstr(Str_NearOneWeak);
            else if (pstSprite->m_dwTypeIndex == 3)
                pStr = getstr(Str_AllRecord);
            else if (pstSprite->m_dwTypeIndex == 4)
                pStr = getstr(Str_ShouYe);
            else if (pstSprite->m_dwTypeIndex == 5)
                pStr = getstr(Str_PrevPage);
            else if (pstSprite->m_dwTypeIndex == 6)
                pStr = getstr(Str_NextPage);
            else if (pstSprite->m_dwTypeIndex == 7)
                pStr = getstr(Str_WeiYe);
            SetCurrIduFontID(FONT_ID_HeiTi16);
    }

		if (dwYUVcolor)
			DrawRoundIcon(pWin,XPG_ROLE_MASK_70x25,pstSprite->m_wPx,pstSprite->m_wPy,pstSprite->m_wWidth,pstSprite->m_wHeight,dwYUVcolor);
		if (pStr)
		{
	        Idu_PrintStringCenterWH(pWin, pStr, pstSprite->m_wPx, pstSprite->m_wPy, 0, pstSprite->m_wWidth,pstSprite->m_wHeight);
	        Idu_SetFontYUV(IDU_FONT_YUVCOLOR_DEFAULT_WHITE);
		}
    
    return PASS;
}


#endif


#endif





