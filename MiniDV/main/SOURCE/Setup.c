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
#include "xpgProcSensorData.h"

BOOL bSetUpChg = false;
static ST_SETUP_MENU g_sSetupMenu;
ST_SETUP_MENU *g_psSetupMenu = &g_sSetupMenu;
static ST_UNSAVE_PARAM g_sUnsaveParam;
ST_UNSAVE_PARAM *g_psUnsaveParam = &g_sUnsaveParam;

//DWORD gSetupMenuValue[SETTING_NUMBER+8];
void ResetUnsaveparam(void)
{
	memset(&g_sUnsaveParam,0, sizeof(g_sUnsaveParam));
}

//--下面三个函数要对应增加或改动
void GetDefaultSetupMenuValue(void)
{
    MP_DEBUG("%s() ", __FUNCTION__);

	memset(&g_sSetupMenu,0, sizeof(g_sSetupMenu));

	g_psSetupMenu->dwSetupFlag=USER_SET_TAG;
	g_psSetupMenu->dwSetupVersion=SETUP_STRUCT_CHANGE_TIMES;
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
#if 0
    g_psSetupMenu->bEnableIcon_LaLiCeShi = 0;
    g_psSetupMenu->bEnableIcon_DuanMianJianCe = 0;
    g_psSetupMenu->bEnableIcon_ZiDongDuiJiao = 0;
    g_psSetupMenu->bEnableIcon_JiaoDuJianCe = 0;
    g_psSetupMenu->bEnableIcon_BaoCunTuXiang = 0;
    g_psSetupMenu->bEnableIcon_HuiChenJianCe = 0;
    g_psSetupMenu->bEnableIcon_RongJieZanTing = 0;
    g_psSetupMenu->bEnableIcon_YunDuanCeLiang = 0;
#endif
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
	g_psSetupMenu->bVolume=8;
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
    g_psSetupMenu->wLockedTimes = 0;
    g_psSetupMenu->sdwRtcOffset = 0;
    memset(g_psSetupMenu->bMADarry, 0, 6);
    memset(g_psSetupMenu->bBackGroundLevel, 0, 2);
    
#endif	
}

#if 1
void Update_gSetupMenuValue(void)
{
	DWORD i;
	BYTE *pbdata=(BYTE *)&g_sSetupMenu;

	g_psSetupMenu->dwSetupChecksum=0;
	for (i=4;i<sizeof(ST_SETUP_MENU);i++)
		g_psSetupMenu->dwSetupChecksum+=pbdata[i];
    MP_DEBUG("Update checksum %p  size=%d",g_psSetupMenu->dwSetupChecksum,sizeof(g_sSetupMenu));

}

