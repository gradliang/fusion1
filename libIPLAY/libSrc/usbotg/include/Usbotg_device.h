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
* Filename		: usbotg_device.h
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)
* Created Date	: 2007/07/26 
* Description:  1.EHCI Data Structure
*               2.EHCI Register
*               3.Others
******************************************************************************** 
*/
#ifndef __USBOTG_DEVICE_H__
#define __USBOTG_DEVICE_H__ 
#include "UtilTypeDef.h"
#include "UtilRegFile.h"
#include "iplaysysconfig.h"
#include "usbotg_api.h"
#include "usbotg_ctrl.h"


//#if SC_USBDEVICE

#define USBOTG_DEVICE_ISO_TEST      DISABLE
#define wFOTGPeri_Port(bOffset) (*((volatile DWORD *)(((DWORD)psUsbOtg->psUsbReg) | (bOffset))))

#define mUsbOtgRmWkupST()			(wFOTGPeri_Port(0x100) & BIT0)
#define mUsbOtgRmWkupSet()		    (wFOTGPeri_Port(0x100) |= BIT0)
#define mUsbOtgRmWkupClr()			(wFOTGPeri_Port(0x100) &= ~BIT0)

#define mUsbOtgTstHalfSpeedEn()		(wFOTGPeri_Port(0x100) |= BIT1)
#define mUsbOtgTstHalfSpeedDis()	(wFOTGPeri_Port(0x100) &= ~BIT1)

#define mUsbOtgGlobIntEnRd()		(wFOTGPeri_Port(0x100) & BIT2)
#define mUsbOtgGlobIntEnSet()		(wFOTGPeri_Port(0x100) |= BIT2)
#define mUsbOtgGlobIntDis()			(wFOTGPeri_Port(0x100) &= ~BIT2)

#define mUsbOtgGoSuspend()			(wFOTGPeri_Port(0x100) |=  BIT3)

#define mUsbOtgSoftRstSet()			(wFOTGPeri_Port(0x100) |=  BIT4)
#define mUsbOtgSoftRstClr()			(wFOTGPeri_Port(0x100) &= ~BIT4)

#define mUsbOtgChipEnSet()			(wFOTGPeri_Port(0x100) |= BIT5)
#define mUsbOtgChipEnClr()                   (wFOTGPeri_Port(0x100) &= ~BIT5)

#define mUsbOtgHighSpeedST()		(wFOTGPeri_Port(0x100) & BIT6)
#define mUsbOtgForceSpeedSet()	    (wFOTGPeri_Port(0x100) |=  BIT9)
#define mUsbOtgForceSpeedClr()	    (wFOTGPeri_Port(0x100) &=  ~BIT9)

// Device address register(0x104)
#define mUsbOtgDevAddrSet(Value)	(wFOTGPeri_Port(0x104) = (DWORD)Value)
#define mUsbOtgCfgST()				(wFOTGPeri_Port(0x104) & BIT7)
#define mUsbOtgCfgSet()				(wFOTGPeri_Port(0x104) |= BIT7)
#define mUsbOtgCfgClr()				(wFOTGPeri_Port(0x104) &= ~BIT7)
#define mUsbOtgDMARst()             (wFOTGPeri_Port(0x100) |= BIT8)
//#define mUsbOtgEPMapRd(EPn)        (*(&g_psUsb->ep1_map + (EPn-1)))

#define mUsbOtgFIFOClr(fifo_num)    (wFOTGPeri_Port(0x1b0+fifo_num*4) |= BIT12) // (wFOTGPeri_Port(0x1b0+fifo_num*4) |= BIT12)

// Test register(0x108)
#define mUsbOtgClrAllFIFOSet()		    (wFOTGPeri_Port(0x108) |= BIT0)
#define mUsbOtgClrAllFIFOClr()		    (wFOTGPeri_Port(0x108) &= ~BIT0)
#define mUsbOtgLoopBackTestSet()	    (wFOTGPeri_Port(0x108) |= BIT1)
#define mUsbOtgLoopBackTestClr()	    (wFOTGPeri_Port(0x108) &= ~BIT1)
#define mUsbOtgExternalSideAddressClr()	(wFOTGPeri_Port(0x108) = BIT2)

// SOF Frame Number register(0x10C)
#define mUsbOtgFrameNo()			(WORD)(wFOTGPeri_Port(0x10C) & 0x7FF)
#define mUsbOtgMicroFrameNo()		(BYTE)((wFOTGPeri_Port(0x10C) & 0x3800)>>11)

// SOF Mask register(0x110)
#define mUsbOtgSOFMaskHS()		(wFOTGPeri_Port(0x110) = 0x44c)
#define mUsbOtgSOFMaskFS()		(wFOTGPeri_Port(0x110) = 0x2710)

// PHY Test Mode Selector register(0x114)
#define mUsbOtgTsMdWr(item)		(wFOTGPeri_Port(0x114) = (DWORD)item)
#define mUsbOtgUnPLGClr()		(wFOTGPeri_Port(0x114) &= ~BIT0)
#define mUsbOtgUnPLGSet()		(wFOTGPeri_Port(0x114) |= BIT0)
// Vendor Specific IO Control register(0x118)

