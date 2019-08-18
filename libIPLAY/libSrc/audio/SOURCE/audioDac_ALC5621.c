/**
 * FileName:    AudioDac_ALC5621.c
 *
 * Author:     	Eddyson
 * Date:     	2012.05.09
 * Updated:		2012.11.29
 * Version:		0.16
 *
 * Description:	Driver for RealTek ALC5621
 * 
 **/ 
#define LOCAL_DEBUG_ENABLE 0


//================ Include section =============================== 
#include "global612.h"
#include "../INCLUDE/audio_hal.h"
#include "mpTrace.h"
#include "UtilRegFile.h"
#include "../../peripheral/include/Hal_adc.h"	// to use funct. to detect battery voltage
//================================================================

#if (AUDIO_DAC == DAC_ALC5621)

//================ Global Variables ============================== 
BOOL ALC5621_RecordMode;
//================================================================


//================ Define Codec Settings ========================= 
#if (SOFT_RECORD || RECORD_AUDIO)
#define ALC5621_USINGHWI2C    		DISABLE
#else
#define ALC5621_USINGHWI2C    		ENABLE
#endif

#define PLL2CLOCK (Clock_PllFreqGet(CLOCK_PLL2_INDEX)/1000000)
#define I2C_RETRY_COUNT 20

//#define ALC5621_OUT_BOTH      0	// Output audio data to Speaker and Earphone
//#define ALC5621_OUT_EXCLUSIVE 1	// Output audio data to Speaker or Earphone

//----------DAC Settings------------------------------------------------------------------------//
#define ALC5621_DAC_W_EQ				ENABLE		//	ENABLE/DISABLE playback using EQ
#if (ALC5621_DAC_W_EQ == ENABLE)
#define ALC5621_DAC_W_EQ_INCREASE_BASS	DISABLE		//	ENABLE : Increase XXdB for frequencies below 100Hz
													//	DISABLE: Decrease XXdB for frequencies below XX-X kHz
#define ALC5621_DAC_W_EQ_20DB_DECADE 	ENABLE		//	ENABLE : EQ using -20dB/Decade
													//	DISABLE: EQ using Shelving Filter
#endif
//----------------------------------------------------------------------------------------------//

//----------ADC Settings------------------------------------------------------------------------//
#define ALC5621_ADC_W_EQ				DISABLE		//	ENABLE/DISABLE recording using EQ
#if (ALC5621_ADC_W_EQ == ENABLE)
#define ALC5621_ADC_W_EQ_20DB_DECADE 	ENABLE		//	ENABLE : EQ using -20dB/Decade
													//	DISABLE: EQ using Shelving Filter
#endif

#define ALC5621_ADC_BYPASS 				DISABLE		// to record and listen the sound from mic
#define ALC5621_ADC_W_AGC 				DISABLE		// to record using AGC
//----------------------------------------------------------------------------------------------//
//================================================================


//================ Define Registers Variables ====================
#define ALC5621_RESET					0x00
#define ALC5621_SPK_OUT_VOL				0x02
#define ALC5621_HP_OUT_VOL				0x04
#define ALC5621_MONO_AUX_OUT_VOL		0x06
#define ALC5621_AUXIN_VOL				0x08
#define ALC5621_LINE_IN_VOL				0x0a
#define ALC5621_STEREO_DAC_VOL			0x0c
#define ALC5621_MIC_VOL					0x0e
#define ALC5621_MIC_ROUTING_CTRL		0x10
#define ALC5621_ADC_REC_GAIN			0x12
#define ALC5621_ADC_REC_MIXER			0x14
#define ALC5621_SOFT_VOL_CTRL_TIME		0x16
#define ALC5621_OUTPUT_MIXER_CTRL		0x1c
#define ALC5621_MIC_CTRL				0x22
#define ALC5621_AUDIO_INTERFACE			0x34
#define ALC5621_STEREO_AD_DA_CLK_CTRL	0x36
#define ALC5621_COMPANDING_CTRL			0x38
#define ALC5621_PWR_MANAG_ADD1			0x3a
#define ALC5621_PWR_MANAG_ADD2			0x3c
#define ALC5621_PWR_MANAG_ADD3			0x3e
#define ALC5621_ADD_CTRL_REG			0x40
#define ALC5621_GLOBAL_CLK_CTRL_REG		0x42
#define ALC5621_PLL_CTRL				0x44
#define ALC5621_GPIO_OUTPUT_PIN_CTRL	0x4a
#define ALC5621_GPIO_PIN_CONFIG			0x4c
#define ALC5621_GPIO_PIN_POLARITY		0x4e
#define ALC5621_GPIO_PIN_STICKY			0x50
#define ALC5621_GPIO_PIN_WAKEUP			0x52
#define ALC5621_GPIO_PIN_STATUS			0x54
#define ALC5621_GPIO_PIN_SHARING		0x56
#define ALC5621_OVER_TEMP_CURR_STATUS	0x58
#define ALC5621_JACK_DET_CTRL			0x5a
#define ALC5621_MISC_CTRL				0x5e
#define ALC5621_PSEDUEO_SPATIAL_CTRL	0x60
#define ALC5621_EQ_CTRL					0x62
#define ALC5621_EQ_MODE_ENABLE			0x66
#define ALC5621_AVC_CTRL				0x68
#define ALC5621_HID_CTRL_INDEX			0x6a
#define ALC5621_HID_CTRL_DATA			0x6c
#define ALC5621_VENDOR_ID1				0x7c
#define ALC5621_VENDOR_ID2				0x7e
//================================================================


//================ Define I2C ====================================
#define ALC5621_DEVICE_ADDRESS    0x1a	// 0x34 >>1 = 0x1a
/*
#define ALC5621_DELAY()	__asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
		                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
		                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");  \
		                    __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
		                    */

#define ALC5621_SCL_HIGH()		g_psGpio->Gpdat0  |=  0x00000001		//set pin37 up
#define ALC5621_SCL_LOW()		g_psGpio->Gpdat0  &= ~0x00000001		//set pin37 down

#define ALC5621_SDA_HIGH()		g_psGpio->Gpdat0  |=  0x00000002		//set pin36 up
#define ALC5621_SDA_LOW()		g_psGpio->Gpdat0  &= ~0x00000002		//set pin36 down
#define ALC5621_SDA_OUT()		g_psGpio->Gpdat0  |=  0x00020000		//set pin2 Out
#define ALC5621_SDA_IN()		g_psGpio->Gpdat0  &= ~0x00020000		//set pin2 In
#define ALC5621_SDA_GET()		((g_psGpio->Gpdat0 & 0x02) >> 1)
//================================================================


//Caculation is based on CPU time 96MHz
void DAC_delay_ns(DWORD ns)
{
	DWORD i;
	if (PLL2CLOCK == 147)
		ns <<=  1;
	ns = ns / 10;	//One instruction consume 10ns //for 138mhz

	for(i = 0; i < ns; i++)
		__asm("nop");
}

void ALC5621_DELAY()
{
	DWORD i,ns;
	ns = 20;
	if (PLL2CLOCK == 147)
		ns <<=  1;

	for(i = 0; i < ns; i++)
		__asm("nop");
}


#if(ALC5621_USINGHWI2C != ENABLE)

void _pullSDIN(BYTE data)
{
	if(data)
		ALC5621_SDA_HIGH();
	else
		ALC5621_SDA_LOW();
}

