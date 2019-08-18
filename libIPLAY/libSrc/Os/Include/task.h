#ifndef TASK_H
#define TASK_H

SDWORD TaskCreate(BYTE id, void *entry, BYTE priority, DWORD stacksize);
SDWORD TaskStartup(BYTE id,...);
void TaskTerminate(BYTE id);
BYTE TaskGetId(void);
#endif
