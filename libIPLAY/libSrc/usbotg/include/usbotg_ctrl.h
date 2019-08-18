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
* Filename		: usbotg_ctrl.h
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)
* Created Date	: 2008/05/19 
* Description:  1.EHCI Data Structure
*               2.EHCI Register
*               3.Others
******************************************************************************** 
*/
#ifndef __USBOTG_CTRL_H__
#define __USBOTG_CTRL_H__ 
#include "utiltypedef.h"
#include "UtilRegFile.h"
#include "usbotg_api.h"
#include "usbotg_std.h"
#include "usbotg_host.h"
#include "usbotg_device.h"
#include "usbotg_device_msdc.h"
#include "usbotg_host_msdc.h"
#include "usbotg_host_sidc.h"
#include "..\libsrc\image\include\jpeg.h"
#include "ptpdps.h"


#if (SC_USBHOST|SC_USBDEVICE)

//=================== 1.Condition Definition  ============================================================
//========================================================================================================
#define   OTGC_Device_Not_Support_Then_Return             1

//=================== 2.Define Macro =====================================================================
//========================================================================================================
//#define OTG_BASE_ADDRESS	                      0x92000000//0x92500000
//#define mbFOTGPort(bOffset)	                       *((volatile BYTE *) ( OTG_BASE_ADDRESS | bOffset))
//#define mwFOTGPort(bOffset)	                       *((volatile UINT16 *) ( OTG_BASE_ADDRESS | bOffset))
//#define mdwFOTGPort(bOffset)	                   *((volatile DWORD *) ( OTG_BASE_ADDRESS | bOffset))

//#define mwHost20Bit_Rd(dwRegVal,wBitNum)                     (dwRegVal&wBitNum)
//#define mwHost20Bit_Set(dwRegVal,wBitNum)                    (dwRegVal|=wBitNum)
//#define mwHost20Bit_Clr(dwRegVal,wBitNum)                    (dwRegVal&=~wBitNum)

//Offset:0x080(OTG Control/Status Register) => Suppose Word-Read & Word-Write
//~B Function

//volatile USB_OTG *gp_UsbOtg = (USB_OTG *) USB_OTG_BASE;


#define mdwOTGC_Control_B_BUS_REQ_Rd()		        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT0)//(gp_UsbOtg->OtgControlStatus &    BIT0)//(mdwFOTGPort(0x80)& BIT0)	
#define mdwOTGC_Control_B_BUS_REQ_Set()  		    (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT0)//(gp_UsbOtg->OtgControlStatus |=   BIT0)  
#define mdwOTGC_Control_B_BUS_REQ_Clr()  		    (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT0))//(gp_UsbOtg->OtgControlStatus &= (~BIT0))  

#define mdwOTGC_Control_B_HNP_EN_Rd()		        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT1)//(gp_UsbOtg->OtgControlStatus &    BIT1)	
#define mdwOTGC_Control_B_HNP_EN_Set()		        (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT1)//(gp_UsbOtg->OtgControlStatus |=   BIT1)   
#define mdwOTGC_Control_B_HNP_EN_Clr()  		    (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT1))//(gp_UsbOtg->OtgControlStatus &= (~BIT1))  

#define mdwOTGC_Control_B_DSCHG_VBUS_Rd()		    (psUsbOtg->psUsbReg->OtgControlStatus &    BIT2)//(gp_UsbOtg->OtgControlStatus &    BIT2)	
#define mdwOTGC_Control_B_DSCHG_VBUS_Set()		    (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT2)//(gp_UsbOtg->OtgControlStatus &    BIT2)    
#define mdwOTGC_Control_B_DSCHG_VBUS_Clr() 		    (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT2))//(gp_UsbOtg->OtgControlStatus &= (~BIT2))  

//~A Function 
#define mdwOTGC_Control_A_BUS_REQ_Rd()	            (psUsbOtg->psUsbReg->OtgControlStatus &    BIT4)//(gp_UsbOtg->OtgControlStatus &    BIT4)	
#define mdwOTGC_Control_A_BUS_REQ_Set()	            (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT4)//(gp_UsbOtg->OtgControlStatus |=   BIT4)    
#define mdwOTGC_Control_A_BUS_REQ_Clr()	            (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT4))//(gp_UsbOtg->OtgControlStatus &= (~BIT4))        

#define mdwOTGC_Control_A_BUS_DROP_Rd()	            (psUsbOtg->psUsbReg->OtgControlStatus &    BIT5)//(gp_UsbOtg->OtgControlStatus &    BIT5)	
#define mdwOTGC_Control_A_BUS_DROP_Set()	        (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT5)//(gp_UsbOtg->OtgControlStatus |=   BIT5)      
#define mdwOTGC_Control_A_BUS_DROP_Clr()	        (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT5))//(gp_UsbOtg->OtgControlStatus &= (~BIT5))      

#define mdwOTGC_Control_A_SET_B_HNP_EN_Rd()	        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT6)//(gp_UsbOtg->OtgControlStatus &    BIT6)	
#define mdwOTGC_Control_A_SET_B_HNP_EN_Set()	    (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT6)//(gp_UsbOtg->OtgControlStatus |=   BIT6)    
#define mdwOTGC_Control_A_SET_B_HNP_EN_Clr()	    (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT6))//(gp_UsbOtg->OtgControlStatus &= (~BIT6))    

#define mdwOTGC_Control_A_SRP_DET_EN_Rd()	        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT7)//(gp_UsbOtg->OtgControlStatus &    BIT7)	
#define mdwOTGC_Control_A_SRP_DET_EN_Set()	        (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT7)//(gp_UsbOtg->OtgControlStatus |=   BIT7)       
	#define mdwOTGC_Control_A_SRP_DET_EN_Clr()	    (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT7))//(gp_UsbOtg->OtgControlStatus &= (~BIT7))    

#define mdwOTGC_Control_A_SRP_RESP_TYPE_Rd()	    (psUsbOtg->psUsbReg->OtgControlStatus &    BIT8)//(gp_UsbOtg->OtgControlStatus &    BIT8)	
#define mdwOTGC_Control_A_SRP_RESP_TYPE_Set(b)	    (psUsbOtg->psUsbReg->OtgControlStatus |= b)//(gp_UsbOtg->OtgControlStatus |= b)      
#define mdwOTGC_Control_A_SRP_RESP_TYPE_Clr()	    (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT8))//(gp_UsbOtg->OtgControlStatus &= (~BIT8))      

#define mdwOTGC_Control_VBUS_FLT_SEL_Rd()	        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT10)//(gp_UsbOtg->OtgControlStatus &    BIT10)	
#define mdwOTGC_Control_VBUS_FLT_SEL_Set()	        (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT10)//(gp_UsbOtg->OtgControlStatus |=   BIT10)     
#define mdwOTGC_Control_VBUS_FLT_SEL_Clr()	        (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT10))//(gp_UsbOtg->OtgControlStatus &= (~BIT10))  


#define mdwOTGC_Control_711MA_Phy_Issue_Rd()        (psUsbOtg->psUsbReg->OtgControlStatus &    BIT28)//(gp_UsbOtg->OtgControlStatus &    BIT28)	
#define mdwOTGC_Control_711MA_Phy_Issue_Set()       (psUsbOtg->psUsbReg->OtgControlStatus |=   BIT28)//(gp_UsbOtg->OtgControlStatus |=   BIT28)  
#define mdwOTGC_Control_711MA_Phy_Issue_Clr()       (psUsbOtg->psUsbReg->OtgControlStatus &= (~BIT28))//(gp_UsbOtg->OtgControlStatus &= (~BIT28))  


#define mdwOTGC_Control_B_SESS_END_Rd()	            (psUsbOtg->psUsbReg->OtgControlStatus & BIT16)//(gp_UsbOtg->OtgControlStatus & BIT16)	  
#define mdwOTGC_Control_B_SESS_VLD_Rd()	            (psUsbOtg->psUsbReg->OtgControlStatus & BIT17)//(gp_UsbOtg->OtgControlStatus & BIT17)	  
#define mdwOTGC_Control_A_SESS_VLD_Rd()	            (psUsbOtg->psUsbReg->OtgControlStatus & BIT18)//(gp_UsbOtg->OtgControlStatus & BIT18)	  
#define mdwOTGC_Control_A_VBUS_VLD_Rd()	            (psUsbOtg->psUsbReg->OtgControlStatus & BIT19)  
#define mdwOTGC_Control_CROLE_Rd()	                ((psUsbOtg->psUsbReg->OtgControlStatus & BIT20)>>20)//((gp_UsbOtg->OtgControlStatus & BIT20)>>20) //0:Host 1:Peripheral
#define mdwOTGC_Control_ID_Rd()	                    ((psUsbOtg->psUsbReg->OtgControlStatus & BIT21)>>21) //0:A-Device 1:B-Device
#define mdwOTGC_Control_Rd()	                    (psUsbOtg->psUsbReg->OtgControlStatus)//(gp_UsbOtg->OtgControlStatus)    
#define mdwOTGC_Control_Speed_Rd()	                ((psUsbOtg->psUsbReg->OtgControlStatus & 0x00C00000)>>22)//((gp_UsbOtg->OtgControlStatus & 0x00C00000)>>22)

