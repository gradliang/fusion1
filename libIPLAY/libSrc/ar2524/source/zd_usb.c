/* ZD1211 USB-WLAN driver for Linux
 *
 * Copyright (C) 2005-2007 Ulrich Kunitz <kune@deine-taler.de>
 * Copyright (C) 2006-2007 Daniel Drake <dsd@gentoo.org>
 * Copyright (C) 2006-2007 Michael Wu <flamingice@sourmilk.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#define LOCAL_DEBUG_ENABLE 0

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/firmware.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/usb.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <net/mac80211.h>
#ifdef LINUX
#include <asm/unaligned.h>
#endif
#include "ndebug.h"

#include "zd_def.h"
#include "zd_mac.h"
#include "zd_usb.h"

#include "taskid.h"

static struct usb_device_id usb_ids[] = {
	/* ZD1211 */
	{ USB_DEVICE(0x0ace, 0x1211), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x07b8, 0x6001), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x126f, 0xa006), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x6891, 0xa727), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0df6, 0x9071), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0df6, 0x9075), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x157e, 0x300b), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x079b, 0x004a), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x1740, 0x2000), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x157e, 0x3204), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0586, 0x3402), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0b3b, 0x5630), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0b05, 0x170c), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x1435, 0x0711), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0586, 0x3409), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0b3b, 0x1630), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0586, 0x3401), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x14ea, 0xab13), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x13b1, 0x001e), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0586, 0x3407), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x129b, 0x1666), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x157e, 0x300a), .driver_info = DEVICE_ZD1211 },
	{ USB_DEVICE(0x0105, 0x145f), .driver_info = DEVICE_ZD1211 },
	/* ZD1211B */
	{ USB_DEVICE(0x0ace, 0x1215), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0ace, 0xb215), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x157e, 0x300d), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x079b, 0x0062), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x1582, 0x6003), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x050d, 0x705c), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x083a, 0xe506), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x083a, 0x4505), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0471, 0x1236), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x13b1, 0x0024), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0586, 0x340f), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0b05, 0x171b), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0586, 0x3410), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0baf, 0x0121), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0586, 0x3412), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0586, 0x3413), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0053, 0x5301), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0411, 0x00da), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x2019, 0x5303), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x129b, 0x1667), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0cde, 0x001a), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0586, 0x340a), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x0471, 0x1237), .driver_info = DEVICE_ZD1211B },
	{ USB_DEVICE(0x07fa, 0x1196), .driver_info = DEVICE_ZD1211B },
	/* "Driverless" devices that need ejecting */
	{ USB_DEVICE(0x0ace, 0x2011), .driver_info = DEVICE_INSTALLER },
	{ USB_DEVICE(0x0ace, 0x20ff), .driver_info = DEVICE_INSTALLER },
	{}
};

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("USB driver for devices with the ZD1211 chip.");
MODULE_AUTHOR("Ulrich Kunitz");
MODULE_AUTHOR("Daniel Drake");
MODULE_VERSION("1.0");
MODULE_DEVICE_TABLE(usb, usb_ids);

#define FW_ZD1211_PREFIX	"zd1211/zd1211_"
#define FW_ZD1211B_PREFIX	"zd1211/zd1211b_"

/* USB device initialization */
static void int_urb_complete(struct urb *urb);

extern void (*NetDeviceAttachEvent_Fn)(u16 idVendor, u16 idProduct, int attached);
extern BOOL UsbOtgCheckPlugOut(WHICH_OTG eWhichOtg);

#ifdef LINUX
static int request_fw_file(
	const struct firmware **fw, const char *name, struct device *device)
{
	int r;

	dev_dbg_f(device, "fw name %s\n", name);

	r = request_firmware(fw, name, device);
	if (r)
		dev_err(device,
		       "Could not load firmware file %s. Error number %d\n",
		       name, r);
	return r;
}
#endif

static inline u16 get_bcdDevice(const struct usb_device *udev)
{
	return le16_to_cpu(udev->descriptor.bcdDevice);
}

enum upload_code_flags {
	REBOOT = 1,
};

/* Ensures that MAX_TRANSFER_SIZE is even. */
#ifdef LINUX
#define MAX_TRANSFER_SIZE (USB_MAX_TRANSFER_SIZE & ~1)
#else
#define MAX_TRANSFER_SIZE 1024
#endif

static int upload_code(struct usb_device *udev,
	const u8 *data, size_t size, u16 code_offset, int flags)
{
	u8 *p;
	int r;

	/* USB request blocks need "kmalloced" buffers.
	 */
	p = kmalloc(MAX_TRANSFER_SIZE, GFP_KERNEL);
	if (!p) {
		dev_err(&udev->dev, "out of memory\n");
		r = -ENOMEM;
		goto error;
	}

	size &= ~1;
	while (size > 0) {
		size_t transfer_size = size <= MAX_TRANSFER_SIZE ?
			size : MAX_TRANSFER_SIZE;

		dev_dbg_f(&udev->dev, "transfer size %u\n", transfer_size);
			memcpy(p, data, transfer_size);
		r = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			USB_REQ_FIRMWARE_DOWNLOAD,
			USB_DIR_OUT | USB_TYPE_VENDOR,
			code_offset, 0, p, transfer_size, 1000 /* ms */);
		if (r < 0) {
			dev_err(&udev->dev,
			       "USB control request for firmware upload"
			       " failed. Error number %d\n", r);
			goto error;
		}
		transfer_size = r & ~1;

		size -= transfer_size;
		data += transfer_size;
		code_offset += transfer_size/sizeof(u16);
	}

	if (flags & REBOOT) {
		u8 ret;

		/* Use "DMA-aware" buffer. */
		r = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			USB_REQ_FIRMWARE_CONFIRM,
			USB_DIR_IN | USB_TYPE_VENDOR,
			0, 0, p, sizeof(ret), 5000 /* ms */);
		if (r != sizeof(ret)) {
			dev_err(&udev->dev,
				"control request firmeware confirmation failed."
				" Return value %d\n", r);
			if (r >= 0)
				r = -ENODEV;
			goto error;
		}
		ret = p[0];
		if (ret & 0x80) {
			dev_err(&udev->dev,
				"Internal error while downloading."
				" Firmware confirm return value %#04x\n",
				(unsigned int)ret);
			r = -ENODEV;
			goto error;
		}
		dev_dbg_f(&udev->dev, "firmware confirm return value %#04x\n",
			(unsigned int)ret);
	}

	r = 0;
error:
	kfree(p);
	return r;
}

static u16 get_word(const void *data, u16 offset)
{
	const __le16 *p = data;
	return le16_to_cpu(p[offset]);
}

#ifdef LINUX
static char *get_fw_name(struct zd_usb *usb, char *buffer, size_t size,
	               const char* postfix)
{
	scnprintf(buffer, size, "%s%s",
		usb->is_zd1211b ?
			FW_ZD1211B_PREFIX : FW_ZD1211_PREFIX,
		postfix);
	return buffer;
}
#endif

