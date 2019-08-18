/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      audio.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   API layer of mx6xx audio device
*       The whole Audio Buffer Pool was allocated at start-up,
*       and each audio buffer ocuppy 16K bytes.
*
*
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "audio.h"
#include "audio_hal.h"
#include "taskid.h"
#if (BLUETOOTH == ENABLE)
#include "btsetting.h"
#include "BtApi.h"
#endif
void inline AudioIntEnable(void);
void inline AudioIntDisable(void);
static BYTE inline Get_Audio_Buf_Num();
extern DWORD MX6xx_AudioGetPTS(void);

static PCM_DATA PcmData;
DWORD PcmChunkCounter = 0;
DWORD AUDIO_BUF_SIZE = 0;
static AUDIO_DEVICE *CurrentAudio = NULL;
DWORD audio_device_pts;
static DWORD uncalc_audio_data_in_pts;
static AUDIO_BUF *Audio_Buf_Pool;
extern DWORD backward_flag;
extern DWORD forward_flag;
extern enum AO_AV_TYPE AO_AV;
/*==============================================*/
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650))
#define AUDIO_BUF_NUM	36    // Increase buffer number to avoid audio playback under flow    10/03/08 C.W
#else 
#define AUDIO_BUF_NUM	24
#endif
#define MALLOC_BUFFER   1	  // If you want to use malloc, please set MALLOC_BUFFER to 1 2010/07/13
#pragma  alignvar(4)

//It temporarily uses "#if" to distinguish to use malloc buffer or not. If it is stable to use malloc buffer
//we will always it. XianWen Note it 2010/07/13
#if MALLOC_BUFFER
static AUDIO_BUF *Audio_Buffer_Pool[AUDIO_BUF_NUM];
#else
static AUDIO_BUF Audio_Buffer_Pool[AUDIO_BUF_NUM];
#endif

static void ClearAudioBuffer();

inline DWORD AudioGetPlayPTS()
{
	return audio_device_pts;
}

inline int AudioSetPlayPTS(DWORD sec)
{
	audio_device_pts = sec;
}

int AudioGetDevicePTS()
{
	DWORD data_size = 0,l_audio_device_pts;
	int extra_pts = 0;
	//Use a local variable to store audio_device_pts to prevent that AudioISR(Interrupt process)
	//changes  audio_device_pts value,and we do not know that it was changed.
	//It will make an error in caculating audio timestamp.
	//Note by XianWen 2010/01/27
	l_audio_device_pts =audio_device_pts;
	if (PcmChunkCounter == 1)
	{
		data_size = MX6xx_AudioGetPTS() * 1000;
		if ((CurrentAudio->m_Config).nBytes > 0)
			extra_pts = (uncalc_audio_data_in_pts + data_size) / (CurrentAudio->m_Config).nBytes;
	}

	return l_audio_device_pts + extra_pts;
}


/*==========Help function=========*/
#define PLAYING2FREE_WATERMARK		2


// @return If audio buffer recycle is allowed, return value is 1
__inline__ BYTE doAudioBufRecycle()
{

	if (CurrentAudio->ABuf_Playing2Free_Num >= PLAYING2FREE_WATERMARK)
		return 1;
    else
		return 0;

}

//Move all buffers of "AUDIO_BUF_STATUS_FREE" status in list "audio_buf_ready" to list "audio_buf_free"
static void __AudioBufRecycle()
{
	AUDIO_BUF *audio_buf_ready;
	if (NULL == CurrentAudio)
		return;
	
	if(doAudioBufRecycle() == 0)
	{
		return;
	}	

	AudioIntDisable();

	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);
	LIST_HEAD *list_head_free = &(CurrentAudio->audio_buf_free);
	LIST_HEAD *list_head1 = list_head_ready->prev;
	LIST_HEAD *list_head2;
	 
	DWORD dwTimeOutCount = 300;
	while (CurrentAudio->ABuf_Playing2Free_Num > 0)
	{
		//Get a audio buffer in the ready list with STATUS_FREE.
		audio_buf_ready = list_entry(list_head1, AUDIO_BUF, list);
		
		while (audio_buf_ready->status != AUDIO_BUF_STATUS_FREE)
		{
			list_head1 = list_head1->prev;
			audio_buf_ready = list_entry(list_head1, AUDIO_BUF, list);
			dwTimeOutCount--;

			if (dwTimeOutCount == 0){
				AudioIntEnable();
				return;
			}
		}

		//Put this free audio_buf into audio_buf_free list
		
		audio_buf_ready->offset = 0;
		audio_buf_ready->status = AUDIO_BUF_STATUS_FREE;
		list_head2 = list_head1->prev;
		list_del(list_head1);
		list_add(list_head1, list_head_free);
		list_head1 = list_head2;

		CurrentAudio->ABuf_Playing2Free_Num--;
		CurrentAudio->ABuf_Free_Num++;
	}
	AudioIntEnable();
}


