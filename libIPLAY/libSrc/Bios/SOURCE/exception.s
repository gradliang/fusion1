
#/*
# 	Exception handler
#*/

#	$1 = at
#	$2 = v0
#	$3 = v1
#	$4..$7 = a0..a3			used for function calling argument
#	$8..$15 = t0..t7
#	$16..$23 = s0..s7
#	$24..$25 = t8..t9
#	$26..$27 = k0..k1
#	$28 = gp
#	$29 = sp
#	$30 = fp
#	$31 = ra

	.file	"exception.s"
	.set	noreorder
	.set	noat

	.equ	BadVAddr, 8
	.equ	STATUS, 12
	.equ	CAUSE, 13
	.equ	EPC, 14

	.equ	STATUS_DEF, 0x1000ff01
	.equ	BEV_MASK, 0x00400000
	.equ	INT_DISABLE_MASK, 0xffff00fe

	.equ	CONTEXT_SIZE, 0x98

	.equ	INT_STATUS_ADDR, 0xA800B000
	.equ	INT_MASK_ADDR, 0xA800B008

	.globl	IntEnable
	.globl	IntDisable
	.globl	SysCallHandler
	.globl  OsExchangeContext
	.globl	BreakHandler
	.globl	BadInstruction
	.globl	Exception
	.globl	SystemInt
	.globl	boolContext
	.globl  uartIntMaskPattern
	.globl  uartIntCauseCleanPattern

/******************************************************************************/

	.ent IntEnable

IntEnable:
	addiu	$sp, $sp, -0xc				# use for save registers
	sw		$ra, 0x0000($sp)
	sw		$3, 0x0004($sp)
	sw		$4, 0x0008($sp)

	la		$3, boolContext
	lw		$4, 0($3)
	nop
	beq		$4, $0, Exit_IntEnable

	lui		$3, %hi(STATUS_DEF)
	addi		$3, %lo(STATUS_DEF)
	mfc0		$4, $12
	or		$4, $4, $3
	mtc0		$4, $12						# $STATUS

Exit_IntEnable:
	lw		$ra, 0x0000($sp)
	lw		$3, 0x0004($sp)
	lw		$4, 0x0008($sp)
	addiu	$sp, $sp, 0xc

	jr		$ra
	nop

	.end IntEnable

/******************************************************************************/

	.ent IntDisable

IntDisable:
	addiu	$sp, $sp, -0x8				# use for save registers
	sw		$2, 0x0000($sp)
	sw		$3, 0x0004($sp)

	la		$2, boolContext
	lw		$3, 0($2)
	nop
	beq		$3, $0, IntDisable_Exit		# if equal to zero, mean running in interrupt, jump to leave

	lui		$3, %hi(INT_DISABLE_MASK)
	addi	$3, %lo(INT_DISABLE_MASK)
	mfc0	$2, $12						# save the original value in $2 for the return value
	and		$3, $2, $3
	mtc0	$3, $12						# $STATUS

IntDisable_Exit:
	lw		$3, 0x0004($sp)
	lw		$2, 0x0000($sp)
	jr		$ra							# the original value in status register will be returned
	addiu	$sp, $sp, 0x8

	.end IntDisable

/******************************************************************************/
/******************************************************************************/

	.section	".vector", "ax"

	.ent Excep_Handler

Excep_Handler:

	# save the context into stack, include all the general registers except
	# k0(r26), k1(r27), gp(r28), sp(r29)), EPC, HI and LO if MAC is built-in

	addiu	$sp, $sp, -CONTEXT_SIZE		# allocate stack for context save

	sw		$1, 0x0004($sp)
	sw		$2, 0x0008($sp)
	sw		$3, 0x000c($sp)
	sw		$4, 0x0010($sp)
	sw		$5, 0x0014($sp)
	sw		$6, 0x0018($sp)
	sw		$7, 0x001c($sp)
	sw		$8, 0x0020($sp)
	sw		$9, 0x0024($sp)
	sw		$10, 0x0028($sp)
	sw		$11, 0x002c($sp)
	sw		$12, 0x0030($sp)
	sw		$13, 0x0034($sp)
	sw		$14, 0x0038($sp)
	sw		$15, 0x003c($sp)
	sw		$16, 0x0040($sp)
	sw		$17, 0x0044($sp)
	sw		$18, 0x0048($sp)
	sw		$19, 0x004c($sp)
	sw		$20, 0x0050($sp)
	sw		$21, 0x0054($sp)
	sw		$22, 0x0058($sp)
	sw		$23, 0x005c($sp)
	sw		$24, 0x0060($sp)
	sw		$25, 0x0064($sp)
	sw		$26, 0x0068($sp)		# $k0
	sw		$30, 0x0078($sp)
	sw		$31, 0x007c($sp)

	mflo	$2						# $LO
	mfhi	$3						# $HI
	mfc0	$4, $12					# STATUS
	mfc0	$6, $14					# $EPC

	sw		$4, 0x0080($sp)
	sw		$2, 0x0084($sp)
	sw		$3, 0x0088($sp)
	sw		$6, 0x0094($sp)

	lui		$1, %hi(INT_DISABLE_MASK)	# abel add, disable int while in exception
	addi	$1, %lo(INT_DISABLE_MASK)
	and		$1, $4, $1
	mtc0	$1, $12						# $STATUS

	# distinguish the interrupt type and dispatch the proper service
	# routine for the interrupt

	mfc0	$1, $13					# $CAUSE
	nop
	andi	$2, $1, 0x003c			# r2 = ExeCode

	beq		$2, $0, Int_Excep		# jump if Unmasked interrupt
	addiu	$2, $2, -0x0004*4

	beq		$2, $0, Adel_Excep		# jump	if Address exception trap, Instruction fetch
	addiu	$2, $2, -0x0004*1

	beq		$2, $0, Ades_Excep		# jump	if Address exception trap, Data fetch
	addiu	$2, $2, -0x0004*3

	beq		$2, $0, Sys_Excep		# jump if SYSCALL instruction trap
	addiu	$2, $2, -0x0004*1

	beq		$2, $0, Bp_Excep		# jump	if BREAK instruction trap
	addiu	$2, $2, -0x0004*1

	beq		$2, $0, Ri_Excep		# jump	if reserved opcode fetch
	addiu	$2, $2, -0x0004*1

	beq		$2, $0, Cpu_Excep		# jump	Coprocessor Usability trap
	addiu	$2, $2, -0x0004*1

	beq		$2, $0, Ov_Excep		# jump	Arithmetic overflow trap
	nop

