/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include <time.h>


#include "xpgTimer.h"

//---------------------------------------------------------------------------------
extern "C"
{
	void _trace_Time__(DWORD id, BYTE const *msg)
	{
		static DWORD t1;
		DWORD t2;

		      t2 = timeGetTime();
		if    (PRINT_TIME & id)
		{
			TRACE2("%s %d\n", msg, t2 - t1);
		}
		t1 = t2;
	}
}

//---------------------------------------------------------------------------------
///
///@ingroup xpgTimer
///@brief   Sleep some time
///
///@param   microsecond - time unit
///
void xpgSleep(DWORD microsecond)
{
	unsigned DWORD t = timeGetTime() + (microsecond / 1000);

	while (timeGetTime() < t);
}

//---------------------------------------------------------------------------------
#ifdef _LINUX
unsigned DWORD timeGetTime()
{
	struct timeval t;
	unsigned DWORD u = 0;

	//unsigned DWORD u1;

	if (gettimeofday(&t, NULL) == 0)
	{
		u = t.tv_usec / 1000 + (t.tv_sec * 1000);
		//if (u > 10) assert(u > u1);
		//MP_DPF("%ld\n", u1);
		//u = u1;
	}

	return u;
}
#endif
//-----------------------------------------------------------------------------------
void xpgWait(BYTE * s)
{
	MP_DPF("wait %s\n", s);
	getchar();
}

//-----------------------------------------------------------------------------------
///
///@ingroup xpgTimer
///@brief   same as xpgSleep
///
///@param   millisecond - unit 
///
void xpgDelay(DWORD millisecond)
{
	//unsigned t = timeGetTime() + millisecond;
	//while (timeGetTime() < t) {   }   
	xpgSleep(millisecond * 1000);
}
