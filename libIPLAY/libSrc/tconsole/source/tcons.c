/*
*******************************************************************************
*                           Magic Pixel Inc. Inc.                             *
*                  Copyright (c) 2007 -, All Rights Reserved                  *
*                                                                             *
* Kevin-shell.                                                                *
*                                                                             *
* File : kshell                                                               *
* By : Kevin Huang                                                            *
*                                                                             *
*                                                                             *
* Description : This is a shell help us to test drivers, please add your      *
*               test function follows the rule.                               *
*                                                                             *
* History : 2007/02/15 File created.                                          *
*                                                                             *
*******************************************************************************
*/

/*************************************************************************/
/***                        Include Files                              ***/
/*************************************************************************/
#include <string.h>
#include "global612.h"
#include "mpTrace.h"
#include "mpapi.h"
#include "os.h"
#include "taskid.h"
#include "tcons.h"
#include "BtApi.h"
//#include "..\..\..\..\STD_DPF\main\include\Ui_timer.h"
#include "..\..\usbotg\include\Usbotg_host.h"
#include "..\..\bios\include\uart.h"
#include "..\..\usbotg\include\Usbotg_bt.h"
//#include "..\..\..\..\STD_DPF\main\include\filebrowser.h"
#include "..\..\file\include\filebrowser.h"
#include "btsetting.h"

/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/


static BYTE KModuleStart = 0;



#define COM_BUFFER_SIZE 512
/* global variable */
static SBYTE lastCmd[HISTORY_NO][CmdStringLength];        // command history buffer
static SDWORD lastCmdIdx;          // last command index for using
static SDWORD saveCmdIdx;          // last command index for saving
static BYTE cmds[CmdStringLength];       // input command buffer
static SDWORD StartPosition = 0;   // start searching position in a string for using in getToken()
static SBYTE promptString[PromptLength] = "[KShell]> ";   // prompt strng
static BYTE parameter[PARA_NUM][PARA_LENGTH];    // the parameter are the tokens following with the cmd
static SDWORD pindex = 0;          // Record the Input string index
static BYTE _buffer[COM_BUFFER_SIZE];
static BYTE * inPtr, * outPtr;

/*************************************************************************/
/***               Internally Visible Constants and Static Data        ***/
/*************************************************************************/

#if ((CHIP_VER & 0xFFFF0000) == CHIP_VER_615)
#define RX_READY 			C_RX_FIFO_THRESHOLD
#else
#define RX_READY 			C_RXTHR_HIT
#endif

//#define C_RXTHR_HIT				0x00020000

#define isAlphNum(char)  ((char>='a'&&char <='z')||(char>='A'&&char <='Z')||(char>='0'&&char <='9'))
#define isDigit(char)     (char>='0'&&char <='9')
#define isHex(char)      ((char>='0'&&char <='9')||(char>='A'&&char <='F')||(char>='a'&&char <='f'))

#define EVENT_MASK 0x7fffffff
#define ERR_SHELL_GENERAL_FAIL -1
#define ERR_SHELL_NODATA -2
#define ERR_SHELL_GETASP_FAIL -3
#ifndef NO_ERR
#define NO_ERR 0
#endif
//#define strlen UtilStringLength08

/* function declaration */
/* system command */
static void Sys_Dumpmem(void);
static void Sys_ReadMem(void);
static void Edit_write(void);
static void Edit_half(void);
static void Edit_byte(void);
static void kmod_exec(void);
static void kmod_nonfree_exec(void);
static void Testproc(void);
static void listCmd(void);
static void prompt(void);

// USB Test Console
static void UsbReadWritePattern(void);

/* Command mapping table */
const struct
{
    SBYTE      *cmd;              /* Name of command */
    voidfun   func;             /* Function to process the command */
} cmdtab[NoCmd] =
{
    /* system command */
    {"dp", Sys_Dumpmem},
    {"rd", Sys_ReadMem},
    {"ew", Edit_write},
    {"eh", Edit_half},
    {"eb", Edit_byte},
    {"kmod", kmod_exec},
    {"kmod", kmod_nonfree_exec},
    {"test", Testproc},

    /* USB command */
    {"usbpt", UsbReadWritePattern},

    /* Help command */
    {"help", listCmd},
    {"?", listCmd},
    {"prompt", prompt},
    {0, 0}                       /* invalid command */
};

static BYTE s32shellMsgId = OS_STATUS_INVALID_ID;
static SDWORD s32shellTaskId = OS_STATUS_INVALID_ID;
static BYTE u08ShellTimerID = -1;

