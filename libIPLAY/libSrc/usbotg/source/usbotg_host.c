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
* Filename		: usbotg_host.c
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)
* Created Date	: 2007/07/31 
* Description:  This program is for USB Host lib function call
******************************************************************************** 
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#define TWICE_READ_COMPARE 0
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "usbotg_api.h"
#include "usbotg_device.h"
#include "usbotg_ctrl.h"
#include "usbotg_host_sm.h"
#include "usbotg_host_setup.h"
#include "taskid.h"
#include "os.h"
#include "devio.h"
#include "ui.h"
#if (SC_USBHOST==ENABLE)
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
#include "usbotg_bt.h"
#endif

#if (NETWARE_ENABLE == ENABLE)
#include "usbotg_wifi.h"
#endif    

#if (USBOTG_WEB_CAM == ENABLE)
#include "usbotg_host_web_cam.h"
#endif

//#define mwHost20_Misc_Physuspend_Set()		                        (mwHost20Bit_Set (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
//#define mwHost20_Misc_Physuspend_Clr()		                        (mwHost20Bit_Clr (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
//#define mwOTG20_Control_DeviceDmaControllerParameterSetting1_Set()  (mwHost20Bit_Set  (gp_UsbOtg->DeviceDmaControllerParameterSetting1, BIT31))  
//#define mwOTG20_Control_DeviceDmaControllerParameterSetting1_Clr()	(mwHost20Bit_Clr  (gp_UsbOtg->DeviceDmaControllerParameterSetting1, BIT31))  
#define USBOTG_HOST_INTERRUPT_TX_TEST 0    
#define MAX_USBH_DETECT_RETRY_CNT   16
#define IS_FIRST_TIME_INIT      TRUE     
#define NOT_FIRST_TIME_INIT     FALSE


#define HOST20_USBINTR_ENABLE (HOST20_USBINTR_SystemError|HOST20_USBINTR_PortChangeDetect|HOST20_USBINTR_USBError|HOST20_USBINTR_CompletionOfTransaction|HOST20_USBINTR_FrameRollover)
//#define HOST20_USBINTR_ENABLE (HOST20_USBINTR_SystemError|HOST20_USBINTR_PortChangeDetect|HOST20_USBINTR_USBError|HOST20_USBINTR_CompletionOfTransaction)

//#define mdwOTGC_Control_A_BUS_DROP_Clr()	        (gp_UsbOtg->OtgControlStatus &= (~BIT5))      
//#define mdwOTGC_Control_A_BUS_REQ_Set()	            (gp_UsbOtg->OtgControlStatus |=   BIT4)    

//#define mwHost20Port(bOffset)			*((volatile DWORD *) ( USB_OTG_BASE + bOffset))
//#define mwHost20BitRd(bOffset,wBitNum)	((mwHost20Port(bOffset)) & wBitNum)

//#define mwHost20Bit_Rd(dwRegVal,wBitNum)                     (dwRegVal&wBitNum)
//#define mwHost20Bit_Set(dwRegVal,wBitNum)                    (dwRegVal|=wBitNum)
//#define mwHost20Bit_Clr(dwRegVal,wBitNum)                    (dwRegVal&=~wBitNum)
//<2>.0x000(Capability Register)
#define mwHost20_HCIVersion_Rd()		                  ((psUsbOtg->psUsbReg->HcCapability >> 16) & 0x0000FFFF)//((gp_UsbOtg->HcCapability >> 16) & 0x0000FFFF)//((mwHost20Port(0x00)>>16)&0x0000FFFF)	
#define mwHost20_CapLength_Rd()		                      (psUsbOtg->psUsbReg->HcCapability & 0x000000FF)//(gp_UsbOtg->HcCapability & 0x000000FF)	

//<3>.0x004(HCSPARAMS - Structural Parameters)
#define mwHost20_NumPorts_Rd()		                      (psUsbOtg->psUsbReg->HcCapabilityParameters & 0x0000000F)//(gp_UsbOtg->HcCapabilityParameters & 0x0000000F)	

//<4>.0x008(HCCPARAMS - Capability Parameters)
#define mbHost20_ProgrammableFrameListFlag_Rd()		      (mwHost20Bit_Rd(psUsbOtg->psUsbReg->HcCapabilityParameters,BIT1))//(mwHost20Bit_Rd(gp_UsbOtg->HcCapabilityParameters,BIT1)) 	//Bit 1

//<4>.0x010(USBCMD - USB Command Register)
#define mwHost20_USBCMD_IntThreshold_Rd()		          ((psUsbOtg->psUsbReg->HcUsbCommand>>16)&0x0000FFFF)//((gp_UsbOtg->HcUsbCommand>>16)&0x0000FFFF)	//Bit 16~23
#define mbHost20_USBCMD_IntThreshold_Set(bValue)		  (psUsbOtg->psUsbReg->HcUsbCommand=((psUsbOtg->psUsbReg->HcUsbCommand&0xFF00FFFF)|(((DWORD)(bValue))<<16)))//(gp_UsbOtg->HcUsbCommand=((gp_UsbOtg->HcUsbCommand&0xFF00FFFF)|(((DWORD)(bValue))<<16)))	//Bit 16~23
//----->Add  "Asynchronous schedule Park mode ENable"
//----->Add  "ASYNchronous schedule Park mode CouNT"

#define mbHost20_USBCMD_ParkMode_Rd()       	          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbCommand, BIT11)>>11)//(mwHost20Bit_Rd   (gp_UsbOtg->HcUsbCommand, BIT11)>>11) 	
#define mbHost20_USBCMD_ParkMode_Set()     	              (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbCommand, BIT11))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbCommand, BIT11))    
#define mbHost20_USBCMD_ParkMode_Clr()	                  (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbCommand, BIT11))//(mwHost20Bit_Clr  (gp_UsbOtg->HcUsbCommand, BIT11))	

#define mbHost20_USBCMD_ParkMode_CNT_Rd()       	      ((psUsbOtg->psUsbReg->HcUsbCommand>>8)&0x00000003)//((gp_UsbOtg->HcUsbCommand>>8)&0x00000003)   
//#define mbHost20_USBCMD_ParkMode_CNT_Set(bValue)     	  (psUsbOtg->psUsbReg->HcUsbCommand=(psUsbOtg->psUsbReg->HcUsbCommand&0xFFFFFCFF)|(( (DWORD) bValue )<<8)  )	//Bit 8~9

#define mbHost20_USBCMD_InterruptOnAsync_Rd()       	  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbCommand, BIT6))//(mwHost20Bit_Rd   (gp_UsbOtg->HcUsbCommand, BIT6)) 	//Bit 6
#define mbHost20_USBCMD_InterruptOnAsync_Set()     	      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbCommand, BIT6))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbCommand, BIT6))    //Bit 6
#define mbHost20_USBCMD_InterruptOnAsync_Clr()	          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbCommand, BIT6))//(mwHost20Bit_Clr  (gp_UsbOtg->HcUsbCommand, BIT6))	//Bit 6

#define mbHost20_USBCMD_AsynchronousEnable_Rd()     	  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbCommand, BIT5))//(gp_UsbOtg->HcUsbCommand, BIT5))     //Bit 5
#define mbHost20_USBCMD_AsynchronousEnable_Set()     	  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbCommand, BIT5))//(gp_UsbOtg->HcUsbCommand, BIT5))    //Bit 5
#define mbHost20_USBCMD_AsynchronousEnable_Clr()	      (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbCommand, BIT5))//(gp_UsbOtg->HcUsbCommand, BIT5))	//Bit 5

#define mbHost20_USBCMD_PeriodicEnable_Rd()     	      (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbCommand, BIT4))//(gp_UsbOtg->HcUsbCommand, BIT4))    //Bit 4
#define mbHost20_USBCMD_PeriodicEnable_Set()     	      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbCommand, BIT4))//(gp_UsbOtg->HcUsbCommand, BIT4))    //Bit 4
#define mbHost20_USBCMD_PeriodicEnable_Clr()	          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbCommand, BIT4))//(gp_UsbOtg->HcUsbCommand, BIT4))	//Bit 4

//#define mbHost20_USBCMD_FrameListSize_Rd()	              ((psUsbOtg->psUsbReg->HcUsbCommand>>2)&0x00000003)//((gp_UsbOtg->HcUsbCommand>>2)&0x00000003)	   //Bit 2~3
//#define mbHost20_USBCMD_FrameListSize_Set(bValue)     	  (psUsbOtg->psUsbReg->HcUsbCommand=((psUsbOtg->psUsbReg->HcUsbCommand&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))//(gp_UsbOtg->HcUsbCommand=((gp_UsbOtg->HcUsbCommand&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))	//Bit 2~3

#define HOST20_USBCMD_FrameListSize_1024                  0x00
#define HOST20_USBCMD_FrameListSize_512                   0x01
#define HOST20_USBCMD_FrameListSize_256                   0x02    

#define mbHost20_USBCMD_HCReset_Rd()	                  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbCommand, BIT1))//(gp_UsbOtg->HcUsbCommand, BIT1))	   //Bit 1
#define mbHost20_USBCMD_HCReset_Set()	                  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbCommand, BIT1))//(gp_UsbOtg->HcUsbCommand, BIT1))   //Bit 1

#define mbHost20_USBCMD_RunStop_Rd()	                  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbCommand, BIT0))//(gp_UsbOtg->HcUsbCommand, BIT0))  //Bit 0
#define mbHost20_USBCMD_RunStop_Set()	                  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbCommand, BIT0))//(gp_UsbOtg->HcUsbCommand, BIT0))  //Bit 0
#define mbHost20_USBCMD_RunStop_Clr()	                  (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbCommand, BIT0))//(gp_UsbOtg->HcUsbCommand, BIT0))  //Bit 0

//<5>.0x014(USBSTS - USB Status Register)
//#define mwHost20_USBSTS_Rd()		                      (gp_UsbOtg->HcUsbStatus) 	
//#define mwHost20_USBSTS_Set(wValue)		                  (gp_UsbOtg->HcUsbStatus=wValue) 	

#define mwHost20_USBSTS_AsynchronousStatus_Rd()		      (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT15))//(gp_UsbOtg->HcUsbStatus, BIT15)) 	//Bit 15

#define mwHost20_USBSTS_PeriodicStatus_Rd()		          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT14))//(gp_UsbOtg->HcUsbStatus, BIT14)) 	//Bit 14

#define mwHost20_USBSTS_Reclamation_Rd()		          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT13))//(gp_UsbOtg->HcUsbStatus, BIT13)) 	//Bit 13

#define mwHost20_USBSTS_HCHalted_Rd()		              (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT12))//(gp_UsbOtg->HcUsbStatus, BIT12)) 	//Bit 12

#define mwHost20_USBSTS_IntOnAsyncAdvance_Rd()		      (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT5))//(gp_UsbOtg->HcUsbStatus, BIT5)) 	//Bit 5
#define mwHost20_USBSTS_IntOnAsyncAdvance_Set()		      (mwHost20_USBSTS_Set(BIT5))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbStatus, BIT5)) 	//Bit 5

#define mwHost20_USBSTS_SystemError_Rd()		          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT4))//(gp_UsbOtg->HcUsbStatus, BIT4))	//Bit 4
#define mwHost20_USBSTS_SystemError_Set()		          (mwHost20_USBSTS_Set(BIT4))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbStatus, BIT4)) 	//Bit 4

#define mwHost20_USBSTS_FrameRollover_Rd()		          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT3))//(gp_UsbOtg->HcUsbStatus, BIT3)) 	//Bit 3
#define mwHost20_USBSTS_FrameRollover_Set()		          (mwHost20_USBSTS_Set(BIT3))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbStatus, BIT3)) 	//Bit 3

#define mwHost20_USBSTS_PortChangeDetect_Rd()		      (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT2))//(gp_UsbOtg->HcUsbStatus, BIT2)) 	//Bit 2
#define mwHost20_USBSTS_PortChangeDetect_Set()		      (mwHost20_USBSTS_Set(BIT2))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbStatus, BIT2)) 	//Bit 2

#define mwHost20_USBSTS_USBError_Rd()		              (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT1))//(gp_UsbOtg->HcUsbStatus, BIT1)) 	//Bit 1
#define mwHost20_USBSTS_USBError_Set()		              (mwHost20_USBSTS_Set(BIT1))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbStatus, BIT1)) 	//Bit 1

#define mwHost20_USBSTS_CompletionOfTransaction_Rd()	  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbStatus, BIT0))//(gp_UsbOtg->HcUsbStatus, BIT0)) 	//Bit 0
#define mwHost20_USBSTS_CompletionOfTransaction_Set()	  (mwHost20_USBSTS_Set(BIT0))//(mwHost20Bit_Set  (gp_UsbOtg->HcUsbStatus, BIT0)) 	//Bit 0

//<6>.0x018(USBINTR - USB Interrupt Enable Register)
//#define mwHost20_USBINTR_Rd()		                      (gp_UsbOtg->HcUsbInterruptEnable) 	
//#define mwHost20_USBINTR_Set(bValue)		              (gp_UsbOtg->HcUsbInterruptEnable=bValue)

#define mwHost20_USBINTR_IntOnAsyncAdvance_Rd()		      (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT5))//(gp_UsbOtg->HcUsbInterruptEnable, BIT5))	//Bit 5
#define mwHost20_USBINTR_IntOnAsyncAdvance_Set()		  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT5))//(gp_UsbOtg->HcUsbInterruptEnable, BIT5)) 	//Bit 5
#define mwHost20_USBINTR_IntOnAsyncAdvance_Clr()		  (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT5))//(gp_UsbOtg->HcUsbInterruptEnable, BIT5)) 	//Bit 5

#define mwHost20_USBINTR_SystemError_Rd()		          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT4))//(gp_UsbOtg->HcUsbInterruptEnable, BIT4)) 	//Bit 4
#define mwHost20_USBINTR_SystemError_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT4))//(gp_UsbOtg->HcUsbInterruptEnable, BIT4)) 	//Bit 4
#define mwHost20_USBINTR_SystemError_Clr()		          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT4))//(gp_UsbOtg->HcUsbInterruptEnable, BIT4)) 	//Bit 4

#define mwHost20_USBINTR_FrameRollover_Rd()		          (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT3))//(gp_UsbOtg->HcUsbInterruptEnable, BIT3))	//Bit 3
#define mwHost20_USBINTR_FrameRollover_Set()		      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT3))//(gp_UsbOtg->HcUsbInterruptEnable, BIT3)) 	//Bit 3
#define mwHost20_USBINTR_FrameRollover_Clr()		      (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT3))//(gp_UsbOtg->HcUsbInterruptEnable, BIT3)) 	//Bit 3

#define mwHost20_USBINTR_PortChangeDetect_Rd()		      (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT2))//(gp_UsbOtg->HcUsbInterruptEnable, BIT2))	//Bit 2
#define mwHost20_USBINTR_PortChangeDetect_Set()		      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT2))//(gp_UsbOtg->HcUsbInterruptEnable, BIT2)) 	//Bit 2
#define mwHost20_USBINTR_PortChangeDetect_Clr()		      (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT2))//(gp_UsbOtg->HcUsbInterruptEnable, BIT2)) 	//Bit 2

#define mwHost20_USBINTR_USBError_Rd()		              (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT1))//(gp_UsbOtg->HcUsbInterruptEnable, BIT1))	//Bit 1
#define mwHost20_USBINTR_USBError_Set()		              (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT1))//(gp_UsbOtg->HcUsbInterruptEnable, BIT1)) 	//Bit 1
#define mwHost20_USBINTR_USBError_Clr()		              (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT1))//(gp_UsbOtg->HcUsbInterruptEnable, BIT1)) 	//Bit 1

#define mwHost20_USBINTR_CompletionOfTransaction_Rd()	  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT0))//(gp_UsbOtg->HcUsbInterruptEnable, BIT0))	//Bit 0
#define mwHost20_USBINTR_CompletionOfTransaction_Set()	  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT0))//(gp_UsbOtg->HcUsbInterruptEnable, BIT0)) 	//Bit 0		
#define mwHost20_USBINTR_CompletionOfTransaction_Clr()	  (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->HcUsbInterruptEnable, BIT0))//(gp_UsbOtg->HcUsbInterruptEnable, BIT0)) 	//Bit 0		

#define HOST20_USBINTR_IntOnAsyncAdvance                  0x20
#define HOST20_USBINTR_SystemError                        0x10
#define HOST20_USBINTR_FrameRollover                      0x08
#define HOST20_USBINTR_PortChangeDetect                   0x04
#define HOST20_USBINTR_USBError                           0x02
#define HOST20_USBINTR_CompletionOfTransaction            0x01

//<7>.0x01C(FRINDEX - Frame Index Register (Address = 01Ch))
//#define mwHost20_FrameIndex_Rd()		                  (gp_UsbOtg->HcFrameIndex&0x00001FFF) 	//Only Read Bit0~Bit12(Skip Bit 13)     	
#define mwHost20_FrameIndex14Bit_Rd()                     (psUsbOtg->psUsbReg->HcFrameIndex&0x00003FFF)//(gp_UsbOtg->HcFrameIndex&0x00003FFF) 	//Only Read Bit0~Bit12(Skip Bit 13)     	
#define mwHost20_FrameIndex_Set(wValue)		              (psUsbOtg->psUsbReg->HcFrameIndex=wValue)//(gp_UsbOtg->HcFrameIndex=wValue) 	

//<8>.0x024(PERIODICLISTBASE - Periodic Frame List Base Address Register (Address = 024h))
//#define mwHost20_PeriodicBaseAddr_Rd()		              (psUsbOtg->psUsbReg->HcPeriodicFrameListBaseAddress)//(gp_UsbOtg->HcPeriodicFrameListBaseAddress) 	     	
//#define mwHost20_PeriodicBaseAddr_Set(wValue)		      (psUsbOtg->psUsbReg->HcPeriodicFrameListBaseAddress=wValue)//(gp_UsbOtg->HcPeriodicFrameListBaseAddress=wValue) 	

//<9>.0x028(ASYNCLISTADDR - Current Asynchronous List Address Register (Address = 028h))
//#define mwHost20_CurrentAsynchronousAddr_Rd()		      (psUsbOtg->psUsbReg->HcCurrentAsynListAddress)//(gp_UsbOtg->HcCurrentAsynListAddress)	     	
//#define mwHost20_CurrentAsynchronousAddr_Set(wValue)	  (gp_UsbOtg->HcCurrentAsynListAddress=wValue) 	

//<10>.0x030(PORTSC - Port Status and Control Register(Address = 030h))
#define mwHost20_PORTSC_Rd()		                      (psUsbOtg->psUsbReg->HcPortStatusAndControl)//(gp_UsbOtg->HcPortStatusAndControl)

#define mwHost20_PORTSC_LineStatus_Rd()		              ((psUsbOtg->psUsbReg->HcPortStatusAndControl>>10)&0x00000003)//((gp_UsbOtg->HcPortStatusAndControl>>10)&0x00000003) 	     	


#define mwHost20_PORTSC_PortReset_Rd()		              mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT8)//(gp_UsbOtg->HcPortStatusAndControl, BIT8) 	     	
//#define mwHost20_PORTSC_PortReset_Set()		              mwHost20Bit_Set   (gp_UsbOtg->HcPortStatusAndControl, BIT8) 	     	
//#define mwHost20_PORTSC_PortReset_Clr()		              mwHost20Bit_Clr   (gp_UsbOtg->HcPortStatusAndControl, BIT8) 	     	

#define mwHost20_PORTSC_ForceSuspend_Rd()		          mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT7)//(gp_UsbOtg->HcPortStatusAndControl, BIT7) 	     	
//#define mwHost20_PORTSC_ForceSuspend_Set()		          mwHost20Bit_Set   (gp_UsbOtg->HcPortStatusAndControl, BIT7) 	     	

#define mwHost20_PORTSC_ForceResume_Rd()		          mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT6)//(gp_UsbOtg->HcPortStatusAndControl, BIT6) 	     	
//#define mwHost20_PORTSC_ForceResume_Set()		          mwHost20Bit_Set   (gp_UsbOtg->HcPortStatusAndControl, BIT6) 	     	
#define mwHost20_PORTSC_ForceResume_Clr()		          mwHost20Bit_Clr   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT6)//(gp_UsbOtg->HcPortStatusAndControl, BIT6) 	     	

#define mwHost20_PORTSC_EnableDisableChange_Rd()		  mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT3)//(gp_UsbOtg->HcPortStatusAndControl, BIT3) 	     	
#define mwHost20_PORTSC_EnableDisableChange_Set()		  mwHost20Bit_Set   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT3)//(gp_UsbOtg->HcPortStatusAndControl, BIT3) 	     	

#define mwHost20_PORTSC_EnableDisable_Rd()		          mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT2)//(gp_UsbOtg->HcPortStatusAndControl, BIT2) 	     	
//#define mwHost20_PORTSC_EnableDisable_Set()		          mwHost20Bit_Set   (gp_UsbOtg->HcPortStatusAndControl, BIT2) 	     	
#define mwHost20_PORTSC_EnableDisable_Clr()		          mwHost20Bit_Clr   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT2)//(gp_UsbOtg->HcPortStatusAndControl, BIT2) 	

#define mwHost20_PORTSC_ConnectChange_Rd()		          mwHost20Bit_Rd    (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT1)//(gp_UsbOtg->HcPortStatusAndControl, BIT1) 	
#define mwHost20_PORTSC_ConnectChange_Set()		          mwHost20Bit_Set   (psUsbOtg->psUsbReg->HcPortStatusAndControl, BIT1)//(gp_UsbOtg->HcPortStatusAndControl, BIT1) 

//#define mwHost20_PORTSC_ConnectStatus_Rd()		          mwHost20Bit_Rd    (gp_UsbOtg->HcPortStatusAndControl, BIT0) 	


#define mwHost20_Misc_LineStatus_Rd()		              ((psUsbOtg->psUsbReg->HcPortStatusAndControl>>10)&0x00000003)//((gp_UsbOtg->HcPortStatusAndControl>>10)&0x00000003) 	     	


//<10>.0x040(Misc. Register(Address = 040h))
//#define mwHost20_Misc_EOF1Time_Set(bValue)		          (gp_UsbOtg->HcMisc=((gp_UsbOtg->HcMisc&0xFFFFFFF3)|(((DWORD)(bValue))<<2)))	//Bit 2~3
#define mwHost20_Misc_Physuspend_Rd()		              (mwHost20Bit_Rd  (psUsbOtg->psUsbReg->HcMisc, BIT6))//(gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
//#define mwHost20_Misc_Physuspend_Set()		              (mwHost20Bit_Set (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
//#define mwHost20_Misc_Physuspend_Clr()		              (mwHost20Bit_Clr (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3

//<11>.0x080(OTG Control / Status Register (Address = 080h))
//#define mwOTG20_Control_HOST_SPD_TYP_Rd()		          ((gp_UsbOtg->OtgControlStatus>>22)&0x00000003) 
#define mwOTG20_Control_A_BUS_REQ_Set()		              (mwHost20Bit_Set  (psUsbOtg->psUsbReg->OtgControlStatus, BIT4))//(gp_UsbOtg->OtgControlStatus, BIT4))  

#define mwHost20_Control_ForceFullSpeed_Rd()		      (psUsbOtg->psUsbReg->OtgControlStatus& BIT12)//(gp_UsbOtg->OtgControlStatus& BIT12)
#define mwHost20_Control_ForceFullSpeed_Set()	          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->OtgControlStatus, BIT12))//(gp_UsbOtg->OtgControlStatus, BIT12))  
#define mwHost20_Control_ForceFullSpeed_Clr()	          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->OtgControlStatus, BIT12))//(gp_UsbOtg->OtgControlStatus, BIT12))

#define mwHost20_Control_ForceHighSpeed_Rd()		      (psUsbOtg->psUsbReg->OtgControlStatus& BIT14)//(gp_UsbOtg->OtgControlStatus& BIT14)
#define mwHost20_Control_ForceHighSpeed_Set()	          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->OtgControlStatus, BIT14))//(gp_UsbOtg->OtgControlStatus, BIT14))  
#define mwHost20_Control_ForceHighSpeed_Clr()	          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->OtgControlStatus, BIT14))//(gp_UsbOtg->OtgControlStatus, BIT14)) 

#define mwOTG20_Control_Phy_Reset_Set()		              (mwHost20Bit_Set  (psUsbOtg->psUsbReg->OtgControlStatus, BIT15))//(gp_UsbOtg->OtgControlStatus, BIT15))  
#define mwOTG20_Control_Phy_Reset_Clr()		              (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->OtgControlStatus, BIT15))//(gp_UsbOtg->OtgControlStatus, BIT15))  
#define mwOTG20_Control_OTG_Reset_Set()		              (mwHost20Bit_Set  (psUsbOtg->psUsbReg->OtgControlStatus, BIT24))//(gp_UsbOtg->OtgControlStatus, BIT24))  
#define mwOTG20_Control_OTG_Reset_Clr()		              (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->OtgControlStatus, BIT24))//(gp_UsbOtg->OtgControlStatus, BIT24))  

//#define mwOTG20_Control_DeviceDmaControllerParameterSetting1_Set()		              (mwHost20Bit_Set  (gp_UsbOtg->DeviceDmaControllerParameterSetting1, BIT31))  
//#define mwOTG20_Control_DeviceDmaControllerParameterSetting1_Clr()		              (mwHost20Bit_Clr  (gp_UsbOtg->DeviceDmaControllerParameterSetting1, BIT31))  

//~A Function 
//#define mdwOTGC_Control_A_BUS_REQ_Rd()	            (mwHost20Bit_Rd   (gp_UsbOtg->OtgControlStatus, BIT4))	
//#define mdwOTGC_Control_A_BUS_REQ_Set()	            (mwHost20Bit_Set  (gp_UsbOtg->OtgControlStatus, BIT4))    
//#define mdwOTGC_Control_A_BUS_REQ_Clr()	            (mwHost20Bit_Clr  (gp_UsbOtg->OtgControlStatus, BIT4))        

//#define mdwOTGC_Control_A_BUS_DROP_Rd()	            (mwHost20Bit_Rd   (gp_UsbOtg->OtgControlStatus, BIT5))	
//#define mdwOTGC_Control_A_BUS_DROP_Set()	        (mwHost20Bit_Set  (gp_UsbOtg->OtgControlStatus, BIT5))      
//#define mdwOTGC_Control_A_BUS_DROP_Clr()	        (mwHost20Bit_Clr  (gp_UsbOtg->OtgControlStatus, BIT5))      


//<12>.0x0C4(Interrupt Mask)
#define mwOTG20_Interrupt_Mask_Rd()		                  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt&0x00000007)//(gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt&0x00000007) 
#define mwOTG20_Interrupt_Mask_Set(bValue)		          (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt=bValue)//(gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt=bValue)
#define mwOTG20_Interrupt_Mask_Device_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT0))//(gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT0)) 
#define mwOTG20_Interrupt_Mask_HOST_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT2))//(gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT2))
#define mwOTG20_Interrupt_OutPut_High_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT3))//(gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT3))
#define mwOTG20_Interrupt_OutPut_High_Clr()		          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT3))//(gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT3))

//<13>.0x100(Device Controller Registers(Address = 100h~1FFh) )
#define mwPeri20_Control_ChipEnable_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceMainControl, BIT5))//(gp_UsbOtg->DeviceMainControl, BIT5)) 	
#define mwPeri20_Control_HALFSPEEDEnable_Set()		      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceMainControl, BIT1))//(gp_UsbOtg->DeviceMainControl, BIT1)) 	
#define mwPeri20_Control_HALFSPEEDEnable_Clr()		      (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->DeviceMainControl, BIT1))//(gp_UsbOtg->DeviceMainControl, BIT1)) 
#define mwPeri20_Control_SoftwareReset_Set()		      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceMainControl, BIT4))//(gp_UsbOtg->DeviceMainControl, BIT4)) 
#define mwPeri20_Control_SoftwareReset_Rd()	              (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->DeviceMainControl, BIT4))//(gp_UsbOtg->DeviceMainControl, BIT4))

#define mwPeri20_Control_Fifo_0_Set() (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceFifo0InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo0InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_1_Set() (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceFifo1InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo1InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_2_Set() (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceFifo2InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo2InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_3_Set() (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceFifo3InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo3InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_0_Rd()  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->DeviceFifo0InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo0InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_1_Rd()  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->DeviceFifo1InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo1InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_2_Rd()  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->DeviceFifo2InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo2InstructionAndByteCount, BIT12)) 
#define mwPeri20_Control_Fifo_3_Rd()  (mwHost20Bit_Rd   (psUsbOtg->psUsbReg->DeviceFifo3InstructionAndByteCount, BIT12))//(gp_UsbOtg->DeviceFifo3InstructionAndByteCount, BIT12)) 



/////////////////////////////////////////////////////////////////////////////////////////////////////////

#if USBOTG_HOST_IOC_POLIING_SWITCH
#define HOST20_USBINTR_ENABLE_M (HOST20_USBINTR_SystemError|HOST20_USBINTR_PortChangeDetect|HOST20_USBINTR_USBError|HOST20_USBINTR_FrameRollover)
static void EnableUsbOtgHostInterruptMsdc(WHICH_OTG eWhichOtg);
static void SetToPollingIocMode (WHICH_OTG eWhichOtg);
static void SetToIsrIocMode (WHICH_OTG eWhichOtg);
static BOOL UseIocIsr4MsdcTx(WHICH_OTG eWhichOtg);
#endif
static void SetUsbhCardPresentFlag(BYTE val, BYTE bLun, WHICH_OTG eWhichOtg);
static int getFreeBufIndex(unsigned long *flag);
static void free_all_mem(WHICH_OTG eWhichOtg);
static void UsbOtgHostClassTask(void);
static void UsbOtgHostDriverTask(void);
static SDWORD UsbOtgHostDriverTaskWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent, BYTE bEventId);
static void UsbOtgHostEventPlugIn(WHICH_OTG eWhichOtg);
static void UsbOtgHostEventPollingEachLun(WHICH_OTG eWhichOtg);
static void UsbOtgHostEventIoc (WHICH_OTG eWhichOtg);
static void UsbOtgHostSetupCmdTimeOut(void *arg);
static void UsbOtgHostEventIocSetup (WHICH_OTG eWhichOtg);
static void UsbOtgHostEventActiveSetup (WHICH_OTG eWhichOtg);
static void UsbOtgHostEventActiveBulk (WHICH_OTG eWhichOtg);
static void UsbOtgHostEventIocBulk (WHICH_OTG eWhichOtg);
static void UsbOtgHostEventActiveInterrupt(WHICH_OTG eWhichOtg);
static void UsbOtgHostEventIocInterrupt(WHICH_OTG eWhichOtg);
static void UsbhStorageUninit (ST_MCARD_DEV *psDev);
static void UsbOtgHostSetSwapBuffer1RangeDisable(WHICH_OTG eWhichOtg);
static void UsbOtgMemInit(WHICH_OTG eWhichOtg);
static DWORD UsbOtgHostGetSidcRxBufferAddress(WHICH_OTG eWhichOtg);
static void UsbHostInit(WHICH_OTG eWhichOtg);
static void OTGC_A_Bus_Drive(PST_USB_OTG_DES psUsbOtg);
static void OTGC_A_Bus_Drop(PST_USB_OTG_DES psUsbOtg);
static void EnableUsbOtgHostInterrupt(WHICH_OTG eWhichOtg);
static void CommandProcess (void *pMcardDev);
static void flib_Host20_ForceSpeed(BYTE bSpeedType, PST_USB_OTG_DES psUsbOtg);
static void UsbOtgHostGetQhdForInterruptEd(WHICH_OTG eWhichOtg);
static void flib_Host20_Send_qTD_Active (  qTD_Structure *spHeadqTD,
                                    qHD_Structure *spTempqHD,
                                    WHICH_OTG eWhichOtg);
void flib_Host20_CheckingForResult_QHD(qHD_Structure *spTempqHD, WHICH_OTG eWhichOtg);
static void flib_Host20_InitStructure(WHICH_OTG eWhichOtg);
static void flib_Host20_Allocate_QHD ( qHD_Structure   *psQHTemp,
                                BYTE            bNextType,
                                BYTE            bAddress,
                                BYTE            bHead,
                                BYTE            bEndPt, 
                                DWORD           wMaxPacketSize,
                                WHICH_OTG       eWhichOtg);
static void UsbOtgHostHandlePlugOutAction(DWORD dwEvent, WHICH_OTG eWhichOtg);
static SDWORD SendMailTrackToUsbOtgHostClassTask(DWORD dwEnvelopeAddr, WHICH_OTG eWhichOtg);
void NetUsb_EventIoc(WHICH_OTG eWhichOtg, int max_endpoints);
static void TwiceReadCompare(ST_MCARD_MAIL   *psMcardRMail, BYTE bLun, WHICH_OTG eWhichOtg);

#if (USBOTG_HOST_ISOC == ENABLE)
static SDWORD UsbOtgHostIsocWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent);
static void UsbOtgHostIsocTask(void);
static void UsbOtgHostGetItdForIsocEd(WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocInActive (WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocInIoc (WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocOutActive (WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocOutIoc (WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocInSetHead(WORD wHead, WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocInSetTail(WORD wTail, WHICH_OTG eWhichOtg);
static WORD UsbOtgHostIsocInGetHead(WHICH_OTG eWhichOtg);
static WORD UsbOtgHostIsocInGetTail(WHICH_OTG eWhichOtg);
static PUSBH_ISOC_BUFFER UsbOtgHostIsocInGetQueueElement(WORD index, WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocOutSetHead(WORD wHead, WHICH_OTG eWhichOtg);
static void UsbOtgHostIsocOutSetTail(WORD wTail, WHICH_OTG eWhichOtg);
static WORD UsbOtgHostIsocOutGetHead(WHICH_OTG eWhichOtg);
static WORD UsbOtgHostIsocOutGetTail(WHICH_OTG eWhichOtg);
//static void UsbOtgHostIsocOutSetQueueElement(BYTE *pData, DWORD dwLength, BYTE tail, WHICH_OTG eWhichOtg);
static PUSBH_ISOC_BUFFER UsbOtgHostIsocOutGetQueueElement(WORD index, WHICH_OTG eWhichOtg);
static void flib_Host20_Issue_Iso (DWORD       wEndPt,
                            DWORD       wMaxPacketSize,
                            DWORD       wSize,
                            DWORD       *pwBufferArray,
                            DWORD       wOffset,
                            BYTE        bDirection,
                            BYTE        bMult, 
                            WHICH_OTG   eWhichOtg);
static void OTGH_PT_ISO(DWORD wEndPt, BYTE bDirection, DWORD wSize, DWORD wMaxPacketSize, WHICH_OTG eWhichOtg);
#endif //#if (USBOTG_HOST_ISOC == ENABLE) 
/////////////////////////////////////////////////////////////////////////////


#if USBOTG_HOST_IOC_POLIING_SWITCH
static void EnableUsbOtgHostInterruptMsdc(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    mwHost20_USBINTR_Set(HOST20_USBINTR_ENABLE_M);
    SystemIntEna(UsbOtgIntCauseGet(eWhichOtg));    
}

static void SetToPollingIocMode (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost;

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    psHost->boIocPolling = 1;
    //mpDebugPrint("SetToPollingIocgMode");
    SetNoNeedToPollingFlag(eWhichOtg); // not polling LUN
    DisableUsbOtgHostInterrupt(eWhichOtg);
    EnableUsbOtgHostInterruptMsdc(eWhichOtg);
}

static void SetToIsrIocMode (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost;

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    psHost->boIocPolling = 0; //  g_ioc_polling = 0; 
    mpDebugPrint("SetToIsrIocMode");
    ClearNoNeedToPollingFlag(eWhichOtg); // need polling LUN
    DisableUsbOtgHostInterrupt(eWhichOtg);
    EnableUsbOtgHostInterrupt(eWhichOtg);
}

static BOOL UseIocIsr4MsdcTx(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost;

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    return !psHost->boIocPolling;
    /*
    if (HOST20_USBINTR_USBError == (mwHost20_USBSTS_Rd() & HOST20_USBINTR_USBError))
    {
        return (mwHost20_USBSTS_Rd() & HOST20_USBINTR_CompletionOfTransaction);
    }
    else
    {
        return 1;
    }
    */
}
#endif
void SetUsbhReadyToReadWrite (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    psHost->boIsReadyToReadWriteUsbh = TRUE;
}

BOOL CheckIfUsbhReadyToReadWrite (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost;

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    return psHost->boIsReadyToReadWriteUsbh;
}

DWORD GetUsbhCardPresentFlag(BYTE bLun, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR pAppDes;
    
    pAppDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    return pAppDes->dPresent[bLun];
}

static void SetUsbhCardPresentFlag(BYTE val, BYTE bLun, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR pAppDes;
    
    pAppDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    pAppDes->dPresent[bLun] = val;
}

void SetHostToResetDeviceFlag(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psUsbOtgHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);

    psUsbOtgHost->boIsHostToResetDevice = TRUE;
}

void SetHostFinalizedFlag(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psUsbOtgHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);

    psUsbOtgHost->boIsHostFinalized = TRUE;
}

void SetCheckMediaPresentTimerFlag(BOOL flag, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    UsbOtgCmpTimeSartFlagSet (flag, eWhichOtg); //gIsCmpTimerStart = val;    
    if (flag == CMP_START_TIMER)
    {
        if (mwHost20_USBINTR_FrameRollover_Rd() == 0)
            mwHost20_USBINTR_FrameRollover_Set();
    }
    else if (flag == CMP_STOP_TIMER)
    {
        if (mwHost20_USBINTR_FrameRollover_Rd() != 0)
            mwHost20_USBINTR_FrameRollover_Clr();
    }
}

BOOL GetCheckMediaPresentTimerFlag(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_MSDC psMsdc;
    psMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    return psMsdc->boIsCmpTimerStart;//gIsCmpTimerStart;
}

DWORD GetRetryCount(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    return psHost->dwRetryCnt;
}

void ClearRetryCount(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    psHost->dwRetryCnt = 0;
}


#define MEM_BLOCK_1024          1024
#define MEM_BLOCK_512           512
#define HOW_MANY_MEM_BLOCK      16
#define SIZE_OF_UNIT            1 // one byte

static int getFreeBufIndex(unsigned long *flag)
{
	int index;
	for (index = 0; index < HOW_MANY_MEM_BLOCK; ++index)
		if ((*flag & (1 << index)) == 0) {
			*flag |= (1 << index);
			return index;
		}
	return -1;
}

static char tmpBuf[2048];
void *allo_mem(DWORD cnt, WHICH_OTG eWhichOtg)
{
        int  index;
        void *rtn = NULL;
        //DWORD *dwpt=0;//(DWORD*)0x80000000;  // unused
        //DWORD fillme = 0;  // unused
        PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

	if (cnt <= MEM_BLOCK_512) {
		index = getFreeBufIndex(&psEhci->sMem512Bytes.flag);
		if (index != -1) {
			rtn = &psEhci->sMem512Bytes.pBuf[index * MEM_BLOCK_512];
			memset((BYTE*)rtn, 0, MEM_BLOCK_512);
		}
		else {
			MP_ASSERT(0);
            MP_DEBUG("alloc mem512 error"); 
            //__asm("break 100");
		}
	}
	else if (cnt <= MEM_BLOCK_1024) {
		index = getFreeBufIndex(&psEhci->sMem1024Bytes.flag);
		if (index != -1) {
			rtn = &psEhci->sMem1024Bytes.pBuf[index * MEM_BLOCK_1024];
			memset((BYTE*)rtn, 0, MEM_BLOCK_1024);
		}
		else MP_DEBUG("alloc mem1024 error"); 
	}
	else if (cnt <= 2048) {
/*
		index = getFreeBufIndex(&mem2048->flag);
		if (index != -1) {
			rtn = &mem2048->buf[index * 2048];
			memset((BYTE*)rtn, 0, 2048);
            dwpt = (DWORD*)((DWORD)rtn + 0x100000);
            fillme=*(dwpt);
		}
		else MP_DEBUG("alloc mem2048 error"); 
*/
		return (DWORD)tmpBuf|0xa0000000;
	}
	else MP_ALERT("alloc undefined size buf 0x%x", cnt);

	return rtn;
}

DWORD free1(void *buf, WHICH_OTG eWhichOtg)
{
	DWORD  index;
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

	if (((DWORD) buf) == UsbOtgHostGetSidcRxBufferAddress(eWhichOtg))
		return;

	for (index = 0; index < HOW_MANY_MEM_BLOCK; ++index)
	{
		if ((DWORD) buf == (DWORD) &psEhci->sMem512Bytes.pBuf[index * MEM_BLOCK_512])
		{
			psEhci->sMem512Bytes.flag &= ~(1 << index);
			return TRUE;
		}
	}

	for (index = 0; index < HOW_MANY_MEM_BLOCK; ++index)
	{
		if ((DWORD) buf == (DWORD) &psEhci->sMem1024Bytes.pBuf[index * MEM_BLOCK_1024])
		{
			psEhci->sMem1024Bytes.flag &= ~(1 << index);
			return TRUE;
		}
	}
/*
	for (index = 0; index < 16; ++index)
	{
		if ((unsigned long) buf == (unsigned long) &mem2048->buf[index * 2048])
		{
			mem2048->flag &= ~(1 << index);
			return TRUE;
		}
	}*/
	MP_DEBUG("free buf error");
	return FALSE;
}

static void free_all_mem(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    psEhci->sMem512Bytes.flag = 0;
    psEhci->sMem1024Bytes.flag = 0;
}


WHICH_OTG GetUsbhPortSupportIsoc(void)
{
    return WEBCAM_USB_PORT; // BT_USB_PORT
}
    


SDWORD UsbOtgHostTaskInit(WHICH_OTG eWhichOtg)
{
    SDWORD sdwRetVal = USBOTG_NO_ERROR;
    //static BYTE is_already_init = 0;
    PST_USB_OTG_DES psUsbOtg;
    BYTE bTaskId;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    if (psUsbOtg->sUsbHost.boIsAlreadyInit == TRUE)
    {
        MP_DEBUG("-USBOTG%d- %s already done", eWhichOtg, __FUNCTION__);
        return sdwRetVal;
    }
    else
    {
        psUsbOtg->sUsbHost.boIsAlreadyInit = TRUE;
    }

    MP_DEBUG("-USBOTG%d- %s Begin", eWhichOtg, __FUNCTION__);

    sdwRetVal = MailboxCreate(UsbOtgHostMailBoxIdGet(eWhichOtg), OS_ATTR_PRIORITY);
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST mail box Id %d create fail %d", eWhichOtg, __FUNCTION__, UsbOtgHostMailBoxIdGet(eWhichOtg), sdwRetVal);
        return USBOTG_MAILBOX_CREATE_ERROR;
    }
    
    bTaskId = UsbOtgHostClassTaskIdGet(eWhichOtg);
    sdwRetVal = TaskCreate(bTaskId, UsbOtgHostClassTask, DRIVER_PRIORITY, 0x1000);
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST ClassTask Id %d create fail %d", eWhichOtg, __FUNCTION__, bTaskId, sdwRetVal);
        return USBOTG_TASK_CREATE_ERROR;
    }
    
    sdwRetVal = TaskStartup(bTaskId, eWhichOtg);
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST ClassTask Id %d Startup fail %d", eWhichOtg, __FUNCTION__, bTaskId, sdwRetVal);
        return USBOTG_TASK_STARTUP_ERROR;
    }

    sdwRetVal = EventCreate(UsbOtgHostDriverEventIdGet(eWhichOtg), (OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR), 0);
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST driver event create Id %d fail %d", eWhichOtg, __FUNCTION__, UsbOtgHostDriverEventIdGet(eWhichOtg), sdwRetVal);
        return USBOTG_EVENT_CREATE_ERROR;
    }
    
    bTaskId = UsbOtgHostDriverTaskIdGet (eWhichOtg);
    sdwRetVal = TaskCreate(bTaskId, UsbOtgHostDriverTask, DRIVER_PRIORITY+1, 0x1800); // 0x1000 -> 0x1800
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST DriverTask Id %d create fail %d", eWhichOtg, __FUNCTION__, bTaskId, sdwRetVal);
        return USBOTG_TASK_CREATE_ERROR;
    }
    
    sdwRetVal = TaskStartup(bTaskId, eWhichOtg);
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST DriverTask Id %d startup fail %d", eWhichOtg, __FUNCTION__, bTaskId, sdwRetVal);
        return USBOTG_TASK_STARTUP_ERROR;
    }

    sdwRetVal = EventCreate(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), (OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR), 0);
    if (sdwRetVal != USBOTG_NO_ERROR)
    {
        MP_ALERT("-USBOTG%d- %s OTGHOST read mass data EventCreate Id %d fail %d", eWhichOtg, __FUNCTION__, UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), sdwRetVal);
        return USBOTG_EVENT_CREATE_ERROR;
    }

#if USBOTG_HOST_ISOC
    WHICH_OTG eSpecificUsbhPort;
    eSpecificUsbhPort = GetUsbhPortSupportIsoc();
    if (eSpecificUsbhPort == eWhichOtg)
    {
        sdwRetVal = EventCreate(USBOTG_HOST_ISOC_EVENT, (OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR), 0);
        if (sdwRetVal != USBOTG_NO_ERROR)
        {
            MP_ALERT("-USBOTG%d- %s OTGHOST ISOC event create Id %d fail %d", eWhichOtg, __FUNCTION__, USBOTG_HOST_ISOC_EVENT, sdwRetVal);
            return sdwRetVal;
        }
        
        sdwRetVal = TaskCreate(USBOTG_HOST_ISOC_TASK, UsbOtgHostIsocTask, CONTROL_PRIORITY+1, 0x1400);    //0x1000 ->0x1400
        if (sdwRetVal != USBOTG_NO_ERROR)
        {
            MP_ALERT("-USBOTG%d- %s OTGHOST ISOC Task Id %d task create fail %d", eWhichOtg, __FUNCTION__, USBOTG_HOST_ISOC_TASK, sdwRetVal);
            return sdwRetVal;
        }
        
        sdwRetVal = TaskStartup(USBOTG_HOST_ISOC_TASK);
        if (sdwRetVal != USBOTG_NO_ERROR)
        {
            MP_ALERT("-USBOTG%d- %s OTGHOST ISOC Task Id %d startup fail %d", eWhichOtg, __FUNCTION__, USBOTG_HOST_ISOC_TASK, sdwRetVal);
            return sdwRetVal;
        }
    }
    else
    {
        MP_ALERT("-USBOTG%d- %s not support ISOC function!!", eWhichOtg, __FUNCTION__);
    }
#endif //#if USBOTG_HOST_ISOC

    MP_DEBUG("-USBOTG%d- %s End", eWhichOtg, __FUNCTION__);
    return sdwRetVal;
    
}

void UsbOtgHostTaskTerminate(WHICH_OTG eWhichOtg)
{
    MP_DEBUG("UsbOtgHostTaskTerminate");
    //__asm("break 100");
    MailboxDestroy  (UsbOtgHostMailBoxIdGet(eWhichOtg));
    TaskTerminate   (UsbOtgHostClassTaskIdGet(eWhichOtg));
    EventDestroy    (UsbOtgHostDriverEventIdGet(eWhichOtg));
    TaskTerminate   (UsbOtgHostDriverTaskIdGet(eWhichOtg));
    EventDestroy    (UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg));
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	             
    EventSet(UI_EVENT,EVENT_BT_DONGLE_OUT);
#endif  
}

static void UsbOtgHostClassTask(void)
{
    volatile ST_MCARD_MAIL *psMcardRMail;
    BYTE bMcardMailId, bMcardTxId;    
    ST_MCARD_DEVS     *pUsbh;
    pSmFunc pSmProc = NULL;
    DWORD envelope_addr;
    BYTE bMailBoxId;

    WHICH_OTG eWhichOtg;
    DWORD dwArgTemp[2];

    TaskStartupParamGet(TaskGetId(), &dwArgTemp[0],&dwArgTemp[1],&dwArgTemp[1],&dwArgTemp[1]);
    eWhichOtg = dwArgTemp[0];
    
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
#if USBOTG_HOST_CDC    
    PUSB_HOST_CDC psHostCdc = &psHost->sCdc;
#endif
#if USBOTG_HOST_HID    
    PUSB_HOST_HID psHostHid = &psHost->sHid;
#endif
    PUSB_HOST_SIDC psHostSidc = &psHost->sSidc;
#if USBOTG_WEB_CAM
    PUSB_HOST_AVDC psHostWebcam = &psHost->sWebcam;
#endif
#if USBOTG_HOST_DATANG
	PUSB_HOST_DATANG psHostDatang = &psHost->sDatang;
#endif
    bMailBoxId = UsbOtgHostMailBoxIdGet(eWhichOtg);
    pUsbh = (ST_MCARD_DEVS*)GetMcardDevTag4Usb();//psMcardDev;
    InitiaizeUsbOtgHost(IS_FIRST_TIME_INIT, eWhichOtg); 
    while(1)
    {
        MailboxReceive(bMailBoxId, &bMcardMailId);
        
        if(MailGetBufferStart(bMcardMailId, (DWORD*)(&psMcardRMail)) == OS_STATUS_OK)
        {
            bMcardTxId = psMcardRMail->wMCardId - pUsbh->bIdOffsetValue;   
            pUsbh->sMDevice[bMcardTxId].sMcardRMail = (ST_MCARD_MAIL *)psMcardRMail;
            envelope_addr = (DWORD) psMcardRMail;

            if (envelope_addr == (DWORD)psHost->psClassTaskEnvelope ||
                envelope_addr == (DWORD)psHost->psDriverTaskEnvelope ||
                envelope_addr == (DWORD)psHostSidc->psPtpEnvelope ||
#if USBOTG_WEB_CAM
                envelope_addr == (DWORD)psHostWebcam->psWebCamEnvelope ||
#endif
#if USBOTG_HOST_CDC
                envelope_addr == (DWORD)psHostCdc->psCdcEnvelope ||
#endif
#if USBOTG_HOST_HID
                envelope_addr == (DWORD)psHostHid->psHidEnvelope ||
#endif
#if USBOTG_HOST_DATANG
			envelope_addr == (DWORD)psHostDatang->psDatangEnvelope ||
#endif
                envelope_addr == (DWORD)psHost->psIsrEnvelope)
            {   // mail comes from filesystem, UsbOtgHostClassTask itself or UsbOtgDriverTask
                pSmProc = GetSmFunctionPointer(psMcardRMail->wStateMachine);
                if (pSmProc != NULL)
                {
                    pSmProc(pUsbh, bMcardTxId, eWhichOtg);
                }
                else
                {
                    MP_DEBUG("pSmProc is NULL!!");
                }
				MailRelease(bMcardMailId);
				MailTrack(bMcardMailId);
            }
            else
            {   // mail comes from FileSystem
				
                //if (&pUsbh->sMDevice[bMcardTxId].CommandProcess != NULL)
                if (pUsbh->sMDevice[bMcardTxId].CommandProcess != NULL)
                {
                    pUsbh->sMDevice[bMcardTxId].CommandProcess(&pUsbh->sMDevice[bMcardTxId]);
                }
                else
                {
                    MP_ALERT("CommandProcess is NULL!!:%d", bMcardTxId);
                    pUsbh->sMDevice[bMcardTxId].swStatus = -1;
                }
				MailRelease(bMcardMailId);
			}
            
            if (psHost->boIsHostToResetDevice == TRUE || psHost->boIsHostFinalized == TRUE)
            {
#if USBOTG_HOST_DATANG
            		// Datang 3G USB Modem don't need interrupt
            		if(!( pUsbhDevDes->sDeviceDescriptor.idVendor == (WORD)byte_swap_of_word(TDSCDMA_USB_DEVICE_VID) && 
				pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(TDSCDMA_USB_PID)))
#endif
            		{
				DisableUsbOtgHostInterrupt(eWhichOtg);
			}
                if (psHost->boIsHostToResetDevice == TRUE)
                {
                    ST_MCARD_MAIL   *pSendMailDrv;
                    
                    psHost->boIsHostToResetDevice = FALSE;
                    //InitiaizeUsbOtgHost(NOT_FIRST_TIME_INIT);
                    pUsbhDevDes->bDeviceStatus = USB_STATE_ATTACHED;

                    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
                    pSendMailDrv->wStateMachine             = SETUP_SM;
                    pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_STOP_STATE;
                    SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
                }
                else if (psHost->boIsHostFinalized == TRUE)
                {
                    InitiaizeUsbOtgHost(IS_FIRST_TIME_INIT, eWhichOtg);
                }                
            }
        }
    }
}
#if USBCAM_DEBUG_ISR_LOST
static DWORD st_dwOtgIoc=0;
#endif

static void UsbOtgHostDriverTask(void)
{
	DWORD dwEvent, dwNextEvent;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    BYTE bEventId = 0;
    WHICH_OTG eWhichOtg;
    DWORD dwArgTemp[2];

    TaskStartupParamGet(TaskGetId(), &dwArgTemp[0],&dwArgTemp[1],&dwArgTemp[1],&dwArgTemp[1]);
    eWhichOtg = dwArgTemp[0];
    
    bEventId = UsbOtgHostDriverEventIdGet(eWhichOtg);
    UsbOtgControllerInit(eWhichOtg); // for debug
	dwNextEvent = 0;
	while (1)
	{
		if (UsbOtgHostDriverTaskWaitEvent(&dwEvent, dwNextEvent, bEventId) == OS_STATUS_OK)
		{
            //MP_DEBUG("USBOTG%d DT Event = 0x%x", eWhichOtg, dwEvent);
            pUsbhDevDes  = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);//gpUsbhDeviceDescriptor;
            UsbOtgHostHandlePlugOutAction(dwEvent, eWhichOtg);
            if (dwEvent & EVENT_DEVICE_PLUG_IN)
            {
                UsbOtgHostEventPlugIn(eWhichOtg);
            }

            if (dwEvent & EVENT_DEVICE_PLUG_OUT)
            {
                UsbOtgHostEventPlugOut(eWhichOtg, FALSE);
            }

            if (dwEvent & EVENT_EHCI_IOC)
            {
#if USBOTG_HOST_IOC_POLIING_SWITCH                
                if (UseIocIsr4MsdcTx(eWhichOtg)==0) 
                {
                    UartOutText("B");
                    //mpDebugPrint("ioc event sts = 0x%x", mwHost20_USBSTS_Rd());
                    return;
                }
#endif                
#if USBCAM_DEBUG_ISR_LOST
    st_dwOtgIoc++;
	if ((st_dwOtgIoc%1000)==0)
		mpDebugPrint("Ioc %d",st_dwOtgIoc);
#endif
                UsbOtgHostEventIoc(eWhichOtg);
            }

            if (dwEvent & EVENT_EHCI_ACTIVE_SETUP)
            {
                UsbOtgHostEventActiveSetup(eWhichOtg);
            }

            if (dwEvent & EVENT_EHCI_ACTIVE_BULK)
            {
                UsbOtgHostEventActiveBulk(eWhichOtg);
            }

#if (USBOTG_HOST_ISOC)
            if (dwEvent & EVENT_EHCI_ACTIVE_ISO_IN)
            {
                //UartOutText("\r\n<A>");
                if (pUsbhDevDes->bIsoInEnable == TRUE)
                {
                    UsbOtgHostIsocInActive(eWhichOtg);
                }
            }

            if (dwEvent & EVENT_EHCI_ACTIVE_ISO_OUT)
            {
                UsbOtgHostIsocOutActive(eWhichOtg);
            }
#endif

            if (dwEvent & EVENT_EHCI_ACTIVE_INTERRUPT)
            {
                UsbOtgHostEventActiveInterrupt(eWhichOtg);
            }
            
            if (dwEvent & EVENT_POLLING_EACH_LUN)
            {
                if (USB_STATE_CONFIGURED != pUsbhDevDes->bDeviceStatus)
                    return;
                UsbOtgHostEventPollingEachLun(eWhichOtg);
            }
        }
    }

//TaskTerminate(); // this will never be excuted 
}


#define EVENT_MASK 0x7fffffff
static SDWORD UsbOtgHostDriverTaskWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent, BYTE bEventId)
{
    SDWORD ret;

    if (dwNextEvent)
    {
        *pdwEvent = dwNextEvent;
        ret = OS_STATUS_OK;
    }
    else
    {
        ret = EventWait(bEventId, EVENT_MASK, OS_EVENT_OR, pdwEvent);
    }
    return ret;
}




//void UsbOtgHostEventClearStall(void)
//{
//    ST_MCARD_MAIL   *pSendMailDrv;
    
//    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER);
//    pSendMailDrv->wStateMachine             = SETUP_SM;
//    pSendMailDrv->wCurrentExecutionState    = SETUP_CLEAR_STALL_STATE;
//	SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv);
//}


static void UsbOtgHostEventPlugIn(WHICH_OTG eWhichOtg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    BYTE bUsbOtgMsdcTxDoneEventId = UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg);
    
    MP_DEBUG("E_IN USBOTG%d", eWhichOtg);
    MP_DEBUG("EventClear EVENT_MSDC_DEVICE_PLUG_OUT,ID=%d ",bUsbOtgMsdcTxDoneEventId);
    EventClear(bUsbOtgMsdcTxDoneEventId, 0);
    //SysTimerProcRemove(UsbOtgHostMsdcPollingForFullSpeed);
    psHost->boIsHostFinalized = FALSE;

    //pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER);
    //pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER);
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_START_STATE;
	SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

void UsbOtgHostEventPlugOut(WHICH_OTG eWhichOtg, BOOL bMailTrack)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    
    MP_DEBUG("-USBOTG%d- %s E_OUT", eWhichOtg, __FUNCTION__);
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_STOP_STATE;

    if(bMailTrack)
        SendMailTrackToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
    else
        SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

static void UsbOtgHostEventPollingEachLun(WHICH_OTG eWhichOtg)
{
    ST_MCARD_MAIL   *pSendMailDrv;
    
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = MSDC_CHECK_MEDIA_PRESENT_SM;
    pSendMailDrv->wCurrentExecutionState    = MSDC_CMP_START_STATE;
	SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
}

#if USBOTG_HOST_INTERRUPT_TX_TEST    
extern int gtt_set_int_evnet;
#endif
static void UsbOtgHostEventIoc (WHICH_OTG eWhichOtg)
{
    BYTE i = 0;
    BYTE j = 0;
    BYTE bMaxNumEndpoints = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

#if USBOTG_HOST_DESC
    BYTE bNumEndpoints = 0;

    if(pUsbhDev->pConfigDescriptor == NULL)
        bNumEndpoints = 0;
    else // GetInterfaceEPs sometimes is ZERO or Non-ZERO between pUsbhDev->pConfigDescriptor == NULL
        bNumEndpoints = GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, pUsbhDev->bDeviceInterfaceIdx, AlterSettingDefaultIdx);
    
    if (bNumEndpoints == 0)
        bMaxNumEndpoints = 2;
    else
        bMaxNumEndpoints = bNumEndpoints+1; // control 
#else // USBOTG_HOST_DESC
    if (pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints == 0)
        bMaxNumEndpoints = 2;
    else
        bMaxNumEndpoints = pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints+1; // control 
#endif // USBOTG_HOST_DESC        
    
#if HAVE_USB_MODEM
    if (pUsbhDev->bIsNetUsb)
    {
        NetUsb_EventIoc(eWhichOtg, bMaxNumEndpoints);
    }
#endif

    for (i = 0; i < bMaxNumEndpoints; i++)
    {
        for (j = 0; j< 2; j++)
        {
            
            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->pstControlqHD[j]))
            {
                if (pUsbhDev->psControlqTD->bStatus_Active == 0)
                {
                    UsbOtgHostEventIocSetup(eWhichOtg);
                }
            }
        }
        
#if (USBCAM_IN_ENABLE!=2)
        for (j = 0; j< pUsbhDev->bMaxBulkOutEpNumber; j++)
        {
#if (NETWARE_ENABLE == ENABLE)
            if (pUsbhDev->isWlan)
            {

                if (Wlan_UsbOtgHostBulkIoc(pUsbhDev->hstBulkOutqHD[j]))
                {
                    pUsbhDev->dwWhichBulkPipeDone |= (1<<j);
                    UsbOtgHostEventIocBulk(eWhichOtg);
                }

            }
            else
#endif

            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstBulkOutqHD[j]))
            {
                if (pUsbhDev->hstBulkOutqTD[j]->bStatus_Active == 0)
                {
                    pUsbhDev->dwWhichBulkPipeDone |= (1<<j);
                    UsbOtgHostEventIocBulk(eWhichOtg);
                }
            }
        }
        
        for (j = 0; j< pUsbhDev->bMaxBulkInEpNumber; j++)
        {
#if (NETWARE_ENABLE == ENABLE)
            if (pUsbhDev->isWlan)
            {

                if (Wlan_UsbOtgHostBulkIoc(pUsbhDev->hstBulkInqHD[0]))
                {
                    pUsbhDev->dwWhichBulkPipeDone |= (1<<16);
                    UsbOtgHostEventIocBulk(eWhichOtg);
                }


            }
            else
#endif

            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstBulkInqHD[j]))
            {
                if (pUsbhDev->hstBulkInqTD[j]->bStatus_Active == 0)
                {
                    pUsbhDev->dwWhichBulkPipeDone |= (1<<(j+16));
                    UsbOtgHostEventIocBulk(eWhichOtg);
                }
            }
        }

        for (j = 0; j< pUsbhDev->bMaxInterruptOutEpNumber; j++)
        {
#if (NETWARE_ENABLE == ENABLE)
            if (pUsbhDev->isWlan)
            {

                if (Wlan_UsbOtgHostBulkIoc(pUsbhDev->hstInterruptOutqHD[j]))
                {
                    pUsbhDev->dwWhichInterruptPipeDone |= (1<<j);
                    UsbOtgHostEventIocInterrupt(eWhichOtg);
                }

            }
            else
#endif

            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstInterruptOutqHD[j]))
            {
                if (pUsbhDev->hstInterruptOutqTD[j]->bStatus_Active == 0)
                {
                    pUsbhDev->dwWhichInterruptPipeDone |= (1<<j);
                    UsbOtgHostEventIocInterrupt(eWhichOtg);
                }
            }
        }
        
        for (j = 0; j< pUsbhDev->bMaxInterruptInEpNumber; j++)
        {
#if (NETWARE_ENABLE == ENABLE)
            if (pUsbhDev->isWlan)
            {

                if (Wlan_UsbOtgHostBulkIoc(pUsbhDev->hstInterruptInqHD[0]))
                {
                    pUsbhDev->dwWhichInterruptPipeDone |= (1<<16);
                    UsbOtgHostEventIocInterrupt(eWhichOtg);
                }


            }
            else
#endif

            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstInterruptInqHD[j]))
            {
                if (pUsbhDev->hstInterruptInqTD[j]->bStatus_Active == 0)
                {
#if USBOTG_HOST_INTERRUPT_TX_TEST    
                    gtt_set_int_evnet = 0;
#endif
                    pUsbhDev->dwWhichInterruptPipeDone |= (1<<(j+16));
                    UsbOtgHostEventIocInterrupt(eWhichOtg);
                }
            }
        }
#endif
        
#if USBOTG_HOST_ISOC
        if (pUsbhDev->bIsoInEnable == TRUE)
        {
            //UartOutText("I");
            for (j = 0; j< pUsbhDev->bMaxIsoInEpNumber; j++)
            //if (pUsbhDev->stIsocDataStream.dwIsoActive[0] || pUsbhDev->stIsocDataStream.dwIsoActive[1])
                UsbOtgHostIsocInIoc(eWhichOtg);
        }
#if (USBCAM_IN_ENABLE!=2)
        for (j = 0; j< pUsbhDev->bMaxIsoOutEpNumber; j++)
            UsbOtgHostIsocOutIoc(eWhichOtg);
#endif
#endif
    } // for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)
}

qTD_Structure *g_spTempqTD;
qHD_Structure *g_spTempqHD;

static void UsbOtgHostSetupCmdTimeOut(void *arg)
{
    WHICH_OTG eWhichOtg = ((BYTE *) arg)[1];
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    ST_MCARD_MAIL   *pSendMailDrv;

    MP_ALERT("--E-- %s UsbOtg%d", __FUNCTION__, eWhichOtg);
    DebugUsbOtgDumpEhciDs(eWhichOtg);
    DebugUsbOtgDumpQhd(g_spTempqHD);
    DebugUsbOtgDumpQtd(g_spTempqTD);
    DebugUsbOtgDumpRegister(eWhichOtg);
    if (UsbOtgHostConnectStatusGet(eWhichOtg)!=0) 
    { // reset device
        UsbHostFinal(USB_HOST_DEVICE_RESET, eWhichOtg);
        //if (HOST20_Attach_Device_Speed_Full == pUsbhDev->bDeviceSpeed)
            //UsbOtgHostEof1Eof2Adjustment(eWhichOtg);
        MP_ALERT("SETUP command time out");

        pUsbhDev->bCmdTimeoutError = 1;
        if (pUsbhDev->fpAppClassSetupIoc!= OTGH_NULL)
        {
            pUsbhDev->fpAppClassSetupIoc(eWhichOtg);
        }

#if USBCAM_IN_ENABLE
		//PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(USBOTG0);
		//mUsbOtgUnPLGSet();//usb disconnect
		//IODelay(200);
		//mUsbOtgUnPLGClr();//usb connect
#else
        pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER, eWhichOtg);
        pSendMailDrv->wStateMachine             = SETUP_SM;
        pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_STOP_STATE;
    	SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);        
#endif
    }
}


static void UsbOtgHostEventIocSetup (WHICH_OTG eWhichOtg)
{
    BYTE    bEdNum = 0;
    DWORD   bpDataPage;
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    SysTimerProcRemoveById(psUsbOtg->timerId);

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if (pUsbhDev->bDeviceAddress > 0)
        bEdNum = 1;
    else
        bEdNum = 0;
    
    //MP_DEBUG("<B> USBOTG%d", eWhichOtg);
    //MP_DEBUG("<B> pUsbhDev->sSetupPB.dwSetupState= %d", pUsbhDev->sSetupPB.dwSetupState);
    flib_Host20_Send_qTD_Done(pUsbhDev->pstControlqHD[bEdNum], eWhichOtg);
    if (pUsbhDev->sSetupPB.dwSetupState == SETUP_DONE_STATE)
    {
        pUsbhDev->sSetupPB.dwSetupState = SETUP_IDLE_STATE;
        if (pUsbhDev->fpAppClassSetupIoc!= OTGH_NULL)
        {
            pUsbhDev->fpAppClassSetupIoc(eWhichOtg);
        }
        else
        {
            SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg), eWhichOtg); // for next setup command
        }
    }
    else if ((pUsbhDev->sSetupPB.dwSetupState == SETUP_STATUS_OUT_STATE)||
             (pUsbhDev->sSetupPB.dwSetupState == SETUP_DATA_OUT_STATE))
    {
    	if (pUsbhDev->sSetupPB.dwSetupState == SETUP_STATUS_OUT_STATE) // BILL TODO for SETUP_STATUS_OUT_STATE only
    	{
            bpDataPage = pUsbhDev->pstControlWorkqTD->ArrayBufferPointer_Word[0];
            psEhci->dwControlRemainBytes = pUsbhDev->pstControlSendLastqTD->bTotalBytes; // For Konica DSC
            memcpy((BYTE*)pUsbhDev->sSetupPB.dwDataAddress, (BYTE*)bpDataPage, pUsbhDev->sSetupPB.wDataInLength);
    	}
        EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_SETUP); // for SETUP_STATUS_OUT_STATE
    }
    else if ((pUsbhDev->sSetupPB.dwSetupState == SETUP_STATUS_IN_STATE)||
             (pUsbhDev->sSetupPB.dwSetupState == SETUP_DATA_IN_STATE))
    {
        EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_SETUP); // for SETUP_STATUS_IN_STATE
    }
    else
    {
        MP_DEBUG1("??? UsbOtgHostEventIocSetup:SetupState = %d", pUsbhDev->sSetupPB.dwSetupState);
    }
}

static void UsbOtgHostEventActiveSetup (WHICH_OTG eWhichOtg)
{
    qTD_Structure   *spTempqTD;
    BYTE            *bpDataPage;
    BYTE            bReturnValue;
    BYTE            bEdNum = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    if (pUsbhDev->bDeviceAddress > 0)
        bEdNum = 1;
    else
        bEdNum = 0;
    
    switch(pUsbhDev->sSetupPB.dwSetupState)
    {
        case SETUP_COMMAND_STATE:
        {
            bpDataPage = pUsbhDev->bDataBuffer;
            spTempqTD = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg);
            spTempqTD->bPID                          = HOST20_qTD_PID_SETUP;        //Bit8~9   
            spTempqTD->bTotalBytes                   = sizeof(USB_CTRL_REQUEST);    //Bit16~30   
            spTempqTD->bDataToggle                   = 0;                           //Bit31 
            spTempqTD->ArrayBufferPointer_Word[0]    = (DWORD)bpDataPage; 
            memcpy(bpDataPage,(BYTE*)&(pUsbhDev->sSetupPB.stCtrlRequest.bRequestType), sizeof(USB_CTRL_REQUEST));
            
            //pUsbhDev->pstWorkqTD = spTempqTD;
            if (pUsbhDev->sSetupPB.wDataInLength > 0)
            {
                pUsbhDev->sSetupPB.dwSetupState = SETUP_DATA_IN_STATE;
            }
            else if (pUsbhDev->sSetupPB.wDataOutLength > 0)
            {
                pUsbhDev->sSetupPB.dwSetupState = SETUP_DATA_OUT_STATE;
            }
            else
            {
                pUsbhDev->sSetupPB.dwSetupState = SETUP_STATUS_IN_STATE;
            }
            //MP_DEBUG("<A> pUsbhDev->sSetupPB.dwSetupState= %d", pUsbhDev->sSetupPB.dwSetupState);
            
            flib_Host20_Send_qTD_Active (   spTempqTD,
                                            pUsbhDev->pstControlqHD[bEdNum],
                                            eWhichOtg);
        }
        break;

        case SETUP_DATA_IN_STATE:
        {
            //<A>.Fill qTD
            bpDataPage = pUsbhDev->bDataBuffer;
            spTempqTD = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg);//0=>qTD              
            spTempqTD->bPID                         = HOST20_qTD_PID_IN;    //Bit8~9   
            spTempqTD->bTotalBytes                  = pUsbhDev->sSetupPB.wDataInLength;           //Bit16~30   
            spTempqTD->bDataToggle                  = 1;                    //Bit31 
            spTempqTD->ArrayBufferPointer_Word[0]   = (DWORD)bpDataPage;

            //pUsbhDev->pstWorkqTD             = spTempqTD;
            pUsbhDev->sSetupPB.dwSetupState = SETUP_STATUS_OUT_STATE;
            //<B>.Active qTD
            flib_Host20_Send_qTD_Active (   spTempqTD,
                                            pUsbhDev->pstControlqHD[bEdNum],
                                            eWhichOtg);
        }
        break;

        case SETUP_STATUS_OUT_STATE:
        {
            //<A>.Fill qTD
            spTempqTD = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg);            
            spTempqTD->bPID         = HOST20_qTD_PID_OUT;    //Bit8~9   
            spTempqTD->bTotalBytes  = 0;                     //Bit16~30   
            spTempqTD->bDataToggle  = 1;                     //Bit31 

            //pUsbhDev->pstWorkqTD             = spTempqTD;
            pUsbhDev->sSetupPB.dwSetupState = SETUP_DONE_STATE;
            //<B>.Active qTD
            flib_Host20_Send_qTD_Active (   spTempqTD,
                                            pUsbhDev->pstControlqHD[bEdNum],
                                            eWhichOtg);
        }
        break;

        case SETUP_DATA_OUT_STATE:
        {
            //<A>.Fill qTD
            bpDataPage = pUsbhDev->bDataBuffer;
            spTempqTD = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg);//0=>qTD 
            spTempqTD->bPID                         = HOST20_qTD_PID_OUT;                   //Bit8~9   
            spTempqTD->bTotalBytes                  = pUsbhDev->sSetupPB.wDataOutLength;   //Bit16~30   
            spTempqTD->bDataToggle                  = 1;                                    //Bit31 
            spTempqTD->ArrayBufferPointer_Word[0]   = (DWORD)bpDataPage; 
            memcpy(bpDataPage, (BYTE*)pUsbhDev->sSetupPB.dwDataAddress, pUsbhDev->sSetupPB.wDataOutLength);

            //pUsbhDev->pstWorkqTD             = spTempqTD;
            pUsbhDev->sSetupPB.dwSetupState = SETUP_STATUS_IN_STATE;
			//<B>.Send qTD
            flib_Host20_Send_qTD_Active (   spTempqTD,
                                            pUsbhDev->pstControlqHD[bEdNum],
                                            eWhichOtg);
        }
        break;

        case SETUP_STATUS_IN_STATE:
        {
            //<A>.Fill qTD
            spTempqTD = (qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg);//0=>qTD              
            spTempqTD->bPID                         = HOST20_qTD_PID_IN;    //Bit8~9   
            spTempqTD->bTotalBytes                  = 0;                    //Bit16~30   
            spTempqTD->bDataToggle                  = 1;                    //Bit31 
            spTempqTD->ArrayBufferPointer_Word[0]   = 0; 

            //pUsbhDev->pstWorkqTD             = spTempqTD;
            pUsbhDev->sSetupPB.dwSetupState = SETUP_DONE_STATE;
            //<B>.Active qTD
            flib_Host20_Send_qTD_Active (   spTempqTD,
                                            pUsbhDev->pstControlqHD[bEdNum],
                                            eWhichOtg);
        }
        break;

        case SETUP_DONE_STATE:
        {
        }
        break;

        default:
        {
            
        }
        break;
    }

    UsbOtgSetSysTimerProc(USBOTG_TRANSACTION_TIME_OUT_CNT, UsbOtgHostSetupCmdTimeOut,TRUE, eWhichOtg);
}

static void UsbOtgHostEventActiveBulk (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDev->fpAppClassBulkActive != OTGH_NULL)
        pUsbhDev->fpAppClassBulkActive(eWhichOtg);
    else
        MP_DEBUG("--E-- %s USBOTG%d fpAppClassBulkActive is NULL!!", __FUNCTION__, eWhichOtg);
}

static void UsbOtgHostEventIocBulk (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDev->fpAppClassBulkIoc != OTGH_NULL)
        pUsbhDev->fpAppClassBulkIoc(eWhichOtg);
    else
        MP_DEBUG("pUsbhDev->fpAppClassBulkIoc is NULL!!");
}

static void UsbOtgHostEventActiveInterrupt(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDev->fpAppClassInterruptActive != OTGH_NULL)
        pUsbhDev->fpAppClassInterruptActive(eWhichOtg);
    else
        MP_DEBUG("pUsbhDev->UsbOtgHostEventActiveInterrupt is NULL!!");
}

static void UsbOtgHostEventIocInterrupt(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDev->fpAppClassInterruptIoc != OTGH_NULL)
        pUsbhDev->fpAppClassInterruptIoc(eWhichOtg);
    else
        MP_DEBUG("pUsbhDev->fpAppClassInterruptIoc is NULL!!");
}


// This function uses for none MailTrack only
DWORD GetUsbhMailEnvelope(WORD sender, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    if (sender == USBOTG_HOST_CLASS_TASK_SENDER)    return (DWORD)psHost->psClassTaskEnvelope;
    if (sender == USBOTG_HOST_DRIVER_TASK_SENDER)   return (DWORD)psHost->psDriverTaskEnvelope;
    if (sender == USBOTG_HOST_FOR_ISR_SENDER)       return (DWORD)psHost->psIsrEnvelope;
    PUSB_HOST_SIDC psHostSidc = &psHost->sSidc;
    if (sender == USBOTG_HOST_FOR_PTP_SENDER)       return (DWORD)psHostSidc->psPtpEnvelope;
#if USBOTG_WEB_CAM
    PUSB_HOST_AVDC psHostWebcam = &psHost->sWebcam;
	if (sender == USBOTG_HOST_FOR_WEB_CAM_SENDER)	return (DWORD)psHostWebcam->psWebCamEnvelope;
#endif
#if USBOTG_HOST_CDC
    PUSB_HOST_CDC psHostCdc = &psHost->sCdc;
	if (sender == USBOTG_HOST_FOR_CDC_SENDER)		return (DWORD)psHostCdc->psCdcEnvelope;
#endif
#if USBOTG_HOST_DATANG
	PUSB_HOST_DATANG psHostDatang = &psHost->sDatang;
	if (sender == USBOTG_HOST_FOR_DATANG_SENDER)		return (DWORD)psHostDatang->psDatangEnvelope;
#endif
#if USBOTG_HOST_HID
    PUSB_HOST_HID psHostHid = &psHost->sHid;
    if (sender == USBOTG_HOST_FOR_HID_SENDER)		return (DWORD)psHostHid->psHidEnvelope;
#endif
    MP_ASSERT(0);
    return 0;
}

void UsbOtgHostSetAddressForUsbAddressState(BYTE device_address, PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc)
{
   psUsbhDevDesc->pstControlqHD[1]->bDeviceAddress = device_address;
}

void UsbOtgHstSetMaxPacketSizeForControlPipe(BYTE max_packet_size, PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc)
{
   //Set the ep0 max packet size 
   psUsbhDevDesc->pstControlqHD[0]->bMaxPacketSize = max_packet_size; 
   psUsbhDevDesc->pstControlqHD[1]->bMaxPacketSize = max_packet_size; 
}

void UsbhStorageInit (ST_MCARD_DEV *psDev, BYTE bType)
{
    WHICH_OTG eWhichOtg = GetWhichUsbOtgByCardId(bType);
    PUSB_HOST_MSDC psHostMsdc = (PUSB_HOST_MSDC)UsbOtgHostMsdcDsGet(eWhichOtg);
    switch (bType)
    {
#if  USB_HOST_ID1_ENABLE//byAlexWang 24jun2007 m2project
        case DEV_USB_HOST_ID1:
        {
            psDev->pbDescriptor = psHostMsdc->bDescriptor[0];
        }
        break;
#endif            
#if  USB_HOST_ID2_ENABLE
        case DEV_USB_HOST_ID2:
        {
            psDev->pbDescriptor = psHostMsdc->bDescriptor[1];
        }
        break;
#endif            
#if  USB_HOST_ID3_ENABLE
        case DEV_USB_HOST_ID3:
        {
            psDev->pbDescriptor = psHostMsdc->bDescriptor[2];
        }
        break;
#endif
#if  USB_HOST_ID4_ENABLE
        case DEV_USB_HOST_ID4:
        {
            psDev->pbDescriptor = psHostMsdc->bDescriptor[3];
        }
        break;
#endif
#if USB_HOST_PTP_ENABLE
        case DEV_USB_HOST_PTP:
        {
            psDev->pbDescriptor = psHostMsdc->bDescriptor[4];
        }
        break;
#endif		
#if  USBOTG1_HOST_ID1_ENABLE//byAlexWang 24jun2007 m2project
            case DEV_USBOTG1_HOST_ID1:
            {
                psDev->pbDescriptor = psHostMsdc->bDescriptor[0];
            }
            break;
#endif            
#if  USBOTG1_HOST_ID2_ENABLE
            case DEV_USBOTG1_HOST_ID2:
            {
                psDev->pbDescriptor = psHostMsdc->bDescriptor[1];
            }
            break;
#endif            
#if  USBOTG1_HOST_ID3_ENABLE
            case DEV_USBOTG1_HOST_ID3:
            {
                psDev->pbDescriptor = psHostMsdc->bDescriptor[2];
            }
            break;
#endif
#if  USBOTG1_HOST_ID4_ENABLE
            case DEV_USBOTG1_HOST_ID4:
            {
                psDev->pbDescriptor = psHostMsdc->bDescriptor[3];
            }
            break;
#endif
#if USBOTG1_HOST_PTP_ENABLE
            case DEV_USBOTG1_HOST_PTP:
            {
                psDev->pbDescriptor = psHostMsdc->bDescriptor[4];
            }
            break;
#endif		
    }
    psDev->wMcardType = bType;
    psDev->Flag.Installed = 1;
    psDev->CommandProcess = CommandProcess;
    MP_DEBUG("UsbhStorageInit");
}

static void UsbhStorageUninit (ST_MCARD_DEV *psDev)
{
    psDev->Flag.Installed   = 0;
    MP_DEBUG("UsbhStorageUninit");
}

void USbhDeviceEnable(E_DEVICE_ID bMcardID)
{
    ST_MCARD_DEVS     *pUsbhMcardDev;
    pUsbhMcardDev = GetMcardDevTag4Usb();
    if ((bMcardID >= DEV_USB_HOST_ID1) ||\
        (bMcardID <= DEV_USBOTG1_HOST_PTP) ||\
        (bMcardID == DEV_USB_WIFI_DEVICE) ||\
        (bMcardID == DEV_USB_ETHERNET_DEVICE) ||\
        (bMcardID == DEV_USB_WEBCAM))
    {
	    UsbhStorageInit(&pUsbhMcardDev->sMDevice[bMcardID], bMcardID);
	    pUsbhMcardDev->dwDevInsCnt++;
	    mpDebugPrint("Enable Device %d (Total Count %d)", bMcardID, pUsbhMcardDev->dwDevInsCnt);
	}
}

void USbhDeviceDisable(BYTE bMcardID)
{
    BYTE            bTransferID;
    ST_MCARD_DEVS   *pUsbhMcardDev;

    pUsbhMcardDev = (ST_MCARD_DEVS*)GetMcardDevTag4Usb();
    bTransferID = bMcardID - pUsbhMcardDev->bIdOffsetValue;
    UsbhStorageUninit(&pUsbhMcardDev->sMDevice[bTransferID]);
}

/*
SWORD UsbhStorage_GetStatus (BYTE bMcardID, WHICH_OTG eWhichOtg)
{
    SWORD   sts = 0;
    BYTE bLun;
    ST_MCARD_DEVS *pUsbh;// = psMcardDev;//= (ST_MCARD_DEVS *)((DWORD)&gUsbhMsdc |0xa0000000);
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);

    pUsbh = (ST_MCARD_DEVS*)GetMcardDevTag4Usb();//psMcardDev;
    bLun = GetLunbyCardId(bMcardID);//bMcardID - pUsbh->bIdOffsetValue;

    //if (gpUsbhDeviceDescriptor->bDeviceStatus != USB_STATE_CONFIGURED)
    if (psHost->boIsReadyToReadWriteUsbh == FALSE)
    {
        sts = -1;
    }
    else
    {
        sts = pUsbh->sMDevice[bLun].swStatus;
    }
    
    if (sts != 0)
    {
        MP_DEBUG("sts is failed");
    }
    
    return sts;
}

SWORD GetUsbhDeviceStatus(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	return psUsbhDevDesc->bDeviceStatus;
}
*/
SWORD Api_UsbhStorageDeviceInit (BYTE bMcardID)
{
    static ST_MCARD_MAIL sMcardRMail;
    BYTE bMcardMailId;
    WHICH_OTG eWhichOtg = GetWhichUsbOtgByCardId((WORD)bMcardID);
    
    MP_DEBUG("%s USBOTG%d bMcardID = %d", __FUNCTION__, eWhichOtg, bMcardID);
    USbhDeviceEnable(bMcardID);
    memset(&sMcardRMail, 0, sizeof(ST_MCARD_MAIL));
    sMcardRMail.wMCardId = bMcardID;
    sMcardRMail.wCmd = INIT_CARD_CMD;
    MailboxSend(UsbOtgHostMailBoxIdGet(eWhichOtg), (BYTE *)(&sMcardRMail.dwBlockAddr), sizeof(ST_MCARD_MAIL), &bMcardMailId);
    MailTrack(bMcardMailId);
    return MCARD_CMD_PASS;
	//return McardDev.sMDevice[sMcardRMail.wMCardId].swStatus;
}

SWORD Api_UsbhStorageDeviceRemove (BYTE bMcardID) 
{
    static ST_MCARD_MAIL sMcardRMail;
    BYTE bMcardMailId;
    WHICH_OTG eWhichOtg = GetWhichUsbOtgByCardId((WORD)bMcardID);

    MP_DEBUG("%s USBOTG%d bMcardID = %d", __FUNCTION__, eWhichOtg, bMcardID);
    USbhDeviceDisable(bMcardID);
    sMcardRMail.wMCardId = bMcardID;
    sMcardRMail.wCmd = REMOVE_CARD_CMD;
    MailboxSend(UsbOtgHostMailBoxIdGet(eWhichOtg), (BYTE *)(&sMcardRMail.dwBlockAddr), sizeof(ST_MCARD_MAIL), &bMcardMailId);
    MailTrack(bMcardMailId);
    return MCARD_CMD_PASS;		
	//return McardDev.sMDevice[sMcardRMail.wMCardId].swStatus;
}

/*
BOOL UsbhStorage_GetDrivePresent(BYTE card_id)
{
    BYTE          bTransferID;
    ST_MCARD_DEVS *pUsbh;

    pUsbh = (ST_MCARD_DEVS*)GetMcardDevTag4Usb();//psMcardDev;
	bTransferID = card_id - pUsbh->bIdOffsetValue;
    return pUsbh->sMDevice[bTransferID].Flag.Present;
}
*/
BOOL UsbhStorage_GetDriveInstalled(BYTE card_id)
{
    BYTE          bTransferID;
    ST_MCARD_DEVS *pUsbh;

    pUsbh = (ST_MCARD_DEVS*)GetMcardDevTag4Usb();//psMcardDev;
	bTransferID = card_id - pUsbh->bIdOffsetValue;
    return pUsbh->sMDevice[bTransferID].Flag.Installed;
}

void UsbOtgHostSetSwapBuffer1RangeEnable(DWORD start, DWORD end, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    
    mOTG20_Wrapper_SwapBufferStart1_Set(start);
    mOTG20_Wrapper_SwapBufferEnd1_Set(end);
    mwOTG20_Wrapper_SwapBUffer1Enable_Set();
}

static void UsbOtgHostSetSwapBuffer1RangeDisable(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    
    if (mwOTG20_Wrapper_SwapBUffer1Enable_Rd() != 0)
        mwOTG20_Wrapper_SwapBUffer1Enable_Clr();
}

void UsbOtgHostSetSwapBuffer2RangeEnable(DWORD start, DWORD end, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg); 
    mOTG20_Wrapper_SwapBufferStart2_Set(start);
    mOTG20_Wrapper_SwapBufferEnd2_Set(end);
    mwOTG20_Wrapper_SwapBUffer2Enable_Set();
}

void UsbOtgHostSetSwapBuffer2RangeDisable(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg); 
    if (mwOTG20_Wrapper_SwapBUffer2Enable_Rd() != 0)
        mwOTG20_Wrapper_SwapBUffer2Enable_Clr();
}

void UsbOtgHostSetSwapMemoryPool(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    UsbOtgHostSetSwapBuffer1RangeEnable (psEhci->dwHostMemoryBaseAddress,
                                        (psEhci->dwHostMemoryBaseAddress+\
                                        (MEM_BLOCK_512*SIZE_OF_UNIT*HOW_MANY_MEM_BLOCK)+\
                                        (MEM_BLOCK_1024*SIZE_OF_UNIT*HOW_MANY_MEM_BLOCK)),
                                        eWhichOtg);
    
}

static void UsbOtgMemInit(WHICH_OTG eWhichOtg)
{
    DWORD *pwData;
    DWORD dwTotalMemorySize = 0;
    WORD wExtraLen = 0;
    PUSB_HOST_EHCI psEhci;
    PST_USB_OTG_DES psUsbOtg;
    
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);    

//    psUsbOtg->dwUsbBuf = SystemGetMemAddr(USB_OTG_BUF_MEM_ID)|((DWORD)0xa0000000);;
    psEhci->dwHostStructureBaseAddress = psUsbOtg->dwUsbBuf;//gHost20_STRUCTURE_BASE_ADDRESS      = g_USBBUF;
    psEhci->dwHostStructureQhdBaseAddress = psEhci->dwHostStructureBaseAddress;//gHost20_STRUCTURE_qHD_BASE_ADDRESS  = gHost20_STRUCTURE_BASE_ADDRESS;
    psEhci->dwHostStructureQtdBaseAddress = psEhci->dwHostStructureQhdBaseAddress + (Host20_qHD_SIZE*Host20_qHD_MAX);//gHost20_STRUCTURE_qTD_BASE_ADDRESS  = gHost20_STRUCTURE_qHD_BASE_ADDRESS+(Host20_qHD_SIZE*Host20_qHD_MAX);

// BEGIN MARK:for isoc, it needs 64k bytes
    //gHost20_STRUCTURE_iTD_BASE_ADDRESS  = gHost20_STRUCTURE_qTD_BASE_ADDRESS+(Host20_qTD_SIZE*Host20_qTD_MAX);
    //gUSB_OTG_HOST_MEMORY_BASE           = gHost20_STRUCTURE_iTD_BASE_ADDRESS+(Host20_iTD_SIZE*Host20_iTD_MAX);
// END MARK:for isoc, it needs 64k bytes

    //gUSB_OTG_HOST_MEMORY_BASE           = gHost20_STRUCTURE_qTD_BASE_ADDRESS+(Host20_qTD_SIZE*Host20_qTD_MAX);

#if USBOTG_HOST_ISOC
    psEhci->dwHostStructureItdBaseAddress = psEhci->dwHostStructureQtdBaseAddress + (Host20_qTD_SIZE*Host20_qTD_MAX);//gHost20_STRUCTURE_iTD_BASE_ADDRESS  = gHost20_STRUCTURE_qTD_BASE_ADDRESS+(Host20_qTD_SIZE*Host20_qTD_MAX);

    if ((Host20_Page_SIZE*Host20_Page_MAX) > Host20_SIDC_BULK_RX_BUFFER_SIZE)
    {
        psEhci->dwHostDataPageBaseAddress = psEhci->dwHostStructureItdBaseAddress + (Host20_iTD_SIZE*Host20_iTD_MAX);//gHost20_DATA_PAGE_BASE_ADDRESS = gHost20_STRUCTURE_iTD_BASE_ADDRESS+(Host20_iTD_SIZE*Host20_iTD_MAX);
        psEhci->dwHostDataPageBaseAddress = (psEhci->dwHostDataPageBaseAddress & 0xfffff000) + 0x1000;//gHost20_DATA_PAGE_BASE_ADDRESS = (gHost20_DATA_PAGE_BASE_ADDRESS & 0xfffff000) + 0x1000; // for 4K-Alignment
        psEhci->dwHostSidcRxBufferAddress = psEhci->dwHostDataPageBaseAddress;//gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS	= gHost20_DATA_PAGE_BASE_ADDRESS;
        psEhci->dwHostMemoryBaseAddress   = psEhci->dwHostDataPageBaseAddress + (Host20_Page_SIZE*Host20_Page_MAX);//gUSB_OTG_HOST_MEMORY_BASE= gHost20_DATA_PAGE_BASE_ADDRESS+(Host20_Page_SIZE*Host20_Page_MAX);
    }
    else
    {
        psEhci->dwHostSidcRxBufferAddress = psEhci->dwHostStructureItdBaseAddress + (Host20_iTD_SIZE*Host20_iTD_MAX);//gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS = gHost20_STRUCTURE_iTD_BASE_ADDRESS+(Host20_iTD_SIZE*Host20_iTD_MAX);
        psEhci->dwHostSidcRxBufferAddress = (psEhci->dwHostSidcRxBufferAddress & 0xfffff000) + 0x1000;//??? why 4K-Alignment ??? gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS	= (gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS & 0xfffff000) + 0x1000; // for 4K-Alignment
        psEhci->dwHostDataPageBaseAddress = psEhci->dwHostSidcRxBufferAddress;//gHost20_DATA_PAGE_BASE_ADDRESS = gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS;
        psEhci->dwHostMemoryBaseAddress = psEhci->dwHostDataPageBaseAddress + (Host20_SIDC_BULK_RX_BUFFER_SIZE);//gUSB_OTG_HOST_MEMORY_BASE = gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS+Host20_SIDC_BULK_RX_BUFFER_SIZE;
    }
#else
    psEhci->dwHostSidcRxBufferAddress = psEhci->dwHostStructureQtdBaseAddress + (Host20_qTD_SIZE*Host20_qTD_MAX);//gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS	= gHost20_STRUCTURE_qTD_BASE_ADDRESS+(Host20_qTD_SIZE*Host20_qTD_MAX);
    psEhci->dwHostSidcRxBufferAddress = (psEhci->dwHostSidcRxBufferAddress & 0xfffff000) + 0x1000;;//gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS	= (gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS & 0xfffff000) + 0x1000; // for 4K-Alignment
    psEhci->dwHostMemoryBaseAddress = psEhci->dwHostSidcRxBufferAddress + Host20_SIDC_BULK_RX_BUFFER_SIZE;//gUSB_OTG_HOST_MEMORY_BASE = gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS+Host20_SIDC_BULK_RX_BUFFER_SIZE;
#endif


    psEhci->sMem512Bytes.pBuf  = (BYTE*)((DWORD)psEhci->dwHostMemoryBaseAddress); //gMem_512_Bytes.pBuf    = (BYTE*)((DWORD)gUSB_OTG_HOST_MEMORY_BASE);
    psEhci->sMem1024Bytes.pBuf = (BYTE*)((DWORD)psEhci->dwHostMemoryBaseAddress+(MEM_BLOCK_512*SIZE_OF_UNIT*HOW_MANY_MEM_BLOCK)); //gMem_1024_Bytes.pBuf   = (BYTE*)((DWORD)gUSB_OTG_HOST_MEMORY_BASE+

    psEhci->dwHostQhdQtdHandleBaseAddress = (DWORD)psEhci->sMem1024Bytes.pBuf+(MEM_BLOCK_1024*SIZE_OF_UNIT*HOW_MANY_MEM_BLOCK);//gHost20_qHD_qTD_HANDLE_BASE_ADDRESS = (DWORD)gMem_1024_Bytes.pBuf+
    psEhci->dwHostStructurePflBaseAddress = //gHost20_STRUCTURE_Preiodic_Frame_List_BASE_ADDRESS  = 
                                      psEhci->dwHostQhdQtdHandleBaseAddress+//gHost20_qHD_qTD_HANDLE_BASE_ADDRESS+
                                      (sizeof(qHD_Structure*)*Host20_qHD_MAX)+
#if USBOTG_HOST_ISOC
                                      (sizeof(qTD_Structure*)*Host20_qTD_MAX)+
                                      (sizeof(iTD_Structure*)*Host20_iTD_MAX);
#else
                                      (sizeof(qTD_Structure*)*Host20_qTD_MAX);
#endif
    wExtraLen = ((psEhci->dwHostStructurePflBaseAddress & 0xfffff000)+0x1000) - (psEhci->dwHostStructurePflBaseAddress);//wExtraLen = ((gHost20_STRUCTURE_Preiodic_Frame_List_BASE_ADDRESS&0xfffff000) + 0x1000)-
                  //(gHost20_STRUCTURE_Preiodic_Frame_List_BASE_ADDRESS);    
    psEhci->dwHostStructurePflBaseAddress += wExtraLen;//gHost20_STRUCTURE_Preiodic_Frame_List_BASE_ADDRESS  += wExtraLen; 

    dwTotalMemorySize = ((Host20_qHD_SIZE*Host20_qHD_MAX)+
                         (Host20_qTD_SIZE*Host20_qTD_MAX)+
                    // BEGIN MARK:for isoc, it needs 64k bytes
#if USBOTG_HOST_ISOC
                         (Host20_iTD_SIZE*Host20_iTD_MAX)+
#endif
                    // END MARK:for isoc, it needs 64k bytes
                         ((MEM_BLOCK_512*SIZE_OF_UNIT*HOW_MANY_MEM_BLOCK)+
                          (MEM_BLOCK_1024*SIZE_OF_UNIT*HOW_MANY_MEM_BLOCK))+
                         (Host20_Preiodic_Frame_SIZE*Host20_Preiodic_Frame_List_MAX+wExtraLen));
    //dwTotalMemorySize += (gUSB_OTG_HOST_MEMORY_BASE- (gHost20_STRUCTURE_qTD_BASE_ADDRESS+(Host20_qTD_SIZE*Host20_qTD_MAX)));
    dwTotalMemorySize += (psEhci->dwHostMemoryBaseAddress - (psEhci->dwHostStructureQtdBaseAddress + (Host20_qTD_SIZE*Host20_qTD_MAX)));
    MP_DEBUG1("dwTotalMemorySize is 0x%x", dwTotalMemorySize); 

    if (dwTotalMemorySize > USB_OTG_BUF_SIZE)
    {
        mpDebugPrint("%s line %d: USB Buffer Size too large:need %d but only %d", __FILE__, __LINE__, dwTotalMemorySize, USB_OTG_BUF_SIZE );
        while(1);
    } 

// clear memory
    pwData = (DWORD*) psEhci->dwHostStructureBaseAddress;//gHost20_STRUCTURE_BASE_ADDRESS;
    memset((BYTE*)pwData, 0, dwTotalMemorySize);
    MP_DEBUG("UsbOtgMemInit END");
}

static DWORD UsbOtgHostGetSidcRxBufferAddress(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    return psEhci->dwHostSidcRxBufferAddress;
}

DWORD UsbOtgHostGetSidcGetObjectRxBufferAddress(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI pEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

	if (pEhci->dwHostSidcGetObjecRxBufferAddress == NULL)
		return NULL;
    return (((pEhci->dwHostSidcGetObjecRxBufferAddress & 0xfffff000) + 0x1000) | 0xa0000000);
}

void UsbOtgHostDeviceDescriptorInit(PST_USBH_DEVICE_DESCRIPTOR pUsbhDev)
{
    BYTE i = 0;

    pUsbhDev->sDeviceDescriptor.bMaxPacketSize    = 8;
    pUsbhDev->sDeviceDescriptor.bLength           = 8;//USB_DT_DEVICE_SIZE;
    pUsbhDev->bActivedQHdIdx                      = 0;
    for (i = 0; i < MAX_NUM_OF_CONFIG; i++)
    {
        pUsbhDev->sConfigDescriptor[i].bLength    = USB_DT_CONFIG_SIZE;
        pUsbhDev->sInterfaceDescriptor[i].bLength = USB_DT_INTERFACE_SIZE;
    }

    for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)
    {
        pUsbhDev->sEndpointDescriptor[i].bLength  = USB_DT_ENDPOINT_SIZE;
        pUsbhDev->dwActivedQHD[i] = 0;
    }
    
    pUsbhDev->hstIntInWorkqTD         = OTGH_NULL;
    pUsbhDev->hstIntInSendLastqTD     = OTGH_NULL;    
    pUsbhDev->hstIntOutWorkqTD         = OTGH_NULL;
    pUsbhDev->hstIntOutSendLastqTD     = OTGH_NULL;   
    pUsbhDev->hstBulkInWorkqTD      = OTGH_NULL;    
    pUsbhDev->hstBulkInSendLastqTD  = OTGH_NULL;    
    pUsbhDev->hstBulkOutWorkqTD     = OTGH_NULL;    
    pUsbhDev->hstBulkOutSendLastqTD = OTGH_NULL;    
    pUsbhDev->psControlqTD          = OTGH_NULL;
    pUsbhDev->hstBulkOutqTD         = OTGH_NULL;
    pUsbhDev->hstBulkInqTD          = OTGH_NULL;
    pUsbhDev->hstInterruptInqTD       = OTGH_NULL;
    pUsbhDev->hstInterruptOutqTD       = OTGH_NULL;
#if (USBOTG_HOST_ISOC == ENABLE)
    pUsbhDev->dwTotalPageNumber = 0;
    pUsbhDev->dwPageIndex = 0;
    for (i= 0; i < Host20_Page_MAX; i++)
        pUsbhDev->dwPageBuffer[i] = 0;
    pUsbhDev->bIsoInEnable = FALSE;
    pUsbhDev->bIsoOutEnable = FALSE;
#endif
}

void UsbHostFinal(SWORD errCode, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost;

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    MP_DEBUG("UsbHostFinal");
    psHost->boIsReadyToReadWriteUsbh = FALSE;//gIsReadyToReadWriteUsbh = FALSE;
    SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);

    if (UsbOtgHostConnectStatusGet(eWhichOtg) != 0)//mwHost20_PORTSC_ConnectStatus_Rd() != 0)
    { // if device still conectted, than check if to reset device or not
        if (  !(errCode == USB_HOST_DEVICE_PLUG_OUT                      ||
                errCode == USB_HOST_NOT_SUPPORTED_DEVICE))//                 ||
//                errCode == USB_HOST_NOT_FLASH_DISK_EMBEDDED_HUB          ||
//                errCode == USB_HOST_NO_DEVICE_ON_HUB_ERR))
        { // except the condition above, then reset device
            if (psHost->boIsUsbhContinueToResetDevice == TRUE)//gIsUsbhContinueToResetDevice == TRUE)
            {
                psHost->boIsHostToResetDevice = TRUE;//psHost->boIsHostToResetDevice = TRUE;
                //gpUsbhDeviceDescriptor->bDeviceStatus   = USB_STATE_RESET;
                return;
            }
        }
        else
        {
            psHost->boIsUsbhContinueToResetDevice = TRUE;//gIsUsbhContinueToResetDevice = TRUE;
        }
    }
    else
    {
        psHost->dwRetryCnt = 0;//g_RetryCnt = 0;
    }

    //psHost->boIsHostFinalized = TRUE;
}

static void UsbHostInit(WHICH_OTG eWhichOtg)
{
    WORD i = 0;	
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc    = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR psAppDesc     = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR psHub              = (PST_HUB_CLASS_DESCRIPTOR)psHost->psHubClassDescriptor;
    PUSB_HOST_SIDC psSidc                       = &psHost->sSidc;
    

//    UsbhDisableMultiLun();
    MP_DEBUG("UsbHostInit BEGIN");
    //#if USBOTG_HOST_DESC
        //MP_DEBUG("\n!!FREE2!!\n");
        //FreeConfigDescriptor(psUsbhDevDesc); // Main-Free: Re-PlugIn
    //#endif

    UsbhEnableMultiLun(eWhichOtg);
    memset((BYTE*)psUsbhDevDesc, 0, MAX_NUM_OF_DEVICE*sizeof(ST_USBH_DEVICE_DESCRIPTOR));
    memset((BYTE*)psAppDesc, 0, MAX_NUM_OF_DEVICE*sizeof(ST_APP_CLASS_DESCRIPTOR));
    memset((BYTE*)psHub, 0, sizeof(ST_HUB_CLASS_DESCRIPTOR));

    memset((BYTE*)psHost->psClassTaskEnvelope, 0, sizeof(ST_MCARD_MAIL));
    memset((BYTE*)psHost->psDriverTaskEnvelope, 0, sizeof(ST_MCARD_MAIL));
    memset((BYTE*)psHost->psIsrEnvelope, 0, sizeof(ST_MCARD_MAIL));
    memset((BYTE*)psSidc->psPtpEnvelope, 0, sizeof(USBH_PTP_MAIL_TAG));

#if USBOTG_HOST_CDC
    PUSB_HOST_CDC psCdc = &psHost->sCdc;
    memset((BYTE*)psCdc->psCdcEnvelope, 0, sizeof(ST_MCARD_MAIL));
#endif

#if USBOTG_HOST_HID
    PUSB_HOST_HID psHid = &psHost->sHid;
    memset((BYTE*)psHid->psHidEnvelope, 0, sizeof(ST_MCARD_MAIL));
#endif

#if USBOTG_WEB_CAM
    PUSB_HOST_AVDC psWebCam = &psHost->sWebcam;
    memset((BYTE*)psWebCam->psWebCamEnvelope, 0, sizeof(ST_MCARD_MAIL));
	psWebCam->psWebCamEnvelope->wMCardId = USB_HOST_ID1;
#endif
    if (eWhichOtg == USBOTG0)
    {
        // start from 1
        psHost->psClassTaskEnvelope->wMCardId = DEV_USB_HOST_ID1;
        psHost->psDriverTaskEnvelope->wMCardId = DEV_USB_HOST_ID1;

        // for PTP
        psSidc->psPtpEnvelope->wMCardId = DEV_USB_HOST_PTP;
    }
    else if (eWhichOtg == USBOTG1)
    {
        // start from 6
        psHost->psClassTaskEnvelope->wMCardId = DEV_USBOTG1_HOST_ID1;
        psHost->psDriverTaskEnvelope->wMCardId = DEV_USBOTG1_HOST_ID1;
        
        // for PTP
        psSidc->psPtpEnvelope->wMCardId = DEV_USBOTG1_HOST_PTP;
    }
    else
    {
        MP_ALERT("%s USBOTG%d cannot recognized!!", __FUNCTION__, eWhichOtg);
    }

    psAppDesc->bDeviceConfigVal = USBH_CONFIG_VALUE_NOT_DEFINED;
    psHub->sHubDescriptor.bDescLength = USB_DT_HUB_DES_SIZE;//sizeof(gpHubClassDescriptor->sHubDescriptor); 
    psHub->bPortNumber = 1;

    psHost->boIsHostFinalized = FALSE;
    psHost->boIsHostToResetDevice = FALSE;
    psHost->boIsReadyToReadWriteUsbh = FALSE;
    ClearDeviceAddress(eWhichOtg);
    ClearNoNeedToPollingFlag(eWhichOtg);
    UsbOtgHostDeviceDescriptorInit(psUsbhDevDesc);
#if (USBOTG_HOST_ISOC == ENABLE)
    UsbOtgHostIsocQueueInit(eWhichOtg);
#endif
    MP_DEBUG("UsbHostInit END");
}

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
void InitiaizeUsbOtgHost(BOOL isFirstTime, WHICH_OTG eWhichOtg)
{
	DWORD cnt = 0;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_HOST psHost = &psUsbOtg->sUsbHost;

    MP_DEBUG("InitiaizeUsbOtgHost:eWhichOtg = %d", eWhichOtg);
    UsbOtgHostDsInit(psHost);
	UsbOtgMemInit(eWhichOtg);
	UsbHostInit(eWhichOtg);
	if (isFirstTime == TRUE)
    {
        psHost->dwRetryCnt++;
        MP_DEBUG("g_RetryCnt = %d", psHost->dwRetryCnt);
        if (psHost->dwRetryCnt == MAX_USBH_DETECT_RETRY_CNT)
        {
            psHost->dwRetryCnt = 0;
            SystemSetErrEvent(UsbOtgHostDetectErrorCodeGet(eWhichOtg));
            psHost->boIsUsbhContinueToResetDevice = FALSE;
        }
    }

    mbHost20_USBCMD_HCReset_Set();
    
#if SC_USBDEVICE        
            mUsbOtgSoftRstSet();     // soft reset OTG  Device config
            mUsbOtgGlobIntDis();     // Disable Device Global Interrupt
            mUsbGC_MHC_INT_Ena();    // Mask Global control host interrupt
#endif
    
    // Turn On VBUS
    mdwOTGC_Control_A_BUS_DROP_Clr(); //Exit when Current Role = Host
    mdwOTGC_Control_A_BUS_REQ_Set();

    // disable async first
    flib_Host20_Asynchronous_Setting(HOST20_Disable, eWhichOtg);
    flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);

    mwPeri20_Control_HALFSPEEDEnable_Clr(); // must mask or IOC not occurs
    flib_Host20_ForceSpeed(0, psUsbOtg);
	flib_Host20_InitStructure(eWhichOtg);
    
#if USBOTG_HOST_DATANG
	// Datang 3G USB Modem don't need interrupt
	PST_USBH_DEVICE_DESCRIPTOR   pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	if(!( pUsbhDevDes->sDeviceDescriptor.idVendor == (WORD)byte_swap_of_word(TDSCDMA_USB_DEVICE_VID) && 
		pUsbhDevDes->sDeviceDescriptor.idProduct == (WORD)byte_swap_of_word(TDSCDMA_USB_PID)))
#endif
	{    
		// Enable interrupts 
		EnableUsbOtgHostInterrupt(eWhichOtg);
	}

    MP_DEBUG("InitiaizeUsbOtgHost END");
}
#else //((STD_BOARD_VER == MP650_FPGA)||(STD_BOARD_VER == MP652_216LQFP_32M_DDR))
/*
void InitiaizeUsbOtgHost(BOOL isFirstTime, WHICH_OTG eWhichOtg)
{
    DWORD cnt = 0;
    
    //__asm("break 100");
#if SC_USBDEVICE        
    mUsbOtgSoftRstSet();     // soft reset OTG  Device config
    mUsbOtgGlobIntDis();     // Disable Device Global Interrupt
    mUsbGC_MHC_INT_Ena();    // Mask Global control host interrupt
#endif
    if (isFirstTime == TRUE)
    {
        UsbOtgMemInit();
        UsbHostInit();
    //}
    //else
    //{
        g_RetryCnt++;
        MP_DEBUG("g_RetryCnt = %d", g_RetryCnt);
        if (g_RetryCnt == MAX_USBH_DETECT_RETRY_CNT)
        {
            g_RetryCnt = 0;
            SystemSetErrEvent(ERR_USBH_DETECT_ERROR);
            gIsUsbhContinueToResetDevice = FALSE;
        }
    }

    mbHost20_USBCMD_HCReset_Set();
    
// suspend PHY clock and enable again to let PHY clock 30MHz work for sure
    mwHost20_Misc_Physuspend_Set();
    mwOTG20_Control_DeviceDmaControllerParameterSetting1_Set();
    IODelay(1);
    mwHost20_Misc_Physuspend_Clr();
    mwOTG20_Control_DeviceDmaControllerParameterSetting1_Clr();
    
    mwOTG20_Wrapper_OWAP_CFG_Set();
    mwOTG20_Wrapper_OWAP_CFG_Clr();
    // Turn On VBUS
    MP_DEBUG("InitiaizeUsbOtgHost ");
   // __asm("break 100");
    mdwOTGC_Control_A_BUS_DROP_Clr(); //Exit when Current Role = Host
    mdwOTGC_Control_A_BUS_REQ_Set();

    // disable async first
    flib_Host20_Asynchronous_Setting(HOST20_Disable, eWhichOtg);
    flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);

    mwPeri20_Control_HALFSPEEDEnable_Clr();
    flib_Host20_ForceSpeed(0);
    flib_Host20_InitStructure();
    EnableUsbOtgHostInterrupt();
    MP_DEBUG("InitiaizeUsbOtgHost END");
}
*/
#endif //((STD_BOARD_VER == MP650_FPGA)||(STD_BOARD_VER == MP652_216LQFP_32M_DDR))

//============================================================================= ok
//		OTGC_A_Bus_Drive()
//		Description:
//		input: none
//		output: none
//=============================================================================
static void OTGC_A_Bus_Drive(PST_USB_OTG_DES psUsbOtg)
{
    mdwOTGC_Control_A_BUS_DROP_Clr(); //Exit when Current Role = Host
    mdwOTGC_Control_A_BUS_REQ_Set();
}

//============================================================================= ok
//		OTGC_A_Bus_Drop()
//		Description:It will Drop the VBUS
//                  <1>.Step1:Close the OTG-Host
//                  <2>.Step2:Set Host function State to 'Reset Mode'
//			        <3>.Step3:Set register 'A_BUS_DROP'
//		input: none
//		output: none
//=============================================================================
static void OTGC_A_Bus_Drop(PST_USB_OTG_DES psUsbOtg)
{
   //flib_Host20_Close(0);

   mdwOTGC_Control_A_BUS_REQ_Clr();
   mdwOTGC_Control_A_BUS_DROP_Set(); //Exit when Current Role = Host 
     
   //Reset Phy
   //OTGC_A_PHY_Reset();		//YPING mark
}

static void EnableUsbOtgHostInterrupt(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    MP_DEBUG("-USBOTG%d- %s", eWhichOtg, __FUNCTION__); 
    mwHost20_USBINTR_Set(HOST20_USBINTR_ENABLE);
    MP_DEBUG("EnableUsbOtgHostInterrupt:IntCause = 0x%x", UsbOtgIntCauseGet(eWhichOtg));
    SystemIntEna(UsbOtgIntCauseGet(eWhichOtg));    
}

void DisableUsbOtgHostInterrupt(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg;
    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    MP_DEBUG("-USBOTG%d- %s", eWhichOtg, __FUNCTION__);   
    mwHost20_USBINTR_Set(0);
    SystemIntDis(UsbOtgIntCauseGet(eWhichOtg));//IC_USBOTG);
}


SDWORD SendMailToUsbOtgHostClassTask(DWORD dwEnvelopeAddr, WHICH_OTG eWhichOtg)
{
    SDWORD          osSts;
    BYTE            mail_id;
    BYTE bMailBoxId = UsbOtgHostMailBoxIdGet(eWhichOtg);
    
    osSts = MailboxSend(bMailBoxId,
                        (BYTE *)dwEnvelopeAddr,
                        sizeof(ST_MCARD_MAIL),
                        &mail_id);
    if (osSts != OS_STATUS_OK)
    {
        MP_DEBUG("MailboxSend Failed:task mail ID = %d; dwEnvelopeAddr = 0x%x; mail_id = %d",\
                 bMailBoxId, dwEnvelopeAddr, mail_id);
    }

    if (dwEnvelopeAddr == GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg))
    {
        ;//MailTrack(mail_id);
    }

    return osSts;
}

SDWORD SendMailTrackToUsbOtgHostClassTask(DWORD dwEnvelopeAddr, WHICH_OTG eWhichOtg)
{
    SDWORD          osSts;
    BYTE            mail_id;
    BYTE bMailBoxId = UsbOtgHostMailBoxIdGet(eWhichOtg);
    
    osSts = MailboxSend(bMailBoxId,
                        (BYTE *)dwEnvelopeAddr,
                        sizeof(ST_MCARD_MAIL),
                        &mail_id);

    MailTrack(mail_id);
    
    if (osSts != OS_STATUS_OK)
    {
        MP_DEBUG("MailboxSend Failed:task mail ID = %d; dwEnvelopeAddr = 0x%x; mail_id = %d",\
                 bMailBoxId, dwEnvelopeAddr, mail_id);
    }

    if (dwEnvelopeAddr == GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg))
    {
        ;//MailTrack(mail_id);
    }

    return osSts;
}

void UsbOtgHostClose(WHICH_OTG eWhichOtg)
{
    // clear memory
    DWORD *pwData;
    DWORD dwTotalMemorySize = 0, wTemp;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

    pwData = (DWORD*)psEhci->dwHostStructureBaseAddress;
    memset((BYTE*)pwData, 0, dwTotalMemorySize);

    UsbOtgHostSetSwapBuffer1RangeDisable(eWhichOtg);
    free_all_mem(eWhichOtg);

    //<1>.Suspend Host
   // flib_Host20_Suspend();

    //<2>.Disable the interrupt
    mwHost20_USBINTR_Set(0);

    //<3>.Clear Interrupt Status
    wTemp=mwHost20_USBSTS_Rd();
    wTemp=wTemp&0x0000003F;
    mwHost20_USBSTS_Set(wTemp);

    OTGC_A_Bus_Drop(psUsbOtg);
    OTGC_A_Bus_Drive(psUsbOtg);
}

// 
//1. if device is belong to msdc or sidc then will return USBOTG0/USBOTG1
//   to call the CommandProcess from UsbOtgHostClassTask.
//2. if device is belong to others class which has its own CommandProcess
//   being called from DeviceTask then will return USBOTG_NONE.
// 
WHICH_OTG GetWhichUsbOtgByCardId (WORD wCardId)
{
    WHICH_OTG eWhichOtg = USBOTG_NONE;
        
    switch (wCardId)
    {
        case DEV_USB_HOST_ID1:
        case DEV_USB_HOST_ID2:
        case DEV_USB_HOST_ID3:
        case DEV_USB_HOST_ID4:
        case DEV_USB_HOST_PTP:
            eWhichOtg = USBOTG0;
            break;
            
        case DEV_USBOTG1_HOST_ID1:
        case DEV_USBOTG1_HOST_ID2:
        case DEV_USBOTG1_HOST_ID3:
        case DEV_USBOTG1_HOST_ID4:
        case DEV_USBOTG1_HOST_PTP:
            eWhichOtg = USBOTG1;
            break;
            
        case DEV_USB_WEBCAM:
            eWhichOtg = WEBCAM_USB_PORT;
            break;
            
        case DEV_USB_WIFI_DEVICE:
        case DEV_USB_ETHERNET_DEVICE:
        default:
            eWhichOtg = USBOTG_NONE;
            break;
    }
    
    return eWhichOtg;
}

/*
WHICH_OTG GetWhichUsbOtgByCardId (WORD wCardId)
{
    if (!( wCardId == DEV_USB_HOST_ID1     ||
           wCardId == DEV_USB_HOST_ID2     ||
           wCardId == DEV_USB_HOST_ID3     ||
           wCardId == DEV_USB_HOST_ID4     ||
           wCardId == DEV_USB_HOST_PTP     ||
           wCardId == DEV_USBOTG1_HOST_ID1 ||
           wCardId == DEV_USBOTG1_HOST_ID2 ||
           wCardId == DEV_USBOTG1_HOST_ID3 ||
           wCardId == DEV_USBOTG1_HOST_ID4 ||
           wCardId == DEV_USBOTG1_HOST_PTP ||
           wCardId == DEV_USB_WIFI_DEVICE  ||
           wCardId == DEV_USB_ETHERNET_DEVICE ||
           wCardId == DEV_USB_WEBCAM
        ))
    {
       return USBOTG_NONE;
    }
    else if (!( wCardId == DEV_USB_HOST_ID1     ||
                wCardId == DEV_USB_HOST_ID2     ||
                wCardId == DEV_USB_HOST_ID3     ||
                wCardId == DEV_USB_HOST_ID4     ||
                wCardId == DEV_USB_HOST_PTP     ||
                wCardId == DEV_USB_WIFI_DEVICE  ||
                wCardId == DEV_USB_ETHERNET_DEVICE ||
                wCardId == DEV_USB_WEBCAM
             ))
    {
        return USBOTG1;
    }
    else
    {
        WHICH_OTG eWhichOtg;
            
        switch (wCardId)
        {
            case DEV_USB_WEBCAM:
                eWhichOtg = WEBCAM_USB_PORT;
                break;
            case DEV_USB_WIFI_DEVICE:
            case DEV_USB_ETHERNET_DEVICE:
                eWhichOtg = WIFI_USB_PORT;
                break;
            default:
                eWhichOtg = USBOTG0;
                break;
        }

        return eWhichOtg;
    }
}
*/
BYTE GetMailBoxIdbyCardId (WORD wCardId)
{
    if ((wCardId == DEV_USB_HOST_ID1) ||
        (wCardId == DEV_USB_HOST_ID2) ||
        (wCardId == DEV_USB_HOST_ID3) ||
        (wCardId == DEV_USB_HOST_ID4) ||
        (wCardId == DEV_USB_HOST_PTP))
    {
        return USBOTG0_HOST_CLASS_MAIL_ID;
    }
    else if ((wCardId == DEV_USBOTG1_HOST_ID1) ||
             (wCardId == DEV_USBOTG1_HOST_ID2) ||
             (wCardId == DEV_USBOTG1_HOST_ID3) ||
             (wCardId == DEV_USBOTG1_HOST_ID4) ||
             (wCardId == DEV_USBOTG1_HOST_PTP))
    {
        return USBOTG1_HOST_CLASS_MAIL_ID;
    }
    else
        return 0;
}

BYTE GetLunbyCardId (WORD wCardId)
{
    if ((wCardId == DEV_USB_HOST_ID1)||(wCardId == DEV_USBOTG1_HOST_ID1))
        return LUN_0;
    else if ((wCardId == DEV_USB_HOST_ID2)||(wCardId == DEV_USBOTG1_HOST_ID2))
        return LUN_1;
    else if ((wCardId == DEV_USB_HOST_ID3)||(wCardId == DEV_USBOTG1_HOST_ID3))
        return LUN_2;
    else if ((wCardId == DEV_USB_HOST_ID4)||(wCardId == DEV_USBOTG1_HOST_ID4))
        return LUN_3;
    else
        return LUN_NONE;
}


#if TWICE_READ_COMPARE
void TwiceReadCompare(ST_MCARD_MAIL   *psMcardRMail, BYTE bLun, WHICH_OTG eWhichOtg)
{
	SWORD     swReadStatus = 0;
	DWORD*   dwReadBuf;
	DWORD     dwCnt;
	DWORD     dwTotalByte = 0;

	dwTotalByte = psMcardRMail->dwBlockCount*512;
	if(ker_mem_get_free_space() < dwTotalByte)
	{
		MP_ALERT("-E- %s ker_mem is not enough 0x%x < 0x%x", __FUNCTION__, ker_mem_get_free_space(), dwTotalByte); 
		__asm("break 100");
	}
	else
	{
		// Alloc new buffer		
		dwReadBuf = (DWORD*)ker_mem_malloc(dwTotalByte);
		if(dwReadBuf == NULL) 
		{
			MP_ALERT("-E- %s Can't get kernel memory", __FUNCTION__);
			__asm("break 100");
		}
		
		//MP_ALERT("-I- Buffer Pointer from USB Driver 0x%x", dwCalBuf);	
		swReadStatus = UsbhMsdcReadData ( bLun,
                                                    psMcardRMail->dwBlockAddr,
                                                    psMcardRMail->dwBlockCount,
                                                    (DWORD)dwReadBuf, // psMcardRMail->dwBuffer,
                                                    eWhichOtg);
		
		// Compare test
		for(dwCnt = 0; dwCnt < dwTotalByte; dwCnt++)
		{

			if(*((BYTE*)dwReadBuf+dwCnt) != *((BYTE*)psMcardRMail->dwBuffer+dwCnt))
			{
				MP_ALERT("-E- Buffer of FS and Driver is different in Cnt %d/%d!!", dwCnt, dwTotalByte);
				MP_ALERT("-I- Buffer Pointer from FS 0x%x", psMcardRMail->dwBuffer);
				MP_ALERT("-I- Buffer Pointer from USB Driver 0x%x", dwReadBuf);	
				MP_ALERT("-E- FS 0x%x", *((BYTE*)psMcardRMail->dwBuffer+dwCnt));
				MP_ALERT("-E- Driver 0x%x", *((BYTE*)dwReadBuf+dwCnt));
				__asm("break 100");
			}
			/*
			else
			{
				MP_ALERT("-I- Buffer of FS and Driver is the same in Cnt %d/%d!!", dwCnt, dwTotalByte);
				MP_ALERT("-I- FS 0x%x", *((BYTE*)psMcardRMail->dwBuffer+dwCnt));
				MP_ALERT("-I- Driver 0x%x", *((BYTE*)dwCalBuf+dwCnt));				
			}
			*/
		}
		ker_mem_free(dwReadBuf);
		UartOutText("$");			
	}

	if(swReadStatus == FAIL)
		MP_ALERT("-E- %s swStatus %d swCalStatus %d", swReadStatus);
}
#endif

static void CommandProcess (void *pMcardDev)
{
    ST_MCARD_MAIL   *psMcardRMail;
    ST_MCARD_DEV    *pDev = pMcardDev;
    WHICH_OTG       eWhichOtg;
    BYTE            bLun;
    
    psMcardRMail = pDev->sMcardRMail;
    eWhichOtg = GetWhichUsbOtgByCardId(pDev->wMcardType);
    if (eWhichOtg == USBOTG_NONE)
    {
       return;
    }

    bLun = GetLunbyCardId(pDev->wMcardType);
	switch (psMcardRMail->wCmd)
	{
		case INIT_CARD_CMD:
            MP_DEBUG("-USBOTG%d- %s INIT_CARD_CMD", eWhichOtg, __FUNCTION__);
			if (pDev->Flag.Detected == 0)
			{
				MP_DEBUG("-USBOTG%d- %s msdc card in", eWhichOtg, __FUNCTION__);
				//card in
				pDev->Flag.Detected = 1;
				pDev->wRenewCounter ++;
				pDev->Flag.Present  = 1;
				//EventSet(UI_EVENT, EVENT_CARD_INIT);                  
				if ((pDev->wMcardType == DEV_USB_HOST_PTP)||(pDev->wMcardType == DEV_USBOTG1_HOST_PTP))
				{
					pDev->Flag.ReadOnly = 1;  // PTP is read only.
				}
			}
			else
			{
				MP_DEBUG("-USBOTG%d- %s msdc card out", eWhichOtg, __FUNCTION__);
				//card out
				//pDev->Flag.Installed     = 0; //Joe 3/21: virtual lun detect error
				pDev->Flag.Detected     = 0;
				pDev->Flag.Present      = 0; 
				pDev->Flag.ReadOnly     = 0; 
				pDev->Flag.PipeEnable   = 0; 
				pDev->swStatus          = 0;
				pDev->dwCapacity        = 0;
				pDev->wSectorSize       = 0;
				//EventSet(UI_EVENT, EVENT_CARD_INIT);                  
			}
			break;

		case REMOVE_CARD_CMD: //Athena 03.11.2006 seperate card in & out
            MP_DEBUG("-USBOTG%d- %s REMOVE_CARD_CMD", eWhichOtg, __FUNCTION__);
			//card out
			//pDev->Flag.Installed     = 0;//Joe 3/21: virtual lun detect error
			pDev->Flag.Detected     = 0;
			pDev->Flag.Present      = 0; 
			pDev->Flag.ReadOnly     = 0; 
			pDev->Flag.PipeEnable   = 0; 
			pDev->swStatus          = 0;
			pDev->dwCapacity        = 0;
			pDev->wSectorSize       = 0;
            SetUsbhCardPresentFlag(0, bLun, eWhichOtg);
			//pAppDes->dPresent[bLun] = 0;
			//EventSet(UI_EVENT, EVENT_CARD_INIT); 
			break;


		case SECTOR_READ_CMD:
		{
#if USBOTG_HOST_PTP
			if ((pDev->wMcardType == DEV_USB_HOST_PTP) || (pDev->wMcardType == DEV_USBOTG1_HOST_PTP))
				pDev->swStatus = UsbOtgHostSidcReadData (   psMcardRMail->dwBlockAddr,
        													psMcardRMail->dwBlockCount,
        													psMcardRMail->dwBuffer,
        													psMcardRMail->dwObjectHandle,
        													eWhichOtg);
			else
#endif	
////////////////////////////////////////////
//  BEGIN : test Write -> Read -> Compare //
////////////////////////////////////////////
#if USBOTG_HOST_TEST
UsbOtgHostTestFunction( (psMcardRMail->wMCardId-1),
                        psMcardRMail->dwBlockAddr,
                        psMcardRMail->dwBlockCount,
                        psMcardRMail->dwBuffer,
                        eWhichOtg);
#endif
////////////////////////////////////////////
//    END : test Write -> Read -> Compare //
////////////////////////////////////////////
                pDev->swStatus = UsbhMsdcReadData ( bLun,
                                                    psMcardRMail->dwBlockAddr,
                                                    psMcardRMail->dwBlockCount,
                                                    psMcardRMail->dwBuffer,
                                                    eWhichOtg);

		#if TWICE_READ_COMPARE
		TwiceReadCompare(psMcardRMail, bLun, eWhichOtg);
		#endif
        }                
        break;
            
        case SECTOR_WRITE_CMD:
        {
#if USBOTG_HOST_PTP
            if (pDev->wMcardType == DEV_USB_HOST_PTP)
                pDev->swStatus = FAIL;
            /*
                pDev->swStatus = UsbOtgHostSidcWriteData (    (psMcardRMail->wMCardId-1),
                                                        psMcardRMail->dwBlockAddr,
                                                        psMcardRMail->dwBlockCount,
                                                        psMcardRMail->dwBuffer,
                                                        pUsbhDevDes,
                                                        pAppDes);
            */
            else
#endif				
                pDev->swStatus = UsbhMsdcWriteData (    bLun,
                                                        psMcardRMail->dwBlockAddr,
                                                        psMcardRMail->dwBlockCount,
                                                        psMcardRMail->dwBuffer,
                                                        eWhichOtg);
        }                
        break;

        case RAW_FORMAT_CMD:
            //pDev->swStatus = Format();
            MP_DEBUG("Not Support!!");
            break;

//#if USBOTG_WEB_CAM
#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )

		case WEB_CAM_START_CMD:
			MP_DEBUG("WEB_CAM_START_CMD");
			webCamStart(eWhichOtg);
		break;

		case  WEB_CAM_STOP_CMD:
			MP_DEBUG("WEB_CAM_STOP_CMD");
			webCamStop(eWhichOtg);
		break;

		case WEB_CAM_GET_VFRAM_CMD:
			MP_DEBUG("WEB_CAM_GET_VFRAM_CMD");
			pDev->swStatus = webCamVideoGetStreamBuffer( psMcardRMail->dwBuffer,eWhichOtg);
		break;

		case WEB_CAM_GET_AFRAM_CMD:
			MP_DEBUG("WEB_CAM_GET_AFRAM_CMD");
			pDev->swStatus = webCamAudioGetStreamBuffer( psMcardRMail->dwBuffer,eWhichOtg);
		break;

#endif

        default:
            MP_ALERT("-E- INVALID CMD %x", psMcardRMail->wCmd);
            break;
    }
    if (pDev->swStatus != USB_NO_ERROR)
    {
        MP_DEBUG1("sts:%x", pDev->swStatus);
        MP_DEBUG2("LBA=0x%x LEN=0x%x fail", psMcardRMail->dwBlockAddr, psMcardRMail->dwBlockCount);
        pDev->swStatus = FAIL;
    }
}



//====================================================================
// * Function Name: UsbOtgHostIsr//flib_Host20_ISR                          
// * Description: 
//   <1>.Read the Interrupt Status
//   <2>.Checking for the Error type interrupt => Halt the system
//   <3>.Check Port Change Status(Resume/Connect Status Change/Port Enable-Disable Change)
//       <3.1>.Process Event-Resume & Clear Status
//       <3.2>.Process Event-ConnectStatusChange & Clear Status
//       <3.3>.Process Event-PortEnableDisableChange & Clear Status
//       <3.4>.Clear Port Change Status 
//       <3.5>.Process the Event of Port Reset
//   <4>.Check Complete Interrupt
// * Input: 
// * OutPut: 
//====================================================================
BYTE UsbOtgHostIsr(WHICH_OTG eWhichOtg)
{
    DWORD wIntStatus;
   // static DWORD chk_count = 0;//psHost->dwChkCount
    PST_USB_OTG_DES psUsbOtg;
    PST_USB_OTG_HOST psHost;
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDes;
    PUSB_HOST_EHCI psEhci;
    BYTE bDriverEventId = 0;
    BYTE bTxDoneEventId = 0;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    psHost = &psUsbOtg->sUsbHost;
    psUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = &psHost->sEhci;
    
    bDriverEventId = UsbOtgHostDriverEventIdGet(eWhichOtg);
    bTxDoneEventId = UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg);

    //<1>.Read the Interrupt data
    wIntStatus=mwHost20_USBSTS_Rd();

    //<4>.Check Complete Interrupt
    if (wIntStatus&HOST20_USBINTR_CompletionOfTransaction) 
    {
        DWORD working_qHD = 0;
        mwHost20_USBSTS_CompletionOfTransaction_Set();
        EventSet(bDriverEventId, EVENT_EHCI_IOC);
    }

    //<2>.Checking for the Error type interrupt => Halt the system
    if (wIntStatus&HOST20_USBINTR_SystemError) {
      MP_ALERT("--E-- %s USBOTG%d System Error... Halt the system...into umlimited while loop!", __FUNCTION__, eWhichOtg);
      while(1);
    }

    if (wIntStatus&HOST20_USBINTR_USBError) {
      mwHost20_USBSTS_USBError_Set();
      psUsbhDevDes->bSendStatusError=1;
      //MP_ALERT("--E-- %s USBOTG%d HOST20_USBINTR_USBError! 0x%x", __FUNCTION__, eWhichOtg, wIntStatus);
    }

    //<3>.Check Port Change Status
    if (wIntStatus&HOST20_USBINTR_PortChangeDetect)
    {         
        int goto_clr = 0;

        psHost->dwChkCount++;
        if (psHost->dwChkCount > 10000)
        {
                psHost->dwChkCount = 0;
                goto_clr = 1;
            //<3.1>.Process Event-Resume & Clear Status
            if (mwHost20_PORTSC_ForceResume_Rd()>0)
            {      
                psUsbhDevDes->bRemoteWakeUpDetected=1;
            }
            //<3.2>.Process Event-ConnectStatusChange & Clear Status
            if (mwHost20_PORTSC_ConnectChange_Rd())
            {
                mwHost20_PORTSC_ConnectChange_Set();
                if (mwHost20_PORTSC_ConnectStatus_Rd()==0)
                {      
                    //MP_ALERT("--E-- %s USBOTG%d event set EVENT_DEVICE_PLUG_OUT", __FUNCTION__, eWhichOtg);
                    flib_Host20_StopRun_Setting(HOST20_Disable, eWhichOtg);
                    psUsbhDevDes->bConnectStatus=USB_STATUS_DISCONNECT;
                    EventSet(bDriverEventId, EVENT_DEVICE_PLUG_OUT);
                }
                else
                {
                    psUsbhDevDes->bConnectStatus=USB_STATUS_CONNECT;
                    psUsbhDevDes->bDeviceStatus = USB_STATE_ATTACHED;                
                    EventSet(bDriverEventId, EVENT_DEVICE_PLUG_IN);
                }
            }

            //<3.3>.Process Event-PortEnableDisableChange & Clear Status
            if (mwHost20_PORTSC_EnableDisableChange_Rd())
            {
                mwHost20_PORTSC_EnableDisableChange_Set();
            }

            //<3.4>.Process the Event of Port Reset
            if (psUsbhDevDes->bPortReset==1) 
            {//During Port Reset
                if (mwHost20_PORTSC_PortReset_Rd()==0)
                {
                    psUsbhDevDes->bPortReset=0;
                }
            }

            //<3.5>.Process the Event of Suspend
            if (psUsbhDevDes->bSuspend==0) 
            {//During Port Reset
                if (mwHost20_PORTSC_ForceSuspend_Rd()==1)
                {
                    psUsbhDevDes->bSuspend=1;
                }
            }
        }
        else
        {
            goto_clr = 0;
        }
        
        if (goto_clr == 1)
        {
            goto_clr = 0;
            //<3.9>.Clear Status of Port Status 
            MP_DEBUG("<Clr>");
            mwHost20_USBSTS_PortChangeDetect_Set();
        }
    }

    //<5>.Checking for frameList = 0 interrupt
    if (wIntStatus&HOST20_USBINTR_FrameRollover)
    {
        mwHost20_USBSTS_FrameRollover_Set();
        if (IsNoNeedToPolling(eWhichOtg) == FALSE)
        {
            MP_DEBUG(";");
            // gwFrameList0Interrupt++;
            if (CMP_START_TIMER == GetCheckMediaPresentTimerFlag(eWhichOtg))
            {
                SetCheckMediaPresentTimerFlag(CMP_STOP_TIMER, eWhichOtg);
                EventSet(bDriverEventId, EVENT_POLLING_EACH_LUN);            
            }
        }
    }

    return (1);
}

static void flib_Host20_ForceSpeed(BYTE bSpeedType, PST_USB_OTG_DES psUsbOtg)
{ 
    switch(bSpeedType)
    {
        case 0:
            mwHost20_Control_ForceFullSpeed_Clr(); 	 
            mwHost20_Control_ForceHighSpeed_Clr();         
        break;
        case 1:
            mwHost20_Control_ForceFullSpeed_Set(); 	 
            mwHost20_Control_ForceHighSpeed_Clr();         
        break;
        case 2:
            mwHost20_Control_ForceFullSpeed_Clr();
            mwHost20_Control_ForceHighSpeed_Set();         
        break; 	
        default:
            MP_DEBUG(">>>Force Speed Input Type Error... \n");
        break;
    }
}

//DWORD g_HD_TD_buffer_index = 0;
//DWORD g_HD_Base_buffer_index = 0;

void UsbOtgHostGetQhdForEachEd(WHICH_OTG eWhichOtg)
{
    DWORD i,j,k;
    WORD wSize;
    BYTE  bInterruptNum,x;
    BYTE *pData;
    WORD wForceInterval = 0;
    WORD wMaxPacketSize = 0 ;
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    if (pUsbhDev->bDeviceStatus != USB_STATE_CONFIGURED)
    { // for control pipe only at beginning
        pUsbhDev->pstControlqHD[0] = (qHD_Structure*)(psEhci->dwHostStructureQhdBaseAddress);
        pUsbhDev->pstControlqHD[1] = (qHD_Structure*)(psEhci->dwHostStructureQhdBaseAddress+Host20_qHD_SIZE*1);
        flib_Host20_Allocate_QHD (  pUsbhDev->pstControlqHD[0],
                                    HOST20_HD_Type_QH,
                                    0,//address
                                    1,//head
                                    0,//Endpt
                                    pUsbhDev->sDeviceDescriptor.bMaxPacketSize, //Size
                                    eWhichOtg
                                  );
        flib_Host20_Allocate_QHD (  pUsbhDev->pstControlqHD[1],
                                    HOST20_HD_Type_QH,
                                    1,//address will be changed in SETUP_SET_ADDRESS_DONE_STATE
                                    0,//head
                                    0,//Endpt
                                    pUsbhDev->sDeviceDescriptor.bMaxPacketSize,//Size,
                                    eWhichOtg
                                  );
        
        pUsbhDev->pstControlqHD[0]->bNextQHDPointer=(((DWORD)pUsbhDev->pstControlqHD[1])>>5);
        pUsbhDev->pstControlqHD[1]->bNextQHDPointer=(((DWORD)pUsbhDev->pstControlqHD[0])>>5);	
        
        psEhci->dwHdTdBufferIndex = psEhci->dwHostQhdQtdHandleBaseAddress;//ok
        psEhci->dwHdBaseBufferIndex = psEhci->dwHostStructureQhdBaseAddress+Host20_qHD_SIZE*2;
    }
    else
    { // for all pipes after getting interface descriptor
        qHD_Structure *pstCurrentqHDTemp    = OTGH_NULL;
        qHD_Structure *pstPreviousqHDTemp   = OTGH_NULL;
        qHD_Structure *pstBulkqHDTemp       = OTGH_NULL;

#if USBOTG_HOST_DESC
        if (GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, pUsbhDev->bDeviceInterfaceIdx, AlterSettingDefaultIdx) > (Host20_qHD_MAX-2))
#else
        if (pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints > (Host20_qHD_MAX-2))
#endif
        {
            MP_DEBUG("Total Number of Endpoints is over 20!!");
            return;
        }

        pUsbhDev->bMaxBulkOutEpNumber = 0;
        pUsbhDev->bMaxBulkInEpNumber = 0;
        pUsbhDev->bMaxInterruptInEpNumber = 0;
        pUsbhDev->bMaxInterruptOutEpNumber = 0;

#if USBOTG_HOST_DESC
        BYTE bNumberOfIsocEp = 0;
    	for ( i = 0; i < GetConfigNumInterface(pUsbhDev, pUsbhDev->bDeviceConfigIdx); i++ ) {
         for (k = 0; k <  pUsbhDev->pConfigDescriptor->pInterface[i].max_altsetting; k++) {
    		for ( j = 0; j < GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, k); j++ ) {
	            switch(GetEndpointAttributes(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, k, j) & 0x03)
	            {
	                case OTGH_ED_BULK:
	                {
	                    BYTE direction;
	                    direction = ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, k, j))&USB_ENDPOINT_DIR_MASK);
	                    if (USB_DIR_OUT == direction)
	                    {
	                        pUsbhDev->bMaxBulkOutEpNumber++;
	                    }
	                    else if (USB_DIR_IN == direction)
	                    {
	                        pUsbhDev->bMaxBulkInEpNumber++;
	                    }
	                    else
	                    {
	                        MP_DEBUG("Bulk unknow dir!!!");
	                    }
	                }
	                break;

	                case OTGH_ED_INT:
	                {
	                    BYTE direction;
	                    direction = ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, k, j))&USB_ENDPOINT_DIR_MASK);
	                    if (USB_DIR_OUT == direction)
	                    {
	                        pUsbhDev->bMaxInterruptOutEpNumber++;
	                    }
	                    else if (USB_DIR_IN == direction)
	                    {
	                        pUsbhDev->bMaxInterruptInEpNumber++;
	                    }
	                    else
	                    {
	                        MP_DEBUG("Int unknow dir!!!");
	                    }
	                }
	                break;

	                case OTGH_ED_ISO:
	                {
	                    BYTE direction;
	                    direction = ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, k, j))&USB_ENDPOINT_DIR_MASK);
	                    if (USB_DIR_OUT == direction)
	                    {
	                        pUsbhDev->bMaxIsoOutEpNumber++;
                                MP_DEBUG("bMaxIsoOutEpNumber %x", pUsbhDev->bMaxIsoOutEpNumber);
	                    }
	                    else if (USB_DIR_IN == direction)
	                    {
	                        pUsbhDev->bMaxIsoInEpNumber++;
                                MP_DEBUG("bMaxIsoInEpNumber %x", pUsbhDev->bMaxIsoInEpNumber);
	                    }
	                    else
	                    {
	                        MP_ALERT("Iso unknow dir!!!");
	                    }
	                }
	                break;

	                default:
	                break;
    	            } // switch(bmAttributes)
        	} // for loop : bNumEndpoints
            } // for loop : max_altsetting

            if (bNumberOfIsocEp < (pUsbhDev->bMaxIsoOutEpNumber+pUsbhDev->bMaxIsoInEpNumber))
            {
                pUsbhDev->bTotalNumberOfInterfaceWithIsocEp++;
	    }
            
            bNumberOfIsocEp = (pUsbhDev->bMaxIsoOutEpNumber+pUsbhDev->bMaxIsoInEpNumber);
    	} // for loop : bNumInterfaces
#else // USBOTG_HOST_DESC
        for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)
        {
            switch(pUsbhDev->sEndpointDescriptor[i].bmAttributes & 0x03)
            {
                case OTGH_ED_BULK:
                {
                    BYTE direction;
                    direction = ((pUsbhDev->sEndpointDescriptor[i].bEndpointAddress)&USB_ENDPOINT_DIR_MASK);
                    if (USB_DIR_OUT == direction)
                    {
                        pUsbhDev->bMaxBulkOutEpNumber++;
                    }
                    else if (USB_DIR_IN == direction)
                    {
                        pUsbhDev->bMaxBulkInEpNumber++;
                    }
                    else
                    {
                        MP_DEBUG("Bulk unknow dir!!!");
                    }
                }
                break;

                case OTGH_ED_INT:
                {
                    BYTE direction;
                    direction = ((pUsbhDev->sEndpointDescriptor[i].bEndpointAddress)&USB_ENDPOINT_DIR_MASK);
                    if (USB_DIR_OUT == direction)
                    {
                        pUsbhDev->bMaxInterruptOutEpNumber++;
                    }
                    else if (USB_DIR_IN == direction)
                    {
                        pUsbhDev->bMaxInterruptInEpNumber++;
                    }
                    else
                    {
                        MP_DEBUG("Interrupt unknow dir!!!");
                    }
                }
                break;

                case OTGH_ED_ISO:
                {
                    BYTE direction;
                    direction = ((pUsbhDev->sEndpointDescriptor[i].bEndpointAddress)&USB_ENDPOINT_DIR_MASK);
                    if (USB_DIR_OUT == direction)
                    {
                        pUsbhDev->bMaxIsoOutEpNumber++;
                        MP_DEBUG("bMaxIsoOutEpNumber %x", pUsbhDev->bMaxIsoOutEpNumber);
                    }
                    else if (USB_DIR_IN == direction)
                    {
                        pUsbhDev->bMaxIsoInEpNumber++;
                        MP_DEBUG("bMaxIsoInEpNumber %x", pUsbhDev->bMaxIsoInEpNumber);
                    }
                    else
                    {
                        MP_ALERT("Iso unknow dir!!!");
                    }
                }
                break;

                default:
                break;
            }
        }
#endif // USBOTG_HOST_DESC
//__asm("break 100");
        BYTE direction;
        BYTE total_bulk_number = 0;
        total_bulk_number = pUsbhDev->bMaxBulkInEpNumber + pUsbhDev->bMaxBulkOutEpNumber;
        if (total_bulk_number > 0)
        {
            //pUsbhDev->hstBulkOutqHD = (qHD_Structure**)gHost20_qHD_qTD_HANDLE_BASE_ADDRESS;
            pUsbhDev->hstBulkOutqHD         = (qHD_Structure**)psEhci->dwHdTdBufferIndex;
            pUsbhDev->hstBulkOutqTD         = (qTD_Structure**)((DWORD)pUsbhDev->hstBulkOutqHD+\
                                                        (sizeof(qHD_Structure*)*pUsbhDev->bMaxBulkOutEpNumber));
            pUsbhDev->hstBulkInqHD          = (qHD_Structure**)((DWORD)pUsbhDev->hstBulkOutqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxBulkOutEpNumber));
            pUsbhDev->hstBulkInqTD          = (qTD_Structure**)((DWORD)pUsbhDev->hstBulkInqHD+\
                                                        (sizeof(qHD_Structure*)*pUsbhDev->bMaxBulkInEpNumber));
            pUsbhDev->hstBulkInWorkqTD      = (qTD_Structure**)((DWORD)pUsbhDev->hstBulkInqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxBulkInEpNumber));
            pUsbhDev->hstBulkInSendLastqTD  = (qTD_Structure**)((DWORD)pUsbhDev->hstBulkInWorkqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxBulkInEpNumber));
            pUsbhDev->hstBulkOutWorkqTD     = (qTD_Structure**)((DWORD)pUsbhDev->hstBulkInSendLastqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxBulkInEpNumber));
            pUsbhDev->hstBulkOutSendLastqTD = (qTD_Structure**)((DWORD)pUsbhDev->hstBulkOutWorkqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxBulkOutEpNumber));
            psEhci->dwHdTdBufferIndex           = (DWORD)((DWORD)pUsbhDev->hstBulkOutSendLastqTD+\
                                                      (sizeof(qTD_Structure*)*pUsbhDev->bMaxBulkOutEpNumber));

            //pstBulkqHDTemp = (qHD_Structure*)(gHost20_STRUCTURE_qHD_BASE_ADDRESS+Host20_qHD_SIZE*2);
            pstBulkqHDTemp = (qHD_Structure*)(psEhci->dwHdBaseBufferIndex);
            psEhci->dwHdBaseBufferIndex += Host20_qHD_SIZE*total_bulk_number;

            BYTE bulk_in_idx = 0;
            BYTE bulk_out_idx = 0;
            flib_Host20_Asynchronous_Setting(HOST20_Disable, eWhichOtg);
#if USBOTG_HOST_DESC
	    	for ( i = 0, k = 0; i < GetConfigNumInterface(pUsbhDev, pUsbhDev->bDeviceConfigIdx); i++ ) {
	    		for ( j = 0; j < GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx); j++ ) {
	                if (OTGH_ED_BULK == (GetEndpointAttributes(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j)& 0x03))
	                {
	                    USB_ENDPOINT_DESCRIPTOR    *pEpDesc = (USB_ENDPOINT_DESCRIPTOR *)GetEndpointStruct(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j);                        
	                    direction = ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j))&USB_ENDPOINT_DIR_MASK);
	                    if (USB_DIR_OUT == direction)
	                    {
	                        pUsbhDev->hstBulkOutqHD[bulk_out_idx] = pstBulkqHDTemp;
	                        pEpDesc->bQHDArrayNum = bulk_out_idx;
	                    }
	                    else
	                    {
	                        pUsbhDev->hstBulkInqHD[bulk_in_idx] = pstBulkqHDTemp;
	                        pEpDesc->bQHDArrayNum = bulk_in_idx;
	                    }
	                    
	                    wMaxPacketSize = byte_swap_of_word(GetEndpointMaxPacketSize(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j));
	                    flib_Host20_Allocate_QHD ( pstBulkqHDTemp,
	                                               HOST20_HD_Type_QH,
	                                               pUsbhDev->bDeviceAddress,
	                                               0,//Head=0
	                                               ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j))&0x0f),
	                                               wMaxPacketSize,
	                                               eWhichOtg
	                                              );
	                    pstBulkqHDTemp->bEdSpeed=pUsbhDev->bDeviceSpeed;
	                    pstCurrentqHDTemp = pstBulkqHDTemp;
	                    if (k == 0)
	                    {
	                        if (USB_DIR_OUT == direction)
	                            k = bulk_out_idx;
	                        else
	                            k = bulk_in_idx;
	                    }
	                    
	                    if (k == 0)
	                    { // first qHD after Control qHD
	                         if (total_bulk_number == 1)
	                            pstCurrentqHDTemp->bNextQHDPointer = (((DWORD)pUsbhDev->pstControlqHD[0])>>5);

	                        pUsbhDev->pstControlqHD[1]->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
	                        pstPreviousqHDTemp = pstCurrentqHDTemp;                            
	                    }
	                    else if (k == (total_bulk_number-1))
	                    { // last qHD
	                        pstPreviousqHDTemp->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
	                        pstCurrentqHDTemp->bNextQHDPointer = (((DWORD)pUsbhDev->pstControlqHD[0])>>5);
	                    }
	                    else
	                    {
	                        pstPreviousqHDTemp->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
	                        pstPreviousqHDTemp = pstCurrentqHDTemp;                            
	                    }
	                    
	                    k++;
	                    pstBulkqHDTemp = (qHD_Structure*)((DWORD)pstBulkqHDTemp + Host20_qHD_SIZE);
	                    if (USB_DIR_OUT == direction)
	                        bulk_out_idx++;
	                    else
	                        bulk_in_idx++;
	    			}
	    		}
	    	}
#else // USBOTG_HOST_DESC
            for (i = 0, j = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)
            {
                if (OTGH_ED_BULK == (pUsbhDev->sEndpointDescriptor[i].bmAttributes & 0x03))
                    direction = ((pUsbhDev->sEndpointDescriptor[i].bEndpointAddress)&USB_ENDPOINT_DIR_MASK);
                    if (USB_DIR_OUT == direction)
                        pUsbhDev->hstBulkOutqHD[bulk_out_idx] = pstBulkqHDTemp;
                    else
                        pUsbhDev->hstBulkInqHD[bulk_in_idx] = pstBulkqHDTemp;
                    
                    wMaxPacketSize = byte_swap_of_word(pUsbhDev->sEndpointDescriptor[i].wMaxPacketSize);
                    flib_Host20_Allocate_QHD ( pstBulkqHDTemp,
                                               HOST20_HD_Type_QH,
                                               pUsbhDev->bDeviceAddress,
                                               0,//Head=0
                                               ((pUsbhDev->sEndpointDescriptor[i].bEndpointAddress)&0x0f),
                                               wMaxPacketSize,
                                               eWhichOtg
                                              );
                    pstBulkqHDTemp->bEdSpeed=pUsbhDev->bDeviceSpeed;
                    pstCurrentqHDTemp = pstBulkqHDTemp;
                    if (j == 0)
                    {
                        if (USB_DIR_OUT == direction)
                            j = bulk_out_idx;
                        else
                            j = bulk_in_idx;
                    }
                    
                    if (j == 0)
                    { // first qHD after Control qHD
                         if (total_bulk_number == 1)
                            pstCurrentqHDTemp->bNextQHDPointer = (((DWORD)pUsbhDev->pstControlqHD[0])>>5);

                        pUsbhDev->pstControlqHD[1]->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
                        pstPreviousqHDTemp = pstCurrentqHDTemp;                            
                    }
                    else if (j == (total_bulk_number-1))
                    { // last qHD
                        pstPreviousqHDTemp->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
                        pstCurrentqHDTemp->bNextQHDPointer = (((DWORD)pUsbhDev->pstControlqHD[0])>>5);
                    }
                    else
                    {
                        pstPreviousqHDTemp->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
                        pstPreviousqHDTemp = pstCurrentqHDTemp;                            
                    }
                    
                    j++;
                    pstBulkqHDTemp = (qHD_Structure*)((DWORD)pstBulkqHDTemp + Host20_qHD_SIZE);
                    if (USB_DIR_OUT == direction)
                        bulk_out_idx++;
                    else
                        bulk_in_idx++;
                } //if (OTGH_ED_BULK == (pUsbhDev->sEndpointDescriptor[i].bmAttributes & 0x03))
            } //for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)
#endif // USBOTG_HOST_DESC            
//            mwHost20_CurrentAsynchronousAddr_Set(gHost20_STRUCTURE_qHD_BASE_ADDRESS);
            //flib_Host20_Asynchronous_Setting(HOST20_Enable);
        } //if (total_bulk_number > 0)
        else
        {
            MP_DEBUG("No Bulk Pipes!!");
        }
        //UsbOtgHostGetQhdForInterruptEd(eWhichOtg); // mark for isoc test
        //UsbOtgHostGetItdForIsocEd(eWhichOtg);
    } // if (pUsbhDev->bDeviceStatus != USB_STATE_CONFIGURED)
}

//
// if bMaxIsoOutEpNumber > 0, bMaxIsoInEpNumber > 0 and bMaxInterruptInEpNumber >0
// then PFL => isoc_out->isoc_in->int
//
void UsbOtgHostPeriodicFrameListInit(BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_HOST psHost;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci;
    BYTE index = 0;
    BYTE x;
    BYTE num_of_isoc_out = 0;
    BYTE num_of_isoc_in = 0;
    BYTE num_of_int = 0;
    BYTE num_of_ep = 0;
    WORD i, j, k;

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = &psHost->sEhci;
    
    flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);

#if (USBOTG_HOST_ISOC == ENABLE)
    if (pUsbhDev->bNumOfIsocFrameList > 0)
    {
        pUsbhDev->pstPeriodicFrameList = (PISOC_FRAME_LIST_STRUCTURE)\
                                         ker_mem_malloc(pUsbhDev->bNumOfIsocFrameList*sizeof(ISOC_FRAME_LIST_STRUCTURE), UsbOtgHostDriverTaskIdGet(eWhichOtg));
        if (pUsbhDev->pstPeriodicFrameList == 0)
        {
            MP_ALERT("%s:%d: bNumOfIsocFrameList = %d alloc memory failed!", __FUNCTION__, __LINE__,\
                pUsbhDev->bNumOfIsocFrameList);
            return;
        }
    }

    PST_USB_OTG_HOST_ISOC_DES pIsocEpDes;

    pIsocEpDes = GetBestIsocDes (bInterfaceClass, bInteraceNumber, USB_DIR_IN,  eWhichOtg);
    if (pIsocEpDes == NULL)
    {
        MP_ALERT("%s:%d:no isoc in pipes!", __FUNCTION__, __LINE__);
    }
    else
    {
        num_of_isoc_in = GetInterfaceEPs(pUsbhDev,
                                         pUsbhDev->bDeviceConfigIdx,
                                         pIsocEpDes->bInterfaceNumber,
                                         pIsocEpDes->bAlternativeSetting);
        MP_ALERT("%s:%d:num_of_isoc_in = %d", __FUNCTION__, __LINE__, num_of_isoc_in);
    }

    pIsocEpDes = GetBestIsocDes (bInterfaceClass, bInteraceNumber, USB_DIR_OUT, eWhichOtg);
    if (pIsocEpDes == NULL)
    {
        MP_ALERT("%s:%d:no isoc out pipes!", __FUNCTION__, __LINE__);
    }
    else
    {
        num_of_isoc_out = GetInterfaceEPs(pUsbhDev,
                                          pUsbhDev->bDeviceConfigIdx,
                                          pIsocEpDes->bInterfaceNumber,
                                          pIsocEpDes->bAlternativeSetting);
        MP_ALERT("%s:%d:num_of_isoc_out = %d", __FUNCTION__, __LINE__, num_of_isoc_out);
    }
    
    num_of_ep = GetInterfaceEPs(pUsbhDev,
                                pUsbhDev->bDeviceConfigIdx,
                                pUsbhDev->bAppInterfaceNumber,
                                AlterSettingDefaultIdx);
                                //pIsocEpDes->bAlternativeSetting);
    BYTE bEpAttribute = 0;
    for ( j = 0; j < num_of_ep; j++ )
    {
        bEpAttribute = GetEndpointAttributes(pUsbhDev,
                                              pUsbhDev->bDeviceConfigIdx,
                                              pUsbhDev->bAppInterfaceNumber,
                                              AlterSettingDefaultIdx,
                                              j);
         if (OTGH_ED_INT == (bEpAttribute&0x03))
         {
            num_of_int++;
         }
    } // for loop : bNumEndpoints
    mpDebugPrint("%d:%d:num_of_isoc_in = %d", eWhichOtg, __LINE__, num_of_isoc_in);
    mpDebugPrint("%d:%d:num_of_isoc_out = %d", eWhichOtg, __LINE__, num_of_isoc_out);
    mpDebugPrint("%d:%d:num_of_int = %d", eWhichOtg, __LINE__, num_of_int);
#else
    num_of_ep = GetInterfaceEPs(pUsbhDev,
                                pUsbhDev->bDeviceConfigIdx,
                                pUsbhDev->bAppInterfaceNumber,
                                AlterSettingDefaultIdx);
                                //pIsocEpDes->bAlternativeSetting);
    BYTE bEpAttribute = 0;
    for ( j = 0; j < num_of_ep; j++ )
    {
        bEpAttribute = GetEndpointAttributes(pUsbhDev,
                                              pUsbhDev->bDeviceConfigIdx,
                                              pUsbhDev->bAppInterfaceNumber,
                                              AlterSettingDefaultIdx,
                                              j);
         if (OTGH_ED_INT == (bEpAttribute&0x03))
         {
            num_of_int++;
         }
    } // for loop : bNumEndpoints
    num_of_isoc_out = 0;
    num_of_isoc_in = 0;
#endif
/*
#if (USBOTG_HOST_ISOC == ENABLE)
    num_of_isoc = (pUsbhDev->bMaxIsoInEpNumber + pUsbhDev->bMaxIsoOutEpNumber);
#else
    num_of_isoc = 0;
#endif
    num_of_int = (pUsbhDev->bMaxInterruptInEpNumber + pUsbhDev->bMaxInterruptOutEpNumber);
*/

    MP_DEBUG("%s:%d:num_of_int = %d", __FUNCTION__, __LINE__, num_of_int);
    if ((num_of_isoc_in+num_of_isoc_out) > 0 || num_of_int > 0)
    {
        for (i = 0; i < Host20_Preiodic_Frame_List_MAX; i++)
        {
            // set T bit to zero by default
            psEhci->psHostFramList->sCell[i].bTerminal=1;
        }

        if ((num_of_isoc_out > 0) && (num_of_isoc_in > 0))
        {
            index = pUsbhDev->bIsocOutEpPflIndex;
            for (i = 0; i < PERIODIC_FRAME_SIZE; i++)
            {
                pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i] = flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
            }
            
            index = pUsbhDev->bIsocInEpPflIndex;
            for (i = 0; i < PERIODIC_FRAME_SIZE; i++)
            {
                pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i]  = flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
                ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i])->bNextLinkPointer =\
                                  pUsbhDev->pstPeriodicFrameList[pUsbhDev->bIsocOutEpPflIndex].dwFrameList[i]>>5;
                ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i])->bType            = HOST20_HD_Type_iTD; 
                ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i])->bTerminate       = 0;// if no next link pointer, so set to 1
            }
        }
        else if ((num_of_isoc_out == 0) && (num_of_isoc_in > 0))
        {
            index = pUsbhDev->bIsocInEpPflIndex;
            for (i = 0; i < PERIODIC_FRAME_SIZE; i++)
            {
                pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i] = flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
                ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i])->bType = HOST20_HD_Type_iTD; 
            }
        }
        else if ((num_of_isoc_out > 0) && (num_of_isoc_in == 0))
        {
            index = pUsbhDev->bIsocOutEpPflIndex;
            for (i = 0; i < PERIODIC_FRAME_SIZE; i++)
            {
                pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i] = flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
                ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i])->bType = HOST20_HD_Type_iTD; 
            }
        }

        
        if (num_of_int > 0)
        {
            //Link qHD to the FameListCell by wInterval
            //Find the Interval-X 
            x=0;
            while(psEhci->dwIntervalMap[x] < pUsbhDev->dwForceInterval)
            {
                x++;
                if (x>10)
                {
                    MP_ALERT("%s:Interval Input Error...into unlimited while loop!!", __FUNCTION__); 
                }
            }
            mpDebugPrint("%s:psEhci->dwIntervalMap[%d] = %d", __FUNCTION__, x, psEhci->dwIntervalMap[x]);
        }

        if ((num_of_isoc_in+num_of_isoc_out) > 0 && num_of_int > 0)
        {
            iTD_Structure *pItdTemp;
            
            if ((num_of_isoc_out > 0)&&(num_of_isoc_in >= 0))
                pItdTemp = (iTD_Structure*)pUsbhDev->pstPeriodicFrameList[pUsbhDev->bIsocOutEpPflIndex].dwFrameList[0];
            else if (num_of_isoc_out == 0 && num_of_isoc_in > 0)
                pItdTemp = (iTD_Structure*)pUsbhDev->pstPeriodicFrameList[pUsbhDev->bIsocInEpPflIndex].dwFrameList[0];

            for (i = 0; i < PERIODIC_FRAME_SIZE; i++, pItdTemp++) // the first 256 elements in PFL
            {
                pItdTemp->bNextLinkPointer                    = ((DWORD)pUsbhDev->hstInterruptInqHD[0])>>5;// assign interrupt qHD to each isochronous iTD which amount is 256.
                pItdTemp->bType                               = HOST20_HD_Type_QH;// set the type of the next link pointer is qHD.
                pItdTemp->bTerminate                          = 0;                
            }

            // isoc IN iTD is the head of the PFL.
            pItdTemp = (iTD_Structure*)pUsbhDev->pstPeriodicFrameList[pUsbhDev->bIsocInEpPflIndex].dwFrameList[0];
                
            for (i = 0; i < PERIODIC_FRAME_SIZE; i++, pItdTemp++) // the first 256 elements in PFL
            {
                psEhci->psHostFramList->sCell[i].bLinkPointer = ((DWORD)pItdTemp)>>5;
                psEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_iTD;     
                psEhci->psHostFramList->sCell[i].bTerminal    = 0;// clear T bit of PFL for linkink iTD and qHD together (256 iTD)
            }
            
            for (i = PERIODIC_FRAME_SIZE; i < Host20_Preiodic_Frame_List_MAX; i++) // the followed elements in PFL
            {
                psEhci->psHostFramList->sCell[i].bLinkPointer = ((DWORD)pUsbhDev->hstInterruptInqHD[0])>>5;
                psEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_QH;     
                psEhci->psHostFramList->sCell[i].bTerminal    = 0;
            }
        }
        else if ((num_of_isoc_in+num_of_isoc_out) > 0 && num_of_int == 0)
        {
            if (num_of_isoc_in > 0)
            {
                index = pUsbhDev->bIsocInEpPflIndex;
                for (i = 0; i < PERIODIC_FRAME_SIZE; i=i+1)      
                {
                    psEhci->psHostFramList->sCell[i].bLinkPointer = pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i]>>5;
                    psEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_iTD;     
                    psEhci->psHostFramList->sCell[i].bTerminal    = 0;
                }
            }

            if (num_of_isoc_out > 0)
            {
                index = pUsbhDev->bIsocOutEpPflIndex;
                for (i = 0; i < PERIODIC_FRAME_SIZE; i=i+1)      
                {
                    psEhci->psHostFramList->sCell[i].bLinkPointer = pUsbhDev->pstPeriodicFrameList[index].dwFrameList[i]>>5;
                    psEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_iTD;     
                    psEhci->psHostFramList->sCell[i].bTerminal    = 0;
                }
            }
        }
        else if ((num_of_isoc_in+num_of_isoc_out) == 0 && num_of_int > 0)
        {
            for (i = 0; i < Host20_Preiodic_Frame_List_MAX; i=i+psEhci->dwIntervalMap[x])
            {
                psEhci->psHostFramList->sCell[i].bLinkPointer = ((DWORD)pUsbhDev->hstInterruptInqHD[0])>>5;  // Must have INT-IN;INT-IN/OUT have the same bInterval
                psEhci->psHostFramList->sCell[i].bType        = HOST20_HD_Type_QH;
                psEhci->psHostFramList->sCell[i].bTerminal    = 0;
            }
        }
        else
        {
            MP_DEBUG("%s:it's impossible into here!!", __FUNCTION__);
        }
    }
    
    mwHost20_PeriodicBaseAddr_Set(psEhci->dwHostStructurePflBaseAddress);    

    if (PERIODIC_FRAME_SIZE == 256)
        mbHost20_USBCMD_FrameListSize_Set(FRAME_LIST_SIZE_256);
    else if (PERIODIC_FRAME_SIZE == 512)
        mbHost20_USBCMD_FrameListSize_Set(FRAME_LIST_SIZE_512);
    else if (PERIODIC_FRAME_SIZE == 1024)
        mbHost20_USBCMD_FrameListSize_Set(FRAME_LIST_SIZE_1024);
    else
        mbHost20_USBCMD_FrameListSize_Set(FRAME_LIST_SIZE_256);
}

static void UsbOtgHostGetQhdForInterruptEd(WHICH_OTG eWhichOtg)
{
    DWORD i,j,k;
    WORD wSize;
    BYTE  bInterruptNum,x;
    //WORD wForceInterval = 0;
    WORD wMaxPacketSize = 0;
    
    PST_USB_OTG_DES psUsbOtg;
    PST_USB_OTG_HOST psHost;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    psHost = &psUsbOtg->sUsbHost;
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = &psHost->sEhci;

    if (pUsbhDev->bDeviceStatus != USB_STATE_CONFIGURED)
    { // for control pipe only at beginning
        MP_DEBUG("UsbOtgHostGetQhdForInterruptEd:not in USB_STATE_CONFIGURED");
        return;
    }
    else
    { // for all pipes after getting interface descriptor
        qHD_Structure *pstCurrentqHDTemp    = OTGH_NULL;
        qHD_Structure *pstPreviousqHDTemp   = OTGH_NULL;    
        qHD_Structure *pstInterruptqHDTemp  = OTGH_NULL;
        
#if USBOTG_HOST_DESC
        if (GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, pUsbhDev->bDeviceInterfaceIdx, AlterSettingDefaultIdx) > (Host20_qHD_MAX-2))
#else        
        if (pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints > (Host20_qHD_MAX-2))
#endif
        {
            MP_DEBUG("UsbOtgHostGetQhdForInterruptEd:Total Number of Endpoints is over 20!!");
            return;
        }
            
        flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);

        BYTE total_int_number = 0;
        total_int_number = pUsbhDev->bMaxInterruptInEpNumber + pUsbhDev->bMaxInterruptOutEpNumber;
        if (total_int_number > 0)
        {
            MP_DEBUG("UsbOtgHostGetQhdForInterruptEd:Total Number of Interrupt Endpoints is %d", total_int_number);
            pUsbhDev->psAppClass->bIntInQHDArrayNum = 0;
            
            //<1>.Disable the Periodic
            pUsbhDev->hstInterruptOutqHD         = (qHD_Structure**)psEhci->dwHdTdBufferIndex;
            pUsbhDev->hstInterruptOutqTD         = (qTD_Structure**)((DWORD)pUsbhDev->hstInterruptOutqHD+\
                                                        (sizeof(qHD_Structure*)*pUsbhDev->bMaxInterruptOutEpNumber));
            pUsbhDev->hstInterruptInqHD          = (qHD_Structure**)((DWORD)pUsbhDev->hstInterruptOutqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxInterruptOutEpNumber));
            pUsbhDev->hstInterruptInqTD          = (qTD_Structure**)((DWORD)pUsbhDev->hstInterruptInqHD+\
                                                        (sizeof(qHD_Structure*)*pUsbhDev->bMaxInterruptInEpNumber));
            pUsbhDev->hstIntInWorkqTD      = (qTD_Structure**)((DWORD)pUsbhDev->hstInterruptInqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxInterruptInEpNumber));
            pUsbhDev->hstIntInSendLastqTD  = (qTD_Structure**)((DWORD)pUsbhDev->hstIntInWorkqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxInterruptInEpNumber));
            pUsbhDev->hstIntOutWorkqTD     = (qTD_Structure**)((DWORD)pUsbhDev->hstIntInSendLastqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxInterruptInEpNumber));
            pUsbhDev->hstIntOutSendLastqTD = (qTD_Structure**)((DWORD)pUsbhDev->hstIntOutWorkqTD+\
                                                        (sizeof(qTD_Structure*)*pUsbhDev->bMaxInterruptOutEpNumber));
            psEhci->dwHdTdBufferIndex           = (DWORD)((DWORD)pUsbhDev->hstIntOutSendLastqTD+\
                                                      (sizeof(qTD_Structure*)*pUsbhDev->bMaxInterruptOutEpNumber));
            pstInterruptqHDTemp = (qHD_Structure*)(psEhci->dwHdBaseBufferIndex);
            psEhci->dwHdBaseBufferIndex += Host20_qHD_SIZE*total_int_number;

            //<2>.Init qHD for Interrupt(7~9) Scan the ED
            BYTE Interrupt_in_idx = 0;
            BYTE Interrupt_out_idx = 0;            
            j=0;
#if 1 //USBOTG_HOST_DESC
	    	for ( i = 0, k = 0; i < GetConfigNumInterface(pUsbhDev, pUsbhDev->bDeviceConfigIdx); i++ ) {
	    		for ( j = 0; j < GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx); j++ ) {
	                if (OTGH_ED_INT == (GetEndpointAttributes(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j)&0x03))
	                {
	                    BYTE direction;
	                    USB_ENDPOINT_DESCRIPTOR    *pEpDesc = (USB_ENDPOINT_DESCRIPTOR *)GetEndpointStruct(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j);
	                    direction = ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j))&USB_ENDPOINT_DIR_MASK);
	                    if (USB_DIR_OUT == direction)
	                    {
	                        pUsbhDev->hstInterruptOutqHD[Interrupt_out_idx] = pstInterruptqHDTemp;
	                        pEpDesc->bQHDArrayNum = Interrupt_out_idx;
                           }
	                    else
	                    {
	                        pUsbhDev->hstInterruptInqHD[Interrupt_in_idx] = pstInterruptqHDTemp;
	                        pEpDesc->bQHDArrayNum = Interrupt_in_idx;
                           }                        
                        
	                    //wForceInterval = GetEndpointInterval(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j);
	                    wMaxPacketSize = byte_swap_of_word(GetEndpointMaxPacketSize(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j));
	                    flib_Host20_Allocate_QHD ( pstInterruptqHDTemp,
	                                               HOST20_HD_Type_QH,
	                                               pUsbhDev->bDeviceAddress,
	                                               0,//Head=0
	                                               ((GetEndpointAddress(pUsbhDev, pUsbhDev->bDeviceConfigIdx, i, AlterSettingDefaultIdx, j))&0x0f),
	                                               wMaxPacketSize,
	                                               eWhichOtg
	                                              );                      
	                    (pstInterruptqHDTemp)->bHighBandwidth          = 1;
	                    (pstInterruptqHDTemp)->bInterruptScheduleMask  = 1;
	                    (pstInterruptqHDTemp)->bEdSpeed                = pUsbhDev->bDeviceSpeed;
	                    (pstInterruptqHDTemp)->bDataToggleControl      = 0; 
	                    (pstInterruptqHDTemp)->bInterruptScheduleMask  = 1;
	                    (pstInterruptqHDTemp)->bHighBandwidth          = 1;  // 1~3              
	                    //(pstInterruptqHDTemp)->bTerminate              = 0;

	                    pstCurrentqHDTemp = pstInterruptqHDTemp;
                        
	                    if (k == 0)
	                    {
	                        if (USB_DIR_OUT == direction)
	                            k = Interrupt_out_idx;
	                        else
	                            k = Interrupt_in_idx;
	                    }
	                    
	                    if (k == 0)
	                    { // first qHD
	                         if (total_int_number == 1)
                                    pstCurrentqHDTemp->bTerminate = 1;

	                        pstPreviousqHDTemp = pstCurrentqHDTemp;                            
	                    }
	                    else if (k == (total_int_number-1))
	                    { // last qHD
	                        pstPreviousqHDTemp->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
                               pstCurrentqHDTemp->bTerminate = 1; 
	                    }
	                    else
	                    {
	                        pstPreviousqHDTemp->bNextQHDPointer = (((DWORD)pstCurrentqHDTemp)>>5);
	                        pstPreviousqHDTemp = pstCurrentqHDTemp;                            
	                    }
	                    
	                    k++;

	                    pstInterruptqHDTemp = (qHD_Structure*)((DWORD)pstInterruptqHDTemp + Host20_qHD_SIZE);
	                    if (USB_DIR_OUT == direction)
	                        Interrupt_out_idx++;
	                    else
	                        Interrupt_in_idx++;
	    			}
                    } // if (OTGH_ED_INT == (pUsbhDev->sEndpointDescriptor[i].bmAttributes & 0x03))
	    	}            
#else
            for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)
            {
                if (OTGH_ED_INT == (pUsbhDev->sEndpointDescriptor[i].bmAttributes & 0x03))
                {
                    pUsbhDev->hstInterruptqHD[j] = pstInterruptqHDTemp;
                    pUsbhDev->dwForceInterval = pUsbhDev->sEndpointDescriptor[i].bInterval;
                    wMaxPacketSize = byte_swap_of_word(pUsbhDev->sEndpointDescriptor[i].wMaxPacketSize);
                    flib_Host20_Allocate_QHD ( pstInterruptqHDTemp,
                                               HOST20_HD_Type_QH,
                                               pUsbhDev->bDeviceAddress,
                                               0,//Head=0
                                               ((pUsbhDev->sEndpointDescriptor[i].bEndpointAddress)&0x0f),
                                               wMaxPacketSize,
                                               eWhichOtg
                                              );
                    (pstInterruptqHDTemp)->bHighBandwidth          = 1;
                    (pstInterruptqHDTemp)->bInterruptScheduleMask  = 1;
                    (pstInterruptqHDTemp)->bEdSpeed                = pUsbhDev->bDeviceSpeed;
                    (pstInterruptqHDTemp)->bDataToggleControl      = 0; 
                    (pstInterruptqHDTemp)->bInterruptScheduleMask  = 1;
                    (pstInterruptqHDTemp)->bHighBandwidth          = 1;  //1~3              
                    //(pstInterruptqHDTemp)->bTerminate              = 0;

                    pstInterruptqHDTemp = (qHD_Structure*)((DWORD)pstInterruptqHDTemp + Host20_qHD_SIZE);
                    j++;
                } // if (OTGH_ED_INT == (pUsbhDev->sEndpointDescriptor[i].bmAttributes & 0x03))
            } //for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)

            i=0;
            while (i<(pUsbhDev->bMaxInterruptEpNumber-1))
            {
                pUsbhDev->hstInterruptqHD[i]->bNextQHDPointer=(((DWORD)pUsbhDev->hstInterruptqHD[i+1])>>5);
                pUsbhDev->hstInterruptqHD[i]->bTerminate=0;
                i++;
            }
            
            pUsbhDev->hstInterruptqHD[i]->bTerminate=1;  
#endif            
/*
            //<3>.Link qHD to the FameListCell by wInterval
            for (i=0;i<Host20_Preiodic_Frame_List_MAX;i++)
                psEhci->psHostFramList->sCell[i].bTerminal=1;

            //Find the Interval-X 
            x=0;
            while(psEhci->dwIntervalMap[x] < pUsbhDev->dwForceInterval)
            {
                x++;
                if (x>10)
                {
                    MP_DEBUG("Interval Input Error...into unlimited while loop!!");	
                    while(1);
                }
            }
                  
            for (i=0;i<Host20_Preiodic_Frame_List_MAX;i=i+psEhci->dwIntervalMap[x])
            {
                psEhci->psHostFramList->sCell[i].bLinkPointer=((DWORD)pUsbhDev->hstInterruptInqHD[0])>>5;  // Must have INT-IN;INT-IN/OUT have the same bInterval
                psEhci->psHostFramList->sCell[i].bTerminal=0;
                psEhci->psHostFramList->sCell[i].bType=HOST20_HD_Type_QH;
            }

            //<4>.Set Periodic Base Address	
            mwHost20_PeriodicBaseAddr_Set(psEhci->dwHostStructurePflBaseAddress);	

            //<5>.Enable the periodic 
            flib_Host20_Periodic_Setting(HOST20_Enable, eWhichOtg);
*/            
        }//if (pUsbhDev->bMaxInterruptEpNumber > 0)
    } // if (pUsbhDev->bDeviceStatus != USB_STATE_CONFIGURED)
}


void UsbOtgHostGetAllEpsInformation (BYTE bInterfaceClass, BYTE bInterfaceNumber, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    UsbOtgHostGetQhdForEachEd(eWhichOtg);
    UsbOtgHostGetQhdForInterruptEd(eWhichOtg);
#if (USBOTG_HOST_ISOC == ENABLE)
    UsbOtgHostGetItdForIsocEd(eWhichOtg);
/* I do not know why has the code as below   
    if (pUsbhDev->bMaxIsoInEpNumber > 0)
    {
        pUsbhDev->bIsocInEpPflIndex = pUsbhDev->bNumOfIsocFrameList;
        pUsbhDev->bNumOfIsocFrameList++;
    }
        
    if (pUsbhDev->bMaxIsoOutEpNumber > 0)
    {
        pUsbhDev->bIsocOutEpPflIndex = pUsbhDev->bNumOfIsocFrameList;
        pUsbhDev->bNumOfIsocFrameList++;
    }
//*/    
#else
    pUsbhDev->bMaxIsoInEpNumber = 0;
    pUsbhDev->bMaxIsoOutEpNumber= 0;
#endif
    pUsbhDev->bAppInterfaceClass = bInterfaceClass;
    pUsbhDev->bAppInterfaceNumber = bInterfaceNumber;
    //UsbOtgHostPrepareFrameListForIsocEp(bInterfaceClass, bInterfaceNumber, eWhichOtg);
    UsbOtgHostPrepareFrameList(bInterfaceClass, bInterfaceNumber, eWhichOtg);
}


void UsbOtgHostPrepareFrameList (BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
    PST_USB_OTG_HOST_ISOC_DES pIsocOutEpDes;

#if (USBOTG_HOST_ISOC == ENABLE)
    pIsocInEpDes = GetBestIsocDes (bInterfaceClass, bInteraceNumber, USB_DIR_IN, eWhichOtg);
    if (pIsocInEpDes == NULL)
    {
        MP_ALERT("%s:%d:pIsocInEpDes is NULL!", __FUNCTION__, __LINE__);
    }
    else
    {
        pUsbhDev->bIsocInEpPflIndex = pUsbhDev->bNumOfIsocFrameList;
        pUsbhDev->bNumOfIsocFrameList++;
    }
    
    pIsocOutEpDes = GetBestIsocDes (bInterfaceClass, bInteraceNumber, USB_DIR_OUT, eWhichOtg);
    if (pIsocOutEpDes == NULL)
    {
        MP_ALERT("%s:%d:pIsocOutEpDes is NULL!", __FUNCTION__, __LINE__);
    }
    else
    {
        pUsbhDev->bIsocOutEpPflIndex = pUsbhDev->bNumOfIsocFrameList;
        pUsbhDev->bNumOfIsocFrameList++;
    }
#else
    pUsbhDev->bNumOfIsocFrameList = 0;
#endif

    MP_DEBUG("%s:bIsocInEpPflIndex = %d",   __FUNCTION__, pUsbhDev->bIsocInEpPflIndex);
    MP_DEBUG("%s:bIsocOutEpPflIndex = %d",  __FUNCTION__, pUsbhDev->bIsocOutEpPflIndex);
    MP_DEBUG("%s:bNumOfIsocFrameList = %d", __FUNCTION__, pUsbhDev->bNumOfIsocFrameList);
    UsbOtgHostPeriodicFrameListInit(bInterfaceClass, bInteraceNumber, eWhichOtg);
}
void UsbOtgHostPrepareFrameListForIsocEp (BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
    PST_USB_OTG_HOST_ISOC_DES pIsocOutEpDes;

    pIsocInEpDes = GetBestIsocDes (bInterfaceClass, bInteraceNumber, USB_DIR_IN, eWhichOtg);
    if (pIsocInEpDes == NULL)
    {
        MP_ALERT("%s:%d:pIsocInEpDes is NULL!", __FUNCTION__, __LINE__);
    }
    else
    {
        pUsbhDev->bIsocInEpPflIndex = pUsbhDev->bNumOfIsocFrameList;
        pUsbhDev->bNumOfIsocFrameList++;
    }
    
    pIsocOutEpDes = GetBestIsocDes (bInterfaceClass, bInteraceNumber, USB_DIR_OUT, eWhichOtg);
    if (pIsocOutEpDes == NULL)
    {
        MP_ALERT("%s:%d:pIsocOutEpDes is NULL!", __FUNCTION__, __LINE__);
    }
    else
    {
        pUsbhDev->bIsocOutEpPflIndex = pUsbhDev->bNumOfIsocFrameList;
        pUsbhDev->bNumOfIsocFrameList++;
    }


    MP_DEBUG("%s:bIsocInEpPflIndex = %d",   __FUNCTION__, pUsbhDev->bIsocInEpPflIndex);
    MP_DEBUG("%s:bIsocOutEpPflIndex = %d",  __FUNCTION__, pUsbhDev->bIsocOutEpPflIndex);
    MP_DEBUG("%s:bNumOfIsocFrameList = %d", __FUNCTION__, pUsbhDev->bNumOfIsocFrameList);
    UsbOtgHostPeriodicFrameListInit(bInterfaceClass, bInteraceNumber, eWhichOtg);
}




void UsbOtgHostInactiveAllqTD (WHICH_OTG eWhichOtg)
{
    BYTE i = 0;
    BYTE j = 0;
    BYTE bMaxNumEndpoints = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    
    MP_DEBUG("UsbOtgHostInactiveAllqTD");
#if USBOTG_HOST_DESC
    if(pUsbhDev->pConfigDescriptor == NULL)
        return; // Had Free qTD
    
    bMaxNumEndpoints = GetInterfaceEPs(pUsbhDev, pUsbhDev->bDeviceConfigIdx, pUsbhDev->bDeviceInterfaceIdx, AlterSettingDefaultIdx);

    if (bMaxNumEndpoints == 0)
        bMaxNumEndpoints = 2;
#else
    if (pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints == 0)
        bMaxNumEndpoints = 2;
    else
        bMaxNumEndpoints = pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints;
#endif
    
    for (i = 0; i < bMaxNumEndpoints; i++)
    {
        for (j = 0; j< 2; j++)
        {
            
            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->pstControlqHD[j]))
            {
                if (pUsbhDev->psControlqTD->bStatus_Active == 1)
                {
                    MP_DEBUG("CTRL Inactive");
                    pUsbhDev->psControlqTD->bTerminate     = 1;
                    pUsbhDev->psControlqTD->bStatus_Active = 0;
                }
            }
        }
        
        for (j = 0; j< pUsbhDev->bMaxBulkOutEpNumber; j++)
        {
            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstBulkOutqHD[j]))
            {
                if (pUsbhDev->hstBulkOutqTD[j]->bStatus_Active == 1)
                {
                    MP_DEBUG("BulkOUT Inactive");
                    pUsbhDev->hstBulkOutqTD[j]->bTerminate     = 1;
                    pUsbhDev->hstBulkOutqTD[j]->bStatus_Active = 0;
                }
            }
        }
        
        for (j = 0; j< pUsbhDev->bMaxBulkInEpNumber; j++)
        {
            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstBulkInqHD[j]))
            {
                if (pUsbhDev->hstBulkInqTD[j]->bStatus_Active == 1)
                {
                    MP_DEBUG("BulkIN Inactive");
                    pUsbhDev->hstBulkInqTD[j]->bTerminate     = 1;
                    pUsbhDev->hstBulkInqTD[j]->bStatus_Active = 0;
                }
            }
        }

        for (j = 0; j< pUsbhDev->bMaxInterruptInEpNumber; j++)
        {
            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstInterruptInqHD[j]))
            {
                if (pUsbhDev->hstInterruptInqTD[j]->bStatus_Active == 1)
                {
                    MP_DEBUG("INT Inactive");
                    pUsbhDev->hstInterruptInqTD[j]->bTerminate     = 1;
                    pUsbhDev->hstInterruptInqTD[j]->bStatus_Active = 0;
                }
            }
        }

        for (j = 0; j< pUsbhDev->bMaxInterruptOutEpNumber; j++)
        {
            if ((pUsbhDev->dwActivedQHD[i] != 0)&&\
                (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstInterruptOutqHD[j]))
            {
                if (pUsbhDev->hstInterruptOutqTD[j]->bStatus_Active == 1)
                {
                    MP_DEBUG("INT Inactive");
                    pUsbhDev->hstInterruptOutqTD[j]->bTerminate     = 1;
                    pUsbhDev->hstInterruptOutqTD[j]->bStatus_Active = 0;
                }
            }
        }
    } // for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)
}

/*
void UsbOtgHostItdAlloc(WHICH_OTG eWhichOtg)
{
    DWORD i,j;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    //__asm("break 100");
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->pstPeriodicFrameList = (ISOC_FRAME_LIST_STRUCTURE)ext_mem_malloc(pUsbhDev->bNumOfIsocFrameList*sizeof(ISOC_FRAME_LIST_STRUCTURE));
    if (pUsbhDev->pstPeriodicFrameList == 0)
    {
        MP_ALERT("%s: alloc memory failed", __FUNCTION__);
        while(1);
    }
    
    for (i = 0; i < pUsbhDev->bNumOfIsocFrameList; i++)
    {
        for (j = 0; j < PERIODIC_FRAME_SIZE; j++)
        {
            pUsbhDev->pstPeriodicFrameList[i].dwFrameList[j] = flib_Host20_GetStructure(Host20_MEM_TYPE_iTD, eWhichOtg);
            ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[i].dwFrameList[j])->bType      = HOST20_HD_Type_iTD; 
            //((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[i].dwFrameList[j])->bTerminate = 0; // no next link pointer, so set to 1
        }
    }
}
*/

void UsbOtgHostStopAllItd(WHICH_OTG eWhichOtg)
{
    DWORD i,j,k;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
#if (USBOTG_WEB_CAM == ENABLE)
    PUSB_HOST_UVC  psUsbhUvc=(PUSB_HOST_UVC)UsbOtgHostUvcDsGet(eWhichOtg);
#endif

    pUsbhDev  = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    for (i = 0; i < pUsbhDev->bNumOfIsocFrameList; i++)
    {
        for (j = 0; j < PERIODIC_FRAME_SIZE; j++)
        {
            for (k = 0; k < (pUsbhDev->bDeviceSpeed == 0 ? 1 : 8); k++)
                ((iTD_Structure *)pUsbhDev->pstPeriodicFrameList[i].dwFrameList[j])->ArrayStatus_Word[k].bStatus = 0; 
        }
    }
    
#if (USBOTG_WEB_CAM == ENABLE)
    psUsbhUvc->eNewFrame               = HAS_NO_FRAME;
    psUsbhUvc->dwFrameNumber           = 0;
    psUsbhUvc->dwOriginalFrameNumber   = 0;
    psUsbhUvc->dwLastFrameNumber       = 0;
    psUsbhUvc->bFrameID                = 0;
    psUsbhUvc->bStartOfFrame           = 0;
    psUsbhUvc->bNewOneFrame            = 0;    
#endif
#if (USBOTG_HOST_ISOC == ENABLE)
    UsbOtgHostIsocStreamDataInit(eWhichOtg);
#endif
    mpDebugPrint("%s:%d:Done", __FUNCTION__, __LINE__);
}


void UsbOtgHostItdFree(WHICH_OTG eWhichOtg)
{
    DWORD i,j;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    ker_mem_free(pUsbhDev->pstPeriodicFrameList);

    for (i = 0; i < pUsbhDev->bNumOfIsocFrameList; i++)
    {
        for (j = 0; j < PERIODIC_FRAME_SIZE; j++)
        {
            flib_Host20_ReleaseStructure(Host20_MEM_TYPE_iTD, pUsbhDev->pstPeriodicFrameList[i].dwFrameList[j], eWhichOtg);
        }
    }
}


BYTE UsbOtgHostGetTotalNumberOfBulkEp(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE bTotalNumberOfBulkEp = 0;

    if (pUsbhDev->bMaxBulkInEpNumber> 0)
        bTotalNumberOfBulkEp++;
    if (pUsbhDev->bMaxBulkOutEpNumber> 0)
        bTotalNumberOfBulkEp++;
    
    return bTotalNumberOfBulkEp;
}



//====================================================================
// * Function Name: flib_Host20_Send_qTD_Active                          
// * Description: 
//   Case-1:1qTD
//   Case-2:2qTD
//   Case-3:3qTD above
// * Input:Type
// * OutPut: 
//====================================================================
static void flib_Host20_Send_qTD_Active (  qTD_Structure *spHeadqTD,
                                    qHD_Structure *spTempqHD,
                                    WHICH_OTG eWhichOtg)
{
	qTD_Structure           *spNewDumyqTD; 
	qTD_Structure  *spOldDumyqTD; 
	qTD_Structure  *spLastqTD;
    DWORD dwSwapStartBuffer = 0;
    DWORD dwSwapStopBuffer = 0;
    //DWORD dwSwapTotalByteLength = 0;
    DWORD i = 0;
    DWORD j = 0;
    DWORD cnt_1;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    //pUsbhDev->pstWorkqTD = spHeadqTD;

    g_spTempqHD = spTempqHD;

	//<1>.Copy Head-qTD to OldDumyqTD
	spOldDumyqTD=(qTD_Structure*)(((DWORD)(spTempqHD->bOverlay_NextqTD))<<5);
	memcpy((BYTE*)spOldDumyqTD,(BYTE*)spHeadqTD,Host20_qTD_SIZE);
/*    
    if (pUsbhDev->hstInterruptqHD[0] == spTempqHD)
    {
        //__asm("break 100");
        pUsbhDev->pstIntWorkqTD = spHeadqTD;
        pUsbhDev->psIntSendLastqTD = spOldDumyqTD;
    }
    else
    {
        pUsbhDev->pstWorkqTD = spHeadqTD;
        pUsbhDev->psSendLastqTD=spOldDumyqTD;
    }
*/
    //for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)
    {
        for (j = 0; j< 2; j++)
        {
            if (pUsbhDev->pstControlqHD[j] == spTempqHD)
            {
                pUsbhDev->pstControlWorkqTD    = spHeadqTD;
                pUsbhDev->pstControlSendLastqTD = spOldDumyqTD;
            }
        }
                    
        for (j = 0; j< pUsbhDev->bMaxBulkInEpNumber; j++)
        {
            if (pUsbhDev->hstBulkInqHD[j] == spTempqHD)
            {
                pUsbhDev->hstBulkInWorkqTD[j]     = spHeadqTD;
                pUsbhDev->hstBulkInSendLastqTD[j]  = spOldDumyqTD;
            }
        }

        for (j = 0; j< pUsbhDev->bMaxBulkOutEpNumber; j++)
        {
            if (pUsbhDev->hstBulkOutqHD[j] == spTempqHD)
            {
                pUsbhDev->hstBulkOutWorkqTD[j]    = spHeadqTD;
                pUsbhDev->hstBulkOutSendLastqTD[j] = spOldDumyqTD;
            }
        }
        
        for (j = 0; j< pUsbhDev->bMaxInterruptInEpNumber; j++)
        {
            if (pUsbhDev->hstInterruptInqHD[j] == spTempqHD)
            {
                pUsbhDev->hstIntInWorkqTD[j]        = spHeadqTD;
                pUsbhDev->hstIntInSendLastqTD[j]     = spOldDumyqTD;
            }
        }

        for (j = 0; j< pUsbhDev->bMaxInterruptOutEpNumber; j++)
        {
            if (pUsbhDev->hstInterruptOutqHD[j] == spTempqHD)
            {
                pUsbhDev->hstIntOutWorkqTD[j]        = spHeadqTD;
                pUsbhDev->hstIntOutSendLastqTD[j]     = spOldDumyqTD;
            }
        }
    } // for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)



    
	//<2>.Prepare new dumy qTD      
	spNewDumyqTD=spHeadqTD;
	spNewDumyqTD->bTerminate=1;

	//<3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1
	spLastqTD=spOldDumyqTD;
    dwSwapStartBuffer = spLastqTD->ArrayBufferPointer_Word[0];
    //i=0;
	while(spLastqTD->bTerminate==0) {
        //i++;
        //dwSwapTotalByteLength += spLastqTD->bTotalBytes;
		spLastqTD=(qTD_Structure*)(((DWORD)(spLastqTD->bNextQTDPointer))<<5);
	};

    //if (i>1)
    //dwSwapTotalByteLength += spLastqTD->bTotalBytes;
    //dwSwapStopBuffer = dwSwapStartBuffer+dwSwapTotalByteLength-1;
	spLastqTD->bNextQTDPointer=((DWORD)spNewDumyqTD)>>5;
	spLastqTD->bTerminate=0; 

	//Link Alternate qTD pointer
	spLastqTD->bAlternateQTDPointer=((DWORD)spNewDumyqTD)>>5;
	spLastqTD->bAlternateTerminate=0;   

	//<4>.Enable Timer
	//flib_Host20_TimerEnable_UnLock(1); //1sec
	//bExitLoop=0;

	//<4>.Set OldDumyqTD->Active=1 
	//gwLastqTDSendOK=0;
	//pUsbhDev->psSendLastqTD=spLastqTD;
	pUsbhDev->bSendStatusError=0;
    
	/*/ for wrapper control begin
	if (spOldDumyqTD->bTotalBytes != 0)
	{
        if (psHost20_qHD_List_Interrupt[0] == spTempqHD)
        {
    		mOTG20_Wrapper_SwapBufferStart2_Set(dwSwapStartBuffer);
    		mOTG20_Wrapper_SwapBufferEnd2_Set(dwSwapStopBuffer);
    		mwOTG20_Wrapper_SwapBUffer2Enable_Set();
        }
        else
        {
    		mOTG20_Wrapper_SwapBufferStart1_Set(dwSwapStartBuffer);
    		mOTG20_Wrapper_SwapBufferEnd1_Set(dwSwapStopBuffer);
    		mwOTG20_Wrapper_SwapBUffer1Enable_Set();
        }
	}
	// for wrapper control end*/

//__asm("break 100");
    for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)
    {
        if (pUsbhDev->dwActivedQHD[i] == 0)
        {
            pUsbhDev->dwActivedQHD[i] = (DWORD)spTempqHD;
            for (j = 0; j< 2; j++)
            {
                if (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->pstControlqHD[j])
                {
                    pUsbhDev->psControlqTD = spLastqTD;
                    goto _TD_AVTIVE_;
                }
            }
            
            for (j = 0; j< pUsbhDev->bMaxBulkOutEpNumber; j++)
            {
                if (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstBulkOutqHD[j])
                {
                    pUsbhDev->hstBulkOutqTD[j] = spLastqTD;
                    goto _TD_AVTIVE_;
                }
            }
            
            for (j = 0; j< pUsbhDev->bMaxBulkInEpNumber; j++)
            {
                if (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstBulkInqHD[j])
                {
                    pUsbhDev->hstBulkInqTD[j] = spLastqTD;
                    goto _TD_AVTIVE_;
                }
            }

            for (j = 0; j< pUsbhDev->bMaxInterruptInEpNumber; j++)
            {
                if (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstInterruptInqHD[j])
                {
                    pUsbhDev->hstInterruptInqTD[j] = spLastqTD;
                    goto _TD_AVTIVE_;
                }
            }
            
            for (j = 0; j< pUsbhDev->bMaxInterruptOutEpNumber; j++)
            {
                if (pUsbhDev->dwActivedQHD[i] == (DWORD)pUsbhDev->hstInterruptOutqHD[j])
                {
                    pUsbhDev->hstInterruptOutqTD[j] = spLastqTD;
                    goto _TD_AVTIVE_;
                }
            }            
        } // if (pUsbhDev->dwActivedQHD[i] == 0)
    } // for (i = 0; i < pUsbhDev->sInterfaceDescriptor[0].bNumEndpoints; i++)

_TD_AVTIVE_:
#if USBOTG_HOST_IOC_POLIING_SWITCH                
    if (UseIocIsr4MsdcTx(eWhichOtg)==0)
    {
        PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
        while((psUsbOtg->psUsbReg->HcUsbStatus & 0x01) == 0x01)
        {
            //if (pUsbhDev->bConnectStatus == 0)
            //    break;
        };
        //if (pUsbhDev->bConnectStatus == 0)
        //    mpDebugPrint("1PLUG OUT");
    }
#endif
	spOldDumyqTD->bStatus_Active=1;
    g_spTempqTD = spOldDumyqTD;

#if USBOTG_HOST_IOC_POLIING_SWITCH                
    if (UseIocIsr4MsdcTx(eWhichOtg)==0)
    {
        cnt_1 = 0;
        PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
        while((psUsbOtg->psUsbReg->HcUsbStatus & 0x01) != 0x01)
        {
            cnt_1++;
            //if (pUsbhDev->bConnectStatus == 0 || cnt_1 > 0xfffff)
            if (cnt_1 > 0xfffff)
                break;
        }

        if (cnt_1 > 0xfffff)
            mpDebugPrint("TIME OUT");
        //if (pUsbhDev->bConnectStatus == 0)
        //    mpDebugPrint("2PLUG OUT");
        mwHost20_USBSTS_CompletionOfTransaction_Set();
        UsbOtgHostEventIoc(eWhichOtg);
    }
#endif
}




//====================================================================
// * Function Name: flib_Host20_Send_qTD_Done                          
// * Description: 
//   Case-1:1qTD
//   Case-2:2qTD
//   Case-3:3qTD above
// * Input:Type
// * OutPut: 
//====================================================================
BYTE flib_Host20_Send_qTD_Done(qHD_Structure *spTempqHD, WHICH_OTG eWhichOtg)
{
	volatile qTD_Structure           *spNewDumyqTD; 
	volatile qTD_Structure           *spReleaseqTD;    
	volatile qTD_Structure           *spReleaseqTDNext;    
    BYTE i = 0;
    BYTE j = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

// force update cache
    SetDataCacheInvalid();

    for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)
    {
        if (pUsbhDev->dwActivedQHD[i] == (DWORD)spTempqHD)
        {
            pUsbhDev->dwActivedQHD[i] = 0;
            break;
        }
    }
/*
    if (pUsbhDev->hstInterruptqHD[0] == (qHD_Structure*)spTempqHD)
    {
        //if (mwOTG20_Wrapper_SwapBUffer2Enable_Rd() != 0)
        //    mwOTG20_Wrapper_SwapBUffer2Enable_Clr();
        spNewDumyqTD = pUsbhDev->pstIntWorkqTD;
        spReleaseqTD = pUsbhDev->psIntSendLastqTD;
    }
    else
    {
    	//if (mwOTG20_Wrapper_SwapBUffer1Enable_Rd() != 0)
    	//	mwOTG20_Wrapper_SwapBUffer1Enable_Clr();
        spNewDumyqTD = pUsbhDev->pstWorkqTD;
        spReleaseqTD = pUsbhDev->psSendLastqTD;
    }
*/
    //for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)
    {
        for (j = 0; j< 2; j++)
        {
            if (pUsbhDev->pstControlqHD[j] == spTempqHD)
            {
                spNewDumyqTD = pUsbhDev->pstControlWorkqTD;
                spReleaseqTD = pUsbhDev->pstControlSendLastqTD;
                goto _TD_DONE_;
            }
        }
                    
        for (j = 0; j< pUsbhDev->bMaxBulkInEpNumber; j++)
        {
            if (pUsbhDev->hstBulkInqHD[j] == spTempqHD)
            {
                pUsbhDev->dwWhichBulkPipeDone &= ~(1<<(j+16));
                spNewDumyqTD = pUsbhDev->hstBulkInWorkqTD[j];
                spReleaseqTD = pUsbhDev->hstBulkInSendLastqTD[j];
                goto _TD_DONE_;
            }
        }

        for (j = 0; j< pUsbhDev->bMaxBulkOutEpNumber; j++)
        {
            if (pUsbhDev->hstBulkOutqHD[j] == spTempqHD)
            {
                pUsbhDev->dwWhichBulkPipeDone &= ~(1<<j);
                spNewDumyqTD = pUsbhDev->hstBulkOutWorkqTD[j];
                spReleaseqTD = pUsbhDev->hstBulkOutSendLastqTD[j];
                goto _TD_DONE_;
            }
        }
        
        for (j = 0; j< pUsbhDev->bMaxInterruptInEpNumber; j++)
        {
            if (pUsbhDev->hstInterruptInqHD[j] == spTempqHD)
            {
            	pUsbhDev->dwWhichInterruptPipeDone &= ~(1<<(j+16));
                spNewDumyqTD = pUsbhDev->hstIntInWorkqTD[j];
                spReleaseqTD = pUsbhDev->hstIntInSendLastqTD[j];
                goto _TD_DONE_;
            }
        }

        for (j = 0; j< pUsbhDev->bMaxInterruptOutEpNumber; j++)
        {
            if (pUsbhDev->hstInterruptOutqHD[j] == spTempqHD)
            {
            	pUsbhDev->dwWhichInterruptPipeDone &= ~(1<<j);
                spNewDumyqTD = pUsbhDev->hstIntOutWorkqTD[j];
                spReleaseqTD = pUsbhDev->hstIntOutSendLastqTD[j];
                goto _TD_DONE_;
            }
        }        
    } // for (i = 0; i < MAX_NUM_OF_ENDPOINT; i++)

_TD_DONE_:
	//<6>.Checking the Result
	//if (gwLastqTDSendOK==0)
    pUsbhDev->bQHStatus = spTempqHD->bOverlay_Status;
	flib_Host20_CheckingForResult_QHD(spTempqHD, eWhichOtg); 	
    if (pUsbhDev->bQHStatus & HOST20_qTD_STATUS_Halted)
    {
        if ((pUsbhDev->bQHStatus & 0x7F) != HOST20_qTD_STATUS_Halted)
        {
            //__asm("break 100");
            MP_DEBUG("bQHStatus = 0x%x", pUsbhDev->bQHStatus);
            pUsbhDev->bQHStatus = 0;
        }
        else
        {
            pUsbhDev->sSetupPB.dwSetupState = SETUP_DONE_STATE;
        }
    }
    else
    {
        pUsbhDev->bQHStatus = 0;
    }
	//<5>.Release the all the qTD (Not include spNewDumyqTD)
	do {
#if USB_WIFI_ENABLE
        struct ehci_qtd *qtd;
        qtd = (struct ehci_qtd *)spReleaseqTD;
        if (qtd->urb)
        {
            usb_urb_complete(qtd->urb, spReleaseqTD);
        }
#endif
		spReleaseqTDNext = (qTD_Structure*)(((DWORD)(spReleaseqTD->bNextQTDPointer))<<5);
		flib_Host20_ReleaseStructure(Host20_MEM_TYPE_qTD,(DWORD)spReleaseqTD, eWhichOtg);
		spReleaseqTD = spReleaseqTDNext;
        if (spReleaseqTD == 0)
        {
            MP_DEBUG("flib_Host20_Send_qTD_Done:no TD released!!");
            MP_DEBUG("spTempqHD = 0x%x", spTempqHD);
            break;
        }
	} while(((DWORD)spReleaseqTD)!=((DWORD)spNewDumyqTD));
}
    

//====================================================================
// * Function Name: flib_Host20_CheckingForResult_QHD                          
// * Description: 
// * Input:Type
// * OutPut: 
//====================================================================
void flib_Host20_CheckingForResult_QHD(qHD_Structure *spTempqHD, WHICH_OTG eWhichOtg)
{
    BYTE bQHStatus;
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if (psUsbhDevDesc->bSendStatusError == 0)
        return;// USB_NO_ERROR;

    //<1>.Analysis the qHD Status
    psUsbhDevDesc->bSendStatusError=0;
    bQHStatus = spTempqHD->bOverlay_Status;
    //MP_ALERT("--E-- %s USBOTG%d USB Status Error 0x%x", __FUNCTION__, eWhichOtg, bQHStatus);
    if (bQHStatus&HOST20_qTD_STATUS_Halted)
    {
        //__asm("break 100");
        MP_DEBUG("--E-- USBOTG%d qHD Status => Halted (<1>.Stall/<2>.Babble/<3>.Error Counter=0)...(Device Not Supported...)", eWhichOtg);
        spTempqHD->bOverlay_Status &= HOST20_qTD_STATUS_Halted;
    }

    if (bQHStatus&HOST20_qTD_STATUS_BufferError)
    {
        MP_DEBUG("--E-- USBOTG%d qHD Status => HOST20_qTD_STATUS_BufferError...", eWhichOtg);
    }

    if (bQHStatus&HOST20_qTD_STATUS_Babble) 
    {
        MP_DEBUG("--E-- USBOTG%d qHD Status => HOST20_qTD_STATUS_Babble...", eWhichOtg);
    }

    if (bQHStatus&HOST20_qTD_STATUS_TransactionError) 
    {
        MP_DEBUG("--E-- USBOTG%d qHD Status => HOST20_qTD_STATUS_TransactionError...", eWhichOtg);
    }

    if (bQHStatus&HOST20_qTD_STATUS_MissMicroFrame) 
    {  //john, what's this ?
        MP_DEBUG("--E-- USBOTG%d qHD Status => HOST20_qTD_STATUS_MissMicroFrame...", eWhichOtg);
    }
   
   //<2>.Clear the status
   spTempqHD->bOverlay_Status=0;
}	


//====================================================================
// * Function Name: flib_Host20_ReleaseStructure                          
// * Description: 
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2 
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
void flib_Host20_ReleaseStructure(BYTE Type,DWORD pwAddress, WHICH_OTG eWhichOtg)
{
   DWORD i;
   DWORD wReleaseNum;
   DWORD *pData;
   DWORD qtd_base_address = 0;
   PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

   pData=(DWORD*)pwAddress;

   qtd_base_address = psEhci->dwHostStructureQtdBaseAddress;
   switch(Type) {
      case Host20_MEM_TYPE_qTD:  
         if (pwAddress<qtd_base_address) {
            MP_DEBUG("1??? Memory release area fail...into umlimited while loop");
            while(1);
         }
             
         if ((pwAddress-qtd_base_address)==0)  //john, why need to check ?
            wReleaseNum=0;	
         else
            wReleaseNum=(pwAddress-qtd_base_address)/Host20_qTD_SIZE;
                
         if (wReleaseNum>=Host20_qTD_MAX) {
            MP_DEBUG("2??? Memory release area fail...into umlimited while loop");
            while(1);
         }

         psEhci->pbHostQtdManage[wReleaseNum]=Host20_MEM_FREE;
             
         for (i=0;i<Host20_qTD_SIZE/4;i++) //qTD size=32 bytes
             *(pData+i)=0;
         break;

#if USBOTG_HOST_ISOC
      case Host20_MEM_TYPE_iTD:
      case Host20_MEM_TYPE_siTD: 
         if (pwAddress < psEhci->dwHostStructureItdBaseAddress) {
            MP_DEBUG("3??? Memory release area fail...into umlimited while loop");
            while(1);
         }

         if ((pwAddress-psEhci->dwHostStructureItdBaseAddress)==0)
            wReleaseNum=0;	
         else
            wReleaseNum=(pwAddress-psEhci->dwHostStructureItdBaseAddress)/Host20_iTD_SIZE;

         if (wReleaseNum>Host20_iTD_MAX) {
            MP_DEBUG("4??? Memory release area fail...into unlimited while loop");
            while(1);
        }
             
         psEhci->pbHostItdManage[wReleaseNum]=Host20_MEM_FREE;
             
         for (i=0;i<Host20_iTD_SIZE/4;i++) //iTD size=64 bytes
             *(pData+i)=0;        
         break;
      case Host20_MEM_TYPE_4K_BUFFER:      
         if (pwAddress < psEhci->dwHostDataPageBaseAddress) {
            MP_DEBUG("5??? Memory release area fail...into umlimited while loop");
            while(1);
        }

         if ((pwAddress-psEhci->dwHostDataPageBaseAddress)==0)
            wReleaseNum=0;	
         else
            wReleaseNum=(pwAddress-psEhci->dwHostDataPageBaseAddress)/Host20_Page_SIZE;

         if (wReleaseNum>Host20_Page_MAX) {
            MP_DEBUG("6??? Memory release area fail...into umlimited while loop");
            while(1);
        }

         psEhci->pbHostDataPageManage[wReleaseNum]=Host20_MEM_FREE;
         break;
#endif     
      default: 
         MP_DEBUG("7??? Memory release type fail...into umlimited while loop");
         while(1);    
         break;		
   }
}


//====================================================================
// * Function Name: flib_Host20_StopRun_Setting                          
// * Description: 
// * Input: 
// * OutPut: 
//====================================================================
void flib_Host20_StopRun_Setting(BYTE bOption, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
        
   if (bOption==HOST20_Enable) {
      if (mbHost20_USBCMD_RunStop_Rd()>0)
         return;	
     
      mbHost20_USBCMD_RunStop_Set();
     
      while(mbHost20_USBCMD_RunStop_Rd()==0);	     	
   }
   else if (bOption==HOST20_Disable) {
      if (mbHost20_USBCMD_RunStop_Rd()==0)
         return;	

      //Start;;Bruce;;06282005;;Make sure the Async schedule is disbale 
      if (mbHost20_USBCMD_AsynchronousEnable_Rd()>0) {//Disable the Asynchronous Schedule
         mbHost20_USBCMD_AsynchronousEnable_Clr(); 	
         while(mwHost20_USBSTS_AsynchronousStatus_Rd()>0);	
      }
      //End;;Bruce;;06282005;;Make sure the Async schedule is disbale 

      //John got from Richard, disable periodical schedule
      if (mbHost20_USBCMD_PeriodicEnable_Rd()>0) {  //Disable the Periodic Schedule
         mbHost20_USBCMD_PeriodicEnable_Clr(); 	
         while(mwHost20_USBSTS_PeriodicStatus_Rd()>0);	
      }

      mbHost20_USBCMD_RunStop_Clr();             
      while(mbHost20_USBCMD_RunStop_Rd()>0);	     	
   }
   else {
	  MP_DEBUG("??? Input Error 'flib_Host20_StopRun_Setting'...into umlimited while loop");
      while(1);
   }
}

//====================================================================
// * Function Name: flib_Host20_Asynchronous_Setting                          
// * Description: 
// * Input: 
// * OutPut: 
//====================================================================
void flib_Host20_Asynchronous_Setting(BYTE bOption, WHICH_OTG eWhichOtg)
{ 
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    
    if (bOption==HOST20_Enable) {
      if (mwHost20_USBSTS_AsynchronousStatus_Rd()>0)
         return;	
     
      mbHost20_USBCMD_AsynchronousEnable_Set();
     
      while(mwHost20_USBSTS_AsynchronousStatus_Rd()==0);
     	
   }
   else if (bOption==HOST20_Disable) {
      if (mwHost20_USBSTS_AsynchronousStatus_Rd()==0)
         return;	
                
      mbHost20_USBCMD_AsynchronousEnable_Clr();
                
      while(mwHost20_USBSTS_AsynchronousStatus_Rd()>0);     	
     	
   }
   else {
      MP_DEBUG("??? USBOTG%d Input Error 'flib_Host20_Asynchronous_Setting'...into umlimited while loop", eWhichOtg);
      while(1);
   }
}


//====================================================================
// * Function Name: flib_Host20_Periodic_Setting                          
// * Description: 
// * Input: 
// * OutPut: 
//====================================================================
void flib_Host20_Periodic_Setting(BYTE bOption, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
        
   if (bOption==HOST20_Enable) 
   	{

      //<1>.If Already enable => return 
      if (mwHost20_USBSTS_PeriodicStatus_Rd()>0)
         return ;
     
      //<2>.Disable Periodic
      mbHost20_USBCMD_PeriodicEnable_Set();
   
      //<3>.Polling Status
      while(mwHost20_USBSTS_PeriodicStatus_Rd()==0);  	
   }
   else if (bOption==HOST20_Disable) 
   	{
      //<1>.If Already Disable => return 
      if (mwHost20_USBSTS_PeriodicStatus_Rd()==0)
         return ;
                 
      //<2>.Enable Periodic
      mbHost20_USBCMD_PeriodicEnable_Clr();
      //<3>.Polling Status
      IODelay(10);// need to double check
       if (mwHost20_USBSTS_PeriodicStatus_Rd()>0)
	   	  mbHost20_USBCMD_HCReset_Set();

      //while(mwHost20_USBSTS_PeriodicStatus_Rd()>0);  
   }
   else {
	  MP_DEBUG("??? Input Error 'flib_Host20_Periodic_Setting'...into umlimited while loop");
      while(1);
   }
}


//************************************************************************************************************
//************************************************************************************************************
//                          *** Group-4:Structure Function ***
//*************************************************************************************************************
//************************************************************************************************************
//====================================================================
// * Function Name: flib_Host20_InitStructure                          
// * Description: 
//              1.Init qHD for Control
//                qHD_C-->qHD_C-->qHD_C
//              2.Init qHD for Bulk
//                |-------------------------|   
//                qHD_C-->qHD_C-->qHD_B-->qHD_B
//                   
//              3.Init qHD for Interrupt
//              4.Init iTD for ISO (Reserved for feature)
// * Input:Type =0 =>iTD
//              =1 =>qTD
//              =2 
// * OutPut: 0:Fail
//           1:ok
//====================================================================
static void flib_Host20_InitStructure(WHICH_OTG eWhichOtg)
{
   DWORD             i;
   DWORD            *pData;
   PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
   PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
   
   psEhci->psHostFramList = (PERIODIC_FRAME_LIST_STRUCTURE*)psEhci->dwHostStructurePflBaseAddress;

   //<2>.For qTD & iTD & 4K-Buffer Manage init
   for (i=0;i<Host20_qTD_MAX;i++) //pbHostQtdManage
        psEhci->pbHostQtdManage[i]=Host20_MEM_FREE;

#if USBOTG_HOST_ISOC
   for (i=0;i<Host20_iTD_MAX;i++)//pbHostItdManage
        psEhci->pbHostItdManage[i]=Host20_MEM_FREE;

   for (i=0;i<Host20_Page_MAX;i++)//pbHostDataPageManage
        psEhci->pbHostDataPageManage[i]=Host20_MEM_FREE;
#endif
//   gpUsbhDeviceDescriptor->bDataBuffer=(BYTE*)flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER);//For Control
   psUsbhDevDesc->bDataBuffer = allo_mem(1024,eWhichOtg);//For Control

/* // move to UsbOtgHostGetQhdForEachEd
   //<3>.For Asynchronous 
   //<3.1>.Init qHD for Control(1~2)
   psHost20_qHD_List_Control[0]=(qHD_Structure*)gHost20_STRUCTURE_qHD_BASE_ADDRESS;
   psHost20_qHD_List_Control[1]=(qHD_Structure*)(gHost20_STRUCTURE_qHD_BASE_ADDRESS+Host20_qHD_SIZE*1);

   flib_Host20_Allocate_QHD(psHost20_qHD_List_Control[0],HOST20_HD_Type_QH,0,1,0,8);//Address=0,Head=1,EndPt=0,Size
   //flib_Host20_Allocate_QHD(psHost20_qHD_List_Control[1],HOST20_HD_Type_QH,1,0,0,64);//Address=1,Head=0,EndPt=0,Size
   flib_Host20_Allocate_QHD(psHost20_qHD_List_Control[1],HOST20_HD_Type_QH,1,0,0,64);//Address=1,Head=0,EndPt=0,Size
  
   //<3.2>.Init qHD for Bulk(4~6)
   psHost20_qHD_List_Bulk[0]=(qHD_Structure*)(gHost20_STRUCTURE_qHD_BASE_ADDRESS+Host20_qHD_SIZE*2);
   psHost20_qHD_List_Bulk[1]=(qHD_Structure*)(gHost20_STRUCTURE_qHD_BASE_ADDRESS+Host20_qHD_SIZE*3);

   flib_Host20_Allocate_QHD(psHost20_qHD_List_Bulk[0],HOST20_HD_Type_QH,1,0,1,64);//Address=1,Head=0,EndPt=1,Size
   flib_Host20_Allocate_QHD(psHost20_qHD_List_Bulk[1],HOST20_HD_Type_QH,1,0,2,64);//Address=1,Head=0,EndPt=2,Size

   //<3.3>.Link the qHD
   psHost20_qHD_List_Control[0]->bNextQHDPointer=(((DWORD)psHost20_qHD_List_Control[1])>>5);
   psHost20_qHD_List_Control[1]->bNextQHDPointer=(((DWORD)psHost20_qHD_List_Control[0])>>5);
*/
}

//====================================================================
// * Function Name: flib_Host20_Interrupt_Init                          
// * Description: 
//   //Reserve=> <1>.If Periodic Enable => Disable Periodic
//   <2>.Disable all the Frame List (Terminal=1)           
//   <3>.Hang the Interrupt-qHD-1 to Frame List
//   
// * Input: wInterval=1~16 => Full Speed => 1ms ~ 32 sec
//                            High Speed => 125us ~ 40.96 sec 
// * OutPut: 
//====================================================================
void flib_Host20_Interrupt_Init(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_HOST_EHCI psEhci = &psUsbOtg->sUsbHost.sEhci;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
#if (USBOTG_HOST_DESC == DISABLE) // Don't assign value
    pUsbhDev->psAppClass->bIntInQHDArrayNum = 0;
#endif
    //<4>.Set Periodic Base Address // dwHostStructurePflBaseAddress
    mwHost20_PeriodicBaseAddr_Set(psEhci->dwHostStructurePflBaseAddress);	

    //<5>.Enable the periodic 
    flib_Host20_Periodic_Setting(HOST20_Enable, eWhichOtg);
}

//====================================================================
// * Function Name: flib_Host20_GetStructure                          
// * Description: 
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2 =>4K Buffer
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
DWORD flib_Host20_GetStructure(BYTE Type, WHICH_OTG eWhichOtg)
{
    DWORD i;
    BYTE bFound;
    qTD_Structure *spTempqTD;
    iTD_Structure  *spTempiTD;
    siTD_Structure  *spTempsiTD;
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

    bFound=0;

    switch(Type) {
      case Host20_MEM_TYPE_qTD:  //For qTD
         for (i=0;i<Host20_qTD_MAX;i++)
        if (psEhci->pbHostQtdManage[i]==Host20_MEM_FREE) {
                bFound=1;
                psEhci->pbHostQtdManage[i]=Host20_MEM_USED;
                break;     
             }

         if (bFound==1){
            spTempqTD=(qTD_Structure*)(psEhci->dwHostStructureQtdBaseAddress+i*Host20_qTD_SIZE);
            spTempqTD->bTerminate=1;         //Bit0  
            spTempqTD->bStatus_Active=0;             //Bit7 
            spTempqTD->bInterruptOnComplete=1;   //Bit15   
            //spTempqTD->bStatus_Active=1;    
            spTempqTD->bAlternateTerminate=1;    
            spTempqTD->bErrorCounter=3;                        
            spTempqTD->bDataToggle=0;                        
            return ((DWORD)spTempqTD|0xa0000000);
         }
         else
         MP_ASSERT(0);
        break;
#if USBOTG_HOST_ISOC
      case Host20_MEM_TYPE_iTD:  //For iTD
         for (i=0;i<Host20_iTD_MAX;i++)
        if (psEhci->pbHostItdManage[i]==Host20_MEM_FREE) {
                bFound=1;
                psEhci->pbHostItdManage[i]=Host20_MEM_USED;
                spTempiTD=(iTD_Structure*)(psEhci->dwHostStructureItdBaseAddress+i*Host20_iTD_SIZE);
                memset(spTempiTD, 0, sizeof(iTD_Structure));
                spTempiTD->bTerminate=1; 
                spTempiTD->bType=HOST20_HD_Type_iTD;      
                return(DWORD)(spTempiTD);
             }
        break;
    /*
      case Host20_MEM_TYPE_siTD:  //For siTD
         for (i=0;i<Host20_iTD_MAX;i++)
        if (psEhci->pbHostItdManage[i]==Host20_MEM_FREE) {
                bFound=1;
                psEhci->pbHostItdManage[i]=Host20_MEM_USED;
                spTempiTD=(iTD_Structure*)(psEhci->dwHostStructureItdBaseAddress+i*Host20_iTD_SIZE);
                spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction=psDevice_AP->bAdd;
                spTempiTD->bTerminate=1; 
                spTempiTD->bType=HOST20_HD_Type_iTD;      
                return(DWORD)(spTempiTD);
             }
        break;
    */
        case Host20_MEM_TYPE_4K_BUFFER:  //For 4K_BUFFER 
        for (i=0;i<Host20_Page_MAX;i++)
        if (psEhci->pbHostDataPageManage[i]==Host20_MEM_FREE) {
               bFound=1;
               psEhci->pbHostDataPageManage[i]=Host20_MEM_USED;
                memset((BYTE*)((DWORD)(psEhci->dwHostDataPageBaseAddress+i*Host20_Page_SIZE)), 0x00, Host20_Page_SIZE);
               return (((DWORD)(psEhci->dwHostDataPageBaseAddress+i*Host20_Page_SIZE)));    
            }
    break;
#endif

     default:
        // return 0;
    break;
    }

    //Not Found...
    MP_ALERT("%s:Type = %d not found!!", __FUNCTION__, Type);
    MP_ALERT("<Type Not Found>");
    return (0);
}


//====================================================================
// * Function Name: flib_Host20_Allocate_QHD                          
// * Description: 
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2 
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
static void flib_Host20_Allocate_QHD ( qHD_Structure   *psQHTemp,
                                BYTE            bNextType,
                                BYTE            bAddress,
                                BYTE            bHead,
                                BYTE            bEndPt, 
                                DWORD           wMaxPacketSize,
                                WHICH_OTG       eWhichOtg)
{
   qTD_Structure *spTempqTD;
  
   //<1>.Set the QHead
  
   //<1.1>.qHD_1 Word
   psQHTemp->bTerminate=0;                 //Bit0  
   psQHTemp->bType=bNextType;              //Bit2~1           
                
   //<1.2>.qHD_2 Word
   psQHTemp->bDeviceAddress=bAddress;             //Bit0~6  
   //?? psQHTemp->bInactiveOnNextTransaction:1; //Bit7           //??
   psQHTemp->bEdNumber=bEndPt;                  //Bit11~8   
   //?? psQHTemp->bEdSpeed:2;                   //Bit13~12 
   //??  psQHTemp->bDataToggleControl=1;         //Bit14:1 for control 0 for Bulk    
   psQHTemp->bDataToggleControl=0;         //Bit14:1 for control 0 for Bulk    
   psQHTemp->bHeadOfReclamationListFlag=bHead;   //Bit15 
   psQHTemp->bMaxPacketSize=wMaxPacketSize;      //Bit16~26 
   //  psQHTemp->bControlEdFlag:1;             //Bit27 
   // psQHTemp->bNakCounter=15;                //Bit28~31 
   psQHTemp->bNakCounter=0;//Temp Solution from 15 to 0                //Bit28~31 
   //<1.3>.qHD_3 Word     
   //psQHTemp->bInterruptScheduleMask:8;     //Bit0~7
   //psQHTemp->bSplitTransactionMask:8;      //Bit8~15
   //psQHTemp->bHubAddr:7;                   //Bit16~22  
   //psQHTemp->bPortNumber:7;                //Bit23~29  
   //psQHTemp->bHighBandwidth:2;             //Bit30~31  
  
   psQHTemp->bOverlay_NextTerminate=1;
   psQHTemp->bOverlay_AlternateNextTerminate=1;
   psQHTemp->bOverlay_Direction = 0;  
   //<2>.allocate dumy qTD

   //<2.1>.Allocate qTD
   spTempqTD=(qTD_Structure*)flib_Host20_GetStructure(0, eWhichOtg);//0=>qTD
   //<2.2>.Put qTD to QHD
   psQHTemp->bOverlay_NextqTD=(((DWORD)spTempqTD)>>5);
   //<2.3>.Active the qTD
   psQHTemp->bOverlay_NextTerminate=0;	
}	

//====================================================================
// * Function Name: flib_Host20_QHD_Control_Init                          
// * Description: 
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2 
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
void flib_Host20_QHD_Control_Init(WHICH_OTG eWhichOtg)
{
    DWORD i =0;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    //<1>.Init Control-0/1
    pUsbhDev->pstControlqHD[0]->bEdSpeed=pUsbhDev->bDeviceSpeed;
    pUsbhDev->pstControlqHD[0]->bInactiveOnNextTransaction = 0;
    pUsbhDev->pstControlqHD[0]->bDataToggleControl         = 1;

    pUsbhDev->pstControlqHD[1]->bEdSpeed=pUsbhDev->bDeviceSpeed;
    pUsbhDev->pstControlqHD[1]->bInactiveOnNextTransaction = 0;
    pUsbhDev->pstControlqHD[1]->bDataToggleControl         = 1;
/*
    //<2>.Init Bulk-0/1
    for (i = 0; i < pUsbhDev->bMaxBulkEpNumber; i++)
    {
        pUsbhDev->hstBulkInqHD[i]->bEdSpeed=pUsbhDev->bDeviceSpeed;
        pUsbhDev->hstBulkInqHD[i]->bInactiveOnNextTransaction =0;
        pUsbhDev->hstBulkInqHD[i]->bDataToggleControl=0;
    }

    for (i = 0; i < pUsbhDev->bMaxInterruptEpNumber; i++)
    {
        pUsbhDev->hstInterruptqHD[i]->bEdSpeed=pUsbhDev->bDeviceSpeed;
        pUsbhDev->hstInterruptqHD[i]->bInactiveOnNextTransaction =0;
        pUsbhDev->hstInterruptqHD[i]->bDataToggleControl=0;
    }
*/    
    //<12>.Enable Asynchronous 
    mbHost20_USBCMD_AsynchronousEnable_Set();   
   
}	


//====================================================================
// * Function Name: flib_Host20_Issue_Bulk                          
// * Description: Input data must be 4K-Alignment 
//               <1>.MaxSize=20 K
//               <2>.Support Only 1-TD
// * Input: 
// * OutPut: 
//====================================================================
qTD_Structure *  flib_Host20_Issue_Bulk_Active_Multi_TD (  BYTE    bArrayListNum,
                                                           WORD    hwSize,
                                                           DWORD   *pwBufferArray,
                                                           DWORD   wOffset,
                                                           BYTE    bDirection,
                                                           BOOL    fActive,
                                                           WHICH_OTG eWhichOtg)
{
    qHD_Structure *spBulkqHD;
    qTD_Structure *spCurrentTempqTD;
    //static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;
    //static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    // Fill TD
    spCurrentTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg); //The qTD will be release in the function "Send"
    spCurrentTempqTD->bTotalBytes=hwSize;           
    spCurrentTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); 
    spCurrentTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++; 
    spCurrentTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
    spCurrentTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
    spCurrentTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
    ((struct ehci_qtd *)spCurrentTempqTD)->urb  = pUsbhDevDes->urb;

    // Analysis the Direction 
    if (bDirection) 
    {
        spCurrentTempqTD->bPID = HOST20_qTD_PID_IN;
        spBulkqHD              = pUsbhDevDes->hstBulkInqHD[bArrayListNum];
    }
    else
    {
        spCurrentTempqTD->bPID = HOST20_qTD_PID_OUT;
        spBulkqHD              = pUsbhDevDes->hstBulkOutqHD[bArrayListNum];
    }

    if (fActive)
    { // Send TD
        psEhci->psPreviousTempqTD->bNextQTDPointer       = (((DWORD)spCurrentTempqTD)>>5);
        psEhci->psPreviousTempqTD->bAlternateQTDPointer  = (((DWORD)spCurrentTempqTD)>>5);             
        spCurrentTempqTD->bTerminate                = 1; // now, spCurrentTempqTD is the last TD
        spCurrentTempqTD->bAlternateTerminate       = 1;             
        spCurrentTempqTD->bStatus_Active            = 1;
        spCurrentTempqTD->bInterruptOnComplete      = 1;             
        flib_Host20_Send_qTD_Active(psEhci->psFirstTempqTD, spBulkqHD, eWhichOtg);
        psEhci->psPreviousTempqTD = OTGH_NULL;
        psEhci->psFirstTempqTD    = OTGH_NULL;
    }
    else
    { // link TD
        spCurrentTempqTD->bTerminate            = 0;
        spCurrentTempqTD->bAlternateTerminate   = 0;             
        spCurrentTempqTD->bInterruptOnComplete  = 0;             
        if (psEhci->psFirstTempqTD == OTGH_NULL)
        {
            psEhci->psFirstTempqTD = spCurrentTempqTD;
        }
        else
        {
            psEhci->psPreviousTempqTD->bNextQTDPointer       = (((DWORD)spCurrentTempqTD)>>5);
            psEhci->psPreviousTempqTD->bAlternateQTDPointer  = (((DWORD)spCurrentTempqTD)>>5);             
            spCurrentTempqTD->bStatus_Active            = 1;
        }
        
        psEhci->psPreviousTempqTD = spCurrentTempqTD;
    }

    return spCurrentTempqTD;
}


//====================================================================
// * Function Name: flib_Host20_Issue_Bulk                          
// * Description: Input data must be 4K-Alignment 
//               <1>.MaxSize=20 K
//               <2>.Support Only 1-TD
// * Input: 
// * OutPut: 
//====================================================================
qTD_Structure *  flib_Host20_Issue_Bulk_Active (   BYTE    bArrayListNum,
                                                   WORD    hwSize,
                                                   DWORD   *pwBufferArray,
                                                   DWORD   wOffset,
                                                   BYTE    bDirection,
                                                   WHICH_OTG eWhichOtg)
{
    qHD_Structure *spBulkqHD;
    qTD_Structure *spTempqTD;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    //<1>.Fill TD
    spTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg); //The qTD will be release in the function "Send"
    spTempqTD->bTotalBytes=hwSize;           
    spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); 
    spTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
    spTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
   ((struct ehci_qtd *)spTempqTD)->urb = pUsbhDevDes->urb;

    //<2>.Analysis the Direction 
    if (bDirection)
    {
        spTempqTD->bPID = HOST20_qTD_PID_IN;
        spBulkqHD       = pUsbhDevDes->hstBulkInqHD[bArrayListNum];
    }
    else
    {
        spTempqTD->bPID = HOST20_qTD_PID_OUT;
        spBulkqHD       = pUsbhDevDes->hstBulkOutqHD[bArrayListNum];
    }

    //<3>.Send TD
    flib_Host20_Send_qTD_Active(spTempqTD, spBulkqHD, eWhichOtg);
    return spTempqTD;
}


//====================================================================
// * Function Name: flib_Host20_Issue_Interrupt                          
// * Description: 
// * Input: 
// * OutPut: 
//====================================================================
BYTE  flib_Host20_Issue_Interrupt_Active (  BYTE    bArrayListNum,
                                            WORD    hwSize,
                                            DWORD   *pwBufferArray,
                                            DWORD   wOffset,
                                            BYTE    bDirection,
                                            WHICH_OTG eWhichOtg)
{
    qHD_Structure *spIntqHD;
    qTD_Structure *spTempqTD;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

   
//__asm("break 100");
   //<1>.Fill TD
   // mpDebugPrint("%s", __FUNCTION__);
   spTempqTD =(qTD_Structure*)flib_Host20_GetStructure(Host20_MEM_TYPE_qTD, eWhichOtg); //The qTD will be release in the function "Send"
   spTempqTD->bTotalBytes=hwSize;           
   spTempqTD->ArrayBufferPointer_Word[0]=(DWORD)((*pwBufferArray++)+wOffset); // john, allow non-alignment 
   spTempqTD->ArrayBufferPointer_Word[1]=(DWORD)*pwBufferArray++;  
   spTempqTD->ArrayBufferPointer_Word[2]=(DWORD)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[3]=(DWORD)*pwBufferArray++; 
   spTempqTD->ArrayBufferPointer_Word[4]=(DWORD)*pwBufferArray++; 
   ((struct ehci_qtd *)spTempqTD)->urb = pUsbhDevDes->urb;

    //<2>.Analysis the Direction 
    if (bDirection)
    {
        spTempqTD->bPID = HOST20_qTD_PID_IN;
        spIntqHD       = pUsbhDevDes->hstInterruptInqHD[bArrayListNum];
    }
    else
    {
        spTempqTD->bPID = HOST20_qTD_PID_OUT;
        spIntqHD       = pUsbhDevDes->hstInterruptOutqHD[bArrayListNum];
    }   
      
   //<3>.Send TD
   flib_Host20_Send_qTD_Active(spTempqTD, spIntqHD, eWhichOtg);
}


//====================================================================
// * Function Name: flib_Host20_Suspend                          
// * Description: 
//   <1>.Make sure PortEnable=1
//   <2>.Write PORTSC->Suspend=1
//   <3>.Waiting for the ISR->PORTSC->Suspend=1
// * Input: 
// * OutPut: 0:OK
//           1:Fail
//====================================================================
BYTE flib_Host20_Suspend(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

   if (mbHost20_USBCMD_RunStop_Rd()==0)    
      return(1);
      
   //<1>.Make sure PortEnable
   if (mwHost20_PORTSC_EnableDisable_Rd()==0)    
      return(1);

   //Start;;Bruce;;06282005;;Make sure the Async schedule is disbale 
   if (mbHost20_USBCMD_AsynchronousEnable_Rd()>0) {//Disable the Asynchronous Schedule
      mbHost20_USBCMD_AsynchronousEnable_Clr(); 	
      while (mwHost20_USBSTS_AsynchronousStatus_Rd()>0);	
   }
   //End;;Bruce;;06282005;;Make sure the Async schedule is disbale 

   //John got from Richard, disable periodical schedule
   if (mbHost20_USBCMD_PeriodicEnable_Rd()>0) {  //Disable the Periodic Schedule
      mbHost20_USBCMD_PeriodicEnable_Clr(); 	
      while(mwHost20_USBSTS_PeriodicStatus_Rd()>0);	
   }

   //<2>.Write PORTSC->Suspend=1
   flib_Host20_StopRun_Setting(HOST20_Disable, eWhichOtg);//For Faraday HW request
   
   //<3>.Write PORTSC->Suspend=1     
   mwHost20_PORTSC_ForceSuspend_Set();

   //<4>.Waiting for the PORTSC->Suspend=1
   while (mwHost20_PORTSC_ForceSuspend_Rd()==0);

   return (0);
}


//====================================================================
// * Function Name: flib_Host20_Resume                          
// * Description: 
//   <1>.Make Sure PORTSC->Suspend =1   
//   <2>.Write PORTSC->ForcePortResume=1
//   <3>.Waiting for the ForcePortResume=0
// * Input: 
// * OutPut: 
//====================================================================
BYTE flib_Host20_Resume(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    //<1>.Make Sure PORTSC->Suspend =1   
    //while (mwHost20_PORTSC_ForceSuspend_Rd()==0);    

    //<2>.Write PORTSC->ForcePortResume=1
    mwHost20_PORTSC_ForceResume_Set();

    //<3>.Waiting for the ForcePortResume=0
    while (mwHost20_PORTSC_ForceResume_Rd()==0);

    //<4>.Write PORTSC->ForcePortResume=0 (Host should not issue the PortChange interrupt)
    //mwHost20_PORTSC_ForceResume_Clr();

    //<5>.Waiting for the ForcePortResume=0
    //while (mwHost20_PORTSC_ForceResume_Rd()>0);

    //<6>.Enable RunStop Bit      
    //flib_Host20_StopRun_Setting(HOST20_Enable);

    //<7>.Checking the Suspend Status
    //if (mwHost20_PORTSC_ForceSuspend_Rd()>0) {
    //  MP_DEBUG("??? Error ...(After Resume, the Suspend Status should be 0.)\n");
    //  while(1);
    //}
    //else 
    //	  sAttachDevice.bSuspend=0;
          
    //Bruce;;06292005;;Enable Asynchronous   
    //flib_Host20_Asynchronous_Setting(HOST20_Enable);

    return (1);  
}


#if 1 // Make_USB // For WIFI
// To Handle Plug-Out Action immediately
//DWORD gLastDriverEvent = EVENT_DEVICE_PLUG_IN;
static void UsbOtgHostHandlePlugOutAction(DWORD dwEvent, WHICH_OTG eWhichOtg)
{
    
    PST_USB_OTG_HOST psHost;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDev;
    ST_MCARD_MAIL   *pSendMailDrv;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);//gpUsbhDeviceDescriptor;
    
    if(dwEvent & EVENT_DEVICE_PLUG_OUT)
    {
        MP_DEBUG("PlugOut %x  %x", psHost->dwLastDriverEvent, dwEvent);
        
        switch(psHost->dwLastDriverEvent)
        {
            case EVENT_EHCI_ACTIVE_SETUP:

                MP_ALERT("--W-- USBOTG%d %s Plug-Out when SETUP Active", eWhichOtg, __FUNCTION__);
                //SysTimerProcRemove(UsbOtgHostSetupCmdTimeOut);
                //UsbHostFinal(USB_HOST_DEVICE_PLUG_OUT, eWhichOtg);  // UsbOtgHostSetupCmdTimeOut will do this.
                SysTimerProcRemoveById(psUsbOtg->timerId);
                if (pUsbhDev->fpAppClassSetupIoc!= OTGH_NULL)
                    pUsbhDev->fpAppClassSetupIoc(eWhichOtg);
                
                break;

            case EVENT_EHCI_ACTIVE_BULK:

                MP_ALERT("--W-- USBOTG%d %s Plug-Out when BULK Active", eWhichOtg, __FUNCTION__);                
                if (pUsbhDev->fpAppClassBulkIoc!= OTGH_NULL)
                    pUsbhDev->fpAppClassBulkIoc(eWhichOtg);
                
                break;  

            case EVENT_EHCI_ACTIVE_INTERRUPT:

                MP_ALERT("--W-- USBOTG%d %s Plug-Out when INT Active", eWhichOtg, __FUNCTION__);                
                if (pUsbhDev->fpAppClassInterruptIoc!= OTGH_NULL)
                    pUsbhDev->fpAppClassInterruptIoc(eWhichOtg);
                
                break;    

#if (USBOTG_HOST_ISOC == ENABLE)
            case EVENT_EHCI_ACTIVE_ISO_IN:

                MP_ALERT("--W-- USBOTG%d %s Plug-Out when ISO Active", eWhichOtg, __FUNCTION__);                
                UsbOtgHostIsocInIoc(eWhichOtg);              
                break;                  

            case EVENT_EHCI_ACTIVE_ISO_OUT:

                MP_ALERT("--W-- USBOTG%d %s Plug-Out when ISO OUT Active", eWhichOtg, __FUNCTION__);                
                UsbOtgHostIsocOutIoc(eWhichOtg);                
                break;
#endif //#if (USBOTG_HOST_ISOC == ENABLE)                
            default:
                // Nothing
                break;
        }

        #if 0 // UsbOtgHostEventPlugOut( ) will do
        pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_ISR_SENDER);
        pSendMailDrv->wStateMachine             = SETUP_SM;
        pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_STOP_STATE;
        SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv);                
        #endif
    }
    
    psHost->dwLastDriverEvent = dwEvent;
}
#endif


PST_USB_OTG_HOST_ISOC_DES GetBestIsocDes (BYTE bInterfaceClass, BYTE bInterfaceNumber, BYTE bDirection, WHICH_OTG eWhichOtg)
{
#if (USBOTG_HOST_ISOC == DISABLE)
    return NULL;
#else
    PST_USB_OTG_HOST_ISOC_DES pIsocDes = NULL;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci;
    BYTE index;
    BYTE i;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    for (i = 0; i < pUsbhDev->bTotalNumberOfInterfaceWithIsocEp; i++)
    {
        /*
        index = psEhci->pstInterfaceByIsocDes[i].bBestIsocInDesIdx;
        pIsocDes = &psEhci->pstInterfaceByIsocDes[i].pstIsocInDes[index];
        mpDebugPrint("-USBOTG%d- %s:wMaxPacketSize = %d",       eWhichOtg,__FUNCTION__, pIsocDes->wMaxPacketSize);
        mpDebugPrint("-USBOTG%d- %s:wInterval = %d",            eWhichOtg,__FUNCTION__, pIsocDes->wInterval);
        mpDebugPrint("-USBOTG%d- %s:bEdPointNumber = %d",       eWhichOtg,__FUNCTION__, pIsocDes->bEdPointNumber);
        mpDebugPrint("-USBOTG%d- %s:bInterfaceNumber = %d",     eWhichOtg,__FUNCTION__, pIsocDes->bInterfaceNumber);
        mpDebugPrint("-USBOTG%d- %s:bAlternativeSetting = %d",  eWhichOtg,__FUNCTION__, pIsocDes->bAlternativeSetting);
*/
        if ((bInterfaceClass == psEhci->pstInterfaceByIsocDes[i].bInterfaceClass) &&\
            (bInterfaceNumber == psEhci->pstInterfaceByIsocDes[i].bInterfaceNumber))
        {
            if (bDirection == USB_DIR_IN)
            {
                index = psEhci->pstInterfaceByIsocDes[i].bBestIsocInDesIdx;
                if (psEhci->pstInterfaceByIsocDes[i].bTotalNumOfIsocInEp != 0)
                {
                    pIsocDes = &psEhci->pstInterfaceByIsocDes[i].pstIsocInDes[index];
                    //mpDebugPrint("-USBOTG%d- %s:wMaxPacketSize = %d",       eWhichOtg,__FUNCTION__, pIsocDes->wMaxPacketSize);
                    //mpDebugPrint("-USBOTG%d- %s:wInterval = %d",            eWhichOtg,__FUNCTION__, pIsocDes->wInterval);
                    //mpDebugPrint("-USBOTG%d- %s:bEdPointNumber = %d",       eWhichOtg,__FUNCTION__, pIsocDes->bEdPointNumber);
                    //mpDebugPrint("-USBOTG%d- %s:bInterfaceNumber = %d",     eWhichOtg,__FUNCTION__, pIsocDes->bInterfaceNumber);
                    //mpDebugPrint("-USBOTG%d- %s:bAlternativeSetting = %d",  eWhichOtg,__FUNCTION__, pIsocDes->bAlternativeSetting);
                    return &psEhci->pstInterfaceByIsocDes[i].pstIsocInDes[index];
                }
            }
            else if (bDirection == USB_DIR_OUT)
            {
                index = psEhci->pstInterfaceByIsocDes[i].bBestIsocOutDesIdx;
                if (psEhci->pstInterfaceByIsocDes[i].bTotalNumOfIsocOutEp != 0)
                    return &psEhci->pstInterfaceByIsocDes[i].pstIsocOutDes[index];
            }
        }
    }
    //IODelay(200);
    //__asm("break 100");

    return pIsocDes;
#endif
}


#if (USBOTG_HOST_ISOC == ENABLE)
static SDWORD UsbOtgHostIsocWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent)
{
    SDWORD ret;
    if (dwNextEvent)
    {
        *pdwEvent = dwNextEvent;
        ret = OS_STATUS_OK;
    }
    else
    {
        ret = EventWait(USBOTG_HOST_ISOC_EVENT, 0x7fffffff, OS_EVENT_OR, pdwEvent);
    }
    return ret;
}

static void UsbOtgHostIsocTask(void)
{
    DWORD dwEvent, dwNextEvent;
    WHICH_OTG eWhichOtg = GetUsbhPortSupportIsoc();
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    dwNextEvent = 0;
    while (1)
    {
        if (UsbOtgHostIsocWaitEvent(&dwEvent, dwNextEvent) == OS_STATUS_OK)
        {
            if(dwEvent & EVENT_SEND_ISOC_IN_DATA_TO)
            {
                if (pUsbhDevDes->fpSendIsocInDataTo != NULL)
                {
                    pUsbhDevDes->fpSendIsocInDataTo(eWhichOtg);
                }
                else
                {
                    MP_ALERT("%s:fpSendIsocInDataTo is NULL", __FUNCTION__);
                }
            }
        }
    }
}

static void UsbOtgHostGetItdForIsocEd(WHICH_OTG eWhichOtg)
{
    BYTE i,j,k,m,n,x;
    
    PST_USB_OTG_DES psUsbOtg;
    PST_USB_OTG_HOST psHost;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci;

    psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    psHost = &psUsbOtg->sUsbHost;
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = &psHost->sEhci;

    if (pUsbhDev->bDeviceStatus != USB_STATE_CONFIGURED)
    { // for control pipe only at beginning
        MP_DEBUG("%s:not in USB_STATE_CONFIGURED", __FUNCTION__);
        return;
    }
    else
    {
        //__asm("break 100");
        if ((pUsbhDev->bMaxIsoInEpNumber > 0) || (pUsbhDev->bMaxIsoOutEpNumber > 0))
        {
            // for all pipes after getting interface descriptor
            iTD_Structure *pstIsocItdTemp  = OTGH_NULL;
            DWORD dwByteLen = 0;
                        
            //<1>.Disable the Periodic
            //flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);

            dwByteLen = (sizeof(ST_USB_OTG_HOST_INF_W_ISOC_DES)*pUsbhDev->bTotalNumberOfInterfaceWithIsocEp);
            psEhci->pstInterfaceByIsocDes = (PST_USB_OTG_HOST_INF_W_ISOC_DES)psEhci->dwHdTdBufferIndex;
            psEhci->dwHdTdBufferIndex     = (DWORD)((DWORD)psEhci->pstInterfaceByIsocDes+dwByteLen);
            memset((BYTE*)psEhci->pstInterfaceByIsocDes, 0, dwByteLen);
            psEhci->pstInterfaceByIsocDes->pstIsocInDes = NULL;
            psEhci->pstInterfaceByIsocDes->pstIsocOutDes = NULL;

            BYTE bTotalNumOfIsocInEp  = 0;
            BYTE bTotalNumOfIsocOutEp = 0;;
            BYTE bInterfaceNumber     = 0xff;
            BYTE bInterfaceClass      = 0xff;
            
            for ( i = 0, m = 0; i < pUsbhDev->pConfigDescriptor->bNumInterfaces; i++ )
            {
                for (k = 0; k <  pUsbhDev->pConfigDescriptor->pInterface[i].max_altsetting; k++)
                {
                    for ( j = 0; j < pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bNumEndpoints; j++ )
                    {
                        if (OTGH_ED_ISO == (pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bmAttributes & 0x03))
                        {
                            BYTE direction;
                            direction = ((pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bEndpointAddress)\
                            &USB_ENDPOINT_DIR_MASK);
                            if (USB_DIR_OUT == direction)
                            {
                                bTotalNumOfIsocOutEp++;
                            }
                            else if (USB_DIR_IN == direction)
                            {
                                bTotalNumOfIsocInEp++;
                            }

                            if (bInterfaceNumber != pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bInterfaceNumber)
                                bInterfaceNumber = pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bInterfaceNumber;
                            if (bInterfaceClass != pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bInterfaceClass)
                                bInterfaceClass = pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bInterfaceClass;
                        }
                         // if (OTGH_ED_ISO)
                    } // for loop : bNumEndpoints
                } // for loop : max_altsetting

                if (bTotalNumOfIsocInEp > 0)
                {
                    dwByteLen = (sizeof(ST_USB_OTG_HOST_ISOC_DES)*bTotalNumOfIsocInEp);
                    psEhci->pstInterfaceByIsocDes[m].pstIsocInDes = (PST_USB_OTG_HOST_ISOC_DES)psEhci->dwHdTdBufferIndex;
                    psEhci->dwHdTdBufferIndex   = (DWORD)((DWORD)psEhci->pstInterfaceByIsocDes[m].pstIsocInDes+dwByteLen);
                    memset((BYTE*)psEhci->pstInterfaceByIsocDes[m].pstIsocInDes, VAL_INVALID, dwByteLen);
                }
                
                if (bTotalNumOfIsocOutEp > 0 )
                {
                    dwByteLen = (sizeof(ST_USB_OTG_HOST_ISOC_DES)*bTotalNumOfIsocOutEp);
                    psEhci->pstInterfaceByIsocDes[m].pstIsocOutDes = (PST_USB_OTG_HOST_ISOC_DES)psEhci->dwHdTdBufferIndex;
                    psEhci->dwHdTdBufferIndex   = (DWORD)((DWORD)psEhci->pstInterfaceByIsocDes[m].pstIsocOutDes+dwByteLen);
                    memset((BYTE*)psEhci->pstInterfaceByIsocDes[m].pstIsocOutDes, VAL_INVALID, dwByteLen);
                }

                if ((bTotalNumOfIsocInEp + bTotalNumOfIsocOutEp) > 0)
                {
                    psEhci->pstInterfaceByIsocDes[m].bInterfaceNumber = bInterfaceNumber;
                    psEhci->pstInterfaceByIsocDes[m].bInterfaceClass  = bInterfaceClass;
                    if (bTotalNumOfIsocInEp > 0)
                        psEhci->pstInterfaceByIsocDes[m].bTotalNumOfIsocInEp = bTotalNumOfIsocInEp;
                    if (bTotalNumOfIsocOutEp > 0)
                        psEhci->pstInterfaceByIsocDes[m].bTotalNumOfIsocOutEp = bTotalNumOfIsocOutEp;
                    m++;
                    bTotalNumOfIsocInEp  = 0;
                    bTotalNumOfIsocOutEp = 0;
                    bInterfaceNumber     = VAL_INVALID;
                    bInterfaceClass      = VAL_INVALID;
                }
            } // for loop : bNumInterfaces
            
            if (pUsbhDev->bMaxIsoInEpNumber > 0 )
            {
                MP_DEBUG("%s:Total Number of Isoc IN Endpoints is %d", __FUNCTION__, pUsbhDev->bMaxIsoInEpNumber);
                pUsbhDev->psAppClass->bIsocInItdArrayNum= 0;
            }
            
            if (pUsbhDev->bMaxIsoOutEpNumber > 0 )
            {
                MP_DEBUG("%s:Total Number of Isoc OUT Endpoints is %d", __FUNCTION__, pUsbhDev->bMaxIsoOutEpNumber);
                pUsbhDev->psAppClass->bIsocOutItdArrayNum= 0;
            }


            WORD wIsocInMaxPkSz = 0;
            WORD wIsocOutMaxPkSz = 0;
            
            x = 0;
            for ( i = 0, m = 0, n = 0; i < pUsbhDev->pConfigDescriptor->bNumInterfaces; i++ )
            {
                for (k = 0; k <  pUsbhDev->pConfigDescriptor->pInterface[i].max_altsetting; k++)
                {
                    for ( j = 0; j < pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bNumEndpoints; j++ )
                    {
                        if (OTGH_ED_ISO == (pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bmAttributes & 0x03))
                        {
                            WORD temp = 0;
                            BYTE direction;
                            direction = ((pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bEndpointAddress)\
                                &USB_ENDPOINT_DIR_MASK);
                            if (USB_DIR_OUT == direction)
                            {
                                psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].wInterval            =\
                                    pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bInterval;
                                psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].wMaxPacketSize       =\
                                    byte_swap_of_word(pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].wMaxPacketSize);                    
                                psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].bEdPointNumber       =\
                                    (pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bEndpointAddress& 0x0f);
                                psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].bInterfaceNumber     = i;
                                psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].bAlternativeSetting  =\
                                    pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bAlternateSetting;
                                psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].bDirection          = USB_DIR_OUT;

                                temp = wIsocOutMaxPkSz;
                                wIsocOutMaxPkSz = max(wIsocOutMaxPkSz, psEhci->pstInterfaceByIsocDes[x].pstIsocOutDes[m].wMaxPacketSize);
                                if (temp < wIsocOutMaxPkSz)
                                    psEhci->pstInterfaceByIsocDes[x].bBestIsocOutDesIdx = m;
                                m++;
                            }
                            else if (USB_DIR_IN == direction)
                            {
                                psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].wInterval           =\
                                    pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bInterval;
                                psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].wMaxPacketSize      =\
                                    byte_swap_of_word(pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].wMaxPacketSize);                    
                                psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].bEdPointNumber      =\
                                    (pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].pEndpoint[j].bEndpointAddress& 0x0f);
                                psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].bInterfaceNumber    = i;
                                psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].bAlternativeSetting =\
                                    pUsbhDev->pConfigDescriptor->pInterface[i].altsetting[k].bAlternateSetting;
                                psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].bDirection          = USB_DIR_IN;

                                temp = wIsocInMaxPkSz;
                                wIsocInMaxPkSz = max(wIsocInMaxPkSz, psEhci->pstInterfaceByIsocDes[x].pstIsocInDes[n].wMaxPacketSize);
                                if (temp < wIsocInMaxPkSz)
                                    psEhci->pstInterfaceByIsocDes[x].bBestIsocInDesIdx = n;
                                n++;
                            }
                            
                            if ((psEhci->pstInterfaceByIsocDes[x].bTotalNumOfIsocOutEp+\
                                 psEhci->pstInterfaceByIsocDes[x].bTotalNumOfIsocInEp) <= (m+n))
                            {
                                x++;
                                m = 0;
                                n = 0;
                                wIsocInMaxPkSz = 0;
                                wIsocOutMaxPkSz = 0;
                            }
                        } // if (OTGH_ED_ISO)
                    } // for loop : bNumEndpoints
                } // for loop : max_altsetting
            } // for loop : bNumInterfaces
        //test
/*        
                BYTE bIsocInEpNum = 0;
                PST_USB_OTG_HOST_ISOC_DES pIsocInDes;
                BYTE bIsocOutEpNum = 0;
                PST_USB_OTG_HOST_ISOC_DES pIsocOutDes;
        
                bIsocInEpNum = GetBestIsocEndPointNumber    (USB_CLASS_WIRELESS_BT, USB_DIR_IN, eWhichOtg);
                pIsocInDes = GetBestIsocDes                 (USB_CLASS_WIRELESS_BT, USB_DIR_IN, eWhichOtg);
                bIsocOutEpNum = GetBestIsocEndPointNumber   (USB_CLASS_WIRELESS_BT, USB_DIR_OUT, eWhichOtg);
                pIsocOutDes = GetBestIsocDes                (USB_CLASS_WIRELESS_BT, USB_DIR_OUT, eWhichOtg);
        
                bIsocInEpNum    = GetBestIsocEndPointNumber (USB_CLASS_VIDEO, USB_DIR_IN, eWhichOtg);
                pIsocInDes      = GetBestIsocDes            (USB_CLASS_VIDEO, USB_DIR_IN, eWhichOtg);
                bIsocOutEpNum   = GetBestIsocEndPointNumber (USB_CLASS_VIDEO, USB_DIR_OUT, eWhichOtg);
                pIsocOutDes     = GetBestIsocDes            (USB_CLASS_VIDEO, USB_DIR_OUT, eWhichOtg);
                
                bIsocInEpNum    = GetBestIsocEndPointNumber (USB_CLASS_AUDIO, USB_DIR_IN, eWhichOtg);
                pIsocInDes      = GetBestIsocDes            (USB_CLASS_AUDIO, USB_DIR_IN, eWhichOtg);
                bIsocOutEpNum   = GetBestIsocEndPointNumber (USB_CLASS_AUDIO, USB_DIR_OUT, eWhichOtg);
                pIsocOutDes     = GetBestIsocDes            (USB_CLASS_AUDIO, USB_DIR_OUT, eWhichOtg);
*/                
            
            //<4>.Set Periodic Base Address 
            //mwHost20_PeriodicBaseAddr_Set(psEhci->dwHostStructurePflBaseAddress);   
            
            //<5>.Enable the periodic 
            //flib_Host20_Periodic_Setting(HOST20_Enable, eWhichOtg);
        }//if ((pUsbhDev->bMaxIsoInEpNumber > 0) || (pUsbhDev->bMaxIsoOutEpNumber > 0))
    } // if (pUsbhDev->bDeviceStatus != USB_STATE_CONFIGURED)
}

static void UsbOtgHostIsocInActive (WHICH_OTG eWhichOtg)
{
    DWORD dwEndPtNum = 0;
    DWORD dwMaxPacketSize = 0;
    PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if (pUsbhDev->bIsoInEnable == FALSE)
        return;
    
    MP_DEBUG("%s", __FUNCTION__);
    if (pUsbhDev->fpGetIsocInDesForIsoInActive != OTGH_NULL)
    {
        pIsocInEpDes = pUsbhDev->fpGetIsocInDesForIsoInActive(pUsbhDev->bAppInterfaceClass, pUsbhDev->bAppInterfaceNumber, eWhichOtg);
        if (pIsocInEpDes == NULL)
        {
            MP_ALERT("%s:%d:pIsocInEpDes is NULL!", __FUNCTION__, __LINE__);
            return;
        }
    }
    else
    {
        MP_ALERT("%s:%d:pUsbhDev->fpGetIsocInDesForIsoInActive is NULL!!", __FUNCTION__, __LINE__);
        return;
    }
    
    dwEndPtNum      = pIsocInEpDes->bEdPointNumber;
    //dwMaxPacketSize = GetIsocEndPointMaxPacketSize((DWORD)pIsocInEpDes->wMaxPacketSize);
    dwMaxPacketSize = (DWORD)pIsocInEpDes->wMaxPacketSize;

    MP_DEBUG("%s:%d", __FUNCTION__, dwMaxPacketSize);
// for isoc in
    BYTE i = 0;
    for (i = 0; i < ISOC_DATA_NUM_BUF; i++)
    {
        if (pUsbhDev->bIsoInEnable == TRUE)
        {
            if (pUsbhDev->stIsocDataStream.dwIsoActive[i] == FALSE )
            {
                //UartOutText("<SO>");
                OTGH_PT_ISO(dwEndPtNum, OTGH_Dir_IN, pUsbhDev->dwIsocInBufferSize, dwMaxPacketSize, eWhichOtg);
            }
        }
        else
            return;
    }
}



static void UsbOtgHostIsocInIoc (WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    iTD_Structure  *spTempiTD;
    DWORD index = 0;
    DWORD j;

    if (pUsbhDevDes->bIsoInEnable == FALSE)
    {
        return;
    }

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        MP_ALERT("%s:%d:Plug-Out!!", __FUNCTION__, __LINE__);
        return;
    }

    if (pUsbhDevDes->fpIsocInDataProcessForIsoInIoc == OTGH_NULL)
    {
        MP_ALERT("pUsbhDev->fpIsocInDataProcessForIsoInIoc is NULL!!");
        return;
    }
    
    if (pUsbhDevDes->stIsocDataStream.dwIsoActive[pUsbhDevDes->stIsocDataStream.dwiTdInt] == TRUE)
    {    
        DWORD dwFrameNumber;
        dwFrameNumber = pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdInt];
        dwFrameNumber = (dwFrameNumber == 0) ? PERIODIC_FRAME_SIZE : dwFrameNumber;
        index = dwFrameNumber - 1;
        spTempiTD = (iTD_Structure *)pUsbhDevDes->pstPeriodicFrameList[pUsbhDevDes->bIsocInEpPflIndex].dwFrameList[index]; 
        MP_DEBUG("%s:%d:dwOriginalFrameNumber[%d] = %d", __FUNCTION__, __LINE__,
            pUsbhDevDes->stIsocDataStream.dwiTdInt, 
            pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdInt]);
        for (j = 0; j < (pUsbhDevDes->bDeviceSpeed == 0 ? 1 : 8); ++j)
        {
            if (spTempiTD->ArrayStatus_Word[j].bInterruptOnComplete == 0)
            {
                continue;
            }

            if ((spTempiTD->ArrayStatus_Word[j].bStatus & HOST20_iTD_Status_Active) == 0)
            {
                MP_DEBUG("%s:ioc frame number = %d", __FUNCTION__, index);
                SDWORD ret;
                // processing data here
                //mpDebugPrint("%d", __LINE__);
                ret = pUsbhDevDes->fpIsocInDataProcessForIsoInIoc(eWhichOtg);
                //mpDebugPrint("%d", __LINE__);
                pUsbhDevDes->stIsocDataStream.dwIsoActive[pUsbhDevDes->stIsocDataStream.dwiTdInt] = FALSE;
                pUsbhDevDes->stIsocDataStream.dwiTdInt = \
                    (++pUsbhDevDes->stIsocDataStream.dwiTdInt == ISOC_DATA_NUM_BUF) ? 0 : pUsbhDevDes->stIsocDataStream.dwiTdInt;

                
                if (pUsbhDevDes->bIsoInEnable == TRUE)
                {
                    EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_ISO_IN);
                }
                
                //mpDebugPrint("%s:%d:j %d:ret %d", __func__, __LINE__, j, ret);
                if (ret == USBOTG_UNKNOW_ERROR)
                {
                    mpDebugPrint("%s:%d:j %d", __func__, __LINE__, j);
                    //__asm("break 100");
                    return;
                }
            }
        }
    }
}

static void UsbOtgHostIsocOutActive (WHICH_OTG eWhichOtg)
{
    DWORD dwEndPtNum = 0;
    DWORD dwMaxPacketSize = 0;
    PST_USB_OTG_HOST_ISOC_DES pIsocOutEpDes;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PUSBH_ISOC_BUFFER pIsocData;
    BYTE *pData = NULL;
    DWORD dwLength = 0;
    
    //MP_DEBUG("%s: begin", __FUNCTION__);
    pUsbhDev        = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDev->fpGetIsocOutDesForIsoOutActive != OTGH_NULL)
        pIsocOutEpDes = pUsbhDev->fpGetIsocOutDesForIsoOutActive(eWhichOtg);
    else
    {
        MP_ALERT("pUsbhDev->fpGetIsocOutDesForIsoOutActive is NULL!!");
        return;
    }

    dwEndPtNum      = pIsocOutEpDes->bEdPointNumber;
    dwMaxPacketSize = pIsocOutEpDes->wMaxPacketSize;

    if (pUsbhDev->stIsocOutDataStream.dwIsoActive[0] == FALSE )
    {
        pIsocData = UsbOtgHostIsocOutDataDequeueGo(eWhichOtg);
        if (pIsocData == NULL)
        {
            //MP_ALERT("A. %s:Queue Empty", __FUNCTION__);
            return;
        }
        
        pUsbhDev->stIsocOutDataStream.dwIsoActive[0] = TRUE;
        pData = (BYTE*)(psEhci->stOtgHostIsoc.dwDataBufferArray[OTGH_PT_ISO_OUT_DATAPAGE_INDEX]);
        //UartOutText("A");
        dwLength = pIsocData->dwLength;
        memcpy(pData, &pIsocData->pbDataBuffer[0], dwLength);
        UsbOtgHostIsocOutDataDequeueReady(eWhichOtg);
        OTGH_PT_ISO(dwEndPtNum, OTGH_Dir_Out, dwLength, dwMaxPacketSize, eWhichOtg); // for test
        
    }
    
    if (pUsbhDev->stIsocOutDataStream.dwIsoActive[1] == FALSE)
    {
        pIsocData = UsbOtgHostIsocOutDataDequeueGo(eWhichOtg);
        if (pIsocData == NULL)
        {
            //MP_ALERT("B. %s:Queue Empty", __FUNCTION__);
            return;
        }
        
        pUsbhDev->stIsocOutDataStream.dwIsoActive[1] = TRUE;
        pData = (BYTE*)(psEhci->stOtgHostIsoc.dwDataBufferArray[OTGH_PT_ISO_OUT_DATAPAGE_INDEX+1]);
        //UartOutText("B");
        dwLength = pIsocData->dwLength;
        memcpy(pData, &pIsocData->pbDataBuffer[0], dwLength);
        UsbOtgHostIsocOutDataDequeueReady(eWhichOtg);
        OTGH_PT_ISO(dwEndPtNum, OTGH_Dir_Out, dwLength, dwMaxPacketSize, eWhichOtg); // for test
    }
}


static void UsbOtgHostIsocOutIoc (WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    iTD_Structure  *spTempiTD;
    BYTE index = 0;
    DWORD j;

    if (UsbOtgHostConnectStatusGet(eWhichOtg) == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }

    if (pUsbhDevDes->stIsocOutDataStream.dwIsoActive[pUsbhDevDes->stIsocOutDataStream.dwiTdInt] == TRUE)
    {
        index = pUsbhDevDes->stIsocOutDataStream.dwLastFrameNumber[pUsbhDevDes->stIsocOutDataStream.dwiTdInt]-1;
        spTempiTD = (iTD_Structure *)pUsbhDevDes->pstPeriodicFrameList[pUsbhDevDes->bIsocOutEpPflIndex].dwFrameList[index]; 
        for (j = 0; j < (pUsbhDevDes->bDeviceSpeed == 0 ? 1 : 8); ++j)
        {
            if (spTempiTD->ArrayStatus_Word[j].bInterruptOnComplete == 0)
            {
                continue;
            }

            if ((spTempiTD->ArrayStatus_Word[j].bStatus & HOST20_iTD_Status_Active) == 0)
            {
                //MP_DEBUG("%s:ioc frame number = %d", __FUNCTION__, index);
                pUsbhDevDes->stIsocOutDataStream.dwIsoActive[pUsbhDevDes->stIsocOutDataStream.dwiTdInt] = FALSE;
                pUsbhDevDes->stIsocOutDataStream.dwiTdInt = (++pUsbhDevDes->stIsocOutDataStream.dwiTdInt == 2) ? 0 : 1;
                if (pUsbhDevDes->stIsocOutDataStream.dwStreamActive)
                {
                    if (pUsbhDevDes->bIsoOutEnable == TRUE)
                    {
                        //MP_DEBUG("%s:EventSet isoc active", __FUNCTION__);
                        //UartOutText("<O>");                        
                        EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_ISO_OUT);
                    }
                }
            }
        }
    }
}


void UsbOtgHostIsocStreamDataInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);;
    DWORD i;

    MP_DEBUG("%s", __FUNCTION__);
    pUsbhDevDes->stIsocDataStream.dwStreamActive = TRUE;
    pUsbhDevDes->stIsocOutDataStream.dwStreamActive = TRUE;


    pUsbhDevDes->stIsocDataStream.dwBufferCurIndex    = 0;
    pUsbhDevDes->stIsocOutDataStream.dwBufferCurIndex = 0;
    for (i = 0; i < ISOC_DATA_NUM_BUF; ++i)
    {
        pUsbhDevDes->stIsocDataStream.dwBufferActive[i]    = 0;
        pUsbhDevDes->stIsocDataStream.dwBufferLength[i]    = 0;
        pUsbhDevDes->stIsocOutDataStream.dwBufferActive[i] = 0;
        pUsbhDevDes->stIsocOutDataStream.dwBufferLength[i] = 0;
    }

    pUsbhDevDes->stIsocDataStream.dwiTdInt    = 0;
    pUsbhDevDes->stIsocDataStream.dwiTdIdx    = 0;
    pUsbhDevDes->stIsocOutDataStream.dwiTdInt = 0;
    pUsbhDevDes->stIsocOutDataStream.dwiTdIdx = 0;
    for (i = 0; i < ISOC_DATA_NUM_BUF; ++i)
    {
        pUsbhDevDes->stIsocDataStream.dwiTdNum[i]                 = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[i]    = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[i]        = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocDataStream.dwIsoActive[i]              = FALSE;
        pUsbhDevDes->stIsocOutDataStream.dwiTdNum[i]              = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocOutDataStream.dwLastFrameNumber[i]     = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocOutDataStream.dwIsoActive[i]           = FALSE;
    }

    pUsbhDevDes->dwIsocInBufferSize = /*OTGH_PT_ISO_DATABUFFER_NUM*/ OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000; // 1K
    pUsbhDevDes->dwIsocOutBufferSize = /*OTGH_PT_ISO_DATABUFFER_NUM*/ OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000; // 1K

    for (i=0;i<Host20_iTD_MAX;i++)
        psUsbHostEhci->pbHostItdManage[i] = Host20_MEM_FREE;
}


void UsbOtgHostIsocStreamDataStart(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);;
    DWORD i;

    MP_DEBUG("%s", __FUNCTION__);
    pUsbhDevDes->stIsocDataStream.dwStreamActive = TRUE;
    pUsbhDevDes->stIsocOutDataStream.dwStreamActive = TRUE;


    pUsbhDevDes->stIsocDataStream.dwBufferCurIndex    = 0;
    pUsbhDevDes->stIsocOutDataStream.dwBufferCurIndex = 0;
    for (i = 0; i < ISOC_DATA_NUM_BUF; ++i)
    {
        pUsbhDevDes->stIsocDataStream.dwBufferActive[i]    = 0;
        pUsbhDevDes->stIsocDataStream.dwBufferLength[i]    = 0;
        pUsbhDevDes->stIsocOutDataStream.dwBufferActive[i] = 0;
        pUsbhDevDes->stIsocOutDataStream.dwBufferLength[i] = 0;
    }

    pUsbhDevDes->stIsocDataStream.dwiTdInt    = 0;
    pUsbhDevDes->stIsocDataStream.dwiTdIdx    = 0;
    pUsbhDevDes->stIsocOutDataStream.dwiTdInt = 0;
    pUsbhDevDes->stIsocOutDataStream.dwiTdIdx = 0;
    for (i = 0; i < ISOC_DATA_NUM_BUF; ++i)
    {
        pUsbhDevDes->stIsocDataStream.dwiTdNum[i]                 = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[i]    = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[i]        = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocDataStream.dwIsoActive[i]              = FALSE;
        pUsbhDevDes->stIsocOutDataStream.dwiTdNum[i]              = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocOutDataStream.dwLastFrameNumber[i]     = PERIODIC_FRAME_SIZE;
        pUsbhDevDes->stIsocOutDataStream.dwIsoActive[i]           = FALSE;
    }

    pUsbhDevDes->dwIsocInBufferSize = /*OTGH_PT_ISO_DATABUFFER_NUM*/ OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000; // 1K
    pUsbhDevDes->dwIsocOutBufferSize = /*OTGH_PT_ISO_DATABUFFER_NUM*/ OTGH_ISO_VIDEO_DATA_BUFFER_NUM * 0x1000; // 1K

    for (i=0;i<Host20_iTD_MAX;i++)
        psUsbHostEhci->pbHostItdManage[i] = Host20_MEM_FREE;


    for(i = 0; i < Host20_Page_MAX; i++)
    {
        psUsbHostEhci->stOtgHostIsoc.dwDataBufferArray[i] = UsbOtgHostGetDataPage(eWhichOtg);
        if (psUsbHostEhci->stOtgHostIsoc.dwDataBufferArray[i] == 0)
        {
            MP_ALERT("%s:%d:NO Page Buffer", __FUNCTION__, __LINE__);
            break;
        }        
    }
}

void UsbOtgHostIsocEnable(WHICH_OTG eWhichOtg)
{
    SDWORD sts = 0;
    MP_DEBUG("%s", __FUNCTION__);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
#if (USBOTG_WEB_CAM == ENABLE)
    PUSB_HOST_UVC  psUsbhUvc = (PUSB_HOST_UVC)UsbOtgHostUvcDsGet(eWhichOtg);

    psUsbhUvc->eNewFrame               = HAS_NO_FRAME;
    psUsbhUvc->dwFrameNumber           = 0;
    psUsbhUvc->dwOriginalFrameNumber   = 0;
    psUsbhUvc->dwLastFrameNumber       = 0;
    psUsbhUvc->bFrameID                = 0;
    psUsbhUvc->bStartOfFrame           = 0;
    psUsbhUvc->bNewOneFrame            = 0;    
#endif
    // for isoc out
    pUsbhDevDes->bIsoOutEnable = FALSE;

    // for isoc in
    pUsbhDevDes->bIsoInEnable = TRUE;
    UsbOtgHostIsocStreamDataStart(eWhichOtg);
    sts = UsbOtgHostIsocQueueMemoryAllocat(pUsbhDevDes->bIsoInEnable, pUsbhDevDes->bIsoOutEnable, eWhichOtg);
    if (sts != USBOTG_NO_ERROR)
    {
        MP_ALERT("%s: Error occurs in UsbOtgHostIsocQueueMemoryAllocat !!", __FUNCTION__);
        return;
    }
    // mark temporary
    //EventSet(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
    EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_ISO_IN);
}


void UsbOtgHostIsocDisable(WHICH_OTG eWhichOtg)
{
    MP_DEBUG("%s:do TaskYield and check bIsoInEnable in IOC and ACT event and iTD", __FUNCTION__);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);


    EventClear(USBOTG_HOST_ISOC_EVENT, EVENT_SEND_ISOC_IN_DATA_TO);
    EventClear(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_ISO_IN);
    
    UsbOtgHostIsocQueueMemoryFree(pUsbhDevDes->bIsoInEnable, pUsbhDevDes->bIsoOutEnable, eWhichOtg);
    
    // for isoc out
    pUsbhDevDes->bIsoOutEnable = FALSE;

    // for isoc in
    pUsbhDevDes->bIsoInEnable = FALSE;
    //TaskYield();
    UsbOtgHostIsocStreamDataStop(eWhichOtg);

    UsbOtgHostStopAllItd(eWhichOtg);
    
    //EventClear(USBOTG_HOST_ISOC_EVENT, EVENT_EHCI_ACTIVE_ISO_IN);
    //EventClear(UsbOtgHostDriverEventIdGet(eWhichOtg), 0);
    mpDebugPrint("%s:%d:Done", __FUNCTION__, __LINE__);
}


void UsbOtgHostIsocStreamDataStop(WHICH_OTG eWhichOtg)
{
    DWORD i;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);;
    PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);


    MP_DEBUG("%s begin", __FUNCTION__);

    pUsbhDevDes->stIsocDataStream.dwStreamActive      = FALSE;
    pUsbhDevDes->stIsocOutDataStream.dwStreamActive   = FALSE;
    
    // init audio buffer stream
    
    for(i = 0; i < Host20_Page_MAX; i++)
    {
        UsbOtgHostReleaseDataPage(eWhichOtg);
    }

    MP_DEBUG("%s end", __FUNCTION__);
}



static void flib_Host20_Issue_Iso ( DWORD       wEndPt,
                                    DWORD       wMaxPacketSize,
                                    DWORD       wSize,
                                    DWORD       *pwBufferArray,
                                    DWORD       wOffset,
                                    BYTE        bDirection,
                                    BYTE        bMult, 
                                    WHICH_OTG   eWhichOtg)
{
    DWORD wCurrentOffset,wCurrentBufferNum,wCurrentLength,wiTDNum,wFrameNumber, wOriginalFrameNumber;
    iTD_Structure  *spTempiTD;
    DWORD *pwLastTransaction;
    DWORD *wIsoiTDAddress;
    BYTE  bCurrentTransactionNum,bCurrentPageNum,bTransactionNumMax;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    BYTE index = 0;
    SDWORD wRemainSize = 0;

    MP_DEBUG("%s begin wSize = %d", __FUNCTION__, wSize);
    //Critical Time Period Start ---------------------------------------------------        
    //PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes = GetBestIsocDes(USB_CLASS_WIRELESS_BT, USB_DIR_IN, eWhichOtg);;
    //UartOutText("<");
    //UartOutValue(pUsbhDevDes->stIsocDataStream.dwiTdIdx, 1);
    //UartOutText(">");
   // __asm("break 100");
    if (bDirection == OTGH_Dir_IN)
    {
        if (pUsbhDevDes->bIsoInEnable == FALSE)
            return;
        
        index = pUsbhDevDes->bIsocInEpPflIndex;
        wOriginalFrameNumber = (mwHost20_FrameIndex_Rd()>>3) & (PERIODIC_FRAME_SIZE-1);
        /*/////////////////////////////
        while (1)
        {
            wOriginalFrameNumber = (mwHost20_FrameIndex_Rd()>>3) & (PERIODIC_FRAME_SIZE-1); // 1024
            mpDebugPrint("%d", wOriginalFrameNumber);
            //mpDebugPrint("0x%x; 0x%x", psUsbOtg->psUsbReg->HcUsbCommand, wOriginalFrameNumber);
        };
        ////////////////////////////*/
        if ((pUsbhDevDes->stIsocDataStream.dwiTdIdx == 0) &&\
            (pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx] ==\
             PERIODIC_FRAME_SIZE))
        {
            pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[0] = wOriginalFrameNumber + 15;
        }
        else 
        {
            BYTE iTdIdx = pUsbhDevDes->stIsocDataStream.dwiTdIdx;
            if (iTdIdx == 0)
            { // 0
                pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[0] = \
                    pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[ISOC_DATA_NUM_BUF-1];
            }
            else
            { // 1 and 2
                pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[iTdIdx] = \
                    pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[iTdIdx-1];
            }
        }

        if (pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx] >= PERIODIC_FRAME_SIZE)
            pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx] -= PERIODIC_FRAME_SIZE;

        wFrameNumber = pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx];
//        mpDebugPrint("%s:%d:dwOriginalFrameNumber[%d] = %d", __FUNCTION__, __LINE__, 
//                        pUsbhDevDes->stIsocDataStream.dwiTdIdx,
//                        pUsbhDevDes->stIsocDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx]);
        //<2>.Allocate the iTD for the Data Buffer
    }
    else if (bDirection == OTGH_Dir_Out)
    {
        index = pUsbhDevDes->bIsocOutEpPflIndex;
        wOriginalFrameNumber = (mwHost20_FrameIndex_Rd()>>3) & (PERIODIC_FRAME_SIZE-1);
        if ((pUsbhDevDes->stIsocOutDataStream.dwiTdIdx == 0) &&\
            (pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx] ==\
             PERIODIC_FRAME_SIZE))
        {
            pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[0] = wOriginalFrameNumber + 0x10;
        }
        else 
        {
            BYTE iTdIdx = pUsbhDevDes->stIsocOutDataStream.dwiTdIdx;
            if (iTdIdx == 0)
            {
                pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[0] = \
                    pUsbhDevDes->stIsocOutDataStream.dwLastFrameNumber[ISOC_DATA_NUM_BUF-1];
            }
            else
            {
                pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[iTdIdx] = \
                    pUsbhDevDes->stIsocOutDataStream.dwLastFrameNumber[iTdIdx-1];
            }
        }

        if (pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx] >= PERIODIC_FRAME_SIZE)
            pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx] -= PERIODIC_FRAME_SIZE;

        wFrameNumber = pUsbhDevDes->stIsocOutDataStream.dwOriginalFrameNumber[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx];
        MP_DEBUG("%s:%d:wFrameNumber = %d", __FUNCTION__, __LINE__, wFrameNumber);
        //<2>.Allocate the iTD for the Data Buffer
    }
    else
    {
        MP_ALERT("%s:1 wrong direction!", __FUNCTION__);
    }

    
    MP_DEBUG("%d:wSize = %d", __LINE__, wSize);
    wSize = wSize - wSize%(wMaxPacketSize*bMult);
    MP_DEBUG("%d:wSize = %d", __LINE__, wSize);
    pUsbhDevDes->dwIsocInBufferSize = wSize;
    wIsoiTDAddress      = (DWORD*)&pUsbhDevDes->pstPeriodicFrameList[index].dwFrameList[0];
    wRemainSize         = wSize;
    wCurrentBufferNum   = 0;
    wiTDNum             = 0;
    wCurrentOffset      = wOffset;

    while(wRemainSize>0)
    {
        //<2.1>.Allocate iTD
        spTempiTD=(iTD_Structure*)wIsoiTDAddress[wFrameNumber++];
        wFrameNumber = (wFrameNumber == PERIODIC_FRAME_SIZE) ? 0 : wFrameNumber;
        MP_DEBUG("%s:%d:wFrameNumber = %d; spTempiTD = 0x%x", __FUNCTION__, __LINE__, (wFrameNumber-1), spTempiTD);

        spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction    |= (wEndPt<<8);
        spTempiTD->ArrayBufferPointer_Word[0].bMultiFunction    |= GetDeviceAddress(eWhichOtg);
        spTempiTD->ArrayBufferPointer_Word[1].bMultiFunction     = (bDirection<<11);
        spTempiTD->ArrayBufferPointer_Word[1].bMultiFunction    |= wMaxPacketSize;
        spTempiTD->ArrayBufferPointer_Word[2].bMultiFunction     = bMult;

        bCurrentTransactionNum=0;
        bCurrentPageNum=0;
        spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer = (((DWORD)*(pwBufferArray+wCurrentBufferNum))>>12); 
        MP_DEBUG("%s:%d:BP = 0x%x", __FUNCTION__, __LINE__, spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer);
        if (pUsbhDevDes->bDeviceSpeed == 0)
            bTransactionNumMax = 1; // for full Speed
        else if (pUsbhDevDes->bDeviceSpeed == 2)
            bTransactionNumMax = 8; // for High Speed
        else if (pUsbhDevDes->bDeviceSpeed == 1)
            MP_ALERT("It's Low Speed!");
            
        //<2.2>.Fill iTD
        while ((wRemainSize)&&(bCurrentTransactionNum < bTransactionNumMax))
        {
            //Fill iTD
            MP_DEBUG("%s:%d:spTempiTD = 0x%x", __FUNCTION__, __LINE__, spTempiTD);
            MP_DEBUG("%s:%d:bCurrentTransactionNum = %d", __FUNCTION__, __LINE__, bCurrentTransactionNum);
            if (wRemainSize<(wMaxPacketSize*bMult))
                wCurrentLength = wRemainSize;
            else 
                wCurrentLength = wMaxPacketSize*bMult;

            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bLength             = wCurrentLength;
            //spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus             = HOST20_iTD_Status_Active;
            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete= 0;
            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bPageSelect         = bCurrentPageNum;
            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bOffset             = wCurrentOffset;
            MP_DEBUG("%s:L = %d; S = 0x%x; IOC = %d;P = %d; Off = %d",
                            __FUNCTION__,
                            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bLength,
                            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus,
                            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete,
                            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bPageSelect,
                            spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bOffset);
            //Maintain the wRemainSize/bCurrentPageNum/wCurrentOffset/wCurrentBufferNum               
            MP_DEBUG(" ");
            wRemainSize    -= wCurrentLength;
            wCurrentOffset += wCurrentLength;
            if (wCurrentOffset >= 4096)
            {
                bCurrentPageNum++;
                wCurrentBufferNum++;

                spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer
                    =((DWORD)*(pwBufferArray + wCurrentBufferNum)>>12);   
                MP_DEBUG("%s:%d:BP = 0x%x", __FUNCTION__, __LINE__, spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer);

                wCurrentOffset -= 4096;	
                //wCurrentOffset = 0;	
            } 

            MP_DEBUG("%s:%d:wCurrentOffset = %d", __FUNCTION__, __LINE__, wCurrentOffset);
            if (NULL == spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer)
            {
                MP_ALERT("bBufferPointer is NULL!!");
                return;
                //IODelay(200);
                //__asm("break 100");
            }


            // active iso transaction
            //spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus = HOST20_iTD_Status_Active;
            
            //Set the finish Complete-Interrupt
            if (wRemainSize==0)
            {
                MP_DEBUG("%s:set IOC", __FUNCTION__);
                spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bInterruptOnComplete=1;	
                pwLastTransaction=(DWORD*)&(spTempiTD->ArrayStatus_Word[bCurrentTransactionNum]);
                if (spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bLength == 0)
                {
                    MP_ALERT("%s:%d:IOC and L=0!!", __FUNCTION__, __LINE__);
                    return;
                }
            }
            
            // active iso transaction
            if (bDirection == OTGH_Dir_IN)
            {
                if (pUsbhDevDes->bIsoInEnable == TRUE)
                    spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus = HOST20_iTD_Status_Active;
                else
                {
                    spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus = 0;
                    break;
            }
            }
            else if (bDirection == OTGH_Dir_Out)
            {
                if (pUsbhDevDes->bIsoOutEnable == TRUE)
                    spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus = HOST20_iTD_Status_Active;
                else
                    break;
            }
            else
            {
                MP_ALERT("%s:2 wrong direction!", __FUNCTION__);
                break;
            }
            //spTempiTD->ArrayStatus_Word[bCurrentTransactionNum].bStatus = HOST20_iTD_Status_Active;

            bCurrentTransactionNum++;    
            MP_DEBUG("%s:%d:spTempiTD = 0x%x", __FUNCTION__, __LINE__, spTempiTD);
            MP_DEBUG("%s:%d:bCurrentTransactionNum = %d", __FUNCTION__, __LINE__, bCurrentTransactionNum);
            MP_DEBUG("%s:CPN = %d; BP = 0x%x; MF = 0x%x",
                            __FUNCTION__,
                            bCurrentPageNum,
                            spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bBufferPointer,
                            spTempiTD->ArrayBufferPointer_Word[bCurrentPageNum].bMultiFunction);
        };//while ((wRemainSize)&&(bCurrentTransactionNum < bTransactionNumMax))
        //<2.3>.Maintain Variable
        MP_DEBUG("wiTDNum = %d", wiTDNum);
        wiTDNum++;
        if (wiTDNum>512)
        {
            MP_ALERT("%s:%d:>>> Waring...iTD Number >512...", __FUNCTION__, __LINE__); 	
            return;
        }

        MP_DEBUG("wRemainSize = %d; wCurrentOffset = %d; wCurrentBufferNum = %d",
                    wRemainSize,
                    wCurrentOffset,
                    wCurrentBufferNum);
        MP_DEBUG("========================================================");

    }; //while(wRemainSize)

    if (bDirection == OTGH_Dir_IN)
    {
        MP_DEBUG("%s:set isoc in init value", __FUNCTION__);
        pUsbhDevDes->stIsocDataStream.dwiTdNum[pUsbhDevDes->stIsocDataStream.dwiTdIdx]          = wiTDNum;
        pUsbhDevDes->stIsocDataStream.dwIsoActive[pUsbhDevDes->stIsocDataStream.dwiTdIdx]       = TRUE;
        pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx] = wFrameNumber;
        //    wiTDNum, wFrameNumber, pUsbhDevDes->stIsocDataStream.dwiTdIdx, 
        //    pUsbhDevDes->stIsocDataStream.dwLastFrameNumber[pUsbhDevDes->stIsocDataStream.dwiTdIdx]);
        pUsbhDevDes->stIsocDataStream.dwiTdIdx = \
            (++pUsbhDevDes->stIsocDataStream.dwiTdIdx == ISOC_DATA_NUM_BUF) ? 0 : pUsbhDevDes->stIsocDataStream.dwiTdIdx;
        //MP_DEBUG("UsbOtgHostIssueIso: end");
    }
    else if (bDirection == OTGH_Dir_Out)
    {
        //MP_DEBUG("%s:set isoc out init value", __FUNCTION__);
        pUsbhDevDes->stIsocOutDataStream.dwiTdNum[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx]          = wiTDNum;
        //pUsbhDevDes->stIsocOutDataStream.dwIsoActive[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx]       = TRUE;
        pUsbhDevDes->stIsocOutDataStream.dwLastFrameNumber[pUsbhDevDes->stIsocOutDataStream.dwiTdIdx] = wFrameNumber;
        pUsbhDevDes->stIsocOutDataStream.dwiTdIdx = \
            (++pUsbhDevDes->stIsocOutDataStream.dwiTdIdx == ISOC_DATA_NUM_BUF) ? 0 : pUsbhDevDes->stIsocOutDataStream.dwiTdIdx;
        //MP_DEBUG("UsbOtgHostIssueIso: end");
    }
}

static void OTGH_PT_ISO(DWORD wEndPt, BYTE bDirection, DWORD wSize, DWORD wMaxPacketSize, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);;
    DWORD bufferIdx = 0;
    BYTE  bMult,bTemp;//,resvd[2];
    DWORD *pwBufferArray;

    if (pUsbhDevDes->bIsoInEnable == FALSE)
        return;

    if (bDirection == OTGH_Dir_IN)
    {
        bufferIdx = pUsbhDevDes->stIsocDataStream.dwiTdIdx*OTGH_ISO_VIDEO_DATA_BUFFER_NUM;
        //mpDebugPrint("%s:bufferIdx = %d", __FUNCTION__, bufferIdx);
        pwBufferArray = (DWORD*)&(psEhci->stOtgHostIsoc.dwDataBufferArray[bufferIdx]);
    }
    else if (bDirection == OTGH_Dir_Out)
    {
        bufferIdx = pUsbhDevDes->stIsocOutDataStream.dwiTdIdx*OTGH_ISO_AUDIO_DATA_BUFFER_NUM+\
                    OTGH_PT_ISO_OUT_DATAPAGE_INDEX;
        pwBufferArray = (DWORD*)&(psEhci->stOtgHostIsoc.dwDataBufferArray[bufferIdx]);
    }
    
    bTemp = wMaxPacketSize>>11;
    switch(bTemp)
    {
        case 0:
            bMult=1;
        break;
        case 1:
            bMult=2;
        break;
        case 2:
            bMult=3;          
        break;
        default:
            MP_ALERT("??? MaxPacketSize Parse Error...");
        while(1);
        break;
    }

    wMaxPacketSize &= ~(BIT12|BIT11);
    MP_DEBUG("%s:bMult = %d; wMaxPacketSize = %d", __FUNCTION__, bMult, wMaxPacketSize);

    if (pUsbhDevDes->bIsoInEnable == TRUE)
    {
        flib_Host20_Issue_Iso(
                            wEndPt, 
                            wMaxPacketSize, 
                            wSize, 
                            pwBufferArray,
                            0,// offset 
                            bDirection, 
                            bMult,
                            eWhichOtg);
    }
}

DWORD GetIsocEndPointMaxPacketSize(DWORD dwMaxPacketSize)
{
    DWORD maxPacketSize = 1;
    while(dwMaxPacketSize > maxPacketSize) maxPacketSize <<= 1;

    return maxPacketSize;
}


BYTE GetBestIsocEndPointNumber (BYTE bInterfaceClass, BYTE bDirection, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci;
    BYTE bEndPointNumber = VAL_INVALID;
    BYTE index;
    BYTE i;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    for (i = 0; i < pUsbhDev->bTotalNumberOfInterfaceWithIsocEp; i++)
    {
        if (bInterfaceClass == psEhci->pstInterfaceByIsocDes[i].bInterfaceClass)
        {
            if (bDirection == USB_DIR_IN)
            {
                index = psEhci->pstInterfaceByIsocDes[i].bBestIsocInDesIdx;
                if (psEhci->pstInterfaceByIsocDes[i].bTotalNumOfIsocInEp != 0)
                    return psEhci->pstInterfaceByIsocDes[i].pstIsocInDes[index].bEdPointNumber;
            }
            else if (bDirection == USB_DIR_OUT)
            {
                index = psEhci->pstInterfaceByIsocDes[i].bBestIsocOutDesIdx;
                if (psEhci->pstInterfaceByIsocDes[i].bTotalNumOfIsocOutEp != 0)
                    return psEhci->pstInterfaceByIsocDes[i].pstIsocOutDes[index].bEdPointNumber;
            }
        }
    }
            
    return bEndPointNumber;
}
/*
PST_USB_OTG_HOST_ISOC_DES GetBestIsocEpDes (BYTE bInterfaceClass, BYTE bInterfaceNumber, BYTE bDirection, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST_ISOC_DES pIsocDes = NULL;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    PUSB_HOST_EHCI psEhci;
    BYTE index;
    BYTE i;

    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    for (i = 0; i < pUsbhDev->bTotalNumberOfInterfaceWithIsocEp; i++)
    {
        if ((bInterfaceClass == psEhci->pstInterfaceByIsocDes[i].bInterfaceClass) &&\
            (bInterfaceNumber == psEhci->pstInterfaceByIsocDes[i].bInterfaceNumber))
        {
            if (bDirection == USB_DIR_IN)
            {
                index = psEhci->pstInterfaceByIsocDes[i].bBestIsocInDesIdx;
                if (psEhci->pstInterfaceByIsocDes[i].bTotalNumOfIsocInEp != 0)
                {
                    pIsocDes = &psEhci->pstInterfaceByIsocDes[i].pstIsocInDes[index];
                    mpDebugPrint("-USBOTG%d- %s:wMaxPacketSize = %d",       eWhichOtg,__FUNCTION__, pIsocDes->wMaxPacketSize);
                    mpDebugPrint("-USBOTG%d- %s:wInterval = %d",            eWhichOtg,__FUNCTION__, pIsocDes->wInterval);
                    mpDebugPrint("-USBOTG%d- %s:bEdPointNumber = %d",       eWhichOtg,__FUNCTION__, pIsocDes->bEdPointNumber);
                    mpDebugPrint("-USBOTG%d- %s:bInterfaceNumber = %d",     eWhichOtg,__FUNCTION__, pIsocDes->bInterfaceNumber);
                    mpDebugPrint("-USBOTG%d- %s:bAlternativeSetting = %d",  eWhichOtg,__FUNCTION__, pIsocDes->bAlternativeSetting);
                    return &psEhci->pstInterfaceByIsocDes[i].pstIsocInDes[index];
                }
            }
            else if (bDirection == USB_DIR_OUT)
            {
                index = psEhci->pstInterfaceByIsocDes[i].bBestIsocOutDesIdx;
                if (psEhci->pstInterfaceByIsocDes[i].bTotalNumOfIsocOutEp != 0)
                    return &psEhci->pstInterfaceByIsocDes[i].pstIsocOutDes[index];
            }
        }
    }

    return pIsocDes;
}
*.
/*
PST_USB_OTG_HOST_ISOC_DES GetIsocEpDes (BYTE bInterfaceClass, BYTE bDirection, BYTE bEpIndex, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST_ISOC_DES pIsocDes = NULL;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

    if (bDirection == USB_DIR_IN)
    {
        return &psEhci->pstInterfaceByIsocDes[0].pstIsocInDes[bEpIndex-1];
    }
    else if (bDirection == USB_DIR_OUT)
    {
        return &psEhci->pstInterfaceByIsocDes[0].pstIsocOutDes[bEpIndex-1];
    }

    return pIsocDes;
}
*/

void UsbOtgHostPrepareDataPage(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE i = 0;

    for(i = 0; i < Host20_Page_MAX; i++)
    {
        pUsbhDev->dwPageBuffer[i] = flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER, eWhichOtg);          
    }

    UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)pUsbhDev->dwPageBuffer[0],
                                        (DWORD)pUsbhDev->dwPageBuffer[0] + Host20_Page_MAX * Host20_Page_SIZE - 1,
                                        eWhichOtg);
    for (i=0;i<Host20_Page_MAX;i++)
        psUsbHostEhci->pbHostDataPageManage[i] = Host20_MEM_FREE;

}

void UsbOtgHostReleaseAllDataPage(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE i = 0;

    if (pUsbhDev->dwPageIndex > Host20_Page_MAX)
    {
        return;
    }
    
    for(i = 0; i < Host20_Page_MAX; i++)
    {
        flib_Host20_ReleaseStructure(Host20_MEM_TYPE_4K_BUFFER, pUsbhDev->dwPageBuffer[i], eWhichOtg);          
        pUsbhDev->dwPageBuffer[i] = 0;
    }    
    
    UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
    for (i=0;i<Host20_Page_MAX;i++)
        psUsbHostEhci->pbHostDataPageManage[i] = Host20_MEM_FREE;
}

DWORD UsbOtgHostGetDataPage(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    DWORD dwPageBuffer = 0;

    if (pUsbhDev->dwPageIndex > Host20_Page_MAX)
    {
        MP_ALERT("%s:NO Page", __FUNCTION__);
        return 0;
    }

    dwPageBuffer = pUsbhDev->dwPageBuffer[pUsbhDev->dwPageIndex];
    MP_DEBUG("%s:pUsbhDev->dwPageBuffer[%d] = 0x%x", __FUNCTION__, pUsbhDev->dwPageIndex, dwPageBuffer);
    pUsbhDev->dwPageIndex++;
    return dwPageBuffer;
}

void UsbOtgHostReleaseDataPage(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->dwPageIndex--;
}

void SetQueueElementByteCount(DWORD dwLength, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->dwQueueElementByteCount = dwLength;
    MP_DEBUG("%s:%d:0x%x", __FUNCTION__, __LINE__, pUsbhDev->dwQueueElementByteCount);
}

DWORD GetQueueElementByteCount(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return pUsbhDev->dwQueueElementByteCount;
}


SDWORD UsbOtgHostIsocQueueMemoryAllocat(BYTE bIsoInEnable, BYTE bIsoOutEnable, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    DWORD dwQueueElementByteCount = 0;
    DWORD dwMemTmp = 0;
    BYTE i = 0;

    dwQueueElementByteCount = GetQueueElementByteCount(eWhichOtg);
    if (dwQueueElementByteCount == 0)
    {
        MP_ALERT("%s:%d:Byte count of a element in the queue is zero!!", __FUNCTION__, __LINE__);
        return USBOTG_NOT_INIT_YET;
    }
    
    MP_DEBUG("%s:%d:dwQueueElementByteCount = 0x%x", __FUNCTION__, __LINE__, dwQueueElementByteCount);
// for isoc out
    if (bIsoOutEnable == TRUE)
    {
        for (i = 0; i < NUMBER_OF_ISOC_Q_ELEMENTS; i++)
        {
            //dwMemTmp = (DWORD)ker_mem_malloc(dwQueueElementByteCount, UsbOtgHostDriverTaskIdGet(eWhichOtg));
            dwMemTmp = (DWORD)ext_mem_malloc(dwQueueElementByteCount, UsbOtgHostDriverTaskIdGet(eWhichOtg));
            if (dwMemTmp == 0)
            {
                MP_ALERT("-E-:-USBOTG%d-:%s:%d:Host not enough memory 1:%d", eWhichOtg, __FUNCTION__, __LINE__, ext_mem_get_free_space());
                mpDebugPrint("%d", ext_mem_get_free_space());
                return USBOTG_NO_MEMORY;
            }
            else
            {
                pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer = (BYTE*) (dwMemTmp|0xA0000000);
            }
            
            memset(&pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer[0], 0, dwQueueElementByteCount);
            pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].dwLength = 0;
        }
        
        pUsbhDev->stUsbhIsocOutBufferQueue.wHead = 0;
        pUsbhDev->stUsbhIsocOutBufferQueue.wTail = 0;
    }
// for isoc in
    if (bIsoInEnable == TRUE)
    {
#if ISOC_QUEUE_DYNAMIC
		IsocWholeQueueBufferAlloc();
	pUsbhDev->stUsbhIsocInBufferQueue.wHead=GetIsocWholeQueueBufferStart();
	pUsbhDev->stUsbhIsocInBufferQueue.wTail=GetIsocWholeQueueBufferStart();
#else
        for (i = 0; i < NUMBER_OF_ISOC_Q_ELEMENTS; i++)
        {
            //dwMemTmp = (DWORD)ker_mem_malloc(dwQueueElementByteCount, UsbOtgHostDriverTaskIdGet(eWhichOtg));
            dwMemTmp = (DWORD)ext_mem_malloc(dwQueueElementByteCount, UsbOtgHostDriverTaskIdGet(eWhichOtg));
            if (dwMemTmp == 0)
            {
                MP_ALERT("-E-:-USBOTG%d-:%s:%d:Host not enough memory 1", eWhichOtg, __FUNCTION__, __LINE__);
                mpDebugPrint("%d", ker_mem_get_free_space());
                mpDebugPrint("%d", ext_mem_get_free_space());
                
                return USBOTG_NO_MEMORY;
            }
            else
            {
                pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer = (BYTE*) (dwMemTmp|0xA0000000);
                MP_DEBUG("%s:%d:alloc ok  pbDataBuffer = 0x%x", __FUNCTION__, __LINE__,\
                    pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer);
            }
            
            memset(&pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer[0], 0, dwQueueElementByteCount);
            pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwLength = 0;
        }

        pUsbhDev->stUsbhIsocInBufferQueue.wHead = 0;
        pUsbhDev->stUsbhIsocInBufferQueue.wTail = 0;
#endif
    }

    return USBOTG_NO_ERROR;
}

SDWORD UsbOtgHostIsocQueueMemoryFree(BYTE bIsoInEnable, BYTE bIsoOutEnable, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE i = 0;
// for isoc out
    if (bIsoOutEnable == TRUE)
    {
        for (i = 0; i < NUMBER_OF_ISOC_Q_ELEMENTS; i++)
        {
            if (pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer != NULL)
            {
                //ker_mem_free(pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer);
                ext_mem_free(pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer);
                pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer = NULL;
                pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].dwLength = 0;
            }
        }
        
        pUsbhDev->stUsbhIsocOutBufferQueue.wHead = 0;
        pUsbhDev->stUsbhIsocOutBufferQueue.wTail = 0;
    }

// for isoc in
    if (bIsoInEnable == TRUE)
    {
#if ISOC_QUEUE_DYNAMIC
		IsocWholeQueueBufferRelease();
#endif
        for (i = 0; i < NUMBER_OF_ISOC_Q_ELEMENTS; i++)
        {
            if (pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer != NULL)
            {
                //ker_mem_free(pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer);
                ext_mem_free(pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer);
                MP_ALERT("%s:%d:free pbDataBuffer = 0x%x", __FUNCTION__, __LINE__,\
                    pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer);
                pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer    = NULL;
                pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwLength        = 0;
                pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwOriginaliTd   = 0;
                pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwLastiTd       = 0;
            }
        }

        pUsbhDev->stUsbhIsocInBufferQueue.wHead = 0;
        pUsbhDev->stUsbhIsocInBufferQueue.wTail = 0;
        MP_ALERT("%s:%d", __FUNCTION__, __LINE__);
    }
}



void UsbOtgHostIsocQueueInit(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE count = 0;
    BYTE i = 0;
// for isoc out
    if (pUsbhDev->bIsoOutEnable == TRUE)
    {
        for (i = 0; i < NUMBER_OF_ISOC_Q_ELEMENTS; i++)
        {
            pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].pbDataBuffer = NULL;
            pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[i].dwLength = 0;
        }
        
        pUsbhDev->stUsbhIsocOutBufferQueue.wHead = 0;
        pUsbhDev->stUsbhIsocOutBufferQueue.wTail = 0;
    }

// for isoc in
    if (pUsbhDev->bIsoInEnable == TRUE)
    {
#if ISOC_QUEUE_DYNAMIC
		IsocWholeQueueBufferReset();
#endif
        for (i = 0; i < NUMBER_OF_ISOC_Q_ELEMENTS; i++)
        {
            pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].pbDataBuffer    = NULL;
            pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwLength        = 0;
            pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwOriginaliTd   = 0;
            pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[i].dwLastiTd       = 0;
        }

        pUsbhDev->stUsbhIsocInBufferQueue.wHead = 0;
        pUsbhDev->stUsbhIsocInBufferQueue.wTail = 0;
    }
}


PUSBH_ISOC_BUFFER UsbOtgHostIsocOutDataDequeueGo(WHICH_OTG eWhichOtg)
{
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;
    WORD head = UsbOtgHostIsocOutGetHead(eWhichOtg);
    WORD tail = UsbOtgHostIsocOutGetTail(eWhichOtg);

    if (head == tail)
    {
        ;//MP_ALERT("OUT:QE:h = t = %d", head);
        //UartOutText("<E>");
    }
    else
    {
        //mpDebugPrint("OUT:Dq(%d)", head);
        pIsocBuff = UsbOtgHostIsocOutGetQueueElement(head, eWhichOtg);
    }

    return pIsocBuff;
}

void UsbOtgHostIsocOutDataDequeueReady(WHICH_OTG eWhichOtg)
{
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;
    WORD head = UsbOtgHostIsocOutGetHead(eWhichOtg);
    WORD tail = UsbOtgHostIsocOutGetTail(eWhichOtg);

    if (head == (NUMBER_OF_ISOC_Q_ELEMENTS-1))
    {
        head = 0;
    }
    else
    {
        head = head+1;
    }
    
    UsbOtgHostIsocOutSetHead(head, eWhichOtg);
}

SDWORD UsbOtgHostIsocOutDataEnqueueGo(BYTE **hData, DWORD dwLength, WHICH_OTG eWhichOtg)
{
    WORD head = 0;
    WORD tail = 0;
    WORD queue_len = 0;

    head = UsbOtgHostIsocOutGetHead(eWhichOtg);
    tail = UsbOtgHostIsocOutGetTail(eWhichOtg);

    if (tail >= head)
    {
        queue_len = tail - head;
    }
    else
    {
        queue_len = (tail + NUMBER_OF_ISOC_Q_ELEMENTS) - head;
    }
    
    if ((head == (tail+1)) || (queue_len == (NUMBER_OF_ISOC_Q_ELEMENTS-1)))
    {
        MP_ALERT("OUT:QF:h = %d, t = %d", head, tail);
        //UartOutText("<F>");
        return USBOTG_ISOC_QUEUE_FULL;
    }
    else
    {
        PUSBH_ISOC_BUFFER pIsocElement = NULL;
        //mpDebugPrint("OUT:Eq(%d)", tail);
        pIsocElement = UsbOtgHostIsocOutGetQueueElement(tail, eWhichOtg);
        *hData = pIsocElement->pbDataBuffer;
        pIsocElement->dwLength = dwLength;
    }

    return USBOTG_NO_ERROR;
}



void UsbOtgHostIsocOutDataEnqueueReady(WHICH_OTG eWhichOtg)
{
    WORD head = 0;
    WORD tail = 0;

    head = UsbOtgHostIsocOutGetHead(eWhichOtg);
    tail = UsbOtgHostIsocOutGetTail(eWhichOtg);
    if (tail == (NUMBER_OF_ISOC_Q_ELEMENTS-1))
    {
        if (head != 0)
            tail = 0;
    }
    else
    {
        if (head != (tail + 1))
            tail = tail + 1;
    }
    
    UsbOtgHostIsocOutSetTail(tail, eWhichOtg);
}

#if ISOC_QUEUE_DYNAMIC


#else
PUSBH_ISOC_BUFFER UsbOtgHostIsocInDataDequeueGo(WHICH_OTG eWhichOtg)
{
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;
    WORD head = UsbOtgHostIsocInGetHead(eWhichOtg);
    WORD tail = UsbOtgHostIsocInGetTail(eWhichOtg);

    if (head == tail)
    {
        ;//MP_ALERT("IN:QE:h = t = %d", head);
        //UartOutText("<E>");
    }
    else
    {
        //mpDebugPrint("IN:Dq(%d)", head);
        //UartOutText(" De");
        //UartOutValue(head, 1);
        pIsocBuff = (PUSBH_ISOC_BUFFER)UsbOtgHostIsocInGetQueueElement(head, eWhichOtg);
    }
    
    return pIsocBuff;
}

void UsbOtgHostIsocInDataDequeueReady(WHICH_OTG eWhichOtg)
{
    PUSBH_ISOC_BUFFER pIsocBuff = NULL;
    WORD head = UsbOtgHostIsocInGetHead(eWhichOtg);
    WORD tail = UsbOtgHostIsocInGetTail(eWhichOtg);

    if (head == (NUMBER_OF_ISOC_Q_ELEMENTS-1))
    {
        head = 0;
    }
    else
    {
        head = head+1;
    }
    
    UsbOtgHostIsocInSetHead(head, eWhichOtg);
}


SDWORD UsbOtgHostIsocInDataEnqueueDataBuffer(BYTE **hData, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg)
{
    WORD head = 0;
    WORD tail = 0;
    WORD queue_len = 0;

    head = UsbOtgHostIsocInGetHead(eWhichOtg);
    tail = UsbOtgHostIsocInGetTail(eWhichOtg);

    if (tail >= head)
    {
        queue_len = tail - head;
    }
    else
    {
        queue_len = (tail + NUMBER_OF_ISOC_Q_ELEMENTS) - head;
    }
    
    if ((head == (tail+1)) || (queue_len == (NUMBER_OF_ISOC_Q_ELEMENTS-1)))
    {
        //MP_ALERT("IN:QF:h = %d, t = %d", head, tail);
        //UartOutText("<F>");
        return USBOTG_ISOC_QUEUE_FULL;
    }
    else
    {
        PUSBH_ISOC_BUFFER pIsocElement = NULL;
        //mpDebugPrint("IN:Eq(%d)", tail);
        //UartOutText(" En");
        //UartOutValue(tail, 1);
        pIsocElement = UsbOtgHostIsocInGetQueueElement(tail, eWhichOtg);
        *hData = pIsocElement->pbDataBuffer;
        pIsocElement->dwOriginalFrameNumber = dwFrameNumber;
        pIsocElement->dwLastFrameNumber = PERIODIC_FRAME_SIZE;
        pIsocElement->dwOriginaliTd = dwItd;
        if (*hData == NULL)
        {
            MP_ALERT("IN:buffer NULL!! The memory in queue maybe free by device plug-out.");
            return USBOTG_UNKNOW_ERROR;
        }
        //UsbOtgHostIsocInSetQueueElement(pData, dwLength, tail, eWhichOtg);
    }

    return USBOTG_NO_ERROR;
}

void UsbOtgHostIsocInDataEnqueueDataLength(DWORD dwDataLength, DWORD dwFrameNumber, DWORD dwItd, WHICH_OTG eWhichOtg)
{
    WORD head = 0;
    WORD tail = 0;
    PUSBH_ISOC_BUFFER pIsocElement = NULL;


    head = UsbOtgHostIsocInGetHead(eWhichOtg);
    tail = UsbOtgHostIsocInGetTail(eWhichOtg);
    pIsocElement = UsbOtgHostIsocInGetQueueElement(tail, eWhichOtg);
    pIsocElement->dwLength = dwDataLength;
    pIsocElement->dwLastFrameNumber = dwFrameNumber;
    pIsocElement->dwLastiTd = dwItd;
/*///////////////////////////////////////////
    if ((pIsocElement->dwLastFrameNumber < 980)&(pIsocElement->dwOriginalFrameNumber < 980))
    {
        if (pIsocElement->dwOriginalFrameNumber > pIsocElement->dwLastFrameNumber)
        {
            mpDebugPrint("Non-sense!!");
            IODelay(200);
            __asm("break 100");
        }
    }
///////////////////////////////////////////*/    
    if (tail == (NUMBER_OF_ISOC_Q_ELEMENTS-1))
    {
        if (head != 0)
            tail = 0;
    }
    else
    {
        if (head != (tail + 1))
            tail = tail + 1;
    }
    
    UsbOtgHostIsocInSetTail(tail, eWhichOtg);
}


static void UsbOtgHostIsocInSetHead(WORD wHead, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->stUsbhIsocInBufferQueue.wHead = wHead;
}

static void UsbOtgHostIsocInSetTail(WORD wTail, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->stUsbhIsocInBufferQueue.wTail = wTail;
}

static WORD UsbOtgHostIsocInGetHead(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return pUsbhDev->stUsbhIsocInBufferQueue.wHead;
}

static WORD UsbOtgHostIsocInGetTail(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return pUsbhDev->stUsbhIsocInBufferQueue.wTail;
}

static PUSBH_ISOC_BUFFER UsbOtgHostIsocInGetQueueElement(WORD index, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return &pUsbhDev->stUsbhIsocInBufferQueue.stIsocBuff[index];
}
/*
static BYTE* UsbOtgHostIsocInSetQueueElement(BYTE *pData, DWORD dwLength, BYTE tail, WHICH_OTG eWhichOtg)
{
    PUSBH_ISOC_BUFFER pIsocElement = NULL;

    if (dwLength > NUMBER_OF_BYTE_COUNT_OF_ELEMENT)
    {
        MP_ALERT("%s:buffer size > %d", __FUNCTION__, NUMBER_OF_BYTE_COUNT_OF_ELEMENT);
        return;
    }
    
    pIsocElement           = UsbOtgHostIsocInGetQueueElement(tail, eWhichOtg);
    pIsocElement->dwLength = dwLength;
    return &pIsocElement->pbDataBuffer[0];
    //memcpy(&pIsocElement->pbDataBuffer[0], pData, pIsocElement->dwLength);
}
*/
#endif

static void UsbOtgHostIsocOutSetHead(WORD wHead, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->stUsbhIsocOutBufferQueue.wHead = wHead;
}

static void UsbOtgHostIsocOutSetTail(WORD wTail, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pUsbhDev->stUsbhIsocOutBufferQueue.wTail = wTail;
}

static WORD UsbOtgHostIsocOutGetHead(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return pUsbhDev->stUsbhIsocOutBufferQueue.wHead;
}

static WORD UsbOtgHostIsocOutGetTail(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return pUsbhDev->stUsbhIsocOutBufferQueue.wTail;
}

static PUSBH_ISOC_BUFFER UsbOtgHostIsocOutGetQueueElement(WORD index, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    return &pUsbhDev->stUsbhIsocOutBufferQueue.stIsocBuff[index];
}
/*
static void UsbOtgHostIsocOutSetQueueElement(BYTE *pData, DWORD dwLength, BYTE tail, WHICH_OTG eWhichOtg)
{
    PUSBH_ISOC_BUFFER pIsocElement = NULL;

    if (dwLength > NUMBER_OF_BYTE_COUNT_OF_ELEMENT)
    {
        MP_ALERT("%s:buffer size > %d", __FUNCTION__, NUMBER_OF_BYTE_COUNT_OF_ELEMENT);
        return;
    }
    
    pIsocElement           = UsbOtgHostIsocOutGetQueueElement(tail, eWhichOtg);
    pIsocElement->dwLength = dwLength;
    memcpy(&pIsocElement->pbDataBuffer[0], pData, pIsocElement->dwLength);
}
*/
#if 0
//====================================================================
// * Function Name: flib_Host20_Isoc_Init                          
// * Description: 
//   <1>.Init FrameList
//   <2>.Enable Periotic schedule
//
// * Input: 
// * OutPut: 
//====================================================================
void  flib_Host20_Isoc_Init (WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_HOST_EHCI psUsbHostEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    DWORD i;
    //BYTE bNumOfIsocFrameList = 1;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDev;
    
    pUsbhDev = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

// should be set before calling flib_Host20_Isoc_Init
//    pUsbhDev->bNumOfIsocFrameList = 1;

    
//    flib_Host20_Periodic_Setting(HOST20_Disable, eWhichOtg);
    
    pUsbhDev->stIsocDataStream.dwBufferCurIndex    = 0;
    pUsbhDev->stIsocOutDataStream.dwBufferCurIndex = 0;
    for (i = 0; i < ISOC_DATA_NUM_BUF; ++i)
    {
        pUsbhDev->stIsocDataStream.dwBufferActive[i]    = 0;
        pUsbhDev->stIsocDataStream.dwBufferLength[i]    = 0;
        pUsbhDev->stIsocOutDataStream.dwBufferActive[i] = 0;
        pUsbhDev->stIsocOutDataStream.dwBufferLength[i] = 0;
    }
    
    pUsbhDev->stIsocDataStream.dwiTdInt    = 0;
    pUsbhDev->stIsocDataStream.dwiTdIdx    = 0;
    pUsbhDev->stIsocOutDataStream.dwiTdInt = 0;
    pUsbhDev->stIsocOutDataStream.dwiTdIdx = 0;
    for (i = 0; i < 2; ++i)
    {
        pUsbhDev->stIsocDataStream.dwiTdNum[i]                 = PERIODIC_FRAME_SIZE;
        pUsbhDev->stIsocDataStream.dwOriginalFrameNumber[i]    = PERIODIC_FRAME_SIZE;
        pUsbhDev->stIsocDataStream.dwLastFrameNumber[i]        = PERIODIC_FRAME_SIZE;
        pUsbhDev->stIsocDataStream.dwIsoActive[i]            = FALSE;
        pUsbhDev->stIsocOutDataStream.dwiTdNum[i]              = PERIODIC_FRAME_SIZE;
        pUsbhDev->stIsocOutDataStream.dwOriginalFrameNumber[i] = PERIODIC_FRAME_SIZE;
        pUsbhDev->stIsocOutDataStream.dwLastFrameNumber[i]     = PERIODIC_FRAME_SIZE;
        pUsbhDev->stIsocOutDataStream.dwIsoActive[i]         = FALSE;
    }

    pUsbhDev->dwIsocInBufferSize = /*OTGH_PT_ISO_DATABUFFER_NUM*/ OTGH_ISO_AUDIO_DATA_BUFFER_NUM * 0x1000; // 1K
    pUsbhDev->stIsocDataStream.pbAudioBuffer[0] = ext_mem_malloc(ISOC_DATA_NUM_BUF*pUsbhDev->dwIsocInBufferSize);
    if (pUsbhDev->stIsocDataStream.pbAudioBuffer[0]  == 0)
    {
        MP_ALERT("%s:pUsbhDev->stIsocDataStream.pbAudioBuffer[0] alloc fail!!");
        while(1);
    }
    else
    {
        for (i = 1; i < ISOC_DATA_NUM_BUF; ++i)
        {
            pUsbhDev->stIsocDataStream.pbAudioBuffer[i] = pUsbhDev->stIsocDataStream.pbAudioBuffer[i-1] + pUsbhDev->dwIsocInBufferSize;
        }
    }


    pUsbhDev->dwIsocOutBufferSize = /*OTGH_PT_ISO_DATABUFFER_NUM*/ OTGH_ISO_AUDIO_DATA_BUFFER_NUM * 0x1000; // 1K
    pUsbhDev->stIsocOutDataStream.pbAudioBuffer[0] = ext_mem_malloc(ISOC_DATA_NUM_BUF*pUsbhDev->dwIsocOutBufferSize);
    if (pUsbhDev->stIsocOutDataStream.pbAudioBuffer[0]  == 0)
    {
        MP_ALERT("%s:pUsbhDev->stIsocOutDataStream.pbAudioBuffer[0] alloc fail!!");
        while(1);
    }
    else
    {
        for (i = 1; i < ISOC_DATA_NUM_BUF; ++i)
        {
            pUsbhDev->stIsocOutDataStream.pbAudioBuffer[i] = pUsbhDev->stIsocOutDataStream.pbAudioBuffer[i-1] + pUsbhDev->dwIsocOutBufferSize;
        }
    }

    for (i=0;i<Host20_iTD_MAX;i++)
        psUsbHostEhci->pbHostItdManage[i] = Host20_MEM_FREE;


    for (i=0;i<Host20_Page_MAX;i++)
        psUsbHostEhci->pbHostDataPageManage[i] = Host20_MEM_FREE;

    for(i = 0; i < (OTGH_PT_ISO_DATABUFFER_NUM+OTGH_PT_ISO_OUT_DATABUFFER_NUM); i++)
    {
        psUsbHostEhci->stOtgHostIsoc.dwDataBufferArray[i] = UsbOtgHostGetDataPage(eWhichOtg);
        if (psUsbHostEhci->stOtgHostIsoc.dwDataBufferArray[i] == 0)
        {
            MP_ALERT("%s:%d:NO Page Buffer", __FUNCTION__, __LINE__);
            break;
        }        
    }
}
#endif // #if 0
#endif // (USBOTG_HOST_ISOC == ENABLE)


#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
void TestModeForHC(WHICH_OTG eWhichOtg)
{
	BYTE i;
	BYTE u8Tmp[52];
	BYTE *pp;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

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
    bOTGCxFxWrRd(FOTG200_DMA2CxFIFO,DIRECTION_IN,u8Tmp,52, eWhichOtg);

	//////////////////////////////////////////////
	// Jay ask to modify, 91-6-5 (End)			//
	//////////////////////////////////////////////

	// Turn on "r_test_packet_done" bit(flag) (Bit 5)
	mUsbOtgTsPkDoneSet();
}
#endif // USBOTG_HOST_USBIF

// For BT's Usage
void UsbHostReset(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    UsbOtgHostInactiveAllqTD(eWhichOtg);
    UsbHostFinal(USB_HOST_DEVICE_RESET, eWhichOtg); 
    DisableUsbOtgHostInterrupt(eWhichOtg);
    if (psHost->boIsHostToResetDevice == TRUE)
    {
        ST_MCARD_MAIL   *pSendMailDrv;
        
        psHost->boIsHostToResetDevice = FALSE;
        //InitiaizeUsbOtgHost(NOT_FIRST_TIME_INIT);
        pUsbhDevDes->bDeviceStatus = USB_STATE_ATTACHED;

        pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_CLASS_TASK_SENDER, eWhichOtg);
        pSendMailDrv->wStateMachine             = SETUP_SM;
        pSendMailDrv->wCurrentExecutionState    = SETUP_INIT_STOP_STATE;
        SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
    }
    else if (psHost->boIsHostFinalized == TRUE)
    {
        InitiaizeUsbOtgHost(IS_FIRST_TIME_INIT, eWhichOtg);
    }
}
MPX_KMODAPI_SET(UsbHostReset);
#endif //SC_USBHOST