extern const u8 WS11Ur[];
extern const size_t WS11Ur_length;
static int handle_version_mismatch(struct zd_usb *usb,
	const struct firmware *ub_fw)
{
	struct usb_device *udev = zd_usb_to_usbdev(usb);
	const struct firmware *ur_fw = NULL;
	int offset;
	int r = 0;
#ifdef LINUX
	char fw_name[128];

	r = request_fw_file(&ur_fw,
		get_fw_name(usb, fw_name, sizeof(fw_name), "ur"),
		&udev->dev);
	if (r)
		goto error;
#else
    ur_fw->data = WS11Ur;
    ur_fw->size = WS11Ur_length;
//    NetPacketDump(ur_fw->data, 64);
    printk(KERN_ERR "USB FIRMWARE: Ur len=%d\n", WS11Ur_length);
#endif

	r = upload_code(udev, ur_fw->data, ur_fw->size, FW_START, REBOOT);
	if (r)
		goto error;

	offset = (E2P_BOOT_CODE_OFFSET * sizeof(u16));
	r = upload_code(udev, ub_fw->data + offset, ub_fw->size - offset,
		E2P_START + E2P_BOOT_CODE_OFFSET, REBOOT);

	/* At this point, the vendor driver downloads the whole firmware
	 * image, hacks around with version IDs, and uploads it again,
	 * completely overwriting the boot code. We do not do this here as
	 * it is not required on any tested devices, and it is suspected to
	 * cause problems. */
error:
#ifdef LINUX
	release_firmware(ur_fw);
#endif
	return r;
}

extern const u8 WS11Ub[];
extern const size_t WS11Ub_length;
extern const u8 WS11UPhr[];
extern const size_t WS11UPhr_length;
static int upload_firmware(struct zd_usb *usb)
{
	int r;
	u16 fw_bcdDevice;
	u16 bcdDevice;
	struct usb_device *udev = zd_usb_to_usbdev(usb);
	const struct firmware *ub_fw = NULL;
	const struct firmware *uph_fw = NULL;
#ifdef LINUX
	char fw_name[128];
#endif

    MP_FUNCTION_ENTER();
	bcdDevice = get_bcdDevice(udev);

#ifdef LINUX
	r = request_fw_file(&ub_fw,
		get_fw_name(usb, fw_name, sizeof(fw_name), "ub"),
		&udev->dev);
	if (r)
		goto error;
#else
    ub_fw->data = WS11Ub;
    ub_fw->size = WS11Ub_length;
    printk(KERN_ERR "USB FIRMWARE: Ub len=%d\n", WS11Ub_length);
#endif

	fw_bcdDevice = get_word(ub_fw->data, E2P_DATA_OFFSET);

	if (fw_bcdDevice != bcdDevice) {
		dev_info(&udev->dev,
			"firmware version %#06x and device bootcode version "
			"%#06x differ\n", fw_bcdDevice, bcdDevice);
		if (bcdDevice <= 0x4313)
			dev_warn(&udev->dev, "device has old bootcode, please "
				"report success or failure\n");

		r = handle_version_mismatch(usb, ub_fw);
		if (r)
			goto error;
	} else {
		dev_dbg_f(&udev->dev,
			"firmware device id %#06x is equal to the "
			"actual device id\n", fw_bcdDevice);
	}


#ifdef LINUX
	r = request_fw_file(&uph_fw,
		get_fw_name(usb, fw_name, sizeof(fw_name), "uphr"),
		&udev->dev);
	if (r)
		goto error;
#else
    uph_fw->data = WS11UPhr;
    uph_fw->size = WS11UPhr_length;
    printk(KERN_ERR "USB FIRMWARE: UPhr len=%d\n", WS11UPhr_length);
#endif

	r = upload_code(udev, uph_fw->data, uph_fw->size, FW_START, REBOOT);
	if (r) {
		dev_err(&udev->dev,
			"Could not upload firmware code uph. Error number %d\n",
			r);
	}

	/* FALL-THROUGH */
    MP_FUNCTION_EXIT();
error:
#ifdef LINUX
	release_firmware(ub_fw);
	release_firmware(uph_fw);
#endif
    MP_FUNCTION_EXIT();
	return r;
}

/* Read data from device address space using "firmware interface" which does
 * not require firmware to be loaded. */
int zd_usb_read_fw(struct zd_usb *usb, zd_addr_t addr, u8 *data, u16 len)
{
	int r;
	struct usb_device *udev = zd_usb_to_usbdev(usb);
	u8 *buf;

    MP_FUNCTION_ENTER();
	/* Use "DMA-aware" buffer. */
	buf = kmalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	r = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
		USB_REQ_FIRMWARE_READ_DATA, USB_DIR_IN | 0x40, addr, 0,
		buf, len, 5000);
	if (r < 0) {
		dev_err(&udev->dev,
			"read over firmware interface failed: %d\n", r);
        MP_FUNCTION_EXIT();
		goto exit;
	} else if (r != len) {
		dev_err(&udev->dev,
			"incomplete read over firmware interface: %d/%d\n",
			r, len);
        MP_FUNCTION_EXIT();
		r = -EIO;
		goto exit;
	}
	r = 0;
		mmcp_memcpy(data, buf, len);
exit:
	kfree(buf);
	return r;
}

#define urb_dev(urb) (&(urb)->dev->dev)

static inline void handle_regs_int(struct urb *urb)
{
	struct zd_usb *usb = urb->context;
	struct zd_usb_interrupt *intr = &usb->intr;
	int len;
	u16 int_num;

//    MP_FUNCTION_ENTER();
	ZD_ASSERT(in_interrupt());
	spin_lock(&intr->lock);

	int_num = le16_to_cpu(*(__le16 *)(urb->transfer_buffer+2));
	if (int_num == CR_INTERRUPT) {
		struct zd_mac *mac = zd_hw_mac(zd_usb_to_hw(urb->context));
		mmcp_memcpy(&mac->intr_buffer, urb->transfer_buffer,
				USB_MAX_EP_INT_BUFFER);
		schedule_work(&mac->process_intr);
	} else if (intr->read_regs_enabled) {
		intr->read_regs.length = len = urb->actual_length;

		if (len > sizeof(intr->read_regs.buffer))
			len = sizeof(intr->read_regs.buffer);
		
		mmcp_memcpy(intr->read_regs.buffer, urb->transfer_buffer, len);
		
		intr->read_regs_enabled = 0;
		complete(&intr->read_regs.completion);
		goto out;
	}

//    MP_FUNCTION_EXIT();
out:
//    MP_FUNCTION_EXIT();
	spin_unlock(&intr->lock);
}

static void int_urb_complete(struct urb *urb)
{
	int r;
	struct usb_int_header *hdr;

//    MP_FUNCTION_ENTER();
    MP_ASSERT(!urb->status);
	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
	case -EINVAL:
	case -ENODEV:
	case -ENOENT:
	case -ECONNRESET:
	case -EPIPE:
		goto kfree;
	default:
		goto resubmit;
	}

	if (urb->actual_length < sizeof(hdr)) {
		dev_dbg_f(urb_dev(urb), "error: urb %p to small\n", urb);
		goto resubmit;
	}

	hdr = urb->transfer_buffer;
//    MP_ALERT("int_urb_complete: urb->transfer_buffer=%p", hdr); 
//    NetPacketDump(hdr, 12);
	if (hdr->type != USB_INT_TYPE) {
		dev_dbg_f(urb_dev(urb), "error: urb %p wrong type\n", urb);
		goto resubmit;
	}

//    mpDebugPrint("%s: hdr->id=%x", __func__, hdr->id);
	switch (hdr->id) {
	case USB_INT_ID_REGS:
		handle_regs_int(urb);
		break;
	case USB_INT_ID_RETRY_FAILED:
#if 0
        NetPacketDump(hdr, 64);
        MP_ASSERT(0);
#endif
		zd_mac_tx_failed(zd_usb_to_hw(urb->context));
		break;
	default:
		dev_dbg_f(urb_dev(urb), "error: urb %p unknown id %x\n", urb,
			(unsigned int)hdr->id);
		goto resubmit;
	}

