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
* Filename      : util_ConfigParser.c
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


DWORD UtilConfigParser(BYTE *src, DWORD srcSize, BYTE *name, BYTE *value)
{
    SDWORD i = 0;
    DWORD state = 0;  //0: parser name, 1: parser value, 2: parser comment, 3: find '='
    DWORD start = (DWORD) src;

    while (srcSize--)
    {
        if (!state)
        {
            if ( ((0x30 <= *src) && (*src <= 0x39)) ||
                 ((0x41 <= *src) && (*src <= 0x5a)) ||
                 ((0x61 <= *src) && (*src <= 0x7a)) ||
                 (*src == '_') )
            {
                name[i++] = *src;
            }
            else if (*src == '=')       //goto check value
            {
                state = 1;
                name[i] = 0;
                i = 0;
            }
            else if (*src == 0x0a)      //recheck name
                i = 0;
            else if (*src == ';')
            {
                name[0] = 0;
                state = 2;
            }
            /*else if(*src == ' ')
            {
                state = 3;
            }
            else    //invalid character
            {
                name[0] = 0;
                state = 2;
            }*/
        }
        else if (state == 1)
        {
            if ((*src == 0x0a) || (*src == ';'))
            {
                //remove ' ' and tab in end of value
                for (i -= 1; i >= 0; i--)
                {
                    if( (0x30 <= value[i] && value[i] <= 0x39) ||
                        (0x41 <= value[i] && value[i] <= 0x5a) ||
                        (0x61 <= value[i] && value[i] <= 0x7a) ||
                        (value[i] == '/') || (value[i] == '\\') ||
                        (value[i] == '.') || (value[i] == '_') )
                    {
                        break;
                    }
                }

                value[i+1] = 0;

                if (*src == ';')
                    state = 2;
                else
                {
                    src++;
                    break;
                }
            }
            else if ((0 == i) && (*src == ' '))
            {
                // Drop
            }
            else
            {
                value[i++] = *src;
            }
        }
        else if (state == 2)
        {
            if (*src == 0x0a)
            {
                if (name[0] && value[0])
                {
                    src++;
                    break;
                }
                else    //recheck name and value
                {
                    state = 0;
                    i = 0;
                }
            }
        }

        src++;
    }

    return ((DWORD)src - start);
}