// for buff0er swap 
#define mOTG20_Wrapper_SwapBufferStart1_Set(val)		  (psUsbOtg->psUsbReg->SwapBufferStart1  = (DWORD)(val))//(gp_UsbOtg->SwapBufferStart1  = (DWORD)(val)) 	
#define mOTG20_Wrapper_SwapBufferEnd1_Set(val)		      (psUsbOtg->psUsbReg->SwapBufferEnd1    = (DWORD)(val))//(gp_UsbOtg->SwapBufferEnd1    = (DWORD)(val)) 	
#define mOTG20_Wrapper_SwapBufferStart2_Set(val)		  (psUsbOtg->psUsbReg->SwapBufferStart2  = (DWORD)(val))//(gp_UsbOtg->SwapBufferStart2  = (DWORD)(val)) 	
#define mOTG20_Wrapper_SwapBufferEnd2_Set(val)		      (psUsbOtg->psUsbReg->SwapBufferEnd2    = (DWORD)(val))//(gp_UsbOtg->SwapBufferEnd2    = (DWORD)(val)) 	
#define mOTG20_Wrapper_MaxWriteBurst_Set(val)		      (psUsbOtg->psUsbReg->WrapperCtrl=((psUsbOtg->psUsbReg->WrapperCtrl&0xFFF0FFFF)|(((DWORD)(val))<<16)))//(gp_UsbOtg->WrapperCtrl=((gp_UsbOtg->WrapperCtrl&0xFFF0FFFF)|(((DWORD)(val))<<16))) //BIT16~19 	
// VBUS Valid is controled by U_VBUSVLD (software); default is controled by OTG PHY (hardware)
#define mwOTG20_Wrapper_U_AVAVLD_SEL_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT12))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT12)) 	
#define mwOTG20_Wrapper_U_BVAVLD_SEL_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT13))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT13)) 	
#define mwOTG20_Wrapper_U_VBUSVLD_SEL_Set()		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT14))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT14)) 	
#define mwOTG20_Wrapper_U_AVAVLD_Set()  		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT20))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT20)) 	
#define mwOTG20_Wrapper_U_BVAVLD_Set()  		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT21))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT21)) 	
#define mwOTG20_Wrapper_U_VBUSVLD_Set()  		          (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT22))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT22)) 	
#define mwOTG20_Wrapper_U_AVAVLD_Clr()  		          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->WrapperCtrl, BIT20))//(mwHost20Bit_Clr  (gp_UsbOtg->WrapperCtrl, BIT20)) 	
#define mwOTG20_Wrapper_U_BVAVLD_Clr()  		          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->WrapperCtrl, BIT21))//(mwHost20Bit_Clr  (gp_UsbOtg->WrapperCtrl, BIT21)) 	
#define mwOTG20_Wrapper_U_VBUSVLD_Clr()  		          (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->WrapperCtrl, BIT22))//(mwHost20Bit_Clr  (gp_UsbOtg->WrapperCtrl, BIT22)) 	

#define mwOTG20_Wrapper_SwapBUffer1Enable_Set()		      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT24))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT24)) 	
#define mwOTG20_Wrapper_SwapBUffer2Enable_Set()		      (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT25))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT25)) 	
#define mwOTG20_Wrapper_SwapBUffer1Enable_Clr()		      (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->WrapperCtrl, BIT24))//(mwHost20Bit_Clr  (gp_UsbOtg->WrapperCtrl, BIT24)) 	
#define mwOTG20_Wrapper_SwapBUffer2Enable_Clr()		      (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->WrapperCtrl, BIT25))//(mwHost20Bit_Clr  (gp_UsbOtg->WrapperCtrl, BIT25)) 	
#define mwOTG20_Wrapper_SwapBUffer1Enable_Rd()		      (psUsbOtg->psUsbReg->WrapperCtrl&BIT24)//(gp_UsbOtg->WrapperCtrl&BIT24) 	
#define mwOTG20_Wrapper_SwapBUffer2Enable_Rd()		      (psUsbOtg->psUsbReg->WrapperCtrl&BIT25)//(gp_UsbOtg->WrapperCtrl&BIT25) 	
#define mwOTG20_Wrapper_SwapAutoClr1Enable_Set()		  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT28))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT28)) 	
#define mwOTG20_Wrapper_SwapAutoClr2Enable_Set()		  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT29))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT29)) 	
#define mwOTG20_Wrapper_OWAP_CFG_Set()		              (mwHost20Bit_Set  (psUsbOtg->psUsbReg->WrapperCtrl, BIT0))//(mwHost20Bit_Set  (gp_UsbOtg->WrapperCtrl, BIT0)) 	
#define mwOTG20_Wrapper_OWAP_CFG_Clr()		              (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->WrapperCtrl, BIT0))//(mwHost20Bit_Clr  (gp_UsbOtg->WrapperCtrl, BIT0)) 	

#define mwHost20_Misc_Physuspend_Rd()		              (mwHost20Bit_Rd  (psUsbOtg->psUsbReg->HcMisc, BIT6))//(mwHost20Bit_Rd  (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
#define mwHost20_Misc_Physuspend_Set()		                        (mwHost20Bit_Set (psUsbOtg->psUsbReg->HcMisc, BIT6))//(mwHost20Bit_Set (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
#define mwHost20_Misc_Physuspend_Clr()		                        (mwHost20Bit_Clr (psUsbOtg->psUsbReg->HcMisc, BIT6))//(mwHost20Bit_Clr (gp_UsbOtg->HcMisc, BIT6)) 	//Bit 2~3
#define mwOTG20_Control_DeviceDmaControllerParameterSetting1_Set()  (mwHost20Bit_Set  (psUsbOtg->psUsbReg->DeviceDmaControllerParameterSetting1, BIT31))//(mwHost20Bit_Set  (gp_UsbOtg->DeviceDmaControllerParameterSetting1, BIT31))  
#define mwOTG20_Control_DeviceDmaControllerParameterSetting1_Clr()	(mwHost20Bit_Clr  (psUsbOtg->psUsbReg->DeviceDmaControllerParameterSetting1, BIT31))//(mwHost20Bit_Clr  (gp_UsbOtg->DeviceDmaControllerParameterSetting1, BIT31))  

#define mwHost20_USBINTR_Rd()		                      (psUsbOtg->psUsbReg->HcUsbInterruptEnable)//(gp_UsbOtg->HcUsbInterruptEnable) 	
#define mwHost20_USBINTR_Set(bValue)		              (psUsbOtg->psUsbReg->HcUsbInterruptEnable=bValue)//(gp_UsbOtg->HcUsbInterruptEnable=bValue)

#define mwHost20_USBSTS_Rd()		                      (psUsbOtg->psUsbReg->HcUsbStatus)//(gp_UsbOtg->HcUsbStatus) 	
#define mwHost20_USBSTS_Set(wValue)		                  (psUsbOtg->psUsbReg->HcUsbStatus=wValue)//(gp_UsbOtg->HcUsbStatus=wValue) 	

#define A_SRP_RESP_TYPE_VBUS	                  0x00	  
#define A_SRP_RESP_TYPE_DATA_LINE                 0x100


//Offset:0x004(OTG Interrupt Status Register) 
#define mdwOTGC_INT_STS_Rd()                        (psUsbOtg->psUsbReg->OtgInterruptStatus)//(gp_UsbOtg->OtgInterruptStatus)
#define mdwOTGC_INT_STS_Clr(wValue)                 (psUsbOtg->psUsbReg->OtgInterruptStatus |= wValue)//(gp_UsbOtg->OtgInterruptStatus |= wValue)    



#define OTGC_INT_BSRPDN                           BIT0  
#define OTGC_INT_ASRPDET                          BIT4
#define OTGC_INT_AVBUSERR                         BIT5

#define OTGC_INT_BSESSEND                         BIT6
#define OTGC_INT_RLCHG                            BIT8
#define OTGC_INT_IDCHG                            BIT9

#define OTGC_INT_OVC                              BIT10
#define OTGC_INT_BPLGRMV                          BIT11
#define OTGC_INT_APLGRMV                          BIT12

#define OTGC_INT_ALL_TYPE                         (OTGC_INT_BSRPDN|OTGC_INT_ASRPDET|OTGC_INT_AVBUSERR|OTGC_INT_BSESSEND|OTGC_INT_RLCHG|OTGC_INT_IDCHG|OTGC_INT_OVC|OTGC_INT_BPLGRMV|OTGC_INT_APLGRMV)
//
#define OTGC_INT_A_TYPE                           (OTGC_INT_ASRPDET|OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG|OTGC_INT_APLGRMV)
//
#define OTGC_INT_B_TYPE                           (OTGC_INT_BSRPDN|OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG)

//Offset:0x008(OTG Interrupt Enable Register) 
#define mdwOTGC_INT_Enable_Rd()                   (psUsbOtg->psUsbReg->OtgInterruptEnable)//(gp_UsbOtg->OtgInterruptEnable)
#define mdwOTGC_INT_Enable_Set(wValue)            (psUsbOtg->psUsbReg->OtgInterruptEnable |= wValue)//(gp_UsbOtg->OtgInterruptEnable |= wValue)
#define mdwOTGC_INT_Enable_Clr(wValue)            (psUsbOtg->psUsbReg->OtgInterruptEnable &= (~wValue))//(gp_UsbOtg->OtgInterruptEnable &= (~wValue))