resubmit:
	r = usb_submit_urb(urb, GFP_ATOMIC);
	if (r) {
		dev_dbg_f(urb_dev(urb), "resubmit urb %p\n", urb);
		goto kfree;
	}
//    MP_FUNCTION_EXIT();
	return;
kfree:
    MP_ASSERT(0);
    MP_FUNCTION_EXIT();
#ifdef LINUX
	kfree(urb->transfer_buffer);
#else
	usb_free_buffer(urb->transfer_buffer);
#endif
}

static inline int int_urb_interval(struct usb_device *udev)
{
	switch (udev->speed) {
	case USB_SPEED_HIGH:
		return 4;
	case USB_SPEED_LOW:
		return 10;
	case USB_SPEED_FULL:
	default:
		return 1;
	}
}

static inline int usb_int_enabled(struct zd_usb *usb)
{
	unsigned long flags;
	struct zd_usb_interrupt *intr = &usb->intr;
	struct urb *urb;

	spin_lock_irqsave(&intr->lock, flags);
	urb = intr->urb;
	spin_unlock_irqrestore(&intr->lock, flags);
	return urb != NULL;
}

//static char int_buf[USB_MAX_EP_INT_BUFFER];
int zd_usb_enable_int(struct zd_usb *usb)
{
	int r;
	struct usb_device *udev;
	struct zd_usb_interrupt *intr = &usb->intr;
	void *transfer_buffer = NULL;
	struct urb *urb;

	dev_dbg_f(zd_usb_dev(usb), "\n");

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		r = -ENOMEM;
        MP_ASSERT(0);
		goto out;
	}

	ZD_ASSERT(!irqs_disabled());
	spin_lock_irq(&intr->lock);
	if (intr->urb) {
		spin_unlock_irq(&intr->lock);
		r = 0;
		goto error_free_urb;
	}
	intr->urb = urb;
    mpDebugPrint("zd_usb_enable_int: usb=%p",usb);
    mpDebugPrint("zd_usb_enable_int: usb->intr.urb=%p",usb->intr.urb);
	spin_unlock_irq(&intr->lock);

	/* TODO: make it a DMA buffer */
	r = -ENOMEM;
#ifdef LINUX
	transfer_buffer = kmalloc(USB_MAX_EP_INT_BUFFER, GFP_KERNEL);
#else
	transfer_buffer = usb_alloc_buffer(USB_MAX_EP_INT_BUFFER);
//	transfer_buffer = allo_mem(USB_MAX_EP_INT_BUFFER);
//	transfer_buffer = int_buf;
    mpDebugPrint("zd_usb_enable_int: transfer_buffer=%p",transfer_buffer);
#endif
	if (!transfer_buffer) {
		dev_dbg_f(zd_usb_dev(usb),
			"couldn't allocate transfer_buffer\n");
		goto error_set_urb_null;
	}

	udev = zd_usb_to_usbdev(usb);
	usb_fill_int_urb(urb, udev, usb_rcvintpipe(udev, EP_INT_IN),
			 transfer_buffer, USB_MAX_EP_INT_BUFFER,
			 int_urb_complete, usb,
			 intr->interval);

	dev_dbg_f(zd_usb_dev(usb), "submit urb %p\n", intr->urb);
	r = usb_submit_urb(urb, GFP_KERNEL);
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
			 "Couldn't submit urb. Error number %d\n", r);
		goto error;
	}

	return 0;
error:
#ifdef LINUX
	kfree(transfer_buffer);
#else
	usb_free_buffer(transfer_buffer);
#endif
error_set_urb_null:
	spin_lock_irq(&intr->lock);
	intr->urb = NULL;
	spin_unlock_irq(&intr->lock);
error_free_urb:
	usb_free_urb(urb);
out:
    MP_ASSERT(0);
	return r;
}

void zd_usb_disable_int(struct zd_usb *usb)
{
	unsigned long flags;
	struct zd_usb_interrupt *intr = &usb->intr;
	struct urb *urb;

	spin_lock_irqsave(&intr->lock, flags);
	urb = intr->urb;
	if (!urb) {
		spin_unlock_irqrestore(&intr->lock, flags);
		return;
	}
	intr->urb = NULL;
	spin_unlock_irqrestore(&intr->lock, flags);

	usb_kill_urb(urb);
#ifdef PLATFORM_MPIXEL
	if (urb->transfer_buffer)
    {
        usb_free_buffer(urb->transfer_buffer);
        urb->transfer_buffer = NULL;
    }
#endif
	dev_dbg_f(zd_usb_dev(usb), "urb %p killed\n", urb);
	usb_free_urb(urb);
}

static void handle_rx_packet(struct zd_usb *usb, const u8 *buffer,
			     unsigned int length)
{
	int i;
	const struct rx_length_info *length_info;
#ifdef PLATFORM_MPIXEL
	struct rx_length_info linfo_buf;
#endif

//    MP_TRACE_LINE();
	if (length < sizeof(struct rx_length_info)) {
		/* It's not a complete packet anyhow. */
		return;
	}
	length_info = (struct rx_length_info *)
		(buffer + length - sizeof(struct rx_length_info));
//	MP_VALIDATE_POINTER(length_info);

	/* It might be that three frames are merged into a single URB
	 * transaction. We have to check for the length info tag.
	 *
	 * While testing we discovered that length_info might be unaligned,
	 * because if USB transactions are merged, the last packet will not
	 * be padded. Unaligned access might also happen if the length_info
	 * structure is not present.
	 */
#ifdef PLATFORM_MPIXEL
    if ((unsigned long)length_info & 1)         /* handle unaligned access */
    {
//        mpDebugPrint("%s: length_info=%p\n", __func__, length_info);
        memcpy(&linfo_buf, length_info, sizeof linfo_buf);
        length_info = &linfo_buf;
//        mpDebugPrint("%s: length_info(new)=%p\n", __func__, length_info);
        MP_TRACE_LINE();
//        mpDebugPrint("%s: tag=%04x\n", __func__, length_info->tag);
    }
#endif
	MP_TRACE_LINE();
	if (get_unaligned_le16(&length_info->tag) == RX_LENGTH_INFO_TAG)
	{
        MP_TRACE_LINE();
		unsigned int l, k, n;
		for (i = 0, l = 0;; i++) {
            MP_TRACE_LINE();
			k = get_unaligned_le16(&length_info->length[i]);
			if (k == 0)
				return;
			n = l+k;
			if (n > length)
				return;
			zd_mac_rx(zd_usb_to_hw(usb), buffer+l, k);
			if (i >= 2)
				return;
			l = (n+3) & ~3;
		}
	} else {
            MP_TRACE_LINE();
		zd_mac_rx(zd_usb_to_hw(usb), buffer, length);
	}
//            MP_TRACE_LINE();
}

static void rx_urb_complete(struct urb *urb)
{
	struct zd_usb *usb;
	struct zd_usb_rx *rx;
	const u8 *buffer;
	unsigned int length;

	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
	case -EINVAL:
	case -ENODEV:
	case -ENOENT:
	case -ECONNRESET:
	case -EPIPE:
		dev_dbg_f(urb_dev(urb), "error %d\n", urb->status);
		return;
	default:
		dev_dbg_f(urb_dev(urb), "error %d\n", urb->status);
		goto resubmit;
	}

	buffer = urb->transfer_buffer;
	length = urb->actual_length;
	usb = urb->context;
	rx = &usb->rx;

	if (length%rx->usb_packet_size > rx->usb_packet_size-4) {
		/* If there is an old first fragment, we don't care. */
		dev_dbg_f(urb_dev(urb), "*** first fragment ***\n");
		ZD_ASSERT(length <= ARRAY_SIZE(rx->fragment));
		spin_lock(&rx->lock);
		
		mmcp_memcpy(rx->fragment, buffer, length);
		rx->fragment_length = length;
		spin_unlock(&rx->lock);
		goto resubmit;
	}

