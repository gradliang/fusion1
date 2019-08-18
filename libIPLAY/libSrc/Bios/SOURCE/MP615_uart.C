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
* Filename      : MP615_uart.c
* Programmer(s) : TY Miao
* Created       : abel yeh
* Descriptions  : UART module for MP615
*******************************************************************************
*/


/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "iplaysysconfig.h"
#include "UtilRegFile.h"
#include "bios.h"
#include "peripheral.h"

#if (CHIP_VER_MSB == CHIP_VER_615)


#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif /* min */

#ifndef UART_TUNE
#define UART_TUNE   DISABLE
#endif

static BOOL uartInitialed = FALSE;
static UART_ISR_CALLBACK uartISR_CallbackFunPtr = 0;
static UART_ISR_CALLBACK huartISR_CallbackFunPtr = 0;

static void uartWaitDelay(DWORD wCount)
{
    DWORD i, j;

    for (i = 0; i < wCount; i++)
    {
        for (j = 0; j < 0x800; j++);
    }
}



//////////////////////////////////////////////////////////////////
//
//  UART Part
//
//////////////////////////////////////////////////////////////////
void UartRegisterCallBackFunc(UART_ISR_CALLBACK funPtr)
{
    uartISR_CallbackFunPtr = funPtr;
}



void UartClearCallBackFunc(DWORD index)
{
    uartISR_CallbackFunPtr = 0;
}



//                     96M / 16                96M / 16
// Target Baud Rate = ----------  => X = ------------------ - 1
//                      X + 1             Target Baud Rate
//
// UART_CLOCK = 0x00020000 + PLL1_FREQ / (16 * UART_BAUD) - 1;
//
void LUart_Init(void)
{
    UART *regUartPtr = (UART *) UART_BASE;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    volatile DWORD tmp;

    if (!IsGdbConnect())
         IntDisable();

    if (!uartInitialed)
    {
        regBiuPtr->BiuArst &= ~BIT6;
        regClockPtr->MdClken &= ~BIT14;         // disable uart clock
        TimerDelay(100);                        // Waiting for UART stable
    }

    regClockPtr->Aux0ClkC = BIT17 + Clock_PllFreqGet(CLOCK_PLL1_INDEX) / (UART_BAUD << 4) - 1;
    regClockPtr->MdClken |= BIT14;      // enable uart clock
    regBiuPtr->BiuArst |= BIT6;
    regUartPtr->UartC = BIT0;           // Enable, LSB first, normal data polarity, 0 byte Rx threshold
    regUartPtr->UartInt = 0;            // disable all interrupt

    while (regUartPtr->UartFifoCnt & 0x003F0000)
        tmp = regUartPtr->UartData;

    regUartPtr->UartInt = M_RX_READY | M_RX_FIFO_THRESHOLD;   // enable received data being ready

    uartInitialed = TRUE;

    if (!IsGdbConnect())
         IntEnable();
    else
        return;

    // connect the output pings with UART
    // IntDisabel/IntEnable will affect BreakHandler
    Gpio_ConfiguraionSet(GPIO_UGPIO_0, GPIO_ALT_FUNC_1, GPIO_OUTPUT_MODE, 0x03, 2);

    SystemIntHandleRegister(IM_UART, UartIsr);
}



/**
 ***************************************************************
 *
 * Wait the int value of UART to be dwStatus
 *
 *  Input  : Status value
 *
 *  Output : Status is right or not
 ***************************************************************
*/
int LWaitUartStatus(DWORD dwStatus)
{
    UART *regUartPtr = (UART *) UART_BASE;
    DWORD dwTimeOutCount = 0x100000;

    while (dwTimeOutCount--)
    {
        if ((regUartPtr->UartInt & dwStatus) == dwStatus)
            return PASS;

//        if (regUartPtr->UartInt & RX_ERROR_MASK)
//            return FAIL;
    }

    return FAIL;
}