#define mdwOTGC_INT_Enable_BSRPDN_Set()           (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT0)//(gp_UsbOtg->OtgInterruptEnable |=  BIT0)   
#define mdwOTGC_INT_Enable_ASRPDET_Set()          (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT4)//(gp_UsbOtg->OtgInterruptEnable |=  BIT4)   
#define mdwOTGC_INT_Enable_AVBUSERR_Set()         (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT5)//(gp_UsbOtg->OtgInterruptEnable |=  BIT5)   
#define mdwOTGC_INT_Enable_RLCHG_Set()            (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT8)//(gp_UsbOtg->OtgInterruptEnable |=  BIT8)   
#define mdwOTGC_INT_Enable_IDCHG_Set()            (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT8)//(gp_UsbOtg->OtgInterruptEnable |=  BIT9)   
#define mdwOTGC_INT_Enable_OVC_Set()              (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT8)//(gp_UsbOtg->OtgInterruptEnable |=  BIT10)   
#define mdwOTGC_INT_Enable_BPLGRMV_Set()          (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT8)//(gp_UsbOtg->OtgInterruptEnable |=  BIT11)   
#define mdwOTGC_INT_Enable_APLGRMV_Set()          (psUsbOtg->psUsbReg->OtgInterruptEnable |=  BIT8)//(gp_UsbOtg->OtgInterruptEnable |=  BIT12)   

#define mdwOTGC_INT_Enable_BSRPDN_Clr()           (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT0)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT0)        
#define mdwOTGC_INT_Enable_ASRPDET_Clr()          (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT4)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT4)        
#define mdwOTGC_INT_Enable_AVBUSERR_Clr()         (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT5)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT5)        
#define mdwOTGC_INT_Enable_RLCHG_Clr()            (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT8)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT8)        
#define mdwOTGC_INT_Enable_IDCHG_Clr()            (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT9)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT9)        
#define mdwOTGC_INT_Enable_OVC_Clr()              (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT10)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT10)        
#define mdwOTGC_INT_Enable_BPLGRMV_Clr()          (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT11)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT11)       
#define mdwOTGC_INT_Enable_APLGRMV_Clr()          (psUsbOtg->psUsbReg->OtgInterruptEnable &= ~BIT12)//(gp_UsbOtg->OtgInterruptEnable &= ~BIT12)       

#define mdwOTGC_Control_Internal_Test()	          (psUsbOtg->psUsbReg->OtgControlStatus |= BIT13)//(gp_UsbOtg->OtgControlStatus |= BIT13)	  
#define mdwOTGC_Control_Internal_Test_Clr()       (psUsbOtg->psUsbReg->OtgControlStatus &= ~ BIT13)//(gp_UsbOtg->OtgControlStatus &= ~ BIT13)	  

#define mwOTG20_Interrupt_Mask_OTG_Clr()		  (mwHost20Bit_Clr  (psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt, BIT1))//(mwHost20Bit_Clr  (gp_UsbOtg->GlobalMaskofHcOtgDevInterrupt, BIT1))
#define mwOTG20_Interrupt_OTG_Enable()		      mwOTG20_Interrupt_Mask_OTG_Clr()


#define mdwOTG_PHY_Enable_Set(wValue)            (psUsbOtg->psUsbReg->PhyUtmiSwCtrl |= wValue)
#define mdwOTG_PHY_Enable_Clr(wValue)            (psUsbOtg->psUsbReg->PhyUtmiSwCtrl &= (~wValue))

#define USB_OTG_HOST            0
//#define USB_OTG_PERIPHERAL      1
#define USB_OTG_A_DEVCIE        0
#define USB_OTG_B_DEVCIE        1

typedef enum 
{
	OTG_ID_A_TYPE = 0,
	OTG_ID_B_TYPE

} OTGC_ID_Type;         

typedef enum 
{
	OTG_CurrentRole_Host = 0,
	OTG_CurrentRole_Peripheral 

} OTGC_CurrentRole; 


typedef enum {
    HAS_NO_FRAME,
    NEW_FRAME_BEGIN,
    NEW_FRAME_END
} FRAME_STATE;


//===================3.Define Stricture ==================================================================
//========================================================================================================


//Device
//some structures pointer are dynamic allocate
//CDC
typedef struct _USB_OTG_DEVICE_CDC_
{
    DWORD  dwCdcBulkReadIdx;//static DWORD gCdcBulkReadIdx = 0;
    DWORD  dwCdcBulkWriteIdx;//static DWORD gCdcBulkWriteIdx = 0;
    BOOL   boIsAbleToUseUsbdCdc;//static BOOL gIsAbleToUseUsbdCdc = FALSE;
    WORD   wTotalByteCnt;//static WORD total_byte_cnt = 0;
    WORD   wByteIndex;//static WORD byte_index = 0;
} USB_DEVICE_CDC, *PUSB_DEVICE_CDC;

//MSDC
// Block Descriptor/
typedef struct _USB_OTG_DEVICE_MSDC_ 
{
	PCBW    psCbw;//PCBW    gpCbw;
    PCSW    psCsw;//PCSW    gpCsw;

    MODE_PARA_HDR6          sModeParaHdr6;//static MODE_PARA_HDR6          gModeParaHdr6;
    MODE_PARA_HDR10         sModeParaHdr10;//static MODE_PARA_HDR10         gModeParaHdr10;
    BLK_DSCRPT              sBlkDscrpt[BLK_DSCRPT_MAX];//static BLK_DSCRPT              gBlkDscrpt[BLK_DSCRPT_MAX];
    PARAMETER_PAGE          sParaPage[PARA_PAGE_MAX];//static PARAMETER_PAGE          gParaPage[PARA_PAGE_MAX];

    CAPACITY_DESCRIPTOR     sCapacityDescriptor;//static CapacityDescriptor      gCapacityDescriptor ;
    MEDIA_INFO_STR          sMediaInfo;//static MediaInfoStr            gMediaInfo;
    DEVICE_INFORMATION_STR  sDeviceInf;//static DeviceInformationStr    gDeviceInf;     // Device information
    REQUEST_SENSE_STR       sRequestSense[MSDC_MAX_LUN];//static RequestSenseStr     gRequestSense[MSDC_MAX_LUN];      // Request sense key

    DWORD					dwTxByteCnt;//static DWORD tx_byte_cnt = 0;
    DWORD					dwRxByteCnt;//static DWORD rx_byte_cnt = 0;
    DWORD					dwWriteLba;//static DWORD write_lba = 0;
    WORD					wSectorCnt;//static WORD  sector_cnt = 0;

    WORD                    wCheckCondition[MSDC_MAX_LUN];//static WORD                gCheckCondition[MSDC_MAX_LUN];
    BOOL                    boIsMediumRemoval[MSDC_MAX_LUN];//static BOOL                gIsMediumRemoval[MSDC_MAX_LUN];
    BOOL                    boIsReadWriteData;//static BOOL                gIsReadWriteData = FALSE;
    BOOL                    boMediaNotPresent[MSDC_MAX_LUN];//static BOOL                gMediaNotPresent[MSDC_MAX_LUN];// = {FALSE, FALSE, FALSE, FALSE};
    BOOL                    boIsPrevent[MSDC_MAX_LUN];//static BOOL                gIsPrevent[MSDC_MAX_LUN];

    #if USBOTG_DEVICE_MSDC_TECO
    BYTE    *pbTecoDataBuff;  // Receive data from psUsbDevDesc->pbBulkTxRxData
    #endif
} USB_DEVICE_MSDC, *PUSB_DEVICE_MSDC;

//SIDC

typedef struct{
    struct
    {
        ConfPrintServiceHost hostConfPrintService;
        DeviceStatus hostDeviceStatus;
    } stPrinterStatus;//printerStatus;
    
    struct
    {
        DpsJobConfig jobConf;
        DpsPrintJobInfo *pPrintJobInfo; //array
        DWORD printJobInfoCount;
        JobStatus jobStatus;
    } stPrintJobInfo;//printJobInfo;
    
    struct
    {
        LastOpType opType;
        Result opResult;
        LastOpStatus opStatus;
    } stDscStatus;//dscStatus;
    
    DpsPrinterCapability stDpsPrinterCapability;//static DpsPrinterCapability dpsPrinterCapability;
    PaperSize stRequestCapabilityPaperSize;//static PaperSize _gRequestCapabilityPaperSize = noPaperSize;
    DWORD stRequestCapability;//static DWORD _gRequestCapability = 0;
    DWORD dwFilePrintCount;// DWORD FilePrintCount = 0;
    
    BOOL boIsNewJobOk;//BOOL gIsNewJobOk = FALSE;
    BOOL boIsGetCapAlready;//BOOL gIsGetCapAlready = FALSE;
    BOOL boIsJobFinished;//BOOL gIsJobFinished = FALSE;
    BOOL boIsStartJob;//BOOL gIsStartJob = FALSE;
    
    DpsPrinterSettings stPrinterSettings; // PBridge Test//DpsPrinterSettings gPrinterSettings;	// PBridge Test
    BYTE *pbXmlBuff; //BYTE gXmlBuff[DPS_BUFF_SIZE];
    DpsJobConfig stPrintJobConfig; //DpsJobConfig printJobConfig;
    DpsPrintJobInfo  *pstPrintInfo; //DpsPrintJobInfo  printInfo[MAX_FILE_PRINT];
    DWORD dwLayoutForPaperSizeIdx;//static DWORD layoutForPaperSizeIndex = 0;
    DWORD dwPaperTypeForPaperSizeIdx;//static DWORD paperTypeForPaperSizeIndex = 0;    
    BOOL boIsPrinting;//static BOOL isPrinting = FALSE;
	BOOL boSend;//static BOOL send = 1;
    DWORD dwDpsPaperTypeLoopCnt; //static int dwDpsPaperTypeLoopCnt = 0;
    DWORD dwDpsLayoutLoopCnt;//static int dwDpsLayoutLoopCnt = 0;   
} PICT_BRIDGE_DPS, *PPICT_BRIDGE_DPS;