void Recover_g_psSetupMenu(void)
{
	DWORD i,dwCheckSum;
	BYTE *pbdata=(BYTE *)&g_sSetupMenu;

    MP_DEBUG("%s() ", __FUNCTION__);

    if (g_psSetupMenu->dwSetupFlag!=USER_SET_TAG||g_psSetupMenu->dwSetupVersion!=SETUP_STRUCT_CHANGE_TIMES)
    {
        mpDebugPrint("Recover_g_psSetupMenu error! falg:%p version %d/%d",g_psSetupMenu->dwSetupFlag,g_psSetupMenu->dwSetupVersion,SETUP_STRUCT_CHANGE_TIMES);
        GetDefaultSetupMenuValue();
    }
	else
	{
		dwCheckSum=0;
		for (i=4;i<sizeof(ST_SETUP_MENU);i++)
			dwCheckSum+=pbdata[i];
		if (dwCheckSum!=g_psSetupMenu->dwSetupChecksum)
		{
	        mpDebugPrint("Recover_g_psSetupMenu Checksum error! %p/%p",dwCheckSum,g_psSetupMenu->dwSetupChecksum);
	        GetDefaultSetupMenuValue();
		}
	}


    if (g_psSetupMenu->wElectrodePos[0]  > 0x0fff)
        g_psSetupMenu->wElectrodePos[0] =  0;
    if (g_psSetupMenu->wElectrodePos[1] > 0x0fff)
        g_psSetupMenu->wElectrodePos[1] =  0;
	if (g_psSetupMenu->bVolume>15)
		g_psSetupMenu->bVolume=15;

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

    Sys_SettingTablePut("MPST", &g_sSetupMenu, sizeof(ST_SETUP_MENU));
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

    if ((retVal = Sys_SettingTableGet("MPST", &g_sSetupMenu, sizeof(ST_SETUP_MENU))) == 0)   // No setting.sys exist, build it
    {
        GetDefaultSetupMenuValue();
        Sys_SettingTablePut("MPST", &g_sSetupMenu, sizeof(ST_SETUP_MENU));
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

    Sys_SettingTablePut("MPST", &g_sSetupMenu, sizeof(ST_SETUP_MENU));
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
	ResetUnsaveparam();
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


//>------------Send setup data to mcu
static DWORD st_dwSetupSendFlag=0,st_dwSetupResendTimes=0;


SWORD  SetupSendTouchVoice(void)
{
	BYTE bTxData[8];

	bTxData[0]=0xab;
	bTxData[1]=3+1;
	if (g_psSetupMenu->bToundSoundEnable)
		bTxData[2]=g_psSetupMenu->bVolume;
	else
		bTxData[2]=0;
	return TSPI_PacketSend(bTxData,0);
}

SWORD  SetupSendSmartBacklight(void)
{
	BYTE bTxData[8];

	bTxData[0]=0xA9;
	bTxData[1]=3+2;
	bTxData[2]=0x07;
	bTxData[3]=g_psSetupMenu->bSmartBacklight;
	return TSPI_PacketSend(bTxData,0);
}

SWORD  SetupSendCloudOnOff(void)
{
	BYTE bTxData[8];

	bTxData[0]=0xA9;
	bTxData[1]=3+2;
	bTxData[2]=0x02;
	bTxData[3]=g_psSetupMenu->bCloudMode;
	return TSPI_PacketSend(bTxData,0);
}

SWORD  SetupSendRedPen(void)
{
	BYTE bTxData[8];

	bTxData[0]=0x95;
	bTxData[1]=3+3;
	bTxData[2]=g_psUnsaveParam->bRedPenEnable;
	bTxData[3]=g_psUnsaveParam->bRedPenHZ;
	if (g_psUnsaveParam->bRedPenTimerEnable)
		bTxData[4]=g_psUnsaveParam->wRedPenTime;
	else
		bTxData[4]=0;
	return TSPI_PacketSend(bTxData,0);
}

SWORD  SetupSendHot(void)
{
	BYTE bTxData[8];

	bTxData[0]=0x96;
	bTxData[1]=3+3;
	bTxData[2]=g_psSetupMenu->bPreHotEnable>0;
	if (g_psSetupMenu->bHotUpMode)
		bTxData[2] |=BIT1;
	bTxData[2] |=g_psSetupMenu->bReSuGuanSheZhi<<4;
	bTxData[3]=g_psSetupMenu->wJiaReWenDu;
	bTxData[4]=g_psSetupMenu->wJiaReShiJian;
	return TSPI_PacketSend(bTxData,0);
}

#pragma alignvar(4)
SWORD(*SetupSendFunctions[]) (void) =
{
	SetupSendHot,
	SetupSendRedPen,
	SetupSendCloudOnOff,
	SetupSendSmartBacklight,
	SetupSendTouchVoice,
	NULL
};
//SETUP_SEND_MAXNUM

void SetupSendFlagSend(void)
{
	BYTE i;
	
	for (i=0;i<32;i++)
	{
		if (!(st_dwSetupSendFlag &(1<<i)))
			continue;
		if (i>=SETUP_SEND_MAXNUM)
			break;
		if (SetupSendFunctions[i] !=NULL)
		{
			if ((*SetupSendFunctions[i])()==PASS)
					SetupSendFlagClear(i);
				
		}
	}

	if (st_dwSetupSendFlag)
	{
		st_dwSetupResendTimes--;
		if (st_dwSetupResendTimes)
			Ui_TimerProcAdd(500, SetupSendFlagSend);
	}
}

void SetupSendFlagSet(DWORD dwFlag)
{
		st_dwSetupSendFlag|=(1<<dwFlag);
		st_dwSetupResendTimes=SETUP_RESET_TIMES;
		SetupSendFlagSend();
}

void SetupSendFlagClear(DWORD dwFlag)
{
		st_dwSetupSendFlag&=~(1<<dwFlag);
}


//<------------------

//------------sub function
void WriteSetupChg(void)
{
	bSetUpChg = 1;
	PutSetupMenuValueWait();

	if (!SystemGetStatus(SYS_STATUS_INIT))
	{
	    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

		if (dwHashKey == xpgHash("FusionSet3"))
		{
			SetupSendFlagSet(SETUP_SEND_HOT);
		}
		else if (dwHashKey == xpgHash("RedLight"))
		{
			SetupSendFlagSet(SETUP_SEND_REDPEN);
		}
		else if (dwHashKey == xpgHash("SetYun"))
		{
			SetupSendFlagSet(SETUP_SEND_CLOUDONOFF);
		}
		else if (dwHashKey == xpgHash("SetSleep"))
		{
			SetupSendFlagSet(SETUP_SEND_SMARTBACKLIGHT);
		}
		else if (dwHashKey == xpgHash("SetSound"))
		{
			SetupSendFlagSet(SETUP_SEND_TOUCHVOICE);
		}
	}
}


void SendUnsaveParam(void)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

	if (dwHashKey == xpgHash("RedLight"))
	{
		SetupSendFlagSet(SETUP_SEND_REDPEN);
	}
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

void InitRecord(STRECORD* pstRecord, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, DWORD bLoss, BYTE * recordName, BYTE* fileName)
{
    pstRecord->bYear = year;
    pstRecord->bMonth = month;
    pstRecord->bDay = day;
    pstRecord->bHour = hour;
    pstRecord->bMinute = minute;
    pstRecord->bSecond = second;
    pstRecord->bFiberLoss = bLoss;
    strncpy(pstRecord->bRecordName, recordName, sizeof(pstRecord->bRecordName)-1);
    pstRecord->bRecordName[sizeof(pstRecord->bRecordName)-1] = 0;
    strncpy(pstRecord->bRecordFileName, fileName, sizeof(pstRecord->bRecordFileName)-1);
    pstRecord->bRecordFileName[sizeof(pstRecord->bRecordFileName)-1] = 0;
}

int initRecordDummyData()
{
    STRECORD  record;
    
    InitRecord(&record, 17, 2, 28, 1, 59, 59, (0<<6) | 1, "REC 1", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 17, 3, 28, 1, 59, 59, (0<<6) | 1, "REC 2", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 17, 4, 28, 1, 59, 59, (0<<6) | 1, "REC 3", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 17, 5, 28, 1, 59, 59, (0<<6) | 1, "REC 4", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 17, 6, 28, 1, 59, 59, (0<<6) | 1, "REC 5", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 17, 7, 28, 1, 59, 59, (0<<6) | 1, "REC 6", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 17, 8, 28, 1, 59, 59, (0<<6) | 1, "REC 7", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 18, 2, 28, 1, 59, 59, (0<<6) | 1, "REC 8", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 18, 5, 28, 1, 59, 59, (0<<6) | 1, "REC 9", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 18, 7, 28, 1, 59, 59, (0<<6) | 1, "REC 10", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 19, 9, 28, 1, 59, 59, (0<<6) | 1, "REC 11", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 19, 12, 28, 1, 59, 59, (0<<6) | 1, "REC 12", "aaa.jpg");
    AddRecord(&record);
    InitRecord(&record, 19, 12, 29, 1, 59, 59, (0<<6) | 1, "REC 13", "aaa.jpg");
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



