#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "ui.h"
#include "display.h"
#include "xpg.h"
#include "Setup.h"
#include "xpgFunc.h"
#include "xpgDrawSprite.h"
#include "uiTouchCtrller.h"

#if (TOUCH_CONTROLLER_ENABLE == ENABLE)

DWORD dwLastTouchActionTime = 0;

static void uiEnterRecordList();
DWORD g_dwRecordListCurrPage = 0;
static BYTE strSetupBackPageName[24] = {0};

static void Dialog_JiaReWenDu_OnClose();
static void Dialog_JiaReShiJian_OnClose();

/*
// Structure declarations
*/


/*
// Constant declarations
*/

////////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////////

SDWORD TouchCtrllerCalProcess(WORD cross_width, WORD cross_single_width)
{
    while(1)
    {
        SDWORD val;
        {
            xpgSearchAndGotoPage("touch");
            xpgUpdateStage();
        }

        MP_ALERT("Adjust start");
        mpx_TcFunctionsAdjust(cross_width, cross_single_width);
        if ( mpx_TcFunctionsAdjustWait(1000*10) != NO_ERR )
        {
            MP_ALERT("Adjust Fail");
            continue;
        }
        MP_ALERT("Adjust 2");
        if ( mpx_TcFunctionsAdjustWait(1000*10) != NO_ERR )
        {
            MP_ALERT("Adjust Fail");
            continue;
        }
        MP_ALERT("Adjust 3");
        if ((val = mpx_TcFunctionsAdjustWait(1000*10)) != NO_ERR )
        {
            MP_ALERT("Adjust Fail");
            continue;
        }
        else
        {
            break;
        }
    }
    return 0;

}



void TouchCtrllerInit(void)
{

    ST_TC_PARA tcPara;

    SDWORD retVal;
    void* callBack = NULL;

    if(callBack == NULL)
    {
        MessageCreate(UI_TC_MSG_ID, OS_ATTR_FIFO, sizeof(ST_TC_DATA) * 128);
        mpx_TcFunctionsStartup(NULL);
    }
    else
        mpx_TcFunctionsStartup(callBack);
}




///
///@ingroup
///
///@brief
///
///@param
///
///@param
///@retval
///
///Remark
void uiTouchCtrllerCallback(void)
{
    MP_DEBUG("__%s__", __FUNCTION__);

    EventSet(UI_EVENT, EVENT_TOUCH_COLTROLLER);
}



void uiTouchMsgReceiver(void)
{
    ST_TC_DATA stTcData;

    while((MessageReceiveWithTO(UI_TC_MSG_ID, &stTcData, 1)) > 0)
    {
        if (SystemGetElapsedTime(dwLastTouchActionTime) < 500)
        {
            //mpDebugPrint("Skip");
            continue;
        }
        
        MP_ALERT("s = %d, x1 = %d, y1 = %d, x2 = %d, y2 = %d", stTcData.status, stTcData.x1, stTcData.y1, stTcData.x2, stTcData.y2);
        uiDispatchTouchSprite(stTcData.x1, stTcData.y1);
    }
}



