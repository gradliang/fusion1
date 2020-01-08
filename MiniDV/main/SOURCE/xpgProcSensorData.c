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
#include "xpgProcSensorData.h"
#include "os.h"
#include "display.h"
#include "devio.h"
#include "taskid.h"
#include "fs.h"
#include "ui.h"
#include "filebrowser.h"
#include "imageplayer.h"
#include "mpapi.h"

#include "Setup.h"
#include "filebrowser.h"
#include "Icon.h"
#include "ui_timer.h"

#include "peripheral.h"

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
WORD g_wElectrodePos[2]={356,356};  //4 电极棒位置
//SWORD g_swCenterOffset=0;  //4 电极棒位置  76

static SWORD st_swFaceX[MOTOR_NUM]={-1,-1,-1,-1};
static SWORD st_swFacexCurPos[MOTOR_NUM]={-1,-1,-1,-1},st_swLastPos[MOTOR_NUM]={-1,-1,-1,-1};
static SWORD st_swFaceY1[MOTOR_NUM]={-1,-1,-1,-1},st_swFaceY2[MOTOR_NUM]={-1,-1,-1,-1},st_swFaceY20[MOTOR_NUM]={-1,-1,-1,-1},st_swFaceY3[MOTOR_NUM]={-1,-1,-1,-1}; //4  上中下  
static BYTE st_bDirectionArry[MOTOR_NUM],st_bMotorBaseStep[4]={15,15,30,30},st_bBaseRetryArry[MOTOR_NUM],st_bMotorHold=0,st_bMotorStaus=0;// 0->STOP 1->RUN BIT0
static BYTE st_bTopVMotorUpValue=0,st_bBottomVMotorUpValue=0,st_bVmotorMoveTimes[2]={0};
static SWORD st_swLastProcStep[2]={0,0},st_swVmotorMoveValue[2][VMOTOR_CNT]; // 0->up motor  1->down motor
//--st_wDiachargeRunTime 前一次放电时间  st_wDischargeTimeSet:设置需要放电的时间或模式
static WORD st_wDiachargeRunTime=0,st_wDischargeTimeSet=0; // st_wDischargeTimeSet:0->off 
static BYTE st_bFiberBlackLevel[SENSER_TOTAL]={0xff,0xff};

static STWELDSTATUS st_WeldStatus;

#if TEST_PLANE
#define MOTO_ADJ_NUM				7
static BYTE st_bStrFace[MOTOR_NUM][POS_STR_LEN], st_bStrCenter[MOTOR_NUM][POS_STR_LEN], st_bStrCore[MOTOR_NUM][POS_STR_LEN],st_bStrHLevel[MOTOR_NUM][POS_STR_LEN], st_bStrInfo[POS_STR_LEN],st_bAdjustMotoStep=0;
static WORD st_wMotoStep[MOTO_ADJ_NUM]={50,50,50,50,10,10,2500};
static BYTE st_bWaitMotoStop=0;
#endif
#if TEST_TWO_LED
static DWORD st_dwBrightness[2]={0};
#endif
BYTE g_bDisplayUseIpw2=1; // 0->ipw1  1->ipw2

#endif


#if TSPI_ENBALE
#define  TX_BUFFER_LENTH								16


#define  RX_HEAD_FLAG1									0x55
#define  RX_HEAD_FLAG2									0x55
#define  RX_HEAD_FLAG3									0xee
#define  RX_BUF_NORMAL_LEN						32
#define  TX_BUF_NORMAL_LEN						32


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

static DWORD st_dwTspiRxBufLen=0,st_dwTspiRxIndex=0;
static BYTE *pbTspiRxBuffer=NULL,st_bTspiBusy=0,st_bTspiAbnormal=0;
static DWORD st_dwTspiTxBufLen=0,st_dwTspiTxMaxLen;  //4      st_dwTspiTxMaxLen:未收到MCU回复确认长度前BIT31为1
static BYTE *pbTspiTxBuffer=NULL,st_bTspiTxRetry=0;


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
		/*
		if (swRet==PASS)
			mpDebugPrintN("-O-");
		else
			mpDebugPrintN("-N-");
			*/
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
			pdwBuf=pbBuf;
			//--flag (4bytes):BYTE 0-7
			for (i=0;i<4;i++)
			{
				pbBuf[i]=(HEAD_EXT>>((3-i)<<3))&0xff;
			}
			for (i=0;i<4;i++)
			{
				pbBuf[4+i]=(HEAD_INFO>>((3-i)<<3))&0xff;
			}
			//--标题 (11bytes):BYTE 8-17   BYTE18=0

			//--status (5bytes): BYTE19-23
			memcpy(&pbBuf[19],(BYTE *)&st_WeldStatus,sizeof (st_WeldStatus));
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

		return swRet;
	}

    return FAIL;
}

SWORD Weld_ReadFileWeldInfo(STREAM* handle,BYTE *pbTitle,STWELDSTATUS *pWeldStatus)
{
	ST_SEARCH_INFO *pSearchInfo;
	DRIVE *pCurDrive;
	DWORD dwFileSize,dwLen,dwFlag0,dwFlag1;
	BYTE *pbBuf=NULL,bHandleNull=0;

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
	pbBuf = (WORD *) ext_mem_malloc(WELD_FILE_INFO_LENTH);
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
	memcpy(pbTitle,&pbBuf[8],11);
	memcpy(pWeldStatus,&pbBuf[19],sizeof (STWELDSTATUS));
	if (pbBuf)
		ext_mem_free(pbBuf);
	if (bHandleNull)
		FileClose(handle);
}

static BYTE *st_pbBuf=NULL;
static DWORD st_dwSendFileLen=0,st_dwSendFileIndex=0;

void WeldSendFileTimer(void)
{
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
	st_pbBuf = (WORD *) ext_mem_malloc(ALIGN_32(dwFileSize));
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
		*pdwRtcCnt|=((DWORD)(pSearchInfo->bName[i]-0x41)<<(i<<2));
	}
	return PASS;
}

#endif


#if (PRODUCT_UI==UI_WELDING)
extern WORD SensorWindow_PosX,SensorWindow_PosY,SensorWindow_Width,SensorWindow_Height;
extern BYTE g_bDisplayMode;

#define		FIBER_EDGE_LEVEL					0x40 //4   从背景到光纤的 黑白边界亮度值
#define		X_STEP											16       //4 快速横向查找步长，必须为4的倍数，一个单位为半个像素
#define		Y_VALID_PIXEL							18       //4 最小半边黑边高度 //really about 40  ,white <30
//#define		Y_LAST_SUM									4       //4 前面电平的统计数 must 4
#define		Y_CONTINUE_VALID_SUM			4       //4 Y轴上连续有效数
#define		Y_CENTER_CONTINUE_VALID_SUM			4       //4 Y轴中心上连续有效数
#define		X_VALID_SUM								40       //4 水平线统计最大值 max=width/X_STEP
#define		X_SHAKE										2         //4 FACE防抖
#define		Y_SHAKE										1         //4 水平误差

#define		Y_VALID_OFFSET						3       //4 水平线统计允许在Y上下偏差像素点

#define		CENTER_MIN_PIXEL					10       //4 中间白条最小高度 //really about 13 
#define		LEVEL_OFFSET								8         //4 亮度偏差

static BYTE st_bInProcWin=0,st_bBackupChanel=0xff,st_bBackupDisplayMode=0xff;
static BYTE st_bCacheWinNum=2;
 // BIT7->new fill wait init IPW  BIT6->first get data BIT5->get data end        BIT3->in fill down   BIT2->in FILL UP      BIT1->need fill down   BIT0->need FILL UP  0->not need fill
static BYTE st_bNeedFillProcWin=0,st_bFillWinFlag=0; 
static WORD st_wFiberWidth=0;

BYTE g_bOPMonline=0;

#if SENSOR_WIN_NUM>1
BYTE GetCachWinNum(void)
{
	return st_bCacheWinNum;
}

void SetCachWinNum(BYTE bNum)
{
	if (st_bCacheWinNum!=bNum)
	{
		st_bCacheWinNum=bNum;
		Sensor_CacheWin_Set();
	}
}
#endif

