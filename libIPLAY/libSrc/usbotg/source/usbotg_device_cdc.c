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
* Filename      : usbotg_device_cdc.c
* Programmer(s) : Calvin Liao
* Created       : 
* Descriptions  : From usbd_cdc.c
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
#include "Usbotg_device.h"
#include "usbotg_device_cdc.h"

#if SC_USBDEVICE
/*
// Constant declarations
*/

/*
// Structure declarations
*/


/*
// Variable declarations
*/

/*
// External Variable declarations
*/
//extern BYTE* gpBulkTxRxData;


/*
// Macro declarations
*/

/*
// Static function prototype
*/
static void UsbdCdcBufferEnqueue(WHICH_OTG eWhichOtg);
static void UsbdCdcBufferDequeue(WHICH_OTG eWhichOtg);
static BOOL UsbdCdcIsBufferFull(WHICH_OTG eWhichOtg);
static BOOL UsbdCdcIsBufferEmpty(WHICH_OTG eWhichOtg);
static DWORD UsbdCdcGetBufferWriteIdx(WHICH_OTG eWhichOtg);
static DWORD UsbdCdcGetBufferReadIdx(WHICH_OTG eWhichOtg);

/*
// Definition of internal functions
*/
void UsbCdcInit(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_DESC psDevDesc = (PUSB_DEVICE_DESC)UsbOtgDevDescGet(eWhichOtg);
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);
    psDevDesc->pbBulkTxRxData = (BYTE*)((DWORD)UsbOtgBufferGet(eWhichOtg)| 0xa0000000);//((DWORD)psUsbOtg->dwUsbBuf | 0xa0000000);
    psDevCdc->boIsAbleToUseUsbdCdc = FALSE;
    //geUsbTransactionState = STATE_CBW;
    //USBDeviceInfSetup();
}

BOOL GetIsAbleToUseUsbdCdc(WHICH_OTG eWhichOtg)
{
    //if (UsbOtgDeviceCheckIdPin(eWhichOtg) == FALSE) // disconnect
    //    gIsAbleToUseUsbdCdc = FALSE;

    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);
    return psDevCdc->boIsAbleToUseUsbdCdc;
}

void SetIsAbleToUseUsbdCdc(BOOL flag, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);
    psDevCdc->boIsAbleToUseUsbdCdc = flag;
}

void UsbPutChar(BYTE data, WHICH_OTG eWhichOtg)
{ // DATA IN
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
  	DWORD dwTimeOutCount = 0x100000;
    volatile DWORD level_1, level_2;
    BYTE      intflag = 0;
    DWORD     dwIcUsbOtg;

    dwIcUsbOtg = UsbOtgIntCauseGet(eWhichOtg);
    if (dwIcUsbOtg == 0)
        return;

    if((isr->MiMask & dwIcUsbOtg) == 0)
    {
        SystemIntEna(dwIcUsbOtg);
        intflag = 1;
    }
    
    while (dwTimeOutCount--)
    {
        //level_1 = mUsbOtgIntGroupRegRd(); //&~ mUsbOtgIntGroupMaskRd();
        //if (level_1 & BIT1)
        //{
            level_2 = mUsbOtgIntSrc1Rd();
            if (level_2 & BIT16) // fifo0 is ready to be written
            {
                bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN, &data, 1,eWhichOtg);
                mUsbOtgFIFODone(FIFO_BULK_IN);
                break;
            }
                
        //}
    }
    //vMyUsbAPWrToFifo(&data, 1, FIFO_BULK_IN);
    //mUsbFIFODone(FIFO_BULK_IN);
    
    if(intflag)
        SystemIntDis(dwIcUsbOtg);

}