typedef struct _USB_OTG_DEVICE_SIDC_ 
{
    PSTI_CONTAINER  psStiCommand;//PSTI_CONTAINER   gpStiCommand;
    PSTI_CONTAINER  psStiResponse;//PSTI_CONTAINER  gpStiResponse;
    PSTI_EVENT      psStiEvent;//PSTI_EVENT     gpStiEvent;
    
    PICT_BRIDGE_DPS sDps;

    BYTE    bStrFile[16];//static char     gStrFile[15] = "";       // file name
//    WORD    wOperationsSupported[NUM_OPERATION_SUPPORTED];//WORD    gOperationsSupported[NUM_OPERATION_SUPPORTED];
//    WORD    wEventsSupported[NUM_EVENT_SUPPORTED];//WORD    gEventsSupported[NUM_EVENT_SUPPORTED];
//    WORD    wDevicePropertiesSupported[NUM_DEVICE_PROP_CODE_SUPPORTED];//WORD   gDevicePropertiesSupported[NUM_DEVICE_PROP_CODE_SUPPORTED];
//    WORD    wCaptureFormats[NUM_CAPTURE_FORMAT_SUPPORTED];//WORD    gCaptureFormats[NUM_CAPTURE_FORMAT_SUPPORTED];
//    WORD    wImageFormats[NUM_IMAGE_FORMAT_SUPPORTED];//WORD    gImageFormats[NUM_IMAGE_FORMAT_SUPPORTED];
//    WORD    wModelString[MODEL_STRING_LENGTH];//WORD    wModelString[MODEL_STRING_LENGTH]
//    WORD    wDeviceVersionString[DEVICE_VERSION_STRING_LENGTH]; // "01.00   "//WORD gDeviceVersionString[DEVICE_VERSION_STRING_LENGTH]; // "01.00   "
//    WORD    wSerialNumberString[SERIAL_NUMBER_STRING_LENGTH];//WORD gSerialNumberString[SERIAL_NUMBER_STRING_LENGTH]; 
    STR16   sParentFileName;//STR16 gParentFileName;    // DCIM
    DWORD   dwPtpStorageIDs;//DWORD   gPtpStorageIDs[];//, ALL_DEVICE_STORAGE_ID};
    DWORD   dwRootObjectHandle;//DWORD   gRootObjectHandle = 0;
    BOOL    boIsImgObjectHandleReady;//BOOL gIsImgObjectHandleReady = FALSE;
    STI_DEVICE_INFO     sStiDeviceInfo;//STI_DEVICE_INFO     gStiDeviceInfo;
    STI_OBJECT_INFO     sStiParentObjectInfo;//STI_OBJECT_INFO     gStiParentObjectInfo;
    DWORD   dwSessionID;//DWORD gSessionID      = 0;
    DWORD   dwTransactionID;//DWORD gTransactionID  = 0;
    DWORD   dwStorageID;//DWORD gStorageID      = 0;
    POBJECT_HEADER   psPtpHandles;//sPtpHandles[MAX_NUM_OF_OBJECTS];//OBJECT_HEADER   gPtpHandles[MAX_NUM_OF_OBJECTS];
    DWORD           dwPtpNumHandles;//DWORD           gPtpNumHandles = 0;
    BOOL        boDpsEnabled;//BOOL        gDpsEnabled       = FALSE;
    DWORD       dwDpsCurrHdl;//DWORD       gDpsCurrHdl       = 0
    DWORD       dwDpsDDiscHdl;//DWORD       gDpsDDiscHdl      = 0  // (<-) Start to connect and will get dpsHDiscHdl
    DWORD       dwDpsDReqHdl;//DWORD       gDpsDReqHdl       = 0  // (<-) will get HRSPONSE.DPS
    DWORD       dwDpsDRespHdl;//DWORD       gDpsDRespHdl      = 0 // (<-) Event(0x4009)
    DWORD       dwDpsHDiscHdl;//DWORD       gDpsHDiscHdl      = 0  // (->) vs. dpsDDiscHdl
    DWORD       dwDpsHReqHdl;//DWORD       gDpsHReqHdl       = 0  // (->) Send host(Print) Status to slave(DSC)
    DWORD       dwDpsHRespHdl;//DWORD       gDpsHRespHdl      = 0 // (->) vs. dpsDReqHdl
    DWORD       dwDpsReqLen;//DWORD       gDpsReqLen        = 0
    DWORD       dwDpsRespLen;//DWORD       gDpsRespLen       = 0
    DWORD   dwDpsState;//DWORD   gDpsState       = DPS_IDLE;
    BYTE  *pbDpsReqBuf;//bDpsReqBuf[PTP_DPS_MAX_XFR];//static BYTE  gDpsReqBuf[PTP_DPS_MAX_XFR]     //DPS Request Buffer
    BYTE  *pbDpsRespBuf;//bDpsRespBuf[PTP_DPS_MAX_XFR];//static BYTE  gDpsRespBuf[PTP_DPS_MAX_XFR];   //DPS Request Buffer
    BOOL    boIsHostReq;//BOOL    gIsHostReq = FALSE;
    BOOL    boIsDeviceReq;//BOOL    gIsDeviceReq = FALSE;
    STREAM *psFile;//STREAM *ghFile = (STREAM*)NULL;
    BOOL    boIsFileSending;//BOOL    gIsFileSending = FALSE;
    SDWORD   sdwImageSize;//SDWORD   gImageSize = 0;
    DWORD   dwImageSizeRead;//DWORD   gImageSizeRead = 0;
    BOOL    boIsDpsSendEvent;//BOOL    gIsDpsSendEvent = FALSE;
    BOOL boIsSendingData;//BOOL gIsSendingData = FALSE;
//    BYTE *pbCancelRequestData;//BYTE *gpCancelRequestData;
//    BYTE *pbGetDeviceStatusRequest;//BYTE *gpGetDeviceStatusRequest;
    // for multi-copy and borderless
    PRINTER_CAPABILITY *psPrinterCap;//PRINTER_CAPABILITY *g_stPrinterCap;
    PRINTER_CAPABILITY sTestPrintCapa;//PRINTER_CAPABILITY testPrintCapa;
    // for Thumbnail information
    ThumbInfo sThumbnailInfo;//ThumbInfo gThumbnailInfo;
//    BYTE bCancelRequestData[CANCEL_REQUEST_DATA_LENGTH];//BYTE gCancelRequestData[CANCEL_REQUEST_DATA_LENGTH];
//    BYTE bGetDeviceStatusRequest[GET_DEVICE_STATUS_REQUETS_LENGTH];//gGetDeviceStatusRequest[GET_DEVICE_STATUS_REQUETS_LENGTH];
    WORD wPrintedFileType;// WORD gPrintedFileType = 0;
    BYTE bIsThumbnail;//BYTE gIsThumbnail = 0;
    BYTE bPtpDataSource;//BYTE    gPtpDataSource    = PTP_DATA_IDLE;
    DWORD dwTotalLen;//static DWORD total_len = 0;    
    BOOL bIsOneHandle;
} USB_DEVICE_SIDC, *PUSB_DEVICE_SIDC;

