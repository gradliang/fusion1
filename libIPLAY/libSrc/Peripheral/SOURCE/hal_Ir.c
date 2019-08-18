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
* Filename      : hal_Ir.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : IR receiver Module
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "peripheral.h"

#ifndef IR_ENABLE
#define IR_ENABLE       0
#endif

#if (IR_ENABLE == 1)

/*
// Constant declarations
*/
//#define __ENABLE_IR_DEBUG_MODE__

// For NEC decoder
#define IR_REPEAT_CODE_MASK_TIMES           3
#define FIRST_IR_REPEAT_CODE_MASK_TIMES     5
#define MASK_REPEAT_TIME                    500
#define MASK_DEBOUNCING_TIME                100
#define IR_NULL_NEC_CODE                    0xFFFFFFFF

#define IR_CAUSE_EVENT_COME                 BIT16
#define IR_CAUSE_EVENT_OVERWRITE            BIT17
#define IR_CAUSE_NOISE_COME                 BIT18
#define IR_MASK_EVENT_COME                  BIT24
#define IR_MASK_EVENT_OVERWRITE             BIT25
#define IR_MASK_NOISE_COME                  BIT26

//#define IR_TIME_BASE                        9080         // 9.08us
#define IR_TIME_BASE                        30300        // 30.3us

#if ((CHIP_VER_MSB == CHIP_VER_615) || (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA)))
#define __IR_CKS_USING_XIN
#else
    #if (Make_EMBEDDED_RTC == 1)
        #if 1//(CHIP_VER == (CHIP_VER_650 | CHIP_VER_A))
        #define __IR_CKS_USING_XIN
        #else
        // using RXIN (RTC clock)
        #endif
    #else
    #define __IR_CKS_USING_XIN
    #endif
#endif

/*
// Function declarations
*/
static void irNullCallbackFuc(DWORD, BYTE);
static void irIsr(void);
static void irGetKey(DWORD);
static void irGetKeySonySIRC(DWORD);


/*
// Variable declarations
*/
static BOOL dropRepeastCode = TRUE;
static DWORD irKeyCode = IR_NULL_NEC_CODE;
static DWORD preRepeatCodeEnterTime = 0;
static DWORD preDataCodeEnterTime = 0;
static ST_IR_KEY *irKeyTablePtr = 0;
static DWORD repeatTimes = FIRST_IR_REPEAT_CODE_MASK_TIMES;
static WORD irCustomerCode = 0xFFFF;
static WORD irKeyTypeNum = 0;
static WORD irDecoderType = IR_NEC_DECODER;
#ifndef __ENABLE_IR_DEBUG_MODE__
static WORD irReportStyle = 3;
#else
static WORD irReportStyle = 0;
#endif
static WORD irBitLength = 32;
static DWORD preIrCode = IR_NULL_NEC_CODE;

static WORD sircStartPeriodMax = 0;
static WORD sircStartPeriodMin = 0;
static WORD sircOnePeriodMin = 0;
static WORD sircStopPeriodMin = 0;

static void (*irIsrCallbackFunctionPtr)(DWORD, BYTE) = irNullCallbackFuc;
static void (*irServicePtr)(DWORD) = irGetKey;

//////////////////////////////////////////////////////////////////
//
// Definition of local functions
//
//////////////////////////////////////////////////////////////////
static void irNullCallbackFuc(DWORD keyCode, BYTE state)
{
    // Do nothing
}



static DWORD pow(BYTE a, BYTE b)
{
    BYTE i;
    DWORD result = 1;

    for (i = 0; i < b; i++)
        result *= a;

    return result;
}



