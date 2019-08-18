
// alarm.c
#include "global612.h"
#include "xpg.h" // for g_stXpgMovie DigitClockShow()
#include "utiltypedef.h"
#include "utilregfile.h"
#include "mpTrace.h"
#include "os.h"
#include "rtc.h"
#include "display.h"
#include "taskid.h"
#include "ui.h"

#include "../../main/include/xpgFunc.h"
#include "../../main/include/ui_timer.h"
#include "../../main/include/setup.h"

// Alarm Function ========================================================================

unsigned int alarmOnOff=0; // 0 -OFF, 1 - ON
unsigned int alarmSetupHour=15, alarmSetupMinute=40, alarmSetupSecond=0;
STREAM *sHandle=NULL;
void AlarmProcess(void);
void AlarmIsrProcess()
{
    mpDebugPrint("AlarmIsrProcess()");
    RemoveTimerProc(AlarmProcess);
    DRIVE *drv = DriveGet(DriveCurIdGet());
    //DRIVE *drv = FileBrowserGetCurDrive();
    //if (DirReset(drv)) return;
    mpDebugPrint("DriveCurIdGet()=%d", DriveCurIdGet());
    int rtv = FileSearch(drv, "05", "mp3", E_FILE_TYPE); // FileSearch(drv, bShortNameBuf, bExt, E_FILE_TYPE)
    if(rtv)
    {
        mpDebugPrint("rtv = 0x%x, Alarm music file - 05.mp3 is not exist!", rtv);
        return; // 05.mp3 is not exist!
    }

    sHandle = FileOpen((DRIVE*)FileBrowserGetCurDrive());
    //mpDebugPrint("sHandle = 0x%x", sHandle);
    if(sHandle == NULL)
    {
        mpDebugPrint("sHandle == NULL");
        return;
    }

    Api_AudioHWEnable();
    extern ST_IMGWIN g_sMovieWin;
    XPG_FUNC_PTR *xpgFuncPtr=xpgGetFuncPtr();
    if (Api_EnterMoviePlayer(sHandle, "mp3", NULL, NULL, &g_sMovieWin, NULL, MUSIC_PLAY_MODE, 0))
    {
        mpDebugPrint("Music playing error");
        FileClose(sHandle);
        g_bAniFlag &= ~ANI_AUDIO;
        return;
    }

    g_bAniFlag |= ANI_AUDIO;
    TimerDelay(1);
    Api_MoviePlay();
    mpDebugPrint("Music playing !");
    RTC_AlarmEnableSet(false);
    //Api_AudioHWDisable();
    mpDebugPrint("AlarmIsrProcess() ----> Exit");
}



///
///@ingroup xpgClock
///@brief   Alarm main routine
///
BYTE RtcSettingBuf[5];
void AlarmProcess(void)
{
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    RtcSettingBuf[5] = stSystemTime.u08Second;
    RtcSettingBuf[4] = stSystemTime.u08Minute;
    RtcSettingBuf[3] = stSystemTime.u08Hour;
    RtcSettingBuf[2] = stSystemTime.u08Day;
    RtcSettingBuf[1] = stSystemTime.u08Month;
    RtcSettingBuf[0] = stSystemTime.u16Year - 2000;
    ST_IMGWIN *psWin=Idu_GetCurrWin();
    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite; // // sprite type 43 = SPRITE_TYPE_DIGITCLOCK,
    //WORD DigitClockTab[8] = {40, 130, 220, 310, 400, 490, 580, 670};
    WORD DigitClockTab[8] = {140+0, 140+65, 140+130, 140+195, 140+260, 140+325, 140+390, 140+455};
    int wY=80, wType=46;
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, RtcSettingBuf[3]/10); // Hour
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[0], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, RtcSettingBuf[3]%10); // Hour
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[1], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[2], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, RtcSettingBuf[4]/10); // Minute
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[3], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, RtcSettingBuf[4]%10); // Minute
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[4], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[5], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, RtcSettingBuf[5]/10); // Second
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[6], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, RtcSettingBuf[5]%10); // Second
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitClockTab[7], wY, pstSprite, 0);

    if(alarmOnOff==1) // Alarm On
    {
        //mpDebugPrint("ALARM_ONOFF == ON!");
        if(RtcSettingBuf[3]==alarmSetupHour && RtcSettingBuf[4]==alarmSetupMinute && RtcSettingBuf[5]==alarmSetupSecond)
        {
            mpDebugPrint("ALARM_TIME %d:%d:%d! ##### Play Music ##########", alarmSetupHour, alarmSetupMinute, alarmSetupSecond);
            AlarmIsrProcess(); // Warning - If it runs together with RTC_RegisterRtcIsrCallback(&AlarmIsrProcess);, then it will make StackBomb!
        }
    }
    g_boNeedRepaint = FALSE;
    AddTimerProc(1000, AlarmProcess);
}
///
///@ingroup xpgClock
///@brief   Alarm start routine
///
void AlarmStart(void)
{
    //mpDebugPrint("AlarmStart()");

    ST_IMGWIN *psWin=Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, 800, 480, RGB2YUV(0, 0, 0));

    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;
    WORD AlarmTab[8] = {140+0, 140+65, 140+130, 140+195, 140+260, 140+325, 140+390, 140+455};
    int wY=260, wType=46;

    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupHour/10); // Hour
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[0], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupHour%10); // Hour
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[1], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[2], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupMinute/10); // Minute
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[3], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupMinute%10); // Minute
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[4], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[5], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupSecond/10); // Second
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[6], wY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupSecond%10); // Second
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[7], wY, pstSprite, 0);
    wType=46;
    //pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 12); // ON
    //xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, 0, 380, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 11); // OFF
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, 140+150, wY+110, pstSprite, 0);
    //AlarmProcess();
    AddTimerProc(100, AlarmProcess);
    //mpDebugPrint("AlarmStart() exit");
}

