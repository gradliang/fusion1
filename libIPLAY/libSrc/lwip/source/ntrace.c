
#define LOCAL_DEBUG_ENABLE 0

//#include <stdio.h>
//#include <stdarg.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include "ndebug.h"
#include "mpTrace.h"
#include "os_mp52x.h"


#if NET_TRACE_ENABLE > 0

#define TRACE_FILE_NAME 	"TRACE"
#define TRACE_DRIVE_ID     (SD_MMC)

#define MAX_TRACE_LINE  80

//#define STR_VERSION  "0.1"

struct net_trace {
    int trace_read;
    int trace_write;
    bool trace_bind;
#define TRACE_SIZE  (640 * 1024)
    char trace_buffer[TRACE_SIZE];
};

static struct net_trace ntrace;
static int ntrace_lock;
static bool trace_init;

//static inline int read_inc(int rd, int len)
static int read_inc(int rd, int len)
{
    int wr = ntrace.trace_write;
    if (rd < wr)
    {
        rd += len + 1;
        if (rd > wr)
            rd = wr;
    }
    else
    {
        rd += len + 1;
        if (rd + MAX_TRACE_LINE >= TRACE_SIZE)
            rd = 0;
    }
    return rd;
}

//static inline int write_inc(int wr, int len)
static int write_inc(int wr, int len)
{
    int rd = ntrace.trace_read;
    if (wr < rd)
    {
        wr += len + 1;
        if (wr >= rd)
            rd = read_inc(rd, strlen(&ntrace.trace_buffer[rd]));
    }
    else 
    {
        wr += len + 1;
        if (wr + MAX_TRACE_LINE >= TRACE_SIZE)
            wr = 0;
    }

    ntrace.trace_read = rd;

    return wr;
}

void ntrace_printf(int level, char *fmt, ...)
{
	va_list ap;
	const int buflen = MAX_TRACE_LINE-10+1;
    char buffer[MAX_TRACE_LINE-10+1];
	int size,wr;
    int cur;
    int len;

    MP_DEBUG("--> %s", __func__);
    mpx_SemaphoreWait(ntrace_lock);

//    if (level < mpx_debug_level)
//        return;
    wr = ntrace.trace_write;
    cur = jiffies;

    va_start(ap, fmt);
    size = vsnprintf(buffer, buflen, fmt, ap);
    va_end(ap);

    len = snprintf(&ntrace.trace_buffer[wr], MAX_TRACE_LINE+1,
            "%05d.%03d %s", cur/HZ, (cur%HZ) * 1000/HZ, buffer);

    ntrace.trace_write = wr + len + 1;

    if (ntrace.trace_write + (MAX_TRACE_LINE * 2) >= TRACE_SIZE)
    {
		mpDebugPrint("ntrace: overflow");
        ntrace.trace_write = 0;
        ntrace.trace_bind = true;
    }

    mpx_SemaphoreRelease(ntrace_lock);
    MP_DEBUG("<-- %s", __func__);
    return;
}

void ntrace_reset(void)
{
    mpx_SemaphoreWait(ntrace_lock);
    ntrace.trace_write = ntrace.trace_read = 0;
    ntrace.trace_bind = false;
    mpx_SemaphoreRelease(ntrace_lock);
}

#ifdef LINUX
static void trace_create(void)
{		
	DRIVE * pDrive = DriveGet(TRACE_DRIVE_ID);
	STREAM * pHandle;
	

	int iRet=CreateFile(pDrive, "TRACE", "TXT");
	if (iRet == 0)
	{
		mpDebugPrint("##### Create trace file success");
		pHandle = FileOpen(pDrive);
		FileWrite(pHandle, "*** TRACE START ***\r\n", 21);
		FileClose(pHandle);
	}
	else
		mpDebugPrint("Create trace file failed");
}
#endif

#if LOG_TO_SD
void ntrace_write2sd(void)
{
	STREAM * pHandle;
    int rd,wr;

	if (ntrace.trace_write == ntrace.trace_read)
		return;

    mpDebugPrint("%s:%d,fn=%s", __func__, __LINE__, TRACE_FILE_NAME ".TXT");
	if (! Mcard_GetFlagPresent(TRACE_DRIVE_ID))
	{
		mpDebugPrint("SD card not found");
		return;
	}

#if 0
	if( Mcard_GetFlagReadOnly(TRACE_DRIVE_ID))
	{
		mpDebugPrint("SD card read-only");	
		return;
	}
#endif

	BYTE OldDriveId = DriveCurIdGet();
	
	pHandle = FileOpenByName(TRACE_DRIVE_ID, TRACE_FILE_NAME,"TXT");

	if (! pHandle)
	{
		trace_create();
		pHandle = FileOpenByName(TRACE_DRIVE_ID, TRACE_FILE_NAME,"TXT");
	}	

	if (pHandle)
	{		
		EndOfFile(pHandle); // move to end of file	
        if (!trace_init)
        {
            const BYTE *iccid, *imei;
            trace_init = true;
//            EdgeGetImeiCcid(&iccid, &imei);
            FileWrite(pHandle, iccid, strlen(iccid));
            FileWrite(pHandle, "\r\n",2);
            FileWrite(pHandle, STR_VERSION, strlen(STR_VERSION));
            FileWrite(pHandle, "\r\n",2);
        }
		
        mpx_SemaphoreWait(ntrace_lock);

        wr = ntrace.trace_write;
        rd = ntrace.trace_read;
//		mpDebugPrint("%s: wr=%d,rd=%d", __func__,wr,rd);
        while (wr != rd)
        {
			char *chp = &ntrace.trace_buffer[rd];
            int len = strlen(chp);
            ntrace.trace_read = read_inc(rd, len);
			mpx_SemaphoreRelease(ntrace_lock);
            FileWrite(pHandle, chp, len);
            FileWrite(pHandle, "\r\n",2);

			mpx_SemaphoreWait(ntrace_lock);
			rd = ntrace.trace_read;
        }
        ntrace.trace_read = rd;

        mpx_SemaphoreRelease(ntrace_lock);

        FileClose(pHandle);
	}
	else
	{
		mpDebugPrint("trace file open failed");
	}
	
	DriveChange(OldDriveId); // jeffery , 20100325
}
#endif

