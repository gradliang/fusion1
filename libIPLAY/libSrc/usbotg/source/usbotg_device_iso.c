///////////////////////////////////////////////////////////////////////////////
//
//	File name: OTG_ISO.c
//	Version: 1.0
//	Date: 2003/8/05
//
//	Author: Andrew
//	Email: yping@faraday.com.tw
//	Phone: (03) 578-7888
//	Company: Faraday Tech. Corp.
//
//	Description: USB ISOCHRONOUS Transfer test Rountine & relatived subroutine
//
///////////////////////////////////////////////////////////////////////////////
#define OTG_ISO_GLOBALS
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "ui.h"
#include "os.h"

#if SC_USBDEVICE

#if USBOTG_DEVICE_ISO_TEST

#define Bulk_AP					0
#define Interrupt_AP			1   //Important:this setting only for cross-connection, NOT for PC test-bench
#define IsochronousIN_AP		2		
#define IsochronousOUT_AP		3	

#define OTG_AP_Satus IsochronousIN_AP
#define FIFO0	0x0
#define FOTG200_DMA2FIFO0 			BIT0
#define DIRECTION_IN		0
#define DIRECTION_OUT		1

#if(OTG_AP_Satus == Bulk_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X10
	#define HS_iProduct             0X20
	#define HS_iSerialNumber        0X00
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0xE0//0XC0
		#define HS_C1_iMaxPower         0X00
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00

				#if (Bulk_Satus == Bulk_FIFO_SingleDir)
				#define HS_C1_I0_A0_EP_NUMBER          0X02
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define HS_C1_I0_A0_EP_NUMBER          0X04
				#endif
				
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_BULK
					#define HS_C1_I0_A0_EP1_MAX_PACKET 0x0200
					#define HS_C1_I0_A0_EP1_bInterval  00
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X02)
					//EP0X02
					#define HS_C1_I0_A0_EP2_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP2_BLKNO      DOUBLE_BLK
					#define HS_C1_I0_A0_EP2_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP2_TYPE       TF_TYPE_BULK
					#define HS_C1_I0_A0_EP2_MAX_PACKET 0x0200
					#define HS_C1_I0_A0_EP2_bInterval  00
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X03)
					//EP0X03
					#define HS_C1_I0_A0_EP3_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP3_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP3_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP3_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP3_MAX_PACKET 0x040
					#define HS_C1_I0_A0_EP3_bInterval  01
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X04)
					//EP0X04
					#define HS_C1_I0_A0_EP4_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP4_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP4_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP4_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP4_MAX_PACKET 0x040
					#define HS_C1_I0_A0_EP4_bInterval  01
				#endif
			#endif
		#endif
	#endif
	
#elif(OTG_AP_Satus == Interrupt_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X10
	#define HS_iProduct             0X20
	#define HS_iSerialNumber        0X00
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0X00
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00
				#define HS_C1_I0_A0_EP_NUMBER          0X02
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK1024BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP1_MAX_PACKET 0x0400
					#define HS_C1_I0_A0_EP1_bInterval  01
				#endif
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X02)
					//EP0X02
					#define HS_C1_I0_A0_EP2_BLKSIZE    BLK1024BYTE
					#define HS_C1_I0_A0_EP2_BLKNO      SINGLE_BLK
					#define HS_C1_I0_A0_EP2_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP2_TYPE       TF_TYPE_INTERRUPT
					#define HS_C1_I0_A0_EP2_MAX_PACKET 0x0400
					#define HS_C1_I0_A0_EP2_bInterval  01
				#endif
			#endif
		#endif
	#endif
	
#elif(OTG_AP_Satus == IsochronousIN_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X10
	#define HS_iProduct             0X20
	#define HS_iSerialNumber        0X00
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0X00
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00
				#define HS_C1_I0_A0_EP_NUMBER          0X01
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK512BYTE//BLK1024BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_ISOCHRONOUS
					#define HS_C1_I0_A0_EP1_MAX_PACKET 128//0x1400//0x400
					#define HS_C1_I0_A0_EP1_bInterval  01
				#endif
			#endif
		#endif
	#endif

