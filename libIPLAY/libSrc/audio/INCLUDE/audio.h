/**
 *  @mainpage Audio Interface
 *  Files of this folder includer below functions
 *
 *  1. Audio Hal layer API
 *
 *  2. Audio decoded Buffer control
 *
 *  3. Audio DAC module drivers
 *
 *  4. Audio hardware module testing code
 */

/**
 ****************************************************************
 *                      Magic Pixel Inc.
 *
 *    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
 *                    All rights reserved.
 *
 *
 *
 * @file:      audio.h
 *
 * Programmer:    Joshua Lu
 *                MPX E120 division
 *
 * Created: 	 03/30/2005
 *
 * Description: 
 *              
 *        
 * Change History (most recent first):
 *
 *     <1>     03/30/2005    Joshua Lu    first file
 ****************************************************************
 */

#ifndef __AUDIO_H__
#define __AUDIO_H__

/***********/
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/*===========List Functions==============*/
typedef struct list_head {
	struct list_head *next, *prev;
}LIST_HEAD;

#define INIT_LIST_HEAD(ptr)  do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)	

//Add LIST_HEAD node "new" after LIST_HEAD node "head"
static __inline__ void list_add(LIST_HEAD *new, LIST_HEAD *head)
{
	LIST_HEAD *list_head1;
	list_head1 = head->next;

	list_head1->prev = new;
	new->next = list_head1;
	new->prev = head;
	head->next = new;
}

static __inline__ void list_del(LIST_HEAD *entry)
{
	LIST_HEAD *entry_P = entry->prev;
	LIST_HEAD *entry_N = entry->next;	

	entry_P->next = entry_N;
	entry_N->prev = entry_P;
}

/*
 * list_entry - get the struct for this entry
 * @param ptr      the &struct list_head pointer
 * @param type     the type of the struct this is embedded in.
 * @param member   the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member)\
	((type *)((char *)(ptr)-(unsigned int)(&((type *)0)->member)))


/// @name AUDIO_CONSTRAINED_BUFFER_SIZE
/// Minimal and maximal constrained size for audio decoding
///
/// NOTE: AUDIO_BUF_SIZE == (1152 * 2 * 2) * 8 == (1024 * 2 * 2) * 9 \n
///       9216/8192 BYTES
/// @{
/// @brief Upper bound of auding decoding data size at one time
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650))
#define AUDIO_BUF_SIZE2         (1024 * 24)
#else
#define AUDIO_BUF_SIZE2         (1024 * 16)
#endif
/// @brief low bound of auding decoding AAC data size at one time
#define AUDIO_BUF_SIZE_AAC      (1024 * 12)
/// @brief Upper bound of auding decoding general data size at one time
#define AUDIO_BUF_SIZE_MPX      (1152 * 12) 
/// @brief Upper bound of auding decoding PCM data size at one time
#define AUDIO_BUF_SIZE_PCM      (1024 * 12)
#define AUDIO_BUF_SIZE_AC3      (1024 * 12)
/// @brief Upper bound of auding decoding AMRNB data size at one time
#define AUDIO_BUF_SIZE_AMRNB 	 1024
/// @}

enum AudioBufStatus{
	AUDIO_BUF_STATUS_FREE    = 0X11,
	AUDIO_BUF_STATUS_READY   = 0X22,
	AUDIO_BUF_STATUS_PLAYING = 0X33
};

typedef struct audio_buf{
	char buf[AUDIO_BUF_SIZE2];
	enum AudioBufStatus status;		//free->ready->playing->free
	unsigned int offset;			//current offset (How may we use in this buffer)
	LIST_HEAD  list;
}AUDIO_BUF;

typedef struct PCM_DATA
{
	DWORD Start;
	DWORD End;
} PCM_DATA;

/* The calculated values in this structure are calculated by AO_Open() */
typedef struct AUDIO_CONFIG {
    DWORD SampleRate;    /** DSP frequency -- samples per second */
    BYTE  DataFormat;    /** Audio Data Format */
    BYTE  Channels;      /** Number of channels: 1 mono, 2 stereo */
    BYTE  Silence;       /** Audio Buffer Silence Value, Calculated */
    //DWORD nSamples;    /** Audio Buffer size in sample unit (power of 2) */
    DWORD nBytes;        /** Audio Buffer size in bytes, Calculated */  // Byte rate   Commented by C.W 090715
    DWORD Fourcc;
} AUDIO_CONFIG;

