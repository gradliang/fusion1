#define LOCAL_DEBUG_ENABLE 0
#include "global612.h"
#include "mpTrace.h"
#include <string.h>
#include "TaskID.h"
#include "AVIplatform.h"
#include "AVIfmt.h"
#include "AVI_utility.h"
#include "record.h"

//#include "AVI_File.h"

//#include "taskid.h"

#if(RECORD_ENABLE)
unsigned int jpg_cnt = 0;
static unsigned int pcm_cnt = 0;

static char *idx_ptr;
static int idx_cnt;
 int index_close =0;

BYTE aviend = 0;
int  aviPackerCnt = 0;
BYTE aviStart = 0;
int file_math=0;
extern WAVEFORMATEX wavformatex;
extern AVIFILE avifile;
extern DWORD fps_record;
int start_1=0;
extern  record_argument rec_argument;

//============================
//STREAM *index_handle;
AVIINDEXENTRY *idx_1;//=MagicPixel_AVIEN_malloc_callback(sizeof(AVIINDEXENTRY));
unsigned int off_1 = 332;
unsigned int jpg_size_1 = 0;
int pcm_size_1 = (1024 * 4);
DWORD index_count=0;
const char *outName = {"index.t"};
unsigned int jpg_size_total = 0;
#if AUTO_GET_RECORD_FPS_TIMER
DWORD g_dwRecordStartTime;
#endif
//============================
#define WriteBack_FileSize 0
/*
 *  This semaphore is used to protect the critical section of AVIEN_fillJpg, AVIEN_fillPcm
 */
//static char semaph = 0;

void lock_semaph()
{
    SemaphoreWait(RECORD_STREAM_CACHE_SEMA_ID);
/*
    while(semaph)
        TaskYield();

    IntDisable();
    semaph = 1;
    IntEnable();
*/
}

void unlock_semaph(int rest)
{
    SemaphoreRelease(RECORD_STREAM_CACHE_SEMA_ID);
/*
    IntDisable();
    semaph = 0;
    IntEnable();
    TaskSleep(rest);
*/
}

#if AUTO_GET_RECORD_FPS_TIMER
void TimerToGetRecordFPS(void)
{
	DWORD dwTimer;
	
	//if (!bRecordMode())
	//	return;
	if (!jpg_cnt)
		return;
	if (aviend)
		dwTimer=SystemGetElapsedTime(g_dwRecordStartTime);
	else
		dwTimer=AUTO_GET_RECORD_FPS_TIMER;
	fps_record=(jpg_cnt*1000+dwTimer/2)/dwTimer;
	avifile.vTimestamp=10000/fps_record;
	
	mpDebugPrint("-%s-:%d",__FUNCTION__,fps_record);
}

#endif

