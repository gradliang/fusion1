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

#define WM8961_DEVICE_ADDRESS    0x4a	// use at mode pin value is 0
#define WM8961_DELAY()	__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
	                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
#if IICM_PIN_USING_PGPIO 
#define WM8961_SCLK_HIGH()		g_psGpio->Pgpdat  |=  0x00000001		//set pin1 up
#define WM8961_SCLK_LOW()		g_psGpio->Pgpdat  &= ~0x00000001		//set pin1 down

#define WM8961_SDIN_HIGH()		g_psGpio->Pgpdat  |=  0x00000002		//set pin2 up
#define WM8961_SDIN_LOW()		g_psGpio->Pgpdat  &= ~0x00000002		//set pin2 down
#define WM8961_SDIN_OUT()		g_psGpio->Pgpdat  |=  0x00020000		//set pin2 Out
#define WM8961_SDIN_IN()		g_psGpio->Pgpdat  &= ~0x00020000		//set pin2 In
#define WM8961_SDIN_GET()		((g_psGpio->Pgpdat & 0x02) >> 1)

#else
#define WM8961_SCLK_HIGH()		g_psGpio->Gpdat0  |=  0x00000001		//set pin1 up
#define WM8961_SCLK_LOW()		g_psGpio->Gpdat0  &= ~0x00000001		//set pin1 down

#define WM8961_SDIN_HIGH()		g_psGpio->Gpdat0  |=  0x00000002		//set pin2 up
#define WM8961_SDIN_LOW()		g_psGpio->Gpdat0  &= ~0x00000002		//set pin2 down
#define WM8961_SDIN_OUT()		g_psGpio->Gpdat0  |=  0x00020000		//set pin2 Out
#define WM8961_SDIN_IN()		g_psGpio->Gpdat0  &= ~0x00020000		//set pin2 In
#define WM8961_SDIN_GET()		((g_psGpio->Gpdat0 & 0x02) >> 1)
#endif

#define WM8961_OUT_BOTH      0	// Output audio data to Speaker and Earphone
#define WM8961_OUT_EXCLUSIVE 1	// Output audio data to Speaker or Earphone
#if (SOFT_RECORD || RECORD_AUDIO)
#define WM8961_USINGHWI2C    DISABLE
#else
#define WM8961_USINGHWI2C    ENABLE
#endif

//#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650))
//#define AUDIO_DAC_USING_PLL2 DISABLE
//#else
//#define	AUDIO_DAC_USING_PLL2 ENABLE
//#endif

//Caculation is based on CPU time 96MHz
void DAC_delay_ns(DWORD ns)
{
	DWORD i;
	ns = ns / 10;	//One instruction consume 10ns

	for(i = 0; i < ns; i++)
		__asm("nop");
}

#if(WM8961_USINGHWI2C != ENABLE)

void _pullSDIN(BYTE data)
{
	if(data)
		WM8961_SDIN_HIGH();
	else
		WM8961_SDIN_LOW();
}

BYTE _checkAck()
{
	BYTE data = 1;

	//mpDebugPrint("in1 Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);

	WM8961_SDIN_IN();
	WM8961_DELAY();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(400);

	data = WM8961_SDIN_GET();
	DAC_delay_ns(300);

	WM8961_SCLK_LOW();
	WM8961_DELAY();

	WM8961_SDIN_OUT();
	DAC_delay_ns(1500);

	if(data){
		mpDebugPrint("WM8961 got wrong ack value...and latch up");
		//while(1);
	}

	return data;
}

BYTE _checkAckinv()
{
	BYTE data = 1;

	WM8961_SDIN_IN();
	WM8961_DELAY();

	WM8961_SCLK_HIGH();
	DAC_delay_ns(400);

	data = WM8961_SDIN_GET();
	DAC_delay_ns(300);

	WM8961_SCLK_LOW();
	WM8961_DELAY();

	WM8961_SDIN_OUT();
	DAC_delay_ns(1500);

	if(!data){
		mpDebugPrint("WM8961 got wrong ack value...and latch up");
		mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
		//while(1);
	}
	else{
		mpDebugPrint("very good");
	}
	return data;
}

