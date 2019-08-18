/*
***********************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      audio_hal.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   HAL layer of mp6xx audio device
*
*
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
*     <2>     01/05/2010    C.W Liu      Modularize audio hal layer
***********************************************************************
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "peripheral.h"
#include "audio_hal.h"
#include "audio.h"
#if BLUETOOTH == ENABLE
#include "BtApi.h"
#endif

extern DWORD backward_flag;
extern DWORD forward_flag;

static HAL_AUDIODAC_T *_halAudioDac = NULL;

/* Function prototype */
static void AudioInit_DAC();
static unsigned char Audio_control=0;

/* Open Audio Interface */
static void _AudioIfOpen(void)
{
	register CLOCK *audio_clock;
	register GPIO *audio_gpio;
	register AIU *aiu;
	BIU * biu;

	aiu         = (AIU *)(AIU_BASE);
	biu         = (BIU *)BIU_BASE;
	audio_clock = (CLOCK *)(CLOCK_BASE);
	audio_gpio  = (GPIO *)(GPIO_BASE);

	#if (AUDIO_DAC!=DAC_INTERNAL || HAVE_AMP_MUTE)
	biu->BiuSrst &= 0xfffffdff;
	biu->BiuSrst |= 0x00000200;
	#endif
	aiu->AiuCtl |= 0x20000000; //clear fifo
	//AIUCLOCK_DISABLE();
	
	#if (AUDIO_DAC == DAC_AC97)
	// AC97 Use External Oscillator
	audio_clock->AudClkC = 0x00000000;

	// Select External AC97 main clock
	// Bit_Clock from AC97 Codec
	//audio_clock->MdClken |= 0x00020000;
	AC97TurnonAudioClk();

	/* AC97 Module Enable, Clear Fifo Buffer by the way */
	aiu->AiuCtl = 0xA0000000;

	if (g_bVolumeIndex & 0x40)	// volume is changed
		MX6xx_AudioSetVolume(g_bVolumeIndex);
	#endif
}

/*
 *    Configure Audio Interface -
 *    L/R channel switch, number of channel, sample size and endian.
 */
static void AudioIfConfig(BYTE channel, WORD samplesize)
{	MP_DEBUG("AudioIfConfig");
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);

#if AUDIO_ENDIAN				//Big Endian Audio Output
	AIU_SET_BIGENDIAN();		MP_DEBUG("AIU_SET_BIGENDIAN");
#if LR_SWITCH
	AIU_SET_LRSWITCH_ENABLE();
#else
	AIU_SET_LRSWITCH_DISABLE();
#endif

#else
	AIU_SET_LITTLEENDIAN();		MP_DEBUG("AIU_SET_LITTLEENDIAN");