#if SENSOR_WIN_NUM
//--初始化一个sensor win对应下的所有光纤相关信息参数
void ResetFiberPara(void)
{
	DWORD i;

	for (i=0;i<MOTOR_NUM;i++)
	{
		//st_swFaceX[i]=-1;
		st_swFacexCurPos[i]=-1;
		st_swLastPos[i]=-1;
		//st_swFaceY1[i]=-1;
		//st_swFaceY2[i]=-1;
		//st_swFaceY20[i]=-1;
		//st_swFaceY3[i]=-1;
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

/*
SWORD GetReallyData(WORD *wData,WORD wDataCnt)
{
	DWORD dwValidlevelcnt,dwYValidStartAver;
	WORD i,k,wContinue=0;
	
	dwValidlevelcnt=0;
	//--找出平均值
	for (i=0;i<wDataCnt;i++)
	{
		dwValidlevelcnt+=wData[i];
	}
	dwYValidStartAver=dwValidlevelcnt/wDataCnt;
	//--找出出现最多的值
	for (k=2;k<10;k++)
	{
		wContinue=0;
		for (i=0;i<wDataCnt;i++)
		{
			if (ABS((SDWORD)wData[i]-(SDWORD)dwYValidStartAver)<k)
				wContinue++;
		}
		if (wContinue>wDataCnt/3*2)
		{
			//--重算平均值
			for (i=0;i<wDataCnt;i++)
			{
				if (ABS((SDWORD)wData[i]-(SDWORD)dwYValidStartAver)>=k)
					wData[i]=0;
			}
			dwValidlevelcnt=0;
			wContinue=0;
			for (i=0;i<wDataCnt;i++)
			{
				if (wData[i])
				{
					dwValidlevelcnt+=wData[i];
					wContinue++;
				}
			}
			return (dwValidlevelcnt/wContinue);
		}
	}

	return FAIL;
}
*/
SWORD GetReallyData(WORD *wData,WORD wDataCnt)
{
	DWORD dwValidcnt,dwYValidValue,dwMax=0,dwMin=0xffffffff,dwLen,dwStep,dwOffset;
	WORD i,j,k;
	
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
		return wData[0];
	if (dwLen>=10)
	{
		dwStep=dwLen/10;
		dwLen=10;
	}
	else
	{
		dwStep=1;
	}
	//--找出出现最多的值
	for (k=1;k<dwLen;k++) // add offset
	{
		//一个个取值出来比较
		for (i=0;i<wDataCnt;i++)
		{
			dwValidcnt=0;
			dwOffset=k*dwStep;
			//去对比其它的值
			for (j=0;j<wDataCnt;j++)
			{
				if (ABS((SDWORD)wData[i]-(SDWORD)wData[j])<dwOffset)
				{
					dwValidcnt++;
				}
			}
			//判断是否是较多的
			if (dwValidcnt>wDataCnt/3*2)
			{
				dwYValidValue=0;
				for (j=0;j<wDataCnt;j++)
				{
					if (ABS((SDWORD)wData[i]-(SDWORD)wData[j])<dwOffset)
					{
						dwYValidValue+=wData[j];
					}
				}
				return dwYValidValue/dwValidcnt;
			}
		}
		
	}

	return FAIL;
}

SDWORD GetFiberBlackLevel(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver,dwValidlevelcnt,dwBlackAverLevel=0,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYStartIndex,wYvalidCnt;
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
		if (bMode==MOTOR_RIGHT_TOP)
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

	if (st_bFillWinFlag |(FILL_WIN_UP|FILL_WIN_DOWN))
		st_bNeedFillProcWin=FILL_WIN_INIT|st_bFillWinFlag; // bit0->up win  bit1->down win   bit7->init mode
	else
		st_bNeedFillProcWin=FILL_WIN_INIT|FILL_WIN_UP|FILL_WIN_DOWN; // bit0->up win  bit1->down win   bit7->init mode
		//st_bNeedFillProcWin=0xe1; // bit0->up win  bit1->down win   bit7->init mode
	#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
			ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
	#else
			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
	#endif //--------------------------
}

void TimerToFillProcWin(DWORD dwTime) // ms
{
	Ui_TimerProcAdd(dwTime, SetFillProcWinFlag);
}

void TimerToFillReferWin(DWORD dwTime,BYTE bWinIndex) // ms
{
	if (bWinIndex<RPOC_WIN_TOTAL)
		st_bFillWinFlag=1<<bWinIndex;
	Ui_TimerProcAdd(dwTime, SetFillProcWinFlag);
}

void AutoStartWeld()
{
	BYTE i;
	
	if (st_dwProcState==SENSOR_IDLE)
	{
		st_dwProcState=SENSOR_FACE_POS1A;
		g_bDisplayMode=0x80;
		st_wDischargeTimeSet=0;
		for (i=0;i<MOTOR_NUM;i++)
		{
			st_swFacexCurPos[i]=-1;
			st_swLastPos[i]=-1;
		}
		TimerToFillProcWin(10);
	}
	else
		st_dwProcState=SENSOR_IDLE;

}

void Weld_StartPause()
{
	DWORD i;

mpDebugPrint("Weld_StartPause- %p",st_dwProcState);
	if (st_dwProcState & BIT30)
	{
		st_dwProcState &=~BIT30;
		EventSet(UI_EVENT, EVENT_PROC_DATA);
	}
	else if (st_dwProcState==SENSOR_IDLE)
	{
		Idu_OsdErase();
		st_dwProcState=SENSOR_FACE_POS1A;
		g_bDisplayMode=0x80;
		st_wDischargeTimeSet=0;
		for (i=0;i<MOTOR_NUM;i++)
		{
			st_swFacexCurPos[i]=-1;
			st_swLastPos[i]=-1;
			st_swFaceX[i]=-1;
			st_swFaceY1[i]=-1;
			st_swFaceY2[i]=-1;
			st_swFaceY20[i]=-1;
			st_swFaceY3[i]=-1;
		}
		TimerToFillProcWin(10);
		SendWeldStaus(0);
	}
	else
	{
		EventClear(UI_EVENT, EVENT_PROC_DATA);
		st_dwProcState |= BIT30;
		//for (i=0;i<MOTOR_NUM;i++)
		//	MotorSetStatus(i+1,MOTOR_STOP);
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
		Ui_TimerProcAdd(500, GetBackgroundLevel);
		return;
	}
	else if (!g_psSetupMenu->bBackGroundLevel[1])
	{
		TimerToFillReferWin(10,FILL_WIN_DOWN);
		Ui_TimerProcAdd(500, GetBackgroundLevel);
		return;
	}
	pWin=(ST_IMGWIN *)&SensorInWin[bSensorIndex];

	y=4;
	pbWinBuffer = (BYTE *) pWin->pdwStart+(g_wElectrodePos[0]-st_bRetryTimes*4)*2+y*pWin->dwOffset;
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

BYTE GetBackgrounValue(BYTE bSensorIndex,BYTE bMode) //bMode:0>enter level  1->work level
{
	if (bMode)
		return g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2;
	return g_psSetupMenu->bBackGroundLevel[bSensorIndex]/3;
}

SWORD SearchLeftFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

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
	for (i=0;i<X_VALID_SUM;i++)
		wYValidStartArry[i]=0;
	wYStartIndex=0;
	for (x=0;x<dwOffset;x+=X_STEP)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		//mpDebugPrint(" %d: ",x>>1);
		while (y>swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]*2/3 )
				{
					//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_BLUE);
					//mpDebugPrintN("%02x ",*pbWinBuffer);
					wValidPixelCnt++;
				}
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]*2/3)
				{
					//Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_BLUE);
					//mpDebugPrintN("%02x ",*pbWinBuffer);
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
					}
				}
				else if (bContinueCnt)
				{
					bContinueCnt=0;
				}
			}

			if (wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				if (wYStartIndex<X_VALID_SUM)
				{
					wYValidStartArry[wYStartIndex++]=y+wValidPixelCnt+wInvalidPixelCnt;
					//计算有效Y起始坐标
					if (wYStartIndex>=4)
					{
						dwYValidStartAver=0;
						for (i=0;i<wYStartIndex;i++)
							dwYValidStartAver+=wYValidStartArry[i];
						dwYValidStartAver/=wYStartIndex;
						wYvalidCnt=0;
						//mpDebugPrint("y: %02x  ",dwYValidStartAver);
						for (i=0;i<wYStartIndex;i++)
						{
							//mpDebugPrintN("%02x ",wYValidStartArry[i]);
							if ((dwYValidStartAver+Y_VALID_OFFSET>wYValidStartArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<wYValidStartArry[i]))
								wYvalidCnt++;
						}
						//mpDebugPrint(" %d ",wYvalidCnt);
						if (wYvalidCnt>wYStartIndex/2)
						{
							wYStartIndex=X_VALID_SUM;
							#if DISPLAY_IN_ONE_WIN
							if (g_bDisplayMode==0x02)
							{
								dwYValidStartAver>>=1;
								if (bMode>1)
									dwYValidStartAver+=pWin->wHeight/2;
							}
							#endif
							if (st_swFaceY3[bMode]!=dwYValidStartAver)
							{
								//st_swFaceNewY3[bMode].wCnt=1;
								st_swFaceY3[bMode]=dwYValidStartAver;
								DEBUG_POS("-----------Y3[%d]=%d ",bMode,st_swFaceY3[bMode]);
								#if OSD_LINE_NUM && !TEST_PLANE
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
								#endif
								#if TEST_PLANE
								if (st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]-st_swFaceY1[bMode])
								{
									dwYValidStartAver=(st_swFaceY3[bMode]+st_swFaceY1[bMode])>>1;
									if (dwYValidStartAver>pWin->wHeight/2)
										sprintf(&st_bStrCenter[bMode][0], "^:%d",(dwYValidStartAver-pWin->wHeight/2));
									else
										sprintf(&st_bStrCenter[bMode][0], "v:%d",pWin->wHeight/2-dwYValidStartAver);
									xpgSetUpdateOsdFlag(1);
								}
								#endif
								//return ENABLE;
							}
							return PASS;
						}
					}
				}
				break;
			}

			y--;
			pbWinBuffer-=dwOffset;
		}
		TaskYield();
		//mpDebugPrint(" ");
	}
	
	return FAIL;
}

SWORD SearchLeftFiberCenter(ST_IMGWIN *pWin,BYTE bMode)
{
	SDWORD sdwRet;
	DWORD dwOffset;
	WORD wValidPixelCnt,wContinueValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
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

	for (i=0;i<X_VALID_SUM;i++)
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
				if (wYStartIndex<X_VALID_SUM)
				{
					wYValidArry[wYStartIndex++]=y-wEndInvalidCnt-(wValidPixelCnt+wInvalidPixelCnt-wEndInvalidCnt)/2;
					//mpDebugPrintN("  -%d:%d  %d %d %d %d  ",wYStartIndex,wYValidArry[wYStartIndex-1],y,wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt);
					//计算有效Y起始坐标
					if (wYStartIndex>=8)
					{
						swYValidStartAver=GetReallyData(wYValidArry,wYStartIndex);
						if (swYValidStartAver>0)
						{
							wYStartIndex=X_VALID_SUM;
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
									#if TEST_PLANE
									sprintf(&st_bStrCore[bMode][0], "Center0:%d", st_swFaceY2[bMode]);
									xpgSetUpdateOsdFlag(1);
									#endif
									#if 0//OSD_LINE_NUM &&!TEST_PLANE
									OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,8+bMode,0,swYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_RED);
									#endif
									//return ENABLE;
								}
							}
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
	DWORD dwOffset,dwYValidStartAver,dwValidlevelcnt,dwBlackAverLevel,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0,swCoreLenth;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel,bSensorIndex;
	SDWORD sdwRet;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!Left core error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
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
	for (i=0;i<X_VALID_SUM;i++)
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
		if (wValidPixelCnt>=X_VALID_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYUp=GetReallyData(wYValidArry,wValidPixelCnt);
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

	for (i=0;i<X_VALID_SUM;i++)
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
						Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_RED);
						wValidPixelCnt++;
						break;
					}
				}
				else
				{
					bContinueCnt=0;
					Idu_OsdPaintArea(x>>1, y, 2, 1, OSD_COLOR_GREEN);
					bLastLevel=*pbWinBuffer;
				}
		}
		if (wValidPixelCnt>=X_VALID_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYDown=GetReallyData(wYValidArry,wValidPixelCnt);
			
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
			#if TEST_PLANE
			sprintf(&st_bStrCore[bMode][0], "Qian xing0:%d", st_swFaceY20[bMode]>>1);
			xpgSetUpdateOsdFlag(1);
			#endif
			#if OSD_LINE_NUM && !TEST_PLANE
			OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16+bMode,x>>1,st_swFaceY20[bMode]>>1,160,2,OSD_COLOR_RED);
		#endif
		}
		return PASS;
	}

	return FAIL;
}

SWORD SearchRightFiberCore(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver,dwValidlevelcnt,dwBlackAverLevel,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0,swCoreLenth;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel,bSensorIndex;
	SDWORD sdwRet;
	BYTE bColorIndex=OSD_COLOR_WHITE;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		MP_DEBUG("!!!RightCore error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
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
	for (i=0;i<X_VALID_SUM;i++)
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
						Idu_OsdPaintArea(x>>1, wYValidArry[wValidPixelCnt], 2, 1, OSD_COLOR_RED);
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
		if (wValidPixelCnt>=X_VALID_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYUp=GetReallyData(wYValidArry,wValidPixelCnt);
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
	for (i=0;i<X_VALID_SUM;i++)
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
		if (wValidPixelCnt>=X_VALID_SUM)
		{
			return FAIL;
		}
		else if (wValidPixelCnt>swCoreLenth ||(wValidPixelCnt>10 &&  x<(X_STEP<<2)))
		{
			wCoreYDown=GetReallyData(wYValidArry,wValidPixelCnt);
			
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
			#if TEST_PLANE
			sprintf(&st_bStrCore[bMode][0], "Qian xing1:%d", st_swFaceY20[bMode]>>1);
			xpgSetUpdateOsdFlag(1);
			#endif
		#if OSD_LINE_NUM && !TEST_PLANE
		OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16+bMode,(x>>1)-160,st_swFaceY20[bMode]>>1,160,2,OSD_COLOR_RED);
		#endif
		return PASS;
	}

	return FAIL;
}


SDWORD SearchTopEdge(ST_IMGWIN *pWin,SWORD swStartX,SWORD swXEnd,SWORD swStartY,SWORD swYEnd)
{
	DWORD dwOffset,dwYValidStartAver,dwYValidStartCnt,*pdwYValidStartArry;
	WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYvalidCnt;
	SWORD i,x,y;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

	pdwYValidStartArry = (DWORD *)ext_mem_malloc(ALIGN_32(swXEnd-swStartX+1));
	if (pdwYValidStartArry==NULL)
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
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
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
				pdwYValidStartArry[wValidLineCnt++]=y-wValidPixelCnt-wInvalidPixelCnt;
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
		dwYValidStartAver=0;
		for (i=0;i<wValidLineCnt;i++)
			dwYValidStartAver+=pdwYValidStartArry[i];
		dwYValidStartAver/=wValidLineCnt;
		//mpDebugPrint("y: %02x  ",dwYValidStartAver);
		//--去掉一些突然跳变的点，如灰尘
		wYvalidCnt=0;
		dwYValidStartCnt=0;
		for (i=0;i<wValidLineCnt;i++)
		{
			//mpDebugPrintN("%02x ",wYValidStartArry[i]);
			if ((dwYValidStartAver+Y_VALID_OFFSET>pdwYValidStartArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<pdwYValidStartArry[i]))
			{
				dwYValidStartCnt+=pdwYValidStartArry[i];
				wYvalidCnt++;
			}
		}
		//mpDebugPrint(" %d ",wYvalidCnt);
		if (wYvalidCnt>wValidLineCnt/2)
		{
			dwYValidStartAver=dwYValidStartCnt/wYvalidCnt;
			ext_mem_free(pdwYValidStartArry);
			return (SDWORD)dwYValidStartAver;
		}
	}

	ext_mem_free(pdwYValidStartArry);
	return FAIL;
}


