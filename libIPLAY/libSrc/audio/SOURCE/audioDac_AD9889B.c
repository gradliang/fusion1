/**
 * FileName     AudioDac_WM8961.c
 *
 * Auther       C.W Liu
 * Date         2009.12.17
 *
 * Description  Driver for Wolfson CODEC WM8961
 * 
 **/ 
#define LOCAL_DEBUG_ENABLE 0

/*
 * Include section 
 */
#include "global612.h"
#include "audio_hal.h"
#include "mpTrace.h"

#define AD9889B_DEVICE_ADDRESS    0x72	// use at mode pin value is 0
#define AD9889B_DELAY()	__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");

#define AD9889B_SCLK_HIGH()		g_psGpio->Gpdat0  |=  0x00000001		//set pin1 up
#define AD9889B_SCLK_LOW()		g_psGpio->Gpdat0  &= ~0x00000001		//set pin1 down

#define AD9889B_SDIN_HIGH()		g_psGpio->Gpdat0  |=  0x00000002		//set pin2 up
#define AD9889B_SDIN_LOW()		g_psGpio->Gpdat0  &= ~0x00000002		//set pin2 down
#define AD9889B_SDIN_OUT()		g_psGpio->Gpdat0  |=  0x00020000		//set pin2 Out
#define AD9889B_SDIN_IN()		g_psGpio->Gpdat0  &= ~0x00020000		//set pin2 In
#define AD9889B_SDIN_GET()		((g_psGpio->Gpdat0 & 0x02) >> 1)

#define AD9889B_OUT_BOTH      0	// Output audio data to Speaker and Earphone
#define AD9889B_OUT_EXCLUSIVE 1	// Output audio data to Speaker or Earphone

#define AD9889B_USINGHWI2C    ENABLE


//Caculation is based on CPU time 96MHz
void DAC_delay_ns(DWORD ns)
{
	DWORD i;
	ns = ns / 10;	//One instruction consume 10ns

	for(i = 0; i < ns; i++)
		__asm("nop");
}

#if 0

void _pullSDIN(BYTE data)
{
	if(data)
		AD9889B_SDIN_HIGH();
	else
		AD9889B_SDIN_LOW();
}

BYTE _checkAck()
{
	BYTE data = 1;

	//mpDebugPrint("in1 Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);

	AD9889B_SDIN_IN();
	AD9889B_DELAY();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(400);

	data = AD9889B_SDIN_GET();
	DAC_delay_ns(300);

	AD9889B_SCLK_LOW();
	AD9889B_DELAY();

	AD9889B_SDIN_OUT();
	DAC_delay_ns(1500);

	if(data){
		mpDebugPrint("WM8961 got wrong ack value...and latch up");
		while(1);
	}

	return data;
}

BYTE _checkAckinv()
{
	BYTE data = 1;

	AD9889B_SDIN_IN();
	AD9889B_DELAY();

	AD9889B_SCLK_HIGH();
	DAC_delay_ns(400);

	data = AD9889B_SDIN_GET();
	DAC_delay_ns(300);

	AD9889B_SCLK_LOW();
	AD9889B_DELAY();

	AD9889B_SDIN_OUT();
	DAC_delay_ns(1500);

	if(!data){
		mpDebugPrint("WM8961 got wrong ack value...and latch up");
		mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
		while(1);
	}
	else{
		mpDebugPrint("very good");
	}
	return data;
}

BYTE _readdata()
{
	BYTE data = 1;

	AD9889B_SCLK_HIGH();
	DAC_delay_ns(400);

	data = AD9889B_SDIN_GET();
	DAC_delay_ns(300);

	AD9889B_SCLK_LOW();
	AD9889B_DELAY();

	//if(data){
	//	mpDebugPrint("fuck WM8961 ack value...and latch up");
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
	AD9889B_SDIN_HIGH();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	AD9889B_SDIN_LOW();
	DAC_delay_ns(700);
	AD9889B_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (AD9889B_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);


		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	AD9889B_SDIN_LOW();		//Set write command bit
	AD9889B_DELAY();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(300);
	AD9889B_SCLK_LOW();
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
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);

		AD9889B_SCLK_LOW();
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
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);

		AD9889B_SCLK_LOW();
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
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);

		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	// Stop stage
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(700);
	AD9889B_SDIN_HIGH();
	AD9889B_DELAY();

	return 1;
}