//	MP_TRACE_LINE();
	spin_lock(&rx->lock);
	if (rx->fragment_length > 0) {
		/* We are on a second fragment, we believe */
		ZD_ASSERT(length + rx->fragment_length <=
			  ARRAY_SIZE(rx->fragment));
		dev_dbg_f(urb_dev(urb), "*** second fragment ***\n");
		mmcp_memcpy(rx->fragment+rx->fragment_length, buffer, length);
		handle_rx_packet(usb, rx->fragment,
			         rx->fragment_length + length);
		rx->fragment_length = 0;
		spin_unlock(&rx->lock);
	} else {
		spin_unlock(&rx->lock);
//        MP_TRACE_LINE();
		handle_rx_packet(usb, buffer, length);
	}

//	MP_TRACE_LINE();
resubmit:
	MP_TRACE_LINE();
	usb_submit_urb(urb, GFP_ATOMIC);
}

static struct urb *alloc_rx_urb(struct zd_usb *usb)
{
	struct usb_device *udev = zd_usb_to_usbdev(usb);
	struct urb *urb;
	void *buffer;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return NULL;
	buffer = usb_buffer_alloc(udev, USB_MAX_RX_SIZE, GFP_KERNEL,
		                  &urb->transfer_dma);
	if (!buffer) {
		usb_free_urb(urb);
		return NULL;
	}

	usb_fill_bulk_urb(urb, udev, usb_rcvbulkpipe(udev, EP_DATA_IN),
		          buffer, USB_MAX_RX_SIZE,
			  rx_urb_complete, usb);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	return urb;
}

static void free_rx_urb(struct urb *urb)
{
	if (!urb)
		return;
	usb_buffer_free(urb->dev, urb->transfer_buffer_length,
		        urb->transfer_buffer, urb->transfer_dma);
	usb_free_urb(urb);
}

int zd_usb_enable_rx(struct zd_usb *usb)
{
	int i, r;
	struct zd_usb_rx *rx = &usb->rx;
	struct urb **urbs;

    MP_FUNCTION_ENTER();
	dev_dbg_f(zd_usb_dev(usb), "\n");

	r = -ENOMEM;
	urbs = kcalloc(RX_URBS_COUNT, sizeof(struct urb *), GFP_KERNEL);
	if (!urbs)
		goto error;
	for (i = 0; i < RX_URBS_COUNT; i++) {
		urbs[i] = alloc_rx_urb(usb);
		if (!urbs[i])
			goto error;
	}

	ZD_ASSERT(!irqs_disabled());
	spin_lock_irq(&rx->lock);
	if (rx->urbs) {
		spin_unlock_irq(&rx->lock);
		r = 0;
		goto error;
	}
	rx->urbs = urbs;
	rx->urbs_count = RX_URBS_COUNT;
	spin_unlock_irq(&rx->lock);

	urb_disable();
	for (i = 0; i < RX_URBS_COUNT; i++) {
		r = usb_submit_urb(urbs[i], GFP_KERNEL);
		if (r)
		{
			goto error_submit;
	}
	}
	urb_enable();

	return 0;
error_submit:
	for (i = 0; i < RX_URBS_COUNT; i++) {
		usb_kill_urb(urbs[i]);
	}
	spin_lock_irq(&rx->lock);
	rx->urbs = NULL;
	rx->urbs_count = 0;
	spin_unlock_irq(&rx->lock);
error:
	if (urbs) {
		for (i = 0; i < RX_URBS_COUNT; i++)
			free_rx_urb(urbs[i]);
	}
    MP_FUNCTION_EXIT();
	return r;
}

void zd_usb_disable_rx(struct zd_usb *usb)
{
	int i;
	unsigned long flags;
	struct urb **urbs;
	unsigned int count;
	struct zd_usb_rx *rx = &usb->rx;

	spin_lock_irqsave(&rx->lock, flags);
	urbs = rx->urbs;
	count = rx->urbs_count;
	spin_unlock_irqrestore(&rx->lock, flags);
	if (!urbs)
		return;

	for (i = 0; i < count; i++) {
		usb_kill_urb(urbs[i]);
		free_rx_urb(urbs[i]);
	}
	kfree(urbs);

	spin_lock_irqsave(&rx->lock, flags);
	rx->urbs = NULL;
	rx->urbs_count = 0;
	spin_unlock_irqrestore(&rx->lock, flags);
}

/**
 * zd_usb_disable_tx - disable transmission
 * @usb: the zd1211rw-private USB structure
 *
 * Frees all URBs in the free list and marks the transmission as disabled.
 */
void zd_usb_disable_tx(struct zd_usb *usb)
{
	struct zd_usb_tx *tx = &usb->tx;
	unsigned long flags;
	struct list_head *pos, *n;

	spin_lock_irqsave(&tx->lock, flags);
	list_for_each_safe(pos, n, &tx->free_urb_list) {
		list_del(pos);
		usb_free_urb(list_entry(pos, struct urb, urb_list));
	}
	tx->enabled = 0;
	tx->submitted_urbs = 0;
	/* The stopped state is ignored, relying on ieee80211_wake_queues()
	 * in a potentionally following zd_usb_enable_tx().
	 */
	spin_unlock_irqrestore(&tx->lock, flags);
}

/**
 * zd_usb_enable_tx - enables transmission
 * @usb: a &struct zd_usb pointer
 *
 * This function enables transmission and prepares the &zd_usb_tx data
 * structure.
 */
void zd_usb_enable_tx(struct zd_usb *usb)
{
	unsigned long flags;
	struct zd_usb_tx *tx = &usb->tx;

	spin_lock_irqsave(&tx->lock, flags);
	tx->enabled = 1;
	tx->submitted_urbs = 0;
	ieee80211_wake_queues(zd_usb_to_hw(usb));
	tx->stopped = 0;
	spin_unlock_irqrestore(&tx->lock, flags);
}

/**
 * alloc_tx_urb - provides an tx URB
 * @usb: a &struct zd_usb pointer
 *
 * Allocates a new URB. If possible takes the urb from the free list in
 * usb->tx.
 */
static struct urb *alloc_tx_urb(struct zd_usb *usb)
{
	struct zd_usb_tx *tx = &usb->tx;
	unsigned long flags;
	struct list_head *entry;
	struct urb *urb;

	spin_lock_irqsave(&tx->lock, flags);
	if (list_empty(&tx->free_urb_list)) {
		urb = usb_alloc_urb(0, GFP_ATOMIC);
		goto out;
	}
	entry = tx->free_urb_list.next;
	list_del(entry);
	urb = list_entry(entry, struct urb, urb_list);
out:
	spin_unlock_irqrestore(&tx->lock, flags);
	return urb;
}

/**
 * free_tx_urb - frees a used tx URB
 * @usb: a &struct zd_usb pointer
 * @urb: URB to be freed
 *
 * Frees the the transmission URB, which means to put it on the free URB
 * list.
 */
static void free_tx_urb(struct zd_usb *usb, struct urb *urb)
{
	struct zd_usb_tx *tx = &usb->tx;
	unsigned long flags;

	spin_lock_irqsave(&tx->lock, flags);
	if (!tx->enabled) {
		usb_free_urb(urb);
		goto out;
	}
	list_add(&urb->urb_list, &tx->free_urb_list);
out:
	spin_unlock_irqrestore(&tx->lock, flags);
}

