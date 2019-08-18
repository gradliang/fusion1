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
* Filename      : util_Num2Str.c
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
///@brief   Utility for converting a number to an ASCII string.
///
///@param   string         The address of target ASCII string.
///@param   num            Number to convert.
///@param   length         The amount of the string to convert.
///
///@return  None.
///
void mpx_UtilNum2String(U08 * string, U32 num, U32 length)
{
    U32 hex = Bin2Bcd(num);

    hex = hex << ((8 - length) * 4);

    while (length)
    {
        *string = hex >> 28;
        hex = hex << 4;
        *string = *string + 0x30;
        if (*string > 0x39)
            *string = *string + 7;
        string++;
        length--;
    }
    *string = 0;

    return;
}

U32 mpx_UtilString2Num(U08 * string, U32 length)
{
    U32 num = 0;

    while (length)
    {
        if(*string >= 0x30 && *string <= 0x39)        
            num = num * 10 + (*string - 0x30);
            
        string++;
        length--;
    }

    return num;
}
