#define LOCAL_DEBUG_ENABLE 				0

#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "idu.h"
#include "sensor.h"
#include "ui.h"


#if (SENSOR_ENABLE == ENABLE)

#if (SENSOR_ENABLE == ENABLE)
SDWORD SensorInputWidth=1280;
SDWORD SensorInputHeight=720;
#if SENSOR_WIN_NUM
BYTE *pbSensorWinBuffer=NULL;
ST_IMGWIN SensorInWin[SENSOR_WIN_NUM];
#endif
BYTE sensor_mode=MODE_RECORD;//MODE_CAPTURE;//MODE_RECORD;
BYTE sensor_IMGSIZE_ID=SIZE_VGA_640x480;//SIZE_2M_1600x1200;//SIZE_VGA_640x480;
//BYTE sensor_IPW_Format=Format_YYUV;

BYTE overlay_enable=0;

WORD SensorWindow_Width   = 320; // panel display size for sensor
WORD SensorWindow_Height  = 240; // panel display size for sensor
WORD SensorWindow_PosX   = 0;
WORD SensorWindow_PosY  = 0;
BYTE SensorWindow_setFlag = 0;/*0: Full screen, 1:set dispaly window*/

static BYTE Display_Enable_Flag = ENABLE;

//BYTE Sensor_Mode = 0;/*Use to enable the flag in scaler_isr*/

/*for PCCam*/
static DWORD g_PCCAM_GetReadyBufCnt=0;
static DWORD g_PCCAM_SwitchBuf_flag=0;/*0: not switch, 1: switch*/
static DWORD g_PCCAM_ReadyBuf_state=0;

static struct SENSOR_SHARE_BUF SensorUsbShareInfo[3];
static BYTE   SensorUsb_Use=0;
 
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))

void API_Sensor_Stop(void)
{
		IPU *ipu = (IPU *) IPU_BASE;
		BYTE bDispalyFlag=Get_Display_flag();
		DWORD dwStarttime=GetSysTime();

		MP_ALERT("---- %s ----", __FUNCTION__);
		Set_Display_flag(DISABLE);
		while (SystemGetElapsedTime(dwStarttime)<1000)
		{
			//close ipw2 encode buffer
			ipu->Ipu_reg_F0 |= BIT7;
			//close ipw1
			ipu->Ipu_reg_F0 |= BIT6;
			xpgDelay(10);
			if ((ipu->Ipu_reg_F0 & (BIT30|BIT31))==(BIT30|BIT31))
				break;
		}
		Set_Display_flag(bDispalyFlag);
		
		ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
		while(1)
		{
			if((ipu->Ipu_reg_100&BIT0)==0)//確定有寫進去
				break;
		}

#if SENSOR_WIN_NUM
		if (pbSensorWinBuffer)
		{
			ext_mem_free(pbSensorWinBuffer);
			pbSensorWinBuffer = NULL;
		}
#endif

		SensorWindow_setFlag = 0;
		API_SensorUsb_DisableShareBuf();

}