//Move first buffer belonged to "CurrentAudio->audio_buf_ready" from "PLAYING" STATUS  to "FREE" STATUS
#if (((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP)) ||\
    ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_HF)))
__inline__ void __AudioBufPlaying2Free()
#else
static __inline__ void __AudioBufPlaying2Free()
#endif
{
	DWORD pts;

	if (CurrentAudio->ABuf_Playing_Num == 0)
		return;

	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);
	AUDIO_BUF *audio_buf_playing = list_entry(list_head_ready->prev, AUDIO_BUF, list);

	DWORD dwTimeOutCount = AUDIO_BUF_NUM;
	while (audio_buf_playing->status != AUDIO_BUF_STATUS_PLAYING)
	{
		list_head_ready = list_head_ready->prev;
		audio_buf_playing = list_entry(list_head_ready->prev, AUDIO_BUF, list);
		dwTimeOutCount--;

		if (dwTimeOutCount == 0) return;
	
	}
	uncalc_audio_data_in_pts += audio_buf_playing->offset * 1000;

	pts = uncalc_audio_data_in_pts / (CurrentAudio->m_Config).nBytes;	//These two lines are used for normalization
	uncalc_audio_data_in_pts -= pts * (CurrentAudio->m_Config).nBytes;
	audio_device_pts += pts;
	audio_buf_playing->status = AUDIO_BUF_STATUS_FREE;
	audio_buf_playing->offset = 0;
	CurrentAudio->ABuf_Playing_Num--;
	CurrentAudio->ABuf_Playing2Free_Num++;
	
}

//Move all buffers belonged to "CurrentAudio->audio_buf_ready" from "PLAYING" STATUS  to "READY" STATUS
static __inline__ void __AudioBufPlaying2Ready()
{
	if (CurrentAudio->ABuf_Playing_Num == 0)
		return;

	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);
	LIST_HEAD *list_ready = list_head_ready;
	AUDIO_BUF *audio_buf_playing = list_entry(list_head_ready->prev, AUDIO_BUF, list);
	DWORD dwTimeOutCount = AUDIO_BUF_NUM;

	do
	{
		if (audio_buf_playing->status == AUDIO_BUF_STATUS_PLAYING )
		{
			audio_buf_playing->status = AUDIO_BUF_STATUS_READY;
			CurrentAudio->ABuf_Playing_Num--;
			CurrentAudio->ABuf_Ready_Num++;
		}

		list_ready = list_ready->prev;
		audio_buf_playing = list_entry(list_ready->prev, AUDIO_BUF, list);
		dwTimeOutCount--;

		if (dwTimeOutCount == 0){
			MP_ALERT("Audio buffer playing to Ready -- Time out");
			return;
		}
	}
	while (list_ready != list_head_ready);

	if (CurrentAudio->ABuf_Playing_Num > 0)
	{
		UartOutText("audio driver: internal list fail!!!\r\n");
	}
}

void __CheckAudioBuf()
{
	/*================*/
	PrintS("\r\n/*=======1=======*/");
	PrintS("\r\n ABuf_Ready_Num:");
	PrintV(CurrentAudio->ABuf_Ready_Num);
	PrintS("### ABuf_Free_Num:");
	PrintV(CurrentAudio->ABuf_Free_Num);
	PrintS("\r\n ABuf_Playing2Free_Num:");
	PrintV(CurrentAudio->ABuf_Playing2Free_Num);
	PrintS("### ABuf_Playing_Num:");
	PrintV(CurrentAudio->ABuf_Playing_Num);

	/*================*/
	LIST_HEAD *list_head_free = &(CurrentAudio->audio_buf_free);
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);
	LIST_HEAD *list_head1;
	DWORD i;
	AUDIO_BUF *audio_buf1;

	PrintS("\r\n/*=======Free list statics=======*/");
	list_head1 = list_head_free->prev;
	i = 0;

	DWORD dwTimeOutCount = AUDIO_BUF_NUM;
	while (list_head1 != list_head_free)
	{
		audio_buf1 = list_entry(list_head1, AUDIO_BUF, list);
		PrintS("\r\n status:");
		PrintV(audio_buf1->status);
		PrintS("###offset:");
		PrintV(audio_buf1->offset);
		list_head1 = list_head1->prev;
		i++;
		dwTimeOutCount--;
		if (dwTimeOutCount == 0) return;
	}
	PrintS("\r\n Free Audio Buf NUM:");
	PrintV(i);

	PrintS("\r\n/*=======Ready list statics=======*/");
	list_head1 = list_head_ready->prev;
	i = 0;
	dwTimeOutCount = AUDIO_BUF_NUM;

	while (list_head1 != list_head_ready)
	{
		audio_buf1 = list_entry(list_head1, AUDIO_BUF, list);
		PrintS("\r\n status:");
		PrintV(audio_buf1->status);
		PrintS("###offset:");
		PrintV(audio_buf1->offset);
		list_head1 = list_head1->prev;
		i++;
		dwTimeOutCount--;
		if (dwTimeOutCount == 0) return;
	}
	PrintS("\r\n Ready Audio Buf NUM:");
	PrintV(i);

	return;
}

