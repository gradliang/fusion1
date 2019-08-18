/**
* FileName     AUDIO_WFSON.c
*
* Auther       Rebecca Hsieh
* Date         2007.11.19
*
* Description  for Wolfson DAC/General Codec            
* 
*/ 
#if (AUDIO_DAC == DAC_WS8956) 

#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "audio_hal.h"

/***************************************************************
*          SET IIC GPIO 20,21                           
*                 20 : SCLK
*                 21 : SDIN
****************************************************************/
#define SET_IIC_SCLK_HIGH()		      g_psGpio->Gpdat1  |= 0x00000010//set pin20 up
#define SET_IIC_SCLK_LOW()		      g_psGpio->Gpdat1  &= 0xffffffef//set pin20 down
#define SET_IIC_SDIN_HIGH()		      g_psGpio->Gpdat1  |= 0x00000020//set pin21 up
#define SET_IIC_SDIN_LOW()		      g_psGpio->Gpdat1  &= 0xffffffdf//set pin21 down

#define SET_IIC_SDIN_OUTPUT()		  g_psGpio->Gpdat1  |= 0x00200000// GPIO 21 output
#define SET_IIC_SDIN_INPUT()		  g_psGpio->Gpdat1  &= 0xFFDFFFFF// GPIO 21 input
#define GET_IIC_ACK()                 g_psGpio->Gpdat1 & 0x00000020

#define SET_IIC_DELAY()			  __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");

void WS_IIC_Init(void);
BYTE WS8956SetAddr(WORD regaddr);

static WORD WMInitDACTab[]=
{    // D15~D9 : Address, D8~D0 : Data
    0x1e00,  //R15
    0x0e00,  //R7   << right justified I2S
    0x3018,  //R24   SPK config. DS. P.41.
    0x2ec1,  //R23   
    0x32c0,  //R25   << Power up VMID[50Kohm] and VREF>> POP
    0x3580,  //R26    <<Power up DACL & DACR>> POP
    0x35f8,  //R26    <<  Power up R/LOUT1 & R/LOUT2 leaving DACL/R set>>
    
    
    0x4500,  //R34    <<Set the LD2LO bit (Left DAC to Left Output) >>
    0x4b00,  //R37    << Set the RD2RO bit (Right DAC to Right Output) >>
       
    0x15ff,  //R10    << Set LDAC Update bit to ．1? & Volume Level to default >>
    0x17ff,  //R11   << Set RDAC Update bit to ．1? & Volume Level to default >>
    0x0500,  //R02   <<  Set LOUT1 Update bit to ．1? & Volume Level to default>>
    0x0700,  //R03   <<  Set ROUT1 Update bit to ．1? & Volume Level to default >>
    0x5100,  //R40   << Set LOUT2 Update bit to ．1? & Volume Level to default  >>
    0x5300,  //R41   <<Set ROUT2 Update bit to ．1? & Volume Level to default >>
    0x5e0c,  //R47  Enable left output mixer and right output 
     /* HP volume*/
    0x0579,  //R02   <<  Set LOUT1 Update bit to ．1? & Volume Level to default>>
    0x0779,  //R03   <<  Set ROUT1 Update bit to ．1? & Volume Level to default >>
    /*SPK volume*/
    0x5179,  //R40   << Set LOUT2 Update bit to ．1? & Volume Level to default  >>
    0x5379,  //R41   <<Set ROUT2 Update bit to ．1? & Volume Level to default >>
    0x0a00,  //R05   << Set DAC mute off>>
    0x62f7,  //R49  ClassD Enable Speaker 
    0xffff,  //end    
};

