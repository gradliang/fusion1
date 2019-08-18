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
* Filename		: usbotg_host.h
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)
* Created Date	: 2007/07/26 
* Description:  1.EHCI Data Structure
*               2.EHCI Register
*               3.Others
******************************************************************************** 
*/
#ifndef __USBOTG_HOST_H__
#define __USBOTG_HOST_H__ 
#include "UtilTypeDef.h"
#include "UtilRegFile.h"
#include "devio.h"
#include "iplaysysconfig.h"
#include "global612.h"
#include "fs.h"
#include "usbotg_std.h"
#include "usbotg_device_sidc.h"  // ???
#include "..\..\mcard\include\mcard.h"
#if USBOTG_HOST_CDC
#include "usbotg_host_cdc.h"
#endif
#if USBOTG_HOST_HID
#include "usbotg_host_hid.h"
#endif
#include "usbotg_api.h"


#define USB_HOST_ISO_TEST DISABLE
//#if (SC_USBHOST==ENABLE)
//=================== 1.Condition Definition  ============================================================
//========================================================================================================
#define Host20_qHD_SIZE	                        0x40    //(64bytes)
#define Host20_qHD_MAX	                        20      //(64x20)   => 1280 bytes

#define Host20_qTD_SIZE	                        0x40    //(32bytes)
#define Host20_qTD_MAX	                        200      //(32x200)   => 6400 bytes 

#define Host20_Preiodic_Frame_SIZE	            0x04    //(4bytes)
#define Host20_Preiodic_Frame_List_MAX	        1024    //(4x1024)  => 4096 bytes  

#define Host20_iTD_SIZE	                        0x40    //(64bytes)
#define Host20_Page_SIZE	                    0x1000  //(4Kbytes)
#if (USBOTG_HOST_ISOC == ENABLE)
#define Host20_iTD_MAX	                        1024    //(64x1024) => 65536 bytes
#define Host20_Page_MAX	                        200 // 12*16+8 => ISOC_DATA_NUM_BUF * OTGH_ISO_VIDEO_DATA_BUFFER_NUM + 8 
#else
#define Host20_iTD_MAX	                        600    //(64x1024) => 65536 bytes
#define Host20_Page_MAX	                        20 // 12*16+8 => ISOC_DATA_NUM_BUF * OTGH_ISO_VIDEO_DATA_BUFFER_NUM + 8 
#endif
//#define Host20_Page_MAX	                        40//(20)//(40)      //(4096x20) => 81920 bytes
//#define Host20_POOL_BUFF_SIZE                   0x1000
//#define Host20_Total_Memory ((Host20_qHD_SIZE*Host20_qHD_MAX)+(Host20_qTD_SIZE*Host20_qTD_MAX)+(Host20_Preiodic_Frame_SIZE*Host20_Preiodic_Frame_List_MAX)+(Host20_iTD_SIZE*Host20_iTD_MAX)+(Host20_Page_SIZE*Host20_Page_MAX))

#if (MEM_2M||USBOTG_HOST_ISOC)
#define Host20_SIDC_BULK_RX_BUFFER_SIZE				0x0000800c // 32KB
#else
#define Host20_SIDC_BULK_RX_BUFFER_SIZE				0x0002000c // 128KB
#endif

#define Host20_MEM_TYPE_qTD               	    0x00
#define Host20_MEM_TYPE_iTD               	    0x01
#define Host20_MEM_TYPE_4K_BUFFER         	    0x02
#define Host20_MEM_TYPE_siTD               	    0x03

#define Host20_MEM_FREE         	            0x01
#define Host20_MEM_USED         	            0x02

#define OTGH_DIR_IN 	                     0x01
#define OTGH_DIR_OUT 	                     0x00	 
#define OTGH_NULL			                 0x00	
#define OTGH_ED_ISO 	                     0x01
#define OTGH_ED_BULK 	                     0x02	 
#define OTGH_ED_INT 	                     0x03	 
#define OTGH_ED_Control	                 0x00		    
#define OTGH_FARADAY_TEST_AP                0x10237856
#define OTGH_SRP_HNP_Enable                 0x03
#define OTGH_Remote_Wake_UP                 0x00000400	 
#define OTGH_Remote_Wake_UP_INT             0x00000008	


#define MAX_NUM_OF_CONFIG       4
#define MAX_NUM_OF_INTERFACE    4
#define MAX_NUM_OF_ENDPOINT     12

#define MAX_NUM_OF_STORAGE_ID   2    // for SIDC
#define MAX_NUM_OF_OBJECT_HANDLES	        500
#define MAX_NUM_OF_FOLDER_OBJECT_HANDLES	100

#define EVENT_MSDC_TRANSACTION_PASSED      BIT1
#define EVENT_MSDC_TRANSACTION_FAILED      BIT2
#define EVENT_MSDC_DEVICE_PLUG_OUT              BIT3
//#define EVENT_READ_DATA_FOR_FILE_SYS_PASSED        BIT1
//#define EVENT_READ_DATA_FOR_FILE_SYS_FAILED      BIT2
//#define EVENT_WRITE_DATA_FOR_FILE_SYS_PASSED       BIT1
//#define EVENT_WRITE_DATA_FOR_FILE_SYS_FAILED     BIT2

#define EVENT_DEVICE_PLUG_IN        BIT1
#define EVENT_DEVICE_PLUG_OUT       BIT2
#define EVENT_EHCI_IOC              BIT3
#define EVENT_POLLING_EACH_LUN      BIT4
#define EVENT_EHCI_ACTIVE_SETUP     BIT5
#define EVENT_EHCI_ACTIVE_BULK      BIT6
#define EVENT_EHCI_ACTIVE_INTERRUPT BIT7
#define EVENT_EHCI_ACTIVE_ISO_IN		BIT8
#define EVENT_EHCI_ACTIVE_ISO_OUT		BIT9
#define EVENT_SEND_ISOC_IN_DATA_TO		BIT10

#define HOST20_Enable                                      0x01
#define HOST20_Disable                                     0x00
#define USBOTG_TRANSACTION_TIME_OUT_CNT 7000 //ms

#define MAX_NUM_OF_CONFIG       4
#define MAX_NUM_OF_INTERFACE    4
#define MAX_NUM_OF_ENDPOINT     12

#define BULK_ONLY_CBW_CDB_SIZE  16
#define MAX_NUM_OF_STORAGE_ID   2    // for SIDC
#define MAX_NUM_OF_OBJECT_HANDLES	        500
#define MAX_NUM_OF_FOLDER_OBJECT_HANDLES	100

#define USB_STI_CONTAINER_SIZE		12    //not counting paramteter size


// USB-IF test mode (Port test control)
#define TEST_NOT_ENABLE     0x00
#define TEST_J_STATE        0x01
#define TEST_K_STATE        0x02
#define TEST_SE0_NAK_STATE  0x03
#define TEST_PACKET_STATE   0x04
#define TEST_FORCE_ENABLE   0x05

//-------------------
// VID/PID
//-------------------
#define HSUPA_USB_DEVICE_VID    0x1DA5  // Qisda 3.5G modem - HSUPA USB Device
#define HSUPA_USB_MSDC_PID      0xF000  // Qisda Mass Storage - H21
#define HSUPA_USB_CDC_PID       0x4522  // Qisda 3.5G modem - H21

#define HUAWEI_HSPA_USB_DEVICE_VID    0x12D1  // Huawei 3G HSPA modem - HSPA USB Device
#define HUAWEI_HSPA_USB_MSDC_PID      0x1446  // Huawei 3G HSPA Mass Storage
#define HUAWEI_HSPA_USB_CDC_PID       0x1436  // Huawei 3G HSPA modem

//-------------------
// Datang LC-6311 TD-SCDMA USB modem VID/PID
//-------------------
#define TDSCDMA_USB_DEVICE_VID	0x1AB7	// Datang 3G modem - TD-SCDMA USB Device
#define TDSCDMA_USB_PID			0x6000	// Datang 3G modem - LC-6311

#define DM_USB_DEVICE_VID		0x0A46	// Davicom USB Device
#define DM9621_USB_PID			0x9621	// Davicom USB 9621 Ethernet
#define DM9621_USB_OTHER_PID	0x9620	// Davicom USB 9621 Ethernet


#define USBH_MJPEG_DROP_FRAME_ISSUE 0
#if USBH_MJPEG_DROP_FRAME_ISSUE
#define USBOTG_HOST_IOC_POLIING_SWITCH 1 // 1:Polling; 0:ISR
#else
#define USBOTG_HOST_IOC_POLIING_SWITCH 0 // 1:Polling; 0:ISR
#endif

/////////////////////////// ISO ////////////////////////////////
#define OTGH_MAX_PACKET_SIZE			0x400
#define HOST20_iTD_Status_Active		0x08
#define OTGH_Dir_IN				0x01
#define OTGH_Dir_Out				0x00

#define FRAME_LIST_SIZE_1024    0 // 1024 elements, 4096 bytes
#define FRAME_LIST_SIZE_512     1 // 512 elements, 2048 bytes
#define FRAME_LIST_SIZE_256     2 // 256 elements, 1024 bytes

//=================== 2.Define Register Macro ================================================================
//========================================================================================================

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define bcd2hex(x)              (((x) >> 4) & 0x0f) * 10 + ((x) & 0x0f)
#define hex2bcd(x)              ((((x) / 10) << 4) | ((x) % 10))
#ifndef __FILE_H
#define hi_byte_of_word(x)      (unsigned char) (((unsigned short)(x)) >> 8)
#define lo_byte_of_word(x)      (unsigned char) ((unsigned short)(x))
#define byte_swap_of_word(x)    (unsigned short)(((unsigned short)(x) >> 8) | ((unsigned short)(x) << 8))
#define hi_byte_of_dword(x)     (unsigned char) ((unsigned long)(x) >> 24)
#define midhi_byte_of_dword(x)  (unsigned char) ((unsigned long)(x) >> 16)
#define midlo_byte_of_dword(x)  (unsigned char) ((unsigned long)(x) >> 8)
#define lo_byte_of_dword(x)     (unsigned char) ((unsigned long)(x))
#define byte_swap_of_dword(x)   (((unsigned long)(x) << 24) | (((unsigned long)(x) & 0x0000ff00) << 8) |\
                                (((unsigned long)(x) & 0x00ff0000) >> 8) | ((unsigned long)(x) >> 24))
#endif
#define uchar_to_ulong(x,y,u,v) (unsigned long) (((unsigned long)x)<<24)|(((unsigned long)y)<<16)|\
                                (((unsigned long)u)<<8)|((unsigned long)(v))
#define uchar_to_ushort(x,y)    (unsigned short)  ((((unsigned short)x)<<8)|((unsigned short)y))

//#define mwHost20Port(bOffset)			*((volatile DWORD *) ( USB_OTG_BASE + bOffset))
//#define mwHost20BitRd(bOffset,wBitNum)	((mwHost20Port(bOffset)) & wBitNum)

#define mwHost20Port(bOffset)			*((volatile DWORD *) ( (DWORD)(psUsbOtg->psUsbReg) + bOffset))
#define mwHost20BitRd(bOffset,wBitNum)	((mwHost20Port(bOffset)) & wBitNum)


#define mwHost20Bit_Rd(dwRegVal,wBitNum)                     (dwRegVal&wBitNum)
#define mwHost20Bit_Set(dwRegVal,wBitNum)                    (dwRegVal|=wBitNum)
#define mwHost20Bit_Clr(dwRegVal,wBitNum)                    (dwRegVal&=~wBitNum)