/****************************************************************************
 **
 ** NAME:           listCmd
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Help list.
 **
 ****************************************************************************/
static void listCmd(void)
{

    mpDebugPrint("\nTest-Console Shell Command:\n");

    /* ================   General Test Console ================================= */
    if(parameter[0][0] == '\0')
    {

        mpDebugPrint("rd  [Addr]        : Read   4 bytes from a specified address of memory");
        mpDebugPrint("dp  [Addr]        : Dump 512 bytes from a specified address of memory");
        mpDebugPrint("ew  [Addr]  Value : Write  4 bytes in a specified address of memory");
        mpDebugPrint("eh  [Addr]  Value : Write  2 bytes in a specified address of memory");
        mpDebugPrint("eb  [Addr]  Value : Write  1 bytes in a specified address of memory");
        // mpDebugPrint("kmod");
    }





    /* ================   USB Test Console ================================= */
    if(parameter[0][0] == '\0'  ||  (!strcmp("usb", &parameter[0][0])))
    {
        mpDebugPrint("usbpt  Port( = 0)  Lun( = 0): Test Read/Write 512 Sectors Pattern for PenDrive");
    }





    /* ================   General Test Console ================================= */
    mpDebugPrint("\n====================================================");
    mpDebugPrint("help or ? : Display command list and description");
    mpDebugPrint("help or ?   name : Display command list for component(name = usb...)");
    mpDebugPrint("\n\n");
    // mpDebugPrint("[ESC]     : Exit");

}

static void delChar(void)
{
    PutUartChar(0x08);
    PutUartChar(0x20);
    PutUartChar(0x08);
}

/****************************************************************************
 **
 ** NAME:           Check32bit
 **
 ** PARAMETERS:     Test HEX or Dec string
 **
 ** RETURN VALUES:  NO_ERR or ERR_SHELL_GENERAL_FAIL
 **
 ** DESCRIPTION:    Check a 32bit address or data is legal or illegal.
 **
 ****************************************************************************/
SDWORD Check32bit(BYTE * TestString)
{
    SDWORD       i;
    SDWORD       StringLenTemp;

    StringLenTemp = strlen(TestString);
    if((*TestString == '\0') || (StringLenTemp > 10))
        return ERR_SHELL_GENERAL_FAIL;

    if(TestString[0] == '0' && TestString[1] == 'x')    //Hex
    {
        for(i = 2; i < StringLenTemp; i++)
            if(!isHex(TestString[i]))
                return (ERR_SHELL_GENERAL_FAIL);
    }
    else                        //Dec
    {
        for(i = 0; i < StringLenTemp; i++)
            if(!isDigit(TestString[i]))
                return (ERR_SHELL_GENERAL_FAIL);
    }
    return (PASS);
}

/****************************************************************************
 **
 ** NAME:           Check16bit
 **
 ** PARAMETERS:     Test HEX or Dec string
 **
 ** RETURN VALUES:  NO_ERR or ERR_SHELL_GENERAL_FAIL
 **
 ** DESCRIPTION:    Check a 16bit address or data is legal or illegal.
 **
 ****************************************************************************/
SDWORD Check16bit(BYTE * TestString)
{
    SDWORD       i;
    SDWORD       StringLenTemp;

    StringLenTemp = strlen(TestString);

    if(TestString[0] == '0' && TestString[1] == 'x')    //Hex
    {
        if((*TestString == '\0') || (StringLenTemp > 6))
            return ERR_SHELL_GENERAL_FAIL;

        for(i = 2; i < StringLenTemp; i++)
            if(!isHex(TestString[i]))
                return (ERR_SHELL_GENERAL_FAIL);
    }
    else                        //Dec
    {
        if((*TestString == '\0') || (StringLenTemp > 5))
            return ERR_SHELL_GENERAL_FAIL;

        for(i = 0; i < StringLenTemp; i++)
            if(!isDigit(TestString[i]))
                return (ERR_SHELL_GENERAL_FAIL);
    }
    return (PASS);
}

/****************************************************************************
 **
 ** NAME:           Check8bit
 **
 ** PARAMETERS:     Test HEX or Dec string
 **
 ** RETURN VALUES:  NO_ERR or ERR_SHELL_GENERAL_FAIL
 **
 ** DESCRIPTION:    Check a 8bit address or data is legal or illegal.
 **
 ****************************************************************************/
