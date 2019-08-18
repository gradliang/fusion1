/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"

#include "os.h"
#include "display.h"
#include "taskid.h"
#include "ui.h"

#define RemoveTimerProc         Ui_TimerProcRemove

//---------------------------------------------------------------------------
// xpgProcessEvent
//---------------------------------------------------------------------------
typedef void (*UXPG_UI_DRAW_KEY_ICON_CALLBACK)(BYTE);

static DWORD g_dwKeyDownTime = 0;
static DWORD g_dwLastKey = 0;

STACTFUNC *xpgActionFunctions;
#if  MAKE_BLUETOOTH	
STACTFUNC *xpgActionBtFunctions;
#endif
#if NETWARE_ENABLE	
STACTFUNC *xpgActionWifiFunctions;
#endif
static UXPG_UI_DRAW_KEY_ICON_CALLBACK xpgUiDrawKeyIconFuncPtr = NULL;

extern DWORD g_bAniFlag;
extern BYTE g_boNeedRepaint;

// in main.c, XpgInit():
// xpgUiActionFunctionRegister(actionFunctions);
void xpgUiActionFunctionRegister(STACTFUNC *xpgActionFunctionsTable)
{
    xpgActionFunctions = xpgActionFunctionsTable;
}
#if  MAKE_BLUETOOTH	
// xpgUiActionBtFunctionRegister(actionBtFunctions);
void xpgUiActionBtFunctionRegister(STACTFUNC *xpgActionBtFunctionsTable)
{
    xpgActionBtFunctions = xpgActionBtFunctionsTable;
}
#endif
#if NETWARE_ENABLE	
// xpgUiActionWifiFunctionRegister(actionWifiFunctions);
void xpgUiActionWifiFunctionRegister(STACTFUNC *xpgActionWifiFunctionsTable)
{
    xpgActionWifiFunctions = xpgActionWifiFunctionsTable;
}
#endif
void xpgUiDrawKeyIconFunctionRegister(void *funcPtr)
{
    xpgUiDrawKeyIconFuncPtr = (UXPG_UI_DRAW_KEY_ICON_CALLBACK) funcPtr;
}

#define   BT_DWACTION_START   500
#define WIFI_DWACTION_START   800
///
///@ingroup xpgUi
///@brief    Process input event/KeyCode then get Goto/Action and UpdateStage
///
///@param   dwEvent - event triggered
///
///@param   dwKeyCode - specified KeyCode
///
///@retval 0 or Action
///
DWORD xpgProcessEvent(DWORD dwEvent, DWORD dwKeyCode)
{
	register STXPGMOVIE *pstMovie = &g_stXpgMovie;
	MP_DEBUG2("event=%04x, key=%02x", dwEvent, dwKeyCode);

	g_dwKeyDownTime = GetSysTime();
	g_dwLastKey = dwKeyCode;
#if ERROR_HANDLER_ENABLE
	if (g_boShowDialog)
	{
		xpgHideDialog();
		return 0;
	}
#endif
    if (dwKeyCode == 0) return 0;

    //xpgDrawPressedButton(dwEvent, (BYTE) (dwKeyCode & 0xff)); // Jonny off 20060616 for redundent
    if ((dwEvent == XPG_EVENT_KEY_DOWN) && xpgUiDrawKeyIconFuncPtr)
        //xpgDrawKeyIcon((BYTE) (dwKeyCode & 0xff));
        xpgUiDrawKeyIconFuncPtr((BYTE) (dwKeyCode & 0xff));

    //IODelay(1000);
    register SWORD i;

    //WORD wCurPage = pstMovie->m_wCurPage;
    for (i = pstMovie->m_dwCmdCount - 1; i >= 0; i--)
    {
        if (dwKeyCode == (pstMovie->m_dwCommand[i][0] + 1) && pstMovie->m_dwCommand[i][3] == dwEvent)
        {
            register DWORD iPage = pstMovie->m_dwCommand[i][1];
            register DWORD dwAction = (DWORD) (pstMovie->m_dwCommand[i][2]);

			g_boNeedRepaint = true;
			if (iPage > 0)
			{
				RemoveTimerProc(xpgUpdateStage);
				xpgGotoPage(iPage - 1);
			}

			if ((dwAction > 0 && dwAction < BT_DWACTION_START) && xpgActionFunctions[dwAction].hCommand)
			{
			     //mpDebugPrint("%s: dwAction = %d, dwKeyCode = %d", __func__, dwAction, dwKeyCode);
			     //mpDebugPrint("%s: xpgActionFunctions[dwAction].hCommand = 0x%X", __func__, xpgActionFunctions[dwAction].hCommand);
#if XPG_STD_DPF_GUI
                (*(xpgActionFunctions[dwAction].hCommand)) (dwKeyCode);
#else			
				(*(xpgActionFunctions[dwAction].hCommand)) ();
#endif				
		    }  		
        #if  MAKE_BLUETOOTH	
            if ((dwAction > BT_DWACTION_START) && (dwAction < WIFI_DWACTION_START))
            {
                mpDebugPrint("dwAction > BT_DWACTION_START, dwAction = %d", dwAction-1);
    			if (xpgActionBtFunctions[dwAction-BT_DWACTION_START].hCommand)
#if XPG_STD_DPF_GUI    			
    			     (*(xpgActionBtFunctions[dwAction-BT_DWACTION_START].hCommand)) (dwKeyCode);
#else    			
    			     (*(xpgActionBtFunctions[dwAction-BT_DWACTION_START].hCommand)) ();
#endif    			     
			}
		#endif
        #if NETWARE_ENABLE	
			if ((dwAction > WIFI_DWACTION_START))
            {
                mpDebugPrint("dwAction > WIFI_DWACTION_START, dwAction = %d", dwAction-1);
    			if (xpgActionWifiFunctions[dwAction-WIFI_DWACTION_START].hCommand)
#if XPG_STD_DPF_GUI    			
    			     (*(xpgActionWifiFunctions[dwAction-WIFI_DWACTION_START].hCommand)) (dwKeyCode);
#else    			
    			     (*(xpgActionWifiFunctions[dwAction-WIFI_DWACTION_START].hCommand)) ();
#endif    			     
			}
		#endif
        				
			#if EREADER_ENABLE
				if(TypesettingStatus==0)
			#endif
				{
					if (g_boNeedRepaint)
						xpgUpdateStage();
				}

			g_dwKeyDownTime = GetSysTime();
			return dwAction;
		}
	}

	return 0;
}