BYTE _checkAck()
{
	BYTE data = 1;

	//mpDebugPrint("in1 Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);

	ALC5621_SDA_IN();
	ALC5621_DELAY();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(400);

	data = ALC5621_SDA_GET();
	DAC_delay_ns(300);

	ALC5621_SCL_LOW();
	ALC5621_DELAY();

	ALC5621_SDA_OUT();
	DAC_delay_ns(1500);

	if(data){
		//mpDebugPrint("ALC5621 got wrong ack value...and latch up");
		//while(1);
		return 1;
	}

	return 0;
}

BYTE _checkAckinv()
{
	BYTE data = 1;

	ALC5621_SDA_IN();
	ALC5621_DELAY();

	ALC5621_SCL_HIGH();
	DAC_delay_ns(400);

	data = ALC5621_SDA_GET();
	DAC_delay_ns(300);

	ALC5621_SCL_LOW();
	ALC5621_DELAY();

	ALC5621_SDA_OUT();
	DAC_delay_ns(1500);

	if(!data){
		mpDebugPrint("ALC5621 got wrong ack value...and latch up");
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

	ALC5621_SCL_HIGH();
	DAC_delay_ns(400);

	data = ALC5621_SDA_GET();
	DAC_delay_ns(300);

	ALC5621_SCL_LOW();
	ALC5621_DELAY();

	//if(data){
	//	mpDebugPrint("fuck ALC5621 ack value...and latch up");
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
   // MP_DEBUG("%s enter", __FUNCTION__);
	BYTE i, tmp;

	//Initial PIN status
	ALC5621_SDA_HIGH();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	ALC5621_SDA_LOW();
	DAC_delay_ns(700);
	ALC5621_SCL_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (ALC5621_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

	ALC5621_SDA_LOW();		//Set write command bit
	ALC5621_DELAY();
	ALC5621_SCL_HIGH();
	//DAC_delay_ns(300);  // original
    DAC_delay_ns(700);
	ALC5621_SCL_LOW();
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
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5621_SCL_LOW();
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
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5621_SCL_LOW();
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
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		//DAC_delay_ns(300);  // original
        DAC_delay_ns(700);

		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	// Stop stage
	ALC5621_SCL_HIGH();
	DAC_delay_ns(700);
	ALC5621_SDA_HIGH();
	ALC5621_DELAY();

	return 1;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	BYTE i, tmp;

	*data = 0;	//Reset input data

	//mpDebugPrint("---- read addr: %x ----", addr);

	//Initial PIN status
	ALC5621_SDA_HIGH();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(400);

	// Start Condiction
	ALC5621_SDA_LOW();
	DAC_delay_ns(700);
	ALC5621_SCL_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (ALC5621_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		DAC_delay_ns(300);


		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

	ALC5621_SDA_LOW();		//Set write command bit
	ALC5621_DELAY();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5621_SCL_LOW();
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
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		DAC_delay_ns(300);

		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

	if(_checkAck())
		return 0;

	//Initial PIN status
	ALC5621_SDA_HIGH();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(400);

	// Sr Condiction
	ALC5621_SDA_LOW();
	DAC_delay_ns(700);
	ALC5621_SCL_LOW();
	DAC_delay_ns(1500);

	// Device address input stage
	for(i = 0; i < 7; i++)
	{
		tmp = (ALC5621_DEVICE_ADDRESS >> (6 - i)) & 0x1;
		//mpDebugPrint("D%d  %d", i, tmp);

		_pullSDIN(tmp);
		ALC5621_DELAY();

		ALC5621_SCL_HIGH();
		DAC_delay_ns(300);


		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

	ALC5621_SDA_HIGH();		//Set read command bit
	ALC5621_DELAY();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5621_SCL_LOW();

	// Control byte2 stage - Read ack and MSByte Data (Total 9 bits)
	ALC5621_SDA_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 8; i++)
	{	
		ALC5621_SCL_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();

		if(i > 0)
		{
			*data |= (tmp << (16 - i));
		}
		//mpDebugPrint("tmp(%d), %x", i, tmp);

		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

#if 0
	if(_checkAck()){}
//		return 0;
#else	// Send Ack ....
	ALC5621_SDA_OUT();
	DAC_delay_ns(1000);
	ALC5621_SDA_LOW();
	ALC5621_DELAY();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5621_SCL_LOW();
	DAC_delay_ns(1500);
#endif
	ALC5621_SDA_IN();
	DAC_delay_ns(1500);

	for(i = 0; i <= 7; i++)
	{
		ALC5621_SCL_HIGH();
		DAC_delay_ns(300);

		tmp = _readdata();
		*data |= (tmp << (7 - i));

		//mpDebugPrint("tmp(%d), %x", i + 9, tmp);

		ALC5621_SCL_LOW();
		DAC_delay_ns(1500);
	}

	MP_DEBUG("	Register 0x%02x - data: 0x%04x", addr, *data);

#if 0
	if(_checkAckinv()){}
//		return 0;
#else	//Send invert ack
	ALC5621_SDA_OUT();
	DAC_delay_ns(1000);
	ALC5621_SDA_HIGH();
	ALC5621_DELAY();
	ALC5621_SCL_HIGH();
	DAC_delay_ns(300);
	ALC5621_SCL_LOW();
	DAC_delay_ns(1500);
#endif

	// Stop stage
	ALC5621_SCL_HIGH();
	DAC_delay_ns(700);
	ALC5621_SDA_HIGH();
	ALC5621_DELAY();

	return 1;
}

int retrycount;
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
		
		mpDebugPrint("%s retry number: %d", __FUNCTION__, retrycount);
		mpDebugPrint("Reg: 0x%04x, data: (wanted)0x%04x != (actual)0x%04x",addr, data, tempRegisterValue);
		mpDebugPrint("ALC5621 got wrong ack value...and latch up");
		return 0;	
}

static DWORD gpcfg1;
static DWORD gpdat1;
// Set the GPIO condition for I2S communication
void ALC5621_GPIO_Init()
{
	g_psGpio->Gpcfg0  &= 0xfffcfffc;		//set gp0~1 as default function
	g_psGpio->Gpdat0  |= 0x00030000;		//Output
	mpDebugPrint("Cfg: %x, dat: %x", g_psGpio->Gpcfg0, g_psGpio->Gpdat0);
}

#else  // using hardware I2C module insted of using software polling method
// 2-wire serial control mode
BYTE _setRegister2(WORD addr, WORD data)
{
	SDWORD readBack;

	I2CM_WtReg8Data16(ALC5621_DEVICE_ADDRESS << 1, (BYTE)addr, data);
	readBack = I2CM_RdReg8Data16(ALC5621_DEVICE_ADDRESS << 1, (BYTE)addr);

	//MP_ALERT("%s - Addr = 0x%04X, 0x%04X, 0x%04X", __FUNCTION__, (DWORD) addr, (DWORD) data, (DWORD) readBack);

	return 0;
}

BYTE _readRegister(WORD addr, volatile DWORD *data)
{
	*data = I2CM_RdReg8Data16(ALC5621_DEVICE_ADDRESS << 1, (BYTE)addr);
	return 0;
}
#endif

/*	  
 *    BoostGain:	0:bypass, 1:20dB, 2:30dB
 *    ADCGain:		0 to 31 ( -16.5 to +30 dB )
 *    MicVol: 		31 to 0 ( -34.5 to +12 dB )
 *	  Digital Volume Boost: 0x00:0dB, 0x01:6dB, 0x10:12dB, 0x11:18dB
 *	  Note: First increase Digital Volume to maximum and the others
 */
//void ALC5621_MicGain(DWORD BoostGain, DWORD ADCGain, DWORD MicVol)
void ALC5621_MicGain(DWORD BoostGain, DWORD ADCGain, DWORD digital_vol_boost)
{
	MP_DEBUG("\n%s",__func__);
	mpDebugPrint("BoostGain: %d,ADCGain: %d, digital_vol_boost: %d",BoostGain, ADCGain, digital_vol_boost);
	int dB;
	volatile DWORD tempRegisterValue;

// Boost Gain	(Reg 0x12)
	dB = (BoostGain+(1*BoostGain))*10;
	//BoostGain = (BoostGain << 10) | (BoostGain << 8);				//mic1 and mic2
	BoostGain = (BoostGain << 10);							//mic1
#if 0
	_setRegister2_w_retry(ALC5621_MIC_CTRL,BoostGain|(0x1<<5));
	mpDebugPrint("########Mic Bias: 0.75 AVDD");
#else
	_setRegister2_w_retry(ALC5621_MIC_CTRL,BoostGain);
	MP_DEBUG("########Mic Bias: 0.9 AVDD");
#endif

	_readRegister(ALC5621_MIC_CTRL,&tempRegisterValue);
	MP_DEBUG("  ALC5621 MIC Boost: %ddB; ALC5621_MIC_CTRL: 0x%04x",dB,tempRegisterValue);
	
// ADC Record Gain (Reg 0x22)
	dB = (ADCGain*1.5)-16.5;
	ADCGain = 0xf000 | ( ADCGain << 7 ) | ADCGain;
	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,ADCGain);
	_readRegister(ALC5621_ADC_REC_GAIN,&tempRegisterValue);
	MP_DEBUG("  ALC5621 ADC Rec Gain: %ddB; ALC5621_ADC_REC_GAIN: 0x%04x",dB,tempRegisterValue);

/*//Mic Volume (Reg 0x0e)
	MicVol = ( (MicVol << 8) | MicVol);							//mic1 and mic2
	MicVol = (MicVol << 8) | 0x1f ;								//mic1 (mic2 at min volume)
	_setRegister2_w_retry(ALC5621_MIC_VOL, MicVol);
	*/
#if 1
// Digital Volume (Reg 0x40)
	_readRegister(ALC5621_ADD_CTRL_REG,&tempRegisterValue);
	tempRegisterValue &= 0xff00;
	_setRegister2_w_retry(ALC5621_ADD_CTRL_REG, tempRegisterValue | (digital_vol_boost << 4));
	_readRegister(ALC5621_ADD_CTRL_REG,&tempRegisterValue);
	MP_DEBUG("ALC5621 Digital Volume: 0x%04x",tempRegisterValue);
#endif

}

extern SDWORD EmbededAdc_ValueGet(E_ADC_GROUP_INDEX group);
/*
 *    Mapping 16 level of UI volume value to hardware value
 */
int ALC5621_ChgVolume(DWORD vol)
{
	MP_DEBUG("\n%s",__func__);

	DWORD vol1, vol2, dB;
	volatile DWORD tempRegisterValue;
	BYTE *pbRightVol = NULL;
	BYTE *pbLeftVol = NULL;
	int i;
	#if 0
	SDWORD adcValue;
	if (!ALC5621_RecordMode) //change volume setting for different SPKVDD voltage
	{
		adcValue = EmbededAdc_ValueGet(E_ADC_GROUP_INDEX_0);	//detect battery actual voltage
		//MP_DEBUG("adcValue: %d",adcValue);
		_readRegister(ALC5621_ADD_CTRL_REG,&tempRegisterValue);
		tempRegisterValue &= 0x00ff;
		if (adcValue <= 6)	//SPKVDD < 4V
		{
			mpDebugPrint("ALC5621 SPKVDD Voltage Below 4V");
			_setRegister2_w_retry(ALC5621_ADD_CTRL_REG,0x5F00 | ( 0x3 << 8) | tempRegisterValue); 			//Class AB/D 1.0Vdd ratio for SPKVDD=3.3V
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCA0A);
		}
		if (adcValue >= 7)	//SPKVDD > 4V
		{
			mpDebugPrint("ALC5621 SPKVDD Voltage Above 4V");
			_setRegister2_w_retry(ALC5621_ADD_CTRL_REG,0x4B00| ( 0x3 << 8) | tempRegisterValue); 			//Class AB/D 1.0Vdd ratio for SPKVDD=4.2V
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xC909);
		}
	}
	#endif

	vol = vol & 0x1f;

	//vol1 = (32-vol*2-4);
	vol1 = (32-vol*2-8);

	MP_DEBUG("vol=%d, vol1=%d",vol,vol1);
	if (ALC5621_RecordMode==FALSE) // Set SPK volume, Max: 0dB, Min:-46.5dB
	{
		mpDebugPrint("ALC5621 Volume Level: %d", vol);
		if(vol==16)
		{
		    _setRegister2_w_retry(ALC5621_SPK_OUT_VOL, 0x0);	// max volume
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xC303);
			//_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xC808);//new settings, 2012.10.30
		}
		else if (vol==15)
		{
			_setRegister2_w_retry(ALC5621_SPK_OUT_VOL, 0x0);	// max volume
//			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xC808);
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xC606);//new settings, 2012.10.30
			
		}
        else if (vol==14)
		{
			_setRegister2_w_retry(ALC5621_SPK_OUT_VOL, 0x0);	// max volume
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xC808);//new settings, 2012.10.30
			
		}
        else if (vol==13)
		{
			_setRegister2_w_retry(ALC5621_SPK_OUT_VOL, 0x0);	// max volume
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCA0A);//new settings, 2012.10.30
			
		}
		else if (vol==0)//(vol1>=31)
		{	
			_setRegister2_w_retry(ALC5621_SPK_OUT_VOL, 0x8080); // mute mode
		}
		else
		{
			vol2 = ( (vol1 << 8) | vol1);
			_setRegister2_w_retry(ALC5621_SPK_OUT_VOL, vol2);
//			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCA0A);
			_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCC0C);//new settings, 2012.10.30
		}
		
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL, vol2);  //max volume = 0, mute=0x27= -46.5db
		_readRegister(ALC5621_SPK_OUT_VOL,&tempRegisterValue);

		
		if (vol1<=0)
		{
			dB = 0;
			MP_DEBUG("ALC5621 SPK Volume: %ddB (MAX), (0x%04x)",dB,tempRegisterValue);
		}
		else if (vol1>=31)
		{
			dB = 31*(1.5);
			MP_DEBUG("ALC5621 SPK Volume: -%ddB (MIN), (0x%04x)",dB,tempRegisterValue);
		}
		else
		{
			dB = 46.5-((32)-vol1)*(1.5);
			MP_DEBUG("ALC5621 SPK Volume: -%ddB, (0x%04x)",dB,tempRegisterValue);
		}

		
	}
	else								// Set Mic volume, Max:+12dB, Min:-34.5dB
	{
		mpDebugPrint("ALC5621 Mic Volume Level: %d", vol);
#if 1
        if (vol==0)
            _setRegister2_w_retry(ALC5621_MIC_VOL, 0x1f1f);
        else
            _setRegister2_w_retry(ALC5621_MIC_VOL, 0x0000);
#else
		if(vol1<=0)
		{
			_setRegister2_w_retry(ALC5621_MIC_VOL, 0x0);	// max volume
		}
		else if (vol1>=31)
		{
			_setRegister2_w_retry(ALC5621_MIC_VOL, 0x1f1f); // min volume (-34.5dB)
		}
		else
		{
			vol2 = ( (vol1 << 8) | vol1);//mic1 and mic2
//			vol2 = ( (vol1 << 8));//mic1
			_setRegister2_w_retry(ALC5621_MIC_VOL, vol2);
		}

		_readRegister(ALC5621_MIC_VOL,&tempRegisterValue);
		if (vol1<=8)
		{
			dB = 12-(vol1)*(1.5);
			MP_DEBUG("ALC5621 MIC Volume: %ddB, (0x%04x)",dB,tempRegisterValue);
		}
		else
		{
			dB = (vol1-1)*(1.5)-12;
			MP_DEBUG("ALC5621 MIC Volume: -%ddB, (0x%04x)",dB,tempRegisterValue);
		}
#endif
	}

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

