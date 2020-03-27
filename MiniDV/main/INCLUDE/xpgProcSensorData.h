#ifndef __XPG__PROC_SENSOR__H__
#define __XPG__PROC_SENSOR__H__

#include "global612.h"


enum {
    SENSOR_IDLE,
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

enum {
    RPOC_WIN0,
    RPOC_WIN1,
    RPOC_WIN_TOTAL,
};


typedef struct {
    BYTE        bFiberMode;
    BYTE        wFiberL;
    BYTE        wFiberR;
    BYTE        bFiberLoss;
    BYTE        bResult;
}STWELDSTATUS;


//--g_dwProcWinFlag  用于拍照等标志位
#define	WIN0_CAPTURE_FLAG										BIT0

//--st_bFillWinFlag st_bNeedFillProcWin
#define	FILL_WIN_UP														BIT0
#define	FILL_WIN_DOWN													BIT1
#define	FILL_WIN_UP_WAIT											BIT2
#define	FILL_WIN_DOWN_WAIT										BIT3

#define	FILL_WIN_INIT													BIT7

 // 0->左上  1->右上 2->左下 3->右下
#define	MOTOR_LEFT_TOP												0
#define	MOTOR_RIGHT_TOP												1
#define	MOTOR_LEFT_BOTTOM										2
#define	MOTOR_RIGHT_BOTTOM										3
#define	MOTOR_NUM															4


#define	MOTOR_STOP														0
#define	MOTOR_NO_HOLD												1
#define	MOTOR_HOLD														2
#define	MOTOR_HOLD_TIMEOUT										30*1000

//  累计多少次垂直马达变化次数
#define	VMOTOR_CNT														3 
//  累计多少个黑边有效点
#define	PIXEL_BLACK_CNT												10
//端面最大对齐次数
#define   POS_RETRY_TIMES											10
#define   ALIGN_HORIZONTAL_REDO_TIMES				2
//最大水平偏差
#define   HORI_OFFSET														3


//string lenth
#define	POS_STR_LEN														16

//OPM lenth
#define	OPM_SEGMEN_NUM												200
#define	OPM_SEGMEN_LEN												16


#define QRCODE_FILE_NAME						"QRCODE"
#define QRCODE_FILE_EXT						"JPG"

#define ABS(A)          ((A) < 0 ? (-A) : (A))


//----debug switch
#define   ALIGN_DEMO_MODE											1
#define   DEBUG_POS_DATA												0



void TSPI_DataProc();
SWORD TSPI_PacketSend(BYTE *pbDataBuf,BYTE bCheckResend);
//void DriveMotor(BYTE bMotorInex,BYTE bSpeed,BYTE bDiv,BYTE bDirector,WORD wStep);
void DriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wStep,BYTE bSpeed);
void MotorSetStatus(BYTE bMotorInex,BYTE bMode);
void MotoHoldTimeoutSet(BYTE bMotorInex,BYTE bMode);
void StopAllMoto(void);
void Discharge(WORD wMode,BYTE bStep);
SWORD ScanFiberUptoDown(ST_IMGWIN *pWin, SWORD x,SWORD y,SWORD swYEnd,BYTE bLowLevel);

#if TEST_PLANE||ALIGN_DEMO_MODE
void  PutAdjOsdString();
#endif
void ShowOSDstring(void);

SWORD Weld_CaptureFile(ST_IMGWIN *pWin);
SWORD Weld_ReadFileWeldInfo(STREAM* handle,BYTE *pbTitle,STWELDSTATUS *pWeldStatus);

#endif


