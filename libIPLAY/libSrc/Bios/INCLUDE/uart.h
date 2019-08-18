#ifndef UART_H
#define UART_H

#include "iplaysysconfig.h"
#include "hal_Ckg.h"

///
///@ingroup     UART_MODULE
///@brief       define HUART-A Index number
#define HUART_A_INDEX           0               // HUART-A

///
///@ingroup     UART_MODULE
///@brief       define HUART-B Index number
#define HUART_B_INDEX           1               // HUART-B

///
///@ingroup     UART_MODULE
///@brief       define HUART-B Index number
#define HUART_NONE_INDEX        3

#ifndef MP_TRACE_DEBUG_PORT
    //#warning Undefine - MP_TRACE_DEBUG_PORT
    #define MP_TRACE_DEBUG_PORT DEBUG_PORT_HUART_A
#endif

typedef void (*UART_ISR_CALLBACK)(DWORD);

#if (CHIP_VER_MSB == CHIP_VER_615)

///////////////////////////////////////////////////////////////////////////
//
//  For MP615 - HUART Part
//
///////////////////////////////////////////////////////////////////////////
#if (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_A)
    #define DEBUG_COM_PORT      HUART_A_INDEX           // UART
#elif (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_B)
    #define DEBUG_COM_PORT      HUART_B_INDEX           // HUART
#else
    //#warning MP_TRACE_DEBUG_PORT was defined DEBUG_PORT_NONE
    #define DEBUG_COM_PORT      HUART_NONE_INDEX
#endif

#define UART_WAIT_RXTH_TIMEOUT  0x00400000
#define UART_WAIT_EMPTY_TIMEOUT 0x00100000

// HUartC Register Difinition
#define HUART_CK_DIV_8          BIT0            ///< bit field of clock divider, 0:div4, 1:div8
#define HUART_BUS_DIR           BIT8            ///< bit field of bit Sequence, 0:MSB first, 1:LSB first
#define HUART_BUS_POL           BIT16           ///< Data bit polarity  0:positive  1: negative
#define HUART_ENABLE            BIT24           ///< Enable HUART

// HUartIntCtl Register Difinition
#define M_RXDMA_DONE            BIT0
#define M_RXTHR_HIT             BIT1
#define M_TXDMA_DONE            BIT2
#define M_TXTHR_HIT             BIT3
#define M_TXBUF_EMP             BIT4
#define C_RXDMA_DONE            BIT16           // RX DMA THR HIT
#define C_RXTHR_HIT             BIT17           // RX BUFFER THR HIT
#define C_TXDMA_DONE            BIT18
#define C_TXTHR_HIT             BIT19
#define C_TXBUF_EMP             BIT20

// HUartBufCtl Register Difinition
#define RECV_THR_FIELD          0x00000003      // bit[1:0]
#define RECV_THR_SHIFT          0               // bit[2:0]

///
///@ingroup     UART_MODULE
///@brief       define HUART Receive buffer thread
typedef enum
{
    RECV_THR_0 = 0,                             ///< thread is > 0 byte
    RECV_THR_8,                                 ///< thread is >= 8 byte
    RECV_THR_16,                                ///< thread is >= 16 byte
    RECV_THR_24,                                ///< thread is >= 24 byte
} E_RECV_BUF_THREAD;

#define TRAN_THR_FIELD          0x00000300      // bit[9:8]
#define TRAN_THR_SHIFT          8

///
///@ingroup     UART_MODULE
///@brief       define HUART Transmit buffer thread
typedef enum
{
    TRAN_THR_4 = 0,                             ///< thread is < 4 byte
    TRAN_THR_8,                                 ///< thread is < 8 byte
    TRAN_THR_12,                                ///< thread is < 12 byte
    TRAN_THR_16,                                ///< thread is < 16 byte
} E_TRAN_BUF_THREAD;

#define RECV_FROM_DMA           BIT16           ///< bit field of get the receive data from DMA
#define TRAN_TO_DMA             BIT24           ///< bit field of put the transmit data to DMA