void AudioBufPoolOpen()
{
	MP_DEBUG("AudioBufPoolReset");
	unsigned int i = 0;

	AudioIntDisable();

	LIST_HEAD *list_head_free = &(CurrentAudio->audio_buf_free);
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);

	INIT_LIST_HEAD(list_head_free);
	INIT_LIST_HEAD(list_head_ready);

	#if !MALLOC_BUFFER
	Audio_Buf_Pool = (AUDIO_BUF *)Audio_Buffer_Pool;
	memset (Audio_Buf_Pool, 0, AUDIO_BUF_NUM * (sizeof (AUDIO_BUF)));
	#endif
	
	for (i = 0; i < AUDIO_BUF_NUM; i ++)
	{

		#if MALLOC_BUFFER
		//Using Mem_malloc to get buffer 
		Audio_Buffer_Pool[i] = (AUDIO_BUF *)mem_malloc(sizeof(AUDIO_BUF));
		if (Audio_Buffer_Pool[i]== NULL)
			MP_ALERT("Audio_Buffer_Pool[%d] is NULL. Memory buffer is not enough");
		memset (Audio_Buffer_Pool[i], 0, sizeof (AUDIO_BUF));
		Audio_Buffer_Pool[i]->offset = 0;
		Audio_Buffer_Pool[i]->status = AUDIO_BUF_STATUS_FREE;
		list_add(&(Audio_Buffer_Pool[i]->list), list_head_free);
		#else
		Audio_Buf_Pool[i].offset = 0;
		Audio_Buf_Pool[i].status = AUDIO_BUF_STATUS_FREE;
		list_add(&(Audio_Buf_Pool[i].list), list_head_free);
		#endif
	}

	CurrentAudio->ABuf_Ready_Num = 0;
	CurrentAudio->ABuf_Playing_Num = 0;
	CurrentAudio->ABuf_Playing2Free_Num = 0;
	CurrentAudio->ABuf_Free_Num = AUDIO_BUF_NUM;

	//audio_device_pts = 0;
	uncalc_audio_data_in_pts = 0;

	AudioIntEnable();

	return;
}
#if MALLOC_BUFFER
static void AudioBufPoolClose()
{
	MP_DEBUG("AudioBufPoolClose");
	unsigned int i = 0;

	AudioIntDisable();

	LIST_HEAD *list_head_free = &(CurrentAudio->audio_buf_free);
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);

	INIT_LIST_HEAD(list_head_free);
	INIT_LIST_HEAD(list_head_ready);
	
	for (i = 0; i < AUDIO_BUF_NUM; i ++)
	{
		mem_free(Audio_Buffer_Pool[i]);
		Audio_Buffer_Pool[i] = NULL;
	  }
	
	
	mpDebugPrint("audio middleware close");
	CurrentAudio->ABuf_Ready_Num = 0;
	CurrentAudio->ABuf_Playing_Num = 0;
	CurrentAudio->ABuf_Playing2Free_Num = 0;
	CurrentAudio->ABuf_Free_Num = AUDIO_BUF_NUM;

	//audio_device_pts = 0;
	uncalc_audio_data_in_pts = 0;

	AudioIntEnable();

	return;
}
#else
static void AudioBufPoolClose()
{
	MP_DEBUG("AudioBufPoolClose");
	AudioBufPoolOpen();
}
#endif

#if MALLOC_BUFFER
static void AudioBufPoolReset()
{
	AudioIntDisable();
	unsigned int i = 0;
	LIST_HEAD *list_head_free = &(CurrentAudio->audio_buf_free);
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);

	INIT_LIST_HEAD(list_head_free);
	INIT_LIST_HEAD(list_head_ready);

	//Audio_Buf_Pool = (AUDIO_BUF *)Audio_Buffer_Pool;
	
	for (i = 0; i < AUDIO_BUF_NUM; i ++)
	{
		//Audio_Buf_Pool[i].offset = 0;
		//Audio_Buf_Pool[i].status = AUDIO_BUF_STATUS_FREE;
		//list_add(&(Audio_Buf_Pool[i].list), list_head_free);
		memset (Audio_Buffer_Pool[i], 0, sizeof (AUDIO_BUF));
		Audio_Buffer_Pool[i]->offset = 0;
		Audio_Buffer_Pool[i]->status = AUDIO_BUF_STATUS_FREE;
		list_add(&(Audio_Buffer_Pool[i]->list), list_head_free);		
	}

	CurrentAudio->ABuf_Ready_Num = 0;
	CurrentAudio->ABuf_Playing_Num = 0;
	CurrentAudio->ABuf_Playing2Free_Num = 0;
	CurrentAudio->ABuf_Free_Num = AUDIO_BUF_NUM;

	//audio_device_pts = 0;
	uncalc_audio_data_in_pts = 0;

	AudioIntEnable();	
	return;
}
#else
static void AudioBufPoolReset()
{
	AudioBufPoolOpen();
	return;
}
#endif	



