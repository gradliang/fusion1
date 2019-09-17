/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

//#define USE_SYSTEM_I2C_FUNCTION 1

#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "idu.h"
#include "sensor.h"
#include "peripheral.h"

//#include "../../SpyCamera/main/include/ui_timer.h"

#if ((SENSOR_ENABLE == ENABLE) && defined(SENSOR_TYPE_NT99140))

/*The buffer pointer reference by JPEG and Sensor input -- Capure mode*/


extern int wait_sensor;
extern int SensorInputWidth;
extern int SensorInputHeight;
#if SENSOR_WIN_NUM
extern BYTE *pbSensorWinBuffer;
extern ST_IMGWIN SensorInWin[SENSOR_WIN_NUM];
#endif
extern BYTE sensor_mode;
extern BYTE sensor_IMGSIZE_ID;
extern BYTE overlay_enable;
extern WORD SensorWindow_Width  ;
extern WORD SensorWindow_Height ;
extern BYTE SensorWindow_setFlag ;
extern WORD SensorWindow_PosX;
extern WORD SensorWindow_PosY;

static void Set_Ipw2(void);

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
/*
static void Local_HW_GPIO_Pin_ssen_0(void)
{
   SetGPIOValue(SENSOR_GPIO_SSEN, 0);
}

static void Local_HW_GPIO_Pin_ssen_1(void)
{
     SetGPIOValue(SENSOR_GPIO_SSEN, 1);
}
*/

#if (PRODUCT_PCBA==PCBA_MAIN_BOARD_V10)
//--1:only 1   2->only 2  4->only3
//--3->1+2   5->1+3  6->2+3
static BYTE st_bDisplayMode=1;

BYTE Sensor_CurChannel_Get(void)
{
	return st_bDisplayMode;
}

void Sensor_ChangeIO_Init(void)
{
		SetGPIOValue((FGPIO | GPIO_27), 1);//OE1  0->enalbe  1->disable
		SetGPIOValue((FGPIO | GPIO_42), 1);//OE2
		SetGPIOValue((FGPIO | GPIO_45), 1);//OE3
}

void Sensor_Channel_Set(BYTE bChannel)
{
		st_bDisplayMode=bChannel;

		if (st_bDisplayMode==2)
		{
			Gpio_DataSet(GPIO_FGPIO_27, GPIO_DATA_HIGH, 1);//OE1  0->enalbe  1->disable
			Gpio_DataSet(GPIO_FGPIO_45, GPIO_DATA_HIGH, 1);//OE3
			Gpio_DataSet(GPIO_FGPIO_42, GPIO_DATA_LOW, 1);//OE2
			//Gpio_DataSet(GPIO_FGPIO_42, GPIO_DATA_HIGH, 1);//OE2
			//Gpio_DataSet(GPIO_FGPIO_45, GPIO_DATA_LOW, 1);//OE3
		}
		else if (st_bDisplayMode==4)
		{
			Gpio_DataSet(GPIO_FGPIO_27, GPIO_DATA_HIGH, 1);//OE1  0->enalbe  1->disable
			Gpio_DataSet(GPIO_FGPIO_42, GPIO_DATA_HIGH, 1);//OE2
			Gpio_DataSet(GPIO_FGPIO_45, GPIO_DATA_LOW, 1);//OE3
		}
		else  //  1
		{
			Gpio_DataSet(GPIO_FGPIO_42, GPIO_DATA_HIGH, 1);//OE2
			Gpio_DataSet(GPIO_FGPIO_45, GPIO_DATA_HIGH, 1);//OE3
			Gpio_DataSet(GPIO_FGPIO_27, GPIO_DATA_LOW, 1);//OE1  0->enalbe  1->disable
		}

}

#elif (PRODUCT_PCBA==PCBA_MAIN_BOARD_V11||PRODUCT_PCBA==PCBA_MAIN_BOARD_V12||PRODUCT_PCBA==PCBA_MAIN_BOARD_V20)
//--st_bCurChannel:0->channel 0  1->channel 1
static BYTE st_bCurChannel=0;

// 0-> display sensor 1   1->sensor 2     2-> 1(up)+2(down)  3->1(left)+2(right) 
BYTE g_bDisplayMode=0x82;// 0x80

void Sensor_DisplayWindow_Set()
{
	ST_IMGWIN *pDstWin=(ST_IMGWIN *)Idu_GetNextWin();

	g_bDisplayMode&=0x0f;
	if (g_bDisplayMode>3)
		g_bDisplayMode=0;

	switch (g_bDisplayMode)
	{
		case 0:
			Sensor_Channel_Set(0);
			API_SetSensorWindow(0, 0, pDstWin->wWidth, pDstWin->wHeight);
			break;

		case 1:
			Sensor_Channel_Set(1);
			API_SetSensorWindow(0, 0, pDstWin->wWidth, pDstWin->wHeight);
			break;

		case 2:
			API_SetSensorWindow(0, 0, pDstWin->wWidth, pDstWin->wHeight>>1);
			break;
		case 3:
			API_SetSensorWindow(0, 0, pDstWin->wWidth>>1, pDstWin->wHeight);
			break;

		default:
			break;
	}

}
void Sensor_DisplayMode_Set()
{
	//mpDebugPrint("bNewMode=%p",g_bDisplayMode);
	Sensor_DisplayWindow_Set();
	Set_Ipw1();
	Set_Ipw2();
}

