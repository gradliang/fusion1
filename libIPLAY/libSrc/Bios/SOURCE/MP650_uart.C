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
* Filename      : MP650_uart.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : UART module for MP650
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

#include "iplaysysconfig.h"
#include "UtilRegFile.h"
#include "bios.h"
#include "peripheral.h"
#include "uart.h"

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )

static UART_ISR_CALLBACK Uart1ISR_CallbackFunPtr = 0;
static UART_ISR_CALLBACK Uart2ISR_CallbackFunPtr = 0;
static BOOL huartAPinMultiplexerEnable = FALSE;

static void uartWaitDelay(DWORD wCount)
{
    DWORD i, j;

    for (i = 0; i < wCount; i++)
    {
        for (j = 0; j < 0x800; j++);
    }
}



/*
void huartCleanCause(DWORD index, DWORD cause)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return;

    cause &= 0x003F0000;
    regUartPtr->HUartIntCtl = (regUartPtr->HUartIntCtl & 0x0000FFFF) | cause;
}
*/



static void huartReset(DWORD index)
{
    BIU *regBiuPtr = (BIU *) BIU_BASE;
    DWORD bitFiled;
    DWORD tmp;

    if (index == HUART_A_INDEX)
        bitFiled = ARST_UARTA;
    else if (index == HUART_B_INDEX)
        bitFiled = ARST_UARTB;
    else
        return;

    // Reset UART
    regBiuPtr->BiuArst |= bitFiled;
    for (tmp = 0; tmp < 0x100; tmp++);
    regBiuPtr->BiuArst &= ~bitFiled;
    for (tmp = 0; tmp < 0x100; tmp++);
    regBiuPtr->BiuArst |= bitFiled;
}



void Uart1Isr(void)
{
    HUART *regUartPtr = (HUART *) HUART1_BASE;
    DWORD cause = regUartPtr->HUartIntCtl;

    cause &= cause << 16;
/*
#if (DEBUG_COM_PORT == HUART_A_INDEX)
    #if (UART_TUNE == ENABLE)
        if (cause & (C_RXTHR_HIT | C_RXDMA_DONE))
            PC_UartRxISR();
    #endif
#endif
*/
    if (Uart1ISR_CallbackFunPtr)
        Uart1ISR_CallbackFunPtr(cause);
    else
        regUartPtr->HUartIntCtl &= ~(cause >> 16);              // clean mask bit when ISR was not registered

    regUartPtr->HUartIntCtl |= regUartPtr->HUartIntCtl << 16;   // clean cause
}



void Uart2Isr(void)
{
    HUART *regUartPtr = (HUART *) HUART2_BASE;
    DWORD cause = regUartPtr->HUartIntCtl;

    cause &= cause << 16;
/*
#if (DEBUG_COM_PORT == HUART_B_INDEX)
    #if (UART_TUNE == ENABLE)
        if (cause & (C_RXTHR_HIT | C_RXDMA_DONE))
            PC_UartRxISR();
    #endif
#endif
*/
    if (Uart2ISR_CallbackFunPtr)
        Uart2ISR_CallbackFunPtr(cause);
    else
        regUartPtr->HUartIntCtl &= ~(cause >> 16);              // clean mask bit when ISR was not registered

    regUartPtr->HUartIntCtl |= regUartPtr->HUartIntCtl << 16;   // clean cause
}



//****************************************************************************
//
//
//
//****************************************************************************

void HUartRegisterCallBackFunc(DWORD index, UART_ISR_CALLBACK funPtr)
{
    if (index == HUART_A_INDEX)
        Uart1ISR_CallbackFunPtr = funPtr;
    else if (index == HUART_B_INDEX)
        Uart2ISR_CallbackFunPtr = funPtr;
}



void HUartClearCallBackFunc(DWORD index)
{
    if (index == HUART_A_INDEX)
        Uart1ISR_CallbackFunPtr = 0;
    else if (index == HUART_B_INDEX)
        Uart2ISR_CallbackFunPtr = 0;
}



