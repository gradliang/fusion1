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
* Filename		: usbotg_utility.c
* Programmer(s)	: Joe Luo (JL)
* Created Date	: 2009/11/30 
* Description:  for dumping message
******************************************************************************** 
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "usbotg_ctrl.h"

#if (SC_USBHOST)

void DebugUsbOtgDumpDma(void)
{
    DWORD dwUsbOtg0DmaSetting;    
    DWORD dwUsbOtg1DmaSetting;
    DWORD dwUsbOtg0DmaCur;    
    DWORD dwUsbOtg1DmaCur;
    DWORD dwUsbOtg0DmaSaa;    
    DWORD dwUsbOtg1DmaSaa;
    DWORD dwUsbOtg0DmaEaa;    
    DWORD dwUsbOtg1DmaEaa;

    dwUsbOtg0DmaSetting = *(DWORD *)0xa800c180;
    dwUsbOtg1DmaSetting = *(DWORD *)0xa800c160;
    dwUsbOtg0DmaCur = *(DWORD *)0xa800c184;
    dwUsbOtg1DmaCur = *(DWORD *)0xa800c164;
    dwUsbOtg0DmaSaa = *(DWORD *)0xa800c188;
    dwUsbOtg1DmaSaa = *(DWORD *)0xa800c168;
    dwUsbOtg0DmaEaa = *(DWORD *)0xa800c18c;
    dwUsbOtg1DmaEaa = *(DWORD *)0xa800c16c;
    MP_DEBUG("################ DUMP DMA SETTING BEGIN ################");
    MP_DEBUG("USBOTG0 DMA Setting = 0x%x", dwUsbOtg0DmaSetting);
    MP_DEBUG("USBOTG0 DMA CUR = 0x%x", dwUsbOtg0DmaCur);
    MP_DEBUG("USBOTG0 DMA SAA = 0x%x", dwUsbOtg0DmaSaa);
    MP_DEBUG("USBOTG0 DMA EAA = 0x%x", dwUsbOtg0DmaEaa);
    //MP_DEBUG("USBOTG1 DMA Setting = 0x%x", dwUsbOtg1DmaSetting);
    //MP_DEBUG("USBOTG1 DMA CUR = 0x%x", dwUsbOtg1DmaCur);
    //MP_DEBUG("USBOTG1 DMA SAA = 0x%x", dwUsbOtg1DmaSaa);
    //MP_DEBUG("USBOTG1 DMA EAA = 0x%x", dwUsbOtg1DmaEaa);
    MP_DEBUG("################ DUMP DMA SETTING BEGIN ################");
}

void DebugUsbOtgDumpEhciDs(WHICH_OTG eWhichOtg) // not including overlap
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    MP_DEBUG("################ DUMP EHCI DS BEGIN ################");
    MP_DEBUG("psEhci = 0x%x", psEhci);
    MP_DEBUG("dwHostStructureBaseAddress = 0x%x", psEhci->dwHostStructureBaseAddress);
    MP_DEBUG("dwHostStructureQhdBaseAddress = 0x%x", psEhci->dwHostStructureQhdBaseAddress);
    MP_DEBUG("dwHostStructureQtdBaseAddress = 0x%x", psEhci->dwHostStructureQtdBaseAddress);
    MP_DEBUG("dwHostStructurePflBaseAddress = 0x%x", psEhci->dwHostStructurePflBaseAddress);
#if USBOTG_HOST_ISOC
    MP_DEBUG("dwHostStructureItdBaseAddress = 0x%x", psEhci->dwHostStructureItdBaseAddress);
    MP_DEBUG("dwHostDataPageBaseAddress = 0x%x", psEhci->dwHostDataPageBaseAddress);
