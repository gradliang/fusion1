/*-----------------------------------------------------------------------------------*/
/* mem.c
 *
 * Movie Player Memory Dynamic Allocation manager.
 *
 * 07.15.2006 Modified by Athena
 */ 
/*-----------------------------------------------------------------------------------*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "string.h" 

#include "linux/list.h"
#include "ndebug.h"
#include "net_device.h"

#define NETBUF_OVERHEAD 32

extern int HeapSemid;
extern void *net_buf_start;
extern void *net_buf_end;                              /* end of buffer pool */

int netpool_create(char *chunk, unsigned int nbufs, unsigned int bufsz);
int netpool_destroy(void);


#if 0

void rt73_netpool_init(void)
{
    netpool_mem_init(RALINK_MAX_NUM_BUFFERS,RALINK_MAX_NETPOOL_ALLOC_SIZE);
}

void ar2524_netpool_init(void)
{
    netpool_mem_init(AR2524_MAX_NUM_BUFFERS,AR2524_MAX_NETPOOL_ALLOC_SIZE);
}

#else

void rt73_netpool_init(void)
{
	u8_t *tmp;
    int ret;
    WHICH_OTG eWhichOtg;
	
    eWhichOtg = WIFI_USB_PORT;

	MP_DEBUG("\n\r!!!===============???rt73_netpool_init???==================!!!\n\r");

    if (!HeapSemid)
    {
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
    }

	netpool_destroy();

	tmp = (u8_t *)SystemGetMemAddr(NETPOOL_BUF_ID);

	net_buf_start = tmp;

    /*
     * 500 * (2528+32) = 1280000 
     */
    netpool_create(tmp, RALINK_MAX_NUM_BUFFERS, RALINK_MAX_NETPOOL_BUFFER_SIZE);
    tmp += RALINK_MAX_NUM_BUFFERS * (RALINK_MAX_NETPOOL_BUFFER_SIZE + NETBUF_OVERHEAD);

	net_buf_end = tmp;
	mpDebugPrint("net_buf_start %x net_buf_end %x",net_buf_start,net_buf_end);

	UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end,eWhichOtg);
}

void ar2524_netpool_init(void)
{
	u8_t *tmp;
    int ret;
    WHICH_OTG eWhichOtg;
	
    eWhichOtg = WIFI_USB_PORT;

	MP_DEBUG("\n\r!!!===============???ar2524_netpool_init???==================!!!\n\r");

    if (!HeapSemid)
    {
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
    }

	netpool_destroy();

	tmp = (u8_t *)SystemGetMemAddr(NETPOOL_BUF_ID);

	net_buf_start = tmp;

    /*
     * NETPOOL_BUF_SIZE = 96 * 4864 = 466944
     * MAX_NETPOOL_BUFFER_SIZE = 4800
     *
     * 32 * MAX_NETPOOL_BUFFER_SIZE = 153600
     * 128 * 2048 = 131072 
     * 96 * 512 = 32768
     * (64+64+64) * 32 = 6144
     */
#if DEMO_PID
    netpool_create(tmp, (96*2), 512);
    tmp += (96*2)*(512 + NETBUF_OVERHEAD);
    netpool_create(tmp, 96, 2048);
    tmp += (96*2)*(2048 + NETBUF_OVERHEAD);
    netpool_create(tmp, 64, AR2524_MAX_NETPOOL_BUFFER_SIZE);
    tmp += (64*2)*(AR2524_MAX_NETPOOL_BUFFER_SIZE + NETBUF_OVERHEAD);
#elif (SHOW_FRAME_TEST && Make_ADHOC && (Make_USB == 3))/*add for AdHoc Demo test*/
    netpool_create(tmp, 96, 2048);
    tmp += 121 * (2048 + NETBUF_OVERHEAD);
    netpool_create(tmp, 64, AR2524_MAX_NETPOOL_BUFFER_SIZE);
    tmp += 64 * (AR2524_MAX_NETPOOL_BUFFER_SIZE + NETBUF_OVERHEAD);
#else
    netpool_create(tmp, 96, 512);
    tmp += 96 * (512 + NETBUF_OVERHEAD);
    netpool_create(tmp, 96, 2048);
    tmp += 96 * (2048 + NETBUF_OVERHEAD);
    netpool_create(tmp, 64, AR2524_MAX_NETPOOL_BUFFER_SIZE);
    tmp += 64 * (AR2524_MAX_NETPOOL_BUFFER_SIZE + NETBUF_OVERHEAD);
#endif

	net_buf_end = tmp;
	mpDebugPrint("net_buf_start %x net_buf_end %x",net_buf_start,net_buf_end);

	UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end,eWhichOtg);
}


void m9ks_netpool_init(void)
{
	u8_t *tmp;
    int ret;
    WHICH_OTG eWhichOtg;
	
    eWhichOtg = WIFI_USB_PORT;

	MP_DEBUG("\n\r!!!===============???ar2524_netpool_init???==================!!!\n\r");

    if (!HeapSemid)
    {
		HeapSemid = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(HeapSemid > 0);
    }

	netpool_destroy();

	tmp = (u8_t *)SystemGetMemAddr(NETPOOL_BUF_ID);

	net_buf_start = tmp;

    netpool_create(tmp, 256, 2400);
    tmp += 256 * (2400 + NETBUF_OVERHEAD);

	net_buf_end = tmp;
	mpDebugPrint("net_buf_start %x net_buf_end %x",net_buf_start,net_buf_end);

	UsbOtgHostSetSwapBuffer2RangeEnable(net_buf_start,net_buf_end,eWhichOtg);
}

#endif

