
#ifndef _UITRON_OS_INTERRUPT_H__
#define _UITRON_OS_INTERRUPT_H__

#include "UtilTypeDef.h"

#define MAX_SYSTEM_INTERRUPT_SERVICE        32
#define MAX_SUB_DMA_INTERRUPT_SERVICE       32

typedef void (*OS_INTERRUPT_CALLBACK_FUNC)(void);

void DmaIntEna(DWORD);
void DmaIntDis(DWORD );
void SystemIntEna(DWORD);
void SystemIntDis(DWORD);
SDWORD SystemIntHandleRegister(DWORD, OS_INTERRUPT_CALLBACK_FUNC);
SDWORD SystemIntHandleRelease(DWORD);
SDWORD DmaIntHandleRegister(DWORD, OS_INTERRUPT_CALLBACK_FUNC);
SDWORD DmaIntHandleRelease(DWORD);
void SystemIntInit(void);

#endif