SWORD SearchLeftFiberFaceAndTopEdge(ST_IMGWIN *pWin,BYTE bMode,BYTE bScanEdge)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD i,x,y,swXEnd,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

	if ((DWORD)pWin==(DWORD)&SensorInWin[1])
		bSensorIndex=SENSER_BOTTOM;
	else
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
	for (i=0;i<X_VALID_SUM;i++)
		wYValidStartArry[i]=0;
	wYStartIndex=0;
	wValidLineCnt=0;
	for (x=0;x<swXEnd;x+=X_STEP)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		if (wYStartIndex>=X_VALID_SUM)
			y=dwYValidStartAver-Y_VALID_OFFSET;
		else
			y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		bContinueCnt=0;
		//mpDebugPrint(" %d: ",x);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]*2/3 )
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex] *2/3 )
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
			if ((x>0)&&wValidPixelCnt>Y_VALID_PIXEL)
			{
				if (wValidPixelCnt>(wInvalidPixelCnt<<3))
				{
					wValidLineCnt++;
					if (bScanEdge && wYStartIndex<X_VALID_SUM)
					{
						wYValidStartArry[wYStartIndex++]=y-wValidPixelCnt-wInvalidPixelCnt;
						//计算有效Y起始坐标
						if (wYStartIndex>=4)
						{
							dwYValidStartAver=0;
							for (i=0;i<wYStartIndex;i++)
								dwYValidStartAver+=wYValidStartArry[i];
							dwYValidStartAver/=wYStartIndex;
							wYvalidCnt=0;
							//mpDebugPrint("y: %02x  ",dwYValidStartAver);
							for (i=0;i<wYStartIndex;i++)
							{
								//mpDebugPrintN("%02x ",wYValidStartArry[i]);
								if ((dwYValidStartAver+Y_VALID_OFFSET>wYValidStartArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<wYValidStartArry[i]))
									wYvalidCnt++;
							}
							//mpDebugPrint(" %d ",wYvalidCnt);
							if (wYvalidCnt>wYStartIndex/2)
							{
								wYStartIndex=X_VALID_SUM;
							#if DISPLAY_IN_ONE_WIN
							if (g_bDisplayMode==0x02)
							{
								dwYValidStartAver>>=1;
								if (bMode>1)
									dwYValidStartAver+=pWin->wHeight/2;
							}
							#endif
								if (st_swFaceY1[bMode]!=dwYValidStartAver)
								{
									st_swFaceY1[bMode]=dwYValidStartAver;
									DEBUG_POS("Y1[%d]=%d ",bMode,st_swFaceY1[bMode]);
									#if OSD_LINE_NUM && !TEST_PLANE
									OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4+bMode,0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
									#endif
								}
							}
						}
					}
					break;
				}
			}

			y++;
			pbWinBuffer+=dwOffset;
		}
		TaskYield();
		//mpDebugPrint(" ");
		//--判断靠边的起始第一个数据是否有效
		if (x==0)
		{
			if (wValidPixelCnt>pWin->wWidth*2/3 ||wInvalidPixelCnt >pWin->wWidth*2/3) // 9/20
			{
				//mpDebugPrint("%d-%d ",wValidPixelCnt,wInvalidPixelCnt);
				break;
			}
		}
		else if (y>=swYEnd)
		{
#if 0// debug
		if (wYStartIndex>=X_VALID_SUM)
			y=dwYValidStartAver-Y_VALID_OFFSET;
		else
			y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;

		mpDebugPrint("x=%d y=%d valid=%d/%d  level=%02x",x>>1,y,wValidPixelCnt,wInvalidPixelCnt,g_psSetupMenu->bBackGroundLevel[bSensorIndex] );
		while (y<swYEnd)
		{
			mpDebugPrintN("%08x ",*(pbWinBuffer));
			y++;
			pbWinBuffer+=dwOffset;
		}
		mpDebugPrint(" ");

#endif
			break;
		}
		else
		{
			//mpDebugPrint("0: %d :%d-%d ",x,wValidPixelCnt,wInvalidPixelCnt);
		}
	}

	//--找到有效端面
	if (wValidLineCnt>4)
	{
		//在最后一个有效端面与第一个无效端面间精确定位
		swXEnd=x;
		x-=X_STEP;
		for (;x<swXEnd;x+=4)
		{
			pbWinBuffer = (BYTE *) pWin->pdwStart+x;
			wValidPixelCnt=0;
			wInvalidPixelCnt=0;
			if (wYStartIndex>=X_VALID_SUM)
				y=dwYValidStartAver-Y_VALID_OFFSET;
			else
				y=swStartY;
			pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
			while (y<swYEnd)
			{
				//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (wValidPixelCnt)
				{
					if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
						wValidPixelCnt++;
					else
						wInvalidPixelCnt++;
				}
				else
				{
					if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex] /2)
					{
						bContinueCnt++;
						if (bContinueCnt>Y_CONTINUE_VALID_SUM)
						{
						//mpDebugPrintN(" %02x----%02x: ",*pbWinBuffer,dwLastValidLevel);
							wValidPixelCnt=bContinueCnt;
							bContinueCnt=0;
						}
					}
					else
					{
						bContinueCnt=0;
					}
				}
				if (wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
				{
					wValidLineCnt++;
					break;
				}

				y++;
				pbWinBuffer+=dwOffset;
			}
			TaskYield();
			if (y>=swYEnd)
			{
				//mpDebugPrint("3: %d :%d-%d ",x,wValidPixelCnt,wInvalidPixelCnt);
				break;
			}
			else
			{
				//mpDebugPrint("2: %d :%d-%d ",x,wValidPixelCnt,wInvalidPixelCnt);
			}
		}
		//if (x<=swXEnd)
		{
			x>>=1;
			//x--;
			if (st_swFaceX[bMode]>=x+X_SHAKE || st_swFaceX[bMode]+X_SHAKE<=x)
			{
				st_swFaceX[bMode]=x;
				DEBUG_POS("X[%d]=%d",bMode,x);
				#if TEST_PLANE
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

				#if 0//OSD_LINE_NUM && !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,bMode,x,(bMode>1 ? pWin->wHeight/2:0),2,pWin->wHeight/2,OSD_COLOR_GREEN);
				#endif
				#if TEST_PLANE
				if (st_swFaceX[bMode]>pWin->wWidth/2)
					sprintf(&st_bStrFace[bMode][0], "%03d<|", st_swFaceX[bMode]-pWin->wWidth/2);
				else
					sprintf(&st_bStrFace[bMode][0], "%03d>|", st_swFaceX[bMode]-pWin->wWidth/2);
				xpgSetUpdateOsdFlag(1);
				#endif
				return ENABLE;
			}
			return PASS;
		}

	}

	return FAIL;
}

SWORD SearchRightFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD i,x,y,swXstart,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

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
	for (i=0;i<X_VALID_SUM;i++)
		wYValidStartArry[i]=0;
	wYStartIndex=0;

	swXstart=dwOffset-4;
	for (x=swXstart;x>0;x-=X_STEP)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		bContinueCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		while (y>swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex] /2)
				{
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
					}
				}
				else if (bContinueCnt)
				{
					bContinueCnt=0;
				}
			}


			if (wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				if (wYStartIndex<X_VALID_SUM)
				{
					wYValidStartArry[wYStartIndex++]=y+wValidPixelCnt+wInvalidPixelCnt;
					//计算有效Y起始坐标
					if (wYStartIndex>=4)
					{
						dwYValidStartAver=0;
						for (i=0;i<wYStartIndex;i++)
							dwYValidStartAver+=wYValidStartArry[i];
						dwYValidStartAver/=wYStartIndex;
						wYvalidCnt=0;
						//mpDebugPrint("y: %02x  ",dwYValidStartAver);
						for (i=0;i<wYStartIndex;i++)
						{
							//mpDebugPrintN("%02x ",wYValidStartArry[i]);
							if ((dwYValidStartAver+Y_VALID_OFFSET>wYValidStartArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<wYValidStartArry[i]))
								wYvalidCnt++;
						}
						//mpDebugPrint(" %d ",wYvalidCnt);
						if (wYvalidCnt>wYStartIndex/2)
						{
							wYStartIndex=X_VALID_SUM;
							#if DISPLAY_IN_ONE_WIN
							if (g_bDisplayMode==0x02)
							{
								dwYValidStartAver>>=1;
								if (bMode>1)
									dwYValidStartAver+=pWin->wHeight/2;
							}
							#endif
							if (st_swFaceY3[bMode]!=dwYValidStartAver)
							{
								st_swFaceY3[bMode]=dwYValidStartAver;
								DEBUG_POS(" ----Y3[%d]=%d ",bMode,st_swFaceY3[bMode]);
								#if OSD_LINE_NUM && !TEST_PLANE
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
								#endif
								#if TEST_PLANE
								if (st_swFaceY1[bMode]>0 && st_swFaceY3[bMode]>st_swFaceY1[bMode])
								{
									dwYValidStartAver=(st_swFaceY3[bMode]+st_swFaceY1[bMode])>>1;
									if (dwYValidStartAver>pWin->wHeight/2)
										sprintf(&st_bStrCenter[bMode][0], "^:%d",(dwYValidStartAver-pWin->wHeight/2));
									else
										sprintf(&st_bStrCenter[bMode][0], "v:%d",pWin->wHeight/2-dwYValidStartAver);
									xpgSetUpdateOsdFlag(1);
								}
								#endif
								return ENABLE;
							}
							return PASS;
						}
					}
				}
								
				break;
			}

			y--;
			pbWinBuffer-=dwOffset;
		}
		TaskYield();
	}
	
	return FAIL;
}

