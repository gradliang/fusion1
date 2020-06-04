#ifndef __UITC_H__
#define __UITC_H__

#define PAGE_RECORD_SIZE           						5
#define KEYBOARD_INPUT_CHAR_LENTH           10

extern DWORD g_dwRecordListCurrPage;

void TouchCtrllerInit(void);
void uiTouchMsgReceiver(void);
void uiDispatchTouchSprite(WORD x1, WORD y1);

void (*dialogOnClose)();

#endif






