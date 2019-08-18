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
* Filename      : MP650_BTuart.c
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

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))

#if ((BLUETOOTH == 1) && (BT_DRIVER_TYPE == BT_UART))

#if (DEBUG_COM_PORT == HUART_A_INDEX)
    #define BT_USING_UART2
#endif

#define BT_HUART_BAUD       921600

#ifndef min
    #define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifdef BT_USING_UART2
    #define BT_HUart_EnaInt()               SystemIntEna(IM_UART2)
    #define BT_HUart_DisInt()               SystemIntDis(IM_UART2)
    #define BT_HUart_FollowCtrlEnable()     HUartFollowCtrl(HUART_B_INDEX, ENABLE)
    #define BT_HUart_FollowCtrlDisable()    HUartFollowCtrl(HUART_B_INDEX, DISABLE)
    #define BT_HUart_IntEnable(mask)        HUartIntEnable(HUART_B_INDEX, mask)
    #define BT_HUart_IntDisable(mask)       HUartIntDisable(HUART_B_INDEX, mask)
#else
    #define BT_HUart_EnaInt()               SystemIntEna(IM_UART)
    #define BT_HUart_DisInt()               SystemIntDis(IM_UART)
    #define BT_HUart_FollowCtrlEnable()     HUartFollowCtrl(HUART_A_INDEX, ENABLE)
    #define BT_HUart_FollowCtrlDisable()    HUartFollowCtrl(HUART_A_INDEX, DISABLE)
    #define BT_HUart_IntEnable(mask)        HUartIntEnable(HUART_A_INDEX, mask)
    #define BT_HUart_IntDisable(mask)       HUartIntDisable(HUART_A_INDEX, mask)
#endif

extern void HUart_RxIND(); //from BT lib

static void BT_UartIsr(DWORD);
static DWORD BT_HUart_Write(void);
DWORD BT_HUart_Read(BYTE *buffer, DWORD buffer_len_avaliable);
void BT_HUart_TxEnaInt(void);
void BT_HUart_TxDisInt(void);
static void BT_HUart_Enable(void);
void BT_HUart_Disable(void);

void BT_HUartInit(void)
{
#ifdef BT_USING_UART2
    HUART *regUartPtr = (HUART *) HUART2_BASE;
#else
    HUART *regUartPtr = (HUART *) HUART1_BASE;
#endif

    //MP_DEBUG("BT HUart_Init Start !! ***\r\n");

    BT_HUart_DisInt();
    BT_HUart_Disable();

#ifdef BT_USING_UART2
    HUartInit(HUART_B_INDEX, DISABLE, BT_HUART_BAUD);
    HUartRegisterCallBackFunc(HUART_B_INDEX, BT_UartIsr);
#else
    HUartInit(HUART_A_INDEX, DISABLE, BT_HUART_BAUD);
    HUartRegisterCallBackFunc(HUART_A_INDEX, BT_UartIsr);
#endif

    // HUartC Register Difinition
    //regUartPtr->HUartC |= HUART_CK_DIV_8;     //Clock divider. 0:div4   1:div8
    //regUartPtr->HUartC |= HUART_BUS_DIR;      //0:msb first  1:lsb first
    //regUartPtr->HUartC &= ~HUART_RTS_DISABLE; //0:enble  1:disable
    //regUartPtr->HUartC &= ~HUART_CTS_DISABLE;
    regUartPtr->HUartC |= HUART_RTS_POL;        //Rx used, RTS=1 when Rx FIFO is full.
    regUartPtr->HUartC &= ~HUART_CTS_POL;       //Tx used, Tx stop transmitting data when CTS=0
    BT_HUart_FollowCtrlEnable();

    // HUartBufCtl Register Difinition
    //regUartPtr->HUartBufCtl &= ~(RECV_THR_FIELD | TRAN_THR_FIELD);
    //regUartPtr->HUartBufCtl |= RECV_THR_0 << RECV_THR_SHIFT;        // threshold Rx: >0Byte, Tx: <4Byte
    //regUartPtr->HUartBufCtl |= TRAN_THR_4 << TRAN_THR_SHIFT;

    //uart->HUartBufCtl |= RF_WIDTH;            //Rx Bus Width - 32bits
    //uart->HUartBufCtl |= WF_WIDTH;            //Tx Bus Width - 32bits
    BT_HUart_IntEnable(M_RXTHR_HIT);            // enable RCVTHR_HIT interrupt (Bit 1)
    //BT_HUart_IntEnable(M_TXBUF_EMP);            // enable TXBUF_EMP interrupt (Bit 4)

    BT_HUart_Enable();                          // Tx,Rx enable
    BT_HUart_EnaInt();                          // HUartIntCtl Register Difinition  // enable only RCVTHR_HIT interrupt (Bit 1)

    //MP_DEBUG("BT_HUartInit End !! ***\r\n");
}



