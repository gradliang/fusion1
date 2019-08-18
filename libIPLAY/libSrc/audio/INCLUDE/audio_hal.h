/**
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* @file:      audio_hal.h
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: Audio hal layer API is provided in this file.
*
*
* Change History (most recent first):
*
*     <2>     07/10/2009    C.W Liu      Write Doxygen comments
*
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/


/**
 *  @defgroup AUDIOHAL Audio Hal layer API
 *  Functions inside this group are audio hal layer API
 */

#ifndef __AUDIO_HAL_H__
#define __AUDIO_HAL_H__

#if ((CHIP_VER & 0xFFFF0000) != CHIP_VER_650)
// Disable below bits - CKE_AC97, CKE_AUDB, CKE_AUDM
// For disable all aiu clock
#define AIUCLOCK_DISABLE()             ((g_psClock->MdClken) &= ~(BIT15 | BIT16 | BIT17))
#else
// Disable below bits - CKE_AUDB, CKE_AUDM for MP65x
#define AIUCLOCK_DISABLE()             ((g_psClock->MdClken) &= ~(BIT15 | BIT16))
#endif

// Enable below bits - CKE_AUDB, CKE_AUDM
// For enabling aiu main clock and bit clock
#define AIU_MAINCLOCK_ENABLE()         ((g_psClock->MdClken) |= (BIT16 | BIT15))

// Enable below bits - GAI_EN, FIFO_CLR, PLY_EN, FSH_MASK
// For enabling aiu playback at  general codec audio interface case
#define AIU_PLAYBACK_GAI_INIT()        (g_psAiu->AiuCtl = (BIT30 | BIT29 | BIT23 | BIT6))

#define AIU_PLAYBACK_IN_ENABLE()       (g_psAiu->AiuCtl |=  (BIT28))
#define AIU_PLAYBACK_IN_DISABLE()      (g_psAiu->AiuCtl &= ~(BIT28))

#define AIU_PLAYBACK_GAI_ENABLE()      (g_psAiu->AiuCtl |=  (BIT23))
#define AIU_PLAYBACK_GAI_DISABLE()     (g_psAiu->AiuCtl &= ~(BIT23))

#define AIU_RECORD_GAI_ENABLE()        (g_psAiu->AiuCtl |=  (BIT22))
#define AIU_RECORD_GAI_DISABLE()       (g_psAiu->AiuCtl &= ~(BIT22))


#define AIU_SET_BIGENDIAN()            ((g_psAiu->AiuCtl1) |=  (BIT30))
#define AIU_SET_LITTLEENDIAN()         ((g_psAiu->AiuCtl1) &= ~(BIT30))

// This definition is used to set DMA audio data
#define AIU_SET_STEREO()               ((g_psAiu->AiuCtl1) |=  (BIT24))
#define AIU_SET_MONO()                 ((g_psAiu->AiuCtl1) &= ~(BIT24))

#define AIU_SET_16BITMODE()            ((g_psAiu->AiuCtl1) |=  (BIT17))
#define AIU_SET_08BITMODE()            ((g_psAiu->AiuCtl1) &= ~(BIT17))


#define AIU_SET_LRSWITCH_ENABLE()      ((g_psAiu->AiuCtl1) |=  (BIT31))
#define AIU_SET_LRSWITCH_DISABLE()     ((g_psAiu->AiuCtl1) &= ~(BIT31))

#define AIU_RESET_ADAC_RAMRST_DISABLE()			((g_psAiu->AiuCtl) &= ~(BIT27))
#define AIU_RESET_ADAC_RAMRST_ENABLE()			((g_psAiu->AiuCtl) |= (BIT27))

#define AIU_RIGHT_JUSTIFIED()			((g_psAiu->AiuCtl1) |= (BIT20))
#define AIU_LEFT_JUSTIFIED()			((g_psAiu->AiuCtl1) &= ~(BIT20))



// Enable below bits - STEREO, S_STEREO, DATA_SYNC, JUSTIFIED, RESOLU, FRAME_CNT
// For setting default audio wave form -
//     1. stereo mode
//     2. data synchronized to FSYNC
//     3. normal FSYNC frame(spec. is wrong and is need corrected)
//     4. right justified
//     5. 16bit audio data
//     6. LR clock length equaled to 32 bit clock length
#define AIU_SET_GENERAL_WAVEFORM()     ((g_psAiu->AiuCtl1) =  (BIT26 | BIT24 | BIT21 | BIT20 | BIT17))