SWORD SearchRightFiberCenter(ST_IMGWIN *pWin,BYTE bMode)
{
	SDWORD sdwRet;
	DWORD dwOffset;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex;
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

	for (i=0;i<X_VALID_SUM;i++)
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
				if (wYStartIndex<X_VALID_SUM)
				{
					wYValidArry[wYStartIndex++]=y-wEndInvalidCnt-(wValidPixelCnt+wInvalidPixelCnt-wEndInvalidCnt)/2;
					//mpDebugPrintN("  -%d:%d  %d %d %d %d  ",wYStartIndex,wYValidArry[wYStartIndex-1],y,wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt);
					//mpDebugPrintN(" -%d: %d  ",wYStartIndex,wYValidArry[wYStartIndex-1]);
					//计算有效Y起始坐标
					if (wYStartIndex>=8)
					{
						swYValidStartAver=GetReallyData(wYValidArry,wYStartIndex);
						if (swYValidStartAver>0)
						{
							wYStartIndex=X_VALID_SUM;
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
									#if TEST_PLANE
									sprintf(&st_bStrCore[bMode][0], "Center1:%d", st_swFaceY2[bMode]);
									xpgSetUpdateOsdFlag(1);
									#endif
									#if 0//OSD_LINE_NUM && !TEST_PLANE
									OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,8+bMode,pWin->wWidth/2,st_swFaceY2[bMode],pWin->wWidth,2,OSD_COLOR_RED);
									#endif
									//return ENABLE;
								}
							}
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

SWORD SearchRightFiberFaceAndTopEdge(ST_IMGWIN *pWin,BYTE bMode,BYTE bScanFace)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD i,x,y,swXEnd,swXstart,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt,bSensorIndex;

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
	for (i=0;i<X_VALID_SUM;i++)
		wYValidStartArry[i]=0;
	wYStartIndex=0;
	wValidLineCnt=0;

	swXstart=dwOffset-4;
	//mpDebugPrint("swXstart: %d  ",swXstart);
	for (x=swXstart;x>0;x-=X_STEP)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		bContinueCnt=0;
		if (wYStartIndex>=X_VALID_SUM)
			y=dwYValidStartAver-Y_VALID_OFFSET;
		else
			y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		//mpDebugPrintN("%02d: ",x>>1);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
				{
					bContinueCnt++;
					if (bContinueCnt>Y_CONTINUE_VALID_SUM)
					{
						wValidPixelCnt=bContinueCnt;
					}
				}
				else if (bContinueCnt)
				{
					bContinueCnt=0;
				}
			}

			if ((x<swXstart)&&wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
			{
				wValidLineCnt++;
				//mpDebugPrintN(" %d-%d %d ",wYStartIndex,wValidPixelCnt,wInvalidPixelCnt);
				if (wYStartIndex<X_VALID_SUM)
				{
					wYValidStartArry[wYStartIndex++]=y-wValidPixelCnt-wInvalidPixelCnt;
					//计算有效Y起始坐标
					if (wYStartIndex>=4)
					{
						dwYValidStartAver=0;
						for (i=0;i<wYStartIndex;i++)
							dwYValidStartAver+=wYValidStartArry[i];
						dwYValidStartAver/=wYStartIndex;
						wYvalidCnt=0;
						//mpDebugPrint("y: %02x  ",dwYValidStartAver);
						for (i=0;i<wYStartIndex;i++)
						{
							//mpDebugPrintN("%02x ",wYValidStartArry[i]);
							if ((dwYValidStartAver+Y_VALID_OFFSET>wYValidStartArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<wYValidStartArry[i]))
								wYvalidCnt++;
						}
						//mpDebugPrint(" %d ",wYvalidCnt);
						if (wYvalidCnt>wYStartIndex/2)
						{
							wYStartIndex=X_VALID_SUM;
							#if DISPLAY_IN_ONE_WIN
							if (g_bDisplayMode==0x02)
							{
								dwYValidStartAver>>=1;
								if (bMode>1)
									dwYValidStartAver+=pWin->wHeight/2;
							}
							#endif
							if (st_swFaceY1[bMode]!=dwYValidStartAver)
							{
								st_swFaceY1[bMode]=dwYValidStartAver;
								DEBUG_POS("-----Y1[%d]=%d ",bMode,st_swFaceY1[bMode]);
								#if OSD_LINE_NUM ///&& !TEST_PLANE
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
								#endif
							}
							if (!bScanFace)
								return PASS;
						}
					}
				}
								
				break;
			}

			y++;
			pbWinBuffer+=dwOffset;
		}
		//mpDebugPrint(" ");
		TaskYield();
			//--判断靠边的起始第一个数据是否有效
		if (x==swXstart)
		{
			if (wValidPixelCnt>pWin->wHeight*2/3 ||wInvalidPixelCnt >pWin->wHeight*2/3)
			{
				//mpDebugPrint("F %d-%d ",wValidPixelCnt,wInvalidPixelCnt);
				break;
			}
		}
		else if (y>=swYEnd)
		{
			//mpDebugPrint("E: %d :%d-%d ",x,wValidPixelCnt,wInvalidPixelCnt);
			break;
		}
		else
		{
			//mpDebugPrint("1: %d :%d-%d ",x,wValidPixelCnt,wInvalidPixelCnt);
		}
	}
	//无有效端面或者所有端都有效
	if (!wValidLineCnt || x<=0)
	{
		//UartOutText("-N-");
	}
	else //find
	{
		//在最后一个有效端面与第一个无效端面间精确定位
		swXEnd=x+X_STEP;
		wValidLineCnt=0;
		for (;x<=swXEnd;x+=4)
		{
			wValidPixelCnt=0;
			wInvalidPixelCnt=0;
			if (wYStartIndex>=X_VALID_SUM)
				y=dwYValidStartAver-Y_VALID_OFFSET;
			else
				y=swStartY;
			pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
			while (y<swYEnd)
			{
				//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (wValidPixelCnt)
				{
					if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
						wValidPixelCnt++;
					else
						wInvalidPixelCnt++;
				}
				else
				{
					if (*pbWinBuffer < g_psSetupMenu->bBackGroundLevel[bSensorIndex]/2)
					{
						bContinueCnt++;
						if (bContinueCnt>Y_CONTINUE_VALID_SUM)
						{
							wValidPixelCnt=bContinueCnt;
						}
					}
					else if (bContinueCnt)
					{
						bContinueCnt=0;
					}
				}
				if (wValidPixelCnt>Y_VALID_PIXEL && wValidPixelCnt>(wInvalidPixelCnt<<3))
				{
					wValidLineCnt=1;
					break;
				}

				y++;
				pbWinBuffer+=dwOffset;
			}
			TaskYield();
			if (wValidLineCnt)
				break;
			
		}
		if (wValidLineCnt)
		{
			x>>=1;
			{
			if (st_swFaceX[bMode]>=x+X_SHAKE || st_swFaceX[bMode]+X_SHAKE<=x)
			{
				st_swFaceX[bMode]=x;
				DEBUG_POS("X[%d]=%d",bMode,x);
				#if TEST_PLANE
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
				#if OSD_LINE_NUM //&& !TEST_PLANE
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,bMode,x,(bMode>1 ? pWin->wHeight/2:0),2,pWin->wHeight/2,OSD_COLOR_GREEN);
				#endif
				#if TEST_PLANE
				if (st_swFaceX[bMode]>pWin->wWidth/2)
					sprintf(&st_bStrFace[bMode][0], "|<%03d", pWin->wWidth/2-st_swFaceX[bMode]);
				else
					sprintf(&st_bStrFace[bMode][0], "|>%03d",  pWin->wWidth/2-st_swFaceX[bMode]);
				xpgSetUpdateOsdFlag(1);
				#endif

				return ENABLE;
			}
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



#define		MOTO_BASE_RETRY_TIMES					10
#define		MOTO_BASE_STEPS									8 // pixel
void DriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wStep,BYTE bSpeed)
{
	BYTE bTxData[10],bChecksum,i,bDataIndex=bMotorInex-1;
	WORD wPulse=wStep;

	//mpDebugPrint("--bMotorInex=%d  bDirection=%d/%d st_bMotorStaus %p",bMotorInex,bDirection,st_bDirectionArry[bDataIndex],st_bMotorStaus);
	if (bMotorInex<5)
	{
		if ((st_bMotorStaus&(1<<bDataIndex)) && st_bDirectionArry[bDataIndex]==bDirection)
			return;
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
	//wStep+=st_bMotorBaseStep[bDataIndex];
	mpDebugPrint("bMotorInex=%d  bDirection=%d wPulse %d",bMotorInex,bDirection,wPulse);
	bTxData[0]=0xa2;
	bTxData[1]=3+5;
	bTxData[2]=bMotorInex; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	bTxData[3]=bDirection; // 0（后退）1（前进
	bTxData[4]=wPulse>>8;
	bTxData[5]=wPulse&0x00ff;
	bTxData[6]=bSpeed; // min 1  max=10
	if (TSPI_PacketSend(bTxData,0)==PASS)
	{
		st_bMotorStaus|=(1<<bDataIndex);
		st_bDirectionArry[bDataIndex]=bDirection;
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
			st_bMotorStaus&= ~(1<<bDataIndex);
	}

}

void TimeoutToStopHoldMoto1(void)
{
	BYTE bMotorInex=1;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void TimeoutToStopHoldMoto2(void)
{
	BYTE bMotorInex=2;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void TimeoutToStopHoldMoto3(void)
{
	BYTE bMotorInex=3;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void TimeoutToStopHoldMoto4(void)
{
	BYTE bMotorInex=4;
	
	MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
	st_bMotorHold &=~(1<<bMotorInex);
}
void MotoHoldTimeoutSet(BYTE bMotorInex,BYTE bMode)
{
	BYTE bTxData[10];

	if (bMode==MOTOR_HOLD && (!(st_bMotorHold&(1<<bMotorInex))))
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
		st_bMotorHold |=(1<<bMotorInex);
	}
	else if (bMode==MOTOR_NO_HOLD && (st_bMotorHold&(1<<bMotorInex)))
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
		st_bMotorHold &=~(1<<bMotorInex);
	}
}

void TimerToReleaseAllHold(void)
{
			MotorSetStatus(1,MOTOR_NO_HOLD);
			MotorSetStatus(2,MOTOR_NO_HOLD);
			MotorSetStatus(3,MOTOR_NO_HOLD);
			MotorSetStatus(4,MOTOR_NO_HOLD);

}

BYTE GetMotoIdxByFiberIdx(BYTE bFiberIndex) //0xa3   bmode:0->stop  BYT4~7   0 停止 1 允许待机  2不允许待机
{
	BYTE bMotorInex;
	
	if (bFiberIndex==MOTOR_RIGHT_TOP)
		bMotorInex=04; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bFiberIndex==MOTOR_RIGHT_BOTTOM)
		bMotorInex=03;
	else if (bFiberIndex==MOTOR_LEFT_BOTTOM)
		bMotorInex=02; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bFiberIndex==MOTOR_LEFT_TOP)
		bMotorInex=01;

	return bMotorInex;

}

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

void MoveHMotorToSpecPosition(BYTE bFiberIndex,SWORD swTx)
{
	SWORD swSx,swDiffX;
	BYTE bMotorInex,bDirection,bSpeed;
	WORD wStep;

	swSx=st_swFaceX[bFiberIndex];
	//if (swSx<0)
	//	return;
	mpDebugPrint("H %d swSx=%d  swTx=%d",bFiberIndex,swSx,swTx);

	if (bFiberIndex==MOTOR_LEFT_BOTTOM)
		bMotorInex=02; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
	else if (bFiberIndex==MOTOR_LEFT_TOP)
		bMotorInex=01;
	else
		return;
	swDiffX=swTx-swSx;
	if (ABS(swDiffX)<=X_SHAKE)
	{
		st_swFacexCurPos[bFiberIndex]=swTx;
		//MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
		return;
	}
	if (swDiffX>0)
		bDirection=1; // 0（后退）1（前进
	else
	{
		bDirection=0;
		swDiffX=-swDiffX;
	}
	if (swDiffX<60)
		wStep=swDiffX<<3;
	else
		wStep=swDiffX*10;
	//if (st_bDirectionArry[bMotorInex]!=bDirection) //1300
	//	wStep+=100;
	if (swDiffX<10)
	{
		MotorSetStatus(bMotorInex,MOTOR_HOLD);
	}
	if (swDiffX>300)
		bSpeed=10;
	else if (swDiffX>60 || swSx>0)
		bSpeed=4;
	else
		bSpeed=3;
	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
}

void MoveVMotorToSpecPosition(BYTE bFiberIndex,SWORD swSx,SWORD swTx)
{
	SWORD swDiffX;
	BYTE bMotorInex,bValueIndex,bDirection,bSpeed,bDirectonUp;
	WORD wStep;

	//swSx=st_swFaceY2[bFiberIndex];
	//if (swTx<0)
	//	return;
	swDiffX=swTx-swSx;
	if (bFiberIndex==MOTOR_RIGHT_TOP)
	{
		bMotorInex=04; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
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
	else if (bFiberIndex==MOTOR_RIGHT_BOTTOM)
	{
		bMotorInex=03;
#if 1
		bValueIndex=1;
		if (st_bVmotorMoveTimes[1]<0xff)
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
						st_bBottomVMotorUpValue=1-st_bBottomVMotorUpValue;
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
		bDirectonUp=st_bBottomVMotorUpValue;
	}
	else
		return;
	if (ABS(swDiffX)<1)
	{
		st_swFacexCurPos[bFiberIndex]=swTx;
		//MotorSetStatus(bMotorInex,MOTOR_NO_HOLD);
		return;
	}
	mpDebugPrint("V swSx=%d  swTx=%d",swSx,swTx);
	if (swDiffX>0)
		bDirection=1-bDirectonUp; //   1->down 0->up
	else
	{
		bDirection=bDirectonUp;
		swDiffX=-swDiffX;
	}
	if (swDiffX<6)
	{
		MotorSetStatus(bMotorInex,MOTOR_HOLD);
	}
	wStep=swDiffX*40; // 123->50
 if (swDiffX>20)
		bSpeed=3;
	else
		bSpeed=2;

	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
}

void Discharge(WORD wMode,BYTE bStep)
{
	BYTE i,bTxData[TX_BUFFER_LENTH];

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
					bTxData[4]=130>>8;
					bTxData[5]=130&0xff;
					break;

				case 2:
					bTxData[2]=0;
					bTxData[3]=130;
					bTxData[4]=1000>>8;
					bTxData[5]=1000&0xff;
					break;

				case 3://1.750v  2300ms
					bTxData[2]=1750>>8;
					bTxData[3]=1750&0xff;
					#if TEST_PLANE
					bTxData[4]=st_wMotoStep[MOTO_ADJ_NUM-1]>>8;
					bTxData[5]=st_wMotoStep[MOTO_ADJ_NUM-1]&0xff;
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
			bTxData[2]=1750>>8;
			bTxData[3]=1750&0xff;
			bTxData[4]=wMode>>8;
			bTxData[5]=wMode&0xff;
		}
	}
	TSPI_PacketSend(bTxData,1);
}