SDWORD Check8bit(BYTE * TestString)
{
    SDWORD       i;
    SDWORD       StringLenTemp;

    StringLenTemp = strlen(TestString);
    if(TestString[0] == '0' && TestString[1] == 'x')    //Hex
    {
        if((*TestString == '\0') || (StringLenTemp > 4))
            return ERR_SHELL_GENERAL_FAIL;

        for(i = 2; i < StringLenTemp; i++)
            if(!isHex(TestString[i]))
                return (ERR_SHELL_GENERAL_FAIL);
    }
    else                        //Dec
    {
        if((*TestString == '\0') || (StringLenTemp > 3))
            return ERR_SHELL_GENERAL_FAIL;

        for(i = 0; i < StringLenTemp; i++)
            if(!isDigit(TestString[i]))
                return (ERR_SHELL_GENERAL_FAIL);
    }
    return (PASS);
}

/****************************************************************************
 **
 ** NAME:           CheckAlign
 **
 ** PARAMETERS:     Tested address
 **
 ** RETURN VALUES:  NO_ERR or ERR_SHELL_GENERAL_FAIL
 **
 ** DESCRIPTION:    Check a 32bit address is 2 or 4 bytes legal alignment.
 **
 ****************************************************************************/
static SDWORD CheckAlign(DWORD * Addr, DWORD alignment)
{
    switch (alignment)
    {
        case 2:
            if((DWORD) Addr & 0x00000001)
                return ERR_SHELL_GENERAL_FAIL;
            break;
        case 4:
            if((DWORD) Addr & 0x00000003)
                return ERR_SHELL_GENERAL_FAIL;
            break;
        default:
            return ERR_SHELL_GENERAL_FAIL;
    }
    return (PASS);
}

/****************************************************************************
 **
 ** NAME:           DumpMem8
 **
 ** PARAMETERS:     Length, Start address
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Dump the specfied address of memory per Byte.
 **
 ****************************************************************************/
void DumpMem8(DWORD iLength, volatile BYTE * StartAddress)
{
    SDWORD       i;
    BYTE       ShowString[12];

    for(i = 0; i < iLength; i++)
    {
        if((i % 16) == 0)
        {
            sprintf(ShowString, "<%8x>", ((DWORD) StartAddress) + i);
            ShowString[10] = '=';
            ShowString[11] = '\0';
            UartOutText(ShowString);
        }

        if(StartAddress[i] >= 0x10)
            sprintf(ShowString, ":%2x\\-", StartAddress[i]);
        else
            sprintf(ShowString, ":0%1x\\-", StartAddress[i]);

        UartOutText(ShowString);
        if(((i + 1) % 16) == 0)
            mpDebugPrint("");
        else if(((i + 1) % 8) == 0)
            UartOutText("=\\-");
    }
}

/****************************************************************************
 **
 ** NAME:           DumpMem32
 **
 ** PARAMETERS:     Length, Start address
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Dump the specfied address of memory per Word.
 **
 ****************************************************************************/
void DumpMem32(DWORD iLength, volatile DWORD * StartAddress)
{
    SDWORD       i, j;
    BYTE       ShowString[12];

    iLength >>= 2;
    for(i = 0; i < iLength; i++)
    {
        if((i % 4) == 0)
        {
            sprintf(ShowString, "<%8x>", ((DWORD) StartAddress) + i * 4);
            ShowString[10] = '=';
            ShowString[11] = '\0';
            UartOutText(ShowString);
        }

        sprintf(ShowString, ":%8x\\-", StartAddress[i]);
        for(j=1;j<9;j++)
            if(ShowString[j] == (BYTE)0x20)
                ShowString[j] = '0';
        UartOutText(ShowString);

        if(((i + 1) % 4) == 0)
            mpDebugPrint("");
        else if(((i + 1) % 2) == 0)
            UartOutText("=\\-");
    }
}

/****************************************************************************
 **
 ** NAME:           StrToInt
 **
 ** PARAMETERS:     pointer of charactor
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Convert a HEX string to a unsigned interger.
 **
 ****************************************************************************/
DWORD StrToInt(BYTE * ptr)
{
    SDWORD       i, val = 0;
    BYTE       c;

    if(ptr[0] == '0' && ptr[1] == 'x')  //Hex
    {
        for(i = 2; i < 10; i++)
        {
            c = ptr[i];
            if(c >= '0' && c <= '9')
                c -= '0';
            else if(c >= 'A' && c <= 'F')
                c = c - 'A' + 10;
            else if(c >= 'a' && c <= 'f')
                c = c - 'a' + 10;
            else
                break;

            val = val * 16 + c;
        }
    }
    else
    {
        for(i = 0; i < 10; i++)
        {
            c = ptr[i];
            if(c >= '0' && c <= '9')
                c -= '0';
            else
                break;

            val = val * 10 + c;
        }
    }
    return (val);
}

