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
* Filename      : hal_IR.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/
#ifndef __HAL_IR_H
#define __HAL_IR_H


////////////////////////////////////////////////////////////
//
// Include section
//
////////////////////////////////////////////////////////////

#include "UtilTypedef.h"
#include "bitsdefine.h"
#include "iplaysysconfig.h"

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////

///
///@ingroup     IR_MODULE
///@brief       data packet of NEC button
#define IR_NECDATA              0x01
///
///@ingroup     IR_MODULE
///@brief       data packet with error of NEC button
#define IR_NECDATA_ERROR        0x02
///
///@ingroup     IR_MODULE
///@brief       repeat data packet of NEC button
#define IR_NECREPEAT            0x03
///
///@ingroup     IR_MODULE
///@brief       data packet of Remote point mouse
#define IR_REMOTE_POINT_MOUSE   0x04
///
///@ingroup     IR_MODULE
///@brief       unrecognized data packet
#define IR_UNRECOGNIZE_DATA     0x1F

///
///@ingroup     IR_MODULE
///@brief       IR decode style for NEC data packet
#define IR_NEC_DECODER                      0

///
///@ingroup     IR_MODULE
///@brief       IR decode style for Sony SIRC by bit pattern
#define IR_SONY_SIRC_DECODER_BIT_TYPE       1

///
///@ingroup     IR_MODULE
///@brief       IR decode style for Sony SIRC by data packet
#define IR_SONY_SIRC_DECODER_PACKET_TYPE    2

////////////////////////////////////////////////////////////
//
//  Structure declaration
//
////////////////////////////////////////////////////////////

///
///@ingroup     IR_MODULE
///@brief       Key mapping between IR data and IR key code
typedef struct ST_IR_KEY_TAG
{
    DWORD dwIrData;                             ///< IR data
    DWORD dwIrKey;                              ///< Key code for UI
    BOOL  boolMaskRepeatCode;                   ///< Define the key's repeat code function
} ST_IR_KEY;



////////////////////////////////////////////////////////////
//
//  Function prototype
//
////////////////////////////////////////////////////////////

///
///@ingroup     IR_MODULE
///@brief       Initial IR module
///
///@param       None
///
///@retval      None
///
///@remark
///
void IR_Init(void);

///
///@ingroup     IR_MODULE
///@brief       DeInitial IR module
///
///@param       None
///
///@retval      None
///
///@remark
///
void IR_DeInit(void);

///
///@ingroup     IR_MODULE
///@brief       Register callback function for IR's interrupt
///
///@param       *irIsrCallBackFunc  callback function's pointer, prototype is void (*)(DWORD, BYTE)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///
///@remark
///
void IR_RegisterCallBackFunc(void (*irIsrCallBackFunc)(DWORD, BYTE));

///
///@ingroup     IR_MODULE
///@brief       Install transfomation table for the IR data code to user key code.
///
///@param       *irKeyTabPtr    Key mapping table between IR code and Key code
///@param       keyNum          No of IR keys in mapping table
///
///@retval      None
///
///@remark
///
void IR_KeyTabSet(ST_IR_KEY *irKeyTabPtr, WORD keyNum);

///
///@ingroup     IR_MODULE
///@brief       Setting Power KeyCode
///
///@param       powerKey    Power Key's IR code
///
///@retval      None
///
///@remark
///
void IR_PowerKeyCodeSet(DWORD powerKey);

///
///@ingroup     IR_MODULE
///@brief       Enable/Disable wakeup function by Power Key
///
///@param       enable      Enable/Disanle power key wakeup function
///                         DISABLE(0) / ENABLE(1)
///
///@retval      None
///
///@remark
///
void IR_PowerKeyWakeup(BOOL enable);

///
///@ingroup     IR_MODULE
///@brief       Pause IR function (Disable IR's interrupt)
///
///@param       None
///
///@retval      None
///
///@remark
///
void IR_KeyPause(void);

///
///@ingroup     IR_MODULE
///@brief       Resume IR function (Enable IR's interrupt)
///
///@param       None
///
///@retval      None
///
///@remark
///
void IR_KeyResume(void);

void IR_TypeConfig(BYTE irDecoder, BYTE bitLength);
void IR_IntCauseClear(void);

#endif // __HAL_IR_H

