#ifndef __XPG__PROC_SENSOR__H__
#define __XPG__PROC_SENSOR__H__

#include "global612.h"




//----debug switch
#define   ALIGN_DEMO_MODE											1 //4 ������ʾ���������ɵ�������ƶ��ľ��뼰�ŵ�ʱ��
#define   DEBUG_POS_DATA												0

#define   ALIGN_NEW_MODE											1//4   �ӹ��˶�������Ѱ���������λ��
#define   ALIGN_FAST_MODE											1//4  ���߹���ͬʱ�ƶ�,��ȷ�ƶ�

#define   FIBER_DISPLAY_IN_SAME_SIDE					1//4  ������������������ʾ����ͬһ������

enum {
    SENSOR_IDLE,
    SENSOR_PROC_INIT,
    SENSOR_FACE_POS1A,
    SENSOR_FACE_POS1B,
    SENSOR_DISCHARGE1,
    SENSOR_AUTO_FOCUS,
    SENSOR_GET_ANGLE,
    SENSOR_FACE_POS2A,	//TOP SENSOR
    SENSOR_FACE_POS2B,  // BOTTOM  SENSOR
    SENSOR_ALIGN_H1A,
    SENSOR_ALIGN_H1B,
    SENSOR_ALIGN_H2A,
    SENSOR_ALIGN_H2B,
    SENSOR_ALIGN_H3A,
    SENSOR_ALIGN_H3B,
    SENSOR_PAUSE,
    SENSOR_DISCHARGE2,
    SENSOR_DISCHARGE3,
    SENSOR_GET_LOSS,
    SENSOR_RPOC_END,
    SENSOR_WELD_TOTAL,
};

//g_swAutoFocusState
enum {
    AF_OFF,
    AF_INIT,
    AF_PROC,
    AF_FINISH,
};

enum {
    GET_CENTER_OFF,
    GET_CENTER_INIT,
    GET_CENTER_WHOLE_FIBER,
    GET_CENTER_LOW_POINT,
    GET_CENTER_FINISH,
};

enum {
    SENSER_TOP,
    SENSER_BOTTOM,
    SENSER_TOTAL,
};

#define		FIBER_EDGE_LEVEL					0x10 //4   �ӱ��������˵� �ڰױ߽�����ֵ
#define		X_START										8       //4 x����ʼɨ���
#define		X_STEP											16       //4 ���ٺ�����Ҳ���������Ϊ4�ı�����һ����λΪ�������
#define		Y_VALID_PIXEL							38       //4 ��С��ߺڱ߸߶� //really about 40  ,white <30   //ʵ�ʰ�߹���Լ90������
//#define		Y_LAST_SUM									4       //4 ǰ���ƽ��ͳ���� must 4
#define		Y_CONTINUE_VALID_SUM			4       //4 Y����������Ч��
#define		Y_CENTER_CONTINUE_VALID_SUM			4       //4 Y��������������Ч��
#define		X_SCAN_MAX_SUM								(800/X_STEP)       //4 ˮƽ��ͳ�����ֵ max=width/X_STEP
#define		X_SCAN_VALID_NUM							20       //4 ˮƽ��ͳ����Чֵ���������ֵ�Ϳ�ʼ���� 
#define		FACEX_SHAKE										2         //4 FACE����
#define		Y_SHAKE										1         //4 ˮƽ���

#define		Y_VALID_OFFSET						3       //4 ˮƽ��ͳ��������Y����ƫ�����ص�

#define		CENTER_MIN_PIXEL					10       //4 �м������С�߶� //really about 13 
#define		LEVEL_OFFSET								12         //4 ����ƫ��

#define		LEVELNESS										4         //4 ˮƽ���ƫ��
#define		ANGEL_OFFSET							180 //160         //4 �Ƕ����ƫ��
#define		CENTER_W_OFFSET					300 // 100         //4 ���м��ɫ�������ƫ��

//--g_dwProcWinFlag  �������յȱ�־λ
#define	WIN0_CAPTURE_FLAG										BIT0

//--st_bFillWinFlag st_bNeedFillProcWin
#define	FILL_WIN_UP														BIT0
#define	FILL_WIN_DOWN													BIT1
#define	FILL_WIN_UP_WAIT											BIT2
#define	FILL_WIN_DOWN_WAIT										BIT3

#define	FILL_WIN_END														BIT6
#define	FILL_WIN_INIT													BIT7

 // 0->����  1->���� 2->���� 3->����
#define	FIBER_LEFT_TOP													0
#define	FIBER_RIGHT_TOP												1
#define	FIBER_LEFT_BOTTOM											2
#define	FIBER_RIGHT_BOTTOM										3
#define	FIBER_WINDOW_NUM											4

#define	FIBER_LEFT															0
#define	FIBER_RIGHT														1
#define	FIBER_NUMBER													2

