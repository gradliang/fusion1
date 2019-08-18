
/**
* FileName     AUDIO.c
*
* Auther       abel yeh
* Date         2004.11.04
*
* Description  for AC97/General Codec            
* 
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "audio_hal.h"

#if (AUDIO_DAC == DAC_AC97)

#define LOCAL_DEBUG_ENABLE 0
#define AC97_AUDIO_TEST  0


BYTE VolumeSequence[VOLUME_DEGREE + 1] = { 0x00,
    0x1f, 0x1d, 0x1b, 0x19, 0x17, 0x15, 0x13, 0x11,
    0x0f, 0x0d, 0x0b, 0x09, 0x07, 0x05, 0x03, 0x01,
};

//Function portotype
static inline int AC97RegWrite(BYTE, WORD);
static inline int AC97CheckReady(void);

/**
 ***************************************************************
 *
 * turn off AC97, ADUB and ADUM clock 
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void TurnoffAudioClk()
{
	CLOCK *audio_clock;

	audio_clock = (CLOCK *) (CLOCK_BASE);

	//audio_clock->MdClken &= 0xfffc7fff;
	AIUCLOCK_DISABLE();
}


/**
 ***************************************************************
 *
 * Enable AC97 clock input 
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void AC97TurnonAudioClk()
{
	register CLOCK *audio_clock;

	audio_clock = (CLOCK *) (CLOCK_BASE);
	AIUCLOCK_DISABLE();
	//audio_clock->MdClken &= 0xFFFc7FFF;
	// Bit_Clock from AC97 Codec
	audio_clock->MdClken |= 0x00020000;
}

/**
 ***************************************************************
 *
 * Config AC97 clock and gpio
 *
 *  Input  : none.
 *
 *  Output : none. 
 ***************************************************************
*/
void AC97AIUCfg(void)
{
	register CLOCK *audio_clock;
	register CHANNEL *audio_dma;
	register GPIO *audio_gpio;

	audio_clock = (CLOCK *) (CLOCK_BASE);
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	audio_gpio = (GPIO *) (GPIO_BASE);

	// AC97 Use External Oscillator
	audio_clock->AudClkC = 0x00000000;


	// Select External AC97 main clock
	// Select External Bit_Clock Source
	// Bit_Clock from AC97 Codec
	//audio_clock->MdClken &= 0xFFFE7FFF;
	//audio_clock->MdClken |= 0x00020000;
	AC97TurnonAudioClk();

	// Dissable AIU_DMA
	audio_dma->Control = 0x00000000;

	// Configure AGPIO to AUDIO Mode
	audio_gpio->Agpcfg = 0x0000001F;
}



/**
 ***************************************************************
 *
 * Write 16 bite data to specific register
 *
 *  Input  : BYTE regaddr : register address
 *           WORD data    : data 
 *
 *  Output : status of AC97 register write   0: ok   -1: fail 
 ***************************************************************
*/
#if 0
int AC97RegWrite(BYTE regaddr, WORD data)
{
#define MAX_RETRY_COUNT 10
	register AIU *aiu;
	int retry1;
	int ok;
	DWORD register_data;

	aiu = (AIU *) (AIU_BASE);
	register_data = ((((DWORD) (regaddr & 0x7f)) << 16) | (DWORD) data) & 0xff7fffff;

	if (regaddr == 0x26)		//0x26 is the address for power-down control
		return 1;

	for (retry1 = 0; retry1 < MAX_RETRY_COUNT; retry1++)
	{
		aiu->AcrgWr = register_data;

		ok = BusyPollRegister32(aiu->AcrgSta, 0x01000000, 0x01000000, 10);
		if (ok)
			break;
	}

	if (retry1 >= MAX_RETRY_COUNT)
		return -1;				/* fail */

	return 0;					/* success */
}
#endif

/**
 ***************************************************************
 *
 * Read 16 bite data of specific register
 *
 *  Input  : BYTE regaddr : register address
 *
 *  Output : WORD data 
 ***************************************************************
*/
#if 0
WORD AC97RegRead(BYTE regaddr)
{
#define MAX_RETRY_COUNT 10
	register AIU *aiu;
	DWORD register_data;
	int retry1;
	int ok;

	aiu = (AIU *) (AIU_BASE);
	register_data = (((DWORD) (regaddr & 0x7f)) << 16) | 0x00800000;

	for (retry1 = 0; retry1 < MAX_RETRY_COUNT; retry1++)
	{
		aiu->AcrgWr = register_data;
		ok = BusyPollRegister32(aiu->AcrgSta, 0x00800000, 0x00800000, 10);
		if (ok)
			break;
	}

	if (retry1 >= MAX_RETRY_COUNT)
		return -1;				/* fail */

	return ((WORD) aiu->AcrgSta & 0x0000FFFF);	/* success */
}
#endif

