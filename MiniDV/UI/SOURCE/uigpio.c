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
* Filename      : uigpio.c
* Programmer(s) : TY Miao
* Created       :
* Descriptions  :
*
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "ui.h"
#include "taskid.h"
#include "devio.h"
#include "os.h"
#include "xpg.h"
#include "..\..\main\include\setup.h"
#include "..\..\main\include\ui_timer.h"
#include "peripheral.h"
#include "uiGpio.h"
#include "..\..\main\include\MiniDV_StateMachine.h"

//////////////////////////////////////////////////////////////////////
//
// Structure declarations
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Variable declarations
//
//////////////////////////////////////////////////////////////////////
static volatile BYTE curKeyCode = KEY_NULL;

//////////////////////////////////////////////////////////////////////
//
// Function prototype
//
//////////////////////////////////////////////////////////////////////

void Ui_SetKey(BYTE keyCode)
{
    curKeyCode = keyCode;
}


BYTE Ui_GetKey(void)
{
    return curKeyCode;
}


void Ui_UsbdSetMode(WHICH_OTG eWhichOtg)
{
#if SC_USBDEVICE

    #if USBDEVICE_CDC_DEBUG

    mpDebugPrint("-USBOTG%d- %s CDC MODE", eWhichOtg, __FUNCTION__);
    Api_UsbdSetMode(USB_AP_CDC_MODE, eWhichOtg);

    #else

   
    MP_ALERT("- g_psSetupMenu->bUsbdMode = %d -", g_psSetupMenu->bUsbdMode);    
     
    switch (g_psSetupMenu->bUsbdMode)
    {
        case SETUP_MENU_USBD_MODE_SIDC:
            mpDebugPrint("-USBOTG%d- %s SIDC MODE", eWhichOtg, __FUNCTION__);
            Api_UsbdSetMode(USB_AP_SIDC_MODE, eWhichOtg);
            break;

        case SETUP_MENU_USBD_MODE_EXTERN:
            mpDebugPrint("-USBOTG%d- %s SideMonitor", eWhichOtg, __FUNCTION__);
            Api_UsbdSetMode(USB_AP_EXTERN_MODE, eWhichOtg);
            break;

		case SETUP_MENU_USBD_MODE_UAVC:
            mpDebugPrint("<set USB_AP_UAVC_MODE>");
            Api_UsbdSetMode(USB_AP_UAVC_MODE, eWhichOtg);
            break;

        case SETUP_MENU_USBD_MODE_MSDC:
        default:
            mpDebugPrint("-USBOTG%d- %s MSDC MODE", eWhichOtg, __FUNCTION__);
            Api_UsbdSetMode(USB_AP_MSDC_MODE, eWhichOtg);
            break;
    }

    #endif

#endif
}

void Ui_UsbdDetectEvent(WHICH_OTG eWhichOtg)
{
#if (SC_USBDEVICE)
    static BYTE g_bBakUsbdMode = 0;

    if (Api_UsbdGetDetectPlugFlag(eWhichOtg) == 1)
    {
        MP_DEBUG("### %s: (Api_UsbdGetDetectPlugFlag(eWhichOtg = %d) == 1)", __FUNCTION__, eWhichOtg);

        if (g_psSetupMenu->bUsbdMode != SETUP_MENU_USBD_MODE_SIDC)
        {
            xpgStopAllAction (); //Jasmine 6/20: stop all actions while usbd plug
        }

        if (g_psSetupMenu->bUsbdMode == SETUP_MENU_USBD_MODE_EXTERN)
        {
            #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
            SideMonitorIn();
            #endif
        }

        if (g_psSetupMenu->bUsbdMode == SETUP_MENU_USBD_DYNAMIC)
        {
            g_bBakUsbdMode = SETUP_MENU_USBD_DYNAMIC; //Jasmine 6/21: keep flag
        }
        else
        {
            Ui_UsbdSetMode(eWhichOtg);//Jasmine 6/20: set usbd mode
            Api_UsbdInit(eWhichOtg); //init usbd
        }
    }
    else
    {
        MP_DEBUG("### %s: (Api_UsbdGetDetectPlugFlag(eWhichOtg = %d) == 0)", __FUNCTION__, eWhichOtg);

        if (g_psSetupMenu->bUsbdMode == SETUP_MENU_USBD_MODE_EXTERN)
        {
            #if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
            SideMonitorOut();
            #endif
        }

        Api_UsbdFinal(eWhichOtg);

        if (g_bBakUsbdMode == SETUP_MENU_USBD_DYNAMIC) //Jasmine 6/21:reset flag to dynamic
            g_psSetupMenu->bUsbdMode = SETUP_MENU_USBD_DYNAMIC;

        //g_bUsbdRetryCount = 0;
        g_bBakUsbdMode = 0;  //reset flag

        if (Api_UsbdGetMode(eWhichOtg) == USB_AP_MSDC_MODE)
        {
            MP_DEBUG("### %s: (Api_UsbdGetMode(eWhichOtg = %d) == USB_AP_MSDC_MODE)", __FUNCTION__, eWhichOtg);
            g_psSystemConfig->dwCurrentOpMode = OP_USBD_MSDC_MODE;
            SystemSysDriveLunEnable(DISABLE);
            RenewAllDrv();
            FileBrowserResetFileList(); // abel 2006.08.23
#if MAKE_XPG_PLAYER
            xpgClearCatch();
            ClrBrowserDrvId(); /* clear g_stBrowser[i].bCurDriveId value, thus xpgChangeMenuMode() can re-scan file list */
#endif
        }
    }
#endif
}

#if GPIO_SIMPLE_KEY_ENABE
void	 SimpleGpioKeyScan()
{
	BYTE bKeycode=KEY_NULL;

	if (GetGPIOValue(AGPIO|GPIO_00))
		bKeycode=KEY_LEFT;
	else if (GetGPIOValue(AGPIO|GPIO_01))
		bKeycode=KEY_RIGHT;
	else if (GetGPIOValue(AGPIO|GPIO_02))
		bKeycode=KEY_ENTER;
	else if (GetGPIOValue(AGPIO|GPIO_03))
		bKeycode=KEY_ENTER;

	if (curKeyCode!=bKeycode)
	{
		curKeyCode=bKeycode;
		mpDebugPrint("curKeyCode=%d",curKeyCode);
		if (curKeyCode)
			EventSet(UI_EVENT, EVENT_GPIO_KEY);
	}

}
#endif