/******************************************************************************/


Excep_Return:		# restore the context that saved in stack

	lw		$4, 0x0080($sp)
	lw		$2, 0x0084($sp)
	lw		$3, 0x0088($sp)
	lw		$6, 0x0094($sp)

	mtlo	$2						# $LO
	mthi	$3						# $HI
	mtc0	$4, $12					# STATUS
	mtc0	$6, $14					# $EPC

	lw		$1, 0x0004($sp)
	lw		$2, 0x0008($sp)
	lw		$3, 0x000c($sp)
	lw		$4, 0x0010($sp)
	lw		$5, 0x0014($sp)
	lw		$6, 0x0018($sp)
	lw		$7, 0x001c($sp)
	lw		$8, 0x0020($sp)
	lw		$9, 0x0024($sp)
	lw		$10, 0x0028($sp)
	lw		$11, 0x002c($sp)
	lw		$12, 0x0030($sp)
	lw		$13, 0x0034($sp)
	lw		$14, 0x0038($sp)
	lw		$15, 0x003c($sp)
	lw		$16, 0x0040($sp)
	lw		$17, 0x0044($sp)
	lw		$18, 0x0048($sp)
	lw		$19, 0x004c($sp)
	lw		$20, 0x0050($sp)
	lw		$21, 0x0054($sp)
	lw		$22, 0x0058($sp)
	lw		$23, 0x005c($sp)
	lw		$24, 0x0060($sp)
	lw		$25, 0x0064($sp)
	lw		$30, 0x0078($sp)
	lw		$31, 0x007c($sp)

	lw		$26, 0x0094($sp)		# save the return address at k0

	addiu	$sp, $sp, CONTEXT_SIZE	# release the stack space

	jr		$26						# return interrupt
	rfe


/******************************************************************************/

Sys_Excep:
	addiu	$4, $sp, 0x0			# first argument "stack point"
	lw		$26, 0x94($sp)
	nop
	lw		$5, 0($26)				# second argument, the instruction code of "syscall xxxx"
	addiu	$26, $26, 4
	sw		$26, 0x94($sp)
	jal		OsExchangeContext		# jump to the operating system default interrupt service routine
	nop
	addiu	$sp, $2, 0x0
	blez	$0, Excep_Return
	nop

/******************************************************************************/

Int_Excep:
	li	$4, INT_STATUS_ADDR		# if uart exception jump to break point exceation
	lw	$3, 0($4)
	nop
	li	$4, INT_MASK_ADDR
	lw	$5, 0($4)
	nop
	and	$3, $3, $5
	la	$4, uartIntMaskPattern
	lw	$5, 0($4)
	nop
	and	$3, $3, $5				# BIT5 - IC_UARTA, BIT17 - IC_UARTB
	bne	$3, $0, Bp_Excep
	nop

Int_Excep_10:
	la	$2, boolContext
	sw	$0, 0($2)			# set flag for the non-context condition
	nop

	addiu	$4, $sp, 0x0
	jal	SystemInt			# jump to the operating system default interrupt service routine
	nop
	addiu	$4, $0, 1			# set flag for the in-context condition
	la	$2, boolContext
	sw	$4, 0($2)
	nop

Int_Excep_End:
	addiu	$4, $sp, 0x0			# first argument "stack point"
	lw	$26, 0x94($sp)
	jal	OsExchangeContext		# jump to the operating system default interrupt service routine
	nop
	addiu	$sp, $2, 0x0
	blez	$0, Excep_Return
	nop



/******************************************************************************/