#elif(OTG_AP_Satus == IsochronousOUT_AP)
	// device configuration:
	#define HS_bDeviceClass         0X00
	#define HS_bDeviceSubClass      0X00
	#define HS_bDeviceProtocol      0X00
	#define HS_iManufacturer        0X10
	#define HS_iProduct             0X20
	#define HS_iSerialNumber        0X00
	#define HS_CONFIGURATION_NUMBER 0X01
	
	#if (HS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define HS_C1_INTERFACE_NUMBER  0X01
		#define HS_C1                   0X01
		#define HS_C1_iConfiguration    0X30
		#define HS_C1_bmAttribute       0XC0
		#define HS_C1_iMaxPower         0X00
	
		#if (HS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define HS_C1_I0_ALT_NUMBER    0X01
			#if (HS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define HS_C1_I0_A0_bInterfaceNumber   0X00
				#define HS_C1_I0_A0_bAlternateSetting  0X00
				#define HS_C1_I0_A0_EP_NUMBER          0X01
				#define HS_C1_I0_A0_bInterfaceClass    0X00
				#define HS_C1_I0_A0_bInterfaceSubClass 0X00
				#define HS_C1_I0_A0_bInterfaceProtocol 0X00
				#define HS_C1_I0_A0_iInterface         0X40
	
				#if (HS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define HS_C1_I0_A0_EP1_BLKSIZE    BLK512BYTE
					#define HS_C1_I0_A0_EP1_BLKNO      TRIBLE_BLK
					#define HS_C1_I0_A0_EP1_DIRECTION  DIRECTION_OUT
					#define HS_C1_I0_A0_EP1_TYPE       TF_TYPE_ISOCHRONOUS
					#define HS_C1_I0_A0_EP1_MAX_PACKET 128//0x200
					#define HS_C1_I0_A0_EP1_bInterval  01
				#endif
			#endif
		#endif
	#endif

#endif

#if (OTG_AP_Satus == Bulk_AP)
	// device configuration:
	#define FS_bDeviceClass         0X00
	#define FS_bDeviceSubClass      0X00
	#define FS_bDeviceProtocol      0X00
	#define FS_iManufacturer        0X10
	#define FS_iProduct             0X20
	#define FS_iSerialNumber        0X00
	#define FS_CONFIGURATION_NUMBER 0X01
	
	#if (FS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define FS_C1_INTERFACE_NUMBER  0X01
		#define FS_C1                   0X01
		#define FS_C1_iConfiguration    0X30
		#define FS_C1_bmAttribute       0XC0
		#define FS_C1_iMaxPower         0X00
	
		#if (FS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define FS_C1_I0_ALT_NUMBER    0X01
			#if (FS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define FS_C1_I0_A0_bInterfaceNumber   0X00
				#define FS_C1_I0_A0_bAlternateSetting  0X00

				#if (Bulk_Satus == Bulk_FIFO_SingleDir)
				#define FS_C1_I0_A0_EP_NUMBER          0X02
				#elif(Bulk_Satus == Bulk_FIFO_BiDir)
				#define FS_C1_I0_A0_EP_NUMBER          0X04
				#endif
				
				#define FS_C1_I0_A0_bInterfaceClass    0X00
				#define FS_C1_I0_A0_bInterfaceSubClass 0X00
				#define FS_C1_I0_A0_bInterfaceProtocol 0X00
				#define FS_C1_I0_A0_iInterface         0X40
	
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define FS_C1_I0_A0_EP1_BLKSIZE    BLK64BYTE
					#define FS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define FS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define FS_C1_I0_A0_EP1_TYPE       TF_TYPE_BULK
					#define FS_C1_I0_A0_EP1_MAX_PACKET 0x0040
					#define FS_C1_I0_A0_EP1_bInterval  00
				#endif
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X02)
					//EP0X02
					#define FS_C1_I0_A0_EP2_BLKSIZE    BLK64BYTE
					#define FS_C1_I0_A0_EP2_BLKNO      DOUBLE_BLK
					#define FS_C1_I0_A0_EP2_DIRECTION  DIRECTION_OUT
					#define FS_C1_I0_A0_EP2_TYPE       TF_TYPE_BULK
					#define FS_C1_I0_A0_EP2_MAX_PACKET 0x0040
					#define FS_C1_I0_A0_EP2_bInterval  00
				#endif
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X03)
					//EP0X03
					#define FS_C1_I0_A0_EP3_BLKSIZE    BLK64BYTE
					#define FS_C1_I0_A0_EP3_BLKNO      SINGLE_BLK
					#define FS_C1_I0_A0_EP3_DIRECTION  DIRECTION_IN
					#define FS_C1_I0_A0_EP3_TYPE       TF_TYPE_INTERRUPT
					#define FS_C1_I0_A0_EP3_MAX_PACKET 0x0040
					#define FS_C1_I0_A0_EP3_bInterval  01
				#endif
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X04)
					//EP0X04
					#define FS_C1_I0_A0_EP4_BLKSIZE    BLK64BYTE
					#define FS_C1_I0_A0_EP4_BLKNO      SINGLE_BLK
					#define FS_C1_I0_A0_EP4_DIRECTION  DIRECTION_OUT
					#define FS_C1_I0_A0_EP4_TYPE       TF_TYPE_INTERRUPT
					#define FS_C1_I0_A0_EP4_MAX_PACKET 0x0040
					#define FS_C1_I0_A0_EP4_bInterval  01
				#endif			
			#endif
		#endif
	#endif
