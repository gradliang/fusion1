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
 * libmms public header
 */

#ifndef HAVE_MMS_H
#define HAVE_MMS_H

//#include <inttypes.h>
#include "utiltypedef.h"
#include "xine_internal.h"
#include "asfheader.h"

#define CMD_HEADER_LEN   40
#define CMD_PREFIX_LEN    8
#define CMD_BODY_LEN   1024

#define ASF_HEADER_LEN 8192

#define BUF_SIZE 102400

struct mms_s {

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

  /* command to send */
  char          scmd[CMD_HEADER_LEN + CMD_BODY_LEN];
  char         *scmd_body; /* pointer to &scmd[CMD_HEADER_LEN] */
  int           scmd_len; /* num bytes written in header */
  
  /* receive buffer */
  uint8_t       buf[BUF_SIZE];
  int           buf_size;
  int           buf_read;
  
  asf_header_t *asf_header;
  uint8_t       asf_header_buffer[ASF_HEADER_LEN];
  uint32_t      asf_header_len;
  uint32_t      asf_header_read;

  int           seq_num;
  char          guid[37];
  int           bandwidth;
  
  off_t         current_pos;
  int           eos;

  uint8_t       live_flag;
  
  uint8_t       playing; 
  int       	start_time;
};

typedef struct mms_s mms_t;

char*    mms_connect_common(int *s ,int *port, char *url, char **host, char **path, char **file);
mms_t*   mms_connect (xine_stream_t *stream, const char *url_, int bandwidth);

int      mms_read (mms_t *this, char *data, int len);
uint32_t mms_get_length (mms_t *this);
void     mms_close (mms_t *this);

size_t   mms_peek_header (mms_t *this, char *data, size_t maxsize);

off_t    mms_get_current_pos (mms_t *this);

void     mms_set_start_time (mms_t *this, int time_offset);

#endif

