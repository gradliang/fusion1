/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      board.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
//#include "debug.h"
/*
void _exit(void)
{
    __asm ("break ");
}
*/
#include "global612.h"

int getpagesize(int *size)
{
	*size = 4096;
	return 4096;
}

static char *heap_ptr = 0;
DWORD heap_max = 0;
char *sbrk(int nbytes)
{
#if 0
	extern __heap[];
	extern __heap_end[];
	char *base;

	if (!heap_ptr)
		heap_ptr = (char *) &__heap;
	base = heap_ptr;
	heap_ptr += nbytes;
	if (heap_ptr > (char *) &__heap_end)
	{
//      //MP_DEBUG("panic: heap overflow\n");
		return 0;
	}

	return base;
#else
#if 0
//  extern __heap[];
//  extern __heap_end[];
	char *base;

	if (!heap_ptr)
		heap_ptr = (char *) HEAP_BUF_START_ADDRESS;
	base = heap_ptr;
	heap_ptr += nbytes;
	//DpWord(heap_ptr);
	if (heap_ptr > (char *) (HEAP_BUF_START_ADDRESS + HEAP_BUF_SIZE))
	{
		DpString("panic: heap overflow\n");
		return 0;
	}
	//if(heap_ptr>heap_max)
	//{
	//   heap_max=heap_ptr;
	//     DpWord(heap_max);
	//} 
	return base;
#else
	DpString("panic: heap disabled\n");
	__asm("break 100");			//BREAK_POINT();
#endif
#endif
}

struct s_mem
{
	unsigned int size;
	unsigned int icsize;
	unsigned int dcsize;
};
void get_mem_info(struct s_mem *mem)
{
	mem->size = 0x1000000;		/* 16 MB of RAM */
}

/*
 * dummy low routine to make newlib happy
 */
int isatty()
{
	return 0;
}

int close(int file)
{
	return 0;
}

int fstat(int file)
{
	return 0;
}

int lseek(int file, int offset, int whence)
{
	return 0;
}

int write(int file, char *buf, int nbytes)
{
	return 0;
}

int read(int file, char *buf, int nbytes)
{
	return 0;
}

void times()
{
	return;
}

/* work around to fix compile/link error when WiFi disabled */
#if (NETWARE_ENABLE != ENABLE)
void gettimeofday()
{
	return;
}
#endif

void getpid()
{
	return;
}

void kill()
{
	return;
}