#define mwHost20_PORTSC_PortReset_Set()		              mwHost20Bit_Set   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT8)//(gp_UsbOtg->HcPortStatusAndControl, BIT8) 	     	
#define mwHost20_PORTSC_PortReset_Clr()		              mwHost20Bit_Clr   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT8)//(gp_UsbOtg->HcPortStatusAndControl, BIT8) 	     	
#define mwHost20_PORTSC_ConnectStatus_Rd()		          mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT0)//(gp_UsbOtg->HcPortStatusAndControl, BIT0) 	
#define mwHost20_PORTSC_ForceSuspend_Set()		          mwHost20Bit_Set   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT7)//(gp_UsbOtg->HcPortStatusAndControl, BIT7) 	     	
#define mwHost20_PORTSC_ForceResume_Set()		          mwHost20Bit_Set   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT6)//(gp_UsbOtg->HcPortStatusAndControl, BIT6) 	     	
#define mwHost20_PORTSC_EnableDisable_Set()		          mwHost20Bit_Set   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT2)//(gp_UsbOtg->HcPortStatusAndControl, BIT2) 	     	

#define mwHost20_FrameIndex_Rd()		                  (psUsbOtg->psUsbReg->HcFrameIndex&0x00001FFF)//(gp_UsbOtg->HcFrameIndex&0x00001FFF) 	//Only Read Bit0~Bit12(Skip Bit 13)     	
#define mwOTG20_Control_HOST_SPD_TYP_Rd()		          ((psUsbOtg->psUsbReg->OtgControlStatus>>22)&0x00000003)//((gp_UsbOtg->OtgControlStatus>>22)&0x00000003) 

#define mdwOTGC_PHY_Rd()                	        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT15)//(gp_UsbOtg->OtgControlStatus &    BIT15)	
#define mdwOTGC_PHY_Set()               	        (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT15)//(gp_UsbOtg->OtgControlStatus |=   BIT15)     
#define mdwOTGC_PHY_Clr()               	        (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT15))//(gp_UsbOtg->OtgControlStatus &= (~BIT15))  

#define mbHost20_USBCMD_ParkMode_CNT_Set(bValue)     	  (psUsbOtg->psUsbReg->HcUsbCommand=(psUsbOtg->psUsbReg->HcUsbCommand&0xFFFFFCFF)|(( (DWORD) bValue )<<8)  )//(gp_UsbOtg->HcUsbCommand=(gp_UsbOtg->HcUsbCommand&0xFFFFFCFF)|(( (DWORD) bValue )<<8)  )	//Bit 8~9
#define mwHost20_Misc_ASYN_SCH_SLPT_Set(bValue)		      (psUsbOtg->psUsbReg->HcMisc=((psUsbOtg->psUsbReg->HcMisc&0xFFFFFFFC)|((DWORD)(bValue))))//(gp_UsbOtg->HcMisc=((gp_UsbOtg->HcMisc&0xFFFFFFFC)|((DWORD)(bValue))))	    //Bit 0~1
#define mwHost20_Misc_EOF1Time_Set(bValue)		          (psUsbOtg->psUsbReg->HcMisc=((psUsbOtg->psUsbReg->HcMisc&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))//(gp_UsbOtg->HcMisc=((gp_UsbOtg->HcMisc&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))	//Bit 2~3
#define mwHost20_Misc_EOF2Time_Set(bValue)		          (psUsbOtg->psUsbReg->HcMisc=((psUsbOtg->psUsbReg->HcMisc&0xFFFFFFCF)|(((DWORD)(bValue))<<4)))//(gp_UsbOtg->HcMisc=((gp_UsbOtg->HcMisc&0xFFFFFFCF)|(((DWORD)(bValue))<<4)))	//Bit 4~5
#define mwHost20_CurrentAsynchronousAddr_Rd()		      (psUsbOtg->psUsbReg->HcCurrentAsynListAddress)//(gp_UsbOtg->HcCurrentAsynListAddress)	     	
#define mwHost20_CurrentAsynchronousAddr_Set(wValue)	  (psUsbOtg->psUsbReg->HcCurrentAsynListAddress=wValue)//(gp_UsbOtg->HcCurrentAsynListAddress=wValue) 	

#define mwHost20_PORTSC_HC_TST_PKDONE()		              mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT20) //(gp_UsbOtg->HcPortStatusAndControl, BIT20) 	     	
#define mbHost20_PORTSC_PortTest_Set(bValue)             (psUsbOtg->psUsbReg->HcPortStatusAndControl=((psUsbOtg->psUsbReg->HcPortStatusAndControl&0xFFF0FFFF)|(((DWORD)(bValue))<<16)))//(gp_UsbOtg->HcPortStatusAndControl=((gp_UsbOtg->HcPortStatusAndControl&0xFFF0FFFF)|(((DWORD)(bValue))<<16)))	//Bit 16~19
#define mwHost20_PORTSC_LineStatus_Rd()                  ((psUsbOtg->psUsbReg->HcPortStatusAndControl>>10)&0x00000003)//((gp_UsbOtg->HcPortStatusAndControl>>10)&0x00000003) 	     	

//<8>.0x024(PERIODICLISTBASE - Periodic Frame List Base Address Register (Address = 024h))
#define mwHost20_PeriodicBaseAddr_Rd()		              (psUsbOtg->psUsbReg->HcPeriodicFrameListBaseAddress)//(gp_UsbOtg->HcPeriodicFrameListBaseAddress) 	     	
#define mwHost20_PeriodicBaseAddr_Set(wValue)		      (psUsbOtg->psUsbReg->HcPeriodicFrameListBaseAddress=wValue)//(gp_UsbOtg->HcPeriodicFrameListBaseAddress=wValue) 	

#define mbHost20_USBCMD_FrameListSize_Rd()	              ((psUsbOtg->psUsbReg->HcUsbCommand>>2)&0x00000003)//((gp_UsbOtg->HcUsbCommand>>2)&0x00000003)	   //Bit 2~3
#define mbHost20_USBCMD_FrameListSize_Set(bValue)     	  (psUsbOtg->psUsbReg->HcUsbCommand=((psUsbOtg->psUsbReg->HcUsbCommand&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))//(gp_UsbOtg->HcUsbCommand=((gp_UsbOtg->HcUsbCommand&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))	//Bit 2~3

/*
#define HOST20_iTD_Status_Active              0x08
#define HOST20_iTD_Status_DataBufferError     0x04
#define HOST20_iTD_Status_BabbleDetect        0x02
#define HOST20_iTD_Status_TransctionError     0x01


#define HOST20_siTD_Status_Active              0x80

#define HOST20_siTD_TP_All                     0x00
#define HOST20_siTD_TP_Begin                   0x01
#define HOST20_siTD_TP_Mid                     0x02
#define HOST20_siTD_TP_End                     0x03
*/

#define HOST20_FrameSize_1024                  0x00
#define HOST20_FrameSize_512                   0x01
#define HOST20_FrameSize_256                   0x02


//<3.6>.OTGHost Configuration Structure => Only Support 2 Configuration / 5 Interface / 1 Class / 5 Endpoint /1 OTG

#define  HOST20_CONFIGURATION_NUM_MAX 0X02 
#define  HOST20_INTERFACE_NUM_MAX     0X05 
#define  HOST20_ENDPOINT_NUM_MAX      0X05 
#define  HOST20_CLASS_NUM_MAX         0x01

#define HOST20_Attach_Device_Speed_Full                  0x00
#define HOST20_Attach_Device_Speed_Low                   0x01
#define HOST20_Attach_Device_Speed_High                  0x02


#define HOST20_qTD_PID_OUT                  0x00
#define HOST20_qTD_PID_IN                   0x01
#define HOST20_qTD_PID_SETUP                0x02


#define HOST20_qTD_STATUS_Active            0x80
#define HOST20_qTD_STATUS_Halted            0x40
#define HOST20_qTD_STATUS_BufferError       0x20
#define HOST20_qTD_STATUS_Babble            0x10
#define HOST20_qTD_STATUS_TransactionError  0x08
#define HOST20_qTD_STATUS_MissMicroFrame    0x04
#define HOST20_qTD_STATUS_Split             0x02
#define HOST20_qTD_STATUS_Ping              0x01

#define HOST20_HD_Type_iTD                  0x00
#define HOST20_HD_Type_QH                   0x01
#define HOST20_HD_Type_siTD                 0x02
#define HOST20_HD_Type_FSTN                 0x03



#define  HOST20_CONFIGURATION_LENGTH  0X09 
#define  HOST20_INTERFACE_LENGTH      0X09 
#define  HOST20_ENDPOINT_LENGTHX      0X07     
#define  HOST20_CLASS_LENGTHX         0X09   

#define HOST20_CONTROL_GetStatus             0x00
#define HOST20_CONTROL_ClearFeature          0x01
#define HOST20_CONTROL_SetFeature            0x03
#define HOST20_CONTROL_SetAddress            0x05
#define HOST20_CONTROL_GetDescriptor         0x06
#define HOST20_CONTROL_SetDescriptor         0x07
#define HOST20_CONTROL_GetConfiguration      0x08
#define HOST20_CONTROL_GetInterface          0x0A
#define HOST20_CONTROL_SetInterface          0x0B  
#define HOST20_CONTROL_SyncFrame             0x0C


#define HOST20_HID_GetReport                 0x01
#define HOST20_HID_GetIdle                   0x02
#define HOST20_HID_GetProtocol               0x03
#define HOST20_HID_SetReport                 0x09
#define HOST20_HID_SetIdle                   0x0A
#define HOST20_HID_SetProtocol               0x0B

#define USB_HUB_MAX_PORT_NUM 	2//4  
#define MAX_NUM_OF_DEVICE		4  

#define CMP_STOP_TIMER      FALSE
#define CMP_START_TIMER     TRUE

#define MAX_HOST_LUN            5

#define VAL_INVALID 0xff

#if (USBOTG_HOST_ISOC == ENABLE)
#define PERIODIC_FRAME_SIZE 1024 // 512 //256 //Host20_Preiodic_Frame_List_MAX
#else
#define PERIODIC_FRAME_SIZE 256 // 512 //256 //Host20_Preiodic_Frame_List_MAX
#endif

enum
{
    LUN_0  = 0,
    LUN_1,
    LUN_2,
    LUN_3,
    PTP_0,
    LUN_NONE,
};

enum
{
    SETUP_IDLE_STATE     = 0,
    SETUP_COMMAND_STATE    ,
    SETUP_DATA_IN_STATE    ,
    SETUP_STATUS_OUT_STATE ,
    SETUP_DATA_OUT_STATE   ,
    SETUP_STATUS_IN_STATE  ,
    SETUP_DONE_STATE       ,
//    SETUP_CLEAR_STALL_STATE       ,
};

enum
{
    BULKONLY_CBW_STATE     = 0,
    BULKONLY_DATA_IN_STATE    ,
    BULKONLY_DATA_OUT_STATE ,
    BULKONLY_CSW_STATE   ,
    BULKONLY_SECOND_CSW_STATE   ,
    BULKONLY_DONE_STATE       ,
 //   BULKONLY_CLEAR_STALL_STATE       ,
};

enum
{ 
   CBI_ADSC_STATE   = 1 ,
   CBI_DATA_IN_STATE    ,
   CBI_DATA_OUT_STATE   ,
   CBI_INTERRUPT_STATE   ,
   CBI_DONE_STATE   ,
};

enum
{
	BULKONLY_SIDC_OP_REQUEST_STATE     = 0,
	BULKONLY_SIDC_DATA_IN_STATE,
	BULKONLY_SIDC_DATA_OUT_STATE,
	BULKONLY_SIDC_RESPONSE_STATE,
};

enum
{
ISO_IN_STATE,
ISO_IN_DONE_STATE,
ISO_OUT_STATE,
ISO_OUT_DONE_STATE,
};

enum
{
    NONE_SM                     = 0,
    SETUP_SM                    = 1,
    SETUP_HUB_SM                = 2,
    MSDC_INIT_SM                = 3,
    MSDC_CHECK_MEDIA_PRESENT_SM = 4,
    SIDC_INIT_SM                = 5,
    SIDC_SCAN_OBJECTS_SM        = 6,
    SIDC_GET_PARTIAL_OBJECTS_SM = 7,
//rick BT
	BT_INIT_SM		= 8,	
    SIDC_GET_OBJECT_SM 		= 9,
    SIDC_GET_OBJECT_INFO_SM 		= 10,
    WIFI_SM			= 11,
    WEB_CAM_SM = 12,
#if USBOTG_HOST_CDC    
    CDC_INIT_SM = 13,
#endif
#if USBOTG_HOST_DATANG
	DATANG_INIT_SM = 14,
#endif
#if USBOTG_HOST_HID    
    HID_INIT_SM = 15,
#endif
	USB_ETHERNET_SM = 16,
};


