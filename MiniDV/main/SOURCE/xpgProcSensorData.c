/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  1
/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "os.h"
#include "display.h"
#include "devio.h"
#include "taskid.h"
#include "fs.h"
#include "ui.h"
#include "filebrowser.h"
#include "imageplayer.h"
#include "mpapi.h"
#include "peripheral.h"

#include "Setup.h"
#include "filebrowser.h"
#include "Icon.h"
#include "ui_timer.h"
#include "xpgFunc.h"
#include "xpgProcSensorData.h"
#include "xpgString.h"
#include "uiTouchCtrller.h"
#include "xpgDrawSprite.h"


//#define DEBUG_POS(...)                   DONOTHING
#define DEBUG_POS                   		mpDebugPrint


#if (SENSOR_ENABLE == ENABLE)
#if SENSOR_WIN_NUM
extern BYTE *pbSensorWinBuffer;
extern ST_IMGWIN SensorInWin[SENSOR_WIN_NUM];

static BYTE  st_bRetryTimes=0,st_bAutoDischarge=0,st_bProcWinIndex=0;
static DWORD st_dwGetCenterState = GET_CENTER_OFF;
static DWORD st_dwProcState = SENSOR_IDLE;//SENSOR_FACE_POS1A;//SENSOR_IDLE; //BIT30->PAUSE
DWORD g_dwProcWinFlag = 0;  // 用于拍照等WIN0_CAPTURE_FLAG
//WORD g_wElectrodePos[2]={356,356};  //4 电极棒位置
//SWORD g_swCenterOffset=0;  //4 电极棒位置  76

static SWORD st_swFaceX[FIBER_WINDOW_NUM]={-1,-1,-1,-1},st_swHlevel[FIBER_WINDOW_NUM]={-1,-1},st_swAngle[FIBER_WINDOW_NUM]={0,0,0,0},st_swCenterW[FIBER_WINDOW_NUM]={0,0,0,0};
static SWORD st_swFaceY1[FIBER_WINDOW_NUM]={-1,-1,-1,-1},st_swFaceY2[FIBER_WINDOW_NUM]={-1,-1,-1,-1},st_swFaceY20[FIBER_WINDOW_NUM]={-1,-1,-1,-1},st_swFaceY3[FIBER_WINDOW_NUM]={-1,-1,-1,-1}; //4  上中下  
static BYTE st_bDirectionArry[FIBER_WINDOW_NUM],st_bBaseRetryArry[FIBER_WINDOW_NUM],st_bMotorHold=0,st_bMotorStaus=0;// 0->STOP 1->RUN BIT0
static DWORD st_dwMotoStartTime[FIBER_WINDOW_NUM],st_dwHMotoPos[2];
static BYTE st_bTopVMotorUpValue=0,st_bBottomVMotorUpValue=0,st_bVmotorMoveTimes[2]={0};
static SWORD st_swLastProcStep[2]={0,0},st_swVmotorMoveValue[2][VMOTOR_CNT]; // 0->up motor  1->down motor
//--st_wDiachargeRunTime 前一次放电时间  st_wDischargeTimeSet:设置需要放电的时间或模式
static WORD st_wDiachargeRunTime=0,st_wDischargeTimeSet=0; // st_wDischargeTimeSet:0->off 
static WORD st_wMotorSpeed[FIBER_WINDOW_NUM]={0,0,0,0};//4  -- 2pixel所需要的脉冲数
static BYTE st_bFiberBlackLevel[SENSER_TOTAL]={0xff,0xff};

static STRECORD st_CurWeldRecordData;
#if TEST_PLANE||ALIGN_DEMO_MODE
#define MAX_ADJ_NUM					9
#define MOTO_ADJ_NUM				6
static BYTE st_bStrFace[FIBER_WINDOW_NUM][POS_STR_LEN], st_bStrCenter[FIBER_WINDOW_NUM][POS_STR_LEN], st_bStrCore[FIBER_WINDOW_NUM][POS_STR_LEN],st_bStrHLevel[FIBER_WINDOW_NUM][POS_STR_LEN], st_bStrInfo[100],st_bAdjustMotoStep=0;
static WORD st_wMotoStep[MAX_ADJ_NUM]={400,400,1000,1000,10,10,38,1120,2400}; // 1300,2400<-1500,2300
static BYTE *st_pbAdjStr[MAX_ADJ_NUM]={"Moto","Moto","Moto","Moto","Moto","Moto","JianJu","TuiJing","FangDian"};
static BYTE st_bWaitMotoStop=0;
#endif
#if TEST_TWO_LED
static DWORD st_dwBrightness[2]={0};
#endif
//BYTE g_bDisplayUseIpw2=1; // 0->ipw1  1->ipw2
#endif

static BYTE st_bSensorMode=0; // 0->scaler mode  1-> 1:1 get center data
BYTE Sensor_GetPicMode(void)
{
	return st_bSensorMode;
}
BYTE g_bKeyExcept=0xff; //disable all
DWORD g_dwMachineErrorFlag=0,g_dwMachineErrorShow=0,g_dwMachineWarningFlag=0;

#if 1 //OPM
//第一个DWORD为HEAD，HEAD内每个DWORD为有效SEG数
static BYTE* st_pOpmLocalBuf=NULL,* st_pOpmCloudBuf=NULL;

/**
 *
 * @param   bMode:  0x01 本地实时数据 0x02本地存储数据 0x03 云端实时数据 0x04 云端历史数据
 *
 * @return   : success for resource data else return NULL
 *
 */
void OpmBufferRelease(void)
{
	if (st_pOpmLocalBuf)
		ext_mem_free(st_pOpmLocalBuf);
	if (st_pOpmCloudBuf)
		ext_mem_free(st_pOpmCloudBuf);
}
//bMode   0x01 本地实时数据 0x02本地存储数据 0x03 云端实时数据 0x04 云端历史数据
BYTE *OpmGetbuffer(BYTE bMode) 
{
	if (bMode==0x02)
	{
		if (st_pOpmLocalBuf==NULL)
		{
			st_pOpmLocalBuf = (BYTE *)ext_mem_malloc(ALIGN_32(OPM_SEGMEN_NUM*OPM_SEGMEN_LEN+1));
			memset(st_pOpmLocalBuf,0,ALIGN_32(OPM_SEGMEN_NUM*OPM_SEGMEN_LEN+1));
		}
		return st_pOpmLocalBuf;
	}
	else if (bMode==0x04)
	{
		if (st_pOpmCloudBuf==NULL)
		{
			st_pOpmCloudBuf = (BYTE *)ext_mem_malloc(ALIGN_32(OPM_SEGMEN_NUM*OPM_SEGMEN_LEN+1));
			memset(st_pOpmCloudBuf,0,ALIGN_32(OPM_SEGMEN_NUM*OPM_SEGMEN_LEN+1));
		}
		return st_pOpmCloudBuf;
	}

	return NULL;

}

DWORD OpmGetTotalNumber(BYTE bMode)
{
	DWORD *pdwHead;
	
	if (pdwHead=(DWORD *)OpmGetbuffer(bMode))
	{
		return *pdwHead;
	}

	return 0;
}

#if 0
int LoadOPMLocalDataFromFile()
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

        if (FileWrite(file_1, (BYTE *)&dwFlag, 4) != 4)
        {
            MP_ALERT("write record1.sys error.");
            goto _OPEN_END;
        }

        for (i = 0; i < GetRecordTotal(); i++)
        {
            STRECORD* pstRecord = GetRecord(i);
            FileWrite(file_1, (BYTE *)pstRecord, sizeof(STRECORD));
        }
        
_OPEN_END:
        if (file_1 != NULL)
            FileClose(file_1);
    }

    return retVal;
}
#endif


#endif

#if TSPI_ENBALE||UART_TO_MCU
#define  TX_BUF_NORMAL_LEN						32
#define  RX_BUF_NORMAL_LEN						32

static DWORD st_dwTspiRxBufLen=0,st_dwTspiRxIndex=0,st_dwCurRxDataLen;
static BYTE *pbTspiRxBuffer=NULL,*pbTspiTxBuffer=NULL,st_bTspiTxRetry=0;
static DWORD st_dwTspiTxBufLen=0,st_dwTspiTxMaxLen;  //4      st_dwTspiTxMaxLen:未收到MCU回复确认长度前BIT31为1
#endif

#if TSPI_ENBALE
#define  TX_BUFFER_LENTH								16


#define  RX_HEAD_FLAG1									0x55
#define  RX_HEAD_FLAG2									0x55
#define  RX_HEAD_FLAG3									0xee


#define SPI_CLK_GPIO                			GPIO_UGPIO_0 //pin80 
#define SPI_DOUT_GPIO                			GPIO_KGPIO_1 //pin119
#define SPI_DIN_GPIO                			GPIO_KGPIO_0 //pin118
#define SPI_WAIT_START                		10000

#define TSPI_CLK_Low 		Gpio_Config2GpioFunc(SPI_CLK_GPIO, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1)
#define TSPI_CLK_High 	Gpio_Config2GpioFunc(SPI_CLK_GPIO, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1)
#define TSPI_DOUT_Low 		Gpio_Config2GpioFunc(SPI_DOUT_GPIO, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1)
#define TSPI_DOUT_High 	Gpio_Config2GpioFunc(SPI_DOUT_GPIO, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1)
#define TSPI_DIN_Low 		Gpio_Config2GpioFunc(SPI_DIN_GPIO, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1)
#define TSPI_DIN_High 	Gpio_Config2GpioFunc(SPI_DIN_GPIO, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1)

static BYTE st_bTspiBusy=0,st_bTspiAbnormal=0;


BYTE TSPI_WaitFree(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if ((Gpio_ValueGet(SPI_CLK_GPIO) == 1)&& (Gpio_ValueGet(SPI_DOUT_GPIO) == 1) && (Gpio_ValueGet(SPI_DIN_GPIO) == 1))
					return 1;
	}
	return 0;
}


BYTE TSPI_Wait_DinLow(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if (Gpio_ValueGet(SPI_DIN_GPIO) == 0)
					return 1;
	}
	return 0;
}

BYTE TSPI_Wait_DinHigh(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if (Gpio_ValueGet(SPI_DIN_GPIO) == 1)
					return 1;
	}
	return 0;
}

BYTE TSPI_Wait_DoutLow(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if (Gpio_ValueGet(SPI_DOUT_GPIO) == 0)
					return 1;
	}
	return 0;
}

BYTE TSPI_Wait_DoutHigh(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if (Gpio_ValueGet(SPI_DOUT_GPIO) == 1)
					return 1;
	}
	return 0;
}

BYTE TSPI_Wait_ClkLow(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if (Gpio_ValueGet(SPI_CLK_GPIO) == 0)
					return 1;
	}
	return 0;
}

BYTE TSPI_Wait_ClkHigh(void)
{
	DWORD i=SPI_WAIT_START;

	while (i--)
	{
	    if (Gpio_ValueGet(SPI_CLK_GPIO) == 1)
					return 1;
	}
	return 0;
}

void TSPI_Init(void)
{
	Gpio_ConfiguraionSet(SPI_CLK_GPIO, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
	Gpio_ConfiguraionSet(SPI_DOUT_GPIO, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
	Gpio_ConfiguraionSet(SPI_DIN_GPIO, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
	if (pbTspiTxBuffer!=NULL && st_dwTspiTxBufLen)
		ext_mem_free(pbTspiTxBuffer);
	if (st_dwTspiTxBufLen<TX_BUF_NORMAL_LEN)
		st_dwTspiTxBufLen=TX_BUF_NORMAL_LEN;
	st_dwTspiTxBufLen=ALIGN_32(st_dwTspiTxBufLen);
	pbTspiTxBuffer = (BYTE *)ext_mem_malloc(st_dwTspiTxBufLen);
	st_dwTspiTxMaxLen=st_dwTspiTxBufLen|BIT31;

	if (pbTspiRxBuffer!=NULL && st_dwTspiRxBufLen)
		ext_mem_free(pbTspiRxBuffer);
	if (st_dwTspiRxBufLen<RX_BUF_NORMAL_LEN)
		st_dwTspiRxBufLen=RX_BUF_NORMAL_LEN;
	st_dwTspiRxBufLen=ALIGN_32(st_dwTspiRxBufLen);
	pbTspiRxBuffer = (BYTE *)ext_mem_malloc(st_dwTspiRxBufLen);
	st_bTspiBusy=0;
}

void TSPI_Reset(void)
{
	TSPI_CLK_High;
	TSPI_DOUT_High;
	TSPI_DIN_High;
	st_bTspiBusy=0;
}

SWORD TSPI_Send(BYTE *pbDataBuf,DWORD dwLenth )
{
	SWORD swRet=PASS;
	DWORD i;
	BYTE k;

	if (!TSPI_WaitFree())
	{
		MP_DEBUG("TSPI busy!");
		return FAIL;
	}
	//Start
	st_bTspiBusy=1;
	TSPI_CLK_Low;
	TSPI_DOUT_Low; 
	if (!TSPI_Wait_DinLow())
	{
		MP_DEBUG("Start no ack(Din=1) !");
		TSPI_Reset();
		return FAIL;
	}

	//Tranfor data
	for (i=0;i<dwLenth;i++)
	{
		for (k=0x80;k>0;k>>=1)
		{
			TSPI_CLK_Low;
			if (!TSPI_Wait_DinLow())
			{
				MP_DEBUG("S Din=1!");
				swRet=FAIL;
				break;
			}
			if (pbDataBuf[i]&k)
				TSPI_DOUT_High;
			else
				TSPI_DOUT_Low; 
			TSPI_CLK_High; 
			if (!TSPI_Wait_DinHigh()) //read
			{
				MP_DEBUG("S Din=0!");
				swRet=FAIL;
				break;
			}
		}
	}

	//stop bit
	TSPI_CLK_Low;
	if (!TSPI_Wait_DinLow())
	{
		MP_DEBUG("Stop Din=1!");
		swRet=FAIL;
	}
	if (swRet==PASS)
	{
		TSPI_DIN_Low; 
		TSPI_DOUT_High;
		TSPI_CLK_High; 
		if (TSPI_Wait_DoutLow())
		{
			TSPI_CLK_Low;
			TSPI_DIN_High; 
			if (TSPI_Wait_DoutHigh())
			{
				TSPI_CLK_High; 
			}
			else
			{
				MP_DEBUG("Stop Dout=0!");
				swRet=FAIL;
			}
		}
		else
		{
			MP_DEBUG("Stop Dout=1!");
			swRet=FAIL;
		}
	}


		TSPI_Reset();
#if 0
		if (swRet==PASS)
			mpDebugPrintN("-O-");
		else
			mpDebugPrintN("-N-");
#endif
		return swRet;
}

void TSPI_TimerToResend(void)
{
	DWORD dwLenth;
	
	if (!st_bTspiTxRetry)
		return;
	if (pbTspiTxBuffer[1])
		dwLenth=pbTspiTxBuffer[1];
	else
		dwLenth=((DWORD)pbTspiTxBuffer[2]<<24)|((DWORD)pbTspiTxBuffer[3]<<16)|((DWORD)pbTspiTxBuffer[4]<<8)|pbTspiTxBuffer[5];
	if (TSPI_Send(pbTspiTxBuffer,dwLenth)!=PASS)
	{
		st_bTspiTxRetry++;
		if (st_bTspiTxRetry<6)
		{
			TSPI_Reset();
			Ui_TimerProcAdd(st_bTspiTxRetry*10, TSPI_TimerToResend);
		}
	}
	else
		st_bTspiTxRetry=0;
}

SWORD TSPI_SendWithAutoResend(BYTE *pbDataBuf,DWORD dwLenth )
{
	BYTE i;
	SWORD swRet;

	swRet=TSPI_Send(pbDataBuf,dwLenth);
	if (swRet!=PASS  && dwLenth<=st_dwTspiTxBufLen)
	{
		st_bTspiTxRetry=1;
		for (i=0;i<dwLenth;i++)
			pbTspiTxBuffer[i]=pbDataBuf[i];
		TSPI_Reset();
		Ui_TimerProcAdd(st_bTspiTxRetry*10, TSPI_TimerToResend);
	}
	else
		st_bTspiTxRetry=0;

	return swRet;
}

SWORD TSPI_PacketSend(BYTE *pbDataBuf,BYTE bCheckResend)
{
	DWORD i,dwLenth;
	
	if (pbDataBuf[1])
		dwLenth=pbDataBuf[1];
	else
		dwLenth=((DWORD)pbDataBuf[2]<<24)|((DWORD)pbDataBuf[3]<<16)|((DWORD)pbDataBuf[4]<<8)|pbDataBuf[5];
	dwLenth--;
	pbDataBuf[dwLenth]=pbDataBuf[0];
	for (i=1;i<dwLenth;i++)
		pbDataBuf[dwLenth]+=pbDataBuf[i];
	if (bCheckResend)
		return TSPI_SendWithAutoResend(pbDataBuf,dwLenth+1);
	return TSPI_Send(pbDataBuf,dwLenth+1);
}

BYTE TSPI_Receiver()
{
	BYTE i,k,bRx[6],bHeadlen;
	SWORD swRet=PASS,swValue;
	DWORD dwLenth;

	if (Gpio_ValueGet(SPI_DIN_GPIO))
		return 0;
	if (!Gpio_ValueGet(SPI_DOUT_GPIO))
		return 0;

	//start ack
	st_bTspiBusy=1;
	TSPI_DOUT_Low;
	for (i=0;i<=st_dwTspiRxBufLen;i++)
	{
		pbTspiRxBuffer[i]=0;
		for (k=0;k<8;k++)
		{
			if (!TSPI_Wait_ClkHigh())
			{
				MP_DEBUG("FAIL clk=0");
				swRet=FAIL;
				break;
			}
			swValue=Gpio_ValueGet(SPI_DIN_GPIO);
			TSPI_DOUT_High;
			if (!k) //check stop 
			{
				if (!Gpio_ValueGet(SPI_DOUT_GPIO) &&  Gpio_ValueGet(SPI_DIN_GPIO)) //stop flag
				{
					TSPI_DIN_Low; //STOP
					swRet=1;
					if (!TSPI_Wait_ClkLow())
					{
						MP_DEBUG("FAIL stop clk=1");
						swRet=FAIL;
					}
					TSPI_DIN_High;
					break;
				}
				if (i>=st_dwTspiRxBufLen)
				{
					MP_DEBUG("TSPI overflow!");
					swRet=FAIL;
					break;
				}
			}
			pbTspiRxBuffer[i]<<=1;
			if (swValue)
				pbTspiRxBuffer[i] |=0x01;
			if (!TSPI_Wait_ClkLow())
			{
				MP_DEBUG("FAIL clk=1");
				swRet=FAIL;
				break;
			}
			TSPI_DOUT_Low;
		}

		if (swRet!=PASS)
			break;
		//判断接收BUFFER是否够大
		if (i==5 && pbTspiRxBuffer[1]==0)  //0xbc->jpg 设备信息数据传输
		{
			for (bHeadlen=0;bHeadlen<6;bHeadlen++)
				bRx[i]=pbTspiRxBuffer[i];
			dwLenth=((DWORD)pbTspiRxBuffer[2]<<24)|((DWORD)pbTspiRxBuffer[3]<<16)|((DWORD)pbTspiRxBuffer[4]<<8)|pbTspiRxBuffer[5];
			ext_mem_free(pbTspiRxBuffer);
			st_dwTspiRxBufLen=ALIGN_32(dwLenth);
			pbTspiRxBuffer = (BYTE *)ext_mem_malloc(st_dwTspiRxBufLen);
			for (bHeadlen=0;bHeadlen<6;bHeadlen++)
				pbTspiRxBuffer[i]=bRx[i];
		}

	}

	if (swRet == 1)
	{
		st_dwTspiRxIndex=i;
		//MP_DEBUG("OK!");
		TSPI_DataProc();
	}
	else
	{
		if (i || k)
			st_dwTspiRxIndex=0;
		else
			st_bTspiAbnormal=1;
	}
	TSPI_Reset();

	if (pbTspiRxBuffer!=NULL && st_dwTspiRxBufLen>RX_BUF_NORMAL_LEN)
	{
		ext_mem_free(pbTspiRxBuffer);
		st_dwTspiRxBufLen=ALIGN_32(RX_BUF_NORMAL_LEN);
		pbTspiRxBuffer = (BYTE *)ext_mem_malloc(st_dwTspiRxBufLen);
	}

	return i;
}

void TSPI_Receive_Check()
{
	if (st_bTspiBusy)
		return;
	if (Gpio_ValueGet(SPI_DIN_GPIO))
	{
		if (st_bTspiAbnormal)
			st_bTspiAbnormal=0;
		return ;
	}
	if (!Gpio_ValueGet(SPI_DOUT_GPIO))
		return ;
	if (st_bTspiAbnormal)
		return;
	EventSet(UI_EVENT, EVENT_TSPI_START);
}

#endif

#if UART_TO_MCU
extern BOOL huartAPinMultiplexerEnable;
void Uart_Send_Action(BYTE *pbDataBuf,DWORD dwLenth )
{
	if (huartAPinMultiplexerEnable)
		HUartMulPinChg(0);// 0->commucation  1->DEBUG
	UartWrite(pbDataBuf,dwLenth);
	//HUartMulPinChg(1);// 0->commucation  1->DEBUG
}

void Uart_TimerToResend(void)
{
	DWORD dwLenth;
	
	if (!st_bTspiTxRetry)
		return;
	if (pbTspiTxBuffer[1])
		dwLenth=pbTspiTxBuffer[1];
	else
		dwLenth=((DWORD)pbTspiTxBuffer[2]<<24)|((DWORD)pbTspiTxBuffer[3]<<16)|((DWORD)pbTspiTxBuffer[4]<<8)|pbTspiTxBuffer[5];
	Uart_Send_Action(pbTspiTxBuffer,dwLenth);
	st_bTspiTxRetry--;
	if (st_bTspiTxRetry)
			Ui_TimerProcAdd(st_bTspiTxRetry*30, Uart_TimerToResend);
}

SWORD TSPI_PacketSend(BYTE *pbDataBuf,BYTE bCheckResend)//uart
{
	DWORD i,dwLenth;
	
	//UartOutValue(pbDataBuf[0], 2);
	if (pbDataBuf[1])
		dwLenth=pbDataBuf[1];
	else
		dwLenth=((DWORD)pbDataBuf[2]<<24)|((DWORD)pbDataBuf[3]<<16)|((DWORD)pbDataBuf[4]<<8)|pbDataBuf[5];
	dwLenth--;
	pbDataBuf[dwLenth]=pbDataBuf[0];
	for (i=1;i<dwLenth;i++)
		pbDataBuf[dwLenth]+=pbDataBuf[i];
	st_bTspiTxRetry=bCheckResend;
	Uart_Send_Action(pbDataBuf,dwLenth+1);
	if (st_bTspiTxRetry)
			Ui_TimerProcAdd(st_bTspiTxRetry*10, Uart_TimerToResend);
}

extern BYTE g_binitok;
static void Uart_Receiver(DWORD cause)
{
	BYTE bData,bDataHead,bNeedProc=0;
	DWORD i;
	static DWORD st_dwLastRxTime;
	BYTE bStirng[16];
	
	//UartOutValue(GetUartChar(), 2);
	//return;
	#if 0
	if (g_binitok)//(cause & C_RXDMA_DONE)
	{
		if (CheckUartStatus(C_RXDMA_DONE)==PASS)
		{
		UartDMARead(pbTspiRxBuffer,st_dwTspiRxBufLen);
		sprintf(bStirng," %02x %02x%02x%02x%02x",pbTspiRxBuffer[0],pbTspiRxBuffer[1],pbTspiRxBuffer[2],pbTspiRxBuffer[3],pbTspiRxBuffer[4]);
		Idu_OsdErase();
		Idu_OSDPrint(Idu_GetOsdWin(),bStirng, 16, 200, OSD_COLOR_BLUE);//  90
		}
	}
	else if (cause & C_RXTHR_HIT)
	#endif
	{
		while(CheckUartStatus(C_RXTHR_HIT)==PASS)
		{
			bData= GetUartChar();
			bDataHead=bData&0xf0;
			if ((st_dwTspiRxIndex && SystemGetElapsedTime(st_dwLastRxTime)>50)||st_dwTspiRxIndex>=20)
			{
				Idu_OsdErase();
				for (i=0;i<st_dwTspiRxIndex;i++)
				{
					sprintf(bStirng,"%02x",bData);
					Idu_OSDPrint(Idu_GetOsdWin(),bStirng, st_dwTspiRxIndex*40, 80, OSD_COLOR_GREEN);//  90
				}
				st_dwTspiRxIndex=0;
				//Idu_OsdErase();
			}
			//sprintf(bStirng,"%02x",bData);
			//Idu_OSDPrint(Idu_GetOsdWin(),bStirng, st_dwTspiRxIndex*40, 40, OSD_COLOR_GREEN);//  90
			st_dwLastRxTime=GetSysTime();
			//st_dwTspiRxIndex++;
			//continue;
			if (st_dwTspiRxIndex>=st_dwTspiRxBufLen)
			{
				st_dwTspiRxIndex=0;
			}
			if (!st_dwTspiRxIndex && bDataHead!=0xb0 && bDataHead != 0xc0)
			{
				//UartOutValue(0x20, 2);
				//UartOutValue(bData, 2);
				Idu_OsdErase();
				sprintf(bStirng,"%02x",bData);
				Idu_OSDPrint(Idu_GetOsdWin(),bStirng, 0, 40, OSD_COLOR_GREEN);//  90
				continue;
			}
			if (st_dwTspiRxIndex==1)
			{
				st_dwCurRxDataLen=bData;
			}
			else if (st_dwTspiRxIndex==5 && !pbTspiRxBuffer[1])
			{
				st_dwCurRxDataLen=((DWORD)pbTspiRxBuffer[5]<<24)|((DWORD)pbTspiRxBuffer[4]<<16)|((DWORD)pbTspiRxBuffer[3]<<8)|((DWORD)pbTspiRxBuffer[2]);
			}
			else if (st_dwTspiRxIndex>1 && st_dwTspiRxIndex+1==st_dwCurRxDataLen)
			{
				bNeedProc=1;
			}
			pbTspiRxBuffer[st_dwTspiRxIndex++]=bData;
		}
		if (bNeedProc)
			EventSet(UI_EVENT, EVENT_PROC_UART_DATA);
	}

}

void Uart_To_Mcu_Init(void)
{
	if (pbTspiTxBuffer!=NULL && st_dwTspiTxBufLen)
		ext_mem_free(pbTspiTxBuffer);
	if (st_dwTspiTxBufLen<TX_BUF_NORMAL_LEN)
		st_dwTspiTxBufLen=TX_BUF_NORMAL_LEN;
	st_dwTspiTxBufLen=ALIGN_32(st_dwTspiTxBufLen);
	pbTspiTxBuffer = (BYTE *)ext_mem_malloc(st_dwTspiTxBufLen);
	st_dwTspiTxMaxLen=st_dwTspiTxBufLen|BIT31;

	if (pbTspiRxBuffer!=NULL && st_dwTspiRxBufLen)
		ext_mem_free(pbTspiRxBuffer);
	if (st_dwTspiRxBufLen<RX_BUF_NORMAL_LEN)
		st_dwTspiRxBufLen=RX_BUF_NORMAL_LEN;
	st_dwTspiRxBufLen=ALIGN_32(st_dwTspiRxBufLen);
	pbTspiRxBuffer = (BYTE *)ext_mem_malloc(st_dwTspiRxBufLen);

#if DEBUG_COM_PORT == HUART_A_INDEX
    HUartRegisterCallBackFunc(HUART_A_INDEX,Uart_Receiver);
#elif DEBUG_COM_PORT == HUART_B_INDEX
    HUartRegisterCallBackFunc(HUART_B_INDEX,Uart_Receiver);
#endif
	EnaUartInt();

}

#endif

#if (PRODUCT_UI==UI_WELDING)
#define MIN_FREE_SPACE             									1000       // KB
#define WELD_FILE_INFO_LENTH             							64  //BYTE
#define HEAD_EXT             												0x65787520  //ext 
#define HEAD_INFO             												0x696e666f  //info
STREAM *GetNewWeldPhotoHandle()
{
	STREAM *handle=NULL;
	SDWORD diskSize;

	diskSize=EarliestFileRemoveToFreeSize(MIN_FREE_SPACE);

	if (diskSize < MIN_FREE_SPACE)
	{
		MP_ALERT("--E-- %s: Disk free space small than %dKB, remain %dKB", __FUNCTION__, MIN_FREE_SPACE, diskSize);
	}
	else
	{
		handle = (STREAM *)CreateFileByRtcCnt("/DCIM/","JPG");
	}

	return handle;
}

SWORD Weld_CaptureFile(ST_IMGWIN *pWin)
{
	STREAM *handle;
	SWORD swRet=PASS;
	BYTE *JpegBuf = NULL,*pbBuf;
	DWORD JpegBufSize,*pdwBuf;
	DWORD IMG_size = 0,i;

	if (DriveCurIdGet()==NULL_DRIVE)
    {
        MP_ALERT("%s: NULL drive index!", __FUNCTION__);
        return FAIL;
    }
	if (SystemGetFlagReadOnly(DriveCurIdGet()))
	{
        MP_ALERT("Write protected !");
	      TaskSleep(4000);
	      return FAIL;
	}

	handle=(STREAM *)GetNewWeldPhotoHandle();
	if (handle)
	{
		JpegBufSize = pWin->wWidth*pWin->wHeight*2;
		JpegBuf = (BYTE*)ext_mem_malloc(JpegBufSize+WELD_FILE_INFO_LENTH);
		//encode jpeg
		IMG_size = ImageFile_Encode_Img2Jpeg(JpegBuf, pWin);
		if (JpegBufSize < IMG_size)
		{
			mpDebugPrint("--E-- %s: memory overflow", __FUNCTION__);
			//free memory
			DeleteFile(handle);
			swRet= -3;
		}
		else
		{
			pbBuf=JpegBuf+IMG_size;
			memset(pbBuf,0,WELD_FILE_INFO_LENTH);
			pdwBuf=(DWORD*)pbBuf;
			//--flag (4bytes):BYTE 0-7
			for (i=0;i<4;i++)
			{
				pbBuf[i]=(HEAD_EXT>>((3-i)<<3))&0xff;
			}
			for (i=0;i<4;i++)
			{
				pbBuf[4+i]=(HEAD_INFO>>((3-i)<<3))&0xff;
			}
			//--record 26 bytes
			ST_SYSTEM_TIME stSystemTime;
			SystemTimeGet(&stSystemTime);
			st_CurWeldRecordData.bYear=stSystemTime.u16Year-2000;//BEGIN_YEAR;
			st_CurWeldRecordData.bMonth=stSystemTime.u08Month;
			st_CurWeldRecordData.bDay=stSystemTime.u08Day;
			st_CurWeldRecordData.bHour=stSystemTime.u08Hour;
			st_CurWeldRecordData.bMinute=stSystemTime.u08Minute;
			st_CurWeldRecordData.bSecond=stSystemTime.u08Second;
			mp_sprintf(st_CurWeldRecordData.bRecordName,"%04d%02d%02d%02d", stSystemTime.u16Year,stSystemTime.u08Month,stSystemTime.u08Day,stSystemTime.u08Hour);

			st_CurWeldRecordData.bResult=1;
			//--status (5bytes): BYTE19-23
			memcpy(&pbBuf[8],(BYTE *)&st_CurWeldRecordData,sizeof (STRECORD));
			FileWrite(handle, (BYTE *) JpegBuf, IMG_size+WELD_FILE_INFO_LENTH);
			FileClose(handle);
			mpDebugPrint("----- %s:ok! %d/%d", __FUNCTION__,IMG_size,IMG_size+WELD_FILE_INFO_LENTH);
		}

		//free memory
		if(JpegBuf != NULL)
		{
			ext_mem_free(JpegBuf);
			JpegBuf = NULL;
		}
		Weld_ReadAllRecord();

		return swRet;
	}

    return FAIL;
}

SWORD Weld_ReadFileWeldInfo(STREAM* handle,STRECORD *pRecordData)
{
	ST_SEARCH_INFO *pSearchInfo;
	DRIVE *pCurDrive;
	DWORD dwFileSize,dwLen,dwFlag0,dwFlag1;
	BYTE *pbBuf=NULL,bHandleNull=0;
	SWORD swRet=FAIL;

	if (handle==NULL)
	{
		bHandleNull=1;
		if (DriveCurIdGet()==NULL_DRIVE)
			xpgChangeDrive(NAND);
		pCurDrive = FileBrowserGetCurDrive();
		pSearchInfo = (ST_SEARCH_INFO *) FileGetCurSearchInfo();
        handle = FileListOpen(pCurDrive, pSearchInfo);
	}
    if (handle == NULL)
        return FILE_NOT_FOUND;

	dwFileSize = FileSizeGet(handle);
	if (dwFileSize<WELD_FILE_INFO_LENTH)
	{
		FileClose(handle);
		return FAIL;
	}
	pbBuf = (BYTE *) ext_mem_malloc(WELD_FILE_INFO_LENTH);
	if (pbBuf==NULL)
	{
		if (bHandleNull)
			FileClose(handle);
		return FAIL;
	}
	Fseek(handle, dwFileSize-WELD_FILE_INFO_LENTH, SEEK_SET);
	dwLen = FileRead(handle, (BYTE *)pbBuf, WELD_FILE_INFO_LENTH);
	memcpy(&dwFlag0,&pbBuf[0],4);
	memcpy(&dwFlag1,&pbBuf[4],4);
	mpDebugPrint("read tag %x %x filesize %d",dwFlag0,dwFlag1,dwFileSize);
	if (dwFlag0==HEAD_EXT  && dwFlag1==HEAD_INFO)
	{
		memcpy(pRecordData,&pbBuf[8],sizeof(STRECORD)-4); // except dwfileindex
		swRet=PASS;
	}
	if (pbBuf)
		ext_mem_free(pbBuf);
	if (bHandleNull)
		FileClose(handle);
	return swRet;
}

void Weld_ReadAllRecord()
{
    DWORD i = 0;
    DWORD total;
    STRECORD recordData;

	g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;
    FileBrowserResetFileList(); /* reset old file list first */
    FileBrowserScanFileList(SEARCH_TYPE);
    total = FileBrowserGetTotalFile();

    ClearAllRecord();
    for (i = 0; i < total; i++)
    {
        FileListSetCurIndex(i);
        if (Weld_ReadFileWeldInfo(NULL, &recordData)==PASS)
        {
			AddRecord(&recordData);
			recordData.dwFileIndex=i;
        }
    }
}


static BYTE *st_pbBuf=NULL;
static DWORD st_dwSendFileLen=0,st_dwSendFileIndex=0;

void WeldSendFileTimer(void)
{
#if UART_TO_MCU
	//Uart_Send(pbTspiTxBuffer,dwSendLen);
#else
	DWORD dwTspiTxMaxLenth=(st_dwTspiTxMaxLen&0x7fffffff)-8,dwSendLen;

	if (st_dwSendFileIndex*dwTspiTxMaxLenth>=st_dwSendFileLen)
	{
		ext_mem_free(st_pbBuf);
		st_pbBuf=NULL;
		st_dwSendFileLen=0;
		return;
	}
	if (st_bTspiTxRetry)
	{
		if (CheckTimerAction(TSPI_TimerToResend))
			Ui_TimerProcAdd(st_bTspiTxRetry*10, TSPI_TimerToResend);
		return;
	}
	if (st_dwSendFileLen>(st_dwSendFileIndex+1)*dwTspiTxMaxLenth)
		dwSendLen=dwTspiTxMaxLenth;
	else
		dwSendLen=st_dwSendFileLen-st_dwSendFileIndex*dwTspiTxMaxLenth;
	pbTspiTxBuffer[0]=0xae;
	pbTspiTxBuffer[1]=0;
	pbTspiTxBuffer[2]=dwSendLen>>24;
	pbTspiTxBuffer[3]=dwSendLen>>16;
	pbTspiTxBuffer[4]=dwSendLen>>8;
	pbTspiTxBuffer[5]=dwSendLen;
	pbTspiTxBuffer[6]=1;
	memcpy(&pbTspiTxBuffer[7],&st_pbBuf[st_dwSendFileIndex*dwTspiTxMaxLenth],dwSendLen-8);
	TSPI_Send(pbTspiTxBuffer,dwSendLen);
	st_dwSendFileIndex++;
	if (st_dwSendFileIndex*dwTspiTxMaxLenth<st_dwSendFileLen)
		Ui_TimerProcAdd(100, WeldSendFileTimer);
	else
	{
		ext_mem_free(st_pbBuf);
		st_pbBuf=NULL;
		st_dwSendFileLen=0;
	}
#endif
}

SWORD Weld_SendFileInit(DWORD dwFileIndex)
{
	STREAM* handle;
	ST_SEARCH_INFO *pSearchInfo;
	DRIVE *pCurDrive;
	DWORD dwFileSize,dwFlag0,dwFlag1;

	if (DriveCurIdGet()==NULL_DRIVE)
	xpgChangeDrive(NAND);
	pCurDrive = FileBrowserGetCurDrive();
	pSearchInfo = (ST_SEARCH_INFO *) FileGetCurSearchInfo();
	handle = FileListOpen(pCurDrive, pSearchInfo);
    if (handle == NULL)
        return FILE_NOT_FOUND;

	dwFileSize = FileSizeGet(handle);
	if (st_pbBuf)
		ext_mem_free(st_pbBuf);
	st_pbBuf = (BYTE *) ext_mem_malloc(ALIGN_32(dwFileSize));
	if (st_pbBuf==NULL)
	{
		FileClose(handle);
		return FAIL;
	}
	st_dwSendFileLen = FileRead(handle, st_pbBuf, dwFileSize);
	st_dwSendFileIndex=0;
	FileClose(handle);
	Ui_TimerProcAdd(10, WeldSendFileTimer);
}


SWORD Weld_FileNameToTime(DWORD dwFileIndex,DWORD *pdwRtcCnt)
{
	ST_SEARCH_INFO *pSearchInfo;
	DWORD i;

	if (dwFileIndex>=FileBrowserGetTotalFile())
		return FAIL;
	pSearchInfo = (ST_SEARCH_INFO *) FileGetSearchInfo(dwFileIndex);
	*pdwRtcCnt=0;
	for (i=0;i<8;i++)
	{
		if (pSearchInfo->bName[i]<0x41 || pSearchInfo->bName[i]>=0x51)
			return FAIL;
		*pdwRtcCnt|=(((DWORD)(pSearchInfo->bName[i]-0x41))<<(i<<2));
	}
	return PASS;
}

BYTE Weld_CheckTimeByFileName(BYTE *pbName)
{
	ST_SEARCH_INFO *pSearchInfo;
	DWORD i,dwFileRtcCnt=0,dwCurRtcCnt,dwDistance;

	if (!g_WeldRecordPage.bMode) //day
		return TRUE;
	for (i=0;i<8;i++)
	{
		if (pbName[i]<0x41 || pbName[i]>=0x51)
			return FALSE;
		dwFileRtcCnt|=(((DWORD)(pbName[i]-0x41))<<(i<<2));
	}
	dwDistance=g_WeldRecordPage.bMode*86400;
	dwCurRtcCnt=RTC_ReadCount();
	if (dwFileRtcCnt<dwCurRtcCnt && dwFileRtcCnt+dwDistance>dwCurRtcCnt)
		return TRUE;
	return FALSE;
}

#if 0
#define OPM_LOCAL_FILE_NAME     "opmlocal"
#define OPM_LOCAL_FILE_EXT        "dat"
#define OPM_LOCAL_FLAG             0x6f706d31 // opm1

int OpmLocalRead()
{
    DRIVE *sysDrv;
    BYTE  sysDrvId = SYS_DRV_ID;
    DWORD confirm_1, confirm_2;
    DWORD value_1, value_2, updateValue;
    DWORD *ptr32;
    SDWORD retVal = PASS;
    DRIVE_PHY_DEV_ID phyDevID = DriveIndex2PhyDevID(sysDrvId);
	STREAM* sHandle = NULL;
	DWORD fileSize;
	DWORD dwFlag;
	BOOL checkTimes = 1,bNewfile=0;
	STRECORD  record;

    if (sysDrvId == NULL_DRIVE)
    {
        MP_ALERT("--E-- %s: Invalid System Drive ID (= %d) defined ! => use current drive instead ...", __FUNCTION__, SYS_DRV_ID);
        sysDrvId = DriveCurIdGet(); /* use current drive */
        if (sysDrvId == NULL_DRIVE)
        {
            MP_ALERT("%s: --E-- Current drive is NULL !!! Abort !!!", __FUNCTION__);
            return FALSE;
        }
        MP_DEBUG("Using current drive-%s !!!", DriveIndex2DrvName(sysDrvId));
    }
	sysDrv = DriveGet(sysDrvId);
	DirReset(sysDrv);

	if (FileSearch(sysDrv, OPM_LOCAL_FILE_NAME, OPM_LOCAL_FILE_EXT, E_FILE_TYPE) != FS_SUCCEED)
	{
		MP_DEBUG("record1.sys is not found in drive-%s !! Create it now.", DriveIndex2DrvName(sysDrvId));
		if (CreateFile(sysDrv, OPM_LOCAL_FILE_NAME, OPM_LOCAL_FILE_EXT) != FS_SUCCEED)
		{
			MP_ALERT("-E- record1.sys in drive-%s can't be created!!!", DriveIndex2DrvName(sysDrvId));
			retVal = FAIL;
			goto _OPEN_END;
		}
		else
			bNewfile=1;
	}

      sHandle = FileOpen(sysDrv);
      fileSize = FileSizeGet(sHandle);
      if (fileSize < 4)
      {
			swRet = DeleteFile(sHandle);
          goto _OPEN_END;
      }
      

      Fseek(sHandle, 0, SEEK_SET);
      if (FileRead(sHandle, (BYTE *) &dwFlag, 4) != 4)
      {
          retVal = FAIL;
          goto _OPEN_END;
      }

      if (dwFlag != OPM_LOCAL_FLAG)
      {
          MP_ALERT("-E- record1.sys flag error.");
          goto _OPEN_END;
      }

      ClearAllRecord();
      
      while (FileRead(sHandle, (BYTE *) &record, sizeof(record)) == sizeof(record))
      {
          AddRecord(&record);
      }
      
_OPEN_END:
      if (sHandle != NULL)
          FileClose(sHandle);

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
        STREAM* sHandle = NULL;
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

        sHandle = FileOpen(sysDrv);
        
        fileSize = FileSizeGet(sHandle);
        if (fileSize < 4)
        {
            goto _OPEN_END;
        }

        if (FileWrite(sHandle, &dwFlag, 4) != 4)
        {
            MP_ALERT("write record1.sys error.");
            goto _OPEN_END;
        }

        for (i = 0; i < GetRecordTotal(); i++)
        {
            STRECORD* pstRecord = GetRecord(i);
            FileWrite(sHandle, pstRecord, sizeof(STRECORD));
        }
        
_OPEN_END:
        if (sHandle != NULL)
            FileClose(sHandle);
    }

    return retVal;
}
#endif


#endif


#if (PRODUCT_UI==UI_WELDING)
extern WORD SensorWindow_PosX,SensorWindow_PosY,SensorWindow_Width,SensorWindow_Height;
extern BYTE g_bDisplayMode;
extern DWORD g_dwWinStartOffset[2];

static BYTE st_bInProcWin=0,st_bBackupDisplayMode=0xff;//st_bBackupChanel=0xff,
 // BIT7->new fill wait init IPW  BIT6->first get data BIT5->get data end        BIT3->in fill down   BIT2->in FILL UP      BIT1->need fill down   BIT0->need FILL UP  0->not need fill
static BYTE st_bNeedFillProcWin=0,st_bFillWinFlag=0; 
static WORD st_wFiberWidth=0;

#if SENSOR_WIN_NUM
//--初始化一个sensor win对应下的所有光纤相关信息参数
void ResetFiberPara(void)
{
	DWORD i;

	for (i=0;i<FIBER_WINDOW_NUM;i++)
	{
		st_swFaceX[i]=-1;
		st_swHlevel[i]=0;
		st_swAngle[i]=0;
		st_swCenterW[i]=0;
		st_swFaceY1[i]=-1;
		st_swFaceY2[i]=-1;
		st_swFaceY20[i]=-1;
		st_swFaceY3[i]=-1;
		st_wMotorSpeed[i]=0;
	}

	//for (i=0;i<SENSER_TOTAL;i++)
	//	st_bFiberBlackLevel[i]=0xff;

}

void PrintWinData()
{
	ST_IMGWIN *pWin;
	BYTE *pbWinBuffer,bValidLevel,bNewLevel,bContinue;
	WORD wValidCnt;
	SWORD x=0,y=0;

	pWin=(ST_IMGWIN *)&SensorInWin[0];
	pbWinBuffer = (BYTE *) pWin->pdwStart;
	bNewLevel=bValidLevel=*pbWinBuffer;
	wValidCnt=1;
	for (y=0;y<pWin->wHeight;y++)
	{
		mpDebugPrint("%02d:",y);
		for (x=0;x<pWin->wWidth/2;x+=2)
		{
			mpDebugPrintN(" %02x ",*pbWinBuffer);
			pbWinBuffer+=4;
		}
	}
	mpDebugPrint("");

}

SDWORD GetAverageData(WORD *wData,WORD wDataCnt)
{
	WORD i;
	DWORD dwTotal=0;

	if (!wDataCnt)
		return FAIL;
	for (i=0;i<wDataCnt;i++)
		dwTotal+=wData[i];

	return dwTotal/wDataCnt;
}

SDWORD GetReallyData(WORD *wData,WORD wDataCnt,WORD *pwVaildCnt,WORD woffset,SWORD swLastData)
{
	DWORD dwValidcnt,dwValidcnt2,dwYValidValue,dwMax=0,dwMin=0xffffffff,dwLen,dwOffset;
	WORD i,j,k,wNeedValid;

	if (pwVaildCnt)
		wNeedValid=*pwVaildCnt;
	else
		wNeedValid=wDataCnt*2/3;
	//--找出最大值与最小值
	for (i=0;i<wDataCnt;i++)
	{
		if (dwMax<wData[i])
			dwMax=wData[i];
		if (dwMin>wData[i])
			dwMin=wData[i];
	}
	dwLen=dwMax-dwMin;
	if (!dwLen)
	{
		if (pwVaildCnt)
			*pwVaildCnt=wDataCnt;
		//mpDebugPrint("ALL=%d ",wData[i]);
		return wData[0];
	}
	/*
	if (dwLen>=10)
	{
		dwStep=dwLen/10;
		dwLen=10;
	}
	else
	{
		dwStep=1;
	}
	*/
	//--找出出现最多的值
	for (k=1;k<=dwLen;k++) // add offset
	{
		//一个个取值出来比较
		for (i=0;i<wDataCnt;i++)
		{
			dwValidcnt=0;
			dwOffset=k;
			if (woffset && dwOffset>woffset)
			{
				if (pwVaildCnt)
					*pwVaildCnt=0;
				//mpDebugPrint("k=%d",k);
				return FAIL;
			}
			//去对比其它的值
			for (j=0;j<wDataCnt;j++)
			{
				if (ABS((SDWORD)wData[i]-(SDWORD)wData[j])<=dwOffset)
				{
					dwValidcnt++;
				}
			}
			//判断是否是较多的
			if (dwValidcnt>wNeedValid)
			{
				dwYValidValue=0;
				for (j=0;j<wDataCnt;j++)
				{
					if (ABS((SDWORD)wData[i]-(SDWORD)wData[j])<=dwOffset)
					{
						dwYValidValue+=wData[j];
					}
				}
				//mpDebugPrint("data[%d]=%d %d/%d-%d offset=%d",i,wData[i],dwYValidValue,dwValidcnt,wDataCnt,dwOffset);
				if (pwVaildCnt)
					*pwVaildCnt=dwValidcnt;
				//return wData[i];
				dwYValidValue=(dwYValidValue+dwValidcnt/2)/dwValidcnt;
				//--新数据要比旧数据多一倍以上
				if (swLastData>=0 && swLastData != dwYValidValue)
				{
					dwValidcnt2=0;
					for (j=0;j<wDataCnt;j++)
					{
						if (ABS(swLastData-(SDWORD)wData[j])<=dwOffset)
						{
							dwValidcnt2++;
						}
					}
					if (dwValidcnt2>dwValidcnt*2/3)
						return swLastData;
				}
				return dwYValidValue;
			}
		}
		
	}

	if (pwVaildCnt)
		*pwVaildCnt=0;
	//mpDebugPrint("dwLen=%d dwOffset=%d",dwLen,dwOffset);
	return FAIL;
}

SDWORD GetFiberBlackLevel(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwValidlevelcnt,dwBlackAverLevel=0,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt;
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel,bSensorIndex;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!Left black error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;

	//--获取实际光纤边黑色部分的亮度值
	{
		swStartY=st_swFaceY1[bMode]+8;
		swYEnd=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/3;
		#if DISPLAY_IN_ONE_WIN
		if (g_bDisplayMode==0x02)
		{
			if (bMode>1)
			{
				swStartY-=pWin->wHeight/2;
				swYEnd-=pWin->wHeight/2;
			}
			swStartY<<=1;
			swYEnd<<=1;
		}
		#endif
		if (bMode==FIBER_RIGHT_TOP)
			x=pWin->wWidth<<1;
		else
			x=st_swFaceX[bMode]<<1;
		if (x>X_STEP)
			x-=X_STEP;
		for (;x>0;x-=4)
		{
			wValidPixelCnt=0;
			bContinueCnt=0;
			y=swStartY;
			pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
			dwValidlevelcnt=0;
			dwBlackAverLevel=0;
			//mpDebugPrint(" x=%d need=%d: ",x,(st_swFaceY3[bMode]-st_swFaceY1[bMode])/5);
			while (y<swYEnd)
			{
				//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (wValidPixelCnt)
				{
					if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex] )
					{
						wValidPixelCnt++;
						dwValidlevelcnt+=*pbWinBuffer;
						//mpDebugPrintN("%02x ",*pbWinBuffer);
						//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_GREEN);
					}
					else
					{
						break;
					}
				}
				else
				{
					if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]  )
					{
						bContinueCnt++;
						dwValidlevelcnt+=*pbWinBuffer;
						//mpDebugPrintN("%02x ",*pbWinBuffer);
						//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_GREEN);
						if (bContinueCnt>Y_CENTER_CONTINUE_VALID_SUM)
						{
							wValidPixelCnt=bContinueCnt;
						}
					}
					else if (bContinueCnt)
					{
						bContinueCnt=0;
						dwValidlevelcnt=0;
					}
				}

				if (wValidPixelCnt==(st_swFaceY3[bMode]-st_swFaceY1[bMode])/5)
				{
					dwBlackAverLevel=dwValidlevelcnt/wValidPixelCnt;
					st_bFiberBlackLevel[0]=st_bFiberBlackLevel[1]=dwBlackAverLevel<<1;
					mpDebugPrint("Core Black 0x%02x ",dwBlackAverLevel);
					return dwBlackAverLevel;
				}

				y++;
				pbWinBuffer+=pWin->dwOffset;
			}
			if (dwBlackAverLevel)
				break;
			TaskYield();
			//mpDebugPrint(" ");
		}
	}

	return FAIL;
}

