/**
 * FileName:    AudioDac_ALC5633Q.c
 *
 * Author:     		Jeter Chen
 * Date:     		2012.10.01
 * Updated:		2013.02.06
 * Version:		0
 * Description:	Driver for RealTek ALC5633Q
 * 
 **/ 
#define LOCAL_DEBUG_ENABLE 0

/*
 * Include section 
 */
#include "global612.h"
#include "../INCLUDE/audio_hal.h"
#include "mpTrace.h"
#include "UtilRegFile.h"
#include "../../peripheral/include/Hal_adc.h"	// to use funct. to detect battery voltage

#define ALC5633Q_DEVICE_ADDRESS    				0x1c	// 0x38 >>1 = 0x1c
//#define AUDIO_DAC_USING_PLL2	0
///////////////  register setting  ///////////////
#define Software_reset  						0x00
#define Speaker_output_control 					0x02
#define Speaker_HP_output_mixer_control 		0x03
#define Headphone_output_volume_control 		0x04
#define AUX_output_control 						0x06
#define Line1_input_volume_control 				0x08
#define Line2_input_volume_control 				0x0A
#define DAC_control 							0x0c
#define DAC_digial_volume_control 				0x0e
#define Microphone_input_control 				0x10
#define ADC_control 							0x12
#define REC_mixer_control	 					0x14
#define ADC_digital_volume_control 				0x16
#define Headphone_mixer_control 				0x18
#define AUX_mixer_control 						0x1A
#define Speaker_mixer_control 					0x1C
#define speaker_amplifier_control 				0x1e
#define Microphone_control 						0x22
#define I2S_audio_serial_data_port_control 		0x34
#define Stereo_ADC_DAC_clock_control 			0x38
#define Power_management_control1 				0x3a
#define Power_management_control2 				0x3b
#define Power_management_control3 				0x3c
#define Power_management_control4 				0x3e
#define General_purpose_control_register1 		0x40 
#define Internal_clock_control 					0x42
#define PLL_function_control 					0x44
#define Digital_beep_gen_irq_control 			0x48
#define internal_status_sticky_control 			0x4a
#define GPIO_control1 							0x4c
#define GPIO_control2 							0x4d
#define General_purpose_control_register2 		0x52
#define Depop_control1 							0x54
#define Depop_control2 							0x56
#define Jack_detect_control_register 			0x5a
#define Soft_volume_control_register1 			0x5c
#define soft_volume_control_register2 			0x5e
#define ALC_function_control1 					0x64
#define ALC_function_control2 					0x65
#define ALC_function_control3 					0x66
#define Pseudo_stereo_spatioal_effect_control 	0x68
#define Index_address 							0x6a
#define Index_data 								0x6c
#define EQ_control1 							0x6e
#define EQ_control2 							0x70
///////////////////////////////////////////////////////////

#define ALC5633Q_DELAY()	__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
		                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
		                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
		                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");

#define ALC5633Q_SCL_HIGH()		g_psGpio->Gpdat0  |=  0x00000001		//set pin37 up
#define ALC5633Q_SCL_LOW()		g_psGpio->Gpdat0  &= ~0x00000001		//set pin37 down

#define ALC5633Q_SDA_HIGH()		g_psGpio->Gpdat0  |=  0x00000002		//set pin36 up
#define ALC5633Q_SDA_LOW()		g_psGpio->Gpdat0  &= ~0x00000002		//set pin36 down
#define ALC5633Q_SDA_OUT()		g_psGpio->Gpdat0  |=  0x00020000		//set pin2 Out
#define ALC5633Q_SDA_IN()		g_psGpio->Gpdat0  &= ~0x00020000		//set pin2 In
#define ALC5633Q_SDA_GET()		((g_psGpio->Gpdat0 & 0x02) >> 1)

//#define ALC5633Q_OUT_BOTH      0	// Output audio data to Speaker and Earphone
//#define ALC5633Q_OUT_EXCLUSIVE 1	// Output audio data to Speaker or Earphone

#if (SOFT_RECORD || RECORD_AUDIO)
#define ALC5633Q_USINGHWI2C    DISABLE
#else
#define ALC5633Q_USINGHWI2C    ENABLE
#endif

#define PLL2CLOCK (Clock_PllFreqGet(CLOCK_PLL2_INDEX)/1000000)

#define I2C_RETRY_COUNT 20

