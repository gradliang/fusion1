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
* Filename      : KeyPad.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "mpTrace.h"

#include "global612.h"
#include "TaskId.h"
#include "peripheral.h"

#if ADC_FOR_KEY_ENABLE
#define __DEBUG_KEY_MESSAGE
//#define __KP_USING_EXCEPTION_TASK__
//#define __ENABLE_ADC_LOW_PASS_FILTER__

#define ADC_DELAY_TIMES							0


#define MAX_KEY_ARRAY_LENGTH                2       // unit is 32bits, means 32 pins.
#define MAX_KEYPAD_ADC_NUMBER               4

static BYTE checkCurrKeyIsReleased(void);
static BYTE currVKeyisEqualToPreVKey(void);
static BYTE checkKeyIsEqual(void);
static void printKeyStatus(void);

static E_KEY_TYPE eKeyScanType = E_KEY_TYPE_KEYSCAN;
static BYTE keyInNum, keyOutNum;
static E_KEY_STATE eKeyState = E_KEY_STATE_STANDBY;
//static BYTE u08KeypadTimerID = 0;
static BYTE keyCodeNum = 0;
static BYTE keyMaskWidth = 0;
static BYTE keyArrayNum = 0;

// Unit is ms
static WORD maxDebouncingTime = 40;
static WORD maxFirstHoleTime = 400;
static WORD maxRepeatHoleTime = 200;
static WORD maxDoubleClickTime = 200;

volatile static DWORD initKeyStatus = 0;
volatile static DWORD keyStatusArray[MAX_KEY_ARRAY_LENGTH] = {0};
volatile static DWORD preKeyStatusArray[MAX_KEY_ARRAY_LENGTH] = {0};
volatile static DWORD preKeyArray[MAX_KEY_ARRAY_LENGTH] = {0};
volatile static DWORD preValidKeyArray[MAX_KEY_ARRAY_LENGTH] = {0};
volatile static DWORD currKeyArray[MAX_KEY_ARRAY_LENGTH] = {0};
volatile static DWORD currValidKeyArray[MAX_KEY_ARRAY_LENGTH] = {0};

volatile static DWORD *keyInPinPtr = 0;
volatile static DWORD *keyOutPinPtr = 0;

static BOOL keypadInstalled = FALSE, keypadTabInstalled = FALSE;
static BOOL keyPadTimerCreated = FALSE;
static BOOL keypadDoubleClickEnable = FALSE;
static void (*keypadCallBackFunPtr)(DWORD, E_KEY_STATE) = NULL;

static ST_KEY_TRANSFORM *keyTransformTabPtr = 0;