///////////////////////////////////////////////////////////////////////////
//
//  For MP615 - UART Part
//
///////////////////////////////////////////////////////////////////////////
#define UART_BAUD               115200

#define C_RX_FIFO_FULL          BIT0
#define C_RX_FIFO_THRESHOLD     BIT1
#define C_RX_ERROR_FRAME        BIT2
#define C_RX_ERR_OVERRUN        BIT3
#define C_RX_READY              BIT4
#define C_TX_EMPTY              BIT5
#define C_TX_READY              BIT6

#define M_RX_FIFO_FULL          BIT16
#define M_RX_FIFO_THRESHOLD     BIT17
#define M_RX_ERROR_FRAME        BIT18
#define M_RX_ERR_OVERRUN        BIT19
#define M_RX_READY              BIT20
#define M_TX_EMPTY              BIT21
#define M_TX_READY              BIT22

#define STATUS_MASK             0x7F
#define RX_ERROR_MASK           0x0F

void UartIsr(void);
void LUart_Init(void);
void LUartOutText(BYTE *buffer);
void LUartOutValue(register DWORD value, register BYTE length);
BYTE LGetUartChar();
void LPutUartChar(BYTE data);
void LPutUartCharDrop(BYTE data);
int LWaitUartStatus(DWORD dwStatus);
void LWaitUartTxComplete();
SDWORD LUartRead(BYTE * buffer, DWORD length);
SDWORD LUartWrite(BYTE *buffer, DWORD length);

void HUartInit(BOOL rxEnable, DWORD baudRate);
SDWORD HUartOutText(BYTE *buffer);
SDWORD HUartOutValue(DWORD value, BYTE length);
SDWORD HUartGetChar(void);
SDWORD HUartPutChar(BYTE data);
SDWORD HUartPutCharDrop(BYTE data);
SDWORD HUartWaitStatus(DWORD dwStatus, DWORD timeOut);
SDWORD HUartWaitStatus(DWORD dwStatus, DWORD timeOut);
void HUartWaitTxComplete(void);
SDWORD HUartBufferRead(BYTE *buffer, DWORD length, BYTE recvThr, BOOL isPolling);
SDWORD HUartBufferWrite(BYTE *buffer, DWORD length, BOOL isPolling);

#else   // MP650/660

///////////////////////////////////////////////////////////////////////////
//
//  For MP650/660
//
///////////////////////////////////////////////////////////////////////////
#if (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_A)
    //#warning MP_TRACE_DEBUG_PORT was defined DEBUG_PORT_HUART_A
    #define DEBUG_COM_PORT      HUART_A_INDEX
#elif (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_B)
    //#warning MP_TRACE_DEBUG_PORT was defined DEBUG_PORT_HUART_B
    #define DEBUG_COM_PORT      HUART_B_INDEX
#else
    //#warning MP_TRACE_DEBUG_PORT was defined DEBUG_PORT_NONE
    #define DEBUG_COM_PORT      HUART_NONE_INDEX
#endif

// HUartC Register Difinition
#define HUART_CK_DIV_8          BIT0            ///< bit field of clock divider, 0:div4, 1:div8
#define HUART_BUS_DIR           BIT8            ///< bit field of bit Sequence, 0:MSB first, 1:LSB first
#define HUART_RTS_POL           BIT9            ///< bit field of RTS control signal polarity
                                                ///< 0: RTS default value would be set to 1 when RTS_POL is written to 0.
                                                ///<    RTS = 0 when Rx FIFO will be full.
                                                ///< 1: RTS default value would be set to 0 when RTS_POL is written to 1.
                                                ///<    RTS = 1 when Rx FIFO will be full.
#define HUART_CTS_POL           BIT10           ///< bit field of CTS control signal polarity
                                                ///< 0: Tx stop transmitting data when CTS = 0.
                                                ///< 1: Tx stop transmitting data when CTS = 1.
