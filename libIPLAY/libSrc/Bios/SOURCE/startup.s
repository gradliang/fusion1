
#/*
#		The COSMOS for MX886x Reset startup procedure
#
#		Copyright(c) 2002, Comtrue Tech. Co.,
#		All rights reserve.
#*/


#	.file		"boot.s"
	.set		noreorder



#/*********************************************************************/
#/*********************************************************************/

	.globl Boot
	.globl IntHandler
	.globl FlushAllCache
	.globl FlushDataCache	
	.globl GetUserBlkStart

	.section	".boot", "ax"
	.ent Boot

Boot:
	nop
	nop
#	/* initialize CP0:STATUS			*/
	li		$2, 0x10000000
	mtc0	$2, $12
	nop

	lui		$sp, %hi(LD_stack)
	addiu	$sp, $sp, %lo(LD_stack)

# clear all the data area including .sbss and .bss
	lui		$2, %hi(_gp)
	addiu	$2, $2, %lo(_gp)

	lui		$3, %hi(LD_bss_end)
	addiu	$3, $3, %lo(LD_bss_end)

clearDataLoop:
	beq		$2, $3, clearDataLoopEnd
	nop

	sw		$0, 0($2)

	j		clearDataLoop
	addiu	$2, $2, 4

clearDataLoopEnd:

#	/* initialize the short data area and move the point to the center of this area */
	lui		$gp, %hi(_gp)
	addiu	$gp, $gp, %lo(_gp)

#	/* jump to C startup procedure	*/
	la		$2, main
	nop

	jr		$2
	nop

	.end Boot

	.text	

led_display:
    li      $7, 0x000000ff
    and     $7, $7, $6
    li      $6, 0x00ff0000
    or      $7, $7, $6

	li      $6, 0xa801c000
	sw      $7, 0($6)    

#    li      $6, 0x40000
#    li      $7, 0x0001
#led_delay:
#    subu    $6, $6, $7
#    bne     $0, $6, led_delay    
    nop

	jr		$31
	nop


#/*********************************************************************/
#/*********************************************************************/
	.ent GetUserBlkStart
	
GetUserBlkStart:	
	lui		$2, %hi(LD_user_block)
	addiu	$2, $2, %lo(LD_user_block)	
	
	jr		$ra	
	nop
		
	.end GetUserBlkStart	


	.ent FlushAllCache

FlushAllCache:
	addiu	$sp, $sp, -4
	sw		$26, 0($sp)

# flush the data cache
	mfc0	$26, $20
	nop
	ori		$26, $26, 1
	xori	$26, $26, 1
	mtc0	$26, $20				# set DInval bit to 0
	ori		$26, $26, 1
	mtc0	$26, $20				# set DInval bit to 1
	nop

# flush the instruction cache
	mfc0	$26, $20
	nop
	ori		$26, $26, 2
	xori	$26, $26, 2
	mtc0	$26, $20				# set IInval bit to 0
	ori		$26, $26, 2
	mtc0	$26, $20				# set IInval bit to 1
	nop

    lw      $26, 0($sp)
	addiu	$sp, $sp, 4
	j		$31
	nop

	.end FlushAllCache
	

	
# ***********************************************************
	.ent FlushDataCache

FlushDataCache:
	addiu	$sp, $sp, -4
	sw		$26, 0($sp)

# flush the data cache
	mfc0	$26, $20
	nop
	ori		$26, $26, 1
	xori	$26, $26, 1
	mtc0	$26, $20				# set DInval bit to 0
	ori		$26, $26, 1
	mtc0	$26, $20				# set DInval bit to 1
	nop

    lw      $26, 0($sp)
	addiu	$sp, $sp, 4
	j		$31
	nop

	.end FlushDataCache



