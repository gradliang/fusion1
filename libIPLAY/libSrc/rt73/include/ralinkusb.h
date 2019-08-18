#ifndef __RT73_USB_H
#define __RT73_USB_H
#include "lwip_incl.h"
#include "UtilTypeDef.h"
#include "atomic.h"
#include "linux/list.h"
#include "list_mpx.h"
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

struct semaphore {
	//atomic_t count;
	int sleepers;
	//wait_queue_head_t wait;
};


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

/**
 * struct usb_device_id - identifies USB devices for probing and hotplugging
 * @match_flags: Bit mask controlling of the other fields are used to match
 *	against new devices.  Any field except for driver_info may be used,
 *	although some only make sense in conjunction with other fields.
 *	This is usually set by a USB_DEVICE_*() macro, which sets all
 *	other fields in this structure except for driver_info.
 * @idVendor: USB vendor ID for a device; numbers are assigned
 *	by the USB forum to its members.
 * @idProduct: Vendor-assigned product ID.
 * @bcdDevice_lo: Low end of range of vendor-assigned product version numbers.
 *	This is also used to identify individual product versions, for
 *	a range consisting of a single device.
 * @bcdDevice_hi: High end of version number range.  The range of product
 *	versions is inclusive.
 * @bDeviceClass: Class of device; numbers are assigned
 *	by the USB forum.  Products may choose to implement classes,
 *	or be vendor-specific.  Device classes specify behavior of all
 *	the interfaces on a devices.
 * @bDeviceSubClass: Subclass of device; associated with bDeviceClass.
 * @bDeviceProtocol: Protocol of device; associated with bDeviceClass.
 * @bInterfaceClass: Class of interface; numbers are assigned
 *	by the USB forum.  Products may choose to implement classes,
 *	or be vendor-specific.  Interface classes specify behavior only
 *	of a given interface; other interfaces may support other classes.
 * @bInterfaceSubClass: Subclass of interface; associated with bInterfaceClass.
 * @bInterfaceProtocol: Protocol of interface; associated with bInterfaceClass.
 * @driver_info: Holds information used by the driver.  Usually it holds
 *	a pointer to a descriptor understood by the driver, or perhaps
 *	device flags.
 *
 * In most cases, drivers will create a table of device IDs by using
 * USB_DEVICE(), or similar macros designed for that purpose.
 * They will then export it to userspace using MODULE_DEVICE_TABLE(),
 * and provide it to the USB core through their usb_driver structure.
 *
 * See the usb_match_id() function for information about how matches are
 * performed.  Briefly, you will normally use one of several macros to help
 * construct these entries.  Each entry you provide will either identify
 * one or more specific products, or will identify a class of products
 * which have agreed to behave the same.  You should put the more specific
 * matches towards the beginning of your table, so that driver_info can
 * record quirks of specific products.
 */
struct usb_device_id {
	/* which fields to match against? */
	__u16		match_flags;

	/* Used for product specific matches; range is inclusive */
	__u16		idVendor;
	__u16		idProduct;
	__u16		bcdDevice_lo;
	__u16		bcdDevice_hi;

	/* Used for device class matches */
	__u8		bDeviceClass;
	__u8		bDeviceSubClass;
	__u8		bDeviceProtocol;

	/* Used for interface class matches */
	__u8		bInterfaceClass;
	__u8		bInterfaceSubClass;
	__u8		bInterfaceProtocol;

	/* not matched against */
	unsigned long	driver_info;
};

/*-------------------------------------------------------------------------*/
#define USB_DEVICE_ID_MATCH_VENDOR		0x0001
#define USB_DEVICE_ID_MATCH_PRODUCT		0x0002

#define USB_DEVICE_ID_MATCH_DEVICE \
		(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)

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
			
#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)

#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)

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
	int transfer_buffer_length;	/* (in) data buffer length */
	int actual_length;		/* (return) actual transfer length */
	unsigned char *setup_packet;	/* (in) setup packet (control only) */
	//dma_addr_t setup_dma;		/* (in) dma addr for setup_packet */
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
	enum _USB_DEVICE_STATE	state;	/* configured, not attached, etc */
	int 	speed;	/* high/full/low (or error) */

	//struct usb_tt	*tt; 		/* low/full speed dev, highspeed hub */
	int		ttport;		/* device port on that tt hub */

	unsigned int toggle[2];		/* one bit for each endpoint
					 * ([0] = IN, [1] = OUT) */

	struct usb_device *parent;	/* our hub, unless we're the root */
	//struct usb_bus *bus;		/* Bus we're part of */
	//struct usb_host_endpoint ep0;

	//struct device dev;		/* Generic device interface */

//	struct usb_device_descriptor descriptor;/* Descriptor */
	//struct usb_host_config *config;	/* All of the configs */

	//struct usb_host_config *actconfig;/* the active configuration */
	//struct usb_host_endpoint *ep_in[16];
	//struct usb_host_endpoint *ep_out[16];

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

	int maxchild;			/* Number of ports if hub */
	struct usb_device *children[USB_MAXCHILDREN];

	int pm_usage_cnt;		/* usage counter for autosuspend */
#ifdef CONFIG_PM
	struct delayed_work autosuspend; /* for delayed autosuspends */
	struct mutex pm_mutex;		/* protects PM operations */

	unsigned auto_pm:1;		/* autosuspend/resume in progress */
	unsigned do_remote_wakeup:1;	/* remote wakeup should be enabled */
#endif
};
static inline unsigned int __create_pipe(struct usb_device *dev,
		unsigned int endpoint)
{
	return (dev->devnum << 8) | (endpoint << 15);
}

#define usb_alloc_urb(iso, flags)	mm_malloc(sizeof(struct urb))
#define usb_free_urb(urb)	        mm_free(urb)

#define usb_sndbulkpipe(dev,endpoint)	\
	((PIPE_BULK << 30) | __create_pipe(dev,endpoint))
#define usb_rcvbulkpipe(dev,endpoint)	\
		((PIPE_BULK << 30) | __create_pipe(dev,endpoint) | USB_DIR_IN)
		
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
#endif
