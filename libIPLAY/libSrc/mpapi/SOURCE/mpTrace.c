
/*
// Include section
*/
#include "iplaysysconfig.h"
#include "mpTrace.h"
#include "bios.h"
#include "display.h"

static BYTE g_iDebugString = 0;

//---------------------------------------------------------------------------
void mpDebugPrintN(const char *fmt, ...)
{
    int count;
    char cMsg[256];
    va_list args;

    va_start(args, fmt);                        /* get variable arg list address */
    count = vsnprintf(cMsg, 255, fmt, args);    /* process fmt & args into buf */
    cMsg[255] = 0;
    va_end(args);

    if (IsGdbConnect())
    {
        IntDisable();
        DebugPrintString(cMsg);
        IntEnable();

        return;
    }

#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc(Api_UsbdGetWhichOtgConnectedPc()) == TRUE)
    {
        count = 0;

        while (cMsg[count])
        {
            UsbPutChar(cMsg[count], Api_UsbdGetWhichOtgConnectedPc());
            count++;
        }
    }

    return;
#endif

    UartOutText((BYTE *) cMsg);
}



//---------------------------------------------------------------------------
void mpDebugPrint(const char *fmt, ...)
{
    int count;
    char cMsg[260];
    va_list args;

#if Make_DIAGNOSTIC_TC
	if(!ChkHandlerEnable())
		return;
#endif

    va_start(args, fmt);                        /* get variable arg list address */
    count = vsnprintf(cMsg, 255, fmt, args);    /* process fmt & args into buf */
    cMsg[count] = '\r';
    cMsg[count + 1] = '\n';
    cMsg[count + 2] = 0;
    cMsg[259] = 0;
    va_end(args);

    if (IsGdbConnect())
    {
        IntDisable();
        DebugPrintString(cMsg);
        IntEnable();

        return;
    }

#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc(Api_UsbdGetWhichOtgConnectedPc()) == TRUE)
    {
        count = 0;

        while (cMsg[count])
        {
            UsbPutChar(cMsg[count], Api_UsbdGetWhichOtgConnectedPc());
            count++;
        }
    }

    return;
#endif

    UartOutText((BYTE *) cMsg);
}




//---------------------------------------------------------------------------
void mpDebugPrintChar(const char data)
{
    if (IsGdbConnect())
    {
        BYTE charData[2] = {0};

        charData[0] = data;
        IntDisable();
        DebugPrintString(charData);
        IntEnable();

        return;
    }

#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc(Api_UsbdGetWhichOtgConnectedPc()) == TRUE)
    {
        UsbPutChar(data, Api_UsbdGetWhichOtgConnectedPc());
    }

    return;
#endif

    PutUartChar(data);
}



//---------------------------------------------------------------------------
void mpDebugPrintValue(DWORD value, BYTE length)
{
    BYTE buf[12];
    WORD bufIndex = 0;
    BYTE byte;

    if (length > 8)
        length = 8;

    value = value << ((8 - length) << 2);

    while (length)
    {
        byte = (value & 0xF0000000) >> 28;
        byte += '0';

        if (byte > '9')
            byte += 7;

        buf[bufIndex] = byte;
        bufIndex++;
        value <<= 4;
        length--;
    }

    buf[bufIndex] = 'H';
    bufIndex++;
    buf[bufIndex] = 0;

    if (IsGdbConnect())
    {
        IntDisable();
        DebugPrintString(buf);
        IntEnable();

        return;
    }

#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc(Api_UsbdGetWhichOtgConnectedPc()) == TRUE)
    {
        BYTE count = 0;

        while (buf[count])
        {
            UsbPutChar(buf[count], Api_UsbdGetWhichOtgConnectedPc());
            count++;
        }
    }

    return;
#endif

    UartOutText((BYTE *) &buf);
}



//---------------------------------------------------------------------------
void mpDebugPrintCharDrop(const char data)
{
    if (IsGdbConnect())
    {
        BYTE charData[2] = {0};

        charData[0] = data;
        IntDisable();
        DebugPrintString(charData);
        IntEnable();

        return;
    }

#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc(Api_UsbdGetWhichOtgConnectedPc()) == TRUE)
    {
        UsbPutChar(data, Api_UsbdGetWhichOtgConnectedPc());
    }

    return;
#endif

    PutUartCharDrop(data);
}



//---------------------------------------------------------------------------
//static WORD msgY;
static BYTE g_iMsgString = 0;