void Sensor_CacheWin_Set()
{
	Set_Ipw2();
	Set_Ipw1();
}


BYTE Sensor_CurChannel_Get(void)
{
	return st_bCurChannel;
}

void Sensor_ChangeIO_Init(void)
{
		SetGPIOValue((PGPIO | GPIO_02), 0);//  0->chanel 1  1->chanel 2
}

void Sensor_Channel_Set(BYTE bChannel)
{
	register GPIO *vGpio;
	vGpio = (GPIO *)(GPIO_BASE);

	st_bCurChannel=bChannel;

	if (st_bCurChannel)
		vGpio->Pgpdat |= 0x00000004;
	else
		vGpio->Pgpdat &= 0xfffffffb;

}

#endif


static void Sensor_Pin_Set(void)
{
    register GPIO *gpio = (GPIO *) (GPIO_BASE);

	//-Set all sensor pin to sensor mode
    gpio->Fgpcfg[0]&=~(BIT13|BIT29);//FGPIO_13=00
    gpio->Fgpcfg[0]|=BIT29;//FGPIO_13=10 ->SenOutClk

    gpio->Fgpcfg[0]&=~( (BIT14|BIT30)|(BIT15|BIT31) );//pin14=00 pin15=00
    gpio->Fgpcfg[0]|=BIT30|BIT31;//FGPIO_14=10 :SenInClk  FGPIO_15=10 :SenPix[0]

    gpio->Fgpcfg[1]&=~( (BIT0|BIT16)|(BIT1|BIT17)|(BIT2|BIT18)|(BIT3|BIT19)|(BIT4|BIT20) );
    gpio->Fgpcfg[1]|=BIT16|BIT17|BIT18|BIT19|BIT20;//FGPIO[16:20]=10 :SenPix[1:5]

    gpio->Fgpcfg[1]&=~( (BIT9|BIT25)|(BIT10|BIT26)|(BIT13|BIT29)|(BIT15|BIT31) );
    gpio->Fgpcfg[1]|=BIT25|BIT26|BIT29|BIT31; //FGPIO 25 26 29 31 :SenPix[6:7] SenHsRef SenVsRef

	//-Set SenPix[0:7]  input mode
    gpio->Fgpdat[0]&=~(BIT31);
    gpio->Fgpdat[1]&=~( (BIT16)|(BIT17)|(BIT18)|(BIT19)|(BIT20) );
    gpio->Fgpdat[1]&=~( (BIT25)|(BIT26)|(BIT29)|(BIT31) );

#if 0//(USE_SYSTEM_I2C_FUNCTION == 0)
    //set gpio pin sclk sdata  //use i2c must set
    gpio->Vgpcfg1 |= BIT7|BIT23; //i2C CLK function3
    gpio->Vgpcfg1 |= BIT8|BIT24; //i2C SDA function3
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    regClockPtr->PinMuxCfg &= ~(BIT3 | BIT4);
    regClockPtr->PinMuxCfg |= BIT4;
#endif

}

static void Local_HW_MCLK_Set(void)
{
    CLOCK *clock = (CLOCK *)CLOCK_BASE;

	//Clkss2[11:7]:
	/*
	SenTo clock select.
	00000 – PLL1
	00001 – PLL1/2
	00010 – Crystal
	00011 – PLL1/4
	00100 – PLL1/6
	00101 – PLL1/8
	*/
    clock->Clkss2 &=~(BIT7|BIT8|BIT9|BIT10|BIT11);//bit7-11 mclk
#if 1
	clock->Clkss2 |= BIT8;  
#else
    //clock->Clkss2 |=BIT8|BIT9|BIT10;
    //clock->Clkss2 |=BIT7|BIT8; //24mhz
    //clock->Clkss2 |=BIT7|BIT9;//12
    //clock->Clkss2 |=BIT9;//16mhz pll1/6
    //clock->Clkss2 |=BIT8|BIT9;//6
    //clock->Clkss2 |=BIT9|BIT10; //20mhz pll2/6
#if (PANEL_ID == PANEL_NONE)
    Clock_PllFreqSet(CLOCK_PLLIDU_INDEX, 144000000);
#endif
    int SensorFrameRate=24;
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
#endif
}



#if 0//(USE_SYSTEM_I2C_FUNCTION == 0)
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

	if (0)//(SensorWindow_setFlag == 1)
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

    MP_DEBUG2("#### WindowW =%d, WindowH=%d",WindowW,WindowH);
    MP_DEBUG2("#### hsub_ratio =%d, vsub_ratio =%d", hsub_ratio, vsub_ratio);
    MP_DEBUG3("#### width_ratio =%d, SensorW =%d,PanelW=%d",width_ratio, SensorW, PanelW);
    MP_DEBUG3("#### height_ratio=%d, SensorH=%d,PanelH=%d",height_ratio, SensorH,PanelH);
    MP_DEBUG3("%s:scale ratio width_ratio=0x%x, height_ratio=0x%x", __FUNCTION__, width_ratio, height_ratio);

    ipu->Ipu_reg_21 = (vsub_ratio<<8) | hsub_ratio;
    ipu->Ipu_reg_22 = width_ratio  << 16;
    ipu->Ipu_reg_23 = height_ratio << 16;

}


