#ifndef BTBROWSER_H
#define BTBROWSER_H

STREAM * BtBrowserFileCreate(WORD * pwSname);
void BtBrowserFileClose(STREAM *phandle);
SWORD BtCheckSpace(DWORD size);
#endif
