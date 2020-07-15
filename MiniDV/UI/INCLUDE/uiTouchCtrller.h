#ifndef __UITC_H__
#define __UITC_H__

#define PAGE_RECORD_SIZE           						6
#define KEYBOARD_INPUT_CHAR_LENTH           10

void TouchCtrllerInit(void);
void uiTouchMsgReceiver(void);
void uiDispatchTouchSprite(WORD x1, WORD y1);

void (*dialogOnClose)();
void Dialog_ElectrodeEnable_Path_OnClose();
void Dialog_Electrode_Enable_Process_OnClose();

#endif