static void Set_Ipw1(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pWin=Idu_GetNextWin();
	SDWORD width_ratio  = 0x8000, height_ratio = 0x8000;
	SDWORD  vsub_ratio = 0,hsub_ratio = 0;
	DWORD SensorW,SensorH,dwDisplayW,dwDisplayH,DisplayWinStartAddr=0;

	MP_DEBUG("_%s_", __FUNCTION__);

	SensorW = SensorInputWidth;
	SensorH = SensorInputHeight;

#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
	if(SensorWindow_setFlag == 1)
	{
		dwDisplayW = SensorWindow_Width;
		dwDisplayH = SensorWindow_Height;
		DisplayWinStartAddr=(pWin->wWidth * SensorWindow_PosY  + SensorWindow_PosX)<<1;
	}
	else
	{
		dwDisplayW = pWin->wWidth;
		dwDisplayH = pWin->wHeight;
	}

#else

#if SENSOR_WIN_NUM
	if (pbSensorWinBuffer)
		pWin=&SensorInWin[0];
#endif
#if (SENSOR_WIN_NUM==2)
		dwDisplayW = pWin->wWidth;
		dwDisplayH = pWin->wHeight;
#else
	dwDisplayW = pWin->wWidth;
	dwDisplayH = pWin->wHeight>>1;
#endif
#endif

    if(SensorW <= dwDisplayW) //width scale up 
    {
      width_ratio  =(SensorW<<15) / dwDisplayW;
      hsub_ratio = 0;  
      MP_DEBUG("SensorInputWidth = %d,dwDisplayW =%d",SensorW,dwDisplayW );
      MP_DEBUG("####### %s: width_ratio = 0x%x", __FUNCTION__, width_ratio);

    }
    else
    {  /*Width scale down*/
		width_ratio  =(SensorW<<15) / dwDisplayW;
		hsub_ratio = (width_ratio >> 15);

		width_ratio = (width_ratio - (hsub_ratio<<15) ) ;
		width_ratio |= BIT15;
		hsub_ratio--;
      MP_DEBUG2("$$$$$$ width_ratio = %p, hsub_ratio =%p", width_ratio, hsub_ratio);
    }

    if(hsub_ratio > 7)
      hsub_ratio = 7;
    if(hsub_ratio < 0)
      hsub_ratio = 0;

    if(width_ratio > 0x8000)
      width_ratio = 0x8000;
    if(width_ratio < 0)
      width_ratio = 0x0001;

   /*==================================*/

    if(SensorH <= dwDisplayH) /*Height scale up*/
    {
      height_ratio =(SensorH<<15)/ dwDisplayH;
      vsub_ratio = 0;
    }
    else
    {  /*Height scale down*/
#if 0
		if ((dwDisplayH * 8) <= SensorH)
			vsub_ratio= 7 ;
		else if((dwDisplayH * 4) <= SensorH)
			vsub_ratio= 3 ;
		else if ((dwDisplayH * 2) <= SensorH)
			vsub_ratio= 1 ;
		else
			vsub_ratio= 0 ;
		height_ratio = SensorH/(vsub_ratio+1);
	if (height_ratio%dwDisplayH == 0)
		height_ratio = 0x8000 ;
	else
		height_ratio = (((height_ratio-1) << 15) / (dwDisplayH-1)) ;
#else

       height_ratio  =(SensorH<<15) / dwDisplayH;
       vsub_ratio = (height_ratio >> 15);

       height_ratio = (height_ratio - (vsub_ratio<<15) ) ;
       height_ratio |= BIT15;
		vsub_ratio--;

		//height_ratio = height_ratio>> 1;
		//height_ratio -= 800;

#endif
       MP_DEBUG2("$$$$$$ height_ratio = %p, vsub_ratio =%p", height_ratio, vsub_ratio);
    }

    
    if(vsub_ratio > 7)
      vsub_ratio = 7;
    if(vsub_ratio <= 0)
      vsub_ratio = 0;

    if(height_ratio > 0x8000)
      height_ratio = 0x8000;
    if(height_ratio <0x0001)
      height_ratio = 0x0001;

    mpDebugPrint("#### hsub_ratio =%p, vsub_ratio =%p", hsub_ratio, vsub_ratio);
    mpDebugPrint("#### width_ratio =%p, SensorW =%d,dwDisplayW=%d",width_ratio, SensorW, dwDisplayW);
    mpDebugPrint("#### height_ratio=%p, SensorH=%d,dwDisplayH=%d",height_ratio, SensorH,dwDisplayH);

    ipu->Ipu_reg_21 = (vsub_ratio<<8) | hsub_ratio;
    ipu->Ipu_reg_22 = width_ratio  << 16;
    ipu->Ipu_reg_23 = height_ratio << 16;
 // HW_SensorMainScale_Set();
#if 0//(USE_IPW1_DISPLAY_MOTHOD == ENABLE)//打开后IPW2显示到屏会有问题
	ipu->Ipu_reg_28 = BIT24;//Enable the windowing after SCL
#endif
	ipu->Ipu_reg_29=dwDisplayH - 1;
	ipu->Ipu_reg_2A=dwDisplayW - 1;    

	//display path for preview mode
	ipu->Ipu_reg_18 = 0x00070707 ;  
	ipu->Ipu_reg_19 = 0x07020000 ;
#if 0//(USE_IPW1_DISPLAY_MOTHOD != ENABLE)
	ipu->Ipu_reg_20 |= BIT1; //use SCL must set  unnormal  关闭后IPW2显示会有绿竖条
#endif
#if 0
    ipu->Ipu_reg_20 |= BIT0; 

    ipu->Ipu_reg_26 &= ~(BIT25|BIT24);
    ipu->Ipu_reg_26 |= BIT25|BIT24;
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
	ipu->Ipu_reg_A3 = ((DWORD) pWin->pdwStart| 0xA0000000)+DisplayWinStartAddr;   



    #if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
	ipu->Ipu_reg_12 = 0x01000000  | BIT16; //BIT16:0->422 1->444
    #else
	ipu->Ipu_reg_12 = 0x01000000 ; // 1:IPW out to DRAM
    #endif
	ipu->Ipu_reg_14 = 0x01000000 ;  //bit 24 :1->Enable the windowing before Overlay
	ipu->Ipu_reg_15 = dwDisplayH -1;
	ipu->Ipu_reg_16 = dwDisplayW-1;

    //IPW DMA OFFSET (in byte unit)
	ipu->Ipu_reg_1B = pWin->dwOffset-dwDisplayW*2;

}

