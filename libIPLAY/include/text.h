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
* Filename      : text.h
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/

#include "global612.h"


/* utility APIs for text encoding/conversion processing [begin] */
U16  mpx_Big5ToUnicode(void ** string);
U16  mpx_Big5GBToUnicode(void ** string);
U16  mpx_UnicodeToBig5(U16 * code);
U16  mpx_UnicodeToBig5GB(U16 * unicode);
U16  mpx_Utf8ToUnicode(void ** string);
U16  mpx_Utf8ToBig5(void ** string);
U16  mpx_Gb2312ToUnicode(void ** string);
/* utility APIs for text encoding/conversion processing [end] */

