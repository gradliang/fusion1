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

#define						SETTING_NUMBER					256  //��Ӧ�������õ�����

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


typedef struct ST_SETUP_MENU_SETTING_VALUE
{
	DWORD dwSetupFlag; //Must be 1st
	DWORD dwSetupLenth; //Must be 2nd
	DWORD dwSetupChecksum; //Must be 3nd

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
    BYTE bCustomizeIconEnable[6];
    BYTE bCloudMode;
    BYTE bBrightness;                   // Liang Du,  0-100
    BYTE bVolume;                       // Yin Liang, 0 -100 
    BYTE bPreHotEnable;                 // Ԥ��ģʽ
    BYTE bHotUpMode;                    // ����ģʽ
    BYTE bRongJieZhiLiang;              // �۽�����
    BYTE bDuiXianFangShi;               // ���˷�ʽ
    BYTE bPingXianFangShi;              // ���Է�ʽ
    BYTE bCurrFusionMode;               // ��ǰ�۽�ģʽ
    MODEPARAM  SM;                      // SMģʽ����
    MODEPARAM  MM;                      // MMģʽ����
    MODEPARAM  DS;                      // DSģʽ����
    MODEPARAM  NZ;                      // NZģʽ����
    MODEPARAM  BIF;                     // BIFģʽ����
    MODEPARAM  CZ1;                     // CZ1ģʽ����
    MODEPARAM  CZ2;                     // CZ2ģʽ����
    MODEPARAM  AUTO;                    // AUTOģʽ����
    BYTE bReSuGuanSheZhi;               // ���ܹ�����
    WORD wJiaReWenDu;                   // �����¶�
    WORD wJiaReShiJian;                 // ����ʱ��
    WORD wShutdownTime;                 // �ػ�ʱ��
    BYTE b24HourFormat;                 // 24 Сʱ��ʽ��ʱ��
    BYTE bDataFormatMMDDYYYY;           // �Ƿ�MMDDYYYY��ʽ������
    BYTE bSmartBacklight;               // ���ܱ���
    BYTE bAutoShutdown;                 // �Զ��ػ�
    BYTE bToundSoundEnable;             // ��������
    BYTE bLanguage;                     // ����
    BYTE bEnableOpenPassword;           // ������������
    BYTE bEnableHirePassword;           // �����������
    char srtOpenPassword[8];            // ��������  
    char strHirePassword[8];            // �������  
    WORD wLockedTimes;                  // �����۽Ӵ���
    
#endif

    //BYTE bReserved[1];

} ST_SETUP_MENU;


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
    BYTE        bRecordName[10];
    BYTE        bFiberMode;
    BYTE        bFiberL;
    BYTE        bFiberR;
    BYTE        bFiberLoss;
    BYTE        bResult;
    BYTE        bChecksum; // 25
    BYTE        bRecordFileName[11];
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





#endif



