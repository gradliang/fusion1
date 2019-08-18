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

#if (SENSOR_ENABLE == ENABLE)
#if SENSOR_WIN_NUM
extern BYTE *pbSensorWinBuffer;
extern ST_IMGWIN SensorInWin[SENSOR_WIN_NUM];

static BYTE  st_bRetryTimes=0,st_bDiachargeTimes=0;
SWORD g_swGetCenterState = GET_CENTER_OFF;
SWORD g_swProcState = SENSOR_IDLE;//SENSOR_FACE_POS1A;//SENSOR_IDLE;
WORD g_wElectrodePos[2]={400,400};  //4 电极棒位置
//SWORD g_swCenterOffset=0;  //4 电极棒位置  76

static SWORD st_swFaceX[MOTOR_NUM]={-1,-1,-1,-1};
static SWORD st_swFacexCurPos[MOTOR_NUM]={-1,-1,-1,-1},st_swLastPos[MOTOR_NUM]={-1,-1,-1,-1};
static SWORD st_swFaceY1[MOTOR_NUM]={-1,-1,-1,-1},st_swFaceY2[MOTOR_NUM]={-1,-1,-1,-1},st_swFaceY20[MOTOR_NUM]={-1,-1,-1,-1},st_swFaceY3[MOTOR_NUM]={-1,-1,-1,-1}; //4  上中下  
static BYTE st_bDirectionArry[MOTOR_NUM],st_bMotorBaseStep[4]={15,15,30,30},st_bBaseRetryArry[MOTOR_NUM],st_bMotorHold=0,st_bMotorStaus=0;// 0->STOP 1->RUN BIT0
static BYTE st_bTopVMotorUpValue=0,st_bBottomVMotorUpValue=0,st_bLastProcStep[2]={0},st_bVmotorMoveTimes[2]={0};
static WORD st_wVmotorMoveValue[2][VMOTOR_CNT]; // 0->up motor  1->down motor
static BYTE st_bDischargeMode=0; // 0->off 

BYTE g_bDisplayUseIpw2=1; // 0->ipw1  1->ipw2

#endif


#if TSPI_ENBALE
#define  TX_BUFFER_LENTH								16


#define  RX_HEAD_FLAG1									0x55
#define  RX_HEAD_FLAG2									0x55
#define  RX_HEAD_FLAG3									0xee
#define  RX_BUFFER_LENTH								16


#define SPI_CLK_GPIO                			GPIO_UGPIO_0 //pin80 
#define SPI_DOUT_GPIO                			GPIO_KGPIO_1 //pin119
#define SPI_DIN_GPIO                			GPIO_KGPIO_0 //pin118
#define SPI_RX_LEN                				16
#define SPI_WAIT_START                		10000

#define TSPI_CLK_Low 		Gpio_Config2GpioFunc(SPI_CLK_GPIO, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1)
#define TSPI_CLK_High 	Gpio_Config2GpioFunc(SPI_CLK_GPIO, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1)
#define TSPI_DOUT_Low 		Gpio_Config2GpioFunc(SPI_DOUT_GPIO, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1)
#define TSPI_DOUT_High 	Gpio_Config2GpioFunc(SPI_DOUT_GPIO, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1)
#define TSPI_DIN_Low 		Gpio_Config2GpioFunc(SPI_DIN_GPIO, GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1)
#define TSPI_DIN_High 	Gpio_Config2GpioFunc(SPI_DIN_GPIO, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1)

static BYTE st_bTspiRxIndex=0,st_bTspiRxArry[RX_BUFFER_LENTH]={0},st_bTspiBusy=0,st_bTspiAbnormal=0;
static BYTE st_bTspiTxArry[RX_BUFFER_LENTH]={0},st_bTspiTxRetry=0;


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
		return swRet;
}

void TSPI_TimerToResend(void)
{

	if (TSPI_Send(st_bTspiTxArry,st_bTspiTxArry[1])!=PASS)
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

SWORD TSPI_SendWithCheck(BYTE *pbDataBuf,DWORD dwLenth )
{
	BYTE i;
	SWORD swRet;

	swRet=TSPI_Send(pbDataBuf,pbDataBuf[1]);
	if (swRet!=PASS)
	{
		st_bTspiTxRetry=1;
		for (i=0;i<dwLenth;i++)
			st_bTspiTxArry[i]=pbDataBuf[i];
		TSPI_Reset();
		Ui_TimerProcAdd(st_bTspiTxRetry*10, TSPI_TimerToResend);
	}
	else
		st_bTspiTxRetry=0;

	return swRet;
}

SWORD TSPI_PacketSend(BYTE *pbDataBuf,DWORD dwLenth ,BYTE bCheckResend)
{
	BYTE i;
	
	if (!dwLenth)
		return FAIL;
	pbDataBuf[1]=dwLenth;
	dwLenth--;
	pbDataBuf[dwLenth]=pbDataBuf[0];
	for (i=1;i<dwLenth;i++)
		pbDataBuf[dwLenth]+=pbDataBuf[i];
	if (bCheckResend)
		return TSPI_SendWithCheck(pbDataBuf,pbDataBuf[1]);
	return TSPI_Send(pbDataBuf,pbDataBuf[1]);
}

BYTE TSPI_Receiver()
{
	BYTE i,k;
	SWORD swRet=PASS,swValue;

	if (Gpio_ValueGet(SPI_DIN_GPIO))
		return 0;
	if (!Gpio_ValueGet(SPI_DOUT_GPIO))
		return 0;

	//start ack
	st_bTspiBusy=1;
	TSPI_DOUT_Low;
	for (i=0;i<SPI_RX_LEN+1;i++)
	{
		st_bTspiRxArry[i]=0;
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
				if (i>=SPI_RX_LEN)
				{
					MP_DEBUG("TSPI overflow!");
					swRet=FAIL;
					break;
				}
			}
			st_bTspiRxArry[i]<<=1;
			if (swValue)
				st_bTspiRxArry[i] |=0x01;
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
	}

	if (swRet == 1)
	{
		st_bTspiRxIndex=i;
		//MP_DEBUG("OK!");
		TSPI_DataProc();
	}
	else
	{
		if (i || k)
			st_bTspiRxIndex=0;
		else
			st_bTspiAbnormal=1;
	}
	TSPI_Reset();
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
extern WORD SensorWindow_Width,SensorWindow_Height;
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
static BYTE st_bCacheWinNum=2,st_bNeedFillProcWin=0;  //   BIT7->new fill wait init IPW  BIT6->first get data BIT5->get data end   BIT1->fill down   BIT0->FILL UP  0->not need fill
static BYTE st_bBackGroundLevel=0;//FIBER_EDGE_LEVEL
static BYTE st_bFiberBlackLevel=0;
static WORD st_wFiberWidth=0;

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

void SetFillProcWinFlag(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
GetSetupMenuValue();
#if  1//PROC_SENSOR_DATA_MODE
		st_bNeedFillProcWin=0xe3; // bit0->up win  bit1->down win   bit7->init mode
		//st_bNeedFillProcWin=0xe1; // bit0->up win  bit1->down win   bit7->init mode
	#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
			ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
	#else
			ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
	#endif //--------------------------
#else
		st_bNeedFillProcWin=1;
#endif
}

void AutoStartWeld()
{
	BYTE i;
	
	if (g_swProcState==SENSOR_IDLE)
	{
		g_swProcState=SENSOR_FACE_POS1A;
		st_bDischargeMode=0;
		for (i=0;i<MOTOR_NUM;i++)
		{
			st_swFacexCurPos[i]=-1;
			st_swLastPos[i]=-1;
		}
		SetFillProcWinFlag();
	}
	else
		g_swProcState=SENSOR_IDLE;

}

void AutoGetFiberLowPoint()
{
	BYTE i;
	
	if (g_swGetCenterState==GET_CENTER_OFF)
	{
		g_swGetCenterState=GET_CENTER_INIT;//GET_CENTER_INIT;GET_CENTER_LOW_POINT
		SetFillProcWinFlag();
	}
	else
		g_swGetCenterState=GET_CENTER_OFF;

}


void GetBackgroundLevel(void)
{
	ST_IMGWIN *pWin;
	BYTE *pbWinBuffer,bValidLevel,bNewLevel,bContinue;
	WORD wValidCnt;
	SWORD y;
	static BYTE st_bRetryTimes=0;

	if (st_bNeedFillProcWin)
	{
		Ui_TimerProcAdd(500, GetBackgroundLevel);
		return;
	}
	pWin=(ST_IMGWIN *)&SensorInWin[0];

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

		if (wValidCnt>pWin->wHeight/16 &&  bValidLevel>0x30) // /8
		{
			//st_bBackGroundLevel=(DWORD)bValidLevel*7/10; // ok1
			st_bBackGroundLevel=(DWORD)bValidLevel*7/10; // 8
			break;
		}
	}

	if (st_bBackGroundLevel)
	{
		mpDebugPrint("---GetBackgroundLevel:bValidLevel %p->%p",bValidLevel,st_bBackGroundLevel);
		//DriveMotor(03,0,300,8);//RIGHT_DOWN_FIBER  DOWN
		//Ui_TimerProcAdd(2000, AutoStartWeld);
	}
	else
	{
		if (st_bRetryTimes<10)
		{
			mpDebugPrint("----bValidLevel=%p wValidCnt=%d ",bValidLevel,wValidCnt);
			st_bRetryTimes++;
			//DriveMotor(01,0,50,8);
			//DriveMotor(02,0,50,8);
			SetFillProcWinFlag();
			Ui_TimerProcAdd(600, GetBackgroundLevel);
		}
		else
		{
			st_bBackGroundLevel=FIBER_EDGE_LEVEL;
			mpDebugPrint("----st_bBackGroundLevel FAIL !!!reset to %p ",st_bBackGroundLevel);
		}
	}

}

