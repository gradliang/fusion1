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
*
* Filename	: usbotg_device.c
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)    modified by Cloud Wu
* Created Date	: 2007/08/01
* Description   : Program Entry
********************************************************************************
*/

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "usbotg_api.h"


#include "usbotg_device.h"
//#include "usbotg_device_msdc.h"
#include "usbotg_device_sidc.h"
#include "usbotg_device_cdc.h"
#include "taskid.h"
#include "ui.h"
#include "os.h"
#include "usbotg_device_extern_sam.h"

#if (SC_USBHOST==ENABLE)
#include "Usbotg_host_sm.h"
#endif

#if SC_USBDEVICE

#define USB_DEVICE_TIMEOUT_CNT  0xffffffff

#define FIFO_EN				1
#define SUSP_BIT				0x200
#define C_ERR				0x1
#define USB_TREND			0x2
#define SUSP_END			0x4
#define C_ERR_Mask_EN		0x1
#define USB_TREND_Mask_EN	0x2
#define SUSP_END_Mask_EN	0x4

#define BULK_ONLY_GET_MAX_LUN		    0xFE
#define BULK_ONLY_MASS_STORAGE_RESET	0xFF

#define ATTACH				0
#define DEFAULT				1
#define ADDRESS				2
#define CONFIGURE			3

#define HIGH_SPEED			0
#define FULL_SPEED			1

#define GET_STATUS			0
#define CLEAR_FEATURE		1
#define RESERVED1			2
#define SET_FEATURE			3
#define RESERVED2			4
#define SET_ADDRESS			5
#define GET_DESCRIPTOR		6
#define SET_DESCRIPTOR		7
#define GET_CONFIGURATION	8
#define SET_CONFIGURATION	9
#define GET_INTERFACE		10
#define SET_INTERFACE		11
#define SYNC_FRAME			12
#define SETUP_ERROR			13

#define DEVICE				1
#define CONFIGURATION		2
#define STRING				3
#define INTERFACE			4
#define ENDPOINT			5
#define DEVICE_QUALIFIER	6
#define OTHER_SPEED			7




	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X10
	#define HS_iProduct             0X20
	#define HS_iSerialNumber        0X00
	#define HS_CONFIGURATION_NUMBER 0X01

	// Configuration 0X01
	#define HS_INTERFACE_NUMBER  0X01
	#define HS_C1                   0X01
	#define HS_iConfiguration    0X30
	#define HS_bmAttribute       0xE0//0XC0
	#define HS_iMaxPower         0X00

	// Interface 0
	#define HS_ALT_NUMBER    0X01

				// AlternateSetting 0X00
	#define HS_bInterfaceNumber   0X00
	#define HS_bAlternateSetting  0X00

	#define HS_EP_NUMBER          0X02

	#define HS_bInterfaceClass    0X00
	#define HS_bInterfaceSubClass 0X00
	#define HS_bInterfaceProtocol 0X00
	#define HS_iInterface         0X40

	//EP0X01
	//#define HS_EP1_BLKSIZE    BLK512BYTE
	//#define HS_EP1_BLKNO      SINGLE_BLK
	//#define HS_EP1_DIRECTION  DIRECTION_IN
	//#define HS_EP1_TYPE       TF_TYPE_BULK
	//#define HS_EP1_MAX_PACKET 0x0200
	//#define HS_EP1_bInterval  00

	//EP0X02
	//#define HS_EP2_BLKSIZE    BLK512BYTE
	//#define HS_EP2_BLKNO      SINGLE_BLK
	//#define HS_EP2_DIRECTION  DIRECTION_OUT
	//#define HS_EP2_TYPE       TF_TYPE_BULK
	//#define HS_EP2_MAX_PACKET 0x0200
	//#define HS_EP2_bInterval  00

	//EP0X03
	#define HS_EP3_BLKSIZE    BLK512BYTE
	#define HS_EP3_BLKNO      SINGLE_BLK
	#define HS_EP3_DIRECTION  DIRECTION_IN
	#define HS_EP3_TYPE       TF_TYPE_INTERRUPT
	#define HS_EP3_MAX_PACKET 0x040
	#define HS_EP3_bInterval  01


// Configuration 1
// Interface 0
// AlternateSetting 0
#define HS_EP_LENGTH			    (EP_LENGTH * HS_EP_NUMBER)
// EP1
#define OTGP_HS_EP1_FIFO_START		FIFO0
#define OTGP_HS_EP1_FIFO_NO		    (HS_EP1_BLKNO * HS_EP1_BLKSIZE)
#define OTGP_HS_EP1_FIFO_CONFIG	    (FIFOEnBit | ((HS_EP1_BLKSIZE - 1) << 4) | ((HS_EP1_BLKNO - 1) << 2) | HS_EP1_TYPE)
#define OTGP_HS_EP1_FIFO_MAP		(((1 - HS_EP1_DIRECTION) << 4) | EP1)
#define OTGP_HS_EP1_MAP			    (OTGP_HS_EP1_FIFO_START |	(OTGP_HS_EP1_FIFO_START << 4)	| (MASK_F0 >> (4*HS_EP1_DIRECTION)))

// EP2
#define OTGP_HS_EP2_FIFO_START		(OTGP_HS_EP1_FIFO_START + OTGP_HS_EP1_FIFO_NO)
#define OTGP_HS_EP2_FIFO_NO		    (HS_EP2_BLKNO * HS_EP2_BLKSIZE)
#define OTGP_HS_EP2_FIFO_CONFIG	    (FIFOEnBit | ((HS_EP2_BLKSIZE - 1) << 4) | ((HS_EP2_BLKNO - 1) << 2) | HS_EP2_TYPE)
#define OTGP_HS_EP2_FIFO_MAP		(((1 - HS_EP2_DIRECTION) << 4) | EP2)
#define OTGP_HS_EP2_MAP			    (OTGP_HS_EP2_FIFO_START |	(OTGP_HS_EP2_FIFO_START << 4)	| (MASK_F0 >> (4*HS_EP2_DIRECTION)))

// EP3	(For Bulk Bi-direction test Interrupt IN Endpoint)
#define OTGP_HS_EP3_FIFO_START		(OTGP_HS_EP2_FIFO_START + OTGP_HS_EP2_FIFO_NO)
#define OTGP_HS_EP3_FIFO_NO		    (HS_EP3_BLKNO * HS_EP3_BLKSIZE)
#define OTGP_HS_EP3_FIFO_CONFIG	    (FIFOEnBit | ((HS_EP3_BLKSIZE - 1) << 4) | ((HS_EP3_BLKNO - 1) << 2) | HS_EP3_TYPE)
#define OTGP_HS_EP3_FIFO_MAP		(((1 - HS_EP3_DIRECTION) << 4) | EP3)
#define OTGP_HS_EP3_MAP			    (OTGP_HS_EP3_FIFO_START |	(OTGP_HS_EP3_FIFO_START << 4)	| (MASK_F0 >> (4*HS_EP3_DIRECTION)))

#define OTGP_FS_EP1_BLKNO        SINGLE_BLK      // 1
#define OTGP_FS_EP1_TYPE         TF_TYPE_BULK    // 2
#define OTGP_FS_EP1_BLKSIZE      BLK64BYTE       // 1
#define OTGP_FS_EP1_MAX_PACKET   FS_MAX_PACKET_SIZE //0x0040
#define OTGP_FS_EP1_DIRECTION    DIRECTION_IN    // 0

#define OTGP_FS_EP1_FIFO_START   FIFO0           // 0
#define OTGP_FS_EP1_FIFO_NO      (FS_EP1_BLKNO * FS_EP1_BLKSIZE)
#define OTGP_FS_EP1_FIFO_CONFIG	(FIFOEnBit | ((FS_EP1_BLKSIZE - 1) << 4) | ((FS_EP1_BLKNO - 1) << 2) | FS_EP1_TYPE)
#define OTGP_FS_EP1_FIFO_MAP     (((1 - FS_EP1_DIRECTION) << 4) | EP1)
#define OTGP_FS_EP1_MAP          (FS_EP1_FIFO_START | (FS_EP1_FIFO_START << 4) | (MASK_F0 >> (4*FS_EP1_DIRECTION)))



#define OTGP_FS_EP2_BLKNO        SINGLE_BLK      // 1
#define OTGP_FS_EP2_TYPE         TF_TYPE_BULK    // 2
#define OTGP_FS_EP2_BLKSIZE      BLK64BYTE      // 1
#define OTGP_FS_EP2_MAX_PACKET   FS_MAX_PACKET_SIZE //0x0040
#define OTGP_FS_EP2_DIRECTION    DIRECTION_OUT   // 0

#define OTGP_FS_EP2_FIFO_START   (FS_EP1_FIFO_START + FS_EP1_FIFO_NO)
#define OTGP_FS_EP2_FIFO_NO      (FS_EP2_BLKNO * FS_EP2_BLKSIZE)
#define OTGP_FS_EP2_FIFO_CONFIG	(FIFOEnBit | ((FS_EP2_BLKSIZE - 1) << 4) | ((FS_EP2_BLKNO - 1) << 2) | FS_EP2_TYPE)
#define OTGP_FS_EP2_FIFO_MAP     (((1 - FS_EP2_DIRECTION) << 4) | EP2)
#define OTGP_FS_EP2_MAP          (FS_EP2_FIFO_START | (FS_EP2_FIFO_START << 4) | (MASK_F0 >> (4*FS_EP2_DIRECTION)))



//#define OTGP_EP3_FIFO_START         (OTGP_HS_EP3_FIFO_START + OTGP_HS_EP2_FIFO_NO)              // 0xE
//#define OTGP_EP3_FIFO_NO            EP3_BLKSIZE         // 1
//#define OTGP_EP3_MAP                (EP3_FIFO_START | (EP3_FIFO_START << 4) | (MASK_F0 >> (4*EP3_DIRECTION)))
//#define OTGP_EP3_FIFO_MAP           (((1 - EP3_DIRECTION) << 4) | EP3) // ((1-0)<<4)|3=0x13
//#define OTGP_EP3_FIFO_CONFIG	    (0x80 | ((EP3_BLKSIZE - 1) << 4) | ((EP3_BLKNO - 1) << 2) | EP3_TYPE)



//usbdevice.c   declaration

// Block Size define
#define BLK512BYTE			1
#define BLK1024BYTE			2

#define BLK64BYTE			1
#define BLK128BYTE			2

// Block toggle number define
#define SINGLE_BLK			1
#define DOUBLE_BLK			2
#define TRIBLE_BLK			3

// Endpoint transfer type
#define TF_TYPE_ISOCHRONOUS	1
#define TF_TYPE_BULK			2
#define TF_TYPE_INTERRUPT		3

// Endpoint or FIFO direction define
#define DIRECTION_IN		0
#define DIRECTION_OUT		1

// Descriptor Table uses the following parameters : fixed
//#define DEVICE_LENGTH		0x12
//#define CONFIG_LENGTH 		0x09
//#define INTERFACE_LENGTH	0x09
//#define EP_LENGTH			0x07
//#define DEVICE_QUALIFIER_LENGTH	0x0A
// Table 9-5. Descriptor Types
#define DT_DEVICE				1
#define DT_CONFIGURATION		2
#define DT_STRING				3
#define DT_INTERFACE			4
#define DT_ENDPOINT				5
#define DT_DEVICE_QUALIFIER	6
#define DT_OTHER_SPEED_CONFIGURATION		7
#define DT_INTERFACE_POWER				8


#define TEST_J			0x02
#define TEST_K			0x04
#define TEST_SE0_NAK	0x08
#define TEST_PKY		0x10

//#define USB_DIR_OUT 	0x00	// to device
//#define USB_DIR_IN		0x80	// to host

#define MESS_ERROR		(0x01 << 0)
#define MESS_WARNING	(0x01 << 1)
#define MESS_INFO		(0x01 << 2)

//=======================================================================//
#define USB_SPEC_VER		0x0200
#define VENDOR_ID           0x0851
#define STI_VENDOR_ID       0x0851
#define STI_PRODUCT_ID      0x1542
#define MSDC_PRODUCT_ID     0x1543
#define VDC_PRODUCT_ID      0x1544
#define EXT_PRODUCT_ID      0x1545 // Side Monitor
#define CDC_PRODUCT_ID				0x1540
#define DEVICE_RELEASE_NO			0x0200
//=======================================================================//

// device configuration:
#define VENDOR_DEVICE_CLASS 0xFF
#define DEVICE_CLASS        0x00
#define DEVICE_SUBCLASS     0x00
#define DEVICEP_ROTOCOL     0x00
#define MANUFACTURER        0x01
#define PRODUCT             0x02
#define SERIALNUMBER        0x03
#define NUMBER_OF_CONFIGURATION 0x01

#define NUMBER_OF_INTERFACE     1
#define INTERFACE_NUMBER        0
#define CONFIGURATION_VALUE     1
#define CONFIGURATION_INDEX     0
#define CONFIGURATION_ATTRIBUTE 0xC0// D7: Reserved (set to one)
                                    // D6: Self-powered
//#define CONFIGURATION_ATTRIBUTE 0xE0// D7: Reserved (set to one)
                                    // D6: Self-powered
                                    // D5: Remote Wakeup
#define CONFIGURATION_MAX_POWER 0x32

#define MASS_STORAGE_CLASS      8
#define MSDC_SUBCLASS           6
#define MSDC_PROTOCOL           0x50

#define STI_SUBCLASS            1
#define STILL_IMAGE_CLASS       6
#define STI_PROTOCOL            1

#define VENDOR_SUBCLASS         0xff
#define VENDOR_CLASS            0xff
#define VENDOR_PROTOCOL         0xff

#define CDC_SUBCLASS            2
#define CDC_CLASS               2
#define CDC_PROTOCOL            1


#define ALTERNATESETTING        0
#define EP_NUMBER               3
#define FIFO_NUMBER             3
#define INTERFACE_INDEX         0

#define CONFIG_TOTAL_LENGTH     (CONFIG_LENGTH + INTERFACE_LENGTH + EP_LENGTH * EP_NUMBER)
#define CDC_CONFIG_TOTAL_LENGTH 0x3E

#define FOTG200_Periph_MAX_EP		8	// 1..10
#define FOTG200_Periph_MAX_FIFO		4	// 0.. 3


#if USBOTG_DEVICE_EXTERN_SAMSUNG // For Samsung MSDC
BYTE gMsdcDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(USB_SPEC_VER),      // bcdUSB
    HI_BYTE_OF_WORD(USB_SPEC_VER),
    DEVICE_CLASS,                       // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(VENDOR_ID_SAMSUNG),         // idVendor
    HI_BYTE_OF_WORD(VENDOR_ID_SAMSUNG),
    LO_BYTE_OF_WORD(MSDC_PRODUCT_ID_800P),   // idProduct
    HI_BYTE_OF_WORD(MSDC_PRODUCT_ID_800P),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};
#else   // For MPX MSDC
BYTE gMsdcDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(USB_SPEC_VER),      // bcdUSB
    HI_BYTE_OF_WORD(USB_SPEC_VER),
    DEVICE_CLASS,                       // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(VENDOR_ID),         // idVendor
    HI_BYTE_OF_WORD(VENDOR_ID),
    LO_BYTE_OF_WORD(MSDC_PRODUCT_ID),   // idProduct
    HI_BYTE_OF_WORD(MSDC_PRODUCT_ID),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};
#endif


BYTE gStiDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(USB_SPEC_VER),      // bcdUSB
    HI_BYTE_OF_WORD(USB_SPEC_VER),
    DEVICE_CLASS,                       // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(STI_VENDOR_ID),     // idVendor
    HI_BYTE_OF_WORD(STI_VENDOR_ID),
    LO_BYTE_OF_WORD(STI_PRODUCT_ID),    // idProduct
    HI_BYTE_OF_WORD(STI_PRODUCT_ID),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};

#if USBOTG_DEVICE_EXTERN_SAMSUNG  // For Samsung SideMonitor
BYTE gExternDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(USB_SPEC_VER),      // bcdUSB
    HI_BYTE_OF_WORD(USB_SPEC_VER),
    VENDOR_DEVICE_CLASS,                // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(VENDOR_ID_SAMSUNG),         // idVendor
    HI_BYTE_OF_WORD(VENDOR_ID_SAMSUNG),
    LO_BYTE_OF_WORD(EXT_PRODUCT_ID_800P),    // idProduct
    HI_BYTE_OF_WORD(EXT_PRODUCT_ID_800P),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};
#else // USBOTG_DEVICE_EXTERN // For MPX SideMonitor
BYTE gExternDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(USB_SPEC_VER),      // bcdUSB
    HI_BYTE_OF_WORD(USB_SPEC_VER),
    VENDOR_DEVICE_CLASS,                // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(VENDOR_ID),         // idVendor
    HI_BYTE_OF_WORD(VENDOR_ID),
    LO_BYTE_OF_WORD(EXT_PRODUCT_ID),    // idProduct
    HI_BYTE_OF_WORD(EXT_PRODUCT_ID),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};
#endif


BYTE gVendorDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(USB_SPEC_VER),      // bcdUSB
    HI_BYTE_OF_WORD(USB_SPEC_VER),
    VENDOR_DEVICE_CLASS,                // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(VENDOR_ID),         // idVendor
    HI_BYTE_OF_WORD(VENDOR_ID),
    LO_BYTE_OF_WORD(VDC_PRODUCT_ID),    // idProduct
    HI_BYTE_OF_WORD(VDC_PRODUCT_ID),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};


BYTE gCdcDeviceDescriptor[DEVICE_LENGTH] =
{
    // EVICE descriptor
    DEVICE_LENGTH,                      // bLength
    DT_DEVICE,                          // bDescriptorType
    LO_BYTE_OF_WORD(0x0110),            // bcdUSB
    HI_BYTE_OF_WORD(0x0110),
    0x02,//DEVICE_CLASS,                       // bDeviceClass
    DEVICE_SUBCLASS,                    // bDeviceSubClass
    DEVICEP_ROTOCOL,                    // bDeviceProtocol
    EP0MAXPACKETSIZE,                   // bMaxPacketSize0
    LO_BYTE_OF_WORD(VENDOR_ID),     // idVendor
    HI_BYTE_OF_WORD(VENDOR_ID),
    LO_BYTE_OF_WORD(CDC_PRODUCT_ID),    // idProduct
    HI_BYTE_OF_WORD(CDC_PRODUCT_ID),
    LO_BYTE_OF_WORD(DEVICE_RELEASE_NO), // bcdDeviceReleaseNumber
    HI_BYTE_OF_WORD(DEVICE_RELEASE_NO),
    MANUFACTURER,                       // iManufacturer
    PRODUCT,                            // iProduct
    SERIALNUMBER,                       // iSerialNumber
    NUMBER_OF_CONFIGURATION             // bNumConfigurations
};


BYTE gCdcConfigDescriptor[CONFIG_LENGTH] =
{
    // ONFIGURATION descriptor
    CONFIG_LENGTH,                          // bLength
    DT_CONFIGURATION,                       // bDescriptorType CONFIGURATION
    LO_BYTE_OF_WORD(CDC_CONFIG_TOTAL_LENGTH),   // wTotalLength, include all descriptors
    HI_BYTE_OF_WORD(CDC_CONFIG_TOTAL_LENGTH),
    0x02,//NUMBER_OF_INTERFACE,                    // bNumInterface
    CONFIGURATION_VALUE,                    // bConfigurationValue
    CONFIGURATION_INDEX,                    // iConfiguration
    0x40,//CONFIGURATION_ATTRIBUTE,                // bmAttribute
                                            // D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
    CONFIGURATION_MAX_POWER                 //iMaxPower (2mA / units)
};

BYTE gConfigDescriptor[CONFIG_LENGTH] =
{
    // ONFIGURATION descriptor
    CONFIG_LENGTH,                          // bLength
    DT_CONFIGURATION,                       // bDescriptorType CONFIGURATION
    LO_BYTE_OF_WORD(CONFIG_TOTAL_LENGTH),   // wTotalLength, include all descriptors
    HI_BYTE_OF_WORD(CONFIG_TOTAL_LENGTH),
    NUMBER_OF_INTERFACE,                    // bNumInterface
    CONFIGURATION_VALUE,                    // bConfigurationValue
    CONFIGURATION_INDEX,                    // iConfiguration
    CONFIGURATION_ATTRIBUTE,                // bmAttribute
                                            // D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
    CONFIGURATION_MAX_POWER                 //iMaxPower (2mA / units)
};


BYTE gMsdcInterfaceDescriptor[INTERFACE_LENGTH] =
{
#if USBOTG_DEVICE_ISO_TEST
    // INTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    VENDOR_CLASS,           // bInterfaceClass
    VENDOR_SUBCLASS,        // bInterfaceSubClass
    VENDOR_PROTOCOL,        // bInterfaceProtocol
    INTERFACE_INDEX,        // iInterface
#else
    // NTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    MASS_STORAGE_CLASS,     // bInterfaceClass
    MSDC_SUBCLASS,          // bInterfaceSubClass
    MSDC_PROTOCOL,          // bInterfaceProtocol
    INTERFACE_INDEX,        // iInterface
#endif
};

BYTE gStiInterfaceDescriptor[INTERFACE_LENGTH] =
{
    // INTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    STILL_IMAGE_CLASS,      // bInterfaceClass
    STI_SUBCLASS,           // bInterfaceSubClass
    STI_PROTOCOL,           // bInterfaceProtocol
    INTERFACE_INDEX,        // iInterface
};

BYTE gVendorInterfaceDescriptor[INTERFACE_LENGTH] =
{
    // INTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    VENDOR_CLASS,           // bInterfaceClass
    VENDOR_SUBCLASS,        // bInterfaceSubClass
    VENDOR_PROTOCOL,        // bInterfaceProtocol
    INTERFACE_INDEX,        // iInterface
};

#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
BYTE gExternInterfaceDescriptor[INTERFACE_LENGTH] =
{
    // INTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    VENDOR_CLASS,           // bInterfaceClass
    VENDOR_SUBCLASS,        // bInterfaceSubClass
    VENDOR_PROTOCOL,        // bInterfaceProtocol
    INTERFACE_INDEX,        // iInterface
};
#endif