BYTE _readRegister(WORD addr, DWORD *data)
{
	BYTE i, tmp;

	*data = 0;	//Reset input data

	//mpDebugPrint("---- read addr: %x ----", addr);

	//Initial PIN status
	AD9889B_SDIN_HIGH();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	AD9889B_SDIN_LOW();
	DAC_delay_ns(700);
	AD9889B_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (AD9889B_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);


		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	AD9889B_SDIN_LOW();		//Set write command bit
	AD9889B_DELAY();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(300);
	AD9889B_SCLK_LOW();
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
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);

		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	//Initial PIN status
	AD9889B_SDIN_HIGH();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(400);

	// Sr Condiction
	AD9889B_SDIN_LOW();
	DAC_delay_ns(700);
	AD9889B_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (AD9889B_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		AD9889B_DELAY();

		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);


		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	AD9889B_SDIN_HIGH();		//Set read command bit
	AD9889B_DELAY();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(300);
	AD9889B_SCLK_LOW();

	// Control byte2 stage - Read ack and MSByte Data (Total 9 bits)
	AD9889B_SDIN_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 8; i++)
	{	
		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();

		if(i > 0)
		{
			*data |= (tmp << (16 - i));
		}
		//mpDebugPrint("tmp(%d), %x", i, tmp);

		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

#if 0
	if(_checkAck()){}
//		return 0;
#else	// Send Ack ....
	AD9889B_SDIN_OUT();
	DAC_delay_ns(1000);
	AD9889B_SDIN_LOW();
	AD9889B_DELAY();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(300);
	AD9889B_SCLK_LOW();
	DAC_delay_ns(1500);
#endif
	AD9889B_SDIN_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 7; i++)
	{
		AD9889B_SCLK_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();
		*data |= (tmp << (7 - i));

		//mpDebugPrint("tmp(%d), %x", i + 9, tmp);

		AD9889B_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	mpDebugPrint("	Get addor 0x%x - data: %x", addr, *data);

#if 0
	if(_checkAckinv()){}
//		return 0;
#else	//Send invert ack
	AD9889B_SDIN_OUT();
	DAC_delay_ns(1000);
	AD9889B_SDIN_HIGH();
	AD9889B_DELAY();
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(300);
	AD9889B_SCLK_LOW();
	DAC_delay_ns(1500);
#endif

	// Stop stage
	AD9889B_SCLK_HIGH();
	DAC_delay_ns(700);
	AD9889B_SDIN_HIGH();
	AD9889B_DELAY();

	return 1;
}

// Set the GPIO condition for I2S communication
void AD9889B_GPIO_Init()
{
	g_psGpio->Gpcfg0  &= 0xfffcfffc;		//set gp0~1 as default function
	g_psGpio->Gpdat0  |= 0x00030000;		//Output
	//mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
}

#else  // using hardware I2C module insted of using software polling method
// 2-wire serial control mode
BYTE _setRegister2(WORD addr, WORD data)
{
	SDWORD readBack;

	I2CM_WtReg8Data8(AD9889B_DEVICE_ADDRESS, (BYTE)addr, data);
	readBack = I2CM_RdReg8Data8(AD9889B_DEVICE_ADDRESS, (BYTE)addr);

	//MP_ALERT("%s - Addr = 0x%04X, 0x%04X, 0x%04X", __FUNCTION__, (DWORD) addr, (DWORD) data, (DWORD) readBack);

	return 0;
}

BYTE _readRegister(WORD addr, DWORD *data)
{
	*data = I2CM_WtReg8Data8(AD9889B_DEVICE_ADDRESS, (BYTE)addr);
	return 0;
}
#endif

/*
 *    Mapping 16 level of UI volune value to hardware value
 */
void AD9889B_ChgVolume(WORD vol)
{
	mpDebugPrint("AD9889B volume level: %d", vol);

	mpDebugPrint("I2CM_RdReg8Data8(0x72,0x4) 0x%x",I2CM_RdReg8Data8(0x72,0x04));	
	mpDebugPrint("I2CM_RdReg8Data8(0x72,0x5) 0x%x",I2CM_RdReg8Data8(0x72,0x05));	
	mpDebugPrint("I2CM_RdReg8Data8(0x72,0x6) 0x%x",I2CM_RdReg8Data8(0x72,0x06));	
	
	mpDebugPrint("I2CM_RdReg8Data8(0x72,0xAF) 0x%x",I2CM_RdReg8Data8(0x72,0xAF));	
	mpDebugPrint("I2CM_RdReg8Data8(0x72,0xC6) 0x%x",I2CM_RdReg8Data8(0x72,0xC6));	



}