#if 0//CAMCODERRECORD_OLDMODE   //frank lin 20110209
int API_Sensor_Capture(STREAM *fileHandle, BYTE Capture_IMGSIZE_ID)
{
#if  defined(SENSOR_TYPE_MAGIC_SENSOR)

	BYTE  *start_addr_PC_tool_Ptr;
	TOOL  *toolCapturePriewBase;
	BYTE  CaputreGetID;
	
	#if (MAGIC_SENSOR_DEBUG) //for Magic Sensor debug only
		start_addr_PC_tool_Ptr = (BYTE *)start_addr_PC_tool_mem_capture_flow;
	#else
		start_addr_PC_tool_Ptr = start_addr_PC_tool;
	#endif
	
	toolCapturePriewBase = (TOOL *)((U32)start_addr_PC_tool_Ptr + CAPTURE_SETTING_BASE);
	CaputreGetID = ((toolCapturePriewBase->b_code1) & 0x00ff0000)>>16;
	MP_DEBUG2("%s:CaputreGetID = %d", __FUNCTION__, CaputreGetID);

	switch(CaputreGetID)
	{
		case 0:
			Capture_IMGSIZE_ID = SIZE_VGA_640x480;
			MP_ALERT("%s: Capture_IMGSIZE_ID =%d", __FUNCTION__, Capture_IMGSIZE_ID);
			break;

		case 1:
			Capture_IMGSIZE_ID = SIZE_XGA_1024x768;
			MP_ALERT("%s: Capture_IMGSIZE_ID =%d", __FUNCTION__, Capture_IMGSIZE_ID);
			break;

		case 2:
			Capture_IMGSIZE_ID = SIZE_SXGA_1280x1024;
			MP_ALERT("%s: Capture_IMGSIZE_ID =%d", __FUNCTION__, Capture_IMGSIZE_ID);
			break;

		case 3:
			Capture_IMGSIZE_ID = SIZE_2M_1600x1200;
			MP_ALERT("%s: Capture_IMGSIZE_ID =%d", __FUNCTION__, Capture_IMGSIZE_ID);
			break;

		case 4:
			Capture_IMGSIZE_ID = SIZE_QXGA_2048x1536;
			MP_ALERT("%s: Capture_IMGSIZE_ID =%d", __FUNCTION__, Capture_IMGSIZE_ID);
			break;

		case 5:
			Capture_IMGSIZE_ID = SIZE_5M_2560x1920;
			MP_ALERT("%s: Capture_IMGSIZE_ID =%d", __FUNCTION__, Capture_IMGSIZE_ID);
			break;	
			
		default:
			MP_ALERT("%s:Error CaputreGetID = %d", __FUNCTION__, CaputreGetID);
			__asm("break 100");
			break;
	}
  
	MP_ALERT("Capture_IMGSIZE_ID =%d",Capture_IMGSIZE_ID);

	return Sensor_Capture_SensorTool(fileHandle, Capture_IMGSIZE_ID);

#else

	#if defined(SENSOR_TYPE_OV5642)	

		return Sensor_Capture_OV5642(fileHandle, Capture_IMGSIZE_ID);

	#elif defined(SENSOR_TYPE_OV2643)

		return Sensor_Capture_OV2643(fileHandle, Capture_IMGSIZE_ID);

	#elif defined(SENSOR_TYPE_OV2643_JH43A)

		return Sensor_Capture_OV2643_JH43A(fileHandle, Capture_IMGSIZE_ID);

	#elif defined(SENSOR_TYPE_NT99140)

		return Sensor_Capture_NT99140(fileHandle, Capture_IMGSIZE_ID);
	
	#elif defined(SENSOR_TYPE_CVBS_INPUT)

       //Capture_IMGSIZE_ID = SIZE_VGA_640x480;
		return Sensor_Capture_CVBS(fileHandle, Capture_IMGSIZE_ID);

	#elif defined(SENSOR_TYPE_HM1355_AWA)

		return Sensor_Capture_HM1355_AWA(fileHandle, Capture_IMGSIZE_ID);

	#else

		#error  Not Sellect SENSOR_TYPE

	#endif

#endif
}


int Sensor_Capture_Encode(STREAM *fileHandle, int SensorInWidth, int SensorInHeight, BYTE sensor_IMG_ID)
{	
	IPU *ipu = (IPU *) IPU_BASE;
	DWORD JpegBufSize=SensorInWidth * SensorInHeight * 2;
	BYTE *JpegBuf;// = (BYTE*)ext_mem_malloc(JpegBufSize);
	DWORD SensorEvent;
	DWORD IMG_size = 0;
	ST_IMGWIN Original_Win;

	int iErrorCode;

    JpegBuf = g_bImageData_Buf;


	iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 5000);//ipw1
	if(iErrorCode < 0)
		goto Error;

	iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 5000);//ipw1
	if(iErrorCode < 0)
		goto Error;

	
	iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 5000);//ipw1
	if(iErrorCode < 0)
		goto Error;
	
    iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 5000);//ipw1
	if(iErrorCode < 0)
		goto Error;
   
	/*close sensor input*/
	ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
	while(1)
	{
		if((ipu->Ipu_reg_100&BIT0)==0)//確定有寫進去
			break;
	}


	//For Debug
	//Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "IPW1", "bin", ipu->Ipu_reg_A3, 1600*1200*2);
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
        Original_Win.pdwStart=ipu->Ipu_reg_F2;
