#ifndef SEMAPHORE_H
#define SEMAPHORE_H

SDWORD SemaphoreCreate(BYTE id, BYTE attr, BYTE count);
SDWORD SemaphoreDestroy(BYTE id);
SDWORD SemaphoreRelease(BYTE id);
SDWORD SemaphoreWait(BYTE id);
SDWORD SemaphorePolling(BYTE id);
#endif