void ALC5621_Set_DAC_EQ(DWORD sRate)
{
#if ALC5621_DAC_W_EQ	// EQ with lowpass and highpass filter
#if ALC5621_DAC_W_EQ_INCREASE_BASS
	MP_DEBUG("ALC5621 DAC EQ: +12dB for freq. below 100Hz, for %d Hz sample rate", sRate);
#endif

	#if ALC5621_DAC_W_EQ_20DB_DECADE
					// -20dB/decade
	MP_DEBUG("ALC5621 DAC EQ: using -20dB/decade, for %d Hz sample rate", sRate);
	_setRegister2_w_retry(0x62,0xC013);    //Control register
	_setRegister2_w_retry(0x66,0x0013);    // EQ mode change en
	_setRegister2_w_retry(0x6A,0x0011);    //index address: input volume
	_setRegister2_w_retry(0x6C,0x0000);    //index value: input volume
	_setRegister2_w_retry(0x6A,0x0012);    //index address: output volume
	_setRegister2_w_retry(0x6C,0x0001);    //index value: output volume
	#else 			//Shelving Filter
	MP_DEBUG("ALC5621 DAC EQ: using Shelving Filter, for %d Hz sample rate", sRate);
	_setRegister2_w_retry(0x62,0x8001);    //Control register
	_setRegister2_w_retry(0x66,0x0001);    // EQ mode change en
	_setRegister2_w_retry(0x6A,0x0011);    //index address: input volume
	_setRegister2_w_retry(0x6C,0x0000);    //index value: input volume
	_setRegister2_w_retry(0x6A,0x0012);    //index address: output volume
	_setRegister2_w_retry(0x6C,0x0001);    //index value: output volume
	#endif

if ((sRate > 7000 ) && (sRate < 11000))			// Set 8kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1D94);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0x2000);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xE001);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1757);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xE000);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xE001);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 11000) && (sRate < 11300))	// Set 11.025kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1E39);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);	   //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0x06DA);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x2F9A);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xEBF7);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x3B84);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x197A);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xD653);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x2F9A);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xEBF7);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1180);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1E5D);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);	   //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x3279);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x776C);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x19F5);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xD44A);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x3279);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1279);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 14000) && (sRate < 22000))	// Set 16kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1EC4);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xEB93);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x25BF);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x3151);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0xB2BF);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1B5C);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xCEAF);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x25BF);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x3151);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1561);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1F19);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xDB44);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x07C0);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x2EE8);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0xE835);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1C90);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xCA4A);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x07C0);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x2EE8);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 23000) && (sRate < 30000))	// Set 24kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1F2C);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xD803);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x27FD);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0xED87);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1CD4);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC960);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x27FD);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x188D);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1F60);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x02FB);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xCF8F);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xE9B5);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x0B5D);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0xF9A3);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1D97);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC6DC);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xE9B5);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x0B5D);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1F8C);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC9A4);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xD8CB);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xEF01);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x0257);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1E3C);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC4DF);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xD8CB);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xEF01);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: HP0:a1
	//_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	//_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
{
	#if ALC5621_DAC_W_EQ_INCREASE_BASS
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1F95);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0x2FB2);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC883);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xD588);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xE904);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x0436);    //index value: HP0:a1
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:H
	#else
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1E5F);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC474);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x0699);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xD588);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xE904);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: HP0:a1
//	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
//	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
#else
{
	MP_ALERT("Samplerate %d is not supported EQ.(8K--48K support)", sRate);
}