static void BT_HUart_Enable(void)
{
#ifdef BT_USING_UART2
    HUART *regUartPtr = (HUART *) HUART2_BASE;
#else
    HUART *regUartPtr = (HUART *) HUART1_BASE;
#endif

    //MP_DEBUG("BT_HUart_Enable !! ***\r\n");

    regUartPtr->HUartC |= HUART_RX_ENABLE | HUART_TX_ENABLE;
}



void BT_HUart_Disable(void)
{
#ifdef BT_USING_UART2
    HUART *regUartPtr = (HUART *) HUART2_BASE;
#else
    HUART *regUartPtr = (HUART *) HUART1_BASE;
#endif

    //MP_DEBUG("BT_HUart_Disable !! ***\r\n");

    regUartPtr->HUartC &= ~(HUART_RX_ENABLE | HUART_TX_ENABLE);       // Disable Rx and Tx
    regUartPtr->HUartIntCtl = 0x003F0000;
}



void BT_HUart_TxEnaInt(void)
{
#ifdef BT_USING_UART2
    HUartIntEnable(HUART_B_INDEX, M_TXBUF_EMP);
#else
    HUartIntEnable(HUART_A_INDEX, M_TXBUF_EMP);
#endif
}



void BT_HUart_TxDisInt(void)
{
#ifdef BT_USING_UART2
    HUartIntDisable(HUART_B_INDEX, M_TXBUF_EMP);
#else
    HUartIntDisable(HUART_A_INDEX, M_TXBUF_EMP);
#endif
}



DWORD BT_HUart_Read(BYTE *buffer, DWORD buffer_len_avaliable)
{
#ifdef BT_USING_UART2
    HUART *regUartPtr = (HUART *) HUART2_BASE;
#else
    HUART *regUartPtr = (HUART *) HUART1_BASE;
#endif
    int j, Bytecount, real_len=0 ;
    int result = 0 ;

    //MP_DEBUG("BT_HUart_Read !! ***\r\n");

    Bytecount = regUartPtr->HUartRBufCnt & RXBUF_CNT;

    if( !Bytecount )  //HW false caution
        return 0;

    real_len = min(Bytecount , buffer_len_avaliable);
    //MP_DEBUG("real_len %d",real_len);

    for (j = 0; j < real_len; j++)
    {
        if( !(regUartPtr->HUartBufCtl & RF_WIDTH) )
        {   //8 bits
            *buffer = (BYTE) (regUartPtr->HUartRdBuf & 0x000000FF);
            buffer++;
        }
        else
        {   //32 bits
            register DWORD tmp = regUartPtr->HUartRdBuf;

            buffer[0] = (BYTE) (tmp & 0xFF);
            buffer[1]= (BYTE) ((tmp >> 8) & 0xFF);
            buffer[2]= (BYTE) ((tmp >> 16) & 0xFF);
            buffer[3]= (BYTE) ((tmp >> 24) & 0xFF);
            buffer += 4;
            j += 4;
        }
    }

    //MP_DEBUG("End Uart1Read !! ***\r\n");

    return real_len ;
}



static DWORD BT_HUart_Write(void)
{
#ifdef BT_USING_UART2
    HUART *regUartPtr = (HUART *) HUART2_BASE;
#else
    HUART *regUartPtr = (HUART *) HUART1_BASE;
#endif
    BYTE Count = 0, i = 0;
    BYTE buffer[32];

    Count = HUart_TxIND(&buffer[0] , 32);  //since empty

    for (i = 0; i < Count; i++)
        regUartPtr->HUartWrBuf = buffer[i];
}



void BT_UartIsr(DWORD cause)
{
#ifdef BT_USING_UART2
    HUART *regUartPtr = (HUART *) HUART2_BASE;
#else
    HUART *regUartPtr = (HUART *) HUART1_BASE;
#endif

    // filtered by mask bit
    cause = cause & (cause << 16);

    if (cause & C_RXTHR_HIT)
        HUart_RxIND();

    if (cause & C_TXBUF_EMP)
        BT_HUart_Write();

    //MP_DEBUG("BT_UartIsr End !! ***\r\n");
}

#endif      // #if ((BLUETOOTH == 1) && (BT_DRIVER_TYPE == BT_UART))

#endif

