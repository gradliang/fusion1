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
* Filename      : MP615_BTuart.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : UART module for BT
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

#if (((CHIP_VER & 0xFFFF0000) == CHIP_VER_615) && (BLUETOOTH == 1) && (BT_DRIVER_TYPE == BT_UART))

#define BT_HUART_BAUD       921600

#ifndef min
    #define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#define BT_HUart_EnaInt()               SystemIntEna(IM_UART)
#define BT_HUart_DisInt()               SystemIntDis(IM_UART)
#define BT_HUart_FollowCtrlEnable()     //HUartFollowCtrl(HUART_A_INDEX, ENABLE)
#define BT_HUart_FollowCtrlDisable()    //HUartFollowCtrl(HUART_A_INDEX, DISABLE)
#define BT_HUart_IntEnable(mask)        HUartIntEnable(mask)
#define BT_HUart_IntDisable(mask)       HUartIntDisable(mask)

extern void HUart_RxIND(); //from BT lib


void BT_HUartInit(void)
{
    UART *regUartPtr = (UART *) UART_BASE;

    //MP_DEBUG("BT HUart_Init Start !! ***\r\n");

    BT_HUart_DisInt();
    BT_HUart_Disable();

    HUartInit(DISABLE, BT_HUART_BAUD);
    HUartRegisterCallBackFunc(BT_UartIsr);
    BT_HUart_IntEnable(M_RXTHR_HIT);                // enable RCVTHR_HIT interrupt (Bit 1)
    //BT_HUart_IntEnable(M_TXBUF_EMP);                // enable TXBUF_EMP interrupt (Bit 4)

    BT_HUart_Enable();                          // Tx,Rx enable
    BT_HUart_EnaInt();                          // HUartIntCtl Register Difinition  // enable only RCVTHR_HIT interrupt (Bit 1)

    //MP_DEBUG("BT_HUartInit End !! ***\r\n");
}



void BT_HUart_Enable(void)
{
    UART *regUartPtr = (UART *) UART_BASE;

    //MP_DEBUG("BT_HUart_Enable !! ***\r\n");

    regUartPtr->HUartC |= HUART_ENABLE;
}



void BT_HUart_Disable(void)
{
    UART *regUartPtr = (UART *) UART_BASE;

    //MP_DEBUG("BT_HUart_Disable !! ***\r\n");

    regUartPtr->HUartC &= ~HUART_ENABLE;
    regUartPtr->HUartIntCtl = 0x003F0000;
}



void BT_HUart_TxEnaInt(void)
{
    HUartIntEnable(M_TXBUF_EMP);
}



void BT_HUart_TxDisInt(void)
{
    HUartIntDisable(M_TXBUF_EMP);
}



DWORD BT_HUart_Read(BYTE *buffer, DWORD buffer_len_avaliable)
{
    UART *regUartPtr = (UART *) UART_BASE;
    DWORD j, Bytecount;
    int result = 0;
    DWORD count;

    count = min(buffer_len_avaliable, (regUartPtr->HUartRBufPtr)&0x1F);
    //mpDebugPrint("count = %d", count);
    //MP_DEBUG("BT_HUart_Read !! ***\r\n");
    for (j = 0; j < count; j++)
    {
        //regUartPtr->HUartBufCtl &= ~RECV_FROM_DMA;
        //regUartPtr->HUartBufCtl &= ~RECV_THR_FIELD;

        //if (CheckHUartStatus(C_RXTHR_HIT) == PASS)
        //if (HUartWaitStatus(C_RXTHR_HIT, 1) == PASS)
        //{
            *buffer = regUartPtr->HUartRdBuf;
            buffer++;
        //    real_len++;
        //}
        //else
        //    break;
    }

    //MP_DEBUG("End Uart1Read !! ***\r\n");

    return count;
}



DWORD BT_HUart_Write(void)
{
    UART *regUartPtr = (UART *) UART_BASE;
    BYTE Count = 0, i = 0;
    BYTE buffer[32];

    Count = HUart_TxIND(&buffer[0] , 32);  //since empty

    for (i = 0; i < Count; i++)
    {
        //if (HUartWaitStatus(C_TXBUF_EMP, UART_WAIT_EMPTY_TIMEOUT) == PASS)
            regUartPtr->HUartWrBuf = buffer[i];
        //else
        //    i--;
    }
}



static void BT_UartIsr(DWORD cause)
{
    UART *regUartPtr = (UART *) UART_BASE;

    // filtered by mask bit
    //cause &= cause << 16;
        if (cause & C_RXTHR_HIT)
        {
            HUart_RxIND();
            mpDebugPrintN("$");
        }
        if (cause & C_TXBUF_EMP)
        {
            BT_HUart_Write();
            mpDebugPrintN("&");
        }

    //MP_DEBUG("BT_UartIsr End !! ***\r\n");
}

#endif      // #if (((CHIP_VER & 0xFFFF0000) == CHIP_VER_615) && (BLUETOOTH == ENABLE))

