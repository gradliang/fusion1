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
* Filename      : mcard.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __MCARD_H
#define __MCARD_H

/*
// Include section 
*/
#include "devio.h"
/*
// Constant declarations
*/

///
///@defgroup CONSTANT Constants
///
#define TRIGGER_BY_INT				1

///
///@defgroup COMMON Common
///@ingroup CONSTANT
///@{

// define mcard command
typedef enum
{
	NULL_CARD_CMD		= 0,
	INIT_CARD_CMD,
	SECTOR_READ_CMD,
	SECTOR_WRITE_CMD,
	RAW_FORMAT_CMD,
	REMOVE_CARD_CMD,
#if (ISP_FUNC_ENABLE)
	ISP_READ_CMD,
	ISP_WRITE_CMD,
#endif
#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
	CODE_BLK_INVALID_CMD,
	CODE_BLK_ERASE_CMD,
	CODE_IDENTIFY_CMD,
#endif
//#if (USBOTG_WEB_CAM || USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
	WEB_CAM_START_CMD,
	WEB_CAM_STOP_CMD,
	WEB_CAM_GET_VFRAM_CMD,
	WEB_CAM_GET_AFRAM_CMD,
#endif
#if NAND_DUMPAP
	RAWPAGE_READ_CMD,
	RAWPAGE_WRITE_CMD,
#endif
} MCARD_DEVCE_CMD_SET;

#define FOR_USBH_PTP_SCAN_OBJECT_NOT_MCARD_CMD               0xFF

// Define MCARD command return value
/// memory command pass
#define MCARD_CMD_PASS          0
/// memory command fail
#define MCARD_CMD_FAIL         	-1
/// memory command invalid
#define MCARD_CMD_INVALID    	-2


/*
// Structure declarations
*/




/*
// shared functions
*/
void McardHwConfig(BYTE cardCtl);
void AtaInit(ST_MCARD_DEV *sDev);
void CfInit (ST_MCARD_DEV *dev);
void MsInit (ST_MCARD_DEV *dev);
void NandInit(ST_MCARD_DEV *sDev);
#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_NAND))
void NandIspInit(ST_MCARD_DEV *sDev);
#endif
void SdInit(ST_MCARD_DEV *sDev);
void SdProgChk();
void Sd2Init(ST_MCARD_DEV *sDev);
void xDInit (ST_MCARD_DEV *sDev);
void spi_dev_init(ST_MCARD_DEV* sDev);
#if (ISP_FUNC_ENABLE && (BOOTUP_TYPE == BOOTUP_TYPE_SPI))
void spi_isp_dev_init(ST_MCARD_DEV* sDev);
#endif
#if USB_WIFI_ENABLE
void UsbWifiInit(ST_MCARD_DEV* sDev);
#endif
#if DM9KS_ETHERNET_ENABLE
void CFEthernetInit(ST_MCARD_DEV * sDev);
#endif
void UsbEthernetInit(ST_MCARD_DEV * sDev);
#if  USBOTG_WEB_CAM
void UsbWebCam_Init(ST_MCARD_DEV * sDev);
#endif
BYTE DetermineSDSlotType(void);

#endif  //__MCARD_H