static BYTE calIrTimeBase(DWORD clkFreq)
{
    BYTE a, b;
    BYTE minL = 0, minM = 0;
    DWORD minTimeBase, minTolerance = 0xFFFFFFFF, curTolerance, curValue;

    //IR_TIME_BASE = ((a + 16) * 2E(b) + 2) * Tsc, a = [0:15], b = [0:15]
    //             = ((a + 16) * 2E(b) + 2) / clkFreq
    for (a = 0; a <= 0x0F; a++)
    {
        for (b = 0; b <= 0x0F; b++)
        {
            curValue = (LONG) ((a + 16) * pow(2, b) + 2) * (LONG) 1000000000 / clkFreq;

            if (curValue > IR_TIME_BASE)
                curTolerance = curValue - IR_TIME_BASE;
            else
                curTolerance = IR_TIME_BASE - curValue;

            if (curTolerance < minTolerance)
            {
                minTimeBase = curValue;
                minTolerance = curTolerance;
                minL = a;
                minM = b;
            }
        }
    }

    minM <<= 4;
    minM |= minL;

    MP_ALERT("\n\rIR Time Base Param is %d", minM);
    MP_ALERT("Tolerance is %d", minTolerance);
    MP_ALERT("Period is %d\n\r", minTimeBase);

    return minM;
}



static void irGpioInit(void)
{
#if (CHIP_VER_MSB == CHIP_VER_615)
    Gpio_ConfiguraionSet(GPIO_GPIO_16, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0, 1);
#else   // MP650/660
    Gpio_ConfiguraionSet(GPIO_GPIO_2, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0, 1);
#endif
}



static void irGetKeySonySIRC(DWORD data)
{
    static WORD bitLength = 0;
    static DWORD irData;
    IR *regIrPtr = (IR *) IR_BASE;
    DWORD IrOthers, i, irStatus = regIrPtr->IrStatus & 0x1F;
    WORD level, period;
    BOOL dataMatch = FALSE;

    if (!irKeyTablePtr)
    {
        MP_ALERT("--E-- IR table not installed yet!!!");
        IR_DeInit();

        return;
    }

    ////////////////////////////////////////////////////////////
    if (irReportStyle == 0)
    {
        IrOthers = regIrPtr->IrOthers;
        level = (IrOthers >> 15) & BIT0;

        if (level == 0)
            return;

        period = (IrOthers >> 16) & 0xFFFF;

        if (((period > sircStartPeriodMin) && (period < sircStartPeriodMax)) || (period > sircStopPeriodMin))
        {   // Start or Stop
            irData = 0;
            bitLength = 0;

            return;
        }
        else
        {
            irData <<= 1;
            bitLength++;

            if (period > sircOnePeriodMin)
            {   // 1
                irData |= 1;
            }

            if (bitLength < irBitLength)
                return;
        }
    }
    ////////////////////////////////////////////////////////////
    else if (irReportStyle == 1)
    {
        irData <<= 1;

        switch (irStatus)
        {
        case 0x11:          // 1
            irData |= 1;
            break;

        case 0x10:          // 0
            break;

        //case 0x12:          // Start
        //case 0x14:          // Stop
        default:
            irData = 0;
            bitLength = 0;
            return;
            break;
        }

        bitLength++;

        if (bitLength < (irBitLength - 1))
            return;
    }
    ////////////////////////////////////////////////////////////
    else if (irReportStyle == 3)
    {
        switch (irStatus)
        {
        case IR_NECDATA_ERROR:
        case IR_NECDATA:
            irData = data;
            break;

        case IR_NECREPEAT:
            return;

            break;

        case IR_REMOTE_POINT_MOUSE:
            MP_ALERT("IR Data packet of Remote point mouse!!!");

            return;
            break;

        //case IR_UNRECOGNIZE_DATA:
        default:
            switch (irStatus)
            {
            case IR_UNRECOGNIZE_DATA:
                MP_DEBUG("IR Unrecognized data packet!!!");
                break;
            default:
                MP_ALERT("Unknow IR status - 0x%02X!!!", irStatus);
                break;
            }

            preIrCode = IR_NULL_NEC_CODE;
            irKeyCode = IR_NULL_NEC_CODE;
            preDataCodeEnterTime = 0;

            return;
            break;
        }
    }
    else
        return;

    if (preIrCode != irData)
    {
        preIrCode = irData;
        preDataCodeEnterTime = GetSysTime();
        MP_DEBUG("N 0x%05X", irData);
    }
    else
    {
        if (SystemGetElapsedTime(preDataCodeEnterTime) < MASK_DEBOUNCING_TIME)
            return;

        preDataCodeEnterTime = GetSysTime();
        MP_DEBUG("O 0x%05X", irData);
    }

    for (i = 0; i < irKeyTypeNum; i++)
    {
        if (irReportStyle == 0)
        {
            if ((irKeyTablePtr + i)->dwIrData == irData)
                dataMatch = TRUE;
        }
        else
        {
            if (((irKeyTablePtr + i)->dwIrData >> 1) == irData)
                dataMatch = TRUE;
        }

        if (dataMatch)
        {
            if ((irKeyTablePtr + i)->boolMaskRepeatCode == ENABLE)
            {
                dropRepeastCode = FALSE;
                preRepeatCodeEnterTime = preDataCodeEnterTime;
            }

            irKeyCode = (irKeyTablePtr + i)->dwIrKey;
            irIsrCallbackFunctionPtr(irKeyCode, IR_NECDATA);

            break;
        }
    }

    if (i == irKeyTypeNum)
    {
        irKeyCode = IR_NULL_NEC_CODE;
        MP_DEBUG("%s - IR Table not found %d %d!!!", __FUNCTION__, i, irKeyTypeNum);
    }
}