#define AIU_SET_LRCLOCK_LEN(num)       ((g_psAiu->AiuCtl1) &=  ~(BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)); \
                                       ((g_psAiu->AiuCtl1) |=   (num))
                                       

#define AIU_I2S_AUDIO_INTERFACE()	g_psAiu->AiuCtl1 &= ~(BIT21);	g_psAiu->AiuCtl1 |= (BIT19); \
									g_psAiu->AiuCtl1 &= ~(BIT20)

/**
 *  @ingroup AUDIOHAL
 *  @brief   Audio hardware module open
 *  @param   flag If the caller is MotionJpeg case, should set this flag.
 *           1: MotionJpeg
 *           0: others
 *  @return  If it is success to open audio device
 *  @retval  <0 error
 *  @retval  >=0 success
 */
int     MX6xx_AudioOpen(int flag);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Configurate audio_out module
 */
void    MX6xx_AudioConfig(WORD rate, BYTE channels, BYTE sample_size);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Configurate only sample rate audio_out module
 */
void    MX6xx_AudioConfigSampleRate(WORD rate);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Close audio out module
 */
void    MX6xx_AudioClose(void);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Pause audio out module
 */
void    MX6xx_AudioPause(void);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Resume audio_out module
 */
void    MX6xx_AudioResume(void);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Get residual data in current DMA transfer
 *  @return  Size of residue data in DMA buffer
 */
DWORD   MX6xx_AudioGetResidue(void); /* Get the number of residual audio data not played, unit:byte */

/**
 *  @ingroup AUDIOHAL
 *  @brief   Get consumed data in current DMA transfer
 *  @return  Size of consumed data in DMA buffer
 */
DWORD   MX6xx_AudioGetPTS(void);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Reset AIU  FIFO buffer
 */
void    MX6xx_AudioClearHardFifo(void);

/**
 *  @ingroup AUDIOHAL
 *  @brief   If audio module is active, mute it,
 *           otherwise try to activate it.
 *  @return  ** This return value has sme problem, need to re-modify. **
 */
BOOL    MX6xx_SetMute(void);

/**
 *  @ingroup AUDIOHAL
 *  @brief   Change audio out device volume
 *  @return  Return the volume be set.
 */
WORD    MX6xx_AudioSetVolume(int iVol);

typedef struct
{
	WORD SampleRate;	///< PCM Audio "Sampling Rate" parameter
						//   Note : Generally AC'97 codec sopports
						//   8000 Hz, 11025 Hz, 16000 Hz, 22050 Hz, 32000 Hz, 44100 Hz, 48000Hz.
						//   unit : Hz

	BYTE SampleSize;	///< PCM Audio "Sample Size" parameter
						//   unit : bit ( only 8 or 16 is available )

	BYTE Channels;		///< PCM Audio "Channels" parameter
						//   unit : channel ( only 1 or 2 is available )
} PCM_CONFIG;


typedef struct HAL_AUDIODAC_T
{
	int (*halInit)(void);           // Init function for general playback case
	int (*halrecInit)(void);        // Init function for record case
	int (*unInit)(void);
	int (*setSamplerate)(DWORD);    // Adjust sample rate for current song
	int (*setPlayback)();           // Adjust hardware setting for preparing to playback a song
	int (*setVolume)(WORD);
}HAL_AUDIODAC_T;

typedef struct
{
	BYTE *start;
	DWORD size;
} PCM_DATA_BLOCK;

extern HAL_AUDIODAC_T _audioDAC_Internal;
extern HAL_AUDIODAC_T _audioDAC_AC97;
extern HAL_AUDIODAC_T _audioDAC_AD9889B;
extern HAL_AUDIODAC_T _audioDAC_AK4387;
extern HAL_AUDIODAC_T _audioDAC_CS4334;
extern HAL_AUDIODAC_T _audioDAC_ES7240;
extern HAL_AUDIODAC_T _audioDAC_ES8328;
extern HAL_AUDIODAC_T _audioDAC_WM8750;
extern HAL_AUDIODAC_T _audioDAC_WM8904;
extern HAL_AUDIODAC_T _audioDAC_ws8956;//audioDac_wfson.c
extern HAL_AUDIODAC_T _audioDAC_WM8960;
extern HAL_AUDIODAC_T _audioDAC_WM8961;
extern HAL_AUDIODAC_T _audioDAC_ALC5621;

#endif
