#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "utiltypedef.h"

typedef enum
{
    STATE_POWER_ON = 0,
    STATE_STANDY_BY,
    STATE_CAMERA_PREVIEW,
    STATE_CAMERA_MODE,
    STATE_CAMCODER_PREVIEW,
    STATE_CAMCODER_MODE,
    STATE_CAMCODER_ERROR,
    STATE_POWER_DOWN_MODE,
    STATE_PC_CAM_MODE,
    STATE_USBD_MSDC_MODE,
    STATE_POWER_DOWN_LOOP,
} E_UI_STATE_MACHINE;

void StateMachine_StateSet(DWORD newState);
void StateMachine_Handler(DWORD dwEvent, BYTE *infoPtr);
E_UI_STATE_MACHINE StateMachine_StateGet(void);
void StateMachine_PowerOnByAdaptor(void);
#endif

