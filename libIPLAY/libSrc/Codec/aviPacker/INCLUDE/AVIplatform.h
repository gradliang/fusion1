#ifndef AVIPLATFORM

#define AVIPLATFORM
//#define AVIPLATFORM_PC
#define AVIPLATFORM_MPX

#ifdef AVIPLATFORM_PC
    #include "AVIpc.h"
#else
    #include "AVImpx.h"
#endif

#endif