BYTE gCdcInterfaceDescriptor1[INTERFACE_LENGTH] =
{
    // INTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    0,//INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    1,//EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    CDC_CLASS,      // bInterfaceClass
    CDC_SUBCLASS,           // bInterfaceSubClass
    CDC_PROTOCOL,           // bInterfaceProtocol
    INTERFACE_INDEX,        // iInterface
};

BYTE gCdcInterfaceDescriptor2[INTERFACE_LENGTH] =
{
    // INTERFACE descriptor
    INTERFACE_LENGTH,       // bLength
    DT_INTERFACE,           // bDescriptorType INTERFACE
    1, //INTERFACE_NUMBER,       // bInterfaceNumber
    ALTERNATESETTING,       // bAlternateSetting
    2,//EP_NUMBER,              // bNumEndpoints(excluding endpoint zero)
    0x0A ,//CDC_CLASS,      // bInterfaceClass
    0,//CDC_SUBCLASS,           // bInterfaceSubClass
    0,//CDC_PROTOCOL,           // bInterfaceProtocol
    0//INTERFACE_INDEX,        // iInterface
};



#define CS_INTERFACE    0x24
#define CS_ENDPOINT     0x25
// Functional Descriptors 1
BYTE gCdcFunctionalDescriptor1[5] =
{
    // Functional descriptor
    0x05,                   // bFunctionLength
    CS_INTERFACE,           // bDescriptorType CS_INTERFACE
    0,                      // bDescriptorSubtype :
                            // Header Functional Descriptor,
                            // which marks the beginning of
                            // the concatenated set of
                            // functional descriptors for the interface
    0x10,                   // bcdCDC :
    0x01                    // USB Class Definitions for Communication
                            // Devices Specification release number in
                            // binary-coded decimal.
};

// Functional Descriptors 2
BYTE gCdcFunctionalDescriptor2[4] =
{
    // Functional descriptor
    0x04,                   // bFunctionLength
    CS_INTERFACE,           // bDescriptorType CS_INTERFACE
    0x02,                   // bDescriptorSubtype :
                            // Abstract Control Management
                            // Functional Descriptor.
    0x06                    // bmCapabilities
};

// Functional Descriptors 3
BYTE gCdcFunctionalDescriptor3[5] =
{
    // Functional descriptor
    0x05,                   // bFunctionLength
    CS_INTERFACE,           // bDescriptorType CS_INTERFACE
    1,                      // bDescriptorSubtype :
                            // Call Management FunctionalDescriptor.
    0x03,                   // bmCapabilities
    1,                      // bDataInterface
};

#if USBOTG_DEVICE_ISO_TEST
BYTE gHsEndpointDescriptorEp1[EP_LENGTH] =
{
    // EP1
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP1),         // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_ISOCHRONOUS|0xc,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),    // wMaxPacketSize
    HI_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),
    4//EP_INTERVAL                 // Interval for polling endpoint for data transfers.
};

/*
07,
05
02
02

01,
00
00
*/
BYTE gHsEndpointDescriptorEp2[EP_LENGTH] =
{
    // EP2
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_OUT | EP2),        // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_ISOCHRONOUS|0xc,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),        // wMaxPacketSize
    HI_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),
    4//EP_INTERVAL                 // Interval for polling endpoint for data transfers.
};

/*
07
05
83
03

08
00
02
*/

BYTE gHsEndpointDescriptorEp3[EP_LENGTH] =
{
    // EP3
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP3),         // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_INTERRUPT,          // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),   // wMaxPacketSize
    HI_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),
    HS_INT_EP_INTERVAL             // Interval for polling endpoint for data transfers.
};
BYTE gFsEndpointDescriptorEp1[EP_LENGTH] =
{
    // EP1
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP1),	        // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_ISOCHRONOUS|0xc,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),	// wMaxPacketSize
    HI_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),
    4//EP_INTERVAL	                     // Interval for polling endpoint for data transfers.
};

BYTE gFsEndpointDescriptorEp2[EP_LENGTH] =
{
    // EP2
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_OUT | EP2),	    // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_ISOCHRONOUS|0xc,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),	// wMaxPacketSize
    HI_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),
    4//EP_INTERVAL              // Interval for polling endpoint for data transfers.
};

BYTE gFsEndpointDescriptorEp3[EP_LENGTH] =
{
    // EP3
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP3),         // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_INTERRUPT,          // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),   // wMaxPacketSize
    HI_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),
    FS_INT_EP_INTERVAL             // Interval for polling endpoint for data transfers.
};

#else //USBOTG_DEVICE_ISO_TEST
BYTE gHsEndpointDescriptorEp1[EP_LENGTH] =
{
    // EP1
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP1),         // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_BULK,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),    // wMaxPacketSize
    HI_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),
    EP_INTERVAL                 // Interval for polling endpoint for data transfers.
};


BYTE gHsEndpointDescriptorEp2[EP_LENGTH] =
{
    // EP2
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_OUT | EP2),        // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_BULK,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),        // wMaxPacketSize
    HI_BYTE_OF_WORD(HS_MAX_PACKET_SIZE),
    EP_INTERVAL                 // Interval for polling endpoint for data transfers.
};


BYTE gHsEndpointDescriptorEp3[EP_LENGTH] =
{
    // EP3
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP3),         // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_INTERRUPT,          // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),   // wMaxPacketSize
    HI_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),
    HS_INT_EP_INTERVAL             // Interval for polling endpoint for data transfers.
};
BYTE gFsEndpointDescriptorEp1[EP_LENGTH] =
{
    // EP1
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP1),	        // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_BULK,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),	// wMaxPacketSize
    HI_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),
    EP_INTERVAL	                     // Interval for polling endpoint for data transfers.
};

BYTE gFsEndpointDescriptorEp2[EP_LENGTH] =
{
    // EP2
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_OUT | EP2),	    // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_BULK,               // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),	// wMaxPacketSize
    HI_BYTE_OF_WORD(FS_MAX_PACKET_SIZE),
    EP_INTERVAL              // Interval for polling endpoint for data transfers.
};

BYTE gFsEndpointDescriptorEp3[EP_LENGTH] =
{
    // EP3
    EP_LENGTH,                  // bLength
    DT_ENDPOINT,                // bDescriptorType ENDPOINT
    (USB_DIR_IN | EP3),         // bEndpointAddress
                                // D7: Direction, 1=IN, 0=OUT
                                // D6..4: Reserved(reset to zero), D3..0: The endpointer number
    TF_TYPE_INTERRUPT,          // bmAttributes
                                // D1..0: Transfer Type 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
                                // if not an isochronous endpoint, D7..2 are Reserved
    LO_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),   // wMaxPacketSize
    HI_BYTE_OF_WORD(INT_MAX_PACKET_SIZE),
    FS_INT_EP_INTERVAL             // Interval for polling endpoint for data transfers.
};
#endif //USBOTG_DEVICE_ISO_TEST

//STRING
BYTE gLangIdString[4] =
{
    //idx == 0x00, LANGID
    0x04,
    0x03,
    0x09,
    0x04
};

BYTE gManufacturerString[MANUFACTURER_SRING_LEN] =
{
	//idx == 0x01
    MANUFACTURER_SRING_LEN,
    DT_STRING,
    0x4d,
    0x00,   //0x16, 0x03, MagicPixel
    0x61,
    0x00,
    0x67,
    0x00,
    0x69,
    0x00,
    0x63,
    0x00,
    0x50,
    0x00,
    0x69,
    0x00,
    0x78,
    0x00,
    0x65,
    0x00,
    0x6c,
    0x00
};

BYTE gProductString[PRODUCT_SRING_LEN] =
{
    //idx == 0x02
    PRODUCT_SRING_LEN,
    DT_STRING,
    0x52,
    0x00 ,  //0x22, 0x03, Removable Disk
    0x65,
    0x00,
    0x6D,
    0x00,
    0x6F,
    0x00,
    0x76,
    0x00,
    0x61,
    0x00,
    0x62,
    0x00,
    0x6C,
    0x00,
    0x65,
    0x00,
    0x20,
    0x00,
    0x44,
    0x00,
    0x69,
    0x00,
    0x73,
    0x00,
    0x6B,
    0x00,
    0x20,
    0x00,
    0x20,
    0x00
};

BYTE gProductUSBcamString[PRODUCT_SRING_LEN] =
{
    50,
    DT_STRING,
    'U',
    0x00,
    'S',
    0x00,
    'B',
    0x00,
    ' ',
    0x00,
    'V',
    0x00,
    'i',
    0x00,
    'd',
    0x00,
    'e',
    0x00,
    'o',
    0x00,
    ' ',
    0x00,
    'C',
    0x00,
    'a',
    0x00,
    'p',
    0x00,
    't',
    0x00,
    'u',
    0x00,
    'r',
    0x00,
    'e',
    0x00,
    ' ',
    0x00,
    'D',
    0x00,
    'e',
    0x00,
    'v',
    0x00,
    'i',
    0x00,
    'c',
    0x00,
    'e',
    0x00,
};

BYTE gCdcProductString[20] =
{
    //idx == 0x02
    0x14,
    DT_STRING,
    0x69,   // iPlay CDC
    0x00 ,
    0x50,
    0x00,
    0x6C,
    0x00,
    0x61,
    0x00,
    0x79,
    0x00,
    0x20,
    0x00,
    0x43,
    0x00,
    0x44,
    0x00,
    0x43,
    0x00,
};

BYTE gSerialnumberString[SERIALNUMBER_SRING_LEN] =
{
    //idx == 0x03
    SERIALNUMBER_SRING_LEN,
    DT_STRING,
    0x4d,   // MP612B or A
    0x00,
    0x50,
    0x00,
    0x36,
    0x00,
    0x31,
    0x00,
    0x32,
    0x00,
    0x42,
    0x00
};


//Device QUALIFIER Descriptor
BYTE gDeviceQualifierDescriptor[DEVICE_QUALIFIER_LENGTH] =
{
    DEVICE_QUALIFIER_LENGTH,
    DT_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00
};

// OTHER SPEED CONFIGURATION Descriptor
BYTE gOtherSpeedConfigurationDescriptor[INTERFACE_LENGTH] =
{
    CONFIG_LENGTH,          //Configuration Descriptor
    DT_OTHER_SPEED_CONFIGURATION,
    LO_BYTE_OF_WORD(CONFIG_TOTAL_LENGTH),   // wTotalLength, include all descriptors
    HI_BYTE_OF_WORD(CONFIG_TOTAL_LENGTH),
    NUMBER_OF_INTERFACE,
    CONFIGURATION_VALUE,
    CONFIGURATION_INDEX,
    CONFIGURATION_ATTRIBUTE,
    CONFIGURATION_MAX_POWER
};


BYTE gOtherSpeedCdcConfigurationDescriptor[INTERFACE_LENGTH] =
{
    // ONFIGURATION descriptor
    CONFIG_LENGTH,                          // bLength
    DT_OTHER_SPEED_CONFIGURATION,                       // bDescriptorType CONFIGURATION
    LO_BYTE_OF_WORD(CDC_CONFIG_TOTAL_LENGTH),   // wTotalLength, include all descriptors
    HI_BYTE_OF_WORD(CDC_CONFIG_TOTAL_LENGTH),
    0x02,//NUMBER_OF_INTERFACE,                    // bNumInterface
    CONFIGURATION_VALUE,                    // bConfigurationValue
    CONFIGURATION_INDEX,                    // iConfiguration
    0x40,//CONFIGURATION_ATTRIBUTE,                // bmAttribute
                                            // D7: Reserved(set to one), D6: Self-powered, D5: Remote Wakeup, D4..0: Reserved(reset to zero)
    CONFIGURATION_MAX_POWER                 //iMaxPower (2mA / units)

};


//function declaration
static BYTE UsbdOtg0GetConnectStatus(void);
static BYTE UsbdOtg1GetConnectStatus(void);
static void vOTGSetEPMaxPacketSize(DWORD size, WHICH_OTG eWhichOtg);
static void vOTGClrEPx(WHICH_OTG eWhichOtg);
static void vOTGEP0RxData(WHICH_OTG eWhichOtg);
static void vOTGFIFO_EPxCfg_HS(WHICH_OTG eWhichOtg);
static void vOTGFIFO_EPxCfg_FS(WHICH_OTG eWhichOtg);
static void OTGP_HNP_Enable(WHICH_OTG eWhichOtg);
static void vOTGEP0TxData(WHICH_OTG eWhichOtg);
static void vFOTG200_Dev_Init(WHICH_OTG eWhichOtg);
static BOOL bGet_OTGstatus(WHICH_OTG eWhichOtg);
static BOOL bClear_OTGfeature(WHICH_OTG eWhichOtg);
static BOOL bSet_OTGfeature(WHICH_OTG eWhichOtg);
static BOOL bSet_OTGaddress(WHICH_OTG eWhichOtg);
static BOOL bSet_OTGdescriptor(WHICH_OTG eWhichOtg);
static void vGet_OTGconfiguration(WHICH_OTG eWhichOtg);
static BOOL bSet_OTGconfiguration(WHICH_OTG eWhichOtg);
static BOOL bGet_OTGinterface(WHICH_OTG eWhichOtg);
static BOOL bSet_OTGinterface(WHICH_OTG eWhichOtg);
static BOOL bOTGStandardCommand(WHICH_OTG eWhichOtg);
static void bOTGBulkOnlyMassStorageReset(WHICH_OTG eWhichOtg); // USB-IF MSC Test (ErrorRecovery)
static BOOL bOTGClassCommand(WHICH_OTG eWhichOtg);
static void vCxIN_VendorTxData(WHICH_OTG eWhichOtg);
static void vOTG_rst(WHICH_OTG eWhichOtg);
static void vOTG_suspend(WHICH_OTG eWhichOtg);
static void vOTG_resm(PST_USB_OTG_DES psUsbOtg);
static void vOTG_ISO_SeqErr(PST_USB_OTG_DES psUsbOtg);
static void vOTG_ISO_SeqAbort(PST_USB_OTG_DES psUsbOtg);
static void vOTG_TX0Byte(PST_USB_OTG_DES psUsbOtg);
static void vOTG_RX0Byte(PST_USB_OTG_DES psUsbOtg);
static void vOTG_ep0abort(WHICH_OTG eWhichOtg);
static void vOTG_ep0tx(WHICH_OTG eWhichOtg);
static void vOTG_ep0end(WHICH_OTG eWhichOtg);
static void vOTG_ep0fail(PST_USB_OTG_DES psUsbOtg);
static void vUsbOtg_FIFO_INT_action(TRANSACTION_STATE eState, WHICH_OTG eWhichOtg);
static void vOTG_F1_Out(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);
static void vOTG_F0_In(WHICH_OTG eWhichOtg);
static void vOTG_F2_In(WHICH_OTG eWhichOtg);
static void UsbOtgDeviceSetMode(BYTE mode, WHICH_OTG eWhichOtg);
static void vOtg_APInit(WHICH_OTG eWhichOtg);
static void OtgSetDescriptor(WHICH_OTG eWhichOtg);
static void UsbOtgDeviceTask(void);



void Api_UsbdInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    PUSB_DEVICE_DESC psDevDesc;

    MP_DEBUG("-usbotg%d- Api_UsbdInit", eWhichOtg);

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    psDevDesc = &psUsbOtg->sUsbDev.sDesc;
#if (SC_USBHOST==ENABLE)
    SetNoNeedToPollingFlag(eWhichOtg);
#endif
    psDevDesc->boIsUsbdInitiated = TRUE;
    OtgSetDescriptor(eWhichOtg);

    vOtg_APInit(eWhichOtg);    // Initialize Device API  parameter & value
    vFOTG200_Dev_Init(eWhichOtg);   //Initialize Device register

    MP_DEBUG("%s mUsbOtgUnPLGClr",__FUNCTION__);
    mUsbOtgUnPLGClr();    //Set UNPLUG 1->0 to present the device had been pluged in
    mUsbOtgIntDevIdleDis();       //  Disable Device Idle interrupt to avoid the USBOTG interrupt issue constantly
    mUsbOtgGlobIntEnSet();
}

void Api_UsbdFinal(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psUsbOtg->sUsbDev.sDesc;

    MP_DEBUG("-USBOTG%d- Api_UsbdFinal", eWhichOtg);
#if (SC_USBHOST==ENABLE)
    ClearNoNeedToPollingFlag(eWhichOtg);
#endif
    vOtg_APInit(eWhichOtg);    // Initialize Device API  parameter & value
    psDevDesc->boIsUsbdInitiated = FALSE;

    MP_DEBUG("%s mUsbOtgUnPLGSet",__FUNCTION__);
    mUsbOtgUnPLGSet();
    UsbOtgDeviceDsInit(eWhichOtg);
}


//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
BOOL Api_UsbdCheckIfConnectedDevice(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_DESC pDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);

    if (UsbOtgCheckError(eWhichOtg) == USB_NO_ERROR)
    {
        return UsbOtgDeviceCheckIdPin(eWhichOtg);
    }

    return FAIL;
}



WHICH_OTG Api_UsbdGetWhichOtgConnectedDevice(void)
{
    BYTE bCnt = 0;
    PST_USB_OTG_DES psUsbOtg;// = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    for(bCnt = 0; bCnt < USBOTG_NONE; bCnt++)
    {
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(bCnt);
        if (psUsbOtg->eFunction == NONE_FUNC)
            continue;
        if(Api_UsbdCheckIfConnectedDevice(bCnt)) // Check if plug-in
            return (WHICH_OTG)bCnt;
    }

    return USBOTG_NONE;
}



//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
BOOL Api_UsbdCheckIfConnectedPc(WHICH_OTG eWhichOtg)
{
    BOOL reVal = FALSE;
    PUSB_DEVICE_DESC pDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);

    if (UsbOtgCheckError(eWhichOtg) == USB_NO_ERROR)
    {
        reVal = UsbOtgDeviceCheckIdPin(eWhichOtg) &&
                ((pDevDesc->bUsbApMode == USB_AP_MSDC_MODE) ||
                 (pDevDesc->bUsbApMode == USB_AP_CDC_MODE)) &&
                (pDevDesc->boIsUsbdInitiated == TRUE);
    }

    return reVal;
}



WHICH_OTG Api_UsbdGetWhichOtgConnectedPc (void)
{
    BYTE bCnt = 0;
    PST_USB_OTG_DES psUsbOtg;// = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    for(bCnt = 0; bCnt < USBOTG_NONE; bCnt++)
    {
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(bCnt);
        if (psUsbOtg->eFunction == NONE_FUNC)
            continue;
        if(Api_UsbdCheckIfConnectedPc(bCnt)) // Check if plug-in
            return (WHICH_OTG)bCnt;
    }

    return USBOTG_NONE;
}


BOOL Api_UsbdCheckIfConnectedPrinter(WHICH_OTG eWhichOtg)
{
    BOOL reVal = FALSE;
    PUSB_DEVICE_DESC pDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);

    if (UsbOtgCheckError(eWhichOtg) == USB_NO_ERROR)
    {
        reVal = UsbOtgDeviceCheckIdPin(eWhichOtg) &&
                (pDevDesc->bUsbApMode == USB_AP_SIDC_MODE) &&
                (pDevDesc->boIsUsbdInitiated == TRUE);
    }

    return reVal;
}

WHICH_OTG Api_UsbdGetWhichOtgConnectedPrinter (void)
{
    BYTE bCnt = 0;
    PST_USB_OTG_DES psUsbOtg;// = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    for(bCnt = 0; bCnt < USBOTG_NONE; bCnt++)
    {
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(bCnt);
        if (psUsbOtg->eFunction == NONE_FUNC)
            continue;
        if(Api_UsbdCheckIfConnectedPrinter(bCnt)) // Check if plug-in
            return (WHICH_OTG)bCnt;
    }

    return USBOTG_NONE;
}

#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
BOOL Api_UsbdCheckIfConnectedSideMonitor(WHICH_OTG eWhichOtg)
{
    BOOL reVal = FALSE;
    PUSB_DEVICE_DESC pDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);

    if (UsbOtgCheckError(eWhichOtg) == USB_NO_ERROR)
    {
        reVal = UsbOtgDeviceCheckIdPin(eWhichOtg) &&
                (pDevDesc->bUsbApMode == USB_AP_EXTERN_MODE) &&
                (pDevDesc->boIsUsbdInitiated == TRUE);
    }

    return reVal;
}

WHICH_OTG Api_UsbdGetWhichOtgConnectedSideMonitor (void)
{
    BYTE bCnt = 0;
    PST_USB_OTG_DES psUsbOtg;// = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    for(bCnt = 0; bCnt < USBOTG_NONE; bCnt++)
    {
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(bCnt);
        if (psUsbOtg->eFunction == NONE_FUNC)
            continue;
        if(Api_UsbdCheckIfConnectedSideMonitor(bCnt)) // Check if plug-in
            return (WHICH_OTG)bCnt;
    }

    return USBOTG_NONE;
}
#endif

BYTE Api_UsbdGetDetectPlugFlag (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);

    return psUsbDev->stDetect.bPlugFlag;
}

