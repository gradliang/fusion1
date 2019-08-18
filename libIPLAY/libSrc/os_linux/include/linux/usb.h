#ifndef __LINUX_USB_H
#define __LINUX_USB_H

#include <linux/mod_devicetable.h>
#include <linux/usb/ch9.h>

#define USB_MAJOR			180
#define USB_DEVICE_MAJOR		189


#ifdef __KERNEL__

#include <linux/interrupt.h>	/* for in_interrupt() */
//#include "lwip_incl.h"
#include "UtilTypeDef.h"
#include "asm/atomic.h"
#include "linux/list.h"
#include "list_mpx.h"
#include <linux/usb/ch9.h>
#include <linux/device.h>
//#include <usbotg_host.h>
#include <usbotg.h>
#include	"ndebug.h"

#include "usbotg_api.h"
#include "usbotg.h"
//#include    "..\..\libIPLAY\libsrc\usbotg\include\usbotg_host_setup.h"

#define USB_MAXCHILDREN		(31)
/* NOTE:  these are not the standard USB_ENDPOINT_XFER_* values!! */
/* (yet ... they're the values used by usbfs) */
#define PIPE_ISOCHRONOUS		0
#define PIPE_INTERRUPT			1
#define PIPE_CONTROL			2
#define PIPE_BULK			3


#include <linux/errno.h>        /* for -ENODEV */
#include <linux/delay.h>	/* for mdelay() */
#include <linux/interrupt.h>	/* for in_interrupt() */
#include <linux/list.h>		/* for struct list_head */
#include <linux/kref.h>		/* for struct kref */
#include <linux/device.h>	/* for struct device */
#include <linux/fs.h>		/* for struct file_operations */
#include <linux/completion.h>	/* for struct completion */
#include <linux/sched.h>	/* for current && schedule_timeout */
#include <linux/mutex.h>	/* for struct mutex */

struct usb_device;
struct usb_driver;
struct wusb_dev;

/**
 * struct usb_host_endpoint - host-side endpoint descriptor and queue
 * @desc: descriptor for this endpoint, wMaxPacketSize in native byteorder
 * @urb_list: urbs queued to this endpoint; maintained by usbcore
 * @hcpriv: for use by HCD; typically holds hardware dma queue head (QH)
 *	with one or more transfer descriptors (TDs) per urb
 * @ep_dev: ep_device for sysfs info
 * @extra: descriptors following this endpoint in the configuration
 * @extralen: how many bytes of "extra" are valid
 *
 * USB requests are always queued to a given endpoint, identified by a
 * descriptor within an active interface in a given USB configuration.
 */
struct usb_host_endpoint {
	struct usb_endpoint_descriptor	desc;
	struct list_head		urb_list;
	void				*hcpriv;
	struct ep_device 		*ep_dev;	/* For sysfs info */

	unsigned char *extra;   /* Extra descriptors */
	int extralen;
};

/* host-side wrapper for one interface setting's parsed descriptors */
struct usb_host_interface {
	struct usb_interface_descriptor	desc;

	/* array of desc.bNumEndpoint endpoints associated with this
	 * interface setting.  these will be in no particular order.
	 */
	struct usb_host_endpoint *endpoint;

	char *string;		/* iInterface string, if present */
	unsigned char *extra;   /* Extra descriptors */
	int extralen;
};

struct usb_interface {
	struct usb_host_interface *cur_altsetting;	/* the currently
					 * active alternate setting */
	unsigned num_altsetting;	/* number of alternate settings */

	int minor;			/* minor number this interface is
					 * bound to */
	struct device dev;		/* interface specific device info */
	int pm_usage_cnt;		/* usage counter for autosuspend */
};

#define	interface_to_usbdev(intf) \
	container_of(intf->dev.parent, struct usb_device, dev)

static inline void *usb_get_intfdata (struct usb_interface *intf)
{
	return (&intf->dev)->driver_data;
}

static inline void usb_set_intfdata(struct usb_interface *intf, void *data)
{
	(&intf->dev)->driver_data = data;
}

struct urb;                                     /* forward declaration */
typedef void (*usb_complete_t)(struct urb *);