SWORD  SendWeldStaus(BYTE bStart)
{
	BYTE i,bTxData[8];

	bTxData[0]=0x96;
	bTxData[1]=3+1;
	if (bStart)
		bTxData[2] =BIT1;
	return TSPI_PacketSend(bTxData,0);
		
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
	//WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
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
			SearchLeftFiberFaceAndTopEdge(pWin,MOTOR_LEFT_TOP,1);
			SearchLeftFiberBottomEdge(pWin,MOTOR_LEFT_TOP);
			if (st_swFaceY1[MOTOR_LEFT_TOP]>0&&st_swFaceY3[MOTOR_LEFT_TOP]>0&&st_swFaceY3[MOTOR_LEFT_TOP]>st_swFaceY1[MOTOR_LEFT_TOP])
			{
				st_wFiberWidth=st_swFaceY3[MOTOR_LEFT_TOP]-st_swFaceY1[MOTOR_LEFT_TOP];
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
	//mpDebugPrint(" SearchLeftFiberFaceAndTopEdge bMode=%d: bScanEdge=%d",bMode,bScanEdge);

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
	if (g_wElectrodePos[0])
		TimerToFillReferWin(dwTime,RPOC_WIN1);
	else
		TimerToFillReferWin(dwTime,RPOC_WIN0);
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
			g_wElectrodePos[0]=0;
			g_wElectrodePos[1]=0;
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
			if (g_wElectrodePos[0]==0)
			{
				pWin=(ST_IMGWIN *)&SensorInWin[0];
			}
			else
			{
				pWin=(ST_IMGWIN *)&SensorInWin[1];
			}
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
			if (g_wElectrodePos[0]==0)
			{
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				swRet=SearchFiberLowPoint(pWin,0);
			}
			else
			{
				pWin=(ST_IMGWIN *)&SensorInWin[1];
				swRet=SearchFiberLowPoint(pWin,1);
			}
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
				MP_DEBUG("GET_CENTER_LOW_POINT   swRet=%d  [%d]",swRet,g_wElectrodePos[0]);
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
					if (g_wElectrodePos[0])
						sprintf(bstring, " %d  %d ", g_wElectrodePos[0],swRet);
					else
						sprintf(bstring, " %d ", swRet);
					Idu_OSDPrint(psWin,bstring, 16, 16, OSD_COLOR_RED);
				}
				if (g_wElectrodePos[0]==0)
				{
					g_wElectrodePos[0]=swRet;
					//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,18,g_wElectrodePos[0],0,2,pWin->wHeight/2,OSD_COLOR_RED);
					g_psSetupMenu->wElectrodePos[0]=g_wElectrodePos[0];
				}
				else
				{
					g_wElectrodePos[1]=swRet;
					//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,19,g_wElectrodePos[1],pWin->wHeight/2,2,pWin->wHeight,OSD_COLOR_RED);
					g_psSetupMenu->wElectrodePos[1]=g_wElectrodePos[1];
				}
				WriteSetupChg();
				st_dwGetCenterState++;
				EventSet(UI_EVENT, EVENT_PROC_DATA);
				bSetEvent=1;
			}
			break;

		case GET_CENTER_FINISH:
		#if 0
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

// 马达复位
void ResetMotor(void)
{
	BYTE bTxData[6];

	bTxData[0]= 0xa4;
	bTxData[1]= 4;
	bTxData[2]= 2;
	TSPI_PacketSend(bTxData,1);
}


void TimerToNextState(void)
{
	st_dwProcState++;
	EventSet(UI_EVENT, EVENT_PROC_DATA);
}

#define   RETRY_POS		10
void Proc_Weld_State()
{
#if TEST_TWO_LED
	ST_OSDWIN * psWin=Idu_GetOsdWin();
	BYTE bString[64];
#endif
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet,swPos1,swPos2;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下
	static BYTE st_bGetRetry=0,st_bRedo;

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
		case SENSOR_FACE_POS1A:
			bMode=MOTOR_LEFT_TOP;
			swPos1=g_wElectrodePos[0]-20;
			if (ABS(swPos1-st_swFacexCurPos[bMode])>X_SHAKE)
			{
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==FAIL)
					st_swFaceX[bMode]=-1;
				MoveHMotorToSpecPosition(bMode,swPos1);
				if (st_swFacexCurPos[bMode]==swPos1)
					SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				TimerToFillReferWin(10,RPOC_WIN0);
			}
			else
			{
				//st_dwProcState=SENSOR_IDLE;
				TimerToFillReferWin(10,RPOC_WIN1);
				g_bDisplayMode=0x81;
				st_dwProcState++;
				st_bGetRetry=RETRY_POS;
				//st_wDischargeTimeSet=1;
				//Discharge(st_wDischargeTimeSet,0);
				st_bRedo=2;
			}
			break;

		case SENSOR_FACE_POS1B:
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			bMode=MOTOR_LEFT_BOTTOM;
			swPos2=g_wElectrodePos[1]-20;
			if (ABS(swPos2-st_swFacexCurPos[bMode])>X_SHAKE)
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==FAIL)
					st_swFaceX[bMode]=-1;
				MoveHMotorToSpecPosition(bMode,swPos2);
				//if (st_swFacexCurPos[bMode]==swPos2)
				//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				TimerToFillReferWin(10,RPOC_WIN1);
			}
			else
			//if ((st_swFacexCurPos[MOTOR_LEFT_TOP]==swPos1)&&(st_swFacexCurPos[MOTOR_LEFT_BOTTOM]==swPos2))
			{
				mpDebugPrint("SENSOR_FACE_POS1B OK");
				st_dwProcState=SENSOR_FACE_POS1B+1;
				g_bDisplayMode=0x80;
				TimerToFillReferWin(10,RPOC_WIN0);
			}
			break;
#if 0
		case SENSOR_ALIGN_H1A:
			bMode=MOTOR_RIGHT_TOP;
			if (st_swFacexCurPos[bMode]!=st_swFaceY2[bMode-1])
			{
				swRet=SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				SearchRightFiberCenter(pWin,bMode);
				if (swRet!=FAIL)
				{
						MoveVMotorToSpecPosition(bMode,st_swFaceY2[bMode-1]);
				}
				TimerToFillProcWin(10);
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H1A OK");
				st_dwProcState++;
			}
			break;

		case SENSOR_ALIGN_H1B:
			bMode=MOTOR_RIGHT_BOTTOM;
			if (st_swFacexCurPos[bMode]!=st_swFaceY2[bMode-1])
			{
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				swRet=SearchRightFiberCenter(pWin,bMode);
				if (swRet!=FAIL)
				{
					MoveVMotorToSpecPosition(bMode,st_swFaceY2[bMode-1]);
				}
				TimerToFillProcWin(10);
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H1B OK");
				st_dwProcState++;
			}
			/*
			if ((st_swFacexCurPos[bMode]==st_swFaceY1[bMode-1])&&(st_swFacexCurPos[MOTOR_RIGHT_TOP]==st_swFaceY1[MOTOR_RIGHT_TOP-1]))
			{
				mpDebugPrint("SENSOR_ALIGN_H2 OK");
				st_dwProcState=SENSOR_ALIGN_H2+1;
			}
			*/
			break;