static void tx_dec_submitted_urbs(struct zd_usb *usb)
{
	struct zd_usb_tx *tx = &usb->tx;
	unsigned long flags;

	spin_lock_irqsave(&tx->lock, flags);
	--tx->submitted_urbs;
	if (tx->stopped && tx->submitted_urbs <= ZD_USB_TX_LOW) {
		ieee80211_wake_queues(zd_usb_to_hw(usb));
		tx->stopped = 0;
	}
	spin_unlock_irqrestore(&tx->lock, flags);
}

static void tx_inc_submitted_urbs(struct zd_usb *usb)
{
	struct zd_usb_tx *tx = &usb->tx;
	unsigned long flags;

	spin_lock_irqsave(&tx->lock, flags);
	++tx->submitted_urbs;
	if (!tx->stopped && tx->submitted_urbs > ZD_USB_TX_HIGH) {
		ieee80211_stop_queues(zd_usb_to_hw(usb));
		tx->stopped = 1;
	}
	spin_unlock_irqrestore(&tx->lock, flags);
}

/**
 * tx_urb_complete - completes the execution of an URB
 * @urb: a URB
 *
 * This function is called if the URB has been transferred to a device or an
 * error has happened.
 */
static void tx_urb_complete(struct urb *urb)
{
	int r;
	struct sk_buff *skb;
	struct ieee80211_tx_info *info;
	struct zd_usb *usb;

//	MP_ASSERT(!urb->status);
	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
	case -EINVAL:
	case -ENODEV:
	case -ENOENT:
	case -ECONNRESET:
	case -EPIPE:
		dev_dbg_f(urb_dev(urb), "urb %p error %d\n", urb, urb->status);
		break;
	default:
		dev_dbg_f(urb_dev(urb), "urb %p error %d\n", urb, urb->status);
		goto resubmit;
	}
free_urb:
	skb = (struct sk_buff *)urb->context;
	/*
	 * grab 'usb' pointer before handing off the skb (since
	 * it might be freed by zd_mac_tx_to_dev or mac80211)
	 */
	info = IEEE80211_SKB_CB(skb);
	usb = &zd_hw_mac(info->driver_data[0])->chip.usb;
	zd_mac_tx_to_dev(skb, urb->status);
	free_tx_urb(usb, urb);
	tx_dec_submitted_urbs(usb);
	return;
resubmit:
	r = usb_submit_urb(urb, GFP_ATOMIC);
	if (r) {
		dev_dbg_f(urb_dev(urb), "error resubmit urb %p %d\n", urb, r);
		goto free_urb;
	}
}

/**
 * zd_usb_tx: initiates transfer of a frame of the device
 *
 * @usb: the zd1211rw-private USB structure
 * @skb: a &struct sk_buff pointer
 *
 * This function tranmits a frame to the device. It doesn't wait for
 * completion. The frame must contain the control set and have all the
 * control set information available.
 *
 * The function returns 0 if the transfer has been successfully initiated.
 */
int zd_usb_tx(struct zd_usb *usb, struct sk_buff *skb)
{
	int r;
	struct usb_device *udev = zd_usb_to_usbdev(usb);
	struct urb *urb;
#ifdef PLATFORM_MPIXEL
    struct sk_buff *origskb = NULL;
#endif

	urb = alloc_tx_urb(usb);
	if (!urb) {
		r = -ENOMEM;
		goto out;
	}

#ifdef PLATFORM_MPIXEL
    /* ---------- handle unaligned buffer ---------- */
    u32 diff;
    if (diff = (u32)skb->data & 0x3)
    {
        struct sk_buff *n;
        u32 needed_headroom;
        MP_DEBUG("%s: diff=%d", __func__, diff);
        needed_headroom = skb_headroom(skb) + 4 - diff;
//    __asm("break 100");
		n = skb_copy_expand(skb, needed_headroom, 0, GFP_ATOMIC);
        MP_ASSERT(n);
        if (!n)
            goto error;
        origskb = skb;
        skb = n;
    }
#endif
	usb_fill_bulk_urb(urb, udev, usb_sndbulkpipe(udev, EP_DATA_OUT),
		          skb->data, skb->len, tx_urb_complete, skb);

	r = usb_submit_urb(urb, GFP_ATOMIC);
	if (r)
		goto error;
	tx_inc_submitted_urbs(usb);
    if (origskb)
		dev_kfree_skb(origskb);
//    MP_FUNCTION_EXIT();
	return 0;
error:
    MP_FUNCTION_EXIT();
	free_tx_urb(usb, urb);
out:
    MP_FUNCTION_EXIT();
	return r;
}

static inline void init_usb_interrupt(struct zd_usb *usb)
{
	struct zd_usb_interrupt *intr = &usb->intr;

#ifdef LINUX
	spin_lock_init(&intr->lock);
#else
    extern int intr_lock_global;
	intr->lock = intr_lock_global;
#endif
	intr->interval = int_urb_interval(zd_usb_to_usbdev(usb));
	init_completion(&intr->read_regs.completion);
	intr->read_regs.completion.udev_index = zd_usb_to_usbdev(usb)->devnum;
	intr->read_regs.cr_int_addr = cpu_to_le16((u16)CR_INTERRUPT);
}

static inline void init_usb_rx(struct zd_usb *usb)
{
	struct zd_usb_rx *rx = &usb->rx;
#ifdef LINUX
	spin_lock_init(&rx->lock);
#else
    extern int rx_lock_global;
	rx->lock = rx_lock_global;
#endif
	if (interface_to_usbdev(usb->intf)->speed == USB_SPEED_HIGH) {
		rx->usb_packet_size = 512;
	} else {
		rx->usb_packet_size = 64;
	}
	ZD_ASSERT(rx->fragment_length == 0);
	MP_ALERT("<-- %s: pkt sz=%d\n", __func__, rx->usb_packet_size);
}

static inline void init_usb_tx(struct zd_usb *usb)
{
	struct zd_usb_tx *tx = &usb->tx;
#ifdef LINUX
	spin_lock_init(&tx->lock);
#else
    extern int tx_lock_global;
	tx->lock = tx_lock_global;
#endif
	tx->enabled = 0;
	tx->stopped = 0;
	INIT_LIST_HEAD(&tx->free_urb_list);
	tx->submitted_urbs = 0;
}

void zd_usb_init(struct zd_usb *usb, struct ieee80211_hw *hw,
	         struct usb_interface *intf)
{
	memset(usb, 0, sizeof(*usb));
	usb->intf = usb_get_intf(intf);
	usb_set_intfdata(usb->intf, hw);
	init_usb_interrupt(usb);
	init_usb_tx(usb);
	init_usb_rx(usb);
}

void zd_usb_clear(struct zd_usb *usb)
{
	usb_set_intfdata(usb->intf, NULL);
#ifdef LINUX
	usb_put_intf(usb->intf);
#endif
	ZD_MEMCLEAR(usb, sizeof(*usb));
	/* FIXME: usb_interrupt, usb_tx, usb_rx? */
}

static const char *speed(enum usb_device_speed speed)
{
	switch (speed) {
	case USB_SPEED_LOW:
		return "low";
	case USB_SPEED_FULL:
		return "full";
	case USB_SPEED_HIGH:
		return "high";
	default:
		return "unknown speed";
	}
}

