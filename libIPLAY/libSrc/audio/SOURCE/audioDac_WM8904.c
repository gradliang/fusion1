/**
 * FileName     AudioDac_WM8904.c
 *
 * Author       C.W Liu		/Eddyson
 * Date         2009.12.17	/2012.04.18
 *
 * Description  Driver for Wolfson CODEC WM8904
 * 
 **/ 
#define LOCAL_DEBUG_ENABLE 0

/*
 * Include section 
 */
#include "global612.h"
#include "audio_hal.h"
#include "mpTrace.h"

#define WM8904_DEVICE_ADDRESS    0x1A	// use at mode pin value is 0
#define WM8904_DELAY()	__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");

#if IICM_PIN_USING_VGPIO
#define WM8904_SCLK_HIGH()		g_psGpio->Vgpdat1  |=  0x00000080		//set pin1 up
#define WM8904_SCLK_LOW()		g_psGpio->Vgpdat1  &= ~0x00000080		//set pin1 down

#define WM8904_SDIN_HIGH()		g_psGpio->Vgpdat1  |=  0x00000002		//set pin2 up
#define WM8904_SDIN_LOW()		g_psGpio->Vgpdat1  &= ~0x00000002		//set pin2 down
#define WM8904_SDIN_OUT()		g_psGpio->Vgpdat1  |=  0x00020000		//set pin2 Out
#define WM8904_SDIN_IN()		g_psGpio->Vgpdat1  &= ~0x00020000		//set pin2 In
#define WM8904_SDIN_GET()		((g_psGpio->Vgpdat1 & 0x00000002) >> 1)

#else
#define WM8904_SCLK_HIGH()		g_psGpio->Gpdat0  |=  0x00000001		//set pin1 up
#define WM8904_SCLK_LOW()		g_psGpio->Gpdat0  &= ~0x00000001		//set pin1 down

#define WM8904_SDIN_HIGH()		g_psGpio->Gpdat0  |=  0x00000002		//set pin2 up
#define WM8904_SDIN_LOW()		g_psGpio->Gpdat0  &= ~0x00000002		//set pin2 down
#define WM8904_SDIN_OUT()		g_psGpio->Gpdat0  |=  0x00020000		//set pin2 Out
#define WM8904_SDIN_IN()		g_psGpio->Gpdat0  &= ~0x00020000		//set pin2 In
#define WM8904_SDIN_GET()		((g_psGpio->Gpdat0 & 0x02) >> 1)

#endif

#if (RECORD_AUDIO)
#define WM8904_USINGHWI2C    DISABLE
#else
#define WM8904_USINGHWI2C    ENABLE
#endif

//Caculation is based on CPU time 96MHz
void DAC_delay_ns(DWORD ns)
{
	DWORD i;
	ns = ns / 1;	//One instruction consume 10ns

	for(i = 0; i < ns; i++)
		__asm("nop");
}

BYTE GetDACGPIOValue(BYTE gpio_num)
{
	register GPIO *dacGpio;
  dacGpio = (GPIO *)(GPIO_BASE);
  DWORD gpval0 = 0 ;
  DWORD gpval1 = 0 ;
  BYTE gpio_num_shift;
  

  if(gpio_num>15)
  {
	 gpio_num_shift = gpio_num-16;
  }
  else
  {
	 gpio_num_shift = gpio_num;
  }

  gpval0 = (1<<gpio_num_shift) ;
  gpval1 = 0xffffffff & (~(1<<gpio_num_shift)) ;
  
  
  if(gpio_num<15)
  {
        if(dacGpio->Fgpdat[0] & gpval0)
			return 1;
		else if(dacGpio->Fgpdat[0] & gpval1)
			return 0;
  }
  else
  {
		if(dacGpio->Fgpdat[1] & gpval0)
			return 1;
		else if(dacGpio->Fgpdat[1] & gpval1)
			return 0;

   }
  
}