#endif	//ALC5621_DAC_W_EQ
}

void ALC5621_Set_ADC_EQ(DWORD sRate)
{
#if ALC5621_ADC_W_EQ	// EQ with lowpass and highpass filter
	MP_DEBUG("ALC5621 DAC EQ: -12dB for freq. below 100Hz, for %d Hz sample rate", sRate);

	#if ALC5621_ADC_W_EQ_20DB_DECADE
					// -20dB/decade
	MP_DEBUG("ALC5621 ADC EQ: using -20dB/decade, for %d Hz sample rate", sRate);
	_setRegister2_w_retry(0x62,0xC813);    //Control register
	_setRegister2_w_retry(0x66,0x0013);    // EQ mode change en
	_setRegister2_w_retry(0x6A,0x0011);    //index address: input volume
	_setRegister2_w_retry(0x6C,0x0000);    //index value: input volume
	_setRegister2_w_retry(0x6A,0x0012);    //index address: output volume
	_setRegister2_w_retry(0x6C,0x0001);    //index value: output volume
	#else 			//Shelving Filter
	MP_DEBUG("ALC5621 ADC EQ: using Shelving Filter, for %d Hz sample rate", sRate);
	_setRegister2_w_retry(0x62,0x8813);    //Control register
	_setRegister2_w_retry(0x66,0x0001);    // EQ mode change en
	_setRegister2_w_retry(0x6A,0x0011);    //index address: input volume
	_setRegister2_w_retry(0x6C,0x0000);    //index value: input volume
	_setRegister2_w_retry(0x6A,0x0012);    //index address: output volume
	_setRegister2_w_retry(0x6C,0x0001);    //index value: output volume
	#endif

if ((sRate > 7000 ) && (sRate < 11000))			// Set 8kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1757);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xD7AE);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xE001);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x0D41);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1D94);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 11000) && (sRate < 11300))	// Set 11.025kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x197A);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xD15E);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x2F9A);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xEBF7);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1180);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1E39);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x19F5);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xCFFF);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x3279);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1279);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1E5D);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 14000) && (sRate < 22000))	// Set 16kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1B5C);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xCC18);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x25BF);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x3151);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1561);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1EC4);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1C90);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC8D9);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x07C0);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x2EE8);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x17F8);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1F19);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 23000) && (sRate < 30000))	// Set 24kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1CD4);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC825);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x27FD);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x188D);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1F2C);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1D97);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC625);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xE9B5);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0x0B5D);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1A43);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1F60);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1E3C);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC47C);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01F3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xD8CB);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xEF01);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1BBC);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1F8C);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
{
	_setRegister2_w_retry(0x6A,0x0000);    //index address: LP0:a1
	_setRegister2_w_retry(0x6C,0x1E5F);    //index value: LP0:a1
	_setRegister2_w_retry(0x6A,0x0001);    //index address: LP0:Ho
	_setRegister2_w_retry(0x6C,0xF405);    //index value: LP0:Ho
	_setRegister2_w_retry(0x6A,0x0002);    //index address: BP1:a1
	_setRegister2_w_retry(0x6C,0xC420);    //index value: BP1:a1
	_setRegister2_w_retry(0x6A,0x0003);    //index address: BP1:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP1:a2
	_setRegister2_w_retry(0x6A,0x0004);    //index address: BP1:Ho
	_setRegister2_w_retry(0x6C,0x01f3);    //index value: BP1:Ho
	_setRegister2_w_retry(0x6A,0x0005);    //index address: BP2:a1
	_setRegister2_w_retry(0x6C,0xD588);    //index value: BP2:a1
	_setRegister2_w_retry(0x6A,0x0006);    //index address: BP2:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP2:a2
	_setRegister2_w_retry(0x6A,0x0007);    //index address: BP2:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP2:Ho
	_setRegister2_w_retry(0x6A,0x0008);    //index address: BP3:a1
	_setRegister2_w_retry(0x6C,0xE904);    //index value: BP3:a1
	_setRegister2_w_retry(0x6A,0x0009);    //index address: BP3:a2
	_setRegister2_w_retry(0x6C,0x1C10);    //index value: BP3:a2
	_setRegister2_w_retry(0x6A,0x000A);    //index address: BP3:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: BP3:Ho
	_setRegister2_w_retry(0x6A,0x000B);    //index address: HP0:a1
	_setRegister2_w_retry(0x6C,0x1F95);    //index value: HP0:a1
	#if !ALC5621_ADC_W_EQ_20DB_DECADE
	_setRegister2_w_retry(0x6A,0x000C);    //index address: HP0:Ho
	_setRegister2_w_retry(0x6C,0x0000);    //index value: HP0:Ho
	#endif
}
#else
{
	MP_ALERT("Samplerate %d is not supported ADC EQ.(8K--48K support)", sRate);
}

#endif	//ALC5621_ADC_W_EQ
}


