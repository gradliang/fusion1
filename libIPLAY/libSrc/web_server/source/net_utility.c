#include <time.h>      /* for time_t definition                       */
#include "UtilTypeDef.h"


extern DWORD GetSysTime();

/* 
 * Implementation of the C run-time library function, time()
 */
time_t net_Time(time_t *timer)
{
    time_t tm;
    tm = (time_t) (GetSysTime() / 1000);
    if (timer)
        *timer = tm;
    return tm;
}


/* $Id$ */