//extern ACTION      geUsbCxFinishAction;
BYTE UsbGetChar(WHICH_OTG eWhichOtg)  /*Need*/ // Not to verify
{ // DATA OUT
    PST_USB_OTG_DES psUsbOtg    = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev    = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_CDC psDevCdc    = &psDev->sCdc;    
    PUSB_DEVICE_DESC psDevDesc  = &psDev->sDesc;
    
    volatile BYTE level_1, level_2;
  	DWORD dwTimeOutCount = 0x100000;
	INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    BYTE      intflag = 0;
    BYTE    byte_return = 0;
    //static WORD total_byte_cnt = 0;
    //static WORD byte_index = 0;
    DWORD     dwIcUsbOtg;

    dwIcUsbOtg = UsbOtgIntCauseGet(eWhichOtg);
    if (dwIcUsbOtg == 0)
        return;

    if((isr->MiMask & dwIcUsbOtg) == 0)
    {
        SystemIntEna(dwIcUsbOtg);
        intflag = 1;
    }

    //UartOutText("O");
    if (psDevCdc->wTotalByteCnt == 0)
    {
        while (dwTimeOutCount--)
        {
            level_1 = mUsbOtgIntGroupRegRd(); //&~ mUsbOtgIntGroupMaskRd();
            if (level_1 & BIT0)
            {
                level_2 = mUsbOtgIntSrc0Rd();
                if (level_2 & BIT0)
                {
                    //MP_DEBUG("SETUP");
                    vOTG_ep0setup(eWhichOtg);
                }

                if (level_2 & BIT2)
                {
                    vOTG_ep0rx(eWhichOtg);
                }

                if (psDev->eUsbCxFinishAction == ACT_STALL)
                {
                    mUsbOtgEP0StallSet();
                }
                else if (psDev->eUsbCxFinishAction == ACT_DONE)
                {
                    mUsbOtgEP0DoneSet();
                }

                psDev->eUsbCxFinishAction = ACT_IDLE; // Clear ACTION
            }
            
            if (level_1 & BIT1)
            {
                level_2 = mUsbOtgIntSrc1Rd();
                if (level_2 & BIT3 || level_2 & BIT2)
                {
                   //UartOutText("<G>");
                   psDevCdc->wTotalByteCnt = mUsbOtgFIFOOutByteCount(FIFO_BULK_OUT);
                   bOTGCxFxWrRd(FOTG200_DMA2FIFO1, DIRECTION_OUT, psDevDesc->pbBulkTxRxData, psDevCdc->wTotalByteCnt, eWhichOtg);
                   break;//return *gpBulkTxRxData;
                }
            }
        }
    }
    else
    {
        //UartOutText("<g>");
        byte_return = *(psDevDesc->pbBulkTxRxData + psDevCdc->wByteIndex);
        psDevCdc->wByteIndex++;
        if (psDevCdc->wByteIndex == psDevCdc->wTotalByteCnt)
        {
           // UartOutText("<F>");
            psDevCdc->wByteIndex = 0;
            psDevCdc->wTotalByteCnt = 0;
        }

        return byte_return;
    }

    

    if(intflag)
        SystemIntDis(dwIcUsbOtg);
    
    return 0;
    
/*
   // vMyUsbAPRdFromFifo(gpBulkTxRxData, 1);
    BYTE data = 0;

    if (UsbdCdcIsBufferEmpty() == TRUE)
    {
        MP_DEBUG("Need To Wait");
    }
    
    data = *(gpBulkTxRxData + UsbdCdcGetBufferReadIdx());
    UsbdCdcBufferDequeue();
    return data;
    */
}

/*
// Definition of local functions 
*/
static DWORD UsbdCdcGetBufferWriteIdx(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);   
    return psDevCdc->dwCdcBulkWriteIdx;
}

static DWORD UsbdCdcGetBufferReadIdx(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);    
    return psDevCdc->dwCdcBulkReadIdx;
}

static void UsbdCdcBufferEnqueue(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);   
    psDevCdc->dwCdcBulkWriteIdx++;
}

static void UsbdCdcBufferDequeue(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);    
    psDevCdc->dwCdcBulkReadIdx++;
}

static BOOL UsbdCdcIsBufferFull(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);
    
    if (psDevCdc->dwCdcBulkWriteIdx == (psDevCdc->dwCdcBulkReadIdx - 1))
    {
        MP_DEBUG("Buffer Full");
        return TRUE;
    }
    
    return FALSE;
}

static BOOL UsbdCdcIsBufferEmpty(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_CDC psDevCdc = (PUSB_DEVICE_CDC)UsbOtgDevCdcGet(eWhichOtg);
    
    if (psDevCdc->dwCdcBulkReadIdx == psDevCdc->dwCdcBulkWriteIdx)
    {
        MP_DEBUG("Buffer Empty");
        return TRUE;
    }
    
    return FALSE;
}

/*
void UsbPutString(BYTE data)
{

}

BYTE UsbGetString(void)
{

}
*/

/*
// Definition of local functions 
*/



#endif  // SC_USBDEVICE