BYTE _readdata()
{
	BYTE data = 1;

	WM8961_SCLK_HIGH();
	DAC_delay_ns(400);

	data = WM8961_SDIN_GET();
	DAC_delay_ns(300);

	WM8961_SCLK_LOW();
	WM8961_DELAY();

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
	WM8961_SDIN_HIGH();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	WM8961_SDIN_LOW();
	DAC_delay_ns(700);
	WM8961_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (WM8961_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);


		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	WM8961_SDIN_LOW();		//Set write command bit
	WM8961_DELAY();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8961_SCLK_LOW();
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
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8961_SCLK_LOW();
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
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8961_SCLK_LOW();
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
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	// Stop stage
	WM8961_SCLK_HIGH();
	DAC_delay_ns(700);
	WM8961_SDIN_HIGH();
	WM8961_DELAY();

	return 1;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	BYTE i, tmp;

	*data = 0;	//Reset input data

	//mpDebugPrint("---- read addr: %x ----", addr);

	//Initial PIN status
	WM8961_SDIN_HIGH();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	WM8961_SDIN_LOW();
	DAC_delay_ns(700);
	WM8961_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (WM8961_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);


		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	WM8961_SDIN_LOW();		//Set write command bit
	WM8961_DELAY();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8961_SCLK_LOW();
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
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);

		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	//Initial PIN status
	WM8961_SDIN_HIGH();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(400);

	// Sr Condiction
	WM8961_SDIN_LOW();
	DAC_delay_ns(700);
	WM8961_SCLK_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (WM8961_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		WM8961_DELAY();

		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);


		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	WM8961_SDIN_HIGH();		//Set read command bit
	WM8961_DELAY();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8961_SCLK_LOW();

	// Control byte2 stage - Read ack and MSByte Data (Total 9 bits)
	WM8961_SDIN_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 8; i++)
	{	
		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();

		if(i > 0)
		{
			*data |= (tmp << (16 - i));
		}
		//mpDebugPrint("tmp(%d), %x", i, tmp);

		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

#if 0
	if(_checkAck()){}
//		return 0;
#else	// Send Ack ....
	WM8961_SDIN_OUT();
	DAC_delay_ns(1000);
	WM8961_SDIN_LOW();
	WM8961_DELAY();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8961_SCLK_LOW();
	DAC_delay_ns(1500);
#endif
	WM8961_SDIN_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 7; i++)
	{
		WM8961_SCLK_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();
		*data |= (tmp << (7 - i));

		//mpDebugPrint("tmp(%d), %x", i + 9, tmp);

		WM8961_SCLK_LOW();
		DAC_delay_ns(1500);
	}

	mpDebugPrint("	Get addor 0x%x - data: %x", addr, *data);

#if 0
	if(_checkAckinv()){}
//		return 0;
#else	//Send invert ack
	WM8961_SDIN_OUT();
	DAC_delay_ns(1000);
	WM8961_SDIN_HIGH();
	WM8961_DELAY();
	WM8961_SCLK_HIGH();
	DAC_delay_ns(300);
	WM8961_SCLK_LOW();
	DAC_delay_ns(1500);
#endif

	// Stop stage
	WM8961_SCLK_HIGH();
	DAC_delay_ns(700);
	WM8961_SDIN_HIGH();
	WM8961_DELAY();

	return 1;
}

// Set the GPIO condition for I2S communication
void WM8961_GPIO_Init()
{
	#if IICM_PIN_USING_PGPIO
	g_psGpio->Pgpcfg  |= 0x00000003;		//set gp0~1 as default function
	g_psGpio->Gpcfg0  &= 0xfffcffff;		//set gp0~1 as default function
	g_psGpio->Pgpdat  |= 0x00030000;		//Output	
	#else
	g_psGpio->Gpcfg0  &= 0xfffcfffc;		//set gp0~1 as default function
	g_psGpio->Gpdat0  |= 0x00030000;		//Output
	#endif
	//mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
}

