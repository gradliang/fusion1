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
* Filename		: usbotg_device_msdc.h
* Programmer(s)	: Cloud Wu
* Created Date	: 2008/08/19 
* Description:  MSDC (Mass Storage Device Class) 
*               
*               
******************************************************************************** 
*/

#ifndef __USBOTG_DEVICE_MSDC_H__
#define __USBOTG_DEVICE_MSDC_H__

#include "iplaysysconfig.h"
//#include "UtilRegFile.h"
#include "UtilTypeDef.h"
#include "usbotg_std.h"
//#include "usbotg_ctrl.h"
#include "usbotg_api.h"

//#if SC_USBDEVICE
#define BLK_DSCRPT_MAX          1       /* Block descriptor max number      */
#define PARA_PAGE_MAX           9       /* Parameter page max number        */
#define MSDC_MAX_LUN            MAX_LUN


/////////////////////////////////////////////////
//    Status byte
/////////////////////////////////////////////////
enum
{
    STS_GOOD            = 0x00,        // Good Status                      
    STS_CMD_FAILED      = 0x01,        // Command Failed                   
    STS_PHASE_ERR       = 0x02,        // Phase Error                      
    STS_STALL           = 0x03,        // Stall                      

    STS_SENDED          = 0xff,        // status data has sended           
    STS_USB_ERR         = 0xfe         // usb send/receive error           
};

#if 0
enum {
    CSW_COMMMAND_PASSED = 0x00 , 
    CSW_COMMMAND_FAILED = 0x01 ,
    CSW_PHASE_ERROR          = 0x02
}


typedef struct {                            // Little endian(31 bytes)
    volatile DWORD  u32Signature;           // 'USBC'
    volatile DWORD  u32Tag;                 // A Command Block Tag sent by the host.
    volatile DWORD  u32DataTransferLength;  // The number of bytes of data 
    volatile BYTE   u8Flags;                // Bit0~5 : Reserved-the host shall set these bits to zero.
                                            // Bit6   : Obsolete.The host shall set this bit to zero.
                                            // Bit7   : Direction - '0'= Data-Out from host to the device.
                                            //                      '1'= Data-In from the device to the host.
    volatile BYTE   u8LUN;                  // The device Logical Unit Number to which the command block is being send.
                                            // Bits 0-3: LUN, 4-7: Reserved
    volatile BYTE	u8CBLength;             // The valid length of the CBWCB in bytes(0x01 through 0x10)
                                            // Bits 0-4: CDB Length, 5-7: Reserved
    volatile BYTE	u8CB[16];               // Command status(byte16~30)
    BYTE            bReserved;
} CBW, *PCBW; 

typedef struct {                        // Little endian(13 bytes)
    volatile DWORD  u32Signature;       // 'USBS'
    volatile DWORD  u32Tag;             // The value received in the SBWTag of the associated CBW.
    volatile DWORD  u32DataResidue;     // Data-Out : Report the difference between the amount of data expected 
                                        // Data_In  : and the amount of data processed by the device.
                                        // Normal : 0x00000000
    volatile BYTE	u8Status;           // '0x00' - Command Passed("good status")
                                        // '0x01' - Command Failed
                                        // '0x02' - Phase Error
                                        // '0x03','0x04' - Reserved(Obsolete)
                                        // '0x05~0xff' - Reserved
    BYTE            bReserved[3];
} CSW, *PCSW; 
#endif
/*
typedef struct {                            // Little endian(31 bytes)
    volatile DWORD  u32Signature;           // 'USBC'
    volatile DWORD  u32Tag;                 // A Command Block Tag sent by the host.
    volatile DWORD  u32DataTransferLength;  // The number of bytes of data 
    volatile BYTE   u8Flags;                // Bit0~5 : Reserved-the host shall set these bits to zero.
                                            // Bit6   : Obsolete.The host shall set this bit to zero.
                                            // Bit7   : Direction - '0'= Data-Out from host to the device.
                                            //                      '1'= Data-In from the device to the host.
    volatile BYTE   u8LUN;                  // The device Logical Unit Number to which the command block is being send.
                                            // Bits 0-3: LUN, 4-7: Reserved
    volatile BYTE	u8CBLength;             // The valid length of the CBWCB in bytes(0x01 through 0x10)
                                            // Bits 0-4: CDB Length, 5-7: Reserved
    volatile BYTE	u8CB[16];               // Command status(byte16~30)
    BYTE            bReserved;
} CBW, *PCBW; 

typedef struct {                        // Little endian(13 bytes)
    volatile DWORD  u32Signature;       // 'USBS'
    volatile DWORD  u32Tag;             // The value received in the SBWTag of the associated CBW.
    volatile DWORD  u32DataResidue;     // Data-Out : Report the difference between the amount of data expected 
                                        // Data_In  : and the amount of data processed by the device.
                                        // Normal : 0x00000000
    volatile BYTE	u8Status;           // '0x00' - Command Passed("good status")
                                        // '0x01' - Command Failed
                                        // '0x02' - Phase Error
                                        // '0x03','0x04' - Reserved(Obsolete)
                                        // '0x05~0xff' - Reserved
    BYTE            bReserved[3];
} CSW, *PCSW; 
*/

