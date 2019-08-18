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
* Filename      : txt_Utf8ToUnicode.c
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


#include "global612.h"
#include "mpTrace.h"



U16 mpx_Utf8ToUnicode(void ** string)
{
    U08 *utf8_point, word_count;
    U16 unicode;

    utf8_point = *string;
    unicode = *utf8_point;
    utf8_point++;
    
    if (unicode < 0x80)
    {
        *string = utf8_point;
        return unicode;
    }
    
    if ((unicode & 0xf0) == 0xe0)
        word_count = 2;
    else if ((unicode & 0xe0) == 0xc0)
    {
        unicode &= 0x1f;
        word_count = 1;
    }
    
    while (word_count)
    {
        unicode <<= 6;
        unicode += *utf8_point;
        unicode -= 0x80;
        utf8_point++;
        word_count--;
    }
    
    *string = utf8_point;

    return unicode;
}



U16 mpx_Utf8ToBig5(void ** string)
{
    U16 unicode, big5code;
    
    unicode = mpx_Utf8ToUnicode(string);
#if FILENAME_BIG5     
    if (unicode > 0x80)
        big5code = mpx_UnicodeToBig5(&unicode);
    else
#endif    
       big5code = unicode;
    
    return big5code;
}


