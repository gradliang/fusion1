/* 
   Unix SMB/CIFS implementation.
   time handling functions

   Copyright (C) Andrew Tridgell 		1992-2004
   Copyright (C) Stefan (metze) Metzmacher	2002   

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include "global612.h"
#include "mpTrace.h"
#include "ttime.h"

/**
 * @file
 * @brief time handling functions
 */

#ifndef TIME_T_MIN
/* we use 0 here, because (time_t)-1 means error */
#define TIME_T_MIN 0
#endif

#if (SIZEOF_LONG == 8)
#define TIME_FIXUP_CONSTANT_INT 11644473600L
#elif (SIZEOF_LONG_LONG == 8)
#define TIME_FIXUP_CONSTANT_INT 11644473600LL
#endif


/* The extra casts work around common compiler bugs.  */
#define _TYPE_SIGNED(t) (! ((t) 0 < (t) -1))
/* The outer cast is needed to work around a bug in Cray C 5.0.3.0.
   It is necessary at least when t == time_t.  */
#define _TYPE_MINIMUM(t) ((t) (_TYPE_SIGNED (t) \
  			      ? ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1) : (t) 0))
#define _TYPE_MAXIMUM(t) ((t) (~ (t) 0 - _TYPE_MINIMUM (t)))

/*
 * we use the INT32_MAX here as on 64 bit systems,
 * gmtime() fails with INT64_MAX
 */

#ifndef TIME_T_MAX
#define TIME_T_MAX _TYPE_MAXIMUM(time_t)
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/**
 External access to time_t_min and time_t_max.
**/
_PUBLIC_ time_t get_time_t_max(void)
{
	return TIME_T_MAX;
}

/**
a gettimeofday wrapper
**/
_PUBLIC_ void GetTimeOfDay(struct timeval *tval)
{
	gettimeofday(tval, NULL);
}


#define TIME_FIXUP_CONSTANT 11644473600LL

time_t convert_timespec_to_time_t(struct timespec ts)
{
	/* 1 ns == 1,000,000,000 - one thousand millionths of a second.
	   increment if it's greater than 500 millionth of a second. */
	if (ts.tv_nsec > 500000000) {
		return ts.tv_sec + 1;
	}
	return ts.tv_sec;
}

struct timespec convert_time_t_to_timespec(time_t t)
{
	struct timespec ts;
	ts.tv_sec = t;
	ts.tv_nsec = 0;
	return ts;
}




/**
check if it's a null unix time
**/
_PUBLIC_ bool null_time(time_t t)
{
	return t == 0 || 
		t == (time_t)0xFFFFFFFF || 
		t == (time_t)-1;
}


/*******************************************************************
  create a 16 bit dos packed date
********************************************************************/
static uint16_t make_dos_date1(struct tm *t)
{
	uint16_t ret=0;
	ret = (((unsigned int)(t->tm_mon+1)) >> 3) | ((t->tm_year-80) << 1);
	ret = ((ret&0xFF)<<8) | (t->tm_mday | (((t->tm_mon+1) & 0x7) << 5));
	return ret;
}

/*******************************************************************
  create a 16 bit dos packed time
********************************************************************/
static uint16_t make_dos_time1(struct tm *t)
{
	uint16_t ret=0;
	ret = ((((unsigned int)t->tm_min >> 3)&0x7) | (((unsigned int)t->tm_hour) << 3));
	ret = ((ret&0xFF)<<8) | ((t->tm_sec/2) | ((t->tm_min & 0x7) << 5));
	return ret;
}

/*******************************************************************
  create a 32 bit dos packed date/time from some parameters
  This takes a GMT time and returns a packed localtime structure
********************************************************************/
static uint32_t make_dos_date(time_t unixdate, int zone_offset)
{
	struct tm *t;
	uint32_t ret=0;

	if (unixdate == 0) {
		return 0;
	}

	unixdate -= zone_offset;

	t = gmtime(&unixdate);
	if (!t) {
		return 0xFFFFFFFF;
	}

	ret = make_dos_date1(t);
	ret = ((ret&0xFFFF)<<16) | make_dos_time1(t);

	return ret;
}