typedef struct {
    BYTE   ModeParaLen;                // Mode parameter length     
    BYTE   MediaType;                  // Medium type               
    BYTE   DevPara;                    // Device specific parameter 
    BYTE   BlkDscrptLen;               // Block descriptor length   
} MODE_PARA_HDR6;

typedef struct {
    BYTE   ModeParaLen[2];              // Mode parameter length      
    BYTE   MediaType;                   // Medium type                
    BYTE   DevPara;                     // Device specific parameter  
    BYTE   Reserved[2];                 // Rserved area               
    BYTE   BlkDscrptLen[2];             // Block descriptor length         
} MODE_PARA_HDR10;
/*
// Block Descriptor/
typedef struct {
    BYTE   DnstyCode;                   // Density code     
    BYTE   BlkNum[3];                   // number of block  
    BYTE   Reserved;                    // Reserved area    
    BYTE   BlkLen[3];                   // length of block  
} BLK_DSCRPT;

// Parameter Page
typedef struct {
    BYTE   PageCode;                   // Code of the Page   
    BYTE   PageLen;                    // Length of the Page 
    BYTE   ModePara[30];
} PARAMETER_PAGE;
*/
/*-----------------------------------
    Read Capacity
-----------------------------------*/
/*
typedef struct {            // Little endian
    DWORD   LastLBA;        // Last logical block address
    DWORD   BlockLength;    // Block length in bytes(MSB first for command return )
    DWORD   BLenLSB;        // LSB first - block length
    BYTE    nClass;         // CFCard - '0'
                            // InternalFlash - '1'
    WORD    StsFlag;        // StsFlag = MediumReady '2'
                            // StsFlag = MediumError '1'
                            // StsFlag = MediumEmpty '4'
                            // StsFlag = MediumBusy  '8' ,opposite - MediaIdle
    BYTE    bReserved;
} MediaInfoStr;

typedef struct {                // Little endian
    BYTE    DeviceType;          // Peripheral device type(bit0~4)
    BYTE    RMB;                // Removable media - bit7='1'
    BYTE	Version;            // Bit0~2 : ANSI Version '0x00'
                                // Bit3~5 : ECMA Version '0x00' for UFI device
                                // Bit6~7 : ISO Version  '0x00' for UFI device
    BYTE    RespDataFormate;    // Bit0~3 : '0x01' for UFI device, (0x00 for HD device ?)
    BYTE    AdditionLen;        // '0x1F' for UFI device, (0x00 for HD device ? )
    BYTE    Reserved[3];        // Reserved
    BYTE    VendorInf[8];       // Vendor information(byte 8~15)
    BYTE    ProductId[16];      // Product identification(byte 16~31)
    BYTE    ProductRev[4];      // Product revision level(4bytes: n.nn)
} DeviceInformationStr;

typedef struct {
    BYTE    ErrorCode;
    BYTE    Reserved1;
    BYTE    SenseKey;
    BYTE    Information[4];
    BYTE    AddSenseLen;
    // eserved[4];
    BYTE    VendorSpec[4];
    BYTE    AddSenseCode;
    BYTE    AddSenseCodeQ;
    //  eserved41[4];
    BYTE   Fru;
    BYTE   SnsKeyInfo[3];
    BYTE   Reserved[2];

} RequestSenseStr;

typedef struct {
    BYTE    Reserved[3] ;
    BYTE    CapacityListLength ;
    DWORD   LastLBA;        // Last logical block address
    DWORD   BlockLength;	// Block length in bytes(MSB first for command return )
} CapacityDescriptor ;	     
*/

typedef struct {
    BYTE   DnstyCode;                   // Density code     
    BYTE   BlkNum[3];                   // number of block  
    BYTE   Reserved;                    // Reserved area    
    BYTE   BlkLen[3];                   // length of block  
} BLK_DSCRPT, *PBLK_DSCRPT;

