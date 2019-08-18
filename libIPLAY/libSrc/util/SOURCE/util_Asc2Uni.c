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
* Filename      : util_Asc2Uni.c
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



///
///@ingroup UTILITY
///@brief   Utility for converting the ASCII string to Unicode (UTF-16) string.
///
///@param   target        Target Unicode (UTF-16) string.
///@param   source        Source ASCII string.
///@param   max_length    The max converting byte count.
///
///@return  Target string pointer.
///
///@remark  This utility assumes that the source is a limited null-terminated string. A string
///         without a null at the end of the string will cause unpreditable result.
///
U16 *mpx_UtilAsc2Uni(U16 * target, U08 * source, U32 max_length)
{
    U16 *result;

    result = target;

    while (*source && max_length)
    {
        *target = (U16) *source;
        target++;
        source++;
        max_length--;
    }

    *target = 0;

    return result;
}