typedef struct _USB_OTG_DEVICE_DESCRIPTOR_
{
    BYTE                *pbConfigHs;//Cofig_HS_Desc[256];
    BYTE                *pbConfigFs;//Cofig_FS_Desc[256];
    BYTE                *pbConfigHsOtherSpeedDesc;//Cofig_HS_Other_Speed_Desc[256];
    BYTE                *pbConfigFsOtherSpeedDesc;//Cofig_FS_Other_Speed_Desc[256];
//    BYTE                *pbMsdcDeviceDescriptor;//gMsdcDeviceDescriptor[DEVICE_LENGTH] =
//    BYTE                *pbSidcDeviceDescriptor;//gStiDeviceDescriptor[DEVICE_LENGTH] =
//    BYTE                *pbVendorDeviceDescriptor;//gVendorDeviceDescriptor[DEVICE_LENGTH] =
//    BYTE                *pbCdcDeviceDescriptor;//gCdcDeviceDescriptor[DEVICE_LENGTH] =
//    BYTE                *pbConfigDescriptor;//gConfigDescriptor[CONFIG_LENGTH] =
//    BYTE                *pbMsdcInterfaceDescriptor;//gMsdcInterfaceDescriptor[INTERFACE_LENGTH] =
//    BYTE                *pbSidcInterfaceDescriptor;//gStiInterfaceDescriptor[INTERFACE_LENGTH] =
//    BYTE                *pbVendorInterfaceDescriptor;//gVendorInterfaceDescriptor[INTERFACE_LENGTH] =
//    BYTE                *pbCdcInterfaceDescriptor1;//gCdcInterfaceDescriptor1[INTERFACE_LENGTH] =
//    BYTE                *pbCdcInterfaceDescriptor2;;//gCdcInterfaceDescriptor2[INTERFACE_LENGTH] =
//    BYTE                *pbCdcFunctionalDescriptor1;//gCdcFunctionalDescriptor1[5] =
//    BYTE                *pbCdcFunctionalDescriptor2;//gCdcFunctionalDescriptor2[4] =
//    BYTE                *pbCdcFunctionalDescriptor3;//gCdcFunctionalDescriptor3[5] =
//    BYTE                *pbHsEndpointDescriptorEp1;//gHsEndpointDescriptorEp1[EP_LENGTH] =
//    BYTE                *pbHsEndpointDescriptorEp2;//gHsEndpointDescriptorEp2[EP_LENGTH] =
//    BYTE                *pbHsEndpointDescriptorEp3;//gHsEndpointDescriptorEp3[EP_LENGTH] =
//    BYTE                *pbFsEndpointDescriptorEp1;//gFsEndpointDescriptorEp1[EP_LENGTH] =
//    BYTE                *pbFsEndpointDescriptorEp2;//gFsEndpointDescriptorEp2[EP_LENGTH] =
//    BYTE                *pbFsEndpointDescriptorEp3;//gFsEndpointDescriptorEp3[EP_LENGTH] =
//    BYTE                *pbLangIdString;//gLangIdString[4] =
//    BYTE                *pbManufacturerString;//gManufacturerString[MANUFACTURER_SRING_LEN] =
//    BYTE                *pbProductString;//gProductString[PRODUCT_SRING_LEN] =
//    BYTE                *pbCdcProductString;//gCdcProductString[20] =
//    BYTE                *pbSerialnumberString;//gSerialnumberString[SERIALNUMBER_SRING_LEN] =
//    BYTE                *pbDeviceQualifierDescriptor;//gDeviceQualifierDescriptor[DEVICE_QUALIFIER_LENGTH] =
//    BYTE                *pbOtherSpeedConfigurationDescriptor;//gOtherSpeedConfigurationDescriptor[INTERFACE_LENGTH] =
//    BYTE                *pbOtherSpeedCdcConfigurationDescriptor;//gOtherSpeedCdcConfigurationDescriptor[INTERFACE_LENGTH] =
    BYTE                *pbDescriptorEx;//gpu8DescriptorEX;//+
    BYTE                *pbBulkTxRxData;//gpBulkTxRxData = 0;
    BYTE                bUsbInterruptLevel1;//gusb_interrupt_level1 = 0;
    BYTE                bUsbConfigValue;//gUsbConfigValue;
    BYTE                bUsbInterfaceValue;//gUsbInterfaceValue;
    BYTE                bUsbInterfaceAlternateSetting;//gUsbInterfaceAlternateSetting;
    BYTE                bUsbMessageLevel;//gUsbMessageLevel;
    BYTE                bUsbDefaultInterfaceValue;//gUsbDefaultInterfaceValue;
    BYTE                bUsbDefaultInterfaceValue2;//gUsbDefaultInterfaceValue2;
    BYTE                bReserved[1];
    WORD                bUsbApMode;//gUsbApMode;
    WORD                wTxRxCounter;//gTxRxCounter;//+
    DWORD               dwBulkTxRxCounter;//gBulkTxRxCounter = 0;
    DWORD               dwBulkTxRxLength;//gBulkTxRxLength = 0;
    BOOL                boIsUsbdInitiated;//gIsUsbdPlugIn = FALSE;
    BOOL                boIsEnableIntEp;//gIsEnableIntEp = FALSE;
    BOOL                boOtgChirpFinish;//gbOTGChirpFinish = FALSE;//+
    BOOL                boOtgEp0HaltSt;//gbOTGEP0HaltSt = FALSE;//+
    BOOL                boOtgHighSpeed;//gbOTGHighSpeed = FALSE;//+
} USB_DEVICE_DESC, *PUSB_DEVICE_DESC;

typedef struct _USB_OTG_DEVICE_
{
    BOOL                boIsAlreadyInit;
    TRANSACTION_STATE   eUsbTransactionState;//geUsbTransactionState;//+
    PSETUP_PACKET       psControlCmd;//gpControlCmd;//+
    COMMAND_TYPE        eUsbCxCommand;//geUsbCxCommand;
    ACTION              eUsbCxFinishAction;//geUsbCxFinishAction;
    BYTE			bCurrentStat; // USB device current state
    struct
    {
        BYTE bPlugFlag;//BYTE g_bUsbdPlugFlag = 0;
        BYTE bStableCount;//DWORD dwStableCount = 0;
    } stDetect;
    USB_DEVICE_DESC     sDesc;//
//cdc
    USB_DEVICE_CDC      sCdc;
//msdc
    USB_DEVICE_MSDC     sMsdc;
//sidc
    USB_DEVICE_SIDC     sSidc;

} ST_USB_OTG_DEVICE, *PST_USB_OTG_DEVICE; 


//host
//CDC
typedef struct _USB_OTG_HOST_CDC_
{
    BYTE *pbCdcVendorCmdData;//BYTE CDC_VENDOR_CMD_DATA[0x10] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x90, 0x30, 0x00, 0x00, 0x00, 0x00};
    BYTE *pbCdcSetLineCodingData;//BYTE CDC_SET_LINE_CODING_DATA[0x07] = {0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08};
    ST_MCARD_MAIL   *psCdcEnvelope;
} USB_HOST_CDC, *PUSB_HOST_CDC;

//HID
typedef struct _USB_OTG_HOST_HID_
{
    BYTE   *pbHidReportData;          // Hid Report Data from USB Packet
    ST_MCARD_MAIL          *psHidEnvelope;    
} USB_HOST_HID, *PUSB_HOST_HID;

//DATANG
#if USBOTG_HOST_DATANG
typedef struct _USB_OTG_HOST_DATANG_
{
	ST_MCARD_MAIL   *psDatangEnvelope;
} USB_HOST_DATANG, *PUSB_HOST_DATANG;
#endif

//MSDC
typedef struct _USB_OTG_HOST_MSDC_
{
    DWORD   dwCbwTag;//DWORD   gCbwTag = 0;
    BOOL boIsModeSense6TryAgain;//BOOL gIsModeSense6TryAgain = FALSE;
    BOOL boSupportMultiLun;//static BOOL gSupportMultiLun = FALSE;
    BOOL boIsMsdcBulkOnly;//static BOOL gIsMsdcBulkOnly = FALSE;
    BOOL boNoNeedToPollingUsbDevice;//static BOOL gNoNeedToPollingUsbDevice = FALSE;//Joe 3/19: USB Hub
    BOOL boIsCmpTimerStart;//BOOL gIsCmpTimerStart = FALSE;
    BYTE bScsiCmd[CBI_CMD_BYTE_CNT];//BYTE scsi_cmd[CBI_CMD_BYTE_CNT];
    BYTE bDescriptor[MAX_HOST_LUN][5];//static BYTE gbDescriptor[MAX_LUN][5] = {"LUN_0", "LUN_1", "LUN_2", "LUN_3", "PTP_0", "PTP_1"}
    BYTE bUsbhDeviceAddress;//static BYTE gUsbhDeviceAddress = 0;
#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
    BYTE bUsbIfTestSts; //gUsbIfTestSts = USB_IF_TEST_DISABLE;
#else
    BYTE bReserved[1];
#endif
    BYTE bModeSense6RetryCnt;//static BYTE try_cnt = 0;
} USB_HOST_MSDC, *PUSB_HOST_MSDC;

//AVDC
#define MJPEG_NUM_BUF	2
#define AUDIO_NUM_BUF	4
#define OTGH_ISO_VIDEO_DATA_BUFFER_NUM		12 // it's better to be common multiple of 3(3073:max isoc tx size) and 4(4096:page size)
#define OTGH_ISO_AUDIO_DATA_BUFFER_NUM		1
#define MAX_NUM_VIDEO_FORMAT_INDEX          8 // it's 7 for Sonix SN9C291
//#define PERIODIC_FRAME_SIZE 256

typedef struct _WEBCAM_VIDEO_STREAM_
{
	DWORD dwStreamActive;//DWORD streamActive;
	DWORD dwBufferCurIndex;//DWORD bufferCurIndex;
	DWORD dwScaniTDIdx;//DWORD scaniTDIdx;
	DWORD dwBufferActive[MJPEG_NUM_BUF];//DWORD bufferActive[MJPEG_NUM_BUF];
	DWORD dwBufferLength[MJPEG_NUM_BUF];//DWORD bufferLength[MJPEG_NUM_BUF];
	BYTE  *pbVideoMjpegBuffer[MJPEG_NUM_BUF];//BYTE  *videoMjpegBuffer[MJPEG_NUM_BUF];
	// ISO
	DWORD dwiTdInt;//DWORD iTDInt;
	DWORD dwiTdIdx;//DWORD iTDIdx;
    DWORD dwiTdNum[2];//DWORD iTDNum[2];
	DWORD dwOriginalFrameNumber[2];//DWORD wOriginalFrameNumber[2];
	DWORD dwLastFrameNumber[2];//DWORD wLastFrameNumber[2];
	DWORD dwIsoInActive[2];//DWORD bISOInActive[2];
} WEBCAM_VIDEO_STREAM, *PWEBCAM_VIDEO_STREAM;

