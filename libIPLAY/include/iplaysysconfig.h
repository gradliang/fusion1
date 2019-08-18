#ifndef __IPLAY_SYS_CONFIG_H__
#define __IPLAY_SYS_CONFIG_H__

#include "flagdefine.h"

#define ___PLATFORM___ 0x662001

#if (___PLATFORM___ == 0x663)
    #include "../../MiniDV/include/Platform_MP663.h"           // MP663
#elif (___PLATFORM___ == 0x6635642)
    #include "../../MiniDV/include/Platform_MP663_DV.h"        // MP663 with OV5642
#elif (___PLATFORM___ == 0x662)
    #include "../../MiniDV/include/Platform_MP662W.h"		   // MP 662W
#elif (___PLATFORM___ == 0x662001)
    #include "../../MiniDV/include/Platform_MP662W_WELD.h"		   // MP 662W
#elif (___PLATFORM___ == 0x662002)
    #include "../../MiniDV/include/Platform_MP662W_SFDT.h"		   // MP 662W
#elif ___PLATFORM___ == 0x663007
    #include "../../MINIDV/include/Platform_MP663_SpyCamera.h"   // 
#else
    #include "../../MINIDV/include/Platform_MP663.h"           // default: MP663
#endif

#if (CHIP_VER_MSB == CHIP_VER_660)
    #if (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_B)
        #undef MP_TRACE_DEBUG_PORT
        #define MP_TRACE_DEBUG_PORT         DEBUG_PORT_NONE
    #endif
#endif

#endif //__IPLAY_SYS_CONFIG_H__

