
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include <linux/usb.h>
#include <linux/completion.h>

#include "global612.h"
#include "mpTrace.h"

#include "usbotg_ctrl.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_cdc.h"

#include "taskid.h"
#include "os.h"

#include "netusb.h"
#include "atcmd_usb.h"

#include "hal_mmcp.h"
#include "net_utility.h"

struct api_context {
	struct completion	done;
	int			status;
};

static struct list_head atcmd_list_usb; /* AT cmd request list (USB interface) */

BOOL __edge_setcmd4(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time);
BOOL __edge_setcmd2(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time);

#if HAVE_USB_MODEM > 0

extern struct qd_usb h21_usb;

int atcmd_uart_lock;

static void atcmd_api_blocking_completion(struct atmcmd_request *req);

/*
 * @note This function is for PPP only, not AT commands.
 *
 * @retval >0   actual number of bytes written.
 * @retval 0    failed
 */
int UsbWrite(int fd, char *data, int len)
{
	int r=0, actual_req_len;
	struct usb_device *udev = h21_usb.udevice;

	if (!udev)
		return 0;

	MP_DEBUG("[PPP] %s: %d", __func__, len);
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
	//Qisda 3.5G USB modem
	if (le16_to_cpu(udev->descriptor.idVendor) == HSUPA_USB_DEVICE_VID &&
		le16_to_cpu(udev->descriptor.idProduct) == HSUPA_USB_CDC_PID)
	{
#endif
	r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_DATA_OUT),
		         data, len, &actual_req_len, 1000 /* ms */);
#if (USBOTG_HOST_CDC ||  USBOTG_HOST_DATANG)
	}
	//Datang 3G USB modem
	else if(le16_to_cpu(udev->descriptor.idVendor) == TDSCDMA_USB_DEVICE_VID &&
			le16_to_cpu(udev->descriptor.idProduct) == TDSCDMA_USB_PID)
	{
		r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_DATA_OUT_DATANG),
				data, len, &actual_req_len, 1000 /* ms */);
	}
#endif

	if (r == 0)
		r = actual_req_len;
	else
		r = 0;
error:
	return r;
}

/*
 * @retval >0   actual number of bytes written.
 * @retval 0    failed
 */
int AtCmd_UsbWrite(char *data, int len)
{
	int r=0, actual_req_len;
	struct usb_device *udev = h21_usb.udevice;
	char *buf;                                  /* usb transfer buffer */

	if(((h21_usb.old_plug_type) && (!h21_usb.new_plug_type)))	//That mean 3G dongle have pluged out.
	{
		mpDebugPrint("[AtCmd_UsbWrite]3G dongle plug out!");
		errno = ENODEV;
            	return -1;
	}

	if (!udev)
		return 0;

	MP_DEBUG("[AT] %s: %s ", __func__, data);
	buf = usb_alloc_buffer(len);
	if (!buf)
	{
		goto error;
	}

	memcpy(buf, data, len);
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
	//Qisda 3.5G USB modem
	if (le16_to_cpu(udev->descriptor.idVendor) == HSUPA_USB_DEVICE_VID &&
		le16_to_cpu(udev->descriptor.idProduct) == HSUPA_USB_CDC_PID)
	{
#endif
	r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_DATA_OUT),
		         buf, len, &actual_req_len, 1000 /* ms */);
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
	}
	//Datang 3G USB modem
	else if(le16_to_cpu(udev->descriptor.idVendor) == TDSCDMA_USB_DEVICE_VID &&
			le16_to_cpu(udev->descriptor.idProduct) == TDSCDMA_USB_PID)
	{
		r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_DATA_OUT_DATANG),
			buf, len, &actual_req_len, 1000 /* ms */);
	}
#endif
	if (r == 0)
		r = actual_req_len;
	else
		r = 0;
error:
	if (buf)
		usb_free_buffer(buf);
	return r;
}

static int atcmd_submit(struct atmcmd_request *req, char *cmd, char *pattern, 
		char *data, int length, int timeout)
{
    req->cmd = cmd;
    req->rx_pattern = pattern;
    req->data = data;
    req->length = length;

    req->complete = atcmd_api_blocking_completion;
    req->timeout = timeout;

	spin_lock(&atcmd_uart_lock);
    list_add_tail(&req->list, &atcmd_list_usb);
	spin_unlock(&atcmd_uart_lock);

	return 0;
}

/*
 * sync call
 */
int AtCmd_Request_Usb(char *cmd, char *pattern, char *result, int length, int timeout)
{
	struct atmcmd_request *req;
	struct api_context ctx;
	int to;                                     /* timed out */
	int r;

	req = mpx_Zalloc(sizeof(*req));
	if (!req)
		return -1;

	init_completion(&ctx.done);
	ctx.done.udev_index = h21_usb.idx;
    ctx.status = -ENOENT;
	req->context = &ctx;

	r = atcmd_submit(req, cmd, pattern,
		         result, length, timeout /* sec */);

	if (r) {
    }

	to = wait_for_completion_timeout(&ctx.done, timeout);
	if (!to) {
		spin_lock(&atcmd_uart_lock);
		list_del(&req->list);
		spin_unlock(&atcmd_uart_lock);

		r = -ETIMEDOUT;
		goto error;
	}

	r = ctx.status;
error:
	SAFE_FREE(req);
	return r;
}

void run_atcmd(void)
{
	int ret;
	struct atmcmd_request *req;

	MP_DEBUG("%s --> ", __func__);
	spin_lock(&atcmd_uart_lock);
	while (!list_empty(&atcmd_list_usb)) {

		req = list_first_entry(&atcmd_list_usb, struct atmcmd_request, list);
		list_del_init(&req->list);
		spin_unlock(&atcmd_uart_lock);

//		atcmd_clear_pending(req);

        if (req->flags & ATCMD_NO_OK)
            ret = __edge_setcmd4(req->cmd, req->rx_pattern, req->data, req->length, req->timeout);
        else if (req->flags & ATCMD_SMS)
            ;
        else
            ret = __edge_setcmd2(req->cmd, req->rx_pattern, req->data, req->length, 60);

//        MP_DEBUG("%s: req=%p", __func__, req);

		if (ret)
			req->status = 0;                    /* OK */
		else
			req->status = -1;                   /* error */
        req->complete(req);

		spin_lock(&atcmd_uart_lock);
	}
	spin_unlock(&atcmd_uart_lock);

}

void atcmd_api_init(void)
{
	int status;

    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	MP_ASSERT(status > 0);
	atcmd_uart_lock = status;

    INIT_LIST_HEAD(&atcmd_list_usb);
}

static void atcmd_api_blocking_completion(struct atmcmd_request *req)
{
	struct api_context *ctx = req->context;

	ctx->status = req->status;
	complete(&ctx->done);
}


#endif //HAVE_USB_MODEM

// vim: :noexpandtab:
