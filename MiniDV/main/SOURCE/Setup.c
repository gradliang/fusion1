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
    g_psSetupMenu->bCustomizeIconEnable[0] = 1;
    g_psSetupMenu->bCustomizeIconEnable[1] = 1;
    g_psSetupMenu->bCustomizeIconEnable[2] = 1;
    g_psSetupMenu->bCustomizeIconEnable[3] = 1;
    g_psSetupMenu->bCustomizeIconEnable[4] = 1;
    g_psSetupMenu->bCustomizeIconEnable[5] = 1;
    g_psSetupMenu->bPreHotEnable = 0;
    g_psSetupMenu->bHotUpMode = SETUP_MENU_HOT_UP_MODE_AUTO;
    g_psSetupMenu->bRongJieZhiLiang = 0;
    g_psSetupMenu->bDuiXianFangShi = 0;
    g_psSetupMenu->bPingXianFangShi = 0;
    g_psSetupMenu->bCurrFusionMode = 0;
    initModeParamDefault(&(g_psSetupMenu->SM));
    initModeParamDefault(&(g_psSetupMenu->MM));
    initModeParamDefault(&(g_psSetupMenu->DS));
    initModeParamDefault(&(g_psSetupMenu->NZ));
    initModeParamDefault(&(g_psSetupMenu->BIF));
    initModeParamDefault(&(g_psSetupMenu->CZ1));
    initModeParamDefault(&(g_psSetupMenu->CZ2));
    initModeParamDefault(&(g_psSetupMenu->AUTO));
    g_psSetupMenu->bReSuGuanSheZhi = 0;
    g_psSetupMenu->wJiaReWenDu = 50;
    g_psSetupMenu->wJiaReShiJian = 12;
    g_psSetupMenu->wShutdownTime = 10;
    g_psSetupMenu->b24HourFormat = 1;
    g_psSetupMenu->bDataFormatMMDDYYYY = 0;
    g_psSetupMenu->bSmartBacklight = 0;
    g_psSetupMenu->bAutoShutdown = 0;
    g_psSetupMenu->bToundSoundEnable = 0;
    g_psSetupMenu->bLanguage = 0;
    g_psSetupMenu->bEnableOpenPassword = 0;
    g_psSetupMenu->bEnableHirePassword = 0;
    memset(g_psSetupMenu->srtOpenPassword, 0, 8);
    memset(g_psSetupMenu->strHirePassword, 0, 8);
    g_psSetupMenu->wLockedTimes = 200;
    
#endif	
}