#else  // using hardware I2C module insted of using software polling method
// 2-wire serial control mode
BYTE _setRegister2(WORD addr, WORD data)
{
	SDWORD readBack;

	I2CM_WtReg8Data16(WM8961_DEVICE_ADDRESS << 1, (BYTE)addr, data);
	readBack = I2CM_RdReg8Data16(WM8961_DEVICE_ADDRESS << 1, (BYTE)addr);

	//MP_ALERT("%s - Addr = 0x%04X, 0x%04X, 0x%04X", __FUNCTION__, (DWORD) addr, (DWORD) data, (DWORD) readBack);

	return 0;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	*data = I2CM_RdReg8Data16(WM8961_DEVICE_ADDRESS << 1, (BYTE)addr);
	return 0;
}
#endif

/*
 *    Mapping 16 level of UI volune value to hardware value
 */
int WM8961_ChgVolume(WORD vol)
{
	mpDebugPrint("WM8961 volume level: %d", vol);

	if(vol <= 6)
		vol *= 10;
	else
		vol = (vol - 6) * 3 + 60;

	if(vol > 80)
		vol = 80;	// Max volume of WM8961 speaker

	//mpDebugPrint("	volume level: %d", vol);
	//mpDebugPrint("		%x", 0x02f + vol);
	//mpDebugPrint("		%x", 0x12f + vol);
	//mpDebugPrint("		%x", 0x5140 + vol);
	//mpDebugPrint("		%x", 0x5340 + vol);

	// Set headphone output volume
	_setRegister2(0x02, 0x0af + vol);
	_setRegister2(0x03, 0x1af + vol);

	// Set speaker output volume
	_setRegister2(0x28, 0x02f + vol);  //R40
	_setRegister2(0x29, 0x12f + vol);  //R41
	// Set Mic volume
	_setRegister2(0x15, 0x02f + vol);
	_setRegister2(0x16, 0x12f + vol);
	return PASS;
}