/*******************************************************************
  interpret a 32 bit dos packed date/time to some parameters
********************************************************************/
void interpret_dos_date(uint32_t date,int *year,int *month,int *day,int *hour,int *minute,int *second)
{
	uint32_t p0,p1,p2,p3;

	p0=date&0xFF; p1=((date&0xFF00)>>8)&0xFF; 
	p2=((date&0xFF0000)>>16)&0xFF; p3=((date&0xFF000000)>>24)&0xFF;

	*second = 2*(p0 & 0x1F);
	*minute = ((p0>>5)&0xFF) + ((p1&0x7)<<3);
	*hour = (p1>>3)&0xFF;
	*day = (p2&0x1F);
	*month = ((p2>>5)&0xFF) + ((p3&0x1)<<3) - 1;
	*year = ((p3>>1)&0xFF) + 80;
}



/**
  return (tv1 - tv2) in microseconds
*/
_PUBLIC_ int64_t usec_time_diff(const struct timeval *tv1, const struct timeval *tv2)
{
	int64_t sec_diff = tv1->tv_sec - tv2->tv_sec;
	return (sec_diff * 1000000) + (int64_t)(tv1->tv_usec - tv2->tv_usec);
}


/**
  return a zero timeval
*/
_PUBLIC_ struct timeval timeval_zero(void)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	return tv;
}

/**
  return true if a timeval is zero
*/
_PUBLIC_ bool timeval_is_zero(const struct timeval *tv)
{
	return tv->tv_sec == 0 && tv->tv_usec == 0;
}

/**
  return a timeval for the current time
*/
_PUBLIC_ struct timeval timeval_current(void)
{
	struct timeval tv;
	GetTimeOfDay(&tv);
	return tv;
}

/**
  return a timeval struct with the given elements
*/
_PUBLIC_ struct timeval timeval_set(uint32_t secs, uint32_t usecs)
{
	struct timeval tv;
	tv.tv_sec = secs;
	tv.tv_usec = usecs;
	return tv;
}


/**
  return a timeval ofs microseconds after tv
*/
_PUBLIC_ struct timeval timeval_add(const struct timeval *tv,
			   uint32_t secs, uint32_t usecs)
{
	struct timeval tv2 = *tv;
	const unsigned int million = 1000000;
	tv2.tv_sec += secs;
	tv2.tv_usec += usecs;
	tv2.tv_sec += tv2.tv_usec / million;
	tv2.tv_usec = tv2.tv_usec % million;
	return tv2;
}

/**
  return the sum of two timeval structures
*/
struct timeval timeval_sum(const struct timeval *tv1,
			   const struct timeval *tv2)
{
	return timeval_add(tv1, tv2->tv_sec, tv2->tv_usec);
}

/**
  return a timeval secs/usecs into the future
*/
_PUBLIC_ struct timeval timeval_current_ofs(uint32_t secs, uint32_t usecs)
{
	struct timeval tv = timeval_current();
//	mpDebugPrint("--> %s: s=%u,us=%u", __func__, tv.tv_sec, tv.tv_usec);
	return timeval_add(&tv, secs, usecs);
}

/**
  compare two timeval structures. 
  Return -1 if tv1 < tv2
  Return 0 if tv1 == tv2
  Return 1 if tv1 > tv2
*/
_PUBLIC_ int timeval_compare(const struct timeval *tv1, const struct timeval *tv2)
{
	if (tv1->tv_sec  > tv2->tv_sec)  return 1;
	if (tv1->tv_sec  < tv2->tv_sec)  return -1;
	if (tv1->tv_usec > tv2->tv_usec) return 1;
	if (tv1->tv_usec < tv2->tv_usec) return -1;
	return 0;
}

