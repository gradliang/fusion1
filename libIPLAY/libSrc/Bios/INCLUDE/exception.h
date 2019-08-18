#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "UtilTypeDef.h"


// type definition
typedef struct
{
	DWORD	r0;
	DWORD	r1;
	DWORD	r2;
	DWORD	r3;
	DWORD	r4;
	DWORD   r5;
	DWORD	r6;
	DWORD   r7;
	DWORD	r8;
	DWORD	r9;
	DWORD	r10;
	DWORD	r11;
	DWORD	r12;
	DWORD	r13;
	DWORD   r14;
	DWORD	r15;
	DWORD	r16;
	DWORD	r17;
	DWORD	r18;
	DWORD	r19;
	DWORD	r20;
	DWORD	r21;
	DWORD	r22;
	DWORD	r23;
	DWORD	r24;
	DWORD	r25;
	DWORD	r26;
	DWORD	r27;
	DWORD	GP;
	DWORD	SP;
	DWORD	FP;
	DWORD	RA;
	DWORD	STATUS;
	DWORD   LO;
	DWORD   HI;
	DWORD	BADVADDR;
	DWORD	CAUSE;
	DWORD	EPC;
} CONTEXT;
#endif

