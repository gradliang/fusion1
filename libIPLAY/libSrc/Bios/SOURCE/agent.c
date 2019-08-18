
/*
==========================================================================================================
Module: GDB agent and debugging API

Description:
    To enable debugger support, one things need to happen.  A breakpoint needs to be generated to begin
    communication.  This is most easily accomplished by a call to BreakPoint().  Breakpoint() simulates
    a breakpoint by executing a BREAK instruction.

    The following gdb commands are supported:

    command         function                               Return value

      g             return the value of the CPU registers  hex data or ENN
      G             set the value of the CPU registers     OK or ENN

      mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
      MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN

      c             Resume at current address              SNN   ( signal NN)
      cAA..AA       Continue at address AA..AA             SNN

      s             Step one instruction                   SNN
      sAA..AA       Step one instruction from AA..AA       SNN

      k             kill

      ?             What was the last sigval ?             SNN   (signal NN)

      bBB..BB	    Set baud rate to BB..BB baud rate	   OK or BNN, then sets

    ll commands and responses are sent with a packet which includes a checksum.  A packet consists of

    <packet info>#<checksum>.

    where
    <packet info> :: <characters representing the command or response>
    <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>

    when a packet is received, it is first acknowledged with either '+' or '-'.	'+' indicates a successful
    transfer.  '-' indicates a failed transfer.

    Example:

    Host:                  Reply:
    $m0,10#2a              +$00010203040506070809101112131415#42

==========================================================================================================

Copyright(c) 2003-2004 by Magic Pixel Inc.
All rights reserved, including the rights of duplication and modification in whole or in part in any form.

==========================================================================================================
*/



// header file including
#include "iplaysysconfig.h"

#include "exception.h"
#include "string.h"
#include "mptrace.h"
#include "bios.h"

// constant and macro definitions
#define Z_RSP_ENABLE        0

#define BUFFER_SIZE         4000

#define OP_SPECIAL          0
#define OP_REGIMM           1
#define OP_J                2
#define OP_JAL              3
#define OP_BEQ              4
#define OP_BNE              5
#define OP_BLEZ             6
#define OP_BGTZ             7

#define SPEC_FUNC_JR        8
#define SPEC_FUNC_JALR      9

#define REGI_FUNC_BLTZ      0
#define REGI_FUNC_BGEZ      1
#define REGI_FUNC_BLTZAL    16
#define REGI_FUNC_BGEZAL    17

#define INSTRUCTION_BREAK   0x0000000d

#define POSIX_SIGTRAP       0x05
#define GDB_FR_EPC          37
#define GDB_FR_FP           72
#define GDB_FR_GP           28
#define GDB_FR_SP           29
#define FR_RESIDUE_COUNT    (90 - 38)

#define LOCAL_BREAK_CODE    ((100 << 16) + INSTRUCTION_BREAK)
#define DEBUG_PRINT_STRING  200
#define DEBUG_PRINT_WORD    300
#define DP_STRING_CODE      ((DEBUG_PRINT_STRING << 16) + INSTRUCTION_BREAK)
#define DP_WORD_CODE        ((DEBUG_PRINT_WORD << 16) + INSTRUCTION_BREAK)

#define NUMBER_OF_BP        10

#define DEBUG_BLOCK_ADDR    0x80001000

typedef struct
{
    DWORD *Address;
    DWORD Instruction;
} BP_BACKUP;



typedef struct                  // this DEBUG BLOCK will locate at 0x80001000 and can not be moved
{                               // so that the loaded GDB stub can access the data that Booted GDB stub
    DWORD Counter;              // put.
    BP_BACKUP BreakPoint[NUMBER_OF_BP];
} DEBUG_BLOCK;


// variable declaration
static DWORD boolGdbConnect = 0;

static BP_BACKUP Step, Branch;
static BYTE inBuffer[BUFFER_SIZE];
static BYTE outBuffer[BUFFER_SIZE];
static BYTE textBuffer[128];
static BYTE *inPoint, *outPoint;
static DWORD localBreakOffset;

