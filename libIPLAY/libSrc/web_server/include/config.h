/*
 * Magic Pixel Inc.
 * Copyright (c) 2008-    
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__


#define WEBS 
#define UEMF 
#define UITRON 
#define WEBS_PAGE_ROM 
//#define WEBS_KEEP_ALIVE_SUPPORT
#define __NO_FCNTL 1

#define WEBS_BREAK()           __asm("break 100")
#define WEBS_DEBUGF(_d_, _x_)  MP_DEBUG _x_
#define time(a)                net_Time(a)


#endif /* __CONFIG_H__ */