/* The fillowing functions are for system command */

/****************************************************************************
 **
 ** NAME:           Sys_Dumpmem
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Dump 512 bytes from the specified address.
 **
 ****************************************************************************/
static void Sys_Dumpmem(void)
{
    DWORD      *addr;

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint
            ("Purpose: Read 512 bytes from a specified address of memory");
        mpDebugPrint("Syntax : dump [addr] <Cr>");
        return;
    }
    mpDebugPrint(&parameter[0][0]);
    if(!Check32bit(&parameter[0][0]))
    {
        addr = (DWORD *) StrToInt(&parameter[0][0]);
        if(CheckAlign(addr, 4) == NO_ERR)
            DumpMem32(512, addr);
        else
            mpDebugPrint("alignment error!");
    }
    else
        mpDebugPrint("illegal parameter!");
}

/****************************************************************************
 **
 ** NAME:           Sys_ReadMem
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Dump 4 bytes from the specified address.
 **
 ****************************************************************************/
static void Sys_ReadMem(void)
{
    DWORD      *addr;

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint
            ("Purpose: Read 4 bytes from a specified address of memory");
        mpDebugPrint("Syntax : read [addr] <Cr>");
        return;
    }
    //mpDebugPrint(&parameter[0][0]);
    if(!Check32bit(&parameter[0][0]))
    {
        addr = (DWORD *) StrToInt(&parameter[0][0]);
        if(CheckAlign(addr, 4) == NO_ERR)
            mpDebugPrint("<%08x>=:0x%08x\n", addr, *(DWORD *)addr);
        else
            mpDebugPrint("alignment error!");
    }
    else
        mpDebugPrint("illegal parameter!");
}

/****************************************************************************
 **
 ** NAME:           Edit_write
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Help list.
 **
 ****************************************************************************/
static void Edit_write(void)
{
    DWORD      *addr;

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint("Purpose: Write 4 bytes in a address of memory");
        mpDebugPrint("Syntax : ew [addr] [word] <Cr>");
        return;
    }

    if(!Check32bit(&parameter[0][0]) && !Check32bit(&parameter[1][0]))
    {
        addr = (DWORD *) StrToInt(&parameter[0][0]);
        if(CheckAlign(addr, 4) == NO_ERR)
            *addr = (DWORD) StrToInt(&parameter[1][0]);
        else
            mpDebugPrint("alignment error!");
    }
    else
        mpDebugPrint("illegal parameter!");
}

/****************************************************************************
 **
 ** NAME:           Edit_half
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Help list.
 **
 ****************************************************************************/
static void Edit_half(void)
{
    WORD      *addr;

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint("Purpose: write a half word in a address of memory");
        mpDebugPrint("Syntax : eh [addr] [half word] <Cr>");
        return;
    }

    if(!Check32bit(&parameter[0][0]) && !Check16bit(&parameter[1][0]))
    {
        addr = (WORD *) StrToInt(&parameter[0][0]);
        if(CheckAlign((DWORD *) addr, 2) == NO_ERR)
            *addr = (WORD) StrToInt(&parameter[1][0]);
        else
            mpDebugPrint("alignment error!");
    }
    else
        mpDebugPrint("illegal parameter!");
}

/****************************************************************************
 **
 ** NAME:           Edit_byte
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Help list.
 **
 ****************************************************************************/
static void Edit_byte(void)
{
    BYTE      *addr;

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint("Purpose: write a byte in a address of memory");
        mpDebugPrint("Syntax : eb [addr] [byte] <Cr>");
        return;
    }

    if(!Check32bit(&parameter[0][0]) && !Check8bit(&parameter[1][0]))
    {
        addr = (BYTE *) StrToInt(&parameter[0][0]);
        *addr = (BYTE) StrToInt(&parameter[1][0]);
    }
    else
        mpDebugPrint("illegal parameter!");
}
void SetKModuleStart(BYTE flag)
{
    KModuleStart = flag;
}
/****************************************************************************
 **
 ** NAME:           kmod_exec
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    execute kmod file.
 **
 ****************************************************************************/