void UsbOtgDeviceDetect(void)
{
    WHICH_OTG eWhichOtg;
    PST_USB_OTG_DES psUsbOtg;// = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psUsbDev;// = (PST_USB_OTG_DEVICE)&psUsbOtg->sUsbDev;

    for(eWhichOtg = 0; eWhichOtg < USBOTG_NONE; eWhichOtg++)  // TEST
    {
        psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

        if (psUsbOtg->eFunction == NONE_FUNC || psUsbOtg->eFunction == HOST_ONLY_FUNC)
            continue;

        psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
        PST_USB_OTG_DEVICE psUsbDevTemp;
        if (eWhichOtg == USBOTG0)
        {
            psUsbDevTemp = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(USBOTG1);
            if (psUsbDevTemp->stDetect.bPlugFlag == 1)
            {
                //MP_DEBUG("another USBOTG%d is already connected!! Do NOT send event to UI.", USBOTG1);
                continue; //return;
            }
        }
        else if (eWhichOtg == USBOTG1)
        {
            psUsbDevTemp = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(USBOTG0);
            if (psUsbDevTemp->stDetect.bPlugFlag == 1)
            {
                //MP_DEBUG("another USBOTG%d is already connected!! Do NOT send event to UI.", USBOTG0);
                continue; //return;
            }
        }

    	if (psUsbDev->stDetect.bPlugFlag == 0)
    	{
            if (mdwOTGC_Control_A_VBUS_VLD_Rd()&& mdwOTGC_Control_ID_Rd())
    		{
    			psUsbDev->stDetect.bStableCount++;

    			if (psUsbDev->stDetect.bStableCount >= 5)
    			{
    				psUsbDev->stDetect.bPlugFlag = 1;
    				Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_ATTACHED);
    				MP_DEBUG("Plug In USBOTG%d", eWhichOtg);

                    // CheckDeviceSetupFIFO(eWhichOtg); // Check Device Setup FIFO

                    if(psUsbDev->sDesc.bUsbApMode == USB_MODE_NONE) // avoid Null Mode before Plug-in for UI.
                        psUsbDev->sDesc.bUsbApMode = USB_AP_MSDC_MODE;


    				if (eWhichOtg == USBOTG0)
    				    EventSet(UI_EVENT, EVENT_USB0_CHG);
                    else if (eWhichOtg == USBOTG1)
    				    EventSet(UI_EVENT, EVENT_USB1_CHG);
                    else
                        MP_ALERT("Plug in Which OTG???");
    			}
    		}
    		else
    		{
    			psUsbDev->stDetect.bStableCount = 0;
    		}
    	}
    	else
    	{
            if (mdwOTGC_Control_ID_Rd() == 0 || mdwOTGC_Control_A_VBUS_VLD_Rd() == 0)
    		{
    			#if USBDEVICE_CDC_DEBUG
    			UsbOtgDeviceReInit(eWhichOtg);
    			SetIsAbleToUseUsbdCdc(FALSE, eWhichOtg);
    			#endif
    			psUsbDev->stDetect.bPlugFlag = 0;
    			psUsbDev->stDetect.bStableCount = 0;
                     Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_NONE);
    			MP_DEBUG("Plug Out USBOTG%d", eWhichOtg);

                /*
                if (eWhichOtg == USBOTG0)
    			    SysTimerProcRemove(UsbdOtg0GetConnectStatus);
                else if (eWhichOtg == USBOTG1)
                    SysTimerProcRemove(UsbdOtg1GetConnectStatus);
                */
                if (eWhichOtg == USBOTG0)
                    EventSet(UI_EVENT, EVENT_USB0_CHG);
                else if (eWhichOtg == USBOTG1)
                    EventSet(UI_EVENT, EVENT_USB1_CHG);
                else
                    MP_ALERT("Plug out from Which OTG???");
    		}
    	}
    }
}

static BYTE UsbdOtg0GetConnectStatus(void)
{
    WHICH_OTG eWhichOtg = USBOTG0;
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    return psUsbDev->stDetect.bPlugFlag;
}

static BYTE UsbdOtg1GetConnectStatus(void)
{
    WHICH_OTG eWhichOtg = USBOTG1;
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    return psUsbDev->stDetect.bPlugFlag;
}

///////////////////// otg device old code  ////////////////////////////

/*
//============================================================================= ok
//		UsbOtgDeviceClose()
//		Description:
//		input: Reserved
//		output: Reserved
//=============================================================================
void UsbOtgDeviceClose(void)
{
	DWORD wTemp;

	//<1>.Clear All the Interrupt
	//wFOTGPeri_Port(0x100) &= ~BIT2
	//*((DWORD *)(FOTG200_BASE_ADDRESS | (0x100))) &= ~BIT2;
	mUsbOtgGlobIntDis();

	//<2>.Clear all the Interrupt Status
	wTemp=mUsbOtgIntGroupRegRd();
	mUsbOtgIntGroupRegSet(0);

	//Interrupt source group 0(0x144)
	wTemp=mUsbOtgIntSrc0Rd();
	mUsbOtgIntSrc0Set(0);

	//Interrupt source group 1(0x148)
	wTemp=mUsbOtgIntSrc1Rd();
	mUsbOtgIntSrc1Set(0);

	//Interrupt source group 2(0x14C)
	wTemp=mUsbOtgIntSrc2Rd();
	mUsbOtgIntSrc2Set(0);

   //<3>.Turn off D+
   if (mdwOTGC_Control_CROLE_Rd()==USB_OTG_PERIPHERAL)//For Current Role = Peripheral
         mUsbOtgUnPLGSet();
}
*/


BOOL bOTGCxFxWrRd(BYTE FIFONum, BYTE dir, BYTE *pu8Buffer, WORD u16Num, WHICH_OTG eWhichOtg)
{
	DWORD wTemp;
    BOOL  ret = TRUE;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

	if (u16Num == 0) //Bruce;;03152005
	   return FALSE;

	// for wrapper control begin
		mOTG20_Wrapper_SwapBufferStart1_Set(pu8Buffer);
		mOTG20_Wrapper_SwapBufferEnd1_Set(pu8Buffer + u16Num - 1);
		mwOTG20_Wrapper_SwapBUffer1Enable_Set();
	// for wrapper control end

	mUsbOtgDmaConfig(u16Num,dir); // DIRECTION_IN=0;DIRECTION_OUT=1;
	mUsbOtgDMA2FIFOSel(FIFONum);
	mUsbOtgDmaAddr(pu8Buffer);
	mUsbOtgDmaStart();

	while(1)
	{
		wTemp = mUsbOtgIntSrc2Rd();
		if(wTemp & BIT8)
		{
			mUsbOtgCxFClr();
			mUsbOtgIntDmaErrClr();
			break;
		}
		if(wTemp & BIT7)
		{
			mUsbOtgIntDmaFinishClr();
			break;
		}
		if((wTemp & 0x7) > 0)//If (Resume/Suspend/Reset) exit
		{
			mUsbOtgDMARst();
			mUsbOtgCxFClr();
			mUsbOtgIntDmaFinishClr();
			break;
		}

#if (USBOTG_HOST_USBIF == DISABLE && USBOTG_HOST_EYE == DISABLE && WEB_CAM_DEVICE == DISABLE)
		if (UsbOtgDeviceCheckIdPin(eWhichOtg) == FALSE)
		{
			#if (USBDEVICE_CDC_DEBUG == DISABLE)
			MP_ALERT("--E-- %s USBOTG%d Read/Write Break!", __FUNCTION__, eWhichOtg);
			#endif
			UsbOtgDeviceReInit(eWhichOtg);
			ret = FALSE;
			break;
		}
#endif

	}
	mwOTG20_Wrapper_SwapBUffer1Enable_Clr();
	mUsbOtgDMA2FIFOSel(FOTG200_DMA2FIFO_Non);
    return ret;
}

//#define bFOTGPeri_Port(bOffset)		*((volatile BYTE *) ( USB_OTG_BASE | (bOffset)))
///////////////////////////////////////////////////////////////////////////////
//		vUsbCxLoopBackTest()
//		Description: Do Cx Data loop back test
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void vUsbCxLoopBackTest(WHICH_OTG eWhichOtg)
{
    BYTE u8TxTmp[64],u8RxTmp[64];
    BYTE *pu8TxTmp,*pu8RxTmp;
    BOOL bResult;
    BYTE bCnt;
    DWORD u32i;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    mdwOTGC_Control_Internal_Test();

    mpDebugPrint("\n\n-USBOTG%d- %s================\n", eWhichOtg, __FUNCTION__);

    pu8TxTmp = (BYTE*)((DWORD)&u8TxTmp[0]  | 0xa0000000);
    pu8RxTmp = (BYTE*)((DWORD)&u8RxTmp[0] | 0xa0000000);

    mUsbOtgTstHalfSpeedEn();

    // set for Cx loop back test enable
    //bFOTGPeri_Port(0x02) = BIT1;
    mUsbOtgLoopBackTestSet();

    for(bCnt = 0; bCnt < 10 ; bCnt++)
    {
        bResult = TRUE;

        // write data
        mpDebugPrint("\r\n-Write Data to FIFO-");
        for(u32i = 0; u32i<64; u32i++)
            *(pu8TxTmp+u32i) = u32i;
        //vOTGCxFWr(pu8TxTmp,64);
        bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,pu8TxTmp,64, eWhichOtg);

        // check Cx Done bit
        mUsbOtgEP0DoneSet();
        //while(!(bFOTGPeri_Port(0x0B) | BIT0));
	while(!mUsbOtgEP0DoneSetRd());

	// set for Cx loop back test clear
	//bFOTGPeri_Port(0x02) = BIT2;
        //bFOTGPeri_Port(0x0B) &= ~BIT0;
        //mUsbLoopBackTestClr();
        mUsbOtgExternalSideAddressClr();
        mUsbOtgEP0DoneClr();

        // read data
        mpDebugPrint("-Read Data from FIFO-");
        memset(pu8RxTmp,0,64);
        //vOTGCxFRd(pu8RxTmp,64);
        bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_OUT,pu8RxTmp,64, eWhichOtg);

        //compare data
        mpDebugPrint("-Compare Data-");
        for(u32i = 0; u32i<64; u32i++)
        {
            if(*(pu8TxTmp+u32i) != *(pu8RxTmp+u32i))  // Fail
            {
                mpDebugPrint("CxTest%d:***Cx loop test error, Tx=0x%x, Rx=0x%x\n",
                u32i,*(pu8TxTmp+u32i),*(pu8RxTmp+u32i));
                bResult = FALSE;
            }
            else // Pass
            {
                //mpDebugPrint("CxTest%d:Cx loop test ok, Tx=0x%x, Rx=0x%x\n",
                //u32i,*(pu8TxTmp+u32i),*(pu8RxTmp+u32i));
            }
        }
        mpDebugPrint(">>%d' Compare Data Result : << %s >>", bCnt+1, bResult ? "PASS" : "FAIL" );
    }
    mpDebugPrint("\n-Finish================\r\n Into while(1) \n");
    while(1);
}


static void vOTGSetEPMaxPacketSize(DWORD size, WHICH_OTG eWhichOtg)
{
	DWORD count;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

	if (size & (~0x7ff) != 0)
		return;
	for(count = 0;count < 8;++count) {
		*(&psUsbOtg->psUsbReg->DeviceInEndpoint1MaxPacketSize + count) &= ~0x7ff;
		*(&psUsbOtg->psUsbReg->DeviceInEndpoint1MaxPacketSize + count) |= size;
		*(&psUsbOtg->psUsbReg->DeviceOutEndpoint1MaxPacketSize + count) &= ~0x7ff;
		*(&psUsbOtg->psUsbReg->DeviceOutEndpoint1MaxPacketSize + count) |= size;
	}
}



///////////////////////////////////////////////////////////////////////////////
//		vOTGClrEPx()
//		Description:
//			1. Clear all endpoint Toggle Bit
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTGClrEPx(WHICH_OTG eWhichOtg)
{
    BYTE u8ep;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    // Clear All EPx Toggle Bit
    for (u8ep = 1; u8ep <= FOTG200_Periph_MAX_EP; u8ep ++)
    {
        mUsbOtgEPinRsTgSet(u8ep);
        mUsbOtgEPinRsTgClr(u8ep);
    }
    for (u8ep = 1; u8ep <= FOTG200_Periph_MAX_EP; u8ep ++)
    {
        mUsbOtgEPoutRsTgSet(u8ep);
        mUsbOtgEPoutRsTgClr(u8ep);
    }
}


///////////////////////////////////////////////////////////////////////////////
//		vOTGEP0RxData()
//		Description:
//			1. Receive data(max or short packet) from host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTGEP0RxData(WHICH_OTG eWhichOtg)
{
	BYTE u8temp;
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	if(psDevDesc->wTxRxCounter < EP0MAXPACKETSIZE)
		u8temp = (BYTE)psDevDesc->wTxRxCounter;
	else
		u8temp = EP0MAXPACKETSIZE;

	psDevDesc->wTxRxCounter -= (WORD)u8temp;
	//vOTGCxFRd(psDevDesc->pbDescriptorEx , u8temp);
    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_OUT,psDevDesc->pbDescriptorEx,u8temp,eWhichOtg);
	psDevDesc->pbDescriptorEx = psDevDesc->pbDescriptorEx + u8temp;

	// end of the data stage
	if (psDevDesc->wTxRxCounter == 0)
	{
		psDev->eUsbCxCommand = CMD_VOID;
		psDev->eUsbCxFinishAction = ACT_DONE;
	}
}


/////////////////////////////////////////////////////
//		vOTGFIFO_EPxCfg_HS(void)
//		Description:
//			1. Configure the FIFO and EPx map
//		input: none
//		output: none
/////////////////////////////////////////////////////
// The following setting is for configuration of hte fifo into:
// in ep1:   fifo0, 512 bytes, BULK(double fifo)
// out ep2: fifo2, 512 bytes, BULK(double fifo)
static void vOTGFIFO_EPxCfg_HS(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psUsbOtg->sUsbDev.sDesc;

#if (WEB_CAM_DEVICE == ENABLE)
	if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE) {
		mUsbOtgEPMapAllClr();
		mUsbOtgFIFOMapAllClr();
		mUsbOtgFIFOConfigAllClr();
		return;
	}
#endif
	if (psDevDesc->bUsbConfigValue == 1)
	{
		if (psDevDesc->bUsbInterfaceValue == 0)
		{
			if (psDevDesc->bUsbInterfaceAlternateSetting == 0)
			{
				//mUsbOtgEPMap(EP1, 0x30);
				//mUsbOtgEPMap(EP2, 0x23);
				//mUsbOtgFIFOMap(FIFO0, 0x11);
				//mUsbOtgFIFOMap(FIFO2, 0x02);
				//mUsbOtgFIFOConfig(FIFO0, 0x26);
				//mUsbOtgFIFOConfig(FIFO2, 0x26);
                mUsbOtgEPMapAllClr();
                mUsbOtgFIFOMapAllClr();
                mUsbOtgFIFOConfigAllClr();

				//EP0X01
				mUsbOtgEPMap(EP1, OTGP_HS_EP1_MAP);
				mUsbOtgFIFOMap(OTGP_HS_EP1_FIFO_START, OTGP_HS_EP1_FIFO_MAP);
				mUsbOtgFIFOConfig(OTGP_HS_EP1_FIFO_START, OTGP_HS_EP1_FIFO_CONFIG);

				//for(i = OTGP_HS_EP1_FIFO_START + 1 ;
				//	i < OTGP_HS_EP1_FIFO_START + OTGP_HS_EP1_FIFO_NO ;
				//    i ++)
				//{
				//	mUsbOtgFIFOConfig(i, (OTGP_HS_EP1_FIFO_CONFIG & (~BIT7)) );
				//}

				mUsbOtgEPMxPtSz(EP1, HS_EP1_DIRECTION, (HS_EP1_MAX_PACKET & 0x7ff) );
				mUsbOtgEPinHighBandSet(EP1 , HS_EP1_DIRECTION , HS_EP1_MAX_PACKET);

				//EP0X02
				mUsbOtgEPMap(EP2, OTGP_HS_EP2_MAP);
				mUsbOtgFIFOMap(OTGP_HS_EP2_FIFO_START, OTGP_HS_EP2_FIFO_MAP);
				mUsbOtgFIFOConfig(OTGP_HS_EP2_FIFO_START, OTGP_HS_EP2_FIFO_CONFIG);

				//for(i = OTGP_HS_EP2_FIFO_START + 1 ;
				////	i < OTGP_HS_EP2_FIFO_START + OTGP_HS_EP2_FIFO_NO ;
				//   i ++)
				//{
				//	mUsbOtgFIFOConfig(i, (OTGP_HS_EP2_FIFO_CONFIG & (~BIT7)) );
				//}

				mUsbOtgEPMxPtSz(EP2, HS_EP2_DIRECTION, (HS_EP2_MAX_PACKET & 0x7ff) );
				mUsbOtgEPinHighBandSet(EP2 , HS_EP2_DIRECTION , HS_EP2_MAX_PACKET);

                #if (USBOTG_DEVICE_ISO_TEST == DISABLE)
				//EP0X03
				mUsbOtgEPMap(EP3, OTGP_HS_EP3_MAP);
				mUsbOtgFIFOMap(OTGP_HS_EP3_FIFO_START, OTGP_HS_EP3_FIFO_MAP);
				mUsbOtgFIFOConfig(OTGP_HS_EP3_FIFO_START, OTGP_HS_EP3_FIFO_CONFIG);

				//for(i = OTGP_HS_EP3_FIFO_START + 1 ;
				//	i < OTGP_HS_EP3_FIFO_START + OTGP_HS_EP3_FIFO_NO ;
				//    i ++)
				//{
				//	mUsbOtgFIFOConfig(i, (OTGP_HS_EP3_FIFO_CONFIG & (~BIT7)) );
				//}

				mUsbOtgEPMxPtSz(EP3, HS_EP3_DIRECTION, (HS_EP3_MAX_PACKET & 0x7ff) );
				mUsbOtgEPinHighBandSet(EP3, HS_EP3_DIRECTION , HS_EP3_MAX_PACKET);
                #endif


//	wFOTGPeri_Port(0x1A0) = 0x33321330;
//	wFOTGPeri_Port(0x1A8) = 0x0f130211;
//	wFOTGPeri_Port(0x1AC) = 0x00232222;

//	wFOTGPeri_Port(0x138) = 0x000e00cf;
//	wFOTGPeri_Port(0x1A0) = 0x33332330;
//	wFOTGPeri_Port(0x1A8) = /*0x3f023f11;//*/0x0f020f11;
//	wFOTGPeri_Port(0x1AC) = /*0x0c220c22;//*/0x06260626;

			}
/*
                MP_DEBUG("Config HS FIFO/EP");
                mUsbOtgEPMap(EP1, OTGP_HS_EP1_MAP);
                mUsbOtgFIFOMap(OTGP_HS_EP1_FIFO_START, OTGP_HS_EP1_FIFO_MAP);
                mUsbOtgFIFOConfig(OTGP_HS_EP1_FIFO_START, OTGP_HS_EP1_FIFO_CONFIG);

                // only one block
                //	for(i = OTGP_HS_EP1_FIFO_START + 1 ;
                //		i < OTGP_HS_EP1_FIFO_START + OTGP_HS_EP1_FIFO_NO ;
                //	    i ++)
                //	{
                //		mUsbFIFOConfig(i, (OTGP_HS_EP1_FIFO_CONFIG & (~BIT7)) );
                //	}

                mUsbOtgEPMxPtSzClr(EP1, HS_EP1_DIRECTION);
                mUsbOtgEPMxPtSz(EP1, HS_EP1_DIRECTION, (HS_EP1_MAX_PACKET & 0x7ff) );
                mUsbOtgEPinHighBandSet(EP1 , HS_EP1_DIRECTION , HS_EP1_MAX_PACKET);

                //EP0x02
                mUsbOtgEPMap(EP2, OTGP_HS_EP2_MAP);
                mUsbOtgFIFOMap(OTGP_HS_EP2_FIFO_START, OTGP_HS_EP2_FIFO_MAP);
                mUsbOtgFIFOConfig(OTGP_HS_EP2_FIFO_START, OTGP_HS_EP2_FIFO_CONFIG);

                // only one block
                //	for(i = OTGP_HS_EP2_FIFO_START + 1 ;
                //		i < OTGP_HS_EP2_FIFO_START + OTGP_HS_EP2_FIFO_NO ;
                //	    i ++)
                //	{
                //		mUsbFIFOConfig(i, (OTGP_HS_EP2_FIFO_CONFIG & (~BIT7)) );
                //	}

                mUsbOtgEPMxPtSz(EP2, HS_EP2_DIRECTION, (HS_EP2_MAX_PACKET & 0x7ff) );
                mUsbOtgEPinHighBandSet(EP2 , HS_EP2_DIRECTION , HS_EP2_MAX_PACKET);

                //EP0x03
                mUsbOtgEPMap(EP3, OTGP_EP3_MAP);
                mUsbOtgFIFOMap(OTGP_EP3_FIFO_START, OTGP_EP3_FIFO_MAP);
                mUsbOtgFIFOConfig(OTGP_EP3_FIFO_START, OTGP_EP3_FIFO_CONFIG);
                mUsbOtgEPMxPtSz(EP3, EP3_DIRECTION, (EP3_MAX_PACKET & 0x7ff));
                mUsbOtgEPinHighBandSet(EP3 , EP3_DIRECTION, EP3_MAX_PACKET);
*/
		}
	}
}


#if 1
// The following setting is for configuration of hte fifo into:
// in ep1:   fifo0, 512 bytes, BULK(double fifo)
// out ep2: fifo2, 512 bytes, BULK(double fifo)
static void vOTGFIFO_EPxCfg_FS(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psUsbOtg->sUsbDev.sDesc;


#if (WEB_CAM_DEVICE == ENABLE)
	if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE) {
		mUsbOtgEPMapAllClr();
		mUsbOtgFIFOMapAllClr();
		mUsbOtgFIFOConfigAllClr();
		return;
	}
#endif

	if (psDevDesc->bUsbConfigValue == 1)
	{
		if (psDevDesc->bUsbInterfaceValue == 0)
		{
			if (psDevDesc->bUsbInterfaceAlternateSetting == 0)
			{
                MP_DEBUG("set EpMap:reg = 0x%x", psUsbOtg->psUsbReg);
				//mUsbOtgEPMap(EP1, 0x30);
				//mUsbOtgEPMap(EP2, 0x23);
				//mUsbOtgFIFOMap(FIFO0, 0x11);
				//mUsbOtgFIFOMap(FIFO2, 0x02);
				//mUsbOtgFIFOConfig(FIFO0, 0x26);
				//mUsbOtgFIFOConfig(FIFO2, 0x26);
                mUsbOtgEPMapAllClr();
                mUsbOtgFIFOMapAllClr();
                mUsbOtgFIFOConfigAllClr();

				//EP0X01
				mUsbOtgEPMap(EP1, OTGP_FS_EP1_MAP);
				mUsbOtgFIFOMap(OTGP_FS_EP1_FIFO_START, OTGP_FS_EP1_FIFO_MAP);
				mUsbOtgFIFOConfig(OTGP_FS_EP1_FIFO_START, OTGP_FS_EP1_FIFO_CONFIG);

				//for(i = OTGP_HS_EP1_FIFO_START + 1 ;
				//	i < OTGP_HS_EP1_FIFO_START + OTGP_HS_EP1_FIFO_NO ;
				//    i ++)
				//{
				//	mUsbOtgFIFOConfig(i, (OTGP_HS_EP1_FIFO_CONFIG & (~BIT7)) );
				//}

				mUsbOtgEPMxPtSz(EP1, FS_EP1_DIRECTION, (OTGP_FS_EP1_MAX_PACKET & 0x7ff) );
				mUsbOtgEPinHighBandSet(EP1 , FS_EP1_DIRECTION , OTGP_FS_EP1_MAX_PACKET);

				//EP0X02
				mUsbOtgEPMap(EP2, OTGP_FS_EP2_MAP);
				mUsbOtgFIFOMap(OTGP_FS_EP2_FIFO_START, OTGP_FS_EP2_FIFO_MAP);
				mUsbOtgFIFOConfig(OTGP_FS_EP2_FIFO_START, OTGP_FS_EP2_FIFO_CONFIG);

				//for(i = OTGP_HS_EP2_FIFO_START + 1 ;
				////	i < OTGP_HS_EP2_FIFO_START + OTGP_HS_EP2_FIFO_NO ;
				//   i ++)
				//{
				//	mUsbOtgFIFOConfig(i, (OTGP_HS_EP2_FIFO_CONFIG & (~BIT7)) );
				//}

				mUsbOtgEPMxPtSz(EP2, FS_EP2_DIRECTION, (OTGP_FS_EP2_MAX_PACKET & 0x7ff) );
				mUsbOtgEPinHighBandSet(EP2 , FS_EP2_DIRECTION , OTGP_FS_EP2_MAX_PACKET);

				//EP0X03
				mUsbOtgEPMap(EP3, OTGP_HS_EP3_MAP);
				mUsbOtgFIFOMap(OTGP_HS_EP3_FIFO_START, OTGP_HS_EP3_FIFO_MAP);
				mUsbOtgFIFOConfig(OTGP_HS_EP3_FIFO_START, OTGP_HS_EP3_FIFO_CONFIG);

				//for(i = OTGP_HS_EP3_FIFO_START + 1 ;
				//	i < OTGP_HS_EP3_FIFO_START + OTGP_HS_EP3_FIFO_NO ;
				//    i ++)
				//{
				//	mUsbOtgFIFOConfig(i, (OTGP_HS_EP3_FIFO_CONFIG & (~BIT7)) );
				//}

				mUsbOtgEPMxPtSz(EP3, HS_EP3_DIRECTION, (HS_EP3_MAX_PACKET & 0x7ff) );
				mUsbOtgEPinHighBandSet(EP3, HS_EP3_DIRECTION , HS_EP3_MAX_PACKET);


//	wFOTGPeri_Port(0x1A0) = 0x33321330;
//	wFOTGPeri_Port(0x1A8) = 0x0f130211;
//	wFOTGPeri_Port(0x1AC) = 0x00232222;

//	wFOTGPeri_Port(0x138) = 0x000e00cf;
//	wFOTGPeri_Port(0x1A0) = 0x33332330;
//	wFOTGPeri_Port(0x1A8) = /*0x3f023f11;//*/0x0f020f11;
//	wFOTGPeri_Port(0x1AC) = /*0x0c220c22;//*/0x06260626;

			}
		}
	}

#if 0
	if (psDevDesc->bUsbConfigValue == 1)
	{
		if (psDevDesc->bUsbInterfaceValue == 0)
		{
			if (psDevDesc->bUsbInterfaceAlternateSetting == 0)
			{
				//mUsbOtgEPMap(EP1, 0x30);
				//mUsbOtgEPMap(EP2, 0x23);
				//mUsbOtgFIFOMap(FIFO0, 0x11);
				//mUsbOtgFIFOMap(FIFO2, 0x02);
				//mUsbOtgFIFOConfig(FIFO0, 0x26);
				//mUsbOtgFIFOConfig(FIFO2, 0x26);
                //__asm("break 100");
                MP_DEBUG("Config HS FIFO/EP");
                mUsbOtgEPMapAllClr();
                mUsbOtgFIFOMapAllClr();
                mUsbOtgFIFOConfigAllClr();

                mUsbOtgEPMap(EP1, FS_EP1_MAP);
                mUsbOtgFIFOMap(FS_EP1_FIFO_START, FS_EP1_FIFO_MAP);
                mUsbOtgFIFOConfig(FS_EP1_FIFO_START, FS_EP1_FIFO_CONFIG);

                // only one block
                //	for(i = OTGP_HS_EP1_FIFO_START + 1 ;
                //		i < OTGP_HS_EP1_FIFO_START + OTGP_HS_EP1_FIFO_NO ;
                //	    i ++)
                //	{
                //		mUsbFIFOConfig(i, (OTGP_HS_EP1_FIFO_CONFIG & (~BIT7)) );
                //	}

                mUsbOtgEPMxPtSzClr(EP1, FS_EP1_DIRECTION);
                mUsbOtgEPMxPtSz(EP1, FS_EP1_DIRECTION, (FS_EP1_MAX_PACKET & 0x7ff) );
                mUsbOtgEPinHighBandSet(EP1 , FS_EP1_DIRECTION , FS_EP1_MAX_PACKET);

                //EP0x02
                mUsbOtgEPMap(EP2, FS_EP2_MAP);
                mUsbOtgFIFOMap(FS_EP2_FIFO_START, FS_EP2_FIFO_MAP);
                mUsbOtgFIFOConfig(FS_EP2_FIFO_START, FS_EP2_FIFO_CONFIG);

                // only one block
                //	for(i = OTGP_HS_EP2_FIFO_START + 1 ;
                //		i < OTGP_HS_EP2_FIFO_START + OTGP_HS_EP2_FIFO_NO ;
                //	    i ++)
                //	{
                //		mUsbFIFOConfig(i, (OTGP_HS_EP2_FIFO_CONFIG & (~BIT7)) );
                //	}

                mUsbOtgEPMxPtSz(EP2, FS_EP2_DIRECTION, (FS_EP2_MAX_PACKET & 0x7ff) );
                mUsbOtgEPinHighBandSet(EP2 , FS_EP2_DIRECTION , FS_EP2_MAX_PACKET);

                //EP0x03
                mUsbOtgEPMap(EP3, OTGP_HS_EP3_MAP);
                mUsbOtgFIFOMap(OTGP_HS_EP3_FIFO_START, OTGP_HS_EP3_FIFO_MAP);
                mUsbOtgFIFOConfig(OTGP_HS_EP3_FIFO_START, OTGP_HS_EP3_FIFO_CONFIG);
                mUsbOtgEPMxPtSz(EP3, EP3_DIRECTION, (EP3_MAX_PACKET & 0x7ff));
                mUsbOtgEPinHighBandSet(EP3 , EP3_DIRECTION, EP3_MAX_PACKET);
			}
		}
	}
#endif
}

//=============================================================================
//		OTGP_HNP_Enable()
//		Description:
//
//		input: none
//		output: none
//=============================================================================
static void OTGP_HNP_Enable(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    //<1>.Set b_Bus_Request
    mdwOTGC_Control_B_BUS_REQ_Set();

    //<2>.Set the HNP enable
    mdwOTGC_Control_B_HNP_EN_Set();
}

///////////////////////////////////////////////////////////////////////////////
//		vOTGEP0TxData()
//		Description:
//			1. Send data(max or short packet) to host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTGEP0TxData(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;
	WORD u8temp;

	do {
		if(psDevDesc->wTxRxCounter < EP0MAXPACKETSIZE)
		{
			u8temp = psDevDesc->wTxRxCounter;
			psDevDesc->wTxRxCounter = 0;
		}
		else
		{
			u8temp = EP0MAXPACKETSIZE;
			psDevDesc->wTxRxCounter -= EP0MAXPACKETSIZE;
		}
    	bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,psDevDesc->pbDescriptorEx,u8temp,eWhichOtg);
		IODelay(200);
		psDevDesc->pbDescriptorEx += u8temp;

	} while (psDevDesc->wTxRxCounter != 0);

	psDev->eUsbCxFinishAction  = ACT_DONE;
	// end of the data stage
	if (psDevDesc->wTxRxCounter == 0)
			psDev->eUsbCxCommand = CMD_VOID;
}


//#define wFOTGPeri_Port(bOffset)	*((volatile DWORD *) ( USB_OTG_BASE | (bOffset)))
///////////////////////////////////////////////////////////////////////////////
//		vFOTG200_Dev_Init()
//		Description:
//			1. Reset all interrupt and clear all fifo data of FOTG200 Device
//			2. Turn on the "Global Interrupt Enable" bit of FOTG200 Device
//			3. Turn on the "Chip Enable" bit of FOTG200 Device
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vFOTG200_Dev_Init(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES     psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

	// suspend counter
	mUsbOtgIdleCnt(7);

	// Clear interrupt
	mUsbOtgIntBusRstClr();
	mUsbOtgIntSuspClr();
	mUsbOtgIntResmClr();
	// Disable all fifo interrupt
	mUsbOtgIntFIFO0_3OUTDis();
	mUsbOtgIntFIFO0_3INDis();
	// Soft Reset
	mUsbOtgSoftRstSet(); 			// All circuit change to which state after Soft Reset?
	mUsbOtgSoftRstClr();
	// Clear all fifo
	mUsbOtgClrAllFIFOSet();			// will be cleared after one cycle.
	// Enable usb200 global interrupt
	mUsbOtgGlobIntEnSet();

      mUsbOtgChipEnSet();
#if 0
	// suspend counter
	//mUsbOtgIdleCnt(7);

	// Clear interrupt
	mUsbOtgIntBusRstClr();
	mUsbOtgIntSuspClr();
	mUsbOtgIntResmClr();

	// Soft Reset
	//mUsbOtgSoftRstSet(); 			// All circuit change to which state after Soft Reset?
	//mUsbOtgSoftRstClr();

	// Clear all fifo
	//mUsbOtgClrAllFIFOSet();			// will be cleared after one cycle.

	//vOTGSetEPMaxPacketSize(0x200);


	// The following setting is for configuration of hte fifo into:
	// in ep1:   fifo0, 512 bytes, BULK(double fifo)
	// out ep2: fifo2, 512 bytes, BULK(double fifo)
	wFOTGPeri_Port(0x138) = 0x000e00cf;
	wFOTGPeri_Port(0x1A0) = 0x33332330;
	wFOTGPeri_Port(0x1A8) = /*0x3f023f11;//*/0x0f020f11;
	wFOTGPeri_Port(0x1AC) = /*0x0c220c22;//*/0x06260626;
	wFOTGPeri_Port(0x100) = 0x24 | BIT1;
	wFOTGPeri_Port(0x114) = 0x0;

	// enter full speed
	wFOTGPeri_Port(0x100) |= BIT9;
#endif

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// USB OTG Device Test
//
////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//		bGet_OTGstatus()
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
//		output: TRUE or FALSE (BOOL)
///////////////////////////////////////////////////////////////////////////////
static BOOL bGet_OTGstatus(WHICH_OTG eWhichOtg)
{
	BYTE u8ep_n;
	BYTE u8fifo_n;
	BOOL bdir;
	BYTE RecipientStatusLow, RecipientStatusHigh;
	BYTE u8Tmp[2];
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	RecipientStatusLow = 0;
	RecipientStatusHigh = 0;

	switch (psDev->psControlCmd->Object) // Judge which recipient type is at first
	{
		case 0: // Device
			// Return 2-byte's Device status (Bit1:Remote_Wakeup, Bit0:Self_Powered) to Host
        	// Notice that the programe sequence of RecipientStatus
			RecipientStatusLow = (mUsbOtgRmWkupST() == 1) ? BIT1 : 0;
			// Bit0: Self_Powered--> DescriptorTable[0x23], D6(Bit 6)
			RecipientStatusLow |= ((gConfigDescriptor[0x07] >> 6) & 0x01);
			break;
 		case 1:					// Interface
			// Return 2-byte ZEROs Interface status to Host
    		break;

		case 2:					// Endpoint
			if(psDev->psControlCmd->Index == 0x00)
       			RecipientStatusLow = (BYTE)psDevDesc->boOtgEp0HaltSt;
			else
			{
				u8ep_n = psDev->psControlCmd->Index & 0x7F;		// which ep will be clear
				bdir = psDev->psControlCmd->Index >> 7;			// the direction of this ep
				if (u8ep_n > FOTG200_Periph_MAX_EP)			// over the Max. ep count ?
					return FALSE;
				else
				{
					u8fifo_n = mUsbOtgEPMapRd(u8ep_n);		// get the relatived FIFO number
					if (bdir == 1)
						u8fifo_n &= 0x0F;
					else
						u8fifo_n >>= 4;
					if (u8fifo_n >= FOTG200_Periph_MAX_FIFO)	// over the Max. fifo count ?
						return FALSE;

														// Check the FIFO had been enable ?
					if ((mUsbOtgFIFOConfigRd(u8fifo_n) & FIFOEnBit) == 0)
						return FALSE;
					if (bdir == 1)						// IN direction ?
						RecipientStatusLow = mUsbOtgEPinStallST(u8ep_n);
					else
						RecipientStatusLow = mUsbOtgEPoutStallST(u8ep_n);
				}
			}
	        	break;
		default :
			return FALSE;
	}

	// return RecipientStatus;
	u8Tmp[0] = RecipientStatusLow;
	u8Tmp[1] = RecipientStatusHigh;
	//vOTGCxFWr( u8Tmp, 2);
    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,u8Tmp,2,eWhichOtg);

	psDev->eUsbCxFinishAction  = ACT_DONE;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		bClear_OTGfeature()
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
//		output: TRUE or FALSE (BOOL)
///////////////////////////////////////////////////////////////////////////////
static BOOL bClear_OTGfeature(WHICH_OTG eWhichOtg)
{
	BYTE u8ep_n;
	BYTE u8fifo_n;
	BOOL bdir;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	switch (psDev->psControlCmd->Value)		// FeatureSelector
	{
		case 0:		// ENDPOINT_HALE
			// Clear "Endpoint_Halt", Turn off the "STALL" bit in Endpoint Control Function Register
			if(psDev->psControlCmd->Index == 0x00)
				psDevDesc->boOtgEp0HaltSt = FALSE;
			else
			{
				u8ep_n = psDev->psControlCmd->Index & 0x7F;		// which ep will be clear
				bdir = psDev->psControlCmd->Index >> 7;			// the direction of this ep
				if (u8ep_n > FOTG200_Periph_MAX_EP)			// over the Max. ep count ?
					return FALSE;
				else
				{
					u8fifo_n = mUsbOtgEPMapRd(u8ep_n);		// get the relatived FIFO number
					if (bdir == 1)
						u8fifo_n &= 0x0F;
					else
						u8fifo_n >>= 4;
					if (u8fifo_n >= FOTG200_Periph_MAX_FIFO)	// over the Max. fifo count ?
						return FALSE;

					// Check the FIFO had been enable ?
					if ((mUsbOtgFIFOConfigRd(u8fifo_n) & FIFOEnBit) == 0)
						return FALSE;
					if (bdir == 1)						// IN direction ?
					{
						mUsbOtgEPinRsTgSet(u8ep_n);		// Set Rst_Toggle Bit
						mUsbOtgEPinRsTgClr(u8ep_n);		// Clear Rst_Toggle Bit
						mUsbOtgEPinStallClr(u8ep_n);		// Clear Stall Bit
					}
					else
					{
						mUsbOtgEPoutRsTgSet(u8ep_n);		// Set Rst_Toggle Bit
						mUsbOtgEPoutRsTgClr(u8ep_n);		// Clear Rst_Toggle Bit
						mUsbOtgEPoutStallClr(u8ep_n);		// Clear Stall Bit
					}
				}
			}
			break;
		case 1 :   		// Device Remote Wakeup
			// Clear "Device_Remote_Wakeup", Turn off the"RMWKUP" bit in Main Control Register
			mUsbOtgRmWkupClr();
			break;
		case 2 :   		// Test Mode
			return FALSE;
		default :
			return FALSE;
	}
	psDev->eUsbCxFinishAction  = ACT_DONE;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		bSet_OTGfeature()
//		Description:
//			1. Process Cx Set feature command.
//		input: none
//		output: TRUE or FALSE (BOOL)
///////////////////////////////////////////////////////////////////////////////
static BOOL bSet_OTGfeature(WHICH_OTG eWhichOtg)
{
	BYTE i;
	BYTE u8ep_n;
	BYTE u8fifo_n;
	BYTE u8Tmp[52];
	BYTE *pp;
	BOOL bdir;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	switch (psDev->psControlCmd->Value)		// FeatureSelector
	{
		case 0:	// ENDPOINT_HALE
			// Set "Endpoint_Halt", Turn on the "STALL" bit in Endpoint Control Function Register
			if(psDev->psControlCmd->Index == 0x00)
				psDevDesc->boOtgEp0HaltSt = TRUE;
			else
			{
				u8ep_n = psDev->psControlCmd->Index & 0x7F;		// which ep will be clear
				bdir = psDev->psControlCmd->Index >> 7;			// the direction of this ep
				if (u8ep_n > FOTG200_Periph_MAX_EP)			// over the Max. ep count ?
					return FALSE;
				else
				{
					u8fifo_n = mUsbOtgEPMapRd(u8ep_n);		// get the relatived FIFO number
					if (bdir == 1)
						u8fifo_n &= 0x0F;
					else
						u8fifo_n >>= 4;
					if (u8fifo_n >= FOTG200_Periph_MAX_FIFO)	// over the Max. fifo count ?
						return FALSE;

														// Check the FIFO had been enable ?
					if ((mUsbOtgFIFOConfigRd(u8fifo_n) & FIFOEnBit) == 0)
						return FALSE;
					if (bdir == 1)						// IN direction ?
						mUsbOtgEPinStallSet(u8ep_n);		// Clear Stall Bit
					else
						mUsbOtgEPoutStallSet(u8ep_n);		// Set Stall Bit
				}
			}
			break;
 		case 1 :   		// Device Remote Wakeup
 			// Set "Device_Remote_Wakeup", Turn on the"RMWKUP" bit in Mode Register
			mUsbOtgRmWkupSet();
	            break;
		case 2 :   		// Test Mode
			switch ((psDev->psControlCmd->Index >> 8))	// TestSelector
			{
				case 0x1:	// Test_J
					mUsbOtgTsMdWr(TEST_J);
					break;
				case 0x2:	// Test_K
					mUsbOtgTsMdWr(TEST_K);
					break;
				case 0x3:	// TEST_SE0_NAK
					mUsbOtgTsMdWr(TEST_SE0_NAK);
					break;
				case 0x4:	// Test_Packet
					mUsbOtgTsMdWr(TEST_PKY);
					mUsbOtgEP0DoneSet();			// special case: follow the test sequence
					//////////////////////////////////////////////
					// Jay ask to modify, 91-6-5 (Begin)		//
					//////////////////////////////////////////////
					pp = u8Tmp;
					for (i=0; i<9; i++)			// JKJKJKJK x 9
					{
						(*pp) = (0x00);
						pp ++;
					}

					(*pp) = (0xAA);
					pp ++;
					(*pp) = (0x00);
					pp ++;

					for (i=0; i<8; i++)			// 8*AA
					{
						(*pp) = (0xAA);
						pp ++;
					}

					for (i=0; i<8; i++)			// 8*EE
					{
						(*pp) = (0xEE);
						pp ++;
					}
					(*pp) = (0xFE);
					pp ++;

					for (i=0; i<11; i++)		// 11*FF
					{
						(*pp) = (0xFF);
						pp ++;
					}

					(*pp) = (0x7F);
					pp ++;
					(*pp) = (0xBF);
					pp ++;
					(*pp) = (0xDF);
					pp ++;
					(*pp) = (0xEF);
					pp ++;
					(*pp) = (0xF7);
					pp ++;
					(*pp) = (0xFB);
					pp ++;
					(*pp) = (0xFD);
					pp ++;
					(*pp) = (0xFC);
					pp ++;
					(*pp) = (0x7E);
					pp ++;
					(*pp) = (0xBF);
					pp ++;
					(*pp) = (0xDF);
					pp ++;
					(*pp) = (0xFB);
					pp ++;
					(*pp) = (0xFD);
					pp ++;
					(*pp) = (0xFB);
					pp ++;
					(*pp) = (0xFD);
					pp ++;
					(*pp) = (0x7E);
					//vOTGCxFWr( u8Tmp, 52);
                    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,u8Tmp,52,eWhichOtg);

					//////////////////////////////////////////////
					// Jay ask to modify, 91-6-5 (End)			//
					//////////////////////////////////////////////

					// Turn on "r_test_packet_done" bit(flag) (Bit 5)
					mUsbOtgTsPkDoneSet();
					break;
				case 0x5:	// Test_Force_Enable
					//FUSBPort[0x08] = 0x20;	//Start Test_Force_Enable
					break;

				default:
					return FALSE;
			}
	         	break;
 		case 3 :   		//For OTG => b_hnp_enable
  			  OTGP_HNP_Enable(eWhichOtg);
            break;
 		case 4 :   		//For OTG => b_hnp_enable
            break;
#if 0
		case 5 :   		//For OTG => b_hnp_enable

  			 printf(">>> Please Connect to an alternate port on the A-device for HNP...\n");
  			  psDev->eUsbCxFinishAction  = ACT_DONE;
            break;
#endif
		default :
			return FALSE;
	}
	psDev->eUsbCxFinishAction  = ACT_DONE;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		bSet_OTGaddress()
//		Description:
//			1. Set USB bus addr to FOTG200 Device register.
//		input: none
//		output: TRUE or FALSE (BOOL)
///////////////////////////////////////////////////////////////////////////////
static BOOL bSet_OTGaddress(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;

	if (psDev->psControlCmd->Value >= 0x0100)
		return FALSE;
	else
	{
		mUsbOtgDevAddrSet(psDev->psControlCmd->Value);
		psDev->eUsbCxFinishAction  = ACT_DONE;
		Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_ADDRESS);
		return TRUE;
	}
}


///////////////////////////////////////////////////////////////////////////////
//		bGet_OTGdescriptor()
//		Description:
//			1. Point to the start location of the correct descriptor.
//			2. set the transfer length and return descriptor information back to host
//		input: none
//		output: TRUE or FALSE (BOOL)
///////////////////////////////////////////////////////////////////////////////
BOOL bGet_OTGdescriptor(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;
    #if 0
    switch ((BYTE)(psDev->psControlCmd->Value >> 8))
    {
        	case DEVICE:				// device descriptor
    		MP_DEBUG("device descriptor");
    		break;
        	case CONFIGURATION:	 		// configuration descriptor
    		MP_DEBUG("configuration descriptor(%x)", (BYTE)psDev->psControlCmd->Value);
    		break;
        	case STRING:   				// string descriptor
    		MP_DEBUG("string descriptor(%x)", (BYTE)psDev->psControlCmd->Value);
    		break;
        	case INTERFACE:   			// interface descriptor
    		MP_DEBUG("interface descriptor");
    		break;
        	case ENDPOINT:   			// endpoint descriptor
    		MP_DEBUG("endpoint descriptor");
    		break;
        	case DEVICE_QUALIFIER:   	// Device_Qualifier descritor
    		MP_DEBUG("Device_Qualifier descritor");
    		break;
        	case OTHER_SPEED:   		// Other_Speed_Configuration
    		MP_DEBUG("Other_Speed_Configuration");
    		break;
    	default:
    		break;
    }
    #endif

	//////////////// Decide the display descriptor length(range) //////////////
    switch ((BYTE)(psDev->psControlCmd->Value >> 8))
    {
		case DEVICE:				// device descriptor
		    switch(psDevDesc->bUsbApMode)
		    {
                case USB_AP_SIDC_MODE: // MTP : different PID
        			psDevDesc->pbDescriptorEx = &gStiDeviceDescriptor[0];
        			psDevDesc->wTxRxCounter = gStiDeviceDescriptor[0];
                    break;

                case USB_AP_CDC_MODE:
			        psDevDesc->pbDescriptorEx = &gCdcDeviceDescriptor[0];
			        psDevDesc->wTxRxCounter = gCdcDeviceDescriptor[0];
                    break;


                case USB_AP_EXTERN_MODE:
                    #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
                    psDevDesc->pbDescriptorEx = &gExternDeviceDescriptor[0];
                    psDevDesc->wTxRxCounter = gExternDeviceDescriptor[0];
                    #endif
                    break;

#if (WEB_CAM_DEVICE==ENABLE)
				case USB_AP_UAVC_MODE:
					{
						psDevDesc->wTxRxCounter = 0x12;
						UsbdWebCamFuncDevDecrpGet(&psDevDesc->wTxRxCounter, &psDevDesc->pbDescriptorEx);
						//mpDebugPrint("psDevDesc->bUsbApMode %d len %d", psDevDesc->bUsbApMode, psDevDesc->wTxRxCounter);
					}
					break;
#endif

                case USB_AP_MSDC_MODE:
                default:
			        psDevDesc->pbDescriptorEx = &gMsdcDeviceDescriptor[0];
			        psDevDesc->wTxRxCounter = gMsdcDeviceDescriptor[0];
                    break;
		    }
			break;

		case CONFIGURATION:	 		// configuration descriptor
								// It includes Configuration, Interface and Endpoint Table
#if (WEB_CAM_DEVICE==ENABLE)
			if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE)
			{
				U32 len=psDev->psControlCmd->Length;
				BOOL ok = UsbdWebCamFuncConfigDecrpGet(0, &len, &psDevDesc->pbDescriptorEx, FALSE);
				mpDebugPrint("((BYTE)psDev->psControlCmd->Value) = %d", ((BYTE)psDev->psControlCmd->Value));
				psDevDesc->wTxRxCounter = len;
				break;
			}
#endif
			switch ((BYTE)psDev->psControlCmd->Value)
			{
				case 0x00:		// configuration no: 0
                        if (psDev->psControlCmd->Length <= CONFIG_LENGTH)
                        {
                            psDevDesc->pbDescriptorEx = &gConfigDescriptor[0];
                            psDevDesc->wTxRxCounter = gConfigDescriptor[0];
                        }
                        else if (psDev->psControlCmd->Length > CONFIG_LENGTH)
                        {
                    		if (mUsbOtgHighSpeedST())      // First judge HS or FS??
        		                psDevDesc->pbDescriptorEx = psDevDesc->pbConfigHs;//&Cofig_HS_Desc[0];
		                    else
                		        psDevDesc->pbDescriptorEx = psDevDesc->pbConfigFs;//&Cofig_FS_Desc[0];


                            if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
                            {
                                psDevDesc->wTxRxCounter = CDC_CONFIG_TOTAL_LENGTH;
                            }
                            else
                            {
                                psDevDesc->wTxRxCounter = CONFIG_TOTAL_LENGTH;
                            }
                        }
                        else
                        {
                            psDevDesc->pbDescriptorEx = &gConfigDescriptor[0];
                            psDevDesc->wTxRxCounter = psDev->psControlCmd->Length;
                        }
			            break;
				//default:
	        	//		return FALSE;
			}
			 break;

		case STRING:   				// string descriptor
								// DescriptorIndex = low_byte of wValue
        switch ((BYTE)psDev->psControlCmd->Value)
        {
            case 0x00:
                psDevDesc->pbDescriptorEx = &gLangIdString[0];
                psDevDesc->wTxRxCounter = gLangIdString[0];
            break;

            case 0x01:
                psDevDesc->pbDescriptorEx = &gManufacturerString[0];
                psDevDesc->wTxRxCounter = gManufacturerString[0];
            break;

            case 0x02:
				if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE) {
                    psDevDesc->pbDescriptorEx = &gProductUSBcamString[0];
                    psDevDesc->wTxRxCounter = gProductUSBcamString[0];
				} else {
	                psDevDesc->pbDescriptorEx = &gProductString[0];
                    psDevDesc->wTxRxCounter = gProductString[0];
				}
                //psDevDesc->pbDescriptorEx = &gProductString[0];
                //psDevDesc->wTxRxCounter = gProductString[0];
            break;

            case 0x03:
			case 0x04:
                psDevDesc->pbDescriptorEx = &gSerialnumberString[0];
                psDevDesc->wTxRxCounter = gSerialnumberString[0];
            break;

            default:

                return FALSE;
        }
			 break;

		case INTERFACE:   			// interface descriptor
			// It cannot be accessed individually, it must follow "Configuraton"
	        	break;

		case ENDPOINT:   			// endpoint descriptor
			// It cannot be accessed individually, it must follow "Configuraton"
	            break;

		case DEVICE_QUALIFIER:   	// Device_Qualifier descritor
	        	psDevDesc->pbDescriptorEx = &gDeviceQualifierDescriptor[0];
   			psDevDesc->wTxRxCounter = gDeviceQualifierDescriptor[0];

	            break;

		case OTHER_SPEED:   		// Other_Speed_Configuration
			// It includes Configuration, Interface and Endpoint Table
			if (psDev->psControlCmd->Length == CONFIG_LENGTH)
			{
                if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
                {
                    psDevDesc->pbDescriptorEx = &gOtherSpeedCdcConfigurationDescriptor[0];
                    psDevDesc->wTxRxCounter = gOtherSpeedCdcConfigurationDescriptor[0];
                }
                else
                {
                    psDevDesc->pbDescriptorEx = &gOtherSpeedConfigurationDescriptor[0];
                    psDevDesc->wTxRxCounter = gOtherSpeedConfigurationDescriptor[0];
                }
			}
            else if (psDev->psControlCmd->Length > CONFIG_LENGTH)
            {
                if (mUsbOtgHighSpeedST())              // First judge HS or FS??
                {
                    psDevDesc->pbDescriptorEx = psDevDesc->pbConfigFsOtherSpeedDesc;//&Cofig_FS_Other_Speed_Desc[0];
                }
                else
                {
                    psDevDesc->pbDescriptorEx = psDevDesc->pbConfigHsOtherSpeedDesc;//&Cofig_HS_Other_Speed_Desc[0];
                }

                if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
                {
                    psDevDesc->wTxRxCounter = CDC_CONFIG_TOTAL_LENGTH;
                }
                else
                {
                    psDevDesc->wTxRxCounter = CONFIG_TOTAL_LENGTH;
                }

            }
	            break;

       	default:
	        	return FALSE;
	}

	if (psDevDesc->wTxRxCounter > psDev->psControlCmd->Length)
		psDevDesc->wTxRxCounter = psDev->psControlCmd->Length;

   			//if (psDevDesc->wTxRxCounter==0)//Bruce;;Add;;Test
   			//   {mUsbOtgEP0DoneSet();//Bruce;;Add;;Test
   			//    return TRUE;//Bruce;;Add;;Test
   			//   }//Bruce;;Add;;Test


	psDev->eUsbCxCommand = CMD_GET_DESCRIPTOR;
	vOTGEP0TxData(eWhichOtg);
	psDev->eUsbCxFinishAction  = ACT_DONE;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		bSet_OTGdescriptor()
