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
* Filename      : usbotg_device_protection.c
* Programmer(s) : Calvin Liao
* Created       :
* Descriptions  : For USB PenDrive Protection
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
#include "usbotg_device_protection.h"
#include "os.h"


#if (SC_USBDEVICE && USBOTG_DEVICE_PROTECTION)
/*
// Constant declarations
*/
#define VENDOR_DATA_SIZE   USB_PROTECTION_DATA_MAX_SIZE

enum
{
	FUNCTION_CODE_00 = 0x00 ,
	FUNCTION_CODE_01 = 0x01 ,
	FUNCTION_CODE_02 = 0x02
};

enum
{
	DATA_CODE_NOTHING              = 0x1A ,
	DATA_CODE_SYNC_DATE          = 0x28 ,
	DATA_CODE_PASSWORD           = 0x30,
	DATA_CODE_PASSWORD_CHG  = 0x40,
	DATA_CODE_PARTITION          = 0xff,
};


/*
// Structure declarations
*/


/*
// Variable declarations
*/
BYTE bData[VENDOR_DATA_SIZE];


/*
// External Variable declarations
*/


/*
// Macro declarations
*/

/*
// Static function prototype
*/
static BOOL (*VendorProtectionPasswordSet) (BYTE* pbString, DWORD dwLen);    // Set
static BOOL (*VendorProtectionPasswordCmp) (BYTE* pbString, DWORD dwLen);  // Compare
static BOOL (*VendorProtectionPatitionSwitch) (BYTE bPartition);

/*
// Definition of internal functions
*/
void VendorGetString(BYTE* pbString, DWORD* pdwLen, BYTE* pbUsbString, DWORD dwUsbLen)
{
    DWORD dwTmpLen = 0;
    (*pdwLen) = 0;

    for(dwTmpLen = 1 ; dwTmpLen <=  dwUsbLen; dwTmpLen+=2)
    {
        if(pbUsbString[dwTmpLen - 1] == 0x00 && pbUsbString[dwTmpLen] == 0x00)
            break;

        pbString[(*pdwLen)]  = pbUsbString[dwTmpLen];
        (*pdwLen) += 1;
    }

    MP_DEBUG("\r\n%s String Len:%d", __FUNCTION__, *pdwLen);
}

BYTE AsciiToNumber(BYTE bAscii)
{
    if(bAscii < 0x30 || bAscii > 0x3a)
    {
        MP_ALERT("-W- not number : Ascii(%x)", bAscii);
        return 0;
    }

    return (bAscii &=~0x30);
}

void VendorProtectionDataDump(USB_PROTECTION_DATA* pVendorProtect, DWORD dwDataSize)
{
    DWORD dwCnt = 0;
    
    if (pVendorProtect->DataCode == DATA_CODE_NOTHING) return;
    
    MP_DEBUG("\r\nData Size : %d", dwDataSize);
    MP_DEBUG("OperCode = 0x%x", pVendorProtect->DataCode);
    for (dwCnt = 0; dwCnt < (dwDataSize - 2); dwCnt++)
        MP_DEBUG("Data[%02d] = 0x%x", dwCnt, pVendorProtect->Data[dwCnt]);
}

WORD VendorProtectionCmd(BYTE** hData, USB_PROTECTION_CMD* pVendorProtect, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg)
{
    WORD    ret = STS_GOOD;

    memset(bData, 0, VENDOR_DATA_SIZE);

    switch (pVendorProtect->FunctionCode)
    {
	case FUNCTION_CODE_00:

		*hData = (BYTE*) bData;
		*pData_residue = 2;

		break;

	case FUNCTION_CODE_01:

		MP_DEBUG("FUNCTION_CODE_01");

		bData[0] = 0x00;
		bData[1] = 0x00;
		bData[2] = 0x00;
		bData[3] = 0x7B;
		bData[4] = 0x6E;
		bData[5] = 0x08;
		bData[6] = 0x00;
		bData[7] = 0x79;
		bData[8] = 0x8C;
		bData[9] = 0x40;
		bData[10] = 0x00;
		bData[11] = 0x50;

		*hData = (BYTE*) bData;
		*pData_residue = 12;

		break;

	case FUNCTION_CODE_02:

		*hData = (BYTE*) bData;
		*pData_residue = 3;  // TEST

		break;

	default:
		MP_ALERT("%s UnKnow FunctionCode 0x%x", __FUNCTION__, pVendorProtect->FunctionCode);
		break;
    }

    return ret;
}