DWORD GetWinBrightness(ST_IMGWIN *pWin,BYTE bSkipX,BYTE bSkipY)// bSkipX->per 2pixel   bSkipY->one line
{
	BYTE *pbWinBuffer;
	DWORD dwBrightness=0,dwCnt=0;
	DWORD x,y;

	pbWinBuffer = (BYTE *)((DWORD) pWin->pdwStart| 0xA0000000);
	if (!bSkipX)
		bSkipX=1;
	if (!bSkipY)
		bSkipY=1;
	for (y=0;y<pWin->wHeight;y+=bSkipY)
	{
		for (x=0;x<pWin->dwOffset;x+=4*bSkipX)
		{
			dwBrightness+=*pbWinBuffer;
			dwCnt++;
			pbWinBuffer+=4*bSkipX;
		}
		pbWinBuffer+=pWin->dwOffset*(bSkipY-1);
	}
	if (dwCnt)
		dwBrightness/=dwCnt;

	return dwBrightness;
}

void SetFillProcWinFlag(void)
{
	IPU *ipu = (IPU *) IPU_BASE;

	#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
		//	ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
	#else
		//	ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
	#endif //--------------------------
	//TaskYield();
	//TaskSleep(10);
	
#if IPW_FAST_MODE
		//st_bBackupChanel=Sensor_CurChannel_Get();
		st_bNeedFillProcWin=FILL_WIN_UP|FILL_WIN_DOWN|FILL_WIN_END;
#else
	if (st_bFillWinFlag &(FILL_WIN_UP|FILL_WIN_DOWN))
		st_bNeedFillProcWin=FILL_WIN_INIT|(st_bFillWinFlag &(FILL_WIN_UP|FILL_WIN_DOWN)); // bit0->up win  bit1->down win   bit7->init mode
	else
		st_bNeedFillProcWin=FILL_WIN_INIT|FILL_WIN_UP|FILL_WIN_DOWN; // bit0->up win  bit1->down win   bit7->init mode
		//st_bNeedFillProcWin=0xe1; // bit0->up win  bit1->down win   bit7->init mode
#endif
	//--显示抖动，可能只更新半边LCD
	//mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());
}

void TimerToFillProcWin(DWORD dwTime) // ms
{
	Ui_TimerProcAdd(dwTime, SetFillProcWinFlag);
}

void TimerToFillReferWin(DWORD dwTime,BYTE bFillWinFlag) // ms
{
	st_bFillWinFlag=bFillWinFlag;
	Ui_TimerProcAdd(dwTime, SetFillProcWinFlag);
}

void TimerCheckToFillReferWin(DWORD dwTime) // ms
{
	if (g_bDisplayMode==0)
		st_bFillWinFlag=FILL_WIN_UP;
	else if (g_bDisplayMode==1)
		st_bFillWinFlag=FILL_WIN_DOWN;
	else
		st_bFillWinFlag=FILL_WIN_UP|FILL_WIN_DOWN;
	Ui_TimerProcAdd(dwTime, SetFillProcWinFlag);
}

void Weld_StartPause()
{
	DWORD i;

	//mpDebugPrint("Weld_StartPause- %p",st_dwProcState);
	if (st_dwProcState & BIT30)
	{
		st_dwProcState &=~BIT30;
		//if (st_dwProcState==SENSOR_DISCHARGE2)
		//	st_dwProcState=SENSOR_FACE_POS2B;
		EventSet(UI_EVENT, EVENT_PROC_DATA);
	}
	else if (st_dwProcState==SENSOR_IDLE)
	{
		st_dwProcState=SENSOR_PROC_INIT;
		EventSet(UI_EVENT, EVENT_PROC_DATA);
	}
	else
	{
		EventClear(UI_EVENT, EVENT_PROC_DATA);
		st_dwProcState |= BIT30;
		StopAllMoto();
	}
	mpDebugPrint("Weld_StartPause %p",st_dwProcState);

}

void GetBackgroundLevel(void)
{
	ST_IMGWIN *pWin;
	BYTE *pbWinBuffer,bValidLevel,bNewLevel,bContinue,bSensorIndex;
	WORD wValidCnt;
	SWORD y;
	static BYTE st_bRetryTimes=0;

	if (st_bNeedFillProcWin)
	{
		Ui_TimerProcAdd(500, GetBackgroundLevel);
		return;
	}
	if ((st_bFillWinFlag&FILL_WIN_UP) && !g_psSetupMenu->bBackGroundLevel[0])
		bSensorIndex=SENSER_TOP;
	else if ((st_bFillWinFlag&FILL_WIN_DOWN) && !g_psSetupMenu->bBackGroundLevel[1])
		bSensorIndex=SENSER_BOTTOM;
	else if (!g_psSetupMenu->bBackGroundLevel[0])
	{
		TimerToFillReferWin(10,FILL_WIN_UP);
		Ui_TimerProcAdd(100, GetBackgroundLevel);
		return;
	}
	else if (!g_psSetupMenu->bBackGroundLevel[1])
	{
		TimerToFillReferWin(10,FILL_WIN_DOWN);
		Ui_TimerProcAdd(100, GetBackgroundLevel);
		return;
	}
	pWin=(ST_IMGWIN *)&SensorInWin[bSensorIndex];

	y=4;
	pbWinBuffer = (BYTE *) pWin->pdwStart+(g_psSetupMenu->wElectrodePos[0]-st_bRetryTimes*8)*2+y*pWin->dwOffset;
	bNewLevel=bValidLevel=*pbWinBuffer;
	wValidCnt=1;
	for (;y<pWin->wHeight/2;y++)
	{
		pbWinBuffer+=pWin->dwOffset;
		//mpDebugPrintN("%02x ",*((BYTE *)pbWinBuffer));
		if (*pbWinBuffer>bValidLevel+LEVEL_OFFSET)
		{
			 // 在新电平内
			if (*pbWinBuffer+LEVEL_OFFSET>bNewLevel  && *pbWinBuffer<bNewLevel+LEVEL_OFFSET)
			{
				bContinue++;
				if (bContinue>pWin->wHeight/8)
				{
					bValidLevel=bNewLevel;
					wValidCnt=bContinue;
					bContinue=0;
				}
			}
			 // 新更亮点
			else if (*pbWinBuffer+LEVEL_OFFSET>bNewLevel)
			{
				bNewLevel=*pbWinBuffer;
				bContinue=1;
				//mpDebugPrintN("-%02x-",*((BYTE *)pbWinBuffer));
			}
			if (wValidCnt<=pWin->wHeight/8)
				wValidCnt=0;
		}
		else if (*pbWinBuffer>bValidLevel-LEVEL_OFFSET)
		{
			wValidCnt++;
			bContinue=0;
		}
		else
		{
			if (wValidCnt<=pWin->wHeight/8)
				wValidCnt=0;
		}

		if (wValidCnt>pWin->wHeight/16 &&  bValidLevel>8) // /8
		{
			g_psSetupMenu->bBackGroundLevel[bSensorIndex]=(DWORD)bValidLevel*7/10; // ok1
			//g_psSetupMenu->bBackGroundLevel[bSensorIndex]=(DWORD)bValidLevel/4; //7/10
			WriteSetupChg();
			break;
		}
	}

	if (g_psSetupMenu->bBackGroundLevel[bSensorIndex])
	{
		mpDebugPrint("---GetBackgroundLevel:%d %p->%p",bSensorIndex,bValidLevel,g_psSetupMenu->bBackGroundLevel[bSensorIndex]);
		//DriveMotor(03,0,300,8);//RIGHT_DOWN_FIBER  DOWN
	}
	else
	{
		if (st_bRetryTimes<10)
		{
			mpDebugPrint("----bValidLevel=%p wValidCnt=%d ",bValidLevel,wValidCnt);
			st_bRetryTimes++;
			//DriveMotor(01,0,50,8);
			//DriveMotor(02,0,50,8);
			TimerToFillProcWin(10);
			Ui_TimerProcAdd(500, GetBackgroundLevel);
			return;
		}
		else
		{
			g_psSetupMenu->bBackGroundLevel[bSensorIndex]=FIBER_EDGE_LEVEL;
			mpDebugPrint("----g_psSetupMenu->bBackGroundLevel[%d] FAIL !!!reset to %p ",bSensorIndex,g_psSetupMenu->bBackGroundLevel[bSensorIndex]);
		}
	}

	if (!g_psSetupMenu->bBackGroundLevel[0])
	{
		st_bFillWinFlag=FILL_WIN_UP;
		Ui_TimerProcAdd(500, GetBackgroundLevel);
	}
	else if (!g_psSetupMenu->bBackGroundLevel[1])
	{
		st_bFillWinFlag=FILL_WIN_DOWN;
		Ui_TimerProcAdd(500, GetBackgroundLevel);
	}
	else
	{
		st_bFillWinFlag=FILL_WIN_UP|FILL_WIN_DOWN;
		//Ui_TimerProcAdd(2000, AutoStartWeld);
	}
	TimerToFillProcWin(10);

}

#define		BG_BLACK_LEVEL					0x02
void ScanBackgroundLevel(void) //  ScanWholeWin
{
	ST_IMGWIN *pWin;
	BYTE *pbWinBuffer,bNewLevel,bSensorIndex;
	SWORD x,y;

	if (st_bNeedFillProcWin)
	{
		Ui_TimerProcAdd(500, ScanBackgroundLevel);
		return;
	}
	if (g_psSetupMenu->bBackGroundLevel[0] && g_psSetupMenu->bBackGroundLevel[1])
		return;
	MP_DEBUG("-- %s-%p-%d %d ",__FUNCTION__,st_bFillWinFlag,g_psSetupMenu->bBackGroundLevel[0],g_psSetupMenu->bBackGroundLevel[1]);
	//MP_DEBUG("-- %s--",__FUNCTION__);
	if ((st_bFillWinFlag&FILL_WIN_UP) && !g_psSetupMenu->bBackGroundLevel[0])
		bSensorIndex=SENSER_TOP;
	else if ((st_bFillWinFlag&FILL_WIN_DOWN) && !g_psSetupMenu->bBackGroundLevel[1])
		bSensorIndex=SENSER_BOTTOM;
	else if (!g_psSetupMenu->bBackGroundLevel[0])
	{
		TimerToFillReferWin(10,FILL_WIN_UP);
		Ui_TimerProcAdd(100, ScanBackgroundLevel);
		return;
	}
	else if (!g_psSetupMenu->bBackGroundLevel[1])
	{
		TimerToFillReferWin(10,FILL_WIN_DOWN);
		Ui_TimerProcAdd(100, ScanBackgroundLevel);
		return;
	}
	pWin=(ST_IMGWIN *)&SensorInWin[bSensorIndex];

	for (x=X_START;x<pWin->wWidth-X_START;x+=4)
	{
		y=ScanFiberUptoDown(pWin,x<<1,2,pWin->wHeight-2,BG_BLACK_LEVEL);
		if (y>Y_VALID_PIXEL)
		{
			mpDebugPrint("!!! %s--x=%d y=%d ",__FUNCTION__,x,y);
			g_psSetupMenu->bBackGroundLevel[bSensorIndex]=FIBER_EDGE_LEVEL;
			break;
		}
	}
	if (y>Y_VALID_PIXEL)
	{
		Ui_TimerProcAdd(1, ScanBackgroundLevel);
		return;
	}
	bNewLevel=0xff;
	for (y=2;y<pWin->wHeight-2;y+=2)
	{
		x=X_START;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x*2+y*pWin->dwOffset;
		for (;x<pWin->wWidth-X_START;x+=4,pbWinBuffer+=8)
		{
			if (*pbWinBuffer<BG_BLACK_LEVEL)
				continue;
			else if (bNewLevel>*pbWinBuffer)
				bNewLevel=*pbWinBuffer;
		}
	}
	g_psSetupMenu->bBackGroundLevel[bSensorIndex]=bNewLevel;
	WriteSetupChg();
	mpDebugPrint("[%s] g_psSetupMenu->bBackGroundLevel[%d]=%p ",__FUNCTION__,bSensorIndex,g_psSetupMenu->bBackGroundLevel[bSensorIndex]);
	Ui_TimerProcAdd(1, ScanBackgroundLevel);
}

SWORD ScanFiberUptoDown(ST_IMGWIN *pWin, SWORD x,SWORD swYstart,SWORD swYEnd,BYTE bLowLevel)
{
	BYTE *pbWinBuffer,bContinueCnt=0;
	WORD wValidPixelCnt=0,wInvalidPixelCnt=0;

		pbWinBuffer = (BYTE *) pWin->pdwStart+x+swYstart*pWin->dwOffset;
		//mpDebugPrintN(" %d: l=%x ",x,bLowLevel);
		while (swYstart<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
					//mpDebugPrintN(" %02x= ",*pbWinBuffer);
				if (*pbWinBuffer < bLowLevel )
					wValidPixelCnt++;
				else
				{
					wInvalidPixelCnt++;
					if (wInvalidPixelCnt>4)
						wValidPixelCnt=0;
				}
			}
			else
			{
				if (*pbWinBuffer < bLowLevel)
				{
					//Idu_OsdPaintArea(x>>1, swYstart, 2, 1, OSD_COLOR_GREEN);
					//if (!bContinueCnt)
					//	mpDebugPrintN(" %02x+ ",*pbWinBuffer);
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
						wInvalidPixelCnt=0;
					}
				}
				else
				{
					//mpDebugPrintN(" %02x- ",*pbWinBuffer);
					bContinueCnt=0;
				}
			}
			 if (/*(x>0)&&*/wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				//mpDebugPrint("x=%d  %d/%d",x,wValidPixelCnt,wInvalidPixelCnt);
				return swYstart-wValidPixelCnt-wInvalidPixelCnt+2;
			}
			swYstart++;
			pbWinBuffer+=pWin->dwOffset;
		}
		//mpDebugPrint("");
#ifndef X_START
		if (x==0||x==pWin->dwOffset)
		{
		//--判断靠边的起始第一个数据是否有效
			if (wValidPixelCnt<pWin->wHeight*2/3 && wInvalidPixelCnt < pWin->wHeight*2/3)
			{
				return PASS;
			}
		}
#endif
		//mpDebugPrint("-x=%d  %d/%d",x,wValidPixelCnt,wInvalidPixelCnt);
		return FAIL;
}

SWORD ScanFiberUptoDownDebug(ST_IMGWIN *pWin, SWORD x,SWORD swYstart,SWORD swYEnd,BYTE bLowLevel)
{
	BYTE *pbWinBuffer,bContinueCnt=0;
	WORD wValidPixelCnt=0,wInvalidPixelCnt=0;

		pbWinBuffer = (BYTE *) pWin->pdwStart+x+swYstart*pWin->dwOffset;
		mpDebugPrint("\r\n");
		//mpDebugPrintN(" %d: l=%x ",x,bLowLevel);
		while (swYstart<swYEnd)
		{
			if (wValidPixelCnt)
			{
					mpDebugPrintN(" %02x= ",*pbWinBuffer);
				if (*pbWinBuffer < bLowLevel )
					wValidPixelCnt++;
				else
				{
					wInvalidPixelCnt++;
					if (wInvalidPixelCnt>4)
						wValidPixelCnt=0;
				}
			}
			else
			{
				if (*pbWinBuffer < bLowLevel)
				{
					Idu_OsdPaintArea(x>>1, swYstart, 2, 1, OSD_COLOR_GREEN);
					if (!bContinueCnt)
						mpDebugPrintN(" %02x+%04d ",*pbWinBuffer,swYstart);
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
						wInvalidPixelCnt=0;
					}
				}
				else
				{
					mpDebugPrintN(" %02x- ",*pbWinBuffer);
					bContinueCnt=0;
				}
			}
			 if (/*(x>0)&&*/wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				//mpDebugPrint("x=%d  %d/%d",x,wValidPixelCnt,wInvalidPixelCnt);
				return swYstart-wValidPixelCnt-wInvalidPixelCnt+2;
			}
			swYstart++;
			pbWinBuffer+=pWin->dwOffset;
		}
		mpDebugPrint("");
#ifndef X_START
		if (x==0||x==pWin->dwOffset)
		{
		//--判断靠边的起始第一个数据是否有效
			if (wValidPixelCnt<pWin->wHeight*2/3 && wInvalidPixelCnt < pWin->wHeight*2/3)
			{
				return PASS;
			}
		}
#endif
		//mpDebugPrint("-x=%d  %d/%d",x,wValidPixelCnt,wInvalidPixelCnt);
		return FAIL;
}
SWORD ScanFiberDowntoUp(ST_IMGWIN *pWin, SWORD x,SWORD swYstart,SWORD swYEnd,BYTE bLowLevel)
{
	BYTE *pbWinBuffer,bContinueCnt=0;
	WORD wValidPixelCnt=0,wInvalidPixelCnt=0;

		pbWinBuffer = (BYTE *) pWin->pdwStart+x+swYstart*pWin->dwOffset;
		//mpDebugPrintN(" %d: l=%x ",x,bLowLevel);
		while (swYstart>swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
					//mpDebugPrintN(" %02x= ",*pbWinBuffer);
				if (*pbWinBuffer < bLowLevel )
					wValidPixelCnt++;
				else
				{
					wInvalidPixelCnt++;
					if (wInvalidPixelCnt>4)
						wValidPixelCnt=0;
				}
			}
			else
			{
				if (*pbWinBuffer < bLowLevel)
				{
					//Idu_OsdPaintArea(x>>1, swYstart, 2, 1, OSD_COLOR_GREEN);
					//if (!bContinueCnt)
					//	mpDebugPrintN(" %02x+ ",*pbWinBuffer);
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
						wInvalidPixelCnt=0;
					}
				}
				else
				{
					//mpDebugPrintN(" %02x- ",*pbWinBuffer);
					bContinueCnt=0;
				}
			}
			 if (/*(x>0)&&*/wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				//mpDebugPrint("x=%d swYstart=%d",x,swYstart+wValidPixelCnt+wInvalidPixelCnt-1);
				return swYstart+wValidPixelCnt+wInvalidPixelCnt;
			}
			swYstart--;
			pbWinBuffer-=pWin->dwOffset;
		}
		//mpDebugPrint("");
#ifndef X_START
		if (x==0||x==pWin->dwOffset)
		{
		//--判断靠边的起始第一个数据是否有效
			if (wValidPixelCnt<pWin->wHeight*2/3 && wInvalidPixelCnt < pWin->wHeight*2/3)
			{
				return PASS;
			}
		}
#endif
		//mpDebugPrint("-x=%d  %d/%d",x,wValidPixelCnt,wInvalidPixelCnt);
		return FAIL;
}


SWORD ScanFiberFaceExistPos(ST_IMGWIN *pWin,BYTE bWinIndex,SWORD x)
{
	
	return ScanFiberUptoDown(pWin,x<<1,0,pWin->wHeight,g_psSetupMenu->bBackGroundLevel[bWinIndex]>>BG_BRIGHT_SHIFT);
}

SDWORD ScanFiberCenterWidth(ST_IMGWIN *pWin,BYTE bMode)
{
	SDWORD sdwRet;
	WORD wValidPixelCnt,wContinueValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,*wYValidArry,wValidWidthCnt;
	SWORD x,y,swStartX,swEndX,swStartY,swYEnd,swYValidStartAver;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex,bBlackBG;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!GetCenter error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	bSensorIndex=bMode>FIBER_RIGHT_TOP;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/6;
	swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/6;
	if (bMode&0x01) //right
	{
		swStartX=st_swFaceX[bMode]+X_START;
		swEndX=pWin->wWidth-X_START;
	}
	else
	{
		swStartX=X_START;
		swEndX=st_swFaceX[bMode]-X_START;
	}
	//--获取实际光纤边黑色部分的亮度值
	if (st_bFiberBlackLevel[bSensorIndex]==0xff)
	{
		sdwRet=GetFiberBlackLevel(pWin,bMode);
		if (sdwRet>=0)
			st_bFiberBlackLevel[bSensorIndex]=sdwRet<<1;
		else
			return FAIL;
	}
	bBlackBG=st_bFiberBlackLevel[bSensorIndex]; // g_psSetupMenu->bBackGroundLevel[bSensorIndex]
	bBlackBG++;

	wYValidArry = (WORD *)ext_mem_malloc(ALIGN_32(swEndX-swStartX)<<1);
	memset(wYValidArry,0,ALIGN_32(swEndX-swStartX)<<1);
	wValidWidthCnt=0;
	for (x=swStartX;x<swEndX;x+=2)
	{
		wValidPixelCnt=0;
		wContinueValidPixelCnt=0;
		wInvalidPixelCnt=0;
		wEndInvalidCnt=0;
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) ((DWORD) pWin->pdwStart| 0xA0000000)+x*2+y*pWin->dwOffset;
		//mpDebugPrint(" x=%d y=%d swYEnd=%d: ",x>>1,y,swYEnd);
		for (;y<swYEnd;y++,pbWinBuffer+=pWin->dwOffset)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			//Idu_OsdPaintArea(x, y, 2, 1, OSD_COLOR_GREEN);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer > bBlackBG )
				{
					wValidPixelCnt++;
					wEndInvalidCnt=0;
				}
				else
				{
					wInvalidPixelCnt++;
					wEndInvalidCnt++;
				//Idu_OsdPaintArea(x, y, 2, 1, OSD_COLOR_GREEN);
				}
			}
			else
			{
				if (*pbWinBuffer > bBlackBG )
				{
					bContinueCnt++;
					if (bContinueCnt>Y_CENTER_CONTINUE_VALID_SUM)
					{
						//mpDebugPrintN(" -- ");
						//Idu_OsdPaintArea(x, y-bContinueCnt, 2, 1, OSD_COLOR_GREEN);
						wValidPixelCnt=bContinueCnt;
					}
				}
				else if (bContinueCnt)
				{
					bContinueCnt=0;
				}
			}
			if (wEndInvalidCnt>Y_VALID_OFFSET)
			{
				break;
			}
		}
		if (bContinueCnt && wValidPixelCnt>wEndInvalidCnt)
		{
			wYValidArry[wValidWidthCnt++]=wValidPixelCnt-wEndInvalidCnt;
			//mpDebugPrintN("%d ",wYValidArry[wValidWidthCnt-1]);
		}
		TaskYield();
		//mpDebugPrint(" ");
	}
	//mpDebugPrint(" ");
	sdwRet=GetAverageData(wYValidArry,wValidWidthCnt);
	if (sdwRet>0)
	{
		wValidPixelCnt=0;
		for (x=0;x<wValidWidthCnt;x++)
		{
			if (sdwRet>wYValidArry[x])
				wValidPixelCnt+=sdwRet-wYValidArry[x];
			else
				wValidPixelCnt+=wYValidArry[x]-sdwRet;
		}
		sdwRet=(wValidPixelCnt<<6)/wValidWidthCnt;
	}
	mpDebugPrint(" Center[%d]=%d  (%d)",bMode,sdwRet,wValidWidthCnt);
	ext_mem_free(wYValidArry);
	return sdwRet;
}