#endif
		case SENSOR_DISCHARGE1:
			//if (st_wDischargeTimeSet!=1)
			{
				st_wDischargeTimeSet=1;
				Discharge(st_wDischargeTimeSet,0);
				Ui_TimerProcAdd(3000, TimerToNextState);
			}
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
			st_dwProcState++;
			g_bDisplayMode=0x80;
			TimerToFillReferWin(10,RPOC_WIN0);
			//st_dwProcState=SENSOR_IDLE;
			break;

		case SENSOR_FACE_POS2A:
			pWin=(ST_IMGWIN *)&SensorInWin[0];
			bMode=MOTOR_LEFT_TOP;
			swPos1=g_wElectrodePos[0]-40;//  34
			if (swPos1 !=st_swFacexCurPos[bMode])
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet!=FAIL)
				{
					MoveHMotorToSpecPosition(bMode,swPos1);//  MOTOR_LEFT_TOP MOTOR_LEFT_BOTTOM
					//if (st_swFacexCurPos[bMode]==swPos1)
					//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				}
				//TimerToFillProcWin(10);
				TimerToFillReferWin(10,RPOC_WIN0);
			}
			else
			{
				bMode=MOTOR_LEFT_TOP;
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==PASS)
					swRet=SearchLeftFiberBottomEdge(pWin,bMode);
				bMode=MOTOR_RIGHT_TOP;
				if (swRet==PASS)
					swRet=SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==PASS)
					swRet=SearchRightFiberBottomEdge(pWin,bMode);

				if (swRet==PASS && g_psSetupMenu->bDuiXianFangShi==0)
				{
					if (g_psSetupMenu->bRongJieZhiLiang==2)
						swRet=SearchLeftFiberCore(pWin,MOTOR_LEFT_TOP);
					//else
					//	swRet=SearchLeftFiberCenter(pWin,MOTOR_LEFT_TOP);
				}
					mpDebugPrint("--SENSOR_FACE_POS2A OK %d -Try %d",swRet,st_bGetRetry);
				if (swRet==PASS || st_bGetRetry==1)
				{
					st_dwProcState++;
					g_bDisplayMode=0x81;
					TimerToFillReferWin(10,RPOC_WIN1);
					st_bGetRetry=RETRY_POS;
				}
				else
				{
					if (!st_bGetRetry)
						st_bGetRetry=RETRY_POS;
					st_bGetRetry--;
					if (!st_bGetRetry)
						st_dwProcState=SENSOR_IDLE;
					TimerToFillReferWin(10,RPOC_WIN0);
				}
			}
			break;

		case SENSOR_FACE_POS2B:
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			bMode=MOTOR_LEFT_BOTTOM;
			//swPos2=pWin->wWidth-g_wElectrodePos[1];
			swPos2=g_wElectrodePos[1];
			if (swPos2 != st_swFacexCurPos[bMode])
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet!=FAIL)
				{
					MoveHMotorToSpecPosition(bMode,swPos2);//  MOTOR_LEFT_TOP MOTOR_LEFT_BOTTOM
					//if (st_swFacexCurPos[bMode]==swPos2)
					//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				}
				//TimerToFillProcWin(10);
				TimerToFillReferWin(10,RPOC_WIN1);
			}
			else
			{
				bMode=MOTOR_LEFT_BOTTOM;
				swRet=PASS;
				if (st_swFaceY1[bMode]<1)
					swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==PASS && st_swFaceY3[bMode]<1)
					swRet=SearchLeftFiberBottomEdge(pWin,bMode);
				bMode=MOTOR_RIGHT_BOTTOM;
				if (swRet==PASS && st_swFaceY1[bMode]<1)
					swRet=SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==PASS && st_swFaceY3[bMode]<1)
					swRet=SearchRightFiberBottomEdge(pWin,bMode);

					mpDebugPrint("--SENSOR_FACE_POS2B %d -Try %d",swRet,st_bGetRetry);

				if (swRet==PASS && g_psSetupMenu->bDuiXianFangShi==0)
				{
					if (g_psSetupMenu->bRongJieZhiLiang==2)
						swRet=SearchLeftFiberCore(pWin,MOTOR_LEFT_TOP);
					//else
					//	swRet=SearchLeftFiberCenter(pWin,MOTOR_LEFT_TOP);
				}
				if (swRet==PASS||st_bGetRetry==1)
				{
					//st_dwProcState++;
					st_bGetRetry=RETRY_POS;
					mpDebugPrint("--SENSOR_FACE_POS2B OK %d %d",g_psSetupMenu->bDuiXianFangShi,g_psSetupMenu->bRongJieZhiLiang);
					MotorSetStatus(GetMotoIdxByFiberIdx(MOTOR_RIGHT_TOP),MOTOR_HOLD);
					if (g_psSetupMenu->bDuiXianFangShi==0 && g_psSetupMenu->bRongJieZhiLiang==2)
						st_dwProcState=SENSOR_ALIGN_H3A;
					else
						st_dwProcState=SENSOR_ALIGN_H2A;
					g_bDisplayMode=0x80;
					TimerToFillReferWin(10,RPOC_WIN0);
				}
				else
				{
					if (!st_bGetRetry)
						st_bGetRetry=RETRY_POS;
					st_bGetRetry--;
					if (!st_bGetRetry)
						st_dwProcState=SENSOR_IDLE;
					TimerToFillReferWin(10,RPOC_WIN1);
				}

			}
			#if 0
			if ((st_swFacexCurPos[MOTOR_LEFT_TOP]==swPos1)&&(st_swFacexCurPos[MOTOR_LEFT_BOTTOM]==swPos2))
			{
				mpDebugPrint("SENSOR_FACE_POS2B OK");
				MotorSetStatus(GetMotoIdxByFiberIdx(MOTOR_RIGHT_TOP),MOTOR_HOLD);
				if (g_psSetupMenu->bDuiXianFangSh==0i && g_psSetupMenu->bRongJieZhiLiang==2)
					st_dwProcState=SENSOR_ALIGN_H3A;
				else
					st_dwProcState=SENSOR_ALIGN_H2A;
			}
			#endif
			break;

		case SENSOR_ALIGN_H2A:
			bMode=MOTOR_RIGHT_TOP;
				mpDebugPrint(" MOTOR_LEFT %d - %d",st_swFaceY1[bMode-1],st_swFaceY3[bMode-1]);
				mpDebugPrint(" MOTOR_RIGHT %d - %d",st_swFaceY1[bMode],st_swFaceY3[bMode]);
			//swPos1=st_swFaceY2[bMode-1];
			swPos1=(st_swFaceY1[bMode-1]+st_swFaceY3[bMode-1])/2;
			if (st_swFaceY3[bMode]<=0)
				swPos2=(st_swFaceY1[bMode]+pWin->wHeight)/2;
			else
				swPos2=(st_swFaceY1[bMode]+st_swFaceY3[bMode])/2;
			if (st_swFacexCurPos[bMode]!=swPos1)
			{
				swRet=SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
				SearchRightFiberBottomEdge(pWin,bMode);
				//SearchRightFiberCenter(pWin,bMode);
				if (swRet!=FAIL)
				{
						MoveVMotorToSpecPosition(bMode,swPos2,swPos1);
				}
				TimerToFillReferWin(10,RPOC_WIN0);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H2A OK  %d - %d",swPos2,swPos1);
				g_bDisplayMode=0x81;
				st_dwProcState++;
				MotorSetStatus(GetMotoIdxByFiberIdx(MOTOR_RIGHT_BOTTOM),MOTOR_HOLD);
				TimerToFillReferWin(10,RPOC_WIN1);
			}
			break;

		case SENSOR_ALIGN_H2B:
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			bMode=MOTOR_RIGHT_BOTTOM;
			//swPos1=st_swFaceY2[bMode-1];
				mpDebugPrint(" LEFT  UP%d - DOWN%d",st_swFaceY1[bMode-1],st_swFaceY3[bMode-1]);
				mpDebugPrint(" RIGHT %d - %d",st_swFaceY1[bMode],st_swFaceY3[bMode]);
			swPos1=(st_swFaceY1[bMode-1]+st_swFaceY3[bMode-1])/2;
			swPos2=(st_swFaceY1[bMode]+st_swFaceY3[bMode])/2;
			if (st_swFacexCurPos[bMode]!=swPos1)
			{
				SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
				swRet=SearchRightFiberBottomEdge(pWin,bMode);
				//swRet=SearchRightFiberCenter(pWin,bMode);
				if (swRet!=FAIL)
				{
					MoveVMotorToSpecPosition(bMode,swPos2,swPos1);
				}
				TimerToFillReferWin(10,RPOC_WIN1);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H2B OK %d - %d",swPos2,swPos1);
				g_bDisplayMode=0x80;
				st_bRedo--;
				if (st_bRedo)
				{
					st_dwProcState=SENSOR_ALIGN_H2A;
					TimerToFillReferWin(10,RPOC_WIN0);
				}
				else
				{
					st_dwProcState=SENSOR_ALIGN_H3B+1;
					//Weld_StartPause();
					TimerToFillReferWin(10,RPOC_WIN0);
				}
			}
			break;

		case SENSOR_ALIGN_H3A:
			bMode=MOTOR_RIGHT_TOP;
			if ((st_swFaceY20[bMode]>>1)!=(st_swFaceY20[MOTOR_LEFT_TOP]>>1))
			{
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				swRet=SearchRightFiberCore(pWin,bMode);
				if (swRet!=FAIL)
				{
						MoveVMotorToSpecPosition(bMode,st_swFaceY20[bMode]>>1,st_swFaceY20[MOTOR_LEFT_TOP]>>1);
				}
				TimerToFillReferWin(10,RPOC_WIN0);
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H3A OK");
				st_dwProcState++;
				g_bDisplayMode=0x81;
				TimerToFillReferWin(10,RPOC_WIN1);
			}
			break;

		case SENSOR_ALIGN_H3B:
			bMode=MOTOR_RIGHT_BOTTOM;
			if ((st_swFaceY20[bMode]>>1)!=(st_swFaceY20[MOTOR_LEFT_BOTTOM]>>1))
			{
				pWin=(ST_IMGWIN *)&SensorInWin[1];
				swRet=SearchRightFiberCore(pWin,bMode);
				if (swRet!=FAIL)
				{
						MoveVMotorToSpecPosition(bMode,st_swFaceY20[bMode]>>1,st_swFaceY20[MOTOR_LEFT_BOTTOM]>>1);
				}
				TimerToFillReferWin(10,RPOC_WIN1);
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H3B OK");
				st_bRedo--;
				if (st_bRedo)
				{
					st_dwProcState=SENSOR_ALIGN_H3A;
					TimerToFillReferWin(10,RPOC_WIN0);
				}
				else
				{
					st_dwProcState=SENSOR_ALIGN_H3B+1;
					Weld_StartPause();
				}
				//if (g_psSetupMenu->bPingXianFangShi<4)
				//	g_bDisplayMode=0x80|g_psSetupMenu->bPingXianFangShi;
				#if 0
				OsdLineInit();
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				bMode=MOTOR_LEFT_TOP;
				SearchLeftFiberCore(pWin,bMode);
				bMode=MOTOR_RIGHT_TOP;
				SearchRightFiberCore(pWin,bMode);

				pWin=(ST_IMGWIN *)&SensorInWin[1];
				bMode=MOTOR_LEFT_BOTTOM;
				SearchLeftFiberCore(pWin,bMode);
				bMode=MOTOR_RIGHT_BOTTOM;
				SearchRightFiberCore(pWin,bMode);
				#endif
				//xpgDelay(1000);
			}
			break;


		case SENSOR_DISCHARGE2:
			if (0)//(st_wDischargeTimeSet!=2)
			{
				st_wDischargeTimeSet=2;
				Discharge(st_wDischargeTimeSet,0);
				Ui_TimerProcAdd(3000, TimerToNextState);
			}
			else
			{
				st_dwProcState++;
				EventSet(UI_EVENT, EVENT_PROC_DATA);
			}
			break;

		case SENSOR_DISCHARGE3:
			if (st_wDischargeTimeSet!=3)
			{
				DriveMotor(01,1,1500,4); // 1600 
				st_wDischargeTimeSet=3;
				Discharge(st_wDischargeTimeSet,0);
				//xpgDelay(10);
				//DriveMotor(01,1,10,10);
				Ui_TimerProcAdd(6000, TimerToNextState);
			}
			else
			{
				st_dwProcState++;
				EventSet(UI_EVENT, EVENT_PROC_DATA);
			}
			break;

		case SENSOR_GET_LOSS:
			SendWeldStaus(1);
			st_dwProcState=SENSOR_IDLE;
			Ui_TimerProcAdd(2000, TimerToReleaseAllHold);
			/*
			bMode=MOTOR_LEFT_TOP;
			SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
			SearchLeftFiberBottomEdge(pWin,bMode);
			SearchLeftFiberCenter(pWin,bMode);
			SearchLeftFiberCore(pWin,MOTOR_LEFT_TOP);
			*/
			break;


		case SENSOR_IDLE:
#if 1//TEST_PLANE
		if (g_bDisplayMode < 0x02)
		{
			if (g_bDisplayMode==1)
				pWin=(ST_IMGWIN *)&SensorInWin[1];
#if 1
	//pWin=(ST_IMGWIN *)Idu_GetCurrWin();
	bMode=MOTOR_LEFT_TOP;
	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
	SearchLeftFiberBottomEdge(pWin,bMode);
	//SearchLeftFiberCenter(pWin,bMode);
	//SearchLeftFiberCore(pWin,bMode);
#endif
#if 0
	bMode=MOTOR_RIGHT_TOP;
	SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
	SearchRightFiberBottomEdge(pWin,bMode);
	//SearchRightFiberCenter(pWin,bMode);
	//SearchRightFiberCore(pWin,bMode);
#endif
	//pWin=(ST_IMGWIN *)&SensorInWin[1];
#if 0
	bMode=MOTOR_LEFT_BOTTOM;
	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
	SearchLeftFiberBottomEdge(pWin,bMode);
	SearchLeftFiberCenter(pWin,bMode);
	SearchLeftFiberCore(pWin,bMode);
#endif
#if 0
	bMode=MOTOR_RIGHT_BOTTOM;
	SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
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

void Proc_SensorData_State()
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet,swPos1,swPos2;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下

	//mpDebugPrint("P %p %d %p",g_psSetupMenu->bBackGroundLevel[1],st_bInProcWin,st_bNeedFillProcWin);
	if (!g_psSetupMenu->bBackGroundLevel[1])
		return ;
	if (st_bInProcWin || st_bNeedFillProcWin)
	{
		EventSet(UI_EVENT, EVENT_PROC_DATA);
		return;
	}
	st_bInProcWin=1;

	if (g_dwProcWinFlag&WIN0_CAPTURE_FLAG)
	{
		memset((BYTE *)&st_WeldStatus,0,sizeof (st_WeldStatus));
		Weld_CaptureFile((ST_IMGWIN *)&SensorInWin[0]);
		g_dwProcWinFlag&=~WIN0_CAPTURE_FLAG;

		//--test code
		BYTE pbTitle[12];
		STWELDSTATUS WeldStatus;
		FileBrowserResetFileList(); /* reset old file list first */
		FileBrowserScanFileList(SEARCH_TYPE);
		FileListSetCurIndex(FileBrowserGetTotalFile()-1);
		Weld_ReadFileWeldInfo(NULL,pbTitle,&WeldStatus);
	}

	Proc_Weld_State();
	Proc_GetFiberLowPoint();

	st_bInProcWin=0;

}