WORD VendorProtectionData(USB_PROTECTION_DATA* pVendorProtect, DWORD dwDataSize, BYTE lun, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_MSDC psUsbDevMsdc = (PUSB_DEVICE_MSDC)UsbOtgDevMsdcGet(eWhichOtg);
    WORD    ret = STS_GOOD;
    BYTE    bString[USB_PROTECTION_DATA_MAX_SIZE] = {0};    
    DWORD   dwDataLen = 0;
    ST_SYSTEM_TIME stNewTime; 

    if (dwDataSize > USB_PROTECTION_DATA_MAX_SIZE) // avoid data over array
    {
        MP_ALERT("-E- %s over data max size(%d) : %d", __FUNCTION__, USB_PROTECTION_DATA_MAX_SIZE, dwDataSize);
        __asm("break 100");
    }

    psUsbDevMsdc->psCsw->u8Status = STS_GOOD;    

    switch (pVendorProtect->DataCode)
    {
        case DATA_CODE_NOTHING:
            // Nothing
            break;

        case DATA_CODE_SYNC_DATE:
            
            MP_ALERT("\r\n<Sync Date/Time>");

            VendorGetString(bString, &dwDataLen, &pVendorProtect->Data[0], dwDataSize - sizeof(pVendorProtect->DataCode));

            stNewTime.u16Year = AsciiToNumber(bString[0])*1000+AsciiToNumber(bString[1])*100+AsciiToNumber(bString[2])*10+AsciiToNumber(bString[3]);
            stNewTime.u08Month = AsciiToNumber(bString[4])*10+AsciiToNumber(bString[5]);
            stNewTime.u08Day = AsciiToNumber(bString[6])*10+AsciiToNumber(bString[7]);
            stNewTime.u08Hour = AsciiToNumber(bString[8])*10+AsciiToNumber(bString[9]);
            stNewTime.u08Minute = AsciiToNumber(bString[10])*10+AsciiToNumber(bString[11]);
            stNewTime.u08Second = AsciiToNumber(bString[12])*10+AsciiToNumber(bString[13]);
            SystemTimeSet(&stNewTime);
            
            break;

        case DATA_CODE_PASSWORD:
            
            MP_ALERT("\r\n<Enter Password>");

            VendorGetString(bString, &dwDataLen, &pVendorProtect->Data[0], dwDataSize - sizeof(pVendorProtect->DataCode));

            if(VendorProtectionPasswordCmp)  
            {
                if(!((VendorProtectionPasswordCmp)(bString, dwDataLen)))
                    psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;  // Send Error to AP
            }
            else
            {
                psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;  // Send Error to AP          
            }
                
            break;

        case DATA_CODE_PASSWORD_CHG:
            
            MP_ALERT("\r\n<Change Password>");

            VendorGetString(bString, &dwDataLen, &pVendorProtect->Data[0], dwDataSize - sizeof(pVendorProtect->DataCode));            
            
            if(VendorProtectionPasswordSet)    
            {                
                if(!((VendorProtectionPasswordSet)(bString, dwDataLen)))
                    psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;  // Send Error to AP
            }
            else
            {
                psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;  // Send Error to AP          
            }
                                     
            break;

        case DATA_CODE_PARTITION:
            
            if (pVendorProtect->Data[0] == 2)
            {
                MP_ALERT("\r\n<Change Partition>");
                
                if(VendorProtectionPatitionSwitch)      
                {                    
                    if(!((VendorProtectionPatitionSwitch)(pVendorProtect->Data[0])))
                        psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;  // Send Error to AP
                }
                else
                {
                    psUsbDevMsdc->psCsw->u8Status = STS_CMD_FAILED;  // Send Error to AP                    
                }
            }
            
            break;

        default:
            MP_ALERT("%s UnKnow DataCode 0x%x", __FUNCTION__, pVendorProtect->DataCode);
            break;
    }

    VendorProtectionDataDump(pVendorProtect, dwDataSize);  // For Debug

    return ret;
}

void Api_UsbdProtectCBFunc(BOOL (*VendorPasswordSet) (BYTE* pbString, DWORD dwLen),
                                                        BOOL (*VendorPasswordCmp) (BYTE* pbString, DWORD dwLen),
                                                        BOOL (*VendorPatitionSwitch) (BYTE bPartition))
{
    VendorProtectionPasswordSet = VendorPasswordSet;    // Set
    VendorProtectionPasswordCmp = VendorPasswordCmp;  // Compare
    VendorProtectionPatitionSwitch = VendorPatitionSwitch;
}

#endif  // (SC_USBDEVICE && USBOTG_DEVICE_PROTECTION)