#define HUART_BUS_POL           BIT16           ///< Data bit polarity  0:positive  1: negative
#define HUART_BUS_STOP          BIT17           ///< Stop bit format  0:1 stop bit  1: 2 stop bit
#define HUART_RX_ENABLE         BIT24           ///< Enable RX
#define HUART_TX_ENABLE         BIT25           ///< Enabel TX
#define HUART_RTS_DISABLE       BIT26
#define HUART_CTS_DISABLE       BIT27

// HUartIntCtl Register Difinition
#define M_RXDMA_DONE            BIT0
#define M_RXTHR_HIT             BIT1
#define M_TXDMA_DONE            BIT2
#define M_TXTHR_HIT             BIT3
#define M_TXBUF_EMP             BIT4
#define M_RX_TIMEOUT            BIT5            // when Rx buffer or DMA is not empty
#define C_RXDMA_DONE            BIT16           // RX DMA THR HIT
#define C_RXTHR_HIT             BIT17           // RX BUFFER THR HIT
#define C_TXDMA_DONE            BIT18
#define C_TXTHR_HIT             BIT19
#define C_TXBUF_EMP             BIT20
#define C_RX_TIMEOUT            BIT21

// HUartBufCtl Register Difinition
#define RECV_THR_FIELD          0x00000007      // bit[2:0]
#define RECV_THR_SHIFT          0

///
///@ingroup     UART_MODULE
///@brief       define HUART Receive buffer thread
typedef enum
{
    RECV_THR_0 = 0,     ///< thread is > 0 byte
    RECV_THR_8,         ///< thread is >= 8 byte
    RECV_THR_16,        ///< thread is >= 16 byte
    RECV_THR_24,        ///< thread is >= 24 byte
    RECV_THR_32,        ///< thread is >= 32 byte
} E_RECV_BUF_THREAD;

#define TRAN_THR_FIELD          0x00000300      // bit[9:8]
#define TRAN_THR_SHIFT          8

///
///@ingroup     UART_MODULE
///@brief       define HUART Transmit buffer thread
typedef enum
{
    TRAN_THR_4 = 0,     ///< thread is < 4 byte
    TRAN_THR_8,         ///< thread is < 8 byte
    TRAN_THR_12,        ///< thread is < 12 byte
    TRAN_THR_16,        ///< thread is < 16 byte
} E_TRAN_BUF_THREAD;

#define RF_WIDTH                BIT4            ///< bit field of bus width for read data, 0: 8 bit, 1: 32 bit
#define WF_WIDTH                BIT12           ///< bit field of bus width for write data, 0: 8 bit, 1: 32 bit
#define DEBUG_SEL               BIT15
#define RECV_FROM_DMA           BIT16           ///< bit field of get the receive data from DMA
#define RX_FLUSH_FROM_DMA       BIT17           ///< bit field of RX flush from DMA
#define TRAN_TO_DMA             BIT24           ///< bit field of put the transmit data to DMA

///
///@ingroup     UART_MODULE
///@brief       define default UART baud rate, bps
#define HUART_BAUD              115200

//HUartRBufCnt Register Difinition
//Receive buffer byte count (HUARFCNT_A)
#define RXBUF_CNT               0x0000003F      //[5:0]  Receive buffer byte count. (Read Only)
#define RX_TO_HITCLR_DIS        BIT6            //[6]RX_TO_HITCLR_DIS Set this bit to 1, then the interrupt cause "C_RX_TIMEOUT" would not be cleared by "C_RXDMA_THR_HIT".
#define RX_TIMEOUT_SET_ECO      BIT7            //[7] RX_TIMEOUT_SET_ECOSet to 1, the "C_RX_TIMEOUT" would be set without BIU transactions affect.
#define RTS                     BIT8            //[8] RTS Rx use. It would be driven to inactive when Rx buffer is full. (Read Only)
#define CTS                     BIT9            //[9] CTS Tx use. Stop transmit data when it is inactive. (Read Only)
#define RFD_SM                  0x00000C00      //[11:10] RFD_SM   Rx DMA state machine. (Read Only)
                                                //        00: idle, 01: request_dma, 10: dma_done, 11: rx_dma_thr_hit