#if ALIGN_NEW_MODE
SWORD SearchLeftFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset;
	WORD wYValidStartArry[X_SCAN_VALID_NUM],wValidLineCnt;
	SWORD x,y,i,swStartY,swYEnd,swYValidStartAver,swRet;
	BYTE *pbWinBuffer,bSensorIndex;

	if (st_swFaceX[bMode]<=0)
		return FAIL;
	dwOffset=pWin->dwOffset;
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;

#if (SENSOR_WIN_NUM==2)
		swStartY=pWin->wHeight-1;
		swYEnd=4;
#else
	if (bMode<2)
	{
		swStartY=pWin->wHeight/2-1;
		swYEnd=0;
	}
	else
	{
		swStartY=pWin->wHeight-1;
		swYEnd=pWin->wHeight/2;
	}
#endif
	wValidLineCnt=0;

	for (x=st_swFaceX[bMode]*2-X_STEP*4;x>0;x-=X_STEP)
	{
		swRet=ScanFiberDowntoUp(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
		if (swRet==FAIL)
		{
			break;
		}
		else if (swRet>0)
		{
			if (wValidLineCnt<X_SCAN_VALID_NUM)
			{
				wYValidStartArry[wValidLineCnt]=swRet-1;
				//mpDebugPrintN(" %d ",wYValidStartArry[wValidLineCnt]);
				//Idu_OsdPaintArea(x>>1, wYValidStartArry[wValidLineCnt], 2, 1, OSD_COLOR_RED);
				wValidLineCnt++;
			}
			else
				break;
		}
		//TaskYield();
	}
	//mpDebugPrint("");
	//计算有效Y起始坐标
	//DEBUG_POS("wValidLineCnt %d ",wValidLineCnt);
	if (wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(&wYValidStartArry[0],wValidLineCnt,NULL,i,st_swFaceY3[bMode]);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			#if DISPLAY_IN_ONE_WIN
			if (g_bDisplayMode==0x02)
			{
				swYValidStartAver>>=1;
				if (bMode>1)
					swYValidStartAver+=pWin->wHeight/2;
			}
			#endif
			if (st_swFaceY3[bMode]!=swYValidStartAver)
			{
				//st_swFaceNewY3[bMode].wCnt=1;
				st_swFaceY3[bMode]=swYValidStartAver;
				DEBUG_POS("-----------Y3[%d]=%d ",bMode,st_swFaceY3[bMode]);
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
				#endif
				#if TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]-st_swFaceY1[bMode])
				{
					swYValidStartAver=(st_swFaceY3[bMode]+st_swFaceY1[bMode])>>1;
					if (swYValidStartAver>pWin->wHeight/2)
						sprintf(&st_bStrCenter[bMode][0], "^:%d",(swYValidStartAver-pWin->wHeight/2));
					else
						sprintf(&st_bStrCenter[bMode][0], "v:%d",pWin->wHeight/2-swYValidStartAver);
					xpgSetUpdateOsdFlag(1);
				}
				#endif
				//return ENABLE;
			}
			return PASS;
		}
	}

	return FAIL;
}
#else
SWORD SearchLeftFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset;
	WORD wYValidStartArry[X_SCAN_VALID_NUM],wValidLineCnt;
	SWORD x,y,i,swStartY,swYEnd,swYValidStartAver,swRet;
	BYTE *pbWinBuffer,bSensorIndex;

	dwOffset=pWin->dwOffset;
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;

#if (SENSOR_WIN_NUM==2)
		swStartY=pWin->wHeight-1;
		swYEnd=4;
#else
	if (bMode<2)
	{
		swStartY=pWin->wHeight/2-1;
		swYEnd=0;
	}
	else
	{
		swStartY=pWin->wHeight-1;
		swYEnd=pWin->wHeight/2;
	}
#endif
	wValidLineCnt=0;

	for (x=X_START;x<dwOffset;x+=X_STEP)
	{
		swRet=ScanFiberDowntoUp(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
		if (swRet==FAIL)
		{
			break;
		}
		else if (swRet>0)
		{
			if (wValidLineCnt<X_SCAN_VALID_NUM)
			{
				wYValidStartArry[wValidLineCnt]=swRet-1;
				//mpDebugPrintN(" %d ",wYValidStartArry[wValidLineCnt]);
				wValidLineCnt++;
			}
		}
		//TaskYield();
	}
	//mpDebugPrint("");
	//计算有效Y起始坐标
	//DEBUG_POS("wValidLineCnt %d ",wValidLineCnt);
	if (wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(&wYValidStartArry[0],wValidLineCnt,NULL,i,st_swFaceY3[bMode]);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			#if DISPLAY_IN_ONE_WIN
			if (g_bDisplayMode==0x02)
			{
				swYValidStartAver>>=1;
				if (bMode>1)
					swYValidStartAver+=pWin->wHeight/2;
			}
			#endif
			if (st_swFaceY3[bMode]!=swYValidStartAver)
			{
				//st_swFaceNewY3[bMode].wCnt=1;
				st_swFaceY3[bMode]=swYValidStartAver;
				DEBUG_POS("-----------Y3[%d]=%d ",bMode,st_swFaceY3[bMode]);
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
				#endif
				#if TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]-st_swFaceY1[bMode])
				{
					swYValidStartAver=(st_swFaceY3[bMode]+st_swFaceY1[bMode])>>1;
					if (swYValidStartAver>pWin->wHeight/2)
						sprintf(&st_bStrCenter[bMode][0], "^:%d",(swYValidStartAver-pWin->wHeight/2));
					else
						sprintf(&st_bStrCenter[bMode][0], "v:%d",pWin->wHeight/2-swYValidStartAver);
					xpgSetUpdateOsdFlag(1);
				}
				#endif
				//return ENABLE;
			}
			return PASS;
		}
	}

	return FAIL;
}
#endif

SWORD SearchLeftFiberCenter(ST_IMGWIN *pWin,BYTE bMode)
{
	SDWORD sdwRet;
	DWORD dwOffset;
	WORD wValidPixelCnt,wContinueValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_SCAN_MAX_SUM],wYStartIndex;
	SWORD x,y,i,swStartY,swYEnd,swYValidStartAver;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex,bBlackBG;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!LeftCenter error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/6;
#if (SENSOR_WIN_NUM==2)
		swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/6;
		#if DISPLAY_IN_ONE_WIN
		if (g_bDisplayMode==0x02)
		{
			if (bMode>1)
			{
				swStartY-=pWin->wHeight/2;
				swYEnd-=pWin->wHeight/2;
			}
			swStartY<<=1;
			swYEnd<<=1;
		}
		#endif
#else
	if (bMode<2)
	{
		swYEnd=pWin->wHeight/2;
	}
	else
	{
		swYEnd=pWin->wHeight;
	}
#endif
	//--获取实际光纤边黑色部分的亮度值
	if (st_bFiberBlackLevel[bSensorIndex]==0xff)
	{
		sdwRet=GetFiberBlackLevel(pWin,bMode);
		if (sdwRet>=0)
			st_bFiberBlackLevel[bSensorIndex]=sdwRet<<1;
		else
			return FAIL;
	}
	bBlackBG=st_bFiberBlackLevel[bSensorIndex]; // g_psSetupMenu->bBackGroundLevel[bSensorIndex]
	bBlackBG++;

	for (i=0;i<X_SCAN_MAX_SUM;i++)
		wYValidArry[i]=0;
	wYStartIndex=0;
	x=st_swFaceX[bMode]<<1;
	x-=X_STEP*4;
	for (x=st_swFaceX[bMode]<<1;x>0;x-=X_STEP)
	{
		wValidPixelCnt=0;
		wContinueValidPixelCnt=0;
		wInvalidPixelCnt=0;
		wEndInvalidCnt=0;
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		//mpDebugPrint(" x=%d y=%d swYEnd=%d: ",x>>1,y,swYEnd);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_GREEN);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer > bBlackBG )
				{
					wValidPixelCnt++;
					wEndInvalidCnt=0;
				}
				else
				{
					wInvalidPixelCnt++;
					wEndInvalidCnt++;
				}
			}
			else
			{
				if (*pbWinBuffer > bBlackBG )
				{
					bContinueCnt++;
					if (bContinueCnt>Y_CENTER_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
					}
				}
				else if (bContinueCnt)
				{
					bContinueCnt=0;
				}
			}

			if (wValidPixelCnt>CENTER_MIN_PIXEL && wEndInvalidCnt>Y_VALID_OFFSET && wValidPixelCnt>((wInvalidPixelCnt-wEndInvalidCnt)<<3))
			{
				if (wYStartIndex<X_SCAN_MAX_SUM)
				{
					wYValidArry[wYStartIndex++]=y-wEndInvalidCnt-(wValidPixelCnt+wInvalidPixelCnt-wEndInvalidCnt)/2;
					//mpDebugPrintN("  -%d:%d  %d %d %d %d  ",wYStartIndex,wYValidArry[wYStartIndex-1],y,wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt);
					//计算有效Y起始坐标
					if (wYStartIndex>X_SCAN_VALID_NUM)
					{
						for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
						{
							swYValidStartAver=GetReallyData(wYValidArry,wYStartIndex,NULL,i,st_swFaceY2[bMode]);
							if (swYValidStartAver>0)
								break;
						}
						if (swYValidStartAver>0)
						{
							#if DISPLAY_IN_ONE_WIN
							if (g_bDisplayMode==0x02)
							{
									swYValidStartAver>>=1;
								if (bMode>1)
									swYValidStartAver+=pWin->wHeight/2;
							}
							#endif
							{
								if (st_swFaceY2[bMode]!=swYValidStartAver)
								{
									//st_swFaceNewY2[bMode].wCnt=1;
									st_swFaceY2[bMode]=swYValidStartAver;
									DEBUG_POS(" Y2[%d]=%d ",bMode,st_swFaceY2[bMode]);
									#if 0//TEST_PLANE||ALIGN_DEMO_MODE
									sprintf(&st_bStrCore[bMode][0], "Center0:%d", st_swFaceY2[bMode]);
									xpgSetUpdateOsdFlag(1);
									#endif
									#if OSD_LINE_NUM &&!(TEST_PLANE||ALIGN_DEMO_MODE)
									OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,8+bMode,0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_RED);
									#endif
									//return ENABLE;
								}
							}
							wYStartIndex=X_SCAN_MAX_SUM;
							return PASS;
						}
					}
				}
				break;
			}

			y++;
			pbWinBuffer+=pWin->dwOffset;
		}
		TaskYield();
		//mpDebugPrint(" ");
	}
	
	return FAIL;
}

SWORD SearchLeftFiberCore(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwValidlevelcnt,dwBlackAverLevel,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_SCAN_MAX_SUM];
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0,swCoreLenth;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel,bSensorIndex;
	SDWORD sdwRet;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!Left core error[%d]!!! %d,%d: ",bMode,st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	swCoreLenth=st_swFaceX[bMode];
	if (swCoreLenth<(pWin->wWidth>>3))
		return FAIL;
	swCoreLenth=swCoreLenth/2/X_STEP;
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;

	//--获取实际光纤边黑色部分的亮度值
	if (st_bFiberBlackLevel[bSensorIndex]==0xff)
	{
		sdwRet=GetFiberBlackLevel(pWin,bMode);
		if (sdwRet>=0)
			st_bFiberBlackLevel[bSensorIndex]=sdwRet<<1;
		else
			return FAIL;
	}
	
	//--找纤芯上边沿
	BYTE bColorIndex=OSD_COLOR_WHITE;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	#if DISPLAY_IN_ONE_WIN
	if (g_bDisplayMode==0x02)
	{
		if (bMode>1)
		{
			swStartY-=pWin->wHeight/2;
			swYEnd-=pWin->wHeight/2;
		}
		swStartY<<=1;
		swYEnd<<=1;
	}
	#endif
	x=st_swFaceX[bMode]<<1;
	x-=X_STEP*4;
	for (i=0;i<X_SCAN_MAX_SUM;i++)
		wYValidArry[i]=0;
	wValidPixelCnt=0;
	for (;x>0;x-=X_STEP)
	{
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		bLastLevel=*pbWinBuffer;
		//mpDebugPrint(" x=%d: ",x);
		for (;y<swYEnd;y++,pbWinBuffer+=pWin->dwOffset)
		{
			if (*pbWinBuffer<st_bFiberBlackLevel[bSensorIndex]*4)
				continue;
			//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (*pbWinBuffer<bLastLevel)
				{
					bContinueCnt++;
					if (bContinueCnt>=3)
					{
						//bColorIndex++;
						wYValidArry[wValidPixelCnt]=y-bContinueCnt+1;
						//Idu_OsdPaintArea(x>>1, wYValidArry[wValidPixelCnt], 2, 1, OSD_COLOR_RED);
						wValidPixelCnt++;
						break;
					}
				}
				else
				{
					bContinueCnt=0;
					//Idu_OsdPaintArea(x>>1, y, 2, 1, bColorIndex);
					bLastLevel=*pbWinBuffer;
				}
		}
		if (wValidPixelCnt>=X_SCAN_MAX_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYUp=GetReallyData(wYValidArry,wValidPixelCnt,NULL,HORI_OFFSET,-1);
		}
		if (wCoreYUp>0)
			break;
		TaskYield();
		//mpDebugPrint(" ");
	}
	//--找纤芯下边沿
	swYEnd =st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	swStartY =st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	#if DISPLAY_IN_ONE_WIN
	if (g_bDisplayMode==0x02)
	{
		if (bMode>1)
		{
			swStartY-=pWin->wHeight/2;
			swYEnd-=pWin->wHeight/2;
		}
		swStartY<<=1;
		swYEnd<<=1;
	}
	#endif
	x=st_swFaceX[bMode]<<1;
	x-=X_STEP*4;

	for (i=0;i<X_SCAN_MAX_SUM;i++)
		wYValidArry[i]=0;
	wValidPixelCnt=0;
	for (;x>0;x-=X_STEP)
	{
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		bLastLevel=*pbWinBuffer;
		//mpDebugPrint(" x=%d: ",x);
		for (;y>swYEnd;y--,pbWinBuffer-=pWin->dwOffset)
		{
				if (*pbWinBuffer<st_bFiberBlackLevel[bSensorIndex]*4)
				{
					if (bContinueCnt)
						bContinueCnt=0;
					continue;
				}
				//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (*pbWinBuffer<bLastLevel)
				{
					bContinueCnt++;
					if (bContinueCnt>=5)
					{
						//bColorIndex++;
						wYValidArry[wValidPixelCnt]=y+bContinueCnt-1;
						//Idu_OsdPaintArea(x>>1, wYValidArry[wValidPixelCnt], 2, 1, OSD_COLOR_RED);
						//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_RED);
						wValidPixelCnt++;
						break;
					}
				}
				else
				{
					bContinueCnt=0;
					//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_GREEN);
					bLastLevel=*pbWinBuffer;
				}
		}
		if (wValidPixelCnt>=X_SCAN_MAX_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYDown=GetReallyData(wYValidArry,wValidPixelCnt,NULL,0,-1);
			
		}
		if (wCoreYDown>0)
			break;
		TaskYield();
		//mpDebugPrint(" ");
	}

	if (wCoreYUp>0 && wCoreYDown>0 &&(st_swFaceY20[bMode]>wCoreYUp+wCoreYDown+1 || wCoreYUp+wCoreYDown>st_swFaceY20[bMode]+1))
	{
		DEBUG_POS(" %d %d ",wCoreYUp,wCoreYDown);
		st_swFaceY20[bMode]=wCoreYUp+wCoreYDown;
		#if 0//DISPLAY_IN_ONE_WIN
		if (g_bDisplayMode==0x02)
		{
			wCoreYUp>>=1;
			if (bMode>1)
				wCoreYUp+=pWin->wHeight/2;
		}
		#endif
		//Idu_OsdPaintArea(x>>1, st_swFaceY20[bMode], 80, 4, OSD_COLOR_RED);
		{
			DEBUG_POS("Y20[%d]=%d ",bMode,st_swFaceY20[bMode]>>1);
			#if TEST_PLANE||ALIGN_DEMO_MODE
			sprintf(&st_bStrCore[bMode][0], "Qian xing0:%d", st_swFaceY20[bMode]>>1);
			xpgSetUpdateOsdFlag(1);
			#endif
			#if OSD_LINE_NUM && !(TEST_PLANE||ALIGN_DEMO_MODE)
			OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16+bMode,x>>1,st_swFaceY20[bMode]>>1,160,2,OSD_COLOR_RED);
		#endif
		}
		return PASS;
	}

	return FAIL;
}

SWORD SearchRightFiberCore(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwValidlevelcnt,dwBlackAverLevel,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_SCAN_MAX_SUM];
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0,swCoreLenth;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel,bSensorIndex;
	SDWORD sdwRet;
	BYTE bColorIndex=OSD_COLOR_WHITE;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!RightCore[%d] error!!! %d,%d: ",bMode,st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	swCoreLenth=pWin->wWidth-st_swFaceX[bMode];
	if (swCoreLenth<(pWin->wWidth>>3))
		return FAIL;
	swCoreLenth=swCoreLenth/2/X_STEP;
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;

	//--获取实际光纤边黑色部分的亮度值
	if (st_bFiberBlackLevel[bSensorIndex]==0xff)
	{
		sdwRet=GetFiberBlackLevel(pWin,bMode);
		if (sdwRet>=0)
			st_bFiberBlackLevel[bSensorIndex]=sdwRet<<1;
		else
			return FAIL;
	}
	
	//--找纤芯上边沿
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	#if DISPLAY_IN_ONE_WIN
	if (g_bDisplayMode==0x02)
	{
		if (bMode>1)
		{
			swStartY-=pWin->wHeight/2;
			swYEnd-=pWin->wHeight/2;
		}
		swStartY<<=1;
		swYEnd<<=1;
	}
	#endif
	x=st_swFaceX[bMode]<<1;
	x+=X_STEP*4;
	for (i=0;i<X_SCAN_MAX_SUM;i++)
		wYValidArry[i]=0;
	wValidPixelCnt=0;
	for (;x<dwOffset;x+=X_STEP)
	{
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		bLastLevel=*pbWinBuffer;
		//mpDebugPrint(" x=%d: ",x);
		for (;y<swYEnd;y++,pbWinBuffer+=pWin->dwOffset)
		{
			if (*pbWinBuffer<st_bFiberBlackLevel[bSensorIndex])
				continue;
			//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (*pbWinBuffer<bLastLevel)
				{
					bContinueCnt++;
					if (bContinueCnt>=3)
					{
						//bColorIndex++;
						wYValidArry[wValidPixelCnt]=y-bContinueCnt+1;
						//Idu_OsdPaintArea(x>>1, wYValidArry[wValidPixelCnt], 2, 1, OSD_COLOR_RED);
						wValidPixelCnt++;
						break;
					}
				}
				else
				{
					bContinueCnt=0;
					//Idu_OsdPaintArea(x>>1, y, 2, 1, bColorIndex);
					bLastLevel=*pbWinBuffer;
				}
		}
		if (wValidPixelCnt>=X_SCAN_MAX_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYUp=GetReallyData(wYValidArry,wValidPixelCnt,NULL,0,-1);
		}
		if (wCoreYUp>0)
			break;
		TaskYield();
		//mpDebugPrint(" ");
	}
	//--找纤芯下边沿
	swYEnd =st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	swStartY =st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	#if DISPLAY_IN_ONE_WIN
	if (g_bDisplayMode==0x02)
	{
	if (bMode>1)
	{
		swStartY-=pWin->wHeight/2;
		swYEnd-=pWin->wHeight/2;
	}
	swStartY<<=1;
	swYEnd<<=1;
	}
	#endif
	x=st_swFaceX[bMode]<<1;
	x+=X_STEP*4;
	for (i=0;i<X_SCAN_MAX_SUM;i++)
		wYValidArry[i]=0;
	wValidPixelCnt=0;
	for (;x<dwOffset;x+=X_STEP)
	{
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		bLastLevel=*pbWinBuffer;
		//mpDebugPrint(" x=%d: ",x);
		for (;y>swYEnd;y--,pbWinBuffer-=pWin->dwOffset)
		{
			if (*pbWinBuffer<st_bFiberBlackLevel[bSensorIndex])
				continue;
			//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (*pbWinBuffer<bLastLevel)
				{
					bContinueCnt++;
					if (bContinueCnt>=3)
					{
						//bColorIndex++;
						wYValidArry[wValidPixelCnt]=y+bContinueCnt-1;
						//Idu_OsdPaintArea(x>>1, wYValidArry[wValidPixelCnt], 2, 1, OSD_COLOR_RED);
						wValidPixelCnt++;
						break;
					}
				}
				else
				{
					bContinueCnt=0;
					//Idu_OsdPaintArea(x>>1, y, 2, 1, bColorIndex);
					bLastLevel=*pbWinBuffer;
				}
		}
		if (wValidPixelCnt>=X_SCAN_MAX_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYDown=GetReallyData(wYValidArry,wValidPixelCnt,NULL,0,-1);
			
		}
		if (wCoreYDown>0)
			break;
		TaskYield();
		//mpDebugPrint(" ");
	}

	if (wCoreYUp>0 &&wCoreYDown>0 &&(st_swFaceY20[bMode]>wCoreYUp+wCoreYDown+1 || wCoreYUp+wCoreYDown>st_swFaceY20[bMode]+1))
	{
		st_swFaceY20[bMode]=(wCoreYUp+wCoreYDown);
		#if 0//DISPLAY_IN_ONE_WIN
		if (g_bDisplayMode==0x02)
		{
			st_swFaceY20[bMode]>>=1;
			if (bMode>1)
				st_swFaceY20[bMode]+=pWin->wHeight/2;
		}
		#endif
			DEBUG_POS("%d Y20[%d]=%d ",bSensorIndex,bMode,st_swFaceY20[bMode]>>1);
			#if TEST_PLANE||ALIGN_DEMO_MODE
			sprintf(&st_bStrCore[bMode][0], "Qian xing1:%d", st_swFaceY20[bMode]>>1);
			xpgSetUpdateOsdFlag(1);
			#endif
		#if OSD_LINE_NUM && !(TEST_PLANE||ALIGN_DEMO_MODE)
		OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16+bMode,(x>>1)-160,st_swFaceY20[bMode]>>1,160,2,OSD_COLOR_RED);
		#endif
		return PASS;
	}

	return FAIL;
}

#if TEST_PLANE||ALIGN_DEMO_MODE
//--水平度
SDWORD SearchTopEdge(ST_IMGWIN *pWin,SWORD swStartX,SWORD swXEnd,SWORD swStartY,SWORD swYEnd)
{
	DWORD dwOffset,dwYValidStartCnt;
	WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,*pwYValidStartArry;
	SWORD i,x,y,swYValidStartAver;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

	if (swXEnd<=swStartX)
		return FAIL;
	pwYValidStartArry = (WORD *)ext_mem_malloc(ALIGN_32(swXEnd-swStartX+1)<<2);
	if (pwYValidStartArry==NULL)
		return FAIL;
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	wValidLineCnt=0;
	for (x=swStartX;x<swXEnd;x+=4)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x*2+y*pWin->dwOffset;
		bContinueCnt=0;
		//mpDebugPrint(" %d: ",x);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT)
				{
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
					}
				}
				else
				{
					bContinueCnt=0;
				}
			}
			if ((wValidPixelCnt>Y_VALID_PIXEL)&&wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				pwYValidStartArry[wValidLineCnt++]=y-wValidPixelCnt-wInvalidPixelCnt+1;
				break;
			}

			y++;
			pbWinBuffer+=pWin->dwOffset;
		}
		TaskYield();
		//mpDebugPrint(" ");

		if (y>=swYEnd)
		{
			break;
		}
	}


	//计算有效Y起始坐标
	if (wValidLineCnt>4)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(pwYValidStartArry,wValidLineCnt,NULL,i,-1);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			ext_mem_free(pwYValidStartArry);
			return (SDWORD)swYValidStartAver;
		}
	}

	ext_mem_free(pwYValidStartArry);
	return FAIL;
}


#endif

SWORD SearchLeftFiberFaceAndTopEdge(ST_IMGWIN *pWin,BYTE bMode,BYTE bScanFace,BYTE bScanTopEdge)
{
	DWORD dwOffset;
	WORD wValidLineCnt,wYValidStartArry[X_SCAN_VALID_NUM];
	SWORD i,x,y,swXEnd,swStartY,swYEnd,swYValidStartAver=-1,swRet;
	BYTE *pbWinBuffer,*pbWinTmpBuf,bSensorIndex;
#if DEBUG_POS_DATA
	WORD wXValidArry[X_SCAN_VALID_NUM];
#endif

	if (bMode==FIBER_LEFT_BOTTOM)
		bSensorIndex=SENSER_BOTTOM;
	else //FIBER_LEFT_TOP
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;
	swXEnd=pWin->dwOffset;//(g_wCenterPos<<1)+(g_wCenterPos>>1);
	//mpDebugPrint(" SearchLeftFiberFaceAndTopEdge bMode=%d: bSensorIndex=%d",bMode,bSensorIndex);

	/* 找端面  */
	// --快速向右找有效端及之后第一个无效端面
#if (SENSOR_WIN_NUM==2)
		swStartY=0;
		swYEnd=pWin->wHeight;
#else
	if (bMode<2)
	{
		swStartY=0;
		swYEnd=pWin->wHeight/2;
	}
	else
	{
		swStartY=pWin->wHeight/2;
		swYEnd=pWin->wHeight;
	}
#endif
	wValidLineCnt=0;
	for (x=X_START;x<swXEnd;x+=X_STEP)
	{
		swRet=ScanFiberUptoDown(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
		if (swRet==FAIL)
		{
			break;
		}
		else if (swRet>0)
		{
			if (wValidLineCnt<X_SCAN_VALID_NUM)
			{
				#if DEBUG_POS_DATA
				wXValidArry[wValidLineCnt]=x;
				#endif
				wYValidStartArry[wValidLineCnt]=swRet-1;
				wValidLineCnt++;
			}
		}
		//TaskYield();
	}
	//计算有效Y起始坐标
	//DEBUG_POS("bScanTopEdge=%d wValidLineCnt %d ",bScanTopEdge,wValidLineCnt);
#if ALIGN_NEW_MODE
	if (bScanTopEdge && wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		i=x;
		wValidLineCnt=0;
		for (x-=X_STEP*4;x>0 ;x-=X_STEP)
		{
			swRet=ScanFiberUptoDown(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
			if (swRet==FAIL)
			{
				break;
			}
			else if (swRet>0)
			{
				if (wValidLineCnt<X_SCAN_VALID_NUM)
				{
					#if DEBUG_POS_DATA
					wXValidArry[wValidLineCnt]=x;
					#endif
					wYValidStartArry[wValidLineCnt]=swRet-1;
					wValidLineCnt++;
				}
				else
					break;
			}
			//TaskYield();
		}
		x=i;
	}
#endif
	swRet=FAIL;
	if (bScanTopEdge && wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(&wYValidStartArry[0],wValidLineCnt,NULL,i,st_swFaceY1[bMode]);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			if (st_swFaceY1[bMode]!=swYValidStartAver)
			{
				st_swFaceY1[bMode]=swYValidStartAver;
				#if DEBUG_POS_DATA
				for (i=0;i<wValidLineCnt;i++)
				{
					pbWinTmpBuf=(BYTE *) pWin->pdwStart+wXValidArry[i]+wYValidStartArry[i]*pWin->dwOffset;
					mpDebugPrintN(" %02x(%d,%d) ",*pbWinTmpBuf,wXValidArry[i]>>1,wYValidStartArry[i]);
				}
				#endif
				DEBUG_POS("Y1[%d]=%d ",bMode,st_swFaceY1[bMode]);
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4+bMode,0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
				#endif
			}
			swRet=PASS;
		}
	}
	if (!bScanFace)
		return swRet;
	//--找到有效端面
	if (wValidLineCnt>4)
	{
		//在最后一个有效端面与第一个无效端面间精确定位
		swXEnd=x-X_STEP;
		for (;x>=swXEnd;x-=4)
		{
			swRet=ScanFiberUptoDown(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
			if (swRet!=FAIL)
			{
				break;
			}
		}
		if (swRet!=FAIL)
		{
			x>>=1;
			DEBUG_POS("X[%d]=%d",bMode,x);
			//if (st_swFaceX[bMode]>=x+FACEX_SHAKE || st_swFaceX[bMode]+FACEX_SHAKE<=x)
			if (st_swFaceX[bMode]!=x)
			{
				st_swFaceX[bMode]=x;
				//DEBUG_POS("X[%d]=%d",bMode,x);
				#if 0///TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceX[bMode]>80 && st_swFaceY1[bMode]>0)
				{
					y=SearchTopEdge(pWin,st_swFaceX[bMode]-80,st_swFaceX[bMode]-20,swStartY,swYEnd);
					//DEBUG_POS("Y[%d]=%d",bMode,y);
					if (y>0)
						sprintf(&st_bStrHLevel[bMode][0], "Shui ping0: %d", y-st_swFaceY1[bMode]);
					else
						st_bStrHLevel[bMode][0]=0;
				}
				else
					st_bStrHLevel[bMode][0]=0;
				xpgSetUpdateOsdFlag(1);
				#endif

				#if OSD_LINE_NUM && !TEST_PLANE
				if (bSensorIndex==SENSER_BOTTOM)
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,bMode,x,(bMode>1 ? pWin->wHeight/2:0),2,pWin->wHeight/2,OSD_COLOR_GREEN);
				#endif
				#if TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceX[bMode]>pWin->wWidth/2)
					sprintf(&st_bStrFace[bMode][0], "%03d<|", st_swFaceX[bMode]-pWin->wWidth/2);
				else
					sprintf(&st_bStrFace[bMode][0], "%03d>|", st_swFaceX[bMode]-pWin->wWidth/2);
				xpgSetUpdateOsdFlag(1);
				#endif
			}
			return PASS;
		}

	}

	return FAIL;
}

//--检测端面角度毛刺
SWORD ScanLeftFaceAngle(ST_IMGWIN *pWin,BYTE bMode, SWORD swYstart,SWORD swYEnd,BYTE bLowLevel)
{
	BYTE *pbWinBuffer,bContinueCnt=0;
	WORD wValidLineCnt=0,*pwValidLenth;
	SWORD i,y,swRet=FAIL,swXstart,swXEnd,swAverLenth;

	if (st_swFaceX[bMode]<10)
		return FAIL;
	//mpDebugPrint("  --%s--:%d->%d  %d ",__FUNCTION__,swYstart,swYEnd,bLowLevel);
	pwValidLenth = (WORD *)ext_mem_malloc(ALIGN_32(swYEnd-swYstart)<<1);
	memset(pwValidLenth,0,ALIGN_32(swYEnd-swYstart)<<1);
	swXstart=ALIGN_CUT_2(st_swFaceX[bMode]-20);
	swXEnd=ALIGN_CUT_2(st_swFaceX[bMode]+20);
	if (swXEnd>=pWin->wWidth)
		swXEnd=pWin->wWidth-1;
	y=swYstart;
	//--统计水平有效长度
	while (y<swYEnd)
	{
		pbWinBuffer = (BYTE *) pWin->pdwStart+swXstart*2+y*pWin->dwOffset;
		//mpDebugPrintN("  P%02x  ",*pbWinBuffer);
		y++;
		if (*pbWinBuffer > bLowLevel)
			continue;
		for (i=swXstart;i<swXEnd;i+=2,pbWinBuffer+=4)
		{
			if (*pbWinBuffer > bLowLevel)
				break;
			pwValidLenth[wValidLineCnt]++;
			if (*(pbWinBuffer+1) > bLowLevel)
				break;
			pwValidLenth[wValidLineCnt]++;
		}
		//--跳过中间芯片的白色部分
		if (y>swYstart+10 && y<swYEnd-10 && pwValidLenth[wValidLineCnt]<15)
		{
			//mpDebugPrintN("  k ");
		}
		else
		{
			//mpDebugPrintN("  %d  ",pwValidLenth[wValidLineCnt]);
			wValidLineCnt++;
		}
	}
	//mpDebugPrint("\r\n " );
	//--找出平均长度
	swAverLenth=GetAverageData(pwValidLenth,wValidLineCnt);
	//mpDebugPrint(" --%s--  swAverLenth=%d wValidLineCnt=%d",__FUNCTION__,swAverLenth,wValidLineCnt);
	if (swAverLenth>0 && wValidLineCnt>4)
	{
		//--去掉上下边角的错误数据
		/*
		for (i=0;i<wValidLineCnt;i++)
		{
			if (pwValidLenth[i]>=swAverLenth)
				break;
			pwValidLenth[i]=0;
		}
		for (i=wValidLineCnt-1;i>=0;i--)
		{
			if (pwValidLenth[i]>=swAverLenth)
				break;
			pwValidLenth[i]=0;
		}
		wReallyLineCnt=0;
		*/
		swRet=0;
	//--找出偏离有效值总数
		for (i=2;i<wValidLineCnt-2;i++)
		{
			if (swAverLenth>pwValidLenth[i])
				swRet+=swAverLenth-pwValidLenth[i];
			else
				swRet+=pwValidLenth[i]-swAverLenth;
		}
		swRet=swRet*100/(wValidLineCnt-4);
	}
	//mpDebugPrint(" --%s--  swAverLenth=%d wValidLineCnt %d",__FUNCTION__,swAverLenth,wValidLineCnt);
	mpDebugPrint(" ANGLE[%d] %d",bMode,swRet);
	ext_mem_free(pwValidLenth);
	return swRet;
}

SWORD ScanRightFaceAngle(ST_IMGWIN *pWin, BYTE bMode,SWORD swYstart,SWORD swYEnd,BYTE bLowLevel)
{
	BYTE *pbWinBuffer,bContinueCnt=0;
	WORD wValidLineCnt=0,*pwValidLenth;
	SWORD i,y,swRet=FAIL,swXstart,swXEnd,swAverLenth;

	if (st_swFaceX[bMode]<10)
		return FAIL;
	//mpDebugPrint("  --%s--:%d->%d ",__FUNCTION__,swYstart,swYEnd);
	pwValidLenth = (WORD *)ext_mem_malloc(ALIGN_32(swYEnd-swYstart)<<1);
	memset(pwValidLenth,0,ALIGN_32(swYEnd-swYstart)<<1);
	swXstart=ALIGN_CUT_2(st_swFaceX[bMode]+20);
	swXEnd=ALIGN_CUT_2(st_swFaceX[bMode]-20);
	if (swXEnd>=pWin->wWidth)
		swXEnd=pWin->wWidth-1;
	y=swYstart;
	//--统计水平有效长度
	while (y<swYEnd)
	{
		pbWinBuffer = (BYTE *) pWin->pdwStart+swXstart*2+y*pWin->dwOffset;
		y++;
		if (*pbWinBuffer > bLowLevel)
			continue;
		for (i=swXstart;i>swXEnd;i-=2,pbWinBuffer-=4)
		{
			if (*pbWinBuffer > bLowLevel)
				break;
			pwValidLenth[wValidLineCnt]++;
			if (*(pbWinBuffer+1) > bLowLevel)
				break;
			pwValidLenth[wValidLineCnt]++;
		}
		//--跳过中间芯片的白色部分
		if (y>swYstart+10 && y<swYEnd-10 && pwValidLenth[wValidLineCnt]<15)
		{
			//mpDebugPrintN("  k ");
		}
		else
		{
			//mpDebugPrintN("  %d  ",pwValidLenth[wValidLineCnt]);
			wValidLineCnt++;
		}
	}
	//mpDebugPrint("\r\n " );
	//--找出平均长度
	swAverLenth=GetAverageData(pwValidLenth,wValidLineCnt);
	//--找出有效长度
	/*
	for (i=1;i<5;i++)
	{
		swAverLenth=GetReallyData(&pwValidLenth[0],wValidLineCnt,NULL,i,-1);
		if (swAverLenth>0)
			break;
	}
	*/
	if (swAverLenth>0 && wValidLineCnt>4)
	{
		//--去掉上下边角的错误数据
		/*
		for (i=0;i<wValidLineCnt;i++)
		{
			if (pwValidLenth[i]>=swAverLenth)
				break;
			pwValidLenth[i]=0;
		}
		for (i=wValidLineCnt-1;i>=0;i--)
		{
			if (pwValidLenth[i]>=swAverLenth)
				break;
			pwValidLenth[i]=0;
		}
		wReallyLineCnt=0;
		*/
		swRet=0;
	//--找出偏离有效值总数
		for (i=2;i<wValidLineCnt-2;i++)
		{
			if (swAverLenth>pwValidLenth[i])
				swRet+=swAverLenth-pwValidLenth[i];
			else
				swRet+=pwValidLenth[i]-swAverLenth;
		}
		swRet=swRet*100/(wValidLineCnt-4);
	}
	//mpDebugPrint("  --%s--  swAverLenth=%d wValidLineCnt %d",__FUNCTION__,swAverLenth,wValidLineCnt);
	//mpDebugPrint(" --%s-- %d",__FUNCTION__,swRet);
	mpDebugPrint(" ANGLE[%d] %d",bMode,swRet);
	ext_mem_free(pwValidLenth);
	return swRet;
}


#if ALIGN_NEW_MODE
SWORD SearchRightFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset;
	WORD wValidLineCnt,wYValidStartArry[X_SCAN_VALID_NUM];
	SWORD i,x,y,swStartY,swYEnd,swYValidStartAver,swRet;
	BYTE *pbWinBuffer,*pbWinTmpBuf,bSensorIndex;
#if DEBUG_POS_DATA
	WORD wXValidArry[X_SCAN_VALID_NUM];
#endif

	if (st_swFaceX[bMode]<=0)
		return FAIL;
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;

	/* 找端面  */
	// --快速向左找有效端及之后第一个无效端面
#if (SENSOR_WIN_NUM==2)
		swStartY=pWin->wHeight-1;
		swYEnd=4;
#else
	if (bMode<2)
	{
		swStartY=pWin->wHeight/2-1;
		swYEnd=0;
	}
	else
	{
		swStartY=pWin->wHeight-1;
		swYEnd=pWin->wHeight/2;
	}
#endif
	wValidLineCnt=0;
	for (x=st_swFaceX[bMode]*2+X_STEP*4;x<dwOffset;x+=X_STEP)
	{
		swRet=ScanFiberDowntoUp(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
		if (swRet==FAIL)
		{
			break;
		}
		else if (swRet>0)
		{
			if (wValidLineCnt<X_SCAN_VALID_NUM)
			{
				wYValidStartArry[wValidLineCnt]=swRet-1;
				//mpDebugPrintN(" %d ",wYValidStartArry[wValidLineCnt]);
				wValidLineCnt++;
			}
			else
				break;
		}
		//TaskYield();
	}
	//mpDebugPrint("");

	//计算有效Y起始坐标
	if (wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(&wYValidStartArry[0],wValidLineCnt,NULL,i,st_swFaceY3[bMode]);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			#if DISPLAY_IN_ONE_WIN
			if (g_bDisplayMode==0x02)
			{
				swYValidStartAver>>=1;
				if (bMode>1)
					swYValidStartAver+=pWin->wHeight/2;
			}
			#endif
			if (st_swFaceY3[bMode]!=swYValidStartAver)
			{
				st_swFaceY3[bMode]=swYValidStartAver;
				#if DEBUG_POS_DATA
				for (i=0;i<wValidLineCnt;i++)
				{
					pbWinTmpBuf=(BYTE *) pWin->pdwStart+wXValidArry[i]+wYValidStartArry[i]*pWin->dwOffset;
					mpDebugPrintN(" %02x(%d,%d) ",*pbWinTmpBuf,wXValidArry[i]>>1,wYValidStartArry[i]);
				}
				#endif
				DEBUG_POS(" --Y3[%d]=%d ",bMode,st_swFaceY3[bMode]);
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
				#endif
				#if TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>st_swFaceY1[bMode])
				{
					swYValidStartAver=(st_swFaceY3[bMode]+st_swFaceY1[bMode])>>1;
					if (swYValidStartAver>pWin->wHeight/2)
						sprintf(&st_bStrCenter[bMode][0], "^:%d",(swYValidStartAver-pWin->wHeight/2));
					else
						sprintf(&st_bStrCenter[bMode][0], "v:%d",pWin->wHeight/2-swYValidStartAver);
					xpgSetUpdateOsdFlag(1);
				}
				#endif
			}
			return PASS;
		}
	}
					
	return FAIL;
}