// Cx configuration and status register(0x11C)

// Cx configuration and FIFO Empty Status register(0x120)
#define mUsbOtgEP0DoneSet()		(wFOTGPeri_Port(0x120) |= BIT0)
#define mUsbOtgEP0DoneSetRd()	(wFOTGPeri_Port(0x120) & BIT0)
#define mUsbOtgEP0DoneClr()		(wFOTGPeri_Port(0x120) &= ~BIT0)

#define mUsbOtgTsPkDoneSet()	(wFOTGPeri_Port(0x120) |= BIT1)
#define mUsbOtgEP0StallSet()	(wFOTGPeri_Port(0x120) |= BIT2)
#define mUsbOtgCxFClr()			(wFOTGPeri_Port(0x120) |= BIT3)

#define mUsbOtgCxFFull()	    (wFOTGPeri_Port(0x120) & BIT4)
#define mUsbOtgCxFEmpty()		(wFOTGPeri_Port(0x120) & BIT5)
#define mUsbOtgCxFByteCnt()		(BYTE)((wFOTGPeri_Port(0x120) & 0x7F000000)>>24)

// IDLE Counter register(0x124)
#define mUsbOtgIdleCnt(time)		(wFOTGPeri_Port(0x124) = (DWORD)time)

// Mask of interrupt group(0x130)
#define mUsbOtgIntGrp0Dis()			(wFOTGPeri_Port(0x130) |= BIT0)
#define mUsbOtgIntGrp1Dis()			(wFOTGPeri_Port(0x130) |= BIT1)
#define mUsbOtgIntGrp2Dis()			(wFOTGPeri_Port(0x130) |= BIT2)

#define mUsbOtgIntGroupMaskRd()	    (wFOTGPeri_Port(0x130))

// Mask of interrupt source group 0(0x134)
#define mUsbOtgIntEP0SetupDis()		(wFOTGPeri_Port(0x134) |= BIT0)
#define mUsbOtgIntEP0InDis()		(wFOTGPeri_Port(0x134) |= BIT1)
#define mUsbOtgIntEP0OutDis()		(wFOTGPeri_Port(0x134) |= BIT2)
#define mUsbOtgIntEP0EndDis()		(wFOTGPeri_Port(0x134) |= BIT3)
#define mUsbOtgIntEP0FailDis()		(wFOTGPeri_Port(0x134) |= BIT4)

#define mUsbOtgIntEP0SetupEn()		(wFOTGPeri_Port(0x134) &= ~(BIT0))
#define mUsbOtgIntEP0InEn()			(wFOTGPeri_Port(0x134) &= ~(BIT1))
#define mUsbOtgIntEP0OutEn()		(wFOTGPeri_Port(0x134) &= ~(BIT2))
#define mUsbOtgIntEP0EndEn()		(wFOTGPeri_Port(0x134) &= ~(BIT3))
#define mUsbOtgIntEP0FailEn()		(wFOTGPeri_Port(0x134) &= ~(BIT4))

#define mUsbOtgIntSrc0MaskRd()		(wFOTGPeri_Port(0x134))

// Mask of interrupt source group 1(0x138)
#define mUsbOtgIntFIFO0_3OUTDis()	(wFOTGPeri_Port(0x138) |= 0xFF)
#define mUsbOtgIntFIFO0_3INDis()	(wFOTGPeri_Port(0x138) |= 0xF0000)

#define mUsbOtgIntF0OUTEn()		    (wFOTGPeri_Port(0x138) &= ~(BIT1 | BIT0))	
#define mUsbOtgIntF0OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT1 | BIT0))	
#define mUsbOtgIntF1OUTEn()		    (wFOTGPeri_Port(0x138) &= ~(BIT3 | BIT2))
#define mUsbOtgIntF1OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT3 | BIT2))
#define mUsbOtgIntF2OUTEn()		    (wFOTGPeri_Port(0x138) &= ~(BIT5 | BIT4))
#define mUsbOtgIntF2OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT5 | BIT4))
#define mUsbOtgIntF3OUTEn()		    (wFOTGPeri_Port(0x138) &= ~(BIT7 | BIT6))
#define mUsbOtgIntF3OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT7 | BIT6))

#define mUsbOtgIntF0INEn()			(wFOTGPeri_Port(0x138) &= ~BIT16)
#define mUsbOtgIntF0INDis()			(wFOTGPeri_Port(0x138) |= BIT16)
#define mUsbOtgIntF2INEn()			(wFOTGPeri_Port(0x138) &= ~BIT18)
#define mUsbOtgIntF2INDis()			(wFOTGPeri_Port(0x138) |= BIT18)

#define mUsbOtgIntSrc1MaskRd()		(wFOTGPeri_Port(0x138))