#endif
    MP_DEBUG("dwHostMemoryBaseAddress = 0x%x", psEhci->dwHostMemoryBaseAddress);
    MP_DEBUG("dwHostQhdQtdHandleBaseAddress = 0x%x", psEhci->dwHostQhdQtdHandleBaseAddress);
    MP_DEBUG("dwHostSidcRxBufferAddress = 0x%x", psEhci->dwHostSidcRxBufferAddress);
    MP_DEBUG("dwHostSidcGetObjecRxBufferAddress = 0x%x", psEhci->dwHostSidcGetObjecRxBufferAddress);
    MP_DEBUG("dwControlRemainBytes = 0x%x", psEhci->dwControlRemainBytes);
    MP_DEBUG("dwHdTdBufferIndex = 0x%x", psEhci->dwHdTdBufferIndex);
    MP_DEBUG("dwHdBaseBufferIndex = 0x%x", psEhci->dwHdBaseBufferIndex);
    MP_DEBUG("psHostFramList = 0x%x", psEhci->psHostFramList);
 /*       MP_DEBUG("psHostFramList = 0x%x", psEhci->psHostFramList);
        DWORD                           dwIntervalMap[11];//DWORD                           waIntervalMap[11]={1,2,4,8,16,32,64,128,256,512,1024};
    
        BYTE                        *pbHostQtdManage;//BYTE             Host20_qTD_Manage[Host20_qTD_MAX];        //1=>Free 2=>used
#if USBOTG_HOST_ISOC
        BYTE                        *pbHostItdManage;//BYTE             Host20_iTD_Manage[Host20_iTD_MAX];        //1=>Free 2=>used  
        BYTE                        *pbHostDataPageManage;//Host20_DataPage_Manage[Host20_Page_MAX];  //1=>Free 2=>used  
#endif
        MEM_BLOCK                   sMem1024Bytes;//MEM_BLOCK gMem_1024_Bytes;
        MEM_BLOCK                   sMem512Bytes;//MEM_BLOCK gMem_512_Bytes;
        qTD_Structure               *psPreviousTempqTD;//static qTD_Structure *st_spPreviousTempqTD = OTGH_NULL;
        qTD_Structure               *psFirstTempqTD;//static qTD_Structure *st_spFirstTempqTD = OTGH_NULL;
*/
     MP_DEBUG("################ DUMP EHCI DS END ################");
}

void DebugUsbOtgDumpQtd(qTD_Structure  *psQtd) // not including overlap
{
    MP_DEBUG("################ DUMP QTD BEGIN ################");
    MP_DEBUG("psQtd = 0x%x", psQtd);
    //<1>.Next_qTD_Pointer Word
    MP_DEBUG("bNextQTDPointer = 0x%x <<5 = 0x%x", psQtd->bNextQTDPointer, psQtd->bNextQTDPointer<<5);
    MP_DEBUG("bTerminate = 0x%x", psQtd->bTerminate);

    //<2>.Alternate Next qTD Word
    MP_DEBUG("bAlternateQTDPointer = 0x%x <<5 = 0x%x", psQtd->bAlternateQTDPointer, psQtd->bAlternateQTDPointer<<5);
    MP_DEBUG("bAlternateTerminate = 0x%x", psQtd->bAlternateTerminate);
         
    //<3>.Status Word     
    MP_DEBUG("bDataToggle = 0x%x", psQtd->bDataToggle);
    MP_DEBUG("bTotalBytes = 0x%x", psQtd->bTotalBytes);
    MP_DEBUG("bInterruptOnComplete = 0x%x", psQtd->bInterruptOnComplete);
    MP_DEBUG("CurrentPage = 0x%x", psQtd->CurrentPage);
    MP_DEBUG("bErrorCounter = 0x%x", psQtd->bErrorCounter);
    MP_DEBUG("bPID = 0x%x", psQtd->bPID);
    MP_DEBUG("bStatus_Active = 0x%x", psQtd->bStatus_Active);
    MP_DEBUG("bStatus_Halted = 0x%x", psQtd->bStatus_Halted);
    MP_DEBUG("bStatus_Buffer_Err = 0x%x", psQtd->bStatus_Buffer_Err);
    MP_DEBUG("bStatus_Babble = 0x%x", psQtd->bStatus_Babble);
    MP_DEBUG("bStatus_Transaction_Err = 0x%x", psQtd->bStatus_Transaction_Err);
    MP_DEBUG("bStatus_Transaction_Err = 0x%x", psQtd->bStatus_Transaction_Err);
    MP_DEBUG("bStatus_SplitState = 0x%x", psQtd->bStatus_SplitState);
    MP_DEBUG("bStatus_SplitState = 0x%x", psQtd->bStatus_SplitState);

    //<4>.Buffer Pointer Word Array     
    MP_DEBUG("ArrayBufferPointer_Word[0] = 0x%x", psQtd->ArrayBufferPointer_Word[0]);
    MP_DEBUG("ArrayBufferPointer_Word[1] = 0x%x", psQtd->ArrayBufferPointer_Word[1]);
    MP_DEBUG("ArrayBufferPointer_Word[2] = 0x%x", psQtd->ArrayBufferPointer_Word[2]);
    MP_DEBUG("ArrayBufferPointer_Word[3] = 0x%x", psQtd->ArrayBufferPointer_Word[3]);
    MP_DEBUG("ArrayBufferPointer_Word[4] = 0x%x", psQtd->ArrayBufferPointer_Word[4]);

    BYTE i;
    BYTE *pbtemp = (BYTE*)psQtd->ArrayBufferPointer_Word[0];
    for (i = 0; i < 8; i++)
    {
        MP_DEBUG("Data[%d] = 0x%x", i, pbtemp[i]);
    }    
    MP_DEBUG("################ DUMP QTD END ################");
}