// The crystal frequency is based on 12 HZ
void WM8961_SetDACSampleRate(DWORD sRate)
{
	register CLOCK *audio_clock;
	int data;
	audio_clock = (CLOCK *) (CLOCK_BASE);
	audio_clock->PLL3Cfg = 0x00770107;
	int PLL2CLOCK = (Clock_PllFreqGet(CLOCK_PLL2_INDEX)/1000000);
	
	if (AUDIO_DAC_USING_PLL2)
	{
		if ( (PLL2CLOCK == 96) || (PLL2CLOCK == 120) || (PLL2CLOCK == 138) || (PLL2CLOCK == 153))
			mpDebugPrint("WM8961 using PLL2 Clock: %d", PLL2CLOCK);
		else
			goto not_supported_samplerate;
	}
	else
		mpDebugPrint("WM8961 using PLL3");

	mpDebugPrint("WM8961 Samplerate: %d", sRate);
	
	if (     (sRate > 7000)  && (sRate < 11000))	// Set 8kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x05);                  // Sample rate control for ALC  -- set 8k
		//audio_clock->AudClkC = 0x000017f0;
		//AIU_SET_LRCLOCK_LEN(0x2C);
		#if !AUDIO_DAC_USING_PLL2
			audio_clock->PLL3Cfg = 0x00777C1F;
			audio_clock->AudClkC = 0x00001700;
			AIU_SET_LRCLOCK_LEN(0x2F);	
		#else 										//note: for PLL2, Master Clock must be around 3Mhz
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000012f9;//8khz
				AIU_SET_LRCLOCK_LEN(0x7c);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000129b;//8khz
				AIU_SET_LRCLOCK_LEN(0x7c);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000011ea;//7.986khz
				AIU_SET_LRCLOCK_LEN(0xbf);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000177C;
				//AIU_SET_LRCLOCK_LEN(0x2D);
				audio_clock->AudClkC = 0x000011fa;	//8.002khz	
				AIU_SET_LRCLOCK_LEN(0xc7);				
			}
			else
				goto not_supported_samplerate;
		#endif
		
	}
	else if((sRate >= 11000) && (sRate < 11300))	// Set 11.025kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x04);                  // Sample rate control for ALC  -- set 11.025/12k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->PLL3Cfg = 0x0077124d;
		audio_clock->AudClkC = 0x000016f0;
		AIU_SET_LRCLOCK_LEN(0x26);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000011f9;//11.0294khz
				AIU_SET_LRCLOCK_LEN(0x87);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000119b;//11.019khz
				AIU_SET_LRCLOCK_LEN(0x87);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000011ea;//11.031khz
				AIU_SET_LRCLOCK_LEN(0x8a);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000179B;
				//AIU_SET_LRCLOCK_LEN(0x21);
				audio_clock->AudClkC = 0x000011fA;	//10.99khz
				AIU_SET_LRCLOCK_LEN(0x90);				
			}
			else
				goto not_supported_samplerate;
		#endif
	}
	else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x04);                  // Sample rate control for ALC  -- set 11.25/12k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->PLL3Cfg = 0x0077124c;
		audio_clock->AudClkC = 0x000016f0;
		AIU_SET_LRCLOCK_LEN(0x22);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000011F9;//12khz
				AIU_SET_LRCLOCK_LEN(0x7c);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000119b;//12khz
				AIU_SET_LRCLOCK_LEN(0x7c);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000011ea;//11.979khz
				AIU_SET_LRCLOCK_LEN(0x7f);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000117C;
				//AIU_SET_LRCLOCK_LEN(0x7C);
				audio_clock->AudClkC = 0x000011fA;//11.98khz
				AIU_SET_LRCLOCK_LEN(0x84);						
			}
			else
				goto not_supported_samplerate;

		#endif
	}
	else if((sRate >= 14000) && (sRate < 20000))	// Set 16kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x03);                  // Sample rate control for ALC  -- set 16k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->PLL3Cfg = 0x00771252;
		audio_clock->AudClkC = 0x000015f0;
		AIU_SET_LRCLOCK_LEN(0x20);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010f9;//16.04khz
				AIU_SET_LRCLOCK_LEN(0xba);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//15.95khz
				AIU_SET_LRCLOCK_LEN(0xbb);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000011ea;//15.9722khz
				AIU_SET_LRCLOCK_LEN(0x5f);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000117C;
				//AIU_SET_LRCLOCK_LEN(0x5D);
				audio_clock->AudClkC = 0x000010fA;		
				AIU_SET_LRCLOCK_LEN(0xC7);						
			}
			else
				goto not_supported_samplerate;
		#endif
	}
	else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x04);                  // Sample rate control for ALC  -- set 22.05/24k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->AudClkC = 0x000013f0;
		AIU_SET_LRCLOCK_LEN(0x20);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010f9;//22.05khz
				AIU_SET_LRCLOCK_LEN(0x87);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//22.0588khz
				AIU_SET_LRCLOCK_LEN(0x87);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000010ea;//22.062khz
				AIU_SET_LRCLOCK_LEN(0x8A);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000139B;
				//AIU_SET_LRCLOCK_LEN(0x21);
				audio_clock->AudClkC = 0x000010fA;		
				AIU_SET_LRCLOCK_LEN(0x90);				
					}
			else
				goto not_supported_samplerate;
		#endif
	}
	else if((sRate >= 23000) && (sRate < 30000))	// Set 24kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x04);                  // Sample rate control for ALC  -- set 22.05/24k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->PLL3Cfg = 0x00771252;
		audio_clock->AudClkC = 0x000013f0;
		AIU_SET_LRCLOCK_LEN(0x20);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010F9;//24khz
				AIU_SET_LRCLOCK_LEN(0x7c);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//24khz
				AIU_SET_LRCLOCK_LEN(0x7c);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000010ea;//23.958khz
				AIU_SET_LRCLOCK_LEN(0x7f);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000117C;
				//AIU_SET_LRCLOCK_LEN(0x3D);
				audio_clock->AudClkC = 0x000010fA;		
				AIU_SET_LRCLOCK_LEN(0x84);						
			}
			else
				goto not_supported_samplerate;
		#endif
	}
	else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x01);                  // Sample rate control for ALC  -- set 32k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->AudClkC = 0x000011f0;
		AIU_SET_LRCLOCK_LEN(0x2c);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010f9;//32khz
				AIU_SET_LRCLOCK_LEN(0x5d);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//31.91khz
				AIU_SET_LRCLOCK_LEN(0x5d);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000010ea;//31.94khz
				AIU_SET_LRCLOCK_LEN(0x5f);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000117C;
				//AIU_SET_LRCLOCK_LEN(0x2F);
				audio_clock->AudClkC = 0x000010fA;		
				AIU_SET_LRCLOCK_LEN(0x62);						
			}
			else
				goto not_supported_samplerate;
		#endif
	}
	else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x00);                  // Sample rate control for ALC  -- set 44.1/48k
		#if !AUDIO_DAC_USING_PLL2		
		audio_clock->AudClkC = 0x000011f0;
		AIU_SET_LRCLOCK_LEN(0x21);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010f9;//44.1176khz
				AIU_SET_LRCLOCK_LEN(0x43);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//44.1176khz
				AIU_SET_LRCLOCK_LEN(0x43);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000010ea;//43.809khz
				AIU_SET_LRCLOCK_LEN(0x45);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000119B;
				//AIU_SET_LRCLOCK_LEN(0x21);
				audio_clock->AudClkC = 0x000010fA;		
				AIU_SET_LRCLOCK_LEN(0x47);		
			}
			else
				goto not_supported_samplerate;
		#endif	
	}
	else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
	{
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x00);                  // Sample rate control for ALC  -- set 44.1/48k
		#if !AUDIO_DAC_USING_PLL2
		audio_clock->PLL3Cfg = 0x00771252;
		audio_clock->AudClkC = 0x000011f0;
		AIU_SET_LRCLOCK_LEN(0x20);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010f9;//48.048khz
				AIU_SET_LRCLOCK_LEN(0x3e);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//47.619khz
				AIU_SET_LRCLOCK_LEN(0x3e);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000010ea;//48.677khz
				AIU_SET_LRCLOCK_LEN(0x3e);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000107C;
				//AIU_SET_LRCLOCK_LEN(0x3D);
				audio_clock->AudClkC = 0x000010fA;		
				AIU_SET_LRCLOCK_LEN(0x41);				
			}
			else
				goto not_supported_samplerate;
		#endif
	}
	else if((sRate >= 64000) && (sRate < 97000))	// Set 96kHz status
	{
		#if !AUDIO_DAC_USING_PLL2
		_setRegister2(0x38, 0x24);
		_setRegister2(0x1b, 0x05);	// Sample rate control for ALC  -- set 8k
		audio_clock->AudClkC = 0x000017f0;
		AIU_SET_LRCLOCK_LEN(0x04);
		#else
			if		(PLL2CLOCK == 96 )
			{
				audio_clock->AudClkC = 0x000010f9;//96.774khz
				AIU_SET_LRCLOCK_LEN(0x1e);
			}
			else if (PLL2CLOCK == 120)
			{
				audio_clock->AudClkC = 0x0000109b;//96.774khz
				AIU_SET_LRCLOCK_LEN(0x1e);
			}
			else if (PLL2CLOCK == 138)
			{
				audio_clock->AudClkC = 0x000010ea;//96.16khz
				AIU_SET_LRCLOCK_LEN(0x1f);
			}
			else if (PLL2CLOCK == 153)
			{
				//audio_clock->AudClkC = 0x0000179B;
				//AIU_SET_LRCLOCK_LEN(0x21);
				audio_clock->AudClkC = 0x00001068;	//95.86khz	
				AIU_SET_LRCLOCK_LEN(0xE3);				
			}
			else
				goto not_supported_samplerate;
		#endif
	}
	else
	{
		not_supported_samplerate:
		if (AUDIO_DAC_USING_PLL2)
		{
			MP_ALERT("WM8961 doesn't support PLL2 %d, only supports 96, 120, 138, 153.", PLL2CLOCK);
		}
		MP_ALERT("Samplerate %d is not support by IIS.(8K--96K support)", sRate);
	}

	
	_setRegister2(0x1e, 0x00);	// Auto clock configuration to WM8961

	// If you want to change PLL value, follow below steps
	// 1. setting PLL config.
	// 2. Disable PLL (ClkCtrl , Clkss_EXT1)
	// 3. Re-enable PLL(ClkCtrl , Clkss_EXT1)