// Mask of interrupt source group 2(DMA int mask)(0x13C)
#define mUsbOtgIntSuspDis()			(wFOTGPeri_Port(0x13C) |= BIT1)		
#define mUsbOtgIntDmaErrDis()		(wFOTGPeri_Port(0x13C) |= BIT8)
#define mUsbOtgIntDmaFinishDis()	(wFOTGPeri_Port(0x13C) |= BIT7)
#define mUsbOtgIntDevIdleDis()            (wFOTGPeri_Port(0x13C) |= BIT9)
#define mUsbOtgIntWakeupDis()            (wFOTGPeri_Port(0x13C) |=BIT10)

#define mUsbOtgIntSuspEn()			(wFOTGPeri_Port(0x13C) &= ~(BIT1))		
#define mUsbOtgIntDmaErrEn()		(wFOTGPeri_Port(0x13C) &= ~(BIT8))
#define mUsbOtgIntDmaFinishEn()		(wFOTGPeri_Port(0x13C) &= ~(BIT7))

#define mUsbOtgIntSrc2MaskRd()		(wFOTGPeri_Port(0x13C))

// Interrupt group (0x140)
#define mUsbOtgIntGroupRegRd()		    (wFOTGPeri_Port(0x140))
#define mUsbOtgIntGroupRegSet(wValue)	(wFOTGPeri_Port(0x140) |= wValue)

// Interrupt source group 0(0x144)
#define mUsbOtgIntSrc0Rd()			(wFOTGPeri_Port(0x144))	
#define mUsbOtgIntEP0AbortClr()		(wFOTGPeri_Port(0x144) &= ~(BIT5))	
#define mUsbOtgIntSrc0Clr()			(wFOTGPeri_Port(0x144) = 0)
#define mUsbOtgIntSrc0Set(wValue)	(wFOTGPeri_Port(0x144) |= wValue)

// Interrupt source group 1(0x148)
#define mUsbOtgIntSrc1Rd()			(wFOTGPeri_Port(0x148))
#define mUsbOtgIntSrc1Set(wValue)	(wFOTGPeri_Port(0x148) |= wValue)

// Interrupt source group 2(0x14C)
#define mUsbOtgIntSrc2Rd()			(wFOTGPeri_Port(0x14C))
#define mUsbOtgIntSrc2Set(wValue)	(wFOTGPeri_Port(0x14C) |= wValue)


#define mUsbOtgIntBusRstClr()		(wFOTGPeri_Port(0x14C) &= ~BIT0)		
#define mUsbOtgIntSuspClr()			(wFOTGPeri_Port(0x14C) &= ~BIT1)		
#define mUsbOtgIntResmClr()			(wFOTGPeri_Port(0x14C) &= ~BIT2)		
#define mUsbOtgIntIsoSeqErrClr()	(wFOTGPeri_Port(0x14C) &= ~BIT3)			
#define mUsbOtgIntIsoSeqAbortClr()	(wFOTGPeri_Port(0x14C) &= ~BIT4)			
#define mUsbOtgIntTX0ByteClr()		(wFOTGPeri_Port(0x14C) &= ~BIT5)			
#define mUsbOtgIntRX0ByteClr()		(wFOTGPeri_Port(0x14C) &= ~BIT6)			
#define mUsbOtgIntDmaFinishClr()	(wFOTGPeri_Port(0x14C) &= ~BIT7)			
#define mUsbOtgIntDmaErrClr()		(wFOTGPeri_Port(0x14C) &= ~BIT8)			
#define mUsbOtgIntDmaFinishRd()	    (wFOTGPeri_Port(0x14C) &BIT7)			

#define mUsbOtgIntDmaFinish()		(wFOTGPeri_Port(0x14C) & BIT7)			
#define mUsbOtgIntDmaErr()			(wFOTGPeri_Port(0x14C) & BIT8)			

// Rx 0 byte packet register(0x150)
#define mUsbOtgIntRX0ByteRd()		    (BYTE)(wFOTGPeri_Port(0x150))
#define mUsbOtgIntRX0ByteSetClr(set)    (wFOTGPeri_Port(0x150) &= ~((DWORD)set))

// Tx 0 byte packet register(0x154)
#define mUsbOtgIntTX0ByteRd()		         (BYTE)(wFOTGPeri_Port(0x154))
#define mUsbOtgIntTX0ByteSetClr(data)		       (wFOTGPeri_Port(0x154) &= ~((DWORD)data))

// ISO sequential Error/Abort register(0x158)
#define mUsbOtgIntIsoSeqErrRd()		        (BYTE)((wFOTGPeri_Port(0x158) & 0xff0000)>>16)
#define mUsbOtgIntIsoSeqErrSetClr(data)		       (wFOTGPeri_Port(0x158) &= ~(((DWORD)data)<<16))

#define mUsbOtgIntIsoSeqAbortRd()	         (BYTE)(wFOTGPeri_Port(0x158) & 0xff)
#define mUsbOtgIntIsoSeqAbortSetClr(data)	       (wFOTGPeri_Port(0x158) &= ~((DWORD)data))

//#define mUsbOtgFIFOClr(fifo_num)   (wFOTGPeri_Port(0x1b0+fifo_num*4) |= BIT12) // (wFOTGPeri_Port(0x1b0+fifo_num*4) |= BIT12)
// IN Endpoint MaxPacketSize register(0x160,0x164,...,0x17C)
#define mUsbOtgEPinHighBandSet(EPn, dir , size )	(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) &= ~(BIT14 |BIT13));  //(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) |= ((((BYTE)(size >> 11)+1) << 13)*(1 - dir)) )
#define mUsbOtgEPMxPtSz(EPn, dir, size)		        (wFOTGPeri_Port(0x160 + (dir * 0x20) + ((EPn - 1) << 2)) = (WORD)(size))
#define mUsbOtgEPMxPtSzClr(EPn, dir)			    (wFOTGPeri_Port(0x160 + (dir * 0x20) + ((EPn - 1) << 2)) = 0)

#define mUsbOtgEPinMxPtSz(EPn)		(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) & 0x7ff)
#define mUsbOtgEPinStallST(EPn)	   ((wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) & BIT11) >> 11)
#define mUsbOtgEPinStallClr(EPn)	(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) &= ~BIT11)
#define mUsbOtgEPinStallSet(EPn)	(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) |=  BIT11)
#define mUsbOtgEPinRsTgClr(EPn)		(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) &= ~BIT12)
#define mUsbOtgEPinRsTgSet(EPn)	    (wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) |=  BIT12)

// OUT Endpoint MaxPacketSize register(0x180,0x164,...,0x19C)
#define mUsbOtgEPoutMxPtSz(EPn)	    ((wFOTGPeri_Port(0x180 + ((EPn - 1) << 2))) & 0x7ff)
#define mUsbOtgEPoutStallST(EPn)	((wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) & BIT11) >> 11)
#define mUsbOtgEPoutStallClr(EPn)	 (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) &= ~BIT11)
#define mUsbOtgEPoutStallSet(EPn)	 (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) |=  BIT11)
#define mUsbOtgEPoutRsTgClr(EPn)	 (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) &= ~BIT12)
#define mUsbOtgEPoutRsTgSet(EPn)	 (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) |=  BIT12)

// Endpoint & FIFO Configuration
// Endpoint 1~4 Map register(0x1a0), Endpoint 5~8 Map register(0x1a4)
#define mUsbOtgEPMap(EPn, MAP)	    (wFOTGPeri_Port(0x1a0) |= (MAP<<(8*(EPn-1))))//(bFOTGPeri_Port(0x1a0 + (EPn-1)) = MAP)
#define mUsbOtgEPMapRd(EPn)		    ((wFOTGPeri_Port(0x1a0)&(0xff<<(8*(EPn-1)))) >> (8*(EPn-1)))//(bFOTGPeri_Port(0x1a0+ (EPn-1)))
#define mUsbOtgEPMapAllClr()		(wFOTGPeri_Port(0x1a0) = 0);(wFOTGPeri_Port(0x1a4) = 0)

// FIFO Map register(0x1a8)
#define mUsbOtgFIFOMap(FIFOn, MAP)	(wFOTGPeri_Port(0x1a8) |= (MAP<<(8*FIFOn)))//(bFOTGPeri_Port(0x1a8 + FIFOn) = MAP)
#define mUsbOtgFIFOMapRd(FIFOn)		((wFOTGPeri_Port(0x1a8)&(0xff<<(8*FIFOn))) >> (8*FIFOn))//(bFOTGPeri_Port(0x1a8 + FIFOn))
#define mUsbOtgFIFOMapAllClr()		(wFOTGPeri_Port(0x1a8) = 0)

// FIFO Configuration register(0x1ac)
#define mUsbOtgFIFOConfig(FIFOn, CONFIG)	(wFOTGPeri_Port(0x1ac) |= (CONFIG<<(8*FIFOn)))//(bFOTGPeri_Port(0x1ac + FIFOn) = CONFIG)
#define mUsbOtgFIFOConfigRd(FIFOn)			((wFOTGPeri_Port(0x1ac)&(0xff<<(8*FIFOn))) >> (8*FIFOn))//(bFOTGPeri_Port(0x1ac + FIFOn))
#define mUsbOtgFIFOConfigAllClr()		    (wFOTGPeri_Port(0x1ac) = 0)
#define FIFOEnBit					        0x20

// FIFO byte count register(0x1b0)
#define mUsbOtgFIFOOutByteCount(fifo_num)	(((wFOTGPeri_Port(0x1b0+fifo_num*4)&0x7ff)))
#define mUsbOtgFIFODone(fifo_num)			(wFOTGPeri_Port(0x1b0+fifo_num*4) |= BIT11)

// DMA target FIFO register(0x1c0)
#define FOTG200_DMA2FIFO_Non 		0
#define FOTG200_DMA2FIFO0 			BIT0
#define FOTG200_DMA2FIFO1 			BIT1
#define FOTG200_DMA2FIFO2 			BIT2
#define FOTG200_DMA2FIFO3 			BIT3
#define FOTG200_DMA2CxFIFO 		    BIT4

#define mUsbOtgDMA2FIFOSel(sel)		(wFOTGPeri_Port(0x1c0) = sel)
#define mUsbOtgDMA2FIFORd()			(wFOTGPeri_Port(0x1c0))

// DMA parameter set 1 (0x1c8)	
#define mUsbOtgDmaConfig(len,Dir)		(wFOTGPeri_Port(0x1c8) = (((DWORD)len)<<8)|((1-Dir)<<1))
//#define mUsbOtgDmaLenRd()				((wFOTGPeri_Port(0x1c8) & 0x1fff0000) >> 8)
#define mUsbOtgDmaLenRd()				((wFOTGPeri_Port(0x1c8) & 0x00FFFFFF00) >> 8)	
#define mUsbOtgDmaConfigRd()			(wFOTGPeri_Port(0x1c8))
#define mUsbOtgDmaConfigSet(set)		(wFOTGPeri_Port(0x1c8) = set)

#define mUsbOtgDmaStart()				(wFOTGPeri_Port(0x1c8) |= BIT0)
#define mUsbOtgDmaStop()				(wFOTGPeri_Port(0x1c8) &= ~BIT0)

// DMA parameter set 2 (0x1cc)	
#define mUsbOtgDmaAddr(addr)			(wFOTGPeri_Port(0x1cc) = (unsigned int)addr)
#define mUsbOtgDmaAddrRd()			    (wFOTGPeri_Port(0x1cc))

// 8 byte command data port(0x1d0)
#define mUsbOtgEP0CmdDataRdDWord()	    (wFOTGPeri_Port(0x1d0))


//Global Controller Register
//Mask of HC/OTG/DEV interrupt  (address = C4h)
#define mUsbGC_MHC_INT_Dis()                    (wFOTGPeri_Port(0xC4) |= BIT2);
#define mUsbGC_MHC_INT_Ena()                   (wFOTGPeri_Port(0xC4) &= ~BIT2);
#define mUsbGC_MOTG_INT_Dis()                  (wFOTGPeri_Port(0xC4) |= BIT1);
#define mUsbGC_MOTG_INT_Ena()                 (wFOTGPeri_Port(0xC4) &= ~BIT1);
#define mUsbGC_MDEV_INT_Dis()                  (wFOTGPeri_Port(0xC4) |= BIT0);
#define mUsbGC_MDEV_INT_Ena()                 (wFOTGPeri_Port(0xC4) &= ~BIT0);

/*
// Constant declarations
*/
#define EP0MAXPACKETSIZE    0x40

// FIFO number define
#define FIFO0	0x0
#define FIFO1	0x1
#define FIFO14	0xE

#define FIFO_BULK_IN    FIFO0
#define FIFO_BULK_OUT	FIFO1
#define FIFO_INT_IN     FIFO14


// Endpoint number define
#define EP0        0x00
#define EP1        0x01
#define EP2        0x02
#define EP3        0x03

#define TEST_J          0x02
#define TEST_K          0x04
#define TEST_SE0_NAK    0x08
#define TEST_PKY        0x10

// Descriptor Table uses the following parameters : fixed
#define DEVICE_LENGTH               0x12
#define CONFIG_LENGTH               0x09
#define INTERFACE_LENGTH            0x09
#define EP_LENGTH                   0x07
#define DEVICE_QUALIFIER_LENGTH     0x0A

#define MANUFACTURER_SRING_LEN         0x16
#define PRODUCT_SRING_LEN              0x32
#define SERIALNUMBER_SRING_LEN         0x0E

#define EP_NUMBER                   3
#define CONFIG_TOTAL_LENGTH     (CONFIG_LENGTH + INTERFACE_LENGTH + EP_LENGTH * EP_NUMBER)

#define NUMBER_OF_CONFIGURATION 0x01
#define NUMBER_OF_INTERFACE     1
#define INTERFACE_NUMBER        0 

#define GET_MAX_LUN         0xFE

#define USB_CDC_SPEED 0x0001c200 // 115200

#define MESS_ERROR      (0x01 << 0)
#define MESS_WARNING    (0x01 << 1)
#define MESS_INFO       (0x01 << 2)

#define HS_MAX_PACKET_SIZE      0x0200 //0x0040 // JL, 02202006, for XP PtpFullTestTool
#define FS_MAX_PACKET_SIZE      0x0040
#define INT_MAX_PACKET_SIZE     0x0040
#define HS_INT_EP_INTERVAL      0x10
#define FS_INT_EP_INTERVAL      0x60 // 1 // for Canon Printer, not to use 1ms
#define EP_INTERVAL             0

// Block Size define
#define BLK512BYTE      1
#define BLK1024BYTE     2

#define BLK64BYTE       1
#define BLK128BYTE      2

// Block toggle number define
#define SINGLE_BLK      1
#define DOUBLE_BLK      2
#define TRIBLE_BLK      3

// Endpoint transfer type
#define TF_TYPE_ISOCHRONOUS     1
#define TF_TYPE_BULK            2
#define TF_TYPE_INTERRUPT       3

// Endpoint or FIFO direction define
#define DIRECTION_IN    0
#define DIRECTION_OUT   1