#else
SWORD SearchRightFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset;
	WORD wValidLineCnt,wYValidStartArry[X_SCAN_VALID_NUM];
	SWORD i,x,y,swXstart,swXEnd,swStartY,swYEnd,swYValidStartAver,swRet;
	BYTE *pbWinBuffer,*pbWinTmpBuf,bSensorIndex;

	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;

	/* 找端面  */
	// --快速向左找有效端及之后第一个无效端面
#if (SENSOR_WIN_NUM==2)
		swStartY=pWin->wHeight-1;
		swYEnd=4;
#else
	if (bMode<2)
	{
		swStartY=pWin->wHeight/2-1;
		swYEnd=0;
	}
	else
	{
		swStartY=pWin->wHeight-1;
		swYEnd=pWin->wHeight/2;
	}
#endif
	wValidLineCnt=0;

	swXstart=dwOffset-X_START;
	if (st_swFaceX[bMode]>0)
		swXEnd=st_swFaceX[bMode]<<1;
	else
		swXEnd=0;
	for (x=swXstart;x>swXEnd;x-=X_STEP)
	{
		swRet=ScanFiberDowntoUp(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
		if (swRet==FAIL)
		{
			break;
		}
		else if (swRet>0)
		{
			if (wValidLineCnt<X_SCAN_VALID_NUM)
			{
				wYValidStartArry[wValidLineCnt]=swRet-1;
				//mpDebugPrintN(" %d ",wYValidStartArry[wValidLineCnt]);
				wValidLineCnt++;
			}
			else
				break;
		}
		//TaskYield();
	}
	//mpDebugPrint("");

	//计算有效Y起始坐标
	if (wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(&wYValidStartArry[0],wValidLineCnt,NULL,i,st_swFaceY3[bMode]);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			#if DISPLAY_IN_ONE_WIN
			if (g_bDisplayMode==0x02)
			{
				swYValidStartAver>>=1;
				if (bMode>1)
					swYValidStartAver+=pWin->wHeight/2;
			}
			#endif
			if (st_swFaceY3[bMode]!=swYValidStartAver)
			{
				st_swFaceY3[bMode]=swYValidStartAver;
				#if DEBUG_POS_DATA
				for (i=0;i<wValidLineCnt;i++)
				{
					pbWinTmpBuf=(BYTE *) pWin->pdwStart+wXValidArry[i]+wYValidStartArry[i]*pWin->dwOffset;
					mpDebugPrintN(" %02x(%d,%d) ",*pbWinTmpBuf,wXValidArry[i]>>1,wYValidStartArry[i]);
				}
				#endif
				DEBUG_POS(" --Y3[%d]=%d ",bMode,st_swFaceY3[bMode]);
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
				#endif
				#if TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>st_swFaceY1[bMode])
				{
					swYValidStartAver=(st_swFaceY3[bMode]+st_swFaceY1[bMode])>>1;
					if (swYValidStartAver>pWin->wHeight/2)
						sprintf(&st_bStrCenter[bMode][0], "^:%d",(swYValidStartAver-pWin->wHeight/2));
					else
						sprintf(&st_bStrCenter[bMode][0], "v:%d",pWin->wHeight/2-swYValidStartAver);
					xpgSetUpdateOsdFlag(1);
				}
				#endif
			}
			return PASS;
		}
	}
					
	return FAIL;
}
#endif

SWORD SearchRightFiberCenter(ST_IMGWIN *pWin,BYTE bMode)
{
	SDWORD sdwRet;
	DWORD dwOffset;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_SCAN_MAX_SUM],wYStartIndex;
	SWORD x,y,i,swStartY,swYEnd,swYValidStartAver;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex,bBlackBG;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!RightCenter error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/6;
	swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/6;
#if (SENSOR_WIN_NUM==2)
		#if DISPLAY_IN_ONE_WIN
		if (g_bDisplayMode==0x02)
		{
			if (bMode>1)
			{
				swStartY-=pWin->wHeight/2;
			}
			swStartY<<=1;
			swYEnd=pWin->wHeight;
		}
		#endif
#else
	if (bMode<2)
	{
		swYEnd=pWin->wHeight/2;
	}
	else
	{
		swYEnd=pWin->wHeight;
	}
#endif
	//--获取实际光纤边黑色部分的亮度值
	if (st_bFiberBlackLevel[bSensorIndex]==0xff)
	{
		sdwRet=GetFiberBlackLevel(pWin,bMode);
		if (sdwRet>=0)
			st_bFiberBlackLevel[bSensorIndex]=sdwRet<<1;
		else
			return FAIL;
	}
	bBlackBG=st_bFiberBlackLevel[bSensorIndex]; // g_psSetupMenu->bBackGroundLevel[bSensorIndex]
	bBlackBG++;

	for (i=0;i<X_SCAN_MAX_SUM;i++)
		wYValidArry[i]=0;
	wYStartIndex=0;
	x=(st_swFaceX[bMode]<<1)+X_STEP*4;
	for (;x<dwOffset;x+=X_STEP)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		bContinueCnt=0;
		wEndInvalidCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		//mpDebugPrint(" %d: ",x);
		//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_RED);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer > bBlackBG )
				{
					wValidPixelCnt++;
					wEndInvalidCnt=0;
				}
				else
				{
					wInvalidPixelCnt++;
					wEndInvalidCnt++;
				}
			}
			else
			{
				if (*pbWinBuffer > bBlackBG)
				{
					bContinueCnt++;
					if (bContinueCnt>Y_CENTER_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
					}
				}
				else if (bContinueCnt)
				{
					bContinueCnt=0;
				}
			}

			if (wValidPixelCnt>CENTER_MIN_PIXEL && wEndInvalidCnt>Y_VALID_OFFSET && wValidPixelCnt>((wInvalidPixelCnt-wEndInvalidCnt)<<3))
			{
					//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_RED);
				if (wYStartIndex<X_SCAN_MAX_SUM)
				{
					wYValidArry[wYStartIndex++]=y-wEndInvalidCnt-(wValidPixelCnt+wInvalidPixelCnt-wEndInvalidCnt)/2;
					//mpDebugPrintN("  -%d:%d  %d %d %d %d  ",wYStartIndex,wYValidArry[wYStartIndex-1],y,wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt);
					//mpDebugPrintN(" -%d: %d  ",wYStartIndex,wYValidArry[wYStartIndex-1]);
					//计算有效Y起始坐标
					if (wYStartIndex>X_SCAN_VALID_NUM)
					{
						for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
						{
							swYValidStartAver=GetReallyData(wYValidArry,wYStartIndex,NULL,i,st_swFaceY2[bMode]);
							if (swYValidStartAver>0)
								break;
						}
						if (swYValidStartAver>0)
						{
							#if DISPLAY_IN_ONE_WIN
							if (g_bDisplayMode==0x02)
							{
									swYValidStartAver>>=1;
								if (bMode>1)
									swYValidStartAver+=pWin->wHeight/2;
							}
							#endif
							{
								if (st_swFaceY2[bMode]!=swYValidStartAver)
								{
									//st_swFaceNewY2[bMode].wCnt=1;
									st_swFaceY2[bMode]=swYValidStartAver;
									DEBUG_POS(" Y2[%d]=%d ",bMode,st_swFaceY2[bMode]);
									#if 0//TEST_PLANE||ALIGN_DEMO_MODE
									sprintf(&st_bStrCore[bMode][0], "Center1:%d", st_swFaceY2[bMode]);
									xpgSetUpdateOsdFlag(1);
									#endif
									#if OSD_LINE_NUM && !(TEST_PLANE||ALIGN_DEMO_MODE)
									OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,8+bMode,pWin->wWidth/2,st_swFaceY2[bMode],pWin->wWidth,2,OSD_COLOR_RED);
									#endif
									//return ENABLE;
								}
							}
							wYStartIndex=X_SCAN_MAX_SUM;
							return PASS;
						}
					}
				}
				break;
			}

			y++;
			pbWinBuffer+=pWin->dwOffset;
		}
		TaskYield();
		//mpDebugPrint(" ");
	}
	//mpDebugPrint(" ");
	
	return FAIL;
}

SWORD SearchRightFiberFaceAndTopEdge(ST_IMGWIN *pWin,BYTE bMode,BYTE bScanFace,BYTE bScanTopEdge)
{
	DWORD dwOffset;
	WORD wValidLineCnt,wYValidStartArry[X_SCAN_VALID_NUM];
	SWORD i,x,y,swXEnd,swXstart,swStartY,swYEnd,swYValidStartAver,swRet;
	BYTE *pbWinBuffer,*pbWinTmpBuf,bSensorIndex;
#if DEBUG_POS_DATA
	WORD wXValidArry[X_SCAN_VALID_NUM];
#endif

	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;

	/* 找端面  */
	// --快速向左找有效端及之后第一个无效端面
#if (SENSOR_WIN_NUM==2)
		swStartY=0;
		swYEnd=pWin->wHeight;
#else
	if (bMode<2)
	{
		swStartY=0;
		swYEnd=pWin->wHeight>>1;
	}
	else
	{
		swStartY=pWin->wHeight>>1;
		swYEnd=pWin->wHeight;
	}
#endif

	wValidLineCnt=0;
	swXstart=dwOffset-X_START;
	//mpDebugPrint("swXstart: %d  ",swXstart);
	for (x=swXstart;x>0;x-=X_STEP)
	{
		swRet=ScanFiberUptoDown(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
		if (swRet==FAIL)
		{
			break;
		}
		else if (swRet>0)
		{
			if (wValidLineCnt<X_SCAN_VALID_NUM)
			{
				#if DEBUG_POS_DATA
				wXValidArry[wValidLineCnt]=x;
				#endif
				wYValidStartArry[wValidLineCnt]=swRet-1;
				wValidLineCnt++;
			}
		}
	}

	//计算有效Y起始坐标
#if ALIGN_NEW_MODE
	if (bScanTopEdge && wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		i=x;
		wValidLineCnt=0;
		for (x+=X_STEP*4;x<dwOffset ;x+=X_STEP)
		{
			swRet=ScanFiberUptoDown(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
			if (swRet==FAIL)
			{
				break;
			}
			else if (swRet>0)
			{
				if (wValidLineCnt<X_SCAN_VALID_NUM)
				{
					#if DEBUG_POS_DATA
					wXValidArry[wValidLineCnt]=x;
					#endif
					wYValidStartArry[wValidLineCnt]=swRet-1;
					wValidLineCnt++;
				}
				else
					break;
			}
			//TaskYield();
		}
		x=i;
	}
#endif

	swRet=FAIL;
	if (bScanTopEdge && wValidLineCnt>=X_SCAN_VALID_NUM)
	{
		for (i=HORI_OFFSET;i<(HORI_OFFSET<<3);i++)
		{
			swYValidStartAver=GetReallyData(&wYValidStartArry[0],wValidLineCnt,NULL,i,st_swFaceY1[bMode]);
			if (swYValidStartAver>0)
				break;
		}
		if (swYValidStartAver>0)
		{
			#if DISPLAY_IN_ONE_WIN
			if (g_bDisplayMode==0x02)
			{
				swYValidStartAver>>=1;
				if (bMode>1)
					swYValidStartAver+=pWin->wHeight/2;
			}
			#endif
			if (st_swFaceY1[bMode]!=swYValidStartAver)
			{
				st_swFaceY1[bMode]=swYValidStartAver;
				#if DEBUG_POS_DATA
				for (i=0;i<wValidLineCnt;i++)
				{
					pbWinTmpBuf=(BYTE *) pWin->pdwStart+wXValidArry[i]+wYValidStartArry[i]*pWin->dwOffset;
					mpDebugPrintN(" %02x(%d,%d) ",*pbWinTmpBuf,wXValidArry[i]>>1,wYValidStartArry[i]);
				}
				#endif
				DEBUG_POS("-----Y1[%d]=%d ",bMode,st_swFaceY1[bMode]);
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
				#endif
			}
			swRet=PASS;
		}
	}

	if (!bScanFace)
		return swRet;
	//无有效端面或者所有端都有效
	if (!wValidLineCnt || x<=0)
	{
		//UartOutText("-N-");
	}
	else //find
	{
		//在最后一个有效端面与第一个无效端面间精确定位
		swXEnd=x+X_STEP;
		//x-=X_STEP;
		//wValidLineCnt=0;
		for (;x<=swXEnd;x+=4)
		{
			swRet=ScanFiberUptoDown(pWin,x,swStartY,swYEnd,g_psSetupMenu->bBackGroundLevel[bSensorIndex]>>BG_BRIGHT_SHIFT);
			if (swRet!=FAIL)
			{
				break;
			}
		}
		if (swRet!=FAIL)
		{
			x>>=1;
			//if (st_swFaceX[bMode]>=x+FACEX_SHAKE || st_swFaceX[bMode]+FACEX_SHAKE<=x)
			if (st_swFaceX[bMode]!=x)
			{
				st_swFaceX[bMode]=x;
				DEBUG_POS("X[%d]=%d",bMode,x);
				#if 0//TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceX[bMode]>80 && st_swFaceY1[bMode]>0)
				{
					y=SearchTopEdge(pWin,st_swFaceX[bMode]+20,st_swFaceX[bMode]+80,swStartY,swYEnd);
					//DEBUG_POS("Y[%d]=%d",bMode,y);
					if (y>0)
						sprintf(&st_bStrHLevel[bMode][0], "Shui ping1: %d", y-st_swFaceY1[bMode]);
					else
						st_bStrHLevel[bMode][0]=0;
				}
				else
					st_bStrHLevel[bMode][0]=0;
				xpgSetUpdateOsdFlag(1);
				#endif
				#if OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,bMode,x,(bMode>1 ? pWin->wHeight/2:0),2,pWin->wHeight/2,OSD_COLOR_GREEN);
				#endif
				#if TEST_PLANE||ALIGN_DEMO_MODE
				if (st_swFaceX[bMode]>pWin->wWidth/2)
					sprintf(&st_bStrFace[bMode][0], "|<%03d", pWin->wWidth/2-st_swFaceX[bMode]);
				else
					sprintf(&st_bStrFace[bMode][0], "|>%03d",  pWin->wWidth/2-st_swFaceX[bMode]);
				xpgSetUpdateOsdFlag(1);
				#endif
			}
			return PASS;
		}
		else
		{
			UartOutText("-n-");
		}


	}
	return FAIL;
}

void ResetMotorPara(void)
{
	BYTE i;
	for (i=0;i<FIBER_WINDOW_NUM;i++)
	{
		st_bDirectionArry[i]=1;
		st_dwMotoStartTime[i]=0;
	}
	st_dwHMotoPos[0]=0;
	st_dwHMotoPos[1]=0;
}

/**************************************************************************/
//moto 1 : max 67000puls
//--speed 10,step 10000,FF or FR  need 788ms, 12.7 puls/ms  65000->5084 :12.78puls/ms
//--speed 5,step 500,     FF or FR  need 116ms, 4.31 puls/ms  65000->13532 :4.8puls/ms
//--speed 3,step 500,     FF or FR  need 332ms, 1.5 puls/ms  65000->40628 :1.6puls/ms
//--speed 2,step 500,     FF or FR  need 644ms, 0.776 puls/ms  65000->81260 :0.8puls/ms
//--speed 1,step 500,     FF or FR  need 1268ms, 0.4 puls/ms  65000->162524 :0.4puls/ms

//--reverse need 200puls(hold on/off)
/**************************************************************************/
#define		MOTO_BASE_RETRY_TIMES					10
#define		MOTO_BASE_STEPS									8 // pixel
void DriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wPulse,BYTE bSpeed)
{
	BYTE bTxData[10],bChecksum,i,bDataIndex=bMotorInex-1;
	//WORD wPulse=wStep;

	//mpDebugPrint("--bMotorInex=%d  bDirection=%d/%d st_bMotorStaus %p",bMotorInex,bDirection,st_bDirectionArry[bDataIndex],st_bMotorStaus);
	if (bMotorInex<5)
	{

		if ((st_bMotorStaus&(1<<bDataIndex)) && st_bDirectionArry[bDataIndex]==bDirection && SystemGetElapsedTime(st_dwMotoStartTime[bDataIndex])<300)
		//if ((st_bMotorStaus&(1<<bDataIndex)) && st_bDirectionArry[bDataIndex]==bDirection)
		//if ((st_bMotorStaus&(1<<bDataIndex)) && st_bDirectionArry[bDataIndex]==bDirection )//&& wPulse>50
			return;
		/*
		if (bDataIndex<2 && bDirection)
		{
			if (st_dwHMotoPos[bDataIndex]>=MOTOR_HORI_MAX_PULSE)
			{
				mpDebugPrint("--bMotorInex=%d  overload %d",bMotorInex,st_dwHMotoPos[bDataIndex]);
				return ;
			}
			if (st_dwHMotoPos[bDataIndex]+wStep >= MOTOR_HORI_MAX_PULSE)
				wStep= MOTOR_HORI_MAX_PULSE-st_dwHMotoPos[bDataIndex];
				
		}
		*/
#if 0
		if (st_bDirectionArry[bDataIndex]==bDirection && wStep< MOTO_BASE_STEPS)
		{
			if (st_bBaseRetryArry[bDataIndex] < MOTO_BASE_RETRY_TIMES )
			{
				st_bMotorBaseStep[bDataIndex]++;
				st_bBaseRetryArry[bDataIndex]=0;
			}
			else
			{
				st_bBaseRetryArry[bDataIndex]++;
			}
		}
		else
		{
			st_bBaseRetryArry[bDataIndex]=0;
		}
#elif 0
	if (bMotorInex<3) // 水平马达
	{
		wPulse=wStep;
	}
	else if (bMotorInex<5) // 垂直马达
	{
		wPulse=wStep*4;//*250;
	}
#endif
	}
	MP_DEBUG("bMotorInex=%d  bDirection=%d wPulse %d",bMotorInex,bDirection,wPulse);
	bTxData[0]=0xa2;
	bTxData[1]=3+5;
	bTxData[2]=bMotorInex; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	bTxData[3]=bDirection; // 0（后退）1（前进
	bTxData[4]=wPulse>>8;
	bTxData[5]=wPulse&0x00ff;
	bTxData[6]=bSpeed; // min 1  max=10
	if (TSPI_PacketSend(bTxData,1)==PASS)
	{
		st_bMotorStaus|=(1<<bDataIndex);
		
		st_dwMotoStartTime[bDataIndex]=GetSysTime();
		if (st_bDirectionArry[bDataIndex]!=bDirection)
		{
			if (wPulse>MOTOR_HORI_REVERSE_PULSE)
				wPulse-=MOTOR_HORI_REVERSE_PULSE;
			else
				wPulse=0;
			st_bDirectionArry[bDataIndex]=bDirection;
		}
		if (bDirection)
			st_dwHMotoPos[bDataIndex]+=wPulse;
		else
		{
			if (st_dwHMotoPos[bDataIndex]>wPulse)
				st_dwHMotoPos[bDataIndex]-=wPulse;
			else
				st_dwHMotoPos[bDataIndex]=0;
		}
	}

}

void MotorSetStatus(BYTE bMotorInex,BYTE bMode) //0xa3   bmode:0->stop  BYT4~7   0 停止 1 允许待机  2不允许待机
{
	BYTE bTxData[10],bDataIndex=bMotorInex-1;;

	if (bMotorInex>7)
		return;
	if (bMode==MOTOR_HOLD||bMode==MOTOR_NO_HOLD)
	{
		MotoHoldTimeoutSet(bMotorInex,bMode);
	}
	else if (bMode==MOTOR_STOP) //stop
	{
		if (!(st_bMotorStaus&(1<<bDataIndex)))
			return;
		mpDebugPrint("bMotorInex=%d  bMode=%d ",bMotorInex,bMode);
		bTxData[0]=0xa3;
		bTxData[1]=3+1;
		bTxData[2]=(bMode<<4)|bMotorInex; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
		if (TSPI_PacketSend(bTxData,1)==PASS)
			;//st_bMotorStaus&= ~(1<<bDataIndex); //由MCU反馈状态比较合适
	}

}

void TimeoutToStopHoldMoto1(void)
{
	BYTE bMotorInex=0;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void TimeoutToStopHoldMoto2(void)
{
	BYTE bMotorInex=1;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void TimeoutToStopHoldMoto3(void)
{
	BYTE bMotorInex=2;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void TimeoutToStopHoldMoto4(void)
{
	BYTE bMotorInex=3;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void MotoHoldTimeoutSet(BYTE bMotorInex,BYTE bMode)
{
	BYTE bTxData[10],bDataIndex=bMotorInex-1;

	if (bMode==MOTOR_HOLD && (!(st_bMotorHold&(1<<bDataIndex))))
	{
		mpDebugPrint("bMotorInex=%d  bMode=%d ",bMotorInex,bMode);
		bTxData[0]=0xa3;
		bTxData[1]=3+1;
		bTxData[2]=(bMode<<4)|bMotorInex; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
		if (TSPI_PacketSend(bTxData,0)!=PASS)
			return;
		switch (bMotorInex)
		{
			case 1:
					Ui_TimerProcAdd(MOTOR_HOLD_TIMEOUT, TimeoutToStopHoldMoto1);
				break;
			case 2:
					Ui_TimerProcAdd(MOTOR_HOLD_TIMEOUT, TimeoutToStopHoldMoto2);
				break;
			case 3:
					Ui_TimerProcAdd(MOTOR_HOLD_TIMEOUT, TimeoutToStopHoldMoto3);
				break;
			case 4:
					Ui_TimerProcAdd(MOTOR_HOLD_TIMEOUT, TimeoutToStopHoldMoto4);
				break;
		}
		st_bMotorHold |=(1<<bDataIndex);
	}
	else if (bMode==MOTOR_NO_HOLD && (st_bMotorHold&(1<<bDataIndex)))
	{
		mpDebugPrint("bMotorInex=%d  bMode=%d ",bMotorInex,bMode);
		bTxData[0]=0xa3;
		bTxData[1]=3+1;
		bTxData[2]=(bMode<<4)|bMotorInex; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
		if (TSPI_PacketSend(bTxData,0)!=PASS)
			return;
		switch (bMotorInex)
		{
			case 1:
				Ui_TimerProcRemove(TimeoutToStopHoldMoto1);
				break;
			case 2:
				Ui_TimerProcRemove(TimeoutToStopHoldMoto2);
				break;
			case 3:
				Ui_TimerProcRemove(TimeoutToStopHoldMoto3);
				break;
			case 4:
				Ui_TimerProcRemove(TimeoutToStopHoldMoto4);
				break;
		}
		st_bMotorHold &=~(1<<bDataIndex);
	}
}

void TimerToReleaseAllHold(void)
{
			MotorSetStatus(1,MOTOR_NO_HOLD);
			MotorSetStatus(2,MOTOR_NO_HOLD);
			MotorSetStatus(3,MOTOR_NO_HOLD);
			MotorSetStatus(4,MOTOR_NO_HOLD);

}
#if 0
BYTE GetMotoIdxByFiberIdx(BYTE bFiberIndex) //0xa3   bmode:0->stop  BYT4~7   0 停止 1 允许待机  2不允许待机
{
	BYTE bMotorInex;
	
	if (bFiberIndex==FIBER_RIGHT_TOP)
		bMotorInex=04; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bFiberIndex==FIBER_RIGHT_BOTTOM)
		bMotorInex=03;
	else if (bFiberIndex==FIBER_LEFT_BOTTOM)
		bMotorInex=02; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bFiberIndex==FIBER_LEFT_TOP)
		bMotorInex=01;

	return bMotorInex;

}
#endif
void MoveAFMotor(BYTE bMotorInex,SWORD swStep)
{
	SWORD swSx,swDiffX;
	BYTE bDirection, bSpeed;
	WORD wStep;

	mpDebugPrint("MoveAFMotor  %d->%d",bMotorInex,swStep);
/*
	if (bMotorInex!=5 && bMotorInex!=6)
	{
		mpDebugPrint("MoveAFMotor  index error%d",bMotorInex);
		return;
	}
	*/
	swDiffX=swStep;
	if (swDiffX>0)
		bDirection=1; // 0（后退）1（前进
	else
	{
		bDirection=0;
		swDiffX=-swDiffX;
	}

	wStep=swDiffX;
	bSpeed=10; //  1-10
	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
}

SWORD MoveHMotorToSpecPosition(BYTE bSensorIndex,SWORD swSx,SWORD swTx)
{
	SWORD swDiffX;
	BYTE bMotorInex,bDirection,bSpeed;
	WORD wStep;

	mpDebugPrint("H %d swSx=%d  swTx=%d",bSensorIndex,swSx,swTx);

#if FIBER_DISPLAY_IN_SAME_SIDE
	if (bSensorIndex==FIBER_LEFT_TOP)
		bMotorInex=MOTOR_H_LEFT; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bSensorIndex==FIBER_RIGHT_TOP)
		bMotorInex=MOTOR_H_RIGHT;
	else
		return FAIL;
#else
	if (bSensorIndex==FIBER_LEFT_BOTTOM)
		bMotorInex=02; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bSensorIndex==FIBER_LEFT_TOP)
		bMotorInex=01;
	else
		return FAIL;
#endif
	swDiffX=swTx-swSx;
	if (ABS(swDiffX)<=FACEX_SHAKE)
	{
		//MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
		return PASS;
	}
	if (swDiffX>0)
		bDirection=1; // 0（后退）1（前进
	else
	{
		bDirection=0;
		swDiffX=-swDiffX;
	}
#if 1
	if (swDiffX<10)
		wStep=swDiffX<<3;
	else if (swDiffX<50)
		wStep=swDiffX<<3;
	else
		wStep=swDiffX<<4;
#else
	if (swDiffX<10)
		wStep=swDiffX<<2;
	else if (swDiffX<60)
		wStep=swDiffX<<3;
	else
		wStep=swDiffX*10;
#endif
	//if (st_bDirectionArry[bMotorInex]!=bDirection) //1300
	//	wStep+=100;
	if (swDiffX<10)
	{
		MotorSetStatus(bMotorInex,MOTOR_HOLD);
	}
	if (swDiffX>300)
		bSpeed=10;
	else if (swDiffX>60)
		bSpeed=5;
	else
		bSpeed=2;
	if (st_dwProcState==SENSOR_FACE_POS2A || st_dwProcState==SENSOR_FACE_POS2B)
	{
		if (bDirection==0 && wStep<500)
		{
			//bDirection=1;
			wStep=500;
		}
			
	}
	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
	return FAIL;
}

SWORD MoveVMotorToSpecPosition(BYTE bFiberIndex,SWORD swSx,SWORD swTx)
{
	SWORD swDiffX;
	BYTE bMotorInex,bValueIndex,bDirection,bSpeed,bDirectonUp;
	WORD wStep;

	if (swTx==swSx)
		return PASS;
	swDiffX=swTx-swSx;
	if (bFiberIndex==FIBER_RIGHT_TOP)
	{
		bMotorInex=MOTOR_V_UP; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
#if 1
		bValueIndex=0;
		if (st_bVmotorMoveTimes[0]<0xff)
		{
			if (st_swLastProcStep[bValueIndex]*swDiffX<=0)
			{
				st_swLastProcStep[bValueIndex]=swDiffX;
				st_bVmotorMoveTimes[bValueIndex]=0;
			}
			else
			{
				st_bVmotorMoveTimes[bValueIndex]++;
				st_swVmotorMoveValue[bValueIndex][2]=st_swVmotorMoveValue[bValueIndex][1];
				st_swVmotorMoveValue[bValueIndex][1]=st_swVmotorMoveValue[bValueIndex][0];
				st_swVmotorMoveValue[bValueIndex][0]=ABS(swDiffX);
				if (st_bVmotorMoveTimes[bValueIndex]>=VMOTOR_CNT)
				{
					if (st_swVmotorMoveValue[bValueIndex][0]>st_swVmotorMoveValue[bValueIndex][1] && st_swVmotorMoveValue[bValueIndex][1]>st_swVmotorMoveValue[bValueIndex][2])
					{
						st_bTopVMotorUpValue=1-st_bTopVMotorUpValue;
						st_bVmotorMoveTimes[bValueIndex]=0xff;
					}
					else if (st_swVmotorMoveValue[bValueIndex][0]<st_swVmotorMoveValue[bValueIndex][1] && st_swVmotorMoveValue[bValueIndex][1]<st_swVmotorMoveValue[bValueIndex][2])
					{
						st_bVmotorMoveTimes[bValueIndex]=0xff;
					}
				}
			}
		}
#endif
		bDirectonUp=st_bTopVMotorUpValue;
	}
	else if (bFiberIndex==FIBER_LEFT_BOTTOM||bFiberIndex==FIBER_RIGHT_BOTTOM)
	{
		bMotorInex=MOTOR_V_DOWN;
#if 1
		bValueIndex=1;
		if (st_bVmotorMoveTimes[1]<0xff)//4  待确认马达移动方向
		{
			if (st_swLastProcStep[bValueIndex]*swDiffX<=0)
			{
				st_swLastProcStep[bValueIndex]=swDiffX;
				st_bVmotorMoveTimes[bValueIndex]=0;
			}
			else
			{
				st_bVmotorMoveTimes[bValueIndex]++;
				st_swVmotorMoveValue[bValueIndex][2]=st_swVmotorMoveValue[bValueIndex][1];
				st_swVmotorMoveValue[bValueIndex][1]=st_swVmotorMoveValue[bValueIndex][0];
				st_swVmotorMoveValue[bValueIndex][0]=ABS(swDiffX);
				if (st_bVmotorMoveTimes[bValueIndex]>=VMOTOR_CNT)
				{
					if (st_swVmotorMoveValue[bValueIndex][0]>st_swVmotorMoveValue[bValueIndex][1] && st_swVmotorMoveValue[bValueIndex][1]>st_swVmotorMoveValue[bValueIndex][2])
					{
						//方向反了
						st_bBottomVMotorUpValue=1-st_bBottomVMotorUpValue;
						st_bVmotorMoveTimes[bValueIndex]=0xff;
					}
					else if (st_swVmotorMoveValue[bValueIndex][0]<st_swVmotorMoveValue[bValueIndex][1] && st_swVmotorMoveValue[bValueIndex][1]<st_swVmotorMoveValue[bValueIndex][2])
					{
						//方向正确
						st_bVmotorMoveTimes[bValueIndex]=0xff;//4   方向确认OK
					}
				}
			}
		}
#endif
		bDirectonUp=st_bBottomVMotorUpValue;
	}
	else
		return FAIL;
	
	mpDebugPrint("V swSx=%d  swTx=%d",swSx,swTx);
	if (swDiffX>0)
		bDirection=1-bDirectonUp; //   1->down 0->up
	else
	{
		bDirection=bDirectonUp;
		swDiffX=-swDiffX;
	}
	wStep=swDiffX*MOTOR_V_MIN_SPEED; // 123->50->45
	bSpeed=8;
	/*
	 if (swDiffX>20)
		bSpeed=5;
	else
		bSpeed=3;
	*/
	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
	return FAIL;
}

void Discharge(WORD wMode,BYTE bStep)
{
	BYTE i,bTxData[16];

	mpDebugPrint("Discharge %d bStep=%d",wMode,bStep);
	//st_wDischargeTimeSet=wMode;
	st_wDiachargeRunTime=wMode;
	if (bStep)
	{
		bTxData[0]=0xae;
		bTxData[1]=12;
		bTxData[2]=0xa2;
		bTxData[3]=2+4;
		bTxData[4]=1;
		bTxData[5]=1;
		bTxData[6]=0;
		bTxData[7]=bStep;
		bTxData[2]=0xa2;
		bTxData[3]=2+4;
		bTxData[4]=2;
		bTxData[5]=1;
		bTxData[6]=0;
		bTxData[7]=bStep;
		bTxData[8]=0xa5;
		bTxData[9]=2+1;
		bTxData[10]=wMode;
	}
	else
	{
		bTxData[0]=0xa5;
		bTxData[1]=3+4;
		if (wMode<4)//0x8000
		{
			switch (wMode)
			{
				case 1:
					bTxData[2]=1750>>8;
					bTxData[3]=1750&0xff;
					bTxData[4]=120>>8;
					bTxData[5]=120&0xff; // 130
					break;

				case 2:
					bTxData[2]=0;
					bTxData[3]=130;
					bTxData[4]=1000>>8;
					bTxData[5]=1000&0xff;
					break;

				case 3://1.750v  2300ms
					bTxData[2]=2200>>8;
					bTxData[3]=2200&0xff;
					#if TEST_PLANE||ALIGN_DEMO_MODE
					bTxData[4]=st_wMotoStep[MAX_ADJ_NUM-1]>>8;
					bTxData[5]=st_wMotoStep[MAX_ADJ_NUM-1]&0xff;
					#else
					bTxData[4]=2300>>8;
					bTxData[5]=2300&0xff;
					#endif
					break;

				case 0:
				default:
					bTxData[2]=0;
					bTxData[3]=0;
					bTxData[4]=0;
					bTxData[5]=0;
					break;
			}
		}
		else
		{
			bTxData[2]=3000>>8;
			bTxData[3]=3000&0xff;
			bTxData[4]=wMode>>8;
			bTxData[5]=wMode&0xff;
		}
	}
	TSPI_PacketSend(bTxData,1);
}

void DischargeAndDriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wPulseBeforDis,WORD wTotalPulse,BYTE bSpeed,WORD wVoltage,WORD wDischargeTime)
{
	BYTE i,bTxData[16];

	st_wDiachargeRunTime=wDischargeTime;
	bTxData[0]=0xa1;
	bTxData[1]=14;
	bTxData[2]=bMotorInex;
	bTxData[3]=bDirection;
	bTxData[4]=wPulseBeforDis>>8;
	bTxData[5]=(BYTE)wPulseBeforDis;
	bTxData[6]=wTotalPulse>>8;
	bTxData[7]=(BYTE)wTotalPulse;
	bTxData[8]=bSpeed;
	bTxData[9]=wVoltage>>8;
	bTxData[10]=(BYTE)wVoltage;
	bTxData[11]=wDischargeTime>>8;
	bTxData[12]=(BYTE)wDischargeTime;
	
	TSPI_PacketSend(bTxData,1);
}


#endif

