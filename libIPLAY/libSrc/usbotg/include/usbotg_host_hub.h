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
* Filename		: usbotg_host_hub.h
* Programmer(s)	: Joe Luo (JL)
* Created Date	: 2008/07/07 
* Description	: 
******************************************************************************** 
*/
#ifndef __USBOTG_HOST_HUB_H__
#define __USBOTG_HOST_HUB_H__
/*
// Include section 
*/
#include "utiltypedef.h"

#if (SC_USBHOST==ENABLE)

void UsbOtgHostHubInterruptInActive (WHICH_OTG eWhichOtg);
void UsbOtgHostHubInterruptInIoc (WHICH_OTG eWhichOtg);
void UsbHubGoToNextDevicePointer (WHICH_OTG eWhichOtg);


#endif // SC_USBHOST

#endif /* End of __USBOTG_HOST_HUB_H__ */