/**
 ***************************************************************
 *
 * Init AC97 codec 
 *
 *  Input  : none
 *
 *  Output : 0: ok   -1: fail 
 ***************************************************************
*/
int AC97CodecInit(void)
{
	register AIU *aiu;
	GPIO *gpio;
	WORD i;

	aiu = (AIU *) (AIU_BASE);

	// AC97 Module Enable;
	aiu->AiuCtl |= 0x80000000;

	// pull high AC97 reset (GP30)   ( abel 2005.08.15)
#if (GPIO_AC97_RESET != GPIO_NULL)
	SetGPIOValue(GPIO_AC97_RESET, 0);
    IODelay(600);
	SetGPIOValue(GPIO_AC97_RESET, 1);
    IODelay(600);
#endif
// Below are for REALTEK ALC202 Use
	if (AC97RegWrite(0x00, 0x0000) < 0)
		return -1;				// Register Reset
	if (AC97RegWrite(0x00, 0x0000) < 0)
		return -1;				// Register Reset
	if (AC97CheckReady() < 0)
		return -1;

	//Mason 20060621    
	//Note : Volume is currently setting before ReadSetUp, btw, the volume be set when system initial is "default" volume.
	//       You must set it again after reading setup from NOR flash.
	MX6xx_AudioSetVolume(g_bVolumeIndex);	//VOL_CHG_0317
	if (AC97RegWrite(0x06, 0x0008) < 0)
		return -1;				// Mono_Out Volume Register
	if (AC97RegWrite(0x0A, 0x8000) < 0)
		return -1;				// PC_Beep Volume Register
	if (AC97RegWrite(0x0C, 0x8008) < 0)
		return -1;				// Phone Volume Register
	if (AC97RegWrite(0x0E, 0x8008) < 0)
		return -1;				// MIC (input) Volume Register
	if (AC97RegWrite(0x10, 0x8808) < 0)
		return -1;				// Line_In Volume Register
	if (AC97RegWrite(0x12, 0x8808) < 0)
		return -1;				// CD (input) Volume Register
	if (AC97RegWrite(0x14, 0x8808) < 0)
		return -1;				// Video (input) Volume Register
	if (AC97RegWrite(0x16, 0x8808) < 0)
		return -1;				// AUX (input) Volume Register  
//    if (AC97RegWrite(0x18, 0x0000) < 0)    return -1;    // PCM_Out Volume Register, 5-bit resolution                        // set as +12 dB Gain
	if (AC97RegWrite(0x18, 0x0808) < 0)
		return -1;				// PCM_Out Volume Register, 5-bit resolution                        // set as +12 dB Gain
	if (AC97RegWrite(0x1A, 0x0000) < 0)
		return -1;				// Record Select Control Register
	if (AC97RegWrite(0x1C, 0x8000) < 0)
		return -1;				// Record Gain Register
	if (AC97RegWrite(0x20, 0x0000) < 0)
		return -1;				// General purpose Register
	if (AC97RegWrite(0x22, 0x0000) < 0)
		return -1;				// 3D Control Register
	if (AC97RegWrite(0x26, 0x010F) < 0)
		return -1;				// Powerdown Control/Status Register
	// set as "PR0 = 1", power down PCM ADC and input MUX                       
	if (AC97RegWrite(0x28, 0x0000) < 0)
		return -1;				// Extended Audio ID Register
	// set bit_5:4 (DSA[1:0]) = 00
	if (AC97RegWrite(0x2A, 0x0405) < 0)
		return -1;				// Extended Audio Status and Control Register
	// set as "VRA = 1" & S/PDIF Disable
	// also assign S/PDIF source data to AC-LINK slot7/8 (AC'97 Default setting while ID=00)
	return 0;
}


/**
 ***************************************************************
 *
 * Init AC97 codec and AIU module
 *
 *  Input  : none
 *
 *  Output : 0: ok   -1: fail 
 ***************************************************************
 */