// The crystal frequency is based on 12 HZ
void AD9889B_SetDACSampleRate(DWORD sRate)
{
	register CLOCK *audio_clock;
	BYTE i=0x15;
	SDWORD tmp_value;
	audio_clock = (CLOCK *) (CLOCK_BASE);
	audio_clock->PLL3Cfg = 0x00770107;


	mpDebugPrint("AD9889B Samplerate: %d", sRate);

	if ( (sRate > 7000)  && (sRate < 11000))	// Set 8kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x04);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);
		I2CM_WtReg8Data8(0x72, 0x0A, 0xC0);
		I2CM_WtReg8Data8(0x72, 0x07, 0x00); 		
		I2CM_WtReg8Data8(0x72, 0x08, 0x92);
		I2CM_WtReg8Data8(0x72, 0x09, 0x58);			
		audio_clock->PLL3Cfg = 0x007795CC;
		audio_clock->AudClkC = 0x000011F0;
		AIU_SET_LRCLOCK_LEN(0x3F);	
	}
	else if((sRate >= 11000) && (sRate < 11300))	// Set 11.25kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x0C);
		I2CM_WtReg8Data8(0x72, 0x03, 0x40);
		I2CM_WtReg8Data8(0x72, 0x0A, 0xC3);
		I2CM_WtReg8Data8(0x72, 0x07, 0x01); 		
		I2CM_WtReg8Data8(0x72, 0x08, 0x42);
		I2CM_WtReg8Data8(0x72, 0x09, 0x5F);	

		audio_clock->PLL3Cfg = 0x0077635E;
		audio_clock->AudClkC = 0x00001710;
		AIU_SET_LRCLOCK_LEN(0x3F);		

	}
	else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x06);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);
		I2CM_WtReg8Data8(0x72, 0x0A, 0xC0);	
		I2CM_WtReg8Data8(0x72, 0x07, 0x00); 		
		I2CM_WtReg8Data8(0x72, 0x08, 0x92);
		I2CM_WtReg8Data8(0x72, 0x09, 0x60);		
		audio_clock->PLL3Cfg = 0x0077C7CC;
		audio_clock->AudClkC = 0x00001170;
		AIU_SET_LRCLOCK_LEN(0x3F);
		
	}
	else if((sRate >= 14000) && (sRate < 20000))	// Set 16kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x08);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);
		I2CM_WtReg8Data8(0x72, 0x0A, 0xC2);
		I2CM_WtReg8Data8(0x72, 0x07, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x08, 0x92);
		I2CM_WtReg8Data8(0x72, 0x09, 0x58);			

		audio_clock->PLL3Cfg = 0x0077635E;
		audio_clock->AudClkC = 0x00001310;
		AIU_SET_LRCLOCK_LEN(0x3F);

	}
	else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x18);
		I2CM_WtReg8Data8(0x72, 0x03, 0x80);
		I2CM_WtReg8Data8(0x72, 0x0A, 0x41);
		I2CM_WtReg8Data8(0x72, 0x07, 0x01); 
		I2CM_WtReg8Data8(0x72, 0x08, 0x42);
		I2CM_WtReg8Data8(0x72, 0x09, 0x58);	

		audio_clock->PLL3Cfg = 0x0077635E;
		audio_clock->AudClkC = 0x00001310;
		AIU_SET_LRCLOCK_LEN(0x3F);

	}
	else if((sRate >= 23000) && (sRate < 30000))	// Set 24kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x0C);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);
		I2CM_WtReg8Data8(0x72, 0x0A, 0xC3);
		I2CM_WtReg8Data8(0x72, 0x07, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x08, 0x92);
		I2CM_WtReg8Data8(0x72, 0x09, 0x58);			
		audio_clock->PLL3Cfg = 0x00777E7F;
		audio_clock->AudClkC = 0x00001130;
		AIU_SET_LRCLOCK_LEN(0x3F);
	}
	else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00);
		I2CM_WtReg8Data8(0x72, 0x02, 0x10);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);
		I2CM_WtReg8Data8(0x72, 0x0A, 0x40);
		I2CM_WtReg8Data8(0x72, 0x15, 0x30);

		audio_clock->PLL3Cfg = 0x00777E7F;
		audio_clock->AudClkC = 0x00001120;		
		//audio_clock->PLL3Cfg = 0x0077D650;
		//audio_clock->AudClkC = 0x00001000;	
		AIU_SET_LRCLOCK_LEN(0x3F);	
	}
	else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x16);
		I2CM_WtReg8Data8(0x72, 0x03, 0x0C);
		I2CM_WtReg8Data8(0x72, 0x0A, 0x40);
		I2CM_WtReg8Data8(0x72, 0x15, 0x00);
		audio_clock->PLL3Cfg = 0x00771BD1;
		audio_clock->AudClkC = 0x000011F0;		
		AIU_SET_LRCLOCK_LEN(0x3F);
	}
	else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x18);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);	
		I2CM_WtReg8Data8(0x72, 0x0A, 0x40);
		I2CM_WtReg8Data8(0x72, 0x15, 0x20);
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x000011F0;
		AIU_SET_LRCLOCK_LEN(0x3F);		
	}
	else if((sRate >= 64000) && (sRate < 97000))	// Set 96kHz status
	{
		I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
		I2CM_WtReg8Data8(0x72, 0x02, 0x30);
		I2CM_WtReg8Data8(0x72, 0x03, 0x00);	
		I2CM_WtReg8Data8(0x72, 0x0A, 0x40);
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001070;
		AIU_SET_LRCLOCK_LEN(0x3F);		
	}
	else
		MP_ALERT("Samplerate %d is not support  by IIS.(8K--96K support)", sRate);
	I2CM_WtReg8Data8(0x72, 0x50, 0x20);

	I2CM_WtReg8Data8(0x72, 0xAF, 0x16);

	I2CM_WtReg8Data8(0x72, 0x40, 0x80);

	I2CM_WtReg8Data8(0x72, 0x45, 0x80);

	I2CM_WtReg8Data8(0x72, 0x14, 0x02);

	I2CM_WtReg8Data8(0x72, 0x0c, 0x86);
	I2CM_WtReg8Data8(0x72, 0x0D, 0x18);

	//_setRegister2(0x1e, 0x00);	// Auto clock configuration to WM8961
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
void  AD9889B_PlayConfig(DWORD sampleRate)
{
#if (AD9889B_USINGHWI2C == ENABLE)
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	//g_psClock->PLL3Cfg = 0x0077ffc3;
	//g_psClock->AudClkC = 0x00001110;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3

	I2CM_FreqChg(AD9889B_DEVICE_ADDRESS<<1, 300000);	// Set correct I2C frequency for enabing I2C setting
#else
	AD9889B_GPIO_Init();
#endif

	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();

	AD9889B_SetDACSampleRate(sampleRate);
}