static void Set_Ipw2(void)
{
	IPU *ipu = (IPU *) IPU_BASE;
	ST_IMGWIN *pWin = (ST_IMGWIN *)Idu_GetCurrWin(),*pSensorWin=(ST_IMGWIN *)&SensorInWin[0];
	WORD DisplayWin_W,DisplayWin_H;
	WORD SrcWidth,SrcHeight;
	DWORD DisplayWinStartAddr=0;
    SDWORD width_ratio=0,height_ratio=0;
	MP_DEBUG("_%s_", __FUNCTION__);

	ipu->Ipu_reg_F5 = 0x01000000 ;  //Enable bit 24, window enable

#if(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
#if SENSOR_WIN_NUM
	if (pbSensorWinBuffer)
		pWin=&SensorInWin[0];
#endif
	SrcWidth=pWin->wWidth;
	SrcHeight=pWin->wHeight;

#if (SENSOR_WIN_NUM==2)
		DisplayWin_W = pSensorWin->wWidth;
		DisplayWin_H = pSensorWin->wHeight;
#else
	DisplayWin_W = pSensorWin->wWidth;
	DisplayWin_H = pSensorWin->wHeight>>1;
#endif
#elif ((TCON_ID==TCON_NONE) || (SENSOR_WITH_DISPLAY != ENABLE))
    SrcWidth=DisplayWin_W = SensorInputWidth;
    SrcHeight=DisplayWin_H = SensorInputHeight;
#else
	SrcWidth=pSensorWin->wWidth;
	SrcHeight=pSensorWin->wHeight;
	if(SensorWindow_setFlag == 1)
	{
		DisplayWin_W = SensorWindow_Width;
		DisplayWin_H = SensorWindow_Height;
		DisplayWinStartAddr=pWin->dwOffset* SensorWindow_PosY  + (SensorWindow_PosX <<1);
	}
	else
	{
		DisplayWin_W = pWin->wWidth;
		DisplayWin_H = pWin->wHeight;
	}
#endif

	if(DisplayWin_W > SrcWidth)
		DisplayWin_W = SrcWidth;
	if(DisplayWin_H > SrcHeight)
		DisplayWin_H = SrcHeight;


//scaling scaling V=input_width/output_width-1
	width_ratio=(SrcWidth<<10)/DisplayWin_W-(1<<10);
	height_ratio=(SrcHeight<<10)/DisplayWin_H-(1<<10);

    //width_ratio-=4; //why ?
    //height_ratio-=4;//why ?
    if(width_ratio>0xC00)
        width_ratio=0xC00;

    if(width_ratio < 0)
        width_ratio = 0;
    
    if(height_ratio>0xC00)
        height_ratio=0xC00;

    if(height_ratio < 0)
        height_ratio = 0;

    MP_DEBUG("#### width_ratio =%p, SrcWidth =%d,DisplayWin_W=%d",width_ratio, SrcWidth, DisplayWin_W);
    MP_DEBUG("#### height_ratio=%p, SrcHeight=%d,DisplayWin_H=%d",height_ratio, SrcHeight, DisplayWin_H);
    //MP_ALERT("%s:scale ratio width_ratio=%d, height_ratio=%d", __FUNCTION__, width_ratio, height_ratio);

    ipu->Ipu_reg_F4 = (1<<31)+(height_ratio<<16)+width_ratio ;

	ipu->Ipu_reg_F6 = DisplayWin_H-1 ;
	ipu->Ipu_reg_F7 = DisplayWin_W-1 ;    
	ipu->Ipu_reg_F1 = (SrcWidth - DisplayWin_W)<<1;

	ipu->Ipu_reg_F2 = ((DWORD) pWin->pdwStart| 0xA0000000)+DisplayWinStartAddr;   
}