void DebugUsbOtgDumpQhd(qHD_Structure  *psQhd) // not including overlap
{
    MP_DEBUG("################ DUMP QHD BEGIN ################");
    MP_DEBUG("psQhd = 0x%x", psQhd);
    //<1>.Next_qHD_Pointer Word
    MP_DEBUG("bNextQHDPointer = 0x%x <<5 = 0x%x", psQhd->bNextQHDPointer, psQhd->bNextQHDPointer<<5);
    MP_DEBUG("bType = 0x%x", psQhd->bType);
    MP_DEBUG("bTerminate = 0x%x", psQhd->bTerminate);
               
    //<2>.qHD_2 Word
    MP_DEBUG("bNakCounter = 0x%x", psQhd->bNakCounter);
    MP_DEBUG("bControlEdFlag = 0x%x", psQhd->bControlEdFlag);
    MP_DEBUG("bMaxPacketSize = 0x%x", psQhd->bMaxPacketSize);
    MP_DEBUG("bHeadOfReclamationListFlag = 0x%x", psQhd->bHeadOfReclamationListFlag);
    MP_DEBUG("bDataToggleControl = 0x%x", psQhd->bDataToggleControl);
    MP_DEBUG("bEdSpeed = 0x%x", psQhd->bEdSpeed);
    MP_DEBUG("bEdNumber = 0x%x", psQhd->bEdNumber);
    MP_DEBUG("bInactiveOnNextTransaction = 0x%x", psQhd->bInactiveOnNextTransaction);
    MP_DEBUG("bDeviceAddress = 0x%x", psQhd->bDeviceAddress);
    
    //<3>.qHD_3 Word     
    MP_DEBUG("bHighBandwidth = 0x%x", psQhd->bHighBandwidth);
    MP_DEBUG("bPortNumber = 0x%x", psQhd->bPortNumber);
    MP_DEBUG("bHubAddr = 0x%x", psQhd->bHubAddr);
    MP_DEBUG("bSplitTransactionMask = 0x%x", psQhd->bSplitTransactionMask);
    MP_DEBUG("bInterruptScheduleMask = 0x%x", psQhd->bInterruptScheduleMask);

    //<4>.Overlay_CurrentqTD 
    MP_DEBUG("bOverlay_CurrentqTD = 0x%x", psQhd->bOverlay_CurrentqTD);    

    //<5>.Overlay_NextqTD
    MP_DEBUG("bOverlay_NextqTD = 0x%x <<5 = 0x%x", psQhd->bOverlay_NextqTD, psQhd->bOverlay_NextqTD<<5);
    
    MP_DEBUG("################ DUMP QHD END ################");
}

