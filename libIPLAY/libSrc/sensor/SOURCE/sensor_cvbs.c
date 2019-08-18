/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#define DumpImageFlag 0
#define debug_IPW1_flag 0
#define USE_SYSTEM_I2C_FUNCTION 1
 
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "idu.h"
#include "sensor.h"

#include "peripheral.h"

#if SAVE_FILE_TO_SPI
#include "..\..\..\..\MiniDV\main\INCLUDE\SPI_Ex.h"
#endif

//#include "../../SpyCamera/main/include/ui_timer.h"

#if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
/*
W = Win_W / VAL_1 * VAL_2
H  = Win_H / VAL_3 
*/

#define VAL_1 3
#define VAL_2 4 // 4
#define VAL_3 2

#endif



#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_CVBS_INPUT))


#define CVBS_RECORD_WIDTH  640
#define CVBS_RECORD_HEIGHT 480


/*The buffer pointer reference by JPEG and Sensor input -- Capure mode*/
BYTE *g_bImageData_Buf = NULL;


extern int wait_sensor;
extern int SensorInputWidth;
extern int SensorInputHeight;
extern int frameCNT;
extern long Start_Time;
extern int DoubleBuffer;
extern ST_IMGWIN SensorInWin[3];
extern int Setflag;
extern BYTE sensor_mode;
extern BYTE sensor_IMGSIZE_ID;
extern BYTE sensor_Input_Format;

extern BYTE overlay_enable;
//extern BYTE *overlay_image_addr;
extern ST_IMGWIN overlay_image_Win;
extern BYTE OverlayKeyY;
extern BYTE OverlayKeyCb;
extern BYTE OverlayKeyCr;

extern ST_IMGWIN Capture_overlay_image_Win;
extern BYTE Capture_OverlayKeyY;
extern BYTE Capture_OverlayKeyCb;
extern BYTE Capture_OverlayKeyCr;

extern BYTE MultiBufFlag;

ST_IMGWIN overlap_win;

extern WORD SensorWindow_Width  ;
extern WORD SensorWindow_Height ;
extern BYTE SensorWindow_setFlag ;
extern WORD SensorWindow_PosX;
extern WORD SensorWindow_PosY;

char StrBuf[20];
char StrBuf_Old[20];

extern BYTE Motion_Detection_Value;

static BYTE g_CVBS_ZoomPreview_flag=0;

static E_SENSOR_ZOOM_LEVEL g_CVBS_ZoomStep=0;

static BYTE g_EvenFrame = 0;


#if ___PLATFORM___ == 0x660
#define for660_test 1
#else
#define for660_test 0
#endif

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

static void Local_HW_GPIO_Pin_ssen_0(void)
{
   SetGPIOValue(SENSOR_GPIO_SSEN, 0);
}

static void Local_HW_GPIO_Pin_ssen_1(void)
{
	 SetGPIOValue(SENSOR_GPIO_SSEN, 1);
}

static void Local_HW_GPIO_Pin_reset_0(void)
{	
	 SetGPIOValue(SENSOR_GPIO_RESET, 0);
}

static void Local_HW_GPIO_Pin_reset_1(void)
{
	 SetGPIOValue(SENSOR_GPIO_RESET, 1);
}



static void Local_HW_GPIO_Pin_Set(void)
{
Local_HW_GPIO_Pin_reset_0();
	  IODelay(10);
Local_HW_GPIO_Pin_reset_1();
	  IODelay(1);


	
#if 0
	register GPIO *gpio = (GPIO *) (GPIO_BASE);
//pin set(==GPIO set)
	gpio->Fgpcfg[0]&=~(BIT13|BIT29);//pin13=00
	gpio->Fgpcfg[0]|=BIT29;//pin13=10

	gpio->Fgpcfg[0]&=~( (BIT15|BIT31)|(BIT14|BIT30) );//pin14=00 pin15=00
	gpio->Fgpcfg[0]|=BIT31|BIT30;

	gpio->Fgpcfg[1]&=~( (BIT0|BIT16)|(BIT1|BIT17)|(BIT2|BIT18)|(BIT3|BIT19)|(BIT4|BIT20) );
	gpio->Fgpcfg[1]|=BIT16|BIT17|BIT18|BIT19|BIT20;

	gpio->Fgpcfg[1]&=~( (BIT9|BIT25)|(BIT10|BIT26)|(BIT13|BIT29)|(BIT15|BIT31) );
	gpio->Fgpcfg[1]|=BIT25|BIT26|BIT29|BIT31;

	gpio->Fgpdat[0]&=~(BIT31);//pin15=00
	gpio->Fgpdat[1]&=~( (BIT16)|(BIT17)|(BIT18)|(BIT19)|(BIT20) );
	gpio->Fgpdat[1]&=~( (BIT25)|(BIT26)|(BIT29)|(BIT31) );

#if for660_test//for660 test
	//pin set(==GPIO set)
	gpio->Pgpcfg&=~(BIT2|BIT18);//reset
	gpio->Pgpcfg|=BIT2;//reset PWDN use function 2
	gpio->Pgpcfg&=~(BIT3|BIT19);//PWDN
	gpio->Pgpcfg|=BIT3;//reset PWDN use function 2
	gpio->Pgpdat|=BIT18|BIT19;
#else
/*
//pwdn vgpio16
	gpio->Vgpcfg1 &= ~(BIT0|BIT16);//sw gpio
	gpio->Vgpdat1 |= BIT16;
//reset vgpio18
	gpio->Vgpcfg1 &= ~(BIT2|BIT18);//sw gpio
	gpio->Vgpdat1 |= BIT18;
*/
#endif
	/*
	while(1)
	{
	  Local_HW_GPIO_Pin_reset_0();
		    IODelay(1);
	  Local_HW_GPIO_Pin_reset_1();
		    IODelay(1);

	  Local_HW_GPIO_Pin_ssen_0();
		    IODelay(1);
	  Local_HW_GPIO_Pin_ssen_1();
		    IODelay(1);


	} */


#if (USE_SYSTEM_I2C_FUNCTION == 0)
	//set gpio pin sclk sdata  //use i2c must set
	#if for660_test//for660 test
	gpio->Gpcfg0|=(BIT0|BIT16);//function3
	gpio->Gpcfg0|=(BIT1|BIT17);//function3
	#else
	gpio->Vgpcfg1 |= BIT7|BIT23; //i2C CLK function3
	gpio->Vgpcfg1 |= BIT8|BIT24; //i2C SDA function3
	CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
	regClockPtr->PinMuxCfg &= ~(BIT3 | BIT4);
	regClockPtr->PinMuxCfg |= BIT4;
	#endif
#endif
	/*
	while(1)
	{
	I2C_Write_Addr16_Value16_LoopMode(0x78>>1,  0xfcfc,  0x7000);
	IODelay(10);
	}*/
#endif
}

static void Local_HW_MCLK_Set(void)
{
	int SensorFrameRate=24;
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	clock->Clkss2 &=~(BIT7|BIT8|BIT9|BIT10|BIT11);//bit7-11 mclk
	//clock->Clkss2 |=BIT8|BIT9|BIT10;
	//clock->Clkss2 |=BIT7|BIT8; //24mhz
	//clock->Clkss2 |=BIT7|BIT9;//12
	//clock->Clkss2 |=BIT9;//16mhz pll1/6
	//clock->Clkss2 |=BIT8|BIT9;//6
	//clock->Clkss2 |=BIT9|BIT10; //20mhz pll2/6
#if (PANEL_ID == PANEL_NONE)
	Clock_PllFreqSet(CLOCK_PLLIDU_INDEX, 144000000);
#endif
	switch(SensorFrameRate)
	{
		case 6:
				clock->Clkss2 |=BIT8|BIT9;//6
			break;
		case 12:
				clock->Clkss2 |=BIT7|BIT9;//12
			break;
		case 16:
				clock->Clkss2 |=BIT9;//16mhz pll1/6
			break;
		case 20:
				clock->Clkss2 |=BIT9|BIT10; //20mhz pll2/6
			break;
		case 24:
			clock->Clkss2 |= BIT8;	
			
			break;
		default:
				clock->Clkss2 |=BIT8|BIT9;//6
			break;

	}

}



#if (USE_SYSTEM_I2C_FUNCTION == 0)
static void Local_HW_I2C_CLK_Set(void)
{
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	//i2c clk
	clock->Clkss2 &=~(BIT19|BIT20|BIT21);
	#if 1//slow i2c
	clock->Clkss2 |=BIT20|BIT21;
	#else
	clock->Clkss2 |=BIT21;
	#endif


}
#endif
static void HW_SensorMainScale_Set(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	SDWORD width_ratio  = 0;
	SDWORD height_ratio = 0;

        DWORD SensorW,SensorH;
	DWORD WindowW,WindowH;
	SDWORD  vsub_ratio = 0,hsub_ratio = 0;

        ST_IMGWIN *win;
	WORD PanelW,PanelH;
	DWORD Scale_ratio_adjust=800;


        SensorW = SensorInputWidth;
        SensorH = SensorInputHeight;

	WindowW = SensorWindow_Width;
	WindowH = SensorWindow_Height;

	if(SensorWindow_setFlag == 1)
	{
		PanelW = SensorWindow_Width;
		PanelH = SensorWindow_Height;
	}
	else
	{
		win = Idu_GetCurrWin();
	
		PanelW = win->wWidth;
		PanelH = win->wHeight;
	}

#if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
     PanelW = ((PanelW/VAL_1) * VAL_2);
     PanelH = PanelH / VAL_3;
#endif

#if MP662W_MINIDV1_UI
	SetScaleForSensorDisplay(SensorW,SensorH>>1,PanelW,PanelH);
#else

	//scaling scaling V=input_width/output_width-1
#if ((TCON_ID==TCON_NONE) || (SENSOR_WITH_DISPLAY != ENABLE))
	width_ratio  = 0x8000;//must set default
	height_ratio = 0x8000;//must set default
#else
    if(SensorW <= PanelW) //width scale up 
    {
      width_ratio  =(SensorW<<15) / PanelW;

      MP_DEBUG("SensorInputWidth = %d,PanelW =%d",SensorW,PanelW );
      MP_DEBUG("####### %s: width_ratio = 0x%x", __FUNCTION__, width_ratio);

      if((SensorWindow_setFlag == 1) && (SensorW == WindowW))
      {
        width_ratio  = 0x8000;
      }

      hsub_ratio = 0;  
    }
    else
    {  /*Width scale down*/
      width_ratio  =(SensorW<<15) / PanelW;
      hsub_ratio = (width_ratio >> 15);

      width_ratio = (width_ratio - (hsub_ratio<<15) ) ;
	   
      MP_DEBUG2("$$$$$$ width_ratio = %d, hsub_ratio =%d", width_ratio, hsub_ratio);

      width_ratio |= BIT15;
    }

    hsub_ratio = hsub_ratio -1;
    width_ratio -=((Scale_ratio_adjust>>1)*3);
	
    if(hsub_ratio > 7)
      hsub_ratio = 7;
    if(hsub_ratio <= 0)
      hsub_ratio = 0;

    if(width_ratio >= 0x8000)
      width_ratio = 0x8000;
	
    if(width_ratio <= 0)
      width_ratio = 0x0001;

   /*==================================*/

    if(SensorH <= PanelH) /*Height scale up*/
    {
      height_ratio =(SensorH<<15)/ PanelH;

      if(height_ratio>=0x8000)
        height_ratio=0x8000;

      if(height_ratio <= 0)
        height_ratio = 0x0001;

      if((SensorWindow_setFlag == 1) && (SensorH == WindowH))
      {
        height_ratio = 0x8000;
      }

      vsub_ratio = 0;
    }
    else
    {  /*Height scale down*/
       height_ratio  =(SensorH<<15) / PanelH;
       vsub_ratio = (height_ratio >> 15);

       height_ratio = (height_ratio - (vsub_ratio<<15) ) ;
	   
       MP_DEBUG2("$$$$$$ height_ratio = %d, vsub_ratio =%d", height_ratio, vsub_ratio);

       height_ratio |= BIT15;
    }

	  height_ratio = height_ratio>> 1;
	  height_ratio -= Scale_ratio_adjust;

      vsub_ratio = vsub_ratio -1;

	
    if(vsub_ratio > 7)
      vsub_ratio = 7;

    if(vsub_ratio <= 0)
      vsub_ratio = 0;

    if(height_ratio >= 0x8000)
      height_ratio = 0x8000;
	
    if(height_ratio <= 0)
      height_ratio = 0x0001;
	
#endif

    mpDebugPrint("#### WindowW =%d, WindowH=%d",WindowW,WindowH);
    mpDebugPrint("#### hsub_ratio =%x, vsub_ratio =%x", hsub_ratio, vsub_ratio);
    mpDebugPrint("#### width_ratio =%d, SensorW =%d,PanelW=%d",width_ratio, SensorW, PanelW);
    mpDebugPrint("#### height_ratio=%d, SensorH=%d,PanelH=%d",height_ratio, SensorH,PanelH);
    mpDebugPrint("%s:scale ratio width_ratio=0x%x, height_ratio=0x%x", __FUNCTION__, width_ratio, height_ratio);

    ipu->Ipu_reg_21 = (vsub_ratio<<8) | hsub_ratio;
    ipu->Ipu_reg_22 = width_ratio  << 16;
    ipu->Ipu_reg_23 = height_ratio << 16;
#endif

}