void TSPI_DataProc(void)
{
	BYTE i,bChecksum,index;
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey,dwTmpData,dwLen;
	
	if (!st_dwTspiRxIndex)
		return;
	if (pbTspiRxBuffer[1])
	{
		for (i=0;i<st_dwTspiRxIndex;i++)
			mpDebugPrintN(" %02x ",pbTspiRxBuffer[i]);
		mpDebugPrint("");
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
		mpDebugPrintN(" checksum error really %p data %p ",bChecksum,pbTspiRxBuffer[st_dwTspiRxIndex-1]);
		return;
	}

	switch (pbTspiRxBuffer[0])
	{
		case 0xb1:
			xpgCb_AutoPowerOff(g_psSetupMenu->bAutoShutdown,g_psSetupMenu->wShutdownTime);
			if (pbTspiRxBuffer[3])
			{
				switch (pbTspiRxBuffer[2])
				{
					case 1://refresh
						#if SHOW_CENTER
						//DriveMotor(01,1,1000,8);//RIGHT_DOWN_FIBER  DOWN
						st_swAFStep=2;
						ProcAFmotorAction();
						//PrintWinData();
						#elif TEST_PLANE
						if (st_bAdjustMotoStep)
						{
							if (st_bToolMotoIndex<5)
								st_wMotoStep[st_bToolMotoIndex-1]-=10;
							else if (st_bToolMotoIndex==6)
								st_wMotoStep[st_bToolMotoIndex-1]-=100;
							else
								st_wMotoStep[st_bToolMotoIndex-1]--;
							sprintf(st_bStrInfo, "Moto[%d]=%d", st_bToolMotoIndex,st_wMotoStep[st_bToolMotoIndex-1]);
							xpgForceUpdateOsd();
						}
						else
						DriveMotor(st_bToolMotoIndex,0,st_wMotoStep[st_bToolMotoIndex-1],8);
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
						#elif TEST_PLANE
						if (st_bAdjustMotoStep)
						{
							if (st_bToolMotoIndex<5)
								st_wMotoStep[st_bToolMotoIndex-1]+=10;
							else if (st_bToolMotoIndex==6)
								st_wMotoStep[st_bToolMotoIndex-1]+=100;
							else
								st_wMotoStep[st_bToolMotoIndex-1]++;
							sprintf(st_bStrInfo, "Moto[%d]=%d", st_bToolMotoIndex,st_wMotoStep[st_bToolMotoIndex-1]);
							xpgForceUpdateOsd();
						}
						else
							DriveMotor(st_bToolMotoIndex,1,st_wMotoStep[st_bToolMotoIndex-1],8);
						#else
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
							Weld_StartPause();
						#endif
						break;
					case 3: // main
						#if SHOW_CENTER
						index=st_bAFMotoIndex;
						if (st_bMotorHold&(1<<index))
						{
							MotorSetStatus(index,MOTOR_NO_HOLD);
							st_bMotorHold &= ~(1<<index);
						}
						if (st_bAFMotoIndex==5)
							st_bAFMotoIndex=6;
						else
							st_bAFMotoIndex=5;
						#elif TEST_PLANE
						index=st_bToolMotoIndex;
						if (st_bMotorHold&(1<<index))
						{
							MotorSetStatus(index,MOTOR_NO_HOLD);
							st_bMotorHold &= ~(1<<index);
						}
						st_bToolMotoIndex++;
						if (st_bToolMotoIndex>MOTO_ADJ_NUM)
							st_bToolMotoIndex=1;
						//mpDebugPrint("st_bToolMotoIndex=%d",st_bToolMotoIndex);
						if (st_bAdjustMotoStep)
						{
							sprintf(st_bStrInfo, "Moto[%d]=%d", st_bToolMotoIndex,st_wMotoStep[st_bToolMotoIndex-1]);
						}
						else
						{
							sprintf(st_bStrInfo, "Moto:%d", st_bToolMotoIndex);
						}
						xpgForceUpdateOsd();
						#else
						if (dwHashKey == xpgHash("Main") )
						{
							WeldModeSet(0);
							xpgCb_EnterCamcoderPreview();
						}
						else
						{
							Idu_OsdErase();
							xpgPreactionAndGotoPage("Main");
							xpgUpdateStage();
						}
						#endif
						break;

					case 4:// X/Y
						#if  SHOW_CENTER
						index=st_bAFMotoIndex;
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
						#elif TEST_PLANE
						/*
						index=st_bToolMotoIndex;
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
						*/
						//g_psSetupMenu->bBackGroundLevel[bSensorIndex] =0;
						//Ui_TimerProcAdd(100, GetBackgroundLevel);
						#else
						if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
						{
							if (g_bDisplayMode<3)
								g_bDisplayMode++;
							else
								g_bDisplayMode=0;
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
						#elif SHOW_CENTER||TEST_PLANE||TEST_TWO_LED
						if (st_bAdjustMotoStep)
						{
							//st_wDischargeTimeSet=3;
							//Discharge(st_wDischargeTimeSet,0);
							MotorSetStatus(1,MOTOR_HOLD);
							DriveMotor(01,1,760,8); // 60
							st_bWaitMotoStop=1;
						}
						else
						{
							if (g_bDisplayMode<3)
								g_bDisplayMode++;
							else
								g_bDisplayMode=0;
							g_bDisplayMode |= 0x80;
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
						if (st_bAutoDischarge)
						{
							st_bAutoDischarge=0;
							Ui_TimerProcRemove(AutoDischarge);
						}
						else
						{
							st_bAutoDischarge=20;
							st_wDischargeTimeSet=600;
							AutoDischarge();
						}
						#endif
						break;
					case 6:
						#if 0
						MotorSetStatus(2,MOTOR_NO_HOLD);
						DriveMotor(02,1,2000,8);//RIGHT_UP_FIBER UP
						//g_bDisplayMode=(g_bDisplayMode+1)|0x80;
						#elif SHOW_CENTER||TEST_PLANE
						//st_dwProcState=SENSOR_IDLE;
						//ResetMotor();
						//st_wDischargeTimeSet=1;
						//Discharge(st_wDischargeTimeSet,0);
						if (st_bAdjustMotoStep)
						{
							st_bAdjustMotoStep=0;
							sprintf(st_bStrInfo, "Moto:%d", st_bToolMotoIndex);
						}
						else
						{
							st_bAdjustMotoStep=1;
							sprintf(st_bStrInfo, "Moto[%d]=%d", st_bToolMotoIndex,st_wMotoStep[st_bToolMotoIndex-1]);
						}
						xpgForceUpdateOsd();

						#elif 1
						//AutoStartWeld();
						//TimerToFillProcWin(10);
						if (st_dwGetCenterState)
							st_dwGetCenterState=0;
						else
							AutoGetFiberLowPoint();
						#endif
						break;


					default:
						break;
				}
    			//AddAutoEnterPreview();
			}
			break;
//马达状态
		case 0xb2:
			st_bMotorStaus=pbTspiRxBuffer[2];
			#if TEST_PLANE
			if (st_bWaitMotoStop && !(st_bMotorStaus&(1<<MOTOR_LEFT_TOP)))
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
				case 1:
					if (pbTspiRxBuffer[2]&BIT0)// 盖子打开
					{
						WeldStopAllAction();
						// 马达复位
						ResetMotor();
					}
					else
					{
						//WeldModeSet(0);
						//xpgCb_EnterCamcoderPreview();
					}
					break;

				case 2: //加热盖子状态
				break;

				case 3:
					if (pbTspiRxBuffer[2]&BIT0)
					{
						g_bOPMonline=1;
					}
					else
					{
						g_bOPMonline=0;
					}
					break;


				default:
					break;
			}
			
			break;
		//放电状态
		case 0xb4:
			if (pbTspiRxBuffer[2]==0x01)
			{
				if (st_dwProcState==SENSOR_DISCHARGE1 || st_dwProcState==SENSOR_DISCHARGE2 ||st_dwProcState==SENSOR_DISCHARGE3)
				{
					Ui_TimerProcRemove(TimerToNextState);
					st_dwProcState++;//=SENSOR_FACE_POS2A;
					EventSet(UI_EVENT, EVENT_PROC_DATA);
					st_wDiachargeRunTime=0;
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
						TimerToFillReferWin(1000,RPOC_WIN0);
					}
					else if (st_dwGetCenterState==GET_CENTER_LOW_POINT)
					{
						TimerToFillReferWin(1000,RPOC_WIN0);
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
				//马达复位完成
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
				st_dwTspiTxMaxLen=((DWORD)pbTspiRxBuffer[3]<<24)|((DWORD)pbTspiRxBuffer[4]<<16)|((DWORD)pbTspiRxBuffer[5]<<8)|((DWORD)pbTspiRxBuffer[6]);
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
					WORD wIndex=((WORD)pbTspiRxBuffer[3]<<8)|pbTspiRxBuffer[4];
					DWORD dwRtc;
					STWELDSTATUS WeldStatus;
					ST_SYSTEM_TIME time;

					if (wIndex<FileBrowserGetTotalFile())
					{
						FileListSetCurIndex(wIndex);
						Weld_ReadFileWeldInfo(NULL,&pbData[9],&pbData[20]);
						//--head
						pbData[0]=0xa6;
						pbData[1]=3+23;
						pbData[2]=wIndex;
						//--time 3-8
						if (Weld_FileNameToTime(wIndex,&dwRtc)!=PASS)
							dwRtc=0;
    					SystemTimeSecToDateConv(dwRtc, &time);
						pbData[3]=time.u16Year-2000;
						pbData[4]=time.u08Month;
						pbData[5]=time.u08Day;
						pbData[6]=time.u08Hour;
						pbData[7]=time.u08Minute;
						pbData[8]=time.u08Second;
						//--titel 9-19
						pbData[19]=0;
						//--WeldStatus 20-24
						TSPI_PacketSend(pbData,0);
					}
				}
				else if (pbTspiRxBuffer[2]==2)//4  查询熔接图片  
				{
					Weld_SendFileInit(((WORD)pbTspiRxBuffer[3]<<8)|pbTspiRxBuffer[4]);
				}
			}
			break;

		//设备信息数据传输
		case 0xbc:
			if (dwLen>9)
			{
				for (i=0;i<6;i++)
				{
					g_psSetupMenu->bMADarry[i]=pbTspiRxBuffer[2+i];
				}
				WriteSetupChg();
				CheckAndWriteFile( DriveGet(DriveCurIdGet()),QRCODE_FILE_NAME,QRCODE_FILE_EXT,&pbTspiRxBuffer[8],dwLen-9);//NAND
			}
			break;

		//环境亮度
		case 0xbd:
			if (g_psSetupMenu->bSmartBacklight)
			{
				if (pbTspiRxBuffer[2] && pbTspiRxBuffer[2]<6)
				{
					BYTE bPwmArry[5]={10,30,50,70,90};
					TimerPwmEnable(2, 240000, bPwmArry[pbTspiRxBuffer[2]-1]);
				}
			}
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
#if 0
		//指令回复
		case 0xbf:
			if (st_bTspiTxRetry && pbTspiRxBuffer[2]==0x01)//0x00->接收错误 0x01->接收成功
			{
				st_bTspiTxRetry=0;
				Ui_TimerProcRemove(TSPI_TimerToResend);
			}
			break;
#endif


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
			dwTmpData=(pbTspiRxBuffer[2]<<8)|pbTspiRxBuffer[3];
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
				g_psUnsaveParam->wRedPenTime=pbTspiRxBuffer[4];
			}
			else
				g_psUnsaveParam->bRedPenTimerEnable=0;
			if (dwHashKey == xpgHash("RedLight"))
				xpgUpdateStage();
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

void DisplaySensorImage(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
//	BYTE bCacheWinIndex=0;


	if (g_bDisplayMode&0xf0)
	{
		Sensor_DisplayMode_Set();
		ResetFiberPara();
	}

	switch (g_bDisplayMode)
	{
		case 0:
			if (Sensor_CurChannel_Get()==0)
			{
				Idu_ChgWin(Idu_GetNextWin());
				pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#endif
			}
			break;

		case 1:
			if (Sensor_CurChannel_Get()==1)
			{
				Idu_ChgWin(Idu_GetNextWin());
				pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
#endif
			}
			break;

		case 2:
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset*SensorWindow_Height;	
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset*SensorWindow_Height;	
#endif
				
			}
			else
			{
					Sensor_Channel_Set(0);
				{
					Idu_ChgWin(Idu_GetNextWin());
					pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
				}
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
				pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
				ipu->Ipu_reg_A3 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);
#else
				ipu->Ipu_reg_F2 = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);	
#endif
				
			}
			else
			{
					Sensor_Channel_Set(0);
					Idu_ChgWin(Idu_GetNextWin());
					pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
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

}

static BYTE st_bFrameIndex=0,st_bDispOff=0;
void DisplaySensorOnCurrWin( BYTE bIpw2)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
	register DWORD *pIpwAddr;

	if (bIpw2)
		pIpwAddr=(DWORD *)&ipu->Ipu_reg_F2;
	else
		pIpwAddr=(DWORD *)&ipu->Ipu_reg_A3;
	if (st_bNeedFillProcWin)
	{
		*pIpwAddr= ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);
		return;
	}

	if (g_bDisplayMode&0xf0)
	{
		Sensor_DisplayMode_Set();
		ResetFiberPara();
		Idu_OsdErase();
		#if TEST_PLANE||TEST_TWO_LED
		TimerToFillProcWin(10);
		#endif
	}