static const BYTE hexChar[] = "0123456789abcdef";
static const BYTE sectorOffsetString[] = "Text=0;Data=0;Bss=0";
static BYTE okString[4];
static BOOL HandlerEn = TRUE;

static void agentPutChar(BYTE buffer)
{
#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc() == TRUE)
        UsbPutChar(buffer);
    else
#endif
        PutUartChar(buffer);
}



static BYTE agentGetChar(void)
{
#if USBDEVICE_CDC_DEBUG
    if (GetIsAbleToUseUsbdCdc() == TRUE)
        return UsbGetChar();
    else
#endif
        return GetUartChar();
}



BYTE *writeByte(BYTE * point, BYTE byte)
{
    *point = hexChar[byte >> 4];
    point++;
    *point = hexChar[byte & 0x0f];

    return ++point;
}



BYTE *writeCharByte(BYTE * point, BYTE text, BYTE byte)
{
    *point = text;

    point++;
    point = writeByte(point, byte);

    return point;
}



BYTE *writeString(BYTE * point, BYTE * string)
{
    while (*string)
    {
        *point = *string;
        point++;
        string++;
    }

    return point;
}



BYTE *writeWord(BYTE * point, DWORD word)
{
    point = writeByte(point, word >> 24);
    point = writeByte(point, (word >> 16) & 0xff);
    point = writeByte(point, (word >> 8) & 0xff);
    point = writeByte(point, word & 0xff);

    return point;
}



BYTE *writeCharWord(BYTE * point, BYTE text, DWORD word)
{
    *point = text;
    point++;
    point = writeWord(point, word);

    return point;
}



BYTE *writeChar(BYTE * point, BYTE text)
{
    *point = text;

    return ++point;
}



BYTE checkInput()
{
    return *inPoint;
}



BYTE readNibble()
{
    register BYTE byte, nibble;

    nibble = *inPoint;
    inPoint++;

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



BYTE readByte()
{
    register BYTE byte;

    byte = readNibble() << 4;
    byte = byte | readNibble();

    return byte;
}



DWORD readWord()
{
	register DWORD word;

	word = readNibble();
	word = word << 4;
	word = word | readNibble();
	word = word << 4;
	word = word | readNibble();
	word = word << 4;
	word = word | readNibble();
	word = word << 4;
	word = word | readNibble();
	word = word << 4;
	word = word | readNibble();
	word = word << 4;
	word = word | readNibble();
	word = word << 4;
	word = word | readNibble();

	return word;
}



DWORD readVariable()
{
    register DWORD variable;
    register BYTE piece;

    variable = 0;

    while (1)
    {
        piece = readNibble();
        if (piece == 0xff)
            break;

        variable = (variable << 4) + piece;
    }

    return variable;
}



BYTE readChar()
{
    return *inPoint++;
}


DWORD readWordBinary()
{
	register DWORD word;

	word = *inPoint++;
	word = word << 8;
	word = word | (*inPoint++);
	word = word << 8;
	word = word | (*inPoint++);
	word = word << 8;
	word = word | (*inPoint++);

	return word;
}


void getPacket()
{
    register BYTE checksum;
    register BYTE byte;

    do
    {
        inPoint = inBuffer;
        checksum = 0;
        while (agentGetChar() != '$');
        byte = 0;

        while (byte != '#')
        {
            checksum += byte;
            byte = agentGetChar();

            if (byte == 0x7d)   // if '}' input, mean the next byte has been escaped
            {
                checksum += byte;       // also need to calculate the checksum
                byte = agentGetChar();   // read one more time
                *inPoint = byte + 0x20; // the escaped byte has been subtracted by 0x20 at sender end
                inPoint++;
            }
            else
            {
                *inPoint = byte;
                inPoint++;
            }
        }

        *inPoint = agentGetChar();
        inPoint++;
        *inPoint = agentGetChar();
        inPoint--;

        byte = readByte();
        if (checksum == byte)
            agentPutChar('+');
        else
            agentPutChar('-');
    } while (checksum != byte);

    // reset the in-buffer point to origin for next accessing
    inPoint = inBuffer;

    // initialize the output buffer
    outBuffer[0] = '$';
    outPoint = &outBuffer[1];
}



BYTE *putPacket(BYTE * buffer, BYTE * point)
{
    register BYTE k;
    register DWORD i, length;

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
    point = writeByte(point, k);

    // send output sequence to GDB
    do
    {
        i = length;
        point = buffer;

        while (i)
        {
            agentPutChar(*point);
            point++;
            i--;
        }
    } while (agentGetChar() == '-'); // if fail then again

    *buffer = '$';

    return buffer + 1;
}



/*
==========================================================================================================
Function:   DpString

Input:
    Message string point.

Return:
    None.

Description:
    Print a string to the GDB console
==========================================================================================================
*/
void DpString(BYTE * string)
{
    __asm("break 200");
}



void DebugPrintString(BYTE * string)
{
    register BYTE *p_byte, *out;
    register BYTE *origin;

    // initialize the output buffer
    textBuffer[0] = '$';
    out = &textBuffer[1];
    out = writeChar(out, 'O');

    p_byte = string;

    while (*p_byte)
    {
        out = writeByte(out, *p_byte);
        p_byte++;
    }

    out = writeChar(out, '0');
    out = writeChar(out, 'a');
    putPacket(textBuffer, out);
}



/*
==========================================================================================================
Function:   DpWord

Input:
    A 32-bits word number.

Return:
    None.

Description:
    Print a 32-bits word to the GDB console.
==========================================================================================================
*/
void DpWord(DWORD number)
{
    __asm("break 300");
}



void DebugPrintWord(DWORD number)
{
    register BYTE *p_byte, *out;
    register BYTE *origin;

    out = &textBuffer[20];
    out = writeWord(out, number);
    *out = 0;

    DebugPrintString(&textBuffer[20]);
}



void singleStep(CONTEXT * regs)
{
    register DWORD inst, target, a, b;
    DWORD jump, branch;
    register DEBUG_BLOCK *db;

    target = regs->EPC;
    jump = branch = 0;
    inst = *((DWORD *) target);

    // the mips instruction mask used in following block can be refered at
    // MIPS32 Architecture for programmer volumn I and II
    switch (inst >> 26)
    {
    case OP_SPECIAL:
        switch (inst & 0x3f)    // R-type with OP=SCECIAL(0)
        {
        case SPEC_FUNC_JR:
        case SPEC_FUNC_JALR:
            inst = (inst >> 21) & 0x1f;
            jump = *(&regs->r0 + inst);
//            DebugPrintString(" JR ");
            break;
        }
        break;

    case OP_REGIMM:
        switch ((inst & 0x001f0000) >> 16)  // I-type with OP=REGIMM(1)
        {
        case REGI_FUNC_BLTZ:
        case REGI_FUNC_BGEZ:
        case REGI_FUNC_BLTZAL:
        case REGI_FUNC_BGEZAL:
            jump = target + 8;
            branch = target + 4 + ((SDWORD) ((SWORD) (inst & 0xffff)) << 2);
//            DebugPrintString(" REG ");
            break;
        }
        break;

    case OP_J:      // J-type
    case OP_JAL:    // J-type
        target = (target + 4) & 0xf0000000;
        inst = (inst << 6) >> 4;
        jump = target + inst;
//        DebugPrintString(" J ");
        break;

    case OP_BEQ:    // I-type with OP<>REGIMM(1)
    case OP_BNE:    // I-type with OP<>REGIMM(1)
    case OP_BLEZ:   // I-type with OP<>REGIMM(1)
    case OP_BGTZ:   // I-type with OP<>REGIMM(1)
        jump = target + 8;
        branch = target + 4 + ((SDWORD) ((SWORD) (inst & 0xffff)) << 2);
//        DebugPrintString(" BRA ");
        break;

    default:
//        DebugPrintString(" NOR ");
        break;
    }

    if (branch == jump || branch == target)
        branch = 0;

    if (jump == 0)
        jump = target + 4;

    Step.Address = (DWORD *) jump;
    Step.Instruction = *(DWORD *) jump;
    *(DWORD *) (jump | 0xa0000000) = INSTRUCTION_BREAK;

    if (branch)
    {
        Branch.Address = (DWORD *) branch;
        Branch.Instruction = *(DWORD *) branch;
        *(DWORD *) (branch | 0xa0000000) = INSTRUCTION_BREAK;
    }
}



/**
==========================================================================================================
Function:   BreakPoint

Input:
    None.

Return:
    None.

Description:
    Generate a break point explicitly by the programmer
==========================================================================================================
*/
void BreakPoint()
{
    __asm("		break	");
}



void replyForLocalBreak(CONTEXT * regs)
{
    okString[0] = 'O';
    okString[1] = 'K';
    okString[2] = 0;

    // reply to the host GDB when the break exception has occurred
    outBuffer[0] = '$';
    outPoint = &outBuffer[1];
    outPoint = writeCharByte(outPoint, 'T', POSIX_SIGTRAP);
    outPoint = writeByte(outPoint, GDB_FR_EPC);
    outPoint = writeCharWord(outPoint, ':', regs->EPC);	// send current program counter, i.e., the EPC
    outPoint = writeCharByte(outPoint, ';', GDB_FR_FP);
    outPoint = writeCharWord(outPoint, ':', regs->FP);	// send Frame Point register
    outPoint = writeCharByte(outPoint, ';', GDB_FR_SP);
    outPoint = writeCharWord(outPoint, ':', regs->SP);	// send the stack point
    outPoint = writeCharByte(outPoint, ';', GDB_FR_GP);
    outPoint = writeCharWord(outPoint, ':', regs->GP);
    outPoint = writeChar(outPoint, ';');
    outPoint = putPacket(outBuffer, outPoint);
}



/**
==========================================================================================================
Function:   BreakHandler

Input:
    regs -
        A pointer point to a CONTEXT structure where contain the values of all the registers
        before enterring the BREAK exception.

Return:
    None.

Description:
    BreakHandler will do all the service for the requests from GDB
==========================================================================================================
*/
static void (*uartIsrCallbackPtr)(void) = NULL;

void BreakHandlerIsrCallbackRegister(void *isrCallbackPtr)
{
    uartIsrCallbackPtr = isrCallbackPtr;
}

#if RESET_EXCEPTION
#include "System.h"

void Restart_AP(void)
{
	MP_ALERT("Restart----");
	IODelay(1000);
#if 1

	DWORD i,dwBootSize,dwCopySize,*pdwSAddr,*pdwTAddr;

	dwBootSize=64*1024;
	pdwSAddr=(DWORD *)SystemGetMemAddr(BACKUP_BOOT_BUF_ID);
	pdwTAddr=(DWORD *)0x80700000;
	dwCopySize=(dwBootSize>>2);
	for ( i=0;i<dwCopySize;i++)
		pdwTAddr[i]=pdwSAddr[i];

void(*JUMP)(void);

 JUMP=pdwTAddr;

(*JUMP)();

#else
    __asm("	nop						");
    __asm("	nop						");
    __asm("	nop						");
    __asm("	nop						");
    __asm("	li		$26, 0x80700000 ");
    __asm("	jr		$26				");
    __asm("	nop						");
#endif
	while(1);
}

#endif


void BreakHandler(CONTEXT * regs)
{
    register BYTE command, *p_byte;
    register DWORD a, b, *p_word;
    register BP_BACKUP *bp;
    register DEBUG_BLOCK *db;
#ifdef MAGIC_SENSOR_INTERFACE_ENABLE
    register BYTE *abs_p_byte;
    extern BYTE *start_addr_PC_tool;

    extern void mpx_toolReload(void);
#endif

    /////////////////////////////////////////////////////////////////
    // Used by tconsole
    if (!HandlerEn)
    {
        TaskContextDeactive();

        if (uartIsrCallbackPtr)
            uartIsrCallbackPtr();

        TaskContextActive();
        regs->STATUS |= 0xff01;//Kevin, the interrupt should be enabled

        return;
    }
    /////////////////////////////////////////////////////////////////
#if RESET_EXCEPTION
	Restart_AP();
	return;
#endif

    db = (DEBUG_BLOCK *) (DEBUG_BLOCK_ADDR);
    boolGdbConnect = 1;

    if (db->Counter == 0)
    {
        // initialize the output buffer
        Step.Address = (DWORD *) 0;
        Branch.Address = (DWORD *) 0;
        //Uart_Init();          // For load code from insight
    }

    db->Counter++;

    // this condition is caused by BreakPoint() function,
    // and PC increment is need to make the program continued
    p_word = (DWORD *) regs->EPC;
    localBreakOffset = 0;

    switch (*p_word)
    {
    case LOCAL_BREAK_CODE:
        localBreakOffset = 4;
        break;

    case DP_STRING_CODE:
        DebugPrintString((BYTE *) regs->r4);
        regs->EPC += 4;
        return;

    case DP_WORD_CODE:
        DebugPrintWord(regs->r4);
        regs->EPC += 4;
        return;
    }

    // GDB agent synchronize with GDB should be done only once
    // recover the origin instruction replaced by BREAK because of step operation
    // normally, only the stepBp is used, but if the step position is on a branch
    // the branchBp will be used for both the TRUE and FALSE branching
    if (Step.Address)
    {
        *Step.Address = Step.Instruction;
        Step.Address = (DWORD *) 0;

        if (Branch.Address)
        {
            *Branch.Address = Branch.Instruction;
            Branch.Address = (DWORD *) 0;
        }
    }

    replyForLocalBreak(regs);

    // wait for any GDB command and do the proper service
    // exit when get the continue or the step commands
    while (1)
    {
        getPacket();
        command = readChar();

        switch (command)
        {
#ifdef MAGIC_SENSOR_INTERFACE_ENABLE
        case 'y':           //read memory packet using mpx tool
            p_byte = readVariable();       // read the start offset address
            abs_p_byte = (BYTE *)((DWORD) start_addr_PC_tool + (DWORD) p_byte);
            a = readVariable();     // read the request length

            while(a)
            {
                outPoint = writeByte(outPoint, *abs_p_byte);
                abs_p_byte++;
                a--;
            }
            break;

        case 'Y':          // write memory packet using mpx tool
            p_byte = (BYTE *) readVariable();       // read start offset address
            b = readVariable();                     // read the length
            p_word = (DWORD *)((DWORD) start_addr_PC_tool + (DWORD) p_byte);

            while(b)
            {   // copy the middle part of the data
                *p_word = readWord();
                p_word++;
                b -= 4;
            }

            outPoint = writeString(outPoint, okString);
            break;

        case 'U':           // write memory packet using mpx tool (binary data)
            p_byte = (BYTE *) readVariable();       // read the start address
            p_word = (DWORD *)((DWORD)start_addr_PC_tool + (DWORD) p_byte);
            a = readVariable();                     // read the length

            while (a)
            {
                *p_word = readWordBinary();         // the only place differ to 'M' packet
                p_word++;
                a -= 4;
            }

            outPoint = writeString(outPoint, okString);
            break;

        case 'V':
            if (checkInput() != '#')
                regs->EPC = readVariable();
            else
                regs->EPC += localBreakOffset;

            FlushAllCache();
            mpx_toolReload();
            boolGdbConnect = 0;

            a = 100000;
            while (a) a--;

            regs->STATUS |= 0xff01;//Kevin, the interrupt should be enabled when continue
            return;
#endif

        case 'g':   // GDB read all the registers
            for (p_word = &regs->r0; p_word <= &regs->EPC; p_word++)
                outPoint = writeWord(outPoint, *p_word);

            for (a = 0; a < FR_RESIDUE_COUNT; a++)
                outPoint = writeWord(outPoint, 0);
            break;

        case 'G':   // GDB write all the register
            for (p_word = &regs->r0; p_word <= &regs->EPC; p_word++)
                *p_word = readWord();

            outPoint = writeString(outPoint, okString);
            break;

        case 'm':
            p_byte = (BYTE *) readVariable();   // read the start address
            a = readVariable();                 // read the request length

            while (a)
            {
                outPoint = writeByte(outPoint, *p_byte);
                p_byte++;
                a--;
            }
            break;

        case 'M':   // write memory packet
            p_byte = (BYTE *) readVariable();       // read start address
            p_word = (DWORD *) p_byte;
            a = readVariable();	// read the length

            while (a)
            {
                *p_word = readWord();
                p_word++;
                a -= 4;
            }

            outPoint = writeString(outPoint, okString);
            break;

        case 'X':   // write memory packet (binary)
            p_byte = (BYTE *) readVariable();       // read the start address
            p_word = (DWORD *) p_byte;
            a = readVariable();     // read the length

            while (a)
            {
                *p_word = readWordBinary();                     // the only place differ to 'M' packet
                p_word++;
                a -= 4;
            }
            outPoint = writeString(outPoint, okString);
            break;

        case '?':
            outPoint = writeCharByte(outPoint, 'S', POSIX_SIGTRAP);
            break;

        case 'P':   // write specific register command
            a = readByte();     // read the register number
            b = readChar();     // bypass the '='
            b = readVariable(); // read the specific value
            *(&regs->r0 + a) = b;
            outPoint = writeString(outPoint, okString);
            break;

        case 'c':
            if (checkInput() != '#')
                regs->EPC = readVariable();
            else
                regs->EPC += localBreakOffset;

            //WaitUartTxComplete();
            FlushAllCache();

            a = 100000;
            while (a) a--;

            regs->STATUS |= 0xff01;//Kevin, the interrupt should be enabled when continue
            return;

        case 's':           // single step command
            singleStep(regs);
            regs->EPC += localBreakOffset;
            //WaitUartTxComplete();
            FlushAllCache();

            a = 100000;
            while (a) a--;

            return;
            break;

#if Z_RSP_ENABLE
        case 'z':
            if (readChar() != '0')
                break;      // only support z0 packet

            readChar();     // bypass ','
            p_word = (DWORD *) readVariable();  // read address

            for (bp = &db->BreakPoint[0]; (DWORD) bp < (DWORD) (&db->BreakPoint[NUMBER_OF_BP]); bp++)
            {
                if (p_word == bp->Address)
                {
                    *p_word = bp->Instruction;
                    bp->Address = 0;
                    break;
                }
            }

            outPoint = writeString(outPoint, okString);
            break;

        case 'Z':
            if (readChar() != '0')
                break;      // only support Z0 packet

            readChar();     // bypass ','
            p_word = (DWORD *) readVariable();  // read address

            for (bp = &db->BreakPoint[0]; (DWORD) bp < (DWORD) (&db->BreakPoint[NUMBER_OF_BP]); bp++)
            {
                if (bp->Address == (DWORD *) 0)
                {
                    bp->Address = p_word;
                    bp->Instruction = *p_word;
                    *p_word = INSTRUCTION_BREAK;
                    break;
                }
            }

            outPoint = writeString(outPoint, okString);
            break;
#endif

        case 'q':
            // write a typical answer back
            outPoint = writeString(outPoint, (BYTE *) sectorOffsetString);
            break;

        case 'H':   // set current program threat
        case 'k':   // terminate the application
        case 'r':   // reset all system
        case 'b':   // set baud rate
            outPoint = writeString(outPoint, okString);
            break;

        default:
            break;
        }

        outPoint = putPacket(outBuffer, outPoint);
    }
}



void BadInstruction(CONTEXT * regs)
{
    mpDebugPrint("TaskGetId() = %d", TaskGetId());
#if 1
    mpDebugPrint(" Bad Instruction at 0x%08X, calling by 0x%08X", regs->EPC, regs->r26);
#else
    DebugPrintString(" Bad Instruction at ");
    DebugPrintWord(regs->EPC);
#endif

    BreakHandler(regs);
}


void Exception(CONTEXT * regs, DWORD excep_type)
{
    TRACELN;
    mpDebugPrint("TaskGetId() = %d, excep_type = 0x%x", TaskGetId(), excep_type);

//    mem_Zones(1);  //just showing info for memory usage, but have no use if memory is already corrupt, so mark this off.

    switch(excep_type)
    {
        case 0:
            mpDebugPrint("load word address mis-align exception at 0x%08X", regs->EPC);
            break;
        case 1:
            mpDebugPrint("save word address mis-align exception at 0x%08X", regs->EPC);
            break;
        case 2:
            mpDebugPrint("cpu exception at 0x%08X", regs->EPC);
            break;
        case 3:
            mpDebugPrint("overflow exception happen at 0x%08X", regs->EPC);
            break;
    }
    BreakHandler(regs);
}




void DebugPrint(char *file, int line, char *fmt, ...)
{
    va_list arg;
    SWORD swCount;
    SBYTE sbBuf[128];

    swCount = mp_sprintf(sbBuf, "%-14s%4d ", file, line);
    va_start(arg, fmt);
    mp_vsprintf(&sbBuf[swCount], fmt, arg);
    va_end(arg);
    DpString(sbBuf);
}



void HandlerEnable(BOOL en)
{
    HandlerEn = en;
}



DWORD IsGdbConnect(void)
{
    return boolGdbConnect;
}


#if Make_DIAGNOSTIC_TC
BOOL ChkHandlerEnable()
{
    return HandlerEn;
}
#endif


DWORD uartIntMaskPattern = 0;
DWORD uartIntCauseCleanPattern = 0xFFFFFFFF;

void agentUartIntInfoChange(WORD debugPort)
{
#if (CHIP_VER_MSB == CHIP_VER_615)
    if ((debugPort == DEBUG_PORT_HUART_A) || (debugPort == DEBUG_PORT_HUART_B))
    {
        uartIntMaskPattern = 0x00000020;
        uartIntCauseCleanPattern = 0xFFFFFFDF;
    }
    else
    {
        uartIntMaskPattern = 0;
        uartIntCauseCleanPattern = 0xFFFFFFFF;
    }
#elif (CHIP_VER_MSB == CHIP_VER_650)
    #ifndef __CHIP_VERIFY__
    if (debugPort == DEBUG_PORT_HUART_A)
    {
        uartIntMaskPattern = 0x00000020;
        uartIntCauseCleanPattern = 0xFFFFFFDF;
    }
    else if (debugPort == DEBUG_PORT_HUART_B)
    {
        uartIntMaskPattern = 0x00020000;
        uartIntCauseCleanPattern = 0xFFFDFFFF;
    }
    else
    {
        uartIntMaskPattern = 0;
        uartIntCauseCleanPattern = 0xFFFFFFFF;
    }
    #else
    uartIntMaskPattern = 0;
    uartIntCauseCleanPattern = 0xFFFFFFFF;
    #endif
#else   // MP660
    #ifndef __CHIP_VERIFY__
    if (debugPort == DEBUG_PORT_HUART_A)
    {
        uartIntMaskPattern = 0x00000020;
        uartIntCauseCleanPattern = 0xFFFFFFDF;
    }
    else
    {
        uartIntMaskPattern = 0;
        uartIntCauseCleanPattern = 0xFFFFFFFF;
    }
    #else
    uartIntMaskPattern = 0;
    uartIntCauseCleanPattern = 0xFFFFFFFF;
    #endif
#endif
}

