/*
 * =====================================================================================
 *
 *       Filename:  iplay.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/18/2012 6:48:37 PM 台北標準時間
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  first_name last_name (fl), fl@my-company.com
 *        Company:  my-company
 *
 * =====================================================================================
 */

#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"

int m_wait_driver_ready(void)
{
	DWORD dwNWEvent;  
	return (int)EventWait(WPA_EVENT, 0x1, OS_EVENT_OR, &dwNWEvent);
}
