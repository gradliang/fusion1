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
* Filename      : usbotg_device_msdc_teco.c
* Programmer(s) : Calvin Liao
* Created       :
* Descriptions  : Vendor MSDC Command for TECO
*******************************************************************************
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
#include "usbotg_device_msdc_teco.h"
#include "os.h"


#if (SC_USBDEVICE && USBOTG_DEVICE_MSDC_TECO)
/*
// Constant declarations
*/
#define SAVE_DATA_TO_SDCARD  0  // Save JPEG File From PC into SD Card
#define MPX_DEVICE_CHAR_MAX   8
#define TECO_DEFAULT_LUN          0

/*
// Structure declarations
*/


/*
// Variable declarations
*/
BYTE    bMpxDevice[MPX_DEVICE_CHAR_MAX] = {'M', 'P', 'X', '&', 'T', 'E', 'C', 'O'};
BYTE    bFakeData[10] = {0};


/*
// External Variable declarations
*/


/*
// Macro declarations
*/


/*
// Static function prototype
*/


/*
// Definition of internal functions
*/

WORD VendorTecoPreview(BOOL bEnable, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    WORD wWidth, wHeight;
    ST_IMGWIN * ImgWin;
    ST_IMGWIN * srcwin;
    ST_IMGWIN srcwin_content;
    DWORD dwCnt = 0;

    if(bEnable == TRUE)  // Enable Preview
    {
        if(psUsbDevMsdc->pbTecoDataBuff == NULL)
        {
            MP_DEBUG("Preview Fail:DataBuffer NULL");
            return STS_CMD_FAILED;  // Send Fail to AP    	
        }

  
        ImgWin = Idu_GetNextWin();
        srcwin = &srcwin_content;
        srcwin->pdwStart = (DWORD)(psUsbDevMsdc->pbTecoDataBuff)|(0xa0000000);
        SystemPanelSizeGet(&wWidth, &wHeight);
        srcwin->wWidth = wWidth;
        srcwin->wHeight = wHeight;
        srcwin->wType = 444;
        srcwin->dwOffset = wWidth*3;

        #if 1 // DMA mode

        Ipu_ImageScaling(srcwin, ImgWin, 0, 0, srcwin->wWidth, srcwin->wHeight, 0, 0, ImgWin->wWidth, ImgWin->wHeight, 0);
        Idu_ChgWin(Idu_GetNextWin());

        #else  // ByPass Mode

        register IDU *idu = (IDU *) IDU_BASE;

        if(idu->RST_Ctrl_Bypass & 0x1)
        {
        Video_Update_Frame(srcwin, 0);
        }
        else
        {
        Ipu_Video_Bypass_Scaling(srcwin, ImgWin, 0, 0, srcwin->wWidth, srcwin->wHeight, 0, 0, ImgWin->wWidth, ImgWin->wHeight, 0);
        }

        #endif
    }
    else  // Disable Preview and Goto Main Page
    {
        //VendorTecoSetIduMode(MODE_422);
	if(psUsbDevMsdc->pbTecoDataBuff != NULL)
	{
		ext_mem_free(psUsbDevMsdc->pbTecoDataBuff);  // Free data buffer
		psUsbDevMsdc->pbTecoDataBuff  = NULL;
	}	
    }

    return STS_GOOD;
}


void VendorTecoSetIduMode(BYTE bMode)
{
    switch (bMode)
    {
    
	case MODE_422:
		if(Idu_GetIs444Mode())
		    Idu_Chg_422_Mode();
		break;

	case MODE_444:
		if(!Idu_GetIs444Mode())
		    Idu_Chg_444_Mode();
		break;	

	default:
		MP_ALERT("%s UnKnow Mode 0x%x", __FUNCTION__, bMode);
		break;
    }
}


WORD VendorTecoSendData(DWORD dwOffset, DWORD dwTransferLen, DWORD dwTotalSize, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);

    MP_DEBUG("%s:Offset %d, TransferLen %d, TotalSize %d", __FUNCTION__, dwOffset, dwTransferLen, dwTotalSize);
    // Get new data when Offset = 0.
    if(dwOffset == 0)
    {
	if(psUsbDevMsdc->pbTecoDataBuff != NULL)
	{
		ext_mem_free(psUsbDevMsdc->pbTecoDataBuff);  // Free data buffer
		psUsbDevMsdc->pbTecoDataBuff  = NULL;
	}	
            
        if(ext_mem_get_free_space() < dwTotalSize)
        {
            MP_ALERT("%s external memory is not enough for data buffer", __FUNCTION__);
            return STS_CMD_FAILED;
        }
            
        psUsbDevMsdc->pbTecoDataBuff = (BYTE *)(ext_mem_malloc(dwTotalSize) | 0xA0000000);
        MP_DEBUG("pbTecoDataBuff Addr 0x%x", psUsbDevMsdc->pbTecoDataBuff);
        memset(psUsbDevMsdc->pbTecoDataBuff , 0, dwTotalSize);
        //__asm("break 100");
    }

    return STS_GOOD;
}


