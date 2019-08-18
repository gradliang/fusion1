#ifndef __XPGTIMER_H
#define __XPGTIMER_H

#ifdef _LINUX
	unsigned DWORD timeGetTime();
#else
	#include <windows.h>
	#include <mmsystem.h>
	
#endif



class xpgTimer
{
  	public:
    	xpgTimer( DWORD resolution );
    	~xpgTimer()		{}
    	
    	void restart();
    	unsigned DWORD elapsed();

  	private:
  		DWORD				m_iResolution;
  		unsigned DWORD	m_uStartTime;

};

// **************************************************************************
inline xpgTimer::xpgTimer( DWORD iResolution )
{
	m_iResolution 	= iResolution;
	m_uStartTime	= 0;
}

// **************************************************************************
inline void xpgTimer::restart()
{
	m_uStartTime = timeGetTime();	
}

// **************************************************************************
inline unsigned DWORD xpgTimer::elapsed()
{	
	unsigned DWORD uCrnTime = timeGetTime();
	if (uCrnTime < m_uStartTime) m_uStartTime = uCrnTime - 1;
		
	return (unsigned DWORD)(uCrnTime - m_uStartTime);// * m_dwResolution / 1000);	
}



void xpgDelay(DWORD millisecond);
void xpgSleep(DWORD microsecond);

#define PRINT_TIME 1

extern "C" {
	
extern void _trace_Time__(DWORD id, const BYTE *msg);
	
#if PRINT_TIME&0xff
	#define traceTime	_trace_Time__
#else
	#ifdef __cplusplus
		#define traceTime static_cast<void>
	#else
		#define traceTime (void)
	#endif
#endif
}
#endif  // #ifndef __XPGTIMER_H