void DebugUsbOtgDumpRegister(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

    MP_DEBUG("################ DUMP Register BEGIN ################");
    MP_DEBUG("USBOTG%d psUsbReg = 0x%x", eWhichOtg, psUsbOtg->psUsbReg);
    MP_DEBUG("HcCapability = 0x%x", psUsbOtg->psUsbReg->HcCapability);
    MP_DEBUG("HcStructuralParameters = 0x%x", psUsbOtg->psUsbReg->HcStructuralParameters);
    MP_DEBUG("HcCapabilityParameters = 0x%x", psUsbOtg->psUsbReg->HcCapabilityParameters);
    MP_DEBUG("HcUsbCommand = 0x%x", psUsbOtg->psUsbReg->HcUsbCommand);
    MP_DEBUG("HcUsbStatus = 0x%x", psUsbOtg->psUsbReg->HcUsbStatus);
    MP_DEBUG("HcUsbInterruptEnable = 0x%x", psUsbOtg->psUsbReg->HcUsbInterruptEnable);
    MP_DEBUG("HcFrameIndex = 0x%x", psUsbOtg->psUsbReg->HcFrameIndex);
    MP_DEBUG("HcPeriodicFrameListBaseAddress = 0x%x", psUsbOtg->psUsbReg->HcPeriodicFrameListBaseAddress);
    MP_DEBUG("HcCurrentAsynListAddress = 0x%x", psUsbOtg->psUsbReg->HcCurrentAsynListAddress);
    MP_DEBUG("HcPortStatusAndControl = 0x%x", psUsbOtg->psUsbReg->HcPortStatusAndControl);
    MP_DEBUG("HcMisc = 0x%x", psUsbOtg->psUsbReg->HcMisc);
    MP_DEBUG("OtgControlStatus = 0x%x", psUsbOtg->psUsbReg->OtgControlStatus);
    MP_DEBUG("OtgInterruptStatus = 0x%x", psUsbOtg->psUsbReg->OtgInterruptStatus);
    MP_DEBUG("OtgInterruptEnable = 0x%x", psUsbOtg->psUsbReg->OtgInterruptEnable);
    MP_DEBUG("GlobalHcOtgDevInterruptStatus = 0x%x", psUsbOtg->psUsbReg->GlobalHcOtgDevInterruptStatus);
    MP_DEBUG("GlobalMaskofHcOtgDevInterrupt = 0x%x", psUsbOtg->psUsbReg->GlobalMaskofHcOtgDevInterrupt);

    MP_DEBUG("DeviceMainControl = 0x%x", psUsbOtg->psUsbReg->DeviceMainControl);
    /*
    MP_DEBUG("DeviceAddress = 0x%x", psUsbOtg->psUsbReg->DeviceAddress);
    MP_DEBUG("DeviceTest = 0x%x", psUsbOtg->psUsbReg->DeviceTest);
    MP_DEBUG("DeviceSofFrameNumber = 0x%x", psUsbOtg->psUsbReg->DeviceSofFrameNumber);
    MP_DEBUG("DeviceSofMaskTimer = 0x%x", psUsbOtg->psUsbReg->DeviceSofMaskTimer);
    MP_DEBUG("PhyTestModeSelector = 0x%x", psUsbOtg->psUsbReg->PhyTestModeSelector);
    MP_DEBUG("DeviceVendorSpecificIoControl = 0x%x", psUsbOtg->psUsbReg->DeviceVendorSpecificIoControl);
    MP_DEBUG("DeviceCxCfgAndStatus = 0x%x", psUsbOtg->psUsbReg->DeviceCxCfgAndStatus);
    MP_DEBUG("DeviceCxCfgAndFifoEmptyStatus = 0x%x", psUsbOtg->psUsbReg->DeviceCxCfgAndFifoEmptyStatus);
    MP_DEBUG("DeviceIdleCounter = 0x%x", psUsbOtg->psUsbReg->DeviceIdleCounter);
    MP_DEBUG("DeviceMaskofInterruptGroup = 0x%x", psUsbOtg->psUsbReg->DeviceMaskofInterruptGroup);
    MP_DEBUG("DeviceMaskofInterruptSrcGroup0 = 0x%x", psUsbOtg->psUsbReg->DeviceMaskofInterruptSrcGroup0);
    MP_DEBUG("DeviceMaskofInterruptSrcGroup1 = 0x%x", psUsbOtg->psUsbReg->DeviceMaskofInterruptSrcGroup1);
    MP_DEBUG("DeviceMaskofInterruptSrcGroup2 = 0x%x", psUsbOtg->psUsbReg->DeviceMaskofInterruptSrcGroup2);
    MP_DEBUG("DeviceInterruptGroup = 0x%x", psUsbOtg->psUsbReg->DeviceInterruptGroup);
    MP_DEBUG("DeviceInterruptSourceGroup0 = 0x%x", psUsbOtg->psUsbReg->DeviceInterruptSourceGroup0);
    MP_DEBUG("DeviceInterruptSourceGroup1 = 0x%x", psUsbOtg->psUsbReg->DeviceInterruptSourceGroup1);
    MP_DEBUG("DeviceInterruptSourceGroup2 = 0x%x", psUsbOtg->psUsbReg->DeviceInterruptSourceGroup2);
    MP_DEBUG("DeviceRxZeroLengthDataPacket = 0x%x", psUsbOtg->psUsbReg->DeviceRxZeroLengthDataPacket);
    MP_DEBUG("DeviceTxZeroLengthDataPacket = 0x%x", psUsbOtg->psUsbReg->DeviceTxZeroLengthDataPacket);
    MP_DEBUG("DeviceIsocSequentialErrorAbort = 0x%x", psUsbOtg->psUsbReg->DeviceIsocSequentialErrorAbort);
    MP_DEBUG("DeviceInEndpoint1MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint1MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint2MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint2MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint3MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint3MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint4MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint4MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint5MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint5MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint6MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint6MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint7MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint7MaxPacketSize);
    MP_DEBUG("DeviceInEndpoint8MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceInEndpoint8MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint1MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint1MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint2MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint2MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint3MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint3MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint4MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint4MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint5MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint5MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint6MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint6MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint7MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint7MaxPacketSize);
    MP_DEBUG("DeviceOutEndpoint8MaxPacketSize = 0x%x", psUsbOtg->psUsbReg->DeviceOutEndpoint8MaxPacketSize);
    MP_DEBUG("DeviceEndpoint1to4Map = 0x%x", psUsbOtg->psUsbReg->DeviceEndpoint1to4Map);
    MP_DEBUG("DeviceEndpoint5to8Map = 0x%x", psUsbOtg->psUsbReg->DeviceEndpoint5to8Map);
    MP_DEBUG("DeviceFifoMap = 0x%x", psUsbOtg->psUsbReg->DeviceFifoMap);
    MP_DEBUG("DeviceFifoConfiguration = 0x%x", psUsbOtg->psUsbReg->DeviceFifoConfiguration);
    MP_DEBUG("DeviceFifo0InstructionAndByteCount = 0x%x", psUsbOtg->psUsbReg->DeviceFifo0InstructionAndByteCount);
    MP_DEBUG("DeviceFifo1InstructionAndByteCount = 0x%x", psUsbOtg->psUsbReg->DeviceFifo1InstructionAndByteCount);
    MP_DEBUG("DeviceFifo2InstructionAndByteCount = 0x%x", psUsbOtg->psUsbReg->DeviceFifo2InstructionAndByteCount);
    MP_DEBUG("DeviceFifo3InstructionAndByteCount = 0x%x", psUsbOtg->psUsbReg->DeviceFifo3InstructionAndByteCount);
    MP_DEBUG("DeviceDmaTargetFifNumber = 0x%x", psUsbOtg->psUsbReg->DeviceDmaTargetFifNumber);
    MP_DEBUG("DeviceDmaControllerParameterSetting1 = 0x%x", psUsbOtg->psUsbReg->DeviceDmaControllerParameterSetting1);
    MP_DEBUG("DeviceDmaControllerParameterSetting2 = 0x%x", psUsbOtg->psUsbReg->DeviceDmaControllerParameterSetting2);
    MP_DEBUG("DeviceDmaControllerParameterSetting3 = 0x%x", psUsbOtg->psUsbReg->DeviceDmaControllerParameterSetting3);
    */
    MP_DEBUG("DmaControllerStatus = 0x%x", psUsbOtg->psUsbReg->DmaControllerStatus);
    MP_DEBUG("SwapBufferStart1 = 0x%x", psUsbOtg->psUsbReg->SwapBufferStart1);
    MP_DEBUG("SwapBufferEnd1 = 0x%x", psUsbOtg->psUsbReg->SwapBufferEnd1);
    MP_DEBUG("SwapBufferStart2 = 0x%x", psUsbOtg->psUsbReg->SwapBufferStart2);
    MP_DEBUG("SwapBufferEnd2 = 0x%x", psUsbOtg->psUsbReg->SwapBufferEnd2);
    MP_DEBUG("WrapperCtrl = 0x%x", psUsbOtg->psUsbReg->WrapperCtrl);
    MP_DEBUG("################ DUMP Register END ################");
}