static void Local_HW_StopIPW1(void)
{
    IPU *ipu = (IPU *) IPU_BASE;
    ipu->Ipu_reg_F0 |= BIT6; //close SenInEn
    while(1)
    {
        if(ipu->Ipu_reg_F0&BIT30)//確定已經完整一個frame寫進去
            break;
    }
}

static void Local_HW_StopIPW2(void)
{
    IPU *ipu = (IPU *) IPU_BASE;
    ipu->Ipu_reg_F0 |= BIT7; //close SenInEn
    while(1)
    {
        if(ipu->Ipu_reg_F0&BIT31)//確定已經完整一個frame寫進去
            break;
    }
}

void Local_Sensor_GPIO_Reset(void)
{
#if (PRODUCT_PCBA==PCBA_MAIN_BOARD_V11||PRODUCT_PCBA==PCBA_MAIN_BOARD_V12||PRODUCT_PCBA==PCBA_MAIN_BOARD_V20)
	WORD gpioNum;

	if (Sensor_CurChannel_Get())
		gpioNum=(FGPIO | GPIO_45);
	else
		gpioNum=(FGPIO | GPIO_30);
	SetGPIOValue(gpioNum, 0);
	IODelay(100);
	SetGPIOValue(gpioNum, 1);

#else
	SetGPIOValue(SENSOR_GPIO_RESET, 0);
	IODelay(100);
	SetGPIOValue(SENSOR_GPIO_RESET, 1);
	//Local_HW_GPIO_Pin_ssen_0();
	//IODelay(100);
#endif
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


#if 0//(USE_SYSTEM_I2C_FUNCTION==0)
    Local_HW_I2C_CLK_Set();

    BIU *regBiuPtr = (BIU *) BIU_BASE;
    regBiuPtr->BiuArst |= ARST_I2CM;
#endif


    Local_HW_MCLK_Set();
    Sensor_Pin_Set();
    //Local_Sensor_GPIO_Reset();

//Sensor input type: 4={(Yg8)(Cb8)}{(Yg8)(Cr8)}   6={(Yg8)(Yg8)}{(Cb8)(Cr8)}
    ipu->Ipu_reg_100 &= ~(BIT16|BIT17|BIT18|BIT19);
	//if(sensor_Input_Format==Format_YYUV)
	//    ipu->Ipu_reg_100 |= BIT18|BIT17;
	// Format_YUYV
    ipu->Ipu_reg_100 |= BIT18;

    ipu->Ipu_reg_100 |= BIT21;//SenDimMeaEn

    ipu->Ipu_reg_100 &= ~BIT22;    //PixGenEn virtual sensor disable
#if VirtualSensorEnable
    //set virtual sensor
        ipu->Ipu_reg_103=(SensorInputHeight<<16)+(SensorInputWidth);
    //mpDebugPrint("fffffffffffffffffffffff=0x%X",(600<<16)+(800));
    ipu->Ipu_reg_100 |= BIT22; //PixGenEn, virtual sensor Enable
#endif


//Hsync, Vsync pos
    ipu->Ipu_reg_100 |= BIT8;//HsRefPos: Hotizontal sync reference point: 0=falling edge; 1=rising edge
    //ipu->Ipu_reg_100 &= ~BIT8;//Vsync pos=0
    //ipu->Ipu_reg_100 |= BIT9;//Hsync pos=1
    ipu->Ipu_reg_100 &= ~BIT9;//VsRefPos :Vertical sync reference point: 0=falling edge; 1=rising edge;

    clock->MdClken |= BIT10;//enable mclk
    //**********************
    //SenInCKS(PCLK,pixel clock form sensor)
    /* Clkss2 bit 15-18 sensor in clock select
	SenIn clock select.
	SenInCK[0]: 0=normal; 1= Add delay;
	SenInCK[1]: 0=normal; 1= invert;
	SenInCK[3:2]: 0=From sensor;
	1=From the pad of clock to sensor;
	2,3=From internal SenToClk.
    */
    clock->Clkss2 &=~(BIT15|BIT16|BIT17|BIT18);
#if VirtualSensorEnable
    clock->Clkss2 |=BIT17|BIT18;
#endif
    clock->MdClken |= 0x00000c00 ;  //enable clock to sensor and clock for sensor_in

	// 设置摄像头那边的数据情况
	//SenIpr Window Position The delay line number of vertical sync signal to valid window|The delay clock number of horizontal sync signal to window
    ipu->Ipu_reg_101=0; 
    //SenIpr Window Dimension   Vertical valid window height by line number. (=VWinHeight+1)|Horizontal valid window width by pixel number.(=HWinWidth+1)
    ipu->Ipu_reg_102 = ((SensorInputHeight-1)<<16)+(SensorInputWidth-1) ;
    ipu->Ipu_reg_108 = 0x00070007 ; //SenIpr Frame Interrupt Timing

    Set_Ipw2();
	Set_Ipw1();

//IPU data flow Configuration
//BIT29-IpwIntSel1:0:IntStatus=Ipw2FrameStart.1: =IpwFrameEnd
//Overlay=0;//bit4~5  0=Just before IPW;1=Just before SCL;2=Just before IPW2; 3=Off.
//Iprfrom=BIT3;//IprFrom:0=From DMA(DmaIpr); 1= From Sensor(SenIpr) 
    DWORD Ssclfrom=2; //bit0~2 SsclFrom:SCL;2=Just before Overlay;3=just after SenIpr
#if 0//(USE_IPW1_DISPLAY_MOTHOD == ENABLE)
//	Ssclfrom=0; //bit0~2 add SCL  3
  Ssclfrom=BIT1 | BIT0; //bit0~2 add SCL
#else
	Ssclfrom=2; //bit0~2 add SCL
#endif
	ipu->Ipu_reg_F0 = BIT29|BIT3|Ssclfrom;

#if 0
    ipu->Ipu_reg_3E = 0x00010100; //SP->CS
    ipu->Ipu_reg_3C=0;//0xC0C00000;
#else
	//		Ipu_SetHueSaturation(6,6);
#endif

    clock->MdClken |= BIT5;
    dmaR->Control = 0x00000001;
    dmaW->Control = 0x00000001;

    SystemIntEna(IM_IPU);
    isr->MiMask |= 0x00000002 ;

    //Enable_IPW2_FrameEnd_Int();
    //ipu->Ipu_reg_1C |= 0x00004000 ;   //ipw2 interrrupt
#if !IPW1_DISABLE
    ipu->Ipu_reg_1C |= BIT15;//ipw1(Bit 15)  ipw2(Bit 14)  interrrupt enable
#endif
#if !IPW2_DISABLE
    ipu->Ipu_reg_1C |= BIT14;//ipw1(Bit 15)  ipw2(Bit 14)  interrrupt enable
#endif
    //Enable_IPW1_FrameEnd_Int();
    //ipu->Ipu_reg_1C |= 0x00008000 ;   //Bit 14 for IPW2 frame end, Bit 15 for IPW1 frame end
    //MP_DEBUG("#### Trigger ####");
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
//DMA *dma = (DMA *)DMA_BASE;
/*
//dma->FDMACTL_EXT1 &= ~BIT24;
dma->FDMACTL_EXT1 &= ~(BIT16|BIT17|BIT18|BIT19|BIT20);
dma->FDMACTL_EXT1 |= BIT24;
dma->FDMACTL_EXT1 |= BIT17;
*/

//dma->FDMACTL_EXT0 &= ~(0xFF);
//dma->FDMACTL_EXT0 |= 0x02;
}