//		Description:
//			1. Point to the start location of the correct descriptor.
//			2. Set the transfer length, and we will save data into sdram when Rx interrupt occure
//		input: none
//		output: TRUE or FALSE (BOOL)
///////////////////////////////////////////////////////////////////////////////
static BOOL bSet_OTGdescriptor(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;
// oid vUsbEP0RxData(void);

    switch ((BYTE)(psDev->psControlCmd->Value >> 8))
    {
        case 1: // device descriptor
        switch(psDevDesc->bUsbApMode)
        {
#if 0
            case USB_AP_SIDC_MODE:
                psDevDesc->pbDescriptorEx = &gStiDeviceDescriptor[0];
                psDevDesc->wTxRxCounter = gStiDeviceDescriptor[0];
            break;
#endif
            case USB_AP_VENDOR_MODE:
                psDevDesc->pbDescriptorEx = &gVendorDeviceDescriptor[0];
                psDevDesc->wTxRxCounter = gVendorDeviceDescriptor[0];
            break;
#if 0
            case USB_AP_CDC_MODE:
                psDevDesc->pbDescriptorEx = &gCdcDeviceDescriptor[0];
                psDevDesc->wTxRxCounter = gCdcDeviceDescriptor[0];
            break;
#endif
            case USB_AP_MSDC_MODE:
            default:
                psDevDesc->pbDescriptorEx = &gMsdcDeviceDescriptor[0];
                psDevDesc->wTxRxCounter = gMsdcDeviceDescriptor[0];
            break;
        }
        /*
        if (psDevDesc->bUsbApMode == USB_AP_MSDC_MODE)
        {
        psDevDesc->pbDescriptorEx = &gDeviceDescriptor[0];
        psDevDesc->wTxRxCounter = gDeviceDescriptor[0];
        }
        else
        {
        psDevDesc->pbDescriptorEx = &gVendorDeviceDescriptor[0];
        psDevDesc->wTxRxCounter = gVendorDeviceDescriptor[0];
        }
        */
        case 2:                 // configuration descriptor
        // It includes Configuration, Interface and Endpoint Table
        // DescriptorIndex = low_byte of wValue
        switch ((BYTE)psDev->psControlCmd->Value)
        {
        case 0x00:              // configuration no: 0
            psDevDesc->pbDescriptorEx = &gConfigDescriptor[0];
            psDevDesc->wTxRxCounter = gConfigDescriptor[0];
        break;
        default:
            return FALSE;
        }
        break;


        case 3:                 // string descriptor
        // DescriptorIndex = low_byte of wValue
        switch ((BYTE)psDev->psControlCmd->Value)
        {
            case 0x00:
                psDevDesc->pbDescriptorEx = &gLangIdString[0];
                psDevDesc->wTxRxCounter = gLangIdString[0];
            break;

            case 0x01:
                psDevDesc->pbDescriptorEx = &gManufacturerString[0];
                psDevDesc->wTxRxCounter = gManufacturerString[0];
            break;

            case 0x02:
#if 0
                if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
                {
                    psDevDesc->pbDescriptorEx = &gCdcProductString[0];
                    psDevDesc->wTxRxCounter = gCdcProductString[0];
                }
                else
                {
                    psDevDesc->pbDescriptorEx = &gProductString[0];
                    psDevDesc->wTxRxCounter = gProductString[0];
                }
#endif
                psDevDesc->pbDescriptorEx = &gProductString[0];
                psDevDesc->wTxRxCounter = gProductString[0];
            break;

            case 0x03:
                psDevDesc->pbDescriptorEx = &gSerialnumberString[0];
                psDevDesc->wTxRxCounter = gSerialnumberString[0];
            break;

            default:
                return FALSE;
        }
        break;
        default:
            return FALSE;
    }

    if (psDevDesc->wTxRxCounter > psDev->psControlCmd->Length)
        psDevDesc->wTxRxCounter = psDev->psControlCmd->Length;


    psDev->eUsbCxCommand = CMD_SET_DESCRIPTOR;

#if 0
	switch ((BYTE)(psDev->psControlCmd->Value >> 8))
	{
		case 1:					// device descriptor
	        	psDevDesc->pbDescriptorEx = &u8OTGDeviceDescriptorEX[0];
	        	psDevDesc->wTxRxCounter = u8OTGDeviceDescriptorEX[0];
	            break;

		case 2:	 				// configuration descriptor
			// It includes Configuration, Interface and Endpoint Table
			// DescriptorIndex = low_byte of wValue
			switch ((BYTE)psDev->psControlCmd->Value)
			{
				case 0x00:		// configuration no: 0
			        	psDevDesc->pbDescriptorEx = &gConfigDescriptor[0];
	        			psDevDesc->wTxRxCounter = gConfigDescriptor[0];
			            break;
				default:
	        			return FALSE;
			}
			 break;


		case 3:   				// string descriptor
			// DescriptorIndex = low_byte of wValue
			switch ((BYTE)psDev->psControlCmd->Value)
			{
				case 0x00:
			        	psDevDesc->pbDescriptorEx = &u8OTGString00DescriptorEX[0];
	        			psDevDesc->wTxRxCounter = u8OTGString00DescriptorEX[0];
			 		break;

			 	case 0x10:
			        	psDevDesc->pbDescriptorEx = &u8OTGString10DescriptorEX[0];
	        			psDevDesc->wTxRxCounter = u8OTGString10DescriptorEX[0];
			 		break;

			 	case 0x20:
			        	psDevDesc->pbDescriptorEx = &u8OTGString20DescriptorEX[0];
	        			psDevDesc->wTxRxCounter = u8OTGString20DescriptorEX[0];
			 		break;

			 	case 0x30:
			        	psDevDesc->pbDescriptorEx = &u8OTGString30DescriptorEX[0];
	        			psDevDesc->wTxRxCounter = u8OTGString30DescriptorEX[0];
			 		break;

			 	case 0x40:
			        	psDevDesc->pbDescriptorEx = &u8OTGString40DescriptorEX[0];
	        			psDevDesc->wTxRxCounter = u8OTGString40DescriptorEX[0];
			 		break;

			 	case 0x50:
			        	psDevDesc->pbDescriptorEx = &u8OTGString50DescriptorEX[0];
	        			psDevDesc->wTxRxCounter = u8OTGString50DescriptorEX[0];
			 		break;

			 	default:
	        			return FALSE;
			}
			 break;
       	default:
	        	return FALSE;
	}

	if (psDevDesc->wTxRxCounter > psDev->psControlCmd->Length)
		psDevDesc->wTxRxCounter = psDev->psControlCmd->Length;

	psDev->eUsbCxCommand = CMD_SET_DESCRIPTOR;
#endif
	psDev->eUsbCxFinishAction  = ACT_DONE;

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		vGet_OTGconfiguration()
//		Description:
//			1. Send 1 bytes configuration value to host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vGet_OTGconfiguration(WHICH_OTG eWhichOtg)
{
	BYTE u8Tmp[2];
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	u8Tmp[0] = psDevDesc->bUsbConfigValue;
	//vOTGCxFWr( u8Tmp, 1);
    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,u8Tmp,1,eWhichOtg);

	psDev->eUsbCxFinishAction  = ACT_DONE;
}



///////////////////////////////////////////////////////////////////////////////
//		bSet_OTGconfiguration()
//		Description:
//			1. Get 1 bytes configuration value from host.
//			2-1. if(value == 0) then device return to address state
//			2-2. if(value match descriptor table)
//					then config success & Clear all EP toggle bit
//			2-3	 else stall this command
//		input: none
//		output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static BOOL bSet_OTGconfiguration(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	if ((BYTE)psDev->psControlCmd->Value == 0)
	{
		psDevDesc->bUsbConfigValue = 0;
		mUsbOtgCfgClr();
	}
	else
	{
		if (mUsbOtgHighSpeedST())					// First judge HS or FS??
		{
			MP_DEBUG("high speed");
			vOTGSetEPMaxPacketSize(0x200, eWhichOtg);

			if ((BYTE)psDev->psControlCmd->Value > NUMBER_OF_CONFIGURATION)
				return FALSE;
			psDevDesc->bUsbConfigValue = (BYTE)psDev->psControlCmd->Value;
			vOTGFIFO_EPxCfg_HS(eWhichOtg);
			mUsbOtgSOFMaskHS();
		}
		else
		{
			MP_DEBUG("full speed");
			vOTGSetEPMaxPacketSize(0x40, eWhichOtg);
            MP_DEBUG("config val = %d", psDev->psControlCmd->Value);
			if ((BYTE)psDev->psControlCmd->Value > NUMBER_OF_CONFIGURATION)
				return FALSE;
			psDevDesc->bUsbConfigValue = (BYTE)psDev->psControlCmd->Value;
			vOTGFIFO_EPxCfg_FS(eWhichOtg);
			mUsbOtgSOFMaskFS();
		}
		mUsbOtgCfgSet();
		vOTGClrEPx(eWhichOtg);
	}

	if ((BYTE)psDev->psControlCmd->Value == 1)
	{
        if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
        { // CDC
            mUsbOtgIntF0INDis(); // do not check interrupt
            mUsbOtgIntF1OUTDis();
			mUsbOtgIntF1OUTEn();
        }
        else
        { // MSDC
#if USBOTG_DEVICE_ISO_TEST
            mUsbOtgIntF0INEn();
#else
            mUsbOtgIntF0INDis();
            if (psDevDesc->bUsbApMode == USB_AP_SIDC_MODE)
            { // SIDC
                mUsbOtgIntF2INEn();
            }
#endif

#if USBOTG_DEVICE_ISO_TEST
            mUsbOtgIntF2OUTEn();
#else
            mUsbOtgIntF1OUTEn();
#endif
        }
	}
#if USBOTG_DEVICE_ISO_TEST
	vOTG_ISO_Initial(mUsbOtgHighSpeedST());
#endif
	psDev->eUsbCxFinishAction  = ACT_DONE;
	Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_CONFIGURED);
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		bGet_OTGinterface()
//		Description:
//			Getting interface
//		input: none
//		output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static BOOL bGet_OTGinterface(WHICH_OTG eWhichOtg)
{
	BYTE u8Tmp[2];
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	if (mUsbOtgCfgST() == 0)
		return FALSE;

	// If there exists many interfaces, Interface0,1,2,...N,
	// You must check & select the specific one
	switch (psDevDesc->bUsbConfigValue)
	{
		// Configuration 1
		case 1:
			if (psDev->psControlCmd->Index > INTERFACE_NUMBER)
				return FALSE;
			break;
		default:
			return FALSE;
	}

	u8Tmp[0] = psDevDesc->bUsbInterfaceAlternateSetting;
	//vOTGCxFWr( u8Tmp, 1);
    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,u8Tmp,1,eWhichOtg);

	psDev->eUsbCxFinishAction  = ACT_DONE;
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
//		bSet_OTGinterface()
//		Description:
//			1-1. If (the device stays in Configured state)
//					&(command match the alternate setting)
//						then change the interface
//			1-2. else stall it
//		input: none
//		output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static BOOL bSet_OTGinterface(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	if (mUsbOtgCfgST() )
	{
		// If there exists many interfaces, Interface0,1,2,...N,
		// You must check & select the specific one
		switch (psDev->psControlCmd->Index)
		{
			case 0:	// Interface0
				if((BYTE)psDev->psControlCmd->Value == psDevDesc->bUsbDefaultInterfaceValue) // USB-IF MSC Test (ErrorRecovery)
				{
					psDevDesc->bUsbInterfaceValue = (BYTE)psDev->psControlCmd->Index;
					psDevDesc->bUsbInterfaceAlternateSetting = (BYTE)psDev->psControlCmd->Value;
					if (mUsbOtgHighSpeedST())					// First judge HS or FS??
						vOTGFIFO_EPxCfg_HS(eWhichOtg);
					else
						vOTGFIFO_EPxCfg_FS(eWhichOtg);
					vOTGClrEPx(eWhichOtg);
					psDev->eUsbCxFinishAction  = ACT_DONE;
					return TRUE;
				}
			case 1:	// Interface1
			case 2:	// Interface2
			default:
				break;
		}
	}
	return FALSE;

}

