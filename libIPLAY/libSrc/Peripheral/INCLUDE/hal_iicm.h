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
* Filename      : hal_IICM.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : I2C Master Module
*******************************************************************************
*/

#ifndef __HAL_IICM_H
#define __HAL_IICM_H

#include "iplaysysconfig.h"

#ifndef HW_IIC_MASTER_ENABLE
#define HW_IIC_MASTER_ENABLE    1
#endif

#ifndef IICM_PIN_USING_PGPIO
#define IICM_PIN_USING_PGPIO    DISABLE
#endif

#ifndef IICM_PIN_USING_VGPIO
#define IICM_PIN_USING_VGPIO    DISABLE
#endif

////////////////////////////////////////////////////////////
//
//  Function prototype
//
////////////////////////////////////////////////////////////

typedef struct {
    SDWORD devId;
    DWORD timing;
} ST_IICM_TIMING_TABLE;


#define MAX_IICM_DEV                10

///
///@ingroup I2C_MODULE
///@brief   Initial I2C master module
///
///@param   None
///
///@retval  None
///
///@remark
///
void I2CM_Init(void);

///
///@ingroup I2C_MODULE
///@brief   DeInitial I2C master module
///
///@param   None
///
///@retval  None
///
///@remark  Turn off I2C power and release I2CM's semaphore
///
void I2CM_DeInit(void);

///
///@ingroup I2C_MODULE
///
///@brief   Configature I2C waveform timing.
///
///@param   startDly    the clock durtion of start to clock change, max value is 255.
///@param   lowDly      the clock durtion of data low, unit is 1 clocks, max value is 255.
///@param   highDly     the clock durtion of data high, unit is 1 clocks, max value is 255.
///@param   dataChDly   the clock durtion of data to clock change delay, unit is 2 clocks, max value is 3 (6 clocks)
///
///@retval  None
///
///@remark  Default clock at 300KHz
///@remark  I2C clock freq = I2C base clock / [ (High duty * 4 + 1) + (Low duty * 4 + 1) ]
///
void I2CM_TimingCfg(BYTE devId, BYTE startDly, BYTE highDly, BYTE lowDly, BYTE dataChDly);

///
///@ingroup I2C_MODULE
///
///@brief   Change I2C waveform timing.
///
///@param   i2cFreq     target frequency for I2C clock(Duty). Unit is Hz.
///
///@retval  None
///
///@remark  The purpose is same as I2CM_TimingCfg.
///@remark  The max freq is 400KHz.
///
void I2CM_FreqChg(BYTE devId, DWORD i2cFreq);

///
///@ingroup I2C_MODULE
///
///@brief   Read one \b byte data from I2C at 8bits register address
///
///@param   DevID   Device ID, include R/W bits
///@param   reg     register index(8bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is full 8bits mode.
///
SDWORD I2CM_RdReg8Data8(BYTE DevID, BYTE reg);

///
///@ingroup I2C_MODULE
///
///@brief   Read one \b byte data from I2C at 8bits register address without restart
///
///@param   DevID   Device ID, include R/W bits
///@param   reg     register index(8bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is full 8bits mode.
///
SDWORD I2CM_RdReg8Data8Nonrestart(BYTE DevID, BYTE reg);

///
///@ingroup I2C_MODULE
///
///@brief   Read one \b word data from I2C at 8bits register address
///
///@param   DevID   Device ID, include R/W bits
///@param   reg     register index(8bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is 8bits address, 16bits data mode.
///
SDWORD I2CM_RdReg8Data16(BYTE DevID, BYTE reg);

///
///@ingroup I2C_MODULE
///
///@brief   Read one \b word data from I2C at 8bits register address without restart
///
///@param   DevID   Device ID, include R/W bits
///@param   reg     register index(8bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is 8bits address, 16bits data mode.
///
SDWORD I2CM_RdReg8Data16Nonrestart(BYTE DevID, BYTE reg);

///
///@ingroup I2C_MODULE
///
///@brief   Write one \b byte data to I2C at 8bits register address
///
///@param   DevID   Device ID, include R/W bits
///@param   bReg    register index(8bits)
///@param   bData   Write data(8bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is full 8bits mode.
///
SDWORD I2CM_WtReg8Data8(BYTE DevID, BYTE bReg, BYTE bData);

///
///@ingroup I2C_MODULE
///
///@brief   Write one \b byte data to I2C at 16bits register address
///
///@param   DevID   Device ID, include R/W bits
///@param   wReg    register index (16bits)
///@param   bData   Write data(8bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is 16bits address, 8bits data mode.
///
SDWORD I2CM_WtReg16Data8(BYTE DevID, WORD wReg, BYTE bData);


///
///@ingroup I2C_MODULE
///
///@brief   Write one \b word data to I2C at 8bits register address
///
///@param   DevID   Device ID, include R/W bits
///@param   bReg    register index (8bits)
///@param   wData   Write data(16bits)
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark  This function is 8bits address, 16bits data mode.
///
SDWORD I2CM_WtReg8Data16(BYTE DevID, BYTE bReg, WORD wData);

///
///@ingroup I2C_MODULE
///
///@brief   Read data from I2C bus by bust mode.
///
///@param   DevID       Device ID, include R/W bits
///@param   regAddr     register index
///@param   regAddr16   1: mean register index is 16bits, 0: mean register index is 8bits
///@param   *inDataBuf  The buffer pointer will be store by I2C.
///@param   readLength  How many bytes will be read by I2C.
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark
///
SDWORD I2CM_Read_BustMode(BYTE DevID, WORD regAddr, BYTE regAddr16, BYTE *inDataBuf, WORD readLength);

///
///@ingroup I2C_MODULE
///
///@brief   Write data to I2C bus by bust mode.
///
///@param   DevID       Device ID, include R/W bits
///@param   *outDataBuf The buffer pointer will be send by I2C.
///@param   readLength  How many bytes will be send by I2C.
///
///@retval  >= 0    Data
///@retval  -1      Timeout
///
///@remark
///
SDWORD I2CM_Write_BustMode(BYTE DevID, BYTE *outDataBuf, WORD dataLength);

SDWORD I2CM_ReadData_BustMode(BYTE DevID, WORD *inDataBuf, WORD readLength);

#if ((HW_IIC_MASTER_ENABLE == 1) || (HW_IIC_SLAVE_ENABLE == 1))
void I2C_ClkSel(void);
#endif

// Wrapper
#define I2CM_WtReg1                 I2CM_WtReg8Data8
#define I2CM_WtReg2(a, b, c, d)     I2CM_WtReg8Data16(a, b, (((WORD) c) << 8) | d)
#define I2CM_RdReg                  I2CM_RdReg8Data8
#define I2CM_WtData                 I2CM_WtReg8Data8

#endif      // #ifndef __HAL_IICM_H

