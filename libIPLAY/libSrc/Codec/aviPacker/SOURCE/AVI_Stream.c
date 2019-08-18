/*
// Include section
*/
#include "global612.h"
#include "AVI_utility.h"
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"
#include "devio.h"
#include "Avifmt.h"
#include "record.h"
#if(RECORD_ENABLE)
#define memcpy			mmcp_memcpy_polling
#define memset			mmcp_memset

#define STREAMCACHE_NUM       4
#define STREAMCACHE_EXP       6
#define AVISTREAMCACHESZ      (256 << 10) //((1024 << 7)+(1024<<6))//+(1024<<5))
#define AVIINDEX_UNIT        16
#define AVIINDEX_total        256
#define AVIINDEX_total_2        (AVIINDEX_total * 2)
#define AVIINDEX_UNIT_2        AVIINDEX_UNIT*AVIINDEX_total_2

#define AVISTREAMCACHETOTALSZ (AVISTREAMCACHESZ * 8)//(AVISTREAMCACHESZ << STREAMCACHE_EXP)
                               //1024*64(<<6)*4(<<2)

#define WriteBack_FileSize 0
extern int task_filewrite;
extern AVIFILE avifile;

extern DWORD t_1,t_2,t_3a,t_3b,t_4a,t_4b,t_5,t_a;

extern DWORD amount1,amount2,amount3a,amount3b,amount4a,amount4b;

extern DWORD FileOpenFinsh;
//static DWORD idx_count;
static BYTE GWAIT = 0;
extern int index_close;

extern DWORD file_size_avi ;
extern DWORD cache_size_avi;
extern DWORD ExceptionCheck;

struct AVI_STREAM_CACHE
{
	BYTE  *buf;
	DWORD  head;
	DWORD  tail;
	BYTE  *index_buf;
	DWORD  idx_count;
};
//=======================
BYTE *getpicturebuf;
//extern STREAM *shandle_jpg;
int jpg_size_2;
//=======================

struct AVI_STREAM_CACHE st_aviStmCache;

void aviStreamTask();

SWORD aviStreamTaskCreate()
{
	SWORD ret;
	//EventCreate(AVIS_EVENT_ID, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
	ret = TaskCreate (WEB_REC_TASK, aviStreamTask, CONTROL_PRIORITY, 0x4000);

	if(ret != OS_STATUS_OK)
		MP_ALERT("avi Stream task crate failed..");

	return ret;
}



SWORD aviStreamStartup()
{
mpDebugPrint("#### aviStreamStartup ... ");
#if 0
	st_aviStmCache.buf = (BYTE*)MagicPixel_AVIEN_malloc_callback(AVISTREAMCACHETOTALSZ);
	st_aviStmCache.index_buf=(BYTE*)MagicPixel_AVIEN_malloc_callback(AVIINDEX_UNIT_2);

	if(st_aviStmCache.buf != 0xA0000000){
		mpDebugPrint("st_aviStmCache buffer: %x", st_aviStmCache.buf);
		st_aviStmCache.head   = 0;
		st_aviStmCache.tail   = 0;
		st_aviStmCache.idx_count=0;
	}
	else{
		mpDebugPrint("Alloc stream fail....shit...");
        __asm("break 100");
		return 0;
	}
#endif

	SDWORD a = TaskStartup(WEB_REC_TASK);
	if(a != OS_STATUS_OK)
	{
		mpDebugPrint("avi stream task initiates fail....");
		return 0;
	}

	return 1;
}



void aviStreamInit()
{
    st_aviStmCache.buf = (BYTE*)MagicPixel_AVIEN_malloc_callback(AVISTREAMCACHETOTALSZ);
    st_aviStmCache.index_buf=(BYTE*)MagicPixel_AVIEN_malloc_callback(AVIINDEX_UNIT_2);
	getpicturebuf=(BYTE*)MagicPixel_AVIEN_malloc_callback(102400);

	file_size_avi=0;
	cache_size_avi=AVISTREAMCACHESZ/1024;
    if(st_aviStmCache.buf != 0xA0000000){
        mpDebugPrint("st_aviStmCache buffer: %x", st_aviStmCache.buf);
        st_aviStmCache.head   = 0;
        st_aviStmCache.tail   = 0;
    }
    else{
        mpDebugPrint("Alloc stream fail....shit...");
        __asm("break 100");
        return 0;
    }

    st_aviStmCache.head = 0;
	st_aviStmCache.tail = 0;
   // TaskTerminate(WEB_REC_TASK);
}