void SetDACGPIOValue(BYTE gpio_num, BYTE value)
{
	register GPIO *dacGpio;
  dacGpio = (GPIO *)(GPIO_BASE);
  DWORD gpval = 0 ;
  BYTE gpio_num_shift;
  
 // mpDebugPrint("#0# Fgpdat[0] = 0x%08x", dacGpio->Fgpdat[0]);

  if(gpio_num<15)
  {
		dacGpio->Fgpdat[0]|= (1<<(gpio_num+16));	// set output
  }
  else
  {
		dacGpio->Fgpdat[0] |= (1<<(gpio_num));	// set output
  }
	
  if(gpio_num>15)
  {
	 gpio_num_shift = gpio_num-16;
  }
  else
  {
	 gpio_num_shift = gpio_num;
  }

  if(value == 1)
  {
	if(gpio_num<15)
  	{
		dacGpio->Fgpdat[0] |= (1<<gpio_num_shift) ;
	}
	else
	{
		dacGpio->Fgpdat[0] |= (1<<gpio_num_shift) ;

	}
  }
  else if(value == 0)
  {
  	gpval = 0xffffffff & (~(1<<gpio_num_shift)) ;
	
	if(gpio_num<15)
  	{
		dacGpio->Fgpdat[0] &= gpval;
	}
	else
	{
		dacGpio->Fgpdat[0] &= gpval;

	}

  }

//	mpDebugPrint("#end# Fgpdat[0] = 0x%08x", dacGpio->Fgpdat[0]);
}

#if(WM8904_USINGHWI2C != ENABLE)

void _pullSDIN(BYTE data)
{
	if(data)
		WM8904_SDIN_HIGH();
	else
		WM8904_SDIN_LOW();
}
static DWORD gpcfg1;
static DWORD gpdat1;

// Set the GPIO condition for I2S communication
void WM8904_GPIO_Init()
{
#if 0//IICM_PIN_USING_VGPIO
		//It saved vgpio1 value because it may have other function have set it.
		//Record it and restore it after finishing to set WM8960  XianWen Chang 2010/06/24
		gpcfg1 = g_psGpio->Vgpcfg1;
		gpdat1 = g_psGpio->Vgpdat1;
		g_psGpio->Vgpcfg1 &= 0xFE7FFE7F;
		g_psGpio->Vgpdat1 |= 0x01800000;
		
#else
		gpcfg1 = g_psGpio->Gpcfg0;
		gpdat1 = g_psGpio->Gpdat0;

		g_psGpio->Gpcfg0  &= 0xfffcfffc;		//set gp0~1 as default function
		g_psGpio->Gpdat0  |= 0x00030000;		//Output
#endif

}

BYTE _checkAck()
{
	BYTE data = 1;

	//mpDebugPrint("in1 Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);

	WM8904_SDIN_IN();
	WM8904_DELAY();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(400);

	data = WM8904_SDIN_GET();
	DAC_delay_ns(300);

	WM8904_SCLK_LOW();
	WM8904_DELAY();

	WM8904_SDIN_OUT();
	DAC_delay_ns(1500);
	if(data){

		//mpDebugPrint("WM8904 got wrong ack value...and latch up");
		while(1);
	}
	return data;
}

BYTE _checkAckinv()
{
	BYTE data = 1;

	WM8904_SDIN_IN();
	WM8904_DELAY();

	WM8904_SCLK_HIGH();
	DAC_delay_ns(400);

	data = WM8904_SDIN_GET();
	DAC_delay_ns(300);

	WM8904_SCLK_LOW();
	WM8904_DELAY();

	WM8904_SDIN_OUT();
	DAC_delay_ns(1500);

	if(!data){
		//mpDebugPrint("WM8904 got wrong ack value...and latch up");
		//mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
		while(1);
	}
	else{
		//mpDebugPrint("very good");
	}
	return data;
}

BYTE _readdata()
{
	BYTE data = 1;

	WM8904_SCLK_HIGH();
	DAC_delay_ns(400);

	data = WM8904_SDIN_GET();
	DAC_delay_ns(300);

	WM8904_SCLK_LOW();
	WM8904_DELAY();

	//if(data){
	//	mpDebugPrint("fuck WM8904 ack value...and latch up");
	//	mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
	//	while(1);
	//}
	//else{
	//	mpDebugPrint("very good");
	//}
	return data;
}