static int scnprint_id(struct usb_device *udev, char *buffer, size_t size)
{
	return scnprintf(buffer, size, "%04hx:%04hx v%04hx %s",
		le16_to_cpu(udev->descriptor.idVendor),
		le16_to_cpu(udev->descriptor.idProduct),
		get_bcdDevice(udev),
		speed(udev->speed));
}

int zd_usb_scnprint_id(struct zd_usb *usb, char *buffer, size_t size)
{
	struct usb_device *udev = interface_to_usbdev(usb->intf);
	return scnprint_id(udev, buffer, size);
}

#ifdef DEBUG
static void print_id(struct usb_device *udev)
{
	char buffer[40];

	scnprint_id(udev, buffer, sizeof(buffer));
	buffer[sizeof(buffer)-1] = 0;
	dev_dbg_f(&udev->dev, "%s\n", buffer);
}
#else
#define print_id(udev) do { } while (0)
#endif

#ifdef LINUX
static int eject_installer(struct usb_interface *intf)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_host_interface *iface_desc = &intf->altsetting[0];
	struct usb_endpoint_descriptor *endpoint;
	unsigned char *cmd;
	u8 bulk_out_ep;
	int r;

	/* Find bulk out endpoint */
	endpoint = &iface_desc->endpoint[1].desc;
	if ((endpoint->bEndpointAddress & USB_TYPE_MASK) == USB_DIR_OUT &&
	    (endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
	    USB_ENDPOINT_XFER_BULK) {
		bulk_out_ep = endpoint->bEndpointAddress;
	} else {
		dev_err(&udev->dev,
			"zd1211rw: Could not find bulk out endpoint\n");
		return -ENODEV;
	}

	cmd = kzalloc(31, GFP_KERNEL);
	if (cmd == NULL)
		return -ENODEV;

	/* USB bulk command block */
	cmd[0] = 0x55;	/* bulk command signature */
	cmd[1] = 0x53;	/* bulk command signature */
	cmd[2] = 0x42;	/* bulk command signature */
	cmd[3] = 0x43;	/* bulk command signature */
	cmd[14] = 6;	/* command length */

	cmd[15] = 0x1b;	/* SCSI command: START STOP UNIT */
	cmd[19] = 0x2;	/* eject disc */

	dev_info(&udev->dev, "Ejecting virtual installer media...\n");
	r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, bulk_out_ep),
		cmd, 31, NULL, 2000);
	kfree(cmd);
	if (r)
		return r;

	/* At this point, the device disconnects and reconnects with the real
	 * ID numbers. */

	usb_set_intfdata(intf, NULL);
	return 0;
}
#endif

int zd_usb_init_hw(struct zd_usb *usb)
{
	int r;
	struct zd_mac *mac = zd_usb_to_mac(usb);

	dev_dbg_f(zd_usb_dev(usb), "\n");

	r = upload_firmware(usb);
	if (r) {
		dev_err(zd_usb_dev(usb),
		       "couldn't load firmware. Error number %d\n", r);
		return r;
	}

#ifdef PLATFORM_MPIXEL
    msleep(10);                                 /* wait for a while */
#endif

#if 1
	r = usb_reset_configuration(zd_usb_to_usbdev(usb));
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
			"couldn't reset configuration. Error number %d\n", r);
		return r;
	}
#endif
//    while (1)
//    {
//        msleep(10);                                 /* wait for a while */
//    }


	r = zd_mac_init_hw(mac->hw);
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
		         "couldn't initialize mac. Error number %d\n", r);
		return r;
	}

	usb->initialized = 1;
	return 0;
}

static int probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	int r;
#ifdef LINUX
	struct usb_device *udev = interface_to_usbdev(intf);
#else
	struct usb_device *udev = interface_to_usbdev(intf);
//	struct usb_device *udev = dev_global;
#endif
	struct zd_usb *usb;
	struct ieee80211_hw *hw = NULL;
	struct net_device *netdev = NULL;

	mpDebugPrint("--> %s\n", __func__);

	print_id(udev);

#ifdef LINUX
	if (id->driver_info & DEVICE_INSTALLER)
		return eject_installer(intf);
#endif

	switch (udev->speed) {
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
	case USB_SPEED_HIGH:
		break;
	default:
		dev_dbg_f(&intf->dev, "Unknown USB speed\n");
		r = -ENODEV;
		goto error;
	}

	r = usb_reset_device(udev);
	if (r) {
		dev_err(&intf->dev,
			"couldn't reset usb device. Error number %d\n", r);
		goto error;
	}

	hw = zd_mac_alloc_hw(intf);
	if (hw == NULL) {
		r = -ENOMEM;
		goto error;
	}

	usb = &zd_hw_mac(hw)->chip.usb;
#ifdef LINUX
	usb->is_zd1211b = (id->driver_info == DEVICE_ZD1211B) != 0;
#else
	usb->is_zd1211b = TRUE; // TODO
#endif

	r = zd_mac_preinit_hw(hw);
	if (r) {
		dev_dbg_f(&intf->dev,
		         "couldn't initialize mac. Error number %d\n", r);
		goto error;
	}

#ifdef PLATFORM_MPIXEL
	hw->rate_control_algorithm = "pid";
#endif
	r = ieee80211_register_hw(hw);
	if (r) {
		dev_dbg_f(&intf->dev,
			 "couldn't register device. Error number %d\n", r);
		goto error;
	}

	mpDebugPrint("<-- %s %d\n", __func__, __LINE__);
	dev_dbg_f(&intf->dev, "successful\n");
	dev_info(&intf->dev, "%s\n", wiphy_name(hw->wiphy));
	return 0;
error:
	mpDebugPrint("<-- %s %d\n", __func__, __LINE__);
	usb_reset_device(interface_to_usbdev(intf));
	if (hw) {
		zd_mac_clear(zd_hw_mac(hw));
		ieee80211_free_hw(hw);
	}
	return r;
}

extern struct net_device   *netdev_global;
struct usb_interface *getUsbInterface();
static int __init usb_init(void);
int dev_open(struct net_device *dev);
int mp_usb_init();
void mp_zd1211_preinit(void);
void mp_zd1211_init(struct usb_device	*usb, struct zd_chip *chip);
void mp_zd1211_open(struct zd_chip *chip);
extern int ar2524_initialized;
extern BYTE wifi_device_unplug;

int usb_ar2524_init(int eWhichOtg)
{
    struct usb_interface *intf = getUsbInterface();
    struct net_device *netdev;
    int ret = 0;
	struct ieee80211_hw *hw = zd_intf_to_hw(intf);
	struct zd_mac *mac;

    wifi_device_unplug = FALSE; // Will move to probe( ) before sending cmd to usb when it integered with RALINK_WIFI  

    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
        ret = -1;
        return ret;
    }
    mp_zd1211_preinit();
    usb_init();
    mp_usb_init();
    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
        ret = -1;
        return ret;
    }
    ret = probe(intf, NULL);
    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
        ret = -1;
        return ret;
    }
    netdev = netdev_global;
    MP_ASSERT(netdev);
    MP_ASSERT(netdev_global->open);

	hw = zd_intf_to_hw(intf);
    MP_ASSERT(hw);
	mac = zd_hw_mac(hw);
    MP_ASSERT(mac);
    mp_zd1211_init(interface_to_usbdev(intf), &mac->chip);

	/*
	 *	Call device private open method
	 */
	set_bit(__LINK_STATE_START, &netdev->state);

	TaskStartup(NETWARE_TASK);  
    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
        ret = -1;
        return ret;
    }
    ret = dev_open(netdev);
    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
        ret = -1;
        return ret; 
    }
    mp_zd1211_open(&mac->chip);
    if(UsbOtgCheckPlugOut(eWhichOtg)) // device plug out
    {
        ret = -1;
        return ret;
    }
	
	//if(wifi_device_unplug)
	#if !Make_ADHOC
        #if MAKE_XPG_PLAYER
	if(ar2524_initialized&&!Wpa_Get_PM()) // enter it after Second-times
	{
		Wpa_Change_Iface();
		//wifi_device_unplug = FALSE;
	}
	else
		Wpa_Set_PM(FALSE);
	#endif
	#endif

    ar2524_initialized++;
    wifi_device_type = WIFI_USB_DEVICE_AR2524;
        
    return ret;
}