// The crystal frequency is based on 12 HZ
void ALC5621_SetDACSampleRate(DWORD sRate)
{
	register CLOCK *audio_clock;
	int data;
	audio_clock = (CLOCK *) (CLOCK_BASE);
	audio_clock->PLL3Cfg = 0x00770107;

	#if AUDIO_DAC_USING_PLL2
	{
		if((PLL2CLOCK == 120)|| (PLL2CLOCK == 153) || (PLL2CLOCK == 147))// || (PLL2CLOCK == 144)  || //(PLL2CLOCK == 160) || (PLL2CLOCK == 166) )
		{
			mpDebugPrint("ALC5621 using PLL2 Clock Freq: %d", PLL2CLOCK);
		}
		else
		{
			MP_ALERT("ALC5621 doesn't support PLL2 %d; Only supports PLL2: 147, 153, 120(8khz, 12khz).", PLL2CLOCK);
		}
	}
	#else
	{
		mpDebugPrint("ALC5621 using PLL3 Clock. PLL3 clock IS NOT stable!!");
		MP_ALERT("ALC5621, not all Sample Rates using PLL3 haven't been tested yet!!!");
	}
	#endif

	mpDebugPrint("ALC5621 DAC set Samplerate: %d", sRate);
	
	if ( (sRate > 7000)  && (sRate < 11000))	// Set 8kHz status
	{
		//_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x0631); //0x166d);//	
										//Adviced clock: MCLK:2.048Mhz, BCLK:512Mhz or 256Mhz
		#if AUDIO_DAC_USING_PLL2
			if	(PLL2CLOCK == 120)
				{
#if 1
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d);
				audio_clock->AudClkC = 0x0000135d;//120/6/10 = 2Mhz
				AIU_SET_LRCLOCK_LEN(0x7e); 
#else
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x0631);
	        	audio_clock->AudClkC = 0x0000139d;//120/6/10 = 2Mhz
	        	AIU_SET_LRCLOCK_LEN(0x3e); 
#endif
				}
			else if	(PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d);
				audio_clock->AudClkC = 0x000013bd;	//147/6/12=2.041Mhz
				AIU_SET_LRCLOCK_LEN(0x3e); 			//7.975khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d);//Mclk(4.096Mhz)/2/256 ~=8khz ,256fs by default
				audio_clock->AudClkC = 0x000013ca;
				AIU_SET_LRCLOCK_LEN(0x7e);
				}
		#else	//not tested yet
			audio_clock->PLL3Cfg = 0x00777c7f;	//PLL3 = 12.288Mhz
			audio_clock->AudClkC = 0x00001230;	//Mclk = 4.096Mhz, Bclk = 1.024Mhz
			AIU_SET_LRCLOCK_LEN(0x7F);			//LRClk = 8khz
		#endif	

	}
	else if((sRate >= 11000) && (sRate < 11300))	// Set 11.025kHz status
	{
		//_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x300d); 	//Mclk(2.8224Mhz)/32/1/1/8  ~=11.025khz ,256fs by default
																//Adviced clock: MCLK:5.6448Mhz
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d); 
				audio_clock->AudClkC = 0x000011C9;	//147/2/13 = 5.653Mhz
				AIU_SET_LRCLOCK_LEN(0xFE);			//11.042khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d); 
				audio_clock->AudClkC = 0x0000118a;	//153/3/9 = 5.666Mhz
				AIU_SET_LRCLOCK_LEN(0xff);			//11.024khz
				}
		#else
			audio_clock->PLL3Cfg = 0x00772fcb;
			audio_clock->AudClkC = 0x00001070;	
			AIU_SET_LRCLOCK_LEN(0x1F);//11.029khz
		#endif	

	}
	else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
	{
		//_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x0631); 	//Mclk(3Mhz)/1/8/1/32 ~=12khz ,256fs by default
																//Adviced clock: MCLK:6.144Mhz
		#if AUDIO_DAC_USING_PLL2
			if	(PLL2CLOCK == 120)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d); 
				audio_clock->AudClkC = 0x0000169b;//120/4/10 = 3Mhz, BCLK= /7=428
				AIU_SET_LRCLOCK_LEN(0x22);		//LCR=12.24kHz	
				}
			else if (PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166D); 
				audio_clock->AudClkC = 0x000011B9;	//147/2/12 = 6.125Mhz
				AIU_SET_LRCLOCK_LEN(0xFE);			//LRC=11.962Khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166D); 
				audio_clock->AudClkC = 0x0000114C;//153/5/5 = 6.12Mhz
				AIU_SET_LRCLOCK_LEN(0xFE);			//LRC=11.923Khz
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
		#if 1
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
			else if (PLL2CLOCK == 147)
			{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d);//256FS
				audio_clock->AudClkC = 0x000013BA;	//147/3/12 = 4.083Mhz
				AIU_SET_LRCLOCK_LEN(0x3E); 			// lrc= 15.950khz
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

		#else
			if	(PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d);//256FS
				audio_clock->AudClkC = 0x000013BA;	//147/3/12 = 4.083Mhz
				AIU_SET_LRCLOCK_LEN(0x3E); 			// lrc= 15.950khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066f); //384fs
				audio_clock->AudClkC = 0x0000134C;//153/5/4 = 6.12Mhz
				AIU_SET_LRCLOCK_LEN(0x5E);			//LRC=15.937Khz
				}
			#endif
		#else									//MCLK, BCLK not stable
			audio_clock->PLL3Cfg = 0x00777c7f;	//PLL3 = 12.288 Mhz
			audio_clock->AudClkC = 0x00001120;	//MCLK =  4.096Mhz, BCLK = 2.048Mhz
			AIU_SET_LRCLOCK_LEN(0x7f);			//LRCLK = 16 Khz
		#endif	
	}
	else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
	{
		//_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x300d);	//Mclk(5.644Mhz)/32/1/1/8 ~=22.05khz ,256fs by default
																//Adviced clock: MCLK:5.644Mhz, BCLK:705Khz
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x000013c9;	//147/2/13 = 5.653Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//22.085Khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x0000138A;//153/6/8 = 5.666Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//22.135Khz
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
			if (PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x000013b9;	//147/2/12 = 6.125Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//29.925khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x0000134c;//153/6/2 = 12.75Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);
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
			if (PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x0000135a;	//147/2/9 = 8.166Mhz
				AIU_SET_LRCLOCK_LEN(0x3e); 			//31.901khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x0000135a;//153/6/2 = 12.75Mhz
				AIU_SET_LRCLOCK_LEN(0x3e); 
				}
		#else	//not tested
			audio_clock->PLL3Cfg = 0x007718cc;
			audio_clock->AudClkC = 0x00001270;	
			AIU_SET_LRCLOCK_LEN(0x7F);
		#endif	
	}
	else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
	{
		_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x166d);//0x3011);	//Mclk(11.2896Mhz)/8/1/1/32 ~=44.1khz ,256fs by default
													//Adviced clock: MCLK:11.2896Mhz, BCLK:	1.4112Khz
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 147)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x000013c8;//147/1/13 = 11.307Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//44.17Khz
				}
			else if (PLL2CLOCK == 153)
				{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x000013d8;//153/1/14 = 10.9285714Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//42.689Khz
				}
		#else //not tested
			audio_clock->PLL3Cfg = 0x00777d85;
			audio_clock->AudClkC = 0x00001070;	
			AIU_SET_LRCLOCK_LEN(0x1F);				//44.117khz
		#endif	
	}
	else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
	{
		//_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x0631); 	//Mclk(3Mhz)/1/8/1/32 ~=48khz ,256fs by default
											//Adviced clock: MCLK:3.072Mhz, BCLK:1.536Khz or 3.072Mhz
		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 147)
			{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x000013b8;//147/1/12 = 12.75Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);		//47.851Khz
			}
			else if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x000013b8;//153/1/12 = 12.75Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);		//49.8Khz
			}
		#else	//not tested
			audio_clock->PLL3Cfg = 0x007718cc;//MCLK ==12.288Mhz
			audio_clock->AudClkC = 0x00001070;		
			AIU_SET_LRCLOCK_LEN(0xFF);
		#endif	
	}
	else if((sRate >= 64000) && (sRate < 97000))	// Set 96kHz status
	{

		#if AUDIO_DAC_USING_PLL2
			if (PLL2CLOCK == 147)
			{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x00001358;	//147/1/6 = 24.5Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//95.703Khz
			}
			else if (PLL2CLOCK == 153)
			{
				_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066d);
				audio_clock->AudClkC = 0x00001358;	//153/1/6 = 25.5 Mhz
				AIU_SET_LRCLOCK_LEN(0x3e);			//99.609Khz
			}
		#else
			goto not_supported_samplerate;
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
		MP_DEBUG("ALC5621: PLL3 disabled");
	#else
		g_psClock->Clkss_EXT1 &= 0xfffffff0;
		g_psClock->Clkss_EXT1 |= (BIT3);			// Using PLL3/1 to be clock source
				
		//	g_psClock->ClkCtrl &= ~0x01000000;		// disable PLL3
		g_psClock->ClkCtrl |= 0x01000000;			// enable PLL3
		MP_DEBUG("ALC5621: PLL3 enabled");
	#endif

	MP_DEBUG("ALC5621 g_psClock->Clkss_EXT1: 0x%x",g_psClock->Clkss_EXT1);