// 2-wire serial control mode
BYTE _setRegister2(WORD addr, WORD data)
{
	BYTE i, tmp;

	//Initial PIN status
	WM8904_SDIN_HIGH();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	WM8904_SDIN_LOW();
	DAC_delay_ns(700);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (WM8904_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);


		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	WM8904_SDIN_LOW();		//Set write command bit
	WM8904_DELAY();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);
		

	if(_checkAck())
		return 0;

	// Control byte1 stage
	// Send address bits
	for(i = 0; i < 8; i++)
	{
		tmp = (addr >>(7 - i)) & 0x1;
		//mpDebugPrint("c1.%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;
	
	// Control byte2 stage - Drnf MSByte Data
	for(i = 0; i < 8; i++)
	{
		tmp = (data >>(15 - i)) & 0x1;
		//mpDebugPrint("c2.%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	// Control byte2 stage - Drnf LSByte Data
	for(i = 0; i < 8; i++)
	{
		tmp = (data >>(7 - i)) & 0x1;
		//mpDebugPrint("c3.%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	// Stop stage
	WM8904_SCLK_HIGH();
	DAC_delay_ns(700);
	WM8904_SDIN_HIGH();
	WM8904_DELAY();

	return 1;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	BYTE i, tmp;

	*data = 0;	//Reset input data

	//mpDebugPrint("---- read addr: %x ----", addr);

	//Initial PIN status
	WM8904_SDIN_HIGH();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	WM8904_SDIN_LOW();
	DAC_delay_ns(700);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (WM8904_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);


		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	WM8904_SDIN_LOW();		//Set write command bit
	WM8904_DELAY();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);

	if(_checkAck())
		return 0;

	// Control byte1 stage
	// Send address bits
	for(i = 0; i < 8; i++)
	{
		tmp = (addr >>(7 - i)) & 0x1;
		//mpDebugPrint("c1.%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	//Initial PIN status
	WM8904_SDIN_HIGH();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(400);

	// Sr Condiction
	WM8904_SDIN_LOW();
	DAC_delay_ns(700);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (WM8904_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8904_DELAY();

		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);


		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	WM8904_SDIN_HIGH();		//Set read command bit
	WM8904_DELAY();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8904_SCLK_LOW();

	// Control byte2 stage - Read ack and MSByte Data (Total 9 bits)
	WM8904_SDIN_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 8; i++)
	{	
		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();

		if(i > 0)
		{
			*data |= (tmp << (16 - i));
		}
		//mpDebugPrint("tmp(%d), %x", i, tmp);

		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}

#if 0
	if(_checkAck()){}
//		return 0;
#else	// Send Ack ....
	WM8904_SDIN_OUT();
	DAC_delay_ns(1000);
	WM8904_SDIN_LOW();
	WM8904_DELAY();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);
#endif
	WM8904_SDIN_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 7; i++)
	{
		WM8904_SCLK_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();
		*data |= (tmp << (7 - i));

		//mpDebugPrint("tmp(%d), %x", i + 9, tmp);

		WM8904_SCLK_LOW();
		DAC_delay_ns(1500);
	}
	DAC_delay_ns(1500);
	//mpDebugPrint("	Get addor 0x%x - data: %x", addr, *data);
	//WM8904_GPIO_Init();

#if 0
	if(_checkAckinv()){}
//		return 0;
#else	//Send invert ack
	WM8904_SDIN_OUT();
	DAC_delay_ns(1000);
	WM8904_SDIN_HIGH();
	WM8904_DELAY();
	WM8904_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8904_SCLK_LOW();
	DAC_delay_ns(1500);
#endif

	// Stop stage
	WM8904_SCLK_HIGH();
	DAC_delay_ns(700);
	WM8904_SDIN_HIGH();
	WM8904_DELAY();

	return 1;
}



#else  // using hardware I2C module insted of using software polling method
// 2-wire serial control mode
BYTE _setRegister2(WORD addr, WORD data)
{
	SDWORD readBack;

	I2CM_WtReg8Data16(WM8904_DEVICE_ADDRESS << 1, (BYTE)addr, data);
	readBack = I2CM_RdReg8Data16(WM8904_DEVICE_ADDRESS << 1, (BYTE)addr);

	//MP_ALERT("%s - Addr = 0x%04X, 0x%04X, 0x%04X", __FUNCTION__, (DWORD) addr, (DWORD) data, (DWORD) readBack);

	return 0;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	*data = I2CM_RdReg8Data16(WM8904_DEVICE_ADDRESS << 1, (BYTE)addr);
	return 0;
}
#endif