#define	SENSOR_UP															0
#define	SENSOR_DOWN														1
#define	SENSOR_NUMBER													2

//  motor index
#define	MOTOR_H_LEFT													1
#define	MOTOR_H_RIGHT													2
#define	MOTOR_V_UP														4
#define	MOTOR_V_DOWN													3
#define	MOTOR_AF_UP														5
#define	MOTOR_AF_DOWN												6

//  motor status bit
#define	MOTOR_HL_BIT													0x01
#define	MOTOR_HR_BIT													0x02
#define	MOTOR_VU_BIT													0x08 // MOTOR_V_UP
#define	MOTOR_VD_BIT													0x04


#define	MOTOR_FB																0
#define	MOTOR_FF																1


#define	MOTOR_STOP														0
#define	MOTOR_NO_HOLD												1
#define	MOTOR_HOLD														2
#define	MOTOR_HOLD_TIMEOUT										30*1000

//���ˮƽƫ��
#define   MOTOR_HORI_MAX_PULSE								66000 // really 67000
#define   MOTOR_HORI_REVERSE_PULSE						300  // 200
//��ֱ���3         
#define   MOTOR_V_REVERSE_PULSE								400  // 
#define   MOTOR_V_MIN_SPEED										45//70  // 1000pulse->7-12pixel ==142

//  �ۼƶ��ٴδ�ֱ���仯����
#define	VMOTOR_CNT														3 
//  �ۼƶ��ٸ��ڱ���Ч��
#define	PIXEL_BLACK_CNT												10
//�������������
#define   POS_RETRY_TIMES											10
#define   ALIGN_HORIZONTAL_REDO_TIMES				2
#define   RETRY_TIMEOUT												10000// ms
//���ˮƽƫ��
#define   HORI_OFFSET														3

//���������趨
#define   BG_BRIGHT_SHIFT											0

//string lenth
#define	POS_STR_LEN														16

#define  RECORD_INC_COUNT       									200
#define LOCAL_OPM_RECORD_FILE_NAME     				"loopmRec"
#define LOCAL_OPM_RECORD_FILE_EXT        				"sys"
#define LOCAL_OPM_RECORD_FILE_FLAG             		0x6c6f7273 //l o r s
#define CLOUD_OPM_RECORD_FILE_NAME     				"clopmRec"
#define CLOUD_OPM_RECORD_FILE_EXT        				"sys"
#define CLOUD_OPM_RECORD_FILE_FLAG             		0x636f7273 //c o r s

//#define QRCODE_FILE_NAME						"qrcode"
//#define QRCODE_FILE_EXT						"jpg"

#define ABS(A)          ((A) < 0 ? (-A) : (A))


#define	HMOTO_REF_SPEED												39  //4  -- 2pixel����Ҫ��������
#define	MOTO_TEST_SPEED													8
#define	MOTO_TEST_PIXEL													50

#define	FEBER_POS0																4 //4 --������
#define	FEBER_POS1																150//4 --ȥ�ҳ�
#define	FEBER_POS2																38 //4 --�۽���ʼλ��

//----UI
//--g_dwMachineErrorFlag g_dwMachineErrorShow
#define	MACHINE_ERROR_LOCKED										BIT0
#define	MACHINE_ERROR_SENSOR										BIT1

//--g_dwMachineWarningFlag
#define	WARNING_OUTSIDE_TEMP_HIGH									BIT0
#define	WARNING_OUTSIDE_TEMP_LOW										BIT1
#define	WARNING_INSIDE_TEMP_HIGH										BIT2
#define	WARNING_INSIDE_TEMP_LOW											BIT3
#define	WARNING_HUMIDITY															BIT4
#define	WARNING_ATMOS_PRESSURE											BIT5
#define	WARNING_ELECTRODE_LESS											BIT6
#define	WARNING_BATTERY_LOW													BIT7
#define	WARNING_NETSIGNAL_LOW												BIT8


typedef struct{
    BYTE    bMode;				/* 0->alll record 3->3days 7->one week */
    WORD    wCurPage;                    
    WORD    wTotalPage;
    DWORD    dwCurIndex;                    
	DWORD dwTotalData;
	//BYTE    bReserve[3];                    
} WeldRecordPage;
extern WeldRecordPage  g_WeldRecordPage;

#define						WELD_RECORD_TITLE_LENTH					11 //4 �۽Ӽ�¼���ⳤ��,��������0
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


//OPM lenth
#define	OPM_SEGMEN_NUM												200
#define	OPM_RECORD_NAME_LENTH								16 //4 OPM��¼���ⳤ��,��������0
#define	OPM_REC_BUF_HEAD_LEN									16 //4 ��¼BUFFERͷ�����ֽ���������־�����ȵ�

//0x01����OPM 0x02�ƶ�OPM
enum {
    OPM_MACHINE_NULL,
    OPM_MACHINE_LOCAL,
    OPM_MACHINE_CLOUD,
};


