/**
 * FileName     AudioDac_CS4334.c
 *
 * Auther       C.W Liu
 * Date         2009.12.17
 *
 * Description  Driver for Wolfson CODEC CS4334
 * 
 **/ 
#define LOCAL_DEBUG_ENABLE 0

/*
 * Include section 
 */
#include "global612.h"
#include "audio_hal.h"
#include "mpTrace.h"

#if (AUDIO_DAC == DAC_CS4334)
/*
 *    Mapping 16 level of UI volune value to hardware value
 */
void CS4334_ChgVolume(WORD vol)
{

}

// The crystal frequency is based on 12 HZ
void CS4334_SetDACSampleRate(DWORD sRate)
{
	register CLOCK *audio_clock;
	int data;
	audio_clock = (CLOCK *) (CLOCK_BASE);

	if (	 (sRate > 7000)  && (sRate < 11000))	// Set 8kHz status
	{
		audio_clock->PLL3Cfg = 0x00774Acc;
		audio_clock->AudClkC = 0x000013F0;
		AIU_SET_LRCLOCK_LEN(0x3F);
	}
	else if((sRate >= 11000) && (sRate < 11300))	// Set 11.25kHz status
	{
		audio_clock->PLL3Cfg = 0x0077103F;
		audio_clock->AudClkC = 0x000013F0;	
		AIU_SET_LRCLOCK_LEN(0x3F);
	}
	else if((sRate >= 11300) && (sRate < 14000))	// Set 12kHz status
	{

		audio_clock->PLL3Cfg = 0x007731cc;
		audio_clock->AudClkC = 0x000013F0;
		AIU_SET_LRCLOCK_LEN(0x3F);		
	}
	else if((sRate >= 14000) && (sRate < 22000))	// Set 16kHz status
	{
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x000012F0;	
		AIU_SET_LRCLOCK_LEN(0x7F);		
	}
	else if((sRate >= 20000) && (sRate < 23000))	// Set 22.05Hz status
	{
		audio_clock->PLL3Cfg = 0x0077107F;//MCLK ==11.289Mhz
		audio_clock->AudClkC = 0x00001370;		
		AIU_SET_LRCLOCK_LEN(0x7F);
	}
	else if((sRate >= 22000) && (sRate < 30000))	// Set 24kHz status
	{
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001370;	
		AIU_SET_LRCLOCK_LEN(0x7F);
	}
	else if((sRate >= 30000) && (sRate < 36000))	// Set 32kHz status
	{
		audio_clock->PLL3Cfg = 0x007718cc;
		audio_clock->AudClkC = 0x00001270;	
		AIU_SET_LRCLOCK_LEN(0x7F);
	}
	else if((sRate >= 36000) && (sRate < 46000))	// Set 44.1kHz status
	{
		audio_clock->PLL3Cfg = 0x0077107F;
		audio_clock->AudClkC = 0x000011F0;	   //MCLK =SCLK=5.6448Mhz
		AIU_SET_LRCLOCK_LEN(0x3F); 

	}
	else if((sRate >= 46000) && (sRate < 64000))	// Set 48kHz status
	{
		audio_clock->PLL3Cfg = 0x007718cc;//MCLK ==12.288Mhz
		audio_clock->AudClkC = 0x000011F0;	
		AIU_SET_LRCLOCK_LEN(0x3F);
	}

	else if((sRate >= 64000) && (sRate < 97000))	// Set 96kHz status
	{
	}
	else
		MP_ALERT("Samplerate %d is not support  by IIS.(8K--48K support)", sRate);

	////_setRegister2(0x1e, 0x00);	// Auto clock configuration to CS4334

	// If you want to change PLL value, follow below steps
	// 1. setting PLL config.
	// 2. Disable PLL (ClkCtrl , Clkss_EXT1)
	// 3. Re-enable PLL(ClkCtrl , Clkss_EXT1)
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source
//	g_psGpio->Ugpcfg  &= 0xfffEfffE;		//set gp0~1 as default function
//	g_psGpio->Ugpdat |= 0x00010000;		//Output
//	g_psGpio->Ugpdat |= 0x00000001;

//	g_psClock->ClkCtrl &= ~0x01000000;		// Disable PLL3 
	g_psClock->ClkCtrl |= 0x01000000;		// enable PLL3

	AIU_I2S_AUDIO_INTERFACE();
	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();
}


// Setting AIU registers and reset sample rate 
void  CS4334_PlayConfig(DWORD sampleRate)
{
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
	g_psClock->Clkss_EXT1 &= 0xfffffff0;
	g_psClock->Clkss_EXT1 |= (BIT3);     // Using PLL3/1 to be clock source
	g_psClock->ClkCtrl &= ~0x01000000;   // Disable PLL3
	//g_psClock->ClkCtrl |= 0x01000000;    // enable PLL3
	AIU_PLAYBACK_GAI_INIT();
	AIU_SET_GENERAL_WAVEFORM();
	CS4334_SetDACSampleRate(sampleRate);
	AIU_I2S_AUDIO_INTERFACE();
}

void CS4334_setPlayback()
{
}

void CS4334_uninit()
{
}

/**
 ***************************************************************
 *
 * Config CS4334 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void CS4334_AIUCfg(void)
{
	// Mater clock and bit clock setting (If speed is not fast enough, hw i2c will fail.)
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
}

void CS4334_InitRecordMode()
{
	MP_ALERT("Not support record sound function");
}

void AudioInit_CS4334(void)
{
	CS4334_AIUCfg();
}

HAL_AUDIODAC_T _audioDAC_CS4334 =
{
	AudioInit_CS4334,
	CS4334_InitRecordMode,	
	CS4334_uninit,
	CS4334_PlayConfig,
	NULL,
	CS4334_ChgVolume
};
#endif