/*
 *    Mapping 16 level of UI volune value to hardware value
 */
int WM8904_ChgVolume(WORD vol)
{
	mpDebugPrint("WM8904 volume level(min=0, max=16): %d", vol);
	if (vol <= 0)
		vol = 0;
	else if (vol > 63)
		vol = 63;
	else
		vol	= vol*4;
	
	//mpDebugPrint("	volume level: %d", vol);
	//mpDebugPrint("		%x", 0x02f + vol);
	//mpDebugPrint("		%x", 0x12f + vol);
	//mpDebugPrint("		%x", 0x5140 + vol);
	//mpDebugPrint("		%x", 0x5340 + vol);

	// Set headphone output volume
	if ( vol == 0 )							
	{
		_setRegister2(0x39, 0x0);		// left Headphone volume mute
		_setRegister2(0x3A, 0x0);		// right Headphone volume mute
	}
	else
	{
		_setRegister2(0x39, vol);
		_setRegister2(0x3A, vol);
	}
	return PASS;
}

// The crystal frequency is based on 12 HZ
void WM8904_SetDACSampleRate(DWORD sRate)
{	
	MP_DEBUG("%s",__func__);
	register CLOCK *audio_clock;
	int data;
	audio_clock = (CLOCK *) (CLOCK_BASE);
	audio_clock->PLL3Cfg = 0x00770107;
	

	//mpDebugPrint("WM8904 Samplerate: %d", sRate);
	if ( (sRate > 7000)  && (sRate < 11000))	// Set 8kHz status
	{
		_setRegister2(0x15, 0x2400);
		audio_clock->PLL3Cfg = 0x00777C7F;
		audio_clock->AudClkC = 0x00001500;	
		AIU_SET_LRCLOCK_LEN(0xFF);			
	}
	else if((sRate >= 11000) && (sRate < 11300))	// Set 11.25kHz status
	{
		_setRegister2(0x15, 0x1C01);
		audio_clock->PLL3Cfg = 0x0077107F;
		audio_clock->AudClkC = 0x00001370;	
		AIU_SET_LRCLOCK_LEN(0xFF);
	}
	else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
	{
		_setRegister2(0x15, 0x1C01);
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001370;
		AIU_SET_LRCLOCK_LEN(0xFF);		
	}
	else if((sRate >= 14000) && (sRate < 22000))	// Set 16kHz status
	{
		_setRegister2(0x15, 0x1802);
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001270;	
		AIU_SET_LRCLOCK_LEN(0xFF);		
	}
	else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
	{
		_setRegister2(0x15, 0x1403);
		audio_clock->PLL3Cfg = 0x0077107F;//MCLK ==11.289Mhz
		audio_clock->AudClkC = 0x00001170;		
		AIU_SET_LRCLOCK_LEN(0xFF);
		
	}
	else if((sRate >= 22000) && (sRate < 30000))	// Set 24kHz status
	{
		_setRegister2(0x15, 0x1403);
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001170;	
		AIU_SET_LRCLOCK_LEN(0xFF);
	}
	else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
	{
		_setRegister2(0x15, 0x1004);
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001270;	
		AIU_SET_LRCLOCK_LEN(0x7F);
	}
	else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
	{
		_setRegister2(0x15, 0x0C05);
		audio_clock->PLL3Cfg = 0x0077107F;//MCLK ==11.289Mhz
		audio_clock->AudClkC = 0x00001070;	
		AIU_SET_LRCLOCK_LEN(0xFF);
	}
	else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
	{
		_setRegister2(0x15, 0x0C05);
		audio_clock->PLL3Cfg = 0x007718cc;//MCLK ==12.288Mhz
		audio_clock->AudClkC = 0x00001070;		
		AIU_SET_LRCLOCK_LEN(0xFF);
	}
	else
		MP_ALERT("Samplerate %d is not support	by IIS.(8K--48K support)", sRate);

	//_setRegister2(0x1e, 0x00);	// Auto clock configuration to WM8904

	// If you want to change PLL value, follow below steps
	// 1. setting PLL config.
	// 2. Disable PLL (ClkCtrl , Clkss_EXT1)
	// 3. Re-enable PLL(ClkCtrl , Clkss_EXT1)
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source

	g_psClock->ClkCtrl &= ~0x01000000;		// Disable PLL3 
	g_psClock->ClkCtrl |= 0x01000000;		// enable PLL3

	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();
}


