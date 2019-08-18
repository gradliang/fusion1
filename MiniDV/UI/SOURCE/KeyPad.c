/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "..\..\main\include\netKeyPad.h"
//#include "netKeyPad.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

//#if NETWARE_ENABLE

ST_KEY_PAD KeyPad;

///
///@ingroup KeyPad
///
///@brief   Set KeyPad char
///
///@param
///
///@retval
///
void SetKeyPadChar(BYTE *pbChar)
{
  MP_DEBUG("SetKeyPadChar");
  KeyPad.pbChar = pbChar;
}

///
///@ingroup KeyPad
///
///@brief   Set KeyPad.ActionEnter
///
///@param
///
///@retval
///
void SetKeyPadActionEnter(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionEnter");
    KeyPad.ActionEnter = action;
}

///
///@ingroup KeyPad
///
///@brief   Set KeyPad.ActionExit
///
///@param
///
///@retval
///
void SetKeyPadActionExit(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionExit");
    KeyPad.ActionExit = action;
}

///
///@ingroup KeyPad
///
///@brief   Set KeyPad.wIndex
///
///@param
///
///@retval
///
void SetKeyPadIndex(WORD index)
{
    MP_DEBUG("SetKeyPadIndex(index=%d)", index);
    KeyPad.wIndex = index;
}

///
///@ingroup KeyPad
///
///@brief   Clear KeyPad string
///
///@param
///
///@retval
///
void KeyPadStrClear()
{
    MP_DEBUG("KeyPadStrClear");
    BYTE i;

    for(i=0; i<MAX_KEYPAD_TEXT; i++)
    {
        KeyPad.bData[i] = 0;
    }

    KeyPad.wDataIndex = 0;
}

///
///@ingroup KeyPad
///
///@brief   KeyPad's string add data
///
///@param   data
///
///@retval
///
void KeyPadStrAddData(BYTE data)
{
    MP_DEBUG("KeyPadStrAddData(data=0x%X)", data);

    if (KeyPad.wDataIndex < (MAX_KEYPAD_TEXT - 1))
    {
        KeyPad.bData[KeyPad.wDataIndex] = data;
        KeyPad.wDataIndex ++;

        if (KeyPad.wDataIndex <= (MAX_KEYPAD_TEXT - 1))
            KeyPad.bData[KeyPad.wDataIndex] = '\0';
    }
}

///
///@ingroup KeyPad
///
///@brief   KeyPad's string delete specified char
///
///@param   ch - specified char
///
///@retval
///
void KeyPadStrDelData(BYTE *ch)
{
    int len;

    MP_DEBUG("KeyPadStrDelData(*ch=0x%X", *ch);
    KeyPad.wDataIndex = strlen(ch);

    if (KeyPad.wDataIndex)
    {
        ch[KeyPad.wDataIndex] = '\0';
        KeyPad.wDataIndex --;
        ch[KeyPad.wDataIndex] = '\0';
    }
}

///
///@ingroup KeyPad
///
///@brief   KeyPad string add string
///
///@param   data - specified string
///
///@retval
///
void KeyPadStrAddStr(BYTE *data)
{
    BYTE i;

    MP_DEBUG("KeyPadStrAddStr(%s)", *data);

    if (KeyPad.wDataIndex < (MAX_KEYPAD_TEXT - 1))
    {
        for (i = 0; i < MAX_KEYPAD_TEXT - KeyPad.wDataIndex; i++)
        {
            if (*(data + i) != NULL)
                KeyPad.bData[KeyPad.wDataIndex + i] = *(data + i);
            else
                break;
        }

        KeyPad.bData[KeyPad.wDataIndex + i] = '\0';
        KeyPad.wDataIndex += i;
    }
}

///
///@ingroup KeyPad
///
///@brief   KeyPad copy string
///
///@param   data - specified string
///
///@retval
///
void KeyPadCpyStr(BYTE *data)
{
    BYTE i;
    BYTE *ps_data;

    MP_DEBUG("KeyPadCpyStr");
    ps_data = data;

    for (i =0; i < KeyPad.wDataIndex; i++)
    {
        *(ps_data + i) = KeyPad.bData[i];
    }

    *(ps_data + i) = 0;
}

