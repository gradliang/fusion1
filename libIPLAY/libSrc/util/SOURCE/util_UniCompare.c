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
* Filename      : util_UniCompare.c
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
///@brief   Unicode string compare utility.
///
///@param   string1   Input Unicode string pointer.
///@param   string2   The Unicode string that to be compared.
///
///@retval  0         The string1 is equal to the string2.
///@retval  != 0      The string2 is not equal to the string2.
///
S32 mpx_UtilUniCompare(U16 * string1, U16 * string2)
{
    while (*string1 && *string2)
    {
        if(*string1 - *string2)
            return 1;
        string1++;
        string2++;
    }

    if (*string1 == *string2)
        return 0;
    else
        return 1;
}

BOOL IsASCIIString(BYTE *pb)
{
    BOOL isStringASCII = 1;
    BYTE indexFile=0;
    while( !(*(pb+indexFile) == 0x00 && *(pb+indexFile+1) == 0x00) )
    {
        if(*(pb+indexFile) >= 0x80)
        {
            isStringASCII = 0; // NOT ASCII
            break;
        }
        else 
        if(indexFile == 0 && *(pb+indexFile) == 0x00)
        {
            isStringASCII = 0; // NOT ASCII
            break; // Unicode with all ASCII
        }
        else    
            indexFile += 1;
    }           

	return isStringASCII;
}