enum _SETUP_SM_
{
    SETUP_INIT_START_STATE                  = 0,
    SETUP_PORT_RESET_STATE                  = 1,
    SETUP_PORT_RESET_DONE_STATE             = 2,
    
    SETUP_DEVICE_INIT_STATE                 = 3,
    SETUP_DEVICE_INIT_DONE_STATE            = 4,
    
    SETUP_GET_DEVICE_DESCRIPTOR_STATE       = 5,
    SETUP_GET_DEVICE_DESCRIPTOR_DONE_STATE  = 6,
    
    SETUP_SET_ADDRESS_STATE                 = 7,
    SETUP_SET_ADDRESS_DONE_STATE            = 8,
   
    SETUP_GET_CONFIG_DESCRIPTOR_STATE       = 9,
    SETUP_GET_CONFIG_DESCRIPTOR_DONE_STATE  = 10,
   
    SETUP_GET_HUB_DESCRIPTOR_STATE          = 11, 
    SETUP_GET_HUB_DESCRIPTOR_DONE_STATE     = 12,
    
    SETUP_GET_STATUS_STATE                  = 13, 
    SETUP_GET_STATUS_DONE_STATE             = 14, 
    
    SETUP_SET_CONFIG_STATE                  = 15,
    SETUP_SET_CONFIG_DONE_STATE             = 16,
    
	//rick BT
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
    SETUP_SET_INTERFACE_STATE               = 17,
    SETUP_SET_INTERFACE_DONE_STATE          = 18,
	
	SETUP_HCI_COMMAND_STATUS_EVENT_STATE	= 19,
	SETUP_HCI_COMMAND_STATUS_EVENT_DONE_STATE	= 20,
#endif    
    SETUP_INIT_STOP_STATE                   = 21,
    SETUP_INIT_STOP_DONE_STATE              = 22,
    
//    SETUP_CLEAR_STALL_STATE                 = 19,
//    SETUP_CLEAR_STALL_DONE_STATE            = 20,    
};

enum _SETUP_HUB_SM_
{
    SETUP_HUB_INIT_START_STATE              = 0,
    SETUP_HUB_SET_PORT_FEATURE_STATE        = 1,
    SETUP_HUB_SET_PORT_FEATURE_DONE_STATE   = 2,

    SETUP_HUB_CLEAR_PORT_FEATURE_STATE      = 3,
    SETUP_HUB_CLEAR_PORT_FEATURE_DONE_STATE = 4,

    SETUP_HUB_GET_PORT_STATUS_STATE         = 5,
    SETUP_HUB_GET_PORT_STATUS_DONE_STATE    = 6,

    SETUP_HUB_DONE_STATE                    = 7,
};

enum _MSDC_INIT_SM_
{
    MSDC_INIT_START_STATE                   = 0,

    MSDC_INIT_STALL_ERROR_STATE             = 1,
    MSDC_INIT_CLEAR_FEATURE_DONE_STATE      = 2,
        
    MSDC_INIT_GET_MAX_LUN_STATE             = 3,
    MSDC_INIT_GET_MAX_LUN_DONE_STATE        = 4,
   
    MSDC_INIT_TUR_STATE                     = 5,
    MSDC_INIT_TUR_DONE_STATE                = 6,
    
    MSDC_INIT_INQUIRY_STATE                 = 7,
    MSDC_INIT_INQUIRY_DONE_STATE            = 8,
    
    MSDC_INIT_MSDC_RESET_STATE                 = 9,
    MSDC_INIT_MSDC_RESET_DONE_STATE            = 10,
//    MSDC_INIT_CLEAR_STALL_STATE                 ,
//    MSDC_INIT_CLEAR_STALL_DONE_STATE            ,    
};

enum _MSDC_CHECK_MEDIA_PRESENT_SM_
{
    MSDC_CMP_START_STATE                       = 0,
    MSDC_CMP_TUR_STATE                         = 1,
    MSDC_CMP_TUR_DONE_STATE                    = 2,
    
    MSDC_CMP_READ_FORMAT_CAPACITIES_STATE      = 3,
    MSDC_CMP_READ_FORMAT_CAPACITIES_DONE_STATE = 4,
    
    MSDC_CMP_READ_CAPACITY_STATE               = 5,
    MSDC_CMP_READ_CAPACITY_DONE_STATE          = 6,
    
    MSDC_CMP_MODE_SENSE_6_STATE                = 7,
    MSDC_CMP_MODE_SENSE_6_DONE_STATE           = 8,
    
    MSDC_CMP_MODE_SENSE_10_STATE               = 9,
    MSDC_CMP_MODE_SENSE_10_DONE_STATE          = 10,
    
    MSDC_CMP_DONE_STATE                        = 11,

#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
    MSDC_CMP_USB_IF_TEST_STATE                 = 12,
#endif  //USBOTG_HOST_USBIF
};

enum _SIDC_INIT_SM_
{
    SIDC_INIT_START_STATE                   = 0,
    SIDC_INIT_START_STATE_DONE              ,
    
    SIDC_INIT_GET_DEVICE_INFO_STATE         ,
    SIDC_INIT_GET_DEVICE_INFO_DONE_STATE    ,
    
    SIDC_INIT_OPEN_SESSION_STATE            ,
    SIDC_INIT_OPEN_SESSION_DONE_STATE       ,
    
    SIDC_INIT_DONE_STATE                    ,
};

enum _SIDC_SCAN_OBJECT_SM_
{
    SIDC_SO_START_STATE                   = 0,
    SIDC_SO_GET_STORAGE_ID_STATE          ,
    SIDC_SO_GET_STORAGE_ID_DONE_STATE     ,
    
    SIDC_SO_GET_STORAGE_INFO_STATE        ,
    SIDC_SO_GET_STORAGE_INFO_DONE_STATE   ,
    
    SIDC_SO_GET_NUM_OF_OBJECT_STATE       ,
    SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE  ,
    
    SIDC_SO_GET_OBJECT_HANDLES_STATE      ,
    SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE ,
    
    SIDC_SO_GET_OBJECT_INFO_STATE         ,
    SIDC_SO_GET_OBJECT_INFO_DONE_STATE    ,
    
    SIDC_SO_DONE_STATE                    ,
};

enum _SIDC_GET_PARTIAL_OBJECT_SM_
{
    SIDC_GP_START_STATE                   = 0,
    SIDC_GP_GET_PARTIAL_OBJECT_STATE         ,
    SIDC_GP_GET_PARTIAL_OBJECT_DONE_STATE    ,
    SIDC_GP_DONE_STATE                       ,
};

enum _SIDC_GET_OBJECT_SM_
{
    SIDC_GO_START_STATE           = 0,
    SIDC_GO_GET_OBJECT_STATE         ,
    SIDC_GO_GET_OBJECT_DONE_STATE    ,
    SIDC_GO_DONE_STATE               ,
};

enum _SIDC_GET_OBJECT_INFO_SM_
{
    SIDC_GOINFO_START_STATE           = 0,
    SIDC_GOINFO_GET_OBJECT_INFO_STATE         ,
    SIDC_GOINFO_GET_OBJECT_INFO_DONE_STATE    ,
    SIDC_GOINFO_DONE_STATE               ,
};

#if 1
enum _WEB_CAM_SM_
{
    WEB_CAM_INIT             = 0,
    WEB_CAM_INIT_GET_PU         ,
    WEB_CAM_INIT_DONE           ,
    WEB_CAM_UVC_SET_VIDEO_FORMAT,
    WEB_CAM_UVC_SET_VIDEO_FORMAT_DONE,
    WEB_CAM_UVC_SETCUR_EXTENSION,
    WEB_CAM_UVC_SETCUR_PROBE    ,
    WEB_CAM_UVC_SETCUR_COMMIT   ,
    WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE,
    WEB_CAM_UVC_GETCUR_STILL_IMAGE_PROBE_DONE,
    WEB_CAM_UVC_SETCUR_STILL_IMAGE_PROBE,
    WEB_CAM_UVC_SETCUR_STILL_IMAGE_COMMIT,
    WEB_CAM_UVC_SETCUR_STILL_IMAGE_TRIGGER,
    WEB_CAM_UVC_SETCUR_VAL_DONE ,
    WEB_CAM_UVC_GETCUR_VAL_DONE ,
    WEB_CAM_UVC_SET_INTERFACE   ,
    WEB_CAM_UVC_ACTIVE_ISOC     ,
    WEB_CAM_UVC_STOP            ,
    WEB_CAM_UVC_STOP_DONE       ,
    WEB_CAM_DONE_STATE          ,
    WEB_CAM_END_STATE
};
#else
enum _WEB_CAM_SM_
{
	WEB_CAM_BEGIN_STATE0							= 0,
	WEB_CAM_BEGIN_STATE1						,

	WEB_CAM_VIDEO_CLASS_INIT_STATE0				,
	WEB_CAM_VIDEO_CLASS_INIT_STATE1				,
	WEB_CAM_VIDEO_CLASS_INIT_STATE2				,

	WEB_CAM_AUDIO_CLASS_INIT_STATE0				,
	WEB_CAM_AUDIO_CLASS_INIT_STATE1				,
	WEB_CAM_AUDIO_CLASS_INIT_DONE_STATE			,

    WEB_CAM_IN_STATE				,
    WEB_CAM_IN_DONE_STATE			,

	WEB_CAM_VIDEO_CLASS_DEINIT_STATE			,
	WEB_CAM_VIDEO_CLASS_DEINIT_DONE_STATE		,

	WEB_CAM_AUDIO_CLASS_DEINIT_STATE			,
	WEB_CAM_AUDIO_CLASS_DEINIT_DONE_STATE		,

    WEB_CAM_END_STATE							,
};
#endif

#if USBOTG_HOST_CDC
enum _CDC_INIT_SM_
{
    CDC_INIT_START_STATE                    = 0,
        
    CDC_INIT_SET_CONTROL_LINE_STATE         = 1, /*Need*/ // Depend on Abstract Control Management Function Discriptor bmCapability 
    CDC_INIT_SET_CONTROL_LINE_DONE_STATE    = 2,

    CDC_INIT_SERIAL_STATE                   = 3, /*Need*/ // Depend on Abstract Control Management Function Discriptor bmCapability 
    CDC_INIT_SERIAL_DONE_STATE              = 4,   

    CDC_INIT_SET_LINE_CODING_STATE          = 5, /*Need*/ // Depend on Abstract Control Management Function Discriptor bmCapability 
    CDC_INIT_SET_LINE_CODING_DONE_STATE     = 6, 

    CDC_INIT_GET_LINE_CODING_STATE          = 7, /*Need*/ // Depend on Abstract Control Management Function Discriptor bmCapability 
    CDC_INIT_GET_LINE_CODING_DONE_STATE     = 8,

    CDC_INIT_VENDOR_CMD_STATE               = 9,
    CDC_INIT_VENDOR_CMD_DONE_STATE          =10,    

    CDC_INIT_READY_FOR_AT_STATE             =11,
    CDC_INIT_READY_FOR_AT_DONE_STATE        =12,
};
#endif

#if USBOTG_HOST_HID
enum _HID_INIT_SM_
{
    HID_INIT_START_STATE                    = 0,

    HID_INIT_GET_DESCRIPTOR_STATE         = 1,   // GetDescriptor (Report)
    HID_INIT_GET_DESCRIPTOR_DONE_STATE    = 2,

    HID_INIT_SET_REPORT_STATE                   = 3,   // SetReport (Output Report) 
    HID_INIT_SET_REPORT_DONE_STATE              = 4,   
    
    HID_INIT_INPUT_REPORT_STATE                   = 5,  // Input Report
    HID_INIT_INPUT_REPORT_DONE_STATE              = 6,   
    
    HID_INIT_STOP_STATE              = 10,   
};
#endif