#if 1
void Update_gSetupMenuValue(void)
{

	*(gSetupMenuValue + 7) = 0x00000001;                  //SETUP_INIT_BIT;

//	*(gSetupMenuValue + 8)  = (DWORD) g_psSetupMenu->bUsbdMode;
#if  (PRODUCT_UI==UI_WELDING)
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
    *(gSetupMenuValue + 25)  =  g_psSetupMenu->bCustomizeIconEnable[0];
    *(gSetupMenuValue + 26)  =  g_psSetupMenu->bCustomizeIconEnable[1];
    *(gSetupMenuValue + 27)  =  g_psSetupMenu->bCustomizeIconEnable[2];
    *(gSetupMenuValue + 28)  =  g_psSetupMenu->bCustomizeIconEnable[3];
    *(gSetupMenuValue + 29)  =  g_psSetupMenu->bCustomizeIconEnable[4];
    *(gSetupMenuValue + 30)  =  g_psSetupMenu->bCustomizeIconEnable[5];
    *(gSetupMenuValue + 31)  =  g_psSetupMenu->bPreHotEnable;
    *(gSetupMenuValue + 32)  =  g_psSetupMenu->bHotUpMode;
    *(gSetupMenuValue + 33)  =  g_psSetupMenu->bRongJieZhiLiang ;
    *(gSetupMenuValue + 34)  =  g_psSetupMenu->bDuiXianFangShi ;
    *(gSetupMenuValue + 35)  =  g_psSetupMenu->bPingXianFangShi ;
    *(gSetupMenuValue + 36)  =  g_psSetupMenu->bCurrFusionMode;
    memcpy(&gSetupMenuValue[37], &(g_psSetupMenu->SM), 11 * 4);
    memcpy(&gSetupMenuValue[48], &(g_psSetupMenu->MM), 11 * 4);
    memcpy(&gSetupMenuValue[59], &(g_psSetupMenu->DS), 11 * 4);
    memcpy(&gSetupMenuValue[70], &(g_psSetupMenu->NZ), 11 * 4);
    memcpy(&gSetupMenuValue[81], &(g_psSetupMenu->BIF), 11 * 4);
    memcpy(&gSetupMenuValue[92], &(g_psSetupMenu->CZ1), 11 * 4);
    memcpy(&gSetupMenuValue[103], &(g_psSetupMenu->CZ2), 11 * 4);
    memcpy(&gSetupMenuValue[114], &(g_psSetupMenu->AUTO), 11 * 4);
    *(gSetupMenuValue + 125) = g_psSetupMenu->bReSuGuanSheZhi;
    *(gSetupMenuValue + 126) = g_psSetupMenu->wJiaReWenDu;
    *(gSetupMenuValue + 127) = g_psSetupMenu->wJiaReShiJian;
    *(gSetupMenuValue + 128) = g_psSetupMenu->wShutdownTime;
    *(gSetupMenuValue + 129) = g_psSetupMenu->b24HourFormat;
    *(gSetupMenuValue + 130) = g_psSetupMenu->bDataFormatMMDDYYYY;
    *(gSetupMenuValue + 131) = g_psSetupMenu->bSmartBacklight;
    *(gSetupMenuValue + 132) = g_psSetupMenu->bAutoShutdown;
    *(gSetupMenuValue + 133) = g_psSetupMenu->bToundSoundEnable;
    *(gSetupMenuValue + 134) = g_psSetupMenu->bLanguage;
    *(gSetupMenuValue + 135) = g_psSetupMenu->bEnableOpenPassword;
    *(gSetupMenuValue + 136) = g_psSetupMenu->bEnableHirePassword;
    memcpy(gSetupMenuValue + 137, g_psSetupMenu->srtOpenPassword, 8);
    memcpy(gSetupMenuValue + 139, g_psSetupMenu->strHirePassword, 8);
    *(gSetupMenuValue + 141) = g_psSetupMenu->wLockedTimes;
    
	MP_DEBUG("--Write setup value  g_psSetupMenu->wElectrodePos[1]=%d",g_psSetupMenu->wElectrodePos[1] );
#endif

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
#if  (PRODUCT_UI==UI_WELDING)
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
    g_psSetupMenu->bCustomizeIconEnable[0] = gSetupMenuValue[25];
    g_psSetupMenu->bCustomizeIconEnable[1] = gSetupMenuValue[26];
    g_psSetupMenu->bCustomizeIconEnable[2] = gSetupMenuValue[27];
    g_psSetupMenu->bCustomizeIconEnable[3] = gSetupMenuValue[28];
    g_psSetupMenu->bCustomizeIconEnable[4] = gSetupMenuValue[29];
    g_psSetupMenu->bCustomizeIconEnable[5] = gSetupMenuValue[30];
    g_psSetupMenu->bPreHotEnable = gSetupMenuValue[31];
    g_psSetupMenu->bHotUpMode = gSetupMenuValue[32];
    g_psSetupMenu->bRongJieZhiLiang = gSetupMenuValue[33];
    g_psSetupMenu->bDuiXianFangShi = gSetupMenuValue[34];
    g_psSetupMenu->bPingXianFangShi = gSetupMenuValue[35];
    g_psSetupMenu->bCurrFusionMode = gSetupMenuValue[36];
    memcpy(&(g_psSetupMenu->SM), &gSetupMenuValue[37], 11 * 4);
    memcpy(&(g_psSetupMenu->MM), &gSetupMenuValue[48], 11 * 4);
    memcpy(&(g_psSetupMenu->DS), &gSetupMenuValue[59], 11 * 4);
    memcpy(&(g_psSetupMenu->NZ), &gSetupMenuValue[70], 11 * 4);
    memcpy(&(g_psSetupMenu->BIF), &gSetupMenuValue[81], 11 * 4);
    memcpy(&(g_psSetupMenu->CZ1), &gSetupMenuValue[92], 11 * 4);
    memcpy(&(g_psSetupMenu->CZ2), &gSetupMenuValue[103], 11 * 4);
    memcpy(&(g_psSetupMenu->AUTO), &gSetupMenuValue[114], 11 * 4);
    g_psSetupMenu->bReSuGuanSheZhi = gSetupMenuValue[125];
    g_psSetupMenu->wJiaReWenDu = gSetupMenuValue[126];
    g_psSetupMenu->wJiaReShiJian = gSetupMenuValue[127];
    g_psSetupMenu->wShutdownTime = gSetupMenuValue[128];
    g_psSetupMenu->b24HourFormat = gSetupMenuValue[129];
    g_psSetupMenu->bDataFormatMMDDYYYY = gSetupMenuValue[130];
    g_psSetupMenu->bSmartBacklight = gSetupMenuValue[131];
    g_psSetupMenu->bAutoShutdown = gSetupMenuValue[132];
    g_psSetupMenu->bToundSoundEnable = gSetupMenuValue[133];
    g_psSetupMenu->bLanguage = gSetupMenuValue[134];
    g_psSetupMenu->bEnableOpenPassword = gSetupMenuValue[135];
    g_psSetupMenu->bEnableHirePassword = gSetupMenuValue[136];
    memcpy(g_psSetupMenu->srtOpenPassword, &gSetupMenuValue[137], 8);
    memcpy(g_psSetupMenu->strHirePassword, &gSetupMenuValue[139], 8);
    g_psSetupMenu->wLockedTimes = gSetupMenuValue[141];
    
	MP_DEBUG("--Read setup value  g_psSetupMenu->wElectrodePos[1]=%d",g_psSetupMenu->wElectrodePos[1] );
#endif

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



static STRECORD **pstRecordList = NULL;
static DWORD    dwRecordTotal = 0;
static DWORD    dwAllocedSlot = 0;
#define  RECORD_INC_COUNT       200
#define RECORD_TABLE_PATH_1     "record1"
#define RECORD_TABLE_EXT        "sys"
#define RECORD_FLAG             0x8864E423

int LoadRecordFromFile()
{
    DRIVE *sysDrv;
    BYTE  sysDrvId = SYS_DRV_ID;
    DWORD confirm_1, confirm_2;
    DWORD value_1, value_2, updateValue;
    DWORD *ptr32;
    SDWORD retVal = PASS;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(sysDrvId);

    //if ((phyDevID != DEV_NAND) && (phyDevID != DEV_SPI_FLASH))
    if (sysDrvId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID (= %d) defined ! => use current drive instead ...", __FUNCTION__, SYS_DRV_ID);
        sysDrvId = DriveCurIdGet(); /* use current drive */

        if (sysDrvId == NULL_DRIVE)
        {
            MP_ALERT("%s: --E-- Current drive is NULL !!! Abort !!!", __FUNCTION__);
            return FALSE;
        }

        MP_ALERT("Using current drive-%s !!!", DriveIndex2DrvName(sysDrvId));
    }


    {
        STREAM* file_1 = NULL;
        DWORD fileSize;
        DWORD dwFlag;
        BOOL checkTimes = 1;
        STRECORD  record;

STTAB_OPEN_START:
        sysDrv = DriveGet(sysDrvId);
		DirReset(sysDrv);

        // record1.sys
        if (FileSearch(sysDrv, RECORD_TABLE_PATH_1, RECORD_TABLE_EXT, E_FILE_TYPE) != FS_SUCCEED)
        {
            MP_DEBUG("record1.sys is not found in drive-%s !! Create it now.", DriveIndex2DrvName(sysDrvId));
            if (CreateFile(sysDrv, RECORD_TABLE_PATH_1, RECORD_TABLE_EXT) != FS_SUCCEED)
            {
                MP_ALERT("-E- record1.sys in drive-%s can't be created!!!", DriveIndex2DrvName(sysDrvId));
                retVal = FAIL;
                goto _OPEN_END;
            }
        }

        file_1 = FileOpen(sysDrv);
        fileSize = FileSizeGet(file_1);
        if (fileSize < 4)
        {
            goto _OPEN_END;
        }
        

        Fseek(file_1, 0, SEEK_SET);
        if (FileRead(file_1, (BYTE *) &dwFlag, 4) != 4)
        {
            retVal = FAIL;
            goto _OPEN_END;
        }

        if (dwFlag != RECORD_FLAG)
        {
            MP_ALERT("-E- record1.sys flag error.");
            goto _OPEN_END;
        }

        ClearAllRecord();
        
        while (FileRead(file_1, (BYTE *) &record, sizeof(record)) == sizeof(record))
        {
            AddRecord(&record);
        }
        
_OPEN_END:
        if (file_1 != NULL)
            FileClose(file_1);
    }

    return retVal;
}

int SaveRecordToFile()
{
    DRIVE *sysDrv;
    BYTE  sysDrvId = SYS_DRV_ID;
    DWORD confirm_1, confirm_2;
    DWORD value_1, value_2, updateValue;
    DWORD *ptr32;
    SDWORD retVal = PASS;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(sysDrvId);

    //if ((phyDevID != DEV_NAND) && (phyDevID != DEV_SPI_FLASH))
    if (sysDrvId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID (= %d) defined ! => use current drive instead ...", __FUNCTION__, SYS_DRV_ID);
        sysDrvId = DriveCurIdGet(); /* use current drive */

        if (sysDrvId == NULL_DRIVE)
        {
            MP_ALERT("%s: --E-- Current drive is NULL !!! Abort !!!", __FUNCTION__);
            return FALSE;
        }

        MP_ALERT("Using current drive-%s !!!", DriveIndex2DrvName(sysDrvId));
    }


    {
        DWORD i;
        STREAM* file_1 = NULL;
        DWORD fileSize;
        DWORD dwFlag = RECORD_FLAG;
        BOOL checkTimes = 1;

STTAB_OPEN_START:
        sysDrv = DriveGet(sysDrvId);
		DirReset(sysDrv);

        // record1.sys
        if (FileSearch(sysDrv, RECORD_TABLE_PATH_1, RECORD_TABLE_EXT, E_FILE_TYPE) != FS_SUCCEED)
        {
            MP_DEBUG("record1.sys is not found in drive-%s !! Create it now.", DriveIndex2DrvName(sysDrvId));
            if (CreateFile(sysDrv, RECORD_TABLE_PATH_1, RECORD_TABLE_EXT) != FS_SUCCEED)
            {
                MP_ALERT("-E- record1.sys in drive-%s can't be created!!!", DriveIndex2DrvName(sysDrvId));
                retVal = FAIL;
                goto _OPEN_END;
            }
        }

        file_1 = FileOpen(sysDrv);
        
        fileSize = FileSizeGet(file_1);
        if (fileSize < 4)
        {
            goto _OPEN_END;
        }

        if (FileWrite(file_1, &dwFlag, 4) != 4)
        {
            MP_ALERT("write record1.sys error.");
            goto _OPEN_END;
        }

        for (i = 0; i < GetRecordTotal(); i++)
        {
            STRECORD* pstRecord = GetRecord(i);
            FileWrite(file_1, pstRecord, sizeof(STRECORD));
        }
        
_OPEN_END:
        if (file_1 != NULL)
            FileClose(file_1);
    }

    return retVal;
}

void AddRecord(STRECORD* pstRecord)
{
    //mpDebugPrint("AddRecord");
    
    if (pstRecord == NULL)
        return;
    
    if (dwAllocedSlot == 0)
    {
        DWORD dwAllocByte = RECORD_INC_COUNT * sizeof(STRECORD*);
        pstRecordList = (STRECORD**) ext_mem_malloc(dwAllocByte);
        memset(pstRecordList, 0, dwAllocByte);
        dwAllocedSlot = RECORD_INC_COUNT;
    }
    else if (dwRecordTotal >= dwAllocedSlot) 
    {
        STRECORD ** newList;
        DWORD dwAllocByte = (dwAllocedSlot + RECORD_INC_COUNT) * sizeof(STRECORD*);
        newList = (STRECORD**) ext_mem_malloc(dwAllocByte);
        memset(newList, 0, dwAllocByte);
        memcpy(newList, pstRecordList, dwAllocedSlot*sizeof(STRECORD*));
        ext_mem_free(pstRecordList);
        pstRecordList = newList;
        dwAllocedSlot = dwAllocedSlot + RECORD_INC_COUNT;
    }
    ///////////////

    STRECORD * newRecord = (STRECORD*) ext_mem_malloc(sizeof(STRECORD));
    memcpy(newRecord, pstRecord, sizeof(STRECORD));
    pstRecordList[dwRecordTotal] = newRecord;
    dwRecordTotal++;
    
}

STRECORD* GetRecord(DWORD dwIndex)
{
    if (dwIndex >= dwRecordTotal)
        return NULL;
    if (pstRecordList == NULL)
        return NULL;
    
    return pstRecordList[dwIndex];
}


DWORD GetRecordTotal()
{
    return dwRecordTotal;
}

void ClearAllRecord()
{
    mpDebugPrint("ClearAllRecord");
    
    DWORD i;
    for (i = 0; i < dwRecordTotal; i++)
    {
        if (pstRecordList[i] != NULL)
        {
            ext_mem_free(pstRecordList[i]);
            pstRecordList[i] = NULL;
        }
    }
    dwRecordTotal = 0;
}

void InitRecord(STRECORD* pstRecord, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, DWORD power, BYTE * recordName, BYTE* fileName)
{
    pstRecord->wYear = year;
    pstRecord->bMonth = month;
    pstRecord->bDay = day;
    pstRecord->bHour = hour;
    pstRecord->bMinute = minute;
    pstRecord->bSecond = second;
    pstRecord->noused = 0;
    pstRecord->dwPowerWaste = power;
    strncpy(pstRecord->bRecordName, recordName, sizeof(pstRecord->bRecordName)-1);
    pstRecord->bRecordName[sizeof(pstRecord->bRecordName)-1] = 0;
    strncpy(pstRecord->bRecordFileName, fileName, sizeof(pstRecord->bRecordFileName)-1);
    pstRecord->bRecordFileName[sizeof(pstRecord->bRecordFileName)-1] = 0;
}

int initRecordDummyData()
{
    STRECORD  record;
    
    InitRecord(&record, 2017, 2, 28, 1, 59, 59, (0<<6) | 1, "REC 1", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2017, 3, 28, 1, 59, 59, (0<<6) | 1, "REC 2", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2017, 4, 28, 1, 59, 59, (0<<6) | 1, "REC 3", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2017, 5, 28, 1, 59, 59, (0<<6) | 1, "REC 4", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2017, 6, 28, 1, 59, 59, (0<<6) | 1, "REC 5", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2017, 7, 28, 1, 59, 59, (0<<6) | 1, "REC 6", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2017, 8, 28, 1, 59, 59, (0<<6) | 1, "REC 7", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2018, 2, 28, 1, 59, 59, (0<<6) | 1, "REC 8", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2018, 5, 28, 1, 59, 59, (0<<6) | 1, "REC 9", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2018, 7, 28, 1, 59, 59, (0<<6) | 1, "REC 10", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2019, 9, 28, 1, 59, 59, (0<<6) | 1, "REC 11", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2019, 12, 28, 1, 59, 59, (0<<6) | 1, "REC 12", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 2019, 12, 29, 1, 59, 59, (0<<6) | 1, "REC 13", "aaa.jpg");
    AddRecord(&record);
}


void initModeParamDefault(MODEPARAM * pstModeParam)
{
    pstModeParam->fangDianZhongXin = 323;
    pstModeParam->rongJieDianYa = 1010;
    pstModeParam->yuRongDianYa = 10;
    pstModeParam->chuChenDianYa = 120;
    pstModeParam->rongJieChongDieLiang = 26;
    pstModeParam->duiJiaoMuBiaoZhi = 40;
    pstModeParam->rongJieShiJian = 3000;
    pstModeParam->yuRongShiJian = 200;
    pstModeParam->chuChenShiJian = 200;
    pstModeParam->qieGeJiaoDuShangXian = (3<<6)|0;
    pstModeParam->fangDianJiaoZhengMuBiaoZhi = 140;
}