#if 0	//to check if sample rate is correct from oscilloscope
	__asm("break 100");
#endif

#if ALC5621_DAC_W_EQ
	ALC5621_Set_DAC_EQ(sRate);
#endif
	
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

}



// Setting AIU registers and reset sample rate 
int  ALC5621_PlayConfig(DWORD sampleRate)
{
	MP_DEBUG("%s",__func__);
#if (ALC5621_USINGHWI2C == ENABLE)
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x0077ffc3;
	g_psClock->AudClkC = 0x00001110;
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	//g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3
	I2CM_FreqChg(ALC5621_DEVICE_ADDRESS << 1, 300000);	// Set correct I2C frequency for enabing I2C setting
#else
	ALC5621_GPIO_Init();
#endif
	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();

	ALC5621_SetDACSampleRate(sampleRate); 
	AIU_I2S_AUDIO_INTERFACE();

	return PASS;
}

int ALC5621_setPlayback()
{
	MP_DEBUG("%s",__func__);
//	return PASS;
}

int ALC5621_uninit()
{
	MP_DEBUG("Uninit ALC5621");
#if 1	//power off
	ALC5621_ChgVolume(0);//for spk
	ALC5621_RecordMode=TRUE;
	ALC5621_ChgVolume(0);//for mic
	ALC5621_RecordMode=FALSE;
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x0000);
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x0000);
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x0000);
	return PASS;
#else	//idle mode
	ALC5621_ChgVolume(0);//for spk
	ALC5621_RecordMode=TRUE;
	ALC5621_ChgVolume(0);//for mic
	ALC5621_RecordMode=FALSE;
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x0000);
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x2000);	//only main bias on
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8000);	//only Vref on
	return PASS;
#endif
	
	MP_DEBUG("Uninit ALC5621 Finished!");
}