// This function is forsaken.....
static DWORD AudioBufPoolFill(void *buf, DWORD size)
{
	AUDIO_BUF *audio_buf_free;
	DWORD space;
	DWORD tmp = size;
	char *buf_offset = (char *) buf;

	AudioIntDisable();

	LIST_HEAD *list_head_free  = &(CurrentAudio->audio_buf_free);
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);

	if (list_head_free->prev == list_head_free) {	//No free audio_buf is avaliable
		AudioIntEnable();
		return 0;
	}

	while (tmp > 0)
	{
		//!!!!!!!!!! We don't take a special case - we do not have enough free buffers....in this loop.

		audio_buf_free = list_entry(list_head_free->prev, AUDIO_BUF, list);
		space = AUDIO_BUF_SIZE - audio_buf_free->offset;

		if (tmp <= space)
		{
			// Copy the data to this free audio_buf
			memcpy(&(audio_buf_free->buf[audio_buf_free->offset]), buf_offset, tmp);
			audio_buf_free->offset += tmp;
			break;
		}
		else
		{
			//Copy part of the data into this free audio_buf
			memcpy(&(audio_buf_free->buf[audio_buf_free->offset]), buf_offset, space);
			audio_buf_free->offset += space;

			//Put this free audio_buf into audio_buf_ready list
			list_del(&(audio_buf_free->list));
			list_add(&(audio_buf_free->list), list_head_ready);
			audio_buf_free->status = AUDIO_BUF_STATUS_READY;
			(CurrentAudio->ABuf_Ready_Num)++;
			(CurrentAudio->ABuf_Free_Num)--;

			buf_offset += space;
			tmp -= space;
		}
	}

	AudioIntEnable();

	return size;
}

/*
 *	The core of AudioBufPoolFetch().
 *	In ISR, developer should call this function instead of AudioBufPoolFetch().
 */
static int audio_pool_fetch_fail=0;
static DWORD __AudioBufPoolFetch(PCM_DATA * pcm_data)
{
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);

	register CHANNEL *audio_dma;
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	register INTERRUPT *interrupt;


	AudioIntDisable();
	
	AUDIO_BUF *audio_buf_ready = list_entry(list_head_ready->prev, AUDIO_BUF, list);

	DWORD dwTimeOutCount = AUDIO_BUF_NUM;
	while (audio_buf_ready->status != AUDIO_BUF_STATUS_READY)
	{
		list_head_ready = list_head_ready->prev;
		audio_buf_ready = list_entry(list_head_ready->prev, AUDIO_BUF, list);
		dwTimeOutCount--;
		if (dwTimeOutCount == 0)
		{
			audio_pool_fetch_fail++;
			MP_ALERT("Audio buffer pool fetch fail...%d",audio_pool_fetch_fail);
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP))
        if(!MpxGetA2dpRecordingFlag())
#endif
			//Mark the following code, it can fix the issue that avsync fails if it 
			//can not fetch audio buffer.
			//ClearAudioBuffer(); 2010/07/01 XW note it and mark it.
			AudioIntEnable();
			return 0;
		}

	}
	audio_pool_fetch_fail = 0;

	audio_buf_ready->status = AUDIO_BUF_STATUS_PLAYING;
	(CurrentAudio->ABuf_Ready_Num)--;
	(CurrentAudio->ABuf_Playing_Num)++;

	pcm_data->Start = (DWORD) (&(audio_buf_ready->buf[0]));
	pcm_data->End   = (DWORD) (&(audio_buf_ready->buf[0]) + audio_buf_ready->offset - 1);

#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP))
    if(!MpxGetA2dpRecordingFlag())
#endif
	AudioIntEnable();

	return audio_buf_ready->offset;
}

// 0: Audio buffer is allowed to fetch
// 1: Audio buffer is not allowed to fetch
static DWORD AllowAudioBufFetch(int flag)
{
	if (flag && (CurrentAudio->ABuf_Ready_Num < CurrentAudio->playCfg.preloadBufNum))
		return 0;

	if (CurrentAudio->ABuf_Ready_Num == 0)
	{
		mpDebugPrint("AllowAudioBufFetch CurrentAudio->ABuf_Ready_Num %d",CurrentAudio->ABuf_Ready_Num);
		return 0;
	}
	return 1;
}

/*
 *	If flag is set 1, we must have more than 2 ready buffer or we can not fetch any ready buffer.
 *	If flag is set 0, we must have more than 0 ready buffer or we can not fetch any ready buffer.
 */
DWORD __inline__ AudioBufPoolFetch(PCM_DATA * pcm_data, int flag)
{
	if(AllowAudioBufFetch(flag)){
//		mpDebugPrint("AudioBufPoolFetch");
		return __AudioBufPoolFetch(pcm_data);
	}
	else
		return 0;
}

