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
#include "xpgString.h"
#include "xpgProcSensorData.h"

#if (TOUCH_CONTROLLER_ENABLE == ENABLE)

#if TOUCH_DIRECT_TO_GUI
static ST_TC_DRIVER st_TcDriver;
#endif
DWORD dwLastTouchActionTime = 0;
static BYTE st_bTouchDown=0;// 0->touch release 1->Touch on   

static void uiEnterRecordList();
DWORD g_dwRecordListCurrPage = 0;
static BYTE st_bGoSetupFrom=0;// 0->from main   1->for work pgage    strSetupBackPageName[24] = {0};

static void Dialog_JiaReWenDu_OnClose();
static void Dialog_JiaReShiJian_OnClose();
static void Dialog__OnClose();
static void Dialog_CheckPassword_CloseBootPassword_OnInput();       // 关闭开机密码之前的检查密码
static void Dialog_CheckPassword_ChangeBootPassword_OnInput();      // 更改开机密码之前的检查密码
static void Dialog_CheckPassword_CloseHirePassword_OnInput();       // 关闭租借密码之前的检查密码
static void Dialog_CheckPassword_ChangeHirePassword_OnInput();      // 更改租借密码之前的检查密码
static void Dialog_SetPassword_BootPassword_OnInput();              // 设置开机密码
static void Dialog_SetPassword_HirePassword_OnInput();              // 更改开机密码
static void Dialog_SetValue_SuoDingRongJieCiShu();                  // 值更改 - 锁定熔接次数
static void Dialog_SetValue_RongJieSheZhi();                        // 值更改 - 熔接设置里的多项值

static void keyboardAddChar(char c);                                // 键盘输入一个



#if TOUCH_DIRECT_TO_GUI
void TouchCtrllerInit(void)
{
	MP_DEBUG("__%s__", __FUNCTION__);
	TcDriverInit(&st_TcDriver);

	if(st_TcDriver.TcInit())
	{
		MP_ALERT("Touch controller init fail");
		return;
	}
}

void uiTouchMsgReceiver(void)
{
    Tc_point  data;

	if ( st_TcDriver.TcGetData(&data) == NO_ERR )
	{
		if (data.reserved==TC_DOWN)
		{
			uiDispatchTouchSprite(data.x1, data.y1);
			st_bTouchDown=1;
		}
		else //if (data.reserved==TC_UP)
		{
			st_bTouchDown=0;
		}
	}

}

#else

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

void uiTouchMsgReceiver(void)
{
    ST_TC_DATA stTcData;

    while((MessageReceiveWithTO(UI_TC_MSG_ID, &stTcData, 1)) > 0)
    {

        if (SystemGetElapsedTime(dwLastTouchActionTime) > 200)
        {
        //MP_ALERT("s = %d, x1 = %d, y1 = %d, x2 = %d, y2 = %d", stTcData.status, stTcData.x1, stTcData.y1, stTcData.x2, stTcData.y2);
        MP_ALERT("%d, %d",  stTcData.x1, stTcData.y1);
#if  (PRODUCT_UI==UI_WELDING)
        uiDispatchTouchSprite(stTcData.x1, stTcData.y1);
#endif
        }
    }
}
#endif

#if  (PRODUCT_UI==UI_WELDING)
SWORD touchSprite_Background(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    
    if (dwHashKey == xpgHash("opmWarn"))
    {
        Free_CacheWin();
        isSelectOnlineOPM = 1;
        xpgPreactionAndGotoPage("opm2");
        xpgUpdateStage();
    }
    return 0;
}