///
///@ingroup     UART_MODULE
///@brief       define the timeout value for RX thread
#define UART_WAIT_RXTH_TIMEOUT  0x00400000

///
///@ingroup     UART_MODULE
///@brief       define the timeout value for TX empty
#define UART_WAIT_EMPTY_TIMEOUT 0x00100000

void Uart1Isr(void);
void Uart2Isr(void);

///
///@ingroup     UART_MODULE
///@brief       Initial UART
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       rxEnable    ENABLE(1)/DISABLE(0) RX function in initial stage.
///@param       baudRate    The spped of HUART specified by baudRate. Unit is bps.
///
///@retval      None
///
///@remark      For MP650 or later only.
///@remark      Default Setting - Disable follow control, RTS/CTS is positive data polarity
///@remark                        LSB first
///@remark      RX threshold is RECV_THR_0(0), TX threshold Rx is TRAN_THR_4(0), PIO mode
///@remark      Interrupt disable, Data bus width is 8bits
///
void HUartInit(DWORD index, BOOL rxEnable, DWORD baudRate);

///
///@ingroup     UART_MODULE
///@brief       Enable HUART's interrupt
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       mask        The mask of HUART's interrupt.\n
///                         M_RXDMA_DONE - Interrupt enable of Rx to DMA thread hit.\n
///                         M_RXTHR_HIT - Interrupt enable of Rx buffer thread hit.\n
///                         M_TXDMA_DONE - Interrupt enable of Tx from DMA done.\n
///                         M_TXTHR_HIT - Interrupt enable of Tx buffer thread hit.\n
///                         M_TXBUF_EMP - Interrupt enable of Tx buffer empty.\n
///                         M_RX_TIMEOUT - Interrupt enable of no data is received for a duration
///                                        that is defined in RX_TIMEOUT_THR when Rx buffer/DMA is not empty.
///
///@retval      None
///
///@remark      For MP650 or later only.
///@remark      For MP650 or later only.
///
void HUartIntEnable(DWORD index, DWORD mask);

///
///@ingroup     UART_MODULE
///@brief       Disable HUART's interrupt
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       mask        The mask of HUART's interrupt.\n
///
///@retval      None
///
///@remark      See HUartIntEnable
///@remark      For MP650 or later only.
///
void HUartIntDisable(DWORD index, DWORD mask);

///
///@ingroup     UART_MODULE
///@brief       Get a char from HUART's RX FIFO by polling.
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///
///@retval      <= 0xFFFF   Data
///@retval      -1          Time out.
///
///@remark      See HUartIntEnable
///@remark      For MP650 or later only.
///
SDWORD HUartGetChar(DWORD index);

///
///@ingroup     UART_MODULE
///@brief       Put a char to HUART's TX FIFO by polling.
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       data        The char will be send.
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See HUartIntEnable
///@remark      For MP650 or later only.
///
SDWORD HUartPutChar(DWORD index, BYTE data);

SDWORD HUartPutCharDrop(DWORD index, BYTE data);

///
///@ingroup     UART_MODULE
///@brief       Wait HUART's specified status by polling.
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       dwStatus    Waitting condiion. Bits mapping to cause of HUART's interrupt.
///                         C_RXDMA_DONE - Cause of M_RXDMA_THR_HIT.\n
///                         C_RXTHR_HIT - Cause of M_RXTHR_HIT.\n
///                         C_TXDMA_DONE - Cause of M_TXDMA_DONE.\n
///                         C_TXTHR_HIT - Cause of C_TXTHR_HIT.\n
///                         C_TXBUF_EMP - Cause of C_TXBUF_EMP.\n
///                         C_RX_TIMEOUT - Cause of C_RX_TIMEOUT.
///@param       timeOut     Time out value. Using loop delay.
///                         If \b timeOut is \b 0, means no time out.
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      For MP650 or later only.
///
SDWORD HUartWaitStatus(DWORD index, DWORD dwStatus, DWORD timeOut);