static DWORD Free_Buf_Mark = 0;
//static 
DWORD AudioBufPoolGetAllocateNum(void)
{
    int i;
	DWORD count = 0;
    for (i = 0; i < AUDIO_BUF_NUM; i ++)
	{
		if (Audio_Buffer_Pool[i] != 0)
			count++;
	}
	return count;
}

DWORD AudioBufPoolGetSpace() // for slideEffect.c, Slideshow MASK calling
{
	DWORD buf_num;
	__AudioBufRecycle();
	if (0 == CurrentAudio)
	{
		return 0;
	}	
	
	if (CurrentAudio->ABuf_Free_Num < (Free_Buf_Mark + 1))
	{
		buf_num = 0;
	}
	else
	{
		buf_num = CurrentAudio->ABuf_Free_Num - (Free_Buf_Mark + 1);
	}
	return buf_num;
}

static float AudioBufPoolGetDelay()
{
	DWORD size = 0;
	DWORD tmp = 0;

	AudioIntDisable();

	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);
	LIST_HEAD *list_head = list_head_ready->prev;
	AUDIO_BUF *audio_buf_ready;

	while (list_head_ready != list_head)
	{
		audio_buf_ready = list_entry(list_head, AUDIO_BUF, list);
		list_head = list_head->prev;	//update the list_head;

		switch (audio_buf_ready->status)
		{
			case AUDIO_BUF_STATUS_READY:
				size += audio_buf_ready->offset;
				break;

			case AUDIO_BUF_STATUS_PLAYING:
				tmp = audio_buf_ready->offset;
				break;

			default:
				break;
		}
	}

	AudioIntEnable();

	if (PcmChunkCounter == 1)
	{
		size += MX6xx_AudioGetResidue();
		if (CurrentAudio->ABuf_Playing_Num > 1)
		{
			size += tmp;
		}
	}

	return ((float) size) / ((float) (CurrentAudio->m_Config).nBytes);	//float operartion due to slow speed of system ??!!!
}

static AUDIO_BUF *GetAudioBuf(void)
{
	AUDIO_BUF *audio_buf_free;
	LIST_HEAD *list_head_free = &(CurrentAudio->audio_buf_free);

	//__AudioBufRecycle();

	if (list_head_free->prev == list_head_free)		//No free audio_buf is avaliable
	{
		mpDebugPrint("[Warning] Get audio buffer fail..");
		return NULL;
	}
	AudioIntDisable();

	audio_buf_free = list_entry(list_head_free->prev, AUDIO_BUF, list);

	list_del(&(audio_buf_free->list));
	(CurrentAudio->ABuf_Free_Num)--;
	//mpDebugPrint("cur free num: %d", CurrentAudio->ABuf_Free_Num);
	AudioIntEnable();

	return audio_buf_free;
}

static void AudioBuf2Ready(AUDIO_BUF * audio_buf)
{
	LIST_HEAD *list_head_ready = &(CurrentAudio->audio_buf_ready);
	AudioIntDisable();
	if (audio_buf->offset > 0)
	{
		//Put this free audio_buf into audio_buf_ready list
//		list_del (&(audio_buf->list));
		list_add(&(audio_buf->list), list_head_ready);
		audio_buf->status = AUDIO_BUF_STATUS_READY;
		(CurrentAudio->ABuf_Ready_Num)++;
//		(CurrentAudio->ABuf_Free_Num)--;
	}
	else	//Put audio buffer into free link-list
	{
		list_add(&(audio_buf->list), &(CurrentAudio->audio_buf_free));
		(CurrentAudio->ABuf_Free_Num)++;
	}

	AudioIntEnable();

	return;
}

/*==============================================*/

/*****************************************************************************/
/**************************Audio Fifo Module Operation************************/
/*****************************************************************************/
///
///@ingroup AUDIOOUTFIFO
///@brief   Clear the Residual Data which hasn't be played out in the audio out device and fifo
///
///@param   _THIS
///
///@return  NULL
///
static void ClearAudioBuffer()
{
	MX6xx_AudioClearHardFifo();
	AudioBufPoolReset();
}

///
///@ingroup AUDIOOUTFIFO
///@brief   Play Audio out if there's enough data in fifo
///
///@return  NULL
///
static unsigned char play_start=0;
static unsigned char play_eof;

#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP))
static void BtPlayAudio(void)
{
	unsigned int PcmBufLen;
        if(GetA2dpSbcSendFlag()==1)
		{
    		AudioIntDisable();
            if(play_start == 0)
            {
                play_start= 1;
                PcmBufLen = AudioBufPoolFetch(&PcmData, 1);
                PcmData.Start = (DWORD)(PcmData.Start | 0xa0000000);
                WaveInProc(PcmData.Start, PcmBufLen);
            }
    		PcmBufLen = AudioBufPoolFetch(&PcmData, 0);
    		if (PcmBufLen)
    		{
                SetA2dpSbcSendFlag(0);
                PcmData.Start = (DWORD)(PcmData.Start | 0xa0000000);
                WaveInProc(PcmData.Start, PcmBufLen);
    		}
            else
            {
                if(play_eof == 1)
                {
                    play_eof = 0;
                    play_start = 0;
#if 0
                    BTA(pXpgEvent)->eType=NONE_XPGEVENT_SBC_FILE_CLOSE;
                    BTA(BtSynCallBack)(BTA(pXpgEvent));
                    __asm("break 100");// stop for only record pcm data of one song when debugging
#endif

                }
            }
		}
		else
			TaskYield();

}
#endif