SWORD SearchLeftFiberBottomEdge(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt;

	dwOffset=pWin->dwOffset;

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
		//mpDebugPrint(" %d: ",x);
		while (y>swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer < st_bBackGroundLevel)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < st_bBackGroundLevel )
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
								dwYValidStartAver>>=1;
							if (bMode>1)
								dwYValidStartAver+=pWin->wHeight/2;
							#endif
							if (st_swFaceY3[bMode]!=dwYValidStartAver)
							{
								st_swFaceY3[bMode]=dwYValidStartAver;
								mpDebugPrint(" st_swFaceY3[%d]=%d ",bMode,st_swFaceY3[bMode]);
								#if OSD_LINE_NUM
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
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
		//mpDebugPrint(" ");
	}
	
	return FAIL;
}

SWORD SearchLeftFiberCenter(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wContinueValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		mpDebugPrint("!!!LeftCenter error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	dwOffset=pWin->dwOffset;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/3;
#if (SENSOR_WIN_NUM==2)
		swYEnd=st_swFaceY3[bMode];
		if (bMode>1)
		{
			swStartY-=pWin->wHeight/2;
			swYEnd-=pWin->wHeight/2;
		}
		#if DISPLAY_IN_ONE_WIN
		swStartY<<=1;
		swYEnd<<=1;
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
				if (*pbWinBuffer > st_bBackGroundLevel)
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
				if (*pbWinBuffer > st_bBackGroundLevel )
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

			if (wValidPixelCnt>CENTER_MIN_PIXEL && wEndInvalidCnt>=Y_VALID_OFFSET && wValidPixelCnt>((wInvalidPixelCnt-wEndInvalidCnt)<<3))
			{
				if (wYStartIndex<X_VALID_SUM)
				{
					wYValidArry[wYStartIndex++]=y-wEndInvalidCnt-(wValidPixelCnt+wInvalidPixelCnt-wEndInvalidCnt)/2;
					//计算有效Y起始坐标
					if (wYStartIndex>=8)
					{
						dwYValidStartAver=0;
						for (i=0;i<wYStartIndex;i++)
							dwYValidStartAver+=wYValidArry[i];
						dwYValidStartAver/=wYStartIndex;
						wYvalidCnt=0;
						//mpDebugPrint("y: %02x  ",dwYValidStartAver);
						for (i=0;i<wYStartIndex;i++)
						{
							//mpDebugPrintN("%02x ",wYValidArry[i]);
							if ((dwYValidStartAver+Y_VALID_OFFSET>wYValidArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<wYValidArry[i]))
								wYvalidCnt++;
						}
						//mpDebugPrint(" %d ",wYvalidCnt);
						if (wYvalidCnt>wYStartIndex/2)
						{
							wYStartIndex=X_VALID_SUM;
							#if DISPLAY_IN_ONE_WIN
								dwYValidStartAver>>=1;
							if (bMode>1)
								dwYValidStartAver+=pWin->wHeight/2;
							#endif
							if (st_swFaceY2[bMode]!=dwYValidStartAver)
							{
								st_swFaceY2[bMode]=dwYValidStartAver;
								mpDebugPrint(" st_swFaceY2[%d]=%d ",bMode,st_swFaceY2[bMode]);
								#if OSD_LINE_NUM
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,8+bMode,0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_RED);
								#endif
								return ENABLE;
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

SDWORD GetFiberBlackLevel()
{
	DWORD dwOffset,dwYValidStartAver,dwValidlevelcnt,dwBlackAverLevel=0,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYStartIndex,wYvalidCnt;
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel,bMode=MOTOR_LEFT_TOP;
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0]; 

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		mpDebugPrint("!!!Left black error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	dwOffset=pWin->dwOffset;

	//--获取实际光纤边黑色部分的亮度值
	{
		swStartY=st_swFaceY1[bMode]+6;
		swYEnd=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/3;
		if (bMode>1)
		{
			swStartY-=pWin->wHeight/2;
			swYEnd-=pWin->wHeight/2;
		}
		#if DISPLAY_IN_ONE_WIN
		swStartY<<=1;
		swYEnd<<=1;
		#endif
		x=st_swFaceX[bMode]<<1;
		if (x>X_STEP*4)
			x-=X_STEP*4;
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
					if (*pbWinBuffer < st_bBackGroundLevel)
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
					if (*pbWinBuffer < st_bBackGroundLevel )
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
					dwBlackAverLevel=dwValidlevelcnt/((st_swFaceY3[bMode]-st_swFaceY1[bMode])/5);
					mpDebugPrint("Core B 0x%02x ",dwBlackAverLevel);
					break;
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

	return dwBlackAverLevel;
}

SWORD SearchLeftFiberCore(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver,dwValidlevelcnt,dwBlackAverLevel,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel;
	SDWORD sdwRet;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		mpDebugPrint("!!!Left core error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	dwOffset=pWin->dwOffset;

	//--获取实际光纤边黑色部分的亮度值
	if (!st_bFiberBlackLevel)
	{
		sdwRet=GetFiberBlackLevel();
		if (sdwRet>0)
			st_bFiberBlackLevel=sdwRet<<1;
		else
			return FAIL;
	}
	
	//--找纤芯上边沿
	BYTE bColorIndex=OSD_COLOR_WHITE;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	if (bMode>1)
	{
		swStartY-=pWin->wHeight/2;
		swYEnd-=pWin->wHeight/2;
	}
	#if DISPLAY_IN_ONE_WIN
	swStartY<<=1;
	swYEnd<<=1;
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
			if (*pbWinBuffer<st_bFiberBlackLevel)
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
		else if (wValidPixelCnt>10)
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
	if (bMode>1)
	{
		swStartY-=pWin->wHeight/2;
		swYEnd-=pWin->wHeight/2;
	}
	#if DISPLAY_IN_ONE_WIN
	swStartY<<=1;
	swYEnd<<=1;
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
			if (*pbWinBuffer<st_bFiberBlackLevel)
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
		else if (wValidPixelCnt>10)
		{
			wCoreYDown=GetReallyData(wYValidArry,wValidPixelCnt);
			
		}
		if (wCoreYDown>0)
			break;
		TaskYield();
		//mpDebugPrint(" ");
	}

	if (wCoreYUp&&wCoreYDown)
	{
		st_swFaceY20[bMode]=(wCoreYUp+wCoreYDown)/2;
		#if DISPLAY_IN_ONE_WIN
			st_swFaceY20[bMode]>>=1;
		if (bMode>1)
			st_swFaceY20[bMode]+=pWin->wHeight/2;
		#endif
		//Idu_OsdPaintArea(x>>1, st_swFaceY20[bMode], 80, 4, OSD_COLOR_RED);
		#if OSD_LINE_NUM
		OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16+bMode,x>>1,st_swFaceY20[bMode],160,2,OSD_COLOR_RED);
		#endif
		return PASS;
	}

	return FAIL;
}

SWORD SearchRightFiberCore(ST_IMGWIN *pWin,BYTE bMode)
{
	DWORD dwOffset,dwYValidStartAver,dwValidlevelcnt,dwBlackAverLevel,dwWhiteAverLevel;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,k,swStartY,swYEnd,swY,swH,wCoreYUp=0,wCoreYDown=0;
	BYTE *pbWinBuffer,bContinueCnt,bLastLevel;
	SDWORD sdwRet;
	BYTE bColorIndex=OSD_COLOR_WHITE;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		mpDebugPrint("!!!RightCenter error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	dwOffset=pWin->dwOffset;

	//--获取实际光纤边黑色部分的亮度值
	if (!st_bFiberBlackLevel)
	{
		sdwRet=GetFiberBlackLevel();
		if (sdwRet>0)
			st_bFiberBlackLevel=sdwRet<<1;
		else
			return FAIL;
	}
	
	//--找纤芯上边沿
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	swYEnd=st_swFaceY3[bMode]-(st_swFaceY3[bMode]-st_swFaceY1[bMode])/4;
	if (bMode>1)
	{
		swStartY-=pWin->wHeight/2;
		swYEnd-=pWin->wHeight/2;
	}
	#if DISPLAY_IN_ONE_WIN
	swStartY<<=1;
	swYEnd<<=1;
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
			if (*pbWinBuffer<st_bFiberBlackLevel)
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
		else if (wValidPixelCnt>10)
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
	if (bMode>1)
	{
		swStartY-=pWin->wHeight/2;
		swYEnd-=pWin->wHeight/2;
	}
	#if DISPLAY_IN_ONE_WIN
	swStartY<<=1;
	swYEnd<<=1;
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
			if (*pbWinBuffer<st_bFiberBlackLevel)
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
		else if (wValidPixelCnt>10)
		{
			wCoreYDown=GetReallyData(wYValidArry,wValidPixelCnt);
			
		}
		if (wCoreYDown>0)
			break;
		TaskYield();
		//mpDebugPrint(" ");
	}

	if (wCoreYUp&&wCoreYDown)
	{
		st_swFaceY20[bMode]=(wCoreYUp+wCoreYDown)/2;
		#if DISPLAY_IN_ONE_WIN
			st_swFaceY20[bMode]>>=1;
		if (bMode>1)
			st_swFaceY20[bMode]+=pWin->wHeight/2;
		#endif
		#if OSD_LINE_NUM
		OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16+bMode,(x>>1)-160,st_swFaceY20[bMode],160,2,OSD_COLOR_RED);
		#endif
		return PASS;
	}

	return FAIL;
}

SWORD SearchLeftFiberFaceAndTopEdge(ST_IMGWIN *pWin,BYTE bMode,BYTE bScanEdge)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD i,x,y,swXEnd,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt;

	dwOffset=pWin->dwOffset;
	swXEnd=pWin->dwOffset;//(g_wCenterPos<<1)+(g_wCenterPos>>1);
	//mpDebugPrint(" SearchLeftFiberFaceAndTopEdge bMode=%d: bScanEdge=%d",bMode,bScanEdge);

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
				if (*pbWinBuffer < st_bBackGroundLevel)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < st_bBackGroundLevel*2/3 )
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
								dwYValidStartAver>>=1;
							if (bMode>1)
								dwYValidStartAver+=pWin->wHeight/2;
							#endif
								if (st_swFaceY1[bMode]!=dwYValidStartAver)
								{
									st_swFaceY1[bMode]=dwYValidStartAver;
									mpDebugPrint(" st_swFaceY1[%d]=%d ",bMode,st_swFaceY1[bMode]);
									#if OSD_LINE_NUM
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
			if (wValidPixelCnt>pWin->wWidth*9/20 ||wInvalidPixelCnt >pWin->wWidth*9/20)
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

		mpDebugPrint("x=%d y=%d valid=%d/%d  level=%02x",x>>1,y,wValidPixelCnt,wInvalidPixelCnt,st_bBackGroundLevel);
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
	//无有效端面或者所有端都有效
	if (!wValidLineCnt)
	{
		//UartOutText("-N-");
	}
	else //find
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
					if (*pbWinBuffer < st_bBackGroundLevel)
						wValidPixelCnt++;
					else
						wInvalidPixelCnt++;
				}
				else
				{
					if (*pbWinBuffer < st_bBackGroundLevel )
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
				mpDebugPrint("X[%d]=%d",bMode,x);
				#if OSD_LINE_NUM
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,bMode,x,(bMode>1 ? pWin->wHeight/2:0),2,pWin->wHeight/2,OSD_COLOR_GREEN);
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
	BYTE *pbWinBuffer,bContinueCnt;

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
				if (*pbWinBuffer < st_bBackGroundLevel)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < st_bBackGroundLevel )
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
								dwYValidStartAver>>=1;
							if (bMode>1)
								dwYValidStartAver+=pWin->wHeight/2;
							#endif
							if (st_swFaceY3[bMode]!=dwYValidStartAver)
							{
								st_swFaceY3[bMode]=dwYValidStartAver;
								mpDebugPrint(" st_swFaceY3[%d]=%d ",bMode,st_swFaceY3[bMode]);
								#if OSD_LINE_NUM
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,12+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
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
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wEndInvalidCnt,wYValidArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD x,y,i,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt;

	if (st_swFaceY1[bMode]<0 || st_swFaceY3[bMode]<0||st_swFaceX[bMode]<0)
	{
		mpDebugPrint("!!!RightCenter error!!! %d,%d: ",st_swFaceY1[bMode],st_swFaceY3[bMode]);
		return FAIL;
	}
	dwOffset=pWin->dwOffset;
	swStartY=st_swFaceY1[bMode]+(st_swFaceY3[bMode]-st_swFaceY1[bMode])/3;
#if (SENSOR_WIN_NUM==2)
		if (bMode>1)
		{
			swStartY-=pWin->wHeight/2;
		}
		#if DISPLAY_IN_ONE_WIN
		swStartY<<=1;
		swYEnd=pWin->wHeight;
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
	for (i=0;i<X_VALID_SUM;i++)
		wYValidArry[i]=0;
	wYStartIndex=0;
	for (x=st_swFaceX[bMode]<<1;x<dwOffset;x+=X_STEP)
	{
		wValidPixelCnt=0;
		wInvalidPixelCnt=0;
		bContinueCnt=0;
		wEndInvalidCnt=0;
		y=swStartY;
		pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
		//mpDebugPrint(" %d: ",x);
		while (y<swYEnd)
		{
			//mpDebugPrintN("%02x ",*pbWinBuffer);
			if (wValidPixelCnt)
			{
				if (*pbWinBuffer > st_bBackGroundLevel)
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
				if (*pbWinBuffer > st_bBackGroundLevel )
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
					//计算有效Y起始坐标
					if (wYStartIndex>=8)
					{
						dwYValidStartAver=0;
						for (i=0;i<wYStartIndex;i++)
							dwYValidStartAver+=wYValidArry[i];
						dwYValidStartAver/=wYStartIndex;
						wYvalidCnt=0;
						//mpDebugPrint("y: %02x  ",dwYValidStartAver);
						for (i=0;i<wYStartIndex;i++)
						{
							//mpDebugPrintN("%02x ",wYValidArry[i]);
							if ((dwYValidStartAver+Y_VALID_OFFSET>wYValidArry[i]) && (dwYValidStartAver-Y_VALID_OFFSET<wYValidArry[i]))
								wYvalidCnt++;
						}
						//mpDebugPrint(" %d ",wYvalidCnt);
						if (wYvalidCnt>wYStartIndex/2)
						{
							wYStartIndex=X_VALID_SUM;
							#if DISPLAY_IN_ONE_WIN
								dwYValidStartAver>>=1;
							if (bMode>1)
								dwYValidStartAver+=pWin->wHeight/2;
							#endif
							if (st_swFaceY2[bMode]!=dwYValidStartAver)
							{
								st_swFaceY2[bMode]=dwYValidStartAver;
								mpDebugPrint(" st_swFaceY2[%d]=%d ",bMode,st_swFaceY2[bMode]);
								#if OSD_LINE_NUM
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,8+bMode,pWin->wWidth/2,dwYValidStartAver,pWin->wWidth,2,OSD_COLOR_RED);
								#endif
								return ENABLE;
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

SWORD SearchRightFiberFaceAndTopEdge(ST_IMGWIN *pWin,BYTE bMode,BYTE bScanFace)
{
	DWORD dwOffset,dwYValidStartAver;
	WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	SWORD i,x,y,swXEnd,swXstart,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt;

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
				if (*pbWinBuffer < st_bBackGroundLevel)
					wValidPixelCnt++;
				else
					wInvalidPixelCnt++;
			}
			else
			{
				if (*pbWinBuffer < st_bBackGroundLevel )
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
								dwYValidStartAver>>=1;
							if (bMode>1)
								dwYValidStartAver+=pWin->wHeight/2;
							#endif
							if (st_swFaceY1[bMode]!=dwYValidStartAver)
							{
								st_swFaceY1[bMode]=dwYValidStartAver;
								mpDebugPrint(" st_swFaceY1[%d]=%d ",bMode,st_swFaceY1[bMode]);
								#if OSD_LINE_NUM
								OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4+bMode,(bMode&0x01)>0 ? pWin->wWidth/2:0,dwYValidStartAver,pWin->wWidth/2,2,OSD_COLOR_BLUE);
								#endif
							}
							if (!bScanFace)
								return 1;
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
			if (wValidPixelCnt>pWin->wHeight*9/20 ||wInvalidPixelCnt >pWin->wHeight*9/20)
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
					if (*pbWinBuffer < st_bBackGroundLevel)
						wValidPixelCnt++;
					else
						wInvalidPixelCnt++;
				}
				else
				{
					if (*pbWinBuffer < st_bBackGroundLevel )
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
			if (st_swFaceX[bMode]>=x+X_SHAKE || st_swFaceX[bMode]+X_SHAKE<=x)
			{
				st_swFaceX[bMode]=x;
				mpDebugPrint("X[%d]=%d",bMode,x);
				#if OSD_LINE_NUM
				OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,bMode,x,(bMode>1 ? pWin->wHeight/2:0),2,pWin->wHeight/2,OSD_COLOR_GREEN);
				#endif
				return ENABLE;
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


SWORD SearchWholeFiber(ST_IMGWIN *pWin)
{
	DWORD dwOffset;
	//WORD wValidPixelCnt,wInvalidPixelCnt,wValidLineCnt,wYValidStartArry[X_VALID_SUM],wYStartIndex,wYvalidCnt;
	WORD wLastY,wCurY;
	SWORD i,x,y,swXEnd,swStartY,swYEnd;
	BYTE *pbWinBuffer,bContinueCnt;

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
			if (*pbWinBuffer < st_bBackGroundLevel )
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

SWORD SearchFiberLowPoint(ST_IMGWIN *pWin,BYTE bWinIndex)
{
	DWORD dwOffset;
	WORD wCurY,wMaxY,wMinY,wSamePoint,wReverse,wReverseMax;
	SWORD i,x,y,swXEnd,swStartY,swYEnd,swLowX,swRet=FAIL;
	BYTE *pbWinBuffer,bContinueCnt,bGetLowPiont,bError=0;

	dwOffset=pWin->dwOffset;
	swXEnd=pWin->dwOffset;//(g_wCenterPos<<1)+(g_wCenterPos>>1);
	//mpDebugPrint(" SearchLeftFiberFaceAndTopEdge bMode=%d: bScanEdge=%d",bMode,bScanEdge);

	swStartY=0;
	swYEnd=pWin->wHeight;

	for (wReverseMax=8;wReverseMax<pWin->wWidth/2;wReverseMax+=4)
	{
		swRet=FAIL; 
		wMaxY=0;
		bGetLowPiont=1;
		wReverse=0;
		wSamePoint=0;
		for (x=X_STEP;x<swXEnd;x+=X_STEP)
		{
			y=swStartY;
			pbWinBuffer = (BYTE *) pWin->pdwStart+x+y*pWin->dwOffset;
			bContinueCnt=0;
			wCurY=0;
			//mpDebugPrint(" %d: ",x);
			while (y<swYEnd)
			{
				//mpDebugPrintN("%02x ",*pbWinBuffer);
				if (*pbWinBuffer < st_bBackGroundLevel)
				{
					bContinueCnt++;
					if (bContinueCnt>=Y_CONTINUE_VALID_SUM)
					{
						wCurY=y-bContinueCnt;
						//Idu_OsdPaintArea(x>>1, pWin->wHeight/2*bWinIndex+wCurY/2, 2, 1, OSD_COLOR_RED);
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
				//光纤断
				bError=1;
				mpDebugPrint("fiber error=%d,x=%d ",bError,x>>1);
				break;
			}
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
					if (wMaxY<wCurY)
					{
						wMaxY=wCurY;
						swLowX=x>>1;
						wReverse=0;
						wSamePoint=0;
						//mpDebugPrint(" (%d,%d)",swLowX,wMaxY);
					}
					else if (wMaxY==wCurY)
					{
						wSamePoint++;
					}
					else
					{
						wReverse++;
						//已找到低点开始回走
						if (wReverse>wReverseMax)
						{
							bGetLowPiont=0;
							wReverse=0;
						}
					}
				}
				else
				{
					if (wMaxY<wCurY)
					{
						wReverse++;
						wSamePoint++;
						// 再次找低点，是个错误
						if (wReverse>wReverseMax) 
						{
							bError=2;
							//放电熔的幅度不够
						}
					}
					else
					{
						wReverse=0;
					}
				}
				
			}
			TaskYield();
			//mpDebugPrint(" ");
		}
		mpDebugPrint(" --bError=%d swLowX=%d bGetLowPiont=%d wSamePoint=%d wReverseMax=%d",bError,swLowX,bGetLowPiont,wSamePoint,wReverseMax);
		mpDebugPrint(" ----wMaxY=%d wMinY=%d",wMaxY,wMinY);
		switch (bError)
		{
			case 0:
				if (bGetLowPiont)
					swRet=FAIL; 
				else
				{
					if ((wSamePoint >40) || (wMaxY-wMinY<30))// pWin->dwOffset/100 -st_wFiberWidth/8
					{
						swRet=1; //re discharge
					}
					else
					{
						swRet=PASS;
						swLowX+=wSamePoint/2;
						//Idu_OsdPaintArea(swLowX, 0, 2, pWin->wHeight, OSD_COLOR_RED);
					}
				}
				break;

			//光纤断
			case 1:
				swRet=FAIL; 
				break;

			// 再次找低点
			case 2:
				if ((wSamePoint >40) || (wMaxY-wMinY<30)) //st_wFiberWidth
				{
					swRet=1; //re discharge
				}
				else
				{
					bGetLowPiont=1;
					wReverse=0;
					wReverseMax+=4;
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

	if (swRet>0)//(swRet==1 || swRet==10)
	{
		st_bDiachargeTimes++;
		Discharge(1000+st_bDiachargeTimes*300,0);
		swRet=0;
	}
	else if (swRet==PASS)
	{
		swRet=swLowX;
	}

	return swRet;
}


#define		MOTO_BASE_RETRY_TIMES					10
#define		MOTO_BASE_STEPS									8 // pixel
void DriveMotor(BYTE bMotorInex,BYTE bDirection,WORD wStep,BYTE bSpeed)
{
	BYTE bTxData[10],bChecksum,i,bDataIndex=bMotorInex-1;
	WORD wPulse=wStep;


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
	if (TSPI_PacketSend(bTxData,bTxData[1],0)==PASS)
	{
		st_bMotorStaus|=(1<<bDataIndex);
		st_bDirectionArry[bDataIndex]=bDirection;
	}

}

void MotorSetStatus(BYTE bMotorInex,BYTE bMode) //0xa3   bmode:0->stop  BYT4~7   0 停止 1 允许待机  2不允许待机
{
	BYTE bTxData[10];

	if (bMotorInex>7)
		return;
	if (bMode==MOTOR_HOLD||bMode==MOTOR_NO_HOLD)
	{
		MotoHoldTimeoutSet(bMotorInex,bMode);
	}
	else if (bMode==MOTOR_STOP) //stop
	{
		mpDebugPrint("bMotorInex=%d  bMode=%d ",bMotorInex,bMode);
		bTxData[0]=0xa3;
		bTxData[1]=3+1;
		bTxData[2]=(bMode<<4)|bMotorInex; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
		TSPI_PacketSend(bTxData,bTxData[1],1);
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
		if (TSPI_PacketSend(bTxData,bTxData[1],0)!=PASS)
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
		if (TSPI_PacketSend(bTxData,bTxData[1],0)!=PASS)
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
	mpDebugPrint("H swSx=%d  swTx=%d",swSx,swTx);

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
	else if (swDiffX>60)
		bSpeed=8;
	else
		bSpeed=4;
	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
}

void MoveVMotorToSpecPosition(BYTE bFiberIndex,SWORD swTx)
{
	SWORD swSx,swDiffX;
	BYTE bMotorInex,bDirection,bSpeed,bDirectonUp;
	WORD wStep;

	swSx=st_swFaceY2[bFiberIndex];
	if (swTx<0)
		return;
	swDiffX=swTx-swSx;
	if (bFiberIndex==MOTOR_RIGHT_TOP)
	{
		bMotorInex=04; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
		if (st_bVmotorMoveTimes[0]<0xff)
		{
			if (st_bLastProcStep[0]!=g_swProcState)
			{
				st_bLastProcStep[0]=g_swProcState;
				st_bVmotorMoveTimes[0]=0;
			}
			else
			{
				st_bVmotorMoveTimes[0]++;
				st_wVmotorMoveValue[0][2]=st_wVmotorMoveValue[0][1];
				st_wVmotorMoveValue[0][1]=st_wVmotorMoveValue[0][0];
				st_wVmotorMoveValue[0][0]=ABS(swDiffX);
				if (st_bVmotorMoveTimes[0]>=VMOTOR_CNT)
				{
					if (st_wVmotorMoveValue[0][0]>st_wVmotorMoveValue[0][1] && st_wVmotorMoveValue[0][1]>st_wVmotorMoveValue[0][2])
					{
						st_bTopVMotorUpValue=1-st_bTopVMotorUpValue;
						st_bVmotorMoveTimes[0]=0xff;
					}
					else if (st_wVmotorMoveValue[0][0]<st_wVmotorMoveValue[0][1] && st_wVmotorMoveValue[0][1]<st_wVmotorMoveValue[0][2])
					{
						st_bVmotorMoveTimes[0]=0xff;
					}
				}
			}
		}
		bDirectonUp=st_bTopVMotorUpValue;
	}
	else if (bFiberIndex==MOTOR_RIGHT_BOTTOM)
	{
		bMotorInex=03;

		if (st_bVmotorMoveTimes[1]<0xff)
		{
			if (st_bLastProcStep[1]!=g_swProcState)
			{
				st_bLastProcStep[1]=g_swProcState;
				st_bVmotorMoveTimes[1]=0;
			}
			else
			{
				st_bVmotorMoveTimes[1]++;
				st_wVmotorMoveValue[1][2]=st_wVmotorMoveValue[1][1];
				st_wVmotorMoveValue[1][1]=st_wVmotorMoveValue[1][0];
				st_wVmotorMoveValue[1][0]=ABS(swDiffX);
				if (st_bVmotorMoveTimes[1]>=VMOTOR_CNT)
				{
					if (st_wVmotorMoveValue[1][0]>st_wVmotorMoveValue[1][1] && st_wVmotorMoveValue[1][1]>st_wVmotorMoveValue[1][2])
					{
						st_bBottomVMotorUpValue=1-st_bBottomVMotorUpValue;
						st_bVmotorMoveTimes[1]=0xff;
					}
					else if (st_wVmotorMoveValue[1][0]<st_wVmotorMoveValue[1][1] && st_wVmotorMoveValue[1][1]<st_wVmotorMoveValue[1][2])
					{
						st_bVmotorMoveTimes[1]=0xff;
					}
				}
			}
		}

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
	wStep=swDiffX*50; // 123
 if (swDiffX>20)
		bSpeed=8;
	else
		bSpeed=4;

	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
}

void MoveVMotorByPosition(BYTE bFiberIndex,SWORD swSx,SWORD swTx)
{
	SWORD swDiffX;
	BYTE bMotorInex,bDirection,bSpeed,bDirectonUp;
	WORD wStep;

	if (swTx<0 || swTx<0)
		return;
	swDiffX=swTx-swSx;
	if (bFiberIndex==MOTOR_RIGHT_TOP)
	{
		bMotorInex=04; // 左马达0x01 右马达0x02   Y上马达0x03  Y下马达0x04
		if (st_bVmotorMoveTimes[0]<0xff)
		{
			if (st_bLastProcStep[0]!=g_swProcState)
			{
				st_bLastProcStep[0]=g_swProcState;
				st_bVmotorMoveTimes[0]=0;
			}
			else
			{
				st_bVmotorMoveTimes[0]++;
				st_wVmotorMoveValue[0][2]=st_wVmotorMoveValue[0][1];
				st_wVmotorMoveValue[0][1]=st_wVmotorMoveValue[0][0];
				st_wVmotorMoveValue[0][0]=ABS(swDiffX);
				if (st_bVmotorMoveTimes[0]>=VMOTOR_CNT)
				{
					if (st_wVmotorMoveValue[0][0]>st_wVmotorMoveValue[0][1] && st_wVmotorMoveValue[0][1]>st_wVmotorMoveValue[0][2])
					{
						st_bTopVMotorUpValue=1-st_bTopVMotorUpValue;
						st_bVmotorMoveTimes[0]=0xff;
					}
					else if (st_wVmotorMoveValue[0][0]<st_wVmotorMoveValue[0][1] && st_wVmotorMoveValue[0][1]<st_wVmotorMoveValue[0][2])
					{
						st_bVmotorMoveTimes[0]=0xff;
					}
				}
			}
		}
		bDirectonUp=st_bTopVMotorUpValue;
	}
	else if (bFiberIndex==MOTOR_RIGHT_BOTTOM)
	{
		bMotorInex=03;

		if (st_bVmotorMoveTimes[1]<0xff)
		{
			if (st_bLastProcStep[1]!=g_swProcState)
			{
				st_bLastProcStep[1]=g_swProcState;
				st_bVmotorMoveTimes[1]=0;
			}
			else
			{
				st_bVmotorMoveTimes[1]++;
				st_wVmotorMoveValue[1][2]=st_wVmotorMoveValue[1][1];
				st_wVmotorMoveValue[1][1]=st_wVmotorMoveValue[1][0];
				st_wVmotorMoveValue[1][0]=ABS(swDiffX);
				if (st_bVmotorMoveTimes[1]>=VMOTOR_CNT)
				{
					if (st_wVmotorMoveValue[1][0]>st_wVmotorMoveValue[1][1] && st_wVmotorMoveValue[1][1]>st_wVmotorMoveValue[1][2])
					{
						st_bBottomVMotorUpValue=1-st_bBottomVMotorUpValue;
						st_bVmotorMoveTimes[1]=0xff;
					}
					else if (st_wVmotorMoveValue[1][0]<st_wVmotorMoveValue[1][1] && st_wVmotorMoveValue[1][1]<st_wVmotorMoveValue[1][2])
					{
						st_bVmotorMoveTimes[1]=0xff;
					}
				}
			}
		}

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
	mpDebugPrint("V0 swSx=%d  swTx=%d",swSx,swTx);
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
	wStep=swDiffX*50; // 123
 if (swDiffX>20)
		bSpeed=8;
	else
		bSpeed=4;

	DriveMotor(bMotorInex,bDirection,wStep,bSpeed);
}

void Discharge(WORD wMode,BYTE bStep)
{
	BYTE i,bTxData[TX_BUFFER_LENTH];

	mpDebugPrint("Discharge %d bStep=%d",wMode,bStep);
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
#if 0
		bTxData[1]=3+1;
		bTxData[2]=wMode;
#else
		bTxData[1]=3+2;
		if (wMode<4)//0x8000
		{
			switch (wMode)
			{
				case 2:
					bTxData[2]=0;
					bTxData[3]=130;
					break;

				case 3://2300
					bTxData[2]=0x08;
					bTxData[3]=0xfc;
					break;

				case 1:
				default:
					bTxData[2]=0;
					bTxData[3]=130;
					break;
			}
		}
		else
		{
			//wMode &= 0x7fff;
			bTxData[2]=wMode>>8;
			bTxData[3]=wMode&0xff;
		}
#endif
	}
	TSPI_PacketSend(bTxData,bTxData[1],1);
}

#endif

void Proc_GetFiberLowPoint(void)
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet;
	BYTE bSetEvent=1;

	if (!g_swGetCenterState)
		return;
	MP_DEBUG("Proc_GetFiberLowPoint   %d ",g_swGetCenterState);

	switch (g_swGetCenterState)
	{
		case GET_CENTER_INIT:
			g_wElectrodePos[0]=0;
			g_wElectrodePos[1]=0;
			st_bRetryTimes=5;
			st_bDiachargeTimes=0;
			Idu_OsdErase();

			Discharge(200,0);
			g_swGetCenterState++;
			break;

		case GET_CENTER_WHOLE_FIBER:
			pWin=(ST_IMGWIN *)&SensorInWin[0];
			if (SearchWholeFiber(pWin)==PASS)
			{
				Discharge(1000,0);
				st_bRetryTimes=5;
				Ui_TimerProcAdd(3000,SetFillProcWinFlag);
				bSetEvent=0;
				g_swGetCenterState++;
			}
			else
			{
				MP_DEBUG("SearchWholeFiber   FAIL !");
				if (st_bRetryTimes)
				{
					st_bRetryTimes--;
					SetFillProcWinFlag();
					bSetEvent=0;
				}
				else
					g_swGetCenterState=GET_CENTER_OFF;
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
			if (swRet==0)
			{
				Ui_TimerProcAdd(3000,SetFillProcWinFlag);
				bSetEvent=0;
			}
			else
			{
				MP_DEBUG("SearchFiberLowPoint   swRet=%d",swRet);
				if (swRet<0 && st_bRetryTimes)
				{
					st_bRetryTimes--;
					SetFillProcWinFlag();
					bSetEvent=0;
				}
				else
				{
					if (swRet<0)
						swRet=pWin->wWidth/2;
					if (g_wElectrodePos[0]==0)
					{
						g_wElectrodePos[0]=swRet;
						OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,18,g_wElectrodePos[0],0,2,pWin->wHeight/2,OSD_COLOR_RED);
						g_psSetupMenu->wElectrodePos[0]=g_wElectrodePos[0];
						WriteSetupChg();
					}
					else
					{
						g_wElectrodePos[1]=swRet;
						OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,19,g_wElectrodePos[1],pWin->wHeight/2,2,pWin->wHeight,OSD_COLOR_RED);
						g_psSetupMenu->wElectrodePos[1]=g_wElectrodePos[1];
						WriteSetupChg();
					}
					g_swGetCenterState++;
				}
			}
			break;

		case GET_CENTER_FINISH:
			if (g_wElectrodePos[0] && g_wElectrodePos[1])
				g_swGetCenterState=GET_CENTER_OFF;
			else
			{
				st_bRetryTimes=5;
				st_bDiachargeTimes=0;
				g_swGetCenterState=GET_CENTER_LOW_POINT;
			}
			break;

		default:
			break;
	}

	if (g_swGetCenterState && bSetEvent)
	{
		EventSet(UI_EVENT, EVENT_PROC_DATA);
	}

}



#define		AF_MOTO_INDEX									1
#define		WHITE_TO_BLACK								0x40
#define		BLACK_TO_WHITE								0x40

SWORD g_swAutoFocusState = AF_OFF;

DWORD st_dwAFbrightChg=0;
WORD st_wAFStartX,st_wAFStartY,st_wWhiteWidth;
SWORD st_AFLastWidth,st_AFMinWidth;
BYTE st_bAFMotoIndex=6,st_bAFReturnTimes,st_bAFFirsetTry;
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
		if (*pbWinBuffer>st_bBackGroundLevel)
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
					if (*pbWinBuffer<st_bBackGroundLevel)
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
						if (*pbWinBuffer>st_bBackGroundLevel)
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
	TSPI_PacketSend(bTxData,bTxData[1],1);
}


void TimerToNextState(void)
{
	g_swProcState++;
}


void Proc_Weld_State()
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet,swPos1,swPos2;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下

#if SENSOR_WIN_NUM
	if (pbSensorWinBuffer==NULL)
		return;
	if (g_swProcState)
	{
		MP_DEBUG("g_swProcState   %d ",g_swProcState);
	}

	switch (g_swProcState)
	{
		case SENSOR_FACE_POS1A:
			bMode=MOTOR_LEFT_TOP;
			swPos1=g_wElectrodePos[0]-20;
			if (st_swFacexCurPos[bMode]!=swPos1)
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==FAIL)
					st_swFaceX[bMode]=-1;
				MoveHMotorToSpecPosition(bMode,swPos1);
				//if (st_swFacexCurPos[bMode]==swPos1)
				//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				SetFillProcWinFlag();
			}
			else
			{
				//g_swProcState=SENSOR_IDLE;
			}
			//break;

		case SENSOR_FACE_POS1B:
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			bMode=MOTOR_LEFT_BOTTOM;
			swPos2=g_wElectrodePos[1]-20;
			if (st_swFacexCurPos[bMode]!=swPos2)
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet==FAIL)
					st_swFaceX[bMode]=-1;
				MoveHMotorToSpecPosition(bMode,swPos2);
				//if (st_swFacexCurPos[bMode]==swPos2)
				//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				SetFillProcWinFlag();
			}
			else
			{
				//g_swProcState++;
			}
			if ((st_swFacexCurPos[MOTOR_LEFT_TOP]==swPos1)&&(st_swFacexCurPos[MOTOR_LEFT_BOTTOM]==swPos2))
			{
				mpDebugPrint("SENSOR_FACE_POS1B OK");
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				SearchLeftFiberBottomEdge(pWin,MOTOR_LEFT_TOP);
				SearchLeftFiberCenter(pWin,MOTOR_LEFT_TOP);
				SearchLeftFiberCore(pWin,MOTOR_LEFT_TOP);

				pWin=(ST_IMGWIN *)&SensorInWin[1];
				SearchLeftFiberBottomEdge(pWin,MOTOR_LEFT_BOTTOM);
				SearchLeftFiberCenter(pWin,MOTOR_LEFT_BOTTOM);
				g_swProcState=SENSOR_FACE_POS1B+1;
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
				SetFillProcWinFlag();
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H1A OK");
				g_swProcState++;
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
				SetFillProcWinFlag();
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H1B OK");
				g_swProcState++;
			}
			/*
			if ((st_swFacexCurPos[bMode]==st_swFaceY1[bMode-1])&&(st_swFacexCurPos[MOTOR_RIGHT_TOP]==st_swFaceY1[MOTOR_RIGHT_TOP-1]))
			{
				mpDebugPrint("SENSOR_ALIGN_H2 OK");
				g_swProcState=SENSOR_ALIGN_H2+1;
			}
			*/
			break;
#endif
		case SENSOR_DISCHARGE1:
			if (st_bDischargeMode!=1)
			{
				st_bDischargeMode=1;
				Discharge(st_bDischargeMode,0);
				Ui_TimerProcAdd(3000, TimerToNextState);
			}
			break;

		case SENSOR_AUTO_FOCUS:
#if 1
			g_swProcState++;
#else
			if (g_swAutoFocusState==AF_OFF)
			{
					g_swAutoFocusState=AF_INIT;
					Proc_AutoFocus();
			}
			else if (g_swAutoFocusState==AF_FINISH)
			{
					g_swAutoFocusState=AF_OFF;
					g_swProcState++;
					g_swProcState=SENSOR_IDLE;
			}
#endif
			break;

		case SENSOR_GET_ANGLE:
			g_swProcState++;
			//g_swProcState=SENSOR_IDLE;
			break;

		case SENSOR_FACE_POS2A:
			bMode=MOTOR_LEFT_TOP;
			swPos1=g_wElectrodePos[0]-30;
			if (st_swFacexCurPos[bMode]!=swPos1)
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet!=FAIL)
				{
					MoveHMotorToSpecPosition(bMode,swPos1);//  MOTOR_LEFT_TOP MOTOR_LEFT_BOTTOM
					//if (st_swFacexCurPos[bMode]==swPos1)
					//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				}
				SetFillProcWinFlag();
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				//g_swProcState++;
			}
			//break;

		case SENSOR_FACE_POS2B:
			pWin=(ST_IMGWIN *)&SensorInWin[1];
			bMode=MOTOR_LEFT_BOTTOM;
			swPos2=pWin->wWidth-g_wElectrodePos[1];
			if (st_swFacexCurPos[bMode]!=swPos2)
			{
				swRet=SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				if (swRet!=FAIL)
				{
					MoveHMotorToSpecPosition(bMode,swPos2);//  MOTOR_LEFT_TOP MOTOR_LEFT_BOTTOM
					//if (st_swFacexCurPos[bMode]==swPos2)
					//	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
				}
				SetFillProcWinFlag();
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				//g_swProcState++;
			}
			if ((st_swFacexCurPos[MOTOR_LEFT_TOP]==swPos1)&&(st_swFacexCurPos[MOTOR_LEFT_BOTTOM]==swPos2))
			{
				mpDebugPrint("SENSOR_FACE_POS2B OK");
				g_swProcState=SENSOR_FACE_POS2B+1;
				MotorSetStatus(GetMotoIdxByFiberIdx(MOTOR_RIGHT_TOP),MOTOR_HOLD);
			}
			break;

		case SENSOR_ALIGN_H2A:
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
				SetFillProcWinFlag();
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H2A OK");
				g_swProcState++;
				MotorSetStatus(GetMotoIdxByFiberIdx(MOTOR_RIGHT_BOTTOM),MOTOR_HOLD);
			}
			break;

		case SENSOR_ALIGN_H2B:
			pWin=(ST_IMGWIN *)&SensorInWin[1];
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
				SetFillProcWinFlag();
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H2B OK");
				g_swProcState++;
			}
			break;

		case SENSOR_ALIGN_H3A:
			bMode=MOTOR_RIGHT_TOP;
			if (st_swFaceY20[bMode]!=st_swFaceY20[MOTOR_LEFT_TOP])
			{
				pWin=(ST_IMGWIN *)&SensorInWin[0];
				swRet=SearchRightFiberCore(pWin,bMode);
				if (swRet!=FAIL)
				{
						MoveVMotorByPosition(bMode,st_swFaceY20[bMode],st_swFaceY20[MOTOR_LEFT_TOP]);
				}
				SetFillProcWinFlag();
			}
			else
			{
				mpDebugPrint("SENSOR_ALIGN_H3A OK");
				g_swProcState++;
			}
			break;

		case SENSOR_ALIGN_H3B:
			bMode=MOTOR_RIGHT_BOTTOM;
			if (st_swFaceY20[bMode]!=st_swFaceY20[MOTOR_LEFT_BOTTOM])
			{
				pWin=(ST_IMGWIN *)&SensorInWin[1];
				swRet=SearchRightFiberCore(pWin,bMode);
				if (swRet!=FAIL)
				{
						MoveVMotorByPosition(bMode,st_swFaceY20[bMode],st_swFaceY20[MOTOR_LEFT_BOTTOM]);
				}
				SetFillProcWinFlag();
			}
			else
			{
				//MotorSetStatus(GetMotoIdxByFiberIdx(bMode),MOTOR_NO_HOLD);
				mpDebugPrint("SENSOR_ALIGN_H3B OK");
				g_swProcState++;
				//g_swProcState=SENSOR_IDLE;
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
				xpgDelay(1000);
			}
			break;


		case SENSOR_DISCHARGE2:
			if (st_bDischargeMode!=2)
			{
				st_bDischargeMode=2;
				Discharge(st_bDischargeMode,0);
				Ui_TimerProcAdd(3000, TimerToNextState);
			}
			break;

		case SENSOR_DISCHARGE3:
			if (st_bDischargeMode!=3)
			{
				st_bDischargeMode=3;
				Discharge(st_bDischargeMode,0);
				//xpgDelay(10);
				//DriveMotor(01,1,10,10);
				DriveMotor(01,1,75*17,4); // 27 
				Ui_TimerProcAdd(6000, TimerToNextState);
			}
			break;

		case SENSOR_GET_LOSS:
			g_swProcState=SENSOR_IDLE;
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
#if 0 //TEST_PLANE
	//pWin=(ST_IMGWIN *)Idu_GetCurrWin();
#if 1
	bMode=MOTOR_LEFT_TOP;
	SearchLeftFiberFaceAndTopEdge(pWin,bMode,1);
	SearchLeftFiberBottomEdge(pWin,bMode);
	SearchLeftFiberCenter(pWin,bMode);
	SearchLeftFiberCore(pWin,bMode);
#endif
#if 0
	bMode=MOTOR_RIGHT_TOP;
	SearchRightFiberFaceAndTopEdge(pWin,bMode,1);
	SearchRightFiberBottomEdge(pWin,bMode);
	SearchRightFiberCenter(pWin,bMode);
	SearchRightFiberCore(pWin,bMode);
#endif
	pWin=(ST_IMGWIN *)&SensorInWin[1];
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

//	SetFillProcWinFlag();
#endif
//	mpCopyWinAreaSameSize(pWin,Idu_GetCurrWin(),0, 0, 0, 0,pWin->wWidth, pWin->wHeight);
	//SetFillProcWinFlag();
		default:
			break;
	}

	xpgUpdatePageOsd();
#endif

	if (g_swProcState)
	{
		EventSet(UI_EVENT, EVENT_PROC_DATA);
	}

}

void Proc_SensorData_State()
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)&SensorInWin[0];
	SWORD swRet,swPos1,swPos2;
	BYTE bMode=0; //4 0->左上  1->右上 2->左下 3->右下

	if (!st_bBackGroundLevel)
		return ;
	if (st_bInProcWin || st_bNeedFillProcWin)
	{
		EventSet(UI_EVENT, EVENT_PROC_DATA);
		return;
	}
	st_bInProcWin=1;

	Proc_Weld_State();
	Proc_GetFiberLowPoint();

	st_bInProcWin=0;

}

void TSPI_DataProc(void)
{
	BYTE i,bChecksum,index;
	
	if (!st_bTspiRxIndex)
		return;
	for (i=0;i<st_bTspiRxIndex;i++)
		mpDebugPrintN(" %02x ",st_bTspiRxArry[i]);
	mpDebugPrint("");
	if (st_bTspiRxIndex<3)
		return;
	if (st_bTspiRxArry[1]!=st_bTspiRxIndex)
		return;
	bChecksum=0;
	for (i=0;i<st_bTspiRxIndex-1;i++)
	{
		bChecksum+=st_bTspiRxArry[i];
	}
	if (bChecksum!=st_bTspiRxArry[st_bTspiRxIndex-1])
	{
		//mpDebugPrintN(" checksum error really %p data %p ",bChecksum,st_bTspiRxArry[st_bTspiRxIndex-1]);
		//return;
	}

	switch (st_bTspiRxArry[0])
	{
		case 0xb1:
			if (st_bTspiRxArry[3])
			{
				#if 1
				switch (st_bTspiRxArry[2])
				{
					case 1:
						#if 0
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
						Idu_OsdErase();
						#else
						//DriveMotor(01,0,1000,8);//RIGHT_DOWN_FIBER  DOWN
						Discharge(800,0);

						#endif
						break;
					case 2:
						#if 0
						//DriveMotor(01,1,1000,8);//RIGHT_DOWN_FIBER  DOWN
						st_swAFStep=100;
						ProcAFmotorAction();
						//PrintWinData();
						#else
						Discharge(1000,0);
						#endif
						break;
					case 3:
						#if 0
						//DriveMotor(02,0,1000,8);//RIGHT_DOWN_FIBER  UP
						st_swAFStep=-100;
						ProcAFmotorAction();
						#else
						Discharge(1500,0);
						#endif
						break;

					case 4:
						#if 0
						//AutoStartWeld();
						//st_bDischargeMode=1;
						//Discharge(st_bDischargeMode,0);
						//st_dwAutoTestTime=0;
						//st_dwAutoTestTotal=50;
						//Ui_TimerProcAdd(1000, AutoTestTimer);
						MotorSetStatus(1,MOTOR_NO_HOLD);
						MotorSetStatus(2,MOTOR_NO_HOLD);
						MotorSetStatus(3,MOTOR_NO_HOLD);
						MotorSetStatus(4,MOTOR_NO_HOLD);
						#elif 1
						if (g_bDisplayMode<3)
							g_bDisplayMode++;
						else
							g_bDisplayMode=0;
						g_bDisplayMode |= 0x80;
						#elif 1
						//DriveMotor(02,1,1000,8);
						SetFillProcWinFlag();
						#else
						#endif
						break;

					case 5:
						#if 0
						//DriveMotor(01,1,200,8);//RIGHT_UP_FIBER DOWN
						MoveAFMotor(6,-100);
						/*
						SetFillProcWinFlag();
						g_swAutoFocusState=AF_INIT;
						Proc_AutoFocus();
						*/
						#elif 0
						if (g_bDisplayMode<3)
							g_bDisplayMode++;
						else
							g_bDisplayMode=0;
						g_bDisplayMode |= 0x80;
						#elif 1
						g_swProcState=SENSOR_IDLE;
						ResetMotor();
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

						#endif
						break;
					case 6:
						#if 0
						MotorSetStatus(2,MOTOR_NO_HOLD);
						DriveMotor(02,1,2000,8);//RIGHT_UP_FIBER UP
						//g_bDisplayMode=(g_bDisplayMode+1)|0x80;
						#elif 0
								MoveAFMotor(6,100);
						#elif 1
						//AutoStartWeld();
						//SetFillProcWinFlag();
						AutoGetFiberLowPoint();
						#endif
						break;


					default:
						break;
				}
			#else
						xpgCb_CaptureWinToFile(Idu_GetCurrWin());
						FileBrowserResetFileList();
						FileBrowserScanFileList(SEARCH_TYPE);
			#endif
			}
			break;

		case 0xb2:
			st_bMotorStaus=st_bTspiRxArry[2];
			if (g_swAutoFocusState!=AF_OFF && !(st_bMotorStaus&(1<<(AF_MOTO_INDEX-1))))
			{
				Ui_TimerProcRemove(SetFillProcWinFlag);
				Ui_TimerProcRemove(Proc_AutoFocus);
				SetFillProcWinFlag();
				Proc_AutoFocus();
			}
			break;

		case 0xb3:
			if (st_bTspiRxArry[2]) //4 盖子打开
			{
				// 马达复位
				ResetMotor();
			}
			break;
		//放电状态
		case 0xb4:
			if (st_bTspiRxArry[2]==0x01)
			{
				if (g_swProcState)
				{
					Ui_TimerProcRemove(TimerToNextState);
					g_swProcState++;//=SENSOR_FACE_POS2A;
				}
			}
			break;
		//复位状态
		case 0xb5:
			if (st_bTspiRxArry[2]==0x01)//复位完成
			{
					//DriveMotor(01,1,200,10);
					//DriveMotor(02,1,200,10);
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
	WORD wW,wH;

	wW=100; //half
	wH=pDstWin->wHeight/6;//half
OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,1,pDstWin->wWidth/2-1-wW,pDstWin->wHeight/2-2,wW<<1,2,OSD_COLOR_RED);
OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,2,pDstWin->wWidth/2-1-1,pDstWin->wHeight/2-1-wH,2,wH<<1,OSD_COLOR_RED);

OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,3,0,pDstWin->wHeight/2-1-wH,pDstWin->wWidth,2,OSD_COLOR_BLUE);
OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,4,0,pDstWin->wHeight/2-1+wH,pDstWin->wWidth,2,OSD_COLOR_BLUE);

xpgUpdatePageOsd();

}
#endif

