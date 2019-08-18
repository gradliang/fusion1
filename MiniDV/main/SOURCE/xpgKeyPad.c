/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpgKeyPad.h"
#include "xpgFunc.h"
#include "setup.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#define KP_CNT_PER_ROW      14


static BYTE gKpCnt = 54;            // depend on xpw keypad layout
static BYTE gTxtLen = 0;            // for user define maximum input-string

ST_XPG_KEY_PAD stXpgKeyPad;


static void (*kpActionUp)(void);
static void (*kpActionDown)(void);
static void (*kpActionLeft)(void);
static void (*kpActionRight)(void);


/*     should add a initial sequence for xpg keypad
void xpgKeyPadInit(BYTE pageNum, BYTE kpcnt, )
{
}
*/


///
///@ingroup XpgKeyPad
///
///@brief   Set XpgKeyPad count
///
///@param
///
///@retval
///
void xpgKeyPadSetCnt(BYTE kpcnt)
{
  MP_DEBUG("SetKeyPadCnt");
  gKpCnt = kpcnt;
}


///
///@ingroup XpgKeyPad
///
///@brief   Set XpgKeyPad count
///
///@param
///
///@retval
///
void xpgKeyPadSetTxtLen(BYTE txtlen)
{
    MP_DEBUG("SetTxtLen");

    if(txtlen > MAX_XPG_KP_TEXT)
    {
        mpDebugPrint("ERROR!! text length overflow!!!");

        return;
    }

    gTxtLen = txtlen;
}


///
///@ingroup XpgKeyPad
///
///@brief   Set XpgKeyPad char
///
///@param
///
///@retval
///
void xpgKeyPadSetChar(BYTE *pbChar)
{
  MP_DEBUG("SetKeyPadChar");
  stXpgKeyPad.pbChar = pbChar;
}


///
///@ingroup XpgKeyPad
///
///@brief   Set KpActionUp
///
///@param
///
///@retval
///
void xpgKeyPadSetActionUp(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionUp");
    kpActionUp = action;
}



///
///@ingroup XpgKeyPad
///
///@brief   Set KpActionDown
///
///@param
///
///@retval
///
void xpgKeyPadSetActionDown(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionDown");
    kpActionDown = action;
}



///
///@ingroup XpgKeyPad
///
///@brief   Set KpActionLeft
///
///@param
///
///@retval
///
void xpgKeyPadSetActionLeft(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionLeft");
    kpActionLeft = action;
}



///
///@ingroup XpgKeyPad
///
///@brief   Set KpActionRight
///
///@param
///
///@retval
///
void xpgKeyPadSetActionRight(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionRight");
    kpActionRight = action;
}



///
///@ingroup XpgKeyPad
///
///@brief   Set XpgKeyPad.ActionEnter
///
///@param
///
///@retval
///
void xpgKeyPadSetActionEnter(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionEnter");
    stXpgKeyPad.ActionEnter = action;
}



///
///@ingroup XpgKeyPad
///
///@brief   Set XpgKeyPad.ActionExit
///
///@param
///
///@retval
///
void xpgKeyPadSetActionExit(void (*action) (void))
{
    MP_DEBUG("SetKeyPadActionExit");
    stXpgKeyPad.ActionExit = action;
}



///
///@ingroup XpgKeyPad
///
///@brief   Set XpgKeyPad.wIndex
///
///@param
///
///@retval
///
void xpgKeyPadSetIndex(WORD index)
{
    MP_DEBUG("SetKeyPadIndex(index=%d)", index);
    stXpgKeyPad.wIndex = index;
}



