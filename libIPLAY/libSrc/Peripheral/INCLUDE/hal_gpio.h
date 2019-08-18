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
* Filename      : hal_GPIO.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

#ifndef __HAL_GPIO_H
#define __HAL_GPIO_H

#include "iplaysysconfig.h"

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////

#define HAL_GPIO_GROUP_MASK     0xFFFF0000
#define HAL_GPIO_NUMBER_MASK    0x0000FFFF
#define GPIO_GPIO_NULL          0xFFFFFFFF

#define HAL_GPIO_GROUP_INDEX    0x00000000
#define HAL_FGPIO_GROUP_INDEX   0x00010000
#define HAL_UGPIO_GROUP_INDEX   0x00020000
#define HAL_VGPIO_GROUP_INDEX   0x00040000
#define HAL_AGPIO_GROUP_INDEX   0x00080000
#define HAL_MGPIO_GROUP_INDEX   0x00100000
#define HAL_OGPIO_GROUP_INDEX   0x00200000
#define HAL_PGPIO_GROUP_INDEX   0x00400000
#define HAL_KGPIO_GROUP_INDEX   0x00800000

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's direction is output
#define GPIO_OUTPUT_MODE        0

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's direction is input
#define GPIO_INPUT_MODE         1

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's data is low
#define GPIO_DATA_LOW           0

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's data is high
#define GPIO_DATA_HIGH          1

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO/FGPIO's interrupt is level trigger
#define GPIO_LEVEL_TRIGGER      0

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO/FGPIO's interrupt is edge trigger
#define GPIO_EDGE_TRIGGER       1

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO/FGPIO's interrupt polarity by high level or rising edge
#define GPIO_ACTIVE_HIGH        0

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO/FGPIO's interrupt polarity by low level or falling edge
#define GPIO_ACTIVE_LOW         1


///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's function to default.
#define GPIO_DEFAULT_FUNC       0

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's function to Alternative function 1
#define GPIO_ALT_FUNC_1         1

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's function to Alternative function 2
#define GPIO_ALT_FUNC_2         2

///
///@ingroup     GPIO_MODULE
///@brief       define GPIO's function to Alternative function 3
#define GPIO_ALT_FUNC_3         3

#include "gpio_mp615.h"
#include "gpio_mp650.h"

#define ERR_GPIO_INVALID_NUMBER -1
#define ERR_GPIO_ZERO_PIN_NUM   -2

///
///@ingroup     GPIO_MODULE
///@brief       Initial GPIO's interrupt service
///
///@param       None
///
///@retval      NO_ERR
///
///@remark
///
SDWORD Gpio_IntInit(void);

///
///@ingroup     GPIO_MODULE
///@brief       Configure GPIO's configuration.
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_GROUP_INDEX
///@param       conf            GPIO's configuration.\n
///                             GPIO_DEFAULT_FUNC(0), GPIO_ALT_FUNC_1(1), GPIO_ALT_FUNC_2(2), GPIO_ALT_FUNC_3(3)
///@param       direction       Valid when conf is setting to GPIO function.\n
///                             GPIO_OUTPUT_MODE(0), GPIO_INPUT_MODE(1)
///@param       outData         bit mapping data valid when conf is setting to GPIO function and direction is GPIO_OUTPUT_MODE.\n
///@param       numPins         number of pins will be setting. At least is 1. (continuous)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///@retval      ERR_GPIO_ZERO_PIN_NUM       Parameter error, numPins is zero.
///
///@remark      GPIO_DATA_LOW(0), GPIO_DATA_HIGH(1)
///
SDWORD Gpio_ConfiguraionSet(E_GPIO_GROUP_INDEX gpioNum, DWORD conf, BYTE direction, WORD outData, BYTE numPins);

///
///@ingroup     GPIO_MODULE
///@brief       Configure GPIO's configuration to GPIO function
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_GROUP_INDEX
///@param       direction       Valid when conf is setting to GPIO function.\n
///                             GPIO_OUTPUT_MODE(0), GPIO_INPUT_MODE(1)
///@param       outData         bit mapping data valid when conf is setting to GPIO function and direction is GPIO_OUTPUT_MODE.\n
///@param       numPins         number of pins will be setting. At least is 1. (continuous)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///@retval      ERR_GPIO_ZERO_PIN_NUM       Parameter error, numPins is zero.
///
///@remark      GPIO_DATA_LOW(0), GPIO_DATA_HIGH(1)
///
SDWORD Gpio_Config2GpioFunc(E_GPIO_GROUP_INDEX gpioNum, BYTE direction, WORD outData, BYTE numPins);