#if USBOTG_DEVICE_ISO_TEST
#define MASK_F0             0xF0
#define HS_EP1_BLKNO        SINGLE_BLK      // 1
#define HS_EP1_TYPE         TF_TYPE_ISOCHRONOUS    // 2
#define HS_EP1_BLKSIZE      BLK1024BYTE      // 1
#define HS_EP1_MAX_PACKET   HS_MAX_PACKET_SIZE// 0x0200 //0x0040 // JL, 03302005, test
#define HS_EP1_DIRECTION    DIRECTION_IN    // 0
#define HS_EP1_FIFO_START   FIFO0           // 0
#define HS_EP1_FIFO_NO      (HS_EP1_BLKNO * HS_EP1_BLKSIZE) // 1*1=1 
#define HS_EP1_MAP          (HS_EP1_FIFO_START | (HS_EP1_FIFO_START << 4) | (MASK_F0 >> (4*HS_EP1_DIRECTION)))
#define HS_EP1_FIFO_MAP     (((1 - HS_EP1_DIRECTION) << 4) | EP1)                
#define HS_EP1_FIFO_CONFIG	(0x80 | ((HS_EP1_BLKSIZE - 1) << 4) | ((HS_EP1_BLKNO - 1) << 2) | HS_EP1_TYPE)

#define HS_EP2_BLKNO        SINGLE_BLK      // 1
#define HS_EP2_TYPE         TF_TYPE_ISOCHRONOUS    // 2
#define HS_EP2_BLKSIZE      BLK1024BYTE      // 1
#define HS_EP2_MAX_PACKET   HS_MAX_PACKET_SIZE //0x0200	//0x0040 // JL, 03302005, test
#define HS_EP2_DIRECTION    DIRECTION_OUT   // 1
#define HS_EP2_FIFO_START   (HS_EP1_FIFO_START + HS_EP1_FIFO_NO) // 0+1=1
#define HS_EP2_FIFO_NO      (HS_EP2_BLKNO * HS_EP2_BLKSIZE)      // 1*1=1
#define HS_EP2_MAP          (HS_EP2_FIFO_START | (HS_EP2_FIFO_START << 4) | (MASK_F0 >> (4*HS_EP2_DIRECTION)))
#define HS_EP2_FIFO_MAP     (((1 - HS_EP2_DIRECTION) << 4) | EP2) // ((1-0)<<4)|2=0x12               
#define HS_EP2_FIFO_CONFIG	(0x80 | ((HS_EP2_BLKSIZE - 1) << 4) | ((HS_EP2_BLKNO - 1) << 2) | HS_EP2_TYPE)

#define FS_EP1_BLKNO        SINGLE_BLK      // 1
#define FS_EP1_TYPE         TF_TYPE_ISOCHRONOUS    // 2
#define FS_EP1_BLKSIZE      BLK64BYTE       // 1
#define FS_EP1_MAX_PACKET   FS_MAX_PACKET_SIZE //0x0040
#define FS_EP1_DIRECTION    DIRECTION_IN    // 0
#define FS_EP1_FIFO_START   FIFO0           // 0
#define FS_EP1_FIFO_NO      (FS_EP1_BLKNO * FS_EP1_BLKSIZE)
#define FS_EP1_MAP          (FS_EP1_FIFO_START | (FS_EP1_FIFO_START << 4) | (MASK_F0 >> (4*FS_EP1_DIRECTION)))
#define FS_EP1_FIFO_MAP     (((1 - FS_EP1_DIRECTION) << 4) | EP1)                
#define FS_EP1_FIFO_CONFIG	(0x80 | ((FS_EP1_BLKSIZE - 1) << 4) | ((FS_EP1_BLKNO - 1) << 2) | FS_EP1_TYPE)

#define FS_EP2_BLKNO        SINGLE_BLK      // 1
#define FS_EP2_TYPE         TF_TYPE_ISOCHRONOUS    // 2
#define FS_EP2_BLKSIZE      BLK64BYTE      // 1
#define FS_EP2_MAX_PACKET   FS_MAX_PACKET_SIZE //0x0040
#define FS_EP2_DIRECTION    DIRECTION_OUT   // 0
#define FS_EP2_FIFO_START   (FS_EP1_FIFO_START + FS_EP1_FIFO_NO)
#define FS_EP2_FIFO_NO      (FS_EP2_BLKNO * FS_EP2_BLKSIZE)
#define FS_EP2_MAP          (FS_EP2_FIFO_START | (FS_EP2_FIFO_START << 4) | (MASK_F0 >> (4*FS_EP2_DIRECTION)))
#define FS_EP2_FIFO_MAP     (((1 - FS_EP2_DIRECTION) << 4) | EP2)                
#define FS_EP2_FIFO_CONFIG	(0x80 | ((FS_EP2_BLKSIZE - 1) << 4) | ((FS_EP2_BLKNO - 1) << 2) | FS_EP2_TYPE)