//bMode   0x01 ����ʵʱ���� 0x02���ش洢���� 0x03 �ƶ�ʵʱ���� 0x04 �ƶ���ʷ����
enum {
    OPM_DATA_NULL,
    OPM_LOCAL_REALTIME,
    OPM_LOCAL_RECORD,
    OPM_CLOUD_REALTIME,
    OPM_CLOUD_RECORD,
};

typedef struct{
    BYTE    bPowerOnOff;				/* 0->off 1->on */
    BYTE    bUnit;				/* 0->uW 1->dbm db */
    BYTE    bCal;				/* 0->off 1->on */
} ST_OPM_PAGE;
extern ST_OPM_PAGE g_stOpmPagePara;


typedef struct{
	BYTE  bMode;			/* bMode   0x01 ����ʵʱ���� 0x02���ش洢���� 0x03 �ƶ�ʵʱ���� 0x04 �ƶ���ʷ���� */
	BYTE	bIndexId[6];				/* ������ʱ����*/
	BYTE	bWaveLenth;		/* BIT0~2: uwС����λ��BIT3~5:����0 ~5  0��850 1��1300 2��1310 3��1490 4��1550 5��1625 BIT6:db  0���� 1Ϊ����BIT7:dbm  0���� 1Ϊ���� */
	BYTE  bDbDbmDot;			/* BIT0~3: dbС����λ��BIT4~7: dbmС����λ�� */
	WORD wuW;
	WORD wdbm;
	WORD wdb; //15 bytes
	BYTE bReserve;
} ST_OPM_REAL_DATA;
extern ST_OPM_REAL_DATA g_stOpmRealData;

typedef struct{
	BYTE  bMode;			/* bMode   0x01 ����ʵʱ���� 0x02���ش洢���� 0x03 �ƶ�ʵʱ���� 0x04 �ƶ���ʷ���� */
	BYTE	bIndexId[6];				/* ������ʱ����*/
	BYTE	bWaveLenth;		/* BIT0~2: uwС����λ��BIT3~5:����0 ~5  0��850 1��1300 2��1310 3��1490 4��1550 5��1625 BIT6:db  0���� 1Ϊ����BIT7:dbm  0���� 1Ϊ���� */
	BYTE  bDbDbmDot;			/* BIT0~3: dbС����λ��BIT4~7: dbmС����λ�� */
	WORD wuW;
	WORD wdbm;
	WORD wdb; //14 bytes
	BYTE bReserve;
	BYTE bName[OPM_RECORD_NAME_LENTH];
} ST_OPM_REC_DATA;

#define	OPM_SEGMEN_LEN												(sizeof(ST_OPM_REAL_DATA)+OPM_RECORD_NAME_LENTH)

extern BYTE g_bKeyExcept;
extern DWORD g_dwMachineErrorFlag,g_dwMachineErrorShow,g_dwMachineWarningFlag;


SWORD TspiSendCmdPolling0xA4(BYTE bCmd);
SWORD  TspiSendSimpleInfo0xAF(BYTE bInfo);
void SendCmdPowerOff();

void uiCb_CheckInStandby(void);
void uiCb_CheckBattery(void);

void TSPI_DataProc();
SWORD TSPI_PacketSend(BYTE *pbDataBuf,BYTE bCheckResend);
//void DriveMotor(BYTE bMotorInex,BYTE bSpeed,BYTE bDiv,BYTE bDirector,WORD wStep);
void DriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wStep,BYTE bSpeed);
void MotorSetStatus(BYTE bMotorInex,BYTE bMode);
void MotoHoldTimeoutSet(BYTE bMotorInex,BYTE bMode);
void StopAllMoto(void);
void WeldStopAllAction(void);
void Discharge(WORD wMode,BYTE bStep);
SWORD ScanFiberUptoDown(ST_IMGWIN *pWin, SWORD x,SWORD y,SWORD swYEnd,BYTE bLowLevel);
void  PutErrorOsdString(SWORD swData0,SWORD swData1,SWORD swData2,SWORD swData3);

#if TEST_PLANE||ALIGN_DEMO_MODE
void  PutAdjOsdString();
#endif
void ShowOSDstring(void);

SWORD Weld_CaptureFile(ST_IMGWIN *pWin);
SWORD Weld_ReadFileWeldInfo(STREAM* handle,STRECORD *pRecordData);
void uiCb_CheckElectrodePos(void);
void uiCb_DisableKeyInput(BYTE bKeyExcept);  //0xff -> skip all key
void uiCb_EnableKeyInput(void);
void Weld_ReadAllRecord();
void AddRecord(STRECORD* pstRecord);
STRECORD* GetRecord(DWORD dwIndex);
DWORD GetRecordTotal();
void ClearAllRecord();

#endif