void AD9889B_setPlayback()
{
	//_setRegister2(0x1a, 0x000f);
	//_setRegister2(0x1a, 0x01fc);
}

void AD9889B_uninit()
{
	//mpDebugPrint("Uninit 8961");
	//_setRegister2(0x1a, 0x0);
}

/**
 ***************************************************************
 *
 * Config WM8961 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void AD9889B_AIUCfg(void)
{
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x00771BD1;
	g_psClock->AudClkC = 0x000010F0;	
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3

	
	// Disable AIU_DMA
	g_psDmaAiu->Control = 0x00000000;

	// Configure AGPIO to AUDIO Mode
	g_psGpio->Agpcfg = 0x0000001F;

#if (AD9889B_USINGHWI2C == ENABLE)
	I2CM_FreqChg(AD9889B_DEVICE_ADDRESS<<1 , 300000);
#else
	AD9889B_GPIO_Init();
#endif
}

void AD9889B_InitRecordMode()
{
}

void AudioInit_AD9889B(void)
{
	mpDebugPrint("Audio dec init...AD9889B");
	volatile DWORD data;
	AD9889B_AIUCfg();
	I2CM_WtReg8Data8(0x72, 0x01, 0x00); 
	I2CM_WtReg8Data8(0x72, 0x02, 0x16);
	I2CM_WtReg8Data8(0x72, 0x03, 0x0C);	
	//I2CM_WtReg8Data8(0x72, 0x0A, 0x41);
	I2CM_WtReg8Data8(0x72, 0x0A, 0x40);
	I2CM_WtReg8Data8(0x72, 0x0c, 0x86);
	I2CM_WtReg8Data8(0x72, 0x0D, 0x10);	
	I2CM_WtReg8Data8(0x72, 0x50, 0x20);	
	I2CM_WtReg8Data8(0x72, 0xAF, 0x16);
	I2CM_WtReg8Data8(0x72, 0x40, 0x80);
	I2CM_WtReg8Data8(0x72, 0x45, 0x80);
	I2CM_WtReg8Data8(0x72, 0x14, 0x02);
	AIU_SET_LRCLOCK_LEN(0x7F);	

	// Select External Bit_Clock Source
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();	
}

HAL_AUDIODAC_T _audioDAC_AD9889B =
{
	AudioInit_AD9889B,
	AD9889B_InitRecordMode,	
	AD9889B_uninit,
	AD9889B_PlayConfig,
	NULL,
	AD9889B_ChgVolume
};

