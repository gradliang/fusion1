// Setup.c
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"
#include "../../../libIPLAY/libSrc/display/include/displaystructure.h"
#include "ui_Timer.h"
#include "Fileplayer.h"
#include "xpgFunc.h"
#include "Icon.h"
#include "slideEffect.h"
#include "cv.h"
#include "Setup.h"
#include "isp.h"
//#include "SetupString.h"
#include "Taskid.h"
#include "ui.h"
#include "mpapi.h"
#include "camera_func.h" 
#include "xpgCamFunc.h"

BOOL bSetUpChg = false;
static ST_SETUP_MENU g_sSetupMenu;
ST_SETUP_MENU *g_psSetupMenu = &g_sSetupMenu;
DWORD gSetupMenuValue[SETTING_NUMBER+8];


//--下面三个函数要对应增加或改动
void GetDefaultSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);

	g_psSetupMenu->dwSetupFlag=USER_SET_TAG;
	g_psSetupMenu->dwSetupLenth=sizeof(ST_SETUP_MENU);

    // system data **************************************************************
    g_psSetupMenu->bUsbdMode = SETUP_MENU_USBD_MODE_MSDC;

	//User data

#if  (PRODUCT_UI==UI_SURFACE)
	g_psSetupMenu->bReferPhoto=0xff;
	g_psSetupMenu->bBackUp=0;
	g_psSetupMenu->bBackDown=0;
	g_psSetupMenu->bPhotoUp=0;
	g_psSetupMenu->bPhotoDown=0;
#endif
#if  (PRODUCT_UI==UI_WELDING)
	g_psSetupMenu->wElectrodePos[0]=0;
	g_psSetupMenu->wElectrodePos[1]=0;

    g_psSetupMenu->bEnableIcon_LaLiCeShi = 0;
    g_psSetupMenu->bEnableIcon_DuanMianJianCe = 0;
    g_psSetupMenu->bEnableIcon_ZiDongDuiJiao = 0;
    g_psSetupMenu->bEnableIcon_JiaoDuJianCe = 0;
    g_psSetupMenu->bEnableIcon_BaoCunTuXiang = 0;
    g_psSetupMenu->bEnableIcon_HuiChenJianCe = 0;
    g_psSetupMenu->bEnableIcon_RongJieZanTing = 0;
    g_psSetupMenu->bEnableIcon_YunDuanCeLiang = 0;
    g_psSetupMenu->bCustomizeIcon[0] = 0;
    g_psSetupMenu->bCustomizeIcon[1] = 1;
    g_psSetupMenu->bCustomizeIcon[2] = 2;
    g_psSetupMenu->bCustomizeIcon[3] = 11;
    g_psSetupMenu->bCustomizeIcon[4] = 4;
    g_psSetupMenu->bCustomizeIcon[5] = 5;
    
#endif	
}

#if 1
void Update_gSetupMenuValue(void)
{

	*(gSetupMenuValue + 7) = 0x00000001;                  //SETUP_INIT_BIT;

//	*(gSetupMenuValue + 8)  = (DWORD) g_psSetupMenu->bUsbdMode;
	*(gSetupMenuValue + 9)  =  g_psSetupMenu->wElectrodePos[0];
	*(gSetupMenuValue + 10)  =  g_psSetupMenu->wElectrodePos[1];

    *(gSetupMenuValue + 11)  =  g_psSetupMenu->bEnableIcon_LaLiCeShi;
    *(gSetupMenuValue + 12)  =  g_psSetupMenu->bEnableIcon_DuanMianJianCe;
    *(gSetupMenuValue + 13)  =  g_psSetupMenu->bEnableIcon_ZiDongDuiJiao;
    *(gSetupMenuValue + 14)  =  g_psSetupMenu->bEnableIcon_JiaoDuJianCe;
    *(gSetupMenuValue + 15)  =  g_psSetupMenu->bEnableIcon_BaoCunTuXiang;
    *(gSetupMenuValue + 16)  =  g_psSetupMenu->bEnableIcon_HuiChenJianCe;
    *(gSetupMenuValue + 17)  =  g_psSetupMenu->bEnableIcon_RongJieZanTing;
    *(gSetupMenuValue + 18)  =  g_psSetupMenu->bEnableIcon_YunDuanCeLiang;
    *(gSetupMenuValue + 19)  =  (int)(g_psSetupMenu->bCustomizeIcon[0]);
    *(gSetupMenuValue + 20)  =  (int)(g_psSetupMenu->bCustomizeIcon[1]);
    *(gSetupMenuValue + 21)  =  (int)(g_psSetupMenu->bCustomizeIcon[2]);
    *(gSetupMenuValue + 22)  =  (int)(g_psSetupMenu->bCustomizeIcon[3]);
    *(gSetupMenuValue + 23)  =  (int)(g_psSetupMenu->bCustomizeIcon[4]);
    *(gSetupMenuValue + 24)  =  (int)(g_psSetupMenu->bCustomizeIcon[5]);
	MP_DEBUG("--Write setup value  g_psSetupMenu->wElectrodePos[1]=%d",g_psSetupMenu->wElectrodePos[1] );

}