void aviStreamUninit()
{
	MagicPixel_AVIEN_free_callback(st_aviStmCache.buf);
	MagicPixel_AVIEN_free_callback(st_aviStmCache.index_buf);
	MagicPixel_AVIEN_free_callback(getpicturebuf);
	st_aviStmCache.head = 0;
	st_aviStmCache.tail = 0;
	st_aviStmCache.idx_count=0;
   // TaskTerminate(WEB_REC_TASK);
}

DWORD aviStreamGetNullSpace()
{
	DWORD freeSz = 0;

	if(st_aviStmCache.tail >= st_aviStmCache.head){
		freeSz += AVISTREAMCACHETOTALSZ - st_aviStmCache.tail;
		freeSz += st_aviStmCache.head;
	}
	else{
		freeSz = st_aviStmCache.head - st_aviStmCache.tail;
	}

	//mpDebugPrint("	Get FreeSize: %d", freeSz);
	return freeSz;
}

inline void aviStreamSetPivot(DWORD *pivot, DWORD size)
{
    IntDisable();
    //mpDebugPrint("set Pivot %x to postion %d", pivot, size);
	*pivot = size;
    IntEnable();
}
int get_onepicture=0;
int write_onepicture=0;

DWORD aviStreamCache(BYTE *buf, DWORD size)//,AVIINDEXENTRY *index,int index_make)
{
	//mpDebugPrint("1");
	//========================
	//MpMemCopy(st_aviStmCache.index_buf +index_count*16, index, 16);
    //========================
    //==========
    if(get_onepicture==1 && size >2000)
    {
		get_onepicture=0;
		write_onepicture=1;
		jpg_size_2=size;
    memcpy(getpicturebuf, buf, size);
	EventSet(AUDIO_ID, BIT3);
	mpDebugPrint("(1)jpg_size_2=%d",jpg_size_2);
	}
	

	//==========
	if(aviStreamGetNullSpace() > size){
		if(st_aviStmCache.tail >= st_aviStmCache.head){
			DWORD tailSz = AVISTREAMCACHETOTALSZ - st_aviStmCache.tail;

			if(tailSz > size){

				memcpy(st_aviStmCache.buf + st_aviStmCache.tail, buf, size);

				aviStreamSetPivot(&st_aviStmCache.tail, st_aviStmCache.tail + size);
			}
			else{
				memcpy(st_aviStmCache.buf + st_aviStmCache.tail, buf, tailSz);
				size -= tailSz;
				memcpy(st_aviStmCache.buf, buf + tailSz, size);
				aviStreamSetPivot(&st_aviStmCache.tail, size);
			}
		}
		else{
			memcpy(st_aviStmCache.buf + st_aviStmCache.tail, buf, size);
			aviStreamSetPivot(&st_aviStmCache.tail, st_aviStmCache.tail + size);
		}
	//	mpDebugPrint("2");

		return size;
	}
	else{
		mpDebugPrintChar('D');
		return 0;
	}
}

DWORD aviStreamindex(AVIINDEXENTRY *index,DWORD index_make)
{


memcpy(st_aviStmCache.index_buf+st_aviStmCache.idx_count*AVIINDEX_UNIT, index, AVIINDEX_UNIT);
st_aviStmCache.idx_count++;
if(st_aviStmCache.idx_count==AVIINDEX_total)
{
EventSet(AUDIO_ID,BIT1);

}
if(st_aviStmCache.idx_count==AVIINDEX_total_2)
{

	//PutUartCharDrop('s');
EventSet(AUDIO_ID,BIT2);
	st_aviStmCache.idx_count=0;
}
//=============

	return 0;

}

// We do real writing when buffer size is over "AVISTREAMCACHESZ"