enum 
{
    COMMAND_PASS        = 0,
    COMMAND_FAILED      = 1,
    PHASE_ERROR         = 2,
};

enum
{
    USB_NO_ERROR                    =  0,
    USB_HOST_DEVICE_PLUG_OUT        = -1,
    USB_SET_EVENT_FAILED            = -2,
    USB_ENUM_ERROR                  = -3,
    USB_CSW_FAILED                  = -4,
    USB_NOT_SUPPORT_HUB_FAILED      = -5,
    USB_STALL_ERROR                 = -6,
    USB_HOST_NOT_SUPPORTED_DEVICE   = -7,
    USB_HOST_DEVICE_RESET           = -8,
    USB_UNKNOW_ERROR                = -9,
};

#ifndef __LINUX_USB_CH9_H
enum _USB_DEVICE_STATE {
    /* NOTATTACHED isn't in the USB spec, and this state acts
    * the same as ATTACHED ... but it's clearer this way.
    */
    USB_STATE_NOTATTACHED = 0,

    /* the chapter 9 device states */
    USB_STATE_ATTACHED      = 1,
    USB_STATE_POWERED       = 2,
    USB_STATE_DEFAULT       = 3,      /* limited function */
    USB_STATE_RESET         = 4,
    USB_STATE_ADDRESS       = 5,
    USB_STATE_CONFIGURED    = 6,   /* most functions */

    USB_STATE_SUSPENDED     = 7,

    /* NOTE:  there are actually four different SUSPENDED
    * states, returning to POWERED, DEFAULT, ADDRESS, or
    * CONFIGURED respectively when SOF tokens flow again.
    */
};
#endif

enum
{
    USBH_NONE_SENDER                = 0,
    USBOTG_HOST_CLASS_TASK_SENDER   = 1,
    USBOTG_HOST_DRIVER_TASK_SENDER  = 2,
    USBOTG_HOST_FOR_PTP_SENDER      = 3,
    USBOTG_HOST_FOR_ISR_SENDER      = 4,
    USBOTG_HOST_FOR_WEB_CAM_SENDER  = 5,
#if USBOTG_HOST_CDC
    USBOTG_HOST_FOR_CDC_SENDER      = 6,
#endif
#if USBOTG_HOST_DATANG
	USBOTG_HOST_FOR_DATANG_SENDER	=7,
#endif
#if USBOTG_HOST_HID
    USBOTG_HOST_FOR_HID_SENDER      = 8,
#endif
};

enum _USB_CONNECT_STATUS {  // For bConnectStatus

    USB_STATUS_DISCONNECT    = 0,    // PLUG_OUT
    USB_STATUS_CONNECT       = 1     // PLUG_IN
};

#if USBOTG_HOST_DATANG
enum _DATANG_INIT_SM_
{
    DATANG_INIT_START_STATE                    	= 0,
    DATANG_INIT_READY_FOR_AT_STATE		= 1,
    DATANG_INIT_READY_FOR_AT_DONE_STATE	= 2,
};
#endif

//=================== 3.Structure Definition =============================================================
//========================================================================================================
typedef struct {
	DWORD flag;
	BYTE *pBuf;
} MEM_BLOCK;



//<3.1>iTD Structure Definition****************************************

typedef struct   {
     
   //<1>.Next_Link_Pointer Word
   DWORD   bLinkPointer:27;         // bit 5-31  
   DWORD   bReserved:2;             // bit 3-4   
   DWORD   bType:2;                 // bit 1-2
   DWORD   bTerminal:1;             // bit 0

} Periodic_Frame_List_Cell_Structure;
/*
typedef struct  {
     
   Periodic_Frame_List_Cell_Structure   sCell[Host20_Preiodic_Frame_List_MAX]; 

} Periodic_Frame_List_Structure;
*/	                                                                
//<3.1>iTD Structure Definition****************************************

 typedef struct _iTD_Status {
     
    //<1>.Next_Link_Pointer Word
    DWORD   bStatus:4;                // bit 28-31 
    DWORD   bLength:12;               // bit 16-17
    DWORD   bInterruptOnComplete:1;   // bit 15
    DWORD   bPageSelect:3;            // bit 12-14
    DWORD   bOffset:12;               // bit 0-11           

 } iTD_Status_Structure;


 typedef struct _iTD_BufferPointer {
     
     //<1>.Next_Link_Pointer Word
    DWORD   bBufferPointer:20;      // bit 12-31       
    DWORD   bMultiFunction:12;      // bit 0-11    
 } iTD_BufferPointer_Structure;

 typedef struct _iTD {
     
    //<1>.Next_Link_Pointer Word
    DWORD   bNextLinkPointer:27;      // bit 5-31
    DWORD   bReserve_1:2;             // bit 3-4
    DWORD   bType:2;                  // bit 1-2
    DWORD   bTerminate:1;             // bit 0     
            
    //<2>.Status Word
    iTD_Status_Structure   ArrayStatus_Word[8];    

    //<3>.Buffer_Pointer Word     
    iTD_BufferPointer_Structure   ArrayBufferPointer_Word[7];                


 } iTD_Structure;


 //<3.2>siTD Structure Definition****************************************


 typedef struct _siTD {
     
     //<1>.1 Word
      DWORD   bNextQHDPointer:27;       // bit 5-31
      DWORD   bReserve_1:2;             // bit 3-4
      DWORD   bType:2;                  // bit 1-2
      DWORD   bTerminate:1;             // bit 0        
                
     //<2>.2 Word
      DWORD   bInOut:1;                 // bit 31           
      DWORD   bPortNumber:7;            // bit 24-30
      DWORD   bReserve_23:1;            // bit 23
      DWORD   bHubAddr:7;               // bit 16-22
      DWORD   bReserve_22:4;            // bit 12-15  
      DWORD   bEdNumber:4;              // bit 8-11  
      DWORD   bReserve_21:1;            // bit 7      
      DWORD   bDeviceAddress:7;         // bit 0-6  

     //<3>.3 Word     
      DWORD   bReserve_31:16;           // bit 16-32               
      DWORD   bC_Mask:8;                // bit 8-15       
      DWORD   bS_Mask:8;                // bit 0-7        

     //<4>.4 Word     
      DWORD   bInterruptOnComplete:1;   // bit 31      
      DWORD   bPageSelect:1;            // bit 30
      DWORD   bReserve_41:4;            // bit 26-29
      DWORD   bTotalBytesToTransfer:10; // bit 16-25         
      DWORD   bC_Prog_Mask:8;           // bit 8-15
      DWORD   bStatus:8;                // bit 0-7     


     //<5>.5 Word     
      DWORD   bBufferPointer_Page0:20;  // bit 12-31         
      DWORD   bCurrentOffset:12;        // bit 0-11  

     //<6>.6 Word     
      DWORD   bBufferPointer_Page1:20;  // bit 12-31
      DWORD   bReserve_61:7;            // bit 5-11
      DWORD   bTransactionPosition:2;   // bit 3-4
      DWORD   bT_Count:3;               // bit 0-2   
      
     //<6>.7 Word     
      DWORD   bBackPointer:27;          // bit 5-31
      DWORD   bReserve_71:4;            // bit 1-4
      DWORD   bBP_Terminate:1;          // bit 0  

 } siTD_Structure;




//<3.2>qTD Structure Definition****************************************

 
 typedef struct _qTD {
     
         //<1>.Next_qTD_Pointer Word
    volatile DWORD   bNextQTDPointer:27;           // bit 5-31
    volatile DWORD   bReserve_1:4;                 // bit 1-4
    volatile DWORD   bTerminate:1;                 // bit 0       

         //<2>.Alternate Next qTD Word
    volatile DWORD   bAlternateQTDPointer:27;      // bit 5-31
    volatile DWORD   bReserve_2:4;                 // bit 1-4
    volatile DWORD   bAlternateTerminate:1;        // bit 0   
         
         //<3>.Status Word     
    volatile DWORD   bDataToggle:1;                // bit 31
    volatile DWORD   bTotalBytes:15;               // bit 16-30
    volatile DWORD   bInterruptOnComplete:1;       // bit 15
    volatile DWORD   CurrentPage:3;                // bit 12-14
    volatile DWORD   bErrorCounter:2;              // bit 10-11
    volatile DWORD   bPID:2;                       // bit 8-9
    volatile DWORD   bStatus_Active:1;             // bit 7
    volatile DWORD   bStatus_Halted:1;             // bit 6
    volatile DWORD   bStatus_Buffer_Err:1;         // bit 5
    volatile DWORD   bStatus_Babble:1;             // bit 4
    volatile DWORD   bStatus_Transaction_Err:1;    // bit 3
    volatile DWORD   bStatus_MissMicroFrame:1;     // bit 2
    volatile DWORD   bStatus_SplitState:1;         // bit 1
    volatile DWORD   bStatus_PingState:1;          // bit 0


         //<4>.Buffer Pointer Word Array     
    volatile DWORD   ArrayBufferPointer_Word[5];  
     
 } qTD_Structure;


//<3.3>qHD Structure Definition****************************************

 typedef struct _qHD {
     
     //<1>.Next_qHD_Pointer Word
      DWORD   bNextQHDPointer:27;           // bit 5-31   
      DWORD   bReserve_1:2;                 // bit 3-4
      DWORD   bType:2;                      // bit 1-2                         
      DWORD   bTerminate:1;                 // bit 0                 
                
     //<2>.qHD_2 Word
      DWORD   bNakCounter:4;                // bit 28-31
      DWORD   bControlEdFlag:1;             // bit 27
      DWORD   bMaxPacketSize:11;            // bit 16-26
      DWORD   bHeadOfReclamationListFlag:1; // bit 15
      DWORD   bDataToggleControl:1;         // bit 14
      DWORD   bEdSpeed:2;                   // bit 12-13 
      DWORD   bEdNumber:4;                  // bit 8-11 
      DWORD   bInactiveOnNextTransaction:1; // bit 7      
      DWORD   bDeviceAddress:7;             // bit 0-6

     //<3>.qHD_3 Word     
      DWORD   bHighBandwidth:2;             // bit 30-31
      DWORD   bPortNumber:7;                // bit 23-29
      DWORD   bHubAddr:7;                   // bit 16-22
      DWORD   bSplitTransactionMask:8;      // bit 8-15
      DWORD   bInterruptScheduleMask:8;     // bit 0-7     

     //<4>.Overlay_CurrentqTD     
      DWORD   bOverlay_CurrentqTD;       

     //<5>.Overlay_NextqTD     
      DWORD   bOverlay_NextqTD:27;          // bit 4-31
      DWORD   bOverlay_Reserve2:4;          // bit 1-3     
      DWORD   bOverlay_NextTerminate:1;     // bit 0

     //<6>.Overlay_AlternateNextqTD     
      DWORD   bOverlay_AlternateqTD:27;             // bit 5-31
      DWORD   bOverlay_NanCnt:4;                    // bit 1-4 
      DWORD   bOverlay_AlternateNextTerminate:1;    // bit 0

     //<7>.Overlay_TotalBytes     
      DWORD   bOverlay_Direction:1;                 // bit 31
      DWORD   bOverlay_TotalBytes:15;               // bit 16-30
      DWORD   bOverlay_InterruptOnComplete:1;       // bit 15
      DWORD   bOverlay_C_Page:3;                    // bit 12-14
      DWORD   bOverlay_ErrorCounter:2;              // bit 10-11
      DWORD   bOverlay_PID:2;                       // bit 8-9
      DWORD   bOverlay_Status:8;                    // bit 0-7

     //<8>.Overlay_BufferPointer0     
      DWORD   bOverlay_BufferPointer_0:20;          // bit 12-31
      DWORD   bOverlay_CurrentOffset:12;            // bit 0-11

     //<9>.Overlay_BufferPointer1     
      DWORD   bOverlay_BufferPointer_1:20;          // bit 12-31
      DWORD   bOverlay_Reserve3:4;                  // bit 8-11
      DWORD   bOverlay_C_Prog_Mask:8;               // bit 0-7
      
     //<10>.Overlay_BufferPointer2     
      DWORD   bOverlay_BufferPointer_2:20;          // bit 12-31
      DWORD   bOverlay_S_Bytes:7;                   // bit 5-11
      DWORD   bOverlay_FrameTag:5;                  // bit 0-4
      
     //<11>.Overlay_BufferPointer3     
      DWORD   bOverlay_BufferPointer_3:20;          // bit 12-31
      DWORD   bOverlay_Reserve4:12;                 // bit 0-11
      
     //<12>.Overlay_BufferPointer4     
      DWORD   bOverlay_BufferPointer_4:20;          // bit 12-31    
      DWORD   bOverlay_Reserve5:12;                 // bit 0-11

 } qHD_Structure;

