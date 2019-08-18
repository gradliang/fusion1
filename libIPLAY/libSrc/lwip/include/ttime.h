/* 
   Unix SMB/CIFS implementation.
   time utility functions
   
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

#ifndef _SAMBA_TIME_H_
#define _SAMBA_TIME_H_

#ifndef _PUBLIC_
#define _PUBLIC_
#endif

/**
 External access to time_t_min and time_t_max.
**/
_PUBLIC_ time_t get_time_t_max(void);

/**
a gettimeofday wrapper
**/
_PUBLIC_ void GetTimeOfDay(struct timeval *tval);

/**
put a dos date into a buffer (time/date format)
This takes GMT time and puts local time in the buffer
**/
_PUBLIC_ void push_dos_date(uint8_t *buf, int offset, time_t unixdate, int zone_offset);

/**
put a dos date into a buffer (date/time format)
This takes GMT time and puts local time in the buffer
**/
_PUBLIC_ void push_dos_date2(uint8_t *buf,int offset,time_t unixdate, int zone_offset);

/**
put a dos 32 bit "unix like" date into a buffer. This routine takes
GMT and converts it to LOCAL time before putting it (most SMBs assume
localtime for this sort of date)
**/
_PUBLIC_ void push_dos_date3(uint8_t *buf,int offset,time_t unixdate, int zone_offset);

/**
  create a unix date (int GMT) from a dos date (which is actually in
  localtime)
**/
_PUBLIC_ time_t pull_dos_date(const uint8_t *date_ptr, int zone_offset);

/**
like make_unix_date() but the words are reversed
**/
_PUBLIC_ time_t pull_dos_date2(const uint8_t *date_ptr, int zone_offset);

/**
  create a unix GMT date from a dos date in 32 bit "unix like" format
  these generally arrive as localtimes, with corresponding DST
**/
_PUBLIC_ time_t pull_dos_date3(const uint8_t *date_ptr, int zone_offset);

/**
return a HTTP/1.0 time string
**/
_PUBLIC_ char *http_timestring(time_t t);

/**
 Return the date and time as a string
**/
_PUBLIC_ char *timestring(time_t t);

/**
  return (tv1 - tv2) in microseconds
*/
_PUBLIC_ int64_t usec_time_diff(const struct timeval *tv1, const struct timeval *tv2);

/**
  return a zero timeval
*/
_PUBLIC_ struct timeval timeval_zero(void);

/**
  return true if a timeval is zero
*/
_PUBLIC_ bool timeval_is_zero(const struct timeval *tv);

/**
  return a timeval for the current time
*/
_PUBLIC_ struct timeval timeval_current(void);

/**
  return a timeval struct with the given elements
*/
_PUBLIC_ struct timeval timeval_set(uint32_t secs, uint32_t usecs);

/**
  return a timeval ofs microseconds after tv
*/
_PUBLIC_ struct timeval timeval_add(const struct timeval *tv,
			   uint32_t secs, uint32_t usecs);

/**
  return the sum of two timeval structures
*/
struct timeval timeval_sum(const struct timeval *tv1,
			   const struct timeval *tv2);

/**
  return a timeval secs/usecs into the future
*/
_PUBLIC_ struct timeval timeval_current_ofs(uint32_t secs, uint32_t usecs);

/**
  compare two timeval structures. 
  Return -1 if tv1 < tv2
  Return 0 if tv1 == tv2
  Return 1 if tv1 > tv2
*/
_PUBLIC_ int timeval_compare(const struct timeval *tv1, const struct timeval *tv2);

/**
  return true if a timer is in the past
*/
_PUBLIC_ bool timeval_expired(const struct timeval *tv);

/**
  return the number of seconds elapsed between two times
*/
_PUBLIC_ double timeval_elapsed2(const struct timeval *tv1, const struct timeval *tv2);

/**
  return the number of seconds elapsed since a given time
*/
_PUBLIC_ double timeval_elapsed(const struct timeval *tv);

/**
  return the lesser of two timevals
*/
_PUBLIC_ struct timeval timeval_min(const struct timeval *tv1,
			   const struct timeval *tv2);

/**
  return the greater of two timevals
*/
_PUBLIC_ struct timeval timeval_max(const struct timeval *tv1,
			   const struct timeval *tv2);

/**
  return the difference between two timevals as a timeval
  if tv1 comes after tv2, then return a zero timeval
  (this is *tv2 - *tv1)
*/
_PUBLIC_ struct timeval timeval_until(const struct timeval *tv1,
			     const struct timeval *tv2);


/**
  return the UTC offset in seconds west of UTC, or 0 if it cannot be determined
 */
_PUBLIC_ int get_time_zone(time_t t);

void interpret_dos_date(uint32_t date,int *year,int *month,int *day,int *hour,int *minute,int *second);

time_t convert_timespec_to_time_t(struct timespec ts);

struct timespec convert_time_t_to_timespec(time_t t);

bool null_timespec(struct timespec ts);

/** Extra minutes to add to the normal GMT to local time conversion. */
extern int extra_time_offset;

#endif /* _SAMBA_TIME_H_ */