#if 0 // no use
///////////////////////////////////////////////////////////////////////////////
//		bSynch_OTGframe()
//		Description:
//			1. If the EP is a Iso EP, then return the 2 bytes Frame number.
//				 else stall this command
//		input: none
//		output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static BOOL bSynch_OTGframe(void)
{
	SBYTE TransferType;
	WORD u16Tmp;

	TransferType = -1;
	// Does the Endpoint support Isochronous transfer type?
	switch(psDev->psControlCmd->Index)
	{
		case 1:		// EP1
			TransferType = gConfigDescriptor[22] & 0x03;
			break;
#if 0
		case 2:		// EP2
			TransferType = gConfigDescriptor[29] & 0x03;
			break;

		case 3:		// EP3
			TransferType = gConfigDescriptor[36] & 0x03;
			break;

		case 4:		// EP4
			TransferType = gConfigDescriptor[43] & 0x03;
			break;

		case 5:		// EP5
			TransferType = gConfigDescriptor[50] & 0x03;
			break;
#endif
		default:
			break;
	}

	if (TransferType == 1)	// Isochronous
	{
		u16Tmp = mUsbOtgFrameNo();
		//vOTGCxFWr( (BYTE*)&u16Tmp, 2);
        bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,(BYTE*)&u16Tmp,2);

		psDev->eUsbCxFinishAction  = ACT_DONE;
		return TRUE;
	}
	else
		return FALSE;
}
#endif // 0
///////////////////////////////////////////////////////////////////////////////
//		bOTGStandardCommand()
//		Description:
//			1. Process standard Cx 8 bytes command.
//		input: none
//		output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static BOOL bOTGStandardCommand(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;
    #if 1
    switch (psDev->psControlCmd->Request)
    {
    	case GET_STATUS:
    		MP_DEBUG("-usbotg%d- get status", eWhichOtg);
    		break;
    	case CLEAR_FEATURE:
    		MP_DEBUG("-usbotg%d- clear feature", eWhichOtg);
    		break;
    	case RESERVED1:
    		MP_DEBUG("-usbotg%d- reserved", eWhichOtg);
    		break;
    	case SET_FEATURE:
    		MP_DEBUG("-usbotg%d- set feature", eWhichOtg);
    		break;
    	case RESERVED2:
    		MP_DEBUG("-usbotg%d- reserved", eWhichOtg);
    		break;
    	case SET_ADDRESS:
    		MP_DEBUG("-usbotg%d- set address", eWhichOtg);
    		break;
    	case GET_DESCRIPTOR:
    		MP_DEBUG("-usbotg%d- get descriptor", eWhichOtg);
    		break;
    	case SET_DESCRIPTOR:
    		MP_DEBUG("-usbotg%d- set descriptor", eWhichOtg);
    		break;
    	case GET_CONFIGURATION:
    		MP_DEBUG("-usbotg%d- get configuration", eWhichOtg);
    		break;
    	case SET_CONFIGURATION:
    		MP_DEBUG("-usbotg%d- set configuration", eWhichOtg);
    		break;
    	case GET_INTERFACE:
    		MP_DEBUG("-usbotg%d- get interface", eWhichOtg);
    		break;
    	case SET_INTERFACE:
    		MP_DEBUG("-usbotg%d- set interface", eWhichOtg);
    		break;
    	case SYNC_FRAME:
    		MP_DEBUG("USBOTG%d - synch frame", eWhichOtg);
    		break;
        case SETUP_ERROR:
    	default:
    		MP_DEBUG("-usbotg%d- undefined standard request code %x", eWhichOtg, psDev->psControlCmd->Request);
    		break;
    }
    #endif

	switch (psDev->psControlCmd->Request) // by Standard Request codes
   	{
   		case GET_STATUS:		// get status
   			return (bGet_OTGstatus(eWhichOtg));

		case CLEAR_FEATURE:		// clear feature
			return (bClear_OTGfeature(eWhichOtg));

		case RESERVED1:		// Reserved for further use
   			break;

		case SET_FEATURE:		// set feature

			return (bSet_OTGfeature(eWhichOtg));

		case RESERVED2:		// Reserved for further use
   			break;

   		case SET_ADDRESS:		// set address
#if (WEB_CAM_DEVICE==ENABLE)
			if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE)
				UsbdWebCamFuncInit();
#endif
   			if (!psDevDesc->boOtgEp0HaltSt)
				return(bSet_OTGaddress(eWhichOtg));
   			break;

		case GET_DESCRIPTOR:		// get descriptor
			if (!psDevDesc->boOtgEp0HaltSt)
				return(bGet_OTGdescriptor(eWhichOtg));
			break;

		case SET_DESCRIPTOR:		// set descriptor
			if (!psDevDesc->boOtgEp0HaltSt)
				return (bSet_OTGdescriptor(eWhichOtg));
	        break;

		case GET_CONFIGURATION:		// get configuration
			if (!psDevDesc->boOtgEp0HaltSt)
				vGet_OTGconfiguration(eWhichOtg);
			return TRUE;

		case SET_CONFIGURATION:		// set configuration
			if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE)
			{
				if (!psDevDesc->boOtgEp0HaltSt)
				{
				    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

					if (mUsbOtgHighSpeedST())					// First judge HS or FS??
						mUsbOtgSOFMaskHS();
					else
						mUsbOtgSOFMaskFS();

					psDevDesc->bUsbConfigValue = (BYTE)psDev->psControlCmd->Value;
					psDev->eUsbCxFinishAction  = ACT_DONE;
					Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_CONFIGURED);
					return TRUE;
				}
				break;
			}
			if (!psDevDesc->boOtgEp0HaltSt)
				return bSet_OTGconfiguration(eWhichOtg);
			break;

		case GET_INTERFACE:	// get interface
			if (!psDevDesc->boOtgEp0HaltSt)
				return(bGet_OTGinterface(eWhichOtg));
	        break;

	    case SET_INTERFACE:	// set interface
#if (WEB_CAM_DEVICE==ENABLE)
			if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE)
			{
				if (!psDevDesc->boOtgEp0HaltSt)
					return UsbdInfSet(eWhichOtg);
				break;
			}
#endif
			if (!psDevDesc->boOtgEp0HaltSt)
				return(bSet_OTGinterface(eWhichOtg));
			break;

		case SYNC_FRAME:	// synch frame
			//if (!psDevDesc->boOtgEp0HaltSt)
			//	return(bSynch_OTGframe());
			break;

		default:
			break;
   	}
   	return FALSE;
}

static void bOTGBulkOnlyMassStorageReset(WHICH_OTG eWhichOtg) // USB-IF MSC Test (ErrorRecovery)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    mUsbOtgClrAllFIFOSet();
    mUsbOtgEPinRsTgSet(EP1);        // Set Rst_Toggle Bit
    mUsbOtgEPinRsTgClr(EP1);        // Clear Rst_Toggle Bit
    mUsbOtgEPinStallClr(EP1);       // Clear Stall Bit
    mUsbOtgEPoutRsTgSet(EP2);       // Set Rst_Toggle Bit
    mUsbOtgEPoutRsTgClr(EP2);       // Clear Rst_Toggle Bit
    mUsbOtgEPoutStallClr(EP2);      // Clear Stall Bit
}

static BOOL bOTGClassCommand(WHICH_OTG eWhichOtg)
{
	BOOL	ret = FALSE;
	BYTE	how_many_lun = 0x0;
	BYTE	data[7];
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

#if (WEB_CAM_DEVICE==ENABLE)
	if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE)
	{
		ret = UsbdWebCamFunctionClassReq(eWhichOtg);
		if (ret != FALSE)
		{
			psDev->eUsbCxFinishAction = ACT_DONE;
		}
		mpDebugPrint("psDev->psControlCmd->Request = %d", psDev->psControlCmd->Request);
		return ret;
	}
#endif
	if (psDev->psControlCmd->Direction == 0x80)
	{
		switch (psDev->psControlCmd->Request)
		{
			case GET_MAX_LUN:
                how_many_lun = Mcard_GetMaxLun()-1;
                MP_DEBUG("-USBOTG%d- Device Lun:0 ~ Lun:%d Total:%d", eWhichOtg, how_many_lun, how_many_lun+1);
				//vOTGCxFWr(&how_many_lun, (DWORD)psDev->psControlCmd->Length);
                bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,&how_many_lun, (DWORD)psDev->psControlCmd->Length,eWhichOtg);
				psDev->eUsbCxFinishAction  = ACT_DONE;
				ret = TRUE;
				break;
			case CANCEL_REQUEST:
				psDev->eUsbCxCommand = CANCEL_REQUEST;
				//vSidcCancelRequest(&psDevDesc->wTxRxCounter, &psDevDesc->pbDescriptorEx);
				MP_DEBUG("CANCEL_REQUEST");
				//__asm("break 100");
				psDev->eUsbCxFinishAction  = ACT_DONE;
				ret = TRUE;
				break;
			case GET_EXTENDED_EVENT_DATA:
				break;
			case DEVICE_RESET_REQUEST:
				break;
			case GET_DEVICE_STATUS:
				psDev->eUsbCxCommand = GET_DEVICE_STATUS;
				//vSidcGetDeviceStatus(&psDevDesc->wTxRxCounter, &psDevDesc->pbDescriptorEx);
				//__asm("break 100");
				MP_DEBUG("GET_DEVICE_STATUS");
				//vOTGCxFWr( psDevDesc->pbDescriptorEx, psDevDesc->wTxRxCounter);
                bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,psDevDesc->pbDescriptorEx,psDevDesc->wTxRxCounter,eWhichOtg);
				psDev->eUsbCxFinishAction  = ACT_DONE;
				ret = TRUE;
				break;
			case GET_LINE_CODING:
				data[0] = LO_BYTE_OF_DWORD(USB_CDC_SPEED);
				data[1] = MIDLO_BYTE_OF_DWORD(USB_CDC_SPEED);
				data[2] = MIDHI_BYTE_OF_DWORD(USB_CDC_SPEED);
				data[3] = HI_BYTE_OF_DWORD(USB_CDC_SPEED);
				data[4] = 0;
				data[5] = 0;
				data[6] = 0x08;
				//vOTGCxFWr( &data[0], (DWORD)7);
                bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,&data[0],(DWORD)7,eWhichOtg);
				psDev->eUsbCxFinishAction  = ACT_DONE;
				ret = TRUE;
				break;
			default:
				MP_DEBUG1("psDev->psControlCmd->Request = %d", psDev->psControlCmd->Request);
				break;
		}
	}
	else
	{
		switch (psDev->psControlCmd->Request)
		{
           case BULK_ONLY_MASS_STORAGE_RESET:  // USB-IF  MSC Test (ErrorRecovery)
                bOTGBulkOnlyMassStorageReset(eWhichOtg);
                psDev->eUsbCxFinishAction  = ACT_DONE;
                ret = TRUE;
                break;
			case SET_LINE_CODING:
				psDevDesc->wTxRxCounter = 7;
				ret = TRUE;
				break;
			case SET_CONTORL_LINE_STATE:
				psDev->eUsbCxFinishAction  = ACT_DONE;
				ret = TRUE;

                            if(psDev->psControlCmd->Value & SET_CONTORL_LINE_STATE_ACTIVATE) // Activate
                                SetIsAbleToUseUsbdCdc(TRUE, eWhichOtg);
                            else
                                SetIsAbleToUseUsbdCdc(FALSE, eWhichOtg);

				break;
			default:
			break;
		}
	}
return ret;
}


static void vCxIN_VendorTxData(WHICH_OTG eWhichOtg)
{
	BYTE u8temp;
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;


	if(psDevDesc->wTxRxCounter < EP0MAXPACKETSIZE)
		u8temp = (BYTE)psDevDesc->wTxRxCounter;
	else
		u8temp = EP0MAXPACKETSIZE;

	psDevDesc->wTxRxCounter -= (WORD)u8temp;
	//vOTGCxFWr(psDevDesc->pbDescriptorEx , u8temp);
    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,psDevDesc->pbDescriptorEx,u8temp,eWhichOtg);
	psDevDesc->pbDescriptorEx = psDevDesc->pbDescriptorEx + u8temp;

	// end of the data stage
	if (psDevDesc->wTxRxCounter == 0)
	{
#if 1
		psDev->eUsbCxFinishAction  = ACT_DONE;
#else
		mUsbOtgEP0DoneSet();
		psDev->eUsbCxFinishAction  = ACT_IDLE;
#endif
		psDev->eUsbCxCommand = CMD_VOID;
	}

}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ep0setup()
//		Description:
//			1. Read 8-byte setup packet.
//			2. Decode command as Standard, Class, Vendor or NOT support command
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void vOTG_ep0setup(WHICH_OTG eWhichOtg)
{
	DWORD u32UsbCmd[2];
	BYTE  c;
	BYTE  *u8UsbCmd;
	//BYTE  u8index;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

	// First we must check if this is the first Cx 8 byte command after USB reset.
	// If this is the first Cx 8 byte command, we can check USB High/Full speed right now.
	if(!psDevDesc->boOtgChirpFinish)
	{
		// first ep0 command after usb reset, means we can check usb speed right now.
		psDevDesc->boOtgChirpFinish = TRUE;

		if (mUsbOtgHighSpeedST())					// First we should judge HS or FS
		{
			psDevDesc->boOtgHighSpeed = TRUE;
#if 0
			// Device stays in High Speed
			// copy Device descriptors (from rom to sdram)
			for (u8index = 0 ; u8index < sizeof(u8OTGHSDeviceDescriptor); u8index ++)
				u8OTGDeviceDescriptorEX[u8index] = u8OTGHSDeviceDescriptor[u8index];
#endif
			// copy Device Qualifierdescriptors (from rom to sdram)
			// bLength
			//gDeviceQualifierDescriptor[0] = DEVICE_QUALIFIER_LENGTH;
			// bDescriptorType Device_Qualifier
			//gDeviceQualifierDescriptor[1] = DT_DEVICE_QUALIFIER;
#if 0
			for (u8index = 2 ; u8index < 8; u8index ++)
				gDeviceQualifierDescriptor[u8index] = u8OTGFSDeviceDescriptor[u8index];
			// Number of Other-speed Configurations
			gDeviceQualifierDescriptor[8] = u8OTGFSDeviceDescriptor[17];
#endif
			//Reserved for future use, must be zero
			//gDeviceQualifierDescriptor[9] = 0x00;
#if 0
			// copy Config descriptors (from rom to sdram)
			for (u8index = 0 ; u8index < sizeof(u8HSConfigOTGDescriptor01); u8index ++)
				gConfigDescriptor[u8index] = u8HSConfigOTGDescriptor01[u8index];

			// copy Other speed Config descriptors (from rom to sdram)
			for (u8index = 0 ; u8index < sizeof(u8FSConfigOTGDescriptor01); u8index ++)
				gOtherSpeedConfigurationDescriptor[u8index] = u8FSConfigOTGDescriptor01[u8index];
#endif
			// Change Descriptor type "DT_OTHER_SPEED_CONFIGURATION"
			//gOtherSpeedConfigurationDescriptor[1] = DT_OTHER_SPEED_CONFIGURATION;
		}
		else
		{
			psDevDesc->boOtgHighSpeed = FALSE;
#if 0
			// Device stays in Full Speed
			// copy Device descriptors (from rom to sram)
			for (u8index = 0 ; u8index < sizeof(u8OTGFSDeviceDescriptor); u8index ++)
				u8OTGDeviceDescriptorEX[u8index] = u8OTGFSDeviceDescriptor[u8index];
#endif
			// copy Device Qualifierdescriptors (from rom to sram)
			// bLength
			//gDeviceQualifierDescriptor[0] = DEVICE_QUALIFIER_LENGTH;
			// bDescriptorType Device_Qualifier
			//gDeviceQualifierDescriptor[1] = DT_DEVICE_QUALIFIER;
#if 0
			for (u8index = 2 ; u8index < 8; u8index ++)
				gDeviceQualifierDescriptor[u8index] = u8OTGHSDeviceDescriptor[u8index];
			// Number of Other-speed Configurations
			gDeviceQualifierDescriptor[8] = u8OTGHSDeviceDescriptor[17];
#endif
			//Reserved for future use, must be zero
			//gDeviceQualifierDescriptor[9] = 0x00;
#if 0
			// copy Config descriptors (from rom to sram)
			for (u8index = 0 ; u8index < sizeof(u8FSConfigOTGDescriptor01); u8index ++)
				gConfigDescriptor[u8index] = u8FSConfigOTGDescriptor01[u8index];
			// copy Other speed Config descriptors (from rom to sram)
			for (u8index = 0 ; u8index < sizeof(u8HSConfigOTGDescriptor01); u8index ++)
				gOtherSpeedConfigurationDescriptor[u8index] = u8HSConfigOTGDescriptor01[u8index];
#endif
			// Change Descriptor type "DT_OTHER_SPEED_CONFIGURATION"
			//gOtherSpeedConfigurationDescriptor[1] = DT_OTHER_SPEED_CONFIGURATION;
		}
#if 0
		// copy String descriptors (from rom to sram)
		for (u8index = 0 ; u8index < sizeof(u8OTGString00Descriptor); u8index ++)
			u8OTGString00DescriptorEX[u8index] = u8OTGString00Descriptor[u8index];

		for (u8index = 0 ; u8index < sizeof(u8OTGString10Descriptor); u8index ++)
			u8OTGString10DescriptorEX[u8index] = u8OTGString10Descriptor[u8index];

		for (u8index = 0 ; u8index < sizeof(u8OTGString20Descriptor); u8index ++)
			u8OTGString20DescriptorEX[u8index] = u8OTGString20Descriptor[u8index];

		for (u8index = 0 ; u8index < sizeof(u8OTGString30Descriptor); u8index ++)
			u8OTGString30DescriptorEX[u8index] = u8OTGString30Descriptor[u8index];

		for (u8index = 0 ; u8index < sizeof(u8OTGString40Descriptor); u8index ++)
			u8OTGString40DescriptorEX[u8index] = u8OTGString40Descriptor[u8index];

		for (u8index = 0 ; u8index < sizeof(u8OTGString50Descriptor); u8index ++)
			u8OTGString50DescriptorEX[u8index] = u8OTGString50Descriptor[u8index];
#endif
	}

	//u32UsbCmd = (DWORD*)malloc(8);
	// Read 8-byte setup packet from FIFO
	mUsbOtgDMA2FIFOSel(FOTG200_DMA2CxFIFO);
	u32UsbCmd[0] = mUsbOtgEP0CmdDataRdDWord();
	u32UsbCmd[1] = mUsbOtgEP0CmdDataRdDWord();
	mUsbOtgDMA2FIFOSel(FOTG200_DMA2FIFO_Non);

	u32UsbCmd[0] = BYTE_SWAP_OF_DWORD(u32UsbCmd[0]);
	u32UsbCmd[1] = BYTE_SWAP_OF_DWORD(u32UsbCmd[1]);

	u8UsbCmd = (BYTE*)&u32UsbCmd[0];
   	c = u8UsbCmd[0];                                   		// get 1st byte

    PSETUP_PACKET pControlCmd;
    pControlCmd = psDev->psControlCmd;

   	pControlCmd->Direction = (unsigned char)(c & 0x80);	// xfer Direction(IN, OUT)
	pControlCmd->Type = (unsigned char)(c & 0x60);         // type(Standard, Class, Vendor)
   	pControlCmd->Object = (unsigned char)(c & 0x03);       // Device, Interface, Endpoint

	pControlCmd->Request = u8UsbCmd[1];	                // get 2nd byte

    pControlCmd->Value = u8UsbCmd[2];                     	// get 3rd byte
	c = u8UsbCmd[3];										// get 4th byte
   	pControlCmd->Value |= (c<<8);

   	pControlCmd->Index = u8UsbCmd[4];						// get 5th byte
    c = u8UsbCmd[5];										// get 6th byte
	pControlCmd->Index |= (c<<8);

   	pControlCmd->Length = u8UsbCmd[6];						// get 7th byte
	c = u8UsbCmd[7];										// get 8th byte
   	pControlCmd->Length |= (c<<8);

    #if (USBOTG_DEVICE_EXTERN||USBOTG_DEVICE_EXTERN_SAMSUNG)
    if( SetupVendorCommand(eWhichOtg)) // Samsung Setup Vendor Command
        return;
    #endif

	//  Command Decode
	if (pControlCmd->Type == 0)							// standard command
	{
		if (bOTGStandardCommand(eWhichOtg) == FALSE)
			psDev->eUsbCxFinishAction = ACT_STALL;
  	}
  	else if (pControlCmd->Type>>5 == 1)   // class command
	{
		if (bOTGClassCommand(eWhichOtg) == FALSE)
			psDev->eUsbCxFinishAction = ACT_STALL;
	}
	else if (pControlCmd->Type>>5 == 2)   // vendor command
	{
		// Vendor command test (for Cx OUT test)
		// If we OUT Cx data as below, FOTG200 Device will wait for .
		if((u8UsbCmd[0] == 0x40)&&
		   (u8UsbCmd[1] == 0x00)&&
		   (u8UsbCmd[2] == 0x00)&&
		   (u8UsbCmd[3] == 0x00)&&
		   (u8UsbCmd[4] == 0x00)&&
		   (u8UsbCmd[5] == 0x00))
		{
			;//vCxOUT_VendorTest();
		}
		else if((u8UsbCmd[0] == 0xC0)&&
			   (u8UsbCmd[1] == 0x00)&&
			   (u8UsbCmd[2] == 0x00)&&
			   (u8UsbCmd[3] == 0x00)&&
			   (u8UsbCmd[4] == 0x00)&&
			   (u8UsbCmd[5] == 0x00))
		{
			//vCxIN_VendorTest();
			vCxIN_VendorTxData(eWhichOtg);
		}
		else
		{
			psDev->eUsbCxFinishAction = ACT_STALL;
		}

	}
	else
	{
		// Invalid(bad) command, Return EP0_STALL flag
		psDev->eUsbCxFinishAction = ACT_STALL;
	}

	//free(u32UsbCmd);
}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_rst()
//		Description:
//			1. Change descriptor table (High or Full speed).
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_rst(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psUsbOtg->sUsbDev.sDesc;
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);

	mUsbOtgDevAddrSet(0);
	//mUsbClrAllFIFOSet();
    OtgSetDescriptor(eWhichOtg);
	// Init AP
	//vUsb_APInit();
	vOtg_APInit(eWhichOtg);

	vFOTG200_Dev_Init(eWhichOtg);
    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        mUsbOtgIntF0INDis(); // do not check interrupt
        mUsbOtgIntF1OUTDis();
        //mUsbIntF1OUTEn();
    }
    else
    {
        mUsbOtgIntF0INDis();
        mUsbOtgIntF1OUTEn();
    }

	mUsbOtgIntBusRstClr();
	psDevDesc->boOtgChirpFinish = FALSE;
	Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_DEFAULT);
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_suspend()
//		Description:
//			1. Clear suspend interrupt, and set suspend register.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_suspend(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
#if 0
	if(u8OTGMessageLevel & MESS_INFO)
		printf("L%x: Bus suspend\n", u8LineOTGCount ++);
#endif
	// We have already set USB suepend counter in vFOTG200_Dev_Init().
	// Before enter into the suspend mode, we must finish all things about USB.
	// And then USB device into Suspend mode.
	mUsbOtgIntSuspClr();
	//Reserve for OTG;;mUsbGoSuspend();
	Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_SUSPENDED);