// Parameter Page
typedef struct {
    BYTE   PageCode;                   // Code of the Page   
    BYTE   PageLen;                    // Length of the Page 
    BYTE   ModePara[30];
} PARAMETER_PAGE, *PPARAMETER_PAGE;

typedef struct {
    BYTE    Reserved[3] ;
    BYTE    CapacityListLength ;
    DWORD   NumOfBlock;        // Number of Blocks
    DWORD   BlockLength;	// Block length in bytes(MSB first for command return )
} CAPACITY_DESCRIPTOR, *PCAPACITY_DESCRIPTOR ;	     

typedef struct {            // Little endian
    DWORD   LastLBA;        // Last logical block address
    DWORD   BlockLength;    // Block length in bytes(MSB first for command return )
    DWORD   BLenLSB;        // LSB first - block length
    BYTE    nClass;         // CFCard - '0'
                            // InternalFlash - '1'
    WORD    StsFlag;        // StsFlag = MediumReady '2'
                            // StsFlag = MediumError '1'
                            // StsFlag = MediumEmpty '4'
                            // StsFlag = MediumBusy  '8' ,opposite - MediaIdle
    BYTE    bReserved;
} MEDIA_INFO_STR, *PMEDIA_INFO_STR;

typedef struct {                // Little endian
    BYTE    DeviceType;          // Peripheral device type(bit0~4)
    BYTE    RMB;                // Removable media - bit7='1'
    BYTE	Version;            // Bit0~2 : ANSI Version '0x00'
                                // Bit3~5 : ECMA Version '0x00' for UFI device
                                // Bit6~7 : ISO Version  '0x00' for UFI device
    BYTE    RespDataFormate;    // Bit0~3 : '0x01' for UFI device, (0x00 for HD device ?)
    BYTE    AdditionLen;        // '0x1F' for UFI device, (0x00 for HD device ? )
    BYTE    Reserved[3];        // Reserved
    BYTE    VendorInf[8];       // Vendor information(byte 8~15)
    BYTE    ProductId[16];      // Product identification(byte 16~31)
    BYTE    ProductRev[4];      // Product revision level(4bytes: n.nn)
} DEVICE_INFORMATION_STR, *PDEVICE_INFORMATION_STR;

typedef struct {
    BYTE    ErrorCode;
    BYTE    Reserved1;
    BYTE    SenseKey;
    BYTE    Information[4];
    BYTE    AddSenseLen;
    // eserved[4];
    BYTE    VendorSpec[4];
    BYTE    AddSenseCode;
    BYTE    AddSenseCodeQ;
    //  eserved41[4];
    BYTE   Fru;
    BYTE   SnsKeyInfo[3];
    BYTE   Reserved[2];

} REQUEST_SENSE_STR, *PREQUEST_SENSE_STR;



//global function
/*
TRANSACTION_STATE eOTGProcessCbw(WORD u16FIFOByteCount);
//TRANSACTION_STATE eOTGProcessCbw(void);
TRANSACTION_STATE eOTGMsDataOut(WORD u16FIFOByteCount);
TRANSACTION_STATE eOTGMsDataIn(void);
TRANSACTION_STATE eOTGMsSendCsw();
*/

/*
WORD ScisCmmandProcess_OTG( BYTE **hData,
                                                   BYTE *pCmd,
                                                   BYTE lun,
                                                   DWORD *pData_residue,
                                                   DWORD expect_tx_length);
*/

extern void UsbMsdcInit(WHICH_OTG eWhichOtg);
extern BOOL MsdcCheckIfReadWriteData(WHICH_OTG eWhichOtg);
extern TRANSACTION_STATE eUsbMsDataInStall(WHICH_OTG eWhichOtg);
extern TRANSACTION_STATE eUsbMsDataOutStall(WHICH_OTG eWhichOtg);
extern TRANSACTION_STATE eUsbMsSendCsw(WHICH_OTG eWhichOtg);
extern TRANSACTION_STATE eUsbProcessCbw(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);
extern TRANSACTION_STATE eUsbMsDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);
extern TRANSACTION_STATE eUsbMsDataIn(WHICH_OTG eWhichOtg);
extern void USBCardStsByUI(BYTE bDriveId, BYTE bCardIn, WHICH_OTG eWhichOtg);


//#endif    //SC_USBDEVICE

#endif  

