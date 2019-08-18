#ifndef EVENT_H
#define EVENT_H
SDWORD EventCreate(BYTE id, BYTE attr, DWORD flag);
SDWORD EventDestroy(BYTE id);
SDWORD EventSet(BYTE id, DWORD pattern);
SDWORD EventWait(BYTE id, DWORD pattern, BYTE mode, DWORD * release);
#endif