/**
 * struct usb_driver - identifies USB interface driver to usbcore
 * @name: The driver name should be unique among USB drivers,
 *	and should normally be the same as the module name.
 * @probe: Called to see if the driver is willing to manage a particular
 *	interface on a device.  If it is, probe returns zero and uses
 *	dev_set_drvdata() to associate driver-specific data with the
 *	interface.  It may also use usb_set_interface() to specify the
 *	appropriate altsetting.  If unwilling to manage the interface,
 *	return a negative errno value.
 * @disconnect: Called when the interface is no longer accessible, usually
 *	because its device has been (or is being) disconnected or the
 *	driver module is being unloaded.
 * @ioctl: Used for drivers that want to talk to userspace through
 *	the "usbfs" filesystem.  This lets devices provide ways to
 *	expose information to user space regardless of where they
 *	do (or don't) show up otherwise in the filesystem.
 * @suspend: Called when the device is going to be suspended by the system.
 * @resume: Called when the device is being resumed by the system.
 * @pre_reset: Called by usb_reset_composite_device() when the device
 *	is about to be reset.
 * @post_reset: Called by usb_reset_composite_device() after the device
 *	has been reset.
 * @id_table: USB drivers use ID table to support hotplugging.
 *	Export this with MODULE_DEVICE_TABLE(usb,...).  This must be set
 *	or your driver's probe function will never get called.
 * @dynids: used internally to hold the list of dynamically added device
 *	ids for this driver.
 * @drvwrap: Driver-model core structure wrapper.
 * @no_dynamic_id: if set to 1, the USB core will not allow dynamic ids to be
 *	added to this driver by preventing the sysfs file from being created.
 * @supports_autosuspend: if set to 0, the USB core will not allow autosuspend
 *	for interfaces bound to this driver.
 *
 * USB interface drivers must provide a name, probe() and disconnect()
 * methods, and an id_table.  Other driver fields are optional.
 *
 * The id_table is used in hotplugging.  It holds a set of descriptors,
 * and specialized data may be associated with each entry.  That table
 * is used by both user and kernel mode hotplugging support.
 *
 * The probe() and disconnect() methods are called in a context where
 * they can sleep, but they should avoid abusing the privilege.  Most
 * work to connect to a device should be done when the device is opened,
 * and undone at the last close.  The disconnect code needs to address
 * concurrency issues with respect to open() and close() methods, as
 * well as forcing all pending I/O requests to complete (by unlinking
 * them as necessary, and blocking until the unlinks complete).
 */
struct usb_driver {
	const char *name;

	int (*probe) (struct usb_interface *intf,
		      const struct usb_device_id *id);

	void (*disconnect) (struct usb_interface *intf);

	int (*ioctl) (struct usb_interface *intf, unsigned int code,
			void *buf);

	int (*suspend) (struct usb_interface *intf, pm_message_t message);
	int (*resume) (struct usb_interface *intf);
	int (*reset_resume)(struct usb_interface *intf);

	int (*pre_reset)(struct usb_interface *intf);
	int (*post_reset)(struct usb_interface *intf);

	const struct usb_device_id *id_table;

#ifdef LINUX
	struct usb_dynids dynids;
	struct usbdrv_wrap drvwrap;
#endif
	unsigned int no_dynamic_id:1;
	unsigned int supports_autosuspend:1;
	unsigned int soft_unbind:1;
};
#define	to_usb_driver(d) container_of(d, struct usb_driver, drvwrap.driver)
 
//-------------------
// VID/PID
//-------------------

#define RT73_USB_DEVICES_VID    0xd107
#define RT73_USB_DEVICES_PID    0x033c
#define RT73_USB_DEVICES1_VID    0xb807
#define RT73_USB_DEVICES1_PID    0x1db2
#define RT73_USB_DEVICES2_VID    0xe818
#define RT73_USB_DEVICES2_PID    0x3862
#define RT73_USB_DEVICES_ASUS_VID    0x050b
#define RT73_USB_DEVICES_ASUS_PID    0x2317     /* Asus WL-167G-v2 */
#define RT73_USB_DEVICES_LANTECH_VID    0x8f14
#define RT73_USB_DEVICES_LANTECH_PID    0x7325     /* Asus WL-167G-v2 */


/* ----------  Supported AR2524 sticks ---------- */

#define AR2524_USB_DEVICE1_VID    0x0ace
#define AR2524_USB_DEVICE1_PID    0x1215


/*
 * Device table entry for "new style" table-driven USB drivers.
 * User mode code can read these tables to choose which modules to load.
 * Declare the table as a MODULE_DEVICE_TABLE.
 *
 * A probe() parameter will point to a matching entry from this table.
 * Use the driver_info field for each match to hold information tied
 * to that match:  device quirks, etc.
 *
 * Terminate the driver's table with an all-zeroes entry.
 * Use the flag values to control which fields are compared.
 */

/*-------------------------------------------------------------------------*/
#define USB_DEVICE_ID_MATCH_VENDOR		0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		0x0002

#define USB_DEVICE_ID_MATCH_DEVICE \
		(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)

#include "usbotg_host.h"

/**
 * USB_DEVICE - macro used to describe a specific usb device
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device.
 */
#define USB_DEVICE(vend,prod) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE, .idVendor = (vend), \
			.idProduct = (prod)
			
#define usb_pipein(pipe)	((pipe) & USB_DIR_IN)
#define usb_pipeout(pipe)	(!usb_pipein(pipe))
			
#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)

#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == PIPE_BULK)

/*
 * urb->transfer_flags:
 */
#define URB_SHORT_NOT_OK	0x0001	/* report short reads as errors */
#define URB_ISO_ASAP		0x0002	/* iso-only, urb->start_frame
					 * ignored */
#define URB_NO_TRANSFER_DMA_MAP	0x0004	/* urb->transfer_dma valid on submit */
#define URB_NO_SETUP_DMA_MAP	0x0008	/* urb->setup_dma valid on submit */
#define URB_NO_FSBR		0x0020	/* UHCI-specific */
#define URB_ZERO_PACKET		0x0040	/* Finish bulk OUT with short packet */
#define URB_NO_INTERRUPT	0x0080	/* HINT: no non-error interrupt
					 * needed */
#define URB_FREE_BUFFER		0x0100	/* Free transfer buffer with the URB */

struct urb
{
	LM_LIST_ENTRY Link;

	/* private: usb core and host controller only fields in the urb */
	//struct kref kref;		/* reference count of the URB */
	//spinlock_t lock;		/* lock for the URB */
	//void *hcpriv;			/* private data for host controller */
	int bandwidth;			/* bandwidth for INT/ISO request */
	atomic_t use_count;		/* concurrent submissions counter */
	u8 reject;			/* submissions will fail */

	/* public: documented fields in the urb that can be used by drivers */
	struct list_head urb_list;	/* list head for use by the urb's
					 * current owner */
	struct usb_device *dev; 	/* (in) pointer to associated device */
	unsigned int pipe;		/* (in) pipe information */
	int status;			/* (return) non-ISO status */
	unsigned int transfer_flags;	/* (in) URB_SHORT_NOT_OK | ...*/
	void *transfer_buffer;		/* (in) associated data buffer */
	//dma_addr_t transfer_dma;	/* (in) dma addr for transfer_buffer */
	u32 transfer_dma;	/* (in) dma addr for transfer_buffer */
	int transfer_buffer_length;	/* (in) data buffer length */
	int actual_length;		/* (return) actual transfer length */
	unsigned char *setup_packet;	/* (in) setup packet (control only) */
//	dma_addr_t setup_dma;		/* (in) dma addr for setup_packet */
	int start_frame;		/* (modify) start frame (ISO) */
	int number_of_packets;		/* (in) number of ISO packets */
	int interval;			/* (modify) transfer interval
					 * (INT/ISO) */
	int error_count;		/* (return) number of ISO errors */
	void *context;			/* (in) context for completion */
	usb_complete_t complete;	/* (in) completion routine */
	//struct usb_iso_packet_descriptor iso_frame_desc[0];
					/* (in) ISO ONLY */
};


/*
 * struct usb_device - kernel's representation of a USB device
 *
 * FIXME: Write the kerneldoc!
 *
 * Usbcore drivers should not set usbdev->state directly.  Instead use
 * usb_set_device_state().
 */
struct usb_device {
	int		devnum;		/* Address on USB bus */
	char		devpath [16];	/* Use in messages: /port/port/... */
	enum usb_device_state	state;	/* configured, not attached, etc */
	enum usb_device_speed	speed;	/* high/full/low (or error) */

	//struct usb_tt	*tt; 		/* low/full speed dev, highspeed hub */
	int		ttport;		/* device port on that tt hub */