static void irGetKey(DWORD irData)
{
    //mpDebugPrint("%s: irData = 0x%08X", __func__, irData);
    register IR *regIrPtr = (IR *) IR_BASE;
    register WORD i, irStstus;

    irStstus = regIrPtr->IrStatus & 0x1F;

    if (!irKeyTablePtr)
    {
        MP_ALERT("--E-- IR table not installed yet!!!");

        IR_DeInit();

        return;
    }

    if ( (irStstus == IR_NECREPEAT) && (preIrCode != irData) && !dropRepeastCode)
    {   // repeat code overwaite data code
        irStstus = IR_NECDATA;
        MP_ALERT("Missed !!!");
    }

    //mpDebugPrint("%s: irStstus = 0x%04X", __func__, irStstus);
    switch (irStstus)
    {
    case IR_NECDATA:
        mpDebugPrint("IR Code = 0x%08X", irData);
        MP_DEBUG("IR Status = 0x%05X", irStstus);

        if (SystemGetElapsedTime(preDataCodeEnterTime) < MASK_DEBOUNCING_TIME)
        {
            if (preIrCode == irData)
            {
                mpDebugPrintN(":<");
                return;
            }
        }

        repeatTimes = FIRST_IR_REPEAT_CODE_MASK_TIMES;
        dropRepeastCode = TRUE;
        preIrCode = irData;
        preDataCodeEnterTime = GetSysTime();

        for (i = 0; i < irKeyTypeNum; i++)
        {
            if ((irKeyTablePtr + i)->dwIrData == irData)
            {
                if ((irKeyTablePtr + i)->boolMaskRepeatCode == ENABLE)
                {
                    dropRepeastCode = FALSE;
                    preRepeatCodeEnterTime = preDataCodeEnterTime;
                }

                irKeyCode = (irKeyTablePtr + i)->dwIrKey;
                irIsrCallbackFunctionPtr(irKeyCode, IR_NECDATA);

                break;
            }
        }

        if (i == irKeyTypeNum)
        {
            irKeyCode = IR_NULL_NEC_CODE;
            MP_DEBUG("%s - IR Table not found %d %d!!!", __FUNCTION__, i, irKeyTypeNum);
        }
        //else mpDebugPrint("Found i = %d", i);
        
        break;

    case IR_NECREPEAT:
        if (!dropRepeastCode)
        {
            MP_DEBUGN("R");

            if (SystemGetElapsedTime(preRepeatCodeEnterTime) > MASK_REPEAT_TIME)
            {
                MP_DEBUG("Repeat timeout !!!\r\n%d %d", preRepeatCodeEnterTime, GetSysTime());
                dropRepeastCode = TRUE;
                preIrCode = IR_NULL_NEC_CODE;

                break;
            }

            preRepeatCodeEnterTime = GetSysTime();

            if (repeatTimes)
                repeatTimes--;
            else
            {
                repeatTimes = IR_REPEAT_CODE_MASK_TIMES;
                irIsrCallbackFunctionPtr(irKeyCode, IR_NECREPEAT);
            }
        }
        else
        {
            MP_DEBUG("Repeat code was masked !!!");
        }
        break;

    case IR_REMOTE_POINT_MOUSE:
        MP_ALERT("IR Data packet of Remote point mouse!!!");
        break;

    //case IR_NECDATA_ERROR:
    //case IR_UNRECOGNIZE_DATA:
    default:
        switch (irStstus)
        {
        case IR_UNRECOGNIZE_DATA:
            MP_ALERT("IR Unrecognized data packet!!!");
            break;
        case IR_NECDATA_ERROR:
            MP_ALERT("IR data packet with error of NEC button!!!");
            break;
        default:
            MP_ALERT("Unknow IR status - 0x%02X!!!", irStstus);
            break;
        }

        repeatTimes = FIRST_IR_REPEAT_CODE_MASK_TIMES;
        dropRepeastCode = TRUE;
        preIrCode = IR_NULL_NEC_CODE;
        irKeyCode = IR_NULL_NEC_CODE;
        break;
    }
}