#else
	Original_Win.pdwStart=ipu->Ipu_reg_A3;
#endif

	switch (sensor_IMG_ID)
	{
		case SIZE_VGA_640x480:
	        Original_Win.wWidth  = 640;
			Original_Win.wHeight = 480;
            break;
				
		case SIZE_XGA_1024x768:
		    Original_Win.wWidth  = 1024;
		    Original_Win.wHeight = 768;
          	break;

		case SIZE_SXGA_1280x1024:
		    Original_Win.wWidth  = 1280;
		    Original_Win.wHeight = 1024;
            break;

	    case SIZE_2M_1600x1200:
		   	Original_Win.wWidth  = 1600;
		   	Original_Win.wHeight = 1200;
       	    break;

		case SIZE_QXGA_2048x1536:
			Original_Win.wWidth  = 2048;
			Original_Win.wHeight = 1536;
		    break;	

		case SIZE_5M_2560x1920:
			Original_Win.wWidth  = 2560;
			Original_Win.wHeight = 1920;
			break;
				
		default:
			MP_ALERT("--E-- %s:Capture size is not support !!!");
			return -1;
			break;
	}
	
    Original_Win.dwOffset=Original_Win.wWidth<<1;

    if (!JpegBuf)
    {
        MP_ALERT("--E-- %s: memory alloc fail !!!", __FUNCTION__);

        return -2;
    }

    JpegBuf = (BYTE *) ((DWORD) JpegBuf | 0xA0000000);

	//--Scaling
	//ST_IMGWIN Target_Win;
	//BYTE *TempBuffer;
	//memset(&Target_Win, 0, sizeof(ST_IMGWIN));
		
	IMG_size=ImageFile_Encode_Img2Jpeg(JpegBuf, &Original_Win);//(JpegBuf, &Original_Win);

	API_Sensor_Stop();
		
	if(JpegBufSize<IMG_size)
	{
			mpDebugPrint("--E-- %s: memory overflow", __FUNCTION__);
            __asm("break 100");

            return -3;
	}
    else
    {
        FileWrite(fileHandle, (BYTE *) JpegBuf, JpegBufSize);
        FileClose(fileHandle);
    }
//	if(JpegBuf != NULL)
//		ext_mem_free(JpegBuf);

    //--Scaling
	//if(Target_Win.pdwStart)
	//	ext_mem_free(Target_Win.pdwStart);

    return PASS;

Error:
	API_Sensor_Stop();
	return FAIL;

}


#endif



void SetSensorOverlayEnable(BYTE flag)
{
		overlay_enable=flag;
}

void SetImageSize(BYTE ID)
{
		sensor_IMGSIZE_ID=ID;
}

void SetSensorInterfaceMode(BYTE Mode)
{
		sensor_mode=Mode;
}

void Drive_Sensor(void)
{
	#if  defined(SENSOR_TYPE_MAGIC_SENSOR)

		Drive_Sensor_SensorTool();

	#elif defined(SENSOR_TYPE_OV5642)

		Drive_Sensor_OV5642();

	#elif defined(SENSOR_TYPE_S5K6AAFX13)

		Drive_Sensor_S5K6AAFX13();

	#elif defined(SENSOR_TYPE_OV2643)

		Drive_Sensor_OV2643();
	
	#elif defined(SENSOR_TYPE_OV2643_JH43A)

		Drive_Sensor_OV2643_JH43A();
		
	#elif defined(SENSOR_TYPE_NT99140)
#if (PRODUCT_UI==UI_WELDING)
		Sensor_ChangeIO_Init();
		Sensor_Channel_Set(1);
		Local_Sensor_GPIO_Reset();
		Drive_Sensor_NT99140();
		//IODelay(1000);
		Sensor_Channel_Set(0);
		Local_Sensor_GPIO_Reset();
#elif (PRODUCT_UI==UI_SURFACE)
		Sensor_ChangeIO_Init();
		Sensor_Channel_Set(1);
		Local_Sensor_GPIO_Reset();
#endif

		Drive_Sensor_NT99140();
	
	#elif defined(SENSOR_TYPE_CVBS_INPUT)
	
		//	Drive_Sensor_CVBS();
	
	#elif defined(SENSOR_TYPE_HM1355_AWA)

		Drive_Sensor_HM1355_AWA();
	
	#else

		#error  Not Sellect SENSOR_TYPE
	
	#endif
	
}