static void kmod_exec(void)
{
    DWORD i;
    SBYTE *argv[PARA_NUM];

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint("Purpose: kmod");
        mpDebugPrint("Syntax : kmod <Cr>");
        return;
    }
    mpDebugPrint("execute %s.elf", &parameter[0][0]);
    i=0;
    while(parameter[i+1][0])
    {
        argv[i] = &parameter[i+1][0];
        i++;
    }
    Loader_init(&parameter[0][0], TRUE, i, argv);
}

/****************************************************************************
 **
 ** NAME:           kmod_nonfree_exec
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    execute kmod file.
 **
 ****************************************************************************/
static void kmod_nonfree_exec(void)
{
    DWORD i;
    SBYTE *argv[PARA_NUM];

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint("Purpose: kmodm");
        mpDebugPrint("Syntax : kmodm <Cr>");
        return;
    }
    mpDebugPrint("execute %s.elf", &parameter[0][0]);
    i=0;
    while(parameter[i+1][0])
    {
        argv[i] = &parameter[i+1][0];
        i++;
    }
    Loader_init(&parameter[0][0], FALSE, i, argv);
}

extern int SPP_Server_Open(void);
extern int SPP_Init(void);
extern int SPP_Client_Connect(void);
/****************************************************************************
 **
 ** NAME:           Testproc
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    execute lua script file.
 **
 ****************************************************************************/
static void Testproc(void)
{
    BYTE tmp[64];
    DWORD len;

    if(!strcmp("-h", &parameter[0][0]))
    {
        mpDebugPrint("Purpose: test");
        mpDebugPrint("Syntax : test <Cr>");
        return;
    }
    #if 0
    switch(parameter[0][0])
    {
        case 'c':
        mpDebugPrint("spp connect");
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_SPP))
        MpxBtSppClientConnect();
#endif
        break;
        case 'o':
        mpDebugPrint("spp server open");
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_SPP))
        MpxBtSppServerOpen();
#endif
        break;
        case 's':
        mpDebugPrint("spp server close");
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_SPP))
        MpxBtSppServerClose();
#endif
        break;
        case 'r':
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_SPP))
        len = MpxBtSppServerRxLen();
#endif
        mpDebugPrint("spp read %d", len);
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_SPP))
        MpxBtSppServerRead(&tmp, len);
#endif
        tmp[len] = 0;
        mpDebugPrint("%s", tmp);
        break;
        case 'w':
        mpDebugPrint("spp write");
#if ((BLUETOOTH == ENABLE)&&(BT_PROFILE_TYPE & BT_SPP))
        MpxBtSppServerWrite(parameter[1], strlen(parameter[1]));
#endif
        break;
        default:
        mpDebugPrint("Please assign the parameter");
    }
#endif
BtApiInit(NULL);
    //HandlerEnable(TRUE);
    //__asm("break 100");
}

/*******************************************************
    Endian Translation Functions
********************************************************/
SWORD endianTrans_short(SWORD source)
{
    SBYTE       buffer[2];
    SWORD      *result;
    BYTE      *temp;

    temp = (SBYTE *) (&source);
    buffer[1] = temp[0];
    buffer[0] = temp[1];

    result = (SWORD *) buffer;

    return (*result);

}

SDWORD endianTrans_long(SDWORD source)
{
    SBYTE       buffer[4];
    SDWORD      *result;
    SBYTE      *temp;

    temp = (SBYTE *) (&source);
    buffer[3] = temp[0];
    buffer[2] = temp[1];
    buffer[1] = temp[2];
    buffer[0] = temp[3];

    result = (SDWORD *) buffer;

    return (*result);

}

/****************************************************************
    FUNCTION : void getToken(char *str, char *dest)
                   get a token from str  and copy to token
    noted:
        need "StartPosition"

        "StartPosition" is a pointer for starting search position

*****************************************************************/
void getToken(SBYTE * str, SBYTE * token)
{
    SDWORD       p;

    p = StartPosition;          /* get the starting search position */
    while(str[p] != '\0')       /* '\0'  end-of-string */
    {
        if((str[p] >= 33) && (str[p] <= 126))   /*  33=> '!'   126=> '~' */
        {
            do
            {
                *token++ = str[p++];
            }
            while((str[p] != ' ') && (str[p] != '\0'));

            StartPosition = p;  /* next position for get token */
            *token = '\0';
            return;
        }
        p++;
    }
    *token = '\0';
}

/****************************************************************
    FUNCTION : void upperToLowerCase(str1)
               convert upper case string to lower case
*****************************************************************/
void upperToLowerCase(SBYTE * str)
{
    while(*str != '\0')
    {
        if((*str >= 'A') && (*str <= 'Z'))
            *str += 'a' - 'A';  /* offset */
        str++;
    }
}