/**
 ***************************************************************
 *
 * Config ALC5621 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void ALC5621_AIUCfg(void)
{
	MP_DEBUG("ALC5621_AIUCfg");
	// Master clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->PLL3Cfg = 0x007718cc;
	g_psClock->AudClkC = 0x00001770;
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

#if (ALC5621_USINGHWI2C == ENABLE)
	I2CM_FreqChg(ALC5621_DEVICE_ADDRESS << 1, 300000);
#else
	ALC5621_GPIO_Init();
#endif

}

int ALC5621_InitRecordMode()
{
	mpDebugPrint("\nALC5621 Audio Record Init... Start");
	volatile DWORD data;
	ALC5621_RecordMode = TRUE;
	
#if SOFT_RECORD	 // It just temporialy used #if to distinguish left and right justified, we will remove it as soon as possible 2010/05/26 XianWen note
	mpDebugPrint("-I- SOFT_RECORD not supported");
#endif	

#if 1	//MIC settings: MIC1 -> ADC Mixer -> I2S out

#if ALC5621_ADC_BYPASS
	mpDebugPrint("ALC5621_ADC_BYPASS");
	#if 1
	//DAC->MONO Mixer -> SPK out
		_setRegister2_w_retry(ALC5621_AUDIO_INTERFACE,0x8000);
		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCA0A);		//mono mixer on for DAC
		_setRegister2_w_retry(ALC5621_OUTPUT_MIXER_CTRL,0x8C00);
	
		_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0000); 	//spk 0db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0808); 	//spk -12db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0c0c); 		//spk -18db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0f0f);	 	//spk -22.5db(half volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x1f1f);		//spk -46db(min volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x8080); 	//spk 0db, muted
	#endif
	
	#if 0	//Power for Codec Init & Spk & Mic
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,(0x8800|0x8000));//spk on
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8800);
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8c00); //micbias current detector enable
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0xffff);
	
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,(0x8002|0x9000|0xf));//spk on
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8002);
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x800A);
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x800F);//mic1 and m2 enable
	
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,(0x20C3|0xA704));
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x20C3);
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0xffff);
	#endif	

		//change path
//		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xc0e0);		// not differential MIC1, mono mixer on and others muted (hp, speaker mono muted), to record and listen
		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xd0d0);		// differential MIC1 MIC2, mono mixer on and others muted (hp, speaker mono muted), to record and listen
//		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xd0e0);		// differential MIC1, mono mixer on and others muted (hp, speaker mono muted), to record and listen
		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xeC0C);		//mono mixer off, to record and listen
#else

	#if 0	//Power for Codec Init & Mic
		MP_DEBUG("ALC5621: Power for Codec Init & Mic");
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8800);
	
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8002);
	//	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x800F);//mic1 and m2 enable
	
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x20C3);
	#endif
	
//		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xc0e0);		// not differential MIC1, mono mixer on and others muted (hp, speaker mono muted), to record and listen
//		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xd0d0);		// differential MIC1 MIC2, mono mixer on and others muted (hp, speaker mono muted), to record and listen
//		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xd0e0);		// differential MIC1, mono mixer on and others muted (hp, speaker mono muted), to record and listen
		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xf0e0);		// differential MIC1 and others muted (mono mixer, hp, speaker mono muted), to record and listen
//		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xeC0C);			//mono mixer off, to record a listen
//		_setRegister2_w_retry(ALC5621_MIC_ROUTING_CTRL,0xe0e0);		// single ended MIC1/MIC2 and others muted (mono mixer, hp, speaker mono muted)
//		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xf0f0);			//differential MIC1 MIC2, others mixer off
#endif

#if 1
	ALC5621_MicGain(1, 22, 3);
	ALC5621_ChgVolume(16);		// set MicVol: 	0 to 16 ( -34.5 to +12 dB )
#else

	//	_setRegister2_w_retry(ALC5621_MIC_CTRL,(0x0000|0x10)); 				//bypass with micbias at 0.75avdd
	//	_setRegister2_w_retry(ALC5621_MIC_CTRL,0x0000); 					//bypass
		_setRegister2_w_retry(ALC5621_MIC_CTRL,0x0400);					//20db boost
	//	_setRegister2_w_retry(ALC5621_MIC_CTRL,0x0a00);					//30db boost	
	
	//	_setRegister2_w_retry(ALC5621_ADC_REC_MIXER,0x3F3F);		//adc recorder mixer control: MIC1 ON, by realtek
		_setRegister2_w_retry(ALC5621_ADC_REC_MIXER,0x1F1F);	//adc recorder mixer control: MIC1 ON, by realtek
	//	_setRegister2_w_retry(ALC5621_ADC_REC_MIXER,0x7F7F);		//adc recorder mixer control: all off
		//_setRegister2_w_retry(ALC5621_ADC_REC_MIXER,0x0);
	
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xFFFF);		//ADC gain at max and only zero-crossing detector --> no good
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0x1c99);		// output to all mixers
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xFF9F);
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xF912);		//by realtek, no output
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0x058b);		//output to hp and mono; to record and listen at the same time
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xF5EB);		//only zero-crossing detector
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xFF9F);		//gain at max and no zero-crossing detector -->no good
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xfa14);//0xF000|0x14|(0x14<<7));//gain 20
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xF000|0x19|(0x19<<7));//ADC gain 25
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xF000|0x16|(0x16<<7));//ADC gain 23
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xfa14|0x2);//0xF000|0x14|(0x14<<7));//ADC gain 20
	//	_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xfc99);//0xF000|0x7|(0x7<<7));//ADC gain 7
		_setRegister2_w_retry(ALC5621_ADC_REC_GAIN,0xfc99); 	//ADC gain +21dB

	//	_setRegister2_w_retry(ALC5621_MIC_VOL,0x001f);		//mic1 at max volume(12db), mic2 mute
		_setRegister2_w_retry(ALC5621_MIC_VOL,0x081f);		//mic1 at volume(0db), mic2 mute
	//	_setRegister2_w_retry(ALC5621_MIC_VOL,0x1f1f);		//mic1 & mic2 at min volume (-34.5db)
	//	_setRegister2_w_retry(ALC5621_MIC_VOL,0x0);		//mic1 and mic2 (12db)
	//	_setRegister2_w_retry(ALC5621_MIC_VOL,0x0808);		 //mic1 and mic2 at 0db
#endif	

#if ALC5621_ADC_W_AGC
	mpDebugPrint("\nRecording with AGC (AVC)");
//	_setRegister2_w_retry(ALC5621_AVC_CTRL,0x800B);
	_setRegister2_w_retry(ALC5621_AVC_CTRL,0x8020);
	
	_setRegister2_w_retry(0x6A,0x0021);//Auto Vol Control Reg 1 index_addr, Thmax
//	_setRegister2_w_retry(0x6C,0x0400);//Auto Vol Control Reg 1 index_data, Thmax (default value)
//	_setRegister2_w_retry(0x6C,0x0A00);//Auto Vol Control Reg 1 index_data, Thmax
	_setRegister2_w_retry(0x6C,0x1388);//Auto Vol Control Reg 1 index_data, Thmax
//	_setRegister2_w_retry(0x6C,0x0688);//Auto Vol Control Reg 1 index_data, Thmax

	_setRegister2_w_retry(0x6A,0x0022);//Auto Vol Control Reg 2 index_addr, Thmin
//	_setRegister2_w_retry(0x6C,0x0390);//Auto Vol Control Reg 2 index_data, Thmin (default value)
//	_setRegister2_w_retry(0x6C,0x0990);//Auto Vol Control Reg 2 index_data, Thmin
//	_setRegister2_w_retry(0x6C,0x12c0);//Auto Vol Control Reg 2 index_data, Thmin
	_setRegister2_w_retry(0x6C,0x1370);//Auto Vol Control Reg 2 index_data, Thmin

	_setRegister2_w_retry(0x6A,0x0023);//Auto Vol Control Reg 3 index_addr, Thnonact
//	_setRegister2_w_retry(0x6C,0x0001);//Auto Vol Control Reg 3 index_data, Thnonact (default value)
	_setRegister2_w_retry(0x6C,0x04e0);//Auto Vol Control Reg 3 index_data, Thnonact

/*	_setRegister2_w_retry(0x6A,0x0024);//Auto Vol Control Reg 4 index_addr, CNTMAXTH1
	_setRegister2_w_retry(0x6C,0x01FF);//Auto Vol Control Reg 4 index_data, CNTMAXTH1 (default value)

	_setRegister2_w_retry(0x6A,0x0025);//Auto Vol Control Reg 5 index_addr, CNTMAXTH2
	_setRegister2_w_retry(0x6C,0x0200);//Auto Vol Control Reg 5 index_data, CNTMAXTH2 (default value)
*/
#else
	MP_DEBUG("Recording without AGC (AVC)");
#endif

#if ALC5621_ADC_W_EQ   // EQ with lowpass and highpass filter
	ALC5621_Set_ADC_EQ(16000);
	//16kHz recording sample rate, it should be the same as AUDIO_OF_VIDEO_REC_SAMPLERATE and AUDIO_REC_SAMPLERATE
#else
	MP_DEBUG("Recording without EQ");
#endif 


#endif

	ALC5621_RecordMode = FALSE;

	mpDebugPrint(" ALC5621 Audio Record Init... Finished");

#if 1
	DWORD tempRegisterValue;
	mpDebugPrint("================================================");
	mpDebugPrint("== Dump_Register() in %s() ==", __func__);
	mpDebugPrint("================================================");
	Dump_Register(); 
#endif

	return PASS;

}

