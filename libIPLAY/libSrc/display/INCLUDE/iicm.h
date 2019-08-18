/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2005, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:    iicm.h
*
* Programmer:    Joe.H
*                MPX E320 division
*
* Created: 9/10/2005
*
* Description:
* Version : 0001
*
****************************************************************
*/
#ifndef __IICM_H
#define __IICM_H

#include "peripheral.h"

/*
//Function prototype
*/

void I2CM_MX88V44_WtTVReg(BYTE DevID,BYTE reg, BYTE data1);
void I2CM_MX88V44_RdTVReg(BYTE DevID, BYTE reg);

#endif //__IICM_H