static void Local_HW_SensorScale_Set(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	SDWORD width_ratio=0;
	SDWORD height_ratio=0;

        ST_IMGWIN *win1;
	WORD PanelW,PanelH;

	WORD SrcWidth,SrcHeight;
	WORD TrgWidth,TrgHeight;
	win1 = Idu_GetCurrWin();
	
	PanelW = win1->wWidth;
	PanelH = win1->wHeight;

	//scaling scaling V=input_width/output_width-1


	MP_DEBUG2("#### SensorWindow_Width =%d, SensorWindow_Height=%d",SensorWindow_Width,SensorWindow_Height);
	MP_DEBUG3("#### width_ratio =%d, SrcWidth =%d,panel_w=%d",width_ratio, SrcWidth, PanelW);
	MP_DEBUG3("#### height_ratio=%d, SrcHeight=%d,panel_H=%d",height_ratio, SrcHeight, PanelH);
	MP_DEBUG3("%s:scale ratio width_ratio=%d, height_ratio=%d", __FUNCTION__, width_ratio, height_ratio);

#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
   
   if(SensorWindow_setFlag == 1)
   {
	 SrcWidth = SensorWindow_Width;
	 SrcHeight = SensorWindow_Height;
   }
   else
   {
   SrcWidth  = PanelW;
   SrcHeight = PanelH;
   }
   
   TrgWidth  = CVBS_getRecordWidth();
   TrgHeight = CVBS_getRecordHeight();

   MP_DEBUG2("---> %s: SrcWidth = %d", __FUNCTION__, SrcWidth);
   MP_DEBUG2("---> %s: SrcHeight = %d", __FUNCTION__, SrcHeight);
   MP_DEBUG2("---> %s: TrgWidth = %d", __FUNCTION__, TrgWidth);
   MP_DEBUG2("---> %s: TrgHeight = %d", __FUNCTION__, TrgHeight);

   width_ratio=(SrcWidth<<10)/TrgWidth-(1<<10);
   height_ratio=(SrcHeight<<10)/TrgHeight-(1<<10);
		
#else	

	SrcWidth  = SensorInputWidth;
    SrcHeight = SensorInputHeight;

    TrgWidth  = PanelW;
	TrgHeight = PanelH;

	if(SensorWindow_setFlag == 1)//SensorWindow_setFlag
	{
		width_ratio=(SrcWidth<<10)/SensorWindow_Width-(1<<10);
		height_ratio=(SrcHeight<<10)/SensorWindow_Height-(1<<10);
	}
	else
	{
		width_ratio=(SrcWidth<<10)/TrgWidth-(1<<10);
		height_ratio=(SrcHeight<<10)/TrgHeight-(1<<10);
	}
#endif


	#if ((TCON_ID==TCON_NONE) || (SENSOR_WITH_DISPLAY != ENABLE))
		width_ratio=(SrcWidth<<10)/320-(1<<10);//must set default
		height_ratio=(SrcHeight<<10)/240-(1<<10);//must set default
	#endif

	
	height_ratio-=4;
	width_ratio-=4;
	if(width_ratio>=0xC00)
		width_ratio=0xC00;

	if(width_ratio <= 0)
		width_ratio = 0;
	
	if(height_ratio>=0xC00)
		height_ratio=0xC00;

	if(height_ratio <= 0)
		height_ratio = 0;

#if(USE_IPW1_DISPLAY_MOTHOD == DISABLE)

	if((SensorWindow_setFlag == 1) && (SrcWidth == SensorWindow_Width) && (SrcHeight == SensorWindow_Height))
	{
		width_ratio  = 0;
		height_ratio = 0;
	}
#endif

	ipu->Ipu_reg_F4 = (1<<31)+(height_ratio<<16)+width_ratio ;

	mpDebugPrint("#### width_ratio =%d, SrcWidth =%d,TrgWidth=%d",width_ratio, SrcWidth, TrgWidth);
	mpDebugPrint("#### height_ratio=%d, SrcHeight=%d,TrgHeight=%d",height_ratio, SrcHeight, TrgHeight);
	mpDebugPrint("----> %s: ipu->Ipu_reg_F4 = 0x%x", __FUNCTION__, ipu->Ipu_reg_F4);

	MP_DEBUG2("----> %s: ipu->Ipu_reg_F4 = 0x%x", __FUNCTION__, ipu->Ipu_reg_F4);

	/*
#if for660_test//for660 test

		#if debug_800x480_panel
			ipu->Ipu_reg_F4 = (1<<31)+(0x197<<16)+0x264 ;//1280 x 720 -> 800x480


		#else
			ipu->Ipu_reg_F4 = (1<<31)+(2052<<16)+3072 ;//1280 x 720 -> 320x240
		#endif

#else

		ipu->Ipu_reg_F4 = (1<<31)+(2052<<16)+3072 ;//1280 x 720 -> 320x240

#endif
	*/

}

static void Local_HW_InputWindow1_Set(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	//ipu->Ipu_reg_102 = 0x02ff03ff ;	// window size
	//ipu->Ipu_reg_102 = (719<<16)+1279 ;


    MP_DEBUG2("%s: SensorInputWidth  = %d ", __FUNCTION__, SensorInputWidth);
    MP_DEBUG2("%s:SensorInputHeight = %d ", __FUNCTION__,SensorInputHeight);
	
	ipu->Ipu_reg_102 = ((SensorInputHeight-1)<<16)+(SensorInputWidth-1) ;
}

static void Local_HW_InputWindow2_Set(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *win = Idu_GetNextWin();
	WORD PanelWidth =0;
	WORD PanelHeight= 0;
	WORD DisplayWin_W,DisplayWin_H;
	// New IPW
	ipu->Ipu_reg_F5 = 0x01000000 ;	//Enable bit 24, window enable
	//ipu->Ipu_reg_F5 = 0 ;
	//ipu->Ipu_reg_F6 = 0x00000257 ;//600 panel size window

	PanelWidth	= win->wWidth;
	PanelHeight = win->wHeight;
#if (USE_IPW1_DISPLAY_MOTHOD == ENABLE)

   // sensor windth , height
  ipu->Ipu_reg_F6 = CVBS_getRecordHeight() - 1 ;
  ipu->Ipu_reg_F7 = CVBS_getRecordWidth()  - 1 ;	
  ipu->Ipu_reg_F1 = 0;

#else

#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))
	if(SensorWindow_setFlag == ENABLE)//SensorWindow_setFlag
	{
		if((SensorWindow_PosX ==0) && (SensorWindow_PosY == 0))
		{
			ipu->Ipu_reg_F6 = SensorWindow_Height-1 ;
			ipu->Ipu_reg_F7 = SensorWindow_Width-1 ;	
			ipu->Ipu_reg_F1 = (PanelWidth - SensorWindow_Width)*2;
		}
		else
		{
			if((PanelWidth - SensorWindow_PosX)<= SensorWindow_Width)
			{
				MP_ALERT("############# case 1");
				DisplayWin_W = PanelWidth - SensorWindow_PosX;
			}else
			{
					
				MP_ALERT("############# case 2");
					
				DisplayWin_W =	SensorWindow_Width;
			}
	
			if((PanelHeight - SensorWindow_PosY)<= SensorWindow_Height)
			{
				MP_ALERT("############# case 1");
				DisplayWin_H = PanelHeight - SensorWindow_PosY;
			}else
			{
					
				MP_ALERT("############# case 2");
					
				DisplayWin_H =	SensorWindow_Height;
			}
	
			MP_ALERT("####DisplayWin_W =%d",DisplayWin_W);
			MP_ALERT("####DisplayWin_H =%d",DisplayWin_H);
				
			ipu->Ipu_reg_F6 = DisplayWin_H-1 ;
			ipu->Ipu_reg_F7 = DisplayWin_W-1 ;	
			ipu->Ipu_reg_F1 = (PanelWidth - DisplayWin_W)*2;
			MP_ALERT("####ipu->Ipu_reg_F1 =%d",ipu->Ipu_reg_F1);			
		}
			
	} 
	else
	{
		if((PanelWidth <= SensorInputWidth) && (PanelHeight <= SensorInputWidth))
		{
			ipu->Ipu_reg_F6 = PanelHeight-1 ;
			ipu->Ipu_reg_F7 = PanelWidth-1 ;
			ipu->Ipu_reg_F1 = 0x00000000 ;
		}
		else
		{
			if(PanelWidth <= SensorInputWidth)
			{
				DisplayWin_W = PanelWidth;
			}else
			{					
				DisplayWin_W =	SensorInputWidth;
			}
	
			if(PanelHeight<= SensorInputWidth)
			{
				DisplayWin_H = PanelHeight;
			}else
			{
				DisplayWin_H =	SensorInputWidth;
			}
	
			MP_ALERT("####DisplayWin_W =%d",DisplayWin_W);
			MP_ALERT("####DisplayWin_H =%d",DisplayWin_H);
				
			ipu->Ipu_reg_F6 = DisplayWin_H-1 ;
			ipu->Ipu_reg_F7 = DisplayWin_W-1 ;	
			ipu->Ipu_reg_F1 = (PanelWidth - DisplayWin_W)*2;
			MP_ALERT("####ipu->Ipu_reg_F1 =%d",ipu->Ipu_reg_F1);
	
			
		}
	}
#else
	ipu->Ipu_reg_F6 = 240-1 ;//must set default
	ipu->Ipu_reg_F7 = 320-1 ;//must set default
	ipu->Ipu_reg_F1 = 0x00000000 ;
#endif

#endif //  !=USE_IPW1_DISPLAY_MOTHOD


}

static void Local_HW_SensorInput_Enable(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu->Ipu_reg_100 |= BIT0; //close SenInEn
	/*
	while(1)
	{
		if(ipu->Ipu_reg_100&BIT0)//絋﹚Τ糶秈
			break;
	}*/
}

static void Local_HW_SensorInput_Disable(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
	while(1)
	{
		if((ipu->Ipu_reg_100&BIT0)==0)//絋﹚Τ糶秈
			break;
	}
}

static void Local_HW_StopIPW1(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu->Ipu_reg_F0 |= BIT6; 
	while(1)
	{
		if(ipu->Ipu_reg_F0&BIT30)//絋﹚竒Ч俱frame糶秈
			break;
	}
}

static void Local_HW_StopIPW2(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu->Ipu_reg_F0 |= BIT7; //close SenInEn
	while(1)
	{
		if(ipu->Ipu_reg_F0&BIT31)//絋﹚竒Ч俱frame糶秈
			break;
	}
}
#if 1


//0:only panel(ipw2)
//1:only ipw1
//2:ipw1+ipw2
static void Global_HW_Path_Set(void)
{
		IPU *ipu = (IPU *) IPU_BASE;

		CloseAndTimerToOpenBacklight(800);
		ipu->Ipu_reg_F0 |= BIT6|BIT7;//close ipw1+ipw2
		#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))
			if(sensor_mode == MODE_PCCAM)
			{
			  //PCCAM mode IPW2 will not write into DRAM(IDU display)
				ipu->Ipu_reg_F0 &= ~BIT6;
			}
			else
			{
				
				if(Get_Display_flag() == ENABLE)
					ipu->Ipu_reg_F0 &= ~BIT7;//open ipw2 (panel)
				else					
					ipu->Ipu_reg_F0 |= BIT7;/*Close IPW2 write path*/

				ipu->Ipu_reg_F0 &= ~BIT6;
			}
		#else
			ipu->Ipu_reg_F0 &= ~BIT6;//open ipw1 no panel
		#endif

#if ONLY_USE_IPW2
			ipu->Ipu_reg_F0 |= BIT6;/*Close IPW1 write path*/						  
#endif	
}



int CaptureCloseDisplay_Flag = 0;
/*
CaptureCloseDisplay_Flag = 0 
  Nornal mode.

CaptureCloseDisplay_Flag = 1
  Capture close the path to display and Skip SCL.
  Because the sensor input width > 1600.
*/

static void Local_HW_Path_Set(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *win = Idu_GetNextWin();
	/************************
	ipw1 set
	***************************/

	// Fill Default Main scaler factor
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE) 
	/*1:1*/
	//ipu->Ipu_reg_22= 0x80000000;
	//ipu->Ipu_reg_23= 0x80000000;

        HW_SensorMainScale_Set();
	
#else
	ipu->Ipu_reg_22 = 0x80000000 ;
        ipu->Ipu_reg_23 = 0x40000000 ;
#endif


#if debug_IPW1_flag
	#if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
		ipu->Ipu_reg_12 = 0x01000000  | BIT16;
	#else
	ipu->Ipu_reg_12 = 0x01000000 ;
	#endif 
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	//ipu->Ipu_reg_15 = 0x00000257 ;
	ipu->Ipu_reg_15 = SensorInputHeight-1 ;
	ipu->Ipu_reg_16 = SensorInputWidth-1 ;


	BYTE *tBuf = (BYTE*)ext_mem_malloc(SensorInputWidth * SensorInputHeight * 2 + 4096);
	if(tBuf == NULL){
	mpDebugPrint("record memory alloc fail...");
	return;
	}
	// IPW DMA
	ipu->Ipu_reg_1B = 0x00000000 ;
	ipu->Ipu_reg_A3	= ((DWORD) tBuf| 0xA0000000);

#endif


#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
		  ipu->Ipu_reg_28 = 0x01000000 ;

          #if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
            ipu->Ipu_reg_29=(win->wHeight / VAL_3) - 1;	
            ipu->Ipu_reg_2A=(((win->wWidth/VAL_1) * VAL_2)) - 1;
	  #else
	if(SensorWindow_setFlag == 1)
	{
	    ipu->Ipu_reg_29=(SensorWindow_Height) - 1;
	    ipu->Ipu_reg_2A=(SensorWindow_Width) - 1;	 
	}
	else
	{
	    ipu->Ipu_reg_29=(win->wHeight) - 1;
	    ipu->Ipu_reg_2A=(win->wWidth) - 1;	  
	}
	  #endif
#endif



	// Scaling path
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)

  #if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
    ipu->Ipu_reg_18 = 0x00010707 ;
    ipu->Ipu_reg_19 = 0x07010000 ;//0x03030000
    ipu->Ipu_reg_20 |= BIT1; //use SCL must set
  
    //Set CSC0
	ipu->Ipu_reg_40 = 0x01000000;// UV add 128 after 3x3 matrix mult
	ipu->Ipu_reg_41 = 0x01000000;// Y = 1 , Cb = 0 
	ipu->Ipu_reg_42 = 0x015E0000;// Cr = 1.371 = 0x15E , Offset = 0	  
	ipu->Ipu_reg_43 = 0x010007AA;// Y = 1 , Cb =	-0x56 => 2`=> 0x7AA
	ipu->Ipu_reg_44 = 0x074E0000;// Cr = -0xB2 => 2`=> 0x74E, Offset = 0
	ipu->Ipu_reg_45 = 0x010001BB;// Y = 1 , Cb = 1.732 = 0x1BB  
	ipu->Ipu_reg_46 = 0x00000000;// Cr = 0, Offset = 0
	  
	//Set CSC1
	ipu->Ipu_reg_47 = 0x00010000;// UV minus 128 after 3x3 matrix mult
	ipu->Ipu_reg_48 = 0x004C0096;//R = 4D ; G = 95
	ipu->Ipu_reg_49 = 0x001D0000;//B = 1D ; OFF = 0	  
	ipu->Ipu_reg_4A = 0x07D407AA;//R = -2C = >2`=>0x7D4 ; G = 0x7AA
	ipu->Ipu_reg_4B = 0x00820000;//B = 82 ; OFF = 7F
	ipu->Ipu_reg_4C = 0x00820793;//R = 82 ; G = -6D => 2`=> 0x793 
	ipu->Ipu_reg_4D = 0x07EB0000;//B = -15 = >2`=>0x7EB ; OFF = 7F
	  
	//--- UVmax 
	ipu->Ipu_reg_B0 = 0x00000000;
	ipu->Ipu_reg_B1 = 0x2a57e24d;
	ipu->Ipu_reg_B2 = 0xd6a91eb3;
	ipu->Ipu_reg_B3 = 0x05d9011f;
	ipu->Ipu_reg_B4 = 0x02ca016b;
	ipu->Ipu_reg_B5 = 0x01200895;
	ipu->Ipu_reg_B6 = 0x0358016b;
	ipu->Ipu_reg_B7 = 0x012005b6;
	ipu->Ipu_reg_B8 = 0x016d02c2;
	ipu->Ipu_reg_B9 = 0x08c50120;
	ipu->Ipu_reg_BA = 0x016d034c;
  #else
    /*SENSOR_TO_FB_RGB888_TYPE != ENABLE*/
	if(sensor_mode != MODE_CAPTURE)
	   {
#if 0	   	
		   ipu->Ipu_reg_18 = 0x00070707 ;  //disable SCL  sensor->ipw1
		   ipu->Ipu_reg_19 = 0x07020000 ;

		//   ipu->Ipu_reg_20 |= BIT1; //use SCL must set

		//   ipu->Ipu_reg_27 |= BIT24;;
#else
ipu->Ipu_reg_18 = 0x00010301 ;
    ipu->Ipu_reg_19 = 0x03030000 ;//0x03030000
    ipu->Ipu_reg_20 |= BIT0; 

	 //frank debug test 20120919
    	ipu->Ipu_reg_26 |=  BIT0; 
		ipu->Ipu_reg_27 |= BIT24;

    //Set CSC0
	ipu->Ipu_reg_40 = 0x01000000;// UV add 128 after 3x3 matrix mult
	ipu->Ipu_reg_41 = 0x01000000;// Y = 1 , Cb = 0 
	ipu->Ipu_reg_42 = 0x015E0000;// Cr = 1.371 = 0x15E , Offset = 0	  
	ipu->Ipu_reg_43 = 0x010007AA;// Y = 1 , Cb =	-0x56 => 2`=> 0x7AA
	ipu->Ipu_reg_44 = 0x074E0000;// Cr = -0xB2 => 2`=> 0x74E, Offset = 0
	ipu->Ipu_reg_45 = 0x010001BB;// Y = 1 , Cb = 1.732 = 0x1BB  
	ipu->Ipu_reg_46 = 0x00000000;// Cr = 0, Offset = 0
	  
	//Set CSC1
	ipu->Ipu_reg_47 = 0x00010000;// UV minus 128 after 3x3 matrix mult
	ipu->Ipu_reg_48 = 0x004C0096;//R = 4D ; G = 95
	ipu->Ipu_reg_49 = 0x001D0000;//B = 1D ; OFF = 0	  
	ipu->Ipu_reg_4A = 0x07D407AA;//R = -2C = >2`=>0x7D4 ; G = 0x7AA
	ipu->Ipu_reg_4B = 0x00820000;//B = 82 ; OFF = 7F
	ipu->Ipu_reg_4C = 0x00820793;//R = 82 ; G = -6D => 2`=> 0x793 
	ipu->Ipu_reg_4D = 0x07EB0000;//B = -15 = >2`=>0x7EB ; OFF = 7F
	  
	//--- UVmax 
	ipu->Ipu_reg_B0 = 0x00000000;
	ipu->Ipu_reg_B1 = 0x2a57e24d;
	ipu->Ipu_reg_B2 = 0xd6a91eb3;
	ipu->Ipu_reg_B3 = 0x05d9011f;
	ipu->Ipu_reg_B4 = 0x02ca016b;
	ipu->Ipu_reg_B5 = 0x01200895;
	ipu->Ipu_reg_B6 = 0x0358016b;
	ipu->Ipu_reg_B7 = 0x012005b6;
	ipu->Ipu_reg_B8 = 0x016d02c2;
	ipu->Ipu_reg_B9 = 0x08c50120;
	ipu->Ipu_reg_BA = 0x016d034c;

#endif

	   }
	   else
	   {
       		if(CaptureCloseDisplay_Flag == 0) 
       		{
				//display path for preview mode
	   			ipu->Ipu_reg_18 = 0x00070707 ;  
	   			ipu->Ipu_reg_19 = 0x07020000 ;
	   			//ipu->Ipu_reg_20 |= BIT1; //use SCL must set
       		}
			else
			{
				//close display path for preview mode
				ipu->Ipu_reg_18 = 0x07070707 ;	//disable SCL  sensor->ipw1
				ipu->Ipu_reg_19 = 0x07000000 ;
			}
	   }
  #endif

#else //-------------------
    /*USE_IPW1_DISPLAY_MOTHOD != ENABLE*/
	if(sensor_mode != MODE_CAPTURE)
	   {
		   ipu->Ipu_reg_18 = 0x00070707 ;  //disable SCL  sensor->ipw1
		   ipu->Ipu_reg_19 = 0x07020000 ;
		   ipu->Ipu_reg_20 |= BIT1; //use SCL must set
	   } 
	   else
	   {
       		if(CaptureCloseDisplay_Flag == 0) 
       		{
				//display path for preview mode
	   			ipu->Ipu_reg_18 = 0x00070707 ;  
	   			ipu->Ipu_reg_19 = 0x07020000 ;
	   			ipu->Ipu_reg_20 |= BIT1; //use SCL must set
       		}
			else
			{
				//close display path for preview mode
				ipu->Ipu_reg_18 = 0x07070707 ;	//disable SCL  sensor->ipw1
				ipu->Ipu_reg_19 = 0x07000000 ;
			}
	   }
#endif
	   
	/************************
	ipw2 set

	***************************/

	// New Path
	//sensor->sscl->ipw2->panel method 1
	//ipu->Ipu_reg_F0 = 0x2000004b ;
	//ipu->Ipu_reg_F0 = 0x2000000b ;
	//ipu->Ipu_reg_F0 = 0x00000004 ;
	//sensor->sscl->ipw2->panel method 2
	//ipu->Ipu_reg_F0 = 0x20000044 ;
	//mpDebugPrint("-------------Ipu_reg_1C=0x%X",ipu->Ipu_reg_1C);


 
	DWORD Ssclfrom=2; //bit0~2 add SCL
	DWORD Iprfrom=BIT3;//BIT3; //BIT3
	DWORD Overlay=0;//bit4~5
 
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
  Ssclfrom=0;//BIT1 | BIT0; //bit0~2 add SCL
  Iprfrom=BIT3;//BIT3; //BIT3 
  Overlay=0;//bit4~5
#else
  Ssclfrom=2; //bit0~2 add SCL
  Iprfrom=BIT3;//BIT3; //BIT3
  Overlay=0;//bit4~5
#endif


	if(Get_Display_flag() == ENABLE)
	{
		ipu->Ipu_reg_F0 = 0x20000000|Overlay|Iprfrom|Ssclfrom;
	}
	else
	{	
		ipu->Ipu_reg_F0 = 0x20000000|Overlay|Iprfrom|Ssclfrom;
		ipu->Ipu_reg_F0 |= BIT29| BIT7 |Overlay|Iprfrom|Ssclfrom;
	}

	//ipu->Ipu_reg_F0 |=BIT7;//stop ipw2
	// New IPW DMA

	//ipu->Ipu_reg_F1 = 0x00000000 ;

	#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))
	DWORD DisplayWinStartAddr;
	WORD PanelW,PanelH;
	ST_IMGWIN *win_sensor;
	extern WORD SensorWindow_PosX;
	extern WORD SensorWindow_PosY;
	extern BYTE SensorWindow_setFlag;
	extern	int Need_changWin;

	#if VIDEO_ON_VIDEO || !ONLY_USE_IPW2
	win_sensor = GetSensorBufferWin();
	#else
	win_sensor = Idu_GetCurrWin();
	#endif
 	PanelW = win_sensor->wWidth;
 	PanelH = win_sensor->wHeight;

	if(SensorWindow_setFlag == ENABLE)
	{
		DisplayWinStartAddr = (PanelW * SensorWindow_PosY * 2) + (SensorWindow_PosX * 2);
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
        ipu->Ipu_reg_A3 = ((DWORD) win_sensor->pdwStart| 0xA0000000)+DisplayWinStartAddr;	
#else
		ipu->Ipu_reg_F2 = ((DWORD) win_sensor->pdwStart| 0xA0000000)+DisplayWinStartAddr;	
#endif
	}
	else
	{
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
	    ipu->Ipu_reg_A3 = ((DWORD) win_sensor->pdwStart| 0xA0000000);	
#else
		ipu->Ipu_reg_F2 = ((DWORD) win_sensor->pdwStart| 0xA0000000);	
#endif
	}
	#endif

	Global_HW_Path_Set();
}



#else
//scale->ipw1,ipw2  use 650 color progressing
static void Local_HW_Path_Set(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *win = Idu_GetNextWin();
	/************************
	ipw1 set

	***************************/

	ipu->Ipu_reg_12 = 0x01000000 ;

	//overlay window
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	//ipu->Ipu_reg_15 = 0x00000257 ;
	ipu->Ipu_reg_15 = 720-1 ;
	ipu->Ipu_reg_16 = 1280-1 ;
	//ipu->Ipu_reg_15 = 480-1 ;
	//ipu->Ipu_reg_16 = 800-1 ;


	//scl window
	ipu->Ipu_reg_28 = 0x01000000 ;

	ipu->Ipu_reg_29 = 720-1 ;
	ipu->Ipu_reg_2A = 1280-1 ;


	BYTE *tBuf = (BYTE*)ext_mem_malloc(1280 * 720 * 2 + 4096);
	if(tBuf == NULL){
		mpDebugPrint("record memory alloc fail...");
		return;
	}
	// IPW DMA
	ipu->Ipu_reg_1B = 0x00000000 ;
	ipu->Ipu_reg_A3	= ((DWORD) tBuf| 0xA0000000);
	//ipu->Ipu_reg_A3	= ((DWORD) win->pdwStart| 0xA0000000);


	ipu->Ipu_reg_21 = 0 ;

	ipu->Ipu_reg_22 = 0x80000000;//scale=1
	ipu->Ipu_reg_23 = 0x80000000;

	mpDebugPrint("***********************************************++");

	// Scaling path
#if 1
	//ipu->Ipu_reg_18 = 0x00070207 ;
	//ipu->Ipu_reg_19 = 0x07030000 ;

	ipu->Ipu_reg_18 = 0x00070707 ;
	ipu->Ipu_reg_19 = 0x07020000 ;

#else
	ipu->Ipu_reg_18 = 0x07070707 ;	//disable SCL  sensor->ipw1
	ipu->Ipu_reg_19 = 0x07000000 ;
#endif

	DWORD Ssclfrom=2; //bit0~2
	DWORD Iprfrom=BIT3;//BIT3; //BIT3
	DWORD Overlay=0;//bit4~5

	ipu->Ipu_reg_F0 = 0x20000000|Overlay|Iprfrom|Ssclfrom;
	//ipu->Ipu_reg_F0 |=BIT7;//ipw2 stop

	// New IPW DMA
	ipu->Ipu_reg_F1 = 0x00000000 ;
	ipu->Ipu_reg_F2 = ((DWORD) win->pdwStart| 0xA0000000);
	//ipu->Ipu_reg_F2 = ((DWORD) temp_win.pdwStart| 0xA0000000);
}

#endif

static void Local_HW_SensorInterFace_Set(void)
{

}

static void Local_HW_SensorInerface_Stop(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu->Ipu_reg_100 &= ~BIT0; //SenInEn
}

static void Local_Sensor_GPIO_Reset(void)
{
	register GPIO *gpio = (GPIO *) (GPIO_BASE);
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	//set gpio pin sclk sdata  //use i2c must set
	//gpio->Gpdat0 |=0x18000000;
	//clock->Clkss2 |=BIT11;//disable mclk
	//clock->Clkss2 &=~BIT11;//enable mclk
  Local_HW_GPIO_Pin_reset_1();
  Local_HW_GPIO_Pin_ssen_0();
	IODelay(100);


}

static BYTE g_setting = 0;
void InitTvp5150()
{
    if(g_setting == 0)
    {
		Local_HW_MCLK_Set();
#if 0
		Local_HW_GPIO_Pin_Set();
		Local_Sensor_GPIO_Reset();
#else
		Local_HW_GPIO_Pin_ssen_1();
		IODelay(20);
		Local_HW_GPIO_Pin_reset_1();
		IODelay(20);
		//GetGPIOValue(SENSOR_GPIO_RESET);
		Local_HW_GPIO_Pin_reset_0();
		IODelay(20);
		Local_HW_GPIO_Pin_reset_1();
		IODelay(20);
		Local_HW_GPIO_Pin_ssen_0();
		IODelay(100);
#endif
	  TVP5150_setting();
	  g_setting = 1;
    }
}

static void Local_HW_Set(void)
{

	CHANNEL *dmaR, *dmaW;
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	IPU *ipu = (IPU *) IPU_BASE;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
	int ret = FAIL, t = 0;


	//******************
#if (USE_SYSTEM_I2C_FUNCTION==0)
	Local_HW_I2C_CLK_Set();

    BIU *regBiuPtr = (BIU *) BIU_BASE;
    regBiuPtr->BiuArst |= ARST_I2CM;
#endif

   InitTvp5150();
	//mpDebugPrint("Set_SenIpr_Control");
	//SenPixMode
	ipu->Ipu_reg_100 &= ~(BIT16|BIT17|BIT18|BIT19);

//if(sensor_Input_Format==Format_YYUV)
//	ipu->Ipu_reg_100 |= BIT18|BIT17;
//else
//	ipu->Ipu_reg_100 |= BIT18;

//frank set YUV420
    ipu->Ipu_reg_100 |= BIT18 | BIT16;


	//SenDimMeaEn
	ipu->Ipu_reg_100 |= 0x00600000;

	ipu->Ipu_reg_100 &= ~0x00400000;	//PixGenEn virtual sensor disable
#if VirtualSensorEnable
	//set virtual sensor
		ipu->Ipu_reg_103=(SensorInputHeight<<16)+(SensorInputWidth);
	//mpDebugPrint("fffffffffffffffffffffff=0x%X",(600<<16)+(800));
	ipu->Ipu_reg_100 |= BIT22; //PixGenEn, virtual sensor Enable
#endif

	ipu->Ipu_reg_100 |= BIT21;//SenDimMeaEn

	//Hsync, Vsync pos

	ipu->Ipu_reg_100 |= BIT8;//Vsync pos=1
	//ipu->Ipu_reg_100 &= ~BIT8;//Vsync pos=0

	//ipu->Ipu_reg_100 |= BIT9;//Hsync pos=1
	ipu->Ipu_reg_100 &= ~BIT9;//Hsync pos=0

#if SENSOR_INPUT_DATA==CCIR656
	ipu->Ipu_reg_100 |= BIT11;// sysnc from data CCIR656 
#else
	ipu->Ipu_reg_100 &= ~BIT11;	
#endif

	clock->MdClken |= BIT10;//enable mclk
	//**********************
	clock->Clkss2 &=~(BIT15|BIT16|BIT17|BIT18);//bit 15-18 sensor in clock select
#if VirtualSensorEnable
	clock->Clkss2 |=BIT17|BIT18;
#endif
	clock->MdClken |= 0x00000c00 ;	//enable clock to sensor and clock for sensor_in

	Local_HW_InputWindow1_Set();

	
	//delay Vsync for TVP5150 
#if MINIDV_YBD_FUNCION

#if THREE_WINDOW_MODE

if (VideoFormat_Is_Ntsc()) // NTSC
	ipu->Ipu_reg_101 = 0x00250020; //V H
else
	ipu->Ipu_reg_101 = 0x00150020;

#else

#if (PRODUCT_PANEL==PANEL_800x600)
	BYTE bVideoFmt=Get_TVP5150_Register(0x8c);
	//mpDebugPrint("bVideoFmt=%p",bVideoFmt);
	bVideoFmt &=0x0e;
if (bVideoFmt==BIT1 || bVideoFmt==BIT2) //PAL
	ipu->Ipu_reg_101 = 0x00250000;
else
#endif
	ipu->Ipu_reg_101 = 0x00150020;

#endif

#else
	ipu->Ipu_reg_101 = 0x00100000;
#endif

	ipu->Ipu_reg_108 = 0x00000000 ;

	Local_HW_InputWindow2_Set();

	Local_HW_SensorScale_Set();

	Local_HW_Path_Set();

#if 0
	ipu->Ipu_reg_3E = 0x00010100; //SP->CS
	ipu->Ipu_reg_3C=0;//0xC0C00000;
#endif

	clock->MdClken |= BIT5;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;

	SystemIntEna(IM_IPU);
	isr->MiMask |= 0x00000002 ;
	//Enable_IPW2_FrameEnd_Int();
	//ipu->Ipu_reg_1C |= 0x00004000 ;	//ipw2 interrrupt
	ipu->Ipu_reg_1C |= BIT15|BIT14;//ipw1 ipw2 interrrupt enable
	//Enable_IPW1_FrameEnd_Int();
	//ipu->Ipu_reg_1C |= 0x00008000 ;	//Bit 14 for IPW2 frame end, Bit 24 for IPW1 frame end
	MP_DEBUG("#### Trigger ####");
	//mpDebugPrint("####============= Trigger =================####");

  //Main_Scaler_from_Sensor_Enable(&temp_win);
  //Main_Scaler_from_Sensor_Enable(trgWin);

	//ipu->Ipu_reg_100 |= 0x00000001 ; //SenInEn
	/*
		mpDebugPrint("ipu->Ipu_reg_1C=0x%08x", ipu->Ipu_reg_1C);
		mpDebugPrint("ipu->Ipu_reg_F0=0x%08x", ipu->Ipu_reg_F0);

		mpDebugPrint("ipu->Ipu_reg_100=0x%08x, addr=0x%08x", ipu->Ipu_reg_100, &ipu->Ipu_reg_100);
		mpDebugPrint("ipu->Ipu_reg_103=0x%08x, addr=0x%08x", ipu->Ipu_reg_103, &ipu->Ipu_reg_103);

		mpDebugPrint("clock->Clkss2=0x%08x", clock->Clkss2);

		mpDebugPrint("clock->MdClken=0x%08x", clock->MdClken);
	*/
DMA *dma = (DMA *)DMA_BASE;
/*
//dma->FDMACTL_EXT1 &= ~BIT24;
dma->FDMACTL_EXT1 &= ~(BIT16|BIT17|BIT18|BIT19|BIT20);
dma->FDMACTL_EXT1 |= BIT24;
dma->FDMACTL_EXT1 |= BIT17;
*/

//dma->FDMACTL_EXT0 &= ~(0xFF);
//dma->FDMACTL_EXT0 |= 0x02;
}




static void Local_HW_Set_capture(void)
{

	CHANNEL *dmaR, *dmaW;
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;
	IPU *ipu = (IPU *) IPU_BASE;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
	int ret = FAIL, t = 0;


	//******************
#if (USE_SYSTEM_I2C_FUNCTION==0)
	Local_HW_I2C_CLK_Set();

    BIU *regBiuPtr = (BIU *) BIU_BASE;
    regBiuPtr->BiuArst |= ARST_I2CM;
#endif


	//Local_HW_MCLK_Set();        --Frank Lin del
	//Local_HW_GPIO_Pin_Set();   --Frank Lin del
	//Local_Sensor_GPIO_Reset(); --Frank Lin del

	ipu->Ipu_reg_100 &= ~(BIT16|BIT17|BIT18|BIT19);

if(sensor_Input_Format==Format_YYUV)
	ipu->Ipu_reg_100 |= BIT18|BIT17;
else
	ipu->Ipu_reg_100 |= BIT18;

	//SenDimMeaEn
	ipu->Ipu_reg_100 |= 0x00600000;

	ipu->Ipu_reg_100 &= ~0x00400000;	//PixGenEn virtual sensor disable
#if VirtualSensorEnable
	//set virtual sensor
		ipu->Ipu_reg_103=(SensorInputHeight<<16)+(SensorInputWidth);
	//mpDebugPrint("fffffffffffffffffffffff=0x%X",(600<<16)+(800));
	ipu->Ipu_reg_100 |= BIT22; //PixGenEn, virtual sensor Enable
#endif

	ipu->Ipu_reg_100 |= BIT21;//SenDimMeaEn

	//Hsync, Vsync pos

	ipu->Ipu_reg_100 |= BIT8;//Vsync pos=1
	//ipu->Ipu_reg_100 &= ~BIT8;//Vsync pos=0

	//ipu->Ipu_reg_100 |= BIT9;//Hsync pos=1
	ipu->Ipu_reg_100 &= ~BIT9;//Hsync pos=0

	clock->MdClken |= BIT10;//enable mclk
	//**********************
	clock->Clkss2 &=~(BIT15|BIT16|BIT17|BIT18);//bit 15-18 sensor in clock select
#if VirtualSensorEnable
	clock->Clkss2 |=BIT17|BIT18;
#endif
	clock->MdClken |= 0x00000c00 ;	//enable clock to sensor and clock for sensor_in

	Local_HW_InputWindow1_Set();

	ipu->Ipu_reg_101=0;

	ipu->Ipu_reg_108 = 0x00070007 ;

	Local_HW_InputWindow2_Set();

	Local_HW_SensorScale_Set();

	Local_HW_Path_Set();

#if 0
	ipu->Ipu_reg_3E = 0x00010100; //SP->CS
	ipu->Ipu_reg_3C=0;//0xC0C00000;
#endif

	clock->MdClken |= BIT5;
	dmaR->Control = 0x00000001;
	dmaW->Control = 0x00000001;

	SystemIntEna(IM_IPU);
	isr->MiMask |= 0x00000002 ;
	ipu->Ipu_reg_1C |= BIT15|BIT14;//ipw1 ipw2 interrrupt enable
	MP_DEBUG("#### Trigger ####");

}


static void MainCropScaleSet(WORD TrgW, WORD TrgH, WORD SrcW, WORD SrcH)
{
	IPU *ipu = (IPU *) IPU_BASE;
	SDWORD width_ratio  = 0;
	SDWORD height_ratio = 0;
	SDWORD  vsub_ratio = 0,hsub_ratio = 0;

	//Width ratio
	width_ratio  =(SrcW<<15) / TrgW;
	width_ratio -= 1;
	hsub_ratio = hsub_ratio -1;
    //width_ratio -=((Scale_ratio_adjust>>1)*3);

	if(hsub_ratio > 7)
      hsub_ratio = 7;
    if(hsub_ratio <= 0)
      hsub_ratio = 0;

    if(width_ratio >= 0x8000)
      width_ratio = 0x8000;
	
    if(width_ratio <= 0)
      width_ratio = 0x0001;

    //Height
     height_ratio =(SrcH<<15)/ TrgH;
     //height_ratio = height_ratio>> 1;
	 height_ratio -= 1;
	// height_ratio -= Scale_ratio_adjust;

     vsub_ratio = vsub_ratio -1;

    if(vsub_ratio > 7)
      vsub_ratio = 7;

    if(vsub_ratio <= 0)
      vsub_ratio = 0;

    if(height_ratio >= 0x8000)
      height_ratio = 0x8000;
	
    if(height_ratio <= 0)
      height_ratio = 0x0001;

    MP_ALERT("#### width_ratio =%d, SensorW =%d,PanelW=%d",width_ratio, SrcW, TrgW);
    MP_ALERT("#### height_ratio=%d, SensorH=%d,PanelH=%d",height_ratio, SrcH,TrgH);
    MP_ALERT("%s:scale ratio width_ratio=0x%x, height_ratio=0x%x", __FUNCTION__, width_ratio, height_ratio);

    ipu->Ipu_reg_21 = (vsub_ratio<<8) | hsub_ratio;
    ipu->Ipu_reg_22 = width_ratio	<< 16;
    ipu->Ipu_reg_23 = height_ratio << 16;

	
}

static CVBS_Zoom_Set(void)
{		
	IPU *ipu = (IPU *) IPU_BASE;
	SWORD ZoomXpos,ZoomYpos;
	DWORD DefaultZoomXpos,DefaultZoomYpos;
		
	ipu->Ipu_reg_F0 |= BIT2 | BIT1 | BIT0;
	MP_DEBUG2("%s: ipu->Ipu_reg_18 =0x%x", __FUNCTION__, ipu->Ipu_reg_18);
	MP_DEBUG2("%s: ipu->Ipu_reg_19 =0x%x", __FUNCTION__, ipu->Ipu_reg_19);
	MP_DEBUG2("%s: ipu->Ipu_reg_F0 =0x%x", __FUNCTION__, ipu->Ipu_reg_F0);
		
	switch(g_CVBS_ZoomStep)
	{
	  case SENSOR_ZOOM_LEVEL_1_33:/*1.33x*/
	    /*540x180 to 720x480*/
	  
	    if(API_Get_Selected_Channel() == 0)
	    {/*CVBS sensor*/
	     /*xpos = 0~(80+80)*/
	     /*ypos = (20) ~(20+52+52)*/
	      DefaultZoomXpos =  80;
		  DefaultZoomYpos = 20 + 52;
	    }
	    else 
	    { /*dvd player*/
	      /*xpos = 0~(74+74)*/
	      /*ypos = (12) ~(12+30+30)*/
	      DefaultZoomXpos = 74;
	      DefaultZoomYpos = 12 + 30;
	    }
					
	    MainCropScaleSet( API_Get_SensorWindow_Width(), API_Get_SensorWindow_Height(), SensorInputWidth, SensorInputHeight);//120);
	
	    ZoomXpos = API_GetZoomXpos();
	    ZoomYpos = API_GetZoomYpos();
	  
	    ipu->Ipu_reg_101 = (DefaultZoomYpos+ZoomYpos)<<16 |((DefaultZoomXpos+ZoomXpos)<<1);
	    ipu->Ipu_reg_20 |= BIT0;
	    ipu->Ipu_reg_20 &= ~BIT1;
	    ipu->Ipu_reg_20 |= BIT25;
	
	    ipu->Ipu_reg_102 = (((SensorInputHeight<<1)-1)<<16)|(SensorInputWidth-1) ;//franklin
				
	    break;
	
	  case SENSOR_ZOOM_LEVEL_1_66:/*1.66x*/
	    /*432x144 to 720x480*/
		
	    if(API_Get_Selected_Channel() == 0)
	    {/*CVBS sensor*/
	     /*xpos = 0~(132+132)*/
	     /*ypos = (20) ~(20+70+70)*/
	      DefaultZoomXpos =  132;
	      DefaultZoomYpos = 20 + 70;		
		  ipu->Ipu_reg_102 = (((SensorInputHeight<<2)-1)<<16)|(SensorInputWidth-1) ;//franklin
	    }
	    else 
	    { /*dvd player*/
	      /*xpos = 0 ~ (128+128) 258*/
		  /*ypos = (12) ~ (12+48+48)*/
		  DefaultZoomXpos = 128;
		  DefaultZoomYpos = 12 + 48;
		  ipu->Ipu_reg_102 = (((SensorInputHeight<<1)-1)<<16)|(SensorInputWidth-1) ;//franklin
	    }
							
	    MainCropScaleSet( API_Get_SensorWindow_Width(), API_Get_SensorWindow_Height(), SensorInputWidth, SensorInputHeight);//120);
			
	    ZoomXpos = API_GetZoomXpos();
	    ZoomYpos = API_GetZoomYpos();
			
	    ipu->Ipu_reg_101 = (DefaultZoomYpos+ZoomYpos)<<16 |((DefaultZoomXpos+ZoomXpos)<<1);
	    ipu->Ipu_reg_20 |= BIT0;
	    ipu->Ipu_reg_20 &= ~BIT1;
	    ipu->Ipu_reg_20 |= BIT25;
						
	    break;
	
			  
	  case SENSOR_ZOOM_LEVEL_2_00:/*2X*/
	    /*
		  360x120 to 720x480 :
		  initial xpos = 180, ypos = 60, 
	        */
				
	    if(API_Get_Selected_Channel() == 0)
	    {/*CVBS sensor*/
	     /*valid width =338+360*/
	      DefaultZoomXpos =  168;//(338/2)
	      DefaultZoomYpos = 20 + 82;//20+(164/2);
		  ipu->Ipu_reg_102 = (((SensorInputHeight<<2)-1)<<16)|(SensorInputWidth-1) ;
	    }
	    else 
	    { /*dvd player*/
	      DefaultZoomXpos = 164;
	      DefaultZoomYpos = 12 + 60;		  
		  ipu->Ipu_reg_102 = (((SensorInputHeight<<2)-1)<<16)|(SensorInputWidth-1) ;
	    }

	    MainCropScaleSet( API_Get_SensorWindow_Width(), API_Get_SensorWindow_Height(), SensorInputWidth, SensorInputHeight);//120);
	
	    ZoomXpos = API_GetZoomXpos();
	    ZoomYpos = API_GetZoomYpos();
	
	    ipu->Ipu_reg_101 = (DefaultZoomYpos+ZoomYpos)<<16 |((DefaultZoomXpos+ZoomXpos)<<1);
	    ipu->Ipu_reg_20 |= BIT0;
	    ipu->Ipu_reg_20 &= ~BIT1;
	    ipu->Ipu_reg_20 |= BIT25;
	
	    break;
		
	  default:
	  case SENSOR_ZOOM_LEVEL_1_00:
/**not*/
        if(API_Get_Selected_Channel() == 0)
	    {/*CVBS sensor*/

	      DefaultZoomXpos =  0;
	      DefaultZoomYpos = 20;
		  ipu->Ipu_reg_102 = (((SensorInputHeight<<2)-1)<<16)|((SensorInputWidth-24)-1) ;

		  MainCropScaleSet( API_Get_SensorWindow_Width(), API_Get_SensorWindow_Height(), (SensorInputWidth-24), SensorInputHeight);//120);		  
	    }
	    else 
	    { /*dvd player*/
	      DefaultZoomXpos = 0;
	      DefaultZoomYpos = 12;		  
		  ipu->Ipu_reg_102 = (((SensorInputHeight<<2)-1)<<16)|((SensorInputWidth-28)-1) ;
		  
		  MainCropScaleSet( API_Get_SensorWindow_Width(), API_Get_SensorWindow_Height(), (SensorInputWidth-28), SensorInputHeight);//120);
	    }
		
	
	    ZoomXpos = API_GetZoomXpos();
	    ZoomYpos = API_GetZoomYpos();
	
	    ipu->Ipu_reg_101 = (DefaultZoomYpos+ZoomYpos)<<16 |((DefaultZoomXpos+ZoomXpos)<<1);
	    ipu->Ipu_reg_20 |= BIT0;
	    ipu->Ipu_reg_20 &= ~BIT1;
	    ipu->Ipu_reg_20 |= BIT25;
	    
	}
	
	ipu->Ipu_reg_F0 |= BIT25;
	
}


E_SENSOR_ZOOM_LEVEL Get_sensorZoomStep(void)
{
	return g_CVBS_ZoomStep;
}

//static 
int Global_Sensor_Run(void)
{
	IPU *ipu = (IPU *) IPU_BASE;

	BYTE *SensorInBuffer;
	SWORD ZoomXpos,ZoomYpos;
	DWORD DefaultZoomXpos,DefaultZoomYpos;
    extern void SetHighHPorch(void);
#if !MINIDV_YBD_FUNCION
	TVP5150_SetChannel();
#endif


#if 1
    if(SensorInWin[0].pdwStart != NULL)
	{
		ext_mem_free(SensorInWin[0].pdwStart);
		SensorInWin[0].pdwStart = NULL;
	}

	if(MultiBufFlag == ENABLE)
	{
		MP_ALERT("####----- alloct 2 buffer");
		SensorInBuffer = (BYTE*)ext_mem_malloc((CVBS_getRecordWidth() * CVBS_getRecordHeight() * 2 + 4096) * 3);
	}else{
		SensorInBuffer = (BYTE*)ext_mem_malloc(CVBS_getRecordWidth() * CVBS_getRecordHeight() * 2 + 4096);
	}

#else
	if(sensor_mode==MODE_CAPTURE)
	{
		if(g_bImageData_Buf != NULL)
		{
		 	ext_mem_free(g_bImageData_Buf);
			g_bImageData_Buf        = NULL;
			SensorInWin[0].pdwStart = NULL;
			
		}
		else
		{   /*Make sure free memory*/
			if(SensorInWin[0].pdwStart != NULL)
			{
		 		ext_mem_free(SensorInWin[0].pdwStart);
				SensorInWin[0].pdwStart = NULL;
			}
		}
		
		/*Capture mode not need double buffer*/
		//DoubleBuffer = 0;

		g_bImageData_Buf = (BYTE*)ext_mem_malloc((SensorInputWidth * SensorInputHeight * 2 )+(SensorInputWidth *16)+4096);
		SensorInBuffer   = g_bImageData_Buf + (SensorInputWidth *16);
	}
	else
	{
		if(SensorInWin[0].pdwStart != NULL)
		{
		 	ext_mem_free(SensorInWin[0].pdwStart);
			SensorInWin[0].pdwStart = NULL;
		}

		if(MultiBufFlag == ENABLE)
		{
			MP_ALERT("####----- alloct 2 buffer");
			SensorInBuffer = (BYTE*)ext_mem_malloc((CVBS_getRecordWidth() * CVBS_getRecordHeight() * 2 + 4096) * 3);
		}else{
			SensorInBuffer = (BYTE*)ext_mem_malloc(CVBS_getRecordWidth() * CVBS_getRecordHeight() * 2 + 4096);
		}
	}
#endif
	memset(&SensorInWin[0], 0, sizeof(ST_IMGWIN));
	SensorInWin[0].pdwStart = (DWORD *)((DWORD)SensorInBuffer | 0xa0000000);
	SensorInWin[0].wWidth   = CVBS_getRecordWidth();
	SensorInWin[0].wHeight  = CVBS_getRecordHeight();
	SensorInWin[0].dwOffset = (SensorInWin[0].wWidth << 1);

	MP_ALERT("Global_Sensor_Run MultiBufFlag =%d W=%d  H=%d", MultiBufFlag,CVBS_getRecordWidth(),CVBS_getRecordHeight());  
	if(MultiBufFlag == ENABLE)
	{
		mpDebugPrint("######_DoubleBuffer OK");
		memset(&SensorInWin[1], 0, sizeof(ST_IMGWIN));
		SensorInWin[1].pdwStart = (DWORD *)((DWORD)(SensorInBuffer + (CVBS_getRecordWidth() * CVBS_getRecordHeight() * 2 + 4096)) | 0xa0000000);
		SensorInWin[1].wWidth   = CVBS_getRecordWidth();
		SensorInWin[1].wHeight  = CVBS_getRecordHeight();
		SensorInWin[1].dwOffset = (SensorInWin[1].wWidth << 1);
		memset(&SensorInWin[2], 0, sizeof(ST_IMGWIN));
		SensorInWin[2].pdwStart = (DWORD *)(((DWORD)SensorInBuffer + ((CVBS_getRecordWidth() * CVBS_getRecordHeight() * 2 + 4096)*2)) | 0xa0000000);
		SensorInWin[2].wWidth	= CVBS_getRecordWidth();
		SensorInWin[2].wHeight	= CVBS_getRecordHeight();
		SensorInWin[2].dwOffset = (SensorInWin[1].wWidth << 1);
		mpClearWin(&SensorInWin[1]);
		mpClearWin(&SensorInWin[2]);

	}
	#if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
		ipu->Ipu_reg_12 = 0x01000000  | BIT16;
	#else
	ipu->Ipu_reg_12 = 0x01000000 ;
	#endif
	ipu->Ipu_reg_14 = 0x01000000 ;	//Enable bit 24
	//ipu->Ipu_reg_15 = 0x00000257 ;
#if ((TCON_ID==TCON_NONE) || (SENSOR_WITH_DISPLAY != ENABLE))
	ipu->Ipu_reg_15 = SensorInputHeight-1 ; //must set default
	ipu->Ipu_reg_16 = SensorInputWidth-1 ; //must set default
#else
      #if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
         
	      ST_IMGWIN *win = Idu_GetNextWin();
	      WORD PanelWidth, PanelHeight;
              #if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
                PanelWidth	= win->wWidth / VAL_1 * VAL_2;
	        PanelHeight   = win->wHeight / VAL_3;
		  
	        ipu->Ipu_reg_15 = PanelHeight - 1;
	        ipu->Ipu_reg_16 = PanelWidth  - 1;

	      #else
		    if(SensorWindow_setFlag == ENABLE)
		    {
	          ipu->Ipu_reg_15 = SensorWindow_Height-1;//480-1;//720-1 ;
	          ipu->Ipu_reg_16 = SensorWindow_Width -1;//1280-1 ;
		    }
			else
			{
   	          ipu->Ipu_reg_15 = win->wHeight-1;//480-1;//720-1 ;
	          ipu->Ipu_reg_16 = win->wWidth -1;//1280-1 ;
			}
	      #endif
      #else
	ipu->Ipu_reg_15 = SensorInWin[0].wHeight-1;//720-1 ;
	ipu->Ipu_reg_16 = SensorInWin[0].wWidth-1;//1280-1 ;
#endif
#endif

	// IPW DMA
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)

	if(SensorWindow_setFlag == ENABLE)
    {
	  ipu->Ipu_reg_1B = (win->wWidth - SensorWindow_Width)<<1;
	}
#else
    ipu->Ipu_reg_1B = 0x00000000 ;
#endif
	
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
	ipu->Ipu_reg_F2 = ((DWORD) SensorInWin[0].pdwStart| 0xA0000000);
#else
	ipu->Ipu_reg_A3	= ((DWORD) SensorInWin[0].pdwStart| 0xA0000000);
#endif

	Global_HW_Path_Set();

    //Initial Even frame flag
    CVBS_EvenFramFlag_Init();
    


	   

#if 1 //Debug Zoom Preview  function
  if(Get_CVBS_ZoomStatus() == 1)
  {
    //close ipw2
    ipu->Ipu_reg_F0 |= BIT7;
       
  	CVBS_Zoom_Set();	
    //reduce panel frame rate and low porch
    SetHighHPorch();
	
  }
#endif
	   
	Sensor_Input_Enable();

		Start_Time=GetSysTime();
		frameCNT=0;
#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
       while(0)
#else
		while(1)
#endif
		{
			if(frameCNT>1)
			{
				//mpDebugPrint("=============sensor OK=================");
				return 1;
			}
	
			if((GetSysTime()-Start_Time)>3000)
			{
				mpDebugPrint("=============sensor Fail=================");
				return 0;
			}
	
		}
	
}







//static
int Sensor_Run_capture(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ipu_reset();

    Local_HW_Set_capture();
		
	if(overlay_enable && (Capture_overlay_image_Win.pdwStart != NULL))
    {
		Capture_overlay_660(&Capture_overlay_image_Win, Capture_OverlayKeyY, Capture_OverlayKeyCb, Capture_OverlayKeyCr);
    }

}







//=================== Sensor =============================

#if OVERLAYENABLE
static void Local_Overlay_Initial(void)
{
		int i;
		for(i=0;i<20;i++)
			StrBuf[i]=0;

		if(overlay_enable)
		{
				overlap_win.pdwStart=(DWORD)overlay_img;
				overlap_win.wWidth=Group_Font_Width+Logo_magicpixel_Width;//608;
				overlap_win.wHeight=Group_Font_Height;//58;
				overlap_win.dwOffset=overlap_win.wWidth*2;
				overlap_win.wX=0;
				overlap_win.wY=0;
				overlay_image_Win=overlap_win;
				OverlayKeyY=0x00;
				OverlayKeyCb=0x80;
				OverlayKeyCr=0x80;
		}

}
static void Local_Overlay_Comb_Font(BYTE * BackFontBuf,BYTE * OneFontBuf,int position)
{   //TYChen write
		DWORD i,j;
		DWORD BackFontBufIndex=position*One_Font_BUF_Width;
		DWORD OneFontBufIndex=0;

		for(i=0;i<One_Font_Height;i++)
		{
				for(j=0;j<One_Font_Width;j++)
				{
						if((j&BIT0)==0)//even pixel
						{
						/*
								BackFontBuf[BackFontBufIndex]=OneFontBuf[One_Font_BUF_Width*i+(j<<1)];//y
								BackFontBuf[BackFontBufIndex+1]=OneFontBuf[One_Font_BUF_Width*i+(j<<1)+1];//y
								BackFontBuf[BackFontBufIndex+2]=OneFontBuf[One_Font_BUF_Width*i+(j<<1)+2];//cb
								BackFontBuf[BackFontBufIndex+3]=OneFontBuf[One_Font_BUF_Width*i+(j<<1)+3];//cr
						*/
								BackFontBuf[BackFontBufIndex]=OneFontBuf[OneFontBufIndex++];//y
								BackFontBuf[BackFontBufIndex+1]=OneFontBuf[OneFontBufIndex++];//y
								BackFontBuf[BackFontBufIndex+2]=OneFontBuf[OneFontBufIndex++];//cb
								BackFontBuf[BackFontBufIndex+3]=OneFontBuf[OneFontBufIndex++];//cr

								BackFontBufIndex+=4;
						}

				}
				BackFontBufIndex=BackFontBufIndex+Group_Font_BUF_Width+Logo_magicpixel_BUF_Width-One_Font_BUF_Width;
		}


}

static void Local_Overlay_Comb_Logo(void)
{   //TYChen write
		DWORD i;
		DWORD BufSize=Logo_magicpixel_Width*Logo_magicpixel_Height*2;
		DWORD BackFontBufIndex=Group_Font_BUF_Width;
		DWORD ChangeLineCnt=0;
		for(i=0;i<BufSize;i++)
		{
				overlay_img[BackFontBufIndex++]=Font_logo[i];
				ChangeLineCnt++;
				if(ChangeLineCnt>=Logo_magicpixel_BUF_Width)
				{
						BackFontBufIndex+=Group_Font_BUF_Width;
						ChangeLineCnt=0;
				}
		}


}

