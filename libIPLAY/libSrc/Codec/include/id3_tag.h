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
** $Id: id3_tag.h,v 1.17 2004/01/23 23:22:46 rob Exp $
**/

#ifndef __ID3_TAG_H__
#define __ID3_TAG_H__

/* ID3v2 tags */
#define TT2     0x545432
#define TP1     0x545031
#define TAL     0x54414C
#define TYE     0x545945
#define COM     0x434F4D
#define TCO     0x54434F
#define TRK     0x54524B

/* ID3v2.3.0 tags */
#define TIT2	0x54495432
#define TPE1	0x54504531
#define TALB	0x54414C42
#define TYER	0x54594552
#define COMM    0x434F4D4D
#define TCON    0x54434F4E
#define TRCK    0x5452434B

typedef struct {
    char title[32];
    char artist[32];
    char album[32];
    char year[8];
    char comment[32];
    char genre[4];
    char track[4];
} id3v1_tag_t;

typedef struct {
    char *title;
    char *artist;
    char *album;
    char *year;
    char *comment;
    char *genre;
    char *track;
} id3v2_tag_t;

#endif