#ifdef AVIPLATFORM_MPX
int AVIEN_createfile(STREAM *file_handle,DWORD imgSz, DWORD wavSz, DWORD sampleRate, DWORD sampleSize)
{
    int i1=0;
    off_1=332;

    if(sampleRate != 8000 && sampleRate != 16000){
        mpDebugPrint("%d samplerate is not supported", sampleRate);
        return 0;
    }
    else if(sampleSize != 8 && sampleSize != 16){
        mpDebugPrint("%d samplesize is not supported", sampleSize);
        return 0;
    }

    aviStreamInit();

    jpg_cnt = 0;
    pcm_cnt = 0;
    aviPackerCnt = 0;
    aviStart = 1;
    start_1++;
	/*
if(file_math==1)
{
	outName = "test_1.avi";
//	aviStreamTaskCreate();
	//aviStreamStartup();

}
*/
    //file_math++;
    //=========================
    avifile.index_count=0;
    avifile.index_handle = PathAPI__Fopen(outName, "rb");

	if (avifile.index_handle)
	{
		DeleteFile(avifile.index_handle);
	}

    avifile.index_handle = PathAPI__Fopen(outName, "wb");
	
    MP_ALERT("Create %s", outName);

    if (avifile.index_handle == NULL)
    {
        MP_ALERT("--E-- Create %s fail !!!", outName);
	    return -1; //Ysliu add 2013/05/08
        __asm("break 100");
    }

    File_ReleaseSpace_For_OverwriteContent(avifile.index_handle);
    idx_1 = (AVIINDEXENTRY *) MagicPixel_AVIEN_malloc_callback(sizeof(AVIINDEXENTRY));

    if ((DWORD) idx_1 == 0xA0000000)
    {
        mpDebugPrint("idx_1 cache alloc fail..., system hangs");
	    return -1; //Ysliu add 2013/05/08
        __asm("break 100");
    }

    //=========================

    char buf[324] = {0};

//    strcpy(outFile, outName);


    //Change to target drive here!
    avifile.pFile   =file_handle; //MagicPixel_AVIEN_fileOpen_callback(outFile, "wb");
//========
/*
    static char fer[1]={'a'};
    FileWrite(file_handle, fer, 1);
    FileClose(file_handle);
    return 0;
*/
//========
    if(avifile.pFile==NULL) return -1;
    //mpDebugPrint("%s: file [%s] created !", __FUNCTION__, avifile.pFile);
    avifile.pcmNum  = 0;
    avifile.aviInfo = (AVIFILEINFO *) MagicPixel_AVIEN_malloc_callback(sizeof(AVIFILEINFO));
    avifile.jbuf    = (BYTE *) MagicPixel_AVIEN_malloc_callback(imgSz + 8);	// 8 space is for RIFF header
    avifile.abuf    = (BYTE *) MagicPixel_AVIEN_malloc_callback((wavSz * PCMCACHE_NUM) + 8);

    if(((DWORD) avifile.aviInfo == 0xA0000000) || ((DWORD) avifile.jbuf == 0xA0000000) || ((DWORD) avifile.abuf == 0xA0000000))
    {
        mpDebugPrint("avi cache alloc fail..., system hangs");
        __asm("break 100");
    }

    avifile.aviInfo->imgSz = imgSz;
    avifile.aviInfo->pcmSz = wavSz;
    avifile.aviInfo->samplerate = sampleRate;
    avifile.aviInfo->samplesize = sampleSize;

    avifile.aTimestampCnt = 0;
    avifile.vTimestampCnt = 0;//wavsz=1024
    avifile.aTimestamp = 10000 * wavSz / (sampleSize / 8) / sampleRate;
//+++++++++++++

#if AUTO_GET_RECORD_FPS_TIMER
	if (fps_record)
		avifile.vTimestamp=10000/fps_record;
	else
		avifile.vTimestamp=167;
#else
    switch(fps_record)
    {
	case 60:
    	avifile.vTimestamp = 167;   // 60frame in one sec
    	break;
	case 50:
    	avifile.vTimestamp = 200;   // 50frame in one sec
    	break;
	case 30:
    	avifile.vTimestamp = 333;   // 30 frame in one sec
    	break;
	case 24:
    	avifile.vTimestamp = 417;   // 24 frame in one sec
    	break;
    case 20:
    	avifile.vTimestamp = 500;   // 20 frame in one sec
        break;
	case 16:
    	avifile.vTimestamp = 625;   // 16 frame in one sec
        break;
	case 12:
    	avifile.vTimestamp = 833;   // 12 frame in one sec
        break;
//	case 6:
	default:
    	avifile.vTimestamp = 167;   // 6 frame in one sec
        break;
    }
#endif
	 MagicPixel_AVIEN_write_callback(buf, 324);
	mpDebugPrint("Audio time stamp: %d, video time stamp: %d", avifile.aTimestamp / 10, avifile.vTimestamp / 10);
	//if(start_1!=0)


    idx_ptr = (char *) MagicPixel_AVIEN_malloc_callback(100 * 1024);

    if ((DWORD) idx_ptr == 0xA0000000)
        __asm("break 100");

    for(i1=0;i1<44000;i1++)
    {
	idx_ptr[i1]=3;

    }
	idx_cnt = 0;
	mpDebugPrint("4p");
#if AUTO_GET_RECORD_FPS_TIMER
	g_dwRecordStartTime=GetSysTime();
	Ui_TimerProcAdd(AUTO_GET_RECORD_FPS_TIMER, TimerToGetRecordFPS);
#endif

    return 0;
}

