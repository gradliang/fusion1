/*
 * Copyright (C) 2002-2003 the xine project
 * 
 * This file is part of xine, a free video player.
 * 
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * libmmsh public header
 */

#ifndef HAVE_MMSH_H
#define HAVE_MMSH_H

#include "../../lwip/include/os_mp52x.h"
#include "xine.h"
#include "asfheader.h"

#define SCRATCH_SIZE             1024
//#define CHUNK_SIZE              65536  /* max chunk size */
#define CHUNK_SIZE              0x5000

#define ASF_HEADER_SIZE          8192  /* max header size */

/* 
 * mmsh specific types 
 */


struct mmsh_s {

  xine_stream_t *stream;

  int           s;

  /* url parsing */
  char         *url;
  char         *proto;
  char         *host;
  int           port;
  char         *user;
  char         *password;
  char         *uri;

  char          str[SCRATCH_SIZE]; /* scratch buffer to built strings */

  asf_header_t *asf_header;
  int           stream_type;  

  /* receive buffer */
  
  /* chunk */
  uint16_t      chunk_type;
  uint16_t      chunk_length;
  uint16_t      chunk_seq_number;
  uint8_t       buf[CHUNK_SIZE];

  int           buf_size;
  int           buf_read;

  uint8_t       asf_header_buffer[ASF_HEADER_SIZE];
  uint32_t      asf_header_len;
  uint32_t      asf_header_read;
  int           seq_num;
  
  int           video_stream;
  int           audio_stream;

  off_t         current_pos;
  int           user_bandwidth;
  
  int           playing;
  unsigned int  start_time;
};
static inline char * os_strndup(const char *s, int len)
{
	char *res;
	if (s == NULL)
		return NULL;
	res = mpx_Malloc(len + 1);
	if (res)
    {
		memcpy(res, s, len);
        res[len] = '\0';
    }
	return res;
}

#define _X_LE_16(x) (((uint16_t)(((uint8_t*)(x))[1]) << 8) | \
                  ((uint16_t)((uint8_t*)(x))[0]))
#define _X_LE_32(x) (((uint32_t)(((uint8_t*)(x))[3]) << 24) | \
                  ((uint32_t)(((uint8_t*)(x))[2]) << 16) | \
                  ((uint32_t)(((uint8_t*)(x))[1]) << 8) | \
                  ((uint32_t)((uint8_t*)(x))[0]))
                  
#define _X_LE_64(x) (((uint64_t)(((uint8_t*)(x))[7]) << 56) | \
                  ((uint64_t)(((uint8_t*)(x))[6]) << 48) | \
                  ((uint64_t)(((uint8_t*)(x))[5]) << 40) | \
                  ((uint64_t)(((uint8_t*)(x))[4]) << 32) | \
                  ((uint64_t)(((uint8_t*)(x))[3]) << 24) | \
                  ((uint64_t)(((uint8_t*)(x))[2]) << 16) | \
                  ((uint64_t)(((uint8_t*)(x))[1]) << 8) | \
                  ((uint64_t)((uint8_t*)(x))[0]))

#define _(String) String
#define strdup(x) os_strdup(x)
#define strndup(x,n) os_strndup(x,n)
#define calloc(n, item) os_zalloc(n*item)
#define malloc(x)	mpx_Malloc(x)
#define realloc(a,b)	mpx_Realloc(a,b)
#define free(x)	mpx_Free(x)

typedef struct mmsh_s mmsh_t;

char*    mmsh_connect_common(int *s ,int *port, char *url, char **host, char **path, char **file);
mmsh_t*  mmsh_connect (xine_stream_t *stream,const char *url, int bandwidth);

int      mmsh_read (mmsh_t *this, char *data, int len);
uint32_t mmsh_get_length (mmsh_t *this);
void     mmsh_close (mmsh_t *this);

size_t   mmsh_peek_header (mmsh_t *this, char *data, size_t maxsize);

off_t    mmsh_get_current_pos (mmsh_t *this);

void     mmsh_set_start_time (mmsh_t *this, int time_offset);

#endif
