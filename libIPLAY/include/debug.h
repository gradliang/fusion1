#ifndef DEBUG_H
#define DEBUG_H

#include "UtilTypeDef.h"

#include "mpTrace.h"

#define BREAK_POINT200()      //__asm__("break 200")
#define BREAK_POINT300()      //__asm__("break 300")


#define CMD_PREAMBLE			0x5a00	

#define	DEBUG_CMD_SET_ENTRY		( CMD_PREAMBLE + 0 )
#define DEBUG_CMD_WRITE_MEM		( CMD_PREAMBLE + 1 )
#define DEBUG_CMD_READ_MEM		( CMD_PREAMBLE + 2 )
#define DEBUG_CMD_RUN				( CMD_PREAMBLE + 3 )
#define DEBUG_CMD_LOAD			( CMD_PREAMBLE + 4 )



	#define DEBUG_INIT()

	#define DEBUG_OUT_TEXT(x)		
	#define DEBUG_OUT_UNI_TEXT(x)	
	#define DEBUG_OUT_U32(x)		

	#define DEBUG_TEXT(x)			
	#define DEBUG_UTEXT(x)			
	#define DEBUG_VALUE(x, y)		
	#define DEBUG_CR()				

	#define DTEXT(x)		//DpString(x);
	#define DUTEXT(x)				
	#define DVALUE(x, y)	 	
#ifdef DSHOW
#undef DSHOW
#endif		
	#define DSHOW(x, y, z)	 
						

    #define DEBUG_CR()		
	#define DEBUG_BREAK()			
	#define DEBUG_LED(x)	

	#define MTEXT(x)	
    #define MVALUE(x)		

	#define ASSERT(x)

#define CHK_MALLOC(s, msg)	//if (!(s)) { BREAK_POINT();} 

#define printf(fmt, ...)
#define fprintf(fd, fmt, ...)
#define mp_msg(mod,lev, args... )
#define mp_dbg(mod,lev, args... )

#define DebugBreak()	    // BREAK_POINT()

#endif // DEBUG_H