#elif (OTG_AP_Satus == Interrupt_AP)
	// device configuration:
	#define FS_bDeviceClass         0X00
	#define FS_bDeviceSubClass      0X00
	#define FS_bDeviceProtocol      0X00
	#define FS_iManufacturer        0X10
	#define FS_iProduct             0X20
	#define FS_iSerialNumber        0X00
	#define FS_CONFIGURATION_NUMBER 0X01
	
	#if (FS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define FS_C1_INTERFACE_NUMBER  0X01
		#define FS_C1                   0X01
		#define FS_C1_iConfiguration    0X30
		#define FS_C1_bmAttribute       0XC0
		#define FS_C1_iMaxPower         0X00
	
		#if (FS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define FS_C1_I0_ALT_NUMBER    0X01
			#if (FS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define FS_C1_I0_A0_bInterfaceNumber   0X00
				#define FS_C1_I0_A0_bAlternateSetting  0X00
				#define FS_C1_I0_A0_EP_NUMBER          0X02
				#define FS_C1_I0_A0_bInterfaceClass    0X00
				#define FS_C1_I0_A0_bInterfaceSubClass 0X00
				#define FS_C1_I0_A0_bInterfaceProtocol 0X00
				#define FS_C1_I0_A0_iInterface         0X40
	
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define FS_C1_I0_A0_EP1_BLKSIZE    BLK64BYTE
					#define FS_C1_I0_A0_EP1_BLKNO      SINGLE_BLK
					#define FS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define FS_C1_I0_A0_EP1_TYPE       TF_TYPE_INTERRUPT
					#define FS_C1_I0_A0_EP1_MAX_PACKET 0x0040
					#define FS_C1_I0_A0_EP1_bInterval  01
				#endif
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X02)
					//EP0X02
					#define FS_C1_I0_A0_EP2_BLKSIZE    BLK64BYTE
					#define FS_C1_I0_A0_EP2_BLKNO      SINGLE_BLK
					#define FS_C1_I0_A0_EP2_DIRECTION  DIRECTION_OUT
					#define FS_C1_I0_A0_EP2_TYPE       TF_TYPE_INTERRUPT
					#define FS_C1_I0_A0_EP2_MAX_PACKET 0x0040
					#define FS_C1_I0_A0_EP2_bInterval  01
				#endif
			#endif
		#endif
	#endif