static void disconnect(struct usb_interface *intf)
{
	struct ieee80211_hw *hw = zd_intf_to_hw(intf);
	struct zd_mac *mac;
	struct zd_usb *usb;
    mpDebugPrint("disconnect");
	/* Either something really bad happened, or we're just dealing with
	 * a DEVICE_INSTALLER. */
	if (hw == NULL)
		return;

	mac = zd_hw_mac(hw);
	usb = &mac->chip.usb;

	dev_dbg_f(zd_usb_dev(usb), "\n");

	ieee80211_unregister_hw(hw);

	/* Just in case something has gone wrong! */
	zd_usb_disable_rx(usb);
	zd_usb_disable_int(usb);

#ifdef LINUX
	/* If the disconnect has been caused by a removal of the
	 * driver module, the reset allows reloading of the driver. If the
	 * reset will not be executed here, the upload of the firmware in the
	 * probe function caused by the reloading of the driver will fail.
	 */
	usb_reset_device(interface_to_usbdev(intf));
#endif

	zd_mac_clear(mac);
	ieee80211_free_hw(hw);
	dev_dbg(&intf->dev, "disconnected\n");
}

void urb_cleanup_run(struct usb_device *udev);
void zd_usb_wifi_unplug(void)
{
    struct usb_interface *intf = getUsbInterface();
	struct usb_device *udev = interface_to_usbdev(intf);
    //for pwd mod
   #if Make_WPA
    wpa_supplicant_set_run_status(FALSE);
   #endif
    usb_data_cleanup();
    urb_cleanup_run(udev);

    if (netdev_global)
    {
        struct net_device *dev = netdev_global;
        netif_carrier_off(dev);
        netif_device_detach(dev);
        if (udev && NetDeviceAttachEvent_Fn)
            NetDeviceAttachEvent_Fn(
                    le16_to_cpu(udev->descriptor.idVendor),
                    le16_to_cpu(udev->descriptor.idProduct),
                    FALSE);
    }

    disconnect(intf);

    tcpCloseAll();
}

#ifdef LINUX
static struct usb_driver driver = {
	.name		= "zd1211rw",
	.id_table	= usb_ids,
	.probe		= probe,
	.disconnect	= disconnect,
};
#endif

struct workqueue_struct *zd_workqueue;

static int __init usb_init(void)
{
	int r;

	pr_debug("%s usb_init()\n", driver.name);

	zd_workqueue = create_singlethread_workqueue("ar2524");
	if (zd_workqueue == NULL) {
		printk(KERN_ERR "%s couldn't create workqueue\n", "ar2524");
		return -ENOMEM;
	}

#ifdef LINUX
	r = usb_register(&driver);
	if (r) {
		destroy_workqueue(zd_workqueue);
		printk(KERN_ERR "%s usb_register() failed. Error number %d\n",
		       driver.name, r);
		return r;
	}
#endif

	pr_debug("%s initialized\n", driver.name);
	return 0;
}

static void __exit usb_exit(void)
{
	pr_debug("%s usb_exit()\n", driver.name);
#ifdef LINUX
	usb_deregister(&driver);
#endif
	destroy_workqueue(zd_workqueue);
}

module_init(usb_init);
module_exit(usb_exit);

static int usb_int_regs_length(unsigned int count)
{
	return sizeof(struct usb_int_regs) + count * sizeof(struct reg_data);
}

static void prepare_read_regs_int(struct zd_usb *usb)
{
	struct zd_usb_interrupt *intr = &usb->intr;

	spin_lock_irq(&intr->lock);
	intr->read_regs_enabled = 1;
	INIT_COMPLETION(intr->read_regs.completion);
	spin_unlock_irq(&intr->lock);
}

static void disable_read_regs_int(struct zd_usb *usb)
{
	struct zd_usb_interrupt *intr = &usb->intr;

	spin_lock_irq(&intr->lock);
	intr->read_regs_enabled = 0;
	spin_unlock_irq(&intr->lock);
}

static int get_results(struct zd_usb *usb, u16 *values,
	               struct usb_req_read_regs *req, unsigned int count)
{
	int r;
	int i;
	struct zd_usb_interrupt *intr = &usb->intr;
	struct read_regs_int *rr = &intr->read_regs;
	struct usb_int_regs *regs = (struct usb_int_regs *)rr->buffer;

//    MP_FUNCTION_ENTER();
	spin_lock_irq(&intr->lock);

	r = -EIO;
	/* The created block size seems to be larger than expected.
	 * However results appear to be correct.
	 */
	if (rr->length < usb_int_regs_length(count)) {
		dev_dbg_f(zd_usb_dev(usb),
			 "error: actual length %d less than expected %d\n",
			 rr->length, usb_int_regs_length(count));
		goto error_unlock;
	}
	if (rr->length > sizeof(rr->buffer)) {
		dev_dbg_f(zd_usb_dev(usb),
			 "error: actual length %d exceeds buffer size %u\n",
			 rr->length, sizeof(rr->buffer));
		goto error_unlock;
	}

	for (i = 0; i < count; i++) {
		struct reg_data *rd = &regs->regs[i];
		if (rd->addr != req->addr[i]) {
			dev_dbg_f(zd_usb_dev(usb),
				 "rd[%d] addr %#06hx expected %#06hx\n", i,
				 le16_to_cpu(rd->addr),
				 le16_to_cpu(req->addr[i]));
			goto error_unlock;
		}
		values[i] = le16_to_cpu(rd->value);
	}

	r = 0;
//    MP_FUNCTION_EXIT();
error_unlock:
	spin_unlock_irq(&intr->lock);
//    MP_FUNCTION_EXIT();
	return r;
}

