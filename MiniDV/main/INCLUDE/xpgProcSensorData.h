#ifndef __XPG__PROC_SENSOR__H__
#define __XPG__PROC_SENSOR__H__

#include "global612.h"




//----debug switch
#define   ALIGN_DEMO_MODE											1 //4 文字显示马达、步长并可调整最后移动的距离及放电时间
#define   DEBUG_POS_DATA												0

#define   ALIGN_NEW_MODE											1//4   从光纤端面向外寻找上下面的位置
#define   ALIGN_FAST_MODE											1//4  两边光纤同时移动,精确移动

#define   FIBER_DISPLAY_IN_SAME_SIDE					1//4  上下两个界面的左边显示的是同一根光纤

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


typedef struct {
    BYTE        bFiberMode;
    BYTE        wFiberL;
    BYTE        wFiberR;
    BYTE        bFiberLoss;
    BYTE        bResult;
}STWELDSTATUS;


#define		FIBER_EDGE_LEVEL					0x10 //4   从背景到光纤的 黑白边界亮度值
#define		X_START										8       //4 x轴起始扫描点
#define		X_STEP											16       //4 快速横向查找步长，必须为4的倍数，一个单位为半个像素
#define		Y_VALID_PIXEL							38       //4 最小半边黑边高度 //really about 40  ,white <30   //实际半边光纤约90个像素
//#define		Y_LAST_SUM									4       //4 前面电平的统计数 must 4
#define		Y_CONTINUE_VALID_SUM			4       //4 Y轴上连续有效数
#define		Y_CENTER_CONTINUE_VALID_SUM			4       //4 Y轴中心上连续有效数
#define		X_SCAN_MAX_SUM								(800/X_STEP)       //4 水平线统计最大值 max=width/X_STEP
#define		X_SCAN_VALID_NUM							20       //4 水平线统计有效值，超过这个值就开始计算 
#define		FACEX_SHAKE										2         //4 FACE防抖
#define		Y_SHAKE										1         //4 水平误差

#define		Y_VALID_OFFSET						3       //4 水平线统计允许在Y上下偏差像素点

#define		CENTER_MIN_PIXEL					10       //4 中间白条最小高度 //really about 13 
#define		LEVEL_OFFSET								12         //4 亮度偏差

#define		LEVELNESS										4         //4 水平最大偏差
#define		ANGEL_OFFSET							180 //160         //4 角度最大偏差
#define		CENTER_W_OFFSET					300 // 100         //4 纤中间白色部分最大偏差

//--g_dwProcWinFlag  用于拍照等标志位
#define	WIN0_CAPTURE_FLAG										BIT0

//--st_bFillWinFlag st_bNeedFillProcWin
#define	FILL_WIN_UP														BIT0
#define	FILL_WIN_DOWN													BIT1
#define	FILL_WIN_UP_WAIT											BIT2
#define	FILL_WIN_DOWN_WAIT										BIT3

#define	FILL_WIN_END														BIT6
#define	FILL_WIN_INIT													BIT7

 // 0->左上  1->右上 2->左下 3->右下
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

//最大水平偏差
#define   MOTOR_HORI_MAX_PULSE								66000 // really 67000
#define   MOTOR_HORI_REVERSE_PULSE						300  // 200
//垂直马达3         
#define   MOTOR_V_REVERSE_PULSE								400  // 
#define   MOTOR_V_MIN_SPEED										45//70  // 1000pulse->7-12pixel ==142

//  累计多少次垂直马达变化次数
#define	VMOTOR_CNT														3 
//  累计多少个黑边有效点
#define	PIXEL_BLACK_CNT												10
//端面最大对齐次数
#define   POS_RETRY_TIMES											10
#define   ALIGN_HORIZONTAL_REDO_TIMES				2
#define   RETRY_TIMEOUT												10000// ms
//最大水平偏差
#define   HORI_OFFSET														3

//背景亮度设定
#define   BG_BRIGHT_SHIFT											0

//string lenth
#define	POS_STR_LEN														16

//OPM lenth
#define	OPM_SEGMEN_NUM												200
#define	OPM_SEGMEN_LEN												16


#define QRCODE_FILE_NAME						"QRCODE"
#define QRCODE_FILE_EXT						"JPG"

#define ABS(A)          ((A) < 0 ? (-A) : (A))


#define	HMOTO_REF_SPEED												39  //4  -- 2pixel所需要的脉冲数
#define	MOTO_TEST_SPEED													8
#define	MOTO_TEST_PIXEL													50

#define	FEBER_POS0																4 //4 --看到纤
#define	FEBER_POS1																150//4 --去灰尘
#define	FEBER_POS2																38 //4 --熔接起始位置

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
SWORD Weld_ReadFileWeldInfo(STREAM* handle,BYTE *pbTitle,STWELDSTATUS *pWeldStatus);

#endif