//Global Variables
BOOL ALC5633Q_RecordMode;
extern BOOL Audio_playback_enable;
/////
extern char ALC5633Q_lookback_mode_flag;
//Caculation is based on CPU time 96MHz
void DAC_delay_ns(DWORD ns)
{
	DWORD i;
	ns = ns / 10;	//One instruction consume 10ns

	for(i = 0; i < ns; i++)
		__asm("nop");
}

#if(ALC5633Q_USINGHWI2C != ENABLE)

void _pullSDIN(BYTE data)
{
	if(data)
		ALC5633Q_SDA_HIGH();
	else
		ALC5633Q_SDA_LOW();
}

BYTE _checkAck()
{
	BYTE data = 1;

	//mpDebugPrint("in1 Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);

	ALC5633Q_SDA_IN();
	ALC5633Q_DELAY();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(400);

	data = ALC5633Q_SDA_GET();
	DAC_delay_ns(300);

	ALC5633Q_SCL_LOW();
	ALC5633Q_DELAY();

	ALC5633Q_SDA_OUT();
	DAC_delay_ns(1500);

	if(data){
		//mpDebugPrint("ALC5633Q got wrong ack value...and latch up");
		//while(1);
		return 1;
	}

	return 0;
}

BYTE _checkAckinv()
{
	BYTE data = 1;

	ALC5633Q_SDA_IN();
	ALC5633Q_DELAY();

	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(400);

	data = ALC5633Q_SDA_GET();
	DAC_delay_ns(300);

	ALC5633Q_SCL_LOW();
	ALC5633Q_DELAY();

	ALC5633Q_SDA_OUT();
	DAC_delay_ns(1500);

	if(!data){
		mpDebugPrint("ALC5633Q got wrong ack value...and latch up");
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

	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(400);

	data = ALC5633Q_SDA_GET();
	DAC_delay_ns(300);

	ALC5633Q_SCL_LOW();
	ALC5633Q_DELAY();

	//if(data){
	//	mpDebugPrint("fuck ALC5633Q ack value...and latch up");
	//	mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
	//	while(1);
	//}
	//else{
	//	mpDebugPrint("very good");
	//}
	return data;
}

BYTE _setRegister2_w_retry(WORD addr, WORD data)
{
	volatile DWORD tempRegisterValue;

    //MP_DEBUG("%s enter", __FUNCTION__);
	int i;
		for (i=0; i<I2C_RETRY_COUNT; i++)			
		{
			if (_setRegister2(addr, data))
			{
				_readRegister(addr,&tempRegisterValue);
				if (i<(I2C_RETRY_COUNT/2))
				{
					DAC_delay_ns(500);
				}
				if (data==tempRegisterValue)
				{	
					if (i>=1)
						MP_DEBUG("%s retry number: %d", __FUNCTION__, i);
					//mpDebugPrint("%s return 1", __FUNCTION__);
					return 1;
				}
			}
		}
		
		mpDebugPrint("ALC5633Q got wrong ack value...and latch up");
		mpDebugPrint("%s's _setRegister2 exceeded max retry number: %d", __FUNCTION__, i);
		mpDebugPrint("Reg: 0x%04x, data: (wanted)0x%04x != (actual)0x%04x",addr, data, tempRegisterValue);
		return 0;	
}


// 2-wire serial control mode
BYTE _setRegister2(WORD addr, WORD data)
{
   // MP_DEBUG("%s enter", __FUNCTION__);
	BYTE i, tmp;

	//Initial PIN status
	ALC5633Q_SDA_HIGH();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	ALC5633Q_SDA_LOW();
	DAC_delay_ns(700);
	ALC5633Q_SCL_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (ALC5633Q_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

	ALC5633Q_SDA_LOW();		//Set write command bit
	ALC5633Q_DELAY();
	ALC5633Q_SCL_HIGH();
	//DAC_delay_ns(300);  // original
    DAC_delay_ns(700);
	ALC5633Q_SCL_LOW();
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
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5633Q_SCL_LOW();
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
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5633Q_SCL_LOW();
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
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	// Stop stage
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(700);
	ALC5633Q_SDA_HIGH();
	ALC5633Q_DELAY();

	return 1;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	BYTE i, tmp;

	*data = 0;	//Reset input data

	//mpDebugPrint("---- read addr: %x ----", addr);

	//Initial PIN status
	ALC5633Q_SDA_HIGH();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	ALC5633Q_SDA_LOW();
	DAC_delay_ns(700);
	ALC5633Q_SCL_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (ALC5633Q_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		DAC_delay_ns(300);


		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

	ALC5633Q_SDA_LOW();		//Set write command bit
	ALC5633Q_DELAY();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5633Q_SCL_LOW();
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
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		DAC_delay_ns(300);

		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	//Initial PIN status
	ALC5633Q_SDA_HIGH();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(400);

	// Sr Condiction
	ALC5633Q_SDA_LOW();
	DAC_delay_ns(700);
	ALC5633Q_SCL_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (ALC5633Q_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		ALC5633Q_DELAY();

		ALC5633Q_SCL_HIGH();
		DAC_delay_ns(300);


		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

	ALC5633Q_SDA_HIGH();		//Set read command bit
	ALC5633Q_DELAY();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5633Q_SCL_LOW();

	// Control byte2 stage - Read ack and MSByte Data (Total 9 bits)
	ALC5633Q_SDA_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 8; i++)
	{	
		ALC5633Q_SCL_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();

		if(i > 0)
		{
			*data |= (tmp << (16 - i));
		}
		//mpDebugPrint("tmp(%d), %x", i, tmp);

		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

#if 0
	if(_checkAck()){}
//		return 0;
#else	// Send Ack ....
	ALC5633Q_SDA_OUT();
	DAC_delay_ns(1000);
	ALC5633Q_SDA_LOW();
	ALC5633Q_DELAY();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5633Q_SCL_LOW();
	DAC_delay_ns(1500);
#endif
	ALC5633Q_SDA_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 7; i++)
	{
		ALC5633Q_SCL_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();
		*data |= (tmp << (7 - i));

		//mpDebugPrint("tmp(%d), %x", i + 9, tmp);

		ALC5633Q_SCL_LOW();
		DAC_delay_ns(1500);
	}

	MP_DEBUG("	Register 0x%02x - data: 0x%04x", addr, *data);

#if 0
	if(_checkAckinv()){}
//		return 0;
#else	//Send invert ack
	ALC5633Q_SDA_OUT();
	DAC_delay_ns(1000);
	ALC5633Q_SDA_HIGH();
	ALC5633Q_DELAY();
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5633Q_SCL_LOW();
	DAC_delay_ns(1500);
#endif

	// Stop stage
	ALC5633Q_SCL_HIGH();
	DAC_delay_ns(700);
	ALC5633Q_SDA_HIGH();
	ALC5633Q_DELAY();

	return 1;
}

static DWORD gpcfg1;
static DWORD gpdat1;
// Set the GPIO condition for I2S communication
void ALC5633Q_GPIO_Init()
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

	I2CM_WtReg8Data16(ALC5633Q_DEVICE_ADDRESS << 1, (BYTE)addr, data);
	readBack = I2CM_RdReg8Data16(ALC5633Q_DEVICE_ADDRESS << 1, (BYTE)addr);

	//MP_ALERT("%s - Addr = 0x%04X, 0x%04X, 0x%04X", __FUNCTION__, (DWORD) addr, (DWORD) data, (DWORD) readBack);

	return 0;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	*data = I2CM_RdReg8Data16(ALC5633Q_DEVICE_ADDRESS << 1, (BYTE)addr);
	return 0;
}
#endif

/*
 *    BoostGain:	bypass:0, 20dB:1, 30dB:2
 *    ADCGain:	0 - 31 ( -16.5 to +30 dB )
 *    MicVol: 		31 to 0 ( -34.5 to +12 dB )
 */
//void ALC5633Q_MicGain(DWORD BoostGain, DWORD ADCGain, DWORD MicVol)
void ALC5633Q_MicGain(DWORD BoostGain, DWORD ADCGain)
{
	MP_DEBUG("%s",__func__);
	int dB;
	volatile DWORD tempRegisterValue;
}

extern SDWORD EmbededAdc_ValueGet(E_ADC_GROUP_INDEX group);
/*
 *    Mapping 16 level of UI volume value to hardware value
 */
int ALC5633Q_ChgVolume(DWORD vol)
{
	MP_DEBUG("%s",__func__);

	DWORD vol1, vol2, dB;
	volatile DWORD tempRegisterValue;
	BYTE *pbRightVol = NULL;
	BYTE *pbLeftVol = NULL;
	int i;
#if 0
	vol = vol & 0x1f;

	vol1 = (32-vol*2-4);

	MP_DEBUG("vol=%d, vol1=%d",vol,vol1);
	if (ALC5633Q_RecordMode==FALSE) // Set SPK volume, Max: 0dB, Min:-46.5dB
	{
		if(vol==16)
		{
		    _setRegister2_w_retry(ALC5633Q_SPK_OUT_VOL, 0x0);	// max volume
			_setRegister2_w_retry(ALC5633Q_STEREO_DAC_VOL,0xC606);
		}
		else if (vol==15)
		{
			_setRegister2_w_retry(ALC5633Q_SPK_OUT_VOL, 0x0);	// max volume
			_setRegister2_w_retry(ALC5633Q_STEREO_DAC_VOL,0xC808);
		}
		else if (vol==0)//(vol1>=31)
		{	
			_setRegister2_w_retry(ALC5633Q_SPK_OUT_VOL, 0x8080); // mute mode
		}
		else
		{
			vol2 = ( (vol1 << 8) | vol1);
			_setRegister2_w_retry(ALC5633Q_STEREO_DAC_VOL,0xCA0A);
			_setRegister2_w_retry(ALC5633Q_SPK_OUT_VOL, vol2);
		}
		
	//	_setRegister2_w_retry(ALC5633Q_SPK_OUT_VOL, vol2);  //max volume = 0, mute=0x27= -46.5db
		_readRegister(ALC5633Q_SPK_OUT_VOL,&tempRegisterValue);

		
		if (vol1<=0)
		{
			dB = 0;
			MP_DEBUG("ALC5633Q SPK Volume: %ddB (MAX), (0x%04x)",dB,tempRegisterValue);
		}
		else if (vol1>=31)
		{
			dB = 31*(1.5);
			MP_DEBUG("ALC5633Q SPK Volume: -%ddB (MIN), (0x%04x)",dB,tempRegisterValue);
		}
		else
		{
			dB = 46.5-((32)-vol1)*(1.5);
			MP_DEBUG("ALC5633Q SPK Volume: -%ddB, (0x%04x)",dB,tempRegisterValue);
		}

		
	}
	else								// Set Mic volume, Max:+12dB, Min:-34.5dB
	{
		if(vol1<=0)
		{
			_setRegister2_w_retry(ALC5633Q_MIC_VOL, 0x0);	// max volume
		}
		else if (vol1>=31)
		{
			_setRegister2_w_retry(ALC5633Q_MIC_VOL, 0x1f1f); // min volume (-34.5dB)
		}
		else
		{
			vol2 = ( (vol1 << 8) | vol1);//mic1 and mic2
//			vol2 = ( (vol1 << 8));//mic1
			_setRegister2_w_retry(ALC5633Q_MIC_VOL, vol2);
		}

		_readRegister(ALC5633Q_MIC_VOL,&tempRegisterValue);
		if (vol1<=8)
		{
			dB = 12-(vol1)*(1.5);
			MP_DEBUG("ALC5633Q MIC Volume: %ddB, (0x%04x)",dB,tempRegisterValue);
		}
		else
		{
			dB = (vol1-1)*(1.5)-12;
			MP_DEBUG("ALC5633Q MIC Volume: -%ddB, (0x%04x)",dB,tempRegisterValue);
		}
	}

#endif



// DAC Reg Test Code
#if 0
//	if 1//(vol == 0)
	//{
		mpDebugPrint("================================================");
		mpDebugPrint("== Dump_Register() in %s() ==", __func__);
		mpDebugPrint("================================================");
		Dump_Register(); 
//	}
#endif

	return PASS;
}


// The crystal frequency is based on 12 HZ
void ALC5633Q_SetDACSampleRate(DWORD sRate)
{

	register CLOCK *audio_clock;
	int data;
	audio_clock = (CLOCK *) (CLOCK_BASE);
	audio_clock->PLL3Cfg = 0x00770107;
	
	#if AUDIO_DAC_USING_PLL2
	{
		if((PLL2CLOCK == 153)|| (PLL2CLOCK == 147) || (PLL2CLOCK == 165))//|| (PLL2CLOCK == 138) || (PLL2CLOCK == 144)  || //(PLL2CLOCK == 160)  )
		{
			mpDebugPrint("ALC5633Q using PLL2 Clock Freq: %d", PLL2CLOCK);
		}
		else
		{
			MP_ALERT("ALC5633Q doesn't support PLL2 = %d; Only supports PLL2: 153, 165 and 147(some sample rates).", PLL2CLOCK);
		}
	}
	#else
	{
		mpDebugPrint("ALC5633Q using PLL3 Clock.");
		MP_ALERT("ALC5633Q, Sample Rates using PLL3 haven't been tested yet!!!");
	}
	#endif

	mpDebugPrint("ALC5633Q DAC set Samplerate: %d", sRate);
	
	if ( (sRate > 7000)  && (sRate < 11000))	// Set 8kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(0x38,0x0000);		//256fs by default
				audio_clock->AudClkC = 0x000013ec;    	//Bclk:510 Mclk:2040 LRClk:7.96
				AIU_SET_LRCLOCK_LEN(0x3f);				//lrc=7.968khz
			}
			else if(PLL2CLOCK == 147)
			{
				_setRegister2_w_retry(0x38,0x0000);		//256fs by default
				audio_clock->AudClkC = 0x0000139E;   	//Bclk:525 Mclk:2100 LRClk:8.2
				AIU_SET_LRCLOCK_LEN(0x3f);				//lrc=8.2khz
			}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0080);
				audio_clock->AudClkC = 0x0000139b;	//165/4/10 = 4.125Mhz, must be above 3Mhz 
				AIU_SET_LRCLOCK_LEN(0x7e);			//8.056khz
			}
		#else
			audio_clock->PLL3Cfg = 0x00777f7c;
			audio_clock->AudClkC = 0x00001350;	
			AIU_SET_LRCLOCK_LEN(0x3F);
		#endif	

	}
	else if((sRate >= 11000) && (sRate < 11300))	// Set 11.025kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(0x38,0x0000); 
				audio_clock->AudClkC = 0x000013AC;//Mclk:2781 Bclk:695 LRclk:10.8
				AIU_SET_LRCLOCK_LEN(0x3F);
				}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0080);
				audio_clock->AudClkC = 0x0000134f;	//MCLK = 165/8/5 = 4.125Mhz, must be above 3Mhz 
				AIU_SET_LRCLOCK_LEN(0x5e);			//10.742khz
			}
		#else
			audio_clock->PLL3Cfg = 0x00772fcb;
			audio_clock->AudClkC = 0x00001070;	
			AIU_SET_LRCLOCK_LEN(0x1F);//11.029khz
		#endif	

	}
	else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(0x38,0x0000); 
				audio_clock->AudClkC = 0x0000114C;//Mclk:3.06 Bclk:765 LRclk:12.35
				AIU_SET_LRCLOCK_LEN(0xFE);			//
				}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0080);
				audio_clock->AudClkC = 0x0000135d;	//MCLK = 165/6/4 = 4.58Mhz, must be above 3.07Mhz
				AIU_SET_LRCLOCK_LEN(0x5e);			//11.935khz
			}
		#else//not tested
			audio_clock->PLL3Cfg = 0x00777f7c;
			audio_clock->AudClkC = 0x00001730;
			AIU_SET_LRCLOCK_LEN(0x1f);		
		#endif	

	}
	else if((sRate >= 14000) && (sRate < 22000))	// Set 16kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(0x38,0x0000); 
				audio_clock->AudClkC = 0x000013BA;//Mclk= 4250Mhz  Bclk:1062  
				AIU_SET_LRCLOCK_LEN(0x3F);			//LRC=16.601Khz
			}
			else if (PLL2CLOCK == 147) ///for PLL2=147
			{
				_setRegister2_w_retry(0x38,0x0000);//256fs by default 
				audio_clock->AudClkC = 0x000013BA;    //Bclk:1020 Mclk:4083 LRClk:15.95
				AIU_SET_LRCLOCK_LEN(0x3f);		//lrc=15.95khz
			}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000137c;	//165/5/8/4 = 1031khz
				AIU_SET_LRCLOCK_LEN(0x3e);			//16.113khz
			}
		#else//not tested
			audio_clock->PLL3Cfg = 0x007718cc;
			audio_clock->AudClkC = 0x00001270;	
			AIU_SET_LRCLOCK_LEN(0xFF);		
		#endif	
	}
	else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000138A;//153/6/8 = 5.666Mhz
				AIU_SET_LRCLOCK_LEN(0x3F);			//22.135Khz
			}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000135c;//165/5/6/4 = 1375khz
				AIU_SET_LRCLOCK_LEN(0x3e);		//21.484khz
			}
		#else//not tested
			audio_clock->PLL3Cfg = 0x00770f21;
			audio_clock->AudClkC = 0x00001070;	
			AIU_SET_LRCLOCK_LEN(0x1F);				//22.058khz
		#endif	
	}
	else if((sRate >= 22000) && (sRate < 30000))	// Set 24kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000134c;//Mclk:6.125  Bclk:1.53 LRClk:23.92
				AIU_SET_LRCLOCK_LEN(0x3e);
			}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000138a;//165/3/9/4 = 6.111Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);		//23.871khz
			}
		#else	//not tested
			audio_clock->PLL3Cfg = 0x007718cc;
			audio_clock->AudClkC = 0x00001170;	
			AIU_SET_LRCLOCK_LEN(0xFF);
		#endif	
	}
	else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000135a;//Mclk:850 Bclk:212 LRclk:32.22
				AIU_SET_LRCLOCK_LEN(0x40);  //
				}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x00001399;//165/2/10/4 = 2.062Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);		//32.226khz
			}
		#else	//not tested
			audio_clock->PLL3Cfg = 0x007718cc;
			audio_clock->AudClkC = 0x00001270;	
			AIU_SET_LRCLOCK_LEN(0x7F);
		#endif	
	}
	else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x000013C8;//Mclk:11.769 Bclk:2.932 
				AIU_SET_LRCLOCK_LEN(0x3F);			//45.973Khz
				}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x0000134a;	//165/3/5/4 = 2.750Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//42.968khz
			}
		#else //not tested
			audio_clock->PLL3Cfg = 0x00777d85;
			audio_clock->AudClkC = 0x00001070;	
			AIU_SET_LRCLOCK_LEN(0x1F);				//44.117khz
		#endif	
	}
	else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
	{
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x000013B8;//Mclk:12.75  Bclk:3.18  LRclk:48.25
				AIU_SET_LRCLOCK_LEN(0x3F);		//4
			}
			else if (PLL2CLOCK == 165)
			{
				_setRegister2_w_retry(0x38,0x0000);
				audio_clock->AudClkC = 0x000013c8;	//165/1/13/4 = 3.173Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//49.579khz
			}
		#else	//not tested
			audio_clock->PLL3Cfg = 0x007718cc;//MCLK ==12.288Mhz
			audio_clock->AudClkC = 0x00001070;		
			AIU_SET_LRCLOCK_LEN(0xFF);
		#endif	
	}
	else
	{
		not_supported_samplerate:
		MP_ALERT("Samplerate %d is not supported by IIS.(8K--48K support)", sRate);
	}

	// If you want to change PLL value, follow below steps
	// 1. setting PLL config.
	// 2. Disable PLL (ClkCtrl , Clkss_EXT1)
	// 3. Re-enable PLL(ClkCtrl , Clkss_EXT1)
	#if AUDIO_DAC_USING_PLL2
		g_psClock->Clkss_EXT1 &= 0xfffffff0;
		MP_DEBUG("ALC5633Q: PLL3 disabled");
	#else
		g_psClock->Clkss_EXT1 &= 0xfffffff0;
		g_psClock->Clkss_EXT1 |= (BIT3);			// Using PLL3/1 to be clock source
				
		//	g_psClock->ClkCtrl &= ~0x01000000;		// disable PLL3
		g_psClock->ClkCtrl |= 0x01000000;			// enable PLL3
		MP_DEBUG("ALC5633Q: PLL3 enabled");
	#endif

	MP_DEBUG("ALC5633Q g_psClock->Clkss_EXT1: 0x%x",g_psClock->Clkss_EXT1);