///
///@ingroup KeyPad
///
///@brief   KeyPad set specified string
///
///@param   data - specified string
///
///@param   len - string length
///
///@retval
///
void KeyPadSetStr(BYTE *data, BYTE len)
{
    BYTE i;

    MP_DEBUG("KeyPadSetStr(len=%d)", len);
    memcpy(KeyPad.bData,data,len);
    KeyPad.wDataIndex = len;
}

///
///@ingroup KeyPad
///
///@brief   Get KeyPad char by index
///
///@param   Index - specified
///
///@retval
///
BYTE GetKeyPadChar(WORD Index)
{
    BYTE *data;

    //MP_DEBUG("GetKeyPadChar(Index=%d)", Index); // log too often
    data = KeyPad.pbChar;

    return *(data + Index);
}

///
///@ingroup KeyPad
///
///@brief   Hook KeyPad's ActionEnter
///
///@param
///
///@retval
///
void KeyPadActionEnter()
{
    MP_DEBUG("KeyPadActionEnter");
    if (KeyPad.ActionEnter != NULL)
        KeyPad.ActionEnter();
}

///
///@ingroup KeyPad
///
///@brief   Hook KeyPad.ActionExit
///
///@param
///
///@retval
///
void KeyPadActionExit()
{
    MP_DEBUG("KeyPadActionExit");
    if (KeyPad.ActionExit != NULL)
        KeyPad.ActionExit();
}

///
///@ingroup KeyPad
///
///@brief   Get KeyPad.wIndex
///
///@param   WORD - index
///
///@retval
///
WORD GetKeyPadIndex()
{
    //MP_DEBUG("GetKeyPadIndex"); // log too  often
    return KeyPad.wIndex;
}

///
///@ingroup KeyPad
///
///@brief   Get KeyPad's string data
///
///@param
///
///@retval
///
BYTE *GetKeyPadString()
{
    MP_DEBUG("GetKeyPadString");
    return &KeyPad.bData[0];
}

///
///@ingroup KeyPad
///
///@brief   Get KeyPad's KEY_PAD+TOTAL constant
///
///@param
///
///@retval
///
WORD GetKeyPadTotal()
{
    MP_DEBUG("GetKeyPadTotal(), KEY_PAD_TOTAL=%d", KEY_PAD_TOTAL);
    return KEY_PAD_TOTAL;
}

//------------------------------------------------------------------------------
// Function for xpg
//------------------------------------------------------------------------------
///
///@ingroup KeyPad
///
///@brief   Main entry routine for Xpg event
///
///@param
///
///@retval
///
void xpgCb_EnterKeyPad() // act168
{
    MP_DEBUG("xpgCb_EnterKeyPad");
#if NETWARE_ENABLE
    EnterNWSetup();
#endif
}


#if 1
///
///@ingroup KeyPad
///
///@brief   KeyPad UP button routine
///
///@param
///
///@retval
///
void xpgCb_KeyPadUp() // act169
{
    WORD index;

    MP_DEBUG("xpgCb_KeyPadUp()");
    index = KeyPad.wIndex;

    if( KeyPad.wIndex == 0)
        KeyPad.wIndex = 	KEY_PAD_SHIFT;
    else if ( KeyPad.wIndex == KEY_PAD_SHIFT)
        KeyPad.wIndex = 0;
    else if(( KeyPad.wIndex >= 1) && ( KeyPad.wIndex <= 10))
        KeyPad.wIndex += 40;
    else if( KeyPad.wIndex == 11)
        KeyPad.wIndex = 54;
    else if( KeyPad.wIndex == 12)
        KeyPad.wIndex = 52; //KEY_PAD_Back;
    else if( KeyPad.wIndex == 13)
        KeyPad.wIndex = 53; //KEY_PAD_Back1;
    else if(( KeyPad.wIndex >= 14) && ( KeyPad.wIndex <= 39))
        KeyPad.wIndex -= 13;
    else if(( KeyPad.wIndex >= 41) && ( KeyPad.wIndex <= 53))
        KeyPad.wIndex -= 14;
    else if ( KeyPad.wIndex == KEY_PAD_Next)
        KeyPad.wIndex = 51;
    //else if ( KeyPad.wIndex == KEY_PAD_Back)
    //    KeyPad.wIndex = 52;
    //else if ( KeyPad.wIndex == KEY_PAD_Back1)
    //    KeyPad.wIndex = 53;

    MP_DEBUG1( "UPKeyPad.wIndex = %d",KeyPad.wIndex);
}

