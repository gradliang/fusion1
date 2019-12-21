#ifndef __XPG__PROC_SENSOR__H__
#define __XPG__PROC_SENSOR__H__

#include "global612.h"


enum {
    SENSOR_IDLE,
    SENSOR_FACE_POS1A,
    SENSOR_FACE_POS1B,
    //SENSOR_ALIGN_H1A,
    //SENSOR_ALIGN_H1B,
    SENSOR_DISCHARGE1,
    SENSOR_AUTO_FOCUS,
    SENSOR_GET_ANGLE,
    SENSOR_FACE_POS2A,
    SENSOR_FACE_POS2B,
    SENSOR_ALIGN_H2A,
    SENSOR_ALIGN_H2B,
    SENSOR_ALIGN_H3A,
    SENSOR_ALIGN_H3B,
    SENSOR_DISCHARGE2,
    SENSOR_DISCHARGE3,
    SENSOR_GET_LOSS,
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

typedef struct{
    WORD    wNewData;
    WORD    wCnt;
} ST_NEWDATACNT;


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
//string lenth
#define	POS_STR_LEN														16


#define ABS(A)          ((A) < 0 ? (-A) : (A))


void TSPI_DataProc();
SWORD TSPI_PacketSend(BYTE *pbDataBuf,DWORD dwLenth ,BYTE bCheckResend);
//void DriveMotor(BYTE bMotorInex,BYTE bSpeed,BYTE bDiv,BYTE bDirector,WORD wStep);
void DriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wStep,BYTE bSpeed);
void MotorSetStatus(BYTE bMotorInex,BYTE bMode);
void MotoHoldTimeoutSet(BYTE bMotorInex,BYTE bMode);
void Discharge(WORD wMode,BYTE bStep);
void ShowOSDstring(void);


#endif