/**
 ***************************************************************
 *
 * Config WS8956 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/  

void WS_IIC_Init()
{
    g_psGpio->Gpcfg1  &= 0xffcfffcf;	//set gp20 gp21  as default function
    g_psGpio->Gpdat1  |= 0x00300000;    //set gp20 gp21 as output
    
}

void WS_IIC_SendStart(void)
{
    //SET_IIC_SDIN_OUTPUT();
    SET_IIC_DELAY();    
	SET_IIC_SDIN_HIGH();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_SDIN_LOW();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
    SET_IIC_DELAY();
}

void WS_IIC_ReStart(void)
{
    //SET_IIC_SDIN_OUTPUT();
    SET_IIC_DELAY();    
	SET_IIC_SDIN_HIGH();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SDIN_LOW();
    SET_IIC_DELAY();
    //SET_IIC_SCLK_LOW();
    //SET_IIC_DELAY();
}

void WS_IIC_SendStop(void)
{
	SET_IIC_SDIN_LOW();
    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_SDIN_HIGH();
    SET_IIC_DELAY();
}

void WS_IIC_SendNACK(void)
{
	SET_IIC_SDIN_HIGH();
    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
    SET_IIC_DELAY();
    SET_IIC_SDIN_HIGH();
    SET_IIC_DELAY();
}


void WS_IIC_SendACK(void)
{
    //SET_IIC_SDIN_OUTPUT();
    SET_IIC_DELAY();
	SET_IIC_SDIN_LOW();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
    
}

BYTE WS_IIC_GetACK(void)
{
    DWORD value;
 
    SET_IIC_SDIN_INPUT();
    SET_IIC_DELAY();

    value = ((GET_IIC_ACK())  >> 5);   // GPIO 21 input
    
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();

    SET_IIC_SCLK_LOW();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SDIN_OUTPUT();
    
   
    return value;
}

BYTE WS_IIC_SendID(BYTE bData)
{
	
    BYTE bTemp;
    bTemp = bData;

    //SET_IIC_SDIN_OUTPUT();
  
    if (bTemp & 0x80)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x40)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x20)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x10)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x08)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x04)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x02)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x01)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();

    return (WS_IIC_GetACK());
}

BYTE WS_IIC_SendByte(BYTE bData)
{
	
    BYTE bTemp;
    bTemp = bData;

    //SET_IIC_SDIN_OUTPUT();
  
    if (bTemp & 0x80)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
    
    if (bTemp & 0x40)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x20)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x10)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x08)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x04)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x02)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();
  
    if (bTemp & 0x01)
      SET_IIC_SDIN_HIGH();
    else
      SET_IIC_SDIN_LOW();

    SET_IIC_DELAY();
    SET_IIC_SCLK_HIGH();
    SET_IIC_DELAY();
    SET_IIC_DELAY();
    SET_IIC_SCLK_LOW();

    SET_IIC_DELAY();
    SET_IIC_DELAY();
    return (WS_IIC_GetACK());
}

void WS_IIC_ReleaseBus()
{ 
    SET_IIC_SCLK_HIGH();   
    SET_IIC_SDIN_HIGH();
}

void ConvertData(BYTE regaddr,WORD data,BYTE *ControlData1,BYTE *ControlData2)
{
    *ControlData1= regaddr + (BYTE)((data & 0x100)>>9);
    *ControlData2= (BYTE) (data & 0x0ff);
}

int WS8956RegWrite(BYTE regaddr, WORD data)
{
	BYTE ret,ControlData1,ControlData2;
			
	if (regaddr < 0x38)
    {
		ConvertData(regaddr, data,&ControlData1,&ControlData2);
		WS_IIC_SendStart();
		ret = WS_IIC_SendID(0x34); //0x34 is the DeviceID for WS8956 Write
		if(ret) return FAIL;

		ret = WS_IIC_SendByte(ControlData1);
		if(ret) return FAIL;

		ret = WS_IIC_SendByte(ControlData2);
		if(ret) return FAIL;                
		WS_IIC_SendStop();
		WS_IIC_ReleaseBus();
	}
    else
        return FAIL;
	
	return PASS;
}//end of WS8956RegWrite

int WS8956RegWrite2(WORD data)
{
	BYTE ret;
	WORD ControlData1,ControlData2;
			
	MP_DEBUG1("data=0x%x",data);
	ControlData1 = (data & 0xff00)>>8;      
	ControlData2 = (data & 0x00ff);   
	WS_IIC_SendStart();

	ret = WS_IIC_SendID(0x34); //0x34 is the DeviceID for WS8956 Write
	if(ret) return FAIL;

	ret = WS_IIC_SendByte((BYTE)ControlData1);
	if(ret) return FAIL;

	ret = WS_IIC_SendByte((BYTE) ControlData2);
	if(ret) return FAIL;                

	WS_IIC_SendStop();
	WS_IIC_ReleaseBus();
	
	return PASS;
}//end of WS8956RegWrite

/*******if want to use, Please check************/
WORD WS_IIC_GetLastByte()
{
	WORD bTemp = 0;

	SET_IIC_SDIN_INPUT();

	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) << 3);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) << 2);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) << 1);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) >> 0);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) >> 1);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) >> 2);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK())>> 3);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) >> 4);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();
	SET_IIC_DELAY();
	SET_IIC_DELAY();
	SET_IIC_SCLK_HIGH();

	bTemp |= ((GET_IIC_ACK()) >> 5);
	SET_IIC_DELAY();
	SET_IIC_SCLK_LOW();

	SET_IIC_DELAY();
	SET_IIC_SDIN_OUTPUT();
	WS_IIC_SendNACK();

	return bTemp;
}