/****************************************************************
    FUNCTION : void strCompare(char *str1, char *str2)
               ==>  1: str1 , str2 are equal
                    0: not equal
*****************************************************************/
SDWORD strCompare(SBYTE * pattern, SBYTE * str1)
{
    upperToLowerCase(str1);
    while(*pattern == *str1)
    {
        if(*pattern == '\0')
            return (1);
        pattern++;
        str1++;
    }
    return (0);
}

/****************************************************************
    FUNCTION :int getCommand(char * str)
*****************************************************************/
SDWORD getCommand(SBYTE * str)
{
    SDWORD       i, j;
    SBYTE       cmdStr[CmdStringLength];

    StartPosition = 0;          /* set the starting search position for getToken() */
    getToken(str, cmdStr);
    if(cmdStr[0] == '\0')       /* no token */
        return (FAIL);

    for(i = 0; i < NoCmd - 1; i++)
    {
        if(cmdtab[i].cmd == 0)  // (cmdtab[i].cmd == NULL)
            return (NoCmd - 1); /* invalid command */

        if(strCompare(cmdtab[i].cmd, cmdStr))
        {
            for(j = 0; j < PARA_NUM; j++)       /* Clear Parameter list */
                parameter[j][0] = '\0';

            for(j = 0; j < PARA_NUM; j++)
            {
                getToken(str, &parameter[j][0]);
                if(parameter[j][0] == '\0')
                    break;      /* No any more parameter */
            }
            return (i);
        }
    }

    return (i);
}

/****************************************************************
    FUNCTION : void clearString(char * str, int n);
          clear string with space char ' '
*****************************************************************/
void clearString(SBYTE * str, SDWORD n)
{
    SDWORD       i;

    for(i = 0; i < n - 1; i++)
    {
        str[i] = ' ';
    }
    str[n - 1] = '\0';
}

/****************************************************************
    FUNCTION : void prompt(void)
*****************************************************************/
static void prompt(void)
{
    SBYTE       temp[PromptLength];
    SDWORD       i;

    i = StartPosition;

    while((i < CmdStringLength) && (cmds[i] == ' '))
    {
        i++;
    }

    memcpy(temp, &cmds[i], PromptLength - 2);

    getToken(cmds, cmds);
    if(cmds[0] != '\0')
    {
        for(i = 0; i < PromptLength - 2; i++)
        {
            if(temp[i] == '\0')
                break;
            promptString[i] = temp[i];
        }
        promptString[i++] = ' ';
        promptString[i] = '\0';
    }
}

/****************************************************************
    FUNCTION : int isValidInput(char * str)
*****************************************************************/
/*      if input string has visiable char then return 1
    else return 0.
*/
SDWORD isValidInput(SBYTE * str)
{
    SDWORD       i = 0;

    while(str[i] != '\0')
    {
        if((str[i] < 33) && (str[i] > 126))     /*   space char ' ' => 32 */
            return (0);
        i++;
    }
    return (1);

}

/****************************************************************************
 **
 ** NAME:           mpxCheckComCount
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:
 **
 ****************************************************************************/
BOOL mpxCheckComCount(void)
{
    if (inPtr!=outPtr)
        return TRUE;
    else
        return FALSE;
}

/****************************************************************************
 **
 ** NAME:           mpxKeyGetc
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    geting a char.
 **
 ****************************************************************************/
SBYTE mpxKeyGetc(void)
{
    SBYTE       data;

    if (inPtr!=outPtr)
        data = (SBYTE)*outPtr++;
    if((outPtr-_buffer)>=COM_BUFFER_SIZE)
        outPtr = _buffer;

    return (data);
}

