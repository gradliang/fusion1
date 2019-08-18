#ifndef __KEYPAD_H
#define __KEYPAD_H

#include "iplaysysconfig.h"

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////

///
///@ingroup     KEYPAD_MODULE
///@brief       define key's state
typedef enum
{
    E_KEY_STATE_STANDBY = 0,
    E_KEY_STATE_PRESS,                  ///< Key pressed state (1)
    E_KEY_STATE_HOLD,                   ///< Key hold state (2)
    E_KEY_STATE_HOLD_CHANGE,            ///< Key hold change state (3)
    E_KEY_STATE_RELEASE,                ///< Key released state (4)
    E_KEY_STATE_DCLICK,                 ///< Double click state (5)
} E_KEY_STATE;

///
///@ingroup     KEYPAD_MODULE
///@brief       define keypad's type
typedef enum
{
    E_KEY_TYPE_KEYSCAN = 0,             ///< KeyScan (0) - Scanline is output low
    E_KEY_TYPE_DIALSCAN,                ///< DialScan (1) - Scanline is output high
    E_KEY_TYPE_ANALOG_NORMAL_HIGH,      ///< ADC type (2), only for MP650 serial.
    E_KEY_TYPE_ANALOG_NORMAL_LOW,       ///< ADC type (3), only for MP650 serial.
} E_KEY_TYPE;


////////////////////////////////////////////////////////////
//
//  Structure declaration
//
////////////////////////////////////////////////////////////

///
///@ingroup     KEYPAD_MODULE
///@brief       define keypad transform table
typedef struct
{
    E_KEY_STATE eKeyState;              ///< state condition for reaction
    DWORD       scanCodeMaskAddr;       ///< pin's bit mapping
    DWORD       u32KeyCode;             ///< relative key code
} ST_KEY_TRANSFORM;


////////////////////////////////////////////////////////////
//
//  Function prototype
//
////////////////////////////////////////////////////////////

///
///@ingroug KEYPAD_MODULE
///@brief   Specify the input/output pins to initialize the keypad array
///
///@param   *pKeyIn     address of the input pins name string array (return line)
///                     For E_KEY_TYPE_KEYSCAN and E_KEY_TYPE_DIALSCAN, there are GPIO's name.
///                     For E_KEY_TYPE_ANALOG_NORMAL_HIGH and E_KEY_TYPE_ANALOG_NORMAL_LOW, there are ADC's index.
///@param   inCount     number of the input pins, at least 1.
///@param   *pKeyOut    address of the output pins name string array (scan line)
///@param   outCount    number of the output pins, at least 0.
///@param   eKeyType    Keypad type. See E_KEY_TYPE.
///                     E_KEY_TYPE_KEYSCAN (0) - Scanline is output low
///                     E_KEY_TYPE_DIALSCAN (1) - Scanline is output high
///                     E_KEY_TYPE_ANALOG_NORMAL_HIGH (2) - Voltage type, normal high
///                     E_KEY_TYPE_ANALOG_NORMAL_LOW (3) - Voltage type, normal low
///
///@return  NO_ERR
///@return  ERR_GPIO_INVALID_NUMBER
///@return  ERR_GPIO_ZERO_PIN_NUM
///@return  -3          No input key
///@return  -4          Not Support ADC type Keypad
///
///@remark  The input pins and the output pins will generate a inCount x outCount keypad.
///         The programmer should depends on the product schematic to decide the content
///         of the parameter of this function.
///
S32 KeyPad_KeypadInit(DWORD *pKeyIn, BYTE inCount, DWORD *pKeyOut, BYTE outCount, E_KEY_TYPE eKeyType);

///
///@ingroup KEYPAD_MODULE
///@brief   Install the keypad scan code to user code transformation table
///
///@param   transTablePtr   Structure pointer of keypad transform table
///@param   transCount      Numbers of key code
///@param   maskBitsWidth   Key's bit mapping width, unit is 32bits (DWORD).
///                         1 : Means 32bits width, that is supported to 32's key status.
///                         2 : Means 64bits width, that is supported to 64's key status.
///
///@return  None
///
///@remark  All the valid key code will be return by mapping the contents of key transform table.
///
///
void KeyPad_KeyTransformTabSet(ST_KEY_TRANSFORM *transTablePtr, BYTE transCount, BYTE maskBitsWidth);

///
///@ingroup KEYPAD_MODULE
///@brief   Start keypad scanning
///
///@param   None
///
///@return  None
///
///@remark  This function should be called after all the necessary keypad parameters
///         have been set.
///
void KeyPad_KeyScanProcStart(void);

///
///@ingroup KEYPAD_MODULE
///@brief   Pause KeyPad function (Disable KeyPad's scan proc)
///
///@param   None
///
///@retval  None
///
///@remark
///
void KeyPad_ScanPause(void);

///
///@ingroup KEYPAD_MODULE
///@brief   Resume KeyPad function (Resume KeyPad's scan proc)
///
///@param   None
///
///@retval  None
///
///@remark
///
void KeyPad_ScanResume(void);

///
///@ingroup KEYPAD_MODULE
///@brief   return the current key state
///
///@param   None
///
///@return  E_KEY_STATE_PRESS
///@return  E_KEY_STATE_HOLD
///@return  E_KEY_STATE_RELEASE
///
///@remark
///
BYTE KeyPad_GetKeyState(void);

///
///@ingroup KEYPAD_MODULE
///@brief   Set key hold and repeat parameters
///
///@param   dbTimes         Debouncing time in mini-second
///@param   keyHoldTime     Key hold time before key repeat in mini-second
///@param   keyResolution   Key repeat duration in mini-second
///
///@return  NO_ERR
///
///@remark  Default value
///         Debouncing time is 40ms.
///         Hold time is 400ms.
///         Repeat period is 200ms.
///
S32 KeyPad_KeyRepeatParamSet(WORD dbTimes, DWORD keyHoldTime, DWORD keyResolution);

///
///@ingroup KEYPAD_MODULE
///@brief   Register callback function for Keypad module
///
///@param   *keypadCallbackFuncPtr  callback function's pointer, prototype is void (*)(DWORD, E_KEY_STATE)
///                                 first parameter is key code
///                                 second parameter is key state. See E_KEY_STATE
///
///@retval  None
///
///@remark
///
void KeyPad_RegisterCallBackFunc(void (*keypadCallbackFuncPtr)(DWORD, E_KEY_STATE));

///
///@ingroup KEYPAD_MODULE
///@brief   Switch the function of Keypad's double click.
///
///@param   isEnable        ENABLE(1)/DISABLE(0) the function of double click.
///
///@return  None
///
///@remark  Default is disable.
///
void KeyPad_DoubleClickEnable(BOOL isEnable);

#endif