static BYTE aviStreamWriteBack()
{
	
    DWORD start=0,end=0;
   DWORD write_size=0;
	
	

    if(!avifile.pFile){
        mpDebugPrint("There are no avaliable file...");
        return 0;
    }

    if(st_aviStmCache.tail >= st_aviStmCache.head){
        //++++mpDebugPrint("W:%d",(st_aviStmCache.tail - st_aviStmCache.head)/1024);
        if(st_aviStmCache.tail - st_aviStmCache.head >= AVISTREAMCACHESZ){ 
			write_size=AVISTREAMCACHESZ;
		//+++oo
#if WriteBack_FileSize			
	if(FileWrite_withWriteBack_FileSize(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size,1)!=write_size)
#else
	if(FileWrite(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size)!=write_size)
#endif
			{ExceptionCheck=0;return 0;}	
            file_size_avi++;
            aviStreamSetPivot(&st_aviStmCache.head, st_aviStmCache.head + AVISTREAMCACHESZ);
        }
        else{
            //mpDebugPrint("	NN1 %d - %d", st_aviStmCache.head, st_aviStmCache.tail - st_aviStmCache.head);
            return 1;
        }
    }
    else{
        //+++	mpDebugPrint("W:-");
        if(AVISTREAMCACHETOTALSZ - st_aviStmCache.head >= AVISTREAMCACHESZ){
			//***********
			write_size=AVISTREAMCACHESZ;
		//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size,1)!=write_size)
#else
	if(FileWrite(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size)!=write_size)
#endif
	{ExceptionCheck=0;return 0;}	
                file_size_avi++;
            if(st_aviStmCache.head + AVISTREAMCACHESZ >= AVISTREAMCACHETOTALSZ)
                aviStreamSetPivot(&st_aviStmCache.head, 0);
            else
                aviStreamSetPivot(&st_aviStmCache.head, st_aviStmCache.head + AVISTREAMCACHESZ);
        }
        else{
            //mpDebugPrint("	NN2 %d - %d", st_aviStmCache.head, AVISTREAMCACHETOTALSZ - st_aviStmCache.head);
            return 1;
        }
    }

    return 1;
}

static char avitmpbuf[512];
static BYTE aviStreamForceWriteBack()
{
	DWORD write_size=0;
	if(aviStreamGetNullSpace() != AVISTREAMCACHETOTALSZ){
		if(st_aviStmCache.tail >= st_aviStmCache.head){
			write_size=st_aviStmCache.tail - st_aviStmCache.head;
//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size,1)!=write_size)
#else		
	if(FileWrite(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size)!=write_size)
#endif
	{ExceptionCheck=0;return 0;}	
		}
		else{
			//mpDebugPrint("2wwwF: %d", AVISTREAMCACHETOTALSZ - st_aviStmCache.head + st_aviStmCache.tail);
			if(AVISTREAMCACHETOTALSZ - st_aviStmCache.head + st_aviStmCache.tail < 512){

				memcpy(avitmpbuf, st_aviStmCache.buf + st_aviStmCache.head, AVISTREAMCACHETOTALSZ - st_aviStmCache.head);
				memcpy(avitmpbuf + AVISTREAMCACHETOTALSZ - st_aviStmCache.head, st_aviStmCache.buf, st_aviStmCache.tail);
			write_size=AVISTREAMCACHETOTALSZ - st_aviStmCache.head + st_aviStmCache.tail;
//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.pFile, avitmpbuf,write_size,1)!=write_size)
#else		
	if(FileWrite(avifile.pFile, avitmpbuf,write_size)!=write_size)
#endif
			{ExceptionCheck=0;return 0;}	

			}
			else{		
			write_size= AVISTREAMCACHETOTALSZ - st_aviStmCache.head;
//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size,1)!=write_size)
#else		
	if(FileWrite(avifile.pFile, st_aviStmCache.buf + st_aviStmCache.head,write_size)!=write_size)
#endif			
{ExceptionCheck=0;return 0;}	
			write_size=st_aviStmCache.tail;
//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.pFile,st_aviStmCache.buf,write_size,1)!=write_size)
#else
	if(FileWrite(avifile.pFile, st_aviStmCache.buf,write_size)!=write_size)
#endif
{ExceptionCheck=0;return 0;}	
			}
		}

		aviStreamSetPivot(&st_aviStmCache.head, st_aviStmCache.tail);
	}
}

// If this function is called, current task will be blocked until avi stream flush all cache data.
void aviStreamForceFlush()
{
	//mpDebugPrint("aviStreamForceFlush...");
	GWAIT = 1;
    EventSet(AUDIO_ID, BIT0);

	while(GWAIT)
		TaskSleep(10);
}

