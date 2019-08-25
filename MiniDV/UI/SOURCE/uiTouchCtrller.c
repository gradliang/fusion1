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

#if (TOUCH_CONTROLLER_ENABLE == ENABLE)

DWORD dwLastTouchActionTime = 0;

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
            xpgSearchAndGotoPage("touch", 5);
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
    
    if (dwHashKey == xpgHash("Main", strlen("Main")))
    {
        if (dwIconId == 0) 
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            xpgSearchAndGotoPage("User", strlen("User"));
            xpgUpdateStage();
        }
        else if (dwIconId == 1) 
        {
            xpgSearchAndGotoPage("Record", strlen("Record"));
            xpgUpdateStage();
        }
        else if (dwIconId == 2) 
        {
            xpgSearchAndGotoPage("FusionSet1", strlen("FusionSet1"));
            xpgUpdateStage();
        }
        else if (dwIconId == 3) 
        {
            xpgSearchAndGotoPage("Manual_work", strlen("Manual_work"));
            xpgUpdateStage();
        }
        else if (dwIconId == 4) 
        {
            Free_CacheWin();
            Idu_GetCacheWin_WithInit();
            DrakWin(Idu_GetCacheWin(), 2, 1);
            xpgSearchAndGotoPage("ToolBox", strlen("ToolBox"));
            xpgUpdateStage();
        }
        else if (dwIconId == 5) 
        {
            xpgSearchAndGotoPage("SetYun", strlen("SetYun"));
            xpgUpdateStage();
        }
        else if (dwIconId == 6) 
        {
            xpgSearchAndGotoPage("FuncSet", strlen("FuncSet"));
            xpgUpdateStage();
        }
    }
    else if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")))
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
    }
    else if (dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
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
        }
        xpgUpdateStage();
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
        xpgSearchAndGotoPage("Main", strlen("Main"));
        xpgUpdateStage();
    }
    return 0;
}

SWORD touchSprite_CloseIcon(STXPGSPRITE * sprite, WORD x, WORD y)
{
    mpDebugPrint("touchSprite_CloseIcon  %d", sprite->m_dwTypeIndex);
    
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if (dwHashKey == xpgHash("User", strlen("User")) || 
        dwHashKey == xpgHash("ToolBox", strlen("ToolBox")))
    {
        xpgSearchAndGotoPage("Main", strlen("Main"));
        xpgUpdateStage();
    }
    return 0;
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
    
    if (dwHashKey == xpgHash("FuncSet", strlen("FuncSet")))
    {
        xpgSearchAndGotoPage("FuncSet2", strlen("FuncSet2"));
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("FuncSet2", strlen("FuncSet2")))
    {
        xpgSearchAndGotoPage("FuncSet", strlen("FuncSet"));
        xpgUpdateStage();
    }
    else if (dwHashKey == xpgHash("SetYun", strlen("SetYun")) ||
        dwHashKey == xpgHash("SetSleep", strlen("SetSleep")) ||
        dwHashKey == xpgHash("SetSound", strlen("SetSound")) ||
        dwHashKey == xpgHash("SetTime", strlen("SetTime")) ||
        dwHashKey == xpgHash("SetPassword", strlen("SetPassword")) ||
        dwHashKey == xpgHash("SetUi", strlen("SetUi")) ||
        dwHashKey == xpgHash("SetInfo", strlen("SetInfo")))
    {
        if (dwSelectorId == 0)
        {
            xpgSearchAndGotoPage("SetYun", strlen("SetYun"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 1)
        {
            xpgSearchAndGotoPage("SetSleep", strlen("SetSleep"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 2)
        {
            xpgSearchAndGotoPage("SetSound", strlen("SetSound"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 3)
        {
            xpgSearchAndGotoPage("SetTime", strlen("SetTime"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 4)
        {
            xpgSearchAndGotoPage("SetPassword", strlen("SetPassword"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 5)
        {
            xpgSearchAndGotoPage("SetUi", strlen("SetUi"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 6)
        {
            xpgSearchAndGotoPage("SetInfo", strlen("SetInfo"));
            xpgUpdateStage();
        }
        
    }
    else if (dwHashKey == xpgHash("FusionSet1", strlen("FusionSet1")) ||
        dwHashKey == xpgHash("FusionSet2", strlen("FusionSet2")) ||
        dwHashKey == xpgHash("FusionSet3", strlen("FusionSet3"))  )
    {
        if (dwSelectorId == 0)
        {
            xpgSearchAndGotoPage("FusionSet1", strlen("FusionSet1"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 1)
        {
            xpgSearchAndGotoPage("FusionSet2", strlen("FusionSet2"));
            xpgUpdateStage();
        }
        else if (dwSelectorId == 2)
        {
            xpgSearchAndGotoPage("FusionSet3", strlen("FusionSet3"));
            xpgUpdateStage();
        }
    }
    
    return 0;
}

SWORD touchSprite_List(STXPGSPRITE * sprite, WORD x, WORD y)
{
    mpDebugPrint("touchSprite_List  %d", sprite->m_dwTypeIndex);
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

    if (dwHashKey == xpgHash("SetYun", strlen("SetYun")))
    {
        if (dwSpriteId == 0)
        {
            g_psSetupMenu->bCloudMode = !g_psSetupMenu->bCloudMode;
            xpgUpdateStage();
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

    if (dwHashKey == xpgHash("SetSleep", strlen("SetSleep")))
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
    else if (dwHashKey == xpgHash("SetSound", strlen("SetSound")))
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