Adel_Excep:
# save the additional context especial for GDB
	sw		$0, 0x0000($sp)
	sw		$26, 0x0068($sp)
	sw		$27, 0x006c($sp)
	sw		$28, 0x0070($sp)

	addiu	$26, $sp, CONTEXT_SIZE
	sw		$26, 0x0074($sp) 		# save the stack point value before the exception happen

	mfc0	$1, $12					# $STATUS
	mfc0	$4, $8					# $BADVADDR
	mfc0	$5, $13					# $CAUSE

	sw		$1, 0x0080($sp)
	sw		$4, 0x008c($sp)
	sw		$5, 0x0090($sp)

	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
	jal	    Exception	
	move  	$5, $0            		# excep_type = 0
	blez	$0, Excep_Return
	nop


/******************************************************************************/

Ades_Excep:
# save the additional context especial for GDB
	sw		$0, 0x0000($sp)
	sw		$26, 0x0068($sp)
	sw		$27, 0x006c($sp)
	sw		$28, 0x0070($sp)

	addiu	$26, $sp, CONTEXT_SIZE
	sw		$26, 0x0074($sp) 		# save the stack point value before the exception happen

	mfc0	$1, $12					# $STATUS
	mfc0	$4, $8					# $BADVADDR
	mfc0	$5, $13					# $CAUSE

	sw		$1, 0x0080($sp)
	sw		$4, 0x008c($sp)
	sw		$5, 0x0090($sp)

	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
	jal	    Exception	
	addiu 	$5, $0, 0x1       		# excep_type = 1
	blez	$0, Excep_Return
	nop

/******************************************************************************/

Bp_Excep:
# save the additional context especial for GDB
	sw	$0, 0x0000($sp)
	sw	$26, 0x0068($sp)
	sw	$27, 0x006c($sp)
	sw	$28, 0x0070($sp)

	addiu	$26, $sp, CONTEXT_SIZE
	sw	$26, 0x0074($sp) 		# save the stack point value before the exception happen

	mfc0	$1, $12					# $STATUS
	mfc0	$4, $8					# $BADVADDR
	mfc0	$5, $13					# $CAUSE

	sw	$1, 0x0080($sp)
	sw	$4, 0x008c($sp)
	sw	$5, 0x0090($sp)

	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
	jal	BreakHandler
	nop

	la      $4, uartIntCauseCleanPattern	# BIT5 - IC_UARTA, BIT17 - IC_UARTB
	lw      $5, 0($4)
	nop
	li      $4, INT_STATUS_ADDR		# clear uart cause
	lw      $3, 0($4)
	nop
	and     $3, $3, $5
	sw      $3, 0($4)
	nop

	blez	$0, Excep_Return
	nop

/******************************************************************************/

Ri_Excep:
# save the additional context especial for GDB
	sw		$0, 0x0000($sp)
	sw		$26, 0x0068($sp)
	sw		$27, 0x006c($sp)
	sw		$28, 0x0070($sp)

	addiu	$26, $sp, CONTEXT_SIZE
	sw		$26, 0x0074($sp) 		# save the stack point value before the exception happen

	mfc0	$1, $12					# $STATUS
	mfc0	$4, $8					# $BADVADDR
	mfc0	$5, $13					# $CAUSE

	sw		$1, 0x0080($sp)
	sw		$4, 0x008c($sp)
	sw		$5, 0x0090($sp)

	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
	jal		BadInstruction
	nop
	blez	$0, Excep_Return
	nop

#	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
#	jal	BadInstruction
#	nop
#	blez	$0, Excep_Return
#	nop

/******************************************************************************/

Cpu_Excep:
# save the additional context especial for GDB
	sw		$0, 0x0000($sp)
	sw		$26, 0x0068($sp)
	sw		$27, 0x006c($sp)
	sw		$28, 0x0070($sp)

	addiu	$26, $sp, CONTEXT_SIZE
	sw		$26, 0x0074($sp) 		# save the stack point value before the exception happen

	mfc0	$1, $12					# $STATUS
	mfc0	$4, $8					# $BADVADDR
	mfc0	$5, $13					# $CAUSE

	sw		$1, 0x0080($sp)
	sw		$4, 0x008c($sp)
	sw		$5, 0x0090($sp)

	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
	jal	    Exception	
	addiu 	$5, $0, 0x2       		# excep_type = 2
	blez	$0, Excep_Return
	nop


/******************************************************************************/

Ov_Excep:
# save the additional context especial for GDB
	sw		$0, 0x0000($sp)
	sw		$26, 0x0068($sp)
	sw		$27, 0x006c($sp)
	sw		$28, 0x0070($sp)

	addiu	$26, $sp, CONTEXT_SIZE
	sw		$26, 0x0074($sp) 		# save the stack point value before the exception happen

	mfc0	$1, $12					# $STATUS
	mfc0	$4, $8					# $BADVADDR
	mfc0	$5, $13					# $CAUSE

	sw		$1, 0x0080($sp)
	sw		$4, 0x008c($sp)
	sw		$5, 0x0090($sp)

	addiu	$4, $sp, 0x0			# $sp now point to the base of context data to be the argument
	jal	    Exception	
	addiu 	$5, $0, 0x3       		# excep_type = 3
	blez	$0, Excep_Return
	nop


/******************************************************************************/
/******************************************************************************/

	.end Excep_Handler

