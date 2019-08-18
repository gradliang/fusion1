#include "fs.h"

#define MPX_NONCACHE(a)    ((DWORD)(a) | 0xA0000000)
#define AVIENDIAN_PAD(a)   ((a) << 24) 
#define AVIENDIAN_CHG
