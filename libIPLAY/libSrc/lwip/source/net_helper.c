// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 0

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "global612.h"
#include "mpTrace.h"
#include "linux/types.h"
#include "linux/list.h"

#include "uip.h"
#include "net_sys.h"
#include "timer.h"
#include "taskid.h"
#include "netware.h"
#include "devio.h"
#include "net_autosearch.h"
#include "net_nic.h"
#include "net_dhcp.h"
#include "..\..\xml\include\netfs.h"
#include "ndebug.h"

#include "net_device.h"
#include "wlan_sys.h"
//#include "os_defs.h"
#include "wireless.h"
#include "os.h"

static ST_SYSTEM_TIME net_clock;

void SystemTimeGet(ST_SYSTEM_TIME *curr_time);
BYTE GetCurYear();
BYTE GetCurMonth();
BYTE GetCurDate();
BYTE GetCurHour();
BYTE GetCurMinute();
BYTE GetCurSecond();

/*
 * Get current date/time in W3C datetime format.
 * This function requires CURL library.
 *
 * clock has to be at least 30 in length (including '\0').
 */
char * GetCurDateTimeW3C(char *clock)
{
    static short datetime_msec;    /* 0 to 999 milliseconds */
    char prefix;
    long toff, offhr, offmin;                   /* time zone offset */
    char *ptr = getenv("TZ");
#if 1
	sprintf(clock, "%04d-%02d-%02dT%02d:%02d:%02d.%dZ",
            8+2000, 12, 2,
            15, 43, 33,
            300);
	return clock;
#endif
    if (!ptr)
    {
        clock[0] = '\0';
        return NULL;
    }
#if HAVE_CURL
    if ((toff=GetTzOffset(ptr)) == -1)
#else
    if (1)
#endif
    {
        clock[0] = '\0';
        return NULL;
    }
    if (toff < 0)
    {
        prefix = '+';
        toff = -toff;
    }
    else
    {
        prefix = '-';
    }
    offhr = toff / 3600;
    offmin = (toff % 3600) / 60;

    SystemTimeGet(&net_clock);
	sprintf(clock, "%04d-%02d-%02dT%02d:%02d:%02d.%d%c%02d%02d",
            GetCurYear()+2000, GetCurMonth(), GetCurDate(),
            GetCurHour(), GetCurMinute(), GetCurSecond(),
            300, prefix, offhr, offmin);
//            datetime_msec++, prefix, offhr, offmin);
//	mpDebugPrint("clock: %s", clock);

    if (datetime_msec > 999)
        datetime_msec = 0;
    return clock;
}

#define TM_YEAR_BASE    1900
static int GetLocalTime (struct tm *tm)
{
  SystemTimeGet(&net_clock);
  tm->tm_year = GetCurYear() + 2000 - TM_YEAR_BASE;
  tm->tm_mon = GetCurMonth() - 1;
  tm->tm_mday = GetCurDate();
  tm->tm_isdst = 0;
  tm->tm_sec = GetCurSecond();
  tm->tm_min = GetCurMinute();
  tm->tm_hour = GetCurHour();

  return 0;
}

static u32 time_usec;
#define HAVE_TIMEFUNC
#ifdef HAVE_TIMEFUNC
#define time(tloc)	mpx_time(tloc)
#endif
static u32 _first_tick;
time_t time(time_t *tloc)
{
    struct tm tm;
    static bool localtime_initialized;
    static u32 last_tick;
    static time_t local;                   /* local time */
    u32 diff;                       /* accumulated ticks */
    float f2, f3;
    int d1;
    time_t ret;

    if (!localtime_initialized)
    {
        memset(&tm, 0, sizeof(struct tm));
        GetLocalTime(&tm);
        local = mktime(&tm);
        _first_tick = 
        last_tick = TickCount;
        localtime_initialized = true;
    }

    diff = (long)TickCount - (long)_first_tick;
    f2 = diff;
    f2 = f2 / HZ;
    d1 = diff / HZ;
    f2 = f2  - d1;
    f3 = f2 * 1000000;
    time_usec = f3;
    ret = local + d1;
    return ret;
}

int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	tp->tv_sec = mpx_time(NULL);
	tp->tv_usec = time_usec;
	return 0;
}