#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_HF))
static void HfBtPlayAudio(void)
{
	unsigned int PcmBufLen;    
    if(GetHfScoSendFlag()==1)
	{
		AudioIntDisable();
        if(play_start == 0)
        {
            play_start= 1;
            PcmBufLen = AudioBufPoolFetch(&PcmData, 1);
            PcmData.Start = (DWORD)(PcmData.Start | 0xa0000000);
            HfWaveInProc(PcmData.Start, PcmBufLen);			
        }
		PcmBufLen = AudioBufPoolFetch(&PcmData, 0);
        
		if (PcmBufLen)
		{
            SetHfScoSendFlag(0);    
            PcmData.Start = (DWORD)(PcmData.Start | 0xa0000000);
            HfWaveInProc(PcmData.Start, PcmBufLen);			
	}
	else
	{
            if(play_eof == 1)
            {
                play_eof = 0;
                play_start = 0;
            }
        }
	}
//rick	else
//rick		TaskYield();

}
#endif
int count_playaudio;
static BYTE PlayAudio(AUDIO_DEVICE * audio_dev)
{
	CHANNEL * const audio_dma = (CHANNEL *) (DMA_AIU_BASE);
//#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP))
//	unsigned int PcmBufLen;
//#endif   
	/* we share many variable with ISR, so disable interrupt now */
	if (audio_dev->m_bPaused == 1)
		return 3;
	
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP))
//    if(MpxGetA2dpRecordingFlag()==1)
    if(GetA2dpConnect() && MpxGetA2dpRecordingFlag() \
        && GetA2dpStreamStart()&&(GetAniFlag()&ANI_AUDIO))
	{	
        BtPlayAudio();
	}
    else
    {
#endif	
		if (PcmChunkCounter == 0)
		{
			AudioIntDisable();
			//If the file size is too small and the play time is less than 1s, SmallSizeFile will be set up to 1.
			//We only use one DMA buffer to transfer. By XW 2009/11/5
			if(audio_dev->playCfg.smallSizeFile == 1){

				if (AudioBufPoolFetch(&PcmData, 0))
				{
					MP_DEBUG("PlayAudio0");
					audio_dma->StartA = PcmData.Start;
					audio_dma->EndA   = PcmData.End;
					audio_dma->StartB = NULL;
					audio_dma->EndB   = NULL;
				}
				else
				{
					
					if (CurrentAudio->ABuf_Playing_Num>0 || CurrentAudio->ABuf_Playing2Free_Num>0 ||CurrentAudio->ABuf_Ready_Num>0)
					{
						MP_ALERT("CurrentAudio->ABuf_Playing_Num %d",CurrentAudio->ABuf_Playing_Num);
						MP_ALERT("CurrentAudio->ABuf_Playing2Free_Num %d",CurrentAudio->ABuf_Playing2Free_Num);
						MP_ALERT("CurrentAudio->ABuf_Ready_Num %d",CurrentAudio->ABuf_Ready_Num);
						if (count_playaudio>10)
						{
							CurrentAudio->ABuf_Playing_Num = 0;
							CurrentAudio->ABuf_Ready_Num = 0;
							CurrentAudio->ABuf_Playing2Free_Num = 0;
						}
						count_playaudio++;
					}
					MP_ALERT("PlayAudio1");
					return 0;
				}
			}
			else{
				//First time, PcmChunkCounter = 0; others it should equals 1.
					if (!AudioBufPoolFetch(&PcmData, 1)){
							AudioIntEnable();
							return 0;
					}

					audio_dma->StartA = PcmData.Start;
					audio_dma->EndA = PcmData.End;

					AudioBufPoolFetch(&PcmData, 0);
					audio_dma->StartB = PcmData.Start;
					audio_dma->EndB   = PcmData.End;
			}
			PcmChunkCounter = 1;

			MX6xx_AudioResume();	/* start audio play */
			AudioIntEnable();
			return 2;
		}
		else{
			return 1;
			/* device busy */
		}
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_A2DP))
	}
#endif
}

void inline AudioIntEnable()
{
    
	IntDisable();
	CurrentAudio->bIntEnabled = 1;
	IntEnable();
}