// Setting AIU registers and reset sample rate 
int  WM8904_PlayConfig(DWORD sampleRate)
{
	MP_DEBUG("%s",__func__);
	// Master clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x0077ffc3;
	g_psClock->AudClkC = 0x00001110;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3
#if (WM8904_USINGHWI2C == ENABLE)	
	I2CM_FreqChg(WM8904_DEVICE_ADDRESS << 1, 300000);	// Set correct I2C frequency for enabing I2C setting
#else
	WM8904_GPIO_Init();
#endif

	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();

if (GetDACGPIOValue(11) == 0) //if is FGPIO11 is low (record mode) or high (playback mode)
{
	MP_DEBUG("GPIO_FGPIO_11 = LOW");
	_setRegister2(0x19, 0x2);  //I2S and 16bit of word length (note that in func AudioIfConfig() should be AIU_SET_08BITMODE(), i guess)	
	AIU_I2S_AUDIO_INTERFACE();MP_DEBUG("WM8904_PlayConfig using AIU_I2S_AUDIO_INTERFACE");
}
else
{
	MP_DEBUG("GPIO_FGPIO_11 = HIGH");
	_setRegister2(0x19, 0x0);  //right-just and 16bit of word length (note that in func AudioIfConfig() should be AIU_SET_08BITMODE(), i guess)	
	AIU_RIGHT_JUSTIFIED();MP_DEBUG("WM8904_PlayConfig using AIU_RIGHT_JUSTIFIED");
}

	WM8904_SetDACSampleRate(sampleRate);
	return PASS;
}

int WM8904_setPlayback()
{	
	MP_DEBUG("%s",__func__);
	//_setRegister2(0x1a, 0x000f);
	//_setRegister2(0x1a, 0x01fc);
	return PASS;
}

int WM8904_uninit()
{
	MP_DEBUG("%s",__func__);
	//_setRegister2(0x1a, 0x0);
	_setRegister2(0x6F, 0x119);
	return PASS;
}