int AVIEN_fillJpeg(unsigned char *buf,DWORD jpeg_size)
{
    int retVal = jpeg_size;

    //mpDebugPrint("avi write: %d/%d", aviend, aviStart);
    lock_semaph();
	if(rec_argument.RecordingAudio==0)
	{
    if (avifile.vTimestampCnt < (avifile.aTimestampCnt + 3333))
    {
        if(aviend || !aviStart){
            MP_ALERT("--E-- %s: error1", __FUNCTION__);
            unlock_semaph(1);
            return 0;
        }

        jpg_size_1 = jpeg_size;// + jpeg_size %2;

        if (aviStreamGetNullSpace() > ((jpg_size_1 + 32) + 4096))
        {
            idx_ptr[idx_cnt++] = 0;
            wFourccHeader("00dc", jpg_size_1);// avifile.aviInfo->imgSz);

            MagicPixel_AVIEN_write_callback((unsigned char *)((DWORD)buf | 0xa0000000), jpg_size_1);
            jpg_cnt++;
            aviPackerCnt++;
            jpg_size_total += jpg_size_1;

            //==============
            idx_1->fccType = mmioFOURCC('0', '0', 'd', 'c');
            idx_1->dwFlag = dwEndianConvert(AVIIF_KEYFRAME);
            idx_1->dwChunkOffset = dwEndianConvert(off_1 - 328);
            idx_1->dwChunkLength = dwEndianConvert(jpg_size_1);
            avifile.index_count++;
            MagicPixel_AVIEN_INDEX(idx_1,1);
            avifile.vTimestampCnt += avifile.vTimestamp;
            off_1 += jpg_size_1 + 8;

            //==============
#if 1       // Duplicate index
            if (avifile.aTimestampCnt > (avifile.vTimestampCnt + avifile.vTimestamp))
            {
                int i, j;
                int index_add;

                index_add = avifile.aTimestampCnt - avifile.vTimestampCnt;
                i = index_add / avifile.vTimestamp;
                j = i;

                while (i > 0)
                {
                    avifile.index_count++;
                    MagicPixel_AVIEN_INDEX(idx_1, 1);
                    i--;
                }

                avifile.vTimestampCnt += j * avifile.vTimestamp;
                MP_ALERT(" I%2d", j);
            }
#endif
        }
        else
        {
            mpDebugPrintChar('D');
            retVal = -1;
        }
    }
    else
    {
//        mpDebugPrintChar('O');
        retVal = -2;
    }

    unlock_semaph(1);

    return retVal;
	}
	else
	{
		
		 if(aviend || !aviStart){
            MP_ALERT("--E-- %s: error1", __FUNCTION__);
            unlock_semaph(1);
            return 0;
        }

        jpg_size_1 = jpeg_size;// + jpeg_size %2;

        if (aviStreamGetNullSpace() > ((jpg_size_1 + 32) + 4096))
        {
            idx_ptr[idx_cnt++] = 0;
            wFourccHeader("00dc", jpg_size_1);// avifile.aviInfo->imgSz);

            MagicPixel_AVIEN_write_callback((unsigned char *)((DWORD)buf | 0xa0000000), jpg_size_1);
            jpg_cnt++;
            aviPackerCnt++;
            jpg_size_total += jpg_size_1;

            //==============
            idx_1->fccType = mmioFOURCC('0', '0', 'd', 'c');
            idx_1->dwFlag = dwEndianConvert(AVIIF_KEYFRAME);
            idx_1->dwChunkOffset = dwEndianConvert(off_1 - 328);
            idx_1->dwChunkLength = dwEndianConvert(jpg_size_1);
            avifile.index_count++;
            MagicPixel_AVIEN_INDEX(idx_1,1);
            avifile.vTimestampCnt += avifile.vTimestamp;
            off_1 += jpg_size_1 + 8;
        }
		 else
        {
            mpDebugPrintChar('D');
            retVal = -1;
        }
		 unlock_semaph(1);
	}
}