void DebugUsbOtgDumpHostDevDesc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR psUsbhDevDesc = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    MP_DEBUG("################ DUMP HostDevDesc BEGIN ################");
    MP_DEBUG("USBOTG%d psUsbhDevDesc = 0x%x", eWhichOtg, psUsbhDevDesc);
    MP_DEBUG(" bDeviceSpeed = 0x%x", psUsbhDevDesc->bDeviceSpeed);
    MP_DEBUG(" bDeviceAddress = 0x%x", psUsbhDevDesc->bDeviceAddress);
    MP_DEBUG(" bDeviceStatus = 0x%x", psUsbhDevDesc->bDeviceStatus);
    MP_DEBUG(" bDeviceConfigVal = 0x%x", psUsbhDevDesc->bDeviceConfigVal);
    MP_DEBUG(" &sSetupPB = 0x%x", &psUsbhDevDesc->sSetupPB);
    MP_DEBUG(" &psAppClass = 0x%x", &psUsbhDevDesc->psAppClass);
    MP_DEBUG(" &psHubClass = 0x%x", &psUsbhDevDesc->psHubClass);
    MP_DEBUG(" &sDeviceDescriptor = 0x%x", &psUsbhDevDesc->sDeviceDescriptor);
    MP_DEBUG(" &sDeviceStatus = 0x%x", &psUsbhDevDesc->sDeviceStatus);
    MP_DEBUG(" pConfigDescriptor = 0x%x", psUsbhDevDesc->pConfigDescriptor);
    MP_DEBUG(" &sConfigDescriptor = 0x%x", &psUsbhDevDesc->sConfigDescriptor);
    MP_DEBUG(" &sInterfaceDescriptor = 0x%x", &psUsbhDevDesc->sInterfaceDescriptor);
    MP_DEBUG(" &sEndpointDescriptor = 0x%x", &psUsbhDevDesc->sEndpointDescriptor);
    MP_DEBUG(" wStateMachine = 0x%x", psUsbhDevDesc->wStateMachine);
    MP_DEBUG(" wCurrentExecutionState = 0x%x", psUsbhDevDesc->wCurrentExecutionState);
    MP_DEBUG(" bUsbhDevDesIdx = 0x%x", psUsbhDevDesc->bUsbhDevDesIdx);
    MP_DEBUG(" bDeviceConfigIdx = 0x%x", psUsbhDevDesc->bDeviceConfigIdx);
    MP_DEBUG(" bDeviceInterfaceIdx = 0x%x", psUsbhDevDesc->bDeviceInterfaceIdx);
    MP_DEBUG(" bMaxBulkInEpNumber = 0x%x", psUsbhDevDesc->bMaxBulkInEpNumber);
    MP_DEBUG(" bMaxBulkOutEpNumber = 0x%x", psUsbhDevDesc->bMaxBulkOutEpNumber);
    MP_DEBUG(" bMaxInterruptInEpNumber = 0x%x", psUsbhDevDesc->bMaxInterruptInEpNumber);
    MP_DEBUG(" bMaxInterruptOutEpNumber = 0x%x", psUsbhDevDesc->bMaxInterruptOutEpNumber);
    //MP_DEBUG(" bBulkEpIndex = 0x%x", psUsbhDevDesc->bBulkEpIndex);
    //MP_DEBUG(" bInterruptEpIndex = 0x%x", psUsbhDevDesc->bInterruptEpIndex);
    MP_DEBUG(" dwWhichBulkPipeDone = 0x%x", psUsbhDevDesc->dwWhichBulkPipeDone);
    MP_DEBUG(" dwWhichInterruptPipeDone = 0x%x", psUsbhDevDesc->dwWhichInterruptPipeDone);
    MP_DEBUG(" pstControlqHD[0] = 0x%x", psUsbhDevDesc->pstControlqHD[0]);
    MP_DEBUG(" pstControlqHD[1] = 0x%x", psUsbhDevDesc->pstControlqHD[1]);
    MP_DEBUG(" hstBulkInqHD = 0x%x", psUsbhDevDesc->hstBulkInqHD);
    MP_DEBUG(" hstBulkOutqHD = 0x%x", psUsbhDevDesc->hstBulkOutqHD);
    MP_DEBUG(" hstInterruptInqHD = 0x%x", psUsbhDevDesc->hstInterruptInqHD);
    MP_DEBUG(" hstInterruptOutqHD = 0x%x", psUsbhDevDesc->hstInterruptOutqHD);
    MP_DEBUG(" psControlqTD = 0x%x", psUsbhDevDesc->psControlqTD);
    MP_DEBUG(" pstControlWorkqTD = 0x%x", psUsbhDevDesc->pstControlWorkqTD);
    MP_DEBUG(" pstControlSendLastqTD = 0x%x", psUsbhDevDesc->pstControlSendLastqTD);
    MP_DEBUG(" hstIntInWorkqTD = 0x%x", psUsbhDevDesc->hstIntInWorkqTD);
    MP_DEBUG(" hstIntInSendLastqTD = 0x%x", psUsbhDevDesc->hstIntInSendLastqTD);
    MP_DEBUG(" hstIntOutWorkqTD = 0x%x", psUsbhDevDesc->hstIntOutWorkqTD);
    MP_DEBUG(" hstIntOutSendLastqTD = 0x%x", psUsbhDevDesc->hstIntOutSendLastqTD);
    MP_DEBUG(" hstBulkInWorkqTD = 0x%x", psUsbhDevDesc->hstBulkInWorkqTD);
    MP_DEBUG(" hstBulkInSendLastqTD = 0x%x", psUsbhDevDesc->hstBulkInSendLastqTD);
    MP_DEBUG(" hstBulkOutWorkqTD = 0x%x", psUsbhDevDesc->hstBulkOutWorkqTD);
    MP_DEBUG(" hstBulkOutSendLastqTD = 0x%x", psUsbhDevDesc->hstBulkOutSendLastqTD);
    MP_DEBUG(" hstBulkOutqTD = 0x%x", psUsbhDevDesc->hstBulkOutqTD);
    MP_DEBUG(" hstBulkInqTD = 0x%x", psUsbhDevDesc->hstBulkInqTD);
    MP_DEBUG(" hstInterruptInqTD = 0x%x", psUsbhDevDesc->hstInterruptInqTD);
    MP_DEBUG(" hstInterruptOutqTD = 0x%x", psUsbhDevDesc->hstInterruptOutqTD);
    MP_DEBUG(" bDataBuffer = 0x%x", psUsbhDevDesc->bDataBuffer);
    MP_DEBUG(" fpAppClassBulkActive = 0x%x", psUsbhDevDesc->fpAppClassBulkActive);
    MP_DEBUG(" fpAppClassBulkIoc = 0x%x", psUsbhDevDesc->fpAppClassBulkIoc);
    MP_DEBUG(" fpAppClassSetupIoc = 0x%x", psUsbhDevDesc->fpAppClassSetupIoc);
    MP_DEBUG(" fpAppClassInterruptActive = 0x%x", psUsbhDevDesc->fpAppClassInterruptActive);
    MP_DEBUG(" fpAppClassInterruptIoc = 0x%x", psUsbhDevDesc->fpAppClassInterruptIoc);
