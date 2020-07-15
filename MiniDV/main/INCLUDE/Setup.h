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
#define						WELD_RECORD_TITLE_LENTH					11 //4 �۽Ӽ�¼���ⳤ��,��������0
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
    DWORD    dwValueMin;
    DWORD    dwValueData;
    DWORD    dwValueMax;
}DIALOGVALUE;

typedef struct {
    BYTE    bTempreature;
    BYTE    bTime;
}HEATPARAM;
extern HEATPARAM ReSuGuan[5];

typedef struct{
    BYTE    bMode;				/* 0->alll record 3->3days 7->one week */
    WORD    wCurPage;                    
    WORD    wTotalPage;
    DWORD    dwCurIndex;                    
	DWORD dwTotalData;
	//BYTE    bReserve[3];                    
} WeldRecordPage;
extern WeldRecordPage  g_WeldRecordPage;

typedef struct {
    //BYTE        bHead;
    //BYTE        bLenth;
    //BYTE        bIndex;
    BYTE        bYear;
    BYTE        bMonth;
    BYTE        bDay;
    BYTE        bHour;
		
    BYTE        bMinute;
    BYTE        bSecond;
    BYTE        bRecordName[WELD_RECORD_TITLE_LENTH];
    BYTE        bFiberMode;
    BYTE        bFiberL;
    BYTE        bFiberR; // 20bytes
		
    BYTE        bFiberLoss;
    BYTE        bResult; // 0->no result  1->OK  2->FAIL
    //BYTE        bReverse[2];
    DWORD        dwFileIndex; //26 BYTE  index in searchinfo
    //BYTE        bChecksum; // 
}STRECORD;



typedef struct {
//����
    BYTE bRedPenEnable;
    BYTE bRedPenHZ;
    BYTE bRedPenTimerEnable;
    WORD bRedPenTime;
//�¶�ѹ����������
    BYTE bTemperatureInhome[2];
    BYTE bTemperatureOuthome[2];
    BYTE bHumidity;
	//--����ֽ���ǰ��Big-Endian�� ����ֽ���ǰ��Small-Endian��
	//--pbTspiRxBuffer[8]|pbTspiRxBuffer[7],�ȷ���λ->С�˴洢���ϵ͵���Ч�ֽڴ���ڽϵ͵Ĵ洢����ַ���ϸߵ��ֽڴ���ڽϸߵĴ洢����
    WORD wPressure; //pbTspiRxBuffer[8]|pbTspiRxBuffer[7],�ȷ���λ->С�˴洢���ϵ͵���Ч�ֽڴ���ڽϵ͵Ĵ洢����ַ���ϸߵ��ֽڴ���ڽϸߵĴ洢����
//����
    BYTE bChargeStatus;
    BYTE bBatteryQuantity;
    BYTE bStandby;
//�����ź�ǿ��
    BYTE bNetSignal;
//���״̬
    BYTE bHollCover;
    BYTE bHeatCover;
    BYTE bCloudOPMonline;
//����汾
    BYTE bDetectNewVersion; // 0->IDLE 1->��ѯ 2->�޸���  0x8x->��ʾCPU�и��� 0xx8��ʾMCU�и���
    WORD wMcuCurVersion;
    WORD wMcuNewVersion;
    WORD wCpuNewVersion;

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
    BYTE bBrightness;                   // ����,  0-100
    BYTE bVolume;                       // ����, 0 -100 
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
    MODEPARAM  SET1;                    // SET1ģʽ����
    MODEPARAM  SET2;                    // SET2ģʽ����
    MODEPARAM  SET3;                    // SET3ģʽ����
    MODEPARAM  SET4;                    // SET4ģʽ����
    BYTE bReSuGuanSheZhi;               // ���ܹ�����
    WORD wJiaReWenDu;                   // �����¶�
    WORD wJiaReShiJian;                 // ����ʱ��
    WORD wShutdownTime;                 // �ػ�ʱ��
    BYTE b24HourFormat;                 // 24 Сʱ��ʽ��ʱ��
    BYTE bDataFormatMMDDYYYY;           // �Ƿ�MMDDYYYY��ʽ������
    BYTE bLowPowerMode;               // �͵�ģʽ
    BYTE bSleepTime;                 // ����ʱ��  UNIT:min
    BYTE bAutoShutdown;                 // �Զ��ػ�UNIT:min
    BYTE bToundSoundEnable;             // ��������
    BYTE bLanguage;                     // ����
    BYTE bUserMode;                     // �û�ģʽ
    BYTE bEnableOpenPassword;           // ������������
    BYTE bEnableHirePassword;           // �����������
    char srtOpenPassword[8];            // ��������  
    char strHirePassword[8];            // �������  
    BYTE bMachineLockMode;                 //4 ����ģʽ:BIT0->��������  BIT1 ->�������� 
    WORD wLockDateYear;                 // ʹ����������--��
    BYTE bLockDateMonth;                // ʹ����������--��
    BYTE bLockDateDay;                  // ʹ����������--��
    WORD wLockedTimes;                  // �����۽Ӵ���
    WORD wUsedTimes;                  // �۽Ӵ���
    //BYTE bHireTime[3];           			//4 ���ʱ��
    //BYTE bLocked;                     		//4 Զ������
    SDWORD sdwRtcOffset;                  //4 ����ʱ�����ƶ�ʱ�����
    BYTE bMADarry[6];            			//MAD ��
    BYTE bBackGroundLevel[2];            			//4  ��������ͷ�ı�������ֵ
    BYTE bElectrodeInfo[9];            			//4  ���к��뼤������ 6+3
    WORD wElectrodeRemainTimes;            			//4  �����ʣ�����
    DWORD dwWorkTotalTimes;            			//4  �۽��ܴ���
    BYTE bMcuLocalVer[2];            			//4  MCU������ذ汾��
    BYTE bMcuServerVer[2];            			//4  MCU����������˰汾��
    BYTE bCpuServerVer[2];            			//4  CPU����������˰汾��
    char bSensorWinOffsetX;							//4 �۽ӽ�������ͷˮƽƫ��
	char bSensorWinOffsetY;	 						//4 �۽ӽ�������ͷ��ֱƫ��
#endif

    //BYTE bReserved[1];

} ST_SETUP_MENU;
#define						SETUP_STRUCT_CHANGE_TIMES					5  //4     ������g_psSetupMenu�ṹ��󣬰Ѵ���ֵ����1

extern ST_SETUP_MENU *g_psSetupMenu;


SDWORD GetSetupMenuValue(void);
void SetupMenuInit(void);
int LoadRecordFromFile();
int SaveRecordToFile();
void AddRecord(STRECORD* pstRecord);
STRECORD* GetRecord(DWORD dwIndex);
DWORD GetRecordTotal();
void ClearAllRecord();
void InitRecord(STRECORD* pstRecord, WORD year, BYTE month, BYTE day, BYTE hour, BYTE minute, BYTE second, DWORD power, BYTE * recordName, BYTE* fileName);


int initRecordDummyData();      // Jia Shu Ju
void initModeParamDefault(MODEPARAM * pstModeParam,BYTE bIndex);

void SetupSendFlagSet(DWORD dwFlag);
void SetupSendFlagClear(DWORD dwFlag);
void SetupSendFlagSend(void);



#endif



