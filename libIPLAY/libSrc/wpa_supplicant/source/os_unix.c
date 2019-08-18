/*
 * wpa_supplicant/hostapd / OS specific functions for UNIX/POSIX systems
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include "os.h"
void os_sleep(os_time_t sec, os_time_t usec)
{
#if 0
	if (sec)
		sleep(sec);
	if (usec)
		usleep(usec);
#endif	
}


int os_get_time(struct os_time *t)
{
#ifdef MP620
    U32 time;                                   /* in ms */
    time = GetSysTime();
    t->sec = time / 1000;
    t->usec = (time * 1000) % 1000000;
#else
#error "no MP620"
    U32 time;                                   /* in ticks */
    time = GetSysTime();
    t->sec = time / HZ;
	//mpDebugPrint("time = %d", time);
    t->usec = (time % HZ) * 1000000 / HZ;
#endif
    return 0;
}


int os_mktime(int year, int month, int day, int hour, int min, int sec,
	      os_time_t *t)
{
	struct tm tm, *tm1;
	time_t t_local, t1, t2;
	os_time_t tz_offset;

	if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
	    hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 ||
	    sec > 60)
		return -1;

	memset(&tm, 0, sizeof(tm));
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;

	t_local = mktime(&tm);

	/* figure out offset to UTC */
	tm1 = localtime(&t_local);
	if (tm1) {
		t1 = mktime(tm1);
		tm1 = gmtime(&t_local);
		if (tm1) {
			t2 = mktime(tm1);
			tz_offset = t2 - t1;
		} else
			tz_offset = 0;
	} else
		tz_offset = 0;

	*t = (os_time_t) t_local - tz_offset;
	return 0;
}


int os_daemonize(const char *pid_file)
{
#if 0
#ifdef __uClinux__
	return -1;
#else /* __uClinux__ */
	if (daemon(0, 0)) {
		perror("daemon");
		return -1;
	}

	if (pid_file) {
		FILE *f = fopen(pid_file, "w");
		if (f) {
			fprintf(f, "%u\n", getpid());
			fclose(f);
		}
	}

	return -0;
#endif /* __uClinux__ */
#endif
	return 0;
}


void os_daemonize_terminate(const char *pid_file)
{
	//if (pid_file)
	//	unlink(pid_file);
}

/*static U08 ram_pattern[32] ={   
								0xBA, 0x92, 0x7C, 0x0E, 0x69, 0xD2,
								0x52, 0x4F, 0x40, 0xF5, 0x59, 0x69,
								0x15, 0xD1, 0xFB, 0x73, 0x19, 0x08,
								0x66, 0x72, 0x58, 0xF5, 0x13, 0xE4,
								0x5C, 0x08, 0x2F, 0x2E, 0xDB, 0x1D,
								0x63, 0xD6 
							};*/
static U08 ram_pattern[32];

int os_get_random(unsigned char *buf, size_t len)
{
    U32 time,i;                                   /* in ms */
	for(i=0;i<32;i++)
	{
	   time = GetSysTime();
	   ram_pattern[i] = (time<<i)&0xFF;
	}

#if 0
		FILE *f;
		size_t rc;
	
		f = fopen("/dev/urandom", "rb");
		if (f == NULL) {
			printf("Could not open /dev/urandom.\n");
			return -1;
		}
	
		rc = fread(buf, 1, len, f);
		fclose(f);
#else
		size_t rc = len;
	
		memcpy(buf, ram_pattern, len);
#endif
		return rc != len ? -1 : 0;

}


unsigned long os_random(void)
{
#if 0
		return random();
#else
		return 0;
#endif
}


char * os_rel2abs_path(const char *rel_path)
{
#if 0
	char *buf = NULL, *cwd, *ret;
	size_t len = 128, cwd_len, rel_len, ret_len;
	int last_errno;

	if (rel_path[0] == '/')
		return strdup(rel_path);

	for (;;) {
		buf = malloc(len);
		if (buf == NULL)
			return NULL;
		cwd = getcwd(buf, len);
		if (cwd == NULL) {
			last_errno = errno;
			free(buf);
			if (last_errno != ERANGE)
				return NULL;
			len *= 2;
			if (len > 2000)
				return NULL;
		} else {
			buf[len - 1] = '\0';
			break;
		}
	}

	cwd_len = strlen(cwd);
	rel_len = strlen(rel_path);
	ret_len = cwd_len + 1 + rel_len + 1;
	ret = malloc(ret_len);
	if (ret) {
		memcpy(ret, cwd, cwd_len);
		ret[cwd_len] = '/';
		memcpy(ret + cwd_len + 1, rel_path, rel_len);
		ret[ret_len - 1] = '\0';
	}
	free(buf);
	return ret;
	#else
	return 0;
	#endif
}


int os_program_init(void)
{
	return 0;
}


void os_program_deinit(void)
{
}


int os_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}


int os_unsetenv(const char *name)
{
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
	unsetenv(name);
	return 0;
#else
	return unsetenv(name);
#endif
}


char * os_readfile(const char *name, size_t *len)
{

#if 0

	FILE *f;
	char *buf;

	f = fopen(name, "rb");
	if (f == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	*len = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = malloc(*len);
	if (buf == NULL) {
		fclose(f);
		return NULL;
	}

	if (fread(buf, 1, *len, f) != *len) {
		fclose(f);
		free(buf);
		return NULL;
	}

	fclose(f);

	return buf;
 #endif
}


void * os_zalloc(size_t size)
{
	void *ptr;
	ptr = os_malloc(size);
	if (ptr)
		os_memset(ptr, 0, size);
	return ptr;
}


size_t os_strlcpy(char *dest, const char *src, size_t siz)
{
	const char *s = src;
	size_t left = siz;

	if (left) {
		/* Copy string up to the maximum size of the dest buffer */
		while (--left != 0) {
			if ((*dest++ = *s++) == '\0')
				break;
		}
	}

	if (left == 0) {
		/* Not enough room for the string; force NUL-termination */
		if (siz != 0)
			*dest = '\0';
		while (*s++)
			; /* determine total src string length */
	}

	return s - src - 1;
}

void * os_strdup(char* string)
{
	char* ptr;
	size_t len;

	if (string == NULL)
	{
	    mpDebugPrint("string == NULL");
		return NULL;
	}
	len = os_strlen(string);
	ptr = os_malloc(len+1);
	if (ptr)
		os_memcpy(ptr, string, len+1);
	return ptr;
}

int os_strcmp (const char *s1, const char *s2)
{ 
  unsigned const char *us1 = (unsigned const char *)s1;
  unsigned const char *us2 = (unsigned const char *)s2;
  int c1a, c1b;
  int c2a, c2b;

  /* If the pointers aren't both aligned to a 16-byte boundary, do the
     comparison byte by byte, so that we don't get an invalid page fault if we
     are comparing a string whose null byte is at the last byte on the last
     valid page.  */
  if (((((long)us1) | ((long)us2)) & 1) == 0)
    {
      c1a = *us1;
      for (;;)
	{
	  c1b = *us2;
	  us1 += 2;
	  if (c1a == '\0')
	    goto ret1;

	  c2a = us1[-1];
	  if (c1a != c1b)
	    goto ret1;

	  c2b = us2[1];
	  us2 += 2;
	  if (c2a == '\0')
	    break;

	  c1a = *us1;
	  if (c2a != c2b)
	    break;
	}

      return c2a - c2b;
    }
  else
    {
      do
	{
	  c1a = *us1++;
	  c1b = *us2++;
	}
      while (c1a != '\0' && c1a == c1b);
    }

 ret1:
  return c1a - c1b;
}