void inline AudioIntDisable()
{
	IntDisable();
	CurrentAudio->bIntEnabled = 0;
	IntEnable();
}
void AO_DMAReset(void)
{
	register CHANNEL * const audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	register AIU * const aiu = (AIU *) (AIU_BASE);
	audio_dma->Control &= ~0x10000;	/* clear buffer end interrupt */
	AudioIntDisable();
	DWORD release;
	//if (!CurrentAudio->bIntEnabled) return;
	if (EventPolling(AV_DECODER_EVENT, EVENT_A_NOT_DECODING, OS_EVENT_OR, &release)==OS_STATUS_POLLING_FAILURE)
	{
		audio_dma->Control &= ~0x1;
		PcmChunkCounter = 0;
	}
	else
	{
		//ClearAudioBuffer();
		audio_dma->Control &= ~0x1;	/* Disable AIU-DMA Buffer End Interrupt */

		#if (AUDIO_DAC != DAC_INTERNAL)
		//AIU_PLAYBACK_GAI_DISABLE(); //reduce pop noise: note bye xianwen 2010/04/07
		//aiu->AiuCtl &= ~0x800000;	/* Playback Deactivated */
		#endif

		PcmChunkCounter = 0;
	}
	AudioIntEnable();
}

///
///@ingroup     AUDIOOUTFIFO
///@brief   Interrupt process to playout next pcm atom data in fifo. If there's not enough data, audio out device will be paused on to wait.
///
///@return  NULL
///
//static void ProcessInt(AUDIO_DEVICE *device)

//Audio interrupt service routine
void AudioISR(void)
{
	register CHANNEL *audio_dma;
	register AIU *aiu;
	aiu = (AIU *) (AIU_BASE);
	audio_dma = (CHANNEL *) (DMA_AIU_BASE);
	audio_dma->Control &= ~0x00010000;	/* clear buffer end interrupt */
	if (!CurrentAudio->bIntEnabled){	
			if (!CurrentAudio->ABuf_Playing_Num){
					audio_dma->Control &= ~0x00000001;
					__AudioBufPlaying2Ready();
					PcmChunkCounter = 0;
			}
			EventSet(AVP_SYNC_EVENT, EVENT_A_PLAYED);
			if(CurrentAudio->ABuf_Ready_Num == 0 && AO_AV == AO_TYPE)	{
				return;
			}
	}
	__AudioBufPlaying2Free();
	if (CurrentAudio->m_bPaused == 1)
	{
		if(forward_flag >0 || backward_flag >0)
		{
			ClearAudioBuffer();
		}
		#if 0
		audio_dma->Control &= ~0x00000001;	/* Disable AIU-DMA Buffer End Interrupt */
			#if (AUDIO_DAC != DAC_INTERNAL)
				AIU_PLAYBACK_GAI_DISABLE();
				//aiu->AiuCtl &= ~0x00800000;	/* Playback Deactivated */
			#endif
		#else
			MX6xx_AudioPause();
		#endif
		__AudioBufPlaying2Ready();
		PcmChunkCounter = 0;
		EventSet(AVP_SYNC_EVENT, EVENT_A_PLAYED);
		return;
	}
	if (__AudioBufPoolFetch(&PcmData))
	{
		//Bit15(DBUF_ID):The DMA buffer under access, 0: buffer A, 1 buffer B.
		if (audio_dma->Control & 0x8000)
		{						//we treat this as buffer A playback completed
			audio_dma->StartA = PcmData.Start;
			audio_dma->EndA = PcmData.End;
		}
		else
		{						//we treat this as buffer B playback completed
			audio_dma->StartB = PcmData.Start;
			audio_dma->EndB = PcmData.End;
		}
		EventSet(AVP_SYNC_EVENT, EVENT_A_PLAYED);
	}
	else
	{
		MP_ALERT("Audio underflow");
		//It makes pop noise if there is no data. It is for Youtube. 2010/01/11
		//We call MX6xx_AudioPause() and set PcmChunkCounter equal 0 to reduce it.
#if 0

		if (CurrentAudio->ABuf_Playing_Num < 2)
		{
			MX6xx_AudioPause();

			if (!CurrentAudio->ABuf_Playing_Num )
			{
				#if (AUDIO_DAC != DAC_INTERNAL)
				AIU_PLAYBACK_GAI_DISABLE();
				//aiu->AiuCtl &= ~0x00800000;	/* Playback Deactivated */
				#endif
				__AudioBufPlaying2Ready();
			}
			PcmChunkCounter = 0;
		}
#else
		if(!CurrentAudio->ABuf_Playing_Num)		//5.24 rebecca add to finish the last buffer in playing buffer
		{
			__AudioBufPlaying2Ready();
			audio_dma->Control &= ~0x00000001;	/* Disable AIU-DMA Buffer End Interrupt */
			PcmChunkCounter = 0;			
		}
		EventSet(AVP_SYNC_EVENT, EVENT_A_PLAYED);
#endif
	}

	return;
}

///@ingroup AUDIOOUT
///@defgroup UNINIT   Uninitialization

///
///@ingroup UNINIT
///@brief   Uninit Audio Out Module, including releasing resources, closing audio hardware and free the driver data.
///
///@return  NULL
///
static void AO_Close(void)
{
	/* Close audio device */
	MX6xx_AudioClose();
	AudioBufPoolClose();
}