int API_Sensor_Initial(void)
{
	int iErrorCode;

	//only for Debug only
	//API_SetSensorWindow(40, 60, 720, 480);
#if (PRODUCT_UI==UI_WELDING)
	Sensor_DisplayWindow_Set();
#elif (PRODUCT_UI==UI_SURFACE)
	API_SetSensorWindow(160, 0, 640, 600);
#endif

	#if  defined(SENSOR_TYPE_MAGIC_SENSOR)

		iErrorCode = Global_Sensor_Initial_SensorTool();

	#elif defined(SENSOR_TYPE_OV5642)	

		iErrorCode = Global_Sensor_Initial_OV5642();

	#elif defined(SENSOR_TYPE_S5K6AAFX13)

		iErrorCode = Global_Sensor_Initial_S5K6AAFX13();

	#elif defined(SENSOR_TYPE_OV2643)

		iErrorCode = Global_Sensor_Initial_OV2643();

	#elif defined(SENSOR_TYPE_OV2643_JH43A)

		iErrorCode = Global_Sensor_Initial_OV2643_JH43A();

	#elif defined(SENSOR_TYPE_NT99140)

		iErrorCode = Global_Sensor_Initial_NT99140();

	#elif defined(SENSOR_TYPE_CVBS_INPUT)

	    iErrorCode = Global_Sensor_Initial_CVBS();

	#elif defined(SENSOR_TYPE_HM1355_AWA)

		iErrorCode = Global_Sensor_Initial_HM1355_AWA();

	#else

		#error	Not Sellect SENSOR_TYPE

	#endif

   API_Set_PCCAM_GetReadyBufCnt(0);/*Initial*/
   Api_Clear_PCCAM_SwitchBuf_flag();
   API_Set_PCCAM_ReadyBuf_state(0);

  API_SensorUsb_SetEmptyFlag(0);
  API_SensorUsb_SetEmptyFlag(1);
  API_SensorUsb_SetEmptyFlag(2);
	
	MP_DEBUG2("%s: After Sensor initial iErrorCode = %d", __FUNCTION__, iErrorCode);

	return iErrorCode;

}

#endif


/*maximum ratio is (sensor input width /Win_Width) or  (sensor input height /Win_Height)*/
/*maximum ratio is 1/4. If ratio > Max ratio, the display window may be cropped from sensor input  */
/*The maximum display window is the size of sensor input.*/
/*Ex. sensor input = 640x480*/
/*      The maximum display window is 640x480.*/
int API_SetSensorWindow(WORD Win_PosX, WORD Win_PosY,WORD Win_Width, WORD Win_Height)
{	
	ST_IMGWIN *win;
	WORD PanelW,PanelH;

	win = Idu_GetCurrWin();
 	PanelW = win->wWidth;
 	PanelH = win->wHeight;

	if((Win_Width == 0) || (Win_Height == 0))
	{
		SensorWindow_Width  = PanelW;
		SensorWindow_Height = PanelH;
	}
	else
	{
		SensorWindow_Width   = Win_Width;
		SensorWindow_Height  = Win_Height;
	}
	if (Win_PosX>=PanelW)
		SensorWindow_PosX=0;
	else
		SensorWindow_PosX    = Win_PosX;
	if (SensorWindow_PosY>=PanelH)
		SensorWindow_PosY=0;
	else
		SensorWindow_PosY    = Win_PosY;
	if (SensorWindow_PosX+SensorWindow_Width>PanelW)
		SensorWindow_Width=PanelW-SensorWindow_PosX;
	if (SensorWindow_PosY+SensorWindow_Height>PanelH)
		SensorWindow_Height=PanelH-SensorWindow_PosY;
	SensorWindow_setFlag = ENABLE;
	MP_DEBUG("## API_SetSensorWindow (%d,%d) (%d*%d)",SensorWindow_PosX,SensorWindow_PosY,SensorWindow_Width,SensorWindow_Height);
	return PASS;	
}