typedef struct AUDIO_PLAY_CFG{
	BYTE playMode;       // Please reference "enum AO_AV_TYPE inside Global.h"
	BYTE preloadBufNum;  // How many buffers preload before playing
	BYTE smallSizeFile;  // Indicate current file size is small(Decoded pcm data may be less than 2frame)
	BYTE rev;
}AUDIO_PLAY_CFG;

/// Abstraction of audio device
typedef struct AUDIO_DEVICE AUDIO_DEVICE;
struct AUDIO_DEVICE {
	/* Common device operations */
	int   (*init)(int rate,int channels,int format, DWORD fourcc); ///< Init audio device
	void  (*uninit)();                                            ///< Un-init audio device
	void  (*pause)();                                             ///< Pause audio device
	void  (*resume)();                                            ///< Resume audio device

	DWORD (*GetAudioSpace)();
	float (*GetDelay)();
	
	/* Audio Buffer Operation */
	DWORD (*FeedAudioBuffer)(void *data, DWORD len);
	void  (*ClearAudioBuffer)();
	
    /* This function waits until it is possible to write a full sound buffer */
	BYTE  (*PlayAudio)(AUDIO_DEVICE *);
	
	void  (*ProcessInt)();	///< Audio Interrupt Process Function
	void  (*StopDMA)();
	AUDIO_CONFIG m_Config;	///< Current audio configuration
	int m_bPaused;			///< Current audio state

	/*=======List Header=======*/
	LIST_HEAD audio_buf_free;
	LIST_HEAD audio_buf_ready;
	DWORD ABuf_Free_Num;
	DWORD ABuf_Ready_Num;
	DWORD ABuf_Playing_Num;
	DWORD ABuf_Playing2Free_Num;

	BYTE bDriveID;
	BYTE bIntEnabled;
	BYTE bReserved[2];

	/*========Audio playing style configurate==========*/
	AUDIO_PLAY_CFG playCfg;

	/*========Direct Audio Output==========*/
	AUDIO_BUF * (*GetAudioBuf_D)(void);
	void (*AudioBuf2Ready_D)(AUDIO_BUF *);
};


/**
 *  @name AUDIO_DECODER_FOURCC Audio decoder Fourcc
 *
 *  All decoders' identification
 */
/// @{ 
#ifndef FOURCC
#define FOURCC(a,b,c,d) ( (a<<24) | (b<<16) | (c<<8) | (d) )  ///< FOURCC caculating method
#endif
#define mp4a        FOURCC('a', '4', 'p', 'm')
#define MP4A        FOURCC('A', '4', 'P', 'M')
#define AMR_NB      0x726d6173
#define AMR         0x616d72
#define MP3_50      0x50
#define MP3_55      0X55
#define MP3_CVBR    0x33706d2e  // ".mp3" CBR/VBR MP3 (MOV files)
#define MP3_FCC     0x5500736d  // "ms\0\x55" older mp3 fcc (MOV files)
#define PCM_1       0x1
#define PCM_RAW     0x20776172  // "raw " (MOV files)
#define PCM_TWOS    0x736f7774  // "twos" (MOV files)  
#define PCM_SOWT    0x74776f73  // "sowt" (MOV files)

#define ULAW        FOURCC('W', 'A', 'L', 'U')
#define ulaw        FOURCC('w', 'a', 'l', 'u')
#define ALAW        FOURCC('W', 'A', 'L', 'A')
#define alaw        FOURCC('w', 'a', 'l', 'a')
#define DVI_ADPCM   0x6d730011
#define AC3_T       0x2000

#define WMA_T       0x160                         // WMA:160-162
#define OGG_T       FOURCC('f','o', 'g', 'g')     // 0x161//OGG
#define RAM_T       FOURCC('F', 'F', 'M', 'R')
#define ram_T       FOURCC('f', 'f', 'm', 'r')
/// @}

#if 1   //Print to the UART
	#define PrintS(string)			//UartOutText(string)
	#define PrintV(value)			//UartOutValue(value, 8)
#else   //Print to the Insight Console
	#define PrintS(string)		DpString(string)
	#define PrintV(value)		DpWord(value)
#endif

extern void AudioISR(void);
extern void AO_DMAReset();

/***********************/
/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif


#endif