int AudioInit_ALC5621(void)
{

	MP_DEBUG("#########################");
	mpDebugPrint("Audio DAC ALC5621 Init...");
	MP_DEBUG("#########################");

	ALC5621_AIUCfg();

	_setRegister2_w_retry(ALC5621_RESET,0x59b4); 	//RESET

#if 1 
	//Codec init
	#if 0 //power for codec init
	MP_DEBUG("ALC5621: Power for Codec Init");
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x2000);					//enable Vref
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8000); 					//enable Mainbias
	#endif
	#if 1 //power for codec init, spk, mic
	MP_DEBUG("ALC5621: Power for Codec Init, Spk, Mic");
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8000|0x8800);			//spk|mic setting order
	//_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x2000|0xA704|0x20C3);		//codec|spk|mic setting order
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,(0x2000|0xA704|0x20C3)&0x3fff);     // grad modify
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8000|0x9000|0x800f);		//codec|spk|mic setting order
	#endif
	#if 0	//power for codec init, spk
	MP_DEBUG("ALC5621: Power for Codec Init & Spk");
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8000);				//spk setting order
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x2000|0xA704);		//codec|spk setting order
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8000|0x9000);		//codec|spk setting order
	#endif
	#if 0	//power for codec init, mic
	MP_DEBUG("ALC5621: Power for Codec Init, Mic");
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8800);				//mic setting order
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0x20C3);				//codec|mic setting order
	//_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x800f);				//mic1 and mic2 enable
	_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x800A);				//mic1 enable
	#endif

	_setRegister2_w_retry(ALC5621_OUTPUT_MIXER_CTRL,0x0740); 		//set output source by default
	_setRegister2_w_retry(ALC5621_ADC_REC_MIXER,0x3F3F); 			//set record source from Mic1 by default
	_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0x0808); 			//set DAC volume to 0dB and output all Mixer
//	_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0x2808); 				//set DAC volume to 0dB and output mono Mixer
//	_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x8888); 					//set HP out volume to ¡V12dB by default
	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x8080); 				//set Speaker volume mute
	_setRegister2_w_retry(ALC5621_AUDIO_INTERFACE,0x8000); 			//slave mode, 16bit
//	_setRegister2_w_retry(ALC5621_AUDIO_INTERFACE,0x800C); 			//slave mode, 32bit	
	_setRegister2_w_retry(ALC5621_STEREO_AD_DA_CLK_CTRL,0x066D); 	//256fs by default
//	_setRegister2_w_retry(ALC5621_ADD_CTRL_REG,0x5F00); 			//Class AB/D 1.0Vdd ratio for SPKVDD=3.3V
//	_setRegister2_w_retry(ALC5621_ADD_CTRL_REG,0x4B00); 			//Class AB/D 1.0Vdd ratio for SPKVDD=4.2V
	_setRegister2_w_retry(ALC5621_ADD_CTRL_REG,0x3700); 				//Class AB/D 1.5Vdd ratio, for SPKVDD=5V

#endif

	AudioPlayBack_ALC5621();
	
#if 1
	//GPIO
	//_setRegister2_w_retry(ALC5621_JACK_DET_CTRL,0x4F00);			//select gpio
	_setRegister2_w_retry(ALC5621_JACK_DET_CTRL,0x4FF0); 		//enable all output when gpio = HIGH and LOW
	//_setRegister2_w_retry(ALC5621_JACK_DET_CTRL,0x8F00);			//select jd1
	//_setRegister2_w_retry(ALC5621_JACK_DET_CTRL,0xcF00);			//select jd2
	_setRegister2_w_retry(ALC5621_GPIO_OUTPUT_PIN_CTRL,0x0002); //gpio drive high
	_setRegister2_w_retry(ALC5621_GPIO_PIN_SHARING,0xc000); 	//jd1, jd2, gpio
	_setRegister2_w_retry(ALC5621_GPIO_PIN_CONFIG,0x180e); 		//MIC bias bypass
#endif

// DAC Reg Test Code
#if 0
	mpDebugPrint("================================================");
	mpDebugPrint("== Dump_Register() in %s() ==", __func__);
	mpDebugPrint("================================================");
	Dump_Register(); 
#endif

	return PASS;
}

int AudioPlayBack_ALC5621(void)
{
	MP_DEBUG("%s",__func__);

#if 1
		//DAC->MONO Mixer -> SPK out
		MP_DEBUG("DAC -> MONO Mixer -> SPK out");
#if 0
		MP_DEBUG("Power for Codec Init & Spks (no MIC)");
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8000);
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0xA704);
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x9000);
#endif
		_setRegister2_w_retry(ALC5621_AUDIO_INTERFACE,0x8000);
		//_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCC0C);	//mono mixer on for DAC
		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0xCA0A);	//mono mixer on for DAC
		_setRegister2_w_retry(ALC5621_OUTPUT_MIXER_CTRL,0x8C00);
	
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0000);		//spk 0db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0808);		//spk -12db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0c0c);		//spk -18db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0f0f);		//spk -22.5db(half volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x1f1f);		//spk -46db(min volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x8080);		//spk 0db, muted
		
#endif
	
#if 0
		//DAC-> HP Mixer -> SPK out
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8000);
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0xA730);
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x9800);
		_setRegister2_w_retry(ALC5621_AUDIO_INTERFACE,0x8000);
		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0x6808);
		_setRegister2_w_retry(ALC5621_OUTPUT_MIXER_CTRL,0x0400);
		_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0000);	//spk 0db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0808);	//spk -12db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0c0c);	//spk -18db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x0f0f);	//spk -22.5db(half volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x1f1f);	//spk -46db(min volume), unmuted
	//	_setRegister2_w_retry(ALC5621_SPK_OUT_VOL,0x8080);	//spk 0db, muted
#endif
		
#if 0
		//DAC-> HP Mixer -> HP out
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD1,0x8030);
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2,0xA730);
		_setRegister2_w_retry(ALC5621_PWR_MANAG_ADD3,0x8600);
		_setRegister2_w_retry(ALC5621_AUDIO_INTERFACE,0x8000);
		_setRegister2_w_retry(ALC5621_STEREO_DAC_VOL,0x6808);
		_setRegister2_w_retry(ALC5621_OUTPUT_MIXER_CTRL,0xc300);
		_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x0000);	//hp 0db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x0808);	//hp -12db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x0c0c);	//hp -18db(max volume), unmuted
	//	_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x0f0f);	//hp -22.5db(half volume), unmuted
	//	_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x1f1f);		//hp -46db(min volume), unmuted
	//	_setRegister2_w_retry(ALC5621_HP_OUT_VOL,0x8080);	//hp 0db, muted
	
#endif

// DAC Reg Test Code
#if 0
	mpDebugPrint("================================================");
	mpDebugPrint("== Dump_Register() in %s() ==", __func__);
	mpDebugPrint("================================================");
	Dump_Register(); 	
#endif
}

#if 1           // add by grad
void ALC5621_AmpPowerOn()
{
    _setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2, 0x2000|0xA704|0x20C3);
}

void ALC5621_AmpPowerOff()
{
    _setRegister2_w_retry(ALC5621_PWR_MANAG_ADD2, (0x2000|0xA704|0x20C3) & 0x3fff);
}

#endif


HAL_AUDIODAC_T _audioDAC_ALC5621 =
{
	AudioInit_ALC5621,
	ALC5621_InitRecordMode,	
	ALC5621_uninit,
	ALC5621_PlayConfig,
	AudioPlayBack_ALC5621,
	ALC5621_ChgVolume
};

void Dump_Register(void)
{
#if LOCAL_DEBUG_ENABLE

	DWORD tempRegisterValue;
	WORD addr; 
	mpDebugPrint("======== start %s ========", __func__);

	for (addr=0; addr <= 0x7e; addr += 1)
	{
		_readRegister(addr,&tempRegisterValue);
	//	mpDebugPrint("addr(0x%02x) = %04x", addr, tempRegisterValue);
	}

	mpDebugPrint("======== end %s ========", __func__); 

#endif
}

#endif