/**
 ***************************************************************
 *
 * read a character from UART, if no data received, function will not return
 * this function can only be called by debug agent and has no sycronization control
 *
 *  Input  : none.
 *
 *  Output : one byte data.
 ***************************************************************
*/
BYTE LGetUartChar(void)
{
    UART *regUartPtr = (UART *) UART_BASE;

    LWaitUartStatus(C_RX_READY);	// wait until data ready for rx

    return (BYTE) regUartPtr->UartData;
}



/**
 ***************************************************************
 *
 * write a character to UART, if output FIFO full then wait
 * this function can only be called by debug agent and has no sycronization control
 *
 *  Input  : one byte data to put.
 *
 *  Output : none.
 ***************************************************************
*/
void LPutUartChar(BYTE data)
{
    UART *regUartPtr = (UART *) UART_BASE;

    if (!LWaitUartStatus(C_TX_READY) == FAIL) return;	// wait until tx fifo ready
    regUartPtr->UartData = data;
}



void LPutUartCharDrop(BYTE data)
{
    UART *regUartPtr = (UART *) UART_BASE;

    regUartPtr->UartData = data;
}



/**
 ***************************************************************
 *
 * wait until the UART TX FIFO empty, uaually called before
 * changing system clock or baud rate
 *
 *  Input  : none.
 *
 *  Output : none.
 ***************************************************************
*/
void LWaitUartTxComplete(void)
{
    register UART *regUartPtr = (UART *) UART_BASE;
    register DWORD counter;

    LWaitUartStatus(C_TX_EMPTY);   // wait until tx fifo empty

    counter = 100000;
    while (counter)
        counter--;              // wait until the tx activity finish
}



//////////////////////////////////////////////////////////////////////////

// move the data from UART to specified buffer for up to 4G bytes length
SDWORD LUartRead(BYTE *buffer, DWORD length)
{
    register UART *regUartPtr = (UART *) (UART_BASE);

    while (length)
    {
        if (LWaitUartStatus(C_RX_READY) == FAIL)
            return FAIL;

        *buffer = regUartPtr->UartData;
        buffer++;
        length--;
    }

    return PASS;
}



SDWORD LUartWrite(BYTE *buffer, DWORD length)
{
    register UART *regUartPtr = (UART *) (UART_BASE);

    while (length)
    {
        if (LWaitUartStatus(C_TX_READY) == FAIL)
            return FAIL;

        regUartPtr->UartData = *buffer;
        buffer++;
        length--;
    }

    return PASS;
}



void LUartOutText(BYTE *buffer)
{
    register UART *regUartPtr = (UART *) UART_BASE;

    while (*buffer)
    {
        if (LWaitUartStatus(C_TX_READY) == FAIL)
            return;     // wait until tx fifo ready

        regUartPtr->UartData = *buffer;
        buffer++;
    }
}



void LUartOutValue(register DWORD value, register BYTE length)
{
    BYTE buf[12];
    WORD bufIndex = 0;
    BYTE byte;

    if (length > 8)
        length = 8;

    buf[0] = '0';
    buf[1] = 'x';
    value = value << ((8 - length) << 2);

    bufIndex = 2;
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

    buf[bufIndex] = 0;

    return LUartOutText((BYTE *) &buf);
}



int CheckHUartStatus(DWORD dwStatus)
{
    UART *regUartPtr = (UART *) UART_BASE;

    if (regUartPtr->HUartIntCtl & dwStatus)
        return PASS;
    else
        return FAIL;
}



//////////////////////////////////////////////////////////////////
//
//  HUART Part
//
//////////////////////////////////////////////////////////////////

void HUartRegisterCallBackFunc(UART_ISR_CALLBACK funPtr)
{
    huartISR_CallbackFunPtr = funPtr;
}



void HUartClearCallBackFunc(DWORD index)
{
    huartISR_CallbackFunPtr = 0;
}



void HUartIntEnable(DWORD mask)
{
    UART *regUartPtr = (UART *) UART_BASE;

    mask &= 0x0000FFFF;
    regUartPtr->HUartIntCtl = (regUartPtr->HUartIntCtl & 0x0000FFFF) | (mask << 16) | mask;
}