static void Local_Overlay_IMG_Update(void)
{		//TYChen write
		int i;
		//__asm("break 100");
		//Comb_Font(overlay_img,Font_symbol_2,0);
		//Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "qqq", "bin", overlay_img, overlay_imgSize);
		//DumpYuvImage2Bmp(608,58,overlay_img);
		ST_SYSTEM_TIME stNewTime;
		SystemTimeGet(&stNewTime);


		for(i=0;i<20;i++)
			StrBuf_Old[i]=StrBuf[i];


    sprintf(StrBuf,"%04d/%02d/%02d %02d:%02d:%02d",stNewTime.u16Year,
		 	stNewTime.u08Month,stNewTime.u08Day,stNewTime.u08Hour,stNewTime.u08Minute,stNewTime.u08Second);

		for(i=0;i<19;i++)
		{

				if(StrBuf[i]==StrBuf_Old[i])
						continue;
		    switch (StrBuf[i])
		    {
		        case '0':
								Local_Overlay_Comb_Font(overlay_img,Font_0,i);
								break;
		        case '1':
								Local_Overlay_Comb_Font(overlay_img,Font_1,i);
								break;
		        case '2':
								Local_Overlay_Comb_Font(overlay_img,Font_2,i);
								break;
		        case '3':
								Local_Overlay_Comb_Font(overlay_img,Font_3,i);
								break;
		        case '4':
								Local_Overlay_Comb_Font(overlay_img,Font_4,i);
								break;
		        case '5':
								Local_Overlay_Comb_Font(overlay_img,Font_5,i);
								break;
		        case '6':
								Local_Overlay_Comb_Font(overlay_img,Font_6,i);
								break;
		        case '7':
								Local_Overlay_Comb_Font(overlay_img,Font_7,i);
								break;
		        case '8':
								Local_Overlay_Comb_Font(overlay_img,Font_8,i);
								break;
		        case '9':
								Local_Overlay_Comb_Font(overlay_img,Font_9,i);
								break;
		        case '/':
								Local_Overlay_Comb_Font(overlay_img,Font_symbol_1,i);
								break;
		        case ':':
								Local_Overlay_Comb_Font(overlay_img,Font_symbol_2,i);
								break;
		        case ' ':
								Local_Overlay_Comb_Font(overlay_img,Font_space,i);
								break;
		        default:
		            break;
		    }
		}
Local_Overlay_Comb_Logo();
//__asm("break 100");
if(overlay_enable)
		Ui_TimerProcAdd(1000, Local_Overlay_IMG_Update);//AddTimerProc(1000, Local_Overlay_IMG_Update);

}
#endif

#if (MOTION_DETECT == ENABLE)
DWORD Motion_Detection_Old_Value=0;
DWORD Diff_Data_Queue[Motion_Detection_Calc_threshold]={0};
DWORD Queue_input_index=0;
//還沒判斷有物體移動時：用來計算經過多少次的frame
//確定有物體移動時：用來計算連續多少frame都是靜止的
DWORD Motion_Detection_Calc_Count=0;//

BYTE Motion_Detection_State=0; //0:detect motion  1:motion+ing detect no motion
//連續變化都小於某值的次數
DWORD Continue_NoMotion_cnt=0;

DWORD Motion_threshold = 5;

BYTE Global_Motion_Detection_Result(void)
{
		DWORD SUM=0;
		Global_Motion_Detection_Calc();
		if(Motion_Detection_State==0)
		{
				if(Motion_Detection_Calc_Count<Motion_Detection_Calc_threshold)
						return 0;
				int i=0;
				for(i=0;i<Motion_Detection_Calc_threshold;i++)
						SUM+=Diff_Data_Queue[i];
				//mpDebugPrint("SUM=%d", SUM);

					static BYTE bMotionValueTimes=0,bTimeMotionValue=0;;
					if (bMotionValueTimes==0)
					{
						bTimeMotionValue = SUM;
						bMotionValueTimes++;
						SUM =0;
					}
					else if (bMotionValueTimes)
					{
							SUM = (bTimeMotionValue+SUM)>>1;
							bMotionValueTimes = 0;
							//mpDebugPrint("SUM=%d", SUM);
							//mpDebugPrint("%s: bTimeMotionValue =%d", __FUNCTION__, bTimeMotionValue);
					}

				if (SUM>Motion_threshold)
				{
						mpDebugPrint("SUM=%d Motion_threshold=%d", SUM,Motion_threshold);
						Global_Motion_Detection_ReSetParam();
						//Motion_Detection_State=1;


						return 1;
				}
				return 0;

		}else{
				if(Continue_NoMotion_cnt>NoMotion_count_threshold)
						return 0;
				return 1;
		}

		//for(i=0;i<Motion_Detection_Calc_threshold;i++)
			//mpDebugPrint("Diff_Data_Queue[%d]=%d",i, Diff_Data_Queue[i]);
}
BYTE Global_Motion_Detection_ReSetParam(void)
{
		int i=0;
		Motion_Detection_Old_Value=0;
		Queue_input_index=0;
		Motion_Detection_Calc_Count=0;
		Motion_Detection_State=0;
		Continue_NoMotion_cnt=0;
		Motion_Detection_Value = 0;
		for(i=0;i<Motion_Detection_Calc_threshold;i++)
			Diff_Data_Queue[i]=0;
}

BYTE Global_Motion_Detection_Calc(void)
{
		DWORD MD_Value=0;/*Motion Detection(Average Luminance.*/
		MD_Value=CVBS_Sensor_GetAvgLuminance();

		if(Motion_Detection_State==0)
		{
				if(Motion_Detection_Calc_Count>0)
				{
						Diff_Data_Queue[Queue_input_index++]=(abs(MD_Value-Motion_Detection_Old_Value))>>1;
						//mpDebugPrint("new diff=%d MD_Value=%d <- %d", Diff_Data_Queue[Queue_input_index-1],MD_Value,Motion_Detection_Old_Value);
				}

				if(Queue_input_index>=Motion_Detection_Calc_threshold)
					Queue_input_index=0;

		}else{
				if(Motion_Detection_Calc_Count>0)
				{
						if(abs(MD_Value-Motion_Detection_Old_Value)<NoMotion_threshold)
								Continue_NoMotion_cnt++;
						else
								Continue_NoMotion_cnt=0;

				}


		}

		Motion_Detection_Calc_Count++;
		Motion_Detection_Old_Value=MD_Value;

}

void Set_Motion_threshold_Level(DWORD value)
{
    // value==1, Low
    // value==2, Normal
    // value==3, High
    if (value == 1)
        Motion_threshold = 60; // 90
    else if (value == 2)
        Motion_threshold = 40;// 60
    else if (value == 3)
        Motion_threshold = 30;// 40
}

#endif


static void Config_Pclk(void)
{	
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
	//regClockPtr->Clkss2 |= BIT16; 
	MP_ALERT("regClockPtr->Clkss2 = 0x%x", regClockPtr->Clkss2);
}


//static 
int Sensor_Run()
{
	
		IPU *ipu = (IPU *) IPU_BASE;
		ipu_reset();
		

        Config_Pclk();

		//IntEnable();
		//砞mpx hw
		Local_HW_Set();		//砞sensor

		if(overlay_enable && (overlay_image_Win.pdwStart != NULL))
        {
			Capture_overlay_660(&overlay_image_Win, OverlayKeyY, OverlayKeyCb, OverlayKeyCr);
        }

		//IntDisable();
#if !VirtualSensorEnable
		//Drive_Sensor();
#endif

		//IntEnable();

		//SensorWait(2000);
		//ゴ秨hw琵sensor瑈秈ㄓ(璶絋﹚sensor块竒铆﹚)
	/*
		ipu->Ipu_reg_F0 &= ~BIT7;//ipw2 start write memory
		while((ipu->Ipu_reg_F0 & BIT31) != 0x00000000);
	*/
}

//**************************
#if 0



