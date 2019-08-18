#define LOCAL_DEBUG_ENABLE 0


#include "global612.h"
#include "mpTrace.h"
#include "typedef.h"


#include "sio.h"

extern U08 u08PPPMainSemaId;
extern U08 u08PPPInSemaId;


//extern U08 u08PPPMainEventId;
extern U08 u08PPPMainRxId;


#ifndef byte_swap_of_dword
#define byte_swap_of_dword(x)   (((DWORD)(x) << 24) | (((DWORD)(x) & 0x0000ff00) << 8) |\
                                (((DWORD)(x) & 0x00ff0000) >> 8) | ((DWORD)(x) >> 24))
#endif

#if HAVE_USB_MODEM
int UsbWrite(int fd, char *data, int len);
#endif

#if UART_EDGE
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len){
	HUART *uart = (HUART *) (HUART1_BASE);
	int i = 0;

	mpx_SemaphoreWait(u08PPPMainSemaId);
	
	//uart->HUartC |= HUART_RX_ENABLE;	
	while (uart->HUartIntCtl & C_RXTHR_HIT)
	{
		data[i] = (BYTE) uart->HUartRdBuf;
		i++;
	}

	mpx_SemaphoreRelease(u08PPPMainSemaId);
	return i;
}

u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len){
	HUART *uart = (HUART *) (HUART1_BASE);
	int i;

	mpx_SemaphoreWait(u08PPPMainSemaId);

	//uart->HUartC |= HUART_TX_ENABLE;		// Enable Tx	
	for(i = 0; i < len ; i++)
	{
		//TaskSleep(10);
		if(WaitUart1Status(C_TXBUF_EMP) == 0)
			uart->HUartWrBuf = data[i];
		else
			UartOutText("Uart1 write fail");
	}


	mpx_SemaphoreRelease(u08PPPMainSemaId);

	return i;
}
#else
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len){
	DWORD dwNWEvent;  
	int i = 0, ret, lens;
	DWORD *point;

	//MP_DEBUG("sio_read");
	mpx_SemaphoreWait(u08PPPInSemaId);

#if HAVE_USB_MODEM > 0
//	RequestData();

//	lens = CopyDatas(data);
#endif
	//if(lens == 0){
	//	EventWait(u08PPPMainRxId, 0x1, OS_EVENT_OR, &dwNWEvent);
	//	
	//	lens = CopyDatas(data);
	//}

	mpx_SemaphoreRelease(u08PPPInSemaId);
	
	point = data;
	while(i < lens){
		*point = byte_swap_of_dword(*point);
		point++;
		i += 4;
	}
	
	return lens;

}

u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len){
	//DWORD dwNWEvent;  
	//int i;

	//mpx_SemaphoreWait(u08PPPMainSemaId);
#if 0
			MP_DEBUG("	len = %d\r\n", len);
			MP_DEBUG("out-- ");
			for(i=0; i<len; i++)
				MP_DEBUG("i = %d, 0x%02X ", i + 1, *(data+i));
			MP_DEBUG("\r\n");
#endif
	//mpx_SemaphoreWait(u08PPPMainSemaId);

#if HAVE_USB_MODEM
	return UsbWrite(fd, data, len);
#endif

	//mpx_SemaphoreRelease(u08PPPMainSemaId);

	//if(AtCmdSend(data, len))
		//EventWait(u08PPPMainEventId, 0x10, OS_EVENT_OR, &dwNWEvent);

	//mpx_SemaphoreRelease(u08PPPMainSemaId);
}

#endif