int AVIEN_fillPcm(unsigned char *buf, DWORD bufLen)
{
    unsigned int p;
    int size;
    const char* chunkId = "01wb";
    int i, j, num;
    short *ptr;
    unsigned char *ptr2;
    unsigned char *c_ptr,*c_ptr2;
    int i1=0;

    lock_semaph();

    if (aviend || !aviStart)
    {
        unlock_semaph(1);
        return 0;
    }

    //mpDebugPrint("avi write: %d/%d", aviend, aviStart);
    size = avifile.aviInfo->pcmSz;

    char *wbuf;
    wbuf = avifile.abuf + 8 + ((avifile.aviInfo->pcmSz ) * avifile.pcmNum);

    //mpDebugPrint("a: %x/%x", wbuf, avifile.abuf);
	if(aviStreamGetNullSpace()<4096)
		{
			mpDebugPrint("#ERROR_fill Pcm is enough space");
		unlock_semaph(1);
	
		return 0;	
		}

    if (avifile.aviInfo->samplesize != 8)
    {
        //mpDebugPrint("convert...from %d to 8", avifile.aviInfo->samplesize);
        ptr = (short *)buf;
        c_ptr = (unsigned char *)(wbuf);
        num = size >> 1;
        j = 0;
        for(i = 0;i < num; i++, j++)
        {
            // Convert from 16bit signed pcm to 8bits unsigned pcm
            *(c_ptr + j) = (*(ptr + i) >> 8) + 127;
        }
        size >>= 1;
    }
    else    // 8bits data
    {
        c_ptr2 = (unsigned char *)(wbuf);
        ptr2 = (char *)buf;
        for(i1=0;i1<1024;i1++)
        {
            *(c_ptr2+i1)= *(ptr2+i1)^0x80;//+127;
        }
       // memcpy(wbuf, buf, size);
    }

    //Sample rate convert case should be added here!!!!!!!
    if(avifile.aviInfo->samplerate != 8000){	//Handle sample rate 16k case
    }

    if(avifile.pcmNum < PCMCACHE_NUM - 1){
        avifile.pcmNum++;
    }
    else{
        wbuf = avifile.abuf;
        avifile.pcmNum = 0;
        p = dwEndianConvert(size * PCMCACHE_NUM);

        memcpy(wbuf, chunkId, 4);
        memcpy((wbuf + 4), (char *)&p, 4);

        //mpDebugPrint("JJ. %x %x %x %x", wbuf[0], wbuf[1], wbuf[2], wbuf[3]);
        MagicPixel_AVIEN_write_callback(wbuf, size * PCMCACHE_NUM + 8);


        idx_ptr[idx_cnt++] = 1;
        pcm_cnt++;
        //===================
//=======================
#if 1
        idx_1->fccType = mmioFOURCC('0', '1', 'w', 'b');
        idx_1->dwFlag = dwEndianConvert(AVIIF_KEYFRAME);
        idx_1->dwChunkOffset = dwEndianConvert(off_1 - 328);
        idx_1->dwChunkLength = dwEndianConvert(pcm_size_1);
        off_1 += pcm_size_1 + 8;
        avifile.index_count++;
        MagicPixel_AVIEN_INDEX(idx_1,1);
#endif
        //=======================
        //mpDebugPrint("a");
        //===================
    }

    avifile.aTimestampCnt += avifile.aTimestamp;
    unlock_semaph(1);

    return 0;
}
#endif


void getPcmInfo(int format, int ch, int samplerate, int bitpersample)
{
    bitpersample = 8;		//Fix sample size temporarily
    wavformatex.wFormatTag       = format;
    wavformatex.nChannels        = ch;
    wavformatex.nSamplesPerSec   = samplerate;
    wavformatex.nAvgBytesPerSec  = ch * samplerate * (bitpersample >> 3);
    wavformatex.nBlockAlign      = (bitpersample >> 3) * ch;
    wavformatex.wBitsPerSample   = bitpersample;
}

