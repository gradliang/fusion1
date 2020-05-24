#ifndef SETUP_H
#define SETUP_H

#include "iplaysysconfig.h"
#include "camcorder_func.h"
#include "camera_func.h"

// for Setup menu setting value
enum{
    SETUP_MENU_MUSIC_REPEAT_ONE     = 0,
    SETUP_MENU_MUSIC_REPEAT_ALL     = 1,
    SETUP_MENU_MUSIC_REPEAT_OFF     = 2,
    SETUP_MENU_MUSIC_REPEAT_ALLONCE = 3,
};

enum{
    SETUP_MENU_PHOTO_FULL_SCREEN_ON         = 0,
    SETUP_MENU_PHOTO_FULL_SCREEN_OFF        = 1,
    SETUP_MENU_PHOTO_FULL_SCREEN_DEFAULT    = 2,
};

enum{
    SETUP_MENU_BackgoundMUSIC_State_OFF     = 0,
    SETUP_MENU_BackgoundMUSIC_State_ON      = 1,
};

enum{
    SETUP_MENU_MOVIE_REPEAT_ONE  = 0,
    SETUP_MENU_MOVIE_REPEAT_ALL  = 1,
    SETUP_MENU_MOVIE_REPEAT_OFF  = 2,
    SETUP_MENU_MOVIE_REPEAT_ALLONCE = 3,
};

enum{
    SETUP_MENU_SLIDE_SHOW_INTERVAL_FAST     = 0,
    SETUP_MENU_SLIDE_SHOW_INTERVAL_NORMAL   = 1,
    SETUP_MENU_SLIDE_SHOW_INTERVAL_SLOW     = 2,
};

enum{
    SETUP_MENU_VIDEO_FORWARD_SECOND_2   = 0,
    SETUP_MENU_VIDEO_FORWARD_SECOND_4   = 1,
    SETUP_MENU_VIDEO_FORWARD_SECOND_8   = 2,
    SETUP_MENU_VIDEO_FORWARD_SECOND_16  = 3,
    SETUP_MENU_VIDEO_FORWARD_SECOND_32  = 4,
};

enum{
    SETUP_MENU_MUSIC_LYRIC_GBK       = 0,
    SETUP_MENU_MUSIC_LYRIC_BIG5      = 1,
    SETUP_MENU_MUSIC_LYRIC_UNICODE   = 2,
};

enum{
    SETUP_MENU_SLIDE_SHOW_MODE_MANUAL = 0,
    SETUP_MENU_SLIDE_SHOW_MODE_AUTO   = 1,
};

enum{
    SETUP_MENU_ON    = 0,
    SETUP_MENU_OFF   = 1,
};

enum{
    SETUP_MENU_SLIDESEPARATE_ON     = 0,
    SETUP_MENU_SLIDESEPARATE_OFF    = 1,
};

enum{
    SETUP_MENU_VIDEO_FULL_SCREEN_ON  = 0,
    SETUP_MENU_VIDEO_FULL_SCREEN_OFF = 1,
};

enum{
    SETUP_MENU_USBD_MODE_MSDC       = 0,
    SETUP_MENU_USBD_MODE_SIDC       = 1,
    SETUP_MENU_USBD_MODE_EXTERN    = 2, // Side Monitor
    SETUP_MENU_USBD_DYNAMIC             = 3,
    SETUP_MENU_USBD_MODE_UAVC		= 4,
};


enum{
    SETUP_MENU_CROP_ORIGINAL = 0,
    SETUP_MENU_CROP_OPTIMAL  = 1,
};

enum{
    SETUP_MENU_AV_LCD = 0,
    SETUP_MENU_AV_INPUT = 1,
    SETUP_MENU_AV_OUTPUTNTSC = 2,
    SETUP_MENU_AV_OUTPUTPAL = 3,
};

enum {
    SETUP_MENU_HOT_UP_MODE_AUTO = 0,
    SETUP_MENU_HOT_UP_MODE_MANUAL = 1,
};

#define						SETTING_NUMBER					256  //对应下面设置的数量

typedef struct {
    unsigned int    fangDianZhongXin;
    unsigned int    rongJieDianYa;
    unsigned int    yuRongDianYa;
    unsigned int    chuChenDianYa;
    unsigned int    rongJieChongDieLiang;
    unsigned int    duiJiaoMuBiaoZhi;
    unsigned int    rongJieShiJian;
    unsigned int    yuRongShiJian;
    unsigned int    chuChenShiJian;
    unsigned int    qieGeJiaoDuShangXian;
    unsigned int    fangDianJiaoZhengMuBiaoZhi;
}MODEPARAM;

typedef struct {
//红光笔
    BYTE bRedPenEnable;
    BYTE bRedPenHZ;
    BYTE bRedPenTimerEnable;
    WORD wRedPenTime;
//温度压力电量环境亮度
    BYTE bTemperatureInhome[2];
    BYTE bTemperatureOuthome[2];
    BYTE bHumidity;
    BYTE bPressure[2];
    BYTE bBattery;

}ST_UNSAVE_PARAM;
extern ST_UNSAVE_PARAM *g_psUnsaveParam;