#endif

	/* Set Number of Channel */
	switch (channel)
	{
		case 1:
			AIU_SET_MONO();		MP_DEBUG("AIU_SET_MONO");
			break;

		case 2:
			AIU_SET_STEREO();	MP_DEBUG("AIU_SET_STEREO");
			break;
	}

	/* Set Sample Size */
	switch (samplesize)
	{
		case 8:
			AIU_SET_08BITMODE();	MP_DEBUG("AIU_SET_08BITMODE");//this means 16bit of word length, i guess
			break;
		case 16:
			AIU_SET_16BITMODE();	MP_DEBUG("AIU_SET_16BITMODE");//this means 32bit of word length, i guess
			break;
	}
}

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615 || STD_BOARD_VER == MP650_FPGA)
static void AudioClockConfig(DWORD Sample_rate, BYTE samplesize)
{
	register CLOCK *audio_clock;
	audio_clock = (CLOCK *) (CLOCK_BASE);

	if (samplesize == 8)
	{
		//PcmConfig.SampleSize = 16;

		if ( (Sample_rate > 7000) && (Sample_rate < 8000)) 
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_08K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x000013f0;//0x00001370;	//for PLL3
		}
		else if ( (Sample_rate > 7999) && (Sample_rate < 11000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_08K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x000013f0;//0x00001370;	//for PLL3
		}
		else if ( (Sample_rate >=11000)&&(Sample_rate< 12000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_11K_DIV_1;//by samplerate
			audio_clock->AudClkC = 0x00001370;	//1370 for PLL3
		}
		else if ( (Sample_rate >=12000)&&(Sample_rate< 16000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_12K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001370;	//for PLL3
		}
		else if ( (Sample_rate >=16000)&&(Sample_rate< 22000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_16K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x000013f0;//0x00001370;	//for PLL3 
		}
		else if ( (Sample_rate >=22000)&&(Sample_rate< 24000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_22K_DIV_1;//by samplerate
			audio_clock->AudClkC = 0x00001330;//0x00001370;	//for PLL3 
		}		
		else
			UartOutText("Can't tell from the Samplerate(8K--96K support) by IIS for samplesize 8 /r/n");
	}
	else
	{
		if ( (Sample_rate >7000)&&(Sample_rate< 11000)) {
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_08K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001370;				//for PLL3
		}
		else if ( (Sample_rate >=11000)&&(Sample_rate< 12000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_11K_DIV_1;//by samplerate
			audio_clock->AudClkC = 0x00001330;				//for PLL3
		}
		else if ( (Sample_rate >=12000)&&(Sample_rate< 16000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_12K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001330;				//for PLL3
		}
		else if ( (Sample_rate >=16000)&&(Sample_rate< 22000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_16K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001370;				//for PLL3 
		}
		else if ( (Sample_rate >=22000)&&(Sample_rate< 24000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_22K_DIV_1;//by samplerate
			audio_clock->AudClkC = 0x00001310;				//for PLL3
		}
		else if ( (Sample_rate >=24000)&&(Sample_rate< 32000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_24K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001310;				//for PLL3
		}
		else if  ( (Sample_rate >=32000)&&(Sample_rate< 44000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_32K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001300;				//for PLL3
		}
		else if( (Sample_rate >=44000)&&(Sample_rate < 48000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_44K_DIV_1;//by samplerate
			audio_clock->AudClkC = 0x00001300;				//for PLL3
		}
		else if ( (Sample_rate >=48000)&&(Sample_rate < 64000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_48K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001300;				//for PLL3
		}
		else if ( (Sample_rate >=64000)&&(Sample_rate < 88000)){
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_64K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001300;				//for PLL3
		}
		else if ( (Sample_rate  >=88000)&&(Sample_rate < 96000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_88K_DIV_1;//by samplerate
			audio_clock->AudClkC = 0x00001300;				//for PLL3
		}
		else if ( (Sample_rate  >=96000)&&(Sample_rate < 97000))
		{
			audio_clock->PLL3Cfg = PLL3_CONFIG_FS_96K_DIV_0;//by samplerate
			audio_clock->AudClkC = 0x00001300;				//for PLL3
		}
		else
			UartOutText("Can't tell from the Samplerate(8K--96K support) by IIS for samplesize 16 /r/n");
	}
}

//This function will become forsaken at mp650 project...
static void I2SConfig(WORD samplerate, BYTE samplesize)
{
	WORD sample_rate;
	register GPIO *audio_gpio;
	register CLOCK *audio_clock;

	audio_gpio = (GPIO *) (GPIO_BASE);
	audio_clock = (CLOCK *) (CLOCK_BASE);

	audio_clock->ClkCtrl &= ~0x01000000;	//Disable PLL3 --- Does this setting conflict the I2S outside dac setting?

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)
	AudioClockConfig(samplerate, samplesize);
#else
	if(_halAudioDac && _halAudioDac->setSamplerate)
		_halAudioDac->setSamplerate(samplerate);
#endif

	audio_clock->Clkss_EXT1 &= 0xfffffff0;
#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)
	audio_clock->Clkss_EXT1 |= 0x0000000b;	// Using PLL3 / 4 as clock source and enable
#else	
	audio_clock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source
#endif

#if (AUDIO_DAC == DAC_WS8956 || AUDIO_DAC == DAC_WM8750L)
	audio_clock->ClkCtrl &= ~0x02000000;	// disable extPLL3
#endif
	audio_clock->ClkCtrl |= 0x01000000;		// enable PLL3

	AIUCLOCK_DISABLE();
	AIU_MAINCLOCK_ENABLE();

	// Configure AGPIO to AUDIO Mode
	#if(AUDIO_DAC == DAC_AK4387)
		AK4387CodecInit();
	#elif (AUDIO_DAC == DAC_WS8956)
		WS8956CodecInit();
	#endif
}
#endif

/*****************************************************************************/
/**************************Audio-Out Module Interface*************************/
/*****************************************************************************/

/*
 *   We use one gpio pin to control headphone amplifier(MP65x/ MP66x series),
 *   please setting this pin in platform header file.
 */
#ifndef GPIO_AMPMUTE
#define GPIO_AMPMUTE NULL
#endif
#ifndef MUTE_ON
#define MUTE_ON	HIGH_ON
#endif

inline void MX_ENABLE_AMP()
{
	if(GPIO_AMPMUTE == NULL || GPIO_AMPMUTE==GPIO_NULL)
		MP_DEBUG("amp mute pin is not definition!");
	else
		SetGPIOValue(GPIO_AMPMUTE, !MUTE_ON);
}

inline void MX_DISABLE_AMP()
{
	if(GPIO_AMPMUTE == NULL|| GPIO_AMPMUTE==GPIO_NULL)
		MP_DEBUG("amp mute pin is not definition!");
	else
		SetGPIOValue(GPIO_AMPMUTE, MUTE_ON);
}


static BYTE audioHW_Enable = 0;
SWORD MX6xx_AudioHW_On()
{
	if(audioHW_Enable)
		return PASS;
	AudioInit_DAC();
	MX6xx_AudioSetVolume(0);
	MX_ENABLE_AMP();

	audioHW_Enable = 1;
	return PASS;
}

SWORD MX6xx_AudioHW_Off()
{
	MX_DISABLE_AMP();

	if(_halAudioDac && _halAudioDac->unInit){
		_halAudioDac->unInit();
		_halAudioDac = NULL;
		audioHW_Enable = 0;
	}
	return PASS;
}


// This function is used to avoid pop noise
// Try to insert null data into hardware for flushing all residual data.
// Actually we can not assure this is root cause of pop noise when starting playing a song.
void MX6xx_AudioFlushBuffer()
{
	#define NULLSIZE  1024
	DWORD StartTime;
	register CHANNEL *audio_dma;
	BYTE *NULLBUF = NULL;

	#if(AUDIO_DAC == DAC_INTERNAL)
	//AIU_PLAYBACK_IN_ENABLE();
	#else
	if (!audioHW_Enable)
	AIU_PLAYBACK_GAI_INIT();
	AIU_PLAYBACK_GAI_ENABLE();
	#endif

	// Clear DMA END interrupt
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	audio_dma->Control = 0;
	AiuDmaClearCallBackFunc();

	NULLBUF = (BYTE *)mem_malloc(NULLSIZE);
	if(NULLBUF){
		DWORD check_aiu_dma=1000;
		memset(NULLBUF, 0, NULLSIZE);

		// set DMA Buffer
		audio_dma->StartA = (DWORD) NULLBUF;
		audio_dma->EndA   = (DWORD) (NULLBUF + NULLSIZE - 1);

		// DMA enable, BUFFER END interrupt enable
		audio_dma->Control = 0x01000001;
		StartTime = GetSysTime();
		IODelay(50);
		while (((g_psDmaAiu->Control & 0x00010000) == 0x00 )&&
				(check_aiu_dma>0)) 
		{
			if(SystemGetElapsedTime(StartTime) > 128)
			{
				MP_ALERT("Wait too long. AIU dma may be dead 1 %s @ %d", __FUNCTION__, __LINE__);
				break;
			}	
			/*check_aiu_dma--;
			IODelay(10);			
			if (check_aiu_dma == 0)
			{
				MP_ALERT("Wait too long. AIU dma may be dead %s @ %d", __FUNCTION__, __LINE__);
			}*/
		}

		mem_free(NULLBUF);
	}
}

/**
 * Open audio_out module
 * flag is for judge the caller is MotionJpeg or not:  
 *    1: MotionJpeg
 *    0: others
 * @retval <0 error
 * @retval >=0 success
 */
int MX6xx_AudioOpen(int flag)
{
	/* Initialize AIU, AC97 and enable AIU_INT */
	register INTERRUPT *interrupt;
	register CHANNEL *audio_dma;
	_AudioIfOpen();

	#if(AUDIO_DAC == DAC_INTERNAL)
	//MX6xx_AudioFlushBuffer();
	#endif
	/*
	 *  Open Audio DMA, Set DMA mode as follows:
	 *  Single Buffer, Cyclic Buffer,
	 *  Line Counter Disable, Address Direction: Inc
	 */
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);

#if 0	//single buffer
	audio_dma->Control = 0x01000002;	/* DMA_CYC:1; IE_DBFEND:1 */
#else	//double buffer, GREGXU
	if (!flag)
		audio_dma->Control = 0x0100000A;	/* DMA_CYC:1; IE_DBFEND:1 */
	else
		audio_dma->Control = 0x01000000;
#endif
	interrupt = (INTERRUPT *) (INT_BASE);
	//Enable AIU DMA interrupt when open audio device
	interrupt->MiMask |= 0x00000001;	//enable DMA interrupt
	DmaIntEna(IM_AIUDM);
    Audio_control =0;
	return 0;
}


/* Configurate audio_out module */
void MX6xx_AudioConfig(WORD rate, BYTE channels, BYTE sample_size)
{

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615  || STD_BOARD_VER == MP650_FPGA)
	I2SConfig(rate, sample_size);
#else
	if(_halAudioDac && _halAudioDac->setSamplerate)
		_halAudioDac->setSamplerate(rate);
#endif

	//This callback function maybe remove soon!
	if(_halAudioDac && _halAudioDac->setPlayback)
		_halAudioDac->setPlayback();

	AudioIfConfig(channels, sample_size);

	return;
}

/* Configurate only sample rate audio_out module, it was only tested with DAC_INTERNAL */
/*This function was added for api API_SetVideoSpeed()*/
/*by Eddyson, 2011.11.03*/
void MX6xx_AudioConfigSampleRate(WORD samplerate)
{

#if (AUDIO_DAC == DAC_INTERNAL)
	if(_halAudioDac && _halAudioDac->setSamplerate)
		_halAudioDac->setSamplerate(samplerate);
#endif

	return;
}

/* Close audio_out module */
void MX6xx_AudioClose(void)
{
	/* Stop AIU(GAI included), DMA and disable AIU_INT */
	register INTERRUPT *interrupt;
	register CHANNEL *audio_dma;
	register AIU *aiu;
	//Gpio_Config2GpioFunc(GPIO_GPIO_6, GPIO_INPUT_MODE,1, 1);//If it has mute circuit, open it.
    Audio_control=1;
	MX6xx_AudioSetVolume(0);
	TimerDelay(1000);		//Wait AIU FIFO transmit. 2010/09/30
	#if HAVE_AMP_MUTE
	SetGPIOValue(GPIO_AMPMUTE, MUTE_ON); //prevent pop noise 2010/09/17 xianwen note
	#endif
	MX6xx_AudioFlushBuffer();
	interrupt = (INTERRUPT *) (INT_BASE);
	interrupt->MiMask &= ~((DWORD) 0x00000800);		// disable AUDIO module interrupt
	DmaIntDis(IM_AIUDM);

	/* Close Audio DMA */
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	audio_dma->Control = 0x00000000;                /* Reset AIU Control Register */

	/* Reset AIU Control Register */
	aiu = (AIU *) (AIU_BASE);
#if (AUDIO_DAC != DAC_INTERNAL)
	aiu->AiuCtl |= (BIT14);    // Set FSH_CLR bit
#else
	//g_psAiu->AiuCtl    = 0x00;//0x08000000;    //RAM_RESET//aiu->AiuCtl = 0;
#endif
	return;
}


/* Pause audio_out module */
void MX6xx_AudioPause(void)
{
	register CHANNEL *audio_dma;

	audio_dma = (CHANNEL *)(DMA_AIU_BASE);
	audio_dma->Control &= ~0x00000001;	/* Disable AIU-DMA Buffer End Interrupt */
	MX6xx_AudioSetVolume(0);
	//Seems to bring big pop noise, close temporarily
/*
	#if(AUDIO_DAC == DAC_INTERNAL)
	AIU_PLAYBACK_IN_DISABLE();
	#else
	AIU_PLAYBACK_GAI_DISABLE();
	#endif
*/	
}


/* Resume audio_out module */
void MX6xx_AudioResume(void)
{
	register CHANNEL *audio_dma;
	register AIU *aiu;

	aiu = (AIU *) (AIU_BASE);
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);

	/* Make sure AC97 Codec stop recording */
	AIU_RECORD_GAI_DISABLE();
	
	/* Playback Activated */
	#if(AUDIO_DAC == DAC_INTERNAL)
	AIU_PLAYBACK_IN_ENABLE();
	AIU_RESET_ADAC_RAMRST_ENABLE();
	#else
	AIU_PLAYBACK_GAI_ENABLE();
	#endif
	MX6xx_AudioSetVolume(g_bVolumeIndex);
	#if SOFT_RECORD
	AIU_LEFT_JUSTIFIED();
	#endif
	#if (AUDIO_DAC == HDMI_AD9889B)
	AIU_LEFT_JUSTIFIED();
	#endif
	//Gpio_Config2GpioFunc(GPIO_GPIO_6, GPIO_OUTPUT_MODE,0, 1);//If it has mute circuit, open it.
	#if HAVE_AMP_MUTE
	SetGPIOValue(GPIO_AMPMUTE, !MUTE_ON);
	#endif
	//Gpio_Config2GpioFunc(GPIO_GPIO_7, GPIO_OUTPUT_MODE,0, 1);
	audio_dma->Control &= ~0x00010000;  /* clear buffer end interrupt */
	audio_dma->Control |=  0x00000001;  /* Enable AIU-DMA Buffer End Interrupt */


	//the second pop noise 
}


/* Get Residual Data in current DMA transfer */
DWORD MX6xx_AudioGetResidue(void)
{
	register CHANNEL *audio_dma;
	DWORD residue = 0;

	audio_dma = (CHANNEL *) (DMA_AIU_BASE);	//audio_dma->Control & 0x8000

	//Bit15(DBUF_ID):The DMA buffer under access, 1 buffer A, 0 buffer B.
	if (audio_dma->Control & 0x8000)
	{
		//Buffer B is under access
		residue = audio_dma->EndB - audio_dma->Current;
	}
	else
	{
		//Buffer A is under access
		residue = audio_dma->EndA - audio_dma->Current;
	}

	return residue;
}

/* Get consumed Data in current DMA transfer */
DWORD MX6xx_AudioGetPTS(void)
{
	register CHANNEL *audio_dma;
	DWORD consumed = 0;

	// For backward avsync issue, when backward issue occurs, stop DMA transfer.  2009/07/29
	if (backward_flag > 0)
	{	
		MP_DEBUG("backward_flag");
		backward_flag = 0;
		return 0;
	}
	if (forward_flag >0)
	{
		MP_DEBUG("forward_flag");
		forward_flag = 0;
		return 0;
	}

	audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	//Bit15(DBUF_ID):The DMA buffer under access, 1 buffer A, 0 buffer B.
	
	//Add a condition to prevent that consumed values are larger than the size of DMA buffer.
	//audio_dma->Current is larger than audio_dma->EndX(X : A or B)  becaue it occurs an interrupt 
	//in reading audio_dma->Current value.audio_dma->Current was changed .
	//Note by XianWen.2010/01/27
	if (audio_dma->Control & 0x8000)
	{
		//Buffer B is under access
		consumed = audio_dma->Current - audio_dma->StartB;	
		if ((DWORD)consumed > (DWORD)(audio_dma->EndB - audio_dma->StartB))
		{
			consumed = 0;
		}
	}
	else
	{
		//Buffer A is under access
		consumed = audio_dma->Current - audio_dma->StartA;	
		if ((DWORD)consumed > (DWORD)(audio_dma->EndA - audio_dma->StartA))
		{
			consumed = 0;
		}
	}
	return consumed;
}

/* Reset audio_out hardware fifo */
void MX6xx_AudioClearHardFifo(void)
{
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);

	/*
	 *  Clear AIU Fifo Buffer
	 *  The bit is cleared automatically by hardware after Fifo buffer is cleard
	 */
	aiu->AiuCtl |= 0x20000000;
}

// If current dac supports mute function, send mute command,
// otherwise set volume to 0.
/*
 * @retval <0 error
 * @retval >=0 success
 */
static int _Audio_SetVolume(WORD wVol)
{
	// Mute Command is under construction!!!!!!!!!
	if(wVol & 0x80)
		wVol = 0;

	// Clean audio volume change indicator
	g_bVolumeIndex &= ~0x40;

	if(_halAudioDac && _halAudioDac->setVolume)
		_halAudioDac->setVolume(wVol);

	return 0;
}


BOOL MX6xx_SetMute(void)
{
	if (g_bVolumeIndex & 0x80)
		g_bVolumeIndex &= ~0x80;
	else
		g_bVolumeIndex |= 0x80;

	_Audio_SetVolume(g_bVolumeIndex);

	return (g_bVolumeIndex >> 7);
}

WORD MX6xx_AudioSetVolume(int iVol)
{
	if(iVol & 0x80)	// We use bit 7 as mute command
		mpDebugPrint("Mute case...");
	if ((iVol & 0x3f) <= 0){
		iVol &= 0xc0;
		iVol |= 0x80;
	}
	else if ((iVol & 0x3f) > VOLUME_DEGREE)
		iVol = (iVol & 0xc0) | VOLUME_DEGREE;
    if(Audio_control==1)
    {
       _Audio_SetVolume(0);
    }
	else
	{
	if(iVol & 0x80)
		MX6xx_SetMute();
	else
		_Audio_SetVolume(iVol);

	#if (AUDIO_DAC == DAC_INTERNAL)
	extern DWORD PcmChunkCounter;
	mpDebugPrint("PcmChunkCounter = %d",PcmChunkCounter);
	if (PcmChunkCounter == 0)
		_Audio_SetVolume(0);
	#endif
    mpDebugPrint("g_bVolumeIndex = %d",g_bVolumeIndex);
	}
	return (iVol);
}

inline void MX6xx_printAIUstatus()
{
	mpDebugPrint("---------- AIU status ----------");
	mpDebugPrint("  Ctl: %x, Ctl1: %x", g_psAiu->AiuCtl, g_psAiu->AiuCtl1);

	mpDebugPrint("  Sta: %x, AcrgWr: %x, AcrgSta: %x, Acslwr: %x, Rden: %x, AcsSta: %x",
		g_psAiu->AiuSta, g_psAiu->AcrgWr,
		g_psAiu->AcrgSta, g_psAiu->AcslWr,
		g_psAiu->AcslRden, g_psAiu->AcslSta
	);

	mpDebugPrint("  DacCtl: %x, TopanaCtl: %x, VolCtl: %x, GainCtl: %x, DwPop: %x",
		g_psAiu->AiuDacCtl, g_psAiu->AiuTopanaCtl,
		g_psAiu->AiuVolCtl, g_psAiu->AiuGainCtl, g_psAiu->AiuDePop
	);

	//mpDebugPrint("keke MdClk: %x, Ext2: %x", g_psClock->MdClken, g_psClock->Clkss_EXT2);
	//mpDebugPrint("Cur: %x(%x) (%x, %x)",
	//	g_psDmaAiu->Current, *(DWORD *)g_psDmaAiu->Current, g_psDmaAiu->StartA, g_psDmaAiu->EndA);

	mpDebugPrint("	PLL3 clock: %x",  g_psClock->PLL3Cfg);
	mpDebugPrint("  Audio clock: %x", g_psClock->AudClkC);
	mpDebugPrint("  clock ext1: %x",  g_psClock->Clkss_EXT1);

	register INTERRUPT *interrupt;
	interrupt = (INTERRUPT *) (INT_BASE);
	mpDebugPrint(" ISR miMask: %x", interrupt->MiMask);
	mpDebugPrint("     mskDma: %x", interrupt->ImaskDma);
	mpDebugPrint("----------------------------");
}

static void AudioInit_DAC()
{
mpDebugPrint("AUDIO_DAC=%d",AUDIO_DAC);
#if (AUDIO_DAC == DAC_INTERNAL)
	_halAudioDac = &_audioDAC_Internal;
#elif (AUDIO_DAC == DAC_AC97)
	_halAudioDac = &_audioDAC_AC97;
#elif (AUDIO_DAC == HDMI_AD9889B)
	_halAudioDac = &_audioDAC_AD9889B;
#elif (AUDIO_DAC == DAC_AK4387)
	_halAudioDac = &_audioDAC_AK4387;
#elif (AUDIO_DAC == DAC_CS4334)
	_halAudioDac = &_audioDAC_CS4334;
#elif (AUDIO_DAC == DAC_ES7240)
	_halAudioDac = &_audioDAC_ES7240;
#elif (AUDIO_DAC == DAC_ES8328)
	_halAudioDac = &_audioDAC_ES8328;
#elif (AUDIO_DAC == DAC_WM8750L)
	_halAudioDac = &_audioDAC_WM8750;
#elif (AUDIO_DAC == DAC_WM8904)
	_halAudioDac = &_audioDAC_WM8904;
#elif (AUDIO_DAC == DAC_WS8956)
	_halAudioDac = &_audioDAC_ws8956;
#elif (AUDIO_DAC == DAC_WM8960)
	_halAudioDac = &_audioDAC_WM8960;
#elif (AUDIO_DAC == DAC_WM8961)
	_halAudioDac = &_audioDAC_WM8961;
#elif (AUDIO_DAC == DAC_ALC5621)
	_halAudioDac = &_audioDAC_ALC5621;
#endif
	if(_halAudioDac)
		_halAudioDac->halInit();

#if (RECORD_AUDIO && AUDIO_DAC!=DAC_INTERNAL)
	if(_halAudioDac && _halAudioDac->halrecInit)
	_halAudioDac->halrecInit();
	AIU_PLAYBACK_GAI_DISABLE();
	AIU_RECORD_GAI_ENABLE();
#endif

	// Initialize audio volume
	MX6xx_AudioSetVolume(g_bVolumeIndex);
	//mpDebugPrint("Audio initial sound...%d", g_bVolumeIndex);
}