int zd_usb_ioread16v(struct zd_usb *usb, u16 *values,
	             const zd_addr_t *addresses, unsigned int count)
{
	int r;
	int i, req_len, actual_req_len;
	struct usb_device *udev;
	struct usb_req_read_regs *req = NULL;
	unsigned long timeout;

//    MP_FUNCTION_ENTER();
	if (count < 1) {
		dev_dbg_f(zd_usb_dev(usb), "error: count is zero\n");
		return -EINVAL;
	}
	if (count > USB_MAX_IOREAD16_COUNT) {
		dev_dbg_f(zd_usb_dev(usb),
			 "error: count %u exceeds possible max %u\n",
			 count, USB_MAX_IOREAD16_COUNT);
		return -EINVAL;
	}
#ifdef LINUX
	if (in_atomic()) {
		dev_dbg_f(zd_usb_dev(usb),
			 "error: io in atomic context not supported\n");
		return -EWOULDBLOCK;
	}
#endif
//    mpDebugPrint("zd_usb_ioread16v: usb=%p",usb);
//    mpDebugPrint("zd_usb_ioread16v: usb->intr.urb=%p",usb->intr.urb);
	if (!usb_int_enabled(usb)) {
		 dev_dbg_f(zd_usb_dev(usb),
			  "error: usb interrupt not enabled\n");
		return -EWOULDBLOCK;
	}

#if 0
	count += 1;
#endif
	req_len = sizeof(struct usb_req_read_regs) + count * sizeof(__le16);
#ifdef LINUX
	req = kmalloc(req_len, GFP_KERNEL);
#else
	req = usb_alloc_buffer(req_len);
//    mpDebugPrint("zd_usb_ioread16v: req=%p",req);
#endif
	if (!req)
		return -ENOMEM;
	req->id = cpu_to_le16(USB_REQ_READ_REGS);
	for (i = 0; i < count; i++)
		req->addr[i] = cpu_to_le16((u16)addresses[i]);
#if 0
    req->addr[1] = cpu_to_le16((u16)FWRAW_REGS_ADDR + 1);
#endif

	udev = zd_usb_to_usbdev(usb);
	prepare_read_regs_int(usb);
	r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_REGS_OUT),
		         req, req_len, &actual_req_len, 1000 /* ms */);
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
			"error in usb_bulk_msg(). Error number %d\n", r);
		goto error;
	}
	if (req_len != actual_req_len) {
		dev_dbg_f(zd_usb_dev(usb), "error in usb_bulk_msg()\n"
			" req_len %d != actual_req_len %d\n",
			req_len, actual_req_len);
		r = -EIO;
		goto error;
	}

	MP_TRACE_LINE();
	timeout = wait_for_completion_timeout(&usb->intr.read_regs.completion,
	                                      msecs_to_jiffies(1000));
    MP_DEBUG("after wait_for_completion_timeout");
	MP_TRACE_LINE();
	if (!timeout) {
		disable_read_regs_int(usb);
		dev_dbg_f(zd_usb_dev(usb), "read timed out\n");
		r = -ETIMEDOUT;
		goto error;
	}

	r = get_results(usb, values, req, count);
//    MP_FUNCTION_EXIT();
error:
#ifdef LINUX
	kfree(req);
#else
	usb_free_buffer(req);
#endif
	return r;
}

int zd_usb_iowrite16v(struct zd_usb *usb, const struct zd_ioreq16 *ioreqs,
	              unsigned int count)
{
	int r;
	struct usb_device *udev;
	struct usb_req_write_regs *req = NULL;
	int i, req_len, actual_req_len;

	if (count == 0)
		return 0;
	if (count > USB_MAX_IOWRITE16_COUNT) {
		dev_dbg_f(zd_usb_dev(usb),
			"error: count %u exceeds possible max %u\n",
			count, USB_MAX_IOWRITE16_COUNT);
		return -EINVAL;
	}
#ifdef LINUX
	if (in_atomic()) {
		dev_dbg_f(zd_usb_dev(usb),
			"error: io in atomic context not supported\n");
		return -EWOULDBLOCK;
	}
#endif

	req_len = sizeof(struct usb_req_write_regs) +
		  count * sizeof(struct reg_data);
#ifdef LINUX
	req = kmalloc(req_len, GFP_KERNEL);
#else
	req = usb_alloc_buffer(req_len);
//    mpDebugPrint("zd_usb_iowrite16v: req=%p",req);
#endif
	if (!req)
		return -ENOMEM;

	req->id = cpu_to_le16(USB_REQ_WRITE_REGS);
	for (i = 0; i < count; i++) {
		struct reg_data *rw  = &req->reg_writes[i];
		rw->addr = cpu_to_le16((u16)ioreqs[i].addr);
		rw->value = cpu_to_le16(ioreqs[i].value);
	}

	udev = zd_usb_to_usbdev(usb);
	r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_REGS_OUT),
		         req, req_len, &actual_req_len, 1000 /* ms */);
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
			"error in usb_bulk_msg(). Error number %d\n", r);
		goto error;
	}
	if (req_len != actual_req_len) {
		dev_dbg_f(zd_usb_dev(usb),
			"error in usb_bulk_msg()"
			" req_len %d != actual_req_len %d\n",
			req_len, actual_req_len);
		r = -EIO;
		goto error;
	}

	/* FALL-THROUGH with r == 0 */
error:
#ifdef LINUX
	kfree(req);
#else
	usb_free_buffer(req);
#endif
	return r;
}

int zd_usb_rfwrite(struct zd_usb *usb, u32 value, u8 bits)
{
	int r;
	struct usb_device *udev;
	struct usb_req_rfwrite *req = NULL;
	int i, req_len, actual_req_len;
	u16 bit_value_template;

#ifdef LINUX
	if (in_atomic()) {
		dev_dbg_f(zd_usb_dev(usb),
			"error: io in atomic context not supported\n");
		return -EWOULDBLOCK;
	}
#endif
	if (bits < USB_MIN_RFWRITE_BIT_COUNT) {
		dev_dbg_f(zd_usb_dev(usb),
			"error: bits %d are smaller than"
			" USB_MIN_RFWRITE_BIT_COUNT %d\n",
			bits, USB_MIN_RFWRITE_BIT_COUNT);
		return -EINVAL;
	}
	if (bits > USB_MAX_RFWRITE_BIT_COUNT) {
		dev_dbg_f(zd_usb_dev(usb),
			"error: bits %d exceed USB_MAX_RFWRITE_BIT_COUNT %d\n",
			bits, USB_MAX_RFWRITE_BIT_COUNT);
		return -EINVAL;
	}
#ifdef DEBUG
	if (value & (~0UL << bits)) {
		dev_dbg_f(zd_usb_dev(usb),
			"error: value %#09x has bits >= %d set\n",
			value, bits);
		return -EINVAL;
	}
#endif /* DEBUG */

	dev_dbg_f(zd_usb_dev(usb), "value %#09x bits %d\n", value, bits);

	r = zd_usb_ioread16(usb, &bit_value_template, CR203);
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
			"error %d: Couldn't read CR203\n", r);
		goto out;
	}
	bit_value_template &= ~(RF_IF_LE|RF_CLK|RF_DATA);

	req_len = sizeof(struct usb_req_rfwrite) + bits * sizeof(__le16);
#ifdef LINUX
	req = kmalloc(req_len, GFP_KERNEL);
#else
	req = usb_alloc_buffer(req_len);
#endif
	if (!req)
		return -ENOMEM;

	req->id = cpu_to_le16(USB_REQ_WRITE_RF);
	/* 1: 3683a, but not used in ZYDAS driver */
	req->value = cpu_to_le16(2);
	req->bits = cpu_to_le16(bits);

	for (i = 0; i < bits; i++) {
		u16 bv = bit_value_template;
		if (value & (1 << (bits-1-i)))
			bv |= RF_DATA;
		req->bit_values[i] = cpu_to_le16(bv);
	}

	udev = zd_usb_to_usbdev(usb);
	r = usb_bulk_msg(udev, usb_sndbulkpipe(udev, EP_REGS_OUT),
		         req, req_len, &actual_req_len, 1000 /* ms */);
	if (r) {
		dev_dbg_f(zd_usb_dev(usb),
			"error in usb_bulk_msg(). Error number %d\n", r);
		goto out;
	}
	if (req_len != actual_req_len) {
		dev_dbg_f(zd_usb_dev(usb), "error in usb_bulk_msg()"
			" req_len %d != actual_req_len %d\n",
			req_len, actual_req_len);
		r = -EIO;
		goto out;
	}

	/* FALL-THROUGH with r == 0 */
out:
#ifdef LINUX
	kfree(req);
#else
	usb_free_buffer(req);
#endif
	return r;
}