typedef struct _WEBCAM_AUDIO_STREAM_
{
	DWORD dwStreamActive;//DWORD streamActive;
	DWORD dwBufferCurIndex;//DWORD bufferCurIndex;
	DWORD dwBufferActive[AUDIO_NUM_BUF];//DWORD bufferActive[AUDIO_NUM_BUF];
	DWORD dwBufferLength[AUDIO_NUM_BUF];//DWORD bufferLength[AUDIO_NUM_BUF];
	BYTE  *pbAudioBuffer[AUDIO_NUM_BUF];//BYTE  *audioBuffer[AUDIO_NUM_BUF];
	// ISO
	DWORD dwiTdInt;;//DWORD iTDInt;
    DWORD dwiTdIdx;//	DWORD iTDIdx;
    DWORD dwiTdNum[2];//DWORD iTDNum[2];
	DWORD dwOriginalFrameNumber[2];//DWORD wOriginalFrameNumber[2];
	DWORD dwLastFrameNumber[2];//DWORD wLastFrameNumber[2];
	DWORD dwIsoInActive[2];//DWORD bISOInActive[2];
} WEBCAM_AUDIO_STREAM, *PWEBCAM_AUDIO_STREAM;

#define OTGH_PT_ISO_DATABUFFER_NUM                2 // //600 Max support 600*4K=2.4M
#define OTGH_PT_ISO_OUT_DATABUFFER_NUM                2 // //600 Max support 600*4K=2.4M
#define OTGH_PT_ISO_OUT_DATAPAGE_INDEX                2 // //600 Max support 600*4K=2.4M
#if (USBOTG_WEB_CAM == ENABLE)
typedef struct
{	
   //DWORD      dwDataBufferArray[OTGH_PT_ISO_DATABUFFER_NUM+OTGH_PT_ISO_OUT_DATABUFFER_NUM];//Max support 8*4K=32K
   DWORD      dwDataBufferArray[Host20_Page_MAX];//Max support 8*4K=32K

}OTGH_PT_ISO_STRUCT, *POTGH_PT_ISO_STRUCT;

typedef struct _USB_OTG_HOST_UVC_
{	
    FRAME_STATE eNewFrame;
    DWORD       dwFrameNumber;
    DWORD       dwOriginalFrameNumber;
    DWORD       dwLastFrameNumber;
    BYTE        bFrameID;
    BYTE        bStartOfFrame;
    BYTE        bNewOneFrame;
    BYTE        bReserved;
}USB_HOST_UVC, *PUSB_HOST_UVC;

typedef struct _USB_OTG_HOST_AVDC_
{
    ST_MCARD_MAIL   *psWebCamEnvelope;//ST_MCARD_MAIL       *gpUsbOtgHostForWebCamEnvelope;
    DWORD dwVideoStreamInterfaceNumber;//static DWORD webCamVideoStreamInterfaceNumber = 0;
    DWORD dwVideoStreamInterfaceAlternateSetting;//static DWORD webCamVideoStreamItervaceAlternateSetting = 0;
    DWORD dwVideoStreamInterfaceEndPointNumber;//static DWORD webCamVideoStreamInterfaceEndPointNumber = 0;
    DWORD dwVideoStreamMaxPacketSize;//static DWORD webCamVideoStreamMaxPacketSize = 0;
    
    DWORD dwVideoStreamTotalFormatIndex;///static DWORD webCamVideoStreamTotalFrameIndex;
    DWORD dwVideoStreamCurrentFormatIndex;///static DWORD webCamVideoStreamTotalFrameIndex;
	DWORD dwVideoStreamFormat[MAX_NUM_VIDEO_FORMAT_INDEX];//static DWORD webCamVideoStreamFrameIndex;
	DWORD dwVideoStreamFormatIndex[MAX_NUM_VIDEO_FORMAT_INDEX];//static DWORD webCamVideoStreamFrameIndex;
    DWORD dwVideoStreamFrameIndex[MAX_NUM_VIDEO_FORMAT_INDEX];//static DWORD webCamVideoStreamFrameIndex;
    DWORD dwVideoStreamSize[MAX_NUM_VIDEO_FORMAT_INDEX];//static DWORD webCamVideoStreamSize[MAX_NUM_FRAME_INDEX]; // width.height
    
    DWORD dwAudioStreamInterfaceNumber;//static DWORD webCamAudioStreamInterfaceNumber = 0;
    DWORD dwAudioStreamInterfaceAlternateSetting;//static DWORD webCamAudioStreamItervaceAlternateSetting = 0;
    DWORD dwAudioStreamInterfaceEndPointNumber;//static DWORD webCamAudioStreamInterfaceEndPointNumber = 0;
    DWORD dwAudioStreamMaxPacketSize;//static DWORD webCamAudioMaxPacketSize = 0;
    DWORD dwAudioFormatType;//static DWORD webCamAudioFormatType = 0;
    DWORD dwAudioFreqRate;//static DWORD webCamAudioFreqRate = 0;
    DWORD dwAudioSampleSize;//static DWORD webCamAudioSampleSize = 0;

    DWORD dwVideoFrameList[PERIODIC_FRAME_SIZE];//static DWORD videoFrameList[PERIODIC_FRAME_SIZE];
    DWORD dwAudioFrameList[PERIODIC_FRAME_SIZE];//static DWORD audioFrameList[PERIODIC_FRAME_SIZE];
    //DWORD dwMjpgBufferSize;;//static DWORD mjpgBufferSize;
    //WEBCAM_VIDEO_STREAM sVideoMjpegStream;//static struct webCamVideoStream videoMjpegStream;

    DWORD dwAudioBufferSize;//static DWORD AUDIOBUFFERSIZE;
    WEBCAM_AUDIO_STREAM sAudioStream;//static struct webCamAudioStream audioStream;
    OTGH_PT_ISO_STRUCT sOtgHostIso;//static OTGH_PT_ISO_Struct sOTGH_PT_ISO;
    PST_USBH_DEVICE_DESCRIPTOR psDeviceAp;//static PST_USBH_DEVICE_DESCRIPTOR psDevice_AP;
    BOOL boVideoStreamInit;//static BOOL videoStremInit = FALSE;
    //DWORD dwMjpegFrameCnt;//static DWORD mjpegFrameCnt = 0;
    //DWORD dwMjpegLostFrameCnt;//static DWORD mjpegLostFrameCnt = 0;
    DWORD dwTimerTick;//static DWORD timerTick = 0;
    DWORD dwDoor;//static DWORD door = 0;
    //BYTE  *pbVideoMjpegBuffer;//static BYTE  *videoMjpegBuffer;
    BYTE  *pbAudioBuffer;//static BYTE  *audioBuffer;
    DWORD dwAudioDataCnt;//static DWORD audioDataCnt = 0;
    DWORD dwTimerTick1;//static DWORD timerTick1 = 0;
    USBH_WEBCAM_VIDEO_FORMAT eVideoFormat;
    DWORD dwFormatIndex;
    DWORD dwFrameIndex;
    DWORD dwCompresionIndex;
    DWORD dwVideoFrameSize;
    USBH_UVC_FORMAT_INFORMATION sUvcFormatInfo[MAX_NUM_VIDEO_FORMAT_INDEX];
} USB_HOST_AVDC, *PUSB_HOST_AVDC;
#endif

typedef struct _USB_OTG_HOST_SIDC_
{
    PUSBH_PTP_MAIL_TAG psPtpEnvelope;//USBH_PTP_MAIL_TAG   *gpUsbOtgHostForPtpEnvelope;
    DWORD dwTransactionId ;//DWORD g_dwTransactionId = 0;
    BOOL boScanObjects;//boUsbOtgHostSidcScanObjects;//BOOL sbUsbOtgHostSidcScanObjects = FALSE;
    BOOL boGetPartialObject;//boUsbOtgHostSidcGetPartialObject;//BOOL sbUsbOtgHostSidcGetPartialObject = FALSE;
    BOOL boGetObject;//boUsbOtgHostSidcGetObject;//BOOL sbUsbOtgHostSidcGetObject = FALSE;
    BOOL boGetObjectInfo;//boUsbOtgHostSidcGetObjectInfo;//BOOL sbUsbOtgHostSidcGetObjectInfo = FALSE;
    DWORD dwBuffer;//DWORD sdwBuffer = 0;
    DWORD dwCount;//DWORD sdwCount = 0;

    struct
    {
        DWORD dwTotalNumberOfStorages;//static DWORD num_of_storages        = 0;
        DWORD dwIndexOfStorages;//static DWORD idx_of_storages        = 0;
        DWORD dwTotalNumberOfObjects;//static DWORD num_of_objects         = 0;
        DWORD dwIndexOfObjects;//static DWORD idx_of_objects         = 0;
        DWORD dwTotalNumberOfFolderObjects;//static DWORD num_of_folder_objects  = 0;
        DWORD dwIndexOfFolderObjects;//static DWORD idx_of_folder_objects  = 0;
        DWORD dwTotalNumberOfVideoObjects;//static DWORD num_of_video_objects   = 0;
        DWORD dwIndexOfVideoObjects;//static DWORD idx_of_video_objects   = 0;
        DWORD dwTotalNumberOfAudioObjects;//static DWORD num_of_audio_objects   = 0;
        DWORD dwIndexOfAudioObjects;//static DWORD idx_of_audio_objects   = 0;
        DWORD dwTotalNumberOfPictureObjects;//static DWORD num_of_picture_objects = 0;
        DWORD dwIndexOfPictureObjects;//static DWORD idx_of_picture_objects = 0;
        PUSBH_PTP_MAIL_TAG pReceiveMailFmFs;//static PUSBH_PTP_MAIL_TAG pReceiveMailFmFs;
        BOOL     boIsSearchFileInfo;//static BOOL     is_search_file_info = FALSE;
        DWORD    *pdwExtArray;//static DWORD    *pdwExtArray;
        DWORD    dwObjectCount;//static DWORD    dwCount = 0;
        DWORD    *pdwRecordElement;//static DWORD    *pdwRecordElement;
        BYTE     bBufIdx;//static BYTE index = 0;
    } stScanObject;

} USB_HOST_SIDC, *PUSB_HOST_SIDC;