int AVIEN_closefile()
{
    int i,timeout=0, error;
    int j_size, p_size;
    int sample_rate, sampleSize;
    unsigned int hdrlSz,moviSz,idx1Sz,riffSz;
    char *buf;
    AVIINDEXENTRY *idx;
    unsigned int off, cnt;
   // unsigned int jpg_size = 0;
    unsigned int pcm_size = 0;
    unsigned int j_cnt, p_cnt;

//	if(aviPackerCnt < 400){
//		return 0;
//	}

	lock_semaph();
    aviend = 1;
    aviStart = 0;
#if AUTO_GET_RECORD_FPS_TIMER
	TimerToGetRecordFPS();
#endif
    sample_rate = avifile.aviInfo->samplerate;
    sampleSize  = avifile.aviInfo->samplesize;
    j_size      = avifile.aviInfo->imgSz;
    p_size      = avifile.aviInfo->pcmSz;
	mpDebugPrint("--p_size_1=%d",p_size);
	if(avifile.aviInfo->samplesize ==16)
	{
    p_size = (p_size >> 1) * PCMCACHE_NUM;	// Because we change to 8 bit mode, p_size is divided by 2.
	}
	else
	{
		 p_size = p_size * PCMCACHE_NUM;
	}
	mpDebugPrint("--p_size_2=%d",p_size);
	//jpg_size = j_size * jpg_cnt;
	pcm_size = p_size * pcm_cnt;
	mpDebugPrint("--pcm_size=%d",pcm_size);
	mpDebugPrint("--pcm_cnt=%d",pcm_cnt);
//--------------------------
hdrlSz = VIDEOstrl_SIZE + 8 + MAINAVIHEADER_SIZE + 8 + 4 + 100;
  // moviSz = jpg_size + pcm_size + (jpg_cnt + pcm_cnt) * 8 + 4;

	  moviSz = jpg_size_total + pcm_size + (jpg_cnt + pcm_cnt) * 8 + 4;

   //idx1Sz = 16 * (jpg_cnt + pcm_cnt);
   idx1Sz = 16 *avifile.index_count;
    // idx1Sz=0;
   riffSz = hdrlSz + moviSz + idx1Sz + 8 + 8 + 8 + 4;



    // ---- Generate AVI Index Section ----

//    wFourccHeader("idx1", idx1Sz);	//Because we just have three image frames, including an audio idx1
	index_close=1;
	aviStreamForceFlush();	//Flush all data before filePostGet
	index_close=0;

	memset(&wavformatex, 0, sizeof(wavformatex));
	   getPcmInfo(0x1, 1, sample_rate, sampleSize);

//******************************************************************************************************

#if  1
//+++oo
#if !WriteBack_FileSize
    if (FileForceSync(avifile.index_handle) != FS_SUCCEED)
    {
        MP_ALERT("--E-- FileForceSync index file fail !!!");
        __asm("break 100");
    }
#endif
/*    avifile.index_handle = PathAPI__Fopen((BYTE *) outName, "rb");
	
    if (avifile.index_handle == NULL)
    {
        MP_ALERT("--E-- Open %s fail !!!", outName);
        __asm("break 100");
    }
*/
    #define TRUCK_SIZE              (64 * 1024)
    Fseek(avifile.index_handle, 0, SEEK_SET);
    BYTE *buffer_ind = NULL;
    DWORD bufferIndSize = TRUCK_SIZE;

    while (!buffer_ind && (bufferIndSize >= 16))
    {
        buffer_ind = (BYTE *) MagicPixel_AVIEN_malloc_callback(bufferIndSize);

        if ((DWORD) buffer_ind == 0xA0000000)
            bufferIndSize >>= 1;
    }

    if (bufferIndSize <= 16)
    {
        MP_ALERT("--E-- malloc for buffer_ind fail !!!");
        __asm("break 100");
    }

    int ind=0;
	mpDebugPrint("-E-index chunk write ok-");
	wFourccHeader_index("idx1", idx1Sz,avifile.pFile);

    while (avifile.index_count > 0)
    {
        if (avifile.index_count > (bufferIndSize / 16))
        {
            avifile.index_count = avifile.index_count - (bufferIndSize / 16);
            FileRead(avifile.index_handle,buffer_ind, bufferIndSize);
			//+++oo
#if WriteBack_FileSize
			FileWrite_withWriteBack_FileSize(avifile.pFile,buffer_ind, bufferIndSize,1);
#else
            FileWrite(avifile.pFile,buffer_ind, bufferIndSize);
#endif
//            mpDebugPrint("0=%u",avifile.index_count);
        }
        else
        {
            FileRead(avifile.index_handle,buffer_ind, avifile.index_count*16);
            //+++oo
#if WriteBack_FileSize
			FileWrite_withWriteBack_FileSize(avifile.pFile,buffer_ind, avifile.index_count*16,1);
#else
			FileWrite(avifile.pFile,buffer_ind, avifile.index_count*16);
#endif
            avifile.index_count=0;
//            mpDebugPrint("1=%u",avifile.index_count);
        }
    }
	//DWORD real_data_end = avifile.pFile->Chain.Point;
	DWORD real_data_end =FilePosGet(avifile.pFile);
    MagicPixel_AVIEN_free_callback((void *) buffer_ind);
    // FileClose(avifile.index_handle);

    if (DeleteFile(avifile.index_handle) != FS_SUCCEED)
    {
        MP_ALERT("--E-- Delete index file fail !!!!");
        __asm("break 100");
    }

    avifile.index_handle = NULL;
#endif

	
    MagicPixel_AVIEN_OutSeek_callback(0); //seek to start position of output file
   
   
    wLISTHeader("RIFF", riffSz, "AVI ");
    // ---- Generate AVI header  ----
    // Build AVI RIFF header
      
    build_AVI_Header();
    // ---- Generate AVI Data Section ----
    wLISTHeader("LIST", moviSz, "movi");

    aviStreamForceFlush();	//Flush residual data
	Fseek(avifile.pFile, real_data_end, SEEK_SET);
    MagicPixel_AVIEN_fileClose_callback(avifile.pFile);
    //MagicPixel_AVIEN_fileClose_callback(avifile.index_handle);
    MagicPixel_AVIEN_free_callback(avifile.aviInfo);
    avifile.aviInfo = NULL;
    MagicPixel_AVIEN_free_callback(avifile.jbuf);
    avifile.jbuf = NULL;
    MagicPixel_AVIEN_free_callback(avifile.abuf);
    avifile.abuf = NULL;
    aviStreamUninit();
    MagicPixel_AVIEN_free_callback(idx_ptr);
    idx_ptr = NULL;
	MagicPixel_AVIEN_free_callback(idx_1);
    idx_1 = NULL;

    idx_cnt = 0;
    jpg_cnt = 0;
    pcm_cnt = 0;
	aviend = 0;  //+++
	aviStart = 0;//+++
	jpg_size_total=0;
	avifile.index_count=0;
	moviSz=0;
	idx1Sz=0;
	//TaskTerminate(AVI_AUDIO_TASK);//+++
	unlock_semaph(1);
    return 0;
}