void HUartIntDisable(DWORD mask)
{
    UART *regUartPtr = (UART *) UART_BASE;
    DWORD cause;

    mask &= 0x0000FFFF;
    cause = mask << 16;
    mask = ~mask;
    regUartPtr->HUartIntCtl = ((regUartPtr->HUartIntCtl & 0x0000FFFF) | cause) & mask;
}



//                     96M/8                96M/8
// Target Baud Rate = --------  => X = ------------------ - 1
//                      X + 1           Target Baud Rate
//
// UART_CLOCK    =   0x00020000 + PLL1_FREQ / (UART_BAUD * 4) - 1;
//
void HUartInit(BOOL rxEnable, DWORD baudRate)
{
    CLOCK *clock = (CLOCK *) CLOCK_BASE;
    UART *regUartPtr = (UART *) UART_BASE;
    BIU *biu = (BIU *) BIU_BASE;
    DWORD tmp;

    if (!IsGdbConnect())
        IntDisable();

    if (!uartInitialed)
    {
        biu->BiuArst &= ~BIT6;
        for (tmp = 0; tmp < 0x100; tmp++);           //delay
        biu->BiuArst |= BIT6;
    }

    clock->UartH_C = BIT17 + Clock_PllFreqGet(CLOCK_PLL1_INDEX) / baudRate / 8 - 1;
    clock->MdClken |= BIT30;                            // enable height speed uart clock
    regUartPtr->HUartC = BIT24 | BIT8 | BIT0;           // Enable HUART, LSB first, positive data polarity, clock divider 8
    regUartPtr->HUartIntCtl = 0;                        // disanle all interrupt
    regUartPtr->HUartBufCtl = 0x00000000;               // threshold Rx: >0Byte, Tx: <4Byte

    if (rxEnable == ENABLE)
    {
        regUartPtr->HUartIntCtl |= M_RXTHR_HIT;         // Enable UART Rx interrupt
    }

    uartWaitDelay(20);
    uartInitialed = TRUE;

    if (!IsGdbConnect())
        IntEnable();
    else
        return;

    // connect the output pings with HUART
    Gpio_ConfiguraionSet(GPIO_UGPIO_0, GPIO_ALT_FUNC_1, GPIO_OUTPUT_MODE, 0x03, 2);
    Gpio_ConfiguraionSet(GPIO_GPIO_4, GPIO_ALT_FUNC_1, GPIO_OUTPUT_MODE, 0x0F, 4);

    SystemIntHandleRegister(IM_UART, UartIsr);
}



SDWORD HUartBusAttrChange(BOOL busDir, BOOL busPol)
{
    UART *regUartPtr = (UART *) UART_BASE;

    regUartPtr->HUartC &= ~(HUART_BUS_POL | HUART_BUS_DIR);
    regUartPtr->HUartC |= (busPol << 16) | (busDir << 8);

    return FAIL;
}



SDWORD HUartWaitStatus(DWORD dwStatus, DWORD timeOut)
{
    DWORD dwCount = timeOut;
    UART *regUartPtr = (UART *) UART_BASE;
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    DWORD orgPattern = isr->MiMask & IM_UART;
    DWORD orgIntMask = regUartPtr->HUartIntCtl & 0x0000FFFF;

    dwStatus &= 0xFFFF0000;

    if (!dwStatus)
        return PASS;            // No any cuase bit to be waiting.

    isr->MiMask &= ~IM_UART;
    regUartPtr->HUartIntCtl |= dwStatus >> 16;

    if (!timeOut)
    {
        while (!(regUartPtr->HUartIntCtl & dwStatus));
        regUartPtr->HUartIntCtl = dwStatus | orgIntMask;
        isr->MiMask |= orgPattern;

        return PASS;
    }
    else
    {
        while (dwCount)
        {
            if (regUartPtr->HUartIntCtl & dwStatus)
            {
                regUartPtr->HUartIntCtl = dwStatus | orgIntMask;
                isr->MiMask |= orgPattern;

                return PASS;
            }

            dwCount--;
        }
    }

    regUartPtr->HUartIntCtl = orgIntMask;
    isr->MiMask |= orgPattern;

    return FAIL;
}



void HUartWaitTxComplete(void)
{
    DWORD counter;

    HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT);

    counter = 100000;
    while (counter)
        counter--;                              // wait until the tx activity finish
}