///
///@ingroup XpgKeyPad
///
///@brief   Clear XpgKeyPad string
///
///@param
///
///@retval
///
void xpgKeyPadStrClear()
{
    MP_DEBUG("KeyPadStrClear");
    BYTE i;

#if 1
    for(i = 0; i < gTxtLen; i++)
    {
        stXpgKeyPad.bData[i] = 0;
    }

    stXpgKeyPad.wDataIndex = 0;
#else
    for(i = 0; i < MAX_XPG_KP_TEXT; i++)
    {
        stXpgKeyPad.bData[i] = 0;
    }

    stXpgKeyPad.wDataIndex = 0;

#endif
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad's string add data
///
///@param   data
///
///@retval
///
void xpgKeyPadStrAddChar(BYTE data)
{
    MP_DEBUG("KeyPadStrAddData(data=0x%X)", data);

#if 1
    if (stXpgKeyPad.wDataIndex < (gTxtLen - 1))
    {
        stXpgKeyPad.bData[stXpgKeyPad.wDataIndex] = data;
        stXpgKeyPad.wDataIndex ++;

        if (stXpgKeyPad.wDataIndex <= (gTxtLen - 1))
            stXpgKeyPad.bData[stXpgKeyPad.wDataIndex] = '\0';
    }
#else
    if (stXpgKeyPad.wDataIndex < (MAX_XPG_KP_TEXT - 1))
    {
        stXpgKeyPad.bData[stXpgKeyPad.wDataIndex] = data;
        stXpgKeyPad.wDataIndex ++;

        if (stXpgKeyPad.wDataIndex <= (MAX_XPG_KP_TEXT - 1))
            stXpgKeyPad.bData[stXpgKeyPad.wDataIndex] = '\0';
    }

#endif
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad's string add data
///
///@param   data
///
///@retval
///
void xpgKeyPadStrDelChar(void)
{
    MP_DEBUG("KeyPadStrDelChar");

    if (stXpgKeyPad.wDataIndex)
    {
        stXpgKeyPad.wDataIndex--;
        stXpgKeyPad.bData[stXpgKeyPad.wDataIndex] = '\0';
    }
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad's string delete specified char
///
///@param   ch - specified char
///
///@retval
///
void xpgKeyPadStrDelData(BYTE *ch)
{
    int len;

    MP_DEBUG("KeyPadStrDelData(*ch=0x%X", *ch);
    stXpgKeyPad.wDataIndex = strlen(ch);

    if (stXpgKeyPad.wDataIndex)
    {
        ch[stXpgKeyPad.wDataIndex] = '\0';
        stXpgKeyPad.wDataIndex --;
        ch[stXpgKeyPad.wDataIndex] = '\0';
    }
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad string add string
///
///@param   data - specified string
///
///@retval
///
void xpgKeyPadStrAddStr(BYTE *data)
{
    BYTE i;

    MP_DEBUG("KeyPadStrAddStr(%s)", *data);
#if 1
    if (stXpgKeyPad.wDataIndex < (gTxtLen - 1))
    {
        for (i = 0; i < gTxtLen - stXpgKeyPad.wDataIndex; i++)
        {
            if (*(data + i) != NULL)
                stXpgKeyPad.bData[stXpgKeyPad.wDataIndex + i] = *(data + i);
            else
                break;
        }

        stXpgKeyPad.bData[stXpgKeyPad.wDataIndex + i] = '\0';
        stXpgKeyPad.wDataIndex += i;
    }
#else
    if (stXpgKeyPad.wDataIndex < (MAX_XPG_KP_TEXT - 1))
    {
        for (i = 0; i < MAX_XPG_KP_TEXT - stXpgKeyPad.wDataIndex; i++)
        {
            if (*(data + i) != NULL)
                stXpgKeyPad.bData[stXpgKeyPad.wDataIndex + i] = *(data + i);
            else
                break;
        }

        stXpgKeyPad.bData[stXpgKeyPad.wDataIndex + i] = '\0';
        stXpgKeyPad.wDataIndex += i;
    }

#endif
}




///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad copy string
///
///@param   data - specified string
///
///@retval
///
void xpgKeyPadCpyStr(BYTE *data)
{
    BYTE i;
    BYTE *ps_data;

    MP_DEBUG("%s", __FUNCTION__);
    ps_data = data;

    for (i =0; i < stXpgKeyPad.wDataIndex; i++)
    {
        *(ps_data + i) = stXpgKeyPad.bData[i];
    }

    *(ps_data + i) = 0;
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad set specified string
///
///@param   data - specified string
///
///@param   len - string length
///
///@retval
///
void xpgKeyPadSetStr(BYTE *data, BYTE len)
{
    BYTE i;

    MP_DEBUG("KeyPadSetStr(len=%d)", len);
    memcpy(stXpgKeyPad.bData,data,len);
    stXpgKeyPad.wDataIndex = len;
}



///
///@ingroup XpgKeyPad
///
///@brief   Get XpgKeyPad char by index
///
///@param   Index - specified
///
///@retval
///
BYTE xpgKeyPadGetChar(WORD Index)
{
    BYTE *data;

    //MP_DEBUG("GetKeyPadChar(Index=%d)", Index); // log too often
    data = stXpgKeyPad.pbChar;

    return *(data + Index);
}



///
///@ingroup XpgKeyPad
///
///@brief   Hook XpgKeyPad's ActionEnter
///
///@param
///
///@retval
///
void xpgKeyPadActionEnter()
{
    MP_DEBUG("KeyPadActionEnter");

    if (stXpgKeyPad.ActionEnter != NULL)
        stXpgKeyPad.ActionEnter();
}



///
///@ingroup XpgKeyPad
///
///@brief   Hook XpgKeyPad.ActionExit
///
///@param
///
///@retval
///
void xpgKeyPadActionExit()
{
    MP_DEBUG("KeyPadActionExit");

    if (stXpgKeyPad.ActionExit != NULL)
        stXpgKeyPad.ActionExit();
}



///
///@ingroup XpgKeyPad
///
///@brief   Get XpgKeyPad.wIndex
///
///@param   WORD - index
///
///@retval
///
WORD xpgKeyPadGetIndex()
{
    //MP_DEBUG("GetKeyPadIndex"); // log too  often

    return stXpgKeyPad.wIndex;
}



///
///@ingroup XpgKeyPad
///
///@brief   Get XpgKeyPad's string data
///
///@param
///
///@retval
///
BYTE* xpgKeyPadGetString(void)
{
    MP_DEBUG("GetKeyPadString");

    return &stXpgKeyPad.bData[0];
}



WORD xpgKeyPadGetStrLen(void)
{
    MP_DEBUG("GetKeyPadString index");

    return stXpgKeyPad.wDataIndex;
}


BYTE xpgKeyPadGetStrChar(BYTE index)
{
    return stXpgKeyPad.bData[index];
}

/*
///
///@ingroup XpgKeyPad
///
///@brief   Get XpgKeyPad's KEY_PAD+TOTAL constant
///
///@param
///
///@retval
///
WORD xpgGetKeyPadTotal()
{
    MP_DEBUG("GetKeyPadTotal(), KEY_PAD_TOTAL=%d", KEY_PAD_TOTAL);

    return KEY_PAD_TOTAL;
}
*/


//------------------------------------------------------------------------------
// Function for xpg
//------------------------------------------------------------------------------
///
///@ingroup XpgKeyPad
///
///@brief   Main entry routine for Xpg event
///
///@param
///
///@retval
///
/*
void xpgCb_EnterKeyPad()
{
    //UartOutText("xpgCb_EnterKeyPad~~~~~~~~~~~~~~~\n");
    MP_DEBUG("xpgCb_EnterKeyPad");
    EnterNWSetup();
}
*/



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad UP button routine
///
///@param
///
///@retval
///
void xpgCb_xpgKpUp(void)
{
    //WORD index;

    MP_DEBUG("xpgCb_KeyPadUp()");
    //index = stXpgKeyPad.wIndex;

    switch(gKpCnt)
    {
        case 56:        // total 56 keys. wIndex number should be modified according xpw layout.
        {
            if(stXpgKeyPad.wIndex < 14)
            {
                stXpgKeyPad.wIndex += 42;        // last row + current index
            }
            else
            {
                stXpgKeyPad.wIndex -= KP_CNT_PER_ROW;
            }

        }
        break;
        /*
        case 54:        // total 54 keys. wIndex number should be modified according xpw layout.
        {
            if( stXpgKeyPad.wIndex == 0)
                stXpgKeyPad.wIndex = KEY_PAD_SHIFT;
            else if ( stXpgKeyPad.wIndex == KEY_PAD_SHIFT)
                stXpgKeyPad.wIndex = 0;
            else if(( stXpgKeyPad.wIndex >= 1) && ( stXpgKeyPad.wIndex <= 10))
                stXpgKeyPad.wIndex += 40;
            else if( stXpgKeyPad.wIndex == 11)
                stXpgKeyPad.wIndex = 54;
            else if( stXpgKeyPad.wIndex == 12)
                stXpgKeyPad.wIndex = KEY_PAD_Back;
            else if( stXpgKeyPad.wIndex == 13)
                stXpgKeyPad.wIndex = KEY_PAD_Back1;
            else if(( stXpgKeyPad.wIndex >= 14) && ( stXpgKeyPad.wIndex <= 39))
                stXpgKeyPad.wIndex -= 13;
            else if(( stXpgKeyPad.wIndex >= 41) && ( stXpgKeyPad.wIndex <= 53))
                stXpgKeyPad.wIndex -= 14;
            else if ( stXpgKeyPad.wIndex == KEY_PAD_Next)
                stXpgKeyPad.wIndex = 51;
            else if ( stXpgKeyPad.wIndex == KEY_PAD_Back)
                stXpgKeyPad.wIndex = 52;
            else if ( stXpgKeyPad.wIndex == KEY_PAD_Back1)
                stXpgKeyPad.wIndex = 53;

        }
        break;
        */
    }

/*
    if(gKpCnt == 54)
    {
        if( KeyPad.wIndex == 0)
            KeyPad.wIndex = 	KEY_PAD_SHIFT;
        else if ( KeyPad.wIndex == KEY_PAD_SHIFT)
            KeyPad.wIndex = 0;
        else if(( KeyPad.wIndex >= 1) && ( KeyPad.wIndex <= 10))
            KeyPad.wIndex += 40;
        else if( KeyPad.wIndex == 11)
            KeyPad.wIndex = 54;
        else if( KeyPad.wIndex == 12)
            KeyPad.wIndex = KEY_PAD_Back;
        else if( KeyPad.wIndex == 13)
            KeyPad.wIndex = KEY_PAD_Back1;
        else if(( KeyPad.wIndex >= 14) && ( KeyPad.wIndex <= 39))
            KeyPad.wIndex -= 13;
        else if(( KeyPad.wIndex >= 41) && ( KeyPad.wIndex <= 53))
            KeyPad.wIndex -= 14;
        else if ( KeyPad.wIndex == KEY_PAD_Next)
            KeyPad.wIndex = 51;
        else if ( KeyPad.wIndex == KEY_PAD_Back)
            KeyPad.wIndex = 52;
        else if ( KeyPad.wIndex == KEY_PAD_Back1)
            KeyPad.wIndex = 53;
    }
    else if(gKpCnt == 56)   // 14 x 4 keypad matrix
    {
        if(KeyPad.wIndex < 14)
        {
            KeyPad.wIndex += 42;        // last row + current index
        }
        else
        {
            KeyPad.wIndex -= KP_CNT_PER_ROW;
        }
    }
*/
    kpActionUp();
    MP_DEBUG1( "UP stXpgKeyPad.wIndex = %d",stXpgKeyPad.wIndex);
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad DOWN button routine
///
///@param
///
///@retval
///
void xpgCb_xpgKpDown(void)
{
    MP_DEBUG("xpgCb_KeyPadDown()");
    //WORD index;

    //index = stXpgKeyPad.wIndex;

    switch(gKpCnt)
    {
        case 56:    // total 56 keys. wIndex number should be modified according xpw layout.
        {
            if(stXpgKeyPad.wIndex > 41)
            {
                stXpgKeyPad.wIndex -= 42;        // current index - last row
            }
            else
            {
                stXpgKeyPad.wIndex += KP_CNT_PER_ROW;
            }


        }
        break;
        /*
        case 54:    // total 54 keys. wIndex number should be modified according xpw layout.
        {
            if( stXpgKeyPad.wIndex == 0)
                stXpgKeyPad.wIndex = KEY_PAD_SHIFT;
            else if ( stXpgKeyPad.wIndex == KEY_PAD_SHIFT)
                stXpgKeyPad.wIndex = 0;
            else if(( stXpgKeyPad.wIndex >= 1) && ( stXpgKeyPad.wIndex <= 26))
                stXpgKeyPad.wIndex += 13;
            else if(( stXpgKeyPad.wIndex >= 27) && ( stXpgKeyPad.wIndex <= 39))
                stXpgKeyPad.wIndex += 14;
            else if(( stXpgKeyPad.wIndex >= 41) && ( stXpgKeyPad.wIndex <= 50))
                stXpgKeyPad.wIndex -= 40;
            else if( stXpgKeyPad.wIndex == 51)
                stXpgKeyPad.wIndex = KEY_PAD_Next;
            else if( stXpgKeyPad.wIndex == 52)
                stXpgKeyPad.wIndex = KEY_PAD_Back;
            else if( stXpgKeyPad.wIndex == 53)
                stXpgKeyPad.wIndex = KEY_PAD_Back1;
            else if( stXpgKeyPad.wIndex == KEY_PAD_Next)
                stXpgKeyPad.wIndex = 11;
            else if( stXpgKeyPad.wIndex == KEY_PAD_Back)
                stXpgKeyPad.wIndex = 12;
            else if( stXpgKeyPad.wIndex == KEY_PAD_Back1)
                stXpgKeyPad.wIndex = 13;
        }
        break;
        */
    }
/*
    if(gKpCnt == 54)
    {
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
            KeyPad.wIndex = KEY_PAD_Back;
        else if( KeyPad.wIndex == 53)
            KeyPad.wIndex = KEY_PAD_Back1;
        else if( KeyPad.wIndex == KEY_PAD_Next)
            KeyPad.wIndex = 11;
        else if( KeyPad.wIndex == KEY_PAD_Back)
            KeyPad.wIndex = 12;
        else if( KeyPad.wIndex == KEY_PAD_Back1)
            KeyPad.wIndex = 13;
    }
    else if(gKpCnt == 56)
    {
        if(KeyPad.wIndex > 41)
        {
            KeyPad.wIndex -= 42;        // current index - last row
        }
        else
        {
            KeyPad.wIndex += KP_CNT_PER_ROW;
        }

    }
*/
    kpActionDown();
    MP_DEBUG1( "Down stXpgKeyPad.wIndex = %d",stXpgKeyPad.wIndex);
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad LEFT button routine
///
///@param
///
///@retval
///
void xpgCb_xpgKpLeft(void)
{
    //UartOutText("xpgCb_KeyPadLeft~~~~~~~~~~~~~~~\n");
    MP_DEBUG("xpgCb_KeyPadLeft()");

    if(stXpgKeyPad.wIndex)
        stXpgKeyPad.wIndex--;
    else
        //KeyPad.wIndex = KEY_PAD_TOTAL - 1;
        stXpgKeyPad.wIndex = gKpCnt - 1;

    //if(KeyPad.wIndex == KEY_PAD_Back)
        //KeyPad.wIndex--;
    kpActionLeft();
    MP_DEBUG1("Left stXpgKeyPad.wIndex = %d",stXpgKeyPad.wIndex);
}



///
///@ingroup XpgKeyPad
///
///@brief   XpgKeyPad RIGHT button routine
///
///@param
///
///@retval
///
void xpgCb_xpgKpRight(void)
{
    MP_DEBUG("xpgCb_KeyPadRight()");
    //UartOutText("xpgCb_KeyPadRight~~~~~~~~~~~~~~~\n");

    //if (KeyPad.wIndex == KEY_PAD_Back)
    //    KeyPad.wIndex++;

    //if (KeyPad.wIndex >= KEY_PAD_TOTAL-1)
    if (stXpgKeyPad.wIndex >= gKpCnt - 1)
        stXpgKeyPad.wIndex = 0;
    else
        stXpgKeyPad.wIndex++;

    kpActionRight();
    MP_DEBUG1("Right stXpgKeyPad.wIndex = %d",stXpgKeyPad.wIndex);
}



//------------------------------------------------------------------------------
///
///@ingroup XpgKeyPad
///
///@brief   Hook XpgKeyPad.ActionEnter
///
///@param
///
///@retval
///
void xpgCb_xpgKpEnter(void)
{
    //UartOutText("xpgCb_KeyPadEnter~~~~~~~~~~~~~~~\n");
    MP_DEBUG("xpgCb_KeyPadEnter()");
    xpgKeyPadActionEnter();
}

//------------------------------------------------------------------------------
///
///@ingroup XpgKeyPad
///
///@brief   Exit from XpgKeyPad Action
///
///@param
///
///@retval
///
void xpgCb_xpgKpExit(void)
{
    //UartOutText("xpgCb_KeyPadExit~~~~~~~~~~~~~~~\n");
    MP_DEBUG("xpgCb_KeyPadExit()");
    xpgKeyPadActionExit();
}