int ExceptionHandle()
{
	lock_semaph();
	//MagicPixel_AVIEN_free_callback(buf);
	//DeleteFile(avifile.index_handle);
	FileClose_NoWriteBack(avifile.index_handle);
    avifile.index_handle = NULL;
	//MagicPixel_AVIEN_fileClose_callback(avifile.pFile);
    FileClose_NoWriteBack(avifile.pFile);
	avifile.pFile = NULL;
	//MagicPixel_AVIEN_fileClose_callback(avifile.index_handle);
    //avifile.index_handle = NULL;
    MagicPixel_AVIEN_free_callback(avifile.aviInfo);
    avifile.aviInfo = NULL;
    MagicPixel_AVIEN_free_callback(avifile.jbuf);
    avifile.jbuf = NULL;

	if (avifile.abuf)
    	MagicPixel_AVIEN_free_callback(avifile.abuf);
	
    avifile.abuf = NULL;
    aviStreamUninit();
    MagicPixel_AVIEN_free_callback(idx_ptr);
    idx_ptr = NULL;
	MagicPixel_AVIEN_free_callback(idx_1);
    idx_1 = NULL;
	idx_cnt = 0;
    jpg_cnt = 0;
    pcm_cnt = 0;
	aviend = 0;  //+++
	aviStart = 0;//+++
	jpg_size_total=0;
	avifile.index_count=0;
	unlock_semaph(1);
    return 0;
}