extern BYTE g_bDisplayMode;
SWORD touchSprite_Icon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    BOOL boNeedWriteSetup = FALSE;
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
            xpgPreactionAndGotoPage("User");
            xpgUpdateStage();
        }
        else if (dwIconId == 1) 
        {
            uiEnterRecordList();
        }
        else if (dwIconId == 2) 
        {
			st_bGoSetupFrom=0;
            xpgPreactionAndGotoPage("FusionSet1");
            xpgUpdateStage();
        }
        else if (dwIconId == 3) 
        {
            //xpgPreactionAndGotoPage("Auto_work");
            //xpgPreactionAndGotoPage("Manual_work");
           // xpgUpdateStage();
           WeldModeSet(1);
			xpgCb_EnterCamcoderPreview();
        }
        else if (dwIconId == 4) 
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            xpgPreactionAndGotoPage("ToolBox");
            xpgUpdateStage();
        }
        else if (dwIconId == 5) 
        {
            xpgPreactionAndGotoPage("SetYun");
            xpgUpdateStage();
        }
        else if (dwIconId == 6) 
        {
			st_bGoSetupFrom=0;
            xpgPreactionAndGotoPage("FuncSet");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("FuncSet"))
    {
#if 0
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
#endif
		g_psSetupMenu->bCustomizeIconEnable[dwIconId] = !g_psSetupMenu->bCustomizeIconEnable[dwIconId];
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
            char nowIdx = dwIconId - 6;
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
                if (g_psSetupMenu->bCustomizeIcon[j1] == nowIdx) {
                    mpDebugPrint("has exist.");
                    return PASS;
                }
            }
            g_psSetupMenu->bCustomizeIcon[foundIdx] = nowIdx;
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
					st_bGoSetupFrom=1;
                xpgPreactionAndGotoPage("FuncSet2");
                xpgUpdateStage();
                return 0;
            }
            g_psSetupMenu->bCustomizeIconEnable[dwIconId] = !g_psSetupMenu->bCustomizeIconEnable[dwIconId];
            //xpgUpdateStage();
            xpgSpriteRedraw(Idu_GetCurrWin(),SPRITE_TYPE_LIGHT_ICON, dwIconId);
            xpgSpriteRedraw(Idu_GetCurrWin(),SPRITE_TYPE_DARK_ICON,  dwIconId);
            WriteSetupChg();
        }
        else if (dwIconId == 6)
        {
			st_bGoSetupFrom=1;
            xpgPreactionAndGotoPage("FusionSet1");
            xpgUpdateStage();
        }
        else if (dwIconId == 7)
        {
			st_bGoSetupFrom=1;
            xpgPreactionAndGotoPage("FuncSet2");
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
			g_bDisplayMode=0x80|g_psSetupMenu->bPingXianFangShi;
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
            xpgPreactionAndGotoPage("FusionModeSet");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("FusionModeSet"))
    {
        if (dwIconId <= 10)
        {
            if (dwIconId == 0)
            {
                pdwEditingFusionValue = & tempModeParam.fangDianZhongXin;
                strDialogTitle = getstr(Str_FangDianZhongXin);
            }
            else if (dwIconId == 1)
            {
                pdwEditingFusionValue = & tempModeParam.rongJieDianYa;
                strDialogTitle = getstr(Str_RongJieDianYa);
            }
            else if (dwIconId == 2)
            {
                pdwEditingFusionValue = & tempModeParam.yuRongDianYa;
                strDialogTitle = getstr(Str_YuRongDianYa);
            }
            else if (dwIconId == 3)
            {
                pdwEditingFusionValue = & tempModeParam.chuChenDianYa;
                strDialogTitle = getstr(Str_ChuChenDianYa);
            }
            else if (dwIconId == 4)
            {
                pdwEditingFusionValue = & tempModeParam.rongJieChongDieLiang;
                strDialogTitle = getstr(Str_RongJieChongDieLiang);
            }
            else if (dwIconId == 5)
            {
                pdwEditingFusionValue = & tempModeParam.duiJiaoMuBiaoZhi;
                strDialogTitle = getstr(Str_DuiJiaoMuBiaoZhi);
            }
            else if (dwIconId == 6)
            {
                pdwEditingFusionValue = & tempModeParam.rongJieShiJian;
                strDialogTitle = getstr(Str_RongJieShiJian);
            }
            else if (dwIconId == 7)
            {
                pdwEditingFusionValue = & tempModeParam.yuRongShiJian;
                strDialogTitle = getstr(Str_YuRongShiJian);
            }
            else if (dwIconId == 8)
            {
                pdwEditingFusionValue = & tempModeParam.chuChenShiJian;
                strDialogTitle = getstr(Str_ChuChenShiJian);
            }
            else if (dwIconId == 9)
            {
                pdwEditingFusionValue = & tempModeParam.qieGeJiaoDuShangXian;
                strDialogTitle = getstr(Str_QieGeJiaoDuShangXian);
            }
            else if (dwIconId == 10)
            {
                pdwEditingFusionValue = & tempModeParam.fangDianJiaoZhengMuBiaoZhi;
                strDialogTitle = getstr(Str_FangDianJiaoZhengMuBiaoZhi);
            }
            
            dialogOnClose = Dialog_SetValue_RongJieSheZhi;
            dwDialogTempValue = *pdwEditingFusionValue;
            if (dwIconId == 9)
                boDialogValueIsFloat = 1;
            else
                boDialogValueIsFloat = 0;
            popupDialog(Dialog_Value, "FusionModeSet");
            xpgUpdateStage();
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
    else if (dwHashKey == xpgHash("ToolBox"))
    {
        if (dwIconId == 0)
        {
			SendCmdA4GetStaus(0x09);
            xpgPreactionAndGotoPage("RedLight");
            xpgUpdateStage();
        }
        else if (dwIconId == 1)
        {
            isSelectOnlineOPM = 0;
            xpgPreactionAndGotoPage("opm1");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("RedLight"))
    {
        if (dwIconId == 0)
        {
        }
        else if (dwIconId == 1)
        {
        }
        else if (dwIconId == 2)
        {
            g_psUnsaveParam->bRedPenEnable = !g_psUnsaveParam->bRedPenEnable;
            if (!g_psUnsaveParam->bRedPenEnable) 
            {
                g_psUnsaveParam->bRedPenHZ = FALSE;
                g_psUnsaveParam->bRedPenTimerEnable = FALSE;
					SendUnsaveParam();
            }
            xpgUpdateStage();
        }
        else if (dwIconId == 3)
        {
            if (g_psUnsaveParam->bRedPenEnable)
            {
                g_psUnsaveParam->bRedPenHZ = !g_psUnsaveParam->bRedPenHZ;
                xpgUpdateStage();
					SendUnsaveParam();
            }
        }
        else if (dwIconId == 4)
        {
            if (g_psUnsaveParam->bRedPenEnable)
            {
                g_psUnsaveParam->bRedPenTimerEnable = !g_psUnsaveParam->bRedPenTimerEnable;
                xpgUpdateStage();
					SendUnsaveParam();
            }
        }
        else if (dwIconId == 6)
        {
            if (g_psUnsaveParam->bRedPenEnable && g_psUnsaveParam->bRedPenTimerEnable)
            {
                g_psUnsaveParam->wRedPenTime += 10;
                if (g_psUnsaveParam->wRedPenTime > 990)
                    g_psUnsaveParam->wRedPenTime = 990;
                xpgUpdateStage();
					SendUnsaveParam();
            }
        }
        else if (dwIconId == 7)
        {
            if (g_psUnsaveParam->bRedPenEnable && g_psUnsaveParam->bRedPenTimerEnable)
            {
                g_psUnsaveParam->wRedPenTime -= 10;
                if (g_psUnsaveParam->wRedPenTime < 10)
                    g_psUnsaveParam->wRedPenTime = 0;
                xpgUpdateStage();
					SendUnsaveParam();
            }
        }
    }
    else if(dwHashKey == xpgHash("opm1") )
    {
        if (dwIconId == 0)
        {
        }
        else if (dwIconId == 1)
        {
            isSelectOnlineOPM = 1;
            xpgPreactionAndGotoPage("opm2");
            xpgUpdateStage();
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            xpgPreactionAndGotoPage("opmWarn");
            xpgUpdateStage();
        }
    }
    else if(dwHashKey == xpgHash("opm2") )
    {
        if (dwIconId == 0)
        {
            isSelectOnlineOPM = 0;
            xpgPreactionAndGotoPage("opm1");
            xpgUpdateStage();
        }
        else if (dwIconId == 1)
        {
            
        }
    }
    else if (dwHashKey == xpgHash("opmList1"))
    {
        if (dwIconId == 0)
        {
            isSelectOnlineOPM = 0;
            xpgPreactionAndGotoPage("opm1");
            xpgUpdateStage();
        }
        else if (dwIconId == 1)
        {
            isSelectOnlineOPM = 1;
            xpgPreactionAndGotoPage("opm2");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("opmList2"))
    {
        if (dwIconId == 0)
        {
            isSelectOnlineOPM = 0;
            xpgPreactionAndGotoPage("opm1");
            xpgUpdateStage();
        }
        else if (dwIconId == 1)
        {
            isSelectOnlineOPM = 1;
            xpgPreactionAndGotoPage("opm2");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("opmWarn"))
    {
        Free_CacheWin();
        isSelectOnlineOPM = 1;
        xpgPreactionAndGotoPage("opm2");
        xpgUpdateStage();
    }
    else if(dwHashKey == xpgHash("Keyboard") )
    {
        dwKeyID = dwIconId;
        boKeyLight = TRUE;
        xpgUpdateStage();
        //xpgDelay(100);
        boKeyLight = FALSE;
        ///===================================
        if (dwKeyID == 29)                      // capslock
        {
            boCapsLock = !boCapsLock;
        }
        else if (dwKeyID == 37)                 // backspace
        {
            DWORD len = strlen(keyboardBuffer);
            if (len) 
                keyboardBuffer[len - 1] = 0;
        }
        else if (dwKeyID == 38)                 // exit
        {
            keyboardGoBack(FALSE);
        }
        else if (dwKeyID == 42)                 // enter
        {
            keyboardGoBack(TRUE);
        }
        else if (dwKeyID == 39)                 // #
        {
            keyboardAddChar('#');
        }
        else if (dwKeyID == 40)                 // ' '
        {
            keyboardAddChar(' ');
        }
        else if (dwKeyID == 41)                 // _
        {
            keyboardAddChar('_');
        }
        else if (dwKeyID <= 9)                  // 0-9
        {
            char arr[] = {'1','2','3','4','5','6','7','8','9','0'};
            keyboardAddChar(arr[dwKeyID]);
        }
        else if (dwKeyID >= 10 && dwKeyID <= 28 ) 
        {
            DWORD getId = dwKeyID - 10;
            if (boCapsLock)
            {
                char arr[] = {'Q','W','E','R','T','Y','U','I','O','P','A','S','D','F','G','H','J','K','L'};
                keyboardAddChar(arr[getId]);
            }
            else
            {
                char arr[] = {'q','w','e','r','t','y','u','i','o','p','a','s','d','f','g','h','j','k','l'};
                keyboardAddChar(arr[getId]);
            }
        }
        else if (dwKeyID >= 30 && dwKeyID <= 36 ) 
        {
            DWORD getId = dwKeyID - 30;
            if (boCapsLock)
            {
                char arr[] = {'Z','X','C','V','B','N','M'};
                keyboardAddChar(arr[getId]);
            }
            else
            {
                char arr[] = {'z','x','c','v','b','n','m'};
                keyboardAddChar(arr[getId]);
            }
        }
        xpgUpdateStage();
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
        else if (dialogType == Dialog_SetBrightness)
        {
            if (dwIconId == 0)
                dwDialogTempValue++;
            else if (dwIconId == 1)
            {
                if (dwDialogTempValue) dwDialogTempValue--;
            }
            else
                return 0;
            if (dwDialogTempValue > 100)
                dwDialogTempValue = 100;
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
            else if (dwIconId == 2)
            {
                boNeedWriteSetup = FALSE;
                if (g_psSetupMenu->wShutdownTime != dwDialogTempValue)
                {
                    g_psSetupMenu->wShutdownTime = dwDialogTempValue;
                    boNeedWriteSetup = TRUE;
                    xpgCb_AutoPowerOff(g_psSetupMenu->bAutoShutdown,g_psSetupMenu->wShutdownTime);
                }
                exitDialog();
                if (boNeedWriteSetup)
                    WriteSetupChg();
            }
            else if (dwIconId == 3)
            {
                exitDialog();
            }
            else
                return 0;
            xpgUpdateStage();
        }
        else if (dialogType == Dialog_SetSound)
        {
            if (dwIconId == 0)
                dwDialogTempValue++;
            else if (dwIconId == 1)
            {
                if (dwDialogTempValue) dwDialogTempValue--;
            }
            else
                return 0;
            if (dwDialogTempValue > 15)
                dwDialogTempValue = 15;
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
				  DWORD dwLastRtc;
					
                SystemTimeGet(&curTime);
                curTime.u08Hour = dwDialogTempValue >> 16;
                curTime.u08Minute = dwDialogTempValue & 0xffff;
				  dwLastRtc=RTC_ReadCount();
                SystemTimeSet(&curTime);
				  g_psSetupMenu->sdwRtcOffset+=dwLastRtc-RTC_ReadCount();
				  WriteSetupChg();
                exitDialog();
            }
            else if (dwIconId == 7)
            {
                exitDialog();
            }
        }
        else if (dialogType == Dialog_SetDate)
        {
            WORD year2 = dwDialogTempValue >> 16;
            WORD month2 = (dwDialogTempValue >> 8) & 0xff;
            WORD day2 = dwDialogTempValue & 0xff;
            if (dwIconId == 0)
            {
                if (year2 > 2000)
                {
                    year2 --;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 1)
            {
                if (year2 < 2050)
                {
                    year2 ++;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 2)
            {
                if (month2 > 1)
                {
                    month2 --;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 3)
            {
                if (month2 < 12)
                {
                    month2 ++;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 4)
            {
                if (day2 >= 1)
                {
                    day2 --;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 5)
            {
                if (day2 < 31)
                {
                    day2++;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 6)
            {
                ST_SYSTEM_TIME curTime;
				  DWORD dwLastRtc;

                SystemTimeGet(&curTime);
                curTime.u16Year = dwDialogTempValue >> 16;
                curTime.u08Month = (dwDialogTempValue >> 8) & 0xff;
                curTime.u08Day = dwDialogTempValue & 0xff;
				  dwLastRtc=RTC_ReadCount();
                SystemTimeSet(&curTime);
				  g_psSetupMenu->sdwRtcOffset+=dwLastRtc-RTC_ReadCount();
				  WriteSetupChg();
                exitDialog();
            }
            else if (dwIconId == 7)
            {
                exitDialog();
            }
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            if (dwIconId == 0)
            {
                g_psSetupMenu->bDataFormatMMDDYYYY = 0;
                xpgUpdateStage();
                WriteSetupChg();
            }
            else if (dwIconId == 1)
            {
                g_psSetupMenu->bDataFormatMMDDYYYY = 1;
                xpgUpdateStage();
                WriteSetupChg();
            }
        }
        else if (dialogType == Dialog_SetLang)
        {
            g_psSetupMenu->bLanguage = dwIconId;
            xpgUpdateStage();
            WriteSetupChg();
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword)
        {
            static char password1[8] = {0};
            static char password2[8] = {0};
            int len = strlen(strEditPassword);
            if (dwIconId >= 0 && dwIconId <= 9)
            {
                if (len < 4)
                {
                    char ch = '0' + dwIconId;
                    strEditPassword[len] = ch;
                    strEditPassword[len+1] = 0;
                    xpgUpdateStage();
                    ////////////////
                    if (len == 3)               // input finish
                    {
                        if (dialogType == Dialog_CheckPassword)
                        {
                            if (dialogOnClose)  dialogOnClose();
                        }
                        else if (dialogType == Dialog_SetPassword1)
                        {
                            strcpy(password1, strEditPassword);
                            exitDialog();
                            memset(strEditPassword, 0, sizeof(strEditPassword));
                            strDialogTitle = getstr(Str_ZaiQueRenMiMa);
                            popupDialog(Dialog_SetPassword2, "SetPassword");
                            xpgUpdateStage();
                        }
                        else if (dialogType == Dialog_SetPassword2)
                        {
                            strcpy(password2, strEditPassword);
                            if (0 == strcmp(password1, password2))
                            {
                                if (dialogOnClose)  
                                    dialogOnClose();
                            }
                            else
                            {
                                exitDialog();
                            }
                        }
                    }
                }
            }
            else if (dwIconId == 11)
            {
                if (len)
                {
                    strEditPassword[len - 1] = 0;
                    xpgUpdateStage();
                }
            }
        }   
        else if(dialogType == Dialog_Value)
        {
            if (dwIconId == 0)
            {
                if (dialogOnClose)  dialogOnClose();
            }
            else if (dwIconId == 1)
            {
                exitDialog();
            }
        }
        else if (dialogType == Dialog_EditValue)
        {
            int len = strlen(strEditValue);
            if (dwIconId >= 0 && dwIconId <= 9)
            {
                WORD wStrWidth = Idu_GetStringWidth(strEditValue, 0);
                if (wStrWidth < 176)
                {
                    strEditValue[len] = '0' + dwIconId;
                    strEditValue[len+1] = 0;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 10)
            {
            }
            else if (dwIconId == 11)
            {
                if (len != 0)
                {
                    strEditValue[len - 1] = 0;
                    xpgUpdateStage();
                }
            }
        }
        
    }
    
    return 0;
}

SWORD touchSprite_RepeatIcon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    DWORD dwIconId = sprite->m_dwTypeIndex;
    
    
    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        int dialogType = xpgGetCurrDialogTypeId();
       if (dialogType == Dialog_SetTime)
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

        }
        else if (dialogType == Dialog_SetDate)
        {
            WORD year2 = dwDialogTempValue >> 16;
            WORD month2 = (dwDialogTempValue >> 8) & 0xff;
            WORD day2 = dwDialogTempValue & 0xff;
            if (dwIconId == 0)
            {
                if (year2 > 2000)
                {
                    year2 --;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 1)
            {
                if (year2 < 2050)
                {
                    year2 ++;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 2)
            {
                if (month2 > 1)
                {
                    month2 --;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 3)
            {
                if (month2 < 12)
                {
                    month2 ++;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 4)
            {
                if (day2 >= 1)
                {
                    day2 --;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
            }
            else if (dwIconId == 5)
            {
                if (day2 < 31)
                {
                    day2++;
                    dwDialogTempValue = (year2 << 16) | (month2 << 8) | day2;
                    xpgUpdateStage();
                }
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
        dwHashKey == xpgHash("Record") || 
        dwHashKey == xpgHash("RedLight") || 
        dwHashKey == xpgHash("opm1") || 
        dwHashKey == xpgHash("opm2") 
        )
    {
        xpgPreactionAndGotoPage("Main");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("FuncSet") || 
        dwHashKey == xpgHash("FuncSet2") ||
        dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3") )
    {
		if (st_bGoSetupFrom)
		{
			xpgCb_EnterCamcoderPreview();
		}
		else
		{
	        xpgPreactionAndGotoPage("Main");
	        xpgUpdateStage();
		}
    }
    else if (dwHashKey == xpgHash("FusionModeSet"))
    {
        xpgPreactionAndGotoPage("FusionSet1");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("opmList1"))
    {
        xpgPreactionAndGotoPage("opm1");
	    xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("opmList2"))
    {
        xpgPreactionAndGotoPage("opm2");
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
        xpgPreactionAndGotoPage("Main");
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
        else if (dialogType == Dialog_SetBrightness)
        {
            if (g_psSetupMenu->bBrightness!= dwDialogTempValue)
            {
                g_psSetupMenu->bBrightness = dwDialogTempValue;
                boNeedWriteSetup = TRUE;
            }
            exitDialog();
            if (boNeedWriteSetup)
                WriteSetupChg();
        }
        else if (dialogType == Dialog_ShutdownTime)
        {
            if (g_psSetupMenu->wShutdownTime != dwDialogTempValue)
            {
                g_psSetupMenu->wShutdownTime = dwDialogTempValue;
                boNeedWriteSetup = TRUE;
                xpgCb_AutoPowerOff(g_psSetupMenu->bAutoShutdown,g_psSetupMenu->wShutdownTime);
            }
            exitDialog();
            if (boNeedWriteSetup)
                WriteSetupChg();
        }
        else if (dialogType == Dialog_SetSound)
        {
            if (g_psSetupMenu->bVolume!= dwDialogTempValue)
            {
                g_psSetupMenu->bVolume = dwDialogTempValue;
                boNeedWriteSetup = TRUE;
            }
            exitDialog();
            if (boNeedWriteSetup)
                WriteSetupChg();
        }
        else if (dialogType == Dialog_SetTime || dialogType == Dialog_SetDate)
        {
            exitDialog();
        }
        else if (dialogType == Dialog_SetDateFormat)
        {
            exitDialog();
        }
        else if (dialogType == Dialog_SetLang)
        {
            exitDialog();
        }
        else if (dialogType == Dialog_About || dialogType == Dialog_Times || dialogType == Dialog_TempInfo || dialogType == Dialog_BatInfo)
        {
            exitDialog();
        }
        else if (dialogType == Dialog_SetPassword1 || dialogType == Dialog_SetPassword2 || dialogType == Dialog_CheckPassword)
        {
            exitDialog();
        }
        else if (dialogType == Dialog_Value)
        {
            exitDialog();
        }
        else if (dialogType == Dialog_EditValue)
        {
            if (strEditValue[0])
            {
                if (boDialogValueIsFloat)
                {
                    unsigned int value1, value2;
                    char *pp;
                    pp = strchr(strEditValue, '.');
                    if (pp == NULL)
                    {
                        value1 = atoi(strEditValue);
                        dwDialogTempValue = value1 << 6;
                    }
                    else
                    {
                        char tempbuf1[128];
                        strncpy(tempbuf1, strEditValue, pp - strEditValue);
                        tempbuf1[pp - strEditValue] = 0;
                        value1 = atoi(tempbuf1);
                        value2 = atoi(pp+1);
                        dwDialogTempValue = (value1 << 6) | value2;
                    }
                }
                else
                {
                    int newValue = atoi(strEditValue);
                    dwDialogTempValue = newValue;
                }
            }
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

static void Dialog_CheckPassword_CloseBootPassword_OnInput()
{
    mpDebugPrint("%s()", __FUNCTION__);
    g_psSetupMenu->bEnableOpenPassword = 0;
    memset(g_psSetupMenu->srtOpenPassword, 0, sizeof(g_psSetupMenu->srtOpenPassword));
    exitDialog();
    WriteSetupChg();
}

static void Dialog_CheckPassword_ChangeBootPassword_OnInput()
{
    exitDialog();
    xpgUpdateStage();
    memset(strEditPassword, 0, sizeof(strEditPassword));
    strDialogTitle = getstr(Str_SheZhiKaiJiMiMa);
    dialogOnClose = Dialog_SetPassword_BootPassword_OnInput;
    popupDialog(Dialog_SetPassword1, "SetPassword");
    xpgUpdateStage();
}

static void Dialog_CheckPassword_CloseHirePassword_OnInput()
{
    mpDebugPrint("%s()", __FUNCTION__);
    g_psSetupMenu->bEnableHirePassword = 0;
    memset(g_psSetupMenu->strHirePassword, 0, sizeof(g_psSetupMenu->strHirePassword));
    exitDialog();
    WriteSetupChg();
}

static void Dialog_CheckPassword_ChangeHirePassword_OnInput()
{
    exitDialog();
    xpgUpdateStage();
    memset(strEditPassword, 0, sizeof(strEditPassword));
    strDialogTitle = getstr(Str_SheZhiZuJieMiMa);
    dialogOnClose = Dialog_SetPassword_HirePassword_OnInput;
    popupDialog(Dialog_SetPassword1, "SetPassword");
    xpgUpdateStage();
}

static void Dialog_SetPassword_BootPassword_OnInput()
{
    mpDebugPrint("%s()", __FUNCTION__);
    g_psSetupMenu->bEnableOpenPassword = 1;
    strcpy(g_psSetupMenu->srtOpenPassword, strEditPassword);
    exitDialog();
    WriteSetupChg();
}

static void Dialog_SetPassword_HirePassword_OnInput()
{
    mpDebugPrint("%s()", __FUNCTION__);
    g_psSetupMenu->bEnableHirePassword = 1;
    strcpy(g_psSetupMenu->strHirePassword, strEditPassword);
    exitDialog();
    WriteSetupChg();
}

static void Dialog_SetValue_SuoDingRongJieCiShu()
{
    g_psSetupMenu->wLockedTimes = dwDialogTempValue;
    exitDialog();
    WriteSetupChg();
}

static void Dialog_SetValue_RongJieSheZhi()
{
    *pdwEditingFusionValue = dwDialogTempValue;
    exitDialog();
    WriteSetupChg();
}

SWORD touchSprite_Text(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    int dialogType = xpgGetCurrDialogTypeId();
    DWORD dwSpriteId = sprite->m_dwTypeIndex;

    mpDebugPrint("touchSprite_Text  %d", dwSpriteId);
    
    if (dwHashKey == xpgHash(DIALOG_PAGE_NAME))
    {
        if (dialogType == Dialog_Value)
        {
            if (dwSpriteId == 0)
            {
                if (boDialogValueIsFloat)
                    sprintf(strEditValue, "%d.%d", dwDialogTempValue>>6, dwDialogTempValue&0x3F);
                else
                    sprintf(strEditValue, "%d", dwDialogTempValue);
                popupDialog(Dialog_EditValue, DIALOG_PAGE_NAME);
                xpgUpdateStage();
            }
        }
    }
    else if (dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2"))
    {
        DWORD * pdwPageId;
        DWORD * pdwTotal;
        DWORD   pageNum;
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
        pageNum = *pdwTotal / 5;
        if (*pdwTotal % 5)
            pageNum++;
        //////////////
        if (dwSpriteId == 2)
        {
            *pdwPageId = 0;
            xpgUpdateStage();
        }
        else if (dwSpriteId == 3)
        {
            if (pageNum > 0 && *pdwPageId >= 1)
            {
                (*pdwPageId) --;
                xpgUpdateStage();
            }
        }
        else if (dwSpriteId == 4)
        {
            if (pageNum > 0 && *pdwPageId < pageNum - 1)
            {
                (*pdwPageId) ++;
                xpgUpdateStage();
            }
        }
        else if (dwSpriteId == 5)
        {
            if (pageNum > 1)
            {
                *pdwPageId = pageNum - 1;
                xpgUpdateStage();
            }
        }
    }
    return 0;
}

SWORD touchSprite_Selector(STXPGSPRITE * sprite, WORD x, WORD y)
{
    DWORD dwSelectorId = sprite->m_dwTypeIndex;
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
	register STXPGBROWSER *pstBrowser = g_pstBrowser;
    mpDebugPrint("touchSprite_Selector  %d", sprite->m_dwTypeIndex);
    
    if (dwHashKey == xpgHash("FuncSet"))
    {
        xpgPreactionAndGotoPage("FuncSet2");
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("FuncSet2"))
    {
        xpgPreactionAndGotoPage("FuncSet");
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
            xpgPreactionAndGotoPage("SetYun");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 1)
        {
            xpgPreactionAndGotoPage("SetSleep");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 2)
        {
            xpgPreactionAndGotoPage("SetSound");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 3)
        {
            xpgPreactionAndGotoPage("SetTime");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 4)
        {
            xpgPreactionAndGotoPage("SetPassword");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 5)
        {
            //xpgPreactionAndGotoPage("SetUi");
            xpgPreactionAndGotoPage("SetInfo");
            xpgUpdateStage();
        }
        //else if (dwSelectorId == 6)
        //{
        //    xpgPreactionAndGotoPage("SetInfo");
        //    xpgUpdateStage();
        //}
        
    }
    else if (dwHashKey == xpgHash("FusionSet1") ||
        dwHashKey == xpgHash("FusionSet2") ||
        dwHashKey == xpgHash("FusionSet3")  )
    {
        if (dwSelectorId == 0)
        {
            xpgPreactionAndGotoPage("FusionSet1");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 1)
        {
            xpgPreactionAndGotoPage("FusionSet2");
            xpgUpdateStage();
        }
        else if (dwSelectorId == 2)
        {
            xpgPreactionAndGotoPage("FusionSet3");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("opm1"))
    {
        if (dwSelectorId == 0)
        {
			xpgPreactionAndGotoPage("opmList1");
			pstBrowser->wListCount = 5;
			pstBrowser->wListIndex= 0;
			pstBrowser->wIndex= 0;
			pstBrowser->wCount= OpmGetTotalNumber(0x02);
			xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("opm2"))
    {
        if (dwSelectorId == 0)
        {
			xpgPreactionAndGotoPage("opmList2");
			pstBrowser->wListCount = 5;
			pstBrowser->wListIndex= 0;
			pstBrowser->wIndex= 0;
			pstBrowser->wCount= OpmGetTotalNumber(0x04);
			xpgUpdateStage();
        }
    }
    
    return 0;
}

static OPMDATAITEM * editingOpmDataItem;
static const char * opmBackPageName;
static void opmKeyboardBack(BOOL boEnter)
{
    if (boEnter)
    {
        strncpy(editingOpmDataItem->itemName, keyboardBuffer, sizeof(editingOpmDataItem->itemName) - 1);
        editingOpmDataItem->itemName[sizeof(editingOpmDataItem->itemName) - 1] = 0;
    }
    Free_CacheWin();
    xpgPreactionAndGotoPage(opmBackPageName);
    xpgUpdateStage();
}

void startEditOpmRecordName(const char * oldPageName, OPMDATAITEM * curItem)
{
    strncpy(keyboardBuffer, curItem->itemName, KEYBOARD_BUFFER_SIZE);
    keyboardBuffer[KEYBOARD_BUFFER_SIZE - 1] = 0;
    editingOpmDataItem = curItem;
    opmBackPageName = oldPageName;
    keyboardGoBack = opmKeyboardBack;
    
    Free_CacheWin();
    Idu_GetCacheWin_WithInit();
    boCapsLock = FALSE;
    xpgPreactionAndGotoPage("Keyboard");
    xpgUpdateStage();
}

static STRECORD * editingFusionDataItem;

static void fusionRecordKeyboardBack(BOOL boEnter)
{
    if (boEnter)
    {
        strncpy(editingFusionDataItem->bRecordName, keyboardBuffer, sizeof(editingFusionDataItem->bRecordName) - 1);
        editingFusionDataItem->bRecordName[sizeof(editingFusionDataItem->bRecordName) - 1] = 0;
    }
    Free_CacheWin();
    xpgPreactionAndGotoPage("Record");
    xpgUpdateStage();
}

void startEditFusionRecordName(STRECORD * pr)
{
    strncpy(keyboardBuffer, pr->bRecordName, KEYBOARD_BUFFER_SIZE);
    keyboardBuffer[KEYBOARD_BUFFER_SIZE - 1] = 0;
    editingFusionDataItem = pr;
    keyboardGoBack = fusionRecordKeyboardBack;
    
    Free_CacheWin();
    Idu_GetCacheWin_WithInit();
    boCapsLock = FALSE;
    xpgPreactionAndGotoPage("Keyboard");
    xpgUpdateStage();
}


static void keyboardAddChar(char c)
{
    int len = strlen(keyboardBuffer);
    if (len < KEYBOARD_INPUT_CHAR_LENTH)
    {
        keyboardBuffer[len] = c;
        keyboardBuffer[len + 1] = 0;
    }
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
        if (dwSpriteId == 1)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            dwDialogTempValue = g_psSetupMenu->bBrightness;
            popupDialog(Dialog_SetBrightness, "SetSleep");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 3)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            dwDialogTempValue = g_psSetupMenu->wShutdownTime;
            popupDialog(Dialog_ShutdownTime, "SetSleep");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("SetSound"))
    {
        if (dwSpriteId == 1)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            dwDialogTempValue = g_psSetupMenu->bVolume;
            popupDialog(Dialog_SetSound, "SetSound");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("SetTime"))
    {
        if (dwSpriteId == 1)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            //////////////////////////
            ST_SYSTEM_TIME curTime;
            SystemTimeGet(&curTime);
            dwDialogTempValue = (curTime.u08Hour<<16) | curTime.u08Minute;
            popupDialog(Dialog_SetTime, "SetTime");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 2)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            //////////////////////////
            ST_SYSTEM_TIME curTime;
            SystemTimeGet(&curTime);
            dwDialogTempValue = (curTime.u16Year<<16) | (curTime.u08Month<< 8) | curTime.u08Day;
            popupDialog(Dialog_SetDate, "SetTime");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 3)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            popupDialog(Dialog_SetDateFormat, "SetTime");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 4)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            popupDialog(Dialog_SetLang, "SetTime");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("SetPassword"))
    {
        if (dwSpriteId == 1)
        {
            if (g_psSetupMenu->bEnableOpenPassword)
            {
                memset(strEditPassword, 0, sizeof(strEditPassword));
                strDialogTitle = getstr(Str_QingShuRuMiMa);
                dialogOnClose = Dialog_CheckPassword_CloseBootPassword_OnInput;
                popupDialog(Dialog_CheckPassword, "SetPassword");
                xpgUpdateStage();
            }
            else
            {
                memset(strEditPassword, 0, sizeof(strEditPassword));
                strDialogTitle = getstr(Str_SheZhiKaiJiMiMa);
                dialogOnClose = Dialog_SetPassword_BootPassword_OnInput;
                popupDialog(Dialog_SetPassword1, "SetPassword");
                xpgUpdateStage();
            }
        }
        else if (dwSpriteId == 3)
        {
            if (g_psSetupMenu->bEnableHirePassword)
            {
                memset(strEditPassword, 0, sizeof(strEditPassword));
                strDialogTitle = getstr(Str_QingShuRuMiMa);
                dialogOnClose = Dialog_CheckPassword_CloseHirePassword_OnInput;
                popupDialog(Dialog_CheckPassword, "SetPassword");
                xpgUpdateStage();
            }
            else
            {
                memset(strEditPassword, 0, sizeof(strEditPassword));
                strDialogTitle = getstr(Str_SheZhiZuJieMiMa);
                dialogOnClose = Dialog_SetPassword_HirePassword_OnInput;
                popupDialog(Dialog_SetPassword1, "SetPassword");
                xpgUpdateStage();
            }
        }
        else if (dwSpriteId == 4)
        {
            if (!g_psSetupMenu->bEnableHirePassword)
            {
            }
        }
        else if (dwSpriteId == 5)
        {
            if (!g_psSetupMenu->bEnableHirePassword)
            {
                strDialogTitle = getstr(Str_SuoDingRongJieCiShu);
                dialogOnClose = Dialog_SetValue_SuoDingRongJieCiShu;
                dwDialogTempValue = g_psSetupMenu->wLockedTimes;
                boDialogValueIsFloat = 0;
                popupDialog(Dialog_Value, "SetPassword");
                xpgUpdateStage();
            }
        }
        else if (dwSpriteId == 6)
        {
            if (g_psSetupMenu->bEnableOpenPassword)
            {
                memset(strEditPassword, 0, sizeof(strEditPassword));
                strDialogTitle = getstr(Str_QingShuRuMiMa);
                dialogOnClose = Dialog_CheckPassword_ChangeBootPassword_OnInput;
                popupDialog(Dialog_CheckPassword, "SetPassword");
                xpgUpdateStage();
            }
        }
        else if (dwSpriteId == 7)
        {
            if (g_psSetupMenu->bEnableHirePassword)
            {
                memset(strEditPassword, 0, sizeof(strEditPassword));
                strDialogTitle = getstr(Str_QingShuRuMiMa);
                dialogOnClose = Dialog_CheckPassword_ChangeHirePassword_OnInput;
                popupDialog(Dialog_CheckPassword, "SetPassword");
                xpgUpdateStage();
            }
        }
    }
    else if (dwHashKey == xpgHash("SetInfo"))
    {
        if (dwSpriteId == 0)
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            mpCopyEqualWin(Idu_GetCurrWin(), Idu_GetCacheWin());
            popupDialog(Dialog_About, "SetInfo");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 1)
        {
            popupDialog(Dialog_Times, "SetInfo");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 2)
        {
            popupDialog(Dialog_TempInfo, "SetInfo");
            xpgUpdateStage();
        }
        else if (dwSpriteId == 3)
        {
            popupDialog(Dialog_BatInfo, "SetInfo");
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("opmList1") || dwHashKey == xpgHash("opmList2"))
    {
        /////
        DWORD dwListId = dwSpriteId;
        
        DWORD * pdwPageId;
        DWORD * pdwTotal;
        OPMDATAITEM * pstItems;
        OPMDATAITEM * curItem;
        const char * xpgPageName;
            
        if (dwHashKey == xpgHash("opmList1"))
        {
            pdwPageId = &opmLocalDataCurrentPage;
            pdwTotal = &opmLocalDataTotal;
            pstItems = localOpmData;
            xpgPageName = "opmList1";
        }
        else
        {
            pdwPageId = &opmCloudDataCurrentPage;
            pdwTotal = &opmCloudDataTotal;
            pstItems = cloudOpmData;
            xpgPageName = "opmList2";
        }
        if ((*pdwPageId) * 5 + dwListId >= *pdwTotal)
            return PASS;
        curItem = & pstItems[(*pdwPageId) * 5 + dwListId];
        startEditOpmRecordName(xpgPageName, curItem);
        
    }
    else if (dwHashKey == xpgHash("Record"))
    {
        char tmpbuff[128];
        DWORD dwListId = dwSpriteId;
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
        // if (PAGE_RECORD_SIZE * g_dwRecordListCurrPage+dwSpriteId <FileBrowserGetTotalFile())
        startEditFusionRecordName(pr);
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
			xpgCb_AutoPowerOff(g_psSetupMenu->bAutoShutdown,g_psSetupMenu->wShutdownTime);
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
    else if (dwHashKey == xpgHash("SetPassword"))
    {
        if (dwSpriteId == 0)
        {
            g_psSetupMenu->bEnableOpenPassword = ! g_psSetupMenu->bEnableOpenPassword;
            xpgUpdateStage();
            WriteSetupChg();
        }
        else if (dwSpriteId == 1)
        {
            g_psSetupMenu->bEnableHirePassword = ! g_psSetupMenu->bEnableHirePassword;
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
       // if (x1 >= 0)
        {
            // scroll left_x = 32, scroll right_x = 282
            if (x1 <=0)
                g_psSetupMenu->bVolume = 0;
            else if (x1 >= sprite->m_wWidth)
                g_psSetupMenu->bVolume = 15;
            else {
                // 282 - 32 = 250
                g_psSetupMenu->bVolume = x1*15/sprite->m_wWidth;
            }
            xpgUpdateStage();
            WriteSetupChg();
        }
    }
}

static void uiEnterRecordList()
{
    DWORD i = 0;
    DWORD total, dwRtcTime;
    
    BYTE pbTitle[12];
    STWELDSTATUS WeldStatus;
    FileBrowserResetFileList(); /* reset old file list first */
    FileBrowserScanFileList(SEARCH_TYPE);
    total = FileBrowserGetTotalFile();

    ClearAllRecord();
    for (i = 0; i < total; i++)
    {
        FileListSetCurIndex(i);
        memset(pbTitle, 0, sizeof(pbTitle));
        memset(&WeldStatus, 0, sizeof(WeldStatus));
        Weld_ReadFileWeldInfo(NULL, pbTitle, &WeldStatus);

        dwRtcTime = 0;
        Weld_FileNameToTime(i, &dwRtcTime);                         // 获取时间 

        ST_SYSTEM_TIME sysTime;
        SystemTimeSecToDateConv(dwRtcTime, &sysTime);

        ///////////////
        STRECORD recordData;
        memset(&recordData, 0, sizeof(recordData));

        recordData.bHead = 0;
        recordData.bLenth = 0;
        recordData.bIndex = 0;
        recordData.bYear = sysTime.u16Year;
        recordData.bMonth = sysTime.u08Month;
        recordData.bDay = sysTime.u08Day;
        recordData.bHour = sysTime.u08Hour;
        recordData.bMinute = sysTime.u08Minute;
        recordData.bSecond = sysTime.u08Second;
        strncpy(recordData.bRecordName, pbTitle, sizeof(recordData.bRecordName) - 1);
        recordData.bRecordName[sizeof(recordData.bRecordName) - 1] = 0;
        recordData.bFiberMode = WeldStatus.bFiberMode;
        recordData.bFiberL = WeldStatus.wFiberL;
        recordData.bFiberR = WeldStatus.wFiberR;
        recordData.bFiberLoss = WeldStatus.bFiberLoss;
        recordData.bResult = WeldStatus.bResult;
        recordData.wFileIndex = i;                              // 保存文件索引号
        recordData.bChecksum = 0;

        AddRecord(&recordData);
    }
    	

    g_dwRecordListCurrPage = 0;                     // 设置当前为第0页
    xpgPreactionAndGotoPage("Record");
    xpgUpdateStage();
}


typedef SWORD(*touchFunction)(STXPGSPRITE *, WORD, WORD);
typedef struct  {
    DWORD           touchFlag;
    touchFunction   touchFunc;
}TouchSpriteFunction;

#define ENABLE_SLIDE        		BIT0
#define ENABLE_REPEAT        	BIT1

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
    {0,             NULL},                           // type27
    {0,             NULL},                           // type28
    {ENABLE_REPEAT,             touchSprite_RepeatIcon},                           // type29
};


void uiDispatchTouchSprite(WORD x1, WORD y1)
{
    int idx;
    STXPGMOVIE *pstMovie = &g_stXpgMovie;
    STXPGSPRITE *pstSprite = NULL;
    STXPGTOUCHINFO * pstTouchInfo = NULL;
    BOOL match;

	xpgCb_AutoPowerOff(g_psSetupMenu->bAutoShutdown,g_psSetupMenu->wShutdownTime);
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
                //if (!(pfunc->touchFlag& ENABLE_SLIDE))
                //    dwLastTouchActionTime = GetSysTime();
				if (!st_bTouchDown || ((pfunc->touchFlag& (ENABLE_SLIDE|ENABLE_REPEAT))&& (SystemGetElapsedTime(dwLastTouchActionTime) > 150)))
				{
					MP_ALERT("%d, %d",  x1, y1);
					pfunc->touchFunc (pstSprite, x1, y1);
					dwLastTouchActionTime = GetSysTime();
				}
				#if (PRODUCT_UI==UI_WELDING)
    			// AddAutoEnterPreview();
				#endif
            }
        }
    }
}
#endif

#endif  //#if (TOUCH_CONTROLLER_ENABLE == ENABLE)