///
///@ingroup     UART_MODULE
///@brief       Wait HUART's C_TXBUF_EMP by polling.
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      For MP650 or later only.
///
void HUartWaitTxComplete(DWORD index);

///
///@ingroup     UART_MODULE
///@brief       Read data from HUART by PIO mode.
///
///@param       index               HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       buffer              A buffer pointer for restore data.
///@param       length              Specified data length will be read.
///@param       isPolling           TRUE(1) - Using polling method to read data.\n
///                                 FALSE(0) - Using interrupt method to read data.
///@param       recvThr             Setting the thread value for recvive.\n
///                                 RECV_THR_0(0)   : > 0 byte\n
///                                 RECV_THR_8(1)   : >= 8 byte\n
///                                 RECV_THR_16(2)  : >= 16 byte\n
///                                 RECV_THR_16(3)  : >= 24 byte\n
///                                 RECV_THR_16(4)  : >= 32 byte\n
///@param       enableHwRxTimeOut   Specified data length will be read.
///@param       timeoutThr          The timeout counter's thread value. (0 ~ 255)\n
///                                 The unit of this counter is 128 UART clock.
///@param       enableFollowCtrl    ENABLE(1)/DISABLE(0) follow control
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      When \b isPolling is \b FALSE, the parameter of *buffer and length will be ignore.
///@remark      For MP650 or later only.
///
SDWORD HUartBufferRead(DWORD index, BYTE *buffer, DWORD length, BYTE recvThr,
                       BOOL isPolling,
                       BOOL enableHwRxTimeOut, BYTE timeoutThr,
                       BOOL enableFollowCtrl);

///
///@ingroup     UART_MODULE
///@brief       Write data to HUART by PIO mode.
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       buffer      A buffer pointer for send data.
///@param       length      Specified data length will be send.
///@param       isPolling   TRUE(1) - Using polling method to write data.
///                         FALSE(0) - Using interrupt method to write data.
///@param       enableFollowCtrl    ENABLE(1)/DISABLE(0) follow control
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      When \b isPolling is \b FALSE, the parameter of *buffer and length will be ignore.
///@remark      For MP650 or later only.
///
SDWORD HUartBufferWrite(DWORD index, BYTE *buffer, DWORD length, BOOL isPolling,
                        BOOL enableFollowCtrl);

///
///@ingroup     UART_MODULE
///@brief       Read data from HUART by DMA mode.
///
///@param       index               HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       buffer              A buffer pointer for restore data. It has 16-byte boundary limit.
///@param       bufLength           Specified buffer's length. It must be multiplex of 16.
///@param       dmaThr              Specified thread value of RX DMA. It must be multiplex of 16.
///@param       isPolling           TRUE(1) - Using polling method to read data.
///                                 FALSE(0) - Using interrupt method to read data.
///@param       enableHwRxTimeOut   Specified data length will be read.
///@param       timeoutThr          The timeout counter's thread value. (0 ~ 255)\n
///                                 The unit of this counter is 128 UART clock.
///@param       enableFollowCtrl    ENABLE(1)/DISABLE(0) follow control
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///@retval      -2                  buffer point to a non 4-DWORD address.
///@retval      -3                  buffer length is not multiplex of 16.
///
///@remark      For MP650 or later only.
///
SDWORD HUartDmaRead(DWORD index, BYTE *buffer, DWORD bufLength, DWORD dmaThr,
                    BOOL isPolling,
                    BOOL enableHwRxTimeOut, BYTE timeoutThr,
                    BOOL enableFollowCtrl);