#if RECORD_AUDIO
#include "../../audio/INCLUDE/wavUtil.h"

//This function just accept a 44byte buffer
void CreateWavHeader(WAV_CONFIG *config, BYTE *buf){	int size = 0;	WAV_HEADER *wHeader = (WAV_HEADER *)buf;	//if(sizeof(buf) != 44){	//	mpDebugPrint("Wrong input buffer size: %d", buf);		//return;	//}	mpDebugPrint("Wav size: %d", config->DataSize + 36);	wHeader->chunkId         = 0x52494646;	                             // RIFF	//wHeader->chunkSize       = dwEndianConvert(config->DataSize + 36);	wHeader->chunkSize       = dwEndianConvert(config->DataSize - 8);	wHeader->format          = 0x57415645;	                             // WAVE	wHeader->subchunkId1     = 0x666d7420;	                             // "fmt "	wHeader->subchunkId1size = dwEndianConvert(16);                      //  16 in PCM CASE	wHeader->audioFormat     = wEndianConvert(1);                        //  1 means PCM	   ---> word	wHeader->numChannel      = wEndianConvert(config->Channels);         //  ---> word	wHeader->sampleRate      = dwEndianConvert(config->SampleRate);	wHeader->byteRate        = dwEndianConvert(config->AvgBytesPerSec);	wHeader->blockAlign      = wEndianConvert(config->BlockAlign);       //  ---> word	wHeader->bitPerSample    = wEndianConvert(config->BitsPerSample);    //  ---> word	wHeader->subchunkId2     = 0x64617461;                               // "data"	wHeader->subchunkId2size = dwEndianConvert(config->DataSize - 44);}

int Audio_createfile(STREAM *file_handle,DWORD imgSz, DWORD wavSz, DWORD sampleRate, DWORD sampleSize)
{
	int ret;
	char wavHeader[44];

	memset(&wavHeader, 0, sizeof(wavHeader));

        avifile.pFile   =file_handle; //MagicPixel_AVIEN_fileOpen_callback(outFile, "wb");
        avifile.aviInfo->pcmSz = wavSz;
        avifile.aviInfo->samplerate = sampleRate;
        avifile.aviInfo->samplesize = sampleSize;
	mpDebugPrint("sampleRate = %4d, sampleSize = %2d",sampleRate, sampleSize);
        aviStart = 1;
        pcm_cnt = 0;

       avifile.pcmNum  = 0;
       avifile.abuf    = (BYTE *) MagicPixel_AVIEN_malloc_callback((wavSz * PCMCACHE_NUM));

       if( (DWORD) avifile.abuf == 0xA0000000 )
       {
        mpDebugPrint("audio cache alloc fail..., system hangs");
        __asm("break 100");
       }	  
	   
	ret = FileWrite( avifile.pFile, wavHeader, 44);	

		
        return 0;
}

int Audio_fillPcm(unsigned char *buf, DWORD bufLen)
{
    unsigned int p;
    int size;
    int i, j, num;
    short *ptr;
    unsigned char *ptr2;
    unsigned char *c_ptr,*c_ptr2;
    char *wbuf;
    int ret;
    int i1 = 0;	
	
    lock_semaph();

    if (aviend || !aviStart)
    {
        unlock_semaph(1);
        return 0;
    }

    //mpDebugPrint("wav write: %d/%d", aviend, aviStart);
    size = avifile.aviInfo->pcmSz;
	
    wbuf = avifile.abuf + ((avifile.aviInfo->pcmSz ) * avifile.pcmNum);
	
#if 1
    //mpDebugPrint("a: %x/%x", wbuf, avifile.abuf);
    if ( 0 ) //avifile.aviInfo->samplesize != 8)
    {
        //mpDebugPrint("convert...from %d to 8", avifile.aviInfo->samplesize);
        ptr = (short *)buf;
        c_ptr = (unsigned char *)(wbuf);
        num = size >> 1;
        j = 0;
        for(i = 0;i < num; i++, j++)
        {
            // Convert from 16bit signed pcm to 8bits unsigned pcm
            *(c_ptr + j) = (*(ptr + i) >> 8) + 127;
        }
        size >>= 1;
    }
    else    // 8bits data
    {
        c_ptr2 = (unsigned char *)(wbuf);
        ptr2 = (char *)buf;
        for(i1=0;i1<1024;i1++)
        {
            *(c_ptr2+i1)= *(ptr2+i1)^0x80;//+127;
//            *(c_ptr2+i1)= *(ptr2+i1);
        }
       // memcpy(wbuf, buf, size);
    }

    //Sample rate convert case should be added here!!!!!!!
    if(avifile.aviInfo->samplerate != 8000){	//Handle sample rate 16k case
    }
#endif

    if(avifile.pcmNum < PCMCACHE_NUM - 1){
        avifile.pcmNum++;
    }
    else{
        wbuf = avifile.abuf;		
        avifile.pcmNum = 0;
	ret = FileWrite(avifile.pFile, wbuf, size * PCMCACHE_NUM);
	//mpDebugPrint("fill size: %d", ret);
	if( ret )  pcm_cnt++;
    }
	
    unlock_semaph(1);

    return 0;
}

int Audio_closefile(void)
{
	WAV_CONFIG pcmInfo;

	char wavHeader[44];

	DWORD p_size;	

	memset(&wavHeader, 0, sizeof(wavHeader));
	memset(&pcmInfo, 0, sizeof(pcmInfo));


	lock_semaph();
	
        aviend = 1;
        aviStart = 0;
        p_size  = avifile.aviInfo->pcmSz;
	mpDebugPrint("--p_size_1=%d",p_size);
	if( 0) //avifile.aviInfo->samplesize ==16)
	{
                  p_size = (p_size >> 1) * PCMCACHE_NUM;	// Because we change to 8 bit mode, p_size is divided by 2.
	}
	else
	{
		  p_size = p_size * PCMCACHE_NUM;
	}
	mpDebugPrint("--p_size_2=%d",p_size);
	#if 0
	pcmInfo.BitsPerSample  = avifile.aviInfo->samplesize;
	pcmInfo.Channels       = 1;
	pcmInfo.DataSize       = p_size * pcm_cnt;
	pcmInfo.SampleRate     = avifile.aviInfo->samplerate;
	pcmInfo.BlockAlign     = pcmInfo.Channels * pcmInfo.BitsPerSample / 8;
	pcmInfo.AvgBytesPerSec = pcmInfo.SampleRate * pcmInfo.Channels * pcmInfo.BitsPerSample / 8;
	#endif

	#if 1  
	pcmInfo.BitsPerSample  = 8;
	pcmInfo.Channels       = 1;
	pcmInfo.DataSize       = p_size * pcm_cnt;
	pcmInfo.SampleRate     = 8000;
	pcmInfo.BlockAlign     = pcmInfo.Channels * pcmInfo.BitsPerSample / 8;
	pcmInfo.AvgBytesPerSec = pcmInfo.SampleRate * pcmInfo.Channels * pcmInfo.BitsPerSample / 8;
	#endif

	mpDebugPrint("Audio WAV Header Info:");
	mpDebugPrint("pcmInfo.BlockAlign = %d ,pcmInfo.Channels = %d ,pcmInfo.BitsPerSample = %d",pcmInfo.BlockAlign,pcmInfo.Channels,pcmInfo.BitsPerSample);

	CreateWavHeader(&pcmInfo, wavHeader);
	SeekSet(avifile.pFile);
	FileWrite(avifile.pFile, wavHeader, 44);
	FileClose(avifile.pFile);
	avifile.pFile = NULL;
        MagicPixel_AVIEN_free_callback(avifile.abuf);
        avifile.abuf = NULL;
	mpDebugPrint("write file OK, size: %d", p_size * pcm_cnt);

        pcm_cnt = 0;
        aviend = 0;
        aviStart = 0;
		
	unlock_semaph(1);
	
        return 0;
}

#endif

#endif //RECORD_ENABLE