///
///@ingroup     GPIO_MODULE
///@brief       Set GPIO's data (Output)
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_GROUP_INDEX
///@param       outData         bit mapping data valid when conf is setting to GPIO function and direction is GPIO_OUTPUT_MODE.\n
///                             GPIO_DATA_LOW(0), GPIO_DATA_HIGH(1)
///@param       numPins         number of pins will be setting. At least is 1. (continuous)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///@retval      ERR_GPIO_ZERO_PIN_NUM       Parameter error, numPins is zero.
///
///@remark      This function would not change any configuraion.\n
///             Before used, AP must makesure the GPIO was configured to GPIO function.
///
SDWORD Gpio_DataSet(E_GPIO_GROUP_INDEX gpioNum, WORD outData, BYTE numPins);

///
///@ingroup     GPIO_MODULE
///@brief       Get GPIO's data (Input)
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_GROUP_INDEX
///@param       *inData         Pointer for save GPIO's bit mapping data of.\n
///                             GPIO_DATA_LOW(0), GPIO_DATA_HIGH(1)
///@param       numPins         number of pins will be setting. At least is 1. (continuous)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///@retval      ERR_GPIO_ZERO_PIN_NUM       Parameter error, numPins is zero.
///
///@remark      This function would not change any configuraion.\n
///             Before used, AP must makesure the GPIO was configured to GPIO function.
///
SDWORD Gpio_DataGet(E_GPIO_GROUP_INDEX gpioNum, WORD *inData, BYTE numPins);

///
///@ingroup     GPIO_MODULE
///@brief	    Configure a specific GPIO pin to trigger a GPIO interrupt
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_INT_GROUP_INDEX
///@param       triggerPolarity Trigger Polarity\n
///                             GPIO_ACTIVE_HIGH(0), GPIO_ACTIVE_LOW(1) when triggerMode is GPIO_LEVEL_TRIGGER.\n
///                             GPIO_ACTIVE_POSITIVE(0), GPIO_ACTIVE_NEGATIVE(1) when triggerMode is GPIO_EDGE_TRIGGER.
///@param       triggerMode     Interrupt pin's trigger mode.\n
///                             GPIO_LEVEL_TRIGGER(0), GPIO_EDGE_TRIGGER(1)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///
///@remark      This function would not change any configuraion.\n
///             Before used, AP must makesure the GPIO was configured to GPIO function.
///
SDWORD Gpio_IntConfig(E_GPIO_INT_GROUP_INDEX gpioNum, DWORD triggerPolarity, DWORD triggerMode);

///
///@ingroup     GPIO_MODULE
///@brief       Register callback function for GPIO's interrupt
///
///@param       gpioNum     GPIO's index number, see E_GPIO_INT_GROUP_INDEX
///@param       gpioIsrPtr  callback function's pointer, prototype is void (*)(WORD)
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///
///@remark
///
SDWORD Gpio_IntCallbackFunRegister(E_GPIO_INT_GROUP_INDEX gpioNum, void (*gpioIsrPtr)(WORD));

///
///@ingroup     GPIO_MODULE
///@brief       Enable GPIO's interrupt
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_INT_GROUP_INDEX
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///
///@remark
///
SDWORD Gpio_IntEnable(E_GPIO_INT_GROUP_INDEX gpioNum);

///
///@ingroup     GPIO_MODULE
///@brief       Disable GPIO's interrupt
///
///@param       gpioNum         GPIO's index number, see \b E_GPIO_INT_GROUP_INDEX
///
///@retval      NO_ERR                      No any error
///@retval      ERR_GPIO_INVALID_NUMBER     Invaild GPIO index
///
///@remark
///
SDWORD Gpio_IntDisable(E_GPIO_INT_GROUP_INDEX gpioNum);

SDWORD Gpio_ConfigurationGet(E_GPIO_GROUP_INDEX, DWORD *);
BOOL Gpio_GpioInstallStatus(void);
SDWORD Gpio_DefaultSettingSet(E_GPIO_GROUP_INDEX, BYTE);
SDWORD Gpio_IntConfigGet(E_GPIO_INT_GROUP_INDEX, BOOL *, BOOL *);

E_GPIO_GROUP_INDEX GetGPIOPinIndex(WORD gpioNum);
E_GPIO_INT_GROUP_INDEX GetGpioIntIndexByGpioPinNum(E_GPIO_GROUP_INDEX gpioNum);

// wrapper
void SetGPIOValue(WORD gpioNum, BYTE gpioValue);
BYTE GetGPIOValue(WORD gpioNum);

#endif      // #ifndef __HAL_GPIO_H