#if 0//CAMCODERRECORD_OLDMODE
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

//if(sensor_Input_Format==Format_YYUV)
//    ipu->Ipu_reg_100 |= BIT18|BIT17;
// Format_YUYV
    ipu->Ipu_reg_100 |= BIT18;

    //SenDimMeaEn
    ipu->Ipu_reg_100 |= 0x00600000;

    ipu->Ipu_reg_100 &= ~0x00400000;    //PixGenEn virtual sensor disable
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
    clock->MdClken |= 0x00000c00 ;  //enable clock to sensor and clock for sensor_in

    Local_HW_InputWindow1_Set();

    ipu->Ipu_reg_101=0;

    ipu->Ipu_reg_108 = 0x00070007 ;

    Set_Ipw2_OutputSize();

    Set_Ipw2_Ratio();

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

int Sensor_Run_capture(void)
{
    IPU *ipu = (IPU *) IPU_BASE;
    ipu_reset();

    Local_HW_Set_capture();
        
}

#endif




//set recording win
int Init_Sensor_Record_Win(void)
{
	ST_IMGWIN *pDstWin=Idu_GetCurrWin();
	WORD i,wWidth,wHeight;


#if SENSOR_WIN_NUM

	if (pbSensorWinBuffer)
	{
		ext_mem_free(pbSensorWinBuffer);
		pbSensorWinBuffer=NULL;
	}
	wWidth=pDstWin->wWidth;
	wHeight=pDstWin->wHeight;
	pbSensorWinBuffer = (BYTE*)ext_mem_malloc( wWidth*2*wHeight* SENSOR_WIN_NUM);
	if (pbSensorWinBuffer==NULL)
	{
		MP_ALERT("--E-- %s: pbSensorWinBuffer NULL", __FUNCTION__);
        __asm("break 100");
	}
	for (i=0;i<SENSOR_WIN_NUM;i++)
		mpWinInit(&SensorInWin[i],(DWORD *)pbSensorWinBuffer+wWidth/2*wHeight*i,wHeight,wWidth);
#endif
}


int Sensor_Start(void)
{
    IPU *ipu = (IPU *) IPU_BASE;

	ipu->Ipu_reg_F0 |= BIT6|BIT7;//close ipw1+ipw2

#if !IPW1_DISABLE
	ipu->Ipu_reg_F0 &= ~BIT6;//open ipw1
#endif
#if !IPW2_DISABLE
	ipu->Ipu_reg_F0 &= ~BIT7;//open ipw2 
#endif

    Sensor_Input_Enable();

}