	unsigned int toggle[2];		/* one bit for each endpoint
					 * ([0] = IN, [1] = OUT) */

	struct usb_device *parent;	/* our hub, unless we're the root */
	//struct usb_bus *bus;		/* Bus we're part of */
	//struct usb_host_endpoint ep0;

	struct device dev;		/* Generic device interface */

	struct usb_device_descriptor descriptor;/* Descriptor */
	//struct usb_host_config *config;	/* All of the configs */

	//struct usb_host_config *actconfig;/* the active configuration */
	struct usb_host_endpoint *ep_in[16];
	struct usb_host_endpoint *ep_out[16];

	char **rawdescriptors;		/* Raw descriptors for each config */

	unsigned short bus_mA;		/* Current available from the bus */
	u8 portnum;			/* Parent port number (origin 1) */
	u8 level;			/* Number of USB hub ancestors */

	unsigned discon_suspended:1;	/* Disconnected while suspended */
	unsigned have_langid:1;		/* whether string_langid is valid */
	int string_langid;		/* language ID for strings */

	/* static strings from the device */
	char *product;			/* iProduct string, if present */
	char *manufacturer;		/* iManufacturer string, if present */
	char *serial;			/* iSerialNumber string, if present */

	struct list_head filelist;
	//struct class_device *class_dev;
	//struct dentry *usbfs_dentry;	/* usbfs dentry entry for the device */

	/*
	 * Child devices - these can be either new devices
	 * (if this is a hub device), or different instances
	 * of this same device.
	 *
	 * Each instance needs its own set of data structures.
	 */

	//int maxchild;			/* Number of ports if hub */
	//struct usb_device *children[USB_MAXCHILDREN];

	int pm_usage_cnt;		/* usage counter for autosuspend */
#ifdef CONFIG_PM
	struct delayed_work autosuspend; /* for delayed autosuspends */
	struct mutex pm_mutex;		/* protects PM operations */

	unsigned auto_pm:1;		/* autosuspend/resume in progress */
	unsigned do_remote_wakeup:1;	/* remote wakeup should be enabled */
#endif
#ifdef PLATFORM_MPIXEL
	struct usb_interface *intf;
#else
#error "PLATFORM_MPIXEL is not defined."
#endif
};

/*
 * use these in module_init()/module_exit()
 * and don't forget MODULE_DEVICE_TABLE(usb, ...)
 */
extern int usb_register_driver(struct usb_driver *, struct module *,
			       const char *);
static inline int usb_register(struct usb_driver *driver)
{
	return usb_register_driver(driver, THIS_MODULE, "");
}

/*
 * timeouts, in milliseconds, used for sending/receiving control messages
 * they typically complete within a few frames (msec) after they're issued
 * USB identifies 5 second timeouts, maybe more in a few cases, and a few
 * slow devices (like some MGE Ellipse UPSes) actually push that limit.
 */
#define USB_CTRL_GET_TIMEOUT	5000
#define USB_CTRL_SET_TIMEOUT	5000

static inline unsigned int __create_pipe(struct usb_device *dev,
		unsigned int endpoint)
{
	return (dev->devnum << 8) | (endpoint << 15);
}

#define usb_init_urb(urb)	memset(urb, 0, sizeof(*urb));

static inline struct urb *usb_alloc_urb(int iso_packets, unsigned int mem_flags)
{
	struct urb *urb;
	urb =	mm_malloc(sizeof(struct urb));
	if (!urb) {
		return NULL;
	}
	usb_init_urb(urb);
	return urb;
}
#define usb_free_urb(urb)	        mm_free(urb)
#define usb_put_urb(u) 

#include "ndebug_copy.h"
/* 
 * MP62X USB hardware requires buffer memory not to cross 4KB boundary.
 */
static inline void *usb_alloc_buffer(size_t size)
{
    void *buf;
buf_realloc:
    buf = net_buf_mem_malloc(size);
    if (!buf)
    {
        MP_ASSERT(0);
        return NULL;
    }
#if 0
    if( ( (unsigned long)buf & 0xfff) + size >  0x1000 )
    {
        net_buf_mem_free(buf);
        goto buf_realloc;
    }
#endif
	return buf;
}
void net_buf_mem_free(void *rmem);
static inline void usb_free_buffer(void *buf)
{
    net_buf_mem_free(buf);
}