void TimerToProcWin(void)
{
	EventSet(UI_EVENT, EVENT_PROC_DATA);
}

void TimerToResetGetCenter(void)
{
	MP_DEBUG("TimerToResetGetCenter   state=%d",st_dwGetCenterState);
	st_dwGetCenterState=GET_CENTER_OFF;
}

void AutoDischarge(void)
{
		Discharge(st_wDischargeTimeSet,0);
}

void AutoGetFiberLowPoint()
{
	BYTE i;
	
	if (st_dwGetCenterState==GET_CENTER_OFF)
	{
		st_dwGetCenterState=GET_CENTER_INIT;//GET_CENTER_INIT;GET_CENTER_LOW_POINT
		TimerToFillProcWin(10);
	}
	else
		st_dwGetCenterState=GET_CENTER_OFF;

}

SWORD SearchWholeFiber(ST_IMGWIN *pWin)
{
	DWORD dwOffset;
	WORD wLastY,wCurY;
	SWORD i,x,y,swXEnd,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;
	swXEnd=pWin->dwOffset;//(g_wCenterPos<<1)+(g_wCenterPos>>1);
	mpDebugPrint(" _SearchWholeFiber_");

	/* 查找整个面是否为一条完整的纤*/
	swStartY=0;
	swYEnd=pWin->wHeight;
	wLastY=0;
	for (x=X_STEP;x<swXEnd;x+=X_STEP)
	{
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		bContinueCnt=0;
		wCurY=0;
		//mpDebugPrint(" %d: ",x>>1);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]  )
			{
				bContinueCnt++;
				if (bContinueCnt>=Y_CONTINUE_VALID_SUM)
				{
					wCurY=y-bContinueCnt;
					//Idu_OsdPaintArea(x>>1, wCurY, 2, 1, OSD_COLOR_RED);
					break;
				}
			}
			else
			{
				bContinueCnt=0;
			}
			y++;
			pbWinBuffer+=dwOffset;
		}
		if (y>=swYEnd)
		{
			mpDebugPrint("No whole fiber in %d ",x>>1);
			break;
		}
#if 0
		if (wLastY)
		{
			if (wLastY+5<wCurY || wLastY>wCurY+5)
			{
				mpDebugPrint("whole fiber error  x=%d wCurY=%d wLastY=%d",x>>1,wCurY,wLastY);
				break;
			}
			else
			{
				wLastY=(wLastY+wCurY)>>1;
			}
		}
		else
		{
			wLastY=wCurY;
		}
#endif
		TaskYield();
		//mpDebugPrint(" ");
	}
#if 0
	if (x>=swXEnd)
	{
		if (!st_wFiberWidth)
		{
			SearchLeftFiberFaceAndTopEdge(pWin,FIBER_LEFT_TOP,1);
			SearchLeftFiberBottomEdge(pWin,FIBER_LEFT_TOP);
			if (st_swFaceY1[FIBER_LEFT_TOP]>0&&st_swFaceY3[FIBER_LEFT_TOP]>0&&st_swFaceY3[FIBER_LEFT_TOP]>st_swFaceY1[FIBER_LEFT_TOP])
			{
				st_wFiberWidth=st_swFaceY3[FIBER_LEFT_TOP]-st_swFaceY1[FIBER_LEFT_TOP];
				return PASS;
			}
		}
	}
#endif
	if (x<swXEnd)
		return FAIL;
	else
		return PASS;
}

#define		X_GETLOW_STEP											4       //4 快速横向查找步长，必须为4的倍数，一个单位为半个像素

SWORD SearchFiberLowPoint(ST_IMGWIN *pWin,BYTE bWinIndex)
{
	DWORD dwOffset;
	WORD wCurY,wMaxY,wMinY,wSamePoint,wGoDown,wGoUp,wReverse,wDeep,wContinueCnt,wUnContinue;
	SWORD i,x,y,swXEnd,swStartY,swYEnd,swLowX,swRet=FAIL;
	BYTE *pbWinBuffer,bGetLowPiont,bError=0,bSensorIndex;

	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
		bSensorIndex=SENSER_TOP;
	dwOffset=pWin->dwOffset;
	swXEnd=pWin->dwOffset-pWin->wWidth/4;//(g_wCenterPos<<1)+(g_wCenterPos>>1);

	swStartY=0;
	swYEnd=pWin->wHeight;

	for (wDeep=8;wDeep<pWin->wWidth/X_STEP;wDeep+=4)
	{
		wMaxY=0;
		bGetLowPiont=1;
		wGoDown=0;
		wGoUp=0;
		wReverse=0;
		wSamePoint=0;
		Idu_OsdErase();
		for (x=pWin->wWidth/4;x<swXEnd;x+=X_GETLOW_STEP)
		{
			y=swStartY;
			pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
			wContinueCnt=0;
			wCurY=0;
			//mpDebugPrint(" %d: ",x);
			while (y<swYEnd)
			{
				//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex] )
				{
					wContinueCnt++;
					if (wContinueCnt>=Y_CONTINUE_VALID_SUM)
					{
						wCurY=y-wContinueCnt;
						//Idu_OsdPaintArea(x>>1, pWin->wHeight/2*bWinIndex+wCurY/2, 2, 1, OSD_COLOR_RED);
						Idu_OsdPaintArea(x>>1, wCurY, 2, 1, OSD_COLOR_RED);
						//mpDebugPrintN("%d ",wCurY);
						break;
					}
				}
				else
				{
					wContinueCnt=0;
				}
				y++;
				pbWinBuffer+=dwOffset;
			}
			if (y>=swYEnd)
			{
				//光纤断
				bError=1;
				mpDebugPrint("fiber error=%d,x=%d ",bError,x>>1);
				break;
			}
			if (wCurY<4 || wCurY>=swYEnd)
				continue;
			if (!wMaxY)
			{
				wMaxY=wMinY=wCurY;
				swLowX=x>>1;
			}
			else
			{
				//mpDebugPrintN("%d ",wCurY);
				if (wMinY>wCurY)
					wMinY=wCurY;
				if (bGetLowPiont)
				{
					if (wCurY>wMaxY)
					{
						//往下走
						wGoDown++;
						if (wGoUp && wGoDown>wDeep/2)
							wGoUp=0;
						wMaxY=wCurY;
						swLowX=x>>1;
						wSamePoint=0;
						//mpDebugPrint(" (%d,%d)",swLowX,wMaxY);
					}
					else if (wMaxY==wCurY)
					{
						wSamePoint++;
					}
					else
					{
						//往上走
						wGoUp++;
						if (wGoUp>wDeep/2)
						{
							if (wGoDown>wDeep)
							{
								//已找到低点开始回走
								bGetLowPiont=0;
								wReverse=0;
							}
							else
							{
								wGoDown=0;
							}
						}
					}
				}
				else
				{
					//又往下走
					if (wCurY>wMaxY)
					{
						wReverse++;
						//wSamePoint++;
						// 再次找低点，是个错误
						if (wReverse>wGoDown/2) 
						{
							bError=2;
							//放电熔的幅度不够
						}
					}
					else if (wReverse && wCurY<wMaxY)
					{
						wReverse=0;
					}
				}
				
			}
			TaskYield();
			//mpDebugPrint(" ");
		}
		//mpDebugPrint(" ");
		mpDebugPrint(" --bError=%d swLowX=%d bGetLowPiont=%d wSamePoint=%d wDeep=%d/%d",bError,swLowX,bGetLowPiont,wSamePoint,wGoDown,wDeep);
		mpDebugPrint(" ----wMaxY=%d wMinY=%d",wMaxY,wMinY);
		switch (bError)
		{
			case 0:
				if (bGetLowPiont)
					swRet=1; 
				else
				{
					if ((wSamePoint >40) || (wMaxY-wMinY<40))// pWin->dwOffset/100 -st_wFiberWidth/8
					{
						swRet=1; //re discharge
					}
					else
					{
						swRet=PASS;
						swLowX+=(wSamePoint*X_GETLOW_STEP>>2);
						//Idu_OsdPaintArea(swLowX, 0, 2, pWin->wHeight, OSD_COLOR_RED);
						#if 0 //4 计算最底点的光纤厚度
						x=swLowX<<1;
						y=wMinY;
						pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
						wContinueCnt=0;
						wUnContinue=0;
						while (y<swYEnd)
						{
							if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex] )
							{
								Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_RED);
								wContinueCnt++;
								wUnContinue=0;
							}
							else if (wContinueCnt)
							{
								 if (wContinueCnt>5)
								 {
									wUnContinue++;
									if (wUnContinue>3)
										break;
								 }
								 else
								 {
									wContinueCnt=0;
								 }
							}
							y++;
							pbWinBuffer+=dwOffset;
						}
						mpDebugPrint(" ----thick %d",wContinueCnt);
						if (!wContinueCnt || wContinueCnt>20)
						{
							if (st_bRetryTimes)
								swRet=2; //re discharge
						}
						#endif

					}
				}
				break;

			//光纤断
			case 1:
				swRet=FAIL; 
				break;

			// 再次找低点
			case 2:
				if ((wSamePoint >40) || (wMaxY-wMinY<40)) //st_wFiberWidth
				{
					swRet=1; //re discharge
				}
				else
				{
					bGetLowPiont=1;
					wGoDown=0;
					wGoUp=0;
					wReverse=0;
					wDeep+=4;
					swRet=10;
				}
				break;

			default:
				swRet=FAIL; 
				break;
		}
		if (swRet<10)
			break;
	}

//--swRet:  <0 FAIL  ==0->retry  >0 ->ok
	if (swRet>0)//(swRet==1 || swRet==10)
	{
		if (st_wDischargeTimeSet)
		{
			//st_wDischargeTimeSet+=200;
			Discharge(st_wDischargeTimeSet,0);
		}
		swRet=0;
	}
	else if (swRet==PASS)
	{
		swRet=swLowX;
	}

	return swRet;
}

void ProcFiberLowFillWin(DWORD dwTime)
{
		TimerToFillReferWin(dwTime,FILL_WIN_UP);
}

void Proc_GetFiberLowPoint(void)
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet;
	BYTE bSetEvent=0;

	if (!st_dwGetCenterState)
		return;
	MP_DEBUG("Proc_GetFiberLowPoint   %d ",st_dwGetCenterState);

	switch (st_dwGetCenterState)
	{
		case GET_CENTER_INIT:
			st_bRetryTimes=1;
			st_wDischargeTimeSet=500;
			Idu_OsdErase();
			if (!g_psSetupMenu->bBackGroundLevel[0] ||!g_psSetupMenu->bBackGroundLevel[1])
			{
				MP_DEBUG("GetFiberLowPoint   BackGroundLevel error !");
				st_dwGetCenterState=GET_CENTER_OFF;
			}
			else
			{
					g_bDisplayMode=0x80;
					st_dwGetCenterState++;
					ProcFiberLowFillWin(10);
					Discharge(1,0);
					Ui_TimerProcAdd(5000, TimerToResetGetCenter);
			}
			break;

		case GET_CENTER_WHOLE_FIBER:
			pWin=(ST_IMGWIN *)&SensorInWin[0];
			if (SearchWholeFiber(pWin)==PASS)
			{
				st_bRetryTimes=5;
				ProcFiberLowFillWin(10);
				st_dwGetCenterState++;
			}
			else
			{
				MP_DEBUG("SearchWholeFiber   FAIL !");
				if (st_bRetryTimes)
				{
					st_bRetryTimes--;
					ProcFiberLowFillWin(10);
				}
				else
					st_dwGetCenterState=GET_CENTER_OFF;
			}
			break;

		case GET_CENTER_LOW_POINT:
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				swRet=SearchFiberLowPoint(pWin,0);
			//--swRet:  <0 FAIL  ==0->retry  >0 ->ok
			if (swRet<=0 && st_bRetryTimes)//retry
			{
					st_bRetryTimes--;
					/*
					if (st_wDischargeTimeSet)
						TimerToFillProcWin(st_wDischargeTimeSet+100);
					else
						TimerToFillProcWin(1000);
						*/
					ProcFiberLowFillWin(10);
					Ui_TimerProcAdd(5000, TimerToResetGetCenter);
					MP_DEBUG("GET_CENTER_LOW_POINT   retry=%d  delay[%d]",st_bRetryTimes,st_wDischargeTimeSet);
			}
			else
			{
				MP_DEBUG("GET_CENTER_LOW_POINT   swRet=%d  [%d]",swRet,g_psSetupMenu->wElectrodePos[0]);
				if (swRet<=0)
				{
					swRet=pWin->wWidth/2;
				}
				else
				{
					ST_OSDWIN * psWin=Idu_GetOsdWin();
					BYTE bstring[16];
					
					Idu_OsdPaintArea(swRet,0,2,pWin->wHeight,OSD_COLOR_RED);
					Idu_OsdPaintArea(pWin->wWidth/2,0,2,pWin->wHeight,OSD_COLOR_GREEN);
					sprintf(bstring, " %d ", swRet);
					Idu_OSDPrint(psWin,bstring, 16, 16, OSD_COLOR_RED);
				}
				g_psSetupMenu->wElectrodePos[0]=swRet;
				WriteSetupChg();
				st_dwGetCenterState++;
				EventSet(UI_EVENT, EVENT_PROC_DATA);
				bSetEvent=1;
			}
			break;

		case GET_CENTER_FINISH:
		#if 1
				st_dwGetCenterState=GET_CENTER_OFF;
		#else
			if (g_wElectrodePos[0] && g_wElectrodePos[1])
				st_dwGetCenterState=GET_CENTER_OFF;
			else
			{
				st_bRetryTimes=3;
				st_wDischargeTimeSet=0;
				st_dwGetCenterState=GET_CENTER_LOW_POINT;
				if (g_wElectrodePos[0]==0)
				{
					g_bDisplayMode=0x80;
				}
				else
				{
					g_bDisplayMode=0x81;
				}
				ProcFiberLowFillWin(10);
			}
			#endif
			break;

		default:
			break;
	}

}



#define		AF_MOTO_INDEX									1
#define		WHITE_TO_BLACK								0x40
#define		BLACK_TO_WHITE								0x40

SWORD g_swAutoFocusState = AF_OFF;

DWORD st_dwAFbrightChg=0;
WORD st_wAFStartX,st_wAFStartY,st_wWhiteWidth;
SWORD st_AFLastWidth,st_AFMinWidth;
static BYTE st_bAFMotoIndex=6,st_bAFReturnTimes,st_bAFFirsetTry,st_bToolMotoIndex=1;
SWORD st_swAFStep=100;

void ProcAFmotorAction()
{
	//st_bAFMotoIndex=6;
	MoveAFMotor(st_bAFMotoIndex,st_swAFStep);
}

void ProcAFData()
{
	BYTE *pbWinBuffer,bCnt,bLastValue,bMaxValue,bMinValue;
	WORD i,x,y,wy1;
	SWORD swNewW,swEndY;
	DWORD dwAverLevel,dwOffsetLevel;
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];

	x=st_wAFStartX;
	y=st_wAFStartY;
	swEndY=pWin->wHeight/2;
	dwAverLevel=0;
	//MP_DEBUG("wX=%d wY=%d : ",wX,wY);
	pbWinBuffer = (BYTE *) pWin->pdwStart+(x<<1)+y*pWin->dwOffset;
	//找光纤中间白条上沿
	bCnt=0;
	for (;y<swEndY;y++)
	{
		if (*pbWinBuffer>g_psSetupMenu->bBackGroundLevel[0])
		{
			bCnt++;
			if (bCnt==4)
				break;
		}
		else
			bCnt=0;
		pbWinBuffer+=pWin->dwOffset;
	}

	//找光纤中间第一次最白转到黑
	if (bCnt<4)
		return;
	{
		bCnt=0;
		bMaxValue=0;
		bLastValue=*pbWinBuffer;
		for (;y<swEndY;y++)
		{
			if (bLastValue>*pbWinBuffer) // last pixel max
			{
				bMaxValue=bLastValue;
				break;
			}
			bLastValue=*pbWinBuffer;
			pbWinBuffer+=pWin->dwOffset;
		}
		if (bMaxValue)
		{
			bMinValue=0;
			for (;y<swEndY;y++)
			{
				if (bLastValue<*pbWinBuffer) // last pixel min
				{
					bMinValue=bLastValue;
					break;
				}
				bLastValue=*pbWinBuffer;
				pbWinBuffer+=pWin->dwOffset;
			}
		}
		/*
		if (bMinValue &&  bMaxValue>bMinValue)
		{
			st_dwAFbrightChg=bMaxValue-bMinValue;
			st_bAFFirsetTry=0;
			st_wAFStartX=x;
		}
		*/
	}

	
	// 判断下一步动作
	if (bMinValue &&  bMaxValue>bMinValue)
	{
		if (st_dwAFbrightChg>bMaxValue-bMinValue) //方向错
		{
			if (st_bAFFirsetTry)
				st_bAFFirsetTry=0;
			else
				st_bAFReturnTimes++;
			st_swAFStep=0-st_swAFStep;
			ProcAFmotorAction();
			g_swAutoFocusState=AF_PROC;
			Ui_TimerProcAdd(100, SetFillProcWinFlag);
		}
		else //方向对
		{
				if (!st_bAFReturnTimes)
				{
					ProcAFmotorAction();
					g_swAutoFocusState=AF_PROC;
					Ui_TimerProcAdd(100, SetFillProcWinFlag);
				}
				else
				{
						MP_DEBUG("AF OK!! try=%d",st_bAFReturnTimes);
						g_swAutoFocusState=AF_FINISH;
						MotorSetStatus(st_bAFMotoIndex,MOTOR_NO_HOLD);
				}
		}
		st_dwAFbrightChg=bMaxValue-bMinValue;
		MP_DEBUG("AF =%d",st_dwAFbrightChg);

	}

}

void Proc_AutoFocus()
{
	SWORD i,x,y,swStartX,swEndX,swStartY,swEndY,bCnt;
	BYTE *pbWinBuffer,bFind,bLastValue,bMaxValue,bMinValue;
	WORD wWhiteCnt,wBlackCnt,wy1,wWhiteWidth;
	DWORD dwData,dwCntLevel;
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];

	if (st_bNeedFillProcWin)
	{
		Ui_TimerProcAdd(10, Proc_AutoFocus);
		return;
	}
	//pWin=(ST_IMGWIN *)Idu_GetCurrWin();

	switch (g_swAutoFocusState)
	{
		case AF_INIT:
			swStartX=8;
			swEndX=pWin->wWidth;
			swStartY=4;
			swEndY=pWin->wHeight/2;
			for (x=swStartX;x<swEndX;x+=16)
			{
				y=swStartY;
				pbWinBuffer = (BYTE *) pWin->pdwStart+(x<<1)+y*pWin->dwOffset;
				// find fiber edge
				bCnt=0;
				for (;y<swEndY;y++)
				{
					if (*pbWinBuffer<g_psSetupMenu->bBackGroundLevel[0])
					{
						bCnt++;
						if (bCnt==4)
						{
							break;
						}
					}
					else
						bCnt=0;
					pbWinBuffer+=pWin->dwOffset;
				}
				//找光纤中间白条上沿
				if (bCnt<4)
					continue;
				{
					bCnt=0;
					for (;y<swEndY;y++)
					{
						if (*pbWinBuffer>g_psSetupMenu->bBackGroundLevel[0])
						{
							bCnt++;
							if (bCnt==4)
							{
								st_wAFStartY=y;
								break;
							}
						}
						else
							bCnt=0;
						pbWinBuffer+=pWin->dwOffset;
					}
				}
				//找光纤中间第一次最白转到黑
				if (bCnt<4)
					continue;
				{
					bCnt=0;
					bMaxValue=0;
					bLastValue=*pbWinBuffer;
					for (;y<swEndY;y++)
					{
						if (bLastValue>*pbWinBuffer) // last pixel max
						{
							bMaxValue=bLastValue;
							break;
						}
						/*
						if (*pbWinBuffer<BLACK_TO_WHITE)
						{
							bCnt++;
							if (bCnt==4)
							{
								st_wWhiteWidth=y-wy1;
								st_wAFStartX=x;
								MP_DEBUG("init w=%d x=%d y=%d",st_wWhiteWidth,x,y);
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,0,x,wy1-5,64,2,OSD_COLOR_RED);
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,1,x,y,64,2,OSD_COLOR_RED);
								xpgUpdatePageOsd();
								st_bAFReturnTimes=0;
								st_bAFFirsetTry=1;
								st_AFLastWidth=y-wy1;
								st_AFMinWidth=st_AFLastWidth;
								st_swAFStep=2;
								ProcAFmotorAction();
								g_swAutoFocusState=AF_PROC;
								Ui_TimerProcAdd(100, SetFillProcWinFlag);
								break;
							}
						}
						*/
						bLastValue=*pbWinBuffer;
						pbWinBuffer+=pWin->dwOffset;
					}
					if (bMaxValue)
					{
						bMinValue=0;
						for (;y<swEndY;y++)
						{
							if (bLastValue<*pbWinBuffer) // last pixel min
							{
								bMinValue=bLastValue;
								break;
							}
							bLastValue=*pbWinBuffer;
							pbWinBuffer+=pWin->dwOffset;
						}
					}
					if (bMinValue &&  bMaxValue>bMinValue)
					{
						st_wAFStartX=x;
						st_dwAFbrightChg=bMaxValue-bMinValue;
						st_bAFFirsetTry=1;
						st_swAFStep=3;
						st_bAFReturnTimes=0;
						ProcAFmotorAction();
						MotorSetStatus(st_bAFMotoIndex,MOTOR_HOLD);
						g_swAutoFocusState=AF_PROC;
						Ui_TimerProcAdd(100, SetFillProcWinFlag);
						MP_DEBUG("AF start (%d,%d)",st_wAFStartX,st_wAFStartY);
						break;
					}
				}

			}
			break;
		case AF_PROC:
			ProcAFData();
			break;

		case AF_FINISH:
				MotorSetStatus(st_bAFMotoIndex,MOTOR_NO_HOLD);
			//g_swAutoFocusState=AF_OFF;
			break;

		case AF_OFF:
		default:
			break;
	}
	if (g_swAutoFocusState!=AF_FINISH && g_swAutoFocusState!=AF_OFF)
		Ui_TimerProcAdd(100, Proc_AutoFocus);
}

void TimerToNextState(void)
{
	if (st_dwProcState==SENSOR_IDLE)
		return;
	st_dwProcState++;
	mpDebugPrint("TimerToNextState %d",st_dwProcState);
	EventSet(UI_EVENT, EVENT_PROC_DATA);
}


#if ALIGN_FAST_MODE
void Proc_Weld_State_Fast()
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD i,swRet,swSrcPos,swTrgPos;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下
	static BYTE st_bRedo,st_bNeedForward[2],st_bRet[FIBER_WINDOW_NUM];
	static SWORD st_swStartPosx[2],st_swLastPos[2],st_swPulse[2];
	static DWORD st_dwRedo1;

#if TSPI_ENBALE
	TSPI_Receive_Check();
#endif
#if SENSOR_WIN_NUM

	//MP_DEBUG("st_dwProcState   %d ",st_dwProcState);
	if (st_dwProcState & BIT30)
		return;

	switch (st_dwProcState)
	{
		case SENSOR_PROC_INIT:
				MP_DEBUG("SENSOR_PROC_INIT");
				Idu_OsdErase();
				//g_bDisplayMode=0x80;
				for (i=0;i<2;i++)
				{
					st_swStartPosx[i]=-3;
					st_swLastPos[i]=-2;
					st_swPulse[i]=0;
					st_bNeedForward[i]=0;
				}
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
				st_bRedo=POS_RETRY_TIMES;
				st_dwRedo1=POS_RETRY_TIMES;
				st_wDischargeTimeSet=0;
				ResetFiberPara();
				TspiSendSimpleInfo0xAF(0x02);
				MotorSetStatus(01,MOTOR_HOLD);
				MotorSetStatus(02,MOTOR_HOLD);
				#if 0//OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,0,g_psSetupMenu->wElectrodePos[0]-180, pWin->wHeight/4,2,pWin->wHeight/2,OSD_COLOR_GREEN);
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,1,g_psSetupMenu->wElectrodePos[0]-40, pWin->wHeight/4,2,pWin->wHeight/2,OSD_COLOR_GREEN);
				#endif
				st_dwProcState++;
				SetFillProcWinFlag();
				break;
				
		case SENSOR_FACE_POS1A://--for POS0
			bMode=FIBER_LEFT_TOP;
			if (!st_bRet[bMode])
			{
				st_bRet[bMode]=ScanFiberFaceExistPos(pWin,SENSOR_UP,FEBER_POS0)!=FAIL;
				if (!st_bRet[bMode])
				{
					DriveMotor(MOTOR_H_LEFT,MOTOR_FF,0xffff,10);
				}
				else
				{
					MP_DEBUG("SENSOR_FACE_POS1A1  STOP");
					//DriveMotor(MOTOR_H_LEFT,MOTOR_FF,10,8);
					MotorSetStatus(MOTOR_H_LEFT,MOTOR_STOP);
				}
			}
			else if (st_swStartPosx[bMode]<=0 && !(st_bMotorStaus&MOTOR_HL_BIT))
			{
				st_swFaceX[bMode]=-1;
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
				if (st_swFaceX[bMode]>0)
				{
					if (st_swLastPos[bMode]!=st_swFaceX[bMode])
					{
						st_swLastPos[bMode]=st_swFaceX[bMode];
					}
					else
					{
						//--纤过了POS1
						if (st_swFaceX[bMode]>g_psSetupMenu->wElectrodePos[0]-FEBER_POS1-MOTO_TEST_PIXEL)
						{
							st_bNeedForward[bMode]=1;
							swRet=st_swFaceX[bMode]-(g_psSetupMenu->wElectrodePos[0]-FEBER_POS1-MOTO_TEST_PIXEL)+20;
							DriveMotor(MOTOR_H_LEFT,MOTOR_FB,swRet*HMOTO_REF_SPEED/2+MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
						}
						else if (st_bNeedForward[bMode])
						{
							//--抵消空转脉冲
							st_bNeedForward[bMode]=0;
							DriveMotor(MOTOR_H_LEFT,MOTOR_FF,MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
						}
						else
						{
							//--走到POS1并测速
							st_swStartPosx[bMode]=st_swFaceX[bMode];
							//st_swPulse[bMode]=(g_psSetupMenu->wElectrodePos[0]-FEBER_POS1-st_swFaceX[bMode])*HMOTO_REF_SPEED>>1;
							st_swPulse[bMode]=(g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3]-10-st_swFaceX[bMode])*HMOTO_REF_SPEED>>1;
							//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,OSD_LINE_NUM-1,st_swStartPosx[bMode],24,2,64,OSD_COLOR_BLUE);
							DriveMotor(MOTOR_H_LEFT,MOTOR_FF,st_swPulse[bMode],MOTO_TEST_SPEED);
						}
					}
				}
				else if (st_dwRedo1)
				{
					st_dwRedo1--;
					//--万一找不到端面时稍移动一下
					DriveMotor(MOTOR_H_LEFT,MOTOR_FF,20,8);
					st_swLastPos[bMode]=0;
				}
			}

			bMode=FIBER_RIGHT_TOP;
			if (!st_bRet[bMode])
			{
				st_bRet[bMode]=ScanFiberFaceExistPos(pWin,SENSOR_DOWN,pWin->wWidth-FEBER_POS0)!=FAIL;
				if (!st_bRet[bMode])
				{
					DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,0xffff,10);
				}
				else
				{
					mpDebugPrint("SENSOR_FACE_POS1A2  STOP");
					//DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,10,8);
					MotorSetStatus(MOTOR_H_RIGHT,MOTOR_STOP);
				}
			}
			else if (st_swStartPosx[bMode]<=0 && !(st_bMotorStaus&MOTOR_HR_BIT))
			{
				st_swFaceX[bMode]=-1;
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1,0);
				if (st_swFaceX[bMode]>0)
				{
					if (st_swLastPos[bMode]!=st_swFaceX[bMode])
					{
						st_swLastPos[bMode]=st_swFaceX[bMode];
					}
					else
					{
						if (st_swFaceX[bMode]<g_psSetupMenu->wElectrodePos[0]+FEBER_POS1+MOTO_TEST_PIXEL)
						{
							//--纤过了POS1
							st_bNeedForward[bMode]=1;
							swRet=g_psSetupMenu->wElectrodePos[0]+FEBER_POS1+MOTO_TEST_PIXEL-st_swFaceX[bMode]+30;
							st_swFaceX[bMode]=-1;
							DriveMotor(MOTOR_H_RIGHT,MOTOR_FB,swRet*HMOTO_REF_SPEED/2+MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
						}
						else if (st_bNeedForward[bMode])
						{
							//--抵消空转脉冲
							st_bNeedForward[bMode]=0;
							DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
						}
						else
						{
							//--走到POS1并测速
							st_swStartPosx[bMode]=st_swFaceX[bMode];
							//st_swPulse[bMode]=(st_swFaceX[bMode]-g_psSetupMenu->wElectrodePos[0]-FEBER_POS1)*HMOTO_REF_SPEED>>1;
							st_swPulse[bMode]=(st_swFaceX[bMode]-g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3]-10)*HMOTO_REF_SPEED>>1;
							//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,OSD_LINE_NUM-1,st_swStartPosx[bMode],24,2,64,OSD_COLOR_BLUE);
							DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,st_swPulse[bMode],MOTO_TEST_SPEED);
						}
					}
				}
				else if (st_dwRedo1)
				{
					st_dwRedo1--;
					//--万一找不到端面时稍移动一下
					DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,20,8);
					st_swLastPos[bMode]=0;
				}
			}

			if (st_swStartPosx[FIBER_LEFT_TOP]>0 && st_swStartPosx[FIBER_RIGHT_TOP]>0)
			{
				MP_DEBUG("SENSOR_FACE_POS1A  OK!");
				st_dwRedo1=POS_RETRY_TIMES;
				st_dwProcState++;
			}
			else if (!st_dwRedo1)
			{
				mpDebugPrint("!!!ERROR!!! SENSOR_FACE_POS1A ");
				st_dwProcState=SENSOR_RPOC_END;
			}
			SetFillProcWinFlag();
			break;
		case SENSOR_FACE_POS1B: //--reached  POS1 and get st_wMotorSpeed
			if (!st_wMotorSpeed[FIBER_LEFT_TOP])
			{
				bMode=FIBER_LEFT_TOP;
				st_swFaceX[bMode]=-1;
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
				//MP_DEBUG("SENSOR_FACE_POS1B L %d",st_swFaceX[bMode]);
				if (st_swLastPos[bMode]!=st_swFaceX[bMode])
				{
					st_swLastPos[bMode]=st_swFaceX[bMode];
				}
				else if (st_swFaceX[bMode]>st_swStartPosx[bMode])
				{
					// about 19.5puls/pixel
					swRet=st_swFaceX[bMode]-st_swStartPosx[bMode];
					st_wMotorSpeed[bMode]=((st_swPulse[bMode]<<1)+(swRet>>1))/swRet;
					if (st_wMotorSpeed[bMode]<HMOTO_REF_SPEED*88/100 || st_wMotorSpeed[bMode]>HMOTO_REF_SPEED*112/100)
						st_wMotorSpeed[bMode]=HMOTO_REF_SPEED;
					MP_DEBUG("st_wMotorSpeed[%d]=%d",bMode,st_wMotorSpeed[bMode]);
				}
			}
			if (!st_wMotorSpeed[FIBER_RIGHT_TOP])
			{
				bMode=FIBER_RIGHT_TOP;
				st_swFaceX[bMode]=-1;
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1,0);
				//MP_DEBUG("SENSOR_FACE_POS1B R %d",st_swFaceX[bMode]);
				if (st_swLastPos[bMode]!=st_swFaceX[bMode])
				{
					st_swLastPos[bMode]=st_swFaceX[bMode];
				}
				else if (st_swFaceX[bMode]<st_swStartPosx[bMode])
				{
					// about 19.5puls/pixel
					swRet=st_swStartPosx[bMode]-st_swFaceX[bMode];
					st_wMotorSpeed[bMode]=((st_swPulse[bMode]<<1)+(swRet>>1))/swRet;
					if (st_wMotorSpeed[bMode]<HMOTO_REF_SPEED*88/100 || st_wMotorSpeed[bMode]>HMOTO_REF_SPEED*112/100)
						st_wMotorSpeed[bMode]=HMOTO_REF_SPEED;
					MP_DEBUG("st_wMotorSpeed[%d]=%d",bMode,st_wMotorSpeed[bMode]);
				}
				//xpgUpdatePageOsd();
			}
			if (st_wMotorSpeed[FIBER_LEFT_TOP]&& st_wMotorSpeed[FIBER_RIGHT_TOP])
			{
				st_dwProcState++;
			}
			else if (!st_dwRedo1)
			{
				mpDebugPrint("!!!ERROR!!! SENSOR_FACE_POS1B ");
				st_dwProcState=SENSOR_RPOC_END;
			}
			SetFillProcWinFlag();
			break;

		case SENSOR_DISCHARGE1:
			if (st_wDischargeTimeSet!=1)
			{
				mpDebugPrint("SENSOR_DISCHARGE1");
				st_wDischargeTimeSet=1;
				Discharge(st_wDischargeTimeSet,0);
				Ui_TimerProcAdd(3000, TimerToNextState);
			}
			break;
		case SENSOR_AUTO_FOCUS:
#if 1
			for (i=0;i<FIBER_WINDOW_NUM;i++)
			{
				st_bRet[i]=0;
			}
			st_dwRedo1=5;//POS_RETRY_TIMES;
			if (g_psSetupMenu->bDuiXianFangShi==0)
				st_dwProcState+=2;
			else
				st_dwProcState++;
			SetFillProcWinFlag();
			//TimerToFillProcWin(200);
#else
			if (g_swAutoFocusState==AF_OFF)
			{
					g_swAutoFocusState=AF_INIT;
					Proc_AutoFocus();
			}
			else if (g_swAutoFocusState==AF_FINISH)
			{
					g_swAutoFocusState=AF_OFF;
					st_dwProcState++;
					st_dwProcState=SENSOR_RPOC_END;
			}
