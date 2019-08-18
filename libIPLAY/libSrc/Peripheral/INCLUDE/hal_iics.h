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
* Filename      : hal_IICS.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : I2C salve module
*******************************************************************************
*/

#ifndef __HAL_IICS_H
#define __HAL_IICS_H

/*
// Include section
*/
#include "iplaysysconfig.h"

/*
// Constant declarations
*/
#ifndef HW_IIC_SLAVE_ENABLE
#define HW_IIC_SLAVE_ENABLE         0
#endif

/*
//Function prototype
*/
void I2CS_WriteRegisterCallBackFunc(void (*I2CS_CallBack_func) (void));
void I2CS_WriteClearCallBackFunc(void);
void I2CS_ReadRegisterCallBackFunc(void (*I2CS_CallBack_func) (void));
void I2CS_ReadClearCallBackFunc(void);

void I2CS_Init(BYTE);

#endif  // __HAL_IICS_H