void ntrace_write2flash(void *fp)
{
    STREAM * pHandle = fp;
    int rd,wr;

    if (!pHandle)
		return;

    FileWrite(pHandle, "*** TRACE Starts ***\r\n", 22);

	if (ntrace.trace_write == ntrace.trace_read)
    {
        FileWrite(pHandle, "*** TRACE Ends ***\r\n", 20);
		return;
    }

    mpx_SemaphoreWait(ntrace_lock);

    wr = ntrace.trace_write;
    rd = ntrace.trace_read;
//		mpDebugPrint("%s: wr=%d,rd=%d", __func__,wr,rd);
    while (wr != rd)
    {
        char *chp = &ntrace.trace_buffer[rd];
        int len = strlen(chp);
        ntrace.trace_read = read_inc(rd, len);
        mpx_SemaphoreRelease(ntrace_lock);
        FileWrite(pHandle, chp, len);
        FileWrite(pHandle, "\r\n",2);

        mpx_SemaphoreWait(ntrace_lock);
        rd = ntrace.trace_read;
    }
    ntrace.trace_read = rd;

    mpx_SemaphoreRelease(ntrace_lock);
    FileWrite(pHandle, "*** TRACE Ends ***\r\n", 20);
}

/*
 * 32 chars in hex is 80 characters long.
 */
void ntrace_dump(int level, uint32_t address, uint32_t size)
{
    uint8_t* ptr = (uint8_t*)address;
    int i = size;
    int cnt, len, bytecnt;
    int val;
    char buf[MAX_TRACE_LINE+2];
    int wr;
    int cur = jiffies;
    
    MP_DEBUG("--> %s", __func__);
    mpx_SemaphoreWait(ntrace_lock);

    if (ntrace.trace_write + (size * 2) >= TRACE_SIZE)
        ntrace.trace_write = 0;

    wr = ntrace.trace_write;

    len = 0;
    bytecnt = cnt = 0;
    while(size > 0)
    {
        size--;
        ++cnt;
        bytecnt++;

        val = *ptr;
        val = (val >> 4) & 0x0f;
        if (val <= 9)
            buf[len++] = val + '0';
        else
            buf[len++] = val - 10 + 'a';
        val = *ptr++;
        val = val & 0x0f;
        if (val <= 9)
            buf[len++] = val + '0';
        else
            buf[len++] = val - 10 + 'a';
        if (cnt == 2)
        {
            buf[len++] = ' ';
            cnt = 0;
        }
        if (bytecnt >= 32)
        {
            buf[len] = '\0';
            len = snprintf(&ntrace.trace_buffer[wr], MAX_TRACE_LINE+1, 
                "%05d.%03d %s", cur/HZ, (cur%HZ) * 1000/HZ, buf);
            wr = write_inc(wr, len);
            len = 0;
            bytecnt = 0;
        }
        else if (size == 0)
        {
            buf[len] = '\0';
            len = snprintf(&ntrace.trace_buffer[wr], MAX_TRACE_LINE+1, 
                "%05d.%03d %s", cur/HZ, (cur%HZ) * 1000/HZ, buf);
            wr = write_inc(wr, len);
            len = 0;
            bytecnt = 0;
        }
    }

    ntrace.trace_write = wr;
    mpx_SemaphoreRelease(ntrace_lock);
    MP_DEBUG("<-- %s", __func__);
}

void ntrace_init(void)
{
	uint32_t status;
    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    MP_ASSERT(status > 0);
	ntrace_lock = status;
    ntrace.trace_buffer[TRACE_SIZE - 1] = '\0';
}

void ntrace_show(void)
{
    int rd,wr;

    if (ntrace.trace_bind)
    {
        ntrace.trace_read = read_inc(ntrace.trace_write, 0);
        ntrace.trace_bind = false;
    }

    mpDebugPrint("%s rd=%u,wr=%u", __func__, ntrace.trace_read, ntrace.trace_write);
	if (ntrace.trace_write == ntrace.trace_read)
		return;

    mpDebugPrint("%s:%d,fn=%s", __func__, __LINE__, TRACE_FILE_NAME ".TXT");
	
	{		
        mpx_SemaphoreWait(ntrace_lock);

        wr = ntrace.trace_write;
        rd = ntrace.trace_read;
        while (wr != rd)
        {
			char *chp = &ntrace.trace_buffer[rd];
            int len = strlen(chp);
            ntrace.trace_read = read_inc(rd, len);
			mpx_SemaphoreRelease(ntrace_lock);
            mpDebugPrint(chp);

			mpx_SemaphoreWait(ntrace_lock);
			rd = ntrace.trace_read;
        }
        ntrace.trace_read = rd;

        mpx_SemaphoreRelease(ntrace_lock);

	}
	

}

#endif
