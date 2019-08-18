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
* Filename      : IICM.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/


/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "iicm.h"

#if ((TCON_ID == TCON_MX88V44) || (TCON_ID == TCON_MX88V44L))

void I2CM_MX88V44_WtTVReg(BYTE DevID, BYTE reg, BYTE data1)
{
    I2CM_WtReg1(DevID, 0x90, reg);
    I2CM_WtReg1(DevID, 0x91, data1);
}



void I2CM_MX88V44_RdTVReg(BYTE DevID, BYTE reg)
{
    I2CM_WtReg1(DevID, 0x90, reg);
    I2CM_RdReg(DevID, 0x91);
}

#endif