typedef struct ST_SETUP_MENU_SETTING_VALUE
{
	DWORD dwSetupChecksum; //Must be 1nd
	DWORD dwSetupFlag; 
	DWORD dwSetupLenth; 
	DWORD dwSetupVersion;

    // USBD mode
    BYTE bUsbdMode;

	// user data
    //BYTE cameraPhotoNumber;                 // 1,3,5
    //BYTE RecordMovieLenth;                 // 1,3,5
    //BYTE sleepmode;
    //BYTE language;                          // 
#if  (PRODUCT_UI==UI_SURFACE)
    BYTE bReferPhoto;
	BYTE bBackUp;
	BYTE bBackDown;
	BYTE bPhotoUp;
	BYTE bPhotoDown;
#endif
#if  (PRODUCT_UI==UI_WELDING)
    WORD wElectrodePos[2];
#if 0
    BYTE bEnableIcon_LaLiCeShi;
    BYTE bEnableIcon_DuanMianJianCe;
    BYTE bEnableIcon_ZiDongDuiJiao;
    BYTE bEnableIcon_JiaoDuJianCe;
    BYTE bEnableIcon_BaoCunTuXiang;
    BYTE bEnableIcon_HuiChenJianCe;
    BYTE bEnableIcon_RongJieZanTing;
    BYTE bEnableIcon_YunDuanCeLiang;
#endif
    char bCustomizeIcon[6];
    BYTE bFunctionIconEnable[12];
    BYTE bCloudMode;
    BYTE bBrightness;                   // 亮度,  0-100
    BYTE bVolume;                       // 音量, 0 -100 
    BYTE bPreHotEnable;                 // 预热模式
    BYTE bHotUpMode;                    // 加热模式
    BYTE bRongJieZhiLiang;              // 熔接质量
    BYTE bDuiXianFangShi;               // 对纤方式
    BYTE bPingXianFangShi;              // 屏显方式
    BYTE bCurrFusionMode;               // 当前熔接模式
    MODEPARAM  SM;                      // SM模式参数
    MODEPARAM  MM;                      // MM模式参数
    MODEPARAM  DS;                      // DS模式参数
    MODEPARAM  NZ;                      // NZ模式参数
    MODEPARAM  BIF;                     // BIF模式参数
    MODEPARAM  CZ1;                     // CZ1模式参数
    MODEPARAM  CZ2;                     // CZ2模式参数
    MODEPARAM  AUTO;                    // AUTO模式参数
    BYTE bReSuGuanSheZhi;               // 热塑管设置
    WORD wJiaReWenDu;                   // 加热温度
    WORD wJiaReShiJian;                 // 加热时间
    WORD wShutdownTime;                 // 关机时间
    BYTE b24HourFormat;                 // 24 小时格式的时间
    BYTE bDataFormatMMDDYYYY;           // 是否MMDDYYYY格式的日期
    BYTE bSmartBacklight;               // 智能背光
    BYTE bAutoShutdown;                 // 自动关机
    BYTE bToundSoundEnable;             // 触屏声音
    BYTE bLanguage;                     // 语言
    BYTE bUserMode;                     // 用户模式
    BYTE bEnableOpenPassword;           // 开启开机密码
    BYTE bEnableHirePassword;           // 开启租借密码
    char srtOpenPassword[8];            // 开机密码  
    char strHirePassword[8];            // 租借密码  
    BYTE bLockDateMode;                 // 使用锁定日期的模式(为0时表示使用锁定次数的模式)
    WORD wLockDateYear;                 // 使用锁定日期--年
    BYTE bLockDateMonth;                // 使用锁定日期--月
    BYTE bLockDateDay;                  // 使用锁定日期--日
    WORD wLockedTimes;                  // 锁定熔接次数
    WORD wUsedTimes;                  // 熔接次数
    BYTE bHireTime[3];           			//4 租借时间
    BYTE bLocked;                     		//4 远程锁定
    SDWORD sdwRtcOffset;                  //4 本地时间与云端时间差异
    BYTE bMADarry[6];            			//MAD 码
    BYTE bBackGroundLevel[2];            			//4  两个摄像头的背景亮度值
    BYTE bElectrodeInfo[15];            			//4  电击棒信息
    
#endif

    //BYTE bReserved[1];

} ST_SETUP_MENU;
#define						SETUP_STRUCT_CHANGE_TIMES					5  //4     更改了g_psSetupMenu结构体后，把此数值递增1

extern ST_SETUP_MENU *g_psSetupMenu;


SDWORD GetSetupMenuValue(void);
void SetupMenuInit(void);



typedef struct Record1 {
    BYTE        bHead;
    BYTE        bLenth;
    BYTE        bIndex;
    BYTE        bYear;
    BYTE        bMonth;
    BYTE        bDay;
    BYTE        bHour;
    BYTE        bMinute;
    BYTE        bSecond;
    BYTE        bRecordName[12];
    BYTE        bFiberMode;
    BYTE        bFiberL;
    BYTE        bFiberR;
    BYTE        bFiberLoss;
    BYTE        bResult;
    WORD        wFileIndex;
    BYTE        bChecksum; // 25
}STRECORD;


int LoadRecordFromFile();
int SaveRecordToFile();
void AddRecord(STRECORD* pstRecord);
STRECORD* GetRecord(DWORD dwIndex);
DWORD GetRecordTotal();
void ClearAllRecord();
void InitRecord(STRECORD* pstRecord, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, DWORD power, BYTE * recordName, BYTE* fileName);


int initRecordDummyData();      // Jia Shu Ju
void initModeParamDefault(MODEPARAM *);

void SetupSendFlagSet(DWORD dwFlag);
void SetupSendFlagClear(DWORD dwFlag);
void SetupSendFlagSend(void);



#endif



