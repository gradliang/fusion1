#ifndef __UITC_H__
#define __UITC_H__

SDWORD TouchCtrllerCalProcess(WORD, WORD);
void TouchCtrllerInit(void);
void uiTouchCtrllerCallback(void);
void uiTouchMsgReceiver(void);

extern DWORD g_dwRecordListCurrPage;
#define PAGE_RECORD_SIZE           (5)

void (*dialogOnClose)();

#endif