#if 0	//to check if sample rate is correct from oscilloscope
	__asm("break 100");
#endif
	
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

}



// Setting AIU registers and reset sample rate 
int  ALC5633Q_PlayConfig(DWORD sampleRate)
{
	MP_DEBUG("%s",__func__);
#if (ALC5633Q_USINGHWI2C == ENABLE)
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x0077ffc3;
	g_psClock->AudClkC = 0x00001110;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	//g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3
	I2CM_FreqChg(ALC5633Q_DEVICE_ADDRESS << 1, 300000);	// Set correct I2C frequency for enabing I2C setting
#else
	ALC5633Q_GPIO_Init();
#endif
	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();

	ALC5633Q_SetDACSampleRate(sampleRate); 
	AIU_I2S_AUDIO_INTERFACE();
#if 0
	mpDebugPrint("================================================");
	mpDebugPrint("== Dump_Register() in %s() ==", __func__);
	mpDebugPrint("================================================");
	Dump_Register(); 
#endif
	return PASS;
}

int ALC5633Q_uninit()
{
	MP_DEBUG("Uninit ALC5633Q");
#if 1	//power off
	ALC5633Q_ChgVolume(0);//for spk
	ALC5633Q_RecordMode=TRUE;
	ALC5633Q_ChgVolume(0);//for mic
	ALC5633Q_RecordMode=FALSE;
	_setRegister2_w_retry(Power_management_control1,0x0000);
	_setRegister2_w_retry(Power_management_control2,0x0000);
	_setRegister2_w_retry(Power_management_control3,0x0000);
	_setRegister2_w_retry(Power_management_control4,0x0000);
#endif
	return PASS;
}

