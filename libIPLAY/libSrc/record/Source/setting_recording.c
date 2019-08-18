#define LOCAL_DEBUG_ENABLE  0
#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "devio.h"
#include "sensor.h"
#include "record.h"
//#include "camcorder_func.h"
#include "ui.h"


#if (RECORD_ENABLE)

void RecordVideotoSDcard(void);
void SensorApplication(void);

extern ST_IMGWIN SensorInWin[3];
extern int count_num;
extern int ce,sce1;
extern int sce;
extern BYTE MultiBufFlag;
extern int now,ok;
extern record_argument rec_argument;

//------------
extern BYTE *tBuf;

void RecordVideotoSDcard(void)
{int temp=0;
    int retVal;
    int CurrImgBufEncoderIndex=0;
    #define AUDIOSAMPLELEN 1024
    DWORD SensorEvent;
    DWORD iSize = 0;
	BYTE SetJpegQTable;

#if THREE_WINDOW_MODE
		if (MultiBufFlag)
		{
			if (sce)
				now=sce-1;
			else
				now=2;
		}
		else
			now=0;

		ok=0;
		//StepRecordFile(&SensorInWin[now]);
#if ONLY_USE_IPW2
		WinVirtualScaleDown(&SensorInWin[now], (ST_IMGWIN *)Idu_GetCurrWin(), VOV_WIN_X, VOV_WIN_Y, VOV_WIN_W, VOV_WIN_H);
		EventSet(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1);
#else
		mpCopyWinAreaSameSize(&SensorInWin[now], (ST_IMGWIN *)Idu_GetCurrWin(), 0, 0, 0, 0, SensorInWin[now].wWidth, SensorInWin[now].wHeight);
#endif
		//mpCopyWinAreaToWinArea((ST_IMGWIN *)Idu_GetCurrWin(), &SensorInWin[now], 0, 0, 0, 0, SensorInWin[now].wWidth, SensorInWin[now].wHeight);
		ok=1;

#elif T530_AV_IN

	if (bRecordMode())
	{
		if(sce1>=count_num)
		{
			sce1=0;
			EventSet(AUDIO_ID, BIT0);
		}
		sce1++;
		//UartOutText("/");
		iSize = ImageFile_Encode_Img2Jpeg_WithQT(tBuf, Idu_GetCurrWin(),8);
		iSize = (iSize + 1) & 0xFFFFFFFE;
		if (GetRemainFreeSize()<0)
		{
		}
		else if (SetRemainFreeSize(GetRemainFreeSize()-(iSize>>4))<0)
		{
			EventSet(UI_EVENT, EVENT_DISK_FULL);
		}
		else
			retVal = AVIEN_fillJpeg(tBuf, iSize);
	}
	//EventSet(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1); // For display chg win
	Idu_ChgWin(Idu_GetNextWin());
	IPU *ipu = (IPU *) IPU_BASE;
	//ipu->Ipu_reg_F2 = ((DWORD) ((ST_IMGWIN*)Idu_GetCurrWin())->pdwStart| 0xA0000000);	
	if(Get_Display_flag() )
		ipu->Ipu_reg_F0 &= ~BIT7;/*Open IPW2 write path*/

#else

	if (bRecordMode())
    {
		if(MultiBufFlag)
		{
			switch (sce)
			{
				case 0:
				temp=2;now=2;
				break;
				case 1:
				temp=0;now=0;
				break;
				case 2:
				temp=1;now=1;
				break;
				default:
				temp=0;now=0;
				break;
			}
		}
		else
		{
			temp=0;now=0;
		}
		if(sce1>=count_num)
		{
			sce1=0;
			EventSet(AUDIO_ID, BIT0);
		}
		sce1++;
		ok=0;

		switch(rec_argument.quantification)
        {
		  case 0: /*High qulity*/
		  	Set_jpegEcodeQualityLevel(0); 
		  	break;

		  case 1: /*Middle qulity*/
		  	Set_jpegEcodeQualityLevel(1);
		  	break;

          default:	
		  case 2: /*Low qulity*/
		  	Set_jpegEcodeQualityLevel(2);
		  	break;
			
        }
		SetJpegQTable = 8; /*dynamic QT*/
#if EnableTimeStamp
	if (SetupCamcorderTimeStampEnableGet())
	{
		static char strbuff[24];
		static ST_SYSTEM_TIME stCurrTime;

		SystemTimeGet(&stCurrTime);
		sprintf(strbuff, "%04d/%02d/%02d %02d:%02d:%02d", stCurrTime.u16Year, stCurrTime.u08Month, stCurrTime.u08Day,
		           stCurrTime.u08Hour, stCurrTime.u08Minute, stCurrTime.u08Second);
          //    Idu_SetFontColor(255, 255, 255);
		Idu_PrintStringWithSize(&SensorInWin[temp], strbuff, SensorInWin[temp].wWidth-228, SensorInWin[temp].wHeight-28, 0, 0, 0, 0);
	}
#endif		
		iSize = ImageFile_Encode_Img2Jpeg_WithQT(tBuf, &SensorInWin[temp],SetJpegQTable);
		iSize = (iSize + 1) & 0xFFFFFFFE;
		retVal = AVIEN_fillJpeg(tBuf, iSize);
		ok=1;
    }


#endif
}
void SensorApplication(void)
{
	RecordVideotoSDcard();
}

#endif