//    MP_DEBUG(" fpAppClassIsoActive = 0x%x", psUsbhDevDesc->fpAppClassIsoActive);
//    MP_DEBUG(" fpAppClassIsoIoc = 0x%x", psUsbhDevDesc->fpAppClassIsoIoc);
    MP_DEBUG(" dwActivedQHD[0] = 0x%x", psUsbhDevDesc->dwActivedQHD[0]);
    MP_DEBUG(" dwActivedQHD[1] = 0x%x", psUsbhDevDesc->dwActivedQHD[1]);
    MP_DEBUG(" dwActivedQHD[2] = 0x%x", psUsbhDevDesc->dwActivedQHD[2]);
    MP_DEBUG(" dwActivedQHD[3] = 0x%x", psUsbhDevDesc->dwActivedQHD[3]);
    MP_DEBUG(" dwActivedQHD[4] = 0x%x", psUsbhDevDesc->dwActivedQHD[4]);
    MP_DEBUG(" bQHStatus = 0x%x", psUsbhDevDesc->bQHStatus);
    MP_DEBUG(" bSendStatusError = 0x%x", psUsbhDevDesc->bSendStatusError);
    MP_DEBUG(" bRemoteWakeUpDetected = 0x%x", psUsbhDevDesc->bRemoteWakeUpDetected);
    MP_DEBUG(" bPortReset = 0x%x", psUsbhDevDesc->bPortReset);
    MP_DEBUG(" bActivedQHdIdx = 0x%x", psUsbhDevDesc->bActivedQHdIdx);
    MP_DEBUG(" bCmdTimeoutError = 0x%x", psUsbhDevDesc->bCmdTimeoutError);
    MP_DEBUG(" bConnectStatus = 0x%x", psUsbhDevDesc->bConnectStatus);
    MP_DEBUG(" bSuspend = 0x%x", psUsbhDevDesc->bSuspend);
    MP_DEBUG(" urb = 0x%x", psUsbhDevDesc->urb);
    //MP_DEBUG(" bDeviceOnHub = 0x%x", psUsbhDevDesc->bDeviceOnHub);
    //MP_DEBUG(" bAdd = 0x%x", psUsbhDevDesc->bAdd);
    MP_DEBUG(" bMaxIsoInEpNumber = 0x%x", psUsbhDevDesc->bMaxIsoInEpNumber);
    MP_DEBUG(" bMaxIsoOutEpNumber = 0x%x", psUsbhDevDesc->bMaxIsoOutEpNumber);
    MP_DEBUG("################ DUMP HostDevDesc END ################");
}