/* pause: 1; on: 0 */
static void AO_Pause()
{
	/* we process pause command in ISR */
	IntDisable();
	CurrentAudio->m_bPaused = 1;
	IntEnable();
}

static void AO_Resume()
{
	/* we process pause command in ISR */
	IntDisable();
	CurrentAudio->m_bPaused = 0;
	IntEnable();
}


/*========================*/
///@brief   Initialize Audio Out Module, and create a thread to play out audio.
///
///@param   int    rate      Sample rate
///@param   int    channels  Number of channles
///@param   int    format    Sample size(bytes)
///@param   DWORD  fourcc    fourcc value
///
///@return  int indicate whether this operation is successful
///
static int AO_Open(int rate, int channels, int format, DWORD fourcc)
{
	if ((channels != 1) && (channels != 2))
	{
		MP_ALERT("Channel number is not allowed");
		return -1;
	}
	if ((format != 2)&&(format != 1))		//Only 16 bits PCM Sample is supported.
	{
		MP_ALERT("Audio format is not allowed");
		return -1;
	}
	count_playaudio = 0;

	Free_Buf_Mark = 1;
	switch (fourcc)
	{
		case MP3_50:
		case MP3_55:
		case MP3_CVBR:
		case MP3_FCC:
			AUDIO_BUF_SIZE = AUDIO_BUF_SIZE_MPX;
			break;

		case mp4a:
		case MP4A:
		case OGG_T:
		case WMA_T:
		case RAM_T:
		case ram_T:
			AUDIO_BUF_SIZE = AUDIO_BUF_SIZE_AAC;
			break;

		case AMR:
		case AMR_NB:
			AUDIO_BUF_SIZE = AUDIO_BUF_SIZE_AMRNB;
			break;
		case AC3_T :
			AUDIO_BUF_SIZE = AUDIO_BUF_SIZE_AC3;
			break;
		case PCM_1:
		case PCM_RAW:
		case PCM_TWOS:
		case PCM_SOWT:
			AUDIO_BUF_SIZE = AUDIO_BUF_SIZE_PCM;
			break;

		default:
			AUDIO_BUF_SIZE = AUDIO_BUF_SIZE_MPX;
			break;
	}

	AUDIO_CONFIG *audio_config = &(CurrentAudio->m_Config);
	audio_config->SampleRate	= (DWORD) rate;
	audio_config->DataFormat	= (BYTE) (format << 3);
	audio_config->Channels		= (BYTE) channels;
	audio_config->nBytes		= (format & 0xFF) * rate * channels;
	audio_config->Silence		= 0x0;
	audio_config->Fourcc		= fourcc;

//#if Libmad_FAAD
//	if (Layer2_Inited)
//	{							// lay2 force mono
//#ifdef LAYERII_FORCE_MONO
//		audio_config->nBytes /= channels;
		//AUDIO_BUF_SIZE /= channels;
//#endif
//	}
//	if (Layer3_Inited && (rate == 22050))
//		AUDIO_BUF_SIZE >>= 1;
//#endif

	audio_device_pts = 0;
	AudioBufPoolOpen();

	/* Open audio device */
	if (MX6xx_AudioOpen(0) < 0)
		return 0;

	/* Configure audio device */
	MX6xx_AudioConfig((WORD) rate, channels, (BYTE) (format << 3));

	PcmChunkCounter = 0;
	CurrentAudio->m_bPaused = 1;

	return 1;
}

BYTE Clear_Audio_Buf()
{
	Audio_Buf_Pool = NULL;
}

int init_audio_driver(AUDIO_DEVICE * audio_dev)
{
	/* Reset the AUDIO_DEVICE structure */
	memset(audio_dev, 0, (sizeof(AUDIO_DEVICE)));

	audio_dev->init		        = AO_Open;
	audio_dev->uninit	        = AO_Close;
	audio_dev->resume	        = AO_Resume;
	audio_dev->pause	        = AO_Pause;
	audio_dev->GetAudioSpace	= AudioBufPoolGetSpace;
	audio_dev->GetDelay			= AudioBufPoolGetDelay;	//AO_GetDelay;
	audio_dev->FeedAudioBuffer	= AudioBufPoolFill;
	audio_dev->ClearAudioBuffer	= ClearAudioBuffer;
	audio_dev->PlayAudio		= PlayAudio;
	audio_dev->ProcessInt		= AudioISR;
	audio_dev->StopDMA          = AO_DMAReset;
	/*=========For Direct Audio Output=========*/
	audio_dev->GetAudioBuf_D	= GetAudioBuf;
	audio_dev->AudioBuf2Ready_D	= AudioBuf2Ready;
	audio_dev->bDriveID			= DriveCurIdGet();
	audio_dev->bIntEnabled		= 1;
	CurrentAudio = audio_dev;

	return 1;
}