#if WEB_CAM_DEVICE
extern void usbdCamClrConfig(WHICH_OTG eWhichOtg);
	usbdCamClrConfig(eWhichOtg);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_resm()
//		Description:
//			1. Clear resume interrupt status and leave supend mode.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_resm(PST_USB_OTG_DES psUsbOtg)
{
#if 0
	if(u8OTGMessageLevel & MESS_INFO)
		printf("L%x: Bus resume\n", u8LineOTGCount ++);
#endif
	mUsbOtgIntResmClr();
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ISO_SeqErr()
//		Description:
//			1. FOTG200 Device Detects High bandwidth isochronous Data PID sequential error.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_ISO_SeqErr(PST_USB_OTG_DES psUsbOtg)
{
	BYTE u8Tmp = mUsbOtgIntIsoSeqErrRd();
	//BYTE i;
	mUsbOtgIntIsoSeqErrSetClr(u8Tmp);
#if 0
	for(i = 1; i < 8; i ++)
	{
		if(u8Tmp & (BIT0 << i))
		{
			if(u8OTGMessageLevel & MESS_INFO)
			{
			   	printf("L%x: EP%x Isochronous Sequential Error\n", u8LineOTGCount ++, i);
			}
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_ISO_SeqAbort()
//		Description:
//			1. FOTG200 Device Detects High bandwidth isochronous Data PID sequential abort.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_ISO_SeqAbort(PST_USB_OTG_DES psUsbOtg)
{
	BYTE u8Tmp = mUsbOtgIntIsoSeqAbortRd();
	//BYTE i;
	mUsbOtgIntIsoSeqAbortSetClr(u8Tmp);
#if 0
	for(i = 1; i < 8; i ++)
	{
		if(u8Tmp & (BIT0 << i))
		{
			if(u8OTGMessageLevel & MESS_INFO)
			{
			   	printf("L%x: EP%x Isochronous Sequential Abort\n", u8LineOTGCount ++, i);
			}
		}
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_TX0Byte()
//		Description:
//			1. Send 0 byte data to host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_TX0Byte(PST_USB_OTG_DES psUsbOtg)
{
	BYTE u8Tmp = mUsbOtgIntTX0ByteRd();
	BYTE i;
	mUsbOtgIntTX0ByteSetClr(u8Tmp);
#if 0
	for(i = 1; i < 8; i ++)
	{
		if(u8Tmp & (BIT0 << i))
		{
			if(u8OTGMessageLevel & MESS_INFO)
			{
//			   	printf("L%x: EP%x IN data 0 byte to host\n", u8LineOTGCount ++, i);
			}
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_RX0Byte()
//		Description:
//			1. Receive 0 byte data from host.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_RX0Byte(PST_USB_OTG_DES psUsbOtg)
{
	BYTE u8Tmp = mUsbOtgIntRX0ByteRd();
	//BYTE i;
	mUsbOtgIntRX0ByteSetClr(u8Tmp);
#if 0
	for(i = 1; i < 8; i ++)
	{
		if(u8Tmp & (BIT0 << i))
		{
			if(u8OTGMessageLevel & MESS_ERROR)
			{
			   	printf("L%x: EP%x OUT data 0 byte to Device\n", u8LineOTGCount ++, i);
			}
		}
	}
#endif
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ep0abort()
//		Description:
//			1. Stall this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_ep0abort(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psUsbOtg->sUsbDev.sDesc;

	mUsbOtgIntEP0AbortClr();								// Clean EP0 abort
	if(psDevDesc->bUsbMessageLevel & (MESS_ERROR))
		;//printf("L%x: EP0 command abort\n", u8LineOTGCount ++);

}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ep0tx()
//		Description:
//			1. Transmit data to EP0 FIFO.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_ep0tx(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;

	switch(psDev->eUsbCxCommand)
	{
		case CMD_GET_DESCRIPTOR:
			vOTGEP0TxData(eWhichOtg);
   			break;
#if 0
		case CMD_CxIN_Vendor:
			vCxIN_VendorTxData();
   			break;
#endif
		default:
MP_DEBUG("stall 1");
			mUsbOtgEP0StallSet();
			break;
	}
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ep0rx()
//		Description:
//			1. Receive data from EP0 FIFO.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void vOTG_ep0rx(WHICH_OTG eWhichOtg)
{
    BYTE i;
    BYTE data[7];
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;

	switch(psDev->psControlCmd->Request)
	{
		case CMD_SET_DESCRIPTOR:
			vOTGEP0RxData(eWhichOtg);
   			break;
#if 0
		case CMD_CxOUT_Vendor:
			vCxOUT_VendorRxData();
			break;
#endif

        case SET_LINE_CODING:
            bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_OUT,data,8,eWhichOtg);
            psDev->eUsbCxCommand = CMD_VOID;
            psDev->eUsbCxFinishAction = ACT_DONE;
            break;

		default:
            MP_DEBUG("stall 2");
			mUsbOtgEP0StallSet();
			break;
	}
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ep0end()
//		Description:
//			1. End this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_ep0end(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;

	psDev->eUsbCxCommand = CMD_VOID;
	mUsbOtgEP0DoneSet();								// Return EP0_Done flag
}


///////////////////////////////////////////////////////////////////////////////
//		vOTG_ep0fail()
//		Description:
//			1. Stall this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_ep0fail(PST_USB_OTG_DES psUsbOtg)
{
MP_DEBUG("stall 3");
	mUsbOtgEP0StallSet();								// Return EP0_Stall
	//if(u8OTGMessageLevel & (MESS_ERROR | MESS_WARNING | MESS_INFO))
	//	printf("L%x: EP0 fail\n", u8LineOTGCount ++);

}


#define CBW_SIGNATE                 0x55534243
#define CSW_SIGNATE                 0x55534253
#define CBW_LEN                         31
#define CSW_LEN                         13
#define CSW_STATUS_SCSI_PASS        0x00
#define CSW_STATUS_SCSI_FAIL        0x01
#define CSW_STATUS_PHASE_ERROR      0x02






///////////////////////////////////////////////////////////////////////////////
//		vUsb_FIFO_INT_action()
//		Description: FIFO interrupt enable or not
//					 depend on the STORAGE state
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
//static void vUsb_FIFO_INT_action(TRANSACTION_STATE eState)
static void vUsbOtg_FIFO_INT_action(TRANSACTION_STATE eState, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    switch(eState)
    {
        case STATE_CBW:
        case STATE_CB_DATA_OUT:
        case STATE_PTP_CMD:
        case STATE_PTP_DATA_OUT:
            //    MP_DEBUG("STATE_CB_DATA_OUT");
            mUsbOtgIntF0INDis();
        break;
        case STATE_CSW:
        case STATE_CB_DATA_IN:
        case STATE_PTP_DATA_IN:
        case STATE_PTP_RES:
        case STATE_DATA_OUT_STALL: // USB-IF MSC Test (CommandSet)
            //   MP_DEBUG("STATE_CB_DATA_IN");
            mUsbOtgIntF0INEn();
        break;
        case STATE_PTP_INT_IN:
        default:
        break;
    }
}


///////////////////////////////////////////////////////////////////////////////
//		vUsb_F1_Out()
//		Description: USB FIFO1 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void vOTG_F1_Out(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;

    //MP_DEBUG("-USBOTG%d- OUT-Token", eWhichOtg);

	switch(psDev->eUsbTransactionState)
	{
        case STATE_DATA_OUT_STALL:
            psDev->eUsbTransactionState = eUsbMsDataOutStall(eWhichOtg);
            break;
		case STATE_CBW:
                    //MP_DEBUG("STATE_CBW");
                    //if( u16FIFOByteCount == 31) {
                    psDev->eUsbTransactionState = eUsbProcessCbw(u16FIFOByteCount, eWhichOtg);
                    if (psDev->eUsbTransactionState == STATE_DATA_OUT_STALL)
                    {
                        MP_DEBUG("STALL");
                    }
                    //}
                    //else  __asm("break 100");
			break;
		case STATE_CB_DATA_OUT:
                    //MP_DEBUG("STATE_CB_DATA_OUT");
                    psDev->eUsbTransactionState = eUsbMsDataOut(u16FIFOByteCount, eWhichOtg);
			break;
            
#if USBOTG_DEVICE_SIDC
		case STATE_PTP_CMD:
                    //MP_DEBUG("CMD");
                    psDev->eUsbTransactionState = eUsbProcessPtp(u16FIFOByteCount, eWhichOtg);

	                if (psDev->eUsbTransactionState == STATE_IDLE)
	                {
	                    MP_DEBUG("STATE_IDLE");
	                    mUsbOtgEPoutStallSet(EP2);
	                }


                    break;
		case STATE_PTP_DATA_OUT:
                    //MP_DEBUG("OUT");
                    psDev->eUsbTransactionState = eUsbPtpDataOut(u16FIFOByteCount, eWhichOtg);
			break;
#endif // USBOTG_DEVICE_SIDC

		#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
		case STATE_EXTERN_OUT:   // Side Monitor
                    //mpDebugPrint("OUT");
                    psDev->eUsbTransactionState = eUsbExternDataOut(u16FIFOByteCount, eWhichOtg);
                    break;
		#endif

		default:
                    MP_DEBUG("default: -usbotg%d- Out State:%x", eWhichOtg, psDev->eUsbTransactionState);
                    MP_DEBUG("length = %x", u16FIFOByteCount);
			break;
	}

#if 0
	switch(psDev->eUsbTransactionState)
	{
		case STATE_CBW:
			if (u16FIFOByteCount == 31){
				psDev->eUsbTransactionState = eUsbProcessCbw();
                     }
                     else
			{
				__asm("break 100");
			}
			break;
		case STATE_CB_DATA_OUT:
			psDev->eUsbTransactionState = eUsbMsDataOut(u16FIFOByteCount);
			break;
		default:
			break;
	}
#endif
	vUsbOtg_FIFO_INT_action(psDev->eUsbTransactionState, eWhichOtg);
}


///////////////////////////////////////////////////////////////////////////////
//		vUsb_F0_In()
//		Description: USB FIFO0 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_F0_In(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

    //MP_DEBUG("-USBOTG%d- IN-Token", eWhichOtg);

	switch(psDev->eUsbTransactionState)
	{
        case STATE_DATA_IN_STALL:
            psDev->eUsbTransactionState = eUsbMsDataInStall(eWhichOtg);
            break;
        case STATE_DATA_OUT_STALL:
            psDev->eUsbTransactionState = eUsbMsDataOutStall(eWhichOtg);
            break;
		case STATE_CSW:
			//MP_DEBUG("STATE_CSW");
            psDev->eUsbTransactionState = eUsbMsSendCsw(eWhichOtg);
			break;
		case STATE_CB_DATA_IN:
			//MP_DEBUG("STATE_CB_DATA_IN");
            psDev->eUsbTransactionState = eUsbMsDataIn(eWhichOtg);
			break;
#if USBOTG_DEVICE_SIDC
        case  STATE_PTP_DATA_IN:
			//MP_DEBUG("IN");
            psDev->eUsbTransactionState = eUsbPtpDataIn(eWhichOtg);
			break;
        case  STATE_PTP_RES:
			//MP_DEBUG("RES");
            psDev->eUsbTransactionState = eUsbPtpSendResponse(eWhichOtg);
            psDevDesc->boIsEnableIntEp = TRUE;
			break;
        case  STATE_PTP_INT_IN:
			//MP_DEBUG("INT_IN");
			break;
#endif // USBOTG_DEVICE_SIDC
	    default:
			MP_DEBUG("default: -usbotg%d- In State:%x", eWhichOtg, psDev->eUsbTransactionState);
			break;
	}

#if 0
	switch(psDev->eUsbTransactionState)
	{
		case STATE_CSW:
			psDev->eUsbTransactionState = eUsbMsSendCsw();
			break;
		case STATE_CB_DATA_IN:
			psDev->eUsbTransactionState = eUsbMsDataIn();
			break;
	    default:
			break;
	}
#endif
	vUsbOtg_FIFO_INT_action(psDev->eUsbTransactionState, eWhichOtg);
}


///////////////////////////////////////////////////////////////////////////////
//		vUsb_F2_In()
//		Description: USB FIFO1 interrupt service process
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vOTG_F2_In(WHICH_OTG eWhichOtg)
{
    TRANSACTION_STATE eState = STATE_IDLE;
    PUSB_DEVICE_DESC psDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);

    //MP_DEBUG("-USBOTG%d- INT-Token", eWhichOtg);

#if USBOTG_DEVICE_SIDC
    DpsProcessJob(GetDpsState(eWhichOtg), eWhichOtg);
    if (psDevDesc->boIsEnableIntEp == TRUE)
    {
        //MP_DEBUG1("I:%d", psDevDesc->boIsEnableIntEp);
        //vUsbOtg_FIFO_INT_action(eUsbPtpSendEvent());

        eState = eUsbPtpSendEvent(eWhichOtg);

        if(eState != STATE_IDLE) // Nothing not to do action
            vUsbOtg_FIFO_INT_action(eState, eWhichOtg);
    }
#endif // USBOTG_DEVICE_SIDC    
}

#if 0
static void vUsb_Bulk_Out(DWORD count)
{
	vOTG_F1_Out(count);
}

static void vOTG_Bulk_In(void)
{
	vOTG_F0_In();
}

static void vOTG_INT_In(void)
{
     vOTG_F2_In();
}
#endif
///////////////////////////////////////////////////////////////////////////////
//		vOTGHandler()
//		Description:
//			1. Service all Usb events
//			2. ReEnable Usb interrupt.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
void vOtgHandler(DWORD usbIntGroupReg, WHICH_OTG eWhichOtg)   // USB-IF (Sync with MP620A and get Out/In token at the same time to handle)
{
    volatile BYTE  otg_Interrupt_level2_g0 = 0;
    volatile DWORD otg_Interrupt_level2_g1 = 0;
    volatile BYTE  otg_Interrupt_level2_g2 = 0;

    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;

    psDevDesc->bUsbInterruptLevel1 = usbIntGroupReg;

    if (psDevDesc->bUsbInterruptLevel1 == 0)
    {
        return;
    }

    if (psDevDesc->bUsbInterruptLevel1 & BIT0) //Group Byte 0
		otg_Interrupt_level2_g0 = mUsbOtgIntSrc0Rd() & ~mUsbOtgIntSrc0MaskRd();
    if (psDevDesc->bUsbInterruptLevel1 & BIT1) //Group Byte 1
		otg_Interrupt_level2_g1 = mUsbOtgIntSrc1Rd() & ~mUsbOtgIntSrc1MaskRd();   // USB-IF MSC Test (ErrorRecovery)
    if (psDevDesc->bUsbInterruptLevel1 & BIT2) //Group Byte 2
		otg_Interrupt_level2_g2 = mUsbOtgIntSrc2Rd() & ~mUsbOtgIntSrc2MaskRd();
	if (psDevDesc->bUsbInterruptLevel1 & BIT2)   //Group Source 2
	{
		if (otg_Interrupt_level2_g2 & BIT0)
		{
			MP_DEBUG("rst");
			vOTG_rst(eWhichOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT1)
		{
			MP_DEBUG("suspend");
			vOTG_suspend(eWhichOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT2)
		{
			MP_DEBUG("resume");
			vOTG_resm(psUsbOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT3)
		{
			mUsbOtgIntIsoSeqErrClr();
			vOTG_ISO_SeqErr(psUsbOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT4)
		{
			mUsbOtgIntIsoSeqAbortClr();
			vOTG_ISO_SeqAbort(psUsbOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT5)
		{
			mUsbOtgIntTX0ByteClr();
			vOTG_TX0Byte(psUsbOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT6)
		{
			mUsbOtgIntRX0ByteClr();
			vOTG_RX0Byte(psUsbOtg);
		}

		if (otg_Interrupt_level2_g2 & BIT7)
		{
			mUsbOtgIntDmaFinishClr();
			#if 0//(OTG_AP_Satus == Bulk_AP)
			if(bOTGDMARunning)
				vOTGCheckDMA();
			else
				printf("L%02x:DMA finish Interrupt error.",u8LineOTGCount++);
			#endif
		}

		if (otg_Interrupt_level2_g2 & BIT8)
		{
			mUsbOtgIntDmaErrClr();
			#if 0//(OTG_AP_Satus == Bulk_AP)
			if(bOTGDMARunning) {
				vOTGCheckDMA();
				printf("L%02x:DMA error.",u8LineOTGCount++);
			}
			else
				printf("L%02x:DMA error Interrupt error.",u8LineOTGCount++);
			#endif
		}

		if ( otg_Interrupt_level2_g2 & BIT9 )
		{
            ;//UartOutText("device idle");
		}

		if ( otg_Interrupt_level2_g2 & BIT10)
		{
            //UartOutText("*");
		}
	}

	if (psDevDesc->bUsbInterruptLevel1 & BIT0)    //Group Source 0
	{
		if (otg_Interrupt_level2_g0 & BIT5)
		{
			vOTG_ep0abort(eWhichOtg);
		}

		if (otg_Interrupt_level2_g0 & BIT2)
		{
			vOTG_ep0rx(eWhichOtg);
		}

		if (otg_Interrupt_level2_g0 & BIT0)
		{
			vOTG_ep0setup(eWhichOtg);
		}
		else if (otg_Interrupt_level2_g0 & BIT3)
		{
			vOTG_ep0end(eWhichOtg);
		}

		if (otg_Interrupt_level2_g0 & BIT1)
		{
			vOTG_ep0tx(eWhichOtg);
		}

		if (otg_Interrupt_level2_g0 & BIT4)
		{
			vOTG_ep0fail(psUsbOtg);
		}

		if (psDev->eUsbCxFinishAction == ACT_STALL)
		{
			//if(u8OTGMessageLevel & (MESS_ERROR | MESS_WARNING | MESS_INFO))
			//printf("L%x: Unsuported EP0 command...Return Cx Stall...\n", u8LineOTGCount ++);
			mUsbOtgEP0StallSet();
		}
		else if (psDev->eUsbCxFinishAction  == ACT_DONE)
		{
			mUsbOtgEP0DoneSet();
		}

		psDev->eUsbCxFinishAction  = ACT_IDLE;

		#if 0
		// YPING Debug
		if(psDevDesc->wTxRxCounter !=0) {
		}
		#endif
	}

	if (psDevDesc->bUsbInterruptLevel1 & BIT1)  //Group Source 1
	{
		// fifo0, 512 bytes, bulk for in   ep1
		// fifo1, 512 bytes, bulk for out ep2
		// fifo2, 512 bytes, int   for in   ep3
		//otg_Interrupt_level2_g1 = mUsbOtgIntSrc1Rd();   // need to verify PictBridge ???
		if (otg_Interrupt_level2_g1 & (BIT2 | BIT3))  // BIT3 for short packet and BIT2 for full packet
		{
			vOTG_F1_Out(mUsbOtgFIFOOutByteCount(FIFO_BULK_OUT), eWhichOtg);

			//otg_Interrupt_level2_g1 = mUsbOtgIntSrc1Rd();   // need to verify PictBridge ???

			if( otg_Interrupt_level2_g1 & BIT18)   //for PTP preTest Tool
			{
				//otg_Interrupt_level2_g1 &= ~BIT18;
				otg_Interrupt_level2_g1 &= ~(BIT18|BIT2|BIT3);
            	psDevDesc->boIsEnableIntEp = FALSE;
			}
		}
#if USBOTG_DEVICE_ISO_TEST
		if (otg_Interrupt_level2_g1 & (BIT4 | BIT5))  // BIT3 for short packet and BIT2 for full packet
		{
			vOTG_ISO_Out(eWhichOtg);
		}
#endif

		if (otg_Interrupt_level2_g1 & BIT16) // fifo0 is ready to be written
		{
#if USBOTG_DEVICE_ISO_TEST
			vOTG_ISO_In(eWhichOtg);
#endif
			if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE)
			{
#if (WEB_CAM_DEVICE==ENABLE)
				BYTE *dataBuf;
				DWORD size;
				if (mUsbOtgHighSpeedST())
					size = jpgDataGet(&dataBuf, 1536,0);
				else
					size = jpgDataGet(&dataBuf, 64);
				wFOTGPeri_Port(0x1b0) |= BIT12;
				bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN, dataBuf, size, eWhichOtg);
#endif
			}
			else
				vOTG_F0_In(eWhichOtg);
		}
#if (WEB_CAM_DEVICE==ENABLE)
		if (otg_Interrupt_level2_g1 & BIT19)   //fifo3 is ready to be writen for iso
		{
			if (Api_UsbdGetMode(eWhichOtg)==USB_AP_UAVC_MODE) {
				BYTE *dataBuf;
				DWORD size;
				size = audioDataGet(&dataBuf, 64);
				bOTGCxFxWrRd(FOTG200_DMA2FIFO3,DIRECTION_IN, dataBuf, size, eWhichOtg);
			}
		}
#endif

		if( otg_Interrupt_level2_g1 & BIT18)   //fifo2 is ready to be writen for interrupt
		{
	        vOTG_F2_In(eWhichOtg);
		}

		//if (otg_Interrupt_level2_g1 & ~(BIT16|BIT4|BIT5))
		if (otg_Interrupt_level2_g1 & ~(BIT4|BIT5)) //calvin test
		{
			//MP_DEBUG("unused int source 1 int");
		}
	}

	// Clear usb interrupt flags
	psDevDesc->bUsbInterruptLevel1 = 0;
	//fLib_EnableInt(IRQ_FOTG200);
}



#if 0
void UsbOTGDeviceInit()
{
	volatile DWORD	sts = 0;
    unsigned long reg;

	//psDevDesc->bUsbApMode = USB_AP_SIDC_MODE;

       //__asm("break 100");

#if 0
	switch(psDevDesc->bUsbApMode)
	{
		case USB_AP_VENDOR_MODE:
			break;
		case USB_AP_SIDC_MODE:
			//gpStiCommand = (PSTI_CONTAINER)((DWORD)(&gStiCommand));// | 0xa0000000);
			//gpStiResponse = (PSTI_CONTAINER)((DWORD)(&gStiResponse));// | 0xa0000000);
			//gpStiEvent = (PSTI_EVENT)((DWORD)(&gStiEvent));// | 0xa0000000);
			//memset((BYTE*)gpStiCommand, 0, OPERATION_REQUEST_LENGTH);
			//memset((BYTE*)gpStiResponse, 0, RESPONSE_LENGTH);
			//memset((BYTE*)gpStiEvent, 0, EVENT_LENGTH);
			break;
		case USB_AP_MSDC_MODE:
		default:
			//USBDeviceInfSetup();
			//gpCbw = (PCBW)((DWORD)(&gCbw));// | 0xa0000000);
			//gpCsw = (PCSW)((DWORD)(&gCsw));// | 0xa0000000);
			//memset((BYTE*)gpCbw, 0, CBW_LEN);
			//memset((BYTE*)gpCsw, 0, CSW_LEN);
                    UsbMsdcInit();
			break;
	}
#endif

	//UsbSetDescriptor();
	OtgSetDescriptor();

	//vSysInit();
	vOtg_APInit();    // Initialize Device API  parameter & value
    vFOTG200_Dev_Init();   //Initialize Device register

   mUsbOtgUnPLGClr();    //Set UNPLUG 1->0 to present the device had been pluged in
    IODelay(1);


    //__asm("break 100");
#if     0
	while(1) {
        /*
        reg = wFOTGPeri_Port(0x1DC);
		if (reg & BIT0)
			MP_DEBUG("crc5");
		if (reg & BIT1)
			MP_DEBUG("crc16");
		if (reg & BIT2)
			MP_DEBUG("seg");
		if (reg & BIT3)
			MP_DEBUG("timeout");
		if (reg & BIT4)
			MP_DEBUG("pid");
		if (reg & BIT5)
			MP_DEBUG("utmi");
		wFOTGPeri_Port(0x1DC) = 0;
*/
		gOTG_interrupt_level1_Save = mUsbOtgIntGroupRegRd();
		gOTG_interrupt_level1_Mask = mUsbOtgIntGroupMaskRd();
		gOTG_interrupt_level1      = gOTG_interrupt_level1_Save & ~gOTG_interrupt_level1_Mask;
#if 0
		if (wFOTGPeri_Port(0x1DC) & BIT1) {
			BYTE buf[1024];
			int index, cnt;
			wFOTGPeri_Port(0x1DC) &= ~BIT1;
			buf[0] = 0;
			cnt = mUsbOtgFIFOOutByteCount(FIFO2);
			MP_DEBUG("crc16 error");
			DpWord(cnt);
			vOTGFxRd(crcBuf, cnt);
			for (index = 0; index < cnt; ++index) {
				mp_sprintf(buf, "%s %x", buf, crcBuf[index]);
				if ((index % 16) == 15) {
					MP_DEBUG(buf);
					buf[0] = 0;
				}
			}
			MP_DEBUG(buf);
		}
#endif
		if (gOTG_interrupt_level1 != 0) {
                        vOtgHandler(gOTG_interrupt_level1);
                }

	}
	//<4>.Close the Peripheral function
	UsbOtgDeviceClose();
#endif
}
#endif  //0

static void UsbOtgDeviceSetMode(BYTE mode, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE  psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);

    psUsbDev->sDesc.bUsbApMode = mode;

    switch(psUsbDev->sDesc.bUsbApMode)
    {
#if USBOTG_DEVICE_SIDC
        case USB_AP_SIDC_MODE:
            UsbSidcInit(eWhichOtg);
            psUsbDev->sDesc.bUsbDefaultInterfaceValue = gStiInterfaceDescriptor[2];//psDevDesc->bUsbDefaultInterfaceValue = gStiInterfaceDescriptor[2];
            MP_DEBUG("-usbotg%d- API_Init SIDC MODE", eWhichOtg);
        break;
#endif // USBOTG_DEVICE_SIDC
        case USB_AP_VENDOR_MODE:
            psUsbDev->sDesc.bUsbDefaultInterfaceValue = gVendorInterfaceDescriptor[2];//psDevDesc->bUsbDefaultInterfaceValue = gVendorInterfaceDescriptor[2];
            MP_DEBUG("-usbotg%d- API_Init VENDOR MODE", eWhichOtg);
        break;

        case USB_AP_CDC_MODE:
            UsbCdcInit(eWhichOtg);
            psUsbDev->sDesc.bUsbDefaultInterfaceValue = gCdcInterfaceDescriptor1[2];//psDevDesc->bUsbDefaultInterfaceValue = gCdcInterfaceDescriptor1[2];
            psUsbDev->sDesc.bUsbDefaultInterfaceValue2 = gCdcInterfaceDescriptor2[2];//psDevDesc->bUsbDefaultInterfaceValue2 = gCdcInterfaceDescriptor2[2];
            MP_DEBUG("-usbotg%d- API_Init CDC MODE", eWhichOtg);
        break;

        #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
        case USB_AP_EXTERN_MODE:
            psUsbDev->eUsbTransactionState = STATE_EXTERN_OUT;
            psUsbDev->sDesc.bUsbDefaultInterfaceValue = gExternInterfaceDescriptor[2];//psDevDesc->bUsbDefaultInterfaceValue = gVendorInterfaceDescriptor[2];
            MP_DEBUG("-usbotg%d- API_Init VENDOR SIDE MONITOR", eWhichOtg);
        break;
        #endif

        case USB_AP_UAVC_MODE:
			#define VC_INF_ID 0
            psUsbDev->sDesc.bUsbDefaultInterfaceValue = VC_INF_ID;//gVendorInterfaceDescriptor[2];//psDevDesc->bUsbDefaultInterfaceValue = gVendorInterfaceDescriptor[2];
			#undef VC_INF_ID
			MP_DEBUG("-usbotg%d- API_Init UAC/UVC MODE", eWhichOtg);
        break;

        case USB_AP_MSDC_MODE:
        default:
            UsbMsdcInit(eWhichOtg);
            MP_DEBUG("-usbotg%d- API_Init MDSC_MODE", eWhichOtg);
            psUsbDev->sDesc.bUsbDefaultInterfaceValue = gMsdcInterfaceDescriptor[2];//psDevDesc->bUsbDefaultInterfaceValue = gMsdcInterfaceDescriptor[2];
        break;
    }
}

static void vOtg_APInit(WHICH_OTG eWhichOtg)
{
    WORD u16_i;
    PST_USB_OTG_DEVICE  psUsbDev;

    psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    psUsbDev->sDesc.wTxRxCounter = 0;//gTxRxCounter = 0;
    psUsbDev->eUsbCxCommand = CMD_VOID;//psDev->eUsbCxCommand = CMD_VOID;
    psUsbDev->sDesc.bUsbConfigValue = 0;//psDevDesc->bUsbConfigValue = 0;
    psUsbDev->sDesc.bUsbDefaultInterfaceValue = 0;//psDevDesc->bUsbDefaultInterfaceValue = 0;
    psUsbDev->sDesc.bUsbInterfaceValue = 0;//psDevDesc->bUsbInterfaceValue = 0;
    psUsbDev->sDesc.bUsbInterfaceAlternateSetting = 0;//psDevDesc->bUsbInterfaceAlternateSetting = 0;
    //gbUsbEP0HaltSt = FALSE;
    //gbUsbEP1HaltSt = FALSE;
    //gbUsbEP2HaltSt = FALSE;
    //gbUsbEP3HaltSt = FALSE;

    psUsbDev->eUsbCxFinishAction = ACT_IDLE;//geUsbCxFinishAction = ACT_IDLE;
    Api_UsbdSetCurrentStat(eWhichOtg, USB_DEV_STATE_POWERED);

    //pDesc->bUsbInterruptLevel1 = 0;

    //psDev->psControlCmd = (PSETUP_PACKET)((DWORD)(&gControlCmd) | 0xa0000000); // it's done in UsbOtgDeviceMemoryInit


    // Init : USB_MODE_NONE / Plug-in : USB_AP_MSDC_MODE when it's USB_MODE_NONE / Plug-out : USB_MODE_NONE
    // it cannot be clear and need to keep the value set by user or default. JL, 20091127
    //psUsbDev->sDesc.bUsbApMode = USB_MODE_NONE;
}

static void OtgSetDescriptor(WHICH_OTG eWhichOtg)
{
    BYTE *pdata;
    PUSB_DEVICE_DESC    psDevDesc;

    psDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
//////////////////////////////////////////////////////////////
// HS Configuration
//////////////////////////////////////////////////////////////
    pdata = psDevDesc->pbConfigHs;//Cofig_HS_Desc;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcConfigDescriptor[0], CONFIG_LENGTH);
    }
    else
    {
        memcpy(pdata, &gConfigDescriptor[0], CONFIG_LENGTH);
    }

    pdata += CONFIG_LENGTH;

    switch (psDevDesc->bUsbApMode)  // Interface Descriptor
    {
        case USB_AP_MSDC_MODE:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_SIDC_MODE:
            memcpy(pdata, &gStiInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_CDC_MODE:
            memcpy(pdata, &gCdcInterfaceDescriptor1[0], INTERFACE_LENGTH);
            break;
        case USB_AP_VENDOR_MODE:
            memcpy(pdata, &gVendorInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
        case USB_AP_EXTERN_MODE:
            memcpy(pdata, &gExternInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #endif
        default:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
    }
    pdata += INTERFACE_LENGTH;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcFunctionalDescriptor1[0], gCdcFunctionalDescriptor1[0]);
        pdata += gCdcFunctionalDescriptor1[0];
        memcpy(pdata, &gCdcFunctionalDescriptor2[0], gCdcFunctionalDescriptor2[0]);
        pdata += gCdcFunctionalDescriptor2[0];
        memcpy(pdata, &gCdcFunctionalDescriptor3[0], gCdcFunctionalDescriptor3[0]);
        pdata += gCdcFunctionalDescriptor3[0];
    }


    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gHsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }
    else
    {
        memcpy(pdata, &gHsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcInterfaceDescriptor2[0], INTERFACE_LENGTH);
        pdata += INTERFACE_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }


//////////////////////////////////////////////////////////////
// FS Configuration
//////////////////////////////////////////////////////////////
    pdata = psDevDesc->pbConfigFs;;//Cofig_FS_Desc;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcConfigDescriptor[0], CONFIG_LENGTH);
    }
    else
    {
        memcpy(pdata, &gConfigDescriptor[0], CONFIG_LENGTH);
    }
    pdata += CONFIG_LENGTH;

    switch (psDevDesc->bUsbApMode)  // Interface Descriptor
    {
        case USB_AP_MSDC_MODE:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_SIDC_MODE:
            memcpy(pdata, &gStiInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_CDC_MODE:
            memcpy(pdata, &gCdcInterfaceDescriptor1[0], INTERFACE_LENGTH);
            break;
        case USB_AP_VENDOR_MODE:
            memcpy(pdata, &gVendorInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
        case USB_AP_EXTERN_MODE:
            memcpy(pdata, &gExternInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #endif
        default:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
    }
    pdata += INTERFACE_LENGTH;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcFunctionalDescriptor1[0], gCdcFunctionalDescriptor1[0]);
        pdata += gCdcFunctionalDescriptor1[0];
        memcpy(pdata, &gCdcFunctionalDescriptor2[0], gCdcFunctionalDescriptor2[0]);
        pdata += gCdcFunctionalDescriptor2[0];
        memcpy(pdata, &gCdcFunctionalDescriptor3[0], gCdcFunctionalDescriptor3[0]);
        pdata += gCdcFunctionalDescriptor3[0];
    }


    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gFsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }
    else
    {
        memcpy(pdata, &gFsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcInterfaceDescriptor2[0], INTERFACE_LENGTH);
        pdata += INTERFACE_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }


//////////////////////////////////////////////////////////////
// HS Other Speed Configuration
//////////////////////////////////////////////////////////////
    pdata = psDevDesc->pbConfigHsOtherSpeedDesc;;//Cofig_HS_Other_Speed_Desc;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gOtherSpeedCdcConfigurationDescriptor[0], CONFIG_LENGTH);
    }
    else
    {
        memcpy(pdata, &gOtherSpeedConfigurationDescriptor[0], CONFIG_LENGTH);
    }
    pdata += CONFIG_LENGTH;

    switch (psDevDesc->bUsbApMode)  // Interface Descriptor
    {
        case USB_AP_MSDC_MODE:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_SIDC_MODE:
            memcpy(pdata, &gStiInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_CDC_MODE:
            memcpy(pdata, &gCdcInterfaceDescriptor1[0], INTERFACE_LENGTH);
            break;
        case USB_AP_VENDOR_MODE:
            memcpy(pdata, &gVendorInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
        case USB_AP_EXTERN_MODE:
            memcpy(pdata, &gExternInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #endif
        default:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
    }
    pdata += INTERFACE_LENGTH;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcFunctionalDescriptor1[0], gCdcFunctionalDescriptor1[0]);
        pdata += gCdcFunctionalDescriptor1[0];
        memcpy(pdata, &gCdcFunctionalDescriptor2[0], gCdcFunctionalDescriptor2[0]);
        pdata += gCdcFunctionalDescriptor2[0];
        memcpy(pdata, &gCdcFunctionalDescriptor3[0], gCdcFunctionalDescriptor3[0]);
        pdata += gCdcFunctionalDescriptor3[0];
    }

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gHsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }
    else
    {
        memcpy(pdata, &gHsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcInterfaceDescriptor2[0], INTERFACE_LENGTH);
        pdata += INTERFACE_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gHsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }

//////////////////////////////////////////////////////////////
// FS Other Speed Configuration
//////////////////////////////////////////////////////////////
    pdata = psDevDesc->pbConfigFsOtherSpeedDesc;//Cofig_FS_Other_Speed_Desc;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gOtherSpeedCdcConfigurationDescriptor[0], CONFIG_LENGTH);
    }
    else
    {
        memcpy(pdata, &gOtherSpeedConfigurationDescriptor[0], CONFIG_LENGTH);
    }
    pdata += CONFIG_LENGTH;

    switch (psDevDesc->bUsbApMode)  // Interface Descriptor
    {
        case USB_AP_MSDC_MODE:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_SIDC_MODE:
            memcpy(pdata, &gStiInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        case USB_AP_CDC_MODE:
            memcpy(pdata, &gCdcInterfaceDescriptor1[0], INTERFACE_LENGTH);
            break;
        case USB_AP_VENDOR_MODE:
            memcpy(pdata, &gVendorInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
        case USB_AP_EXTERN_MODE:
            memcpy(pdata, &gExternInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
        #endif
        default:
            memcpy(pdata, &gMsdcInterfaceDescriptor[0], INTERFACE_LENGTH);
            break;
    }
    pdata += INTERFACE_LENGTH;

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcFunctionalDescriptor1[0], gCdcFunctionalDescriptor1[0]);
        pdata += gCdcFunctionalDescriptor1[0];
        memcpy(pdata, &gCdcFunctionalDescriptor2[0], gCdcFunctionalDescriptor2[0]);
        pdata += gCdcFunctionalDescriptor2[0];
        memcpy(pdata, &gCdcFunctionalDescriptor3[0], gCdcFunctionalDescriptor3[0]);
        pdata += gCdcFunctionalDescriptor3[0];
    }

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gFsEndpointDescriptorEp3[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }
    else
    {
        memcpy(pdata, &gFsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp3[0], INTERFACE_LENGTH);
        pdata += EP_LENGTH;
    }

    if (psDevDesc->bUsbApMode == USB_AP_CDC_MODE)
    {
        memcpy(pdata, &gCdcInterfaceDescriptor2[0], INTERFACE_LENGTH);
        pdata += INTERFACE_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp1[0], EP_LENGTH);
        pdata += EP_LENGTH;
        memcpy(pdata, &gFsEndpointDescriptorEp2[0], EP_LENGTH);
        pdata += EP_LENGTH;
    }
}


void UsbOtgDeviceIsr(WHICH_OTG eWhichOtg)
{
    ST_MESSAGE_CONTENT   stMessage;
    SDWORD sdRetVal;
    PST_USB_OTG_DES psUsbOtg;
    BYTE bMessageId;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    bMessageId = UsbOtgDeviceMessageIdGet(eWhichOtg);
#if (WEB_CAM_DEVICE)
	if (wFOTGPeri_Port(0x148) & (BIT16|BIT19)) {
	    mUsbOtgGlobIntDis();
	    vOtgHandler(mUsbOtgIntGroupRegRd() & ~mUsbOtgIntGroupMaskRd(), eWhichOtg);
	    mUsbOtgGlobIntEnSet();
	} else {
#else
	{
#endif
	    mUsbOtgGlobIntDis();
	    stMessage.dwArgument1 = mUsbOtgIntGroupRegRd() & ~mUsbOtgIntGroupMaskRd();
	    sdRetVal = MessageDrop(bMessageId, (BYTE *)(&stMessage), sizeof(stMessage));
	}
}



SDWORD UsbOtgDeviceTaskInit(WHICH_OTG eWhichOtg)
{
    SDWORD sdRetVal = USBOTG_NO_ERROR;
    BYTE bTaskId;
    PST_USB_OTG_DES psUsbOtg;
    DWORD dwIntCause;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    MP_DEBUG("-usbotg%d- boIsAlreadyInit %x", eWhichOtg, psUsbOtg->sUsbDev.boIsAlreadyInit);

    if (psUsbOtg->sUsbDev.boIsAlreadyInit == TRUE)
    {
        MP_DEBUG("UsbOtgDeviceTaskInit for USBOTG%d:already done", eWhichOtg);
        return sdRetVal;
    }
    else
    {
        psUsbOtg->sUsbDev.boIsAlreadyInit = TRUE;
    }

    dwIntCause = UsbOtgIntCauseGet(eWhichOtg);
    MP_DEBUG("BEGIN UsbOtgDeviceTaskInit for USBOTG%d", eWhichOtg);
    SystemIntDis(dwIntCause);

    // create and start UsbOtgDeviceTask
    sdRetVal = MessageCreate(UsbOtgDeviceMessageIdGet(eWhichOtg), OS_ATTR_FIFO, 256);
    if( sdRetVal != 0)
    {
        MP_ALERT("--E-- %s UsbOtgDeviceTaskInit MessageCreate Id %d Fail %d", __FUNCTION__, UsbOtgDeviceMessageIdGet(eWhichOtg), sdRetVal);
        return USBOTG_MESSAGE_CREATE_ERROR;
    }

    bTaskId = UsbOtgDeviceTaskIdGet(eWhichOtg);

	sdRetVal = TaskCreate(bTaskId, UsbOtgDeviceTask, ISR_PRIORITY, 0x2000);

    if (sdRetVal != 0)
    {
        MP_ALERT("--E-- %s UsbOtgDeviceTaskInit USB%d_DEVICE_TASK create fail %d", __FUNCTION__, bTaskId, sdRetVal);
        return USBOTG_TASK_CREATE_ERROR;
    }

    sdRetVal = TaskStartup(bTaskId, eWhichOtg);
    if (sdRetVal != 0)
    {
        MP_ALERT("--E-- %s UsbOtgDeviceTaskInit USB%d_DEVICE_TASK startup fail %d", __FUNCTION__, bTaskId, sdRetVal);
        return USBOTG_TASK_STARTUP_ERROR;
    }

    SystemIntEna(dwIntCause);
    MP_DEBUG("END UsbOtgDeviceTaskInit for USBOTG%d END", eWhichOtg);
    return sdRetVal;
}

void UsbOtgDeviceTaskTerminate(WHICH_OTG eWhichOtg)
{
    BYTE bMessageId;
    BYTE bTaskId;

    bMessageId = UsbOtgDeviceMessageIdGet(eWhichOtg);
    bTaskId = UsbOtgDeviceTaskIdGet(eWhichOtg);
    MP_DEBUG("UsbOtgHostTaskTerminate");
    MessageDestroy(bMessageId);
    TaskTerminate(bTaskId);
}

static void UsbOtgDeviceTask(void)
{
    ST_MESSAGE_CONTENT stMessage;
    PST_USB_OTG_DES psUsbOtg;
    BYTE bMessageId;
    WHICH_OTG eWhichOtg;
    DWORD dwArgTemp[2];

    TaskStartupParamGet(TaskGetId(), &dwArgTemp[0],&dwArgTemp[1],&dwArgTemp[1],&dwArgTemp[1]);
    eWhichOtg = dwArgTemp[0];
    MP_DEBUG("UsbOtgDeviceTask for USBOTG%d", eWhichOtg);
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    bMessageId = UsbOtgDeviceMessageIdGet(eWhichOtg);
#if 1
	UsbOtgControllerInit(eWhichOtg);
    mwOTG20_Interrupt_OTG_Enable();

	vFOTG200_Dev_Init(eWhichOtg);
#else
    UsbOtgDeviceInit();
#endif

	while(1)
	{
		MessageReceive(bMessageId, (BYTE *)(&stMessage));
		vOtgHandler(stMessage.dwArgument1, eWhichOtg);       //  interrupt request handler
		TaskYield();
		mUsbOtgGlobIntEnSet();     //Enable  interrupt issue after processing the interrupt request
    }
}

USB_DEV_CLASS_MODE Api_UsbdGetMode(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_DESC psUsbDevDesc;

    psUsbDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    return psUsbDevDesc->bUsbApMode;//psDevDesc->bUsbApMode;
}

void Api_UsbdSetMode(USB_DEV_CLASS_MODE mode, WHICH_OTG eWhichOtg)
{
    vOtg_APInit(eWhichOtg);
    UsbOtgDeviceSetMode(mode, eWhichOtg);
}

#endif // 0


USB_DEV_CUR_STATE Api_UsbdGetCurrentStat(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);

    return psUsbDev->bCurrentStat;
}

void Api_UsbdSetCurrentStat(WHICH_OTG eWhichOtg, USB_DEV_CUR_STATE enUsbState)
{
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);

    if(enUsbState >= USB_DEV_STATE_MAX)
    {
        MP_ALERT("--E-- %s over", __FUNCTION__);
        return;
    }

    psUsbDev->bCurrentStat = enUsbState;
}

#endif //SC_USBDEVICE