/**
  return true if a timer is in the past
*/
_PUBLIC_ bool timeval_expired(const struct timeval *tv)
{
	struct timeval tv2 = timeval_current();
	if (tv2.tv_sec > tv->tv_sec) return true;
	if (tv2.tv_sec < tv->tv_sec) return false;
	return (tv2.tv_usec >= tv->tv_usec);
}

/**
  return the number of seconds elapsed between two times
*/
_PUBLIC_ double timeval_elapsed2(const struct timeval *tv1, const struct timeval *tv2)
{
	return (tv2->tv_sec - tv1->tv_sec) + 
	       (tv2->tv_usec - tv1->tv_usec)*1.0e-6;
}

/**
  return the number of seconds elapsed since a given time
*/
_PUBLIC_ double timeval_elapsed(const struct timeval *tv)
{
	struct timeval tv2 = timeval_current();
	return timeval_elapsed2(tv, &tv2);
}

/**
  return the lesser of two timevals
*/
_PUBLIC_ struct timeval timeval_min(const struct timeval *tv1,
			   const struct timeval *tv2)
{
	if (tv1->tv_sec < tv2->tv_sec) return *tv1;
	if (tv1->tv_sec > tv2->tv_sec) return *tv2;
	if (tv1->tv_usec < tv2->tv_usec) return *tv1;
	return *tv2;
}

/**
  return the greater of two timevals
*/
_PUBLIC_ struct timeval timeval_max(const struct timeval *tv1,
			   const struct timeval *tv2)
{
	if (tv1->tv_sec > tv2->tv_sec) return *tv1;
	if (tv1->tv_sec < tv2->tv_sec) return *tv2;
	if (tv1->tv_usec > tv2->tv_usec) return *tv1;
	return *tv2;
}

/**
  return the difference between two timevals as a timeval
  if tv1 comes after tv2, then return a zero timeval
  (this is *tv2 - *tv1)
*/
_PUBLIC_ struct timeval timeval_until(const struct timeval *tv1,
			     const struct timeval *tv2)
{
	struct timeval t;
	if (timeval_compare(tv1, tv2) >= 0) {
		return timeval_zero();
	}
	t.tv_sec = tv2->tv_sec - tv1->tv_sec;
	if (tv1->tv_usec > tv2->tv_usec) {
		t.tv_sec--;
		t.tv_usec = 1000000 - (tv1->tv_usec - tv2->tv_usec);
	} else {
		t.tv_usec = tv2->tv_usec - tv1->tv_usec;
	}
	return t;
}


/*******************************************************************
yield the difference between *A and *B, in seconds, ignoring leap seconds
********************************************************************/
static int tm_diff(struct tm *a, struct tm *b)
{
	int ay = a->tm_year + (1900 - 1);
	int by = b->tm_year + (1900 - 1);
	int intervening_leap_days =
		(ay/4 - by/4) - (ay/100 - by/100) + (ay/400 - by/400);
	int years = ay - by;
	int days = 365*years + intervening_leap_days + (a->tm_yday - b->tm_yday);
	int hours = 24*days + (a->tm_hour - b->tm_hour);
	int minutes = 60*hours + (a->tm_min - b->tm_min);
	int seconds = 60*minutes + (a->tm_sec - b->tm_sec);

	return seconds;
}


int extra_time_offset=0;

/**
  return the UTC offset in seconds west of UTC, or 0 if it cannot be determined
 */
_PUBLIC_ int get_time_zone(time_t t)
{
	struct tm *tm = gmtime(&t);
	struct tm tm_utc;
	if (!tm)
		return 0;
	tm_utc = *tm;
	tm = localtime(&t);
	if (!tm)
		return 0;
	return tm_diff(&tm_utc,tm)+60*extra_time_offset;
}



/**
 Check if it's a null timespec.
**/

bool null_timespec(struct timespec ts)
{
	return ts.tv_sec == 0 || 
		ts.tv_sec == (time_t)0xFFFFFFFF || 
		ts.tv_sec == (time_t)-1;
}


