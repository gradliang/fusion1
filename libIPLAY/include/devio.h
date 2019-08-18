#ifndef __DEVIO_H
#define __DEVIO_H

/*
// Include section
*/

#include "iplaysysconfig.h"
#include "utiltypedef.h"

/*
// Constant declarations
*/
///
///@addtogroup COMMON
///
///@{

typedef enum
{
    DEV_NULL = 0,    ///< For Null Device
    // OTG-0
    DEV_USB_HOST_ID1,       ///< For USB_HOST_ID1
    DEV_USB_HOST_ID2,       ///< For USB_HOST_ID2
    DEV_USB_HOST_ID3,       ///< For USB_HOST_ID3
    DEV_USB_HOST_ID4,       ///< For USB_HOST_ID4
    DEV_USB_HOST_PTP,       ///< For USB_PTP
    // OTG-1
    DEV_USBOTG1_HOST_ID1,   ///< For USBOTG1_HOST_ID1
    DEV_USBOTG1_HOST_ID2,   ///< For USBOTG1_HOST_ID2
    DEV_USBOTG1_HOST_ID3,   ///< For USBOTG1_HOST_ID3
    DEV_USBOTG1_HOST_ID4,   ///< For USBOTG1_HOST_ID4
    DEV_USBOTG1_HOST_PTP,   ///< For USBOTG1_PTP
    DEV_NAND_ISP,           ///< For NAND Flash ISP
    DEV_SPI_FLASH_ISP,      ///< For SPI Flash ISP
    DEV_NAND,               ///< For NAND Flash
    DEV_SPI_FLASH,          ///< For SPI Flash
    DEV_SD_MMC,             ///< For SD card 1
    DEV_MS,                 ///< For Memory Stick card
    DEV_SD2,                ///< For SD card 2
    DEV_CF,                 ///< For CF card
    DEV_XD,                 ///< For XD card
    DEV_HD,                 ///< For HD Drive
    DEV_SM,                 ///< For SM card
    DEV_USB_WIFI_DEVICE,    ///< For USB_WiFi
    DEV_CF_ETHERNET_DEVICE, ///< For CF_ETHERNET
    DEV_USB_ETHERNET_DEVICE,///< For USB_ETHERNET
    DEV_USB_WEBCAM,         ///< For USB_WEBCAM
    MAX_DEVICE_DRV          ///< Maximum number of device drives
} E_DEVICE_ID;


/*
// Structure declarations
*/
struct ST_MCARD_MAIL_TAG
{
    DWORD   dwBlockAddr;
    DWORD   dwBlockCount;
    DWORD   dwBuffer;
    DWORD   dwObjectHandle;
    DWORD   dwProgress;		// for asynchronized R/W
    E_DEVICE_ID    wMCardId;
    WORD    wCmd;
    WORD    wStateMachine;
    WORD    wCurrentExecutionState;
    SWORD   swStatus;
};
typedef struct ST_MCARD_MAIL_TAG ST_MCARD_MAIL;

struct ST_MCARD_DEV_TAG
{
                                    // Display related attributes
    BYTE          *pbDescriptor;    //   used to describe device itself
    E_DEVICE_ID   wMcardType;       //   card type
    WORD          wRenewCounter;    //   each time the deivce plug_in, this
                                    //   counter will increament by 1
    BYTE          *pbIcon;          //   point to the icon buffer
    ST_MCARD_MAIL *sMcardRMail;
                                    // Property related attributes
    DWORD         dwCapacity;       //   number of sector
    WORD          wSectorSize;      //   bytes per sector
    WORD          wSectorSizeExp;   //   exponential of sector size
    WORD          wProperty1;       //   usage depend on each device    !! Use this flag as card's subtype  C.W 100729 !!
    WORD          wProperty2;       //   usage depend on each device
                                    // Status releated attributes
    SWORD         swStatus;
    DWORD         dwAccumulation;

    struct
    {
        unsigned  Present        :      1;
        unsigned  Detected       :      1;
        unsigned  Installed      :      1;
        unsigned  ReadOnly       :      1;
        unsigned  PipeEnable     :      1;
        unsigned  Accumulatable  :      1;
    } Flag;

    void (*CommandProcess) (void *);
};
typedef struct ST_MCARD_DEV_TAG ST_MCARD_DEV;


struct ST_MCARD_DEVS_TAG
{
    struct ST_MCARD_DEV_TAG sMDevice[MAX_DEVICE_DRV];
    DWORD dwDevInsCnt;
    E_DEVICE_ID dwDevLunCurSetting[MAX_LUN + 1];
    BYTE bIdOffsetValue;
};
typedef struct ST_MCARD_DEVS_TAG ST_MCARD_DEVS;

#define STD_MCARD_SUBSYSTEM_MODE  0x00
#define DIS_CARD_DETECTION        0x01
#define DIS_WRITE_PROTECTION      0x02

#endif //__DEVIO_H