// At UsbOtgDeviceDetect(void) in Usbotg_device.c
void CheckDeviceSetupFIFO(WHICH_OTG eWhichOtg)
{
    BYTE dataBuf[64];
    BYTE dataBuf1[64];
    
    DWORD dwTotCnt = 10; // Compare Times 
    DWORD doCnt = 0; 
    DWORD index;
    DWORD testOK;
    DWORD USB_OTG_BASE = 0;

    
    MP_DEBUG("\n-usbotg%d- Check Device Setup FIFO", eWhichOtg);
    MP_DEBUG("Compare Pattern %d Times between Write/Read FIFO", dwTotCnt);
    MP_DEBUG("Pattern : 64BYTE from [0] = 0 to [63] = 63");
    
    USB_OTG_BASE = (eWhichOtg == USBOTG0 ? USBOTG0_BASE : USBOTG1_BASE);

    MP_DEBUG("*((DWORD *)(%x)) = 0x%x =?= 0x00300000", USB_OTG_BASE+0x080, *((DWORD *)(USB_OTG_BASE+0x080)));

    *((DWORD *)(USB_OTG_BASE+0x1c8)) |= BIT31;
    MP_DEBUG("-1-");
    *((DWORD *)(USB_OTG_BASE+0x1c8)) &= ~BIT31;
    MP_DEBUG("-2-");

    *((DWORD *)(USB_OTG_BASE+0x138))  |= 0x000e00cf;
    MP_DEBUG("-3-");
    *((DWORD *)(USB_OTG_BASE+0x1a0))  |= 0x33332330;
    MP_DEBUG("-4-");
    *((DWORD *)(USB_OTG_BASE+0x1a8))  |= 0x0f020f11;
    MP_DEBUG("-5-");
    *((DWORD *)(USB_OTG_BASE+0x1ac))  |= 0x06260626;
    MP_DEBUG("-6-");
    *((DWORD *)(USB_OTG_BASE+0x160))  |= 0x00000040;
    MP_DEBUG("-7-");
    *((DWORD *)(USB_OTG_BASE+0x184))  |= 0x00000040;
    MP_DEBUG("-8-");
    *((DWORD *)(USB_OTG_BASE+0x108))  |= 0x00000002;
    MP_DEBUG("-9-");
    *((DWORD *)(USB_OTG_BASE+0x130))  |= 0x00000007;
    MP_DEBUG("-10-");
    *((DWORD *)(USB_OTG_BASE+0x114))  |= 0x00000000; //Unplug
    MP_DEBUG("-11-");
    *((DWORD *)(USB_OTG_BASE+0x0c4))  |= 0x0000000f;
    MP_DEBUG("-12-");
    *((DWORD *)(USB_OTG_BASE+0x1c0))  |= 0x00000010; // CXF FIFO ACT
    MP_DEBUG("-13-");
    *((DWORD *)(USB_OTG_BASE+0x100))  |= 0x00000024;
    MP_DEBUG("-14-");

	while(doCnt < dwTotCnt)
	{
        MP_DEBUG("*((DWORD *)(%x)) = 0x%x =?= 0x00300000", USB_OTG_BASE+0x080, *((DWORD *)(USB_OTG_BASE+0x080)));
		doCnt++;

		for (index = 0; index < 64; ++index)
		{
			dataBuf[index]  = (BYTE) index ;
			dataBuf1[index] = 0;
		}

		MP_DEBUG("Loop Test Begin : %d Times", doCnt);

		*((DWORD *)(USB_OTG_BASE+0x1c0)) = 0x10;
		*((DWORD *)(USB_OTG_BASE+0x1cc)) = (DWORD) &dataBuf[0];
		*((DWORD *)(USB_OTG_BASE+0x1c8)) = 0x4002;

		*((DWORD *)(USB_OTG_BASE+0x120)) |= BIT3;
		while ((*((volatile DWORD *)(USB_OTG_BASE+0x120)) & BIT5) != BIT5);

		MP_DEBUG("Copy DRAM addr 0x%x to FIFO addr 0x%x", (DWORD) &dataBuf[0], *((DWORD *)(USB_OTG_BASE+0x120)));
		*((DWORD *)(USB_OTG_BASE+0x1c8)) = 0x1;
		while((*((volatile DWORD *)(USB_OTG_BASE+0x14c)) & BIT7) == 0);
        
		MP_DEBUG("Copy DRAM to FIFO Done.....\n");
        
		*((DWORD *)(USB_OTG_BASE+0x14c)) &= ~BIT7;
		*((DWORD *)(USB_OTG_BASE+0x1c0)) = 0x10;
		*((DWORD *)(USB_OTG_BASE+0x1cc)) = (DWORD) &dataBuf1[0];
		*((DWORD *)(USB_OTG_BASE+0x1c8)) = 0x4000;

		while ((*((volatile DWORD *)(USB_OTG_BASE+0x120)) & BIT4) != BIT4);

		MP_DEBUG("Copy FIFO addr 0x%x to DRAM addr 0x%x", *((DWORD *)(USB_OTG_BASE+0x120)), (DWORD) &dataBuf1[0]);
		*((DWORD *)(USB_OTG_BASE+0x1c8)) |= 0x1;
		while((*((volatile DWORD *)(USB_OTG_BASE+0x14c)) & BIT7) == 0);
		MP_DEBUG("Copy FIFO to DRAM Done.....\n");
		*((DWORD *)(USB_OTG_BASE+0x14c)) &= ~BIT7;

		testOK = TRUE;
        
		MP_DEBUG("Start to Compare.....\n");
		for (index = 0; index < 0x40; ++index)
		{
			if (dataBuf[index] != dataBuf1[index])
			{
				testOK = FALSE;
				MP_DEBUG("Write[%d] %x != Read[%d] %x", index, dataBuf[index], index, dataBuf1[index]);
			}
		}
        
		if (testOK)
		{
			MP_DEBUG("Test OK!!");
		}
		else 
		{
            MP_DEBUG("Test FAIL!!");
			DebugUsbOtgDumpRegister(eWhichOtg);
			while(1);
		}
        
		MP_DEBUG("Loop Test End");
	}
}

#endif