// only for isp or message really want show on screen, like text mode
void mpPrintMessage(const char *cMsg)
{
#if BMP_FONT_ENABLE
#if 1
    ST_IMGWIN *pWin;

    pWin = Idu_GetCurrWin();

    WORD x = 80;

    if (pWin != NULL)
    {
        int y;

        y = g_iMsgString * 24 + 48;

        if (y > pWin->wHeight - 48)
        {
            g_iMsgString = 0;
            y = 48;
            mpPaintWin(pWin, RGB2YUV(9, 53, 88));
        }

        //Idu_PrintDebugString(pWin, (BYTE *) cMsg, x, y);
		Idu_PrintStringWithSize(pWin, (BYTE *) cMsg, x, y, 0, 0, 0, 0);
        g_iMsgString++;
    }
#else
    static int iMsgString;
    register ST_OSDWIN *OSDWin;

    OSDWin = Idu_GetOsdWin();

    if (OSDWin != NULL)
    {
        if (iMsgString * 24 + 60 > OSDWin->wHeight)
        {
            iMsgString = 0;

            Idu_OsdErase();
        }

        //Idu_PrintDebugString(Idu_GetCurrWin(),(BYTE *)cMsg, 80,iMsgString * 24);
        Idu_OsdPutDebugStr(OSDWin, (BYTE *) cMsg, 80, iMsgString * 24);
        iMsgString++;
    }
#endif

    mpDebugPrint("%s", (BYTE *) cMsg);
#endif
}


//---------------------------------------------------------------------------
#if MP_TRACE_TIME_ENABLE
static DWORD g_dwMpTraceTime = 0;
#endif

void mpTraceTimeStart()
{
#if MP_TRACE_TIME_ENABLE
    g_dwMpTraceTime = GetSysTime();
#endif
}



//---------------------------------------------------------------------------
// iDelayTime indicate how long the message will keep on screen
//---------------------------------------------------------------------------
DWORD mpPrintTraceTime(char *cMsg, int iDelayTime)
{
#if MP_TRACE_TIME_ENABLE
    DWORD t = SystemGetElapsedTime(g_dwMpTraceTime);

    mpDebugPrint("%s %d", cMsg, t);

    if (iDelayTime > 0)
        TimerDelay(iDelayTime);

    g_dwMpTraceTime = GetSysTime();

    return t;
#else
    return 0;
#endif
}



//****************************************************************************
//
// For uart debug port
//
//****************************************************************************
void Uart_Init(void)
{
#if 1//( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        HUartA_PinMultiplexerEnable(HUARTA_PIN_MULTIPLEXER);
        BreakHandlerIsrCallbackRegister(Uart1Isr);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        BreakHandlerIsrCallbackRegister(Uart2Isr);
	#else
		return;
    #endif

#if UART_TO_MCU
    HUartInit(DEBUG_COM_PORT, ENABLE, HUART_BAUD);
#else
    HUartInit(DEBUG_COM_PORT, DISABLE, HUART_BAUD);
#endif

#else
    BreakHandlerIsrCallbackRegister(UartIsr);

    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        LUart_Init();
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        HUartInit(ENABLE, UART_BAUD);
    #else
        return;
    #endif
#endif

    //agentUartIntInfoChange(DEBUG_COM_PORT);
}




BYTE GetUartChar(void)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    return HUartGetChar(DEBUG_COM_PORT);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        return LGetUartChar();
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        return (BYTE) HUartGetChar();
    #else
        return 0;
    #endif
#endif
}



void PutUartChar(BYTE data)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartPutChar(DEBUG_COM_PORT, data);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        LPutUartChar(data);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        HUartPutChar(data);
    #endif
#endif
}



void PutUartCharDrop(BYTE data)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartPutCharDrop(DEBUG_COM_PORT, data);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        LPutUartCharDrop(data);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        HUartPutCharDrop(data);
    #endif
#endif
}



void UartOutText(BYTE *buffer)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartOutText(DEBUG_COM_PORT, buffer);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        LUartOutText(buffer);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        HUartOutText(buffer);
    #endif
#endif
}



void UartOutValue(DWORD value, BYTE length)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartOutValueHex(DEBUG_COM_PORT, value, length);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        LUartOutValue(value, length);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        HUartOutValue(value, length);
    #endif
#endif
}



int WaitUartStatus(DWORD dwStatus)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartWaitStatus(DEBUG_COM_PORT, dwStatus, UART_WAIT_EMPTY_TIMEOUT);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        return LWaitUartStatus(dwStatus);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        return HUartWaitStatus(dwStatus, UART_WAIT_EMPTY_TIMEOUT);
    #else
        return FAIL;
    #endif
#endif
}



void WaitUartTxComplete(void)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartWaitTxComplete(DEBUG_COM_PORT);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        LWaitUartTxComplete();
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        HUartWaitTxComplete();
    #endif