SDWORD HUartBufferRead(BYTE *buffer, DWORD length, BYTE recvThr,
                       BOOL isPolling)
{
    UART *regUartPtr = (UART *) UART_BASE;

    if (recvThr > RECV_THR_24)
        recvThr = RECV_THR_24;

    regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;
    regUartPtr->HUartBufCtl = (regUartPtr->HUartBufCtl & ~RECV_THR_FIELD) | (((DWORD) recvThr) << RECV_THR_SHIFT);

    if (isPolling)
    {
        while (length)
        {
            if (HUartWaitStatus(C_RXTHR_HIT, UART_WAIT_RXTH_TIMEOUT) == FAIL)
            {
                return FAIL;
            }

            *buffer = (BYTE) regUartPtr->HUartRdBuf;
            buffer++;
            length--;
        }
    }

    return PASS;
}



SDWORD HUartDmaRead(BYTE *buffer, DWORD bufLength, DWORD dmaThr,
                    BOOL isPolling)
{
    DWORD status = PASS;
    UART *regUartPtr = (UART *) UART_BASE;

    if (((DWORD) buffer) & 0x0F)
    {
        mpDebugPrint("--E-- HUART DMA read buffer must be align to 4-DWORD boundary !!!\r\n");

        return -2;
    }

    if (bufLength & 0x0F)
    {
        mpDebugPrint("--W-- HUART DMA read buffer length must be multiplex of 16 !!!\r\n");

        return -3;
    }

    if (!isPolling)
        HandlerEnable(DISABLE);

    regUartPtr->HUartIntCtl &= ~M_RXDMA_DONE;
    regUartPtr->HUartDmaLen = (regUartPtr->HUartDmaLen & 0xFFFF0000) | (dmaThr & 0x0000FFFF);
    regUartPtr->HUartDmaSA = (DWORD) buffer;
    regUartPtr->HUartDmaEA = (DWORD) (buffer + bufLength);
    regUartPtr->HUartBufCtl |= RECV_FROM_DMA;

    if (isPolling)
    {
        status = HUartWaitStatus(M_RXDMA_DONE, UART_WAIT_RXTH_TIMEOUT);
        regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;
    }
    else
    {
        regUartPtr->HUartIntCtl |= M_RXDMA_DONE;
    }

    return PASS;
}