void DisplaySensorImage(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();
//	BYTE bCacheWinIndex=0;


	if (g_bDisplayMode&0xf0)
		Sensor_DisplayMode_Set();

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

void DisplaySensorOnCurrWin( BYTE bIpw2)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetCurrWin();
	DWORD *pIpwAddr;

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
		Sensor_DisplayMode_Set();

	switch (g_bDisplayMode)
	{
		case 2:
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+pDstWin->dwOffset*SensorWindow_Height;	
				
			}
			else
			{
				Sensor_Channel_Set(0);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
			}
			break;

		case 3:
			if (Sensor_CurChannel_Get()==0)
			{
				Sensor_Channel_Set(1);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000)+(SensorWindow_Width<<1);
				
			}
			else
			{
				Sensor_Channel_Set(0);
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
			}
			break;

			default:
				break;
	}

}

void CacheSensorData( BYTE bIpw2)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)&SensorInWin[0];
	BYTE bCacheWinIndex=0;
	DWORD *pIpwAddr;


	if (st_bNeedFillProcWin)
	{
		if (bIpw2)
			pIpwAddr=(DWORD *)&ipu->Ipu_reg_F2;
		else
			pIpwAddr=(DWORD *)&ipu->Ipu_reg_A3;

		if (st_bNeedFillProcWin&0x40)
		{
			//mpDebugPrint("cache in");
			st_bBackupChanel=Sensor_CurChannel_Get();
			st_bNeedFillProcWin &=0x3f;
			st_bNeedFillProcWin |=0x20;//for stop
		}
		if ((st_bNeedFillProcWin&0x03)==0x03)
		{
				if (Sensor_CurChannel_Get())
				{
					bCacheWinIndex=2;
					st_bNeedFillProcWin &=0xfd;
				}
				else
				{
					bCacheWinIndex=1;
					st_bNeedFillProcWin &=0xfe;
				}
		}
		else if (st_bNeedFillProcWin&0x02)
		{
				Sensor_Channel_Set(1);
				bCacheWinIndex=2;
				st_bNeedFillProcWin &=0xfd;
		}
		else if (st_bNeedFillProcWin&0x01)
		{
				Sensor_Channel_Set(0);
				bCacheWinIndex=1;
				st_bNeedFillProcWin &=0xfe;
		}
		else if ((st_bNeedFillProcWin&0x20)==0x20)
		{
			//mpDebugPrint("cache end");
			//ipu->Ipu_reg_F2 = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);	
			//mpCopyWinAreaSameSize((ST_IMGWIN *)&SensorInWin[0],(ST_IMGWIN *)Idu_GetCurrWin(),0, 0, 0, 0,pDstWin->wWidth, pDstWin->wHeight);
			if (st_bBackupChanel<2 && st_bBackupChanel!=Sensor_CurChannel_Get())
				Sensor_Channel_Set(st_bBackupChanel);
			st_bNeedFillProcWin=0;
			bCacheWinIndex=0;
			*pIpwAddr = ((DWORD)((ST_IMGWIN *)Idu_GetNextWin()->pdwStart)| 0xA0000000);	
			EventSet(UI_EVENT, EVENT_PROC_DATA);
		}

		if (bCacheWinIndex==1) // up win
		{
				*pIpwAddr = ((DWORD) pDstWin->pdwStart| 0xA0000000);	
		}
		else if (bCacheWinIndex==2) // down win
		{
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
			else
				ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1
		}
	}

}
void ProcIpwFrameEvent(void) //IPW2
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();

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
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)&SensorInWin[0];
	BYTE bCacheWinIndex=0;


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