#if !AUDIO_DAC_USING_PLL2	
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source

	g_psClock->ClkCtrl &= ~0x01000000;		// Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;		// enable PLL3
#else
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
#endif


	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();
}


// Setting AIU registers and reset sample rate 
int  WM8961_PlayConfig(DWORD sampleRate)
{
#if (WM8961_USINGHWI2C == ENABLE)
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x0077ffc3;
	g_psClock->AudClkC = 0x00001110;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3
	I2CM_FreqChg(WM8961_DEVICE_ADDRESS << 1, 300000);	// Set correct I2C frequency for enabing I2C setting
#else
	WM8961_GPIO_Init();
#endif
	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();

	WM8961_SetDACSampleRate(sampleRate);
	return PASS;
}

int WM8961_setPlayback()
{
	_setRegister2(0x1a, 0x000f);
	_setRegister2(0x1a, 0x01fc);
	return PASS;
}

int WM8961_uninit()
{
	//mpDebugPrint("Uninit 8961");
	_setRegister2(0x1a, 0x0);
	return PASS;
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
void WM8961_AIUCfg(void)
{
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
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

#if (WM8961_USINGHWI2C == ENABLE)
	I2CM_FreqChg(WM8961_DEVICE_ADDRESS << 1, 300000);
#else
	WM8961_GPIO_Init();
#endif
}

int WM8961_InitRecordMode()
{
	volatile DWORD data;
#if SOFT_RECORD	 // It just temporialy used #if to distinguish left and right justified, we will remove it as soon as possible 2010/05/26 XianWen note
	_setRegister2(0x07, 0x0001);
	//_setRegister2(0x04, 0xE0);//Maybe it causes audio quality is bad. Mark
#endif	
	// Enable input PGA and set input PGA volume
	_setRegister2(0x00, 0x2f);
	_setRegister2(0x01, 0x2f);
	
	_setRegister2(0x00, 0x13f);

	// Set ADC volume
	_setRegister2(0x15, 0x8f);
	_setRegister2(0x16, 0x8f);
	_setRegister2(0x16, 0x18f);

	_setRegister2(0x30, 0x1);	// Mic bias voltage control //If  playback and record at the same time , mark this register setting
//	_setRegister2(0x30, 0x0);	// Mic bias voltage control

	// For verification

	WM8961_setPlayback();
	_setRegister2(0x20, 0x00);	// L mic boost
	_setRegister2(0x21, 0x00);	// R mic boost

	_setRegister2(0x19, 0x31);

	_setRegister2(0x39, 0xf8);	// Output to DAC 
	_setRegister2(0x3a, 0xf4);	// Output to DAC
	//Note :if noise in recorded data , turn off mic bias 2010/03/29 xianwen
//	_setRegister2(0x19, 0x0fe);	// VMID selection      > Turn on mic bias
	_setRegister2(0x19, 0x0fc);	// VMID selection > Turn off mic bias


//	_setRegister2(0x48, 0x01);
//	_setRegister2(0x3c, 0x88);
//	_setRegister2(0x3c, 0xcc);
	IODelay(256);


	_setRegister2(0x06, 0x180);		// Enable high pass filter
//	_setRegister2(0x11, 0x1fb);		// ALC control	
//	_setRegister2(0x14, 0xf1);		// Noise gate

	mpDebugPrint("--- Audio record init...finish ---");
	_readRegister(0x19, &data);
	_readRegister(0x1a, &data);
	_readRegister(0x30, &data);
	return PASS;
}

int AudioInit_WM8961(void)
{
	mpDebugPrint("Audio dec init...WM8961");
	volatile DWORD data;
	WM8961_AIUCfg();

#if 0
	//Reset WM8961
	_setRegister2(0x0f, 0x0000);

	//Enable system clock
	_setRegister2(0x08, 0x01f4);


	_setRegister2(0x1c, 0x0018);


	_setRegister2(0x19, 0x00c0);
	_readRegister(0x19, &data);

	_setRegister2(0x48, 0x0001);
	DAC_delay_ns(1000000);
	DAC_delay_ns(1000000);
	DAC_delay_ns(1000000);
	DAC_delay_ns(1000000);
	DAC_delay_ns(1000000);
	DAC_delay_ns(1000000);

//	_setRegister2(0x1a, 0x000f);
	
	_setRegister2(0x45, 0x0011);


//	_setRegister2(0x33, 0x0001);

#if SOFT_RECORD
		_setRegister2(0x07, 0x0001);
#else
		_setRegister2(0x07, 0x0000);
#endif

	// Enable power for all output source
	_setRegister2(0x1a, 0x01fc);

	// Enable speaker
	_setRegister2(0x33, 0x0000);
	_setRegister2(0x31, 0x00c0);


	_setRegister2(0x1e, 0x00);	// Auto clock configuration to WM8961

	// Enable DAC soft mute
	_setRegister2(0x06,  0x0d);
	_setRegister2(0x47, 0x0000);
	_setRegister2(0x44, 0x0);

	//ClassW power saving enable
	//_setRegister2(0x52, 0x0003);

	_setRegister2(0x0e, 0x60);
	// Headphone output enable
	_setRegister2(0x45, 0x00ff);

	_setRegister2(0x05, 0x0);

	_setRegister2(0x3d, 0x0088);
	_setRegister2(0x3d, 0x00cc);

#else    // Using sequence writer
	volatile DWORD a = 0;

	//Reset WM8961
	_setRegister2(0x0f, 0x0000);
	_readRegister(0x0f, &a);
	//mpDebugPrint("0x0f: %x", a);

	_setRegister2(0x08, 0x01F4);
	_setRegister2(0x52, 0x0003);
	_setRegister2(0x57, 0x0020);
	_setRegister2(0x5A, 0x0080);

	_readRegister(0x5D, &a);

	UartOutText(".");
	while(a){
		IODelay(1000);
		_readRegister(0x5D, &a);
		//mpDebugPrint("5D: %x", a);
		//UartOutText(".");
	}	
	UartOutText(".");
// It just temporialy used #if to distinguish left and right justified, we will remove it as soon as possible 2010/05/26 XianWen note
#if SOFT_RECORD
	_setRegister2(0x07, 0x0001);
#else
	_setRegister2(0x07, 0x0000);
#endif
	_setRegister2(0x1A, 0x01FC);
	_setRegister2(0x33, 0x0004);
	_setRegister2(0x31, 0x00C0);
	_setRegister2(0x28, 0x0000);
	_setRegister2(0x29, 0x0000);
	_setRegister2(0x29, 0x0100);
	UartOutText("\r\n");
#endif
	return PASS;
}

HAL_AUDIODAC_T _audioDAC_WM8961 =
{
	AudioInit_WM8961,
	WM8961_InitRecordMode,	
	WM8961_uninit,
	WM8961_PlayConfig,
	NULL,
	WM8961_ChgVolume
};

