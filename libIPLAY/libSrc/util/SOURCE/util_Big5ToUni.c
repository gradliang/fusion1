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
* Filename      : util_Big5ToUni.c
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
#include "text.h"



///
///@ingroup UTILITY
///@brief   Utility for converting the pure U16 Big5 string to UTF-16 Unicode string.
///
///@param   target    Target UTF-16 Unicode string.
///@param   source    Source U16 Big5 string.
///
///@return  Target string pointer.
///
///@remark  This utility assumes that the source is a limited null-terminated string. A string
///         without a null at the end of the string will cause unpreditable result.
///
U16 *mpx_UtilBig5ToUnicodeU16(U16 * target, U16 * source)
{
    U16 code, *result, *ptr_UTF16;
    U08 *ptr_ch;

    result = target;
    
    ptr_UTF16 = source;
    while (*ptr_UTF16)
    {
        ptr_ch = (U08 *) ptr_UTF16;
#if FILENAME_BIG5         
        code = mpx_Big5ToUnicode(&ptr_ch);  // mpx_Big5ToUnicode() may increment the value of input string pointer
#else
        code = 0;
#endif        
        *target = code;

        ptr_UTF16++;
        target++;
    }
    *target = 0;

    return result;
}