///
///@ingroup KeyPad
///
///@brief   KeyPad DOWN button routine
///
///@param
///
///@retval
///
void xpgCb_KeyPadDown() // act170
{
    MP_DEBUG("xpgCb_KeyPadDown()");
    WORD index;

    index = KeyPad.wIndex;

    if( KeyPad.wIndex == 0)
        KeyPad.wIndex = KEY_PAD_SHIFT;
    else if ( KeyPad.wIndex == KEY_PAD_SHIFT)
        KeyPad.wIndex = 0;
    else if(( KeyPad.wIndex >= 1) && ( KeyPad.wIndex <= 26))
        KeyPad.wIndex += 13;
    else if(( KeyPad.wIndex >= 27) && ( KeyPad.wIndex <= 39))
        KeyPad.wIndex += 14;
    else if(( KeyPad.wIndex >= 41) && ( KeyPad.wIndex <= 50))
        KeyPad.wIndex -= 40;
    else if( KeyPad.wIndex == 51)
        KeyPad.wIndex = KEY_PAD_Next;
    else if( KeyPad.wIndex == 52)
        KeyPad.wIndex = 12; //KEY_PAD_Back;
    else if( KeyPad.wIndex == 53)
        KeyPad.wIndex = 13; //KEY_PAD_Back1;
    else if( KeyPad.wIndex == KEY_PAD_Next)
        KeyPad.wIndex = 11;
    //else if( KeyPad.wIndex == KEY_PAD_Back)
    //    KeyPad.wIndex = 12;
    //else if( KeyPad.wIndex == KEY_PAD_Back1)
    //    KeyPad.wIndex = 13;

    MP_DEBUG1( "Down KeyPad.wIndex = %d",KeyPad.wIndex);
}

///
///@ingroup KeyPad
///
///@brief   KeyPad LEFT button routine
///
///@param
///
///@retval
///
void xpgCb_KeyPadLeft() // act171
{
    MP_DEBUG("xpgCb_KeyPadLeft()");

    if(KeyPad.wIndex)
        KeyPad.wIndex--;
    else
        KeyPad.wIndex = KEY_PAD_TOTAL - 1;

    //if(KeyPad.wIndex == KEY_PAD_Back)
    //    KeyPad.wIndex--;

    MP_DEBUG1("Left KeyPad.wIndex = %d",KeyPad.wIndex);
}

///
///@ingroup KeyPad
///
///@brief   KeyPad RIGHT button routine
///
///@param
///
///@retval
///
void xpgCb_KeyPadRight() // act172
{
    MP_DEBUG("xpgCb_KeyPadRight()");

    //if (KeyPad.wIndex == KEY_PAD_Back)
    //    KeyPad.wIndex++;

    if (KeyPad.wIndex >= KEY_PAD_TOTAL-1)
        KeyPad.wIndex = 0;
    else
        KeyPad.wIndex++;

    MP_DEBUG1("Right KeyPad.wIndex = %d",KeyPad.wIndex);
}

#endif

///
///@ingroup KeyPad
///
///@brief   Hook KeyPad.ActionEnter
///
///@param
///
///@retval
///
void xpgCb_KeyPadEnter() // act173
{
    MP_DEBUG("xpgCb_KeyPadEnter()");
    KeyPadActionEnter(); // net_setup.c's NWSetupEnter(), In EnterNWSetup(), it will SetKeyPadActionEnter(NWSetupEnter);
}

///
///@ingroup KeyPad
///
///@brief   Exit from KeyPad Action
///
///@param
///
///@retval
///
void xpgCb_KeyPadExit() // act174
{
    MP_DEBUG("xpgCb_KeyPadExit()");
    KeyPadActionExit(); // net_setup.c's NWSetupExit(), In EnterNWSetup(), it will SetKeyPadActionEnter(NWSetupExit);
}

//#endif