void HUartIntEnable(DWORD index, DWORD mask)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return;

    mask &= 0x0000003F;
    regUartPtr->HUartIntCtl = (regUartPtr->HUartIntCtl & 0x0000FFFF) | (mask << 16) | mask;
}



void HUartIntDisable(DWORD index, DWORD mask)
{
    DWORD cause = mask << 16;
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return;

    mask = ~mask;
    mask &= 0x0000003F;
    regUartPtr->HUartIntCtl = ((regUartPtr->HUartIntCtl & 0x0000FFFF) | (cause & 0x003F0000)) & mask;
}



static BYTE huartA_Cks = CLOCK_PLL2_INDEX, huartB_Cks = CLOCK_PLL2_INDEX;

void HUartClkSourceSel(DWORD index, E_PLL_INDEX clkSel)
{
    if (index == HUART_A_INDEX)
        huartA_Cks = clkSel;
    else if (index == HUART_B_INDEX)
        huartB_Cks = clkSel;
    else
        return;
}



//                     PLL / 8               PLL / 8
// Target Baud Rate = ---------  => X = ------------------ - 1
//                      X + 1            Target Baud Rate
//
//UART_CLOCK    =   0x00020000 + PLL1_FREQ / (8 * UART_BAUD) - 1;
//
void HUartInit(DWORD index, BOOL rxEnable, DWORD baudRate)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    HUART *regUartPtr;
    DWORD bitField = CKE_UARTA;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return;

    if (!IsGdbConnect())
        IntDisable();

    if (index == HUART_A_INDEX)
    {   // Enable HUARTA clock divider
        regClockPtr->Clkss_EXT1 &= ~BIT28;

        if (huartA_Cks == CLOCK_PLL1_INDEX)         // PLL1
        regClockPtr->UartH_C = BIT17 + ((Clock_PllFreqGet(CLOCK_PLL1_INDEX) / (baudRate << 3) - 1) & 0xFFFF);
        else if (huartA_Cks == CLOCK_PLL2_INDEX)    // PLL2
        regClockPtr->UartH_C = BIT18 + BIT17 + ((Clock_PllFreqGet(CLOCK_PLL2_INDEX) / (baudRate << 3) - 1) & 0xFFFF);
        else if (huartA_Cks == CLOCK_PLL3_INDEX)    // PLL3
        {
            regClockPtr->Clkss_EXT1 |= BIT28;
            regClockPtr->UartH_C = BIT17 + ((Clock_PllFreqGet(CLOCK_PLL3_INDEX) / (baudRate << 3) - 1) & 0xFFFF);
        }
        else
            __asm("break 100");
    }
    else
    {   // Enable HUARTB clock divider
        regClockPtr->Clkss_EXT1 &= ~BIT24;

        if (huartB_Cks == CLOCK_PLL1_INDEX)
            regClockPtr->Aux0ClkC = BIT17 + ((Clock_PllFreqGet(CLOCK_PLL1_INDEX) / (baudRate << 3) - 1) & 0xFFFF);
        else if (huartB_Cks == CLOCK_PLL2_INDEX)
            regClockPtr->Aux0ClkC = BIT18 + BIT17 + ((Clock_PllFreqGet(CLOCK_PLL2_INDEX) / (baudRate << 3) - 1) & 0xFFFF);
        else if (huartB_Cks == CLOCK_PLL3_INDEX)
        {
            regClockPtr->Clkss_EXT1 |= BIT24;
            regClockPtr->Aux0ClkC = BIT17 + ((Clock_PllFreqGet(CLOCK_PLL3_INDEX) / (baudRate << 3) - 1) & 0xFFFF);
        }
        else
            __asm("break 100");

        bitField = CKE_UARTB;
    }

    regClockPtr->MdClken |= bitField;
    huartReset(index);

    // LSB first, positive data polarity, clock divider 8
    // Disable RTS/CTS, clock divid by 8
    regUartPtr->HUartC = HUART_CTS_DISABLE | HUART_RTS_DISABLE |
                         HUART_CTS_POL | HUART_RTS_POL | HUART_BUS_DIR |
                         HUART_CK_DIV_8;
    regUartPtr->HUartIntCtl = 0x003F0000;               // disable all interrupt
    regUartPtr->HUartBufCtl = 0x00000000;

    if (rxEnable == ENABLE)
    {
        if (index == HUART_A_INDEX)
        {
            if (huartAPinMultiplexerEnable == FALSE)
                regClockPtr->PduEn3 |= BIT0;                    // GP0 PAD PDU_EN
            else
            {
                #if (CHIP_VER_MSB == CHIP_VER_650)
                regClockPtr->PduEn2 |= BIT29;                   // KGP0 PAD PDU_EN
                #else   // MP660
                regClockPtr->PduEn1 |= BIT26;                   // HURX_A PAD PDU_EN
                #endif
            }
        }
#if (CHIP_VER_MSB == CHIP_VER_650)
        else
        {
            regClockPtr->PduEn1 |= BIT30;               // HURX_B PAD PDU_EN
        }
#endif

        regUartPtr->HUartC |= HUART_RX_ENABLE;
        regUartPtr->HUartIntCtl |= M_RXTHR_HIT;         // Enable UART Rx interrupt

        // Clean Rx FIFO
        while (regUartPtr->HUartRBufCnt & 0x00003F)
            regUartPtr->HUartRdBuf;
    }

    uartWaitDelay(20);

    if (!IsGdbConnect())
         IntEnable();
    else
        return;

    if (index == HUART_A_INDEX)
    {
        // connect the output pings with HUART
        #if (CHIP_VER_LSB == CHIP_VER_FPGA)
        Gpio_ConfiguraionSet(GPIO_UGPIO_0, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #endif

        regClockPtr->PinMuxCfg &= ~(BIT1 | BIT0);

        if (huartAPinMultiplexerEnable == FALSE)
            Gpio_ConfiguraionSet(GPIO_GPIO_0, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        else
        {
            #if (CHIP_VER_MSB == CHIP_VER_650)
            regClockPtr->PinMuxCfg |= BIT1;
            Gpio_ConfiguraionSet(GPIO_KGPIO_1, GPIO_ALT_FUNC_2, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
            #else   // MP660
            regClockPtr->PinMuxCfg |= BIT0;
			if (rxEnable == ENABLE)
			{
	            Gpio_ConfiguraionSet(GPIO_UGPIO_0, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
			}
			else
			{
	            Gpio_ConfiguraionSet(GPIO_UGPIO_1, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1);//TX
	            Gpio_ConfiguraionSet(GPIO_UGPIO_0, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 1); //UGPIO00
			}
            #endif
        }
    }
#if (CHIP_VER_MSB == CHIP_VER_650)
    else
    {
        Gpio_ConfiguraionSet(GPIO_UGPIO_4, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
    }
#endif

    if (index == HUART_A_INDEX)
        SystemIntHandleRegister(IM_UART, Uart1Isr);
    else
        SystemIntHandleRegister(IM_UART2, Uart2Isr);

    uartWaitDelay(20);
}



SDWORD HUartWaitStatus(DWORD index, DWORD dwStatus, DWORD timeOut)
{
    DWORD dwCount = timeOut;
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return -4;

    dwStatus &= 0xFFFF0000;

    if (!dwStatus)
        return PASS;            // No any cuase bit to be waiting.

    if (!timeOut)
    {
        while (!(regUartPtr->HUartIntCtl & dwStatus));
        regUartPtr->HUartIntCtl |= dwStatus;

        return PASS;
    }
    else
    {
        while (dwCount--)
        {
            if (regUartPtr->HUartIntCtl & dwStatus)
            {
                regUartPtr->HUartIntCtl |= dwStatus;

                return PASS;
            }
        }
    }

    return FAIL;
}



void HUartWaitTxComplete(DWORD index)
{
    DWORD counter;

    HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT);

    counter = 100000;
    while (counter)
        counter--;                              // wait until the tx activity finish
}



/*
void HUartRxDmaTimeoutThr(DWORD index, BYTE timeOutThr)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return;

    regUartPtr->HUartRBufCnt = (regUartPtr->HUartRBufCnt & 0xFF00FFFF) | (((DWORD) timeOutThr) << 16);
}
*/



SDWORD HUartBufferRead(DWORD index, BYTE *buffer, DWORD length, BYTE recvThr,
                       BOOL isPolling,
                       BOOL enableHwRxTimeOut, BYTE timeoutThr,
                       BOOL enableFollowCtrl)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return FAIL;

    if (recvThr > RECV_THR_32)
        recvThr = RECV_THR_32;

    regUartPtr->HUartC &= ~HUART_RX_ENABLE;
    regUartPtr->HUartRBufCnt = (regUartPtr->HUartRBufCnt & 0xFF00FFFF) | (((DWORD) timeoutThr) << 16);
    regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;
    regUartPtr->HUartBufCtl = (regUartPtr->HUartBufCtl & ~RECV_THR_FIELD) | (((DWORD) recvThr) << RECV_THR_SHIFT);
    HUartIntDisable(index, M_RXDMA_DONE | M_RXTHR_HIT | M_RX_TIMEOUT);

    if (ContextStatusGet() == 0)
        isPolling = TRUE;

    if (!isPolling)
    {
        if (enableHwRxTimeOut)
            HUartIntEnable(index, M_RXTHR_HIT | M_RX_TIMEOUT);
        else
            HUartIntEnable(index, M_RXTHR_HIT);
    }

    HUartFollowCtrl(index, enableFollowCtrl);
    regUartPtr->HUartC |= HUART_RX_ENABLE;

    if (isPolling)
    {
        DWORD status = C_RXTHR_HIT;

        if (enableHwRxTimeOut)
            status |= C_RX_TIMEOUT;

        while (length)
        {
            if (HUartWaitStatus(index, status, UART_WAIT_RXTH_TIMEOUT) == FAIL)
                return FAIL;

            *buffer = (BYTE) regUartPtr->HUartRdBuf;
            buffer++;
            length--;
        }
    }

    return PASS;
}



SDWORD HUartDmaRead(DWORD index, BYTE *buffer, DWORD bufLength, DWORD dmaThr,
                    BOOL isPolling,
                    BOOL enableHwRxTimeOut, BYTE timeoutThr,
                    BOOL enableFollowCtrl)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return FAIL;

    if (((DWORD) buffer) & 0x0F)
    {
        MP_ALERT("--E-- HUART DMA read buffer must be align to 4-DWORD boundary !!!\r\n");

        return -2;
    }

    if (bufLength & 0x0F)
    {
        MP_ALERT("--E-- HUART DMA read buffer length must be multiplex of 16 !!!\r\n");

        return -3;
    }

    if (dmaThr & 0x0F)
    {
        MP_ALERT("--E-- HUART DMA length must be multiplex of 16 !!!\r\n");

        return -4;
    }

    regUartPtr->HUartC &= ~HUART_RX_ENABLE;
    regUartPtr->HUartDmaLen &= 0xFFFF0000;
    regUartPtr->HUartDmaLen |= (dmaThr >> 4);  //receive unit : 16 bytes, 4DWORD
    regUartPtr->HUartRBufCnt = (regUartPtr->HUartRBufCnt & 0xFF00FFFF) | (((DWORD) timeoutThr) << 16);
    regUartPtr->HUartRxDmaSA = (DWORD) buffer;
    regUartPtr->HUartRxDmaEA = (DWORD) (buffer + bufLength);
    regUartPtr->HUartBufCtl |= RECV_FROM_DMA;
    HUartIntDisable(index, M_RXDMA_DONE | M_RXTHR_HIT | M_RX_TIMEOUT);

    if (ContextStatusGet() == 0)
        isPolling = TRUE;

    if (!isPolling)
    {
        if (enableHwRxTimeOut)
            HUartIntEnable(index, M_RXDMA_DONE | M_RX_TIMEOUT);
        else
            HUartIntEnable(index, M_RXDMA_DONE);
    }

    HUartFollowCtrl(index, enableFollowCtrl);
    regUartPtr->HUartC |= HUART_RX_ENABLE;

    if (isPolling)
    {
        DWORD status = C_RXTHR_HIT;
        SDWORD retVal;

        if (enableHwRxTimeOut)
            status |= C_RX_TIMEOUT;

        retVal = HUartWaitStatus(index, status, UART_WAIT_RXTH_TIMEOUT);

        if ((((DWORD) buffer) & BIT29) == 0)
            SetDataCacheInvalid();

        return retVal;

    }

    return PASS;
}



SDWORD HUartBufferWrite(DWORD index, BYTE *buffer, DWORD length, BOOL isPolling,
                        BOOL enableFollowCtrl)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return FAIL;

    if (HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
        return FAIL;

    regUartPtr->HUartC &= ~HUART_TX_ENABLE;
    regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;

    if (ContextStatusGet() == 0)
        isPolling = TRUE;

    if (isPolling)
        HUartIntDisable(index, M_TXTHR_HIT | M_TXDMA_DONE | M_TXBUF_EMP);
    else
        HUartIntEnable(index, M_TXBUF_EMP);

    HUartFollowCtrl(index, enableFollowCtrl);
    regUartPtr->HUartC |= HUART_TX_ENABLE;

    if (isPolling)
    {
        while (length)
        {
            if (HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
                return FAIL;

            regUartPtr->HUartWrBuf = *buffer;
            buffer++;
            length--;
        }
    }

    return PASS;
}



SDWORD HUartDmaWrite(DWORD index, BYTE *buffer, DWORD length, BOOL isPolling,
                     BOOL enableFollowCtrl)
{
    SDWORD status = PASS;
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return -4;

    if (!length)
        return -2;

    if (HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
        return -3;

    regUartPtr->HUartC &= ~HUART_TX_ENABLE;
    regUartPtr->HUartDmaLen &= 0x0000FFFF;
    regUartPtr->HUartDmaLen |= length << 16;        // unit is bytes
    regUartPtr->HUartTxDmaSA = (DWORD) buffer;
    regUartPtr->HUartTxDmaEA = ((DWORD) (buffer + length + 15)) & 0xFFFFFFF0;
    regUartPtr->HUartBufCtl |= TRAN_TO_DMA;

    if (ContextStatusGet() == 0)
        isPolling = TRUE;

    if (isPolling)
        HUartIntDisable(index, M_TXBUF_EMP);
    else
        HUartIntEnable(index, M_TXDMA_DONE);

    HUartFollowCtrl(index, enableFollowCtrl);
    regUartPtr->HUartC |= HUART_TX_ENABLE;

    if (isPolling)
    {
        status = HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT);

        //regUartPtr->HUartC &= ~HUART_TX_ENABLE;
        regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;
    }

    return status;
}




SDWORD HUartOutText(DWORD index, BYTE *buffer)
{
    HUART *regUartPtr;
    BYTE counter;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
    {
        //HUartOutText(HUART_A_INDEX, "HUartOutText - Wrong HUART Index");

        return;
    }

    if (HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
        return FAIL;

    regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;
    HUartFollowCtrl(index, DISABLE);
    regUartPtr->HUartC |= HUART_TX_ENABLE;

    while (*buffer)
    {
        if (HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == FAIL)
            return FAIL;
#if 1
        for (counter = 0; (counter < 16) && *buffer; counter++)
        {
            regUartPtr->HUartWrBuf = *buffer;
            buffer++;
        }
#else
        regUartPtr->HUartWrBuf = *buffer;
        buffer++;
#endif
    }

    return PASS;
}



SDWORD HUartOutValue(DWORD index, DWORD value, BYTE length)
{
    return HUartOutValueHex(index, value, length);
}



SDWORD HUartOutValueHex(DWORD index, DWORD value, BYTE length)
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

    return HUartOutText(index, (BYTE *) &buf);
}



SDWORD HUartOutValueDec(DWORD index, DWORD value, BYTE length)
{
    BYTE tmpBuf[12];

    if (length > 10)
        length = 10;

    value = value << ((10 - length) << 2);
    sprintf(tmpBuf, "%u ", value);

    return HUartOutText(index, tmpBuf);
}



SDWORD HUartGetChar(DWORD index)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return -1;

    regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;
    regUartPtr->HUartC |= HUART_RX_ENABLE;

    if (HUartWaitStatus(index, C_RXTHR_HIT, UART_WAIT_RXTH_TIMEOUT) == PASS)
    {
        return regUartPtr->HUartRdBuf;
    }
    else
    {
        return -1;
    }
}



SDWORD HUartPutChar(DWORD index, BYTE data)
{
    SDWORD status;
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return -4;

    regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;
    regUartPtr->HUartC |= HUART_TX_ENABLE;
    status = HUartWaitStatus(index, C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT);

    if (status == PASS)
        regUartPtr->HUartWrBuf = (DWORD) data;

    return status;
}



SDWORD HUartPutCharDrop(DWORD index, BYTE data)
{
    SDWORD status;
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return -4;

    regUartPtr->HUartBufCtl &= ~TRAN_TO_DMA;
    regUartPtr->HUartC |= HUART_TX_ENABLE;
    regUartPtr->HUartWrBuf = (DWORD) data;

    return PASS;
}



void HUartFollowCtrl(DWORD index, BOOL enable)
{
    HUART *regUartPtr;

    if (index == HUART_A_INDEX)
        regUartPtr = (HUART *) HUART1_BASE;
    else if (index == HUART_B_INDEX)
        regUartPtr = (HUART *) HUART2_BASE;
    else
        return;

    if (index == HUART_A_INDEX)
    {
        if (enable)
        {
        #if (CHIP_VER_LSB == CHIP_VER_FPGA)
            Gpio_ConfiguraionSet(GPIO_UGPIO_2, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #else
            Gpio_ConfiguraionSet(GPIO_GPIO_2, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
        #endif

            regUartPtr->HUartC &= ~(HUART_RTS_DISABLE | HUART_CTS_DISABLE);
        }
        else
        {   // Don't change the configure of there pins.
            regUartPtr->HUartC |= HUART_RTS_DISABLE | HUART_CTS_DISABLE;
        }
    }
    else
    {
#if (CHIP_VER_MSB == CHIP_VER_650)
        if (enable)
        {
            // connect the output pings with HUARTB
            Gpio_ConfiguraionSet(GPIO_UGPIO_6, GPIO_ALT_FUNC_1, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 2);
            regUartPtr->HUartC &= ~(HUART_RTS_DISABLE | HUART_CTS_DISABLE);
        }
        else
        {   // Don't change the configure of there pins.
            regUartPtr->HUartC |= HUART_RTS_DISABLE | HUART_CTS_DISABLE;
        }
#else
        MP_ALERT("CHIP-%04X is not support HUART-B !!!", CHIP_VER_MSB);
        __asm("break 100");
#endif
    }
}




void HUartA_PinMultiplexerEnable(BOOL enable)
{
    if (enable == ENABLE)
        huartAPinMultiplexerEnable = ENABLE;
    else
        huartAPinMultiplexerEnable = DISABLE;
}



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(HUartInit);
MPX_KMODAPI_SET(HUartIntEnable);
MPX_KMODAPI_SET(HUartIntDisable);
MPX_KMODAPI_SET(HUartGetChar);
MPX_KMODAPI_SET(HUartPutChar);
MPX_KMODAPI_SET(HUartBufferRead);
MPX_KMODAPI_SET(HUartBufferWrite);
MPX_KMODAPI_SET(HUartDmaRead);
MPX_KMODAPI_SET(HUartDmaWrite);
MPX_KMODAPI_SET(HUartOutText);
MPX_KMODAPI_SET(HUartOutValueHex);
MPX_KMODAPI_SET(HUartOutValueDec);
MPX_KMODAPI_SET(HUartFollowCtrl);
MPX_KMODAPI_SET(HUartRegisterCallBackFunc);
MPX_KMODAPI_SET(HUartClearCallBackFunc);
MPX_KMODAPI_SET(HUartOutValue);
#endif

#endif      // #if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )

