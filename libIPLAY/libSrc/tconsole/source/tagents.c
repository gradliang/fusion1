/*
*******************************************************************************
*                           Magic Pixel Inc. Inc.                             *
*                  Copyright (c) 2009 -, All Rights Reserved                  *
*                                                                             *
* Handle ASP packet                                                           *
*                                                                             *
* File : tagents.c                                                            *
* By : Kevin Huang                                                            *
*                                                                             *
*                                                                             *
* Description : Get ASP like packet and dothe handshake, this code is         *
*               modified from the agents.c                                    *
*                                                                             *
* History : 2009/03/19 File created.                                          *
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

/*************************************************************************/
/***               Public Variable Declaration                         ***/
/*************************************************************************/
#define BUFFER_SIZE		(4096)
#define MAX_ARGC 32

// variable declaration
static BYTE tConsInBuffer[BUFFER_SIZE];
static BYTE tConsOutBuffer[BUFFER_SIZE];
static BYTE *tConsInPoint, *tConsOutPoint;

static const BYTE tConsHexChar[] = "0123456789abcdef";
static const BYTE tConsOkString[4] = "OK";

/****************************************************************************
 **
 ** NAME:           tConsWriteByte
 **
 ** PARAMETERS:     point, byte
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsWriteByte(BYTE * point, BYTE byte)
{
    *point = tConsHexChar[byte >> 4];

    point++;
    *point = tConsHexChar[byte & 0x0f];

    return ++point;
}

/****************************************************************************
 **
 ** NAME:           tConsWriteCharByte
 **
 ** PARAMETERS:     point, text, byte
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsWriteCharByte(BYTE * point, BYTE text, BYTE byte)
{
    *point = text;

    point++;
    point = tConsWriteByte(point, byte);

    return point;
}

/****************************************************************************
 **
 ** NAME:           tConsWriteString
 **
 ** PARAMETERS:     point, string
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsWriteString(BYTE * point, BYTE * string)
{
    while (*string)
    {
        *point = *string;
        point++;
        string++;
    }

    return point;
}

/****************************************************************************
 **
 ** NAME:           tConsWriteWord
 **
 ** PARAMETERS:     point, word
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsWriteWord(BYTE * point, DWORD word)
{
    point = tConsWriteByte(point, word >> 24);
    point = tConsWriteByte(point, (word >> 16) & 0xff);
    point = tConsWriteByte(point, (word >> 8) & 0xff);
    point = tConsWriteByte(point, word & 0xff);

    return point;
}

/****************************************************************************
 **
 ** NAME:           tConsWriteCharWord
 **
 ** PARAMETERS:     point, text, word
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsWriteCharWord(BYTE * point, BYTE text, DWORD word)
{
    *point = text;
    point++;
    point = tConsWriteWord(point, word);

    return point;
}

/****************************************************************************
 **
 ** NAME:           tConsWriteChar
 **
 ** PARAMETERS:     point, text
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsWriteChar(BYTE * point, BYTE text)
{
    *point = text;

    return ++point;
}

/****************************************************************************
 **
 ** NAME:           tConsReadNibble
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE tConsReadNibble(void)
{
    BYTE byte, nibble;

    nibble = *tConsInPoint;
    tConsInPoint++;

    if (nibble < '0')
        nibble = 0xff;
    else if (nibble <= '9')
        nibble = nibble - '0';
    else if (nibble < 'A')
        nibble = 0xff;
    else if (nibble <= 'F')
        nibble = nibble - 'A' + 10;
    else if (nibble < 'a')
        nibble = 0xff;
    else if (nibble <= 'f')
        nibble = nibble - 'a' + 10;
    else
        nibble = 0xff;

    return nibble;
}

/****************************************************************************
 **
 ** NAME:           tConsReadByte
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE tConsReadByte(void)
{
    BYTE byte;

    byte = tConsReadNibble() << 4;
    byte = byte | tConsReadNibble();

    return byte;
}

/****************************************************************************
 **
 ** NAME:           tConsReadWord
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static DWORD tConsReadWord(void)
{
    DWORD word;

    word = tConsReadNibble();
    word = word << 8;
    word = word | tConsReadNibble();
    word = word << 8;
    word = word | tConsReadNibble();
    word = word << 8;
    word = word | tConsReadNibble();

    return word;
}

/****************************************************************************
 **
 ** NAME:           tConsReadVariable
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static DWORD tConsReadVariable(void)
{
    DWORD variable;
    BYTE piece;

    variable = 0;

    while (1)
    {
        piece = tConsReadNibble();
        if (piece == 0xff)
            break;

        variable = (variable << 4) + piece;
    }

    return variable;
}

/****************************************************************************
 **
 ** NAME:           tConsReadChar
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE tConsReadChar(void)
{
    return *tConsInPoint++;
}

/****************************************************************************
 **
 ** NAME:           myGetUartChar
 **
 ** PARAMETERS:     data
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Modify the official version of GetUartChar().
 **                 the Zero could be the data.
 **
 ****************************************************************************/
static SBYTE myGetUartChar(BYTE *data)
{
    if(mpxCheckComCount())
    {
        *data = (BYTE) mpxKeyGetc();
        return PASS;
    }
    else
        return FAIL;
}

/****************************************************************************
 **
 ** NAME:           tConsRecv
 **
 ** PARAMETERS:     
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Wait for UART income.
 **
 ****************************************************************************/
static SDWORD tConsRecv(BYTE * data)
{
    DWORD count;

    count = 100;
    while( count-->0 && myGetUartChar(data)<0)
    {
        TaskSleep(10);
    }
    if (count)
        return PASS;
    else
        return FAIL;
}

/****************************************************************************
 **
 ** NAME:           tConsGetPacket
 **
 ** PARAMETERS:     escape
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Start to get packet into INbuffer.
 **
 ****************************************************************************/
static SDWORD tConsGetPacket(DWORD * escape)
{
    BYTE checksum;
    BYTE byte;

    tConsInPoint = tConsInBuffer;
    checksum = 0;
    byte = 0;
    *escape = 0;
    while (byte != '#')
    {
        checksum += byte;
        if (tConsRecv(&byte))
            return FAIL;

        if (byte == 0x7d)	// if '}' input, mean the next byte has been escaped
        {
            checksum += byte;	// also need to calculate the checksum
            if (tConsRecv(&byte))// read one more time
                return FAIL;
            *tConsInPoint = byte + 0x20;	// the escaped byte has been subtracted by 0x20 at sender end
            tConsInPoint++;
            (*escape)++;
        }
        else
        {
            *tConsInPoint = byte;
            tConsInPoint++;
        }
    }

    if (tConsRecv(tConsInPoint))
        return FAIL;
    tConsInPoint++;
    if (tConsRecv(tConsInPoint))
        return FAIL;
    tConsInPoint--;

    byte = tConsReadByte();
    if (checksum == byte)
        PutUartChar('+');
    else
        PutUartChar('-');

    if (checksum != byte)
        return FAIL;
    else
    {
        // reset the in-buffer point to origin for next accessing
        tConsInPoint = tConsInBuffer;

        // initialize the output buffer
        tConsOutBuffer[0] = '$';
        tConsOutPoint = &tConsOutBuffer[1];
        return PASS;
    }
}

/****************************************************************************
 **
 ** NAME:           tConsPutPacket
 **
 ** PARAMETERS:     buffer, point
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static BYTE *tConsPutPacket(BYTE * buffer, BYTE * point)
{
    BYTE k, tmp;
    DWORD i, length;
    SDWORD status;

    // terminate the output sequence
    *point = '#';
    point = buffer + 1;

    // calculate the checksum
    length = 4;
    k = 0;
    while (*point != '#')
    {
        length++;
        k += *point;
        point++;
    }

    // attach the checksum
    point++;
    point = tConsWriteByte(point, k);

    // send output sequence to GDB
    do
    {
        i = length;
        point = buffer;

        while (i)
        {
            PutUartChar(*point);
            point++;
            i--;
        }
        status = tConsRecv(&tmp);
    } while ( tmp == '-' && status!=0 );	// if fail then again


    *buffer = '$';

    return buffer + 1;
}

/****************************************************************************
 **
 ** NAME:           countArgc
 **
 ** PARAMETERS:     size, p_byte, argv
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    
 **
 ****************************************************************************/
static DWORD countArgc(DWORD size, BYTE *p_byte, SBYTE *argv[])
{
    DWORD count=0;
    BYTE *start;
    
    start = p_byte;
    while (size--)
    {
        if ( !(*p_byte) )
        {
            argv[count] = start;
            count++;
            start = p_byte + 1;
        }
        p_byte++;
        if (count >= MAX_ARGC)
            break;
    }
    
    return count;
}

/****************************************************************************
 **
 ** NAME:           tConsHandler
 **
 ** PARAMETERS:     None
 **
 ** RETURN VALUES:  Result
 **
 ** DESCRIPTION:    Start to wait for the next charactor after '$'.
 **
 ****************************************************************************/
void tConsHandler(void)
{
    BYTE command, *p_byte;
    DWORD argc, size, a, b;
    DRIVE *sDrv;
    STREAM *shandle;
    SDWORD ret, i;
    DWORD escape;
    SBYTE *argv[MAX_ARGC];
    static SDWORD result=0;
    
    // wait for any GDB command and do the proper service
    // exit when get the continue or the step commands
    if (tConsGetPacket(&escape))//can not receive the complete packet
        return;
    else
    {
        command = tConsReadChar();
        switch (command)
        {
            case 'm':
            p_byte = (BYTE *) tConsReadVariable();	// read the start address
            a = tConsReadVariable();	// read the request length

            while (a)
            {
                tConsOutPoint = tConsWriteByte(tConsOutPoint, *p_byte);
                p_byte++;
                a--;
            }
            break;

            case 'M':				// write memory packet
            p_byte = (BYTE *) tConsReadVariable();	// read start address
            p_byte += 0x20000000;
            a = tConsReadVariable();	// read the length
            while (a)
            {
                *p_byte = tConsReadByte();
                p_byte++;
                a--;
            }
            tConsOutPoint = tConsWriteString(tConsOutPoint, (BYTE *)tConsOkString);
            break;

            case 'X':				// write memory packet (binary)
            p_byte = (BYTE *) tConsReadVariable();	// read the start address
            p_byte += 0x20000000;
            a = tConsReadVariable()-escape;	// read the length
            while (a)
            {
                *p_byte = tConsReadChar();	// the only place differ to 'M' packet
                p_byte++;
                a--;
            }
            tConsOutPoint = tConsWriteString(tConsOutPoint, (BYTE *)tConsOkString);
            break;

            case 'L':				// save kmod file (binary)
            case 'Q':
            a = tConsReadVariable()-escape;	// read the length
            p_byte = (BYTE *)ext_mem_malloc(a);
            for(i=0; i<a; i++)
            {
                *(p_byte+i) = tConsReadChar();	// the only place differ to 'M' packet
            }
            tConsOutPoint = tConsWriteString(tConsOutPoint, (BYTE *)tConsOkString);
            sDrv=DriveChange(SD_MMC);
            if ( command == 'L' )
            {
                if (FileSearch(sDrv, "SYS", "ELF", E_FILE_TYPE) == 0)
                {
                    shandle = FileOpen(sDrv);
                    DeleteFile(shandle);
        			  FileClose(shandle);
                }
                ret=CreateFile(sDrv, "SYS", "ELF");
                if (!ret)
                {
                    shandle = FileOpen(sDrv);
                    FileWrite(shandle, p_byte, a);
                    FileClose(shandle);
                }
            }
            else
            {
                FileSearch(sDrv, "SYS", "ELF", E_FILE_TYPE);
                shandle = FileOpen(sDrv);
                //Seek(shandle, FileSizeGet(shandle));
                EndOfFile(shandle);
                FileWrite(shandle, p_byte, a);
                FileClose(shandle);
            }
            ext_mem_free(p_byte);
            break;
            case 'K':				// execute kmod file (binary)
            size = a = tConsReadVariable()-escape;	// read the length
            p_byte = (BYTE *) ext_mem_malloc(a);
            for(i=0; i<a; i++)
            {
                *(p_byte+i) = tConsReadChar();	// the only place differ to 'M' packet
            }
            tConsOutPoint = tConsWriteString(tConsOutPoint, (BYTE *)tConsOkString);
            break;
            case 'r':
            tConsOutPoint = tConsWriteWord(tConsOutPoint, (DWORD)result);
            break;
            default:
            ;
        }
        tConsOutPoint = tConsPutPacket(tConsOutBuffer, (BYTE *)tConsOutPoint);
        if (command == 'K')
        {
            //BYTE tmp;
            
            argc = countArgc(size, p_byte, argv);
            result = Loader_init("SYS", TRUE, (SDWORD)argc, argv);
            ext_mem_free(p_byte);
            PutUartChar('+');//end
        }
    }
}