BYTE GetCurYear()
{
    return net_clock.u16Year - 2000;
}
BYTE GetCurMonth()
{
    return net_clock.u08Month;
}
BYTE GetCurDate()
{
    return net_clock.u08Day;
}
BYTE GetCurHour()
{
    return net_clock.u08Hour;
}
BYTE GetCurMinute()
{
    return net_clock.u08Minute;
}

BYTE GetCurSecond()
{
    return net_clock.u08Second;
}

int vasprintf(char **strp, const char *fmt, va_list ap)
{
    char *p = mm_malloc(256);

    if (!p)
        return -1;

    *strp = p;
    return vsnprintf(p, 256, fmt, ap);
}

void ktime_get_ts(struct timespec *ts)
{
    ts->tv_sec = GetSysTime() / 1000;
}

void synchronize_net(void)
{

}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))
#define BITOP_WORD(nr)          ((nr) / BITS_PER_LONG)
#include <linux/bitops.h>
#endif

/*
 * This implementation of find_{first,next}_zero_bit was stolen from
 * Linus' asm-alpha/bitops.h.
 */
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
                                 unsigned long offset)
{
    const unsigned long *p = addr + BITOP_WORD(offset);
    unsigned long result = offset & ~(BITS_PER_LONG-1);
    unsigned long tmp;

    if (offset >= size)
        return size;
    size -= result;
    offset %= BITS_PER_LONG;
    if (offset) {
        tmp = *(p++);
        tmp |= ~0UL >> (BITS_PER_LONG - offset);
        if (size < BITS_PER_LONG)
            goto found_first;
        if (~tmp)
            goto found_middle;
        size -= BITS_PER_LONG;
        result += BITS_PER_LONG;
    }
    while (size & ~(BITS_PER_LONG-1)) {
        if (~(tmp = *(p++)))
            goto found_middle;
        result += BITS_PER_LONG;
        size -= BITS_PER_LONG;
    }
    if (!size)
        return result;
    tmp = *p;

found_first:
    tmp |= ~0UL << size;
    if (tmp == ~0UL)        /* Are any bits zero? */
        return result + size;   /* Nope. */
found_middle:
    return result + ffz(tmp);
}

unsigned char _ctype[12];

u32 random32()
{
    MP_ASSERT(0);
    return 0;
}

void compat_led_brightness_set(struct led_classdev *led_cdev,
			       enum led_brightness brightness)
{
#ifdef LINUX
	struct led_timer *led = led_get_timer(led_cdev);

	if (led)
		led_stop_software_blink(led);

	return led_cdev->brightness_set(led_cdev, brightness);
#else
    MP_ASSERT(0);
#endif
}
EXPORT_SYMBOL(compat_led_brightness_set);

#ifdef CONFIG_NO_MALLOC
/* ----------  C library functions  ---------- */

void *malloc(size_t size)
{
    mpDebugPrint("malloc called sz=%d", size);
    MP_ASSERT(0);

}

_VOID free(_PTR mem)
{
    mpDebugPrint("free called %p", mem);
    MP_ASSERT(0);
    mpx_Free(mem);

}
strndupa()
{
    mpDebugPrint("strndupa called");
    MP_ASSERT(0);

}
_PTR _malloc_r(struct _reent *ptr, size_t len)
{
    MP_DEBUG("%s called", __func__);
    return mpx_Malloc(len);
}
_VOID _free_r(struct _reent *ptr, _PTR mem)
{
    mpDebugPrint("%s called", __func__);
    mpx_Free(mem);
}
void *_realloc_r(struct _reent *ptr, _PTR mem, size_t sz)
{
    mpDebugPrint("%s called", __func__);
    MP_ASSERT(0);
    return 0;
}
void abort(void)
{
    MP_ASSERT(0);
    exit(-1);
}
void exit(int status)
{
    MP_ASSERT(0);
    exit(-1);
//	__asm("break 100");
}

int fprintf(FILE *stream, const char *format, ...)
{
    MP_ASSERT(0);

}
int printf(const char *format, ...)
{
    MP_ASSERT(0);
}
int __assert(const char *format, ...)
{
    MP_ASSERT(0);
}

#endif
