
#ifndef __NETKEYPAD_H
#define __NETKEYPAD_H
#include "corelib.h"

/*
// Include section
*/

/*
//Variable declaration
*/
#if Make_WPS
#define KEY_PAD_TOTAL           57
#else
#define KEY_PAD_TOTAL           56
#endif

#define KEY_PAD_LEFT            51
#define KEY_PAD_RIGHT           53
#define KEY_PAD_UP              38
#define KEY_PAD_DOWN            52
#define KEY_PAD_CLEAR           13

#define KEY_PAD_SHIFT           40 // A/a : Upper/Lower case toggle

#define KEY_PAD_Next            54 // page 'WifiKeySetup'
#define KEY_PAD_Pre             54 // page 'DHCPSetup', 'YouTubeLogin'

#define KEY_PAD_Back            55 // page 'WiFiKeySetup'
#define KEY_PAD_WirelessStart   55 // page 'DHCPSetup'
#define KEY_PAD_OK              55 // page 'YouTubeLogin', 'UserEdit2', 'UserEdit3', 'SnapFishLogin'

#define KEY_PAD_WPS             56 

#define MAX_KEYPAD_TEXT         128 // in KeyPad.c

typedef struct
{
    BYTE *pbChar;                   // the pointer of character to display on char area
    BYTE bData[MAX_KEYPAD_TEXT];    // save the input char
    void (*ActionEnter)(void);      // the handle of enter key
    void (*ActionExit)(void);       // the handle of exit key
    WORD wDataIndex;                // the offset in bData
    WORD wIndex;                    // key selected
} ST_KEY_PAD;

/*
//Macro declaration
*/


/*
//Function prototype
*/


#endif // __NETKEYPAD_H