void printIPU_sensor(void)
{
		IPU *ipu = (IPU *) IPU_BASE;
    mpDebugPrint("ipu->Ipu_reg_10=0x%08x",  ipu->Ipu_reg_10);
    mpDebugPrint("ipu->Ipu_reg_11=0x%08x",  ipu->Ipu_reg_11);
    mpDebugPrint("ipu->Ipu_reg_12=0x%08x",  ipu->Ipu_reg_12);
    mpDebugPrint("ipu->Ipu_res0=0x%08x",    ipu->Ipu_res0);
    mpDebugPrint("ipu->Ipu_reg_14=0x%08x",  ipu->Ipu_reg_14);
    mpDebugPrint("ipu->Ipu_reg_15=0x%08x",  ipu->Ipu_reg_15);
    mpDebugPrint("ipu->Ipu_reg_16=0x%08x",  ipu->Ipu_reg_16);
    mpDebugPrint("ipu->Ipu_reg_17=0x%08x",  ipu->Ipu_reg_17);
    mpDebugPrint("ipu->Ipu_reg_18=0x%08x",  ipu->Ipu_reg_18);
    mpDebugPrint("ipu->Ipu_reg_19=0x%08x",  ipu->Ipu_reg_19);
    mpDebugPrint("ipu->Ipu_reg_1A=0x%08x",  ipu->Ipu_reg_1A);
    mpDebugPrint("ipu->Ipu_reg_1B=0x%08x",  ipu->Ipu_reg_1B);
    mpDebugPrint("ipu->Ipu_reg_1C=0x%08x",  ipu->Ipu_reg_1C);
    mpDebugPrint("ipu->Ipu_reg_1D=0x%08x",  ipu->Ipu_reg_1D);
    mpDebugPrint("ipu->Ipu_reg_1E=0x%08x",  ipu->Ipu_reg_1E);
    mpDebugPrint("ipu->Ipu_reg_1F=0x%08x",  ipu->Ipu_reg_1F);
    mpDebugPrint("ipu->Ipu_reg_20=0x%08x",  ipu->Ipu_reg_20);
    mpDebugPrint("ipu->Ipu_reg_21=0x%08x",  ipu->Ipu_reg_21);
    mpDebugPrint("ipu->Ipu_reg_22=0x%08x",  ipu->Ipu_reg_22);
    mpDebugPrint("ipu->Ipu_reg_23=0x%08x",  ipu->Ipu_reg_23);
    mpDebugPrint("ipu->Ipu_reg_24=0x%08x",  ipu->Ipu_reg_24);
    mpDebugPrint("ipu->Ipu_reg_25=0x%08x",  ipu->Ipu_reg_25);
    mpDebugPrint("ipu->Ipu_reg_26=0x%08x",  ipu->Ipu_reg_26);
    mpDebugPrint("ipu->Ipu_reg_27=0x%08x",  ipu->Ipu_reg_27);
    mpDebugPrint("ipu->Ipu_reg_28=0x%08x",  ipu->Ipu_reg_28);
		mpDebugPrint("ipu->Ipu_reg_29=0x%08x",  ipu->Ipu_reg_29);
		mpDebugPrint("ipu->Ipu_reg_2A=0x%08x",  ipu->Ipu_reg_2A);
		mpDebugPrint("ipu->Ipu_reg_2B=0x%08x",  ipu->Ipu_reg_2B);
		//mpDebugPrint("ipu->Ipu_res1[4]=0x%08x",ipu->Ipu_res1[4]);
    mpDebugPrint("ipu->Ipu_reg_30=0x%08x",  ipu->Ipu_reg_30);
    mpDebugPrint("ipu->Ipu_reg_31=0x%08x",  ipu->Ipu_reg_31);
    mpDebugPrint("ipu->Ipu_reg_32=0x%08x",  ipu->Ipu_reg_32);
    mpDebugPrint("ipu->Ipu_reg_33=0x%08x",  ipu->Ipu_reg_33);
    mpDebugPrint("ipu->Ipu_reg_34=0x%08x",  ipu->Ipu_reg_34);
    mpDebugPrint("ipu->Ipu_reg_35=0x%08x",  ipu->Ipu_reg_35);
    mpDebugPrint("ipu->Ipu_reg_36=0x%08x",  ipu->Ipu_reg_36);
    mpDebugPrint("ipu->Ipu_reg_37=0x%08x",  ipu->Ipu_reg_37);
    mpDebugPrint("ipu->Ipu_reg_38=0x%08x",  ipu->Ipu_reg_38);
    mpDebugPrint("ipu->Ipu_reg_39=0x%08x",  ipu->Ipu_reg_39);
    mpDebugPrint("ipu->Ipu_reg_3A=0x%08x",  ipu->Ipu_reg_3A);
    mpDebugPrint("ipu->Ipu_reg_3B=0x%08x",  ipu->Ipu_reg_3B);
    mpDebugPrint("ipu->Ipu_reg_3C=0x%08x",  ipu->Ipu_reg_3C);
    mpDebugPrint("ipu->Ipu_reg_3D=0x%08x",  ipu->Ipu_reg_3D);
    mpDebugPrint("ipu->Ipu_reg_3E=0x%08x",  ipu->Ipu_reg_3E);
    //mpDebugPrint("ipu->Ipu_res2=0x%08x",    ipu->Ipu_res2);
    mpDebugPrint("ipu->Ipu_reg_40=0x%08x",  ipu->Ipu_reg_40);
    mpDebugPrint("ipu->Ipu_reg_41=0x%08x",  ipu->Ipu_reg_41);
    mpDebugPrint("ipu->Ipu_reg_42=0x%08x",  ipu->Ipu_reg_42);
    mpDebugPrint("ipu->Ipu_reg_43=0x%08x",  ipu->Ipu_reg_43);
    mpDebugPrint("ipu->Ipu_reg_44=0x%08x",  ipu->Ipu_reg_44);
    mpDebugPrint("ipu->Ipu_reg_45=0x%08x",  ipu->Ipu_reg_45);
    mpDebugPrint("ipu->Ipu_reg_46=0x%08x",  ipu->Ipu_reg_46);
    mpDebugPrint("ipu->Ipu_reg_47=0x%08x",  ipu->Ipu_reg_47);
    mpDebugPrint("ipu->Ipu_reg_48=0x%08x",  ipu->Ipu_reg_48);
    mpDebugPrint("ipu->Ipu_reg_49=0x%08x",  ipu->Ipu_reg_49);
    mpDebugPrint("ipu->Ipu_reg_4A=0x%08x",  ipu->Ipu_reg_4A);
    mpDebugPrint("ipu->Ipu_reg_4B=0x%08x",  ipu->Ipu_reg_4B);
    mpDebugPrint("ipu->Ipu_reg_4C=0x%08x",  ipu->Ipu_reg_4C);
    mpDebugPrint("ipu->Ipu_reg_4D=0x%08x",  ipu->Ipu_reg_4D);
    //mpDebugPrint("ipu->Ipu_res3[2]=0x%08x", ipu->Ipu_res3[2]);
    mpDebugPrint("ipu->Ipu_reg_50=0x%08x",  ipu->Ipu_reg_50);
    mpDebugPrint("ipu->Ipu_reg_51=0x%08x",  ipu->Ipu_reg_51);
    mpDebugPrint("ipu->Ipu_reg_52=0x%08x",  ipu->Ipu_reg_52);
    mpDebugPrint("ipu->Ipu_reg_53=0x%08x",  ipu->Ipu_reg_53);
    mpDebugPrint("ipu->Ipu_reg_54=0x%08x",  ipu->Ipu_reg_54);
    mpDebugPrint("ipu->Ipu_reg_55=0x%08x",  ipu->Ipu_reg_55);
    mpDebugPrint("ipu->Ipu_reg_56=0x%08x",  ipu->Ipu_reg_56);
    mpDebugPrint("ipu->Ipu_reg_57=0x%08x",  ipu->Ipu_reg_57);
    mpDebugPrint("ipu->Ipu_reg_58=0x%08x",  ipu->Ipu_reg_58);
    //mpDebugPrint("ipu->Ipu_res4[7]=0x%08x", ipu->Ipu_res4[7]);
    mpDebugPrint("ipu->Ipu_reg_60=0x%08x",  ipu->Ipu_reg_60);
    mpDebugPrint("ipu->Ipu_reg_61=0x%08x",  ipu->Ipu_reg_61);
    mpDebugPrint("ipu->Ipu_reg_62=0x%08x",  ipu->Ipu_reg_62);
    mpDebugPrint("ipu->Ipu_reg_63=0x%08x",  ipu->Ipu_reg_63);
    mpDebugPrint("ipu->Ipu_reg_64=0x%08x",  ipu->Ipu_reg_64);
    mpDebugPrint("ipu->Ipu_reg_65=0x%08x",  ipu->Ipu_reg_65);
    mpDebugPrint("ipu->Ipu_reg_66=0x%08x",  ipu->Ipu_reg_66);
    mpDebugPrint("ipu->Ipu_reg_67=0x%08x",  ipu->Ipu_reg_67);
    mpDebugPrint("ipu->Ipu_reg_68=0x%08x",  ipu->Ipu_reg_68);
    mpDebugPrint("ipu->Ipu_reg_69=0x%08x",  ipu->Ipu_reg_69);
    mpDebugPrint("ipu->Ipu_reg_6A=0x%08x",  ipu->Ipu_reg_6A);
    mpDebugPrint("ipu->Ipu_reg_6B=0x%08x",  ipu->Ipu_reg_6B);
    mpDebugPrint("ipu->Ipu_reg_6C=0x%08x",  ipu->Ipu_reg_6C);
    mpDebugPrint("ipu->Ipu_reg_6D=0x%08x",  ipu->Ipu_reg_6D);
    mpDebugPrint("ipu->Ipu_reg_6E=0x%08x",  ipu->Ipu_reg_6E);
    mpDebugPrint("ipu->Ipu_reg_6F=0x%08x",  ipu->Ipu_reg_6F);
    mpDebugPrint("ipu->Ipu_reg_70=0x%08x",  ipu->Ipu_reg_70);
    mpDebugPrint("ipu->Ipu_reg_71=0x%08x",  ipu->Ipu_reg_71);
    //mpDebugPrint("ipu->Ipu_res5[14]=0x%08x",ipu->Ipu_res5[14]);
    mpDebugPrint("ipu->Ipu_reg_80=0x%08x",  ipu->Ipu_reg_80);
    mpDebugPrint("ipu->Ipu_reg_81=0x%08x",  ipu->Ipu_reg_81);
    mpDebugPrint("ipu->Ipu_reg_82=0x%08x",  ipu->Ipu_reg_82);
    //mpDebugPrint("ipu->Ipu_res6[13]=0x%08x",ipu->Ipu_res6[13]);
    mpDebugPrint("ipu->Ipu_reg_90=0x%08x",  ipu->Ipu_reg_90);
    mpDebugPrint("ipu->Ipu_reg_91=0x%08x",  ipu->Ipu_reg_91);
    mpDebugPrint("ipu->Ipu_reg_92=0x%08x",  ipu->Ipu_reg_92);
    mpDebugPrint("ipu->Ipu_reg_93=0x%08x",  ipu->Ipu_reg_93);
    mpDebugPrint("ipu->Ipu_reg_94=0x%08x",  ipu->Ipu_reg_94);
    //mpDebugPrint("ipu->Ipu_res7[11]=0x%08x",ipu->Ipu_res7[11]);
    mpDebugPrint("ipu->Ipu_reg_A0=0x%08x",  ipu->Ipu_reg_A0);
    mpDebugPrint("ipu->Ipu_reg_A1=0x%08x",  ipu->Ipu_reg_A1);
    mpDebugPrint("ipu->Ipu_reg_A2=0x%08x",  ipu->Ipu_reg_A2);
    mpDebugPrint("ipu->Ipu_reg_A3=0x%08x",  ipu->Ipu_reg_A3);
    mpDebugPrint("ipu->Ipu_reg_A4=0x%08x",  ipu->Ipu_reg_A4);
    //mpDebugPrint("ipu->Ipu_res8[11]=0x%08x",ipu->Ipu_res8[11]);
    mpDebugPrint("ipu->Ipu_reg_B0=0x%08x",  ipu->Ipu_reg_B0);
    mpDebugPrint("ipu->Ipu_reg_B1=0x%08x",  ipu->Ipu_reg_B1);
    mpDebugPrint("ipu->Ipu_reg_B2=0x%08x",  ipu->Ipu_reg_B2);
    mpDebugPrint("ipu->Ipu_reg_B3=0x%08x",  ipu->Ipu_reg_B3);
    mpDebugPrint("ipu->Ipu_reg_B4=0x%08x",  ipu->Ipu_reg_B4);
    mpDebugPrint("ipu->Ipu_reg_B5=0x%08x",  ipu->Ipu_reg_B5);
    mpDebugPrint("ipu->Ipu_reg_B6=0x%08x",  ipu->Ipu_reg_B6);
    mpDebugPrint("ipu->Ipu_reg_B7=0x%08x",  ipu->Ipu_reg_B7);
    mpDebugPrint("ipu->Ipu_reg_B8=0x%08x",  ipu->Ipu_reg_B8);
    mpDebugPrint("ipu->Ipu_reg_B9=0x%08x",  ipu->Ipu_reg_B9);
    mpDebugPrint("ipu->Ipu_reg_BA=0x%08x",  ipu->Ipu_reg_BA);
    //mpDebugPrint("ipu->Ipu_res9[53]=0x%08x",ipu->Ipu_res9[53]);
    mpDebugPrint("ipu->Ipu_reg_F0=0x%08x",  ipu->Ipu_reg_F0);
    mpDebugPrint("ipu->Ipu_reg_F1=0x%08x",  ipu->Ipu_reg_F1);
    mpDebugPrint("ipu->Ipu_reg_F2=0x%08x",  ipu->Ipu_reg_F2);
    mpDebugPrint("ipu->Ipu_reg_F3=0x%08x",  ipu->Ipu_reg_F3);
    mpDebugPrint("ipu->Ipu_reg_F4=0x%08x",  ipu->Ipu_reg_F4);
    mpDebugPrint("ipu->Ipu_reg_F5=0x%08x",  ipu->Ipu_reg_F5);
    mpDebugPrint("ipu->Ipu_reg_F6=0x%08x",  ipu->Ipu_reg_F6);
    mpDebugPrint("ipu->Ipu_reg_F7=0x%08x",  ipu->Ipu_reg_F7);
    mpDebugPrint("ipu->Ipu_reg_F8=0x%08x",  ipu->Ipu_reg_F8);
    //mpDebugPrint("ipu->Ipu_res10[7]=0x%08x",ipu->Ipu_res10[7]);
    mpDebugPrint("ipu->Ipu_reg_100=0x%08x", ipu->Ipu_reg_100);
    mpDebugPrint("ipu->Ipu_reg_101=0x%08x", ipu->Ipu_reg_101);
    mpDebugPrint("ipu->Ipu_reg_102=0x%08x", ipu->Ipu_reg_102);
    mpDebugPrint("ipu->Ipu_reg_103=0x%08x", ipu->Ipu_reg_103);
    mpDebugPrint("ipu->Ipu_reg_104=0x%08x", ipu->Ipu_reg_104);
    mpDebugPrint("ipu->Ipu_reg_105=0x%08x", ipu->Ipu_reg_105);
    mpDebugPrint("ipu->Ipu_reg_106=0x%08x", ipu->Ipu_reg_106);
    mpDebugPrint("ipu->Ipu_reg_107=0x%08x", ipu->Ipu_reg_107);
    mpDebugPrint("ipu->Ipu_reg_108=0x%08x", ipu->Ipu_reg_108);
    mpDebugPrint("ipu->Ipu_reg_109=0x%08x", ipu->Ipu_reg_109);
    mpDebugPrint("ipu->Ipu_reg_10A=0x%08x", ipu->Ipu_reg_10A);
    mpDebugPrint("ipu->Ipu_reg_10B=0x%08x", ipu->Ipu_reg_10B);


}
#endif
//**************************

void CVBS_EvenFramFlag_Init(void)
{
	g_EvenFrame = 0;
}

BYTE CVBS_Get_EvenFramFlag(void)
{
	return g_EvenFrame;
}


void CVBS_Clear_EvenFramFlag(void)
{
	g_EvenFrame = 0;
}

void CVBS_Set_EvenFramFlag(void)
{
	g_EvenFrame = 1;
}

int Global_Sensor_Initial_CVBS(void)
{
	extern BYTE Sensor_Mode;
	IPU *ipu = (IPU *) IPU_BASE;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
	DWORD SensorEvent;
	ST_IMGWIN *win = Idu_GetNextWin();
	DWORD t = 0, val = 0 ;

	E_SENSOR_ZOOM_LEVEL Get_CVBS_ZoomStep;

	if(Get_CVBS_ZoomStatus()  == 1)
	{
	  Get_CVBS_ZoomStep = g_CVBS_ZoomStep ;
	  Sensor_CVBS_ZoomPreview(40,60, Get_CVBS_ZoomStep);
	  return;
	}

	Start_Time = 0 ;
	
	//DoubleBuffer=0;

	Motion_Detection_Value=0;
#if 1
  //sensor_IMGSIZE_ID = SIZE_480P_720x480;//frank debug
  switch(sensor_IMGSIZE_ID)
  {
  	case SIZE_480P_720x480:
#if THREE_WINDOW_MODE
		SetStepWindowSize();
#else
	  API_SetCVBS_RecordSize(720,480);
#endif
	  break;
		
  	default:
	  MP_ALERT("--E-- %s: MODE_RECORD", __FUNCTION__);
	  MP_ALERT("--E-- %s: Not support resolution (sensor_IMGSIZE_ID =%d) ", __FUNCTION__, sensor_IMGSIZE_ID);
	  return FAIL;
  }
#else
  if(sensor_mode==MODE_RECORD)
  {
	if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240)
	{
    	API_SetCVBS_RecordSize(320 ,240);
	}else if(sensor_IMGSIZE_ID==SIZE_CIF_352x240)
	{
		API_SetCVBS_RecordSize(352, 240);
    }else	if(sensor_IMGSIZE_ID==SIZE_VGA_640x480)
    {
        API_SetCVBS_RecordSize(640, 480);	
    }else if(sensor_IMGSIZE_ID==SIZE_480P_720x480)
    {
        API_SetCVBS_RecordSize(720,480);
    }else if(sensor_IMGSIZE_ID==SIZE_XGA_1024x768)
    {
        API_SetCVBS_RecordSize(1024,768);	
    }else if(sensor_IMGSIZE_ID == SIZE_720P_1280x720)
    {
  	    API_SetCVBS_RecordSize(1280, 720);	
	}else
	{
		MP_ALERT("--E-- %s: MODE_RECORD", __FUNCTION__);
		MP_ALERT("--E-- %s: Not support resolution (sensor_IMGSIZE_ID =%d) ", __FUNCTION__, sensor_IMGSIZE_ID);
		return FAIL;
	}


  }else if(sensor_mode==MODE_CAPTURE)
  {
    //VGA preview
    //SensorInputWidth  = 640;
	//SensorInputHeight = 480;				

  }else if(sensor_mode==MODE_PCCAM)
  {
    if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240){
      API_SetCVBS_RecordSize(320, 240);
    }
    else if(sensor_IMGSIZE_ID == SIZE_QCIF_176x144){
      API_SetCVBS_RecordSize(176, 144);
    }
    else
	{		
      MP_ALERT("--E-- %s: MODE_PCCAM", __FUNCTION__);
      MP_ALERT("--E-- %s: Not support resolution (sensor_IMGSIZE_ID =%d) ", __FUNCTION__, sensor_IMGSIZE_ID);
 	  return FAIL;
    }

  }else
  {
    MP_ALERT("--E-- %s: sensor_mode = %d", __FUNCTION__, sensor_mode);
    MP_ALERT("--E-- %s: Not support sensor inter face mode.", __FUNCTION__);
    return FAIL;
  }
#endif
  Set_CVBS_Input_resolution();


  /*Need to check here. The mode and the resoultion are supported or not.*/
  /*If the resoultion is not supported, "return FAIL" here.*/


  //EventCreate(SENSOR_IPW_FRAME_END_EVENT_ID, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
  

  Sensor_Run();

#if ((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))
       // Idu_ChgWin(win);
#endif
   
  IntEnable();
  Global_Sensor_Run();

  Sensor_Mode=1;	