unsigned int alarmSetupIndex=0;
unsigned int alarmCount=0;
///
///@ingroup xpgClock
///@brief   Alarm setup show time user interface
///
void AlarmSetupShowTime(void)
{
    ST_IMGWIN *psWin=Idu_GetCurrWin();
    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;
    WORD alarmTab[8] = {140+0, 140+65, 140+130, 140+195, 140+260, 140+325, 140+390, 140+455};
    int wY=260, wType=46;
    if(alarmSetupIndex==0) {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupHour/10); // Hour
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[0], wY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupHour%10); // Hour
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[1], wY, pstSprite, 0);
    } else if(alarmSetupIndex==1) {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupMinute/10); // Minute
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[3], wY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupMinute%10); // Minute
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[4], wY, pstSprite, 0);
    } else if(alarmSetupIndex==2) {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupSecond/10); // Second
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[6], wY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, alarmSetupSecond%10); // Second
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[7], wY, pstSprite, 0);
    } else if(alarmSetupIndex==3) {
        if(alarmOnOff==1) { // ON
            pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 12); // ON
            xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, 140+150, wY+110, pstSprite, 0);
        } else {
            pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 11); // OFF
            xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, 140+150, wY+110, pstSprite, 0);
        }
    }
}
///
///@ingroup xpgClock
///@brief   Alarm setup main routine, recursive by timer
///
void AlarmSetupShow(void)
{
    //mpDebugPrint("AlarmSetupShow(), alarmCount=%d", alarmCount);
    //mpDebugPrint(",");
    ST_IMGWIN *psWin=Idu_GetCurrWin();
    int wY=260, wType=46;
    //while(1)
    {
        if(alarmCount%2)
        {
            AlarmSetupShowTime();
        } else {
            if(alarmSetupIndex==0)
                Idu_PaintWinArea(psWin, 140+0,   wY, 130, 110, RGB2YUV(0, 0, 0));
            else if(alarmSetupIndex==1)
                Idu_PaintWinArea(psWin, 140+195, wY, 130, 110, RGB2YUV(0, 0, 0));
            else if(alarmSetupIndex==2)
                Idu_PaintWinArea(psWin, 140+390, wY, 130, 110, RGB2YUV(0, 0, 0));
            else if(alarmSetupIndex==3) // ON/OFF
                Idu_PaintWinArea(psWin, 140+130, wY+110, 200, 100, RGB2YUV(0, 0, 0));

        }
        alarmCount++;
        if(alarmCount==10000) alarmCount=0; // loop again
        AddTimerProc(500, AlarmSetupShow);
    }
}
///
///@ingroup xpgClock
///@brief   Alarm Setup event entry
///
void AlarmSetup(void)
{
    //mpDebugPrint("AlarmSetup()");
    Setup_SetNoneXpgMode(SETUP_KEYMAP_ALARM); // in global612.h
    //RemoveTimerProc(AlarmProcess);
    g_boNeedRepaint = FALSE;
    AlarmSetupShowTime();
    AlarmSetupShow();
    //mpDebugPrint("AlarmSetup() exit");
}
///
///@ingroup xpgClock
///@brief   Exit from Alarm
///
void AlarmExit(void)
{
    //mpDebugPrint("AlarmExit()");
    RemoveTimerProc(AlarmProcess);
    xpgStopAudio(); // if AlarmSound play then stop
    //mpDebugPrint("AlarmExit() exit");
}