static BYTE checkKeyTableIsMatch(ST_KEY_TRANSFORM *keyTransformTabPtr)
{
    WORD i, j;
    DWORD *addr;

    addr = (DWORD *) keyTransformTabPtr->scanCodeMaskAddr;

    if (eKeyState == keyTransformTabPtr->eKeyState)
    {
        for (i = 0, j = keyMaskWidth - 1; i < keyArrayNum; i++, j--)
        {
            if ( keyStatusArray[i] != *(addr + j))
                return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}



static void getKey(void)
{
    MP_DEBUG("%s", __func__);
    DWORD *tmpPreValidKey;
    DWORD *tmpCurValidKey;
    ST_KEY_TRANSFORM *tmpKeyTransformTabPtr;
    WORD j;
    BYTE i;
    BYTE keypresstimes, keyrelestimes;

    tmpPreValidKey = (DWORD *) preValidKeyArray;
    tmpCurValidKey = (DWORD *) currValidKeyArray;
    tmpKeyTransformTabPtr = keyTransformTabPtr;

    for (i = 0; i < keyCodeNum; i++)
    {
        if (checkKeyTableIsMatch(tmpKeyTransformTabPtr) == TRUE)
        {
            if (keypadCallBackFunPtr)
                keypadCallBackFunPtr(tmpKeyTransformTabPtr->u32KeyCode, eKeyState);

            #ifdef __DEBUG_KEY_MESSAGE
            mpDebugPrint("KEY CODE = %d, KEY STATE = %d", tmpKeyTransformTabPtr->u32KeyCode,
                                                          eKeyState);
            #endif

            return;
        }

        tmpKeyTransformTabPtr++;
    }

    #ifdef __DEBUG_KEY_MESSAGE
    mpDebugPrint("KP Table not found !!!");
    #endif
}



static void recordChangedKey(void)
{
    BYTE i;

    for (i = 0 ; i < keyArrayNum ; i++)
    {
        if (eKeyState == E_KEY_STATE_PRESS)
            preKeyStatusArray[i] = keyStatusArray[i];

        keyStatusArray[i] = currValidKeyArray[i] ^ preValidKeyArray[i];
    }
}



static BYTE checkKeyStatusIsEqual(void)
{
    BYTE i;

    for (i = 0; i < keyArrayNum; i++)
    {
        if (keyStatusArray[i] != preKeyStatusArray[i])
            return FALSE;
    }

    return TRUE;
}



static void setCurrValidKeyToPreValidKey(void)
{
    BYTE i;

    for (i = 0 ; i < keyArrayNum ; i++)
        preValidKeyArray[i] = currValidKeyArray[i];
}



static void setCurrKeyToCurrValidKey(void)
{
    BYTE i;

    for (i = 0 ; i < keyArrayNum ; i++)
        currValidKeyArray[i] = currKeyArray[i];
}



static void resetCurrValidKey(void)
{
    BYTE i;

    for (i = 0 ; i < keyArrayNum ; i++)
        currValidKeyArray[i] = initKeyStatus;
}



static void resetPreValidKey(void)
{
    BYTE i;

    for (i = 0 ; i < keyArrayNum ; i++)
        preValidKeyArray[i] = initKeyStatus;
}



static BYTE checkCurrKeyIsReleased(void)
{
    BYTE i;

    for (i = 0; i < keyArrayNum; i++)
    {
        if (currKeyArray[i] != initKeyStatus)
            return FALSE;
    }

    return TRUE;
}



static BYTE currVKeyisEqualToPreVKey(void)
{
    BYTE i = 0;

    while ((currValidKeyArray[i] == preValidKeyArray[i]) && (i < keyArrayNum))
    {
        i++;
    }

    if (i != keyArrayNum)
        return FALSE;

    return TRUE;
}



static BYTE checkKeyIsEqual(void)
{
    BYTE i;

    for (i = 0; i < keyArrayNum; i++)
    {
        if (currKeyArray[i] != preKeyArray[i])
        {
            for (i = 0; i < keyArrayNum; i++)
            {
                preKeyArray[i] = currKeyArray[i];
            }

            return FALSE;
        }
    }

    return TRUE;
}



static void printKeyStatus(void)
{
#ifdef __DEBUG_KEY_MESSAGE
    BYTE i;

    MP_DEBUG("eKeyState = %d", (WORD) eKeyState);

    for (i = 0 ; i < keyArrayNum ; i++)
        MP_DEBUG("keyStatusArray[%d] = 0x%08X", (WORD) i, keyStatusArray[i]);
#endif
}



static void kpWaitDelay(DWORD wCount)
{
    DWORD i, j;

    for (i = 0; i < wCount; i++)
    {
        for (j = 0; j < 0x800; j++);
    }
}


static BOOL checkDoubleClick = FALSE;

static DWORD prePressDbTime;
static DWORD preHoleChangeDbTime;
static DWORD preHoleDbTime;
static DWORD preReleaseDbTime;
static DWORD preDoubleClickDbTime;

#ifdef __KP_USING_EXCEPTION_TASK__
static ST_EXCEPTION_MSG stMsgContent4Keypad = {0};
static volatile WORD dropKeypadMsg = FALSE;

static void keyScanTimerProc(void)
{
    //MP_ALERT("%s", __func__); // log too often
    SDWORD retVal;

    if (dropKeypadMsg == FALSE)
    {
        dropKeypadMsg = TRUE;

        retVal = MessageDrop((BYTE) EXCEPTION_MSG_ID, (BYTE *) &stMsgContent4Keypad, sizeof(ST_EXCEPTION_MSG));

        if (retVal != OS_STATUS_OK)
        {
            dropKeypadMsg = FALSE;
            MP_ALERT("--E-- keypad message drop !!!");
        }
    }
}
#endif



#ifdef __ENABLE_ADC_LOW_PASS_FILTER__
#define __FILTER_ORDER_NUMBER       3

static SDWORD preAdcValue[MAX_KEYPAD_ADC_NUMBER][__FILTER_ORDER_NUMBER];
static SDWORD preValidAdcValue[MAX_KEYPAD_ADC_NUMBER];
#endif

static void keyScanProc(void)
{
    //MP_ALERT("%s", __func__);
    DWORD currTime;
    WORD rdBit;
    BYTE i,j;
    BYTE keyIndx = 0;
    BYTE keyCompareIsEqual;
    BYTE currKeyIsReleased;

    if (!keypadInstalled || !keypadTabInstalled || !keypadCallBackFunPtr)
    {
        MP_ALERT("keyScanProc - Not installed yet !!");
        KeyPad_ScanResume();

#ifdef __KP_USING_EXCEPTION_TASK__
        dropKeypadMsg = FALSE;
#endif

        return;
    }

    // Reset current key status
    for (i = 0; i < keyArrayNum; i++)
        currKeyArray[i] = initKeyStatus;

    //////////////////////////////////////////////////////////////////////////////////
    //
    //  ADC type
    //
    //////////////////////////////////////////////////////////////////////////////////
    switch (eKeyScanType) // E_KEY_TYPE eKeyScanType = E_KEY_TYPE_KEYSCAN;
    {
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    case E_KEY_TYPE_ANALOG_NORMAL_HIGH: ///< ADC type (2), only for MP650 serial.
    case E_KEY_TYPE_ANALOG_NORMAL_LOW:  ///< ADC type (3), only for MP650 serial.
        {
            SDWORD adcValue, error, historyAdcValue;
            DWORD bitMapping;
            BYTE j;

            for (i = 0; i < keyInNum; i++)
            {
                adcValue = EmbededAdc_ValueGet((E_ADC_GROUP_INDEX) keyInPinPtr[i]);
                historyAdcValue = 0;

#ifdef __ENABLE_ADC_LOW_PASS_FILTER__
                for (j = 0; j < __FILTER_ORDER_NUMBER; j++)
                    historyAdcValue += preAdcValue[keyInNum][j];

                if (historyAdcValue > (adcValue * __FILTER_ORDER_NUMBER))
                    error = historyAdcValue - (adcValue * __FILTER_ORDER_NUMBER);
                else
                    error = (adcValue * __FILTER_ORDER_NUMBER) - historyAdcValue;

                if (error != 0)
                {
                    for (j = 1; j < __FILTER_ORDER_NUMBER; j++)
                        preAdcValue[keyInNum][j - 1] = preAdcValue[keyInNum][j];

                    preAdcValue[keyInNum][__FILTER_ORDER_NUMBER - 1] = adcValue;
                    adcValue = preValidAdcValue[keyInNum];
                }
                else
                {
                    preValidAdcValue[keyInNum] = adcValue;
                }
#endif

                bitMapping = (BIT0 << adcValue) << (16 * (i % 2));

                if (eKeyScanType == E_KEY_TYPE_ANALOG_NORMAL_HIGH)
                {
                    if (adcValue != 15)
                    {
                        MP_DEBUG("adcValue = %d", adcValue);
							#ifdef __DEBUG_KEY_MESSAGE
							mpDebugPrint("adcValue = %d", adcValue);
							#endif
                        currKeyArray[i >> 1] &= ~bitMapping;
                    }
                }
                else    // E_KEY_TYPE_ANALOG_NORMAL_LOW
                {
						#if ADC_DELAY_TIMES
						static DWORD st_dwTimes =0;
						static SDWORD st_sdwadcvalue=0;
						if (adcValue)
						{
							if (st_sdwadcvalue != adcValue)
							{
								st_sdwadcvalue =adcValue;
								adcValue=0;
								st_dwTimes = 0;
							}
							else if (st_dwTimes < ADC_DELAY_TIMES)
							{
								st_dwTimes ++;
								adcValue=0;
							}
						}

						#endif
                    if (adcValue)
                    {
                        MP_DEBUG("KP adcValue = %d", adcValue);
							#ifdef __DEBUG_KEY_MESSAGE
			   				mpDebugPrint("KP adcValue = %d", adcValue);
			   				#endif
                        currKeyArray[i >> 1] |= bitMapping;
                    }
                }
            }
        }
        break;
#endif

    //////////////////////////////////////////////////////////////////////////////////
    //
    //  GPIO scan
    //
    //////////////////////////////////////////////////////////////////////////////////
    case E_KEY_TYPE_KEYSCAN: // 2
    case E_KEY_TYPE_DIALSCAN:
        //////////////////////////////////////////////////////////////////////////////////
        //
        //  No scan line
        //
        //////////////////////////////////////////////////////////////////////////////////
        if (keyOutNum == 0)
        {
            for (i = 0; i < keyInNum; i++)
            {
                if (Gpio_DataGet(keyInPinPtr[i], &rdBit, 1) != NO_ERR)
                    mpDebugPrint("--E-- 0x%08X 0x%08X", (DWORD) keyInPinPtr, keyInPinPtr[i]);

                if (rdBit == 1)
                {
                    currKeyArray[keyIndx >> 5] |= (BIT0 << (keyIndx % 32));
                }
                else //if (rdbit == 0)
                {
                    currKeyArray[keyIndx >> 5] &= ~(BIT0 << (keyIndx % 32));
                }

                keyIndx++;
            }
        }
        //////////////////////////////////////////////////////////////////////////////////
        //
        //
        //
        //////////////////////////////////////////////////////////////////////////////////
        else
        {
            for (j = 0; j < keyOutNum; j++)
            {
                if (eKeyScanType == E_KEY_TYPE_KEYSCAN)
                    Gpio_Config2GpioFunc(keyOutPinPtr[j], GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1);
                else
                    Gpio_Config2GpioFunc(keyOutPinPtr[j], GPIO_OUTPUT_MODE, GPIO_DATA_HIGH, 1);

                for (i = 0; i < keyInNum; i++)
                {
                    if (Gpio_DataGet(keyInPinPtr[i], &rdBit, 1) != NO_ERR)
                        mpDebugPrint("-E- 0x%08X 0x%08X", (DWORD) keyInPinPtr, keyInPinPtr[i]);

                    if (rdBit)
                    {
                        currKeyArray[keyIndx >> 5] |= (BIT0 << (keyIndx % 32));
                    }
                    else
                    {
                        currKeyArray[keyIndx >> 5] &= ~(BIT0 << (keyIndx % 32));
                    }

                    keyIndx++;
                }

                if (eKeyScanType == E_KEY_TYPE_KEYSCAN)
                    // Set to input mode - Multi Key
                    Gpio_Config2GpioFunc(keyOutPinPtr[j], GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
                else
                    Gpio_DataSet(keyOutPinPtr[j], GPIO_DATA_LOW, 1);
            }
        }
        break;

    default:
#ifdef __KP_USING_EXCEPTION_TASK__
        dropKeypadMsg = FALSE;
#endif
        return;
        break;
    }

    //////////////////////////////////////////////////////////////////////////////////
    //
    //
    //
    //////////////////////////////////////////////////////////////////////////////////
    currTime = GetSysTime();
    keyCompareIsEqual = checkKeyIsEqual();
    currKeyIsReleased = checkCurrKeyIsReleased();
    MP_DEBUG("eKeyState = %d", eKeyState); 
    // if press and release key, then eKeyStae == 1
    switch (eKeyState)
    {
    case E_KEY_STATE_STANDBY: // 0
        if ( (keyCompareIsEqual == TRUE) && (currKeyIsReleased == FALSE) )
        {
            if (SystemGetElapsedTime(prePressDbTime) >= maxDebouncingTime)
            {
                eKeyState = E_KEY_STATE_PRESS;
                setCurrKeyToCurrValidKey();
                recordChangedKey();
                setCurrValidKeyToPreValidKey();
                printKeyStatus();
                getKey();

                preReleaseDbTime = currTime;
                preHoleDbTime = currTime;
            }
        }
        else
        {
            if (checkDoubleClick)
            {
                if (SystemGetElapsedTime(prePressDbTime) >= maxDebouncingTime)
                {   // missed release code from E_KEY_STATE_RELEASE
                    //mpDebugPrint("\r\nSR -");
                    eKeyState = E_KEY_STATE_RELEASE;
                    printKeyStatus();
                    getKey();

                    eKeyState = E_KEY_STATE_STANDBY;
                    resetCurrValidKey();
                    resetPreValidKey();
                    prePressDbTime = currTime;
                    checkDoubleClick = FALSE;
                }
            }
            else
                prePressDbTime = currTime;
        }
        break;

    case E_KEY_STATE_PRESS: // 1
        if ( (keyCompareIsEqual == TRUE) && (currKeyIsReleased == FALSE) )
        {
            MP_DEBUG("if ( (keyCompareIsEqual == TRUE) && (currKeyIsReleased == FALSE) )"); 
            preReleaseDbTime = currTime;

            if (SystemGetElapsedTime(preHoleDbTime) >= maxFirstHoleTime)
            {
                preHoleDbTime = currTime;
                eKeyState = E_KEY_STATE_HOLD;

                printKeyStatus();
                getKey();
                checkDoubleClick = FALSE;
            }
        }
        else if ( (keyCompareIsEqual == FALSE) && (currKeyIsReleased == FALSE) )
        {   // key changed
            MP_DEBUG(" if ( (keyCompareIsEqual == FALSE) && (currKeyIsReleased == FALSE) )"); 
            eKeyState = E_KEY_STATE_HOLD_CHANGE;
            printKeyStatus();
            getKey();

            eKeyState = E_KEY_STATE_STANDBY;
            resetCurrValidKey();
            resetPreValidKey();
            prePressDbTime = currTime;
            checkDoubleClick = FALSE;
        }
        else
        {   // release
            
            preHoleDbTime = currTime;

            if (SystemGetElapsedTime(preReleaseDbTime) >= maxDebouncingTime)
            {
                MP_DEBUG(" else(release)"); 
                eKeyState = E_KEY_STATE_RELEASE;
                setCurrKeyToCurrValidKey();
                recordChangedKey();
                setCurrValidKeyToPreValidKey();

                if (keypadDoubleClickEnable)
                {
                    MP_DEBUG(" if (keypadDoubleClickEnable)"); 
                    if (checkDoubleClick == TRUE)
                    {
                        //mpDebugPrint("\r\nCDC -");

                        if (checkKeyStatusIsEqual() == TRUE)
                        {
                            eKeyState = E_KEY_STATE_DCLICK;
                        }

                        printKeyStatus();
                        getKey();

                        eKeyState = E_KEY_STATE_STANDBY;
                        resetCurrValidKey();
                        resetPreValidKey();

                        prePressDbTime = currTime;
                        checkDoubleClick = FALSE;
                    }
                    else
                    {
                        preDoubleClickDbTime = currTime;
                    }
                }
                else
                {
                    MP_DEBUG(" else"); 
                    printKeyStatus();
                    getKey();

                    eKeyState = E_KEY_STATE_STANDBY;
                    resetCurrValidKey();
                    resetPreValidKey();

                    prePressDbTime = currTime;
                }
            }
        }
        break;

    case E_KEY_STATE_RELEASE: // 4
        if (currKeyIsReleased == TRUE)
        {   // still release state
            if (SystemGetElapsedTime(preDoubleClickDbTime) >= maxDoubleClickTime)
            {
                eKeyState = E_KEY_STATE_RELEASE;
                printKeyStatus();
                getKey();

                resetCurrValidKey();
                resetPreValidKey();
                eKeyState = E_KEY_STATE_STANDBY;
                prePressDbTime = currTime;
                checkDoubleClick = FALSE;
            }
        }
        else
        {   // new key pressed
            //mpDebugPrint("\r\nNK -");
            resetCurrValidKey();
            resetPreValidKey();
            eKeyState = E_KEY_STATE_STANDBY;
            prePressDbTime = currTime;

            if (keypadDoubleClickEnable)
                checkDoubleClick = TRUE;
        }
        break;

    case E_KEY_STATE_HOLD: // 2
        if ( (keyCompareIsEqual == TRUE) && (currKeyIsReleased == FALSE) )
        {
            preReleaseDbTime = currTime;

            if (SystemGetElapsedTime(preHoleDbTime) >= maxRepeatHoleTime)
            {
                preHoleDbTime = currTime;
                eKeyState = E_KEY_STATE_HOLD;
                printKeyStatus();
                getKey();
            }
        }
        else if ( (keyCompareIsEqual == FALSE) && (currKeyIsReleased == FALSE) )
        {   // new key pressed
            preHoleChangeDbTime = currTime;
            preReleaseDbTime = currTime;
            eKeyState = E_KEY_STATE_HOLD_CHANGE;
            checkDoubleClick = FALSE;
        }
        else
        {   //release
            preHoleDbTime = currTime;

            if (SystemGetElapsedTime(preReleaseDbTime) >= maxDebouncingTime)
            {
                prePressDbTime = currTime;
                eKeyState = E_KEY_STATE_RELEASE;
                setCurrKeyToCurrValidKey();
                recordChangedKey();
                setCurrValidKeyToPreValidKey();

                printKeyStatus();
                getKey();

                eKeyState = E_KEY_STATE_STANDBY;
                checkDoubleClick = FALSE;
            }
        }
        break;

    case E_KEY_STATE_HOLD_CHANGE: // 3
        if ( (keyCompareIsEqual == TRUE) && (currKeyIsReleased == FALSE) )
        {
            preReleaseDbTime = currTime;

            if (SystemGetElapsedTime(preHoleChangeDbTime) >= maxFirstHoleTime)
            {
                setCurrKeyToCurrValidKey();
                resetPreValidKey();
                recordChangedKey();
                setCurrValidKeyToPreValidKey();

                printKeyStatus();
                getKey();

                eKeyState = E_KEY_STATE_PRESS;
                preHoleDbTime = currTime;
                preReleaseDbTime = currTime;
            }
        }
        else if ( (keyCompareIsEqual == FALSE) && (currKeyIsReleased == FALSE) )
        {   // new key pressed
            preHoleChangeDbTime = currTime;
            preReleaseDbTime = currTime;
        }
        else
        {   //release
            preHoleChangeDbTime = currTime;

            if (SystemGetElapsedTime(preReleaseDbTime) >= maxDebouncingTime)
            {
                prePressDbTime = currTime;
                eKeyState = E_KEY_STATE_RELEASE;
                setCurrKeyToCurrValidKey();
                recordChangedKey();
                setCurrValidKeyToPreValidKey();

                printKeyStatus();
                getKey();

                eKeyState = E_KEY_STATE_STANDBY;
                checkDoubleClick = FALSE;
            }
        }
        break;
    }

#ifdef __KP_USING_EXCEPTION_TASK__
    dropKeypadMsg = FALSE;
#endif
}



BYTE KeyPad_GetKeyState(void)
{
    return eKeyState;
}




void KeyPad_ScanPause(void)
{
    if (keyPadTimerCreated)
    {
#ifdef __KP_USING_EXCEPTION_TASK__
        SysTimerProcPause(keyScanTimerProc);
#else
        SysTimerProcPause(keyScanProc);
#endif
    }
}



void KeyPad_ScanResume(void)
{
    if (keyPadTimerCreated)
    {
#ifdef __KP_USING_EXCEPTION_TASK__
        SysTimerProcResume(keyScanTimerProc);
#else
        SysTimerProcResume(keyScanProc);
#endif
    }
}



void KeyPad_KeyTransformTabSet(ST_KEY_TRANSFORM *transTablePtr, BYTE transCount, BYTE maskBitsWidth)
{
    keypadTabInstalled = FALSE;

    if ((maskBitsWidth == 0) || (transCount == 0) || !transTablePtr)
    {
        MP_ALERT("--E-- Key Transform Tab is empty !!!");

        return;
    }

    keyTransformTabPtr = (ST_KEY_TRANSFORM *) transTablePtr;
    keyCodeNum = transCount;
    keyMaskWidth = maskBitsWidth;

    if (keypadInstalled == TRUE)
    {
        if (keyArrayNum > maskBitsWidth)
        {
            MP_ALERT("TransformTab maskBitsWidth is too small");

            return;
        }
    }

    MP_DEBUG("keyTransformTabPtr = 0x%08X", (DWORD) keyTransformTabPtr);
    keypadTabInstalled = TRUE;
}



S32 KeyPad_KeypadInit(DWORD *pKeyIn, BYTE inCount, DWORD *pKeyOut, BYTE outCount, E_KEY_TYPE eKeyType)
{
    S32 retCode;
    BYTE i;

    MP_DEBUG("%s : inCount = %d, outCount = %d", __func__, inCount, outCount);
    MP_DEBUG("%s : eKeyType = %d(2=E_KEY_TYPE_KEYSCAN or E_KEY_TYPE_ANALOG_NORMAL_HIGH - conflict, need to clear!)", __func__, eKeyType);
    MP_DEBUG("eKeyType = 2 is for easy to debug both case by set compiler option!");
    keyPadTimerCreated = FALSE;
#ifdef __KP_USING_EXCEPTION_TASK__
    stMsgContent4Keypad.dwTag = 0;
    stMsgContent4Keypad.msgAddr = 0;
    dropKeypadMsg = FALSE;
#endif
    keypadInstalled = FALSE;
    eKeyScanType = eKeyType;
    keyInNum = inCount;
    keyOutNum = outCount;

    MP_DEBUG("inCount = %d", inCount);
    MP_DEBUG("outCount = %d", outCount);
    MP_DEBUG("u08KeyArrayNum = %d", keyArrayNum);

    if ( (inCount == 0) || !pKeyIn )
    {
        MP_DEBUG("--E-- There is no key input !!!\r\n");

        return -3;
    }

#if (CHIP_VER_MSB == CHIP_VER_615)
    if ( (eKeyType == E_KEY_TYPE_ANALOG_NORMAL_HIGH) || (eKeyType == E_KEY_TYPE_ANALOG_NORMAL_LOW) )
    {
        MP_ALERT("--E-- Not Support ADC type Keypad !!!\r\n");

        return -4;
    }
#endif

    ///////////////////////////////////////////////////////////////////
    //
    //  Embedded ADC
    //
    ///////////////////////////////////////////////////////////////////
    switch (eKeyType)
    {
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case E_KEY_TYPE_ANALOG_NORMAL_HIGH:
    case E_KEY_TYPE_ANALOG_NORMAL_LOW:
        #ifdef __ENABLE_ADC_LOW_PASS_FILTER__
        memset((BYTE *) &preAdcValue, 255, sizeof(preAdcValue));
        memset((BYTE *) &preValidAdcValue, 255, sizeof(preValidAdcValue));
        #endif

        keyInNum %= MAX_KEYPAD_ADC_NUMBER;          // Max ADC number is 4
        keyArrayNum = 2;
        keyOutNum = 0;

        for (i = 0; i < keyArrayNum; i++)
        {
            if (eKeyScanType == E_KEY_TYPE_ANALOG_NORMAL_HIGH)
            {
                preKeyArray[i] = 0xFFFFFFFF;
                preValidKeyArray[i] = 0xFFFFFFFF;
                currKeyArray[i] = 0xFFFFFFFF;
                currValidKeyArray[i] = 0xFFFFFFFF;
                keyStatusArray[i] = 0xFFFFFFFF;
                preKeyStatusArray[i] = 0xFFFFFFFF;
                initKeyStatus = 0xFFFFFFFF;
            }
            else
            {
                preKeyArray[i] = 0;
                preValidKeyArray[i] = 0;
                currKeyArray[i] = 0;
                currValidKeyArray[i] = 0;
                keyStatusArray[i] = 0;
                preKeyStatusArray[i] = 0;
                initKeyStatus = 0;
            }
        }

        keyInPinPtr = pKeyIn;
        keyOutPinPtr = 0;

        MP_DEBUG("u32PtrKeyInPin = 0x%08X", (DWORD) keyInPinPtr);

        for (i = 0; i < inCount; i++)
        {
            if (eKeyType == E_KEY_TYPE_ANALOG_NORMAL_HIGH)
                EmbededAdc_LPF_Initial((E_ADC_GROUP_INDEX) pKeyIn[i], ENABLE, 15);
            else
                EmbededAdc_LPF_Initial((E_ADC_GROUP_INDEX) pKeyIn[i], ENABLE, 0);

            EmbededAdc_Enable((E_ADC_GROUP_INDEX) pKeyIn[i], ADC_CLK_SEL_PLL2 | ADC_CLK_DIV_16);
            MP_DEBUG("ADC-%d Enabled", pKeyIn[i]);
        }
        break;
#endif

    ///////////////////////////////////////////////////////////////////
    //
    //  GPIO
    //
    ///////////////////////////////////////////////////////////////////
    case E_KEY_TYPE_KEYSCAN:
    case E_KEY_TYPE_DIALSCAN:
        if (outCount == 0) // inCount = 1, outCount = 0
        {
            keyArrayNum = (inCount >> 5) + 1; // 1 / 32 + 1 = 1
            pKeyOut = 0;
        }
        else
        {
            keyArrayNum = ((inCount * outCount) >> 5) + 1; // (1 * 0) / 32 + 1 = 1
            
        }

        MP_DEBUG("%s : keyArrayNum = %d", __func__, keyArrayNum);
        if (keyArrayNum > MAX_KEY_ARRAY_LENGTH)
        {
            MP_ALERT("--E-- Key number was exceed to max key number - %d", MAX_KEY_ARRAY_LENGTH << 5);

            return -5;
        }

        for (i = 0; i < keyArrayNum; i++)
        {
            switch (eKeyScanType)
            {
            case E_KEY_TYPE_KEYSCAN:
            default:
                eKeyScanType = E_KEY_TYPE_KEYSCAN;
                preKeyArray[i] = 0xFFFFFFFF;
                preValidKeyArray[i] = 0xFFFFFFFF;
                currKeyArray[i] = 0xFFFFFFFF;
                currValidKeyArray[i] = 0xFFFFFFFF;
                keyStatusArray[i] = 0xFFFFFFFF;
                preKeyStatusArray[i] = 0xFFFFFFFF;
                initKeyStatus = 0xFFFFFFFF;
                break;
            case E_KEY_TYPE_DIALSCAN:
                preKeyArray[i] = 0;
                preValidKeyArray[i] = 0;
                currKeyArray[i] = 0;
                currValidKeyArray[i] = 0;
                keyStatusArray[i] = 0;
                preKeyStatusArray[i] = 0;
                initKeyStatus = 0;
                break;
            }
        }

        keyInPinPtr = pKeyIn;
        keyOutPinPtr = pKeyOut;

        MP_DEBUG("u32PtrKeyInPin = 0x%08X", (DWORD) keyInPinPtr);
        MP_DEBUG("u32PtrKeyOutPin = 0x%08X", (DWORD) keyOutPinPtr);

        for (i = 0; i < inCount; i++)
        {
            retCode = Gpio_Config2GpioFunc(pKeyIn[i], GPIO_INPUT_MODE, 0, 1);
            MP_DEBUG("pKeyIn[%d] = 0x%08X", (WORD) i, pKeyIn[i]);

            if (retCode != NO_ERR)
                return retCode;
        }

        for (i = 0; i < outCount; i++)
        {
            if (eKeyScanType == E_KEY_TYPE_KEYSCAN)
                retCode = Gpio_Config2GpioFunc(pKeyOut[i], GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);
            else
                retCode = Gpio_Config2GpioFunc(pKeyOut[i], GPIO_OUTPUT_MODE, GPIO_DATA_LOW, 1);

            MP_DEBUG("pKeyOut[%d] = 0x%08X", (WORD) i, pKeyOut[i]);

            if (retCode != NO_ERR)
                return retCode;
        }
        break;
    }

    if (keypadTabInstalled == TRUE)
    {
        if (keyArrayNum > keyMaskWidth)
        {
            MP_ALERT("TransformTab maskBitsWidth is too small !!!");
            __asm("break 100");

            return -3;
        }
    }

    prePressDbTime = GetSysTime();

    eKeyState = E_KEY_STATE_STANDBY;
    checkDoubleClick = FALSE;
    keypadInstalled = TRUE;

    return NO_ERR;
}



void KeyPad_KeyScanProcStart(void)
{
    MP_DEBUG("%s: keypadInstalled = %d, keypadTabInstalled = %d", __func__, keypadInstalled, keypadTabInstalled);
    if (!keypadInstalled || !keypadTabInstalled)
    {
        MP_ALERT("--E-- Keypad Scan Proc Start Failure ...\r\n");
        keyPadTimerCreated = FALSE;
#ifdef __KP_USING_EXCEPTION_TASK__
        stMsgContent4Keypad.dwTag = 0;
        stMsgContent4Keypad.msgAddr = 0;
        dropKeypadMsg = FALSE;
#endif

        return;
    }

    MP_DEBUG("%s: keyPadTimerCreated = %d", __func__, keyPadTimerCreated);
    if (!keyPadTimerCreated)
    {
        SDWORD retVal;

#ifdef __KP_USING_EXCEPTION_TASK__
        MP_DEBUG("%s: __KP_USING_EXCEPTION_TASK__ def", __func__);
        dropKeypadMsg = FALSE;
        stMsgContent4Keypad.dwTag = ExceptionTagReister(keyScanProc);

        if (ExceptionTagCheck(stMsgContent4Keypad.dwTag) )
        {
            MP_ALERT("--E-- Keypad Exception Register Failure ...\r\n");
            keyPadTimerCreated = FALSE;
            stMsgContent4Keypad.dwTag = 0;

            return;
        }

        if (SysTimerProcAdd(16, keyScanTimerProc, FALSE) < 0)
        {
            MP_ALERT("--E-- Keypad Timer Start Failure ...\r\n");
            keyPadTimerCreated = FALSE;
            ExceptionTagRelease(stMsgContent4Keypad.dwTag);
            stMsgContent4Keypad.dwTag = 0;

            return;
        }
#else
        if (SysTimerProcAdd(16, keyScanProc, FALSE) < 0)
        {
            MP_ALERT("--E-- Keypad Timer Start Failure ...\r\n");
            keyPadTimerCreated = FALSE;

            return;
        }
#endif

        keyPadTimerCreated = TRUE;
    }
}



S32 KeyPad_KeyRepeatParamSet(WORD dbTimes, DWORD keyHoldTime, DWORD keyResolution)
{
    maxDebouncingTime = dbTimes;
    maxFirstHoleTime = keyHoldTime;
    maxRepeatHoleTime = keyResolution;

    return NO_ERR;
}



void KeyPad_RegisterCallBackFunc(void (*keypadCallbackFuncPtr)(DWORD, E_KEY_STATE))
{
    keypadCallBackFunPtr = keypadCallbackFuncPtr;

    if (!keypadCallBackFunPtr && keyPadTimerCreated)
    {
#ifdef __KP_USING_EXCEPTION_TASK__
        SysTimerProcRemove(keyScanTimerProc);
#else
        SysTimerProcRemove(keyScanProc);
#endif
        keyPadTimerCreated = FALSE;
    }
}



void KeyPad_DoubleClickEnable(BOOL isEnable)
{
    keypadDoubleClickEnable = isEnable;
}
#endif