#endif
			break;

		case SENSOR_GET_ANGLE:
			#if 1
			//TOP
			//--left
			bMode=FIBER_LEFT_TOP;
			if (!st_bRet[bMode])
			{
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
				SearchLeftFiberBottomEdge(pWin,bMode);
				SearchLeftFiberCenter(pWin,bMode);
				SearchLeftFiberCore(pWin,bMode);
				if (st_swFaceX[bMode]>50 && st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>0 && st_swFaceY2[bMode] && st_swFaceY20[bMode]>0)
				{
						st_swHlevel[bMode]=SearchTopEdge(pWin,4,st_swFaceX[bMode]>>1,st_swFaceY1[bMode],st_swFaceY2[bMode]);
						if (st_swHlevel[bMode]!=FAIL)
							st_swHlevel[bMode]=(st_swFaceY1[bMode]-st_swHlevel[bMode])*100/st_swFaceX[bMode];
						mpDebugPrint(" SENSOR_GET_ANGLE  st_swHlevel[%d]=%d",bMode, st_swHlevel[bMode]);
						if (ABS(st_swHlevel[bMode])>LEVELNESS)
						{
							st_bRet[bMode]=1;
						}
						else
						{
							st_swCenterW[bMode]=ScanFiberCenterWidth(pWin,bMode);
							if (st_swCenterW[bMode]>CENTER_W_OFFSET+CENTER_W_OFFSET/2)
							{
									st_bRet[bMode]=2;
							}
							else if (st_swCenterW[bMode]<CENTER_W_OFFSET)
							{
								st_swAngle[bMode]=ScanLeftFaceAngle(pWin,bMode,st_swFaceY1[bMode],st_swFaceY3[bMode],g_psSetupMenu->bBackGroundLevel[SENSOR_UP]>>BG_BRIGHT_SHIFT);
								#if 1
								if (st_swAngle[bMode]>ANGEL_OFFSET+ANGEL_OFFSET/2)
									st_bRet[bMode]=3;
								else if (st_swAngle[bMode]<ANGEL_OFFSET)
									st_bRet[bMode]=0xff;
								#endif
							}
						}
				}
			}
			//TOP
			//--right
			bMode=FIBER_RIGHT_TOP;
			if (!st_bRet[bMode])
			{
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				SearchRightFiberCenter(pWin,bMode);
				SearchLeftFiberCore(pWin,bMode);
				if (st_swFaceX[bMode]>0 && st_swFaceX[bMode]<pWin->wWidth && st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>0 && st_swFaceY2[bMode] && st_swFaceY20[bMode]>0)
				{
						st_swHlevel[bMode]=SearchTopEdge(pWin,(pWin->wWidth+st_swFaceX[bMode])>>1,pWin->wWidth-4,st_swFaceY1[bMode],st_swFaceY2[bMode]);
						if (st_swHlevel[bMode]!=FAIL)
							st_swHlevel[bMode]=(st_swFaceY1[bMode]-st_swHlevel[bMode])*100/(pWin->wWidth-st_swFaceX[bMode]);
						mpDebugPrint(" SENSOR_GET_ANGLE  st_swHlevel[%d]=%d",bMode, st_swHlevel[bMode]);
						if (ABS(st_swHlevel[bMode])>LEVELNESS)
						{
							st_bRet[bMode]=1;
						}
						else
						{
							st_swCenterW[bMode]=ScanFiberCenterWidth(pWin,bMode);
							if (st_swCenterW[bMode]>CENTER_W_OFFSET+CENTER_W_OFFSET/2)
							{
									st_bRet[bMode]=2;
							}
							else if (st_swCenterW[bMode]<CENTER_W_OFFSET)
							{
								st_swAngle[bMode]=ScanLeftFaceAngle(pWin,bMode,st_swFaceY1[bMode],st_swFaceY3[bMode],g_psSetupMenu->bBackGroundLevel[SENSOR_UP]>>BG_BRIGHT_SHIFT);
								#if 1
								if (st_swAngle[bMode]>ANGEL_OFFSET+ANGEL_OFFSET/2)
									st_bRet[bMode]=3;
								else if (st_swAngle[bMode]<ANGEL_OFFSET)
									st_bRet[bMode]=0xff;
								#endif
							}
						}
				}
			}
			#endif
			//BOTTOM
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			#if 1
			//--left
			bMode=FIBER_LEFT_BOTTOM;
			if (!st_bRet[bMode])
			{
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
				SearchLeftFiberBottomEdge(pWin,bMode);
				SearchLeftFiberCenter(pWin,bMode);
				SearchLeftFiberCore(pWin,bMode);
				if (st_swFaceX[bMode]>50 && st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>0 && st_swFaceY2[bMode] && st_swFaceY20[bMode]>0)
				{
						st_swHlevel[bMode]=SearchTopEdge(pWin,4,st_swFaceX[bMode]>>1,st_swFaceY1[bMode],st_swFaceY2[bMode]);
						if (st_swHlevel[bMode]!=FAIL)
							st_swHlevel[bMode]=(st_swFaceY1[bMode]-st_swHlevel[bMode])*100/st_swFaceX[bMode];
						mpDebugPrint(" SENSOR_GET_ANGLE  st_swHlevel[%d]=%d",bMode, st_swHlevel[bMode]);
						if (ABS(st_swHlevel[bMode])>LEVELNESS)
						{
							st_bRet[bMode]=1;
						}
						else
						{
							st_swCenterW[bMode]=ScanFiberCenterWidth(pWin,bMode);
							if (st_swCenterW[bMode]>CENTER_W_OFFSET+CENTER_W_OFFSET/2)
							{
									st_bRet[bMode]=2;
							}
							else if (st_swCenterW[bMode]<CENTER_W_OFFSET)
							{
								st_swAngle[bMode]=ScanLeftFaceAngle(pWin,bMode,st_swFaceY1[bMode],st_swFaceY3[bMode],g_psSetupMenu->bBackGroundLevel[SENSOR_UP]>>BG_BRIGHT_SHIFT);
								#if 1
								if (st_swAngle[bMode]>ANGEL_OFFSET+ANGEL_OFFSET/2)
									st_bRet[bMode]=3;
								else if (st_swAngle[bMode]<ANGEL_OFFSET)
									st_bRet[bMode]=0xff;
								#endif
							}
						}
				}
			}
			#endif
			#if 1
			//BOTTOM
			//--right
			bMode=FIBER_RIGHT_BOTTOM;
			if (!st_bRet[bMode])
			{
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				SearchRightFiberCenter(pWin,bMode);
				SearchLeftFiberCore(pWin,bMode);
				if (st_swFaceX[bMode]>0 && st_swFaceX[bMode]<pWin->wWidth && st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>0 && st_swFaceY2[bMode] && st_swFaceY20[bMode]>0)
				{
						st_swHlevel[bMode]=SearchTopEdge(pWin,(pWin->wWidth+st_swFaceX[bMode])>>1,pWin->wWidth-4,st_swFaceY1[bMode],st_swFaceY2[bMode]);
						if (st_swHlevel[bMode]!=FAIL)
							st_swHlevel[bMode]=(st_swFaceY1[bMode]-st_swHlevel[bMode])*100/(pWin->wWidth-st_swFaceX[bMode]);
						mpDebugPrint(" SENSOR_GET_ANGLE  st_swHlevel[%d]=%d",bMode, st_swHlevel[bMode]);
						if (ABS(st_swHlevel[bMode])>LEVELNESS)
						{
							st_bRet[bMode]=1;
						}
						else
						{
							st_swCenterW[bMode]=ScanFiberCenterWidth(pWin,bMode);
							if (st_swCenterW[bMode]>CENTER_W_OFFSET+CENTER_W_OFFSET/2)
							{
									st_bRet[bMode]=2;
							}
							else if (st_swCenterW[bMode]<CENTER_W_OFFSET)
							{
								st_swAngle[bMode]=ScanLeftFaceAngle(pWin,bMode,st_swFaceY1[bMode],st_swFaceY3[bMode],g_psSetupMenu->bBackGroundLevel[SENSOR_UP]>>BG_BRIGHT_SHIFT);
								#if 1
								if (st_swAngle[bMode]>ANGEL_OFFSET+ANGEL_OFFSET/2)
									st_bRet[bMode]=3;
								else if (st_swAngle[bMode]<ANGEL_OFFSET)
									st_bRet[bMode]=0xff;
								#endif
							}
						}
				}
			}
			#endif
			if (st_bRet[FIBER_LEFT_TOP]==0xff && st_bRet[FIBER_RIGHT_TOP]==0xff && st_bRet[FIBER_LEFT_BOTTOM]==0xff && st_bRet[FIBER_RIGHT_BOTTOM]==0xff)
			{
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
				st_dwProcState++;
			}
			else if (!st_dwRedo1)
			{
					mpDebugPrint("!!!ERROR!!!  SENSOR_GET_ANGLE  %d %d %d %d",st_bRet[0] , st_bRet[1],st_bRet[2] , st_bRet[3]);
					PutErrorOsdString(st_bRet[0] , st_bRet[1],st_bRet[2] , st_bRet[3]);
					st_dwProcState=SENSOR_RPOC_END;
			}
			st_dwRedo1--;
			SetFillProcWinFlag();
			break;
		//水平移动到最后的间距
#if 1
		case SENSOR_FACE_POS2A: // TOP
			pWin=(ST_IMGWIN *)&SensorInWin[0];
			bMode=FIBER_LEFT_TOP;
			if (!st_bRet[bMode])
			{
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
				#if TEST_PLANE||ALIGN_DEMO_MODE
				swTrgPos=g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3];
				#else
				swTrgPos=g_psSetupMenu->wElectrodePos[0]-40;
				#endif
				if (MoveHMotorToSpecPosition(bMode,st_swFaceX[bMode],swTrgPos)==PASS)
				{
					st_bRet[bMode]=1;
				}
			}
			//break;
		case SENSOR_FACE_POS2B://BOTTOM
			bMode=FIBER_RIGHT_TOP;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,0);
			if (!st_bRet[bMode])
			{
				if (MoveHMotorToSpecPosition(bMode,g_psSetupMenu->wElectrodePos[0],st_swFaceX[bMode])==PASS)
				{
					st_bRet[bMode]=1;
				}
			}
			
			if (st_bRet[FIBER_LEFT_TOP] && st_bRet[FIBER_RIGHT_TOP])
			{
				mpDebugPrint(" SENSOR_FACE_POS2B  OK! ->%d",g_psSetupMenu->bDuiXianFangShi);
				if (!g_psSetupMenu->bDuiXianFangShi)
					st_dwProcState=SENSOR_ALIGN_H3A;
				else
					st_dwProcState=SENSOR_ALIGN_H1A;
				//	st_dwProcState=SENSOR_ALIGN_H3A;
				st_bRedo=ALIGN_HORIZONTAL_REDO_TIMES;
				st_dwRedo1=POS_RETRY_TIMES;
				for (i=0;i<2;i++)
				{
					st_swLastPos[i]=-2;
					st_bNeedForward[i]=0;
				}
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
			}
			SetFillProcWinFlag();
			break;
#else
		case SENSOR_FACE_POS2A: // pos2
			if (SearchLeftFiberFaceAndTopEdge(pWin,FIBER_LEFT_TOP,1,0)==PASS && SearchRightFiberFaceAndTopEdge(pWin,FIBER_RIGHT_TOP,1,0)==PASS)
			{
				#if TEST_PLANE||ALIGN_DEMO_MODE
				swRet=g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3]-st_swFaceX[FIBER_LEFT_TOP]-10;
				#else
				swRet=g_psSetupMenu->wElectrodePos[0]-FEBER_POS2-st_swFaceX[FIBER_LEFT_TOP]-10;
				#endif
				if (st_swFaceX[FIBER_LEFT_TOP]>0 && swRet>0)
				{
					swTrgPos =swRet*st_wMotorSpeed[FIBER_LEFT_TOP]>>1;
					DriveMotor(MOTOR_H_LEFT,MOTOR_FF,swTrgPos,MOTO_TEST_SPEED);
				}

				#if TEST_PLANE||ALIGN_DEMO_MODE
				swRet=st_swFaceX[FIBER_RIGHT_TOP]-g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3]-10;
				#else
				swRet=st_swFaceX[FIBER_RIGHT_TOP]-g_psSetupMenu->wElectrodePos[0]-FEBER_POS2-10;
				#endif
				if (st_swFaceX[FIBER_RIGHT_TOP]>0 && swRet>0)
				{
					swTrgPos =swRet*st_wMotorSpeed[FIBER_RIGHT_TOP]>>1;
					DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,swTrgPos,MOTO_TEST_SPEED);
				}
				MP_DEBUG("SENSOR_FACE_POS2A  OK!");
				for (i=0;i<2;i++)
				{
					st_swLastPos[i]=-2;
					st_bNeedForward[i]=0;
				}
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
				st_dwRedo1=GetSysTime();
				st_dwProcState++;
			}
			SetFillProcWinFlag();
			break;
		//确认最后的水平间距
		case SENSOR_FACE_POS2B: // BOTTOM
			if (!st_bRet[0])
			{
				bMode=FIBER_LEFT_TOP;
				if (st_bNeedForward[bMode])
				{
					//--抵消空转脉冲
					st_bNeedForward[bMode]=0;
					DriveMotor(MOTOR_H_LEFT,MOTOR_FF,MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
				}
				else if (!(st_bMotorStaus&MOTOR_HL_BIT))
				{
					st_swFaceX[bMode]=-1;
					SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
					if (st_swLastPos[bMode]!=st_swFaceX[bMode])
					{
						//--去抖
						st_swLastPos[bMode]=st_swFaceX[bMode];
					}
					else if (st_swFaceX[bMode]>0 && st_swLastPos[bMode]==st_swFaceX[bMode])
					{
						//--TOP
						#if TEST_PLANE||ALIGN_DEMO_MODE
						swTrgPos=g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3];
						#else
						swTrgPos=g_psSetupMenu->wElectrodePos[0]-FEBER_POS2;
						#endif
						if (st_swFaceX[bMode]<swTrgPos-30)
						{
							//--未到指定位置
							swTrgPos=swTrgPos-st_swFaceX[bMode]-10;
							DriveMotor(MOTOR_H_LEFT,MOTOR_FF,swTrgPos*HMOTO_REF_SPEED/2,MOTO_TEST_SPEED);
						}
						else if (st_swFaceX[bMode]>swTrgPos)
						{
							//--走过了指定位置
							st_bNeedForward[bMode]=1;
							swRet=st_swFaceX[bMode]-swTrgPos+10;
							DriveMotor(MOTOR_H_LEFT,MOTOR_FB,swRet*HMOTO_REF_SPEED/2+MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
						}
						else
						{
							st_bRet[0]=1;
						}
					}
				}
			}
			
			if (!st_bRet[1])
			{
				bMode=FIBER_RIGHT_TOP;
				if (st_bNeedForward[bMode])
				{
					//--抵消空转脉冲
					st_bNeedForward[bMode]=0;
					DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
				}
				else if (!(st_bMotorStaus&MOTOR_HR_BIT))
				{
					st_swFaceX[bMode]=-1;
					SearchRightFiberFaceAndTopEdge(pWin,bMode,1,0);
					if (st_swLastPos[bMode]!=st_swFaceX[bMode])
					{
						//--去抖
						st_swLastPos[bMode]=st_swFaceX[bMode];
					}
					else if (st_swFaceX[bMode]>0 && st_swLastPos[bMode]==st_swFaceX[bMode])
					{
						//--BOTTOM
						#if TEST_PLANE||ALIGN_DEMO_MODE
						swTrgPos=g_psSetupMenu->wElectrodePos[0]+st_wMotoStep[MAX_ADJ_NUM-3];
						#else
						swTrgPos=g_psSetupMenu->wElectrodePos[0]+FEBER_POS2;
						#endif
						if (st_swFaceX[bMode]>swTrgPos+30)
						{
							//--未到指定位置
							swRet=st_swFaceX[bMode]-swTrgPos-10;
							DriveMotor(MOTOR_H_RIGHT,MOTOR_FF,swRet*HMOTO_REF_SPEED/2,MOTO_TEST_SPEED);
						}
						else if (st_swFaceX[bMode]<swTrgPos)
						{
							//--走过了指定位置
							st_bNeedForward[bMode]=1;
							swRet=swTrgPos-st_swFaceX[bMode]+10;
							DriveMotor(MOTOR_H_RIGHT,MOTOR_FB,swRet*HMOTO_REF_SPEED/2+MOTOR_HORI_REVERSE_PULSE,MOTO_TEST_SPEED);
						}
						else
						{
							st_bRet[1]=1;
						}
					}
				}
			}

			//--RESULT
			if (st_bRet[0]&& st_bRet[1])
			{
				mpDebugPrint(" SENSOR_FACE_POS2B  OK! ->%d",g_psSetupMenu->bDuiXianFangShi);
				if (g_psSetupMenu->bDuiXianFangShi==0)
					st_dwProcState=SENSOR_ALIGN_H3A;
				else
					st_dwProcState=SENSOR_ALIGN_H1A;
				st_bRedo=ALIGN_HORIZONTAL_REDO_TIMES;
				st_dwRedo1=POS_RETRY_TIMES;
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
				st_dwProcState++;
			}
			else if (SystemGetElapsedTime(st_dwRedo1)>RETRY_TIMEOUT)
			{
				mpDebugPrint("!!!ERROR!!! SENSOR_FACE_POS2B ");
				st_dwProcState=SENSOR_RPOC_END;
			}
			SetFillProcWinFlag();
			break;
#endif

		//--包层对齐
		case SENSOR_ALIGN_H1A:
		//--中间白芯对齐
		case SENSOR_ALIGN_H2A:
			//TOP
			//--left
			bMode=FIBER_LEFT_TOP;
			SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
			SearchLeftFiberBottomEdge(pWin,bMode);

			//BOTTOM
			//--right
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			bMode=FIBER_RIGHT_BOTTOM;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);

			if (st_swLastPos[0]==st_swFaceY1[FIBER_LEFT_TOP] && st_swLastPos[1]==st_swFaceY1[FIBER_RIGHT_BOTTOM])
			{
				st_swFaceY1[FIBER_RIGHT_BOTTOM]-=2;
				st_swFaceY3[FIBER_RIGHT_BOTTOM]-=2;
				mpDebugPrint(" FIBER_LEFT_TOP %d - %d=%d",st_swFaceY1[FIBER_LEFT_TOP],st_swFaceY3[FIBER_LEFT_TOP],(st_swFaceY1[FIBER_LEFT_TOP]+st_swFaceY3[FIBER_LEFT_TOP])/2);
				mpDebugPrint(" FIBER_RIGHT_BOTTOM %d - %d=%d",st_swFaceY1[FIBER_RIGHT_BOTTOM],st_swFaceY3[FIBER_RIGHT_BOTTOM],(st_swFaceY1[FIBER_RIGHT_BOTTOM]+st_swFaceY3[FIBER_RIGHT_BOTTOM])/2);
				mpDebugPrint("SENSOR_ALIGN_H1A OK");
				MotorSetStatus(MOTOR_V_UP,MOTOR_HOLD);
				MotorSetStatus(MOTOR_V_DOWN,MOTOR_HOLD);
				for (i=0;i<2;i++)
				{
					st_bNeedForward[i]=0;
				}
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
				st_dwRedo1=POS_RETRY_TIMES;
				st_dwProcState++;
			}
			else
			{
				if (st_swLastPos[0]!=st_swFaceY1[FIBER_LEFT_TOP]  && st_swFaceY1[FIBER_LEFT_TOP] >0)
				{
					st_swLastPos[0]=st_swFaceY1[FIBER_LEFT_TOP];
					st_swFaceY1[FIBER_LEFT_TOP] =-1;
				}
				if (st_swLastPos[1]!=st_swFaceY1[FIBER_RIGHT_BOTTOM]  && st_swFaceY1[FIBER_RIGHT_BOTTOM] >0)
				{
					st_swLastPos[1]=st_swFaceY1[FIBER_RIGHT_BOTTOM];
					st_swFaceY1[FIBER_RIGHT_BOTTOM] =-1;
				}
				st_dwRedo1--;
				if (!st_dwRedo1)
				{
					mpDebugPrint("!!!ERROR!!! SENSOR_ALIGN_H1A ");
					st_dwProcState=SENSOR_RPOC_END;
				}
			}
			SetFillProcWinFlag();
			break;
		//--调整可移动端
		case SENSOR_ALIGN_H1B:
		case SENSOR_ALIGN_H2B:
			//TOP
			//--right
			if (!st_bRet[0] && !(st_bMotorStaus&MOTOR_VU_BIT))
			{
				bMode=FIBER_RIGHT_TOP;
				SearchRightFiberFaceAndTopEdge(pWin,bMode,0,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				mpDebugPrint(" FIBER_RIGHT_TOP %d - %d",st_swFaceY1[bMode],st_swFaceY3[bMode]);
				if (st_swFaceY1[bMode]>0 || st_swFaceY3[bMode]>0)
				{
					if (st_swFaceY3[bMode]<=0)
						swSrcPos=(st_swFaceY1[bMode]+pWin->wHeight)>>1;
					else
						swSrcPos=(st_swFaceY1[bMode]+st_swFaceY3[bMode])>>1;
					if (st_swFaceY3[bMode-1]<0)
						swTrgPos=(st_swFaceY1[bMode-1]+pWin->wHeight)>>1;
					else
						swTrgPos=(st_swFaceY1[bMode-1]+st_swFaceY3[bMode-1])>>1;
					if (MoveVMotorToSpecPosition(bMode,swSrcPos,swTrgPos)==PASS)
					{
						if (st_bNeedForward[0])
						{
							mpDebugPrint("SENSOR_ALIGN_H1Ba OK  %d - %d",swSrcPos,swTrgPos);
							st_bRet[0]=1;
						}
						else
						{
							st_bNeedForward[0]=1;
						}
					}
				}
			}

			if (!st_bRet[1] && !(st_bMotorStaus&MOTOR_VD_BIT))
			{
				//BOTTOM
				pWin=(ST_IMGWIN *)&SensorInWin[1];
				//--left
				bMode=FIBER_LEFT_BOTTOM;
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,st_swFaceX[bMode]<0,1);
				SearchLeftFiberBottomEdge(pWin,bMode);
				if (st_swFaceY3[bMode]<=0)
					swSrcPos=(st_swFaceY1[bMode]+pWin->wHeight)>>1;
				else
					swSrcPos=(st_swFaceY1[bMode]+st_swFaceY3[bMode])>>1;
				if (st_swFaceY3[FIBER_RIGHT_BOTTOM]<=0)
					swTrgPos=(st_swFaceY1[FIBER_RIGHT_BOTTOM]+pWin->wHeight)>>1;
				else
					swTrgPos=(st_swFaceY1[FIBER_RIGHT_BOTTOM]+st_swFaceY3[FIBER_RIGHT_BOTTOM])>>1;
				mpDebugPrint(" FIBER_LEFT_BOTTOM (%d,%d)->(%d,%d)",st_swFaceY1[bMode],st_swFaceY3[bMode],st_swFaceY1[FIBER_RIGHT_BOTTOM],st_swFaceY3[FIBER_RIGHT_BOTTOM]);
				if (MoveVMotorToSpecPosition(bMode,swSrcPos,swTrgPos)==PASS)
				{
						if (st_bNeedForward[1])
						{
							mpDebugPrint("SENSOR_ALIGN_H1Bb OK %d - %d",swSrcPos,swTrgPos);
							st_bRet[1]=1;
						}
						else
						{
							st_bNeedForward[1]=1;
						}
				}
			}
			if (st_bRet[0]&&st_bRet[1])
			{
					st_bRedo--;
					if (st_bRedo)
						st_dwProcState--;
					else
						st_dwProcState=SENSOR_PAUSE;
			}
			SetFillProcWinFlag();
			break;
		//--纤芯对齐 
		case SENSOR_ALIGN_H3A:
			//TOP
			//--left
			bMode=FIBER_LEFT_TOP;
			SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchLeftFiberBottomEdge(pWin,bMode);
			SearchLeftFiberCore(pWin,bMode);
			//BOTTOM
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			//--right
			bMode=FIBER_RIGHT_BOTTOM;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);
			SearchRightFiberCore(pWin,bMode);

			if (st_swLastPos[0]==st_swFaceY20[FIBER_LEFT_TOP] && st_swLastPos[1]==st_swFaceY20[FIBER_RIGHT_BOTTOM])
			{
				mpDebugPrint(" FIBER_LEFT_CORE %d ",st_swFaceY20[FIBER_LEFT_TOP]);
				mpDebugPrint(" FIBER_RIGHT_CORE %d ",st_swFaceY20[FIBER_RIGHT_BOTTOM]);
				MotorSetStatus(MOTOR_V_UP,MOTOR_HOLD);
				MotorSetStatus(MOTOR_V_DOWN,MOTOR_HOLD);
				for (i=0;i<FIBER_WINDOW_NUM;i++)
				{
					st_bRet[i]=0;
				}
				st_dwRedo1=POS_RETRY_TIMES;
				st_dwProcState++;
			}
			if (st_swLastPos[0]!=st_swFaceY20[FIBER_LEFT_TOP]  && st_swFaceY20[FIBER_LEFT_TOP] >0)
			{
				st_swLastPos[0]=st_swFaceY20[FIBER_LEFT_TOP];
				st_swFaceY20[FIBER_LEFT_TOP] =-1;
			}
			if (st_swLastPos[1]!=st_swFaceY20[FIBER_RIGHT_BOTTOM]  && st_swFaceY20[FIBER_RIGHT_BOTTOM] >0)
			{
				st_swLastPos[1]=st_swFaceY20[FIBER_RIGHT_BOTTOM];
				st_swFaceY20[FIBER_RIGHT_BOTTOM] =-1;
			}

			st_dwRedo1--;
			if (!st_dwRedo1)
			{
				mpDebugPrint("!!!ERROR!!! SENSOR_ALIGN_H3A ");
				st_dwProcState=SENSOR_RPOC_END;
			}
			SetFillProcWinFlag();
			break;

		case SENSOR_ALIGN_H3B:
			//TOP
			//--right
			if (!st_bRet[0] && !(st_bMotorStaus&MOTOR_VU_BIT))
			{
				bMode=FIBER_RIGHT_TOP;
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				SearchRightFiberCore(pWin,bMode);
				if (st_swFaceY20[bMode] && MoveVMotorToSpecPosition(bMode,st_swFaceY20[bMode]>>1,st_swFaceY20[FIBER_LEFT_TOP]>>1)==PASS)
				{
					mpDebugPrint("SENSOR_ALIGN_H3B1 OK");
					st_bRet[0]=1;
				}
			}

			if (!st_bRet[1] && !(st_bMotorStaus&MOTOR_VD_BIT))
			{
				//BOTTOM
				pWin=(ST_IMGWIN *)&SensorInWin[1];
				//--left
				bMode=FIBER_LEFT_BOTTOM;
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
				SearchLeftFiberBottomEdge(pWin,bMode);
				SearchLeftFiberCore(pWin,bMode);

				if (st_swFaceY20[bMode]>0 && MoveVMotorToSpecPosition(bMode,st_swFaceY20[bMode]>>1,st_swFaceY20[FIBER_RIGHT_BOTTOM]>>1)==PASS)
				{
					mpDebugPrint("SENSOR_ALIGN_H3B2 OK");
					st_bRet[1]=1;
				}
			}
			if (st_bRet[0]&&st_bRet[1])
			{
					st_bRedo--;
					if (st_bRedo)
						st_dwProcState--;
					else
						st_dwProcState=SENSOR_PAUSE;
			}
			SetFillProcWinFlag();
			break;

		case SENSOR_PAUSE:
			st_dwProcState++;
			//st_dwProcState=SENSOR_RPOC_END;
			EventSet(UI_EVENT, EVENT_PROC_DATA);
			//Weld_StartPause();
			break;

		case SENSOR_DISCHARGE2:
			#if 0
			if (0)//(st_wDischargeTimeSet!=2)
			{
				st_wDischargeTimeSet=2;
				Discharge(st_wDischargeTimeSet,0);
				Ui_TimerProcAdd(10000, TimerToNextState);
			}
			else
			#endif
			{
				st_dwProcState++;
				EventSet(UI_EVENT, EVENT_PROC_DATA);
			}
			break;

		case SENSOR_DISCHARGE3:
			if (st_wDischargeTimeSet!=3)
			{
				#if 0
				#if TEST_PLANE||ALIGN_DEMO_MODE
				DriveMotor(01,1,st_wMotoStep[MAX_ADJ_NUM-2],4);
				#else
				DriveMotor(01,1,1500,4); // 1600 
				#endif
				st_wDischargeTimeSet=3;
				Discharge(st_wDischargeTimeSet,0);
				//xpgDelay(10);
				//DriveMotor(01,1,10,10);
				Ui_TimerProcAdd(12000, TimerToNextState);
				#elif 1
				st_wDischargeTimeSet=3;
				DischargeAndDriveMotor(MOTOR_H_LEFT,MOTOR_FF,0,st_wMotoStep[MAX_ADJ_NUM-2],4,2200,st_wMotoStep[MAX_ADJ_NUM-1]);
				#else
				SearchLeftFiberFaceAndTopEdge(pWin,FIBER_LEFT_TOP,1,0);
				SearchRightFiberFaceAndTopEdge(pWin,FIBER_RIGHT_TOP,1,0);
				if (st_swFaceX[FIBER_LEFT_TOP]>0 && st_swFaceX[FIBER_RIGHT_TOP]>0)
				{
					swRet=st_swFaceX[FIBER_RIGHT_TOP]-st_swFaceX[FIBER_LEFT_TOP];
					if (swRet<st_wMotoStep[MAX_ADJ_NUM-3] || swRet>st_wMotoStep[MAX_ADJ_NUM-2]*2)
					{
						mpDebugPrint("!!!ERROR!!! SENSOR_DISCHARGE3  %d",swRet);
						st_dwProcState=SENSOR_RPOC_END;
					}
					else
					{
						WORD wTotalPulse=(swRet-st_wMotoStep[MAX_ADJ_NUM-3])*st_wMotorSpeed[MOTOR_H_LEFT]+st_wMotoStep[MAX_ADJ_NUM-2];
						st_wDischargeTimeSet=3;
						mpDebugPrint("SENSOR_DISCHARGE3  Pixel %d,pulse %d/%d ",swRet,wTotalPulse-st_wMotoStep[MAX_ADJ_NUM-2],wTotalPulse);
						DischargeAndDriveMotor(MOTOR_H_LEFT,MOTOR_FF,wTotalPulse-st_wMotoStep[MAX_ADJ_NUM-2],wTotalPulse,MOTO_TEST_SPEED,1750,st_wMotoStep[MAX_ADJ_NUM-1]);
						Ui_TimerProcAdd(12000, TimerToNextState);
					}
				}
				#endif
				if (g_psSetupMenu->wElectrodeRemainTimes)
					g_psSetupMenu->wElectrodeRemainTimes--;
				if (g_psSetupMenu->wElectrodeRemainTimes<5)
					g_dwMachineWarningFlag|=WARNING_ELECTRODE_LESS;
				g_psSetupMenu->dwWorkTotalTimes++;
				WriteSetupChg();
			}
			break;

		case SENSOR_GET_LOSS:
			mpDebugPrint("SENSOR_GET_LOSS");
			memset((BYTE *)&st_CurWeldRecordData,0,sizeof (st_CurWeldRecordData));
			st_CurWeldRecordData.bResult=1;
			st_dwProcState++;
			EventSet(UI_EVENT, EVENT_PROC_DATA);
			break;


		case SENSOR_RPOC_END:
			mpDebugPrint("SENSOR_RPOC_END");
			TimerToReleaseAllHold();
			TspiSendSimpleInfo0xAF(0x03);
			st_dwProcState=SENSOR_IDLE;
			g_dwProcWinFlag|=WIN0_CAPTURE_FLAG;
			break;

		case SENSOR_IDLE:
			#if 0
				bMode=FIBER_LEFT_BOTTOM;
				pWin=(ST_IMGWIN *)&SensorInWin[1];
			
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
				xpgUpdatePageOsd();
				//TimerCheckToFillReferWin(10);
				//Idu_OsdErase();
				//ScanFiberUptoDownDebug(pWin,300,0,pWin->wHeight,g_psSetupMenu->bBackGroundLevel[1]>>BG_BRIGHT_SHIFT);
				#endif
			break;

		default:
			break;
	}

//	xpgUpdatePageOsd();
#endif


}
#else
void Proc_Weld_State()
{
#if TEST_TWO_LED
	ST_OSDWIN * psWin=Idu_GetOsdWin();
	BYTE bString[64];
#endif
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet,swSrcPos,swTrgPos;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下
	static BYTE st_bRedo;

#if TSPI_ENBALE
	TSPI_Receive_Check();
#endif
#if SENSOR_WIN_NUM
	if (pbSensorWinBuffer==NULL)
		return;
	//MP_DEBUG("st_dwProcState   %d ",st_dwProcState);
	if (st_dwProcState & BIT30)
		return;

	switch (st_dwProcState)
	{
		case SENSOR_PROC_INIT:
			Idu_OsdErase();
			g_bDisplayMode=0x80;
			st_wDischargeTimeSet=0;
			ResetFiberPara();
			TspiSendSimpleInfo0xAF(0x02);
			st_dwProcState++;
			TimerToFillProcWin(10);
			break;
			
		case SENSOR_FACE_POS1A:
			mpDebugPrint("SENSOR_FACE_POS1A");
			bMode=FIBER_LEFT_TOP;
			pWin=(ST_IMGWIN *)&SensorInWin[0];
			swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
			//if (swRet==PASS)
				swRet=MoveHMotorToSpecPosition(bMode,st_swFaceX[bMode],g_psSetupMenu->wElectrodePos[0]-40);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_UP);
			}
			else
			{
				TimerToFillReferWin(10,FILL_WIN_DOWN);
				g_bDisplayMode=0x81;
				st_dwProcState++;
				//st_dwProcState=SENSOR_IDLE;
				//st_wDischargeTimeSet=1;
				//Discharge(st_wDischargeTimeSet,0);
			}
			break;

		case SENSOR_FACE_POS1B:
			bMode=FIBER_LEFT_BOTTOM;
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
			//mpDebugPrint("POS1B %d",st_swFaceX[bMode]);
			//if (swRet==PASS)
				swRet=MoveHMotorToSpecPosition(bMode,st_swFaceX[bMode],pWin->wWidth-g_psSetupMenu->wElectrodePos[0]-40);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_DOWN);
			}
			else
			{
				mpDebugPrint("SENSOR_FACE_POS1B OK");
				st_dwProcState=SENSOR_DISCHARGE1;//SENSOR_GET_ANGLE;//SENSOR_DISCHARGE1;
				g_bDisplayMode=0x80;
				TimerToFillReferWin(10,FILL_WIN_UP);
			}
			break;
		case SENSOR_DISCHARGE1:
			mpDebugPrint("SENSOR_DISCHARGE1");
			st_wDischargeTimeSet=1;
			Discharge(st_wDischargeTimeSet,0);
			Ui_TimerProcAdd(10000, TimerToNextState);
			break;

		case SENSOR_AUTO_FOCUS:
#if 1
			//st_bFillWinFlag=FILL_WIN_UP|FILL_WIN_DOWN;
			st_dwProcState++;
			EventSet(UI_EVENT, EVENT_PROC_DATA);
#else
			if (g_swAutoFocusState==AF_OFF)
			{
					g_swAutoFocusState=AF_INIT;
					Proc_AutoFocus();
			}
			else if (g_swAutoFocusState==AF_FINISH)
			{
					g_swAutoFocusState=AF_OFF;
					st_dwProcState++;
					st_dwProcState=SENSOR_IDLE;
			}