#define EP3_BLKNO           SINGLE_BLK          // 1
#define EP3_TYPE            TF_TYPE_INTERRUPT   // 3
#define EP3_BLKSIZE         BLK64BYTE           // 1
#define EP3_MAX_PACKET      0x0040
#define EP3_DIRECTION       DIRECTION_IN        // 0
#define EP3_FIFO_START      FIFO14              // 0xE
#define EP3_FIFO_NO         EP3_BLKSIZE         // 1
#define EP3_MAP             (EP3_FIFO_START | (EP3_FIFO_START << 4) | (MASK_F0 >> (4*EP3_DIRECTION)))
#define EP3_FIFO_MAP        (((1 - EP3_DIRECTION) << 4) | EP3) // ((1-0)<<4)|3=0x13               
#define EP3_FIFO_CONFIG	    (0x80 | ((EP3_BLKSIZE - 1) << 4) | ((EP3_BLKNO - 1) << 2) | EP3_TYPE)
#else
#define MASK_F0             0xF0
#define HS_EP1_BLKNO        SINGLE_BLK      // 1
#define HS_EP1_TYPE         TF_TYPE_BULK    // 2
#define HS_EP1_BLKSIZE      BLK512BYTE      // 1
#define HS_EP1_MAX_PACKET   HS_MAX_PACKET_SIZE// 0x0200 //0x0040 // JL, 03302005, test
#define HS_EP1_DIRECTION    DIRECTION_IN    // 0
#define HS_EP1_FIFO_START   FIFO0           // 0
#define HS_EP1_FIFO_NO      (HS_EP1_BLKNO * HS_EP1_BLKSIZE) // 1*1=1 
#define HS_EP1_MAP          (HS_EP1_FIFO_START | (HS_EP1_FIFO_START << 4) | (MASK_F0 >> (4*HS_EP1_DIRECTION)))
#define HS_EP1_FIFO_MAP     (((1 - HS_EP1_DIRECTION) << 4) | EP1)                
#define HS_EP1_FIFO_CONFIG	(0x80 | ((HS_EP1_BLKSIZE - 1) << 4) | ((HS_EP1_BLKNO - 1) << 2) | HS_EP1_TYPE)

#define HS_EP2_BLKNO        SINGLE_BLK      // 1
#define HS_EP2_TYPE         TF_TYPE_BULK    // 2
#define HS_EP2_BLKSIZE      BLK512BYTE      // 1
#define HS_EP2_MAX_PACKET   HS_MAX_PACKET_SIZE //0x0200	//0x0040 // JL, 03302005, test
#define HS_EP2_DIRECTION    DIRECTION_OUT   // 1
#define HS_EP2_FIFO_START   (HS_EP1_FIFO_START + HS_EP1_FIFO_NO) // 0+1=1
#define HS_EP2_FIFO_NO      (HS_EP2_BLKNO * HS_EP2_BLKSIZE)      // 1*1=1
#define HS_EP2_MAP          (HS_EP2_FIFO_START | (HS_EP2_FIFO_START << 4) | (MASK_F0 >> (4*HS_EP2_DIRECTION)))
#define HS_EP2_FIFO_MAP     (((1 - HS_EP2_DIRECTION) << 4) | EP2) // ((1-0)<<4)|2=0x12               
#define HS_EP2_FIFO_CONFIG	(0x80 | ((HS_EP2_BLKSIZE - 1) << 4) | ((HS_EP2_BLKNO - 1) << 2) | HS_EP2_TYPE)

#define FS_EP1_BLKNO        SINGLE_BLK      // 1
#define FS_EP1_TYPE         TF_TYPE_BULK    // 2
#define FS_EP1_BLKSIZE      BLK64BYTE       // 1
#define FS_EP1_MAX_PACKET   FS_MAX_PACKET_SIZE //0x0040
#define FS_EP1_DIRECTION    DIRECTION_IN    // 0
#define FS_EP1_FIFO_START   FIFO0           // 0
#define FS_EP1_FIFO_NO      (FS_EP1_BLKNO * FS_EP1_BLKSIZE)
#define FS_EP1_MAP          (FS_EP1_FIFO_START | (FS_EP1_FIFO_START << 4) | (MASK_F0 >> (4*FS_EP1_DIRECTION)))
#define FS_EP1_FIFO_MAP     (((1 - FS_EP1_DIRECTION) << 4) | EP1)                
#define FS_EP1_FIFO_CONFIG	(0x80 | ((FS_EP1_BLKSIZE - 1) << 4) | ((FS_EP1_BLKNO - 1) << 2) | FS_EP1_TYPE)

#define FS_EP2_BLKNO        SINGLE_BLK      // 1
#define FS_EP2_TYPE         TF_TYPE_BULK    // 2
#define FS_EP2_BLKSIZE      BLK64BYTE      // 1
#define FS_EP2_MAX_PACKET   FS_MAX_PACKET_SIZE //0x0040
#define FS_EP2_DIRECTION    DIRECTION_OUT   // 0
#define FS_EP2_FIFO_START   (FS_EP1_FIFO_START + FS_EP1_FIFO_NO)
#define FS_EP2_FIFO_NO      (FS_EP2_BLKNO * FS_EP2_BLKSIZE)
#define FS_EP2_MAP          (FS_EP2_FIFO_START | (FS_EP2_FIFO_START << 4) | (MASK_F0 >> (4*FS_EP2_DIRECTION)))
#define FS_EP2_FIFO_MAP     (((1 - FS_EP2_DIRECTION) << 4) | EP2)                
#define FS_EP2_FIFO_CONFIG	(0x80 | ((FS_EP2_BLKSIZE - 1) << 4) | ((FS_EP2_BLKNO - 1) << 2) | FS_EP2_TYPE)