//////////////////////////////////////////////////////////////////
//
// Definition of external functions
//
//////////////////////////////////////////////////////////////////
void IR_TypeConfig(BYTE irDecoder, BYTE bitLength)
{
    irDecoderType = irDecoder;
    irBitLength = bitLength;
}



void IR_Init(void)
{
    IR *regIrPtr = (IR *) IR_BASE;
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD tmp;

#if (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA))
    // using old design for FPGA, mean RXIN is XIN
    mSetIrCks(IRCKS_RXIN);
#else
    #ifdef __IR_CKS_USING_XIN
    mSetIrCks(IRCKS_XIN);
    #else
    mSetIrCks(IRCKS_RXIN);
    #endif
#endif

    irGpioInit();
    regIrPtr->IrCtrl = 0x00009600;
    regClockPtr->MdClken |= CKE_IR;

    // Reset IR
    regBiuPtr->BiuArst |= ARST_IR;
    for (tmp = 0; tmp < 0x2000; tmp++);
    regBiuPtr->BiuArst &= ~ARST_IR;
    for (tmp = 0; tmp < 0x2000; tmp++);
    regBiuPtr->BiuArst |= ARST_IR;

    switch (irDecoderType)
    {
    case IR_NEC_DECODER:
        MP_ALERT("NEC type remote controllor -");
        irBitLength = 32;
#ifndef __ENABLE_IR_DEBUG_MODE__
        irReportStyle = 3;
#endif

        regIrPtr->IrCtrl = 0x00009000 | (irReportStyle << 9);
#ifdef __IR_CKS_USING_XIN

    #if (IR_TIME_BASE == 9080)              // 9.08us
        // Need 9.08us
        #if (MAIN_CRYSTAL_CLOCK == 13500000)
        regIrPtr->IrTimeBase = 0x0000002E;  // IRClock = 13.5MHz, TimeBase = 9.037us = [(14+16)*2E(2) + 2]/(13.5x10E6)
        #elif (MAIN_CRYSTAL_CLOCK == 8000000)
        regIrPtr->IrTimeBase = 0x00000022;  // IRClock = 8MHz, TimeBase = 9.25us = [(2+16)*2E(2) + 2]/(8x10E6)
        #elif (MAIN_CRYSTAL_CLOCK == 16000000)
        regIrPtr->IrTimeBase = 0x00000032;  // IRClock = 16MHz, TimeBase = 9.125us = [(2+16)*2E(3) + 2]/(16x10E6)
        #elif (MAIN_CRYSTAL_CLOCK == 12000000)
        regIrPtr->IrTimeBase = 0x0000002B;  // IRClock = 12MHz, TimeBase = 9.166us = [(11+16)*2E(2) + 2]/(12x10E6)
        #else
        regIrPtr->IrTimeBase = calIrTimeBase(MAIN_CRYSTAL_CLOCK);
        #endif

        regIrPtr->IrLStart = 0x00BFE66E;    // H = 230, L= 110, X = 2, D = 31
        regIrPtr->IrLRepeat = 0x00BFE632;   // H = 230, L = 50, X = 2 , D = 31
        regIrPtr->IrLBit1 = 0x003F30A0;     // H = 48, L = 160, X = 0, D = 31
        regIrPtr->IrLBit0 = 0x003F3030;     // H = 48, L = 48, X = 0, D = 31
        //half range : 0x2E ~ 0x46
        regIrPtr->IrTSetting = 0x2E1A8632;  // H = 130, L = 53, Half = 53, X = 0, d = 27
    #elif (IR_TIME_BASE == 30300)    // 30.3us
        regIrPtr->IrTimeBase = calIrTimeBase(MAIN_CRYSTAL_CLOCK);

        regIrPtr->IrLStart = 0x00727A30;    // X = 1, D = 32
        regIrPtr->IrLRepeat = 0x00727A0C;   // X = 1 , D = 32
        regIrPtr->IrLBit1 = 0x0014082D;     // X = 0, D = 20
        regIrPtr->IrLBit0 = 0x00140808;     // X = 0, D = 20
        //half range : 0x2E ~ 0x46
        regIrPtr->IrTSetting = 0x0A141E0A;  // H = 130, L = 53, Half = 53, X = 0, d = 27
    #else
        #error Not support this IR time base
    #endif

        irServicePtr = irGetKey;
#else   // Using RTC clock
        regIrPtr->IrTimeBase = 0xFF;
#endif
        break;

    case IR_SONY_SIRC_DECODER_BIT_TYPE:
    case IR_SONY_SIRC_DECODER_PACKET_TYPE:
        MP_ALERT("SONY SIRC %dbits type remote controllor -", irBitLength);

#ifdef __IR_CKS_USING_XIN
        regIrPtr->IrTimeBase = calIrTimeBase(MAIN_CRYSTAL_CLOCK);
#else
        regIrPtr->IrTimeBase = 0xFF;
#endif

#ifndef __ENABLE_IR_DEBUG_MODE__
        if (IR_SONY_SIRC_DECODER_BIT_TYPE)
            irReportStyle = 0;
        else
        {
            //irReportStyle = 1;
            irReportStyle = 3;
        }
#endif

        regIrPtr->IrCtrl = 0x00009000 | (irReportStyle << 9);
        irServicePtr = irGetKeySonySIRC;

#if (IR_TIME_BASE == 30300)     // 30.3us
        if (irReportStyle == 0)
        {
            sircStartPeriodMax = 83;
            sircStartPeriodMin = 74;
            sircOnePeriodMin = 24;
            sircStopPeriodMin = 84;
        }
        else
        {
            regIrPtr->IrLStart = (2 << 22) | (3 << 16) | (18 << 8) | 3;     // 3ms(2.4 + 0.6)
            regIrPtr->IrLRepeat = (0 << 22) | (0 << 16) | (1 << 8) | 1;   // 7ms(7 + 0)
            regIrPtr->IrLBit1 = (2 << 22) | (3 << 16) | (8 << 8) | 3;       // 1.8ms(1.2 + 0.6)
            regIrPtr->IrLBit0 = (2 << 22) | (3 << 16) | (3 << 8) | 3;       // 1.2ms(0.6 + 0.6)
            //half range : 0x2E ~ 0x46
            regIrPtr->IrTSetting = 0x0A141E0A;  // H = 130, L = 53, Half = 53, X = 0, d = 27
            regIrPtr->IrExpected = (regIrPtr->IrExpected & 0xFFFFFFE0) | (irBitLength - 2);
        }
#else
        //#error Not support this IR time base for Sony SIRC
#endif
        break;
    }

#if (CHIP_VER_MSB != CHIP_VER_615)
    regIrPtr->IrCtrlPin |= BIT1;        // IrCtrlPinPolarity is low active
#endif

    irIsrCallbackFunctionPtr = irNullCallbackFuc;
    repeatTimes = FIRST_IR_REPEAT_CODE_MASK_TIMES;
    dropRepeastCode = TRUE;
    irKeyCode = 0xFFFFFFFF;
    preRepeatCodeEnterTime = 0;
    preDataCodeEnterTime = 0;

    //regIrPtr->IrCtrl = 0x00009001 | BIT14 | IR_MASK_EVENT_COME;     // Enable remote mouse
    regIrPtr->IrCtrl = 0x00009001 | (irReportStyle << 9) | IR_MASK_EVENT_COME;  // Disable remote mouse
    SystemIntHandleRegister(IM_IR, irIsr);
    SystemIntEna(IM_IR);
}       //end of IR_Init