//=================== Sensor =============================


int Sensor_Setup()
{
        IPU *ipu = (IPU *) IPU_BASE;
        ipu_reset();

        //先設mpx hw
        Local_HW_Set();     //在設sensor

		//给sensor下参数
        Drive_Sensor();

        //SensorWait(2000);
        //打開hw讓sensor流進來(要先確定sensor輸出已經穩定)
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

int Global_Sensor_Initial_NT99140(void)
{
    //extern BYTE Sensor_Mode;
    //IPU *ipu = (IPU *) IPU_BASE;
    //INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    //DWORD SensorEvent;
    //ST_IMGWIN *win = Idu_GetNextWin();
    //DWORD t = 0, val = 0 ;

    //Sensor_Mode=1;
    if(sensor_mode==MODE_RECORD)
	{
		if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240)
		{
			SensorInputWidth=320;
			SensorInputHeight=240;
		}else if(sensor_IMGSIZE_ID==SIZE_CIF_352x240)
		{
		    SensorInputWidth=352;
		    SensorInputHeight=240;
		}else   if(sensor_IMGSIZE_ID==SIZE_VGA_640x480)
		{
		    SensorInputWidth=640;
		    SensorInputHeight=480;
		}else if(sensor_IMGSIZE_ID==SIZE_480P_720x480)
		{
		    SensorInputWidth=800;
		    SensorInputHeight=480;
		}else if(sensor_IMGSIZE_ID==SIZE_SVGA_800x480)
		{
		    SensorInputWidth=800;
		    SensorInputHeight=480;
		}else if(sensor_IMGSIZE_ID==SIZE_SVGA_800x600)
		{
		    SensorInputWidth=800;
		    SensorInputHeight=600;
		}else if(sensor_IMGSIZE_ID==SIZE_XGA_1024x768)
		{
		    SensorInputWidth=1024;
		    SensorInputHeight=768;
		}else if(sensor_IMGSIZE_ID == SIZE_720P_1280x720)
		{
		    SensorInputWidth=1280;
		    SensorInputHeight=720;
		}else
		{
			MP_ALERT("--E-- %s: MODE_RECORD", __FUNCTION__);
			MP_ALERT("--E-- %s: Not support resolution (sensor_IMGSIZE_ID =%d) ", __FUNCTION__, sensor_IMGSIZE_ID);
			return FAIL;
		}


	}
	else if(sensor_mode==MODE_CAPTURE)
	{
		//VGA preview
		SensorInputWidth  = 1280;
		SensorInputHeight = 720;
	}
	else if(sensor_mode==MODE_PCCAM)
	{
		if(sensor_IMGSIZE_ID==SIZE_720P_1280x720){
			SensorInputWidth=1280;
			SensorInputHeight=720;
		}
		else if(sensor_IMGSIZE_ID==SIZE_VGA_640x480){
			SensorInputWidth=640;
			SensorInputHeight=480;
		}
		else if(sensor_IMGSIZE_ID==SIZE_QVGA_320x240){
			SensorInputWidth=320;
			SensorInputHeight=240;
		}
		else if(sensor_IMGSIZE_ID == SIZE_QCIF_176x144){
			SensorInputWidth=176;
			SensorInputHeight=144;
		}
		else
		{       
			MP_ALERT("--E-- %s: MODE_PCCAM", __FUNCTION__);
			MP_ALERT("--E-- %s: Not support resolution (sensor_IMGSIZE_ID =%d) ", __FUNCTION__, sensor_IMGSIZE_ID);
			return FAIL;
		}


	}
	else
	{
		MP_ALERT("--E-- %s: sensor_mode = %d", __FUNCTION__, sensor_mode);
		MP_ALERT("--E-- %s: Not support sensor inter face mode.", __FUNCTION__);
		return FAIL;
	}

	/*Need to check here. The mode and the resoultion are supported or not.*/
	/*If the resoultion is not supported, "return FAIL" here.*/

	//EventCreate(SENSOR_IPW_FRAME_END_EVENT_ID, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);  //why redo it?
	Init_Sensor_Record_Win();
	Sensor_Setup();
#if 0//((TCON_ID != TCON_NONE) && (SENSOR_WITH_DISPLAY == ENABLE))

	Idu_ChgWin(win);

#if (TCON_ID == TCON_I80Mode_128x128)
	ILI9163_WRITE_FRAME();
#endif

	/*
#if (TCON_ID == TCON_I80Mode_128x128)
	//mpDebugPrint("ILI9163");
	ILI9163_WRITE_FRAME();
#else
	//mpDebugPrint("Others");
	Idu_ChgWin(win);
#endif
	*/
#endif
	IntEnable();
	Sensor_Start();
	//printIPU_sensor();


#if 0  /*debug only*/

    //Wait event
    //EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT0, OS_EVENT_OR, &SensorEvent, 8000);//ipw1
    //EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT1, OS_EVENT_OR, &SensorEvent, 8000);//ipw2


    /*  for debug :dump data
    BYTE *SensorInJpegBuffer=(BYTE*)ext_mem_malloc(320*240*2);
    DWORD jpegsize=GetSensorJpegImage(SensorInJpegBuffer);
    Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "img", "jpg", SensorInJpegBuffer, jpegsize);
    

    //printIPU();
    CLOCK *clock = (CLOCK *)CLOCK_BASE;
    int loop ;
    for(loop = 0 ; loop < 1000 ; loop++)
    {

        IODelay(4000);


        mpDebugPrint("ipu->Ipu_reg_10A=0x%08x sensor hsync count", ipu->Ipu_reg_10A);
        mpDebugPrint("ipu->Ipu_reg_10B=0x%08x sensor vsync count", ipu->Ipu_reg_10B);
        mpDebugPrint("ipu->Ipu_reg_2B=0x%08x SCL window", ipu->Ipu_reg_2B);
        mpDebugPrint("ipu->Ipu_reg_F8=0x%08x SSCL window", ipu->Ipu_reg_F8);
        mpDebugPrint("ipu->Ipu_reg_17=0x%08x overlay window", ipu->Ipu_reg_17);
        mpDebugPrint("clock->Clkss2=0x%08x",clock->Clkss2);
        mpDebugPrint(" ");
        /*
        mpDebugPrint("ipu->Ipu_reg_108=0x%08x", ipu->Ipu_reg_108);
        mpDebugPrint("ipu->Ipu_reg_109=0x%08x", ipu->Ipu_reg_109);
        mpDebugPrint("ipu->Ipu_reg_F0=0x%08x", ipu->Ipu_reg_F0);

        mpDebugPrint("ipu->Ipu_reg_F4=0x%08x ", ipu->Ipu_reg_F4);
        mpDebugPrint("ipu->Ipu_reg_F6=0x%08x ", ipu->Ipu_reg_F6);
        mpDebugPrint("ipu->Ipu_reg_F7=0x%08x ", ipu->Ipu_reg_F7);
        mpDebugPrint("ipu->Ipu_reg_1C=0x%08x",ipu->Ipu_reg_1C);

        mpDebugPrint("clock->MdClken=0x%08x",clock->MdClken);
        mpDebugPrint("clock->Clkss1=0x%08x",clock->Clkss1);
        mpDebugPrint("clock->Clkss_EXT0=0x%08x ", clock->Clkss_EXT0);
        mpDebugPrint("clock->Clkss_EXT1=0x%08x ", clock->Clkss_EXT1);
        mpDebugPrint("clock->Clkss_EXT2=0x%08x ", clock->Clkss_EXT2);

        if(ipu->Ipu_reg_1C&BIT7)
        {mpDebugPrint("ooooooooooooooooooooooo");
            ipu->Ipu_reg_1C|=BIT7;
        }


        mpDebugPrint("ipu->Ipu_reg_15=0x%08x", ipu->Ipu_reg_15);
        mpDebugPrint("ipu->Ipu_reg_16=0x%08x", ipu->Ipu_reg_16);
        mpDebugPrint("ipu->Ipu_reg_18=0x%08x", ipu->Ipu_reg_18);
        mpDebugPrint("ipu->Ipu_reg_19=0x%08x", ipu->Ipu_reg_19);
        mpDebugPrint("ipu->Ipu_reg_20=0x%08x", ipu->Ipu_reg_20);
        mpDebugPrint("ipu->Ipu_reg_22=0x%08x", ipu->Ipu_reg_22);
        mpDebugPrint("ipu->Ipu_reg_23=0x%08x", ipu->Ipu_reg_23);
        mpDebugPrint("ipu->Ipu_reg_102=0x%08x", ipu->Ipu_reg_102);
        mpDebugPrint(" ");

        mpDebugPrint("ipu->Ipu_reg_101=0x%08x, addr=0x%08x", ipu->Ipu_reg_101, &ipu->Ipu_reg_101);
        mpDebugPrint("ipu->Ipu_reg_102=0x%08x", ipu->Ipu_reg_102);
        mpDebugPrint("ipu->Ipu_reg_103=0x%08x", ipu->Ipu_reg_103);
        mpDebugPrint("ipu->Ipu_reg_104=0x%08x", ipu->Ipu_reg_104);
        mpDebugPrint("ipu->Ipu_reg_105=0x%08x", ipu->Ipu_reg_105);
        mpDebugPrint("ipu->Ipu_reg_106=0x%08x", ipu->Ipu_reg_106);
        mpDebugPrint("ipu->Ipu_reg_107=0x%08x", ipu->Ipu_reg_107);
        mpDebugPrint("ipu->Ipu_reg_108=0x%08x", ipu->Ipu_reg_108);
        mpDebugPrint("ipu->Ipu_reg_109=0x%08x", ipu->Ipu_reg_109);


        if(ipu->Ipu_reg_109 & 0x000c0000)
        {
            mpDebugPrint("#### Int ####");
            ipu->Ipu_reg_109 |= 0x000c0000 ;
            mpDebugPrint("check ipu->Ipu_reg_109=0x%08x", ipu->Ipu_reg_109);
        }
    */
    }
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

#endif

#endif