void AlarmEnableProcess()
{
    MP_DEBUG("AlarmEnableProcess()");

    //mpDebugPrint("alarmSetupHour=%d", alarmSetupHour);
    //mpDebugPrint("alarmSetupMinute=%d", alarmSetupMinute);
    //mpDebugPrint("alarmSetupSecond=%d", alarmSetupSecond);
    ST_SYSTEM_TIME *curr_time;
    SystemTimeGet(curr_time);
    //mpDebugPrint("curr_time->u16Year=%d, curr_time->u08Month=%d, curr_time->u08Day=%d", curr_time->u16Year, curr_time->u08Month, curr_time->u08Day);
    //mpDebugPrint("curr_time->u08Hour=%d, curr_time->u08Minute=%d, curr_time->u08Second=%d", curr_time->u08Hour, curr_time->u08Minute, curr_time->u08Second);
    curr_time->u08Hour = alarmSetupHour;
    curr_time->u08Minute = alarmSetupMinute;
    curr_time->u08Second = alarmSetupSecond+5;
    //mpDebugPrint("curr_time->u08Hour=%d, curr_time->u08Minute=%d, curr_time->u08Second=%d", curr_time->u08Hour, curr_time->u08Minute, curr_time->u08Second);
    SystemTimeAlarmSet(curr_time);
    //RTC_AlarmEnableSet(true);
    //RTC_RegisterRtcIsrCallback(&AlarmIsrProcess);
    //RTC_AlarmIeEnableSet(true);
    //mpDebugPrint("AlarmEnableProcess() ----> Exit");
}

void AlarmDisableProcess()
{
    MP_DEBUG("AlarmDisableProcess()");
}
///
///@ingroup xpgClock
///@brief   Alarm Setup key event UP protine
///
void AlarmSetupUp()
{
    MP_DEBUG("AlarmSetupUp");
    //mpDebugPrint("AlarmSetupUp()");
    switch(alarmSetupIndex)
    {
    case 0 : alarmSetupHour++;   if(alarmSetupHour==24) alarmSetupHour=0; break;
    case 1 : alarmSetupMinute++; if(alarmSetupMinute==60) alarmSetupMinute=0; break;
    case 2 : alarmSetupSecond++; if(alarmSetupSecond==60) alarmSetupSecond=0; break;
    case 3 : if(alarmOnOff==0)
             {
                alarmOnOff++;
                AlarmEnableProcess();
             } else
             {
                 alarmOnOff=0;
                 AlarmDisableProcess();
             }
              break;
    }
}
///
///@ingroup xpgClock
///@brief   Alarm Setup key event DOWN protine
///
void AlarmSetupDown()
{
    MP_DEBUG("AlarmSetupDown");
    //mpDebugPrint("AlarmSetupDown(), alarmSetupIndex=%d", alarmSetupIndex);
    switch(alarmSetupIndex)
    {
    case 0 : if(alarmSetupHour>0) alarmSetupHour--;
             else alarmSetupHour=23;
             break;
    case 1 : if(alarmSetupMinute>0) alarmSetupMinute--;
             else alarmSetupMinute=59;
             break;
    case 2 : if(alarmSetupSecond>0) alarmSetupSecond--;
             else alarmSetupSecond=59;
             break;
    case 3 : if(alarmOnOff==0)
             {
                alarmOnOff = 1;
                AlarmEnableProcess();
             }
             else
             {
                alarmOnOff=0;
                AlarmDisableProcess();
             }
             break;
    }
}
///
///@ingroup xpgClock
///@brief   Alarm Setup key event RIGHT protine
///
void AlarmSetupRight()
{
    MP_DEBUG("AlarmSetupRight");
    //mpDebugPrint("AlarmSetupRight()");
    AlarmSetupShowTime(); // show last setting
    alarmSetupIndex++;
    if(alarmSetupIndex==4) alarmSetupIndex=0;
}
///
///@ingroup xpgClock
///@brief   Alarm Setup key event LEFT protine
///
void AlarmSetupLeft()
{
    MP_DEBUG("AlarmSetupLeft");
    //mpDebugPrint("AlarmSetupLeft()");
    AlarmSetupShowTime(); // show last setting
    if(alarmSetupIndex>0) alarmSetupIndex--;
    else alarmSetupIndex=3;
    if(alarmCount%2) alarmCount++;
}
///
///@ingroup xpgClock
///@brief   Alarm Setup Entry init routine
///
void AlarmSetupEnter()
{
    MP_DEBUG("AlarmSetupEnter");
    //mpDebugPrint("AlarmSetupEnter()");
    AlarmSetupShowTime(); // show last setting
    RemoveTimerProc(AlarmSetupShow);
    Setup_SetXpgMode();
}
///
///@ingroup xpgClock
///@brief   Exit from Alarm Setup
///
void AlarmSetupExit()
{
    MP_DEBUG("AlarmSetupExit");
    //mpDebugPrint("AlarmSetupExit()");
    AlarmSetupShowTime(); // show last setting
    RemoveTimerProc(AlarmSetupShow);
    Setup_SetXpgMode();
}