#define EP3_BLKNO           SINGLE_BLK          // 1
#define EP3_TYPE            TF_TYPE_INTERRUPT   // 3
#define EP3_BLKSIZE         BLK64BYTE           // 1
#define EP3_MAX_PACKET      0x0040
#define EP3_DIRECTION       DIRECTION_IN        // 0
#define EP3_FIFO_START      FIFO14              // 0xE
#define EP3_FIFO_NO         EP3_BLKSIZE         // 1
#define EP3_MAP             (EP3_FIFO_START | (EP3_FIFO_START << 4) | (MASK_F0 >> (4*EP3_DIRECTION)))
#define EP3_FIFO_MAP        (((1 - EP3_DIRECTION) << 4) | EP3) // ((1-0)<<4)|3=0x13               
#define EP3_FIFO_CONFIG	    (0x80 | ((EP3_BLKSIZE - 1) << 4) | ((EP3_BLKNO - 1) << 2) | EP3_TYPE)
#endif

#define MAIN_PROC_SIZE (3*1024*1024)


/*
// Macro declarations
*/
#define HI_BYTE_OF_WORD(x)      (BYTE)(((WORD)(x)) >> 8)
#define LO_BYTE_OF_WORD(x)      (BYTE)((WORD)(x))
#define HI_BYTE_OF_DWORD(x)     (BYTE)((DWORD)(x) >> 24)
#define MIDHI_BYTE_OF_DWORD(x)  (BYTE)((DWORD)(x) >> 16)
#define MIDLO_BYTE_OF_DWORD(x)  (BYTE)((DWORD)(x) >> 8)
#define LO_BYTE_OF_DWORD(x)     (BYTE)((DWORD)(x))
#define BYTE_SWAP_OF_WORD(x)	(WORD)(((WORD)(x) >> 8) | ((WORD)(x) << 8))
#define BYTE_SWAP_OF_DWORD(x)   (((DWORD)(x) << 24) | (((DWORD)(x) & 0x0000ff00) << 8) |\
                                (((DWORD)(x) & 0x00ff0000) >> 8) | ((DWORD)(x) >> 24))
#define BYTE_TO_DWORD(x,y,u,v)  (DWORD)(((DWORD)x)<<24)|(((DWORD)y)<<16)|\
                                (((DWORD)u)<<8)|((DWORD)(v))
#define BYTE_TO_WORD(x,y)       (WORD)((((WORD)x)<<8)|((WORD)y))



/*
// Structure declarations
*/
typedef enum {
    STATE_IDLE,
    STATE_CBW,
    STATE_CB_DATA_IN,
    STATE_CB_DATA_OUT,
    STATE_CSW,
    STATE_DATA_IN_STALL,
    STATE_DATA_OUT_STALL,
    STATE_PTP_CMD,
    STATE_PTP_DATA_IN,
    STATE_PTP_DATA_OUT,
    STATE_PTP_RES,
    STATE_PTP_INT_IN,

#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )    
    STATE_EXTERN_OUT,
#endif

} TRANSACTION_STATE;

typedef enum {
    CMD_VOID,                   // No command
    CMD_GET_DESCRIPTOR,         // Get_Descriptor command
    CMD_SET_DESCRIPTOR,          // Set_Descriptor command
} COMMAND_TYPE;

typedef enum {
    ACT_IDLE,
    ACT_DONE,
    ACT_STALL,
} ACTION;

typedef struct _Setup_Packet_
{
    BYTE Direction;	/*Data transfer direction: IN, OUT*/
    BYTE Type;          /*Request Type: Standard, Class, Vendor*/
    BYTE Object;        /*Recipient: Device, Interface, Endpoint,other*/
    BYTE Request;       /*Refer to Table 9-3*/
    WORD Value;
    WORD Index;
    WORD Length;
    WORD reserved;
} SETUP_PACKET, *PSETUP_PACKET;

void vUsbCxLoopBackTest(WHICH_OTG eWhichOtg);
void UsbOtgDeviceDetect(void);
void vOTG_ep0rx(WHICH_OTG eWhichOtg);
void UsbOtgDeviceIsr(WHICH_OTG eWhichOtg);
void UsbOtgDeviceTaskTerminate(WHICH_OTG eWhichOtg);
void vOTG_ep0setup(WHICH_OTG eWhichOtg);
BOOL bOTGCxFxWrRd(BYTE FIFONum, BYTE dir, BYTE *pu8Buffer, WORD u16Num, WHICH_OTG eWhichOtg);
void vOtgHandler(DWORD usbIntGroupReg, WHICH_OTG eWhichOtg);   // USB-IF (Sync with MP620A and get Out/In token at the same time to handle)
SDWORD UsbOtgDeviceTaskInit(WHICH_OTG eWhichOtg);


//#endif //SC_USBDEVICE

#endif // __USBOTG_DEVICE_H__