//<3.4>.Test Condition Definition****************************************


 typedef struct {
     
      BYTE   bStructureEnable; //Enable = 0x66  Disable=>Others
      BYTE   bInterruptThreshod;  //01,02,04,08,10,20,40                
      BYTE   bAsynchronousParkMode; //00=>Disable,01=>Enable     
      BYTE   bAsynchronousParkModeCounter; //01,02,03         
      BYTE   bFrameSize; //00,01,02 
          
 } Host20_Init_Condition_Structure;



//<3.5>.Host20's Attach Device Info Structure****************************************

//OTGHost Device Structure
 typedef struct
 {	
	BYTE bDEVICE_LENGTH;					// bLength
	BYTE bDT_DEVICE;						// bDescriptorType
	BYTE bVerLowByte;			            // bcdUSB
	BYTE bVerHighByte;
	
	BYTE bDeviceClass;			            // bDeviceClass
	BYTE bDeviceSubClass;			        // bDeviceSubClas;
	BYTE bDeviceProtocol;			        // bDeviceProtocol
	BYTE bEP0MAXPACKETSIZE;				    // bMaxPacketSize0
	
	BYTE bVIDLowByte;			            // idVendor
	BYTE bVIDHighByte;
	BYTE bPIDLowByte;			            // idProduct
	BYTE bPIDHighByte;
	BYTE bRNumLowByte;	                    // bcdDeviceReleaseNumber
	BYTE bRNumHighByte;

	BYTE bManufacturer;			            // iManufacturer
	BYTE bProduct;				            // iProduct
	BYTE bSerialNumber; 			        // iSerialNumber
	BYTE bCONFIGURATION_NUMBER;			    // bNumConfigurations		
 }OTGH_Descriptor_Device_Struct;

 
 typedef struct
 {	
    //<3>.Define for ED-OTG
    BYTE   bED_OTG_Length;
    BYTE   bED_OTG_bDescriptorType;
    BYTE   bED_OTG_bAttributes;
 }OTGH_Descriptor_OTG_Struct;
 
 typedef struct
 {	
     //<3>.Define for ED-1
    BYTE   bED_Length;
    BYTE   bED_bDescriptorType;
    BYTE   bED_EndpointAddress;
    BYTE   bED_bmAttributes;
    BYTE   bED_wMaxPacketSizeLowByte;
    BYTE   bED_wMaxPacketSizeHighByte;
    BYTE   bED_Interval;    
 }OTGH_Descriptor_EndPoint_Struct;


 typedef struct
 {	 
   BYTE   bClass_LENGTH;
   BYTE   bClaNumberss;
   BYTE   bClassVerLowByte;
   BYTE   bClassVerHighByte;
   BYTE   bCityNumber;
   BYTE   bFollowDescriptorNum;
   BYTE   bReport;
   BYTE   bLengthLowByte;
   BYTE   bLengthHighByte;       
 }OTGH_Descriptor_Class_Struct;


 typedef struct
 {	
     //<2>.Define for Interface-1
	BYTE bINTERFACE_LENGTH;		// bLength
	BYTE bDT_INTERFACE;			// bDescriptorType INTERFACE
	BYTE bInterfaceNumber;         // bInterfaceNumber
	BYTE bAlternateSetting;	    // bAlternateSetting
	BYTE bEP_NUMBER;			    // bNumEndpoints(excluding endpoint zero)
	BYTE bInterfaceClass;	        // bInterfaceClass
	BYTE bInterfaceSubClass;       // bInterfaceSubClass
	BYTE bInterfaceProtocol;       // bInterfaceProtocol
	BYTE bInterface;		        // iInterface
    
    OTGH_Descriptor_Class_Struct      sClass[HOST20_CLASS_NUM_MAX];
    OTGH_Descriptor_EndPoint_Struct   sED[HOST20_ENDPOINT_NUM_MAX];    
 }OTGH_Descriptor_Interface_Struct;

 typedef struct
 {	
 	BYTE  bCONFIG_LENGTH;					// bLength
	BYTE  bDT_CONFIGURATION;				// bDescriptorType CONFIGURATION
	BYTE  bTotalLengthLowByte;	            // wTotalLength, include all descriptors
	BYTE  bTotalLengthHighByte;
	BYTE  bINTERFACE_NUMBER;			    // bNumInterface
	BYTE  bConfigurationValue;				// bConfigurationValue
	BYTE  bConfiguration;			        // iConfiguration
	BYTE  bAttribute;				        // bmAttribute
	BYTE  bMaxPower;				        // iMaxPower (2mA units)

    OTGH_Descriptor_Interface_Struct        sInterface[HOST20_INTERFACE_NUM_MAX];
 }OTGH_Descriptor_Configuration_Only_Struct;



 //Support Configuration x2 
 //        Interface     x5
 //        EndPoint      x5
 //        OTG           X1 

 typedef struct
 {	
	//<1>.Basic Information
    BYTE                                   bDeviceOnHub;
    BYTE                                   bOnHubPortNumber;
	BYTE                                   bAdd;
    BYTE                                   bConnectStatus;
    BYTE                                   bPortEnableDisableStatus;    
    BYTE                                   bSpeed;  //0=>Low Speed / 1=>Full Speed / 2 => High Speed
    BYTE                                   bPortReset;  
    BYTE                                   bSuspend;  
    volatile BYTE                          bRemoteWakeUpDetected;  
    BYTE                                   bSendOK;
    BYTE                                   bSendStatusError;
    
    qTD_Structure                           *psSendLastqTD;    
    BYTE                                   *bDataBuffer;
	//<2>.Descriptor Information
    OTGH_Descriptor_Device_Struct            sDD;
    OTGH_Descriptor_Configuration_Only_Struct   saCD[HOST20_CONFIGURATION_NUM_MAX];
    OTGH_Descriptor_OTG_Struct              sOTG; 
   
    //BYTE                                   bReportDescriptor[0x74];
    //BYTE                                   bStringLanguage[10];
    //BYTE                                   bStringManufacture[0xFF];
    //BYTE                                   bStringProduct[0xFF];
    //BYTE                                   bStringSerialN[0xFF];  
    //<3>.For ISO Information    
    //BYTE                                   bISOTransferEnable;
    //DWORD                                  wISOiTDAddress[1024];

 }Host20_Attach_Device_Structure;

 //<3.7>.Control Command Structure
 typedef struct {
    BYTE   bmRequestType; //(In/Out),(Standard...),(Device/Interface...)
    BYTE   bRequest;      //GetStatus .....
    BYTE   wValueLow;     //Byte2  
    BYTE   wValueHigh;    //Byte3 
    BYTE   wIndexLow;     //Byte4
    BYTE   wIndexHigh;    //Byte5
    BYTE   wLengthLow;    //Byte6
    BYTE   wLengthHigh;   //Byte7          
 } Host20_Control_Command_Structure;


 //<3.8>.BufferPointerArray
typedef struct {
    DWORD   BufferPointerArray[8];  
} Host20_BufferPointerArray_Structure;
 


 //<3.8>.ISO_FrameBufferMode
typedef struct {
    DWORD            wFrameBufferAddress;  
    DWORD            wFrameBufferAddressOffset;        
    DWORD            wSize; 
    DWORD            wReceiveCounter;             
    DWORD            CurrentiTDNum;
} Host20_ISO_FixBufferMode_Structure;



typedef struct {
    BYTE    bLength;
    BYTE    bDescriptorType;
    WORD    bcdUSB;
    BYTE    bDeviceClass;
    BYTE    bDeviceSubClass;
    BYTE    bDeviceProtocol;
    BYTE    bMaxPacketSize;
} USB_DESCRIPTOR_HEADER, *PUSB_DESCRIPTOR_HEADER;

