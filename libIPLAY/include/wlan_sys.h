/** @file wlan_sys.h
 *  @brief This header file contains MP5XX/MP6XX OS-related defines.
 *
 *  Copyright Magic Pixel Inc., 2008-
 */

#ifndef _WLAN_SYS_H_
#define _WLAN_SYS_H_

#include "BitsDefine.h"

#define EVENT_WIFI_CMDDONE 	  			BIT0		/* for 88W8686 command done */
#define EVENT_WIFI_TEST    				BIT1		/* for RTL8711s testing */
#define EVENT_WIFI_INTERRUPT			BIT2		/* for 88W8686/RTL8711s interrupt */
#define EVENT_WIFI_TIMER				BIT3		/* for RTL8711s timer */

#endif /* _WLAN_SYS_H_ */