//After calling aviStreamForceWriteBack(), it brings some errors at normal aviStreamWriteBack() case.
//So we should not support operations of SEEK and GETPOS.
void aviStreamTask()
{
    DWORD aviStreamEvent;
	DWORD write_size=0;
	BYTE a;

mpDebugPrint("#### aviStreamTask() runs ...");
	//while(FileOpenFinsh==0)TaskSleep(20);

	while(1)
	{
        EventWait(AUDIO_ID, BIT0|BIT1|BIT2|BIT3, OS_EVENT_OR, &aviStreamEvent);
		while(ExceptionCheck==0)
				{
			EventClear(AUDIO_ID, ~BIT0);	
			EventClear(AUDIO_ID, ~BIT1);
			EventClear(AUDIO_ID, ~BIT2);	
			EventClear(AUDIO_ID, ~BIT3);
			mpDebugPrint("--a");
			TaskSleep(20);
				}

		if(aviStreamEvent & BIT0){
	    	if(aviStreamWriteBack()==0)
			{a = VIDEO_REC_OP_RECORDING_EXCEPTION;
             MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
			 continue;}
		}

		if ((aviStreamEvent & (BIT1 | BIT2)) == (BIT1 | BIT2))
		{
            MP_ALERT("--E-- ");
		}

		if(aviStreamEvent & BIT1){
			write_size=AVIINDEX_total*AVIINDEX_UNIT;
//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.index_handle,st_aviStmCache.index_buf,write_size,1)!=write_size)
#else	
	if(FileWrite(avifile.index_handle,st_aviStmCache.index_buf,write_size)!=write_size)
#endif
		{ExceptionCheck=0;
			a = VIDEO_REC_OP_RECORDING_EXCEPTION;
             MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
			continue;}	
		}

		if(aviStreamEvent & BIT2){
		//***********
		write_size=AVIINDEX_total*AVIINDEX_UNIT;
		//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.index_handle,st_aviStmCache.index_buf+AVIINDEX_total*AVIINDEX_UNIT,write_size,1)!=write_size)
#else	
	if(FileWrite(avifile.index_handle,st_aviStmCache.index_buf+AVIINDEX_total*AVIINDEX_UNIT,write_size)!=write_size)
#endif
{ExceptionCheck=0;
		a = VIDEO_REC_OP_RECORDING_EXCEPTION;
        MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
		continue;}    
		}
		if(aviStreamEvent & BIT3){
#if 1 //where use is 
			mpDebugPrint("-ERROR  BIT3 ");
#else
		write_size=jpg_size_2;
		//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(shandle_jpg, getpicturebuf,write_size,1)!=write_size)
#else
		if(FileWrite(shandle_jpg, getpicturebuf,write_size)!=write_size)
#endif
{ExceptionCheck=0;
		a = VIDEO_REC_OP_RECORDING_EXCEPTION;
        MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
		continue;}	
		FileClose(shandle_jpg);
		EventSet(REC_ERR_EVENT,BIT3);
#endif
		}
//=========================
		if(GWAIT){
			if(aviStreamForceWriteBack()==0)
			{
			 a = VIDEO_REC_OP_RECORDING_EXCEPTION;
             MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
			continue;
			}
			if(index_close==1)
			{
	if(st_aviStmCache.idx_count<=AVIINDEX_total)
	{
		write_size=st_aviStmCache.idx_count*AVIINDEX_UNIT;
		//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.index_handle,st_aviStmCache.index_buf ,write_size,1)!=write_size)
#else
	if(FileWrite(avifile.index_handle,st_aviStmCache.index_buf ,write_size)!=write_size)
#endif
	{ExceptionCheck=0;
		a = VIDEO_REC_OP_RECORDING_EXCEPTION;
        MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
		continue;}	
	}
	else
	{
	
		write_size=(st_aviStmCache.idx_count-AVIINDEX_total)*AVIINDEX_UNIT;
		//+++oo
#if WriteBack_FileSize
	if(FileWrite_withWriteBack_FileSize(avifile.index_handle,st_aviStmCache.index_buf+(AVIINDEX_total*AVIINDEX_UNIT),write_size,1)!=write_size)
#else
	if(FileWrite(avifile.index_handle,st_aviStmCache.index_buf+(AVIINDEX_total*AVIINDEX_UNIT),write_size)!=write_size)		
#endif
{ExceptionCheck=0;
		a = VIDEO_REC_OP_RECORDING_EXCEPTION;
        MessageSend(REC_OP_MSG_ID,(BYTE *) &a, sizeof(BYTE));
		continue;}	
		
	}
			}
	GWAIT = 0;
		}

	}
}
#endif