/* USB_DT_DEVICE: Device descriptor */
typedef struct {
    BYTE    bLength;
    BYTE    bDescriptorType;

    WORD    bcdUSB;
    BYTE    bDeviceClass;
    BYTE    bDeviceSubClass;
    BYTE    bDeviceProtocol;
    BYTE    bMaxPacketSize;
    WORD    idVendor;
    WORD    idProduct;
    WORD    bcdDevice;
    BYTE    iManufacturer;
    BYTE    iProduct;
    BYTE    iSerialNumber;
    BYTE    bNumConfigurations;
    BYTE    reserved[2];
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

/* USB_DT_EXTRA: Extra descriptor */
typedef struct {
    BYTE    bFunctionLength;
    BYTE    bDescriptorType;
    BYTE    bDescriptorSubtype;
    BYTE    *bData;
} USB_EXTRA_DESCRIPTOR, *PUSB_EXTRA_DESCRIPTOR;

/* USB_DT_ENDPOINT: Endpoint descriptor */
typedef struct {
    BYTE    bLength;
    BYTE    bDescriptorType;

    BYTE    bEndpointAddress;
    BYTE    bmAttributes;
    WORD    wMaxPacketSize;
    BYTE    bInterval;
    BYTE    bQHDArrayNum;  // For qHD array number
    //BYTE  bRefresh;       // NOTE:  these two are _only_ in audio endpoints.
    //BYTE  bSynchAddress;  // use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof.
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR;

/* USB_DT_INTERFACE: Interface descriptor */
typedef struct {
    BYTE  bLength;
    BYTE  bDescriptorType;

    BYTE  bInterfaceNumber;
    BYTE  bAlternateSetting;
    BYTE  bNumEndpoints;
    BYTE  bInterfaceClass;
    BYTE  bInterfaceSubClass;
    BYTE  bInterfaceProtocol;
    BYTE  iInterface;
    BYTE  reserved; 
    WORD  extra_num;
    USB_EXTRA_DESCRIPTOR     *extra;  // Including CS_INTERFACE  CS_ENDPOINT
    USB_ENDPOINT_DESCRIPTOR  *pEndpoint;
    
} USB_INTERFACE_DESCRIPTOR, *PUSB_INTERFACE_DESCRIPTOR;

typedef struct {
    USB_INTERFACE_DESCRIPTOR  *altsetting;
    DWORD  act_altsetting; /* active alternate setting */
    DWORD  num_altsetting; // ??  /* number of alternate settings */
    DWORD  max_altsetting; /* total memory allocated */
    
} USB_INTERFACE, *PUSB_INTERFACE;

typedef struct {
    BYTE    bLength;
    BYTE    bDescriptorType;
    
    volatile WORD   wTotalLength;
    BYTE    bNumInterfaces;
    BYTE    bConfigurationValue;
    BYTE    iConfiguration;
    BYTE    bmAttributes;
    BYTE    bMaxPower;
    BYTE    reserved[3];
    USB_INTERFACE  *pInterface;
    
} USB_CONFIG_DESCRIPTOR, *PUSB_CONFIG_DESCRIPTOR;

typedef struct {
	BYTE bAltsettings;
	BYTE ExtraFuncs[11]; // Index by bAltsetting // Including CS_INTERFACE  CS_ENDPOINT
    
}USB_INTERFACE_NUMBER, *PUSB_INTERFACE_NUMBER;

typedef struct {
    BYTE    bDescLength;
    BYTE    bDescriptorType;
    BYTE    bNbrPorts;
    WORD    wHubCharacteristics;
    BYTE    bPwrOn2PwrGood;
    BYTE    bHubContrCurrent;
    BYTE    bDeviceRemovable;
    BYTE    bPortPwrCtrlMask;
//    BYTE    reserved[3];
} USB_HUB_DESCRIPTOR, *PUSB_HUB_DESCRIPTOR;

typedef struct _USB_DEVICE_STATUS_
{
    WORD   bfReserved:14;       // bits 2-15
    WORD   bfRemoteWakeup:1;    // bit  1
    WORD   bfSelfPowered:1;     // bit  0
//    WORD   reserved;
} USB_DEVICE_STATUS, *PUSB_DEVICE_STATUS; 


// the data (WORD) get from hardware, so high low byte need to reverse
typedef struct _HUB_PORT_STATUS_
{
    WORD   bfReserved1:3;       // bits 5-7
    WORD   bfReset:1;           // bit  4
    WORD   bfOverCurrent:1;     // bit  3
    WORD   bfSuspend:1;         // bit  2
    WORD   bfEnable:1;          // bit  1
    WORD   bfConnection:1;      // bit  0
    
    WORD   bfReserved2:3;       // bits 13-15
    WORD   bfIndicator:1;       // bit  12
    WORD   bfTestMode:1;        // bit  11
    WORD   bfHighSpeed:1;       // bit  10
    WORD   bfLowSpeed:1;        // bit  9
    WORD   bfPower:1;           // bit  8
} HUB_PORT_STATUS, *PHUB_PORT_STATUS;

typedef struct _HUB_PORT_CHANGE_
{
    WORD   bfReserved1:3;       // bits 5-7
    WORD   bfReset:1;           // bit  4
    WORD   bfOverCurrent:1;     // bit  3
    WORD   bfSuspend:1;         // bit  2
    WORD   bfEnable:1;          // bit  1
    WORD   bfConnection:1;      // bit  0
    
    WORD   bfReserved:8;       // bits 8-15
} HUB_PORT_CHANGE, *PHUB_PORT_CHANGE;

typedef struct _HUB_GET_PORT_STATUS_
{
    HUB_PORT_STATUS sHubPortStatus;
    HUB_PORT_CHANGE sHubPortChange;
} HUB_GET_PORT_STATUS, *PHUB_GET_PORT_STATUS;

/*-------------------------------------------------------------------------*/



typedef struct {
	BYTE	bRequestType;
	BYTE	bRequest;
	WORD	wValue;
	WORD	wIndex;
	WORD	wLength;
} USB_CTRL_REQUEST, *PUSB_CTRL_REQUEST;

typedef struct _ST_USBOTG_SETUP_PB_
{
    WORD                wDataInLength;
    WORD                wDataOutLength;
    DWORD               dwDataAddress;
    USB_CTRL_REQUEST    stCtrlRequest;
    DWORD               dwSetupState;
} ST_USBOTG_SETUP_PB, *PST_USBOTG_SETUP_PB;
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
struct urb;                                     /* forward declaration */

struct ehci_qtd {
    qTD_Structure   qTD;
    struct urb      *urb;
};

/*
 * qTD Token word
 */
#define	QTD_TOGGLE	(1 << 31)	/* data toggle */
#define	QTD_LENGTH(tok)	(((tok)>>16) & 0x7fff)
#define	QTD_IOC		(1 << 15)	/* interrupt on complete */
#define	QTD_CERR(tok)	(((tok)>>10) & 0x3)
#define	QTD_PID(tok)	(((tok)>>8) & 0x3)
#define	QTD_STS_ACTIVE	(1 << 7)	/* HC may execute this */
#define	QTD_STS_HALT	(1 << 6)	/* halted on error */
#define	QTD_STS_DBE	(1 << 5)	/* data buffer error (in HC) */
#define	QTD_STS_BABBLE	(1 << 4)	/* device was babbling (qtd halted) */
#define	QTD_STS_XACT	(1 << 3)	/* device gave illegal response */
#define	QTD_STS_MMF	(1 << 2)	/* incomplete split transaction */
#define	QTD_STS_STS	(1 << 1)	/* split transaction state */
#define	QTD_STS_PING	(1 << 0)	/* issue PING? */

struct ehci_qhd {
    qHD_Structure   qTD;
    qTD_Structure   *dummy; 
    qTD_Structure   *head; 
};


///////////////////////////////////////////
typedef struct _USB_OTG_HOST_ISOC_DES_
{
    WORD    wMaxPacketSize;
    WORD    wInterval;
    BYTE    bEdPointNumber;
    BYTE    bInterfaceNumber;
    BYTE    bAlternativeSetting;
    BYTE    bDirection;
} ST_USB_OTG_HOST_ISOC_DES, *PST_USB_OTG_HOST_ISOC_DES; 



/* move to usbotg_api.h
typedef struct _BIT_MAP_PROCESSING_UNIT_
{
    WORD   bmWbc:1;                 // bit  7 // White Balance Component
    WORD   bmWbt:1;                 // bit  6 // White Balance Temperature
    WORD   bmGamma:1;               // bit  5
    WORD   bmSharpness:1;           // bit  4
    WORD   bmSaturation:1;          // bit  3
    WORD   bmHue:1;                 // bit  2
    WORD   bmContrast:1;            // bit  1
    WORD   bmBrightness:1;          // bit  0
    
    WORD   bmDigitalMultiplierLimit:1;  // bit  15
    WORD   bmDigitalMultiplier:1;       // bit  14
    WORD   bmWbcAuto:1;                 // bit  13
    WORD   bmWbtAuto:1;                 // bit  12
    WORD   bmHueAuto:1;                 // bit  11
    WORD   bmPowerLineFreq:1;           // bit  10
    WORD   bmGain:1;                    // bit  9
    WORD   bmBacklightCompensation:1;   // bit  8
} UVC_BM_PROCESSING_UNIT, *PUVC_BM_PROCESSING_UNIT;

typedef struct _UVC_BIT_MAP_CCC_ // Containing Capabilities of the Control
{
    BYTE   bmReserved:3;      // bits 5-7
    BYTE   bmAsynchronous:1;  // bit  4
    BYTE   bmAutoupdate:1;    // bit  3
    BYTE   bmDisable:1;       // bit  2
    BYTE   bmSetValue:1;      // bit  1
    BYTE   bmGetValue:1;      // bit  0
} UVC_BIT_MAP_CCC, *PUVC_BIT_MAP_CCC;

typedef struct _UVC_GET_REQUEST_OF_PROCESSING_UNIT_
{
    UVC_BIT_MAP_CCC         sInfo;
    WORD                    wStatus;
    BYTE                    bControlSelector;
    WORD                    wMin;
    WORD                    wMax;
    WORD                    wRes;
    WORD                    wDef;
} UVC_GET_REQUEST_OF_PROCESSING, *PUVC_GET_REQUEST_OF_PROCESSING;

typedef struct _UVC_PROCESSING_UNIT_
{
    UVC_BM_PROCESSING_UNIT  sUnit; 
    BYTE                    bUnitID;
    BYTE                    bControlUnits;
} UVC_PROCESSING_UNIT, *PUVC_PROCESSING_UNIT;
*/


//////////////////////////////////////////////////////////////////////////////


typedef struct _ST_HUB_CLASS_DESCRIPTOR_
{
    USB_HUB_DESCRIPTOR      sHubDescriptor;
    USB_ENDPOINT_DESCRIPTOR sInterruptInDescriptor;	
    HUB_GET_PORT_STATUS     sHubGetPortStatus[USB_HUB_MAX_PORT_NUM];
    qHD_Structure           **hstInterruptInqHD;
    qTD_Structure           **hstIntInWorkqTD;
    qTD_Structure           **hstIntInSendLastqTD;    
    qTD_Structure           **hstInterruptInqTD;    
    void                    (*fpAppClassInterruptActive)(WHICH_OTG);
    void                    (*fpAppClassInterruptIoc)   (WHICH_OTG);
    BYTE                    bMaxInterruptInEpNumber;
    BYTE                    bDeviceAddress;
    BYTE                    bIsPortChanged;
    BYTE                    bPortNumber;
    BYTE                    bPortStatusChange;
    BYTE                    bReerved[3];
} ST_HUB_CLASS_DESCRIPTOR, *PST_HUB_CLASS_DESCRIPTOR;

typedef struct  {
    DWORD   dwFrameList[Host20_Preiodic_Frame_List_MAX];
} ISOC_FRAME_LIST_STRUCTURE, *PISOC_FRAME_LIST_STRUCTURE;


typedef struct _ST_APP_CLASS_DESCRIPTOR_
{
    BYTE                    bDeviceAddress; 
    BYTE                    bDeviceConfigVal;
    BYTE                    bAppDesIdx; 
    BYTE                    bReserved0;
    USB_ENDPOINT_DESCRIPTOR sBulkInDescriptor;
    USB_ENDPOINT_DESCRIPTOR sBulkOutDescriptor;
    DWORD                   dDataOutLen;
    DWORD                   dDataInLen;
    DWORD                   dDataBuffer;
    DWORD                   dDataOutBuffer;
    DWORD                   dIntDataBuffer;
    BYTE                    bIntDataInLen;
    BYTE                    bReserved1[3];
    DWORD                   dLba;
#if USBOTG_HOST_PTP
    STI_CONTAINER           sStiCommand;
    STI_CONTAINER           sStiResponse;
    DWORD                   dSorageIdsIdx;
    DWORD                   dSorageIds[MAX_NUM_OF_STORAGE_ID];
    STI_STORAGE_INFO        sStorageInfos[MAX_NUM_OF_STORAGE_ID];
    DWORD                   dNumOfObjectsInStorage[MAX_NUM_OF_STORAGE_ID];
    DWORD                   dObjectHandlesIdx;
    DWORD                   dObjectHandles[MAX_NUM_OF_OBJECT_HANDLES];
    DWORD                   dFolderObjectHandlesIdx;
    DWORD                   dFolderObjectHandles[MAX_NUM_OF_FOLDER_OBJECT_HANDLES];
	DWORD					dOPUsed;
#endif    
#if USBOTG_HOST_CDC
    CDC_DISCRIPTOR          sCdcFunction;
#endif  
#if USBOTG_HOST_HID
    BYTE     bHidDescriptorNum;       // Total count of Hid Decriptor
    BYTE     bHidDescriptorCurrent;  // Current Hid Decriptor
    BYTE     bHidReserved[2];
    BYTE    *pbBuffer;
    USB_HID_DESCRIPTOR *psHidDescriptor;  // Hid Decriptor structure
    DWORD	dwCurKey;
#endif  
#if USBOTG_WEB_CAM
    WEBCAM_STATE            eWebcamState;
#endif
    //CBW                     sCbw;
    //CSW                     sCsw;
    CBW                     *psCbw;
    CSW                     *psCsw;
    DWORD                   dwBulkOnlyState;
    //BYTE                    *pbDataPage_In[5]; 
    //BYTE                    *pbDataPage_Out[5];      	    					
    DWORD                   dPresent[MAX_HOST_LUN]; /* 1:present ,0: absent */
    BYTE                    bSectorExp;
    BYTE                    bMaxLun;
    BYTE                    bDevLun;
    BYTE                    bBulkInQHDArrayNum;
    BYTE                    bBulkOutQHDArrayNum;
    BYTE                    bIntInQHDArrayNum;
    BYTE                    bIntOutQHDArrayNum;
    BYTE                    bIsocInItdArrayNum;
    BYTE                    bIsocOutItdArrayNum;
    BYTE                    bReserved2[3];
    SWORD                   swBulkOnlyError;
    WORD                    bReserved4;
    DWORD                   dRequestSenseCode[MAX_HOST_LUN];
    DWORD                   dEventSet[MAX_HOST_LUN]; /* 1:setevent ,0: not set */
    USB_ENDPOINT_DESCRIPTOR sInterruptInDescriptor;  // for fuji4700 CBI 
} ST_APP_CLASS_DESCRIPTOR, *PST_APP_CLASS_DESCRIPTOR;

#define ISOC_DATA_NUM_BUF	16
typedef struct _ISOC_DATA_STREAM_
{
    DWORD dwStreamActive;//DWORD streamActive;
    DWORD dwBufferCurIndex;//DWORD bufferCurIndex;
    DWORD dwBufferActive[ISOC_DATA_NUM_BUF];//DWORD bufferActive[AUDIO_NUM_BUF];
    DWORD dwBufferLength[ISOC_DATA_NUM_BUF];//DWORD bufferLength[AUDIO_NUM_BUF];
    BYTE  *pbAudioBuffer[ISOC_DATA_NUM_BUF];//BYTE  *audioBuffer[AUDIO_NUM_BUF];
    // ISO
    DWORD dwiTdInt;;//DWORD iTDInt;
    DWORD dwiTdIdx;//	DWORD iTDIdx;
    DWORD dwiTdNum[ISOC_DATA_NUM_BUF];//DWORD iTDNum[2];
    DWORD dwOriginalFrameNumber[ISOC_DATA_NUM_BUF];//DWORD wOriginalFrameNumber[2];
    DWORD dwLastFrameNumber[ISOC_DATA_NUM_BUF];//DWORD wLastFrameNumber[2];
    DWORD dwIsoActive[ISOC_DATA_NUM_BUF];//DWORD bISOInActive[2];
} ISOC_DATA_STREAM, *PISOC_DATA_STREAM;

#define NUMBER_OF_ISOC_Q_ELEMENTS       16 //32//16//8//4
//#define NUMBER_OF_BYTE_COUNT_OF_ELEMENT (128*0x1000)//(10*OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000)// * 8)//2048 //512
typedef struct _ST_USBH_ISOC_BUFFER_
{
    BYTE  *pbDataBuffer;
    DWORD dwLength;
    DWORD dwOriginalFrameNumber;
    DWORD dwOriginaliTd;
    DWORD dwLastFrameNumber;
    DWORD dwLastiTd;
} USBH_ISOC_BUFFER, *PUSBH_ISOC_BUFFER;

typedef struct _ST_USBH_ISOC_BUFFER_QUEUE_
{
    USBH_ISOC_BUFFER stIsocBuff[NUMBER_OF_ISOC_Q_ELEMENTS];
    WORD             wHead;
    WORD             wTail;
} USBH_ISOC_BUFFER_Q, *PUSBH_ISOC_BUFFER_Q;
  


typedef struct _ST_USBH_DEVICE_DESCRIPTOR_
{
    BYTE                        bDeviceSpeed;       /* 0:Full 1:Low 2:High */
    BYTE                        bDeviceAddress;
    BYTE                        bDeviceStatus; 
    BYTE                        bDeviceConfigVal;
    ST_USBOTG_SETUP_PB          sSetupPB;
    PST_APP_CLASS_DESCRIPTOR    psAppClass; // Just pointer was assign to ST_USB_OTG_HOST.psAppClassDescriptor after usb enumeration.
    PST_HUB_CLASS_DESCRIPTOR    psHubClass;
    USB_DEVICE_DESCRIPTOR       sDeviceDescriptor;
    USB_DEVICE_STATUS           sDeviceStatus;
//#if USBOTG_HOST_DESC
    USB_CONFIG_DESCRIPTOR       *pConfigDescriptor; // Please call function (usbotg_host_Setup.c) to get members under this via check structure whether NULL.
//#else    
    USB_CONFIG_DESCRIPTOR       sConfigDescriptor[MAX_NUM_OF_CONFIG];
    USB_INTERFACE_DESCRIPTOR    sInterfaceDescriptor[MAX_NUM_OF_INTERFACE];
    USB_ENDPOINT_DESCRIPTOR     sEndpointDescriptor[MAX_NUM_OF_ENDPOINT];
//#endif    
    WORD                        wStateMachine;
    WORD                        wCurrentExecutionState;
    BYTE                        bUsbhDevDesIdx; 
    BYTE                        bDeviceConfigIdx; 
    BYTE                        bDeviceInterfaceIdx;
    BYTE                        bMaxBulkInEpNumber;
    BYTE                        bMaxBulkOutEpNumber;
    BYTE                        bMaxInterruptInEpNumber; //bMaxInterruptEpNumber
    BYTE                        bMaxInterruptOutEpNumber;
    BYTE                        bTotalNumberOfInterfaceWithIsocEp;
    BYTE                        bNumOfIsocFrameList;//bNumOfIsocFrameList;
    BYTE                        bIsocInEpPflIndex;
    BYTE                        bIsocOutEpPflIndex;
    BYTE                        bIsoInEnable;
    BYTE                        bIsoOutEnable;
    BYTE                        bAppInterfaceClass;
    BYTE                        bAppInterfaceNumber;
    BYTE                        bIsocAlternateSetting;
    DWORD                       dwWhichBulkPipeDone;
    DWORD                       dwWhichInterruptPipeDone;
    qHD_Structure               *pstControlqHD[2]; // [0] is for beginning; [1] is for USB_STATE_ADDRESS
    qHD_Structure               **hstBulkInqHD;     // h:handle; st:structure
    qHD_Structure               **hstBulkOutqHD;     // h:handle; st:structure
    qHD_Structure               **hstInterruptInqHD;  // **hstInterruptqHD;
    qHD_Structure               **hstInterruptOutqHD;
    qTD_Structure               *psControlqTD;    
    qTD_Structure               *pstControlWorkqTD;
    qTD_Structure               *pstControlSendLastqTD;
    qTD_Structure               **hstIntInWorkqTD;  // **hstIntWorkqTD
    qTD_Structure               **hstIntInSendLastqTD; // **hstIntSendLastqTD
    qTD_Structure               **hstInterruptInqTD;// **hstInterruptqTD
    qTD_Structure               **hstIntOutWorkqTD;
    qTD_Structure               **hstIntOutSendLastqTD;
    qTD_Structure               **hstInterruptOutqTD; 
    qTD_Structure               **hstBulkInWorkqTD;    
    qTD_Structure               **hstBulkInSendLastqTD;    
    qTD_Structure               **hstBulkInqTD; 
    qTD_Structure               **hstBulkOutWorkqTD;    
    qTD_Structure               **hstBulkOutSendLastqTD;    
    qTD_Structure               **hstBulkOutqTD;    
    iTD_Structure               **hstIsocInItd;
    iTD_Structure               **hstIsocOutItd;
    BYTE                        *bDataBuffer;
    void                        (*fpAppClassBulkActive)     (WHICH_OTG);
    void                        (*fpAppClassBulkIoc)        (WHICH_OTG);
    void                        (*fpAppClassSetupIoc)       (WHICH_OTG);
    void                        (*fpAppClassInterruptActive)(WHICH_OTG);
    void                        (*fpAppClassInterruptIoc)   (WHICH_OTG);

    PST_USB_OTG_HOST_ISOC_DES   (*fpGetIsocInDesForIsoInActive)   (BYTE,BYTE,WHICH_OTG);
    SDWORD                      (*fpIsocInDataProcessForIsoInIoc) (WHICH_OTG);
    PST_USB_OTG_HOST_ISOC_DES   (*fpGetIsocOutDesForIsoOutActive) (WHICH_OTG);
    void                        (*fpSendIsocInDataTo)             (WHICH_OTG);
    SDWORD                      (*fpParseIsocInData)              (BYTE*, DWORD);

    DWORD                       dwActivedQHD[Host20_qHD_MAX];
    BYTE                        bQHStatus;
    BYTE                        bSendStatusError;
    BYTE                        bRemoteWakeUpDetected;  
    BYTE                        bPortReset;
    BYTE                        bActivedQHdIdx;
    BYTE                        bCmdTimeoutError;
    BYTE                        bConnectStatus;
    BYTE                        bSuspend;
    struct urb                  *urb;
    BYTE                        isWlan;         /* is WLAN dongle */
    BYTE                        bIsNetUsb;      /* billwang */
    BYTE                        bMaxIsoInEpNumber;
    BYTE                        bMaxIsoOutEpNumber;
#if USB_HOST_ISO_TEST
    //<3>.For ISO Information    
    DWORD                       bMaxIniTDNum;
    DWORD                       wOriginalFrameNumber;
    DWORD                       wISOiTDAddress[1024];
#endif
    //JL, 20100419: for isoc
    ISOC_DATA_STREAM            stIsocDataStream;
    ISOC_DATA_STREAM            stIsocOutDataStream;
    PISOC_FRAME_LIST_STRUCTURE  pstPeriodicFrameList;//ISOC_FRAME_LIST_STRUCTURE
    DWORD                       dwIsocInBufferSize;
    DWORD                       dwIsocOutBufferSize;
    DWORD                       dwIsocInDataCount;
    BYTE                        *pbIsocInBuffer;
    DWORD                       dwForceInterval;
    DWORD                       dwTotalPageNumber;
    DWORD                       dwPageIndex;
    DWORD                       dwPageBuffer[Host20_Page_MAX];
    USBH_ISOC_BUFFER_Q          stUsbhIsocOutBufferQueue;
    USBH_ISOC_BUFFER_Q          stUsbhIsocInBufferQueue;
    DWORD                       dwQueueElementByteCount;
    UVC_PROCESSING_UNIT         sUvcProcessUnit;
    PUVC_GET_REQUEST_OF_PROCESSING psUvcReq;
    UVC_PROCESSING_CONTROLS     eCurPuControl;
    UVC_STILL_IMAGE             sUvcStillImageInfo;
} ST_USBH_DEVICE_DESCRIPTOR, *PST_USBH_DEVICE_DESCRIPTOR;


///////////////////////////////////////
// PTP Mail TAG
///////////////////////////////////////
// PTP Mail TAG
typedef struct _ST_USBH_PTP_MAIL_TAG_
{
    ST_SEARCH_INFO*     psSearchInfo;
    DWORD               dwMaxElement;
    DWORD               dwBuffer;
    DWORD               dwRecordElementAddr;
    DWORD               reserved;
    E_DEVICE_ID         wMCardId;
    WORD                wCmd;
    WORD                wStateMachine;
    WORD                wCurrentExecutionState;
    SWORD               reserved1;
} USBH_PTP_MAIL_TAG, *PUSBH_PTP_MAIL_TAG;


void SetUsbhReadyToReadWrite (WHICH_OTG eWhichOtg);
BOOL CheckIfUsbhReadyToReadWrite (WHICH_OTG eWhichOtg);
DWORD GetUsbhCardPresentFlag(BYTE bLun, WHICH_OTG eWhichOtg);
void SetHostToResetDeviceFlag(WHICH_OTG eWhichOtg);
void SetHostFinalizedFlag(WHICH_OTG eWhichOtg);
void SetCheckMediaPresentTimerFlag(BOOL flag, WHICH_OTG eWhichOtg);
BOOL GetCheckMediaPresentTimerFlag(WHICH_OTG eWhichOtg);
DWORD GetRetryCount(WHICH_OTG eWhichOtg);
void ClearRetryCount(WHICH_OTG eWhichOtg);
void *allo_mem(DWORD cnt, WHICH_OTG eWhichOtg);
DWORD free1(void *buf, WHICH_OTG eWhichOtg);
SDWORD UsbOtgHostTaskInit(WHICH_OTG eWhichOtg);
void UsbOtgHostTaskTerminate(WHICH_OTG eWhichOtg);
void UsbOtgHostEventPlugOut(WHICH_OTG eWhichOtg, BOOL bMailTrack);
DWORD GetUsbhMailEnvelope(WORD sender, WHICH_OTG eWhichOtg);
void UsbOtgHostSetAddressForUsbAddressState(BYTE device_address, PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc);
void UsbOtgHstSetMaxPacketSizeForControlPipe(BYTE max_packet_size, PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc);
void UsbhStorageInit (ST_MCARD_DEV *psDev, BYTE bType);
void USbhDeviceDisable(BYTE bMcardID);
BOOL UsbhStorage_GetDriveInstalled(BYTE card_id);
void UsbOtgHostSetSwapBuffer1RangeEnable(DWORD start, DWORD end, WHICH_OTG eWhichOtg);
void UsbOtgHostSetSwapBuffer2RangeEnable(DWORD start, DWORD end, WHICH_OTG eWhichOtg);
void UsbOtgHostSetSwapBuffer2RangeDisable(WHICH_OTG eWhichOtg);
void UsbOtgHostSetSwapMemoryPool(WHICH_OTG eWhichOtg);
DWORD UsbOtgHostGetSidcGetObjectRxBufferAddress(WHICH_OTG eWhichOtg);
void UsbOtgHostDeviceDescriptorInit(PST_USBH_DEVICE_DESCRIPTOR pUsbhDev);
void UsbHostFinal(SWORD errCode, WHICH_OTG eWhichOtg);
void DisableUsbOtgHostInterrupt(WHICH_OTG eWhichOtg);
SDWORD SendMailToUsbOtgHostClassTask(DWORD dwEnvelopeAddr, WHICH_OTG eWhichOtg);
void UsbOtgHostClose(WHICH_OTG eWhichOtg);
WHICH_OTG GetWhichUsbOtgByCardId (WORD wCardId);
BYTE GetMailBoxIdbyCardId (WORD wCardId);
BYTE GetLunbyCardId (WORD wCardId);
BYTE UsbOtgHostIsr(WHICH_OTG eWhichOtg);
void UsbOtgHostGetQhdForEachEd(WHICH_OTG eWhichOtg);
void UsbOtgHostInactiveAllqTD (WHICH_OTG eWhichOtg);

void flib_Host20_ReleaseStructure(BYTE Type,DWORD pwAddress, WHICH_OTG eWhichOtg);
void flib_Host20_StopRun_Setting(BYTE bOption, WHICH_OTG eWhichOtg);
void flib_Host20_Asynchronous_Setting(BYTE bOption, WHICH_OTG eWhichOtg);
void flib_Host20_Periodic_Setting(BYTE bOption, WHICH_OTG eWhichOtg);
void flib_Host20_Interrupt_Init(WHICH_OTG eWhichOtg);
void flib_Host20_QHD_Control_Init(WHICH_OTG eWhichOtg);
BYTE flib_Host20_Send_qTD_Done(qHD_Structure *spTempqHD, WHICH_OTG eWhichOtg);
BYTE flib_Host20_Issue_Interrupt_Active (BYTE    bArrayListNum, WORD    hwSize, DWORD   *pwBufferArray, DWORD   wOffset, BYTE    bDirection, WHICH_OTG eWhichOtg);
DWORD flib_Host20_GetStructure(BYTE Type, WHICH_OTG eWhichOtg);
qTD_Structure *  flib_Host20_Issue_Bulk_Active_Multi_TD (  BYTE    bArrayListNum,
                                                           WORD    hwSize,
                                                           DWORD   *pwBufferArray,
                                                           DWORD   wOffset,
                                                           BYTE    bDirection,
                                                           BOOL    fActive,
                                                           WHICH_OTG eWhichOtg);
qTD_Structure *  flib_Host20_Issue_Bulk_Active (   BYTE    bArrayListNum,
                                                   WORD    hwSize,
                                                   DWORD   *pwBufferArray,
                                                   DWORD   wOffset,
                                                   BYTE    bDirection,
                                                   WHICH_OTG eWhichOtg);

BYTE flib_Host20_Suspend(WHICH_OTG eWhichOtg);
BYTE flib_Host20_Resume(WHICH_OTG eWhichOtg);
void UsbHostReset(WHICH_OTG eWhichOtg); // For BT's Usage

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
void InitiaizeUsbOtgHost(BOOL isFirstTime, WHICH_OTG eWhichOtg);
#endif 

BYTE UsbOtgHostGetTotalNumberOfBulkEp(WHICH_OTG eWhichOtg);

#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
void TestModeForHC(WHICH_OTG eWhichOtg);
#endif // USBOTG_HOST_USBIF

#if (USBOTG_HOST_ISOC == ENABLE)
///
///@ingroup USB OTG Inter Use
///@brief   Get the best of isoc end point number.
///
///@param   bInterfaceClass        Interface Class Definition.
/// - USB_CLASS_PER_INTERFACE	    0	/* for DeviceClass */
/// - USB_CLASS_AUDIO	            1
/// - USB_CLASS_COMM		    2   /* Communications Device Class */
/// - USB_CLASS_HID		    3
/// - USB_CLASS_PHYSICAL	    5
/// - USB_CLASS_STILL_IMAGE	    6
/// - USB_CLASS_PRINTER	            7
/// - USB_CLASS_MASS_STORAGE	    8
/// - USB_CLASS_HUB	            9
/// - USB_CLASS_CDC_DATA	    0x0a   /* Communications Device Class DATA */
/// - USB_CLASS_CSCID               0x0b	/* chip+ smart card */
/// - USB_CLASS_CONTENT_SEC	    0x0d	/* content security */
/// - USB_CLASS_VIDEO		    0x0E	/* VIDEO CLASS */
/// - USB_CLASS_WIRELESS_BT	    0xE0    /* for BlueTooth */
/// - USB_CLASS_APP_SPEC	    0xfe
/// - USB_CLASS_VENDOR_SPEC	    0xff
///@param   bDirection      Direction of end point.
/// - USB_DIR_IN.
/// - USB_DIR_OUT.
///@param   eWhichOtg   assign which USB OTG
/// - USBOTG0
/// - USBOTG1
///
///@retval VAL_INVALID(0xff) means error cause of no such as end point can be returned.
///@retval 1 means the returned end pointer number is valid.
///
BYTE GetBestIsocEndPointNumber (BYTE bInterfaceClass, BYTE bDirection, WHICH_OTG eWhichOtg);
///
///@ingroup USB OTG Inter Use
///@brief   Get the best of isoc end point related info such as 
/// wMaxPacketSize, wInterval, bEdPointNumber, bInterfaceNumber, bAlternativeSetting, bDirection
///
///@param   bInterfaceClass        Interface Class Definition.
/// - USB_CLASS_PER_INTERFACE	    0	/* for DeviceClass */
/// - USB_CLASS_AUDIO	            1
/// - USB_CLASS_COMM		    2   /* Communications Device Class */
/// - USB_CLASS_HID		    3
/// - USB_CLASS_PHYSICAL	    5
/// - USB_CLASS_STILL_IMAGE	    6
/// - USB_CLASS_PRINTER	            7
/// - USB_CLASS_MASS_STORAGE	    8
/// - USB_CLASS_HUB	            9
/// - USB_CLASS_CDC_DATA	    0x0a   /* Communications Device Class DATA */
/// - USB_CLASS_CSCID               0x0b	/* chip+ smart card */
/// - USB_CLASS_CONTENT_SEC	    0x0d	/* content security */
/// - USB_CLASS_VIDEO		    0x0E	/* VIDEO CLASS */
/// - USB_CLASS_WIRELESS_BT	    0xE0    /* for BlueTooth */
/// - USB_CLASS_APP_SPEC	    0xfe
/// - USB_CLASS_VENDOR_SPEC	    0xff
///@param   bDirection      Direction of end point.
/// - USB_DIR_IN.
/// - USB_DIR_OUT.
///@param   eWhichOtg   assign which USB OTG
/// - USBOTG0
/// - USBOTG1
///
///@retval NULL means error cause of no such as end point can be returned.
///@retval not NULL means the returned pointer is valid.
///
PST_USB_OTG_HOST_ISOC_DES GetBestIsocDes (BYTE bInterfaceClass, BYTE bInterfaceNumber, BYTE bDirection, WHICH_OTG eWhichOtg);
PST_USB_OTG_HOST_ISOC_DES GetIsocEpDes (BYTE bInterfaceClass, BYTE bInterfaceNumber, BYTE bDirection, BYTE bEpIndex, WHICH_OTG eWhichOtg);

void OTGH_PT_ISO_IN(DWORD wEndPt,DWORD wSize, DWORD wMaxPacketSize, WHICH_OTG eWhichOtg);
void UsbOtgHostIsocEnable(WHICH_OTG eWhichOtg);
void UsbOtgHostIsocDisable(WHICH_OTG eWhichOtg);
void UsbOtgHostIsocStreamDataInit(WHICH_OTG eWhichOtg);
void UsbOtgHostIsocStreamDataStart(WHICH_OTG eWhichOtg);
void UsbOtgHostIsocStreamDataStop(WHICH_OTG eWhichOtg);
//void UsbOtgHostIsocDataProcess(WHICH_OTG eWhichOtg);
void UsbOtgHostIssueIso(DWORD       wEndPt,
                        DWORD       wMaxPacketSize,
                        DWORD       wSize,
                        DWORD       *pwBufferArray,
                        DWORD       wOffset,
                        BYTE        bDirection,
                        BYTE        bMult, 
                        WHICH_OTG   eWhichOtg);

BOOL UsbOtgHostEnableIsoc(WHICH_OTG eWhichOtg);
BYTE UsbOtgHostGetTotalNumberOfPriodicEp(WHICH_OTG eWhichOtg);
void UsbOtgHostPrepareDataPage(WHICH_OTG eWhichOtg);
void UsbOtgHostReleaseAllDataPage(WHICH_OTG eWhichOtg);
DWORD UsbOtgHostGetDataPage(WHICH_OTG eWhichOtg);

void SetQueueElementByteCount(DWORD dwLength, WHICH_OTG eWhichOtg);
DWORD GetQueueElementByteCount(WHICH_OTG eWhichOtg);
SDWORD UsbOtgHostIsocQueueMemoryAllocat(BYTE bIsoInEnable, BYTE bIsoOutEnable, WHICH_OTG eWhichOtg);
SDWORD UsbOtgHostIsocQueueMemoryFree(BYTE bIsoInEnable, BYTE bIsoOutEnable, WHICH_OTG eWhichOtg);
void UsbOtgHostIsocQueueInit(WHICH_OTG eWhichOtg);
PUSBH_ISOC_BUFFER UsbOtgHostIsocOutDataDequeue(WHICH_OTG eWhichOtg);
PUSBH_ISOC_BUFFER UsbOtgHostIsocInDataDequeue(WHICH_OTG eWhichOtg);
PUSBH_ISOC_BUFFER UsbOtgHostIsocOutDataDequeueGo(WHICH_OTG eWhichOtg);
PUSBH_ISOC_BUFFER UsbOtgHostIsocInDataDequeueGo(WHICH_OTG eWhichOtg);
SDWORD UsbOtgHostIsocOutDataEnqueueGo(BYTE **hData, DWORD dwLength, WHICH_OTG eWhichOtg);
SDWORD UsbOtgHostIsocOutDataEnqueue(BYTE *pData, DWORD dwLength, WHICH_OTG eWhichOtg);
SDWORD UsbOtgHostIsocInDataEnqueueDataBuffer(BYTE **hData, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg);
void UsbOtgHostIsocInDataEnqueueDataLength(DWORD dwDataLength, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg);
void UsbOtgHostIsocOutDataEnqueueReady(WHICH_OTG eWhichOtg);
void UsbOtgHostIsocOutDataDequeueReady(WHICH_OTG eWhichOtg);
void UsbOtgHostIsocInDataDequeueReady(WHICH_OTG eWhichOtg);
DWORD GetIsocEndPointMaxPacketSize(DWORD dwMaxPacketSize);
void UsbOtgHostStopAllItd(WHICH_OTG eWhichOtg);
void UsbOtgHostItdFree(WHICH_OTG eWhichOtg);
void UsbOtgHostPrepareFrameListForIsocEp (BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg);

#endif // (USBOTG_HOST_ISOC == ENABLE)


//////////////////////////////////////////////////////////////////////////////

/*********************************************************
// Host control 
///
///@defgroup    USBH_CTRL Polling and ISR switcher 
///@ingroup     USB_Host
///The API functions are for ISR and polling switch. That is for MSDC pen-drive read only.
///

///
///@ingroup USBH_CTRL
///@brief   Switch to polling IOC bit to confirm transaction done.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///@remark  This API function should be called after USB enumeration done.
///
void SetToPollingIocMode (WHICH_OTG eWhichOtg);
///
///@ingroup USBH_CTRL
///@brief   Switch to wait ISR to confirm transaction done.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///@remark  This API function should be called after USB enumeration done.
///
void SetToIsrIocMode (WHICH_OTG eWhichOtg);
*********************************************************/

//#endif  //SC_USBHOST


#endif //__USB_OTG_HOST_H__