void Recover_g_psSetupMenu(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);

    if (gSetupMenuValue[7]!=0x00000001)
    {
        mpDebugPrint("Recover_g_psSetupMenu error! %p",gSetupMenuValue[7]);
        return; //Jasmine 4/26 CH: if init bit not equal to 1, then use default value only
    }
 //   if (gSetupMenuValue[8] <= SETUP_MENU_USBD_MODE_UAVC)
 //       g_psSetupMenu->bUsbdMode = (BYTE) gSetupMenuValue[8];
    if (gSetupMenuValue[9] <= 0x0fff)
        g_psSetupMenu->wElectrodePos[0] =  gSetupMenuValue[9];
    if (gSetupMenuValue[10] <= 0x0fff)
        g_psSetupMenu->wElectrodePos[1] =  gSetupMenuValue[10];

    g_psSetupMenu->bEnableIcon_LaLiCeShi = gSetupMenuValue[11];
    g_psSetupMenu->bEnableIcon_DuanMianJianCe = gSetupMenuValue[12];
    g_psSetupMenu->bEnableIcon_ZiDongDuiJiao = gSetupMenuValue[13];
    g_psSetupMenu->bEnableIcon_JiaoDuJianCe = gSetupMenuValue[14];
    g_psSetupMenu->bEnableIcon_BaoCunTuXiang = gSetupMenuValue[15];
    g_psSetupMenu->bEnableIcon_HuiChenJianCe = gSetupMenuValue[16];
    g_psSetupMenu->bEnableIcon_RongJieZanTing = gSetupMenuValue[17];
    g_psSetupMenu->bEnableIcon_YunDuanCeLiang = gSetupMenuValue[18];
    g_psSetupMenu->bCustomizeIcon[0] = (int)(gSetupMenuValue[19]);
    g_psSetupMenu->bCustomizeIcon[1] = (int)(gSetupMenuValue[20]);
    g_psSetupMenu->bCustomizeIcon[2] = (int)(gSetupMenuValue[21]);
    g_psSetupMenu->bCustomizeIcon[3] = (int)(gSetupMenuValue[22]);
    g_psSetupMenu->bCustomizeIcon[4] = (int)(gSetupMenuValue[23]);
    g_psSetupMenu->bCustomizeIcon[5] = (int)(gSetupMenuValue[24]);

	MP_DEBUG("--Read setup value  g_psSetupMenu->wElectrodePos[1]=%d",g_psSetupMenu->wElectrodePos[1] );
}



int PutDefaultSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);
    int retVal = PASS;

    if(Sys_SettingTableOpen() != PASS)
    {
        MP_ALERT("--E-- %s- Can't open setting table.", __FUNCTION__);
        return FAIL;
    }

    Sys_SettingTablePut("MPST", gSetupMenuValue, sizeof(gSetupMenuValue));
    MP_DEBUG("setting.sys put succeed");

    if(retVal = Sys_SettingTableClose() == FAIL)                                // save setting table to system drive
    {
        MP_ALERT("-E- Can't save setting table to system drive!!!");
        return FAIL;
    }
    MP_DEBUG("setting.sys close succeed");
    return PASS;
}