#elif(OTG_AP_Satus == IsochronousIN_AP)
	// device configuration:
	#define FS_bDeviceClass         0X00
	#define FS_bDeviceSubClass      0X00
	#define FS_bDeviceProtocol      0X00
	#define FS_iManufacturer        0X10
	#define FS_iProduct             0X20
	#define FS_iSerialNumber        0X00
	#define FS_CONFIGURATION_NUMBER 0X01
	
	#if (FS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define FS_C1_INTERFACE_NUMBER  0X01
		#define FS_C1                   0X01
		#define FS_C1_iConfiguration    0X30
		#define FS_C1_bmAttribute       0XE0
		#define FS_C1_iMaxPower         0X00
	
		#if (FS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define FS_C1_I0_ALT_NUMBER    0X01
			#if (FS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define FS_C1_I0_A0_bInterfaceNumber   0X00
				#define FS_C1_I0_A0_bAlternateSetting  0X00
				#define FS_C1_I0_A0_EP_NUMBER          0X01
				#define FS_C1_I0_A0_bInterfaceClass    0X00
				#define FS_C1_I0_A0_bInterfaceSubClass 0X00
				#define FS_C1_I0_A0_bInterfaceProtocol 0X00
				#define FS_C1_I0_A0_iInterface         0X40
	
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define FS_C1_I0_A0_EP1_BLKSIZE    BLK1024BYTE
					#define FS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define FS_C1_I0_A0_EP1_DIRECTION  DIRECTION_IN
					#define FS_C1_I0_A0_EP1_TYPE       TF_TYPE_ISOCHRONOUS
					#define FS_C1_I0_A0_EP1_MAX_PACKET 0x03FC
					#define FS_C1_I0_A0_EP1_bInterval  01
				#endif
			#endif
		#endif
	#endif

#elif(OTG_AP_Satus == IsochronousOUT_AP)
	// device configuration:
	#define FS_bDeviceClass         0X00
	#define FS_bDeviceSubClass      0X00
	#define FS_bDeviceProtocol      0X00
	#define FS_iManufacturer        0X10
	#define FS_iProduct             0X20
	#define FS_iSerialNumber        0X00
	#define FS_CONFIGURATION_NUMBER 0X01
	
	#if (FS_CONFIGURATION_NUMBER >= 0X01)
		// Configuration 0X01
		#define FS_C1_INTERFACE_NUMBER  0X01
		#define FS_C1                   0X01
		#define FS_C1_iConfiguration    0X30
		#define FS_C1_bmAttribute       0XE0
		#define FS_C1_iMaxPower         0X00
	
		#if (FS_C1_INTERFACE_NUMBER >= 0x01)
			// Interface 0
			#define FS_C1_I0_ALT_NUMBER    0X01
			#if (FS_C1_I0_ALT_NUMBER >= 0X01) 
				// AlternateSetting 0X00
				#define FS_C1_I0_A0_bInterfaceNumber   0X00
				#define FS_C1_I0_A0_bAlternateSetting  0X00
				#define FS_C1_I0_A0_EP_NUMBER          0X01
				#define FS_C1_I0_A0_bInterfaceClass    0X00
				#define FS_C1_I0_A0_bInterfaceSubClass 0X00
				#define FS_C1_I0_A0_bInterfaceProtocol 0X00
				#define FS_C1_I0_A0_iInterface         0X40
	
				#if (FS_C1_I0_A0_EP_NUMBER >= 0X01)
					//EP0X01
					#define FS_C1_I0_A0_EP1_BLKSIZE    BLK1024BYTE
					#define FS_C1_I0_A0_EP1_BLKNO      DOUBLE_BLK
					#define FS_C1_I0_A0_EP1_DIRECTION  DIRECTION_OUT
					#define FS_C1_I0_A0_EP1_TYPE       TF_TYPE_ISOCHRONOUS
					#define FS_C1_I0_A0_EP1_MAX_PACKET 0x03FC
					#define FS_C1_I0_A0_EP1_bInterval  01
				#endif
			#endif
		#endif
	#endif
#endif

typedef unsigned char INT8U;
typedef unsigned int INT16U;
typedef unsigned long INT32U;

#define ISO_Wrap 	254//254

INT8U u8ISOOTGInit=FALSE;

BYTE *u32OTGISOArray;
BYTE *u32ISOOTGOutArray;
BYTE *u32ISOOTGInArray;

INT32U u32ISOOTGInTransferCount;
INT32U u32ISOOTGOutTransferCount;
INT32U u32ISOOTGInTest[4096];

BOOL bOTGHighSpeed;

INT8U u8ISOOTGOutCount ;

extern WORD gUsbApMode;

///////////////////////////////////////////////////////////////////////////////
//		vOTG_ISO_Initial() 
//		Description: ISO buffer initial
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
#define ISOBUFFSIZE (1024 * 64)
static unsigned char u32ISOOTGOutArray1[ISOBUFFSIZE];
static unsigned char u32ISOOTGInArray1[ISOBUFFSIZE];
static unsigned char u32OTGISOArray1[ISOBUFFSIZE];