int AudioInit_AC97(void)
{
	register INTERRUPT *interrupt;

	AC97AIUCfg();
	if (AC97CodecInit() < 0)
		return -1;

	interrupt = (INTERRUPT *) (INT_BASE);
	interrupt->MiMask |= 0x00000800;	// enable AUDIO module interrupt

	return 0;
}

//Below code is move from Audio_hal.c
//Because we do not have any boards embedded with Ac97 for testing
//If there are any problems of AC97 DAC, we should survey this section and all of this file again
//Careful!!																C.W 080904

/**
 * @retval <0 error
 * @retval >=0 success
 */
static inline int AC97CheckReady(void)
{
	register AIU *aiu;
	int ok;

	aiu = (AIU *) (AIU_BASE);
	ok = BusyPollRegister32(aiu->AiuSta, 0x80000000, 0x80000000, 10);
	if (!ok)
	{
		return -1;
	}

	return 0;
}


/**
 * @retval <0 error
 * @retval >=0 success
 */
static inline int AC97RegWrite(BYTE regaddr, WORD data)
{
#define MAX_RETRY_COUNT 10
	register AIU *aiu;
	int retry1;
	int ok;
	DWORD register_data;

	aiu = (AIU *) (AIU_BASE);
	register_data = ((((DWORD) (regaddr & 0x7f)) << 16) | (DWORD) data) & 0xff7fffff;

	if (regaddr == 0x26)		//0x26 is the address for power-down control
		return 1;

	for (retry1 = 0; retry1 < MAX_RETRY_COUNT; retry1++)
	{
		aiu->AcrgWr = register_data;

		ok = BusyPollRegister32(aiu->AcrgSta, 0x01000000, 0x01000000, 10);
		if (ok)
			break;
	}

	if (retry1 >= MAX_RETRY_COUNT)
		return -1;				/* fail */

	return 0;					/* success */
}

/**
 * @retval <0 error
 * @retval >=0 success
 */
static inline int AC97RegRead(BYTE regaddr)
{
#define MAX_RETRY_COUNT 10
	register AIU *aiu;
	DWORD register_data;
	int retry1;
	int ok;

	aiu = (AIU *) (AIU_BASE);
	register_data = (((DWORD) (regaddr & 0x7f)) << 16) | 0x00800000;

	for (retry1 = 0; retry1 < MAX_RETRY_COUNT; retry1++)
	{
		aiu->AcrgWr = register_data;
		ok = BusyPollRegister32(aiu->AcrgSta, 0x00800000, 0x00800000, 10);
		if (ok)
			break;
	}

	if (retry1 >= MAX_RETRY_COUNT)
		return -1;				/* fail */

	return ((WORD) aiu->AcrgSta & 0x0000FFFF);	/* success */
}


/* Configure AC97 Codec */
WORD AC97_setSamplerate(WORD samplerate)
{
	WORD sample_rate;

	// Wait AC97 Codec Ready
	if (AC97CheckReady() < 0)
		return 0;				/* TASK SWITCH may happen here */

	/* Set Sampling Rate */
	/* Set to AC'97 Codec's Audio Sample Rate Control Register our PCM playback rate */
	if (AC97RegWrite(0x2C, samplerate) < 0)
		return 0;				/* TASK SWITCH may happen here */
	sample_rate = AC97RegRead(0x2c);	/* read out the sample rate, TASK SWITCH may happen here */

	return sample_rate;
}

/**
 * AC97 Volume : 0 = Max Volume , 0x3f = Mute
 * 
 * @retval <0 error
 * @retval >=0 success
 */
int AC97SetVolume(WORD wVol)
{
	register AIU *aiu;
	WORD wAC97Vol;

	wAC97Vol = VolumeSequence[(wVol & 0x3f)];
	wAC97Vol = (((wVol & 0x80) | wAC97Vol) << 8) | wAC97Vol;

	aiu = (AIU *) (AIU_BASE);
	if ((aiu->AiuCtl >> 31) == 0)
	{
		g_bVolumeIndex |= 0x40;
		return 1;
	}
	if (AC97CheckReady() < 0)
		return -1;
	if (AC97RegWrite(0x02, wAC97Vol) < 0)
		return -1;				// Master Volume Register, 6-bit resolution
	if (AC97RegWrite(0x04, wAC97Vol) < 0)
		return -1;				// Master Volume Register, 6-bit resolution

	return 0;
}

HAL_AUDIODAC_T _audioDAC_AC97 =
{
	AudioInit_AC97,
	NULL,	
	NULL,
	AC97_setSamplerate,
	NULL,
	AC97SetVolume
};
#endif