/**
 ***************************************************************
 *
 * Config WM8904 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void WM8904_AIUCfg(void)
{	
	MP_DEBUG("%s",__func__);
	// Master clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x00771252;
	g_psClock->AudClkC = 0x000011f0;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3

	// Select External Bit_Clock Source
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

	// Disable AIU_DMA
	g_psDmaAiu->Control = 0x00000000;

	// Configure AGPIO to AUDIO Mode
	g_psGpio->Agpcfg = 0x0000001F;

#if (WM8904_USINGHWI2C == ENABLE)
	I2CM_FreqChg(WM8904_DEVICE_ADDRESS << 1, 300000);
#else
	WM8904_GPIO_Init();
#endif
}

int WM8904_InitRecordMode()
{
	mpDebugPrint("Audio Record mode init...WM8904");
	volatile DWORD a = 0;

	WM8904_AIUCfg();
	
#if 0	//set manually FGPIO11=low 
	g_psGpio->Fgpcfg[0]  &= 0xf7fff7ff;		//IO Control Change to ADC
//	g_psGpio->Fgpdat[0]  |= 0x08000000;		//Output
//	g_psGpio->Fgpdat[0]  &= 0xfffff7ff;		//Output
	SetDACGPIOValue(10,0);	//doesn't matter fgpdat10 is '1' or '0', but fgpdat11 is dependent of fgpdat10
	SetDACGPIOValue(11,0);	//The FGPIO11 direction control FGPDIR[11]  and FGPCFG[11] (or said, FGPIO_SEL) are both incorrectly connected from  FGPDIR[10] & FGPCFG[10].
							//So, the work around is to use FGPDIR[10]  & FGPCFG[10] to control FGPDIR[11] & FGPCFG[11] 
							//(but the driving output data FGPDAT[10] &  FGPDAT[11] are still individually driven.)
	MP_DEBUG("FGPIO10&FGPIO11: LOW");
#endif

#if 0	//print all registers data
mpDebugPrint("====before init rec====");
readallreg();
mpDebugPrint("====before init rec====");
#endif

if (GetDACGPIOValue(11) == 0)
{
	//ADCL ADCR signal path
#if 0
	//single-ended mode
//	_setRegister2(0x2e,0x84);
//	_setRegister2(0x2f,0x84);

	_setRegister2(0x2e,0x10);	//there's sound
	_setRegister2(0x2f,0x10);	//there's sound //mono mic, IN2L inverter,only used left side mic

//	_setRegister2(0x2e,0x4);
//	_setRegister2(0x2f,0x4);
#else
	//differential mic mode
//	_setRegister2(0x2e,0x86);
//	_setRegister2(0x2f,0x86);	//0x2e 0x48 //mono mic,only used left side mic
	_setRegister2(0x2e,0xc6);	//Commom mode enable
	_setRegister2(0x2f,0xc6);	//Commom mode enable //mono mic,only used left side mic
#endif

	//ADC & DAC control
	_setRegister2(0x21,0x0600); //0x21 0x0600			//DAC digital 1, set DAC_MUTERATE, DAC_UNMUTERATE

	//HPF
//	_setRegister2(0x26,0x10); //0x26 0x10				

	//Audio interface
	_setRegister2(0x18, 0x50);  ////0x18 0x50
	#if SOFT_RECORD
		mpDebugPrint("DAC WM8904 may not work properly at SOFT_RECORD mode !!");
		_setRegister2(0x19, 0x1);  //left-justified and 16bit of word length (note that in func AudioIfConfig() should be AIU_SET_08BITMODE(), i guess)
	#else
		#if 0
			_setRegister2(0x19, 0x0); //right-justified  and 16bit, it works, but still doesn't work to record avi
			MP_DEBUG("WM8904_InitRecordMode using AIU_RIGHT_JUSTIFIED");
		#else
			_setRegister2(0x19, 0x2);  //I2S and 16bit of word length (note that in func AudioIfConfig() should be AIU_SET_08BITMODE(), i guess)	
			//AIU_I2S_AUDIO_INTERFACE(); // it is set in another function
			MP_DEBUG("WM8904_InitRecordMode using AIU_I2S_AUDIO_INTERFACE");
		#endif
	#endif

	//Power Management
	_setRegister2(0x0C,0x03); //ADC PGA L,R Input ENABLE
	_setRegister2(0x12,0x0F); //ADC and DAC Power Manag, Output and input at the same time 
	_setRegister2(0x12,0x03); //ADC Power Manag, Output and input at the same time 
	_setRegister2(0x06,0x01); //mic bias ENABLE
	_setRegister2(0x0c, 0x03); //input PGA Power		//power enable of RIGHT and LEFT mic

	_setRegister2(0x04, 0x0001);			//Bias Control 0
	_setRegister2(0x05, 0x0043);			//VMID control 0
	_setRegister2(0x07, 0x01);				//Mic Bias Control 1

	//Dynamic Range Control (DRC)	// to amplify input sound, but the noise will be also amplified
	_readRegister(0x28,&a);a = a | (1<<15);
	_setRegister2(0x28, a);
	_readRegister(0x29,&a);a = a | (0xf);
	_setRegister2(0x29, a);
	_setRegister2(0x2a, 0x10);

	//Right input & Left input volume
	_setRegister2(0x2c,0x1f);
//	_setRegister2(0x2d,0x1f);
//	_setRegister2(0x2c,0x9f);					//mute LEFT side mic
	_setRegister2(0x2d,0x9f);				//mute RIGHT side mic

	//ADC digital L/R Input volume
	_setRegister2(0x24,0xff); //left side
	_setRegister2(0x25,0x0); //right side


#if 0 // code to verify mic sound from headphone
		_setRegister2(0x12,0x0F); //ADC and DAC Power Manag, Output and input at the same time 

#if 0//	Bypass mic sound to hearphone test
		mpDebugPrint("Bypass mic sound to headphone without ADC/DAC...");
		_setRegister2(0x3d,0x0c); //input Left/Right PGA (analog bypass)
		_setRegister2(0x0e,0x03); //hearphone PGA L,R output ENABLE
		_setRegister2(0x39,0x139); //Analog Out1 left, no sound, why??
		_setRegister2(0x3a,0x139); //Analog Out1 right,	there's only sound from leftside in rightside of hearphone, why???
#else
		//_setRegister2(0x0e,0x00); //hearphone PGA L,R output DISABLE
		//_setRegister2(0x3d,0x00); //no input Left/Right PGA (analog bypass)
		_setRegister2(0x39,0x0); //Analog Out1 left, no sound, why??
		_setRegister2(0x3a,0x0); //Analog Out1 right, there's only sound from leftside in rightside of hearphone, why???
#endif

#if 0	//sidetone
		_setRegister2(0x20,0xffa);//sidetone
		_setRegister2(0x3d,0xc);//output mux control - DAC
#endif

#if 1	//path from digital audio interface
		_setRegister2(0x20,0xff0);//no sidetone
		_setRegister2(0x1e,0xc0);//DACL_VOL
		_setRegister2(0x1f,0xc0);//DACR_VOL
		_setRegister2(0x21,0x0);//dac unmute
		_setRegister2(0x3d,0xc);//output mux control - DAC
#else
	//	_setRegister2(0x21,0x4);//dac mute	
		_setRegister2(0x1e,0x0);//DACL_VOL mute
		_setRegister2(0x1f,0x0);//DACR_VOL mute
		_setRegister2(0x21,0x4);//dac mute
		_setRegister2(0x20,0x0);

		_setRegister2(0x39, 0x100);		// left Headphone volume mute
		_setRegister2(0x3A, 0x100);		// right Headphone volume mute
#endif

#else
	_setRegister2(0x39, 0x0);		// left Headphone volume mute
	_setRegister2(0x3A, 0x0);		// right Headphone volume mute
	_setRegister2(0x1e,0x0);//DACL_VOL mute
	_setRegister2(0x1f,0x0);//DACR_VOL mute
	//_setRegister2(0x21,0x4);//dac mute
	//_setRegister2(0x12,0x0);//Power Manager, headphone power off
#endif
}
else
{
	mpDebugPrint("--E-- To Init Record Mode of WM8904, FGPIO11 must be set as HIGH!!");
}

#if 0	//print all registers data
mpDebugPrint("====after init rec====");
readallreg();
mpDebugPrint("====after init rec====");
#endif

	return PASS;

}


int AudioInit_WM8904(void)
{
	mpDebugPrint("Audio DAC WM8904 init...");
	volatile DWORD data;
	WM8904_AIUCfg();

	g_psGpio->Fgpcfg[0]  &= 0xf7fff7ff;		//set gp0~1 as default function
//	g_psGpio->Fgpdat[0]  |= 0x08000800;		//Output
#if 0	//set manually FGPIO11=high
	SetDACGPIOValue(10,1);	//doesn't matter fgpdat10 is '1' or '0', but fgpdat11 is dependent of fgpdat10
	SetDACGPIOValue(11,1);	//The FGPIO11 direction control FGPDIR[11]	and FGPCFG[11] (or said, FGPIO_SEL) are both incorrectly connected from  FGPDIR[10] & FGPCFG[10].
							//So, the work around is to use FGPDIR[10]	& FGPCFG[10] to control FGPDIR[11] & FGPCFG[11] 
							//(but the driving output data FGPDAT[10] &  FGPDAT[11] are still individually driven.)
	MP_DEBUG("FGPIO10&FGPIO11: HIGH");
#endif

	volatile DWORD a = 0;

	//Reset WM8904
	_setRegister2(0x00, 0x8904);
	//_readRegister(0x00, &a);
	//mpDebugPrint("0x0f: %x", a);

	//_setRegister2(0x04, 0x0001);
	//_setRegister2(0x6C, 0x0100);
	//_setRegister2(0x6F, 0x0100);
	//_setRegister2(0x21, 0x0000);
	//_readRegister(0x70, &a);

	/*UartOutText(".");
	while((a & 0x1)){
		IODelay(1000);
		_readRegister(0x70, &a);
		UartOutText("&");
	}*/	
	/*_setRegister2(0x16, 0x0007);	
	_setRegister2(0x04, 0x0001);
	_setRegister2(0x05, 0x0043);
	
	_setRegister2(0x0E, 0x0003);
	_setRegister2(0x5A, 0x0011);
	_setRegister2(0x5A, 0x0033);
	_setRegister2(0x43, 0x0003);
	_setRegister2(0x44, 0x0003);
	_setRegister2(0x5A, 0x0077);
	_setRegister2(0x5A, 0x00ff);
	_setRegister2(0x12, 0x000C);
	_setRegister2(0x14, 0x0000);
	_setRegister2(0x15, 0x2400);

	_setRegister2(0x19, 0x0000);
	_setRegister2(0x21, 0x0000);
	_setRegister2(0x43, 0x0003);*/

	_setRegister2(0x16, 0x0007);

	_setRegister2(0x6c, 0x0111);	//write sequencer enable

	_setRegister2(0x6D, 0x00FF);	//write sequencer 1
	_setRegister2(0x6E, 0x0000);	//write sequencer 2
	_setRegister2(0x6F, 0x0100);	//write sequencer 3
	_setRegister2(0x21, 0x0000);	//DAC digital 1


	_readRegister(0x70, &a);	//read write sequencer 4, what is it for??

	while((a & 0x1))			//what is it for??
	{
		IODelay(2000);
		_readRegister(0x70, &a);
		//mpDebugPrint("5D: %x", a);
		//UartOutText(".");
	}