SDWORD HUartBufferWrite(BYTE *buffer, DWORD length, BOOL isPolling)
{
    UART *regUartPtr = (UART *) UART_BASE;

    if (HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
        return FAIL;

    regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;

    if (isPolling)
    {
        while (length)
        {
            if (HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
                return FAIL;

            regUartPtr->HUartWrBuf = *buffer;
            buffer++;
            length--;
        }
    }

    return PASS;
}



SDWORD HUartDmaWrite(BYTE *buffer, DWORD length, BOOL isPolling)
{
    UART *regUartPtr = (UART *) UART_BASE;
    SDWORD status = PASS;

    if (!length)
        return -2;

    if (HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
        return -3;

    if (!isPolling)
        HandlerEnable(DISABLE);

    regUartPtr->HUartIntCtl &= ~M_TXDMA_DONE;
    regUartPtr->HUartDmaLen &= 0x0000FFFF;
    regUartPtr->HUartDmaLen |= length << 16;        // unit is bytes
    regUartPtr->HUartDmaSA = (DWORD) buffer;
    regUartPtr->HUartDmaEA = (DWORD) (buffer + length - 1);
    regUartPtr->HUartBufCtl |= TRAN_TO_DMA;

    if (isPolling)
    {
        status = HUartWaitStatus(C_TXDMA_DONE, UART_WAIT_EMPTY_TIMEOUT);
        regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;
    }
    else
    {
        regUartPtr->HUartIntCtl |= M_TXDMA_DONE;
    }

    return status;
}



SDWORD HUartOutText(BYTE *buffer)
{
    UART *regUartPtr = (UART *) UART_BASE;

    regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;

    while (*buffer)
    {
        if (HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
            return FAIL;

        regUartPtr->HUartWrBuf = *buffer;
        buffer++;
    }

    return PASS;
}



SDWORD HUartOutValue(DWORD value, BYTE length)
{
    BYTE buf[12];
    WORD bufIndex = 0;
    BYTE byte;

    if (length > 8)
        length = 8;

    buf[0] = '0';
    buf[1] = 'x';
    value = value << ((8 - length) << 2);

    bufIndex = 2;
    while (length)
    {
        byte = (value & 0xF0000000) >> 28;
        byte += '0';

        if (byte > '9')
            byte += 7;

        buf[bufIndex] = byte;
        bufIndex++;
        value = value << 4;
        length--;
    }

    buf[bufIndex] = 0;

    HUartOutText(buf);
}



SDWORD HUartGetChar(void)
{
    UART *regUartPtr = (UART *) UART_BASE;

    regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;
    regUartPtr->HUartBufCtl &= ~RECV_THR_FIELD;

    if (HUartWaitStatus(C_RXTHR_HIT, UART_WAIT_RXTH_TIMEOUT) == PASS)
    {
        return regUartPtr->HUartRdBuf;
    }
    else
    {
        return -1;
    }
}



SDWORD HUartPutChar(BYTE data)
{
    UART *regUartPtr = (UART *) UART_BASE;

    regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;

    if (HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == PASS)
    {
        regUartPtr->HUartWrBuf = (DWORD) data;

        return PASS;
    }

    return FAIL;
}



SDWORD HUartPutCharDrop(BYTE data)
{
    UART *regUartPtr = (UART *) UART_BASE;

    regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;

    if (HUartWaitStatus(C_TXBUF_EMP, 1) == PASS)
    {
        regUartPtr->HUartWrBuf = (DWORD) data;

        return PASS;
    }

    return FAIL;
}



static void uart1Isr(void)
{
    UART *regUartPtr = (UART *) UART_BASE;
    volatile DWORD cause;

    cause = regUartPtr->UartInt & 0x0000FFFF;
    cause &= (regUartPtr->UartInt >> 16);

    if (!cause)
        return;

    //PutUartChar('A');
/*
#if (DEBUG_COM_PORT == HUART_A_INDEX)
    #if (UART_TUNE == ENABLE)
        if (cause & C_RX_FIFO_THRESHOLD)
            PC_UartRxISR();
    #endif
#endif
*/
    if (uartISR_CallbackFunPtr)
        uartISR_CallbackFunPtr(cause);
    else
    {
        if (cause & C_RX_READY)
        {
            while (LWaitUartStatus(C_RX_READY) == TRUE)
                regUartPtr->UartData;
        }

        cause &= ~C_RX_READY;
        regUartPtr->UartInt &= ~(cause << 16);
    }
}



static void huartIsr(void)
{
    UART *regUartPtr = (UART *) UART_BASE;
    DWORD cause;

    cause = regUartPtr->HUartIntCtl & 0xFFFF0000;
    cause &= regUartPtr->HUartIntCtl << 16;
    //mpDebugPrint("Hint = 0x%x", regUartPtr->HUartIntCtl);
    if (!cause)
        return;

    //PutUartChar('B');
/*
#if (DEBUG_COM_PORT == HUART_B_INDEX)
    #if (UART_TUNE == ENABLE)
        if (cause & C_RXTHR_HIT)
            PC_UartRxISR();
    #endif
#endif
*/
    if (huartISR_CallbackFunPtr)
        huartISR_CallbackFunPtr(cause);
    else
    {
        if (cause & C_RXTHR_HIT)
        {
            while (HUartWaitStatus(C_RXTHR_HIT, UART_WAIT_RXTH_TIMEOUT) == TRUE)
                regUartPtr->HUartRdBuf;
        }

        cause &= ~C_RXTHR_HIT;
        regUartPtr->HUartIntCtl &= ~(cause >> 16);
    }
}



void UartIsr(void)
{
    uart1Isr();
    huartIsr();
}


////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
//MPX_KMODAPI_SET(UartOutText);
//MPX_KMODAPI_SET(PutUartChar);
#endif

#endif

