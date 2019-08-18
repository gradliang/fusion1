#ifndef __UIKEYPAD_C__
#define __UIKEYPAD_C__

#define MAX_XPG_KP_TEXT         32

typedef struct
{
    BYTE *pbChar;                   // the pointer of character to display on char area
    BYTE bData[MAX_XPG_KP_TEXT];    // save the input char
    void (*ActionEnter)(void);      // the handle of enter key
    void (*ActionExit)(void);       // the handle of exit key
    WORD wDataIndex;                // the offset in bData
    WORD wIndex;                    // key selected
} ST_XPG_KEY_PAD;


void xpgKeyPadSetCnt(BYTE);
void xpgKeyPadSetChar(BYTE *);
void xpgKeyPadSetActionEnter(void (*action) (void));
void xpgKeyPadSetActionExit(void (*action) (void));
void xpgKeyPadSetActionUp(void (*action) (void));
void xpgKeyPadSetActionDown(void (*action) (void));
void xpgKeyPadSetActionLeft(void (*action) (void));
void xpgKeyPadSetActionRight(void (*action) (void));
void xpgKeyPadSetIndex(WORD);
void xpgKeyPadStrClear();
void xpgKeyPadStrAddChar(BYTE );
void xpgKeyPadStrDelChar(void);
void xpgKeyPadStrDelData(BYTE *);
void xpgKeyPadStrAddStr(BYTE *);
void xpgKeyPadCpyStr(BYTE *);
void xpgKeyPadSetStr(BYTE *, BYTE);
BYTE xpgKeyPadGetChar(WORD);
void xpgKeyPadActionEnter();
void xpgKeyPadActionExit();
WORD xpgKeyPadGetIndex();
BYTE* xpgKeyPadGetString();
WORD xpgKeyPadGetStrLen(void);
void xpgKeyPadSetTxtLen(BYTE);


#endif