WORD VendorTecoCmd(BYTE** hData, USB_TECO_CMD* pVendorCmd, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
    WORD wWidth = 0, wHeight = 0, ret = STS_GOOD;
    DWORD dwOffset =0, dwTransferLen = 0, dwTotalSize = 0;

    if(lun != TECO_DEFAULT_LUN)
        return STS_CMD_FAILED;

    switch (pVendorCmd->bFunctionCode)
    {
	case FC_GET_MPX_DEVICE: // DATA-IN

		MP_DEBUG("GET_MPX_DEVICE");

		*hData = bMpxDevice;
		*pData_residue = MPX_DEVICE_CHAR_MAX;

		break;

	case FC_SET_IDU_MODE: // Non-Data

		MP_DEBUG("SET_IDU_MODE");
		
		// Set IDU Mode
		VendorTecoSetIduMode(pVendorCmd->bData[0]);
		    
		*pData_residue = 0; // Goto CSW

		break;

	case FC_SEND_DATA: // DATA-OUT

		MP_DEBUG("SEND_DATA");		

		*hData = (BYTE *)UsbOtgBufferGet(eWhichOtg);//g_USBBUF; for DATA-OUT
		
		dwOffset = BYTE_TO_DWORD(pVendorCmd->bData[0], pVendorCmd->bData[1], pVendorCmd->bData[2], pVendorCmd->bData[3]);
		dwTransferLen = BYTE_TO_DWORD(pVendorCmd->bData[4], pVendorCmd->bData[5], pVendorCmd->bData[6], pVendorCmd->bData[7]);
		dwTotalSize = BYTE_TO_DWORD(pVendorCmd->bData[8], pVendorCmd->bData[9], pVendorCmd->bData[10], pVendorCmd->bData[11]);		
		ret = VendorTecoSendData(dwOffset, dwTransferLen, dwTotalSize, eWhichOtg);

		*pData_residue = dwTransferLen;

		break;

	case FC_PREVIEW: // Non-Data

		MP_DEBUG("PREVIEW");
		
		// Send data to IDU
		ret = VendorTecoPreview(pVendorCmd->bData[0], eWhichOtg);
            		
		*pData_residue = 0; // Goto CSW

		break;

	case FC_GET_RESOLUTION: // DATA-IN

		MP_DEBUG("GET_RESOLUTION");	

		SystemPanelSizeGet(&wWidth, &wHeight);
		bFakeData[0] = (wWidth >> 8) & 0xff ;
		bFakeData[1] = wWidth & 0xff;
		bFakeData[2] = (wHeight >> 8) & 0xff ;
		bFakeData[3] = wHeight & 0xff;
		*hData = bFakeData;
		*pData_residue = 4;

		break;		

	default:
		MP_ALERT("%s UnKnow FunctionCode 0x%x", __FUNCTION__, pVendorCmd->bFunctionCode);
		break;
    }

    return ret;
}


WORD VendorTecoDataOut(BYTE* pVendorData, DWORD dwDataSize, BYTE lun, WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg    = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    USB_TECO_CMD* pVendorCmd;
    DWORD dwCbwcbOffset =0, dwCbwcbTransferLen = 0, dwCbwcbTotalSize = 0;    
    WORD    ret = STS_GOOD, wRet = FALSE;
    DWORD dwTotalDataSize, dwCurOffset;
    WORD wFifoDataSize;
    volatile BYTE bIntLevel2;  

    pVendorCmd = (USB_TECO_CMD*)&(psUsbDevMsdc->psCbw->u8CB[0]);
    if(pVendorCmd->bFunctionCode == FC_SEND_DATA)
    {
        dwCbwcbOffset = BYTE_TO_DWORD(pVendorCmd->bData[0], pVendorCmd->bData[1], pVendorCmd->bData[2], pVendorCmd->bData[3]);
        dwCbwcbTransferLen = BYTE_TO_DWORD(pVendorCmd->bData[4], pVendorCmd->bData[5], pVendorCmd->bData[6], pVendorCmd->bData[7]);
        dwCbwcbTotalSize = BYTE_TO_DWORD(pVendorCmd->bData[8], pVendorCmd->bData[9], pVendorCmd->bData[10], pVendorCmd->bData[11]);    
        dwTotalDataSize = dwDataSize;
        dwCurOffset = 0;

        while(dwTotalDataSize > 0)
        {
            if (UsbOtgDeviceCheckIdPin(eWhichOtg) == FALSE)
            {
                MP_ALERT("-W- %s Disconnect while receive data", __FUNCTION__);
                return STS_CMD_FAILED;
            }
        
            bIntLevel2 = mUsbOtgIntSrc1Rd() &~ mUsbOtgIntSrc1MaskRd();
            if ((bIntLevel2 & BIT3) || (bIntLevel2 & BIT2))
            {            
                wFifoDataSize = mUsbOtgFIFOOutByteCount(FIFO_BULK_OUT);
                wRet = bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,(BYTE*)((DWORD)psUsbDevMsdc->pbTecoDataBuff + dwCbwcbOffset + dwCurOffset), wFifoDataSize, eWhichOtg);
                if(!wRet)
                {
                    MP_ALERT("%s Get Data From FIFO Fail", __FUNCTION__);
                    return STS_CMD_FAILED;
                }
                dwCurOffset += wFifoDataSize;
                dwTotalDataSize -= wFifoDataSize;
            }
        }

        #if SAVE_DATA_TO_SDCARD // Save JPEG File From PC into SD Card        // Save first 512 bytes for test
        if((dwCbwcbOffset+dwDataSize) >= dwCbwcbTotalSize) 
        {
            Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "MSDC", "BIN", psUsbDevMsdc->pbTecoDataBuff, dwCbwcbTotalSize);
        }
        #endif



    }
    else
    {
        ret = STS_CMD_FAILED;
    }

    return ret;
}


#endif  // (SC_USBDEVICE && USBOTG_DEVICE_PROTECTION)

