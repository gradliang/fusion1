/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : imageplayer.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __EPUBSTRUCT_H
#define __EPUBSTRUCT_H


typedef struct _XML_Data 		XML_Data_t;
typedef struct _XML_Tag		XML_Tag_t;
typedef struct _XML_Mask  	XML_Mask;
	
struct _XML_Mask 
{
	short x;
	short y;
	BYTE mask;
};

struct _XML_Tag
{	
	char *tag;
	int tagnum;
};

struct _XML_Data
{
	int tagnum;	
	int   depth;
	BYTE tagdata;
	int fontsize;
	
	int 	imgwidth;
	int 	imgheight;
	int 	imghorsizescaled;
	int 	imgversizescaled;
	
	 void *tag;

	int tablecase;
	int tablecaselevel;
	int strlength;
	BYTE GlyphSetMode;
	char *strdata;
	long *lstrdata;
	XML_Mask *pmask;
	XML_Data_t *pre,*next;	
};
;

#endif //__IMAGE_PLAYER_H