/****************************************************************
    FUNCTION : int getString(char * buffer)
*****************************************************************/
/* get command string with history function (function Up arrow or Down arrow)

    if press ESC then return 0 (not successful), else return 1.
*/
SDWORD getString(SBYTE * buffer)
{
    SDWORD       Timeout;
    SBYTE       aChar;

    if(mpxCheckComCount() == 0)    //no char
        return ERR_SHELL_NODATA;

    if((aChar = mpxKeyGetc()) != 0x0d)    /* 0x0d => Enter */
    {
        if(aChar == 0x1b)       /* ESC */
        {
            Timeout = 0xFFFF;
            while((mpxCheckComCount() == 0) && (--Timeout != 0));
            if(Timeout == 0)
            {
                buffer[0] = '\0';       /* Clear cmd buffer */
                pindex = 0;     /* Reset the index */
                mpDebugPrint("Esc--> canel..");
                return (ERR_SHELL_GENERAL_FAIL);        /* not valid command */
            }

            if((aChar = mpxKeyGetc()) == 0x5b)    /* control chars, function keys (2 chars) */
            {
                if(!mpxCheckComCount())    //no char
                    return ERR_SHELL_NODATA;

                if((aChar = mpxKeyGetc()) == 0x41)        /* Up arrow, repeat last command */
                {
                    if(lastCmd[lastCmdIdx][0] != '\0')
                    {
                        buffer[pindex] = '\0';  /* noted for command length in cmdCopy() */
                        strcpy(buffer, &lastCmd[lastCmdIdx][0]);
                        while(pindex > 0)
                        {
                            delChar();
                            pindex--;
                        }
                        pindex = strlen(buffer);
                        lastCmdIdx =
                            (lastCmdIdx - 1 + HISTORY_NO) % HISTORY_NO;
                        saveCmdIdx = (saveCmdIdx - 1) % HISTORY_NO;
                        UartOutText(buffer);
                    }
                }
                else if(aChar == 0x42)  /* Down arrow, repeat previous command */
                {
                    if(lastCmd[(lastCmdIdx + 2) % HISTORY_NO][0] != '\0')
                    {
                        buffer[pindex] = '\0';  /* noted for command length in cmdCopy() */
                        strcpy(buffer,
                               &lastCmd[(lastCmdIdx + 2) % HISTORY_NO][0]);
                        while(pindex > 0)
                        {
                            delChar();
                            pindex--;
                        }
                        pindex = strlen(buffer);
                        lastCmdIdx = saveCmdIdx;
                        saveCmdIdx = (saveCmdIdx + 1) % HISTORY_NO;
                        UartOutText(buffer);
                    }
                }
            }
        }
        else if(aChar == 8)     /* backspace */
        {
            if(pindex != 0)
            {
                delChar();
                pindex--;
            }
        }
        else if (aChar == '$')//ASP packet
        {
            buffer[pindex] = '\0';  /* clear the buffer of the input command */
            pindex = 0;
            return ERR_SHELL_GETASP_FAIL;
        }
        else if((aChar >= 32) && (aChar <= 126))        /* print out the normal char */
        {
            if(pindex < CmdStringLength - 1)    /* < buffer size 128 bytes */
            {
                buffer[pindex++] = aChar;
                PutUartChar((BYTE)aChar);
            }
        }
        return ERR_SHELL_NODATA;
    }
    else
    {
        buffer[pindex] = '\0';  /* add end-of-line to the input command */
        pindex = 0;             /* Reset the index */
        if(isValidInput(buffer))        /* if input a valid char */
        {
            if(buffer[0] != '\0')
            {
                strcpy(&lastCmd[saveCmdIdx][0], buffer);
                lastCmdIdx = saveCmdIdx;
                saveCmdIdx = (saveCmdIdx + 1) % HISTORY_NO;
                return (PASS);
            }
            else
                return (ERR_SHELL_GENERAL_FAIL);
        }
        else
        {
            mpDebugPrint("Bad Command!");
            buffer[0] = '\0';   /* Clear cmd buffer */
            return (ERR_SHELL_GENERAL_FAIL);
        }
    }
}

/****************************************************************************
 **
 ** NAME:           Shell_init
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    initial the variable of the shell.
 **
 ****************************************************************************/
void Shell_init(void)
{
    SDWORD       i;

    /* initial buffer for save command string */
    for(i = 0; i < HISTORY_NO; i++)
        lastCmd[i][0] = '\0';

    lastCmdIdx = 0;
    saveCmdIdx = 1;
    cmds[0] = '\0';

    //listCmd();
    UartOutText(promptString);
}

/****************************************************************************
 **
 ** NAME:           ShellEntry
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    The entry point of the Kevin shell.
 **
 ****************************************************************************/
static void ShellEntry(void)
{
    SDWORD       c, isValid;
    while(mpxCheckComCount())
    {
        if((isValid = getString(cmds)) == NO_ERR)
        {
            mpDebugPrint("");
            c = getCommand(cmds);
            switch (c)
            {
                case -1:       /* no token, done nothing */
                    break;
                case NoCmd - 1:
                    mpDebugPrint("Bad Command!");
                    break;
                default:
                    (*cmdtab[c].func) ();       /* execute the function */
            }
            UartOutText(promptString);
            //clearString( cmds, CmdStringLength );
            cmds[0] = '\0';
        }
        else if(isValid == ERR_SHELL_GETASP_FAIL)
        {
            tConsHandler();
            Shell_init();
        }
        else if(isValid == ERR_SHELL_GENERAL_FAIL)
        {
            mpDebugPrint("");
            UartOutText(promptString);
        }
    }
}