void WeldDataInit(void)
{
	ST_IMGWIN *pWin=(ST_IMGWIN *)Idu_GetNextWin();

#if SHOW_CENTER
	g_bDisplayMode=0x80;
	g_swProcState = SENSOR_IDLE;
#elif TEST_PLANE
	//g_bDisplayMode=0x80;
	g_swProcState = SENSOR_IDLE;
#else
	g_bDisplayMode=0x82;
#endif

	if (g_psSetupMenu->wElectrodePos[0]>0  && g_psSetupMenu->wElectrodePos[0]<pWin->wWidth)
	{
		g_wElectrodePos[0]=g_psSetupMenu->wElectrodePos[0];
		OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,18,g_wElectrodePos[0],0,2,pWin->wHeight/2,OSD_COLOR_RED);
	}
	if (g_psSetupMenu->wElectrodePos[1]>0  && g_psSetupMenu->wElectrodePos[1]<pWin->wWidth)
	{
		g_wElectrodePos[1]=g_psSetupMenu->wElectrodePos[1];
		OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,19,g_wElectrodePos[1],pWin->wHeight/2,2,pWin->wHeight,OSD_COLOR_RED);
	}
	g_psSetupMenu->wElectrodePos[1]=400;
	WriteSetupChg();

	//ResetMotor();
	Ui_TimerProcAdd(5000, SetFillProcWinFlag);
	Ui_TimerProcAdd(5100, GetBackgroundLevel);
}

#endif
#endif



