/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: aac_info.h,v 1.3 2003/07/29 08:20:11 menno Exp $
**/

#ifndef __AAC_INFO_H__
#define __AAC_INFO_H__

typedef struct {
    unsigned char shoutcast;
    unsigned char mp4file;
    unsigned char version;
    unsigned char header_type;
    unsigned char object_type;
    unsigned char channels;
    unsigned char reserved[2];
    unsigned long sampling_rate;
    unsigned long bitrate;
    unsigned long length;
    /* for SHOUTcast streaming */
    unsigned long block_size;
    unsigned long skip_bytes;
} NeAACDecFileInfo;

uint32_t read_callback(void *user_data, void *buffer, uint32_t length);
uint32_t seek_callback(void *user_data, uint64_t position);
int get_aac_track(mp4ff_t *infile);
static int get_mp4_info(int file_length, NeAACDecFileInfo *info);
static int get_aac_info(int file_length, NeAACDecFileInfo *info, int seconds);
static int read_adif_header(int file_length, NeAACDecFileInfo *info);
static int read_adts_header(int file_length, NeAACDecFileInfo *info, int seconds);

#endif