#endif
}



SDWORD UartRead(BYTE *buffer, DWORD length)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    return HUartBufferRead(DEBUG_COM_PORT, buffer, length, RECV_THR_0, TRUE, DISABLE, 0xFF, DISABLE);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        return LUartRead(buffer, length);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        return HUartBufferRead(buffer, length, RECV_THR_0, TRUE);
    #else
        return FAIL;
    #endif
#endif
}



SDWORD UartWrite(BYTE *buffer, DWORD length)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    return HUartBufferWrite(DEBUG_COM_PORT, buffer, length, TRUE, DISABLE);
#else
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        return LUartWrite(buffer, length);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        return HUartBufferWrite(buffer, length, TRUE);
    #else
        return FAIL;
    #endif
#endif
}



SDWORD UartDMARead(BYTE *buffer, DWORD length)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    return HUartDmaRead(DEBUG_COM_PORT, buffer, length, length, TRUE, DISABLE, 0xFF, DISABLE);
#else
    #if (DEBUG_COM_PORT == HUART_B_INDEX)
        return HUartDmaRead(buffer, length, length, TRUE);
    #else
        return FAIL;
    #endif
#endif
}



SDWORD UartDMAWrite(BYTE *buffer, DWORD length)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    return HUartDmaWrite(DEBUG_COM_PORT, (BYTE *) buffer, length, TRUE, DISABLE);
#else
    #if (DEBUG_COM_PORT == HUART_B_INDEX)
        return HUartDmaWrite(buffer, length, TRUE);
    #else
        return FAIL;
    #endif
#endif
}



void UartIntEnable(DWORD mask)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartIntEnable(DEBUG_COM_PORT, mask);
#else
    UART *regUartPtr = (UART *) UART_BASE;

    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        mask &= 0xFFFF0000;
        regUartPtr->UartInt |= mask;
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        mask &= 0x0000FFFF;
        regUartPtr->HUartIntCtl = (regUartPtr->HUartIntCtl & 0x0000FFFF) | (mask << 16) | mask;
    #endif
#endif
}



void UartIntDisable(DWORD mask)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    HUartIntDisable(DEBUG_COM_PORT, mask);
#else
        UART *regUartPtr = (UART *) UART_BASE;

    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        mask &= 0xFFFF0000;
        regUartPtr->UartInt &= ~mask;
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        DWORD cause;

        mask &= 0x0000FFFF;
        cause = mask << 16;
        mask = ~mask;
        regUartPtr->HUartIntCtl = ((regUartPtr->HUartIntCtl & 0x0000FFFF) | cause) & mask;
    #endif
#endif
}



void EnaUartInt(void)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        SystemIntEna(IM_UART);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        SystemIntEna(IM_UART2);
    #endif
#else
    #if (DEBUG_COM_PORT != HUART_NONE_INDEX)
    SystemIntEna(IM_UART);
    #endif
#endif
}



void DisUartInt(void)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    #if (DEBUG_COM_PORT == HUART_A_INDEX)
        SystemIntDis(IM_UART);
    #elif (DEBUG_COM_PORT == HUART_B_INDEX)
        SystemIntDis(IM_UART2);
    #endif
#else
    #if (DEBUG_COM_PORT != HUART_NONE_INDEX)
    SystemIntDis(IM_UART);
    #endif
#endif
}



/**
 ***************************************************************
 *
 * Check the int value of UART to be dwStatus
 *
 *  Input  : Status value
 *
 *  Output : Status is right or not
 ***************************************************************
*/
int CheckUartStatus(DWORD dwStatus)
{
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
        return HUartWaitStatus(DEBUG_COM_PORT, dwStatus, 1);
#else
    #if (DEBUG_COM_PORT == HUART_B_INDEX)
        return CheckUartStatus(dwStatus);
    #else
        return CheckHUartStatus(dwStatus);
    #endif
#endif
}



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(UartOutText);
MPX_KMODAPI_SET(UartOutValue);
MPX_KMODAPI_SET(Uart_Init);
//MPX_KMODAPI_SET(UartRead);
//MPX_KMODAPI_SET(UartDMARead);
//MPX_KMODAPI_SET(UartWrite);
//MPX_KMODAPI_SET(UartDMAWrite);
//MPX_KMODAPI_SET(UartIntEnable);
//MPX_KMODAPI_SET(UartIntDisable);
MPX_KMODAPI_SET(GetUartChar);
MPX_KMODAPI_SET(PutUartChar);

MPX_KMODAPI_SET(mpDebugPrint);
#endif