void vOTG_ISO_Initial(BOOL speed)
{
	INT32U u32i, u32OTGISOArraySize,u32PacketSize;
	INT32U u32OTGISOArrayCount = 0;
mpDebugPrint("vOTG_ISO_Initial");

	bOTGHighSpeed = speed;
	if(u8ISOOTGInit == FALSE)
	{
	#if 0
		if(bOTGHighSpeed)
			u32PacketSize = HS_C1_I0_A0_EP1_MAX_PACKET & 0x7ff;
		else
			u32PacketSize = FS_C1_I0_A0_EP1_MAX_PACKET ;
	#else
		u32PacketSize = ((HS_C1_I0_A0_EP1_MAX_PACKET& 0x7ff)>FS_C1_I0_A0_EP1_MAX_PACKET)?
						(HS_C1_I0_A0_EP1_MAX_PACKET& 0x7ff): FS_C1_I0_A0_EP1_MAX_PACKET;
	#endif
			
		u32ISOOTGOutArray = (BYTE *)(((DWORD) &u32ISOOTGOutArray1[0]) | 0xa0000000);
		u32ISOOTGInArray  = (BYTE *)(((DWORD) &u32ISOOTGInArray1[0])  | 0xa0000000);
		u32OTGISOArray    = (BYTE *)(((DWORD) &u32OTGISOArray1[0])    | 0xa0000000);
#if 1
		for(u32i = 0; u32i < ISOBUFFSIZE; u32i++)
		{		
			*(u32ISOOTGInArray + u32i) = (INT8U)u32i;;
		}
#endif
		mpDebugPrint("== %x %x %x ==", u32ISOOTGOutArray1, u32ISOOTGInArray1, u32OTGISOArray1);
		mpDebugPrint("speed %s", bOTGHighSpeed ? "hs" : "fs");

		u8ISOOTGInit = TRUE;
	}

}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_ISO_In() 
//		Description: Use PIO/DMA send Isochronous data
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
DWORD dwCounter = 0;
DWORD dwSize = 1;
INT32U u32ISO_RX_Count;
//#define wFOTGPeri_Port(bOffset)		*((volatile DWORD *) ( USB_OTG_BASE | (bOffset)))
//#define mUsbOtgFIFODone(fifo_num)			(wFOTGPeri_Port(0x1b0+fifo_num*4) |= BIT11)

void vOTG_ISO_In(WHICH_OTG eWhichOtg)
{
	INT32U u32PacketSize = 0x400;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	INT32U i;

	if (u8ISOOTGInit==FALSE || u32ISO_RX_Count == 0)
		return;
	mpDebugPrint("vOTG_ISO_In %x %x", u32ISO_RX_Count, u32ISOOTGOutArray[0]);

	bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN,(BYTE*)((DWORD)u32ISOOTGOutArray),u32ISO_RX_Count, eWhichOtg);
	u32ISO_RX_Count=0;
}

///////////////////////////////////////////////////////////////////////////////
//		vOTG_ISO_Out() 
//		Description: Use PIO/DMA get Isochronous data
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
#define FOTG200_DMA2FIFO2 			BIT2
#define mUsbOtgFIFOOutByteCount(fifo_num)	(((wFOTGPeri_Port(0x1b0+fifo_num*4)&0x7ff)))
void vOTG_ISO_Out(WHICH_OTG eWhichOtg)
{
	//INT32U u32PacketSize ;
	PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	int i;

	for (i = 0; i < 1024; ++i)
		u32ISOOTGOutArray[i] = 0xaa;

	u32ISO_RX_Count = mUsbOtgFIFOOutByteCount(2);

	mpDebugPrint("vOTG_ISO_Out %x", u32ISO_RX_Count);

	bOTGCxFxWrRd(FOTG200_DMA2FIFO2,DIRECTION_OUT,(BYTE*)((DWORD)u32ISOOTGOutArray),u32ISO_RX_Count,eWhichOtg);
}

#endif // USBOTG_DEVICE_ISO_TEST
#endif // SC_USBDEVICE