/**
 ***************************************************************
 *
 * Config ALC5633Q clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void ALC5633Q_AIUCfg(void)
{
	//MP_DEBUG("ALC5633Q_AIUCfg");
	#if 0
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x00771252;
	g_psClock->AudClkC = 0x000011f0;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3
    #endif

	g_psClock->PLL3Cfg = 0x007718CC;
	g_psClock->AudClkC = 0x00001570;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);	 // Using PLL3/1 to be clock source
	g_psClock->Clkss_EXT2 |= 0x00040000;	//Using since mp650 series
	g_psClock->ClkCtrl &= ~0x01000000;	 // Disable PLL3
	g_psClock->ClkCtrl |= 0x01000000;	 // enable PLL3
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
	ALC5633Q_GPIO_Init();
#endif

}

#define ALC5633Q_LookBack DISABLE// to record and listen the sound from mic
int ALC5633Q_InitRecordMode()
{
	//mpDebugPrint(" =====ALC5633Q Audio Record Init... Start=====");
	volatile DWORD data;
	ALC5633Q_RecordMode = TRUE;

  
	#if SOFT_RECORD	 // It just temporialy used #if to distinguish left and right justified, we will remove it as soon as possible 2010/05/26 XianWen note
		mpDebugPrint("-I- SOFT_RECORD not supported");
	#endif

		_setRegister2_w_retry(0x3A,0x9800); //Reg3A 9800
	    _setRegister2_w_retry(0x3B,0x0C28); //Reg3B 0C28
	    _setRegister2_w_retry(0x3C,0xE000); //Reg3C E000
	    _setRegister2_w_retry(0x3E,0x0000); //Reg3E 0000
	    _setRegister2_w_retry(0x14,0x7D7F); //Reg14 REC MIXER CONTROL7B7B
	    _setRegister2_w_retry(0x10,0x8808); //Reg10 MICROPONE input control 8808 MIC1 Volume:12dB
	    _setRegister2_w_retry(0x22,0x1000); //Microphone control boost:30dB
	    _setRegister2_w_retry(0x12,0x0013); //ADC control boost  +8dB
		
		ALC5633Q_RecordMode = FALSE;

		//mpDebugPrint(" ALC5633Q Audio Record Init... Finished");
		#if 1  //EQ test
		_setRegister2_w_retry(0x70,0x0043);
		_setRegister2_w_retry(0x6e,0x9080);//0xB080
		_setRegister2_w_retry(0x6A,0x0000);
		_setRegister2_w_retry(0x6C,0x1757);
		_setRegister2_w_retry(0x6A,0x0001);
		_setRegister2_w_retry(0x6C,0xF405);
		_setRegister2_w_retry(0x6A,0x0002);
		_setRegister2_w_retry(0x6C,0xD360);
		_setRegister2_w_retry(0x6A,0x0003);
		_setRegister2_w_retry(0x6C,0x104E);
		_setRegister2_w_retry(0x6A,0x0004);
		_setRegister2_w_retry(0x6C,0x0C73);
		_setRegister2_w_retry(0x6A,0x0005);
		_setRegister2_w_retry(0x6C,0x0000);
		_setRegister2_w_retry(0x6A,0x0006);
		_setRegister2_w_retry(0x6C,0x0D41);
		_setRegister2_w_retry(0x6A,0x0007);
		_setRegister2_w_retry(0x6C,0x0000);
		_setRegister2_w_retry(0x6A,0x0008);
		_setRegister2_w_retry(0x6C,0xE001);
		_setRegister2_w_retry(0x6A,0x0009);
		_setRegister2_w_retry(0x6C,0x0D41);
		_setRegister2_w_retry(0x6A,0x000A);
		_setRegister2_w_retry(0x6C,0x0000);
		_setRegister2_w_retry(0x6A,0x0013);
		_setRegister2_w_retry(0x6C,0xE001);
		_setRegister2_w_retry(0x6A,0x0014);
		_setRegister2_w_retry(0x6C,0x0D41);
		_setRegister2_w_retry(0x6A,0x0015);
		_setRegister2_w_retry(0x6C,0x0000);
		_setRegister2_w_retry(0x6A,0x000B);
		_setRegister2_w_retry(0x6C,0x0A65);
		//_setRegister2_w_retry(0x6A,0x000C);
		//_setRegister2_w_retry(0x6C,0x0000);
		_setRegister2_w_retry(0x6A,0x000D);
		_setRegister2_w_retry(0x6C,0xE1D5);
		_setRegister2_w_retry(0x6A,0x000E);
		_setRegister2_w_retry(0x6C,0x0AAA);
		_setRegister2_w_retry(0x6A,0x000F);
		_setRegister2_w_retry(0x6C,0x1235);
		_setRegister2_w_retry(0x6A,0x0011);
		_setRegister2_w_retry(0x6C,0x0000);
		_setRegister2_w_retry(0x6A,0x0012);
		_setRegister2_w_retry(0x6C,0x0003);
		#endif
	
#if 0
	mpDebugPrint("================================================");
	mpDebugPrint("== Dump_Register() in %s() ==", __func__);
	mpDebugPrint("================================================");
	Dump_Register(); 
#endif
	return PASS;

}

int AudioInit_ALC5633Q(void)
{

	MP_DEBUG("#########################");
	mpDebugPrint("Audio DAC ALC5633Q Init...");
	MP_DEBUG("#########################");

	ALC5633Q_AIUCfg();

	_setRegister2_w_retry(Software_reset,0x0001); 	//RESET

	_setRegister2_w_retry(0x3A,0xA020); //Reg3A 9800
	_setRegister2_w_retry(0x3B,0x0000); //Reg3B 0C28
	_setRegister2_w_retry(0x3C,0xA000); //Reg3C E000
	_setRegister2_w_retry(0x3E,0x0000); //Reg3E 0000

	
			
#if 0
	mpDebugPrint("================================================");
	mpDebugPrint("== Dump_Register() in %s() ==", __func__);
	mpDebugPrint("================================================");
	Dump_Register(); 
#endif
				

	return PASS;
}

int AudioPlayBack_ALC5633Q(void)
{
	MP_DEBUG("%s",__func__);
	#if 1 //codec init and playback   Speaker
    _setRegister2_w_retry(0x56,0xB000); //Reg56  B000
    _setRegister2_w_retry(0x3A,0x87E0); //Reg3A 87C0
    _setRegister2_w_retry(0x3B,0xC000); //Reg3B C000
    _setRegister2_w_retry(0x3C,0xE000); //Reg3C E00B
    _setRegister2_w_retry(0x3E,0xC000); //Reg3E 0C00
    _setRegister2_w_retry(0x18,0x3E3E); //Reg18 3E3E
    _setRegister2_w_retry(0x1C,0x04FF); //Reg04 4040
    _setRegister2_w_retry(0x03,0x0020); //Reg03 0020
    _setRegister2_w_retry(0x02,0x6000); //Reg03 0020
	_setRegister2_w_retry(0x1E,0x0000); //Reg03 0020
	_setRegister2_w_retry(0x0C,0x0030); //Reg03 0020
	#endif
}

void Dump_Register(void)
{
#if LOCAL_DEBUG_ENABLE

	DWORD tempRegisterValue;
	WORD addr; 

	mpDebugPrint("======== start %s ========", __func__);

	for (addr=0; addr <= 0x7e; addr += 1)
	{
		_readRegister(addr,&tempRegisterValue);
		//mpDebugPrint("addr(0x%02x) = %04x", addr, tempRegisterValue);
	}

	mpDebugPrint("======== end %s ========", __func__); 

#endif
}

HAL_AUDIODAC_T _audioDAC_ALC5633Q =
{
	AudioInit_ALC5633Q,
	ALC5633Q_InitRecordMode,	
	ALC5633Q_uninit,
	ALC5633Q_PlayConfig,
	AudioPlayBack_ALC5633Q,
	ALC5633Q_ChgVolume
};