#if 0
#if NETWARE_ENABLE
struct urb_wait_taskqueue_s {
	struct list_head task_list;
    struct mutex task_list_mutex;
};
typedef struct _USB_OTG_HOST_WIFI_
{
    int urb_sema;//int urb_sema = 0;
    int usb_sema;//int usb_sema = 1;
    _timer   usb_timer_head;//_timer   usb_timer_head;  // testing
    int      usb_timer_semaphore;//int      usb_timer_semaphore;   // for testing
    LM_LIST_CONTAINER pUrbOutList;//LM_LIST_CONTAINER pUrbOutList;
    LM_LIST_CONTAINER pUrbOutList2;//LM_LIST_CONTAINER pUrbOutList2;
    LM_LIST_CONTAINER pUrbInList;//LM_LIST_CONTAINER pUrbInList;
    LM_LIST_CONTAINER pUrbOutList_Int;//LM_LIST_CONTAINER pUrbOutList_Int;       /* USB Interrupt pipe */
    LM_LIST_CONTAINER pUrbInList_Int;//LM_LIST_CONTAINER pUrbInList_Int;       /* USB Interrupt pipe */
    LM_LIST_CONTAINER pUrbInCompleteList;//LM_LIST_CONTAINER pUrbInCompleteList;       
    BYTE wifi_device_type;//BYTE wifi_device_type = 3;
    //For wpa supplicant check device unplug to change interface
    BYTE wifi_device_unplug;//BYTE wifi_device_unplug = FALSE;
    
    struct urb_wait_taskqueue_s urb_wait_taskqueue;//static struct urb_wait_taskqueue_s urb_wait_taskqueue;
    qTD_Structure *st_spPreviousTempqTD;//static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;
    
    qTD_Structure *st_spFirstTempqTD;//static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;
    struct usb_device   dev_global;//struct usb_device   dev_global;
    struct usb_interface   intf_global;//static struct usb_interface   intf_global;
    
    struct usb_host_endpoint ep_buf[4+1];//static struct usb_host_endpoint ep_buf[4+1];
    
} USB_HOST_WIFI, *PUSB_HOST_WIFI;
#endif //NETWARE_ENABLE
typedef struct _USB_OTG_HOST_BT_
{
//    BYTE *pbBT_Buffer;//BYTE *gpBT_Buffer;
    BYTE *pbBT_Buffer1;//BYTE *gpBT_Buffer1;
    BYTE *pbBT_Buffer_Bulk_In;//BYTE *gpBT_Buffer_Bulk_In;
    BYTE *pbBT_Buffer_Bulk_Out;//BYTE *gpBT_Buffer_Bulk_Out;
    BYTE *pbBT_Buffer_Int_In;//BYTE *gpBT_Buffer_Int_In;
    BYTE *pbBT_Buffer;;//BYTE gBT_Buffer[BULK_IN_LENGTH*4];
    DWORD dwBT_USB_EVENT;//int g_BT_USB_EVENT = 0;
    BYTE bNext_Flow ;//BYTE bNext_Flow = 1;
    BYTE bReserved;
} USB_HOST_BT, *PUSB_HOST_BT;
#endif


typedef struct _USB_OTG_HOST_INF_W_ISOC_DES_
{
    PST_USB_OTG_HOST_ISOC_DES   pstIsocInDes;
    PST_USB_OTG_HOST_ISOC_DES   pstIsocOutDes;
    BYTE                        bBestIsocInDesIdx;
    BYTE                        bBestIsocOutDesIdx;
    BYTE                        bInterfaceNumber;
    BYTE                        bInterfaceClass;
    BYTE                        bTotalNumOfIsocInEp;
    BYTE                        bTotalNumOfIsocOutEp;
    BYTE                        bReserved[2];
} ST_USB_OTG_HOST_INF_W_ISOC_DES, *PST_USB_OTG_HOST_INF_W_ISOC_DES; 

typedef struct  {
     
   Periodic_Frame_List_Cell_Structure   sCell[Host20_Preiodic_Frame_List_MAX]; 

} PERIODIC_FRAME_LIST_STRUCTURE;

typedef struct _USB_OTG_HOST_EHCI_
{
    DWORD                       dwHostStructureBaseAddress;//DWORD gHost20_STRUCTURE_BASE_ADDRESS = 0;
    DWORD                       dwHostStructureQhdBaseAddress;//DWORD gHost20_STRUCTURE_qHD_BASE_ADDRESS = 0;
    DWORD                       dwHostStructureQtdBaseAddress;//DWORD gHost20_STRUCTURE_qTD_BASE_ADDRESS = 0;
    DWORD                       dwHostStructurePflBaseAddress;//DWORD gHost20_STRUCTURE_Preiodic_Frame_List_BASE_ADDRESS = 0;
#if USBOTG_HOST_ISOC
    DWORD                       dwHostStructureItdBaseAddress;//DWORD gHost20_STRUCTURE_iTD_BASE_ADDRESS = 0;
    DWORD                       dwHostDataPageBaseAddress;//DWORD gHost20_DATA_PAGE_BASE_ADDRESS = 0;
    OTGH_PT_ISO_STRUCT          stOtgHostIsoc;
    PST_USB_OTG_HOST_INF_W_ISOC_DES pstInterfaceByIsocDes;
#endif
    DWORD                       dwHostMemoryBaseAddress;//DWORD gUSB_OTG_HOST_MEMORY_BASE = 0;
    DWORD                       dwHostQhdQtdHandleBaseAddress;//DWORD gHost20_qHD_qTD_HANDLE_BASE_ADDRESS = 0;
    DWORD                       dwHostSidcRxBufferAddress;//DWORD gUSB_OTG_HOST_SIDC_RX_BUFFER_ADDRESS = 0;
    DWORD                       dwHostSidcGetObjecRxBufferAddress;//DWORD gUSB_OTG_HOST_SIDC_GET_OBJECT_RX_BUFFER_ADDRESS = 0;
    DWORD                       dwControlRemainBytes;//DWORD gControlRemainBytes = 0; // For Konica DSC
    DWORD                       dwHdTdBufferIndex;//DWORD g_HD_TD_buffer_index = 0;
    DWORD                       dwHdBaseBufferIndex;//DWORD g_HD_Base_buffer_index = 0;
    PERIODIC_FRAME_LIST_STRUCTURE  *psHostFramList;//Periodic_Frame_List_Structure  *psHost20_FramList;
    DWORD                           dwIntervalMap[11];//DWORD                           waIntervalMap[11]={1,2,4,8,16,32,64,128,256,512,1024};

    BYTE                        *pbHostQtdManage;//BYTE             Host20_qTD_Manage[Host20_qTD_MAX];        //1=>Free 2=>used
    BYTE                        *pbHostItdManage;//BYTE             Host20_iTD_Manage[Host20_iTD_MAX];        //1=>Free 2=>used  
    BYTE                        *pbHostDataPageManage;//Host20_DataPage_Manage[Host20_Page_MAX];  //1=>Free 2=>used  
    MEM_BLOCK                   sMem1024Bytes;//MEM_BLOCK gMem_1024_Bytes;
    MEM_BLOCK                   sMem512Bytes;//MEM_BLOCK gMem_512_Bytes;
    qTD_Structure               *psPreviousTempqTD;//static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;
    qTD_Structure               *psFirstTempqTD;//static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;
} USB_HOST_EHCI, *PUSB_HOST_EHCI;