int WS8956RegRead(BYTE regaddr)
{
	BYTE ret;
	int  data;

	if (regaddr < 0x38)
	{
		WS_IIC_SendStart();
		ret = WS_IIC_SendID(0x34); //0x34 is the DeviceID for WS8956 write
		if(ret) return FAIL;

		ret = WS_IIC_SendByte(regaddr);
		if(ret) return FAIL;
		ret=0x34;

		WS_IIC_ReStart();
		           
		//WS_IIC_SendStart();
		ret = WS_IIC_SendID(ret | 1); //0x35 is the DeviceID for WS8956 Read
		if(ret) return FAIL;

		data = (int) WS_IIC_GetLastByte();//read register value
		WS_IIC_SendStop();
		WS_IIC_ReleaseBus();
	}
	else
		return FAIL;            
    
    return data;

}

/**
 ***************************************************************
 *
 * Init WS8956 codec
 *
 *  Input  : none
 *
 *  Output : none
 ***************************************************************
*/

void AIUCfg_WS8956(void)
{
	g_psClock->ClkCtrl &= ~0x02000000;  //disable extPLL3
	g_psClock->ClkCtrl |= 0x01000000;	// enable PLL3
	g_psClock->PLL3Cfg = PLL3_CONFIG_FS_22K_DIV_1;//by samplerate
	g_psClock->AudClkC = 0x00001310;	//for PLL3  /2 =MCLK/4=ACLK
	g_psClock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source
	//g_psClock->Clkss_EXT1 = 0x00000008;	//for PLL3/1 and enable


	// Select External Bit_Clock Source
	// Bit_Clock from WS8956 Codec
	//g_psClock->MdClken &= 0xFFFeFFFF;	//CKE_AUDB disable,  c cke_ac97 disable
	//g_psClock->MdClken |= 0x00018000;	// 2 CKE_AC97 enable,1: CKE_AUDB enable  CKE_AUDM enable
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

	// Dissable AIU_DMA
	g_psDmaAiu->Control = 0x00000000;

	// Configure AGPIO to AUDIO Mode
	g_psGpio->Agpcfg = 0x0000001F;

	//MP612 AIU GPIO configuration
	WS_IIC_Init();
}//end of AIUCfg_WS8956

void WS896setVolume(WORD wVol)
{
    WS8956RegWrite2(0x0500 + wVol);
    WS8956RegWrite2(0x0700 + wVol);
    WS8956RegWrite2(0x5100 + wVol);
    WS8956RegWrite2(0x5300 + wVol);
}

void  WS8956CodecInit(void)
{
	BYTE i=0,ret;
	DWORD tmp;
	//MP612 AIU GPIO configuration
	WS_IIC_Init();

	g_bVolumeIndex &= 0x3f;

	while( WMInitDACTab[i] != 0xffff)
	{
		ret = WS8956RegWrite2(WMInitDACTab[i]);
		if(ret == FAIL)
			UartOutText("WriteReg Fail\r\n");
		if (i == 6)
			IODelay(2000);
		i++;
	}

	tmp = VolumeSequence[g_bVolumeIndex];
	MP_DEBUG2("tmp=0x%x,g_bVolumeIndex=0x%x", tmp, g_bVolumeIndex);

	WS8956RegWrite2(0x0500 + tmp);
	WS8956RegWrite2(0x0700 + tmp);
	WS8956RegWrite2(0x5100 + tmp);
	WS8956RegWrite2(0x5300 + tmp);

	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();
	AIU_SET_LRCLOCK_LEN(0x3f);
	//g_psAiu->AiuCtl  = 0x60800040;	//0x6080f070;	//AC97_EN , GAI_EN , FIFO_CLR
//	g_psAiu->AiuCtl1 = 0x0532003f;	//0x4512003f; //0x451a003f;	// Frame_CNT = 64  
}//end of WolfsonCodecInit2

void AudioClose_WS8956(void)
{
	WS8956RegWrite2(0x0a08);  //r5 mute
}
/**
 ***************************************************************
 *
 * Init IIS WS8956 codec and AIU module
 *
 *  Input  : none
 *
 *  Output : none 
 ***************************************************************
 */
void AudioInit_WS8956(void)
{
	register INTERRUPT *interrupt;
	
	AIUCfg_WS8956();
	
	interrupt = (INTERRUPT *)(INT_BASE);
    interrupt->MiMask |= 0x00000800;//enable AUDIO module interrupt
	
}

HAL_AUDIODAC_T _audioDAC_ws8956 =
{
	AudioInit_WS8956,
	NULL,	
	AudioClose_WS8956,
	NULL,
	NULL,
	WS896setVolume
};
#endif