/****************************************************************************
 **
 ** NAME:           tConsTask
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  None
 **
 ** DESCRIPTION:    Shell task main function.
 **
 ****************************************************************************/
void tConsTask(void)
{
    DWORD dwEvent;

//    TaskChangePriority(DRIVER_PRIORITY);
    IntEnable();

    while(1)
    {
        EventWait(TCONS_EVENT, EVENT_MASK, OS_EVENT_OR, &dwEvent);
        ShellEntry();
    }
}

static void UartCB(DWORD cause)
{
    BYTE event;
    if (cause & RX_READY)
    {
        while(CheckUartStatus(RX_READY)==PASS)
        {
            *inPtr++ = GetUartChar();
            if ((inPtr-_buffer)>=COM_BUFFER_SIZE)
                inPtr = _buffer;
        }
        if(KModuleStart == 0)
        {
        EventSet(TCONS_EVENT, 0x01);
    }
        else
        {
            event = 0x55;
            MessageDrop(BT_UI_MSG_ID, (BYTE *)&event, 1);
        }
    }
}

/****************************************************************************
 **
 ** NAME:           tConsoleEnable
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    initial, create Shell task.
 **
 ****************************************************************************/
void tConsoleEnable(void)
{
    SDWORD sdwRetVal;

    inPtr=outPtr=_buffer;
#if DEBUG_COM_PORT == HUART_A_INDEX
    HUartRegisterCallBackFunc(HUART_A_INDEX,UartCB);
#elif DEBUG_COM_PORT == HUART_B_INDEX
    HUartRegisterCallBackFunc(HUART_B_INDEX,UartCB);
#endif
    HandlerEnable(FALSE);//Diable GDB agent
    TaskCreate(TCONS_TASK, tConsTask, CONTROL_PRIORITY, 0x4000*4);
    sdwRetVal = EventCreate(TCONS_EVENT, (OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR), 0);
    TaskStartup (TCONS_TASK);

    Shell_init();
}

/****************************************************************************
 **
 ** NAME:           tConsoleDisable
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:
 **
 ****************************************************************************/
void tConsoleDisable(void)
{
    HUartClearCallBackFunc(HUART_B_INDEX);
    EventDestroy(TCONS_EVENT);
    TaskTerminate(TCONS_TASK);
    HandlerEnable(TRUE);
}

/****************************************************************************
 **
 ** NAME                :   UsbReadWritePattern
 **
 ** PARAMETERS     :   Port( = 0)  & Lun( = 0)
 **
 ** RETURN VALUES :  None
 **
 ** DESCRIPTION    :   Test Read/Write 512 Sectors Pattern for PenDrive
 **
 ****************************************************************************/
static void UsbReadWritePattern(void)
{
    BYTE    bLun = 0;
    WHICH_OTG   eWhichOtg = USBOTG0;

    //MP_ALERT("Para1:%d Para2:%d", parameter[0][0], parameter[1][0]);

    if(parameter[0][0] != '\0')
    {
        eWhichOtg = parameter[0][0] - 0x30;
        if(eWhichOtg < USBOTG0 || eWhichOtg > USBOTG1)
        {
            MP_ALERT("--E-- Port Error !!");
            return;
        }
    }

    if(parameter[1][0] != '\0')
    {
        bLun = parameter[1][0] - 0x30;
        if(bLun < 0 || bLun >= MAX_HOST_LUN)
        {
            MP_ALERT("--E-- Lun Error !!");
            return;
        }
    }

    MP_ALERT("-USBOTG%d- LUN:%d Start Testing...", eWhichOtg, bLun);
    UsbOtgHostTestFunction (bLun, 0, 0, 0, eWhichOtg);

}

MPX_KMODAPI_SET(SetKModuleStart);
MPX_KMODAPI_SET(TaskGetId);



MPX_KMODAPI_SET(mpxCheckComCount);
MPX_KMODAPI_SET(mpxKeyGetc);
MPX_KMODAPI_SET(MessageReceive);
MPX_KMODAPI_SET(MessageDrop);
MPX_KMODAPI_SET(MessageCreate);

// Event.c
MPX_KMODAPI_SET(EventWait);
MPX_KMODAPI_SET(EventPolling);
MPX_KMODAPI_SET(EventClear);