typedef struct _USB_OTG_HOST_
{
    BOOL                        boIsAlreadyInit;
    BYTE                        bCurUsbhDeviceDescriptorIndex;  // MAX_NUM_OF_DEVICE
    BYTE                        bCurAppClassDescriptorIndex;    // MAX_NUM_OF_DEVICE
    BYTE                        bReserved[2];    // MAX_NUM_OF_DEVICE
    PST_USBH_DEVICE_DESCRIPTOR  psUsbhDeviceDescriptor;//PST_USBH_DEVICE_DESCRIPTOR  gpUsbhDeviceDescriptor;
    PST_APP_CLASS_DESCRIPTOR    psAppClassDescriptor; // Real-Memory-Location  //PST_APP_CLASS_DESCRIPTOR     gpAppClassDescriptor;
    PST_HUB_CLASS_DESCRIPTOR    psHubClassDescriptor;//PST_HUB_CLASS_DESCRIPTOR     gpHubClassDescriptor;
    ST_MCARD_MAIL               *psClassTaskEnvelope;//ST_MCARD_MAIL       *gpUsbOtgHostClassTaskEnvelope;
    ST_MCARD_MAIL               *psDriverTaskEnvelope;//ST_MCARD_MAIL       *gpUsbOtgHostDriverTaskEnvelope;
    ST_MCARD_MAIL               *psIsrEnvelope;//ST_MCARD_MAIL       *gpUsbOtgHostForIsrEnvelope;
    USB_HOST_EHCI               sEhci;

    BOOL                        boIsHostFinalized;//static BOOL gIsHostFinalized = FALSE;
    BOOL                        boIsHostToResetDevice;//static BOOL gIsHostToResetDevice = FALSE
    BOOL                        boIsReadyToReadWriteUsbh;//static BOOL gIsReadyToReadWriteUsbh = FALSE
    BOOL                        boIsUsbhContinueToResetDevice;//static BOOL gIsUsbhContinueToResetDevice = TRUE
    DWORD                       dwRetryCnt;//static DWORD g_RetryCnt = 0
    DWORD                       dwLastDriverEvent;//DWORD gLastDriverEvent = EVENT_DEVICE_PLUG_IN;
    DWORD                       dwChkCount;//static DWORD chk_count = 0;    
    BOOL                        boIocPolling;//BOOL g_ioc_polling = 0;
    struct
    {
        BYTE bEofRetryCnt;//static BYTE test_item = 0;
        BYTE bConfigIndex;//static BYTE                 config_idx = 0;
        BYTE bInterfaceIndex;//static BYTE                 interface_idx = 0;
        BYTE bReserved;
        BOOL boIsProcessedBiuReset;//static BOOL isProcessedBiuReset = FALSE;
        BOOL boIsHostToResetDevice;//static BOOL isHostToResetDevice = FALSE;
    } stSetupSm;
    struct
    {
        WORD wSetPortFeatureSelector;//static WORD                 set_port_feature_selector = 0;
        WORD wClearPortFeatureSelector;//static WORD                 clear_port_feature_selector = 0;
        WORD wPortNumber;//static WORD                 port_num = 1;
        BYTE bInitPortNum;//static BYTE                 bInitPortNUm = 1;
        BYTE bReserved;
    } stHub;
#if (USBOTG_HOST_CDC == ENABLE)
//cdc
    USB_HOST_CDC    sCdc;
#endif
//msdc
    USB_HOST_MSDC   sMsdc;
#if (USBOTG_WEB_CAM == ENABLE)
//avdc
    USB_HOST_AVDC   sWebcam;
    USB_HOST_UVC    sUvc;
#endif
//sidc
    USB_HOST_SIDC   sSidc;
//wifi
//#if (NETWARE_ENABLE == ENABLE)
//    USB_HOST_WIFI   sWifi;
//#endif
//bt
//    USB_HOST_BT     sBt;
#if USBOTG_HOST_DATANG
	USB_HOST_DATANG	sDatang;
#endif
#if (USBOTG_HOST_HID == ENABLE)
//hid
    USB_HOST_HID    sHid;
#endif
} ST_USB_OTG_HOST, *PST_USB_OTG_HOST; 


typedef struct _USB_OTG_DES_
{
    #if SC_USBDEVICE
    ST_USB_OTG_DEVICE   sUsbDev;
    #endif
    #if SC_USBHOST
    ST_USB_OTG_HOST     sUsbHost;
    #endif	
    PUSB_OTG            psUsbReg;
    DWORD               dwUsbBuf;//memory
    DWORD				timerId;
    BYTE                bUsbOtgArg[4]; // for the parametr of timer function
    USB_OTG_FUNC        eFunction;
} ST_USB_OTG_DES, *PST_USB_OTG_DES; 
/*********************************************************
///
///@ingroup USBOTG_INIT_API
///@brief   USB interrupt service routine.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///@remark  check the interrupt issued from host (BIT2), OTG (BIT1) or device (BIT0)
///controller to call the related function respectively.
///
void UsbIsr(WHICH_OTG eWhichOtg);
*********************************************************/

/*
typedef struct OTGC_STS
{
	OTGC_ID_Type                     ID;                       //0:A-Type   1: B-Type
    volatile OTGC_CurrentRole        CurrentRole;	           //1:Host  0:Peripheral
    BYTE                            bVBUSAlwaysOn;             //0:For Standard-ISR 1:For AP-ISR          

       
    DWORD                           wCurrentInterruptMask;
    volatile BYTE                   bVBUS_Vaild;        
    BYTE                            bHostISRType;             //0:For Standard-ISR 1:For AP-ISR          
    volatile BYTE                   bBusResetRequest;
    volatile BYTE                   bPortSuspendStatusChange;        
    volatile BYTE                   bPortEnableStatusChange;       
    volatile BYTE                   bPortConnectStatusChange;        
    volatile BYTE                   bDevice_RESET_Received;        
    volatile BYTE                   bDevice_RemoteWakeUp_Received;
                                     
    BYTE                            A_bGet_SRP_HNP_Support;
    volatile BYTE                   A_bASRPDET; 
    volatile BYTE                   B_bBSRPDN; 
                                     
    volatile BYTE                   A_BPLGRMV;        
    volatile BYTE                   A_APLGRMV; 
    volatile BYTE                   IDCHG;
	DWORD                           wOTGH_Interrupr_Save ;        
	DWORD                           wOTGP_Interrupr_Save ;        

} OTGC_STS;
*/
	
//===================4.Define Extern Function =====================================================================
//========================================================================================================
void UsbOtgControllerInit(WHICH_OTG eWhichOtg);
void UsbOtgHostCurDevDescSet(WHICH_OTG eWhichOtg, BYTE bIndex);
void UsbOtgHostCurAppClassDescSet(WHICH_OTG eWhichOtg, BYTE bIndex);
void UsbOtgHostDsInit(PST_USB_OTG_HOST psUsbHost);
void UsbOtgDeviceDsInit(WHICH_OTG eWhichOtg);
void UsbOtgDeviceReInit(WHICH_OTG eWhichOtg);
void UsbOtgBiuReset(WHICH_OTG eWhichOtg);
void UsbOtgCmpTimeSartFlagSet (BOOL flag, WHICH_OTG eWhichOtg);


BYTE UsbOtgHostMailBoxIdGet (WHICH_OTG eWhichOtg);
BYTE UsbOtgHostClassTaskIdGet (WHICH_OTG eWhichOtg);
BYTE UsbOtgHostDriverTaskIdGet (WHICH_OTG eWhichOtg);
BYTE UsbOtgHostDriverEventIdGet (WHICH_OTG eWhichOtg);
BYTE UsbOtgHostMsdcTxDoneEventIdGet (WHICH_OTG eWhichOtg);
BYTE UsbOtgDeviceTaskIdGet (WHICH_OTG eWhichOtg);
BYTE UsbOtgDeviceMessageIdGet (WHICH_OTG eWhichOtg);

DWORD UsbOtgIntCauseGet (WHICH_OTG eWhichOtg);
DWORD UsbOtgBufferGet (WHICH_OTG eWhichOtg);
DWORD UsbOtgHostConnectStatusGet(WHICH_OTG eWhichOtg);
DWORD UsbOtgHostDetectErrorCodeGet (WHICH_OTG eWhichOtg);

SDWORD UsbOtgCheckError(WHICH_OTG eWhichOtg);
SDWORD UsbOtgSetSysTimerProc (DWORD dwWaitTimeCnt, void *pFunc, BOOL isOneShot, WHICH_OTG eWhichOtg);

PUSB_HOST_EHCI UsbOtgHostEhciDsGet (WHICH_OTG eWhichOtg);
PUSB_HOST_CDC UsbOtgHostCdcDsGet (WHICH_OTG eWhichOtg);
PUSB_HOST_MSDC UsbOtgHostMsdcDsGet (WHICH_OTG eWhichOtg);
PUSB_HOST_SIDC UsbOtgHostSidcDsGet (WHICH_OTG eWhichOtg);

PUSB_DEVICE_DESC UsbOtgDevDescGet (WHICH_OTG eWhichOtg);
PUSB_DEVICE_MSDC UsbOtgDevMsdcGet (WHICH_OTG eWhichOtg);
PUSB_DEVICE_SIDC UsbOtgDevSidcGet (WHICH_OTG eWhichOtg);
PUSB_DEVICE_CDC UsbOtgDevCdcGet (WHICH_OTG eWhichOtg);
PPICT_BRIDGE_DPS UsbOtgDevSidcPtpDpsGet (WHICH_OTG eWhichOtg);

PST_USB_OTG_DES UsbOtgDesGet (WHICH_OTG eWhichOtg);
PST_USB_OTG_HOST UsbOtgHostDsGet (WHICH_OTG eWhichOtg);
PST_USB_OTG_DEVICE UsbOtgDevDsGet (WHICH_OTG eWhichOtg);

PST_USBH_DEVICE_DESCRIPTOR UsbOtgHostDevDescGet (WHICH_OTG eWhichOtg);
PST_APP_CLASS_DESCRIPTOR UsbOtgHostDevAppDescGet (WHICH_OTG eWhichOtg);
PST_HUB_CLASS_DESCRIPTOR UsbOtgHostDevHubDescGet (WHICH_OTG eWhichOtg);


#if (USBOTG_WEB_CAM == ENABLE)
PUSB_HOST_AVDC UsbOtgHostAvdcDsGet (WHICH_OTG eWhichOtg);
#endif

#if SC_USBDEVICE    
BOOL UsbOtgDeviceCheckIdPin(WHICH_OTG eWhichOtg);
#endif

#endif //(SC_USBHOST|SC_USBDEVICE)

#endif //__USBOTG_CTRL_H__