//printIPU_sensor();
#if 0  /*debug only*/

	//Wait event
	//EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT0, OS_EVENT_OR, &SensorEvent, 8000);//ipw1
	//EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1, OS_EVENT_OR, &SensorEvent, 8000);//ipw2


	//for debug :dump data
	BYTE *SensorInJpegBuffer=(BYTE*)ext_mem_malloc(320*240*2);
	DWORD jpegsize=GetSensorJpegImage(SensorInJpegBuffer);
	Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "img", "jpg", SensorInJpegBuffer, jpegsize);
	


	//DumpYuvImage2Bmp(win);


#endif



	return PASS;
}

/*
void SensorTaskCreate()
{
	EventCreate(SENSOR_IPW_FRAME_END_EVENT_ID, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	TaskCreate(SENSOR_TASK, SensorTask, DRIVER_PRIORITY, 0x4000);
	TaskStartup(SENSOR_TASK);
}
*/

int Sensor_Capture_CVBS(STREAM *fileHandle, BYTE Capture_IMGSIZE_ID)
{
	MP_DEBUG1("## %s ##", __FUNCTION__);		

	int iErrorCode;

    IPU *ipu = (IPU *) IPU_BASE;
	DWORD SensorEvent;
	DWORD TimeOutCnt = 0x100000;
	ST_IMGWIN Original_Win;

	DWORD JpegBufSize;
	DWORD IMG_size = 0;

	BYTE *JpegBuf = NULL;

    

    if(Get_CVBS_ZoomStatus() == 1)
    {
    }
	else
	{
    //open IPW2 for preview encode buffer
	ipu->Ipu_reg_F0 &= ~BIT7;
	}
	
	iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 1000);//wait CVBS IPW2
    MP_DEBUG1("%s: Wait Event 1", __FUNCTION__);
	if(iErrorCode < 0)
		goto Error;

	iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 1000);//wait CVBS IPW2
    MP_DEBUG1("%s: Wait Event 2", __FUNCTION__);

    if(iErrorCode < 0)
		goto Error;

	iErrorCode = EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT2, OS_EVENT_OR, &SensorEvent, 1000);//wait CVBS IPW2
    MP_DEBUG1("%s: Wait Event 3", __FUNCTION__);

    if(iErrorCode < 0)
		goto Error;
    
    /*close sensor input*/
	ipu->Ipu_reg_100 &= ~BIT0; //close SenInEn
	while(TimeOutCnt >0)
	{
		if((ipu->Ipu_reg_100&BIT0)==0)//絋﹚Τ糶秈
			break;
		TimeOutCnt--;
	}

	//close IPW2 for preview encode buffer
	ipu->Ipu_reg_F0 |= BIT7;

	if(TimeOutCnt == 0)
	{
		MP_ALERT("%s: Capture close SenInEn timeout", __FUNCTION__);
		goto Error;
	}

	

    JpegBufSize = CVBS_getRecordWidth()*CVBS_getRecordHeight()*2;
    JpegBuf = (BYTE*)ext_mem_malloc(JpegBufSize);
    if(JpegBuf==NULL)
    {   
        MP_ALERT("%s: JpegBuf if NULL!", __FUNCTION__);
        goto Error;
    }
    ///////  start to encode JPEG /////////
    //
    if(Get_CVBS_ZoomStatus() == 1)
    {	 
	  //Use IPW1 to encode JPEG
	  Original_Win.pdwStart = ipu->Ipu_reg_A3;
    }
	else
	{
	  //Use IPW2 to encode JPEG
      Original_Win.pdwStart = ipu->Ipu_reg_F2;
	}

    //Capture_IMGSIZE_ID = SIZE_480P_720x480;//frank debug
    switch(Capture_IMGSIZE_ID)
    {
        case SIZE_480P_720x480:
            Original_Win.wWidth  = 720;
            Original_Win.wHeight = 480;
            Original_Win.dwOffset=Original_Win.wWidth<<1;
        break;
		   
        default:
            MP_ALERT("%s:Not Support capture IMGSIZE. ID=%d", __FUNCTION__,Capture_IMGSIZE_ID);
            goto Error;
	}

    //encode jpeg
    WriteTimeStampStringToPhoto(&Original_Win);
    IMG_size = ImageFile_Encode_Img2Jpeg(JpegBuf, &Original_Win);//(JpegBuf, &Original_Win);

    if (JpegBufSize < IMG_size)
    {
        mpDebugPrint("--E-- %s: memory overflow", __FUNCTION__);
        //free memory
        if(JpegBuf != NULL)
        {
            ext_mem_free(JpegBuf);
            JpegBuf = NULL;
        }

        __asm("break 100");
        return -3;
   }
   else
   {
#if SAVE_FILE_TO_SPI
        extern BOOL g_boSaveFileToSPI;
        if (g_boSaveFileToSPI)
        {
            WORD wFreePages;
            DWORD dwNewIndex;
            int error;
            wFreePages = spifs_GetFreePageCount2();
			mpDebugPrint("FreeSize=%dByte",wFreePages*SPI_PAGE_SIZE);
            if (IMG_size > wFreePages*SPI_PAGE_SIZE)    // full
            {
                ST_FILE_BROWSER *psBrowser;
                ST_SEARCH_INFO * pSearchInfo;
                DWORD dwTotalFileCount;
                
                mpDebugPrint("No enough SPI space to save jpeg file.");
                mpDebugPrint("Try delete old files.");
                psBrowser = (ST_FILE_BROWSER *)&g_psSystemConfig->sFileBrowser;
                pSearchInfo = &psBrowser->sImgAndMovFileList[psBrowser->dwImgAndMovCurIndex];
                dwTotalFileCount = psBrowser->dwImgAndMovTotalFile;
                
                spi_ClearOldFileSpaceForNewFile(pSearchInfo, dwTotalFileCount);

                wFreePages = spifs_GetFreePageCount2();
                if (IMG_size > wFreePages*SPI_PAGE_SIZE)
                    iErrorCode = -1;
                else
                    spifs_WriteBufferToNewFile(&dwNewIndex, JpegBuf, IMG_size, SPI_FILE_TYPE_JPG_FILE, &error);
            }
            else
            {
                spifs_WriteBufferToNewFile(&dwNewIndex, JpegBuf, IMG_size, SPI_FILE_TYPE_JPG_FILE, &error);
            }
        }
        else
#endif
        {
            FileWrite(fileHandle, (BYTE *) JpegBuf, IMG_size);
            FileClose(fileHandle);
        }
    }

    //free memory
    if(JpegBuf != NULL)
    {
        ext_mem_free(JpegBuf);
        JpegBuf = NULL;
    }

    ipu->Ipu_reg_100 |= BIT0; //SenInEn
    
	return iErrorCode;

Error:
	//API_Sensor_Stop();
	/*When exit caputer mod, need to call API_Sensor_Stop() to free preview memoy */

	ipu->Ipu_reg_100 |= BIT0; //SenInEn
	return FAIL;


}



#endif


void Sensor_CVBS_ChangeToRecode(DWORD X_pos, DWORD Y_pos, DWORD DisplayW, DWORD DisplayH)
{	
	MP_DEBUG1("## %s ##", __FUNCTION__);
	
	extern BYTE Sensor_Mode;
	IPU *ipu = (IPU *) IPU_BASE;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
	DWORD SensorEvent;
	ST_IMGWIN *win = Idu_GetNextWin();
	DWORD t = 0, val = 0 ;
	
	E_SENSOR_ZOOM_LEVEL Get_CVBS_ZoomStep;

#if 0 //franklin test 

API_SelectChannel(0);

//Sensor_CVBS_ZoomPreview(40,60, SENSOR_ZOOM_LEVEL_1_33);
//Sensor_CVBS_ZoomPreview(40,60, SENSOR_ZOOM_LEVEL_1_66);

API_SetZoomPos(0, 0, SENSOR_ZOOM_LEVEL_1_33);

Sensor_CVBS_ZoomPreview(40,60, SENSOR_ZOOM_LEVEL_1_33);

#endif

   MP_DEBUG2("%s:----> Get_CVBS_ZoomStatus() = %d", __FUNCTION__, Get_CVBS_ZoomStatus());

    if(Get_CVBS_ZoomStatus() != 1)
    {

	  API_Sensor_Stop();	
	  API_SetSensorWindow( X_pos, Y_pos, DisplayW, DisplayH);

	  Start_Time = 0 ;
	  Sensor_Mode=1;	
	  Motion_Detection_Value=0;

	  Dma_FdmaFifoTimingAdjust(0);

      Set_CVBS_Input_resolution();
	  Sensor_Run();	
	
	  IntEnable();
	  Global_Sensor_Run();
    }
	else
	{		
		  //Zoom
	  if(Get_CVBS_ZoomStatus()	== 1)
	  {
	    Get_CVBS_ZoomStep = g_CVBS_ZoomStep ;
	    Sensor_CVBS_ZoomPreview(40,60, Get_CVBS_ZoomStep); 
	  }
	}
    //printIPU_sensor();

#if 0 //frank test CVBS Zoom preview
	
	Sensor_CVBS_ZoomPreview(40,60, 3);
	return ;
	
#endif


}

void Set_CVBS_EnterZoomMode(void)
{
	g_CVBS_ZoomPreview_flag = 1;
}

BYTE Get_CVBS_ZoomStatus(void)
{
	return g_CVBS_ZoomPreview_flag;
}

void Set_CVBS_ExitZoomMode(void)
{
	g_CVBS_ZoomPreview_flag = 0;
}

void Sensor_CVBS_ZoomPreview(DWORD X_pos, DWORD Y_pos, E_SENSOR_ZOOM_LEVEL ZoomStep)
{
    extern BYTE Sensor_Mode;
	IPU *ipu = (IPU *) IPU_BASE;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
	DWORD SensorEvent;
	ST_IMGWIN *win = Idu_GetNextWin();
	DWORD t = 0, val = 0 ;

	API_Sensor_Stop();

#if 0 
/*Set background color to black color*/
/*Need to set the same color in curret win and next win*/
	ST_IMGWIN *win1;
	win1=Idu_GetNextWin();
	mpClearWin(win1);

	win1=Idu_GetCurrWin();
	mpClearWin(win1);
#endif 	
	
	Dma_FdmaFifoTimingAdjust(1);

	Set_CVBS_EnterZoomMode();
	
	API_SetCVBS_RecordSize(800, 480);

	API_SetSensorWindow( X_pos, Y_pos, 800, 480);

	Start_Time = 0 ;
	Sensor_Mode=1;	
	Motion_Detection_Value=0;
	g_CVBS_ZoomStep = ZoomStep;

    switch(g_CVBS_ZoomStep)
    {
      case SENSOR_ZOOM_LEVEL_1_33:/*1.33x*/	  	
		SensorInputWidth  = 700;
		SensorInputHeight = 180;
	  	break;

	  case SENSOR_ZOOM_LEVEL_1_66:/*1.66x*/	  	
		SensorInputWidth  = 432;
		SensorInputHeight = 144;
	  	break;	
		
      case SENSOR_ZOOM_LEVEL_2_00:/*2X*/
	      SensorInputWidth  = 432; // 360
          SensorInputHeight = 144;// 120
        break;

	  default:	
      case SENSOR_ZOOM_LEVEL_1_00:
	      SensorInputWidth  = 720;
          SensorInputHeight = 240;
	  	break;
		
      
	  
    }

     MP_DEBUG2("@@@@ %s: SensorInputWidth = %d", __FUNCTION__, SensorInputWidth);
     MP_DEBUG2("@@@@ %s: SensorInputHeight= %d", __FUNCTION__, SensorInputHeight);

	Sensor_Run();
			
	IntEnable();
	Global_Sensor_Run();

	//frank debug 
	#if 0
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret1;
		BYTE Capture_IMGSIZE_ID = SIZE_480P_720x480;
		UartOutText("creat jpeg file");
		sDrv=DriveGet(SD_MMC);
		ret1=CreateFile(sDrv, "output_debug_zoom", "jpg");
		if (ret1) UartOutText("create file fail\r\n");
		shandle=FileOpen(sDrv);
			
		  Sensor_Capture_CVBS(shandle, Capture_IMGSIZE_ID);
	#endif

	//ShowZoomAction(1,g_CVBS_ZoomStep);

	

	//printIPU_sensor();
}

void Sensor_CVBS_ZoomPreview_stop(void)
{
	API_Sensor_Stop();	
	Set_CVBS_ExitZoomMode();

	
	//only for Debug only : display 720x480
	API_SetSensorWindow(0, 0, 800, 480);

    Global_Sensor_Initial_CVBS();
}

void xpgCb_PreviewZoomUp()
{
	
	if (!g_CVBS_ZoomPreview_flag)
		g_CVBS_ZoomStep  = SENSOR_ZOOM_LEVEL_1_00;
	if (g_CVBS_ZoomStep >= SENSOR_ZOOM_LEVEL_2_00)
	{
		Sensor_CVBS_ZoomPreview_stop();
		return;
	}
	g_CVBS_ZoomStep++;
	mpDebugPrint("_%s_:g_CVBS_ZoomStep=%d",__func__,g_CVBS_ZoomStep);

	Sensor_CVBS_ZoomPreview(0, 0, g_CVBS_ZoomStep);
   
}


void xpgCb_PreviewZoomDown()
{

	if (!g_CVBS_ZoomPreview_flag)
	{
		g_CVBS_ZoomStep=SENSOR_ZOOM_LEVEL_2_00;
		Sensor_CVBS_ZoomPreview(0, 0, g_CVBS_ZoomStep);
		return;
	}
	if (g_CVBS_ZoomStep > SENSOR_ZOOM_LEVEL_2_00)
		g_CVBS_ZoomStep=SENSOR_ZOOM_LEVEL_2_00;
	else if ( g_CVBS_ZoomStep<=SENSOR_ZOOM_LEVEL_1_00)
		g_CVBS_ZoomStep++;
		
	g_CVBS_ZoomStep--;
	mpDebugPrint("_%s_:g_CVBS_ZoomStep=%d",__func__,g_CVBS_ZoomStep);

	if ( g_CVBS_ZoomStep==SENSOR_ZOOM_LEVEL_1_00)
		Sensor_CVBS_ZoomPreview_stop();
	else
		Sensor_CVBS_ZoomPreview(0, 0, g_CVBS_ZoomStep);
    
}

#endif


