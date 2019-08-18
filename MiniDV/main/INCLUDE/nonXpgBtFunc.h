#ifndef __NONXPGBTFUNC_H__
#define __NONXPGBTFUNC_H__


// setup utility functions
void nonXpgBtSetSearchedFlag(BYTE);
BYTE nonXpgBtGetSearchedFlag(void);

/*
void nonXpgBtDrawOsdSetup(SETUP*);
BYTE nonXpgBtSetupCntVisableSetup(SETUP*);
void nonXpgBtSetupSetSetupVisableCnt(SETUP*);
void nonXpgBtSetupEventDown(SETUP*);
void nonXpgBtSetupEventUp(SETUP*);
void nonXpgBtSetupEventEnter(SETUP*);
void nonXpgBtSetupEventExit(SETUP*);
*/


// Bt setup
void nonXpgBtSetupMenu(void);

void nonXpgBtMyDevSetupMenu(void);
void nonXpgBtMyDeviceUp(void);
void nonXpgBtMyDeviceDown(void);
void nonXpgBtMyDeviceSrch(void);
void nonXpgBtMyDeviceDel(void);
void nonXpgBtMyDeviceDelAll(void);
void nonXpgBtSendTo(void);



#endif