/* Create various pipes... */
#define usb_sndctrlpipe(dev,endpoint)	\
	((PIPE_CONTROL << 30) | __create_pipe(dev,endpoint))
#define usb_rcvctrlpipe(dev,endpoint)	\
	((PIPE_CONTROL << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev,endpoint)	\
	((PIPE_BULK << 30) | __create_pipe(dev,endpoint))
#define usb_rcvbulkpipe(dev,endpoint)	\
		((PIPE_BULK << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev,endpoint)	\
	((PIPE_INTERRUPT << 30) | __create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev,endpoint)	\
	((PIPE_INTERRUPT << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
		
void usb_hisr_poll_handler ();
void UsbWifiBulkOnlyActive (WHICH_OTG eWhichOtg);
void UsbWifiBulkOnlyIoc (WHICH_OTG eWhichOtg);
void UsbWifiSetupIoc(WHICH_OTG eWhichOtg);

qTD_Structure *  flib_Host20_Issue_Bulk_Active_Multi_TD (  BYTE    bArrayListNum,
                 WORD    hwSize, DWORD   *pwBufferArray, DWORD   wOffset,
                 BYTE    bDirection, BOOL    fActive, WHICH_OTG eWhichOtg);

qTD_Structure *  flib_Host20_Issue_Bulk_Active (BYTE    bArrayListNum,
                 WORD    hwSize, DWORD   *pwBufferArray,
                 DWORD   wOffset, BYTE    bDirection,
                 WHICH_OTG eWhichOtg);

/*-------------------------------------------------------------------*
 *                         SYNCHRONOUS CALL SUPPORT                  *
 *-------------------------------------------------------------------*/

extern int usb_control_msg(struct usb_device *dev, unsigned int pipe,
	__u8 request, __u8 requesttype, __u16 value, __u16 index,
	void *data, __u16 size, int timeout);

/**
 * usb_fill_bulk_urb - macro to help initialize a bulk urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete_fn: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 *
 * Initializes a bulk urb with the proper information needed to submit it
 * to a device.
 */
static inline void usb_fill_bulk_urb(struct urb *urb,
				     struct usb_device *dev,
				     unsigned int pipe,
				     void *transfer_buffer,
				     int buffer_length,
				     usb_complete_t complete_fn,
				     void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}

/**
 * usb_fill_int_urb - macro to help initialize a interrupt urb
 * @urb: pointer to the urb to initialize.
 * @dev: pointer to the struct usb_device for this urb.
 * @pipe: the endpoint pipe
 * @transfer_buffer: pointer to the transfer buffer
 * @buffer_length: length of the transfer buffer
 * @complete_fn: pointer to the usb_complete_t function
 * @context: what to set the urb context to.
 * @interval: what to set the urb interval to, encoded like
 *	the endpoint descriptor's bInterval value.
 *
 * Initializes a interrupt urb with the proper information needed to submit
 * it to a device.
 * Note that high speed interrupt endpoints use a logarithmic encoding of
 * the endpoint interval, and express polling intervals in microframes
 * (eight per millisecond) rather than in frames (one per millisecond).
 */
static inline void usb_fill_int_urb (struct urb *urb,
				     struct usb_device *dev,
				     unsigned int pipe,
				     void *transfer_buffer,
				     int buffer_length,
				     usb_complete_t complete_fn,
				     void *context,
				     int interval)
{
	//spin_lock_init(&urb->lock);
	//mpDebugPrint("usb_fill_int_urb pipe = %x", pipe);
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
	//TODO
	//if (dev->speed == USB_SPEED_HIGH)
	//	urb->interval = 1 << (interval - 1);
	//else
		urb->interval = interval;
	urb->start_frame = -1;
	
#if 0
		mpDebugPrint("urb->dev %x",urb->dev);
		mpDebugPrint("urb->pipe %x",urb->pipe);
		mpDebugPrint("urb->transfer_buffer %x",urb->transfer_buffer);
		mpDebugPrint("urb->transfer_buffer_length %x",urb->transfer_buffer_length);
		mpDebugPrint("urb->complete %x",urb->complete);
		mpDebugPrint("urb->context %x",urb->context);
#endif

}
#endif  /* __KERNEL__ */

#endif