SDWORD GetSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);
    SDWORD retVal = PASS;

    if(Sys_SettingTableOpen() != PASS)
    {
        MP_ALERT("--E-- Can't open setting table.");
        return FAIL;
    }

    MP_DEBUG("setting.sys open succeed");

    if ((retVal = Sys_SettingTableGet("MPST", gSetupMenuValue, sizeof(gSetupMenuValue))) == 0)   // No setting.sys exist, build it
    {
        GetDefaultSetupMenuValue();
        Sys_SettingTablePut("MPST", gSetupMenuValue, sizeof(gSetupMenuValue));
        MP_DEBUG("setting.sys put succeed");

    }

    MP_DEBUG("setting.sys get succeed");

    if(retVal = Sys_SettingTableClose() == FAIL)                                // save setting table to system drive
    {
        MP_ALERT("-E- Can't save setting table to system drive!!!");
        return FAIL;
    }

    MP_DEBUG("setting.sys close succeed");

    Recover_g_psSetupMenu();

    return retVal;
}

SDWORD PutSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);
    SDWORD retVal = PASS;

    Update_gSetupMenuValue();

    if(Sys_SettingTableOpen() != PASS)
    {
        MP_ALERT("--E-- %s- Can't open setting table.", __FUNCTION__);
        return FAIL;
    }

    Sys_SettingTablePut("MPST", gSetupMenuValue, sizeof(gSetupMenuValue));
    MP_DEBUG("setting.sys put succeed");

    if(retVal = Sys_SettingTableClose() == FAIL)                                // save setting table to system drive
    {
        MP_ALERT("-E- Can't save setting table to system drive!!!");
        return FAIL;
    }
    MP_DEBUG("setting.sys close succeed");
    return PASS;
}

#else

void ReadData_To_psSetupMenu(ST_SETUP_MENU *pstSetupMenu)
{
    MP_DEBUG("%s() ", __FUNCTION__);

	if (pstSetupMenu->dwSetupFlag==USER_SET_TAG && pstSetupMenu->dwSetupLenth==sizeof(ST_SETUP_MENU))
	{
		mpDebugPrint("Read setup OK!");
		memcpy((BYTE *)&g_sSetupMenu,pstSetupMenu, sizeof(g_sSetupMenu));
	}
	else
	{
		GetDefaultSetupMenuValue();
	}
}

SDWORD GetSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);
	SDWORD retVal = PASS;
	BYTE *pbBuffer = (BYTE*) ext_mem_malloc(ALIGN_32(sizeof(ST_SETUP_MENU)));


	retVal= IspFunc_ReadUST(pbBuffer, sizeof(ST_SETUP_MENU));
	if (retVal==PASS)
	{
	    ReadData_To_psSetupMenu((ST_SETUP_MENU *)pbBuffer);
	}

    return retVal;
}

SDWORD PutSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);
    //SDWORD retVal = PASS;

	return IspFunc_WriteUST((BYTE *)&g_sSetupMenu, sizeof(g_sSetupMenu));

}

#endif

void PutSetupMenuValueWait(void)
{
    static BYTE st_bIdleTime = 0;

    if (!bSetUpChg)
        return;

    if (g_bAniFlag&& st_bIdleTime<4)                                // for playing music
    {
        AddTimerProc(1000, PutSetupMenuValueWait);
        return;
    }
    bSetUpChg = 0;
    st_bIdleTime=0;;
    PutSetupMenuValue();
}



void SetupFunctionEffect()
{

//	ChangeLanguage(g_bNCTable);
#if AUDIO_ON
	MX6xx_AudioSetVolume(g_bVolumeIndex);
#endif


}

void SetupMenuInit(void)
{
    MP_DEBUG("-%s() -", __FUNCTION__);
	GetDefaultSetupMenuValue();
	GetSetupMenuValue();
	SetupFunctionEffect();


}


void SetupMenuReset(void)
{
	xpgStopAllAction();
	GetDefaultSetupMenuValue();
	WriteSetupChg();
	xpgDelay(10);
	SetupMenuInit();
}

//------------sub function
void WriteSetupChg(void)
{
	bSetUpChg = 1;
	PutSetupMenuValueWait();
}

void TimerToWriteSetup(DWORD dwTime)
{
	Ui_TimerProcAdd(dwTime, WriteSetupChg);
}