#endif
			break;


		case SENSOR_GET_ANGLE:
			mpDebugPrint("SENSOR_GET_ANGLE");
			st_dwProcState++;
			g_bDisplayMode=0x80;
			TimerToFillReferWin(10,FILL_WIN_UP);
			//st_dwProcState=SENSOR_IDLE;
			break;

		case SENSOR_FACE_POS2A: // TOP
			bMode=FIBER_LEFT_TOP;
			pWin=(ST_IMGWIN *)&SensorInWin[0];
			SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
			#if TEST_PLANE||ALIGN_DEMO_MODE
			swTrgPos=g_psSetupMenu->wElectrodePos[0]-st_wMotoStep[MAX_ADJ_NUM-3];
			#else
			swTrgPos=g_psSetupMenu->wElectrodePos[0]-40;
			#endif
			swRet=MoveHMotorToSpecPosition(bMode,st_swFaceX[bMode],swTrgPos); //34
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_UP);
			}
			else
			{
				st_dwProcState++;
				g_bDisplayMode=0x81;
				TimerToFillReferWin(0,FILL_WIN_DOWN);
				mpDebugPrint("--SENSOR_FACE_POS2A OK! %d/%d",st_swFaceX[bMode],swTrgPos);
			}
			break;

		case SENSOR_FACE_POS2B://BOTTOM
			bMode=FIBER_LEFT_BOTTOM;
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,0);
			swRet=MoveHMotorToSpecPosition(bMode,st_swFaceX[bMode],pWin->wWidth-g_psSetupMenu->wElectrodePos[0]);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_DOWN);
			}
			else
			{
					mpDebugPrint("--SENSOR_FACE_POS2B OK %d/%d",st_swFaceX[bMode],pWin->wWidth-g_psSetupMenu->wElectrodePos[0]);
					MotorSetStatus(MOTOR_V_UP,MOTOR_HOLD);
					if (g_psSetupMenu->bRongJieZhiLiang==1)
						st_dwProcState=SENSOR_ALIGN_H2A;
					else if (g_psSetupMenu->bRongJieZhiLiang==2)
						st_dwProcState=SENSOR_ALIGN_H3A;
					else
						st_dwProcState=SENSOR_ALIGN_H1A;
					st_bRedo=ALIGN_HORIZONTAL_REDO_TIMES;
					g_bDisplayMode=0x80;
					TimerToFillReferWin(10,FILL_WIN_UP);
			//st_dwProcState=SENSOR_IDLE;
			}
			break;

		case SENSOR_ALIGN_H1A:
			//TOP
			//--left
			bMode=FIBER_LEFT_TOP;
			if (st_swFaceX[bMode]==-1 || st_swFaceY1[bMode]==-1)
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
			if (st_swFaceY3[bMode]==-1)
				SearchLeftFiberBottomEdge(pWin,bMode);
			//--right
			bMode=FIBER_RIGHT_TOP;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);

			mpDebugPrint(" MOTOR_LEFT %d - %d",st_swFaceY1[bMode-1],st_swFaceY3[bMode-1]);
			mpDebugPrint(" MOTOR_RIGHT %d - %d",st_swFaceY1[bMode],st_swFaceY3[bMode]);
			if ((st_swFaceY1[bMode-1] <0  && st_swFaceY3[bMode-1]<0 ) ||(st_swFaceY1[bMode]<0 && st_swFaceY3[bMode]<0))
			{
				TimerToFillReferWin(0,FILL_WIN_UP);
			}
			else
			{
				if (st_swFaceY3[bMode-1]<0)
					swTrgPos=(st_swFaceY1[bMode-1]+pWin->wHeight)>>1;
				else
					swTrgPos=(st_swFaceY1[bMode-1]+st_swFaceY3[bMode-1])>>1;
				if (st_swFaceY3[bMode]<=0)
					swSrcPos=(st_swFaceY1[bMode]+pWin->wHeight)>>1;
				else
					swSrcPos=(st_swFaceY1[bMode]+st_swFaceY3[bMode])>>1;
				swRet=MoveVMotorToSpecPosition(bMode,swSrcPos,swTrgPos);
				if (swRet!=PASS)
				{
					TimerToFillReferWin(0,FILL_WIN_UP);
				}
				else
				{
					//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
					mpDebugPrint("SENSOR_ALIGN_H1A OK  %d - %d",swSrcPos,swTrgPos);
					g_bDisplayMode=0x81;
					st_dwProcState++;
					MotorSetStatus(MOTOR_V_DOWN,MOTOR_HOLD);
					TimerToFillReferWin(10,FILL_WIN_DOWN);
				}
			}
			break;

		case SENSOR_ALIGN_H1B:
			//BOTTOM
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			//--left
			bMode=FIBER_LEFT_BOTTOM;
			if (st_swFaceY1[bMode]<0||st_swFaceX[bMode]<0)
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
			if (st_swFaceY3[bMode]<0)
				SearchLeftFiberBottomEdge(pWin,bMode);
			//--bottom
			bMode=FIBER_RIGHT_BOTTOM;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);

			swTrgPos=(st_swFaceY1[bMode-1]+st_swFaceY3[bMode-1])>>1;
			swSrcPos=(st_swFaceY1[bMode]+st_swFaceY3[bMode])>>1;
			mpDebugPrint(" LEFT  UP%d - DOWN%d",st_swFaceY1[bMode-1],st_swFaceY3[bMode-1]);
			mpDebugPrint(" RIGHT %d - %d",st_swFaceY1[bMode],st_swFaceY3[bMode]);
			swRet=MoveVMotorToSpecPosition(bMode,swSrcPos,swTrgPos);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_DOWN);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H1B OK %d - %d",swSrcPos,swTrgPos);
				g_bDisplayMode=0x80;
				st_bRedo--;
				if (st_bRedo)
				{
					st_dwProcState=SENSOR_ALIGN_H1A;
					TimerToFillReferWin(10,FILL_WIN_UP);
				}
				else
				{
					st_dwProcState=SENSOR_PAUSE;
					EventSet(UI_EVENT, EVENT_PROC_DATA);
				}
			}
			break;

		case SENSOR_ALIGN_H2A:
			//TOP
			//--left
			bMode=FIBER_LEFT_TOP;
			if (st_swFaceX[bMode]==-1 || st_swFaceY1[bMode]==-1)
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
			if (st_swFaceY3[bMode]==-1)
				SearchLeftFiberBottomEdge(pWin,bMode);
			if (st_swFaceY2[bMode]==-1)
				SearchLeftFiberCenter(pWin,bMode);
			//--right
			bMode=FIBER_RIGHT_TOP;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);
			SearchRightFiberCenter(pWin,bMode);

			mpDebugPrint(" CENTER A %d -> %d",st_swFaceY2[bMode],st_swFaceY2[bMode-1]);
			swRet=MoveVMotorToSpecPosition(bMode,st_swFaceY2[bMode],st_swFaceY2[bMode-1]);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_UP);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H2A OK  %d - %d",swSrcPos,swTrgPos);
				g_bDisplayMode=0x81;
				st_dwProcState++;
				MotorSetStatus(MOTOR_V_DOWN,MOTOR_HOLD);
				TimerToFillReferWin(10,FILL_WIN_DOWN);
			}
			break;

		case SENSOR_ALIGN_H2B:
			//BOTTOM
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			//--left
			bMode=FIBER_LEFT_BOTTOM;
			if (st_swFaceY1[bMode]<0||st_swFaceX[bMode]<0)
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
			if (st_swFaceY3[bMode]<0)
				SearchLeftFiberBottomEdge(pWin,bMode);
			if (st_swFaceY2[bMode]<0)
					SearchLeftFiberCenter(pWin,bMode);
			//--bottom
			bMode=FIBER_RIGHT_BOTTOM;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);
			SearchRightFiberCenter(pWin,bMode);

			mpDebugPrint(" CENTER B %d -> %d",st_swFaceY2[bMode],st_swFaceY2[bMode-1]);
			swRet=MoveVMotorToSpecPosition(bMode,st_swFaceY2[bMode],st_swFaceY2[bMode-1]);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_DOWN);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H2B OK %d - %d",swSrcPos,swTrgPos);
				g_bDisplayMode=0x80;
				st_bRedo--;
				if (st_bRedo)
				{
					st_dwProcState=SENSOR_ALIGN_H2A;
					TimerToFillReferWin(10,FILL_WIN_UP);
				}
				else
				{
					st_dwProcState=SENSOR_PAUSE;
					EventSet(UI_EVENT, EVENT_PROC_DATA);
				}
			}
			break;

		case SENSOR_ALIGN_H3A:
			//TOP
			//--left
			bMode=FIBER_LEFT_TOP;
			if (st_swFaceX[bMode]==-1 || st_swFaceY1[bMode]==-1)
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
			if (st_swFaceY3[bMode]==-1)
				SearchLeftFiberBottomEdge(pWin,bMode);
			if (st_swFaceY2[bMode]==-1)
				SearchLeftFiberCenter(pWin,bMode);
			if (st_swFaceY20[bMode]==-1)
				SearchLeftFiberCore(pWin,bMode);
			//--right
			bMode=FIBER_RIGHT_TOP;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);
			SearchRightFiberCenter(pWin,bMode);
			swRet=MoveVMotorToSpecPosition(bMode,st_swFaceY20[bMode]>>1,st_swFaceY20[bMode-1]>>1);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_UP);
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H3A OK");
				st_dwProcState++;
				g_bDisplayMode=0x81;
				TimerToFillReferWin(10,FILL_WIN_DOWN);
			}
			break;

		case SENSOR_ALIGN_H3B:
			//BOTTOM
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			//--left
			bMode=FIBER_LEFT_BOTTOM;
			if (st_swFaceY1[bMode]<0||st_swFaceX[bMode]<0)
				SearchLeftFiberFaceAndTopEdge(pWin,bMode,0,1);
			if (st_swFaceY3[bMode]<0)
				SearchLeftFiberBottomEdge(pWin,bMode);
			if (st_swFaceY2[bMode]<0)
					SearchLeftFiberCenter(pWin,bMode);
			if (st_swFaceY20[bMode]<0)
					SearchLeftFiberCore(pWin,bMode);
			//--bottom
			bMode=FIBER_RIGHT_BOTTOM;
			SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
			SearchRightFiberBottomEdge(pWin,bMode);
			SearchRightFiberCenter(pWin,bMode);
			SearchRightFiberCore(pWin,bMode);

			swRet=MoveVMotorToSpecPosition(bMode,st_swFaceY20[bMode]>>1,st_swFaceY20[bMode-1]>>1);
			if (swRet!=PASS)
			{
				TimerToFillReferWin(0,FILL_WIN_DOWN);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H3B OK");
				st_bRedo--;
				if (st_bRedo)
				{
					st_dwProcState=SENSOR_ALIGN_H3A;
					TimerToFillReferWin(10,FILL_WIN_UP);
				}
				else
				{
					st_dwProcState=SENSOR_PAUSE;
					EventSet(UI_EVENT, EVENT_PROC_DATA);
				}
				//if (g_psSetupMenu->bPingXianFangShi<4)
				//	g_bDisplayMode=0x80|g_psSetupMenu->bPingXianFangShi;
				#if 0
				OsdLineInit();
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				bMode=FIBER_LEFT_TOP;
				SearchLeftFiberCore(pWin,bMode);
				bMode=FIBER_RIGHT_TOP;
				SearchRightFiberCore(pWin,bMode);

				pWin=(ST_IMGWIN *)&SensorInWin[1];
				bMode=FIBER_LEFT_BOTTOM;
				SearchLeftFiberCore(pWin,bMode);
				bMode=FIBER_RIGHT_BOTTOM;
				SearchRightFiberCore(pWin,bMode);
				#endif
				//xpgDelay(1000);
			}
			break;

		case SENSOR_PAUSE:
			st_dwProcState=SENSOR_DISCHARGE2;
			Weld_StartPause();
			//st_dwProcState=SENSOR_IDLE;
			//EventSet(UI_EVENT, EVENT_PROC_DATA);
			break;

		case SENSOR_DISCHARGE2:
			#if 0
			if (0)//(st_wDischargeTimeSet!=2)
			{
				st_wDischargeTimeSet=2;
				Discharge(st_wDischargeTimeSet,0);
				Ui_TimerProcAdd(10000, TimerToNextState);
			}
			else
			#endif
			{
				st_dwProcState++;
				EventSet(UI_EVENT, EVENT_PROC_DATA);
			}
			break;

		case SENSOR_DISCHARGE3:
			mpDebugPrint("SENSOR_DISCHARGE3");
			if (st_wDischargeTimeSet!=3)
			{
				#if TEST_PLANE||ALIGN_DEMO_MODE
				DriveMotor(01,1,st_wMotoStep[MAX_ADJ_NUM-2],4);
				#else
				DriveMotor(01,1,1500,4); // 1600 
				#endif
				st_wDischargeTimeSet=3;
				Discharge(st_wDischargeTimeSet,0);
				//xpgDelay(10);
				//DriveMotor(01,1,10,10);
				Ui_TimerProcAdd(12000, TimerToNextState);
			}
			else
			{
				//st_dwProcState++;
				//EventSet(UI_EVENT, EVENT_PROC_DATA);
			}
			break;

		case SENSOR_GET_LOSS:
			mpDebugPrint("SENSOR_GET_LOSS");
			st_dwProcState++;
			break;

		case SENSOR_RPOC_END:
			mpDebugPrint("SENSOR_RPOC_END");
			TspiSendSimpleInfo0xAF(0x03);
			st_dwProcState=SENSOR_IDLE;
			TimerToReleaseAllHold();
			break;

		case SENSOR_IDLE:
#if TEST_PLANE||ALIGN_DEMO_MODE
		if (g_bDisplayMode < 0x02)
		{
			if (g_bDisplayMode==1)
				pWin=(ST_IMGWIN *)&SensorInWin[1];
#if 1
	//pWin=(ST_IMGWIN *)Idu_GetCurrWin();
	bMode=FIBER_LEFT_TOP;
	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
	SearchLeftFiberBottomEdge(pWin,bMode);
	//SearchLeftFiberCenter(pWin,bMode);
	//SearchLeftFiberCore(pWin,bMode);
#endif
#if 1
	bMode=FIBER_RIGHT_TOP;
	SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
	SearchRightFiberBottomEdge(pWin,bMode);
	//SearchRightFiberCenter(pWin,bMode);
	//SearchRightFiberCore(pWin,bMode);
#endif
	//pWin=(ST_IMGWIN *)&SensorInWin[1];
#if 0
	bMode=FIBER_LEFT_BOTTOM;
	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1,1);
	SearchLeftFiberBottomEdge(pWin,bMode);
	SearchLeftFiberCenter(pWin,bMode);
	SearchLeftFiberCore(pWin,bMode);
#endif
#if 0
	bMode=FIBER_RIGHT_BOTTOM;
	SearchRightFiberFaceAndTopEdge(pWin,bMode,1,1);
	SearchRightFiberBottomEdge(pWin,bMode);
	SearchRightFiberCenter(pWin,bMode);
	SearchRightFiberCore(pWin,bMode);
#endif
#if 1
	if (g_bDisplayMode==0)
		st_bFillWinFlag=FILL_WIN_UP;
	else if (g_bDisplayMode==1)
		st_bFillWinFlag=FILL_WIN_DOWN;
#endif
	TimerToFillProcWin(10);

	}
#endif
#if TEST_TWO_LED
	for (bMode=0;bMode<2;bMode++)
	{
		swRet=GetWinBrightness((ST_IMGWIN *)&SensorInWin[bMode],8,2);
		if (st_dwBrightness[bMode]!=swRet)
		{
			st_dwBrightness[bMode]=swRet;
			sprintf(bString, "%d", st_dwBrightness[bMode]);
			Idu_OsdPaintArea(pWin->wWidth/2,pWin->wHeight*bMode/2+20, 80, 40, 0);
			Idu_OSDPrint(psWin,bString, pWin->wWidth/2,pWin->wHeight*bMode/2+20, OSD_COLOR_RED);
		}
	}
	TimerToFillProcWin(100);
#endif

//	mpCopyWinAreaSameSize(pWin,Idu_GetCurrWin(),0, 0, 0, 0,pWin->wWidth, pWin->wHeight);
	//TimerToFillProcWin(10);
		default:
			break;
	}

	xpgUpdatePageOsd();
#endif

	//if (st_dwProcState)
	{
		//EventSet(UI_EVENT, EVENT_PROC_DATA);
	}

}
#endif

BYTE SensorActionBusy(void)
{
	if (st_dwProcState || st_dwGetCenterState)
		return 1;
	return 0;
}
void Proc_SensorData_State()
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下

	//mpDebugPrint("P %p %d %p",g_psSetupMenu->bBackGroundLevel[1],st_bInProcWin,st_bNeedFillProcWin);
	//if (!g_psSetupMenu->bBackGroundLevel[1])
	//	return ;
	if (st_bInProcWin)// || st_bNeedFillProcWin
	{
		EventSet(UI_EVENT, EVENT_PROC_DATA);
		return;
	}
	st_bInProcWin=1;

	if (g_dwProcWinFlag&WIN0_CAPTURE_FLAG)
	{
		Weld_CaptureFile((ST_IMGWIN *)&SensorInWin[0]);
		//Weld_CaptureFile((ST_IMGWIN *)Idu_GetCurrWin());
		g_dwProcWinFlag&=~WIN0_CAPTURE_FLAG;

	}

#if ALIGN_FAST_MODE
	Proc_Weld_State_Fast();
#else
	Proc_Weld_State();
#endif
	Proc_GetFiberLowPoint();

	st_bInProcWin=0;

}

//------TSPI通讯发送数据,还有一部分在SETUP.C

//power
void SendCmdPowerOff()
{
	BYTE bTxData[8];

	MP_DEBUG("SendCmdPowerOff");
	bTxData[0]=0x98;
	bTxData[1]=3+1;
	bTxData[2]=0;
	TSPI_PacketSend(bTxData,0);
}

//--RTC
void TspiSendRTC(void)
{
	BYTE bTxData[10];
	ST_SYSTEM_TIME curTime;

	bTxData[0]=0x99;
	bTxData[1]=3+6;
	SystemTimeGet(&curTime);
	bTxData[2]=curTime.u16Year-2000;
	bTxData[3]=curTime.u08Month;
	bTxData[4]=curTime.u08Day;
	bTxData[5]=curTime.u08Hour;
	bTxData[6]=curTime.u08Minute;
	bTxData[7]=curTime.u08Second;

	TSPI_PacketSend(bTxData,1);
}


//--查询指令
SWORD TspiSendCmdPolling0xA4(BYTE bCmd)
{
	BYTE bTxData[8];

	bTxData[0]=0xa4;
	bTxData[1]=3+1;
	bTxData[2]=bCmd;
	return TSPI_PacketSend(bTxData,1);
}

// 马达复位
void ResetMotor(void)
{
	TspiSendCmdPolling0xA4(0x02);
}

//0xa6 uitouchcontrol.c

//--电击棒激活请求
//0xa7 uitouchcontrol.c

//--电击棒信息
void TspiSendElectrodeInfo(void)
{
	BYTE bTxData[18];

	bTxData[0]=0xa8;
	bTxData[1]=3+15;
	memcpy(&bTxData[2],&g_psSetupMenu->bElectrodeInfo[0],11);
	TSPI_PacketSend(bTxData,1);
}

//--开关指令
//--bDevice : 0x01 整锁机 0x02 云端模式 0x03关机 0x04一键锁定（云端）0x05 本地光功率计状态 0x06 云端光功率计状态 0x07 智能背光状态
//--bMode :BYTE bDevice
SWORD  TspiSendOnOffOrder0xA9(BYTE bDevice,BYTE bMode)
{
	BYTE i,bTxData[8];

	bTxData[0]=0xA9;
	bTxData[1]=3+2;
	bTxData[2] =bDevice;
	bTxData[3] =bMode;
	return TSPI_PacketSend(bTxData,1);
}

//--锁定信息
void TspiSendLockInfo(void)
{
	BYTE bTxData[8];

	bTxData[0]=0xaa;
	bTxData[1]=3+4;
	if (g_psSetupMenu->bMachineLockMode&BIT1)
	{
		bTxData[2]=2;
		bTxData[3]=g_psSetupMenu->wLockedTimes&0x00ff;
		bTxData[4]=g_psSetupMenu->wLockedTimes>>8;
		bTxData[5]=0;
	}
	else
	{
		bTxData[2]=1;
		bTxData[3]=g_psSetupMenu->wLockDateYear-2000;
		bTxData[4]=g_psSetupMenu->bLockDateMonth;
		bTxData[5]=g_psSetupMenu->bLockDateDay;
	}
	TSPI_PacketSend(bTxData,1);
}


SWORD  TspiSendSimpleInfo0xAF(BYTE bInfo)
{
	BYTE i,bTxData[8];

	bTxData[0]=0xAF;
	bTxData[1]=3+1;
	bTxData[2] =bInfo;
	return TSPI_PacketSend(bTxData,1);
}

//------TSPI通讯接收处理
//DWORD g_dwTestTime;
void TSPI_DataProc(void)
{
	BYTE i,bChecksum,index,*pbBuffer;
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey,dwTmpData,dwLen,*pdwBuffer;
	BYTE bStirng[64];
	
	if (!st_dwTspiRxIndex)
		return;
	if (pbTspiRxBuffer[1])
	{
		
		sprintf(bStirng," %02x %02x%02x%02x%02x",pbTspiRxBuffer[0],pbTspiRxBuffer[1],pbTspiRxBuffer[2],pbTspiRxBuffer[3],pbTspiRxBuffer[4]);
		Idu_OsdErase();
		Idu_OSDPrint(Idu_GetOsdWin(),bStirng, 16, 120, OSD_COLOR_GREEN);//  90
	//	for (i=0;i<st_dwTspiRxIndex;i++)
	//		mpDebugPrintN(" %02x ",pbTspiRxBuffer[i]);
	//	mpDebugPrint("");
	}
	if (st_dwTspiRxIndex<3)
		return;
	if (pbTspiRxBuffer[1])
		dwLen=pbTspiRxBuffer[1];
	else
		dwLen=((DWORD)pbTspiRxBuffer[2]<<24)|((DWORD)pbTspiRxBuffer[3]<<16)|((DWORD)pbTspiRxBuffer[4]<<8)|pbTspiRxBuffer[5];
	if (dwLen!=st_dwTspiRxIndex)
		return;
	bChecksum=0;
	for (i=0;i<st_dwTspiRxIndex-1;i++)
	{
		bChecksum+=pbTspiRxBuffer[i];
	}
	if (bChecksum!=pbTspiRxBuffer[st_dwTspiRxIndex-1])
	{
		//mpDebugPrint(" checksum error really %p data %p ",bChecksum,pbTspiRxBuffer[st_dwTspiRxIndex-1]);
		mpDebugPrint("!!!");
		return;
	}
	//接收成功
	if (!(pbTspiRxBuffer[0]==0xbf && pbTspiRxBuffer[2]<0x03))
		TspiSendSimpleInfo0xAF(0x01);
	if (g_psUnsaveParam->bStandby &&(pbTspiRxBuffer[0]==0xb1))
	{
		TurnOnBackLight();
		TimerToBacklightOff(5000);
	}
	if (g_bKeyExcept&&(pbTspiRxBuffer[0]==0xb1)&&(pbTspiRxBuffer[2]!=g_bKeyExcept))
	{
		MP_DEBUG("Skip key %d",pbTspiRxBuffer[2]);
		return;
	}
	switch (pbTspiRxBuffer[0])
	{
		case 0xb1:
			UiCb_CheckSleepAndShut();
			if (CheckAndTurnOnBackLight())
				return;
			if (pbTspiRxBuffer[3]==1) //4 短按
			{
				switch (pbTspiRxBuffer[2])
				{
					case 1://refresh
						#if SHOW_CENTER
						//DriveMotor(01,1,1000,8);//RIGHT_DOWN_FIBER  DOWN
						st_swAFStep=2;
						ProcAFmotorAction();
						//PrintWinData();
						#elif TEST_PLANE||ALIGN_DEMO_MODE
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (st_bAdjustMotoStep)
							{
								if (st_bToolMotoIndex<5)
									st_wMotoStep[st_bToolMotoIndex-1]-=20;
								else if (st_bToolMotoIndex==8)
									st_wMotoStep[st_bToolMotoIndex-1]-=5;
								else if (st_bToolMotoIndex==6||st_bToolMotoIndex==9)
									st_wMotoStep[st_bToolMotoIndex-1]-=50;
								else
									st_wMotoStep[st_bToolMotoIndex-1]--;
								PutAdjOsdString();
							}
							else
							{
								DriveMotor(st_bToolMotoIndex,0,st_wMotoStep[st_bToolMotoIndex-1],8);
								TimerCheckToFillReferWin(10);
							//DriveMotor(02,0,5000,10);
							//g_dwTestTime=GetSysTime();
							//mpDebugPrint("Start FR:%d",g_dwTestTime);
							}
						}
						#else
						//st_bFillWinFlag=FILL_WIN_UP;
						//TimerToFillProcWin(10);
						//g_dwProcWinFlag|=WIN0_CAPTURE_FLAG;
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (g_bDisplayMode<3)
								g_bDisplayMode++;
							else
								g_bDisplayMode=0;
							g_bDisplayMode |= 0x80;
						}
						ResetMotor();
						#endif
						break;
					case 2: // start/pause
						#if SHOW_CENTER
						//DriveMotor(02,0,1000,8);//RIGHT_DOWN_FIBER  UP
						st_swAFStep=-2;
						ProcAFmotorAction();
						#elif TEST_PLANE||ALIGN_DEMO_MODE
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (st_bAdjustMotoStep)
							{
								if (st_bToolMotoIndex<5)
									st_wMotoStep[st_bToolMotoIndex-1]+=20;
								else if (st_bToolMotoIndex==8)
									st_wMotoStep[st_bToolMotoIndex-1]+=5;
								else if (st_bToolMotoIndex==6||st_bToolMotoIndex==9)
									st_wMotoStep[st_bToolMotoIndex-1]+=50;
								else
									st_wMotoStep[st_bToolMotoIndex-1]++;
								PutAdjOsdString();
							}
							else
							{
								DriveMotor(st_bToolMotoIndex,1,st_wMotoStep[st_bToolMotoIndex-1],8);
								TimerCheckToFillReferWin(10);
								//DriveMotor(02,1,5000,10);
								//g_dwTestTime=GetSysTime();
								//mpDebugPrint("Start FF:%d",g_dwTestTime);
							}
						}
						#else
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
							Weld_StartPause();
						#endif
						break;
					case 3: // main
						#if SHOW_CENTER
						index=st_bAFMotoIndex-1;
						if (st_bMotorHold&(1<<index))
						{
							MotorSetStatus(index,MOTOR_NO_HOLD);
							st_bMotorHold &= ~(1<<index);
						}
						if (st_bAFMotoIndex==5)
							st_bAFMotoIndex=6;
						else
							st_bAFMotoIndex=5;
						#elif TEST_PLANE||ALIGN_DEMO_MODE
						if (g_psUnsaveParam->bHollCover)
						{
							if (dwHashKey == xpgHash("Main") )
							{
								WeldModeSet(0);
								xpgCb_EnterCamcoderPreview();
							}
							else
							{
								Idu_OsdErase();
								xpgSearchtoPageWithAction("Main");
								xpgUpdateStage();
							}
						}
						else
						{
							 if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
							 {
							 	#if 1
								
								index=st_bToolMotoIndex-1;
								if (st_bMotorHold&(1<<index))
								{
									MotorSetStatus(index,MOTOR_NO_HOLD);
									st_bMotorHold &= ~(1<<index);
								}
								st_bToolMotoIndex++;
								if (st_bToolMotoIndex>MAX_ADJ_NUM)
									st_bToolMotoIndex=1;
								//mpDebugPrint("st_bToolMotoIndex=%d",st_bToolMotoIndex);
								PutAdjOsdString();
								#else
								MotorSetStatus(02,MOTOR_STOP);
								#endif
							 }
							 else
							 {
								WeldModeSet(0);
								xpgCb_EnterCamcoderPreview();
							 }
						}
						#else
						if (dwHashKey == xpgHash("Main") )
						{
							WeldModeSet(0);
							xpgCb_EnterCamcoderPreview();
						}
						else
						{
							Idu_OsdErase();
							xpgSearchtoPageWithAction("Main");
							xpgUpdateStage();
						}
						#endif
						break;

					case 4:// X/Y
						#if  SHOW_CENTER
						index=st_bAFMotoIndex-1;
						if (st_bMotorHold&(1<<index))
						{
							MotorSetStatus(index,MOTOR_NO_HOLD);
							st_bMotorHold &= ~(1<<index);
							mpDebugPrint(" motor %d hold off",index);
						}
						else
						{
							MotorSetStatus(index,MOTOR_HOLD);
							st_bMotorHold|=(1<<index);
							mpDebugPrint(" motor %d hold on",index);
						}
						#elif TEST_PLANE||ALIGN_DEMO_MODE
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (st_bAdjustMotoStep)
							{
								#if 1
								if (st_dwGetCenterState)
									st_dwGetCenterState=0;
								else
									AutoGetFiberLowPoint();
								#else
								if (st_bSensorMode)
									st_bSensorMode=0;
								else
									st_bSensorMode=1;
								#if 0
								IPU *ipu = (IPU *) IPU_BASE;
								DWORD dwIpudata=ipu->Ipu_reg_F0;

								ipu->Ipu_reg_F0 |= (BIT6|BIT7);
								Drive_Sensor_NT99140();
								mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());
								Idu_ChgWin(Idu_GetNextWin());
								ipu->Ipu_reg_F0 =dwIpudata;
								#else
								xpgCb_StopAllSensorWork();
								mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());
								Idu_ChgWin(Idu_GetNextWin());
    							Camcorder_PreviewStart(CAMCORDER_RESOLUTION_800x480);
    							#endif
								#endif
							}
							else
							{
								#if 1
								Weld_StartPause();
								#else
								index=st_bToolMotoIndex-1;
								if (st_bMotorHold&(1<<index))
								{
									MotorSetStatus(index,MOTOR_NO_HOLD);
									st_bMotorHold &= ~(1<<index);
									mpDebugPrint(" motor %d hold off",index+1);
								}
								else
								{
									MotorSetStatus(index,MOTOR_HOLD);
									st_bMotorHold|=(1<<index);
									mpDebugPrint(" motor %d hold on",index+1);
								}
								#endif
							}

						}
						#else
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (g_bDisplayMode<3)
								g_bDisplayMode++;
							else
								g_bDisplayMode=0;
							g_psSetupMenu->bPingXianFangShi=g_bDisplayMode;
							WriteSetupChg();
							g_bDisplayMode |= 0x80;
						}
						#endif
						break;

					case 5:
						#if 0
						//DriveMotor(01,1,200,8);//RIGHT_UP_FIBER DOWN
						MoveAFMotor(6,-100);
						/*
						TimerToFillProcWin(10);
						g_swAutoFocusState=AF_INIT;
						Proc_AutoFocus();
						*/
						#elif SHOW_CENTER||TEST_PLANE||ALIGN_DEMO_MODE||TEST_TWO_LED
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (st_bAdjustMotoStep)
							{
								//MotorSetStatus(st_bToolMotoIndex,MOTOR_HOLD);
								//DriveMotor(01,1,760,8); // 60
								//st_bWaitMotoStop=1;
								if (st_bAutoDischarge)
								{
									st_bAutoDischarge=0;
									Ui_TimerProcRemove(AutoDischarge);
								}
								else
								{
									st_bAutoDischarge=20;
									st_wDischargeTimeSet=1000;
									AutoDischarge();
								}
							}
							else
							{
								if (g_bDisplayMode<3)
									g_bDisplayMode++;
								else
									g_bDisplayMode=0;
								g_psSetupMenu->bPingXianFangShi=g_bDisplayMode;
								WriteSetupChg();
								g_bDisplayMode |= 0x80;
								TimerCheckToFillReferWin(1000);
							}
						}
						#elif 0
						if (g_bDisplayUseIpw2)
							g_bDisplayUseIpw2=0;
						else
							g_bDisplayUseIpw2=1;

						IPU *ipu = (IPU *) IPU_BASE;
						ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);
						ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);	
						mpClearWin(Idu_GetCurrWin());
						#else
						#endif
						break;
					case 6:
						#if SHOW_CENTER||TEST_PLANE||ALIGN_DEMO_MODE
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							//st_dwProcState=SENSOR_IDLE;
							//ResetMotor();
							//st_wDischargeTimeSet=1;
							//Discharge(st_wDischargeTimeSet,0);
							if (st_bAdjustMotoStep)
							{
								st_bAdjustMotoStep=0;
							}
							else
							{
								st_bAdjustMotoStep=1;
							}
							PutAdjOsdString();
						}
						#else

						#endif
						break;


					default:
						break;
				}
    			//AddAutoEnterPreview();
			}
			else 	if (pbTspiRxBuffer[3]==3)  //4 长按
			{
				switch (pbTspiRxBuffer[2])
				{
					case 1:
					memset((BYTE *)&st_CurWeldRecordData,0,sizeof (st_CurWeldRecordData));
					g_dwProcWinFlag|=WIN0_CAPTURE_FLAG;
					SetFillProcWinFlag();
					break;
					
					case 2:
					//DriveMotor(01,1,65000,10);
					break;
					
					case 3:
					#if 1
					Idu_OsdErase();
					xpgSearchtoPageWithAction("Main");
					xpgUpdateStage();
					#else
					ResetMotor();
					g_psSetupMenu->bBackGroundLevel[0]=0;
					g_psSetupMenu->bBackGroundLevel[1]=0;
					Ui_TimerProcAdd(1000, ScanBackgroundLevel);
					#endif
					break;
					
					case 4:
					break;
					
					case 5:
					Discharge(1,0);
					break;
					
					case 6:
					break;

					default:
						break;
				}

			}

			break;
//马达状态
		case 0xb2:
			//mpDebugPrint("-Stop- %d",SystemGetElapsedTime(g_dwTestTime));
			st_bMotorStaus=pbTspiRxBuffer[2];
			mpDebugPrint("-Stop- %p",st_bMotorStaus);
			//TimerCheckToFillReferWin(500);
			#if TEST_PLANE
			if (st_bWaitMotoStop && !(st_bMotorStaus&MOTOR_HL_BIT))
			{
				st_bWaitMotoStop=0;
				st_wDischargeTimeSet=3;
				Discharge(st_wDischargeTimeSet,0);
				DriveMotor(01,1,850,8); // 60
			}
			#endif
			if (g_swAutoFocusState!=AF_OFF && !(st_bMotorStaus&(1<<(AF_MOTO_INDEX-1))))
			{
				Ui_TimerProcRemove(SetFillProcWinFlag);
				Ui_TimerProcRemove(Proc_AutoFocus);
				TimerToFillProcWin(10);
				Proc_AutoFocus();
			}
			break;