void Sensor_Input_Enable(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu->Ipu_reg_100 |= BIT0; //SenInEn
}

/*set_flag == ENABLE , IPW2 write into SDRAM*/
/*set_flag == DISABLE , IPW2 do not  write into SDRAM*/
void Set_Display_flag(BYTE set_flag)
{
	Display_Enable_Flag = set_flag;
}

BYTE Get_Display_flag(void)
{
	return Display_Enable_Flag;
}


void API_Open_Write_Into_EncBuf(void)
{
	IPU *ipu = (IPU *) IPU_BASE;

	ipu->Ipu_reg_F0 &= ~BIT7;//open IPW2
}

void API_Close_Write_Into_EncBuf(void)
{
	IPU *ipu = (IPU *) IPU_BASE;

	ipu->Ipu_reg_F0 |= BIT7;/*Close IPW2 write path*/
}

void API_Open_IPW1(void)
{
	IPU *ipu = (IPU *) IPU_BASE;

	ipu->Ipu_reg_F0 &= ~BIT6; //open IPW1

}

void API_Close_IPW1(void)
{
	IPU *ipu = (IPU *) IPU_BASE;

	ipu->Ipu_reg_F0 |= BIT6;/*Close IPW1 write path*/						  
}

#endif


/*for PCCam*/
DWORD API_PCCAM_GetReadyBufCnt(void)
{
	return g_PCCAM_GetReadyBufCnt;
}

/*for PCCam*/

void API_Set_PCCAM_GetReadyBufCnt(DWORD SetReadyBufCnt)
{
	g_PCCAM_GetReadyBufCnt = SetReadyBufCnt;
}


DWORD Api_Get_PCCAM_SwitchBuf_flag(void)
{
    return g_PCCAM_SwitchBuf_flag;
}


void Api_Set_PCCAM_SwitchBuf_flag(void)
{
    return g_PCCAM_SwitchBuf_flag = 1;
}

void Api_Clear_PCCAM_SwitchBuf_flag(void)
{
    return g_PCCAM_SwitchBuf_flag = 0;
}


DWORD API_Get_PCCAM_ReadyBuf_state(void)
{
	return g_PCCAM_ReadyBuf_state;
}

void API_Set_PCCAM_ReadyBuf_state(DWORD SetState)
{
	g_PCCAM_ReadyBuf_state = SetState;
}



DWORD API_SensorUsbUsedBufNum(void)
{
	return 3;
}

void API_SensorUsb_EnableShareBuf(void)
{
mpDebugPrint("%s", __func__);
	SensorUsb_Use = 1;
}

void API_SensorUsb_DisableShareBuf(void)
{
mpDebugPrint("%s", __func__);
	SensorUsb_Use = 0;
}

BYTE API_SensorUsb_GetShareBuf_status(void)
{
	return SensorUsb_Use;
}


void API_SensorUsb_SetEmptyFlag(DWORD Num)
{
	SensorUsbShareInfo[Num].Empty_Flag = 1;
}

void API_SensorUsb_ClrEmptyFlag(DWORD Num)
{
	SensorUsbShareInfo[Num].Empty_Flag = 0;
}


BYTE API_SensorUsb_GetEmptyFlag(DWORD Num)
{
	return SensorUsbShareInfo[Num].Empty_Flag;
}

struct SENSOR_SHARE_BUF *API_SensorUsbGetShareBufInfo(DWORD Num)
{
	return &SensorUsbShareInfo[Num];
}

void API_SensorUsbSetShareBufInfo(DWORD Num, BYTE *BufPtr, BYTE Empty_Flag)
{
	SensorUsbShareInfo[Num].Buf = BufPtr;
	SensorUsbShareInfo[Num].Empty_Flag = Empty_Flag;
}


#endif /*SENSOR_ENABLE*/