#if 1
//    mpDebugPrintN("  %08x %08x  ", ipu->Ipu_reg_10A,ipu->Ipu_reg_10B);
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
				if (st_bFrameIndex==1)
				{
					//mpDebugPrintN("A %08x ", ipu->Ipu_reg_10A);
					if ((ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
					{
						Sensor_Channel_Set(0);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
					}
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
					if ((ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
					{
						Sensor_Channel_Set(1);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+pDstWin->dwOffset*SensorWindow_Height;	
					}
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
					if ((ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
					{
						Sensor_Channel_Set(0);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);	
					}
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
					if ((ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
					{
						Sensor_Channel_Set(1);
						st_bFrameIndex=0;
						pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
						*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1)+(SensorWindow_Width<<1);	
					}
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
	else if (st_bDispOff && (ipu->Ipu_reg_10A>>16)==pDstWin->wHeight)
	{
		*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+DisplayWindowOffset_Get();
		st_bDispOff=0;
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

void CacheSensorData( BYTE bIpw2)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)&SensorInWin[0];
	BYTE bCacheWinIndex=0xff;
	DWORD *pIpwAddr;


	if (st_bNeedFillProcWin)
	{
		if (bIpw2)
			pIpwAddr=(DWORD *)&ipu->Ipu_reg_F2;
		else
			pIpwAddr=(DWORD *)&ipu->Ipu_reg_A3;

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
					bCacheWinIndex=2;
				}
				else
				{
					bCacheWinIndex=1;
				}
		}
		else if (st_bNeedFillProcWin&FILL_WIN_DOWN)
		{
				bCacheWinIndex=2;
		}
		else if (st_bNeedFillProcWin&FILL_WIN_UP)
		{
				bCacheWinIndex=1;
		}
		else// if ((st_bNeedFillProcWin&0x20)==0x20)
		{
			//mpDebugPrint("cache end");
			//ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);	
			//mpCopyWinAreaSameSize((ST_IMGWIN *)&SensorInWin[0],(ST_IMGWIN *)Idu_GetCurrWin(),0, 0, 0, 0,pDstWin->wWidth, pDstWin->wHeight);
			//xpgDelay(1000);
			if (st_bBackupChanel<2 && st_bBackupChanel!=Sensor_CurChannel_Get())
			{
				Sensor_Channel_Set(st_bBackupChanel);
				ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);
				st_bDispOff=1;
			}
			else
			{
				ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetCurrWin()->pdwStart)| 0xA0000000)+DisplayWindowOffset_Get();
			}
			ipu->Ipu_reg_A3 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);	

			st_bBackupChanel=0xff;
			st_bNeedFillProcWin=0;
			bCacheWinIndex=0;
			//EventSet(UI_EVENT, EVENT_PROC_DATA);
			Ui_TimerProcAdd(10, TimerToProcWin);
		}
		//else if (st_bNeedFillProcWin)
		//	st_bNeedFillProcWin=0;

		if (bCacheWinIndex==1) // up win
		{
			if (Sensor_CurChannel_Get() !=0)
				Sensor_Channel_Set(0);
			st_bNeedFillProcWin &=~FILL_WIN_UP;
			st_bNeedFillProcWin |=FILL_WIN_UP_WAIT;
			*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
		}
		else if (bCacheWinIndex==2) // down win
		{
			if (Sensor_CurChannel_Get() !=1)
				Sensor_Channel_Set(1);
			st_bNeedFillProcWin &=~FILL_WIN_DOWN;
			st_bNeedFillProcWin |=FILL_WIN_DOWN_WAIT;

			if (st_bCacheWinNum==2)
			{
				pDstWin=(ST_IMGWIN *)&SensorInWin[1];
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
			}
			else
			{
				*pIpwAddr  = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->wWidth*pDstWin->wHeight;
			}
		}

		if (st_bNeedFillProcWin)
		{
			if (bIpw2)
				ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
		}
	}
	if (!bIpw2) //  IPW1 must open ,because it is IPW2`s  base
			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1

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
#elif 0
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


#if 0
#elif 1
	if (!g_bDisplayUseIpw2)
		DisplaySensorOnCurrWin(0);
	else
		CacheSensorData(0);
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

void ShowOSDstring(void)
{
	ST_OSDWIN * psWin=Idu_GetOsdWin();
	ST_IMGWIN *pWin=(ST_IMGWIN *)Idu_GetNextWin();
		
#if TEST_PLANE
	if (st_bStrInfo[0])
	{
		Idu_OSDPrint(psWin,st_bStrInfo, 8, 8, OSD_COLOR_RED);
	}

	if (g_bDisplayMode==0x00||g_bDisplayMode==0x01)
	{
		//--face
		if (st_bStrFace[MOTOR_LEFT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrFace[MOTOR_LEFT_TOP][0], pWin->wWidth/2-80, 48, OSD_COLOR_RED);
		}
		if (st_bStrFace[MOTOR_RIGHT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrFace[MOTOR_RIGHT_TOP][0], pWin->wWidth/2+40, 48, OSD_COLOR_RED);
		}
		//--水平度
		if (st_bStrHLevel[MOTOR_LEFT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrHLevel[MOTOR_LEFT_TOP][0], 8, 48, OSD_COLOR_RED);//  90
		}
		if (st_bStrHLevel[MOTOR_RIGHT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrHLevel[MOTOR_RIGHT_TOP][0], pWin->wWidth-200, 48, OSD_COLOR_RED);
		}
		//--center
		if (st_bStrCenter[MOTOR_LEFT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrCenter[MOTOR_LEFT_TOP][0], 8, (st_swFaceY3[MOTOR_LEFT_TOP]+st_swFaceY1[MOTOR_LEFT_TOP])>>1, OSD_COLOR_RED);
		}
		if (st_bStrCenter[MOTOR_RIGHT_TOP][0])
		{
			Idu_OSDPrint(psWin,&st_bStrCenter[MOTOR_RIGHT_TOP][0], pWin->wWidth-80, (st_swFaceY3[MOTOR_RIGHT_TOP]+st_swFaceY1[MOTOR_RIGHT_TOP])>>1, OSD_COLOR_RED);
		}
		//--core
		if (st_bStrCore[MOTOR_LEFT_TOP][0])
		{
			//Idu_OSDPrint(psWin,&st_bStrCore[MOTOR_LEFT_TOP][0], 120,(st_swFaceY20[MOTOR_LEFT_TOP]>>1)-10, OSD_COLOR_RED);
			Idu_OSDPrint(psWin,&st_bStrCore[MOTOR_LEFT_TOP][0], 120,st_swFaceY2[MOTOR_LEFT_TOP]-10, OSD_COLOR_RED);
		}
		if (st_bStrCore[MOTOR_RIGHT_TOP][0])
		{
			//Idu_OSDPrint(psWin,&st_bStrCore[MOTOR_RIGHT_TOP][0], pWin->wWidth/2+40, (st_swFaceY20[MOTOR_RIGHT_TOP]>>1)-10, OSD_COLOR_RED);
			Idu_OSDPrint(psWin,&st_bStrCore[MOTOR_RIGHT_TOP][0], pWin->wWidth/2+40, st_swFaceY2[MOTOR_RIGHT_TOP]-10, OSD_COLOR_RED);
		}

	}



#endif
}

void WeldStopAllAction(void)
{
	DWORD i;

	st_dwGetCenterState = GET_CENTER_OFF;
	st_dwProcState = SENSOR_IDLE;
	//--stop moto
	for (i=0;i<MOTOR_NUM;i++)
		MotorSetStatus(i+1,MOTOR_STOP);
	//--stop discharge
	if (st_wDiachargeRunTime)
	{
		Discharge(0,0);
	}
	st_bAutoDischarge=0;
	st_wDischargeTimeSet=0;
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
	if (g_psSetupMenu->bPingXianFangShi<4)
		g_bDisplayMode=0x80|g_psSetupMenu->bPingXianFangShi;
	else
	{
		g_psSetupMenu->bPingXianFangShi=2;
		g_bDisplayMode=0x82;
		WriteSetupChg();
	}
#endif

#if TEST_PLANE
	for (i=0;i<MOTOR_NUM;i++)
	{
		st_bStrFace[i][0]=0;
		st_bStrCenter[i][0]=0;
		st_bStrCore[i][0]=0;
		st_bStrHLevel[i][0]=0;
	}
	//st_bStrInfo[0]=0;
	sprintf(st_bStrInfo, "Moto:%d", st_bToolMotoIndex);
#endif

//--查询MCU可接收的最长指令长度
	SendCmdA4GetStaus(0x0a);

//--读取电极棒位置
	if (g_psSetupMenu->wElectrodePos[0]>0  && g_psSetupMenu->wElectrodePos[0]<pWin->wWidth)
	{
		g_wElectrodePos[0]=g_psSetupMenu->wElectrodePos[0];
		//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,18,g_wElectrodePos[0],0,2,pWin->wHeight/2,OSD_COLOR_RED);
	}
	if (g_psSetupMenu->wElectrodePos[1]>0  && g_psSetupMenu->wElectrodePos[1]<pWin->wWidth)
	{
		g_wElectrodePos[1]=g_psSetupMenu->wElectrodePos[1];
		//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,19,g_wElectrodePos[1],pWin->wHeight/2,2,pWin->wHeight,OSD_COLOR_RED);
	}
	MP_DEBUG("POS  (%d %d)",g_wElectrodePos[0],g_wElectrodePos[1]);
	//WriteSetupChg();
	xpgCb_AutoPowerOff(g_psSetupMenu->bAutoShutdown,g_psSetupMenu->wShutdownTime);

	//ResetMotor();
	if ((g_bDisplayMode&0x0f)==0)
		st_bFillWinFlag=FILL_WIN_UP;
	else if ((g_bDisplayMode&0x0f)==1)
		st_bFillWinFlag=FILL_WIN_DOWN;
	if (!g_psSetupMenu->bBackGroundLevel[0]||!g_psSetupMenu->bBackGroundLevel[1])
	{
		Ui_TimerProcAdd(3000, SetFillProcWinFlag);
		Ui_TimerProcAdd(3100, GetBackgroundLevel);
	}
}

#endif
#endif