//检测状态
		case 0xb3:
			switch (pbTspiRxBuffer[2]>>4)
			{
				case 1://4 霍尔盖子状态
					if (pbTspiRxBuffer[2]&BIT0)// 盖子打开
					{
						UiCb_CheckSleepAndShut();
						CheckAndTurnOnBackLight();
						g_psUnsaveParam->bHollCover=1;
						WeldStopAllAction();
						// 马达复位
						ResetMotor();
						ResetFiberPara();
					}
					else
					{
						g_psUnsaveParam->bHollCover=0;
						//WeldModeSet(0);
						//xpgCb_EnterCamcoderPreview();
					}
					break;

				case 2: //4 加热盖子状态
					g_psUnsaveParam->bHeatCover=(pbTspiRxBuffer[2]&BIT0);
					break;

				case 3: //4 云端OPM在线状态
					g_psUnsaveParam->bCloudOPMonline=(pbTspiRxBuffer[2]&BIT0);
					break;

				default:
					break;
			}
			
			break;
		//放电状态
		case 0xb4:
			if (pbTspiRxBuffer[2]==0x01)
			{
				st_wDiachargeRunTime=0;
				if (st_dwProcState==SENSOR_DISCHARGE1 || st_dwProcState==SENSOR_DISCHARGE2 ||st_dwProcState==SENSOR_DISCHARGE3)
				{
					Ui_TimerProcRemove(TimerToNextState);
					st_dwProcState++;//=SENSOR_FACE_POS2A;
					EventSet(UI_EVENT, EVENT_PROC_DATA);
				}
				if (st_bAutoDischarge)
				{
					st_bAutoDischarge--;
					Ui_TimerProcAdd(1000, AutoDischarge);
				}
				else
				{
					if (st_dwGetCenterState==GET_CENTER_INIT)
					{
						st_dwGetCenterState++;
						TimerToFillReferWin(1000,FILL_WIN_UP);
					}
					else if (st_dwGetCenterState==GET_CENTER_LOW_POINT)
					{
						TimerToFillReferWin(1000,FILL_WIN_UP);
					}
				}
			}
			else if (pbTspiRxBuffer[2]==0x02) 
			{
				//断电
				xpgStopAllAction();
				st_dwProcState=0;
				st_dwGetCenterState=0;
				TurnOffBackLight();
			}
			else if (pbTspiRxBuffer[2]==0x03) 
			{
				//马达复位错误
			}
			else if (pbTspiRxBuffer[2]==0x04) 
			{
				//左马达复位完成
				st_dwHMotoPos[0]=0;
			}
			else if (pbTspiRxBuffer[2]==0x06) 
			{
				//右马达复位完成
				st_dwHMotoPos[1]=0;
			}
			break;
		//数据回复
		case 0xb5:
			//复位完成
			/*
			if (pbTspiRxBuffer[2]==0x01)
			{
			}
			*/
			if (pbTspiRxBuffer[1]!=8)
				break;
			if (pbTspiRxBuffer[2]==0x02)
			{
				//MCU可接收的最长指令长度
				memcpy((BYTE *)&st_dwTspiTxMaxLen,&pbTspiRxBuffer[3],4);
			}
			break;
		//AF镜头
		case 0xb6:
			if (pbTspiRxBuffer[2]&BIT2) //  1->AFL镜头未超出行程
			{
			}
			if (pbTspiRxBuffer[2]&BIT3)//  1->AFL镜头未超出行程
			{
			}
			break;
		//熔接记录查询
		case 0xb7:
			if ((pbTspiRxBuffer[2]==1||pbTspiRxBuffer[2]==2) && pbTspiRxBuffer[3]>0)
			{
				if (pbTspiRxBuffer[2]==1)//4  查询熔接记录
				{
					BYTE pbData[30];
					DWORD dwIndex=(((DWORD)pbTspiRxBuffer[6])<<24)|(((DWORD)pbTspiRxBuffer[5])<<16)|(((DWORD)pbTspiRxBuffer[4])<<8)|pbTspiRxBuffer[3];
					STRECORD * pr;

					if (dwIndex<GetRecordTotal())
					{
						memset(pbData,0,30);
						//--head
						pbData[0]=0xa6;
						pbData[1]=3+23;
						pbData[2]=pbTspiRxBuffer[3];
						pbData[3]=pbTspiRxBuffer[4];
						pbData[4]=pbTspiRxBuffer[5];
						pbData[5]=pbTspiRxBuffer[6];
						
						pr = GetRecord(dwIndex);
						if (pr)
						{
							//FileListSetCurIndex(pr->dwFileIndex);
						//--record
							memcpy(&pbData[6],&pr->bYear,sizeof(STRECORD)-4);
							//Weld_ReadFileWeldInfo(NULL,&pbData[6]);
						}
						

						TSPI_PacketSend(pbData,0);
					}
				}
				else if (pbTspiRxBuffer[2]==2)//4  查询熔接图片  
				{
					Weld_SendFileInit(((WORD)pbTspiRxBuffer[4]<<8)|pbTspiRxBuffer[3]);
				}
			}
			break;

		//温度压力电量环境亮度
		case 0xb8:
			if (dwLen>=10)
			{
				g_dwMachineWarningFlag &= 0xfffffc00;
				if (pbTspiRxBuffer[2]!=g_psUnsaveParam->bTemperatureInhome[0])
				{
					index=pbTspiRxBuffer[2]&BIT7;
					i=pbTspiRxBuffer[2]&0x7f;
					if (index>0)
						g_dwMachineWarningFlag|=WARNING_INSIDE_TEMP_LOW;
					else if (!index && i>60)
						g_dwMachineWarningFlag|=WARNING_INSIDE_TEMP_HIGH;
				}
				if (pbTspiRxBuffer[4]!=g_psUnsaveParam->bTemperatureOuthome[0])
				{
					index=pbTspiRxBuffer[4]&BIT7;
					i=pbTspiRxBuffer[4]&0x7f;
					if (index>0 && i>20)
						g_dwMachineWarningFlag|=WARNING_OUTSIDE_TEMP_LOW;
					else if (!index && i>60)
						g_dwMachineWarningFlag|=WARNING_OUTSIDE_TEMP_HIGH;
				}
				if (pbTspiRxBuffer[6]!=g_psUnsaveParam->bHumidity)
				{
					if (pbTspiRxBuffer[6]>60)
						g_dwMachineWarningFlag|=WARNING_HUMIDITY;
				}
				if ((((WORD)pbTspiRxBuffer[8]<<8) |pbTspiRxBuffer[7]) != g_psUnsaveParam->wPressure)
				{
					g_psUnsaveParam->wPressure=((WORD)pbTspiRxBuffer[8]<<8) |pbTspiRxBuffer[7];
					if (g_psUnsaveParam->wPressure<50 || g_psUnsaveParam->wPressure>200)
						g_dwMachineWarningFlag|=WARNING_ATMOS_PRESSURE;
				}
				//memset(&g_psUnsaveParam->bTemperatureInhome[0],0,8);
				memcpy(&g_psUnsaveParam->bTemperatureInhome[0],&pbTspiRxBuffer[2],dwLen-3-2);
				//MP_DEBUG(" %02x %02x %02x %02x ",g_psUnsaveParam->bTemperatureInhome[0],g_psUnsaveParam->bTemperatureInhome[1],g_psUnsaveParam->bTemperatureOuthome[0],g_psUnsaveParam->bTemperatureOuthome[1]);// wPressure 6900 0069 
				MP_DEBUG("wPressure %02x %04x ",g_psUnsaveParam->bHumidity,g_psUnsaveParam->wPressure);// wPressure 6900 0069 
				if ( g_pstXpgMovie->m_pstCurPage->m_dwHashKey == xpgHash("Main"))
					xpgUpdateStage();
			}
			break;

		//查询命令
		case 0xb9:
			switch (pbTspiRxBuffer[2])
			{
				case 0x01:
					TspiSendLockInfo();
					break;
				case 0x02:
					SetupSendTouchVoice();
					break;
				case 0x03:
					TspiSendElectrodeInfo();
					break;
				case 0x04:
					TspiSendRTC();
					break;
			}
			break;

		//远程锁定
		case 0xba:
			switch (pbTspiRxBuffer[2])
			{
				case 0x00:
					g_psSetupMenu->bMachineLockMode=0;
					break;
				case 0x01:
					g_psSetupMenu->bMachineLockMode=0xff;
					break;
				case 0x02:
					g_psSetupMenu->bMachineLockMode|=BIT0;
					g_psSetupMenu->wLockDateYear=2000+pbTspiRxBuffer[3];
					g_psSetupMenu->bLockDateMonth=pbTspiRxBuffer[4];
					g_psSetupMenu->bLockDateDay=pbTspiRxBuffer[5];
					break;
				case 0x03:
					g_psSetupMenu->bMachineLockMode|=BIT1;
					g_psSetupMenu->wLockedTimes=((WORD)pbTspiRxBuffer[4]<<8)|pbTspiRxBuffer[3];
					break;

				default:
					return;
			}
			WriteSetupChg();
			break;

		//激活电击棒信息
		case 0xbb:
			if (dwLen<=14)
			{
				if (pbTspiRxBuffer[2]==0xfe) //4    特殊指令
				{
				memcpy(&g_psSetupMenu->bElectrodeInfo[0],&pbTspiRxBuffer[2],dwLen-3);
				}
				else if (g_pstXpgMovie->m_pstCurPage->m_dwHashKey== xpgHash(DIALOG_PAGE_NAME) && xpgGetCurrDialogTypeId() == Dialog_Note_Electrode_Enable_Process)
				{
					//--0->表示连不上服务器 1->序列号不存在 2->此序列号已激活
					if (pbTspiRxBuffer[0]==0)
					{
						//转到人工输入
						Dialog_Electrode_Enable_Process_OnClose();
					}
					else if (pbTspiRxBuffer[0]<10)
					{
							dwDialogValue.dwValueData=pbTspiRxBuffer[0];
							strDialogTitle = getstr(Str_Note);
							dialogOnClose = exitDialog;
							uiCb_DisableKeyInput(0xff);
							popupDialog(Dialog_Note_ElectrodeEnable_FAIL, ((STXPGPAGE *)xpgMovieSearchPage("Main"))->m_wIndex,Idu_GetCacheWin());
							Ui_TimerProcAdd(3000, uiCb_EnableKeyInput);
							Ui_TimerProcAdd(3000, exitDialog);
					}
					else
					{
						WORD wCheckCode=((WORD)pbTspiRxBuffer[14])<<8|pbTspiRxBuffer[13];
						
						strDialogTitle = getstr(Str_Note);
						dialogOnClose = exitDialog;
						uiCb_DisableKeyInput(0xff);
						if (ProduceElectrodeCheckCode()==wCheckCode)
						{
							popupDialog(Dialog_Note_ElectrodeEnable_PASS, ((STXPGPAGE *)xpgMovieSearchPage("Main"))->m_wIndex,Idu_GetCacheWin());
							Ui_TimerProcAdd(3000, uiCb_CheckElectrodePos);
						}
						else
						{
							dwDialogValue.dwValueData=0;
							popupDialog(Dialog_Note_ElectrodeEnable_FAIL, ((STXPGPAGE *)xpgMovieSearchPage("Main"))->m_wIndex,Idu_GetCacheWin());
							Ui_TimerProcAdd(3000, uiCb_EnableKeyInput);
							Ui_TimerProcAdd(3000, exitDialog);
						}
						xpgUpdateStage();
					}
				}
			}
			break;

		//设备信息数据传输
		case 0xbc:
			if (dwLen>=9)
			{
				memcpy(&g_psSetupMenu->bMADarry[0],&pbTspiRxBuffer[2],6);
				WriteSetupChg();
				//CheckAndWriteFile( DriveGet(SYS_DRV_ID),QRCODE_FILE_NAME,QRCODE_FILE_EXT,&pbTspiRxBuffer[8],dwLen-9);//NAND
			}
			break;

		//环境亮度
		case 0xbd:
			break;

		//加热数据
		case 0xbe:
			g_psSetupMenu->bPreHotEnable=(pbTspiRxBuffer[2]&BIT0);
			g_psSetupMenu->bHotUpMode=(pbTspiRxBuffer[2]&BIT1)>1;
			g_psSetupMenu->bReSuGuanSheZhi=pbTspiRxBuffer[2]>>4;
			g_psSetupMenu->wJiaReWenDu=pbTspiRxBuffer[3];
			g_psSetupMenu->wJiaReShiJian=pbTspiRxBuffer[4];
			WriteSetupChg();
			break;
		//简单状态发送
		case 0xbf:
			switch (pbTspiRxBuffer[2])
			{
				case 0x00://0x00->接收错误 0x01->接收成功
					if (st_bTspiTxRetry)
					{
						st_bTspiTxRetry--;
						#if UART_TO_MCU
						Uart_TimerToResend();
						#else
						TSPI_TimerToResend();
						Ui_TimerProcRemove(TSPI_TimerToResend);
						#endif
					}
				break;
				case 0x01:
					if (st_bTspiTxRetry)
					{
						st_bTspiTxRetry=0;
						//Ui_TimerProcRemove(TSPI_TimerToResend);
					}
				break;
				case 0x02:
					g_psUnsaveParam->bStandby=1;
				break;
				case 0x03:
					g_psUnsaveParam->bStandby=0;
				break;
			}
			break;

		//云端/本地OPM数据
		case 0xc2:
			if (dwLen<18)
				break;
			pbBuffer=OpmGetbuffer(pbTspiRxBuffer[2]);
			if (pbBuffer)
			{
				pdwBuffer=(DWORD *)pbBuffer;
				if (*pdwBuffer<OPM_SEGMEN_NUM)
				{
					*pdwBuffer=*pdwBuffer+1;
					pbBuffer+=(*pdwBuffer-1)*OPM_SEGMEN_LEN+1;
					memcpy(pbBuffer,&pbTspiRxBuffer[3],14);
				}
			}
			break;

		//云端时间
		case 0xc4:
			g_psSetupMenu->sdwRtcOffset=0;

			ST_SYSTEM_TIME new_time;
			new_time.u16Year=2000+pbTspiRxBuffer[2];
			new_time.u08Month=pbTspiRxBuffer[3];
			new_time.u08Day=pbTspiRxBuffer[4];
			new_time.u08Hour=pbTspiRxBuffer[5];
			new_time.u08Minute=pbTspiRxBuffer[6];
			new_time.u08Second=pbTspiRxBuffer[7];
			SystemTimeSet(&new_time);
			WriteSetupChg();
			break;

		//开机新密码
		case 0xc5:
			dwTmpData=(pbTspiRxBuffer[3]<<8)|pbTspiRxBuffer[2];
			g_psSetupMenu->srtOpenPassword[0]= '0' + (dwTmpData%10000)/1000;
			g_psSetupMenu->srtOpenPassword[1]= '0' + (dwTmpData%1000)/100;
			g_psSetupMenu->srtOpenPassword[2]= '0' + (dwTmpData%100)/10;
			g_psSetupMenu->srtOpenPassword[3]= '0' + (dwTmpData%10);
			g_psSetupMenu->srtOpenPassword[4]= 0;
			break;

		//红光笔数据
		case 0xc6:
			g_psUnsaveParam->bRedPenEnable=pbTspiRxBuffer[2];
			g_psUnsaveParam->bRedPenHZ=pbTspiRxBuffer[3];
			if (pbTspiRxBuffer[4])
			{
				g_psUnsaveParam->bRedPenTimerEnable=1;
				g_psUnsaveParam->bRedPenTime=pbTspiRxBuffer[4];
			}
			else
				g_psUnsaveParam->bRedPenTimerEnable=0;
			if (dwHashKey == xpgHash("RedLight"))
				xpgUpdateStage();
			uiCb_VFLHzFlash();
			break;

		//充电状态与电量
		case 0xc7:
			g_psUnsaveParam->bChargeStatus=pbTspiRxBuffer[2];
			if (g_psUnsaveParam->bChargeStatus)
			{
				UiCb_CheckSleepAndShut();
				CheckAndTurnOnBackLight();
			}
			else
			{
				if (g_psUnsaveParam->bBatteryQuantity !=pbTspiRxBuffer[3])
				{
					g_psUnsaveParam->bBatteryQuantity=pbTspiRxBuffer[3];
					if (g_psUnsaveParam->bBatteryQuantity>100)
						g_psUnsaveParam->bBatteryQuantity=100;
					if ((g_dwMachineWarningFlag & WARNING_BATTERY_LOW)&&(g_psUnsaveParam->bBatteryQuantity>20)&&(dwHashKey == xpgHash(DIALOG_PAGE_NAME)))
					{
						g_dwMachineWarningFlag &= ~WARNING_BATTERY_LOW;
						if (dialogOnClose == DialogCb_ExitLowPowerPopWarning)
							DialogCb_ExitLowPowerPopWarning();
					}
					else if (!(g_dwMachineWarningFlag & WARNING_BATTERY_LOW) && g_psUnsaveParam->bBatteryQuantity<=20)
					{
							g_dwMachineWarningFlag |= WARNING_BATTERY_LOW;
							if (!SensorActionBusy() && (dwHashKey != xpgHash(DIALOG_PAGE_NAME)))
								xpgUpdateStage();
					}
				}
				if (g_psUnsaveParam->bStandby)
					SendCmdPowerOff();
			}
			break;

		//网络信号强度
		case 0xc8:
			if (g_psUnsaveParam->bNetSignal !=pbTspiRxBuffer[2])
			{
				//mpDebugPrint("signal %d",pbTspiRxBuffer[2]);
				g_psUnsaveParam->bNetSignal=pbTspiRxBuffer[2];
				if (!(g_dwMachineWarningFlag & WARNING_NETSIGNAL_LOW) && g_psUnsaveParam->bNetSignal<2)
				{
					g_dwMachineWarningFlag |= WARNING_NETSIGNAL_LOW;
					if (!SensorActionBusy() && (dwHashKey != xpgHash(DIALOG_PAGE_NAME)))
						xpgUpdateStage();
				}
				else if ((g_dwMachineWarningFlag & WARNING_NETSIGNAL_LOW) && g_psUnsaveParam->bNetSignal>=2)
				{
					g_dwMachineWarningFlag &= ~WARNING_NETSIGNAL_LOW;
					if (dialogOnClose == DialogCb_ExitLowNetsignalPopWarning)
						DialogCb_ExitLowNetsignalPopWarning();
				}
			}
			break;

		default:
			break;

	}
}


#if SHOW_CENTER
static BYTE st_bShowin=0;
void ShowOSdline()
{
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
	WORD wX,wY,wW,wH;

	wW=100; //half
	wH=pDstWin->wHeight/6;//half
	//中间的+字
	OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,1,pDstWin->wWidth/2-1-wW,pDstWin->wHeight/2-2,wW<<1,2,OSD_COLOR_RED);
	OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,2,pDstWin->wWidth/2-1-1,pDstWin->wHeight/2-1-wH,2,wH<<1,OSD_COLOR_RED);
	//水平两条线
	OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,3,0,pDstWin->wHeight/2-1-wH,pDstWin->wWidth,2,OSD_COLOR_BLUE);
	OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4,0,pDstWin->wHeight/2-1+wH,pDstWin->wWidth,2,OSD_COLOR_BLUE);

	//垂直两条线
	wW=2;
	wH=pDstWin->wHeight;
	wX=pDstWin->wWidth/2-50;
	wY=0;
	OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,5,wX,wY,wW,wH,OSD_COLOR_RED);
	wX=pDstWin->wWidth/2+50;
	OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,6,wX,wY,wW,wH,OSD_COLOR_RED);

	xpgUpdatePageOsd();

}
#endif

#if IPW_FAST_MODE
extern BYTE st_bCurChannel;
static BYTE st_bIpwSkipFrame=0;
void ProcIpwIsr(DWORD dwRegValue)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();

	//stop IPW1 IPW2
	//ipu->Ipu_reg_F0 |= 0x000000c0; 
	if (g_bDisplayMode&0xf0)
	{
		Sensor_DisplayMode_Set();
		#if OSD_LINE_NUM
		OsdLineInit();
		#endif
		Idu_OsdErase();
		goto IPW_ISR_END;
	}

//    mpDebugPrintN("  %08x %08x  ", ipu->Ipu_reg_10A,ipu->Ipu_reg_10B);
	if ((ipu->Ipu_reg_10A>>16)!=pDstWin->wHeight)
	{
		mpDebugPrintN("  ! ");
		goto IPW_ISR_END;
	}
	
	//UartOutText(" - ");
	Sensor_Channel_Swap();
	if (st_bNeedFillProcWin==FILL_WIN_END)
	{
			//Ui_TimerProcAdd(10, TimerToProcWin);
			EventSet(UI_EVENT, EVENT_PROC_DATA);
			if (st_dwProcState==SENSOR_FACE_POS1A)
				st_bNeedFillProcWin=FILL_WIN_UP|FILL_WIN_DOWN|FILL_WIN_END;
			else
				st_bNeedFillProcWin=0;
	}
	if (st_bCurChannel)
	{
		if ((st_bNeedFillProcWin&FILL_WIN_DOWN))//(dwRegValue&BIT7)
		{
			//UartOutText(" 1 ");
			st_bNeedFillProcWin &=0xfd;//FILL_WIN_DOWN
			ipu->Ipu_reg_A3 = ((DWORD) SensorInWin[1].pdwStart| 0xA0000000);	
			//ipu->Ipu_reg_A3 = ((DWORD) ((ST_IMGWIN *)Idu_GetCurrWin())->pdwStart| 0xA0000000);	
			//ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset;
		}
		#if 1
		if (g_bDisplayMode==1)
			ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset[1];
		else // (g_bDisplayMode==0)
			ipu->Ipu_reg_F2 = ((DWORD) ((ST_IMGWIN *)Idu_GetNextWin())->pdwStart| 0xA0000000);
		#else
		if (g_bDisplayMode==1)
			ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset;
		else if (g_bDisplayMode==2)
			ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset+pDstWin->dwOffset*SensorWindow_Height;
		else if (g_bDisplayMode==3)
			ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset+(SensorWindow_Width<<1);
		else // (g_bDisplayMode==0)
			ipu->Ipu_reg_F2 = ((DWORD) ((ST_IMGWIN *)Idu_GetNextWin())->pdwStart| 0xA0000000);
		#endif
	}
	else
	{
		if ((st_bNeedFillProcWin&FILL_WIN_UP)) // up win//(dwRegValue&BIT7)
		{
			//UartOutText(" 0 ");
			st_bNeedFillProcWin &=0xfe;//FILL_WIN_UP
			ipu->Ipu_reg_A3 = ((DWORD) SensorInWin[0].pdwStart| 0xA0000000);	
			//ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
			//ipu->Ipu_reg_F2 = ((DWORD) ((ST_IMGWIN *)Idu_GetNextWin())->pdwStart| 0xA0000000);
		}
		#if 1
		if (g_bDisplayMode==1)
			ipu->Ipu_reg_F2 = ((DWORD) ((ST_IMGWIN *)Idu_GetNextWin())->pdwStart| 0xA0000000);
		else
			ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset[0];
		#endif
	}


	IPW_ISR_END:
	if (st_bNeedFillProcWin)
	{
			ipu->Ipu_reg_F0 &= 0xffffff3f; //open IPW1 IPW2
			//ipu->Ipu_reg_F0 &= 0xffffffbf; //open IPW1
	}
	else
	{
			ipu->Ipu_reg_F0 &= 0xffffff7f; //open IPW2 ~BIT7
	}

}
#else
static BYTE st_bFrameIndex=0;
void DisplaySensorOnCurrWin( BYTE bIpw2)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
	register DWORD *pIpwAddr;

	if (bIpw2)
		pIpwAddr=(DWORD *)&ipu->Ipu_reg_F2;
	else
		pIpwAddr=(DWORD *)&ipu->Ipu_reg_A3;
/*
	if (st_bNeedFillProcWin)
	{
		*pIpwAddr= ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);
		return;
	}
*/
	if (g_bDisplayMode&0xf0)
	{
		Sensor_DisplayMode_Set();
		//ResetFiberPara();
#if OSD_LINE_NUM
		OsdLineInit();
#endif
		Idu_OsdErase();
		#if TEST_PLANE||ALIGN_DEMO_MODE||TEST_TWO_LED
		if (!st_dwProcState)
			TimerCheckToFillReferWin(10);
		#endif
	}
//return;
#if 1
//    mpDebugPrintN("  %08x %08x  ", ipu->Ipu_reg_10A,ipu->Ipu_reg_10B);
	if ((ipu->Ipu_reg_10A>>16)!=pDstWin->wHeight)
		return;
	if (g_bDisplayMode==2)
	{
		#if defined(SENSOR_TYPE_NT99140)
			if (Sensor_CurChannel_Get())
			{
				Sensor_Channel_Set(0);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
			}
			else
			{
				Sensor_Channel_Set(1);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+pDstWin->dwOffset*SensorWindow_Height;	
			}
		//	IODelay(10);

		#else
			if (Sensor_CurChannel_Get())
			{
				if (st_bFrameIndex)
				{
					//mpDebugPrintN("A %08x ", ipu->Ipu_reg_10A);
					Sensor_Channel_Set(0);
					st_bFrameIndex=0;
					pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
					*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
				}
				else
				{
					//mpDebugPrintN("B %08x ", ipu->Ipu_reg_10A);
					st_bFrameIndex++;
					*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+pDstWin->dwOffset*SensorWindow_Height;	
				}
			}
			else
			{
				if (st_bFrameIndex==1)
				{
					//mpDebugPrintN("C %08x ", ipu->Ipu_reg_10A);
						Sensor_Channel_Set(1);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+pDstWin->dwOffset*SensorWindow_Height;	
				}
				else
				{
					//mpDebugPrintN("D %08x ", ipu->Ipu_reg_10A);
					st_bFrameIndex++;
					*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
				}
			}
	#endif
	}
	else if (g_bDisplayMode==3)
	{
		#if defined(SENSOR_TYPE_NT99140)
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);
				
			}
			else
			{
				Sensor_Channel_Set(0);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
			}
		#else
			if (Sensor_CurChannel_Get())
			{
				if (st_bFrameIndex==1)
				{
						Sensor_Channel_Set(0);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
				}
				else
				{
					//mpDebugPrintN("B %08x ", ipu->Ipu_reg_10A);
					st_bFrameIndex++;
					*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);	
				}
			}
			else
			{
				if (st_bFrameIndex==1)
				{
					//mpDebugPrintN("C %08x ", ipu->Ipu_reg_10A);
						Sensor_Channel_Set(1);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);	
				}
				else
				{
					//mpDebugPrintN("D %08x ", ipu->Ipu_reg_10A);
					st_bFrameIndex++;
					*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
				}
			}
		#endif
	}
	else //if (st_bDispOff && (ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
	{
		*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset[0];
		//st_bDispOff=0;
	}

#else
	switch (g_bDisplayMode)
	{
		case 2:
			DisplayWinStartAddr=pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+DisplayWinStartAddr+pDstWin->dwOffset*SensorWindow_Height;	
				
			}
			else
			{
				Sensor_Channel_Set(0);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+DisplayWinStartAddr;	
			}
			break;

		case 3:
			DisplayWinStartAddr=pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+DisplayWinStartAddr+(SensorWindow_Width<<1);
				
			}
			else
			{
				Sensor_Channel_Set(0);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+DisplayWinStartAddr;	
			}
			break;

			default:
				break;
	}
#endif
}

void DisplaySensorIpw2Data(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();

	if (g_bDisplayMode&0xf0)
	{
		Sensor_DisplayMode_Set();
#if OSD_LINE_NUM
		OsdLineInit();
#endif
		Idu_OsdErase();
	}

//    mpDebugPrintN("  %08x %08x  ", ipu->Ipu_reg_10A,ipu->Ipu_reg_10B);
	if ((ipu->Ipu_reg_10A>>16)!=pDstWin->wHeight)
	{
		mpDebugPrintN("  ! ");
		return;
	}
    //mpDebugPrintN("  %d  ", ipu->Ipu_reg_10A>>16);
    //mpDebugPrintN("   %08x %08x   ", ipu->Ipu_reg_10A,ipu->Ipu_reg_10B);
	if (g_bDisplayMode==2)
	{
		if (st_bNeedFillProcWin)
		{
			Idu_ChgWin(Idu_GetNextWin());
			pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
			if (Sensor_CurChannel_Get())
			{
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+pDstWin->dwOffset*SensorWindow_Height;	
			}
			else
			{
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
			}
		}
		else
		{
			if (Sensor_CurChannel_Get())
			{
				Idu_ChgWin(Idu_GetNextWin());
				pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
				Sensor_Channel_Set(0);
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
			}
			else
			{
				Sensor_Channel_Set(1);
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+pDstWin->dwOffset*SensorWindow_Height;	
			}
		}
	}
	else if (g_bDisplayMode==3)
	{
		if (st_bNeedFillProcWin)
		{
			Idu_ChgWin(Idu_GetNextWin());
			pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
			if (Sensor_CurChannel_Get())
			{
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);
			}
			else
			{
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
			}
		}
		else
		{
			if (Sensor_CurChannel_Get())
			{
				Idu_ChgWin(Idu_GetNextWin());
				pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
				Sensor_Channel_Set(0);
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
				
			}
			else
			{
				Sensor_Channel_Set(1);
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);
			}
		}
	}
	else if (g_bDisplayMode== Sensor_CurChannel_Get())
	{
		Idu_ChgWin(Idu_GetNextWin());
		pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
		ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+g_dwWinStartOffset[0];
	}

}

void Sensor_Channel_ChangeInCache(BYTE bChannel)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();

	Sensor_Channel_Set(bChannel);
	//--解决有时下面的WIN显示到上面的WIN了
	if (bChannel)
	{
		if (g_bDisplayMode==2)
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);
		else if (g_bDisplayMode==3)
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);
	}
	else
	{
		if (g_bDisplayMode==2)
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
		else if (g_bDisplayMode==3)
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
	}
}

void CacheSensorData( BYTE bIpw2)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)&SensorInWin[0];
	BYTE bCacheWinIndex=0xff;

	if (st_bNeedFillProcWin)
	{
		MP_DEBUG("--%s-- %p",__FUNCTION__,st_bNeedFillProcWin);

		if (st_bNeedFillProcWin&FILL_WIN_INIT)
		{
			//mpDebugPrint("cache in g_bDisplayMode=%p st_bNeedFillProcWin=%p",g_bDisplayMode,st_bNeedFillProcWin);
			st_bBackupChanel=Sensor_CurChannel_Get();
			st_bNeedFillProcWin &=~FILL_WIN_INIT;
			//st_bNeedFillProcWin |=0x20;//for stop
		}

		if (st_bNeedFillProcWin&FILL_WIN_UP_WAIT)
		{
			if ((ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
				st_bNeedFillProcWin &=~FILL_WIN_UP_WAIT;
			else
				st_bNeedFillProcWin |=FILL_WIN_UP;
		}
		else if (st_bNeedFillProcWin&FILL_WIN_DOWN_WAIT)
		{
			if ((ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
				st_bNeedFillProcWin &=~FILL_WIN_DOWN_WAIT;
			else
				st_bNeedFillProcWin |=FILL_WIN_DOWN;
		}
		
		if ((st_bNeedFillProcWin&(FILL_WIN_UP|FILL_WIN_DOWN))==(FILL_WIN_UP|FILL_WIN_DOWN))
		{
			//one sensor online
				if (Sensor_CurChannel_Get())
				{
					bCacheWinIndex=FILL_WIN_DOWN;
				}
				else
				{
					bCacheWinIndex=FILL_WIN_UP;
				}
		}
		else if (st_bNeedFillProcWin&FILL_WIN_DOWN)
		{
				bCacheWinIndex=FILL_WIN_DOWN;
		}
		else if (st_bNeedFillProcWin&FILL_WIN_UP)
		{
				bCacheWinIndex=FILL_WIN_UP;
		}
		else// if ((st_bNeedFillProcWin&0x20)==0x20)
		{
			//mpDebugPrint("cache end");
			//ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);	
			//mpCopyWinAreaSameSize((ST_IMGWIN *)&SensorInWin[0],(ST_IMGWIN *)Idu_GetCurrWin(),0, 0, 0, 0,pDstWin->wWidth, pDstWin->wHeight);
			//xpgDelay(1000);
			#if 1
			if (st_bBackupChanel<2 && st_bBackupChanel!=Sensor_CurChannel_Get())
				Sensor_Channel_ChangeInCache(st_bBackupChanel);
			#else
			if (st_bBackupChanel<2 && st_bBackupChanel!=Sensor_CurChannel_Get())
			{
				Sensor_Channel_ChangeInCache(st_bBackupChanel);
				ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);
				//st_bDispOff=1;
			}
			else
			{
				ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetCurrWin()->pdwStart)| 0xA0000000)+g_dwWinStartOffset;
			}
			ipu->Ipu_reg_A3 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);	
			#endif

			st_bBackupChanel=0xff;
			st_bNeedFillProcWin=0;
			bCacheWinIndex=0;
			//EventSet(UI_EVENT, EVENT_PROC_DATA);
			Ui_TimerProcAdd(10, TimerToProcWin);
		}
		//else if (st_bNeedFillProcWin)
		//	st_bNeedFillProcWin=0;

		if (bCacheWinIndex==FILL_WIN_UP) // up win
		{
			if (Sensor_CurChannel_Get() !=0)
				Sensor_Channel_ChangeInCache(0);
			st_bNeedFillProcWin &=~FILL_WIN_UP;
			st_bNeedFillProcWin |=FILL_WIN_UP_WAIT;
			ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
		}
		else if (bCacheWinIndex==FILL_WIN_DOWN) // down win
		{
			if (Sensor_CurChannel_Get() !=1)
				Sensor_Channel_ChangeInCache(1);
			st_bNeedFillProcWin &=~FILL_WIN_DOWN;
			st_bNeedFillProcWin |=FILL_WIN_DOWN_WAIT;
			pDstWin=(ST_IMGWIN *)&SensorInWin[1];
			ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
		}

		if (st_bNeedFillProcWin)
		{
			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
		}

	}
//	if (!bIpw2) //  IPW1 must open ,because it is IPW2`s  base
//			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1

}
void ProcIpwFrameEvent(void) //IPW2
{
	//IPU *ipu = (IPU *) IPU_BASE;
	//ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();

#if 0
		//API_Close_Write_Into_EncBuf();
		ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
		while(1)
		{
			if((ipu->Ipu_reg_100&BIT0)==0)//絋﹚Τ糶秈
				break;
		}

		DisplaySensorImage();

		ipu->Ipu_reg_100 |= BIT0;
#elif 1
		DisplaySensorIpw2Data();
		//DisplaySensorOnCurrWin(1);
		//DisplaySensorImage();
	//Ipu_ImageScaling(pSrcWin, pDstWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight, 0, 0, pDstWin->wWidth, pDstWin->wHeight, 0);
	//mpCopyWinAreaSameSize(pSrcWin,pDstWin,0, 0, 0, 0,pDstWin->wWidth, pDstWin->wHeight);

#else  //display in currwin
	if (g_bDisplayUseIpw2)
	{
		DisplaySensorOnCurrWin(1);
		//ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
	}
	else
	{
		CacheSensorData(1);
	}
#endif
#if SHOW_CENTER
	if (!st_bShowin)
	{
		st_bShowin=1;
		ShowOSdline();
	}
#endif

}

void ProcIpwBit0Event(void) // IPW 1  same as record,for cache process data
{
	//IPU *ipu = (IPU *) IPU_BASE;
	//ST_IMGWIN *pDstWin=(ST_IMGWIN *)&SensorInWin[0];
	//BYTE bCacheWinIndex=0;


#if 1
		CacheSensorData(0);
#elif 0
	if (g_bDisplayUseIpw2)
		CacheSensorData(0);
	else
		DisplaySensorOnCurrWin(0);
	/*
	#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
			ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
	#else
			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
	#endif //--------------------------
*/
#else
	if (g_bDisplayMode&0xf0)
		Sensor_DisplayMode_Set();

	switch (g_bDisplayMode)
	{
		case 2:
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset*SensorWindow_Height;	
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset*SensorWindow_Height;	
#endif
				
			}
			else
			{
				Sensor_Channel_Set(0);
				pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#endif
			}
			break;

		case 3:
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);	
#endif
				
			}
			else
			{
				Sensor_Channel_Set(0);
				pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#endif
			}
			break;

			default:
				break;
	}

	#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
			ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
	#else
			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
	#endif //--------------------------

#endif



}
#endif

#if TEST_PLANE||ALIGN_DEMO_MODE
void  PutAdjOsdString()
{
	if (st_bAdjustMotoStep)
	{
		if (st_bToolMotoIndex<=MOTO_ADJ_NUM)
			sprintf(st_bStrInfo, "%s[%d]=[%d]", st_pbAdjStr[st_bToolMotoIndex-1],st_bToolMotoIndex,st_wMotoStep[st_bToolMotoIndex-1]);
		else
			sprintf(st_bStrInfo, "%s:[%d]", st_pbAdjStr[st_bToolMotoIndex-1],st_wMotoStep[st_bToolMotoIndex-1]);
	}
	else
	{
		if (st_bToolMotoIndex<=MOTO_ADJ_NUM)
			sprintf(st_bStrInfo, "%s:%d", st_pbAdjStr[st_bToolMotoIndex-1],st_bToolMotoIndex);
		else
			sprintf(st_bStrInfo, "%s:%d", st_pbAdjStr[st_bToolMotoIndex-1],st_wMotoStep[st_bToolMotoIndex-1]);
	}
	xpgForceUpdateOsd();

}

void  PutErrorOsdString(SWORD swData0,SWORD swData1,SWORD swData2,SWORD swData3)
{
	//sprintf(st_bStrInfo, "%d-%d     0:%d  1:%d  2:%d  3:%d",st_swHlevel[0],st_swHlevel[1], st_swAngle[0],st_swAngle[1],st_swAngle[2],st_swAngle[3]);
	sprintf(st_bStrInfo, "0:%d  1:%d  2:%d  3:%d",swData0,swData1,swData2,swData3);
	xpgForceUpdateOsd();

}
#endif

void ShowOSDstring(void)
{
	ST_OSDWIN * psWin=Idu_GetOsdWin();
	ST_IMGWIN *pWin=(ST_IMGWIN *)Idu_GetNextWin();
		
#if TEST_PLANE||ALIGN_DEMO_MODE
	if (st_bStrInfo[0])
	{
		Idu_OSDPrint(psWin,st_bStrInfo, 8, 8, OSD_COLOR_RED);
	}

	if (g_bDisplayMode==0x00||g_bDisplayMode==0x01)
	{
		//--face
		if (st_bStrFace[FIBER_LEFT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrFace[FIBER_LEFT_TOP][0], pWin->wWidth/2-80, 48, OSD_COLOR_RED);
		}
		if (st_bStrFace[FIBER_RIGHT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrFace[FIBER_RIGHT_TOP][0], pWin->wWidth/2+40, 48, OSD_COLOR_RED);
		}
		//--水平度
		if (st_bStrHLevel[FIBER_LEFT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrHLevel[FIBER_LEFT_TOP][0], 8, 48, OSD_COLOR_RED);//  90
		}
		if (st_bStrHLevel[FIBER_RIGHT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrHLevel[FIBER_RIGHT_TOP][0], pWin->wWidth-200, 48, OSD_COLOR_RED);
		}
		//--center
		if (st_bStrCenter[FIBER_LEFT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrCenter[FIBER_LEFT_TOP][0], 8, (st_swFaceY3[FIBER_LEFT_TOP]+st_swFaceY1[FIBER_LEFT_TOP])>>1, OSD_COLOR_RED);
		}
		if (st_bStrCenter[FIBER_RIGHT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrCenter[FIBER_RIGHT_TOP][0], pWin->wWidth-80, (st_swFaceY3[FIBER_RIGHT_TOP]+st_swFaceY1[FIBER_RIGHT_TOP])>>1, OSD_COLOR_RED);
		}
		//--core
		if (st_bStrCore[FIBER_LEFT_TOP][0])
		{
			//Idu_OSDPrint(psWin,&st_bStrCore[FIBER_LEFT_TOP][0], 120,(st_swFaceY20[FIBER_LEFT_TOP]>>1)-10, OSD_COLOR_RED);
			Idu_OSDPrint(psWin,&st_bStrCore[FIBER_LEFT_TOP][0], 120,st_swFaceY2[FIBER_LEFT_TOP]-10, OSD_COLOR_RED);
		}
		if (st_bStrCore[FIBER_RIGHT_TOP][0])
		{
			//Idu_OSDPrint(psWin,&st_bStrCore[FIBER_RIGHT_TOP][0], pWin->wWidth/2+40, (st_swFaceY20[FIBER_RIGHT_TOP]>>1)-10, OSD_COLOR_RED);
			Idu_OSDPrint(psWin,&st_bStrCore[FIBER_RIGHT_TOP][0], pWin->wWidth/2+40, st_swFaceY2[FIBER_RIGHT_TOP]-10, OSD_COLOR_RED);
		}

	}



#endif
}

void StopAllMoto(void)
{
	DWORD i;

	//--stop moto
	for (i=0;i<FIBER_WINDOW_NUM;i++)
		MotorSetStatus(i+1,MOTOR_STOP);
}

void WeldStopAllAction(void)
{

	st_dwGetCenterState = GET_CENTER_OFF;
	st_dwProcState = SENSOR_IDLE;
	//--stop discharge
	if (st_wDiachargeRunTime)
	{
		Discharge(0,0);
	}
	st_bAutoDischarge=0;
	st_wDischargeTimeSet=0;
	StopAllMoto();
}

void WeldDataInit(void)
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)Idu_GetNextWin();
	DWORD i;

#if SHOW_CENTER
	g_bDisplayMode=0x82;
	st_dwProcState = SENSOR_IDLE;
#elif TEST_PLANE
	g_bDisplayMode=0x80;
	st_dwProcState = SENSOR_IDLE;
#else
	MP_DEBUG("displaymode %d",g_psSetupMenu->bPingXianFangShi);
	if (g_psSetupMenu->bPingXianFangShi<4)
		g_bDisplayMode=0x80|g_psSetupMenu->bPingXianFangShi;
	else
	{
		g_psSetupMenu->bPingXianFangShi=2;
		g_bDisplayMode=0x82;
		WriteSetupChg();
	}
#endif

	ResetMotorPara();

#if TEST_PLANE||ALIGN_DEMO_MODE
	for (i=0;i<FIBER_WINDOW_NUM;i++)
	{
		st_bStrFace[i][0]=0;
		st_bStrCenter[i][0]=0;
		st_bStrCore[i][0]=0;
		st_bStrHLevel[i][0]=0;
	}
	PutAdjOsdString();
#endif

//--查询MCU可接收的最长指令长度
	TspiSendCmdPolling0xA4(0x0a);

//--读取电极棒位置
	if (g_psSetupMenu->wElectrodePos[0]<pWin->wWidth/4 ||  g_psSetupMenu->wElectrodePos[0]>pWin->wWidth*3/4)
	{
		g_psSetupMenu->wElectrodePos[0]=pWin->wWidth/2;
	}
	/*
	if (g_psSetupMenu->wElectrodePos[1]>0  && g_psSetupMenu->wElectrodePos[1]<pWin->wWidth)
	{
		g_wElectrodePos[1]=g_psSetupMenu->wElectrodePos[1];
	}
	*/
	MP_DEBUG("wElectrodePos %d",g_psSetupMenu->wElectrodePos[0]);
	//WriteSetupChg();
	UiCb_CheckSleepAndShut();

	//ResetMotor();
	//g_psSetupMenu->bBackGroundLevel[0]=0;
	//g_psSetupMenu->bBackGroundLevel[1]=0;
	if (!g_psSetupMenu->bBackGroundLevel[0]||!g_psSetupMenu->bBackGroundLevel[1])
	{
		Ui_TimerProcAdd(3100, GetBackgroundLevel);
	}
	else
	{
		mpDebugPrint("BackGroundLevel 0x%02x-0x%02x",g_psSetupMenu->bBackGroundLevel[0],g_psSetupMenu->bBackGroundLevel[1]);
	}
	TimerCheckToFillReferWin(10);
}

#endif
#endif