void IR_DeInit(void)
{
    IR *regIrPtr = (IR *) IR_BASE;
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD tmp;

    SystemIntDis(IM_IR);
    SystemIntHandleRelease(IM_IR);
    regIrPtr->IrCtrl &= ~BIT0;
    // Reset IR
    regBiuPtr->BiuArst &= ~ARST_IR;
    for (tmp = 0; tmp < 0x2000; tmp++);
    regClockPtr->MdClken &= ~CKE_IR;

    irIsrCallbackFunctionPtr = irNullCallbackFuc;
    repeatTimes = FIRST_IR_REPEAT_CODE_MASK_TIMES;
    dropRepeastCode = TRUE;
    irKeyCode = 0xFFFFFFFF;
}



void IR_PowerKeyCodeSet(DWORD powerKey)
{
#if (CHIP_VER_MSB == CHIP_VER_615)
    MP_ALERT("IR_PowerKeyCodeSet - Chip-%4X is not supported !!!", CHIP_VER >> 16);
#else   // MP650/660
    IR *regIrPtr = (IR *) IR_BASE;

    regIrPtr->IrExpectedData = powerKey;
#endif
}



void IR_PowerKeyWakeup(BOOL enable)
{
#if (CHIP_VER_MSB == CHIP_VER_615)
    MP_ALERT("IR_PowerKeyWakeup - Chip-%4X is not supported !!!", CHIP_VER >> 16);
#else   // MP650/660
    IR *regIrPtr = (IR *) IR_BASE;

    if (enable)
    {
        regIrPtr->IrCtrl |= BIT11;
        regIrPtr->IrCtrlPin |= BIT0 | BIT1;     // IrCrelPinEn enable, IrCtrlPinPolarity is low active
    }
    else
    {
        regIrPtr->IrCtrl &= ~BIT11;
        regIrPtr->IrCtrlPin &= ~BIT0;           // IrCrelPinEn disable
    }
#endif
}