SWORD touchSprite_Background(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Icon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    DWORD dwIconId = sprite->m_dwTypeIndex;
    mpDebugPrint("touchSprite_Icon  %d", dwIconId);
    
    if (dwHashKey == xpgHash("Main"))
    {
        if (dwIconId == 0) 
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            xpgSearchAndGotoPage("User");
            xpgUpdateStage();
        }
        else if (dwIconId == 1) 
        {
            uiEnterRecordList();
        }
        else if (dwIconId == 2) 
        {
            strcpy(strSetupBackPageName, "Main");
            xpgSearchAndGotoPage("FusionSet1");
            xpgUpdateStage();
        }
        else if (dwIconId == 3) 
        {
            //xpgSearchAndGotoPage("Auto_work");
            xpgSearchAndGotoPage("Manual_work");
            xpgUpdateStage();
        }
        else if (dwIconId == 4) 
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            xpgSearchAndGotoPage("ToolBox");
            xpgUpdateStage();
        }
        else if (dwIconId == 5) 
        {
            xpgSearchAndGotoPage("SetYun");
            xpgUpdateStage();
        }
        else if (dwIconId == 6) 
        {
            strcpy(strSetupBackPageName, "Main");
            xpgSearchAndGotoPage("FuncSet");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("FuncSet"))
    {
        BYTE * pb;
        if (dwIconId == 0)
            pb = &(g_psSetupMenu->bEnableIcon_LaLiCeShi);
        else if (dwIconId == 1)
            pb = &(g_psSetupMenu->bEnableIcon_DuanMianJianCe);
        else if (dwIconId == 2)
            pb = &(g_psSetupMenu->bEnableIcon_ZiDongDuiJiao);
        else if (dwIconId == 3)
            pb = &(g_psSetupMenu->bEnableIcon_JiaoDuJianCe);
        else if (dwIconId == 4)
            pb = &(g_psSetupMenu->bEnableIcon_BaoCunTuXiang);
        else if (dwIconId == 5)
            pb = &(g_psSetupMenu->bEnableIcon_HuiChenJianCe);
        else if (dwIconId == 6)
            pb = &(g_psSetupMenu->bEnableIcon_RongJieZanTing);
        else
            pb = &(g_psSetupMenu->bEnableIcon_YunDuanCeLiang);
        *pb = !(*pb);
        xpgUpdateStage();
        WriteSetupChg();
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        if (dwIconId < 6)
        {
            if (g_psSetupMenu->bCustomizeIcon[dwIconId] < 0)
            {
                mpDebugPrint("it is empty.");
                return PASS;
            }
            g_psSetupMenu->bCustomizeIcon[dwIconId] = -1;
        }
        else
        {
            DWORD nowIdx = dwIconId - 6;
            BOOL found = FALSE;
            DWORD foundIdx;
            for (foundIdx = 0; foundIdx < 6; foundIdx++) {
                if (g_psSetupMenu->bCustomizeIcon[foundIdx] < 0) {
                    found = TRUE;
                    break;
                }
            }
            if (found == FALSE)
            {
                mpDebugPrint("not found empty.");
                return PASS;
            }

            DWORD j1;
            for (j1 = 0; j1 < 6; j1++) {
                if (g_psSetupMenu->bCustomizeIcon[j1] == (int)nowIdx) {
                    mpDebugPrint("has exist.");
                    return PASS;
                }
            }
            g_psSetupMenu->bCustomizeIcon[foundIdx] = (int)nowIdx;
            g_psSetupMenu->bCustomizeIconEnable[foundIdx] = 1;
            WriteSetupChg();
        }
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("Auto_work"))
    {
        if (dwIconId <= 5)
        {
            if (g_psSetupMenu->bCustomizeIcon[dwIconId] < 0)
            {
                strcpy(strSetupBackPageName, "Auto_work");
                xpgSearchAndGotoPage("FuncSet2");
                xpgUpdateStage();
                return 0;
            }
            g_psSetupMenu->bCustomizeIconEnable[dwIconId] = !g_psSetupMenu->bCustomizeIconEnable[dwIconId];
            xpgUpdateStage();
            WriteSetupChg();
        }
        else if (dwIconId == 6)
        {
            strcpy(strSetupBackPageName, "Auto_work");
            xpgSearchAndGotoPage("FusionSet1");
            xpgUpdateStage();
        }
        else if (dwIconId == 7)
        {
            strcpy(strSetupBackPageName, "Auto_work");
            xpgSearchAndGotoPage("FuncSet2");
            xpgUpdateStage();
        }
        else if (dwIconId == 8)
        {
        }
    }
    else if (dwHashKey == xpgHash("FusionSet3"))
    {
        if (dwIconId == 0)
        {
            if (g_psSetupMenu->bHotUpMode != SETUP_MENU_HOT_UP_MODE_AUTO)
            {
                g_psSetupMenu->bHotUpMode = SETUP_MENU_HOT_UP_MODE_AUTO;
                xpgUpdateStage();
                WriteSetupChg();
            }
        }
        else if (dwIconId == 1)
        {
            if (g_psSetupMenu->bHotUpMode != SETUP_MENU_HOT_UP_MODE_MANUAL)
            {
                g_psSetupMenu->bHotUpMode = SETUP_MENU_HOT_UP_MODE_MANUAL;
                xpgUpdateStage();
                WriteSetupChg();
            }
        }
        else if (dwIconId == 2)
        {
            g_psSetupMenu->bPreHotEnable = !g_psSetupMenu->bPreHotEnable;
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
    else if (dwHashKey == xpgHash("FusionSet2"))
    {
        if (dwIconId == 0 || dwIconId == 1 || dwIconId == 2)
        {
        	g_psSetupMenu->bRongJieZhiLiang = dwIconId;
        	xpgUpdateStage();
        	WriteSetupChg();
        }
        else if (dwIconId == 3 || dwIconId == 4 )
        {
        	g_psSetupMenu->bDuiXianFangShi = dwIconId - 3;
        	xpgUpdateStage();
        	WriteSetupChg();
        }
        else if (dwIconId == 5 || dwIconId == 6 || dwIconId == 7 || dwIconId == 8)
        {
        	g_psSetupMenu->bPingXianFangShi = dwIconId - 5;
        	xpgUpdateStage();
        	WriteSetupChg();
        }
    }
    else if (dwHashKey == xpgHash("FusionSet1"))
    {
        if (dwIconId < 8)
        {
            g_psSetupMenu->bCurrFusionMode = dwIconId;
            xpgUpdateStage();
        	WriteSetupChg();
        }
        else if (dwIconId == 8)
        {
            MODEPARAM * pstModeParam;
            int mode;
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
        
            memcpy(&tempModeParam, pstModeParam, sizeof(MODEPARAM));
            xpgSearchAndGotoPage("FusionModeSet");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("FusionModeSet"))
    {
        if (dwIconId <= 10)
        {
        }
        else if (dwIconId == 11)
        {
            initModeParamDefault(&tempModeParam);
            xpgUpdateStage();
        }
        else if (dwIconId == 12)
        {
            MODEPARAM * pstModeParam;
            int mode;
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
            memcpy(pstModeParam, &tempModeParam, sizeof(MODEPARAM));
            WriteSetupChg();
        }
        else if (dwIconId == 13)
        {
        }
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        if (dwIconId != g_psSetupMenu->bLanguage)
        {
            g_psSetupMenu->bLanguage = dwIconId;
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
    else if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        if (dialogType == Dialog_ReSuGuan)
        {
            dwDialogTempValue = dwIconId;
            xpgUpdateStage();
        }
        else if (dialogType == Dialog_ModifyNumber)
        {
            if (dwIconId == 0)
                dwDialogTempValue ++;
            else if (dwIconId == 1)
            {
                if (dwDialogTempValue)
                    dwDialogTempValue --;
            }
            else
                return 0;
            xpgUpdateStage();
        }
        else if (dialogType == Dialog_ShutdownTime)
        {
            if (dwIconId == 0)
                dwDialogTempValue += 10;
            else if (dwIconId == 1)
            {
                if (dwDialogTempValue >= 20)
                    dwDialogTempValue -= 10;
            }
            else
                return 0;
            xpgUpdateStage();
        }
        else if (dialogType == Dialog_SetTime)
        {
            WORD hour2 = dwDialogTempValue >> 16;
            WORD minute2 = dwDialogTempValue & 0xffff;
            if (dwIconId == 0)
            {
                if (hour2 != 0)
                {
                    hour2 --;
                    dwDialogTempValue = (hour2 << 16) | minute2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 1)
            {
                if (hour2 < 23)
                {
                    hour2 ++;
                    dwDialogTempValue = (hour2 << 16) | minute2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 2)
            {
                if (minute2 != 0)
                {
                    minute2 --;
                    dwDialogTempValue = (hour2 << 16) | minute2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 3)
            {
                if (minute2 < 59)
                {
                    minute2 ++;
                    dwDialogTempValue = (hour2 << 16) | minute2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 4)
            {
                if (hour2 >= 12)
                {
                    hour2 -= 12;
                    dwDialogTempValue = (hour2 << 16) | minute2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 5)
            {
                if (hour2 < 12)
                {
                    hour2 += 12;
                    dwDialogTempValue = (hour2 << 16) | minute2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 6)
            {
                ST_SYSTEM_TIME curTime;
                SystemTimeGet(&curTime);
                curTime.u08Hour = dwDialogTempValue >> 16;
                curTime.u08Minute = dwDialogTempValue & 0xffff;
                SystemTimeSet(&curTime);
                exitDialog();
            }
            else if (dwIconId == 7)
            {
                exitDialog();
            }
        }

        
    }
    
    return 0;
}


SWORD touchSprite_LightIcon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_DarkIcon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Mask(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Title(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Aside(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_BackIcon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    mpDebugPrint("touchSprite_BackIcon  %d", sprite->m_dwTypeIndex);
    
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("SetYun") ||
        dwHashKey == xpgHash("SetSleep") ||
        dwHashKey == xpgHash("SetSound") ||
        dwHashKey == xpgHash("SetTime") ||
        dwHashKey == xpgHash("SetPassword") ||
        dwHashKey == xpgHash("SetUi") ||
        dwHashKey == xpgHash("SetInfo") ||
        dwHashKey == xpgHash("Record"))
    {
        xpgSearchAndGotoPage("Main");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("FuncSet") || 
        dwHashKey == xpgHash("FuncSet2") ||
        dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3") )
    {
        xpgSearchAndGotoPage("Main");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("FusionModeSet"))
    {
        xpgSearchAndGotoPage("FusionSet1");
        xpgUpdateStage();
    }
    
    return 0;
}

SWORD touchSprite_CloseIcon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    BOOL boNeedWriteSetup = FALSE;
    mpDebugPrint("touchSprite_CloseIcon  %d", sprite->m_dwTypeIndex);
    
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("User") || 
        dwHashKey == xpgHash("ToolBox"))
    {
        xpgSearchAndGotoPage("Main");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
        if (dialogType == Dialog_ReSuGuan)
        {
            if (g_psSetupMenu->bReSuGuanSheZhi != dwDialogTempValue)
            {
                g_psSetupMenu->bReSuGuanSheZhi = dwDialogTempValue;
                boNeedWriteSetup = TRUE;
            }
            exitDialog();
            if (boNeedWriteSetup)
                WriteSetupChg();
        }
        else if (dialogType == Dialog_ModifyNumber)
        {
            if (dialogOnClose != NULL)
            {
                dialogOnClose();
            }
        }
        else if (dialogType == Dialog_ShutdownTime)
        {
            if (g_psSetupMenu->wShutdownTime != dwDialogTempValue)
            {
                g_psSetupMenu->wShutdownTime = dwDialogTempValue;
                boNeedWriteSetup = TRUE;
            }
            exitDialog();
            if (boNeedWriteSetup)
                WriteSetupChg();
        }
        else if (dialogType == Dialog_SetTime)
        {
            exitDialog();
        }
        
    }
    return 0;
}

void (*dialogOnClose)() = NULL;

static void Dialog_JiaReWenDu_OnClose()
{
    mpDebugPrint("%s()", __FUNCTION__);
    g_psSetupMenu->wJiaReWenDu = dwDialogTempValue;
    exitDialog();
    WriteSetupChg();
}

static void Dialog_JiaReShiJian_OnClose()
{
    mpDebugPrint("%s()", __FUNCTION__);
    g_psSetupMenu->wJiaReShiJian = dwDialogTempValue;
    exitDialog();
    WriteSetupChg();
}


SWORD touchSprite_Text(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Selector(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwSelectorId = sprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    mpDebugPrint("touchSprite_Selector  %d", sprite->m_dwTypeIndex);
    
    if (dwHashKey == xpgHash("FuncSet"))
    {
        xpgSearchAndGotoPage("FuncSet2");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        xpgSearchAndGotoPage("FuncSet");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("SetYun") ||
        dwHashKey == xpgHash("SetSleep") ||
        dwHashKey == xpgHash("SetSound") ||
        dwHashKey == xpgHash("SetTime") ||
        dwHashKey == xpgHash("SetPassword") ||
        dwHashKey == xpgHash("SetUi") ||
        dwHashKey == xpgHash("SetInfo"))
    {
        if (dwSelectorId == 0)
        {
            xpgSearchAndGotoPage("SetYun");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 1)
        {
            xpgSearchAndGotoPage("SetSleep");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 2)
        {
            xpgSearchAndGotoPage("SetSound");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 3)
        {
            xpgSearchAndGotoPage("SetTime");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 4)
        {
            xpgSearchAndGotoPage("SetPassword");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 5)
        {
            xpgSearchAndGotoPage("SetUi");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 6)
        {
            xpgSearchAndGotoPage("SetInfo");
            xpgUpdateStage();
        }
        
    }
    else if (dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3")  )
    {
        if (dwSelectorId == 0)
        {
            xpgSearchAndGotoPage("FusionSet1");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 1)
        {
            xpgSearchAndGotoPage("FusionSet2");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 2)
        {
            xpgSearchAndGotoPage("FusionSet3");
            xpgUpdateStage();
        }
    }
    
    return 0;
}

SWORD touchSprite_List(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwSpriteId = sprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    
    mpDebugPrint("touchSprite_List  %d", sprite->m_dwTypeIndex);
    
    if (dwHashKey == xpgHash("FusionSet3"))
    {
        if (dwSpriteId == 2)
        {
            popupDialog(Dialog_ReSuGuan, "FusionSet3");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 3)
        {
            dwDialogTempValue = g_psSetupMenu->wJiaReWenDu;
            dialogOnClose = Dialog_JiaReWenDu_OnClose;
            popupDialog(Dialog_ModifyNumber, "FusionSet3");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 4)
        {
            dwDialogTempValue = g_psSetupMenu->wJiaReShiJian;
            dialogOnClose = Dialog_JiaReShiJian_OnClose;
            popupDialog(Dialog_ModifyNumber, "FusionSet3");
            xpgUpdateStage();
        }
        else
            return 0;
    }
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        if (dwSpriteId == 3)
        {
            dwDialogTempValue = g_psSetupMenu->wShutdownTime;
            popupDialog(Dialog_ShutdownTime, "SetSleep");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        if (dwSpriteId == 1)
        {
            ST_SYSTEM_TIME curTime;
            SystemTimeGet(&curTime);
            dwDialogTempValue = (curTime.u08Hour<<16) | curTime.u08Minute;
            popupDialog(Dialog_SetTime, "SetTime");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 2)
        {
            //popupDialog(Dialog_SetDate, "SetTime");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 3)
        {
            //popupDialog(Dialog_SetDateFormat, "SetTime");
            xpgUpdateStage();
        }
    }
    
    return 0;
}

SWORD touchSprite_Dialog(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Status(STXPGSPRITE * sprite, WORD x, WORD y)
{
    return 0;
}

SWORD touchSprite_Radio(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwSpriteId = sprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("SetYun"))
    {
        if (dwSpriteId == 0)
        {
            g_psSetupMenu->bCloudMode = !g_psSetupMenu->bCloudMode;
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
    else if (dwHashKey == xpgHash("SetSleep"))
    {
        if (dwSpriteId == 0)
        {
            g_psSetupMenu->bSmartBacklight = !g_psSetupMenu->bSmartBacklight;
            xpgUpdateStage();
            WriteSetupChg();
        }
        else if (dwSpriteId == 1)
        {
            g_psSetupMenu->bAutoShutdown = !g_psSetupMenu->bAutoShutdown;
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        if (dwSpriteId == 0)
        {
            g_psSetupMenu->bToundSoundEnable = !g_psSetupMenu->bToundSoundEnable;
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        if (dwSpriteId == 0)
        {
            g_psSetupMenu->b24HourFormat = ! g_psSetupMenu->b24HourFormat;
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
    
    return 0;
}

SWORD touchSprite_Scroll(STXPGSPRITE * sprite, WORD x, WORD y)
{
    //mpDebugPrint("touchSprite_Scroll  %d, %d", x, y);
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    int x1 = x - sprite->m_wPx;
    int y1 = y - sprite->m_wPy;

    if (dwHashKey == xpgHash("SetSleep"))
    {
        if (x1 >= 0 && y1 >= 0)
        {
            // scroll left_x = 32, scroll right_x = 282
            if (x1 <= 32)
                g_psSetupMenu->bBrightness = 0;
            else if (x1 >= 282)
                g_psSetupMenu->bBrightness = 100;
            else {
                // 282 - 32 = 250
                g_psSetupMenu->bBrightness = (x1-32)*100/250;
            }
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        if (x1 >= 0 && y1 >= 0)
        {
            // scroll left_x = 32, scroll right_x = 282
            if (x1 <= 32)
                g_psSetupMenu->bVolume = 0;
            else if (x1 >= 282)
                g_psSetupMenu->bVolume = 100;
            else {
                // 282 - 32 = 250
                g_psSetupMenu->bVolume = (x1-32)*100/250;
            }
            xpgUpdateStage();
        }
    }
}

static void uiEnterRecordList()
{
    g_dwRecordListCurrPage = 0;
    xpgSearchAndGotoPage("Record");
    xpgUpdateStage();
}


typedef SWORD(*touchFunction)(STXPGSPRITE *, WORD, WORD);
typedef struct  {
    DWORD           touchFlag;
    touchFunction   touchFunc;
}TouchSpriteFunction;

#define ENABLE_SLIDE        0x01

TouchSpriteFunction touchSpriteFunctions[] = {
    {0,             NULL},                          
    {0,             NULL},                           // type1 
    {0,             NULL},                           // type2
    {0,             NULL},                           // type3
    {0,             NULL},                           // type4
    {0,             NULL},                           // type5
    {0,             NULL},                           // type6
    {0,             NULL},                           // type7
    {0,             NULL},                           // type8
    {0,             NULL},                           // type9
    {0,             NULL},                           // type10
    {0,             touchSprite_Background},         // type11
    {0,             touchSprite_Icon},               // type12
    {0,             touchSprite_LightIcon},          // type13
    {0,             touchSprite_DarkIcon},           // type14
    {0,             touchSprite_Mask},               // type15
    {0,             touchSprite_Title},              // type16
    {0,             touchSprite_Aside},              // type17
    {0,             touchSprite_BackIcon},           // type18
    {0,             touchSprite_CloseIcon},          // type19
    {0,             touchSprite_Text},               // type20
    {0,             touchSprite_Selector},           // type21
    {0,             touchSprite_List},               // type22
    {0,             touchSprite_Dialog},             // type23
    {0,             touchSprite_Status},             // type24
    {0,             touchSprite_Radio},              // type25
    {ENABLE_SLIDE,  touchSprite_Scroll},             // type26
};


void uiDispatchTouchSprite(WORD x1, WORD y1)
{
    int idx;
    STXPGMOVIE *pstMovie = &g_stXpgMovie;
    STXPGSPRITE *pstSprite = NULL;
    STXPGTOUCHINFO * pstTouchInfo = NULL;
    BOOL match;

    if (pstMovie->m_dwSpriteCount == 0)
        return;

    match = FALSE;
    for (idx = pstMovie->m_dwSpriteCount - 1; idx >= 0; idx--)
    {
        pstSprite = &(pstMovie->m_astSprite[idx]);
        pstTouchInfo = &(pstSprite->m_touchInfo);
        //mpDebugPrint("%d, %d, %d, %d, %d, %d, %d", x1, y1,
        //    pstTouchInfo->m_boEnable, pstTouchInfo->m_touchAreaX, pstTouchInfo->m_touchAreaY, 
        //    pstTouchInfo->m_touchAreaX + pstTouchInfo->m_touchAreaW, pstTouchInfo->m_touchAreaY + pstTouchInfo->m_touchAreaH);

        if (pstTouchInfo->m_boEnable && pstSprite->m_boVisible) 
        {
            if (x1 >= pstTouchInfo->m_touchAreaX && y1 >= pstTouchInfo->m_touchAreaY &&
                x1 < (pstTouchInfo->m_touchAreaX + pstTouchInfo->m_touchAreaW) && 
                y1 < (pstTouchInfo->m_touchAreaY + pstTouchInfo->m_touchAreaH))
            {
                match = TRUE;
                break;
            }
        }
    }

    if (match)
    {
        //mpDebugPrint("match");
        if (pstSprite->m_dwType < sizeof(touchSpriteFunctions)/sizeof(TouchSpriteFunction))
        {
            TouchSpriteFunction * pfunc;
            pfunc = & (touchSpriteFunctions[pstSprite->m_dwType]);
                
            if (pfunc->touchFunc != NULL) 
            {
                if (!(pfunc->touchFlag& ENABLE_SLIDE))
                    dwLastTouchActionTime = GetSysTime();
                pfunc->touchFunc (pstSprite, x1, y1);
            }
        }
    }
}


#endif  //#if (TOUCH_CONTROLLER_ENABLE == ENABLE)