///
///@ingroup     UART_MODULE
///@brief       Write data to HUART by DMA mode
///
///@param       index               HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       buffer              A buffer pointer for send data. It has 16-byte boundary limit.
///@param       length              Specified data length will be send.
///@param       isPolling           TRUE(1) - Using polling method to read data.
///                                 FALSE(0) - Using interrupt method to read data.
///@param       enableFollowCtrl    ENABLE(1)/DISABLE(0) follow control
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///@retval      -2                  buffer point to a non 4-DWORD address.
///@retval      -3                  buffer length is not multiplex of 16.
///
///@remark      For MP650 or later only.
///
SDWORD HUartDmaWrite(DWORD index, BYTE *buffer, DWORD length, BOOL isPolling,
                     BOOL enableFollowCtrl);

///
///@ingroup     UART_MODULE
///@brief       Send a string to HUART
///
///@param       index               HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       buffer              A buffer pointer for send data.
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///
///@remark      For MP650 or later only.
///
SDWORD HUartOutText(DWORD index, BYTE *buffer);

///
///@ingroup     UART_MODULE
///@brief       Convert a number to string and send to HUART (HEX format).
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       value       The number will be convert and send.
///@param       length      value width will be see.
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///
///@remark      For MP650 or later only.
///
SDWORD HUartOutValueHex(DWORD index, DWORD value, BYTE length);

///
///@ingroup     UART_MODULE
///@brief       Convert a number to string and send to HUART. (Dec format)
///
///@remark      See HUartOutValue
///@remark      For MP650 or later only.
///
SDWORD HUartOutValueDec(DWORD index, DWORD value, BYTE length);

///
///@ingroup     UART_MODULE
///@brief       ENABLE(1)/DISABLE(0) HUART's follow control.
///
///@remark      See HUartOutValue
///@remark      For MP650 or later only.
///
void HUartFollowCtrl(DWORD index, BOOL enable);

///
///@ingroup     UART_MODULE
///@brief       Register callback function for HUART's interrupt
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       funPtr      callback function's pointer, prototype is void (*)(DWORD)
///
///@retval      None
///
///@remark
///
void HUartRegisterCallBackFunc(DWORD index, UART_ISR_CALLBACK funPtr);

///
///@ingroup     UART_MODULE
///@brief       Deregister callback function for HUART's interrupt
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///
///@retval      None
///
///@remark
///
void HUartClearCallBackFunc(DWORD index);

///
///@ingroup     UART_MODULE
///@brief       See HUartOutValueHex
///
///@remark      For MP650 or later only.
///
SDWORD HUartOutValue(DWORD index, DWORD value, BYTE length);

///
///@ingroup     UART_MODULE
///@brief       Enable HUART-A's pin multiplexer.
///
///@param       enable      1(ENABLE) / 0 (DISABLE)
///
///@remark      For MP65x serial - ENABLE(using KGPIO-1/2), DISABLE(using GPIO-0/1)
///@remark      For MP66x serial - ENABLE(using UGPIO-0/1), DISABLE(using GPIO-0/1)
///
void HUartA_PinMultiplexerEnable(BOOL enable);

///
///@ingroup     UART_MODULE
///@brief       Select the clock source of HUART
///
///@param       index       HUART module select, HUART_A_INDEX(0), HUART_B_INDEX(1)
///@param       clkSel      clock source index, see \bE_PLL_INDEX.
///
///@remark      For MP650 or later only.
///
void HUartClkSourceSel(DWORD index, E_PLL_INDEX clkSel);

#endif



///////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////
#if (BLUETOOTH == ENABLE)
void BT_HUartInit(void);
void BT_UartIsr(DWORD);
DWORD BT_HUart_Write(void);
DWORD BT_HUart_Read(BYTE *buffer, DWORD buffer_len_avaliable);
void BT_HUart_TxEnaInt(void);
void BT_HUart_TxDisInt(void);
void BT_HUart_Enable(void);
void BT_HUart_Disable(void);
#endif

#endif      // #ifndef UART_H