static void irIsr(void)
{
    IR *regIrPtr = (IR *) IR_BASE;
    DWORD data;
    DWORD cause;

    data = regIrPtr->IrData;
    cause = regIrPtr->IrCtrl & 0x00FF0000;              // cause - bit16~23
    cause &= (regIrPtr->IrCtrl >> 8) & 0x00FF0000;      // mask - bit24~31
    regIrPtr->IrCtrl &= ~(IR_CAUSE_NOISE_COME | IR_CAUSE_EVENT_OVERWRITE | IR_CAUSE_EVENT_COME);

    if (cause & IR_CAUSE_EVENT_COME)
    {
#ifdef __ENABLE_IR_DEBUG_MODE__
        static BYTE length = 0;
        static WORD irdata[66];
        static BYTE irLevel[66];

        if (irReportStyle == 0)
        {
            data = regIrPtr->IrOthers;
            irdata[length] = (data >> 16) & 0x7FFF;
            irLevel[length] = (data >> 15) & BIT0;
            length++;

            if (length > (irBitLength << 1))
            {
                for (length = 0; length <= (irBitLength << 1); length++)
                    MP_ALERT("Level-%01d Duty-%05d", irLevel[length], irdata[length]);

                length = 0;
                MP_ALERT("");
            }
        }
        else if (irReportStyle == 1)
        {
            data = regIrPtr->IrStatus & 0x1F;

            switch (data)
            {
            case 0x10:
                irdata[length] = 0;
                length++;
                break;

            case 0x11:
                irdata[length] = 1;
                length++;
                break;

            default:
                length = 1;
                irdata[0] = data;
                break;
            }

            if (length > irBitLength)
            {
                mpDebugPrintN("%02d ", irdata[0]);
                data = 0;

                for (length = 0; length <= irBitLength; length++)
                {
                    data <<= 1;
                    data |= irdata[length];
                }

                mpDebugPrintN("0x%08X ", data);
                length = 0;
                MP_ALERT("");
            }
        }
        else if (irReportStyle == 3)
        {
            MP_ALERT("IR Status = %d", regIrPtr->IrStatus & 0x1F);
            MP_ALERT("IR Data = 0x%08X", data);
        }
#else
        irServicePtr(data);
#endif
    }

    if (cause & IR_CAUSE_EVENT_OVERWRITE)
    {
        MP_ALERT("IR Event Overwrite !!!");
    }

    if (cause & IR_CAUSE_NOISE_COME)
    {
        MP_ALERT("IR Noise Come !!!");
    }
}



