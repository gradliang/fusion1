/******************************************************************                      Magic Pixel Inc.**    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan*                    All rights reserved.**** Filename:      audioVerify.c** Programmer:    C.W  Liu*                MPX S250 division** Created: 	 03/18/2009** Description:   *		This file is used for testing audio hardware*			at FPGA or chip verification stage.* *		If you want to use this file, please edit makefile.*** Change History (most recent first):*     <1>     03/18/2009    C.W Liu    first file*****************************************************************//*// define this module show debug message or not,  0 : disable, 1 : enable*/#define LOCAL_DEBUG_ENABLE 0/*// Include section */#include "audiodata.h"#if AUDIO_HAL_TESTING_MODE#include "mpTrace.h"#include "audio.h"#include "audio_hal.h"#include "wavUtil.h"#include "../../demux/INCLUDE/filter_graph.h"#include "../../demux/INCLUDE/audio_out.h"typedef struct{	BYTE *start;	DWORD size;} PCM_DATA_BLOCK;PCM_DATA_BLOCK PcmBufer;PCM_CONFIG PcmConfig;///////////////////////////////////////////////////////////////////////////////      function call for play WAV file/////////////////////////////////////////////////////////////////////////////int DecodeWavHader(WAV_CONFIG * Config, PCM_DATA_BLOCK * Bufer){	PCM_DATA_BLOCK *WavBufer;	WAV_CONFIG *wavcfg;	BYTE *stream;	DWORD BufferOffset, chunksize, mark;	WORD wdata;	WavBufer = Bufer;	wavcfg = Config;	stream = WavBufer->start;	wavcfg->Channels = 0;	wavcfg->SampleRate = 0;	wavcfg->AvgBytesPerSec = 0;	wavcfg->BlockAlign = 0;	wavcfg->BitsPerSample = 0;	wavcfg->DataOffset = 0;	wavcfg->DataSize = 0;	// check group ID "RIFF"	MGET_DWORD(stream, mark);	if (mark != 0x46464952)		return 1;	// get group size	MGET_DWORD(stream, chunksize);	if (!chunksize)		return 1;	// check riff type "WAVE"	MGET_DWORD(stream, mark);	if (mark != 0x45564157)		return 1;	BufferOffset = 12;	while (WavBufer->size > BufferOffset)	{		MGET_DWORD(stream, mark);		MGET_DWORD(stream, chunksize);		BufferOffset += 8;		switch (mark)		{		case 0x20746D66:		// fmt			MGET_WORD(stream, wdata);	// format Tage			if (wdata != 0x0001)				return 1;		// only support no compress wav file			MGET_WORD(stream, wavcfg->Channels);			MGET_DWORD(stream, wavcfg->SampleRate);			MGET_DWORD(stream, wavcfg->AvgBytesPerSec);			MGET_WORD(stream, wavcfg->BlockAlign);			MGET_WORD(stream, wavcfg->BitsPerSample);			if (wavcfg->BlockAlign != (wavcfg->BitsPerSample >> 3) * (wavcfg->Channels))				return 1;			BufferOffset += chunksize;			break;		case 0x61746164:		// data			wavcfg->DataOffset = BufferOffset;			wavcfg->DataSize = chunksize;			BufferOffset += chunksize;			MGET_SKIPBYTE(stream, chunksize);			break;		default:			BufferOffset += chunksize;			MGET_SKIPBYTE(stream, chunksize);			break;		}	}	/* while */	if (wavcfg->DataSize == 0)		return 1;	if (wavcfg->SampleRate == 0)		return 1;	return 0;}WORD PCMPlayStart(PCM_DATA_BLOCK * buffer){	register AIU *aiu;	register CHANNEL *audio_dma;	WORD sampleRate;	aiu = (AIU *) (AIU_BASE);	audio_dma = (CHANNEL *) (DMA_AIU_BASE);	//At this test PCM_0 type song, we need to set Little endian case	aiu->AiuCtl1 &= 0xBFFFFFFF; /*set "ENDIAN" = 0 as "little endian" AVi  */	// Clear DMA END interrupt	audio_dma->Control = 0;	// set DMA Buffer	audio_dma->StartA = (DWORD) buffer->start;	audio_dma->EndA   = (DWORD) buffer->start + buffer->size - 1;	//mpDebugPrint("%x, %x, %x, %x, %x", *(buffer->start), *(buffer->start+1), *(buffer->start+2), *(buffer->start+3), *(buffer->start+4));	//mpDebugPrint("dma %x, %x", audio_dma->StartA, audio_dma->EndA);	// DMA enable, BUFFER END interrupt enable	audio_dma->Control = 0x01000001;	return sampleRate;}void I2SCfg_8K(){	WORD sample_rate;	register CLOCK *audio_clock;	register GPIO *audio_gpio;	audio_clock = (CLOCK *)(CLOCK_BASE);	audio_gpio  = (GPIO *) (GPIO_BASE);	audio_clock->ClkCtrl &= ~0x01000000;	//Disable PLL3	// Set clock frequency	audio_clock->PLL3Cfg = 0x00770107;				//0x0002821a	audio_clock->AudClkC = 0x000017f0;				//for PLL3#if (AUDIO_DAC == DAC_INTERNAL)	// Temporal setting for DAC_INTERNAL	audio_clock->PLL3Cfg = 0x00771a82;				//0x0002821a	audio_clock->AudClkC = 0x00001070;	audio_clock->AudClkC &= ~0x00000300;#endif#if (AUDIO_DAC == DAC_WM8960)	audio_clock->PLL3Cfg = 0x007718cc;	audio_clock->AudClkC = 0x00001570;	#endif#if (AUDIO_DAC == DAC_WM8904)		audio_clock->PLL3Cfg = 0x00777C7F;		audio_clock->AudClkC = 0x00001500;	#endif	audio_clock->Clkss_EXT1 &= 0xfffffff0;	audio_clock->Clkss_EXT1 |= (BIT3);    // Using PLL3/1 to be clock source	audio_clock->ClkCtrl |= 0x01000000;		// enable PLL3	AIUCLOCK_DISABLE();	AIU_MAINCLOCK_ENABLE();	//g_psClock->MdClken &= 0xFFFc7FFF;	//CKE_AUDB disable,  c cke_ac97 disable	//g_psClock->MdClken |= 0x00018000;#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)	g_psClock->MdClken |= 0x00000001;	#else		g_psClock->Clkss_EXT2 |= 0x00040000;	//Using since mp650 series	mpDebugPrint("MdClk: %x, Ext2: %x", g_psClock->MdClken, g_psClock->Clkss_EXT2);#endif#if (AUDIO_DAC == DAC_INTERNAL)	MPXDAC_Set_OutputSource();	g_psAiu->AiuDacCtl = 0x00000011;	// Should change to 0x00000010 when Internal codec is ok!!!!!!	g_psAiu->AiuDePop = 0;	g_psAiu->AiuCtl |= 0x10000000;	AIU_SET_GENERAL_WAVEFORM();#elif(AUDIO_DAC == DAC_AK4387)	AK4387CodecInit();	// Configure AGPIO to AUDIO Mode	audio_gpio->Agpcfg = 0x0000001F;//	audio_gpio->Agpcfg = 0x0000000F;#elif(AUDIO_DAC == DAC_WM8961)	AIU_PLAYBACK_GAI_INIT();	AIU_SET_GENERAL_WAVEFORM();	AIU_SET_LRCLOCK_LEN(0x2c);	audio_gpio->Agpcfg = 0x0000001F;#elif(AUDIO_DAC == DAC_WM8960)	AIU_PLAYBACK_GAI_INIT();	AIU_SET_GENERAL_WAVEFORM();	AIU_SET_LRCLOCK_LEN(0xFF);	audio_gpio->Agpcfg = 0x0000001F;	#elif(AUDIO_DAC == DAC_WM8904)	AIU_PLAYBACK_GAI_INIT();	AIU_SET_GENERAL_WAVEFORM();	AIU_SET_LRCLOCK_LEN(0xFF);	//AIU_RIGHT_JUSTIFIED();	audio_gpio->Agpcfg = 0x0000001F;		#endif}/* Configure Audio Interface */void AudioIfConfig2(void){	register AIU *aiu;	aiu = (AIU *) (AIU_BASE);#if AUDIO_ENDIAN				//Big Endian Audio Output	AIU_SET_BIGENDIAN();#else	AIU_SET_LITTLEENDIAN();#endif	/* Set Number of Channel */	switch (PcmConfig.Channels)	{		case 1:			AIU_SET_MONO();			break;		case 2:			AIU_SET_STEREO();			break;	}	/* Set Sample Size */	switch (PcmConfig.SampleSize)	{		case 8:			AIU_SET_08BITMODE();			break;		case 16:			AIU_SET_16BITMODE();			break;	}}//Those functions are only for harware testing used with testing patternvoid simpleHalCfg(){	I2SCfg_8K();	AudioIfConfig2();}void AudioTest(){	register AIU *aiu;	WAV_CONFIG WavCfg;	register CLOCK *audio_clock;		while(1)	//Keep playing testing pattern	{		PcmBufer.start = &Test_music[0];		PcmBufer.size = 0x8000;		DecodeWavHader(&WavCfg, &PcmBufer);		PcmBufer.start = &Test_music[44];		PcmBufer.size = 0x8000 - 44;		PcmConfig.SampleRate = WavCfg.SampleRate;		PcmConfig.SampleSize = WavCfg.BitsPerSample;		PcmConfig.Channels   = WavCfg.Channels;		/* Configure audio device */		mpDebugPrint("Sample rate: %d, SampleSize: %d, Channels: %d",			PcmConfig.SampleRate, PcmConfig.SampleSize, PcmConfig.Channels);				simpleHalCfg();		PCMPlayStart(&PcmBufer);		while ((g_psDmaAiu->Control & 0x00010000) == 0x00) {			//UartOutText(".");			//mpDebugPrint("Cur: %x(%x) (%x, %x)", g_psDmaAiu->Current, *(DWORD *)g_psDmaAiu->Current, g_psDmaAiu->StartA, g_psDmaAiu->EndA);		}		//mpDebugPrint("keke MdClk: %x, Ext2: %x", g_psClock->MdClken, g_psClock->Clkss_EXT2);		UartOutText(">");		UartOutText("\r\n\r\n");		TaskYield();	}}//Please call this function to start audio playback hardware testingvoid audioTest_main(){	mpDebugPrint("Start audio testing...");#if(AUDIO_DAC == DAC_INTERNAL)	AudioInit_M6xxDAC();#elif(AUDIO_DAC == DAC_WM8961)	AudioInit_WM8961();#elif(AUDIO_DAC == DAC_WM8960)	AudioInit_WM8960();#elif(AUDIO_DAC == DAC_WM8904)	AudioInit_WM8904();#endif		MX6xx_AudioSetVolume(7);	// Volume range is from 1 to 16	AudioTest();}#endif