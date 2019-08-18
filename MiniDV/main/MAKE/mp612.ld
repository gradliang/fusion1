OUTPUT_ARCH(mips)

MEMORY
{
	MEMORY_SIZE		: org = 0x80002000, LENGTH = 32768k
}

SECTIONS
{
	/********************************************************************************/

	/* 0x80000000 ~ 0x8000000f used for memory test */
	.boot		0x80000010  :   {*(.boot) }
	.vector		0x80000080	:	{*(.vector)}

	/********************************************************************************/

	.text		0x80002000	:	{
								. = ALIGN(32);
								*(.adjustpll)
								. = ALIGN(32);
								*(.adjustmemcs)
								. = ALIGN(32);
								*(.pwdcwait)
								. = ALIGN(32);
								*(.memfreqchg)
								. = ALIGN(32);
								*(.setdramtiming)
								. = ALIGN(32);
 								*(.dlldelay)
								. = ALIGN(4);
								*(.eh_frame)
								*(.rodata.str1.4)
								*(.text)
								*(.data)
								*(.rodata)
								} > MEMORY_SIZE

	/********************************************************************************/
	. = ALIGN(4);
	LD_kmodapi_start = .;
	.kmodapi           :   {
	                        *(.kmodapi)
	                        } > MEMORY_SIZE
	LD_kmodapi_end = .;

	.sdata : {*(.sdata)} > MEMORY_SIZE

	_sram_start = .;
  	.sram : {*(.sram) *(.sram1) *(.sram_code)} > MEMORY_SIZE
  	_sram_end = .;

	kernel_module_sarea = .;
	.kernel_module . : {. = . + 0x8000-SIZEOF(.sdata)-SIZEOF(.sram)-4;} > MEMORY_SIZE
	kernel_module_sarea_end = .;

	. = ALIGN(4);
	_gp = .;

	/* .lit8 is for libm */
	.lit8 					:	{
							*(.lit8)
							} > MEMORY_SIZE

	.sbss					:	{
							*(.comm)
							*(.lcomm)
							*(.scommon)
							*(.sbss)
							} > MEMORY_SIZE

	.bss 					:	{
							*(.bss)
							} > MEMORY_SIZE

	. = ALIGN(4);
	LD_bss_end = .;

  /********************************************************************************/
	.stack		.		:	{
							. = . + 0x2000;
						} > MEMORY_SIZE
	LD_stack = .;

	. = ALIGN(4);
	LD_user_block = .;
}