void IR_RegisterCallBackFunc(void (*irIsrCallBackFunc)(DWORD, BYTE))
{
    if (irIsrCallBackFunc)
        irIsrCallbackFunctionPtr = irIsrCallBackFunc;
    else
        irIsrCallbackFunctionPtr = irNullCallbackFuc;
}



void IR_KeyTabSet(ST_IR_KEY *irKeyTabPtr, WORD keyNum)
{
    if (!irKeyTabPtr || (keyNum == 0))
    {
        irKeyTablePtr = 0;
        irKeyTypeNum = 0;
        irCustomerCode = 0xFFFF;

        MP_ALERT("--E-- IR_KeyTabSet Fail !!!");

        return;
    }

    irKeyTablePtr = irKeyTabPtr;
    irKeyTypeNum = keyNum;
    irCustomerCode = (irKeyTabPtr->dwIrData >> 16);

    MP_DEBUG("IR Table = 0x%08X", (DWORD) irKeyTablePtr);
    MP_DEBUG("IR Table Num = %d", irKeyTypeNum);
    MP_DEBUG("Customer Code = 0x%04X", irCustomerCode);
}



void IR_KeyPause(void)
{
    IR *regIrPtr = (IR *) IR_BASE;

    regIrPtr->IrCtrl &= ~BIT0;
}



void IR_KeyResume(void)
{
    IR *regIrPtr = (IR *) IR_BASE;

    regIrPtr->IrCtrl |= BIT0;
}



void IR_IntCauseClear(void)
{
    IR *regIrPtr = (IR *) IR_BASE;

    regIrPtr->IrCtrl &= ~(IR_CAUSE_NOISE_COME | IR_CAUSE_EVENT_OVERWRITE | IR_CAUSE_EVENT_COME);
}



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(IR_Init);
MPX_KMODAPI_SET(IR_DeInit);
MPX_KMODAPI_SET(IR_RegisterCallBackFunc);
MPX_KMODAPI_SET(IR_KeyPause);
MPX_KMODAPI_SET(IR_KeyResume);
MPX_KMODAPI_SET(IR_PowerKeyCodeSet);
MPX_KMODAPI_SET(IR_PowerKeyWakeup);
#endif

#endif      // #if (IR_ENABLE == 1)

