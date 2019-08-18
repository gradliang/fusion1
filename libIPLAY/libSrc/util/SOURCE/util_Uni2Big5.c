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
* Filename      : util_Uni2Big5.c
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
///@brief   Utility for converting the U16 (Unicode, Big5) string to Big5 (U08) string.
///
///@param   target    Target Big5 (U08) string.
///@param   source    Source U16 (Unicode, Big5) string.
///
///@return  Target string pointer.
///
///@remark  This utility assumes that the source is a limited null-terminated string. A string
///         without a null at the end of the string will cause unpreditable result.
///
U08 *mpx_UtilUnicodeToBig5U08(U08 * target, U16 * source)
{
    U16 code; U08 temp, *result;

    result = target;
    code = mpx_UnicodeToBig5(source);
    while (code)
    {
        if(code < 0x80) // ASCII
        {
            *target = (U08) (code & 0xFF);
            target++;
        }
        else // Two bytes (Unicode, Big5, GB) codes
        {
            temp = (U08) (code & 0xFF);
            *target = (U08) (code >> 8);
            target++;
            *target = temp;
            *target++;
        }
        source++;
        code = mpx_UnicodeToBig5(source);
    }
    *target = 0;

    return result;
}