#if 1
	_setRegister2(0x19, 0x0);	//digital audio interfaceformat, right-justified, 16bit
	MP_DEBUG("AudioInit_WM8904 using AIU_RIGHT_JUSTIFIED");
#else
	_setRegister2(0x19, 0x2);	//digital audio interfaceformat, I2S, 16bit
	MP_DEBUG("AudioInit_WM8904 using AIU_I2S_AUDIO_INTERFACE");
#endif

	_setRegister2(0x15, 0x2400);	//clock rates 1

	_setRegister2(0x21, 0x0004);	//DAC digital 1
	_setRegister2(0x43, 0x000f);	//DC servo 0

#if 1	//path from digital audio interface
	_setRegister2(0x20,0xff0);//no sidetone
//	_setRegister2(0x1e,0x100);	//DAC digital volume left
//	_setRegister2(0x1f,0x100);	//DAC digital volume right	
	_setRegister2(0x1e,0x10);	//DAC digital volume LEFT
	_setRegister2(0x1f,0x10);	//DAC digital volume RIGHT
	_setRegister2(0x21,0x0);//dac soft unmute
	_setRegister2(0x3d,0x0);//output mux control - DAC
	_setRegister2(0x5a,0xff);//Analog HP 0 (pop supression control)	
	_setRegister2(0x0e,0x2);//HPL_PGA_ENA
	_setRegister2(0x0f,0x2);//HPR_PGA_ENA
	
#endif
	
	return PASS;
}

void readallreg(void)
{
	mpDebugPrint("=====read all registers=====\n");
	volatile DWORD a = 0;
	volatile DWORD b = 0;
	//mpDebugPrint("a: %x", a);
	
	for (a=0;a<=157;a++)
	{
		_readRegister(a, &b);
		mpDebugPrint("0x%x: %x", a, b);
	}
	_readRegister(0xc6, &b);
	mpDebugPrint("0xC6: %x", b);
	_readRegister(0xF7, &b);
	mpDebugPrint("0xF7: %x", b);
	_readRegister(0xF8, &b);
	mpDebugPrint("0xF8: %x", b);
	mpDebugPrint("==========\n");
}

HAL_AUDIODAC_T _audioDAC_WM8904 =
{
	AudioInit_WM8904,
	WM8904_InitRecordMode,	//NULL,
	WM8904_uninit,
	WM8904_PlayConfig,
	NULL,
	WM8904_ChgVolume
};

