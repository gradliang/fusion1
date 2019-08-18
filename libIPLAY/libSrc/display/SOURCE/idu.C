/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
//for UDU reg message print
//#define IDU_DEBUG

/*
// Include section
*/
#include "global612.h"
#include "taskid.h"
#include "mpTrace.h"
#include "idu.h"
#include "display.h"
#include "FontDisp.h"
#include "System.h"
#include "peripheral.h"

#include "../../sensor/INCLUDE/sensor.h"


volatile static BOOL needchangeWin = FALSE;
volatile static ST_IMGWIN *nextImgWinPtr = &sWin[0];
volatile DWORD decodeFrameRate = 0;
volatile static BOOL needReleaseSemaphore = FALSE;

BYTE g_IDU_444_422_Flag = 0;	// 0 is 422, 1 is 444
static BYTE g_bDisplayStatus;
extern void AUOLED_Init(BYTE i);

static int g_Sensor_RecodeMode=0;

#define SWAP_FLAG	0

#define ANTIVIBR_ENABLE 1


#if GAMMA_new
BYTE gammatable[16][16] = {
{0x0, 0xA8, 0xB5,  0xC2, 0xCA, 0xCF, 0xD8, 0xDE, 0xE3, 0xE8, 0xEB, 0xEE, 0xF4, 0xF8, 0xFC, 0xFF},	//r = 0.1
{0x0, 0x6F, 0x7F,  0x93, 0x9F, 0xA8, 0xB7, 0xC2, 0xCA, 0xD2, 0xD8, 0xDE, 0xE9, 0xF1, 0xF9, 0xFF},	//r = 0.2
{0x0, 0x49, 0x5A,  0x6F, 0x7D, 0x89, 0x9A, 0xA8, 0xB4, 0xBE, 0xC7, 0xCF, 0xDE, 0xEA, 0xF5, 0xFF},	//r = 0.3
{0x0, 0x30, 0x3F,  0x54, 0x63, 0x6F, 0x83, 0x93, 0xA0, 0xAC, 0xB7, 0xC2, 0xD4, 0xE4, 0xF2, 0xFF},	//r = 0.4
{0x0, 0x27, 0x35,  0x49, 0x58, 0x64, 0x78, 0x89, 0x97, 0xA4, 0xB0, 0xBB, 0xCF, 0xE0, 0xF1, 0xFF},	//r= 0.45
{0x0, 0x27, 0x35,  0x49, 0x58, 0x64, 0x78, 0x89, 0x97, 0xA4, 0xB0, 0xBB, 0xCF, 0xE0, 0xF1, 0xFF},	//r= 0.5
{0x0, 0x27, 0x35,  0x49, 0x58, 0x64, 0x78, 0x89, 0x97, 0xA4, 0xB0, 0xBB, 0xCF, 0xE0, 0xF1, 0xFF},	//r= 0.6
{0x0, 0x0D, 0x16,  0x24, 0x30, 0x3b, 0x4F, 0x61, 0x71, 0x80, 0x8F, 0x9D, 0xB8, 0xD1, 0xE9, 0xFF},	//r = 0.7
{0x0, 0x09, 0x0F,  0x1B, 0x26, 0x30, 0x43, 0x54, 0x64, 0x74, 0x84, 0x93, 0xAF, 0xCB, 0xE6, 0xFF},	//r = 0.8
{0x0, 0x06, 0x0B,  0x15, 0x1E, 0x27, 0x38, 0x49, 0x59, 0x69, 0x79, 0x89, 0xA7, 0xC5, 0xE3, 0xFF},	//r = 0.9
{0x0, 0x04, 0x08,  0x10, 0x18, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0xA0, 0xC0, 0xE0, 0xFF},	//r = 1
{0x0, 0x02, 0x05,  0x0C, 0x12, 0x19, 0x28, 0x37, 0x47, 0x57, 0x67, 0x77, 0x98, 0xBA, 0xDD, 0xFF},	//r = 1.1
{0x0, 0x01, 0x04,  0x09, 0x0E, 0x15, 0x22, 0x30, 0x3F, 0x4E, 0x5E, 0x6F, 0x91, 0xB5, 0xDA, 0xFF},	//r = 1.2
{0x0, 0x01, 0x02,  0x06, 0x0B, 0x11, 0x1D, 0x2A, 0x38, 0x47, 0x57, 0x67, 0x8A, 0xB0, 0xD7, 0xFF},	//r = 1.3
{0x0, 0x00, 0x02,  0x05, 0x09, 0x0D, 0x18, 0x24, 0x32, 0x40, 0x50, 0x61, 0x84, 0xAB, 0xD4, 0xFF},	//r = 1.4
{0x0, 0x00, 0x01,  0x04, 0x07, 0x0B, 0x14, 0x20, 0x2C, 0x3A, 0x4A, 0x5A, 0x7E, 0xA6, 0xD1, 0xFF},	//r = 1.5
};
#endif
//***********************************************************************
//                                      Variable
//***********************************************************************

///
///@ingroup group_IDU
///@brief   Change to the win that you want to show
///
///@param   ST_IMGWIN *win The win you want to put to screen
///
///@retval  NONE
///
///@remark  The function will change the input win to be current win, and the other
///         win as next win
///
#if (IDU_CHANGEWIN_MODE == 0)
void Idu_ChgWin(ST_IMGWIN * ImgWin)
{
	register CHANNEL *iduDmaReg = (CHANNEL *) (DMA_IDU_BASE);
    ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;
    DWORD bufPixelCnt = ImgWin->wWidth * ImgWin->wHeight;

//	mpDebugPrint("############ Idu_ChgWin ....... ");

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
    IDU *idu = (IDU *)IDU_BASE;
	if(idu->RST_Ctrl_Bypass & 0x1)
	{
		MP_ALERT("Idu_ChgWin while IDU in bypass mode !!");
//		__asm("break 100");
		return;
	}
#endif

    if (ImgWin == psCurrWin)
        return;

		MP_DEBUG("Idu_ChgWin N");
    SemaphoreWait(IDU_SEMA_ID);

    IntDisable();

	//update current win
	if (ImgWin == &sWin[0])
	{
		psCurrWin = &sWin[0];
		psNextWin = &sWin[1];
        #ifdef IDU_DEBUG
        UartOutText("C0 ");
        #endif
	}
	else if (ImgWin == &sWin[1])
	{
		psCurrWin = &sWin[1];
#ifdef ENABLE_TRIPLE_FB
		psNextWin = &sWin[2];
#else
		psNextWin = &sWin[0];
#endif

       #ifdef IDU_DEBUG
        UartOutText("C1 ");
       #endif
	}
#ifdef ENABLE_TRIPLE_FB
	else //if (ImgWin == &sWin[2])
	{
		psCurrWin = &sWin[2];
		psNextWin = &sWin[0];
        #ifdef IDU_DEBUG
        UartOutText("C2 ");
        #endif
	}
#endif

	if (pstTCON->bInterlace == INTERLACE)
	{   // First change bufferA
   		if ((iduDmaReg->Control & BIT15) == 1)      // operating on buffer B
        {
            if ((iduDmaReg->EndB - iduDmaReg->Current) > (bufPixelCnt >> 3))
            {   // 1/16 pixels
        		iduDmaReg->StartA = (DWORD) psCurrWin->pdwStart;
                // End address is Start + (Width * (Height - 1) * 2) - 1
                // line 1, 3, 5, ...., Height-1
        		iduDmaReg->EndA = (DWORD) psCurrWin->pdwStart + ((bufPixelCnt - psCurrWin->wWidth) << 1) - 1;
            }
        }
	}
	else
    {
        if ((iduDmaReg->EndA - iduDmaReg->Current) > (bufPixelCnt >> 3))
    	{
    		if ((iduDmaReg->Control & BIT15) == 0)  // operating on buffer A
    		{   // 1/16 pixels
    			iduDmaReg->StartB = (DWORD) psCurrWin->pdwStart;
                #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		        if(g_IDU_444_422_Flag)	// 444 mode
		        {
			        MP_DEBUG("IDU DMA 444 mode 1");
                    if((idu->RST_Ctrl_Bypass & BIT31) == 0)
                    {
                        while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
		                idu->RST_Ctrl_Bypass = 0x8000aa02 ;
		                //idu->RST_Ctrl_Bypass = 0x80005502 ;
		                //idu->RST_Ctrl_Bypass = 0x80000002 ;
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 3) - 1;
                     }
    			    iduDmaReg->EndB =(DWORD) psCurrWin->pdwStart + (bufPixelCnt *3) - 1;
		            iduDmaReg->StartA = iduDmaReg->StartB;
		            iduDmaReg->EndA = iduDmaReg->EndB;
                }
		        else
                #endif
                {
                    if((idu->RST_Ctrl_Bypass & BIT31) != 0)
                    {
                        while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
			            //idu->RST_Ctrl_Bypass &= 0x7fffffff ;
			            idu->RST_Ctrl_Bypass &= 0x7fff55ff ;
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 2) - 1;
                    }
                    iduDmaReg->EndB =	(DWORD) psCurrWin->pdwStart + (bufPixelCnt << 1) - 1;
                }
            }
            else
    	    {
    		    iduDmaReg->StartA = (DWORD) psCurrWin->pdwStart;
                #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		        if(g_IDU_444_422_Flag)	// 444 mode
		        {
			        MP_DEBUG("IDU DMA 444 mode 2");
                    if((idu->RST_Ctrl_Bypass & BIT31) == 0)
                    {
                        while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
		                idu->RST_Ctrl_Bypass = 0x8000aa02 ;
		                //idu->RST_Ctrl_Bypass = 0x80005502 ;
		                //idu->RST_Ctrl_Bypass = 0x80000002 ;
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 3) - 1;
                     }
                    iduDmaReg->EndA =(DWORD) psCurrWin->pdwStart + (bufPixelCnt *3) - 1;
                    iduDmaReg->StartB = iduDmaReg->StartA;
		            iduDmaReg->EndB = iduDmaReg->EndA;
                }
		        else
                #endif
                {
                    if((idu->RST_Ctrl_Bypass & BIT31) != 0)
                    {
                        while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
			            //idu->RST_Ctrl_Bypass &= 0x7fffffff ;
			            idu->RST_Ctrl_Bypass &= 0x7fff55ff ;
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 2) - 1;
                    }
                    iduDmaReg->EndA = (DWORD) psCurrWin->pdwStart + (bufPixelCnt << 1) - 1;
                }
             }
  		}
    }
    decodeFrameRate++;
    nextImgWinPtr = psCurrWin;
    needchangeWin = TRUE;
    SystemIntEna(IM_DMA);
    DmaIntEna(IM_IDUDM);
    iduDmaReg->Control &= ~BIT16;       // clear IDU buffer end interrupt indicator
		iduDmaReg->Control |= BIT24;        // Enable IDU DAM buffer end interrupt
    EventClear(IDU_EVENT_ID, ~IDU_CHANGE_WIN_FINISHED);
    IntEnable();

#ifndef ENABLE_TRIPLE_FB
    {
        DWORD release;

        EventWaitWithTO(IDU_EVENT_ID, IDU_CHANGE_WIN_FINISHED, OS_EVENT_OR, &release, 500);
    }
#endif
}
#else
//***********************************************************************
//                                      Variable
//***********************************************************************
void Idu_ChgWinWaitFinish(ST_IMGWIN * ImgWin, BOOL needWaitFinish);

///
///@ingroup group_IDU
///@brief   Change to the win that you want to show
///
///@param   ST_IMGWIN *win The win you want to put to screen
///
///@retval  NONE
///
///@remark  The function will change the input win to be current win, and the other
///         win as next win
///
void Idu_ChgWin(ST_IMGWIN * ImgWin)
{
    Idu_ChgWinWaitFinish(ImgWin, FALSE);
}

void Idu_ChgWinWaitFinish(ST_IMGWIN * ImgWin, BOOL needWaitFinish)
{
    register CHANNEL *iduDmaReg = (CHANNEL *) (DMA_IDU_BASE);
    ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;
    DWORD bufByteCnt = ImgWin->dwOffset * ImgWin->wHeight;
    DWORD bufByteCntLimit = bufByteCnt >> 3;
	DWORD bufPixelCnt = ImgWin->wWidth * ImgWin->wHeight;

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
    IDU *idu = (IDU *)IDU_BASE;
    if(idu->RST_Ctrl_Bypass & 0x1)
    {
        MP_ALERT("--E-- Idu_ChgWin while IDU in bypass mode !!");
        //__asm("break 100");
        return;
    }
#endif

    if (ImgWin == psCurrWin)
        return;

    //update current win
    IntDisable();

    if (ImgWin == &sWin[0])
    {
        psCurrWin = &sWin[0];
        psNextWin = &sWin[1];
#ifdef IDU_DEBUG
        UartOutText("C0 ");
#endif
    }
    else if (ImgWin == &sWin[1])
    {
        psCurrWin = &sWin[1];
#ifdef ENABLE_TRIPLE_FB
        psNextWin = &sWin[2];
#else
        psNextWin = &sWin[0];
#endif

        #ifdef IDU_DEBUG
        UartOutText("C1 ");
        #endif
    }
#ifdef ENABLE_TRIPLE_FB
    else //if (ImgWin == &sWin[2])
    {
        psCurrWin = &sWin[2];
        psNextWin = &sWin[0];
#ifdef IDU_DEBUG
        UartOutText("C2 ");
#endif
    }
#endif

    MP_DEBUG("Idu_ChgWin N");
    SemaphoreWait(IDU_SEMA_ID);
    IntDisable();

    if (pstTCON->bInterlace == INTERLACE)
    {   // First change bufferA
        if ((iduDmaReg->Control & BIT15) == 1)      // operating on buffer B
        {
            if ((iduDmaReg->EndB - iduDmaReg->Current) > bufByteCntLimit )
            {   // 1/16 pixels
                iduDmaReg->StartA = (DWORD) psCurrWin->pdwStart;
                // End address is Start + (Width * (Height - 1) * 2) - 1
                // line 1, 3, 5, ...., Height-1
                iduDmaReg->EndA = (DWORD) psCurrWin->pdwStart + (bufByteCnt - psCurrWin->dwOffset) - 1;
            }
        }
    }
    else
    {
        if ((iduDmaReg->EndA - iduDmaReg->Current) > bufByteCntLimit )
        {
            if ((iduDmaReg->Control & BIT15) == 0)  // operating on buffer A
            {   // 1/16 pixels

               iduDmaReg->StartB = (DWORD) psCurrWin->pdwStart;
                #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		        if(g_IDU_444_422_Flag)	// 444 mode
		        {
			        MP_DEBUG("IDU DMA 444 mode 1");
    			    iduDmaReg->EndB =(DWORD) psCurrWin->pdwStart + (bufPixelCnt *3) - 1;
		            iduDmaReg->StartA = iduDmaReg->StartB;
		            iduDmaReg->EndA = iduDmaReg->EndB;
                    if((idu->RST_Ctrl_Bypass & BIT31) == 0)
                    {
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 3) - 1;
		                while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x00000000);
		                while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x38000000);
		                idu->RST_Ctrl_Bypass = 0x8000aa02 ;
		                //idu->RST_Ctrl_Bypass = 0x80005502 ;
		                //idu->RST_Ctrl_Bypass = 0x80000002 ;
                     }
                }
		        else
                #endif
                {
                	
                 iduDmaReg->EndB = (DWORD) psCurrWin->pdwStart + bufByteCnt - 1;
                    if((idu->RST_Ctrl_Bypass & BIT31) != 0)
                    {
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 2) - 1;
			            idu->RST_Ctrl_Bypass &= 0x7fffffff ;
                    }
                }

            }
            else
            {
                   		    iduDmaReg->StartA = (DWORD) psCurrWin->pdwStart;
                #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		        if(g_IDU_444_422_Flag)	// 444 mode
		        {
			        MP_DEBUG("IDU DMA 444 mode 2");
			        iduDmaReg->EndA =(DWORD) psCurrWin->pdwStart + (bufPixelCnt *3) - 1;
                    iduDmaReg->StartB = iduDmaReg->StartA;
		            iduDmaReg->EndB = iduDmaReg->EndA;
                    if((idu->RST_Ctrl_Bypass & BIT31) == 0)
                    {
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 3) - 1;
		                while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x00000000);
		                while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x38000000);
		                idu->RST_Ctrl_Bypass = 0x8000aa02 ;
		                //idu->RST_Ctrl_Bypass = 0x80005502 ;
		                //idu->RST_Ctrl_Bypass = 0x80000002 ;
                     }
                }
		        else
                #endif
                {
				   iduDmaReg->EndA = (DWORD) psCurrWin->pdwStart + bufByteCnt - 1;
                    if((idu->RST_Ctrl_Bypass & BIT31) != 0)
                    {
		                iduDmaReg->LineCount = (0 << 16) + (ImgWin->wWidth * 2) - 1;
			            idu->RST_Ctrl_Bypass &= 0x7fffffff ;
                    }
                }
            }
        }
    }

//    decodeFrameRate++;
    nextImgWinPtr = psCurrWin;
    needchangeWin = TRUE;

    if (needWaitFinish)
        needReleaseSemaphore = FALSE;
    else
        needReleaseSemaphore = TRUE;

    //iduDmaReg->Control &= ~BIT24;           // Disable IDU DMA buffer end interrupt
    iduDmaReg->Control |= BIT24;            // Enable IDU DMA buffer end interrupt
    IntEnable();
    SystemIntEna(IM_DMA);
    DmaIntEna(IM_IDUDM);
    EventClear(IDU_EVENT_ID, ~IDU_CHANGE_WIN_FINISHED);

	#if (SENSOR_TO_FB_RGB888_TYPE == ENABLE)
	SemaphoreRelease(IDU_SEMA_ID); 
    #endif

#ifndef ENABLE_TRIPLE_FB
    if (needWaitFinish)
    {
        DWORD release;

        if (EventWaitWithTO(IDU_EVENT_ID, IDU_CHANGE_WIN_FINISHED, OS_EVENT_OR, &release, 500) != OS_STATUS_OK)
            MP_ALERT("--E-- Change win timeout !!!!");

        SemaphoreRelease(IDU_SEMA_ID);
    }
#endif
}
#endif
#if 0
void Idu_ChgWin(ST_IMGWIN * ImgWin)
{
	register CHANNEL *dma;
	register IDU *idu = (IDU *)IDU_BASE;
	dma = (CHANNEL *) (DMA_IDU_BASE);

  ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;

	if (ImgWin == psCurrWin) return;

	dma->Control |= 0x01000000;	//dma buffer end interrupt
	dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt indicator

	//update current win
	if (ImgWin == &sWin[0])
	{
		psCurrWin = &sWin[0];
		psNextWin = &sWin[1];
	}
	else
	{
		psCurrWin = &sWin[1];
		psNextWin = &sWin[0];
	}

	if (pstTCON->bInterlace == INTERLACE)
	{
		while ((dma->Control & 0x00018000) != 0x00018000); //  TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		dma->StartA = ((DWORD) ImgWin->pdwStart | 0xa0000000);
		dma->EndA = dma->StartA + (ImgWin->wWidth * (ImgWin->wHeight - 1) * 2) - 1;

		while ((dma->Control & 0x00018000) != 0x00010000); // TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt
#if ANTIVIBR_ENABLE
		dma->StartB = dma->StartA;
		dma->EndB = dma->EndA;
#else
		dma->StartB = dma->StartA + ImgWin->wWidth * 2;
		dma->EndB = dma->EndA + (ImgWin->wWidth * 2) - 1;
#endif
	}
	else
	{
		while ((dma->Control & 0x00018000) != 0x00018000); // TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		if(g_IDU_444_422_Flag)
		{
			MP_DEBUG("idu->RST_Ctrl_Bypass=%x", idu->RST_Ctrl_Bypass);
			idu->RST_Ctrl_Bypass |= 0x80000000 ;
		}
		else
		{
			MP_DEBUG("idu->RST_Ctrl_Bypass=%x", idu->RST_Ctrl_Bypass);
			idu->RST_Ctrl_Bypass &= 0x7fffffff ;
		}
#endif

		dma->StartA = ((DWORD) ImgWin->pdwStart | 0xa0000000);
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		if(g_IDU_444_422_Flag)	// 444 mode
		{
			mpDebugPrint("IDU DMA 444 mode");
			dma->EndA =
			((DWORD) ImgWin->pdwStart | 0xa0000000) + (ImgWin->wWidth * ImgWin->wHeight * 3) - 1;
		}
		else
#endif
		{
			dma->EndA =
				((DWORD) ImgWin->pdwStart | 0xa0000000) + (ImgWin->wWidth * ImgWin->wHeight * 2) - 1;
		}

		while ((dma->Control & 0x00018000) != 0x00010000); // TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		dma->StartB = dma->StartA;
		dma->EndB = dma->EndA;
	}
}
#endif

#if (IDU_CHANGEWIN_MODE == 0)
void Idu_WaitBufferEnd()
{
	register CHANNEL *iduDmaReg = (CHANNEL *) (DMA_IDU_BASE);
    ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;
    DWORD release;

    if (pstTCON->wIduClock < 270)
        return;

	if (!(iduDmaReg->Control & BIT0))
		return;					        // IDU DMA not enable

    SemaphoreWait(IDU_SEMA_ID);

    EventClear(IDU_EVENT_ID, ~(IDU_FRAME_END_A | IDU_FRAME_END_B));
    SystemIntEna(IM_DMA);
    DmaIntEna(IM_IDUDM);
	iduDmaReg->Control |= BIT24;        // Enable IDU DAM buffer end interrupt

	if (pstTCON->bInterlace == INTERLACE)
	{   // 200ms => about 5 frame/s
        EventWaitWithTO(IDU_EVENT_ID, IDU_FRAME_END_B, OS_EVENT_OR, &release, 200);
	}
	else
	{   // 200ms => about 5 frame/s
        EventWaitWithTO(IDU_EVENT_ID, IDU_FRAME_END_A | IDU_FRAME_END_B, OS_EVENT_OR, &release, 200);
	}

	iduDmaReg->Control &= ~BIT24;       // Disable IDU DAM buffer end interrupt

    SemaphoreRelease(IDU_SEMA_ID);
}
#else
void Idu_WaitBufferEnd()
{
    static BYTE semaUsed[2] = {0};
    BYTE eventIndex;
    register IDU *idu = (IDU *)IDU_BASE;
    register CHANNEL *iduDmaReg = (CHANNEL *) (DMA_IDU_BASE);
    ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;
    DWORD release;
    SDWORD resp;

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    if (idu->RST_Ctrl_Bypass & 0x1)
    {
        MP_ALERT("--E-- Idu_ChgWin while IDU in bypass mode !!");
        //__asm("break 100");
        return;
    }
#endif

    if (pstTCON->wIduClock < 270)
        return;

    if (!(iduDmaReg->Control & BIT0))
        return;                         // IDU DMA not enable

    SemaphoreWait(IDU_WAIT_BE_SEMA_ID);
    IntDisable();

    if (semaUsed[0] == 0)
    {
        semaUsed[0] = 1;
        eventIndex = 0;
    }
    else
    {
        semaUsed[1] = 1;
        eventIndex = 1;
    }

    SystemIntEna(IM_DMA);
    DmaIntEna(IM_IDUDM);
    iduDmaReg->Control |= BIT24;        // Enable IDU DAM buffer end interrupt

    if (eventIndex == 0)
        EventClear(IDU_EVENT_ID, ~(IDU_FRAME_END_A | IDU_FRAME_END_B));
    else
        EventClear(IDU_EVENT_ID, ~(IDU_FRAME_END_A2 | IDU_FRAME_END_B2));

    if (pstTCON->bInterlace == INTERLACE)
    {   // 200ms => about 5 frame/s
        if (eventIndex == 0)
            resp = EventWaitWithTO(IDU_EVENT_ID, IDU_FRAME_END_B, OS_EVENT_OR, &release, 200);
        else
            resp = EventWaitWithTO(IDU_EVENT_ID, IDU_FRAME_END_B2, OS_EVENT_OR, &release, 200);
    }
    else
    {   // 200ms => about 5 frame/s
        if (eventIndex == 0)
            resp = EventWaitWithTO(IDU_EVENT_ID, IDU_FRAME_END_A | IDU_FRAME_END_B, OS_EVENT_OR, &release, 200);
        else
            resp = EventWaitWithTO(IDU_EVENT_ID, IDU_FRAME_END_A2 | IDU_FRAME_END_B2, OS_EVENT_OR, &release, 200);
    }

    if (resp != OS_STATUS_OK)
    {
        MP_ALERT("--E-- %s: Event IDU_EVENT_ID timeout !!!!", __FUNCTION__);
    }

    if (eventIndex == 0)
        semaUsed[0] = 0;
    else
        semaUsed[1] = 0;

    SemaphoreRelease(IDU_WAIT_BE_SEMA_ID);
}


#define _MAX_NUM_OF_BUFFER_END_INFO_PTR_        2
void *bufferEndInfoCallbakc[_MAX_NUM_OF_BUFFER_END_INFO_PTR_] = {0};



#endif

#if 0
void Idu_WaitBufferEnd()
{
	register CHANNEL *dma;
    ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;

    if (pstTCON->wIduClock < 270)
        return;
mpDebugPrint("Idu_WaitBufferEnd");
	dma = (CHANNEL *) (DMA_IDU_BASE);

	if (!(dma->Control & 0x00000001))
		return;					//idu dma not enable
	dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt indicator
	dma->Control |= 0x01000000;	//dma buffer end interrupt

	if (pstTCON->bInterlace == INTERLACE)
	{
		while ((dma->Control & 0x00018000) != 0x00018000);	// TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

	}
	else
	{
		//wait for dma buffer end
		while (~dma->Control & 0x00010000);	//TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt
	}
}
#endif

void Idu_WinDmaInit(ST_IMGWIN * ImgWin, BYTE bInterlace)
{
	register CHANNEL *dma;

	dma = (CHANNEL *) (DMA_IDU_BASE);

	dma->Control = 0x00000000;
	dma->StartA = (DWORD) ImgWin->pdwStart | 0xa0000000;

	if (bInterlace)
	{
		dma->StartA = ((DWORD) ImgWin->pdwStart | 0xa0000000);
		dma->EndA =
			((DWORD) ImgWin->pdwStart | 0xa0000000) + (ImgWin->wWidth * (ImgWin->wHeight - 1) * 2) - 1;
#if ANTIVIBR_ENABLE
    dma->StartB = dma->StartA;
		dma->EndB = dma->EndA;
#else
		dma->StartB = dma->StartA + (ImgWin->wWidth * 2);
		dma->EndB = dma->EndA + (ImgWin->wWidth * 2) -1;
#endif
		dma->LineCount = ((ImgWin->wWidth * 2) << 16) + (ImgWin->wWidth * 2) - 1;
		dma->Control = 0x0000000e;	//double buffer, line counter, and cyclic dma buffering enable
	}
	else
	{
	
		//progressive
		dma->EndA = dma->StartA + ((ImgWin->wWidth * ImgWin->wHeight * 2 ) - 1);
		dma->StartB = dma->StartA;
		dma->EndB = dma->EndA;
		dma->LineCount = (0 << 16) + (ImgWin->wWidth * 2) - 1;
		//dma->Control = 0x00000006;  //line counter, and cyclic dma buffering enable
		dma->Control = 0x0000000e;	//double buffer, line counter, and cyclic dma buffering enable
       
	}
}


///
///@ingroup IMAGE_WIN
///@brief Check bus width between MP612 and TCON
///
///@param NONE
///
///@retval 0:Analog signal / Others:Bus width of digital signal
///
///@remark
///
BYTE Idu_ChkBusWidth()
{
    BYTE bBusType = g_psSystemConfig->sScreenSetting.pstCurTCON->bBusType;
    switch (bBusType)
    {
        case DISP_TYPE_CCIR656:
        case DISP_TYPE_LCD:
            return 8;
        case DISP_TYPE_CCIR601:
//mp612/615/600 CCIR601 8bit = CCIR656+H/V
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_600)
	          return 8;
#else
            return 16;
#endif
        case DISP_TYPE_ITU709:
            return 16;
        case DISP_TYPE_DVI_666:
            return 18;
        case DISP_TYPE_DVI_888:
        case DISP_TYPE_RGB24:
            return 24;
        case DISP_TYPE_COMPONENT:
        case DISP_TYPE_COMPOSITE:
        case DISP_TYPE_D_SUB:
        case DISP_TYPE_S_VIDEO:
            return 0;
    }
}
///
///@ingroup IMAGE_WIN
///@brief Turn on IDU/OSD module
///
///@param NONE
///
///@retval NONE
///
///@remark Mason modified 20061020, this function turn on the module only
///
void ImgDisplayON(void)
{
	register IDU *idu;
	register CHANNEL *dma, *dmaOSD;

	idu = (IDU *) (IDU_BASE);
	dma = (CHANNEL *) (DMA_IDU_BASE);
	dmaOSD = (CHANNEL *) (DMA_OSD_BASE);

#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
	idu->TvCtrl0 |= 0x20000000; //fix 1080i
#endif
	dma->Control |= 0x00000001;	//idu dma enable
	#if OSD_ENABLE
	dmaOSD->Control |= 0x00000001;	//osd dma enable
	#else
	dmaOSD->Control &= ~0x00000001; //osd dma disable
	#endif
	idu->IduSts = 0x00000300;	//OSD FIFO underflow flag and ID FIFO underflow flag enable
	idu->IduCtrl0 |= (OSD_ENABLE << 12); //OSD_EN = 1
	//jes - may be adjusted by case
	#ifdef DEMO_PID
	idu->IduCtrl0 |= 0x00000019;	//ID FIFO threshold = 0x11,  IDU enable
	#else
	idu->IduCtrl0 |= 0x00000011;	//ID FIFO threshold = 0x10,  IDU enable
	#endif
	//idu->IduCtrl0 |= 0x00000001; //ID FIFO threshold = 0x00, for panel usage, TV may enlarge this; enable IDU
	                             //Modified from MP600 Griffy
}
///
///@ingroup IMAGE_WIN
///@brief Turn on IDU/OSD module
///
///@param NONE
///
///@retval NONE
///
///@remark Mason modified 20061020, this function turn on the module only
///
///@demo code
///			TurnOffBackLight();
///			Idu_DisplayOff();
///            	IODelay(2000);
///			Idu_DisplayON();
///			IODelay(2000);
///			TurnOnBackLight();

void Idu_DisplayON(void)
{
    MP_DEBUG("%s", __func__);
	register IDU *idu;
	register CHANNEL *dma, *dmaOSD;

	idu = (IDU *) (IDU_BASE);
	dma = (CHANNEL *) (DMA_IDU_BASE);
	dmaOSD = (CHANNEL *) (DMA_OSD_BASE);
	register BIU *biu = (BIU *) BIU_BASE;

	struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

	biu->BiuSrst &= 0xfffffff7;
	biu->BiuSrst |= 0x00000008;

	//InitWins(pstScreen->wInnerWidth, pstScreen->wInnerHeight);
	Idu_WinDmaInit(Idu_GetCurrWin(), pstScreen->pstCurTCON->bInterlace);	//set DMA
	Idu_ClockInit(pstScreen->pstCurTCON); //dot clock griffy
    Idu_SetTVSync(pstScreen->pstCurTCON);
    Idu_BusInit(pstScreen->pstCurTCON);
    Idu_ScaleInit(pstScreen->wInnerWidth, pstScreen->wInnerHeight, pstScreen->pstCurTCON->wWidth, pstScreen->pstCurTCON->wHeight);

#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
	idu->TvCtrl0 |= 0x20000000; //fix 1080i
#endif
	dma->Control |= 0x00000001;	//idu dma enable
	#if OSD_ENABLE
	dmaOSD->Control |= 0x00000001;	//osd dma enable
	#else
	dmaOSD->Control &= ~0x00000001; //osd dma disable
	#endif
	idu->IduSts = 0x00000300;	//OSD FIFO underflow flag and ID FIFO underflow flag enable
	idu->IduCtrl0 |= (OSD_ENABLE << 12); //OSD_EN = 1
	//jes - may be adjusted by case
	#ifdef DEMO_PID
	idu->IduCtrl0 |= 0x00000019;	//ID FIFO threshold = 0x11,  IDU enable
	#else
	idu->IduCtrl0 |= 0x00000011;	//ID FIFO threshold = 0x10,  IDU enable
	#endif
	//idu->IduCtrl0 |= 0x00000001; //ID FIFO threshold = 0x00, for panel usage, TV may enlarge this; enable IDU
	                             //Modified from MP600 Griffy
#if OSD_ENABLE
    Idu_OSDInit((DWORD *) SystemGetMemAddr(OSD_BUF_MEM_ID),
      ALIGN_CUT_16(pstScreen->pstCurTCON->wWidth), pstScreen->pstCurTCON->wHeight, pstScreen->pstCurTCON->bInterlace, OSD_BIT_WIDTH,0);
#endif
	Idu_ChgWin(Idu_GetCurrWin());

}

///
///@ingroup IMAGE_WIN
///@brief   Turn off screen
///
///@param   NONE
///
///@retval  NONE
///
void Idu_DisplayOff()
{
    MP_DEBUG("%s", __func__);
	register IDU *idu;
	register CHANNEL *dma;
	register CLOCK *clock;

	dma = (CHANNEL *) (DMA_IDU_BASE);
	dma->Control = 0;

	dma = (CHANNEL *) (DMA_OSD_BASE);
	dma->Control = 0;

	idu = (IDU *) (IDU_BASE);
	idu->IduCtrl0 &= 0xfffffffe;
	//fail
	//idu->IduCtrl0 = 0;
	//idu->HScaleCtrl0 = idu->VScaleCtrl0 = 0;
	Idu_OsdOnOff(0);
	clock = (CLOCK *) (CLOCK_BASE);
	clock->MdClken &= 0xfbfffcff;	// [8]IDUCK, [9]LVDS ,,[26]IDU2CK  disable
	#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	idu->ATCON_srgb_ctrl &= ~0x00010000 ; //trun off LVDS
	#endif

}


///
///@ingroup IMAGE_WIN
///@brief Looking for TCON and Panel by ID
///
///@param BYTE bTCONId : The ID of TCON
///       BYTE bPanelId : The ID of Panel
///       WORD wIWidth : Width of IDU module, not necessary to be the same with the width from TCON
///       WORD wIHeight : See above
///
///@retval NONE
///
///@remark This function will looking for the table and connect the pointer of system config to the
///        corresponding entry of table.
///
#if LOCAL_DEBUG_ENABLE
void xpgDumppstCurTCON()
{
    MP_DEBUG("%s", __func__);
   
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
    ST_SCREEN_TABLE *pstScreenTable = &g_mstScreenTable[g_psSystemConfig->sScreenSetting.bScreenIndex];
    MP_DEBUG("%s: pstScreenTable->bPanelId = %d", __func__, pstScreenTable->bPanelId);
    MP_DEBUG("%s: pstScreenTable->bTCONId  = %d", __func__, pstScreenTable->bTCONId);
    MP_DEBUG("%s: pstScreen->pstCurTCON->bId = %d", __func__, pstScreen->pstCurTCON->bId);
    
    mpDebugPrint("****PANEL_ID(%d): %s  ****",  pstScreenTable->bPanelId, PANEL_ID_NAME[pstScreenTable->bPanelId]);
	mpDebugPrint("****TCON_ID(%d) : %s  ****",  pstScreenTable->bTCONId,  TCON_ID_NAME[pstScreenTable->bTCONId]);
	mpDebugPrint("****IDU_CLOCK   : %d ****",  pstScreen->pstCurTCON->wIduClock);
	
    mpDebugPrint("pstScreen->pstCurTCON->bId = %d", pstScreen->pstCurTCON->bId);
    mpDebugPrint("pstScreen->pstCurTCON->bBusType = %d", pstScreen->pstCurTCON->bBusType);
    mpDebugPrint("pstScreen->pstCurTCON->bClockSource = PLL(%d)", pstScreen->pstCurTCON->bClockSource);
    mpDebugPrint("pstScreen->pstCurTCON->bInterlace = %d", pstScreen->pstCurTCON->bInterlace);
    mpDebugPrint("pstScreen->pstCurTCON->wIduClock = %d", pstScreen->pstCurTCON->wIduClock);
    mpDebugPrint("pstScreen->pstCurTCON->wPixClock = %d", pstScreen->pstCurTCON->wPixClock);
    mpDebugPrint("pstScreen->pstCurTCON->wWidth = %d", pstScreen->pstCurTCON->wWidth);
    mpDebugPrint("pstScreen->pstCurTCON->wHeight = %d", pstScreen->pstCurTCON->wHeight);
    mpDebugPrint("pstScreen->pstCurTCON->dwPLL2Clk = %d", pstScreen->pstCurTCON->dwPLL2Clk);
}
#endif    

// SystemConfigInit() call first
void Idu_TableSearch(BYTE bTCONId, BYTE bPanelId, WORD wIWidth, WORD wIHeight)
{
    MP_DEBUG("%s: bTCONId = %d, bPanelId = %d, wIWidth = %d, wIHeight = %d", __func__, bTCONId, bPanelId, wIWidth, wIHeight);
    DWORD i;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

//    if (pstScreen->pstCurTCON->bId != bTCONId)
    {
        for (i = 0; g_mstTCON[i].bId; i ++)
        {
            if (g_mstTCON[i].bId == bTCONId)
            {
                MP_DEBUG("g_mstTCON[%d].bId == bTCONId", i);
                pstScreen->pstCurTCON = &g_mstTCON[i];
                break;
            }
        }
    }
    if (g_mstTCON[i].bId == TCON_NONE)
    {
        pstScreen->pstCurTCON = &g_mstTCON[i];
    }
//    if (pstScreen->pstPanel->bId != bPanelId)
    {
        for (i = 0; g_mstPanel[i].bId; i ++)
        {
            if (g_mstPanel[i].bId == bPanelId)
            {
                MP_DEBUG("g_mstPanel[%d].bId == bPanelId", i);
                pstScreen->pstPanel = &g_mstPanel[i];
                break;
            }
        }
    }
    if (g_mstPanel[i].bId == PANEL_NONE)
    {
        pstScreen->pstPanel = &g_mstPanel[i];
    }
    
    if (wIWidth == 0 || wIHeight == 0)
    {
        MP_DEBUG("wIWidth == 0 || wIHeight == 0");
        pstScreen->wInnerWidth = pstScreen->pstCurTCON->wWidth;
        pstScreen->wInnerHeight = pstScreen->pstCurTCON->wHeight;
    }
    else
    {
        pstScreen->wInnerWidth = wIWidth;
        pstScreen->wInnerHeight = wIHeight;
    }
    
    if (pstScreen->pstPanel->wWidthInPix == BYPASS)
    {
        pstScreen->pstPanel->wWidthInPix = pstScreen->pstCurTCON->wWidth;
        pstScreen->pstPanel->wHeightInPix = pstScreen->pstCurTCON->wHeight;
    }
    
#if LOCAL_DEBUG_ENABLE
    xpgDumppstCurTCON();
#endif
}


#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
void Idu_ClockInit(ST_TCON *pstTCON)
{
    MP_DEBUG("%s: pstTCON->wIduClock = %d", __func__, pstTCON->wIduClock);
	register IDU *idu = (IDU *) IDU_BASE;
	CLOCK *clock = (CLOCK *) CLOCK_BASE;
	DWORD dwTrgCks, dwCrystalCks, dwGCD;
	BYTE N_value, M_value, Charge_Pump, divider = 0 ;

    dwTrgCks = (DWORD)(pstTCON->wIduClock) * 100000;
    if(dwTrgCks <= 60000000)
    {
    	if((dwTrgCks * 2) > 60000000)
    	{
    		dwTrgCks = dwTrgCks * 2 ;
    		divider = 1 ;
    	}
    	else if((dwTrgCks * 7) >= 1200*100000)
    	{
    		dwTrgCks = dwTrgCks * 7 ;
    		dwTrgCks = dwTrgCks / 2 ;
    		divider = 2 ;
    	}
    	else if((dwTrgCks * 7) >= 600*100000)
    	{
    		dwTrgCks = dwTrgCks * 7 ;
    		divider = 3 ;
    	}
    }

    if(pstTCON->dwPLL2Clk==BYPASS)
    {
        Clock_PllFreqSet(CLOCK_PLLIDU_INDEX, dwTrgCks);//dwTrgCks 27000000
    }
    else
    {
	    Clock_PllCfgSet(CLOCK_PLLIDU_INDEX, pstTCON->dwPLL2Clk);
    }
    /*
    dwCrystalCks = MAIN_CRYSTAL_CLOCK / 100000 ;
    dwGCD = Get_GCD(dwTrgCks, dwCrystalCks);
    if(dwGCD == dwCrystalCks)
    	dwGCD = dwGCD >> 1 ;
    dwTrgCks = dwTrgCks / dwGCD ;
    dwCrystalCks = dwCrystalCks / dwGCD ;
    if(dwTrgCks > 256 || dwCrystalCks > 256)
    {
    	MP_ALERT("IDU clock calculation fail");
    }
    N_value = dwTrgCks - 1 ;
    M_value = dwCrystalCks - 1 ;
    Charge_Pump = 2 ;
    clock->PLLIduCfg = (Charge_Pump << 16) + (N_value << 8) + (M_value);
    clock->PLLIduCfg &= ~0x03000000 ;	//Set BIT24 and BIT25 with 0
    */
#if (CHIP_VER_MSB == CHIP_VER_650)
    clock->Clkss1 &= ~0x000000f8;
#else
    clock->Clkss1 &= ~0x000038C0;
#endif
        //clock->Clkss1 |= (divider << 3) ;
        //mpDebugPrint("------- clock->Clkss1=%p  divider=%d dwTrgCks=%d",clock->Clkss1,divider,dwTrgCks);
    //Enable IDU PLL
    clock->ClkCtrl &= ~0x08000000 ;
    IODelay(50);
    clock->ClkCtrl |= 0x08000000 ;

#if 0//(CHIP_VER_MSB == CHIP_VER_650)
    if(pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_666 ||
        pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_888 ||
        pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666 ||
        pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_888)
    {
        MP_DEBUG("Set LVDS CLK");
        clock->MdClken |= 0x00000200 ;
        if(pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666 || pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_888)
        {
            MP_DEBUG("Double channel CLK");
            clock->Clkss1 |= (2 << 3) ;
        }
        else
        {
            MP_DEBUG("Single channel CLK");
            clock->Clkss1 |= (3 << 3) ;
        }
    }
    else
    {
        clock->Clkss1 |= (divider << 3) ;
    }
       //MP66x no support LVDS
#else   // MP660
   /* if(pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_666 ||
        pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_888 ||
        pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666 ||
        pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_888)
    {
        MP_DEBUG("Set LVDS CLK");
        clock->MdClken |= 0x00000200 ;
        if(pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666 || pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_888)
        {
            MP_DEBUG("Double channel CLK");
            clock->Clkss1 |= (2 << 11) ;
        }
        else
        {
            MP_DEBUG("Single channel CLK");
            clock->Clkss1 |= (3 << 11) ;
        }
    }
    else*/

    if(pstTCON->bBusType == DISP_TYPE_RGB8_SERIAL)
    {
	//divider = 3 ;
    }

    {
        clock->Clkss1 |= (divider << 11) ;
    }
        mpDebugPrint("------- clock->Clkss1=%p  divider=%d dwTrgCks=%d",clock->Clkss1,divider,dwTrgCks);

#endif
#if (CHIP_VER_MSB == CHIP_VER_660)
    if ((pstTCON->bBusType == DISP_TYPE_CCIR656)||(pstTCON->bBusType == DISP_TYPE_CCIR601) || (pstTCON->bBusType == DISP_TYPE_RGB_A) ||(pstTCON->bBusType == DISP_TYPE_RGB8_SERIAL))
    {
		clock->MdClken |= 0x04000000;
	}
#endif
    clock->MdClken |= 0x00000100;	// IDU clock enable

}
#endif

///
///@ingroup IMAGE_WIN
///@brief Setting TV sync.
///
///@param ST_TCON *pstTCON : The TCON to be used
///
///@retval NONE
///
///@remark This function will set TV sync register. If this TCON is in interlace signal, the valid
///        vertical period will be set to half of display height.
///
static void Idu_SetTVSync(ST_TCON *pstTCON)
{
	register IDU *idu = (IDU *) IDU_BASE;
    ST_TV_SYNC *pstTVSync = &pstTCON->stTVSync;
	DWORD h_sync, v_sync, h_back_porch, h_front_porch, v_back_porch, v_front_porch, display_x, display_y;

	h_sync = (pstTVSync->wHSync - 1);
	h_back_porch = (pstTVSync->wHBkPorch - 1);
	h_front_porch = (pstTVSync->wHFrPorch - 1);
	display_x = (pstTCON->wWidth - 1);
	v_sync = (pstTVSync->wVSync - 1);
	v_back_porch = (pstTVSync->wVBkPorch - 1);
	v_front_porch = (pstTVSync->wVFrPorch - 1);
    if (pstTCON->bInterlace == INTERLACE)
         display_y = ((pstTCON->wHeight >> 1) - 1);
	else
	 #if (PANEL_ID != PANEL_ILI9163C)
        display_y = (pstTCON->wHeight - 1);
     #else
        display_y = (pstTCON->wHeight/2 - 1);
     #endif

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	idu->TvHCtrl0 = (((h_back_porch & 0x1ff) << 16) | (h_sync & 0x1ff));
	idu->TvHCtrl1 = (((h_front_porch & 0x1ff) << 16) | (display_x & 0x7ff));
	idu->TvVCtrl0 = (((v_back_porch & 0x1ff) << 16) | (v_sync & 0x1ff));
	idu->TvVCtrl1 = (((v_front_porch & 0x1ff) << 16) | (display_y & 0x7ff));
#else
	idu->TvHCtrl0 = (((h_back_porch & 0xff) << 8) | (h_sync & 0xff));
	idu->TvHCtrl1 = (((h_front_porch & 0xff) << 16) | (display_x & 0x7ff));
	idu->TvVCtrl0 = (((v_back_porch & 0xff) << 8) | (v_sync & 0xff));
	idu->TvVCtrl1 = (((v_front_porch & 0xff) << 16) | (display_y & 0x7ff));
#endif
/*	idu->TvHCtrl0 = (((h_back_porch & 0xff) << 8) | (h_sync & 0xff));
	idu->TvHCtrl1 = (((h_front_porch & 0xff) << 16) | (display_x & 0x7ff));
	idu->TvVCtrl0 = (((v_back_porch & 0xff) << 8) | (v_sync & 0xff));
	idu->TvVCtrl1 = (((v_front_porch & 0xff) << 16) | (display_y & 0x7ff));

	*/
    if (pstTCON->bInterlace == INTERLACE)   //set P1EVEN, may by case, from MP6000 Griffy
    	idu->TvVCtrl0 |= (1 << 30);

}

///
///@ingroup IMAGE_WIN
///@brief Setting TV control.
///
///@param ST_TCON *pstTCON : The TCON to be used
///
///@retval NONE
///
///@remark This function will set TVCtrl0 and TVCtrl1, refer spec for detail.
///
static void Idu_TVCtrlInit(ST_TCON *pstTCON)
{
	register IDU *idu = (IDU *) IDU_BASE;
#if 0   // Using another format selection to represent, Modify from MP600 Griffy
	if (pstTCON->bBusType == DISP_TYPE_D_SUB && pstTCON->wWidth <= 720)
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000001;
	else if (pstTCON->bBusType == DISP_TYPE_D_SUB && pstTCON->wWidth > 720)
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000002;
	else if (pstTCON->wWidth == 720 && pstTCON->bInterlace == INTERLACE) //FORMAT_SEL
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000000;
	else if (pstTCON->wWidth == 720 && pstTCON->bInterlace == PROGRASSIVE)
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000001;
	else if (pstTCON->bBusType == DISP_TYPE_COMPONENT && pstTCON->wWidth == 1280 && pstTCON->bInterlace == PROGRASSIVE)
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000002;
	else if (pstTCON->bBusType == DISP_TYPE_COMPONENT && pstTCON->wWidth == 1920 && pstTCON->bInterlace == INTERLACE)
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000003;
    else
        MP_DEBUG("-E- Selected output format isn't supported!");

	if (pstTCON->wWidth == 720 && pstTCON->bInterlace == INTERLACE) //DO_SEL
    {
		if (pstTCON->bBusType == DISP_TYPE_COMPOSITE)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000000;
		else if (pstTCON->bBusType == DISP_TYPE_S_VIDEO)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000004;
		else if (pstTCON->bBusType == DISP_TYPE_COMPONENT)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000008;
		//else if (pstTCON->bBusType == DISP_TYPE_RGB)  //We've never tune this kind of output
    	//	idu->TvVCtrl0 = (idu->TvVCtrl0 & 0xfffffff3) + 0x0000000c;
        else
            MP_DEBUG("-E- Selected output format isn't supported!");
    }
    else
    {
		if (pstTCON->bBusType == DISP_TYPE_COMPONENT)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000000;
        else
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x0000000c;
    }

    if (pstTCON->bBusType == DISP_TYPE_COMPONENT || pstTCON->bBusType == DISP_TYPE_D_SUB)   //SYNCO_SEL
        idu->TvCtrl0 = (idu->TvCtrl0 & 0xffffffcf) + 0x00000030;
    else
        idu->TvCtrl0 = (idu->TvCtrl0 & 0xffffffcf) + 0x00000000;
#else // This section is new, from MP600 Griffy
//FORMAT_SEL
    if (pstTCON->wHeight <= 480 && pstTCON->bInterlace == INTERLACE)  //480i
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000000;
    else if (pstTCON->wHeight <= 480 && pstTCON->bInterlace == PROGRASSIVE) //480p
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000001;
    else if (pstTCON->wHeight <= 720 && pstTCON->bInterlace == PROGRASSIVE) //720p
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000002;
    else if (pstTCON->wHeight <= 1080 && pstTCON->bInterlace == INTERLACE) //1080i
		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffffc) + 0x00000003;
    else
        MP_DEBUG("-E- Selected output format isn't supported!");

//DO_SEL
    if (pstTCON->wHeight<= 480 && pstTCON->bInterlace == INTERLACE) //480i
    {
		if (pstTCON->bBusType == DISP_TYPE_COMPOSITE)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000000;
		else if (pstTCON->bBusType == DISP_TYPE_S_VIDEO)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000004;
		else if (pstTCON->bBusType == DISP_TYPE_COMPONENT)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000008;
		//else if (pstTCON->bBusType == DISP_TYPE_RGB)  //We've never tune this kind of output
    //	idu->TvVCtrl0 = (idu->TvVCtrl0 & 0xfffffff3) + 0x0000000c;
        	else
            	MP_DEBUG("-E- Selected output format isn't supported!");
    }
    else
    {
		if (pstTCON->bBusType == DISP_TYPE_COMPONENT)
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x00000000;
        	else
    		idu->TvCtrl0 = (idu->TvCtrl0 & 0xfffffff3) + 0x0000000c;
    }

//SYNCO_SEL
    if (pstTCON->bBusType == DISP_TYPE_COMPONENT || pstTCON->bBusType == DISP_TYPE_D_SUB)
        idu->TvCtrl0 = (idu->TvCtrl0 & 0xffffffcf) + 0x00000030;
    else
        idu->TvCtrl0 = (idu->TvCtrl0 & 0xffffffcf) + 0x00000000;
#endif

	if (pstTCON->wWidth == 720 && pstTCON->wHeight == 576)
		idu->IduCtrl0 |= 0x00080000;	//PAL
	else if (pstTCON->wWidth == 720 && pstTCON->wHeight == 480)
		idu->IduCtrl0 &= 0xfff7ffff;	//NTSC
}
///
///@ingroup IMAGE_WIN
///@brief Setting TV control.
///
///@param ST_TCON *pstTCON : The TCON to be used
///
///@retval NONE
///
///@remark This function will set color space conversion of TVCtrl0 and TVCtrl1, refer spec for detail.
///
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_600)
static void Idu_ColorConv(ST_TCON *pstTCON)
{
    register IDU *idu = (IDU *) (IDU_BASE);
    DWORD RG_OC0 = 0, RG_OC1 = 0, RG_OC2 = 0;
    DWORD RG_MC00 = 0, RG_MC01 = 0, RG_MC02 = 0;
    DWORD RG_MC10 = 0, RG_MC11 = 0, RG_MC12 = 0;
    DWORD RG_MC20 = 0, RG_MC21 = 0, RG_MC22 = 0;

//3x3 color matrix have right sideline
//    idu->TvCtrl0 |= 0x06000000;	//enable 3 matrix CSC, disable YCbCr to RGB
      idu->TvCtrl0 &= ~0x06000000; //diable 3x3, enable fix translate to avoid right sideline
//for RGB color space
    if((pstTCON->bBusType == DISP_TYPE_DVI_666) \
		||(pstTCON->bBusType == DISP_TYPE_DVI_888) \
		||(pstTCON->bBusType == DISP_TYPE_RGB24))
    {
	    RG_OC0 = 0x14d, RG_OC1 = 0x87, RG_OC2 = 0x11e;
	    RG_MC00 = 0xff, RG_MC01 = 0x0, RG_MC02 = 0x166;
	    RG_MC10 = 0xf0, RG_MC11 = 0x3a8, RG_MC12 = 0x34a;
	    RG_MC20 = 0xf0, RG_MC21 = 0x1c5, RG_MC22 = 0x0;
    }
//for YUV color space, CCIR601/656/709
    else
    {
	    RG_OC0 = 0, RG_OC1 = 0, RG_OC2 = 0;
	    RG_MC00 = 256, RG_MC01 = 0, RG_MC02 = 0;
	    RG_MC10 = 0, RG_MC11 = 256, RG_MC12 = 0;
	    RG_MC20 = 0, RG_MC21 = 0, RG_MC22 = 256;
    }

    idu->Iducsc0 = ((RG_OC2 & 0x1ff) << 18) + ((RG_OC1 & 0x1ff) << 9) + (RG_OC0 & 0x1ff);
    idu->Iducsc1 = ((RG_MC02 & 0x3ff) << 20) + ((RG_MC01 & 0x3ff) << 10) + (RG_MC00 & 0x3ff);
    idu->Iducsc2 = ((RG_MC12 & 0x3ff) << 20) + ((RG_MC11 & 0x3ff) << 10) + (RG_MC10 & 0x3ff);
    idu->Iducsc3 = ((RG_MC22 & 0x3ff) << 20) + ((RG_MC21 & 0x3ff) << 10) + (RG_MC20 & 0x3ff);
}
#else
static void Idu_ColorConv(ST_TCON *pstTCON)
{
	register IDU *idu = (IDU *) (IDU_BASE);
    DWORD SF_Y = 0x8c, SF_U_B = 0x71, SF_V_R = 0xa0, SF_NB = 0, SF_NR = 0;

    switch (pstTCON->bBusType)
    {
        case DISP_TYPE_CCIR601:
            SF_U_B = 0x71;
            SF_Y = 0x8c;
            SF_NR = 0xff;
            SF_NB = 0;
            SF_V_R = 0xff;
            break;
        case DISP_TYPE_COMPONENT:
            SF_U_B = 0x78;
            SF_Y = 0x75;
            SF_NR = 0;
            SF_NB = 0;
            SF_V_R = 0x88;
            break;
        case DISP_TYPE_D_SUB:
            SF_U_B = 0xe7;
            SF_Y = 0x82;
            SF_NR = 0x5d;
            SF_NB = 0x2d;
            SF_V_R = 0xb7;
            break;
        case DISP_TYPE_DVI_666:
        case DISP_TYPE_DVI_888:
            #if 0//Original, from annoymous
            SF_U_B = 0xe7;
            SF_Y = 0x82;
            SF_NR = 0x7d;
            SF_NB = 0x35;
            SF_V_R = 0xc7;
            #else//From Fengrs 20061103
            SF_U_B = 0xc5;
            SF_Y = 0x00;
            SF_NR = 0xb6;
            SF_NB = 0x5c;
            SF_V_R = 0xff;
            #endif
            break;
        case DISP_TYPE_COMPOSITE:
        case DISP_TYPE_S_VIDEO:
        case DISP_TYPE_CCIR656:
        case DISP_TYPE_LCD:
        default:
            break;
    }
    idu->TvCtrl0 = (idu->TvCtrl0 & 0xfe0000ff) + ((SF_U_B & 0x1ff) << 16) + ((SF_Y & 0xff) << 8);
    idu->TvCtrl1 = ((SF_NR & 0x1ff) << 20) + ((SF_NB & 0x1ff) << 10) + (SF_V_R & 0x1ff);
}
#endif



void Idu_InternelTCONInit(void)
{
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_600 || (CHIP_VER & 0xffff0000) == CHIP_VER_650 || (CHIP_VER & 0xffff0000) == CHIP_VER_660)
    register IDU *idu = (IDU *) (IDU_BASE);
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
    ST_TCON *pstTCON = pstScreen->pstCurTCON;

	if((pstTCON->wWidth == 800) && (pstTCON->wHeight == 480)){
//easyTCON650
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;
    idu->Gpoh1 = 0x00000000;

    idu->Gpov2 = 0xa1dff009;
    idu->Gpoh2 = 0xa1bc6001;

    idu->Gpov3 = 0x00000000;
    idu->Gpoh3 = 0x00000000;

    idu->Gpov4 = 0xa1dfa1de;
    idu->Gpoh4 = 0x23122156;

    idu->Gpov5 = 0xa1dfa000;
    idu->Gpoh5 = 0xa156202e;

    idu->Gpov6 = 0xa1dfa000;
    idu->Gpoh6 = 0xa156202e;

    idu->Gpov7 = 0xa1dfa000;
    idu->Gpoh7 = 0xe01e601a;

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;

    idu->Gpov9 = 0xa1dfa000;
    idu->Gpoh9 = 0xa31b231a;

    idu->Gpoctrl0 = 0x02000800;
    idu->Gpoctrl1 = 0x90200001;
    }
    else if ((pstTCON->wWidth == 852) && (pstTCON->wHeight == 600)){
//easyTCON650
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;
    idu->Gpoh1 = 0x00000000;

//SPOL
    idu->Gpov2 = 0x5001f3ff;
    idu->Gpoh2 = 0x90006001;

    idu->Gpov3 = 0x00000000;
    idu->Gpoh3 = 0x00000000;

//STV
    idu->Gpov4 = 0xa1dfa1de;
    idu->Gpoh4 = 0x23462346;

//CKV
    idu->Gpov5 = 0x5000a000;
    idu->Gpoh5 = 0xa18a202e;

//GOEV
    idu->Gpov6 = 0x81dfc00c;
    idu->Gpoh6 = 0xe09d211d;

//SLD
    idu->Gpov7 = 0xa1dfa000;
    idu->Gpoh7 = 0xe00e600a;

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;

//SSTH
    idu->Gpov9 = 0xa1dfa000;
    idu->Gpoh9 = 0xb0313030;

    idu->Gpoctrl0 = 0x02000800;
    idu->Gpoctrl1 = 0x90200000;
    }
#if 0//PANEL_ID == PANEL_AT080TN42//kernel 080814
	else if ((pstTCON->wWidth == 800) && (pstTCON->wHeight == 600)){
//easyTCON650	    
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;  
    idu->Gpoh1 = 0x00000000;
//SPOL
    idu->Gpov2 = 0xa257f009;
    idu->Gpoh2 = 0xa1bc6001;

    idu->Gpov3 = 0x00000000;
    idu->Gpoh3 = 0x00000000;
//STV 
    idu->Gpov4 = 0xa257a256;
    idu->Gpoh4 = 0x23122156;
//CKV  
    idu->Gpov5 = 0xa257a000;
    idu->Gpoh5 = 0xa156202e;
//GOEV
    idu->Gpov6 = 0xa257a000;
    idu->Gpoh6 = 0xa156202e;
//SLD
    idu->Gpov7 = 0xa257a000;
    idu->Gpoh7 = 0xe0176011;//kernel set 0xe0186012 as 0xe0176011 080814

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;
//SSTH 
    idu->Gpov9 = 0xa257a000;
    idu->Gpoh9 = 0xa310230e;//kernel set 0xa310230f as  0xa310230e 080814
    
    idu->Gpoctrl0 = 0x02000800;
    idu->Gpoctrl1 = 0x90200000;
	}
#else
	else if ((pstTCON->wWidth == 800) && (pstTCON->wHeight == 600)){
//easyTCON650	    
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;  
    idu->Gpoh1 = 0x00000000;
//SPOL
    idu->Gpov2 = 0xa257f009;
    idu->Gpoh2 = 0xa1bc6001;

    idu->Gpov3 = 0x00000000;
    idu->Gpoh3 = 0x00000000;
//STV 
    idu->Gpov4 = 0xa257a256;
    idu->Gpoh4 = 0x23122156; 	
//CKV  
    idu->Gpov5 = 0xa257a000;
    idu->Gpoh5 = 0xa156202e;
//GOEV
    idu->Gpov6 = 0xa257a000;
    idu->Gpoh6 = 0xa156202e;
//SLD
    idu->Gpov7 = 0xa257a000;
    idu->Gpoh7 = 0xe0186012;

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;
//SSTH 
    idu->Gpov9 = 0xa257a000;
    idu->Gpoh9 = 0xa310230f;
    //idu->Gpoh9 = 0xa3132312;
    //idu->Gpoh9 = 0xa31b231a;

    idu->Gpoctrl0 = 0x02000800;
    idu->Gpoctrl1 = 0x90200000;
	}
#if 0
	else if ((pstTCON->wWidth == 800) && (pstTCON->wHeight == 600)){
//easyTCON650
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;
    idu->Gpoh1 = 0x00000000;

    idu->Gpov2 = 0xa257f009;
    idu->Gpoh2 = 0xa1bc6001;

    idu->Gpov3 = 0x00000000;
    idu->Gpoh3 = 0x00000000;

    idu->Gpov4 = 0xa257a256;
    idu->Gpoh4 = 0x23122156;

    idu->Gpov5 = 0xa257a000;
    idu->Gpoh5 = 0xa156202e;

    idu->Gpov6 = 0xa257a000;
    idu->Gpoh6 = 0xa156202e;

    idu->Gpov7 = 0xa257a000;
    idu->Gpoh7 = 0xe01e601a;

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;

    idu->Gpov9 = 0xa257a000;
    idu->Gpoh9 = 0xa31b231a;	//idu->Gpoh9 = 0xa3132312;
    //idu->Gpoh9 = 0xa31a2319;

    idu->Gpoctrl0 = 0x02000800;
    idu->Gpoctrl1 = 0x90200000;
	}
#endif
#endif

	else if ((pstTCON->wWidth == 480) && (pstTCON->wHeight == 640))
	{
//480x640
#if 0
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;
    idu->Gpoh1 = 0x00000000;

    idu->Gpov2 = 0x00000000;
    idu->Gpoh2 = 0x00000000;

    idu->Gpov3 = 0x90049003;
    idu->Gpoh3 = 0xa1df21df;

    idu->Gpov4 = 0x90029001;
    idu->Gpoh4 = 0xa1df21df;

    idu->Gpov5 = 0x9002a002;
    idu->Gpoh5 = 0xb0266020;

    idu->Gpov6 = 0x9002a002;
    idu->Gpoh6 = 0xb0026017;

    idu->Gpov7 = 0x9002a002;
    idu->Gpoh7 = 0xb0273003;

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;

    idu->Gpov9 = 0x9002a002;
    idu->Gpoh9 = 0xa1da2000;

#else
    idu->Gpov0 = 0x00000000;
    idu->Gpoh0 = 0x00000000;

    idu->Gpov1 = 0x00000000;
    idu->Gpoh1 = 0x00000000;

    idu->Gpov2 = 0x00000000;
    idu->Gpoh2 = 0x00000000;

    idu->Gpov3 = 0x90019000;
    idu->Gpoh3 = 0xb0322150;

    idu->Gpov4 = 0x90019000;
    idu->Gpoh4 = 0xa1082030;

    idu->Gpov5 = 0x9000a000;
    idu->Gpoh5 = 0xb0266020;

    idu->Gpov6 = 0x9000a000;
    idu->Gpoh6 = 0xb0026017;

    idu->Gpov7 = 0x9000a000;
    idu->Gpoh7 = 0xb0263003;

    idu->Gpov8 = 0x00000000;
    idu->Gpoh8 = 0x00000000;

    idu->Gpov9 = 0x9000a000;
    idu->Gpoh9 = 0xa1da2000;
#endif
    idu->Gpoctrl0 = 0x00108000;
    idu->Gpoctrl1 = 0x90208000;
	}
    else
        MP_ALERT("\r\n %s: Not support panel - pstTCON->wWidth(%d), pstTCON->wHeight(%d)", __func__, pstTCON->wWidth, pstTCON->wHeight);

    idu->IduCtrl0 |= 0x00400000;	//enable dithering
#endif
}

void Idu_InternalATCONInit(ST_TCON *pstTCON)
{
	IDU *idu = (IDU *)(IDU_BASE);
#if ((CHIP_VER &0xffff0000) == CHIP_VER_650)

#ifdef NEW_IDU
	idu->Gpoh0 = 0x90076001;
	idu->Gpov0 = 0x20e9300c;

	idu->Gpoh1 = 0x90076001;
	idu->Gpov1 = 0x20e933ff;

	idu->Gpoh2 = 0xe0006001;
	idu->Gpov2 = 0x100033ff;
	/*idu->Gpoh2 = pstTCON->stTVSync.wHBkPorch - 3;
	idu->Gpoh2 |= 0x90006000;
	idu->Gpov2 = (pstTCON->stTVSync.wVBkPorch & 0x1)? 0 : 1;
	idu->Gpov2 |= 0x10001000;*/

	idu->Gpoh3 = 0xa1906001;
	idu->Gpov3 = 0x20e91000;

	//idu->Gpoh4 = 0x10021002;
	//idu->Gpov4 = 0x10011000;

	idu->Gpoh4 = pstTCON->wWidth >> 1;
	idu->Gpoh4 |= ((pstTCON->wWidth & 0xfffffffe) << 15);
	idu->Gpoh4 |= 0x20002000;
	idu->Gpov4 = (pstTCON->wWidth -3) << 16;
	idu->Gpov4 = 0x10011000;

	//idu->Gpoh5 = 0xb01a3000;
	//idu->Gpov5 = 0x20e923ff;

	idu->Gpoh5 = (pstTCON->stTVSync.wHFrPorch - 3) << 16;
	idu->Gpoh5 |= 0xe0003000;
	idu->Gpov5 = 0x00000000;//(pstTCON->wHeight- 1) << 16;
	idu->Gpov5 |= 0x00006000;

	//idu->Gpoh6 = 0xb0002132;
	//idu->Gpov6 = 0x20e923ff;

	idu->Gpoh6 = (pstTCON->stTVSync.wHFrPorch -10) << 16;
	idu->Gpoh6 |= 0xe0001000;
	idu->Gpov6 = idu->Gpov5;

	//idu->Gpoh7 = 0xb01a3000;
	//idu->Gpov7 = 0x20e923ff;

	idu->Gpoh7 = 0xa0001000;
	idu->Gpov7 = idu->Gpov5;

	idu->Gpoh8 = 0xe034205f;
	idu->Gpov8 = 0x20e9200c;

	//idu->Gpoh9 = 0xa12f212e;
	//idu->Gpov9 = 0x20e923ff;

	idu->Gpoh9 = (pstTCON->wWidth - 5) << 16;
	idu->Gpoh9 |= pstTCON->wWidth - 6;
	idu->Gpoh9 |= 0xa0002000;

	idu->Gpov9 = idu->Gpov5;
	//idu->Gpov9 = 0x20e923ff;

	idu->Gpoctrl0= 0x000f8c62;
	idu->Gpoctrl1 = 0x83000001;
#else
	if ((pstTCON->wWidth == 320) && (pstTCON->wHeight == 234))
	{
	//320x234
		idu->Gpov0= 0x20e933ff;
		idu->Gpoh0 = 0x90076001;

		idu->Gpov1 = 0x20e933ff;
		idu->Gpoh1 = 0x90076001;

 		idu->Gpov2 = 0x100033ff;
		idu->Gpoh2 = 0x90006001;

		idu->Gpov3 = 0x20e91000;
		idu->Gpoh3 = 0xa1906001;

 		idu->Gpov4 = 0x10011000;
		idu->Gpoh4 = 0x10021002;

		idu->Gpov5 = 0x20e923ff;
		//idu->Gpoh5 = 0xb00a2126;
		idu->Gpoh5 = 0xe0103001;

 		idu->Gpov6 = 0x20e923ff;
		//idu->Gpoh6 = 0xb0002132;
		idu->Gpoh6 = 0xe0196008;

		idu->Gpov7 = 0x20e923ff;
		//idu->Gpoh7 = 0xb01a3000;
		idu->Gpoh7 = 0xe0166007;

 		idu->Gpov8 = 0x20e9200c;
		idu->Gpoh8 = 0xe034205f;

		idu->Gpov9 = 0x20e923ff;
		idu->Gpoh9 = 0xa12f212e;

		/*idu->Gpoctrl0= 0x000f8c63;//7e;
		idu->Gpoctrl1= 0x80800001; */
		idu->Gpoctrl0= 0x000f8862;//0x000f8c7e;//63;//7e;

		idu->Gpoctrl1= 0x83000001;//0x83000001;

 		/*idu->TvHCtrl0= 0x00340009;
		idu->TvHCtrl1 = 0x001a013f;
		idu->TvVCtrl0 = 0x00060002;
		idu->TvVCtrl1 = 0x000a00e9;*/
	}
	if ((pstTCON->wWidth == 480) && (pstTCON->wHeight == 234))
	{
//480x234
		idu->Gpov0= 0x20e9300c;//0x20e933ff;
		idu->Gpoh0 = 0x90076001;

		idu->Gpov1 = 0x20e933ff;
		idu->Gpoh1 = 0x90076001;

		idu->Gpov2 = 0x20e9300c;//0x100033ff;
		idu->Gpoh2 = 0x90076001;//0xe0006001;

		idu->Gpov3 = 0x20e91000;
		idu->Gpoh3 = 0xa1906001;

		idu->Gpov4 = 0x10021001;//0x10031002;
		idu->Gpoh4 = 0x10021002;

		idu->Gpov5 = 0x20e923ff;
		idu->Gpoh5 = 0xe0001000;//0xb00a21c6;

		idu->Gpov6 = 0x20e923ff;
		idu->Gpoh6 = 0xb00021d0;

		idu->Gpov7 = 0x20e923ff;
		idu->Gpoh7 = 0xb01a3000;

		idu->Gpov8 = 0x20e9200c;
		idu->Gpoh8 = 0xe034205f;

		idu->Gpov9 = 0x20e923ff;
		idu->Gpoh9 = 0xa1d021cf;

		idu->Gpoctrl0= 0x000f8862;//0x000f8c7e;//63;//7e;

		//idu->Gpoctrl1= 0x83000001;//0x83000001;
		idu->Gpoctrl1= 0x83008001;
	}
#endif
#endif
}

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
DWORD DtdClkSetting = 0;
DWORD DtdRegSetting = 0;
DWORD Idu_GetDtdRegSetting()
{
    return DtdRegSetting;
}
void Idu_SetDACClk()		//Set the DAC clock as the source of idu clock
{
#if TCON_ANALOG_FUNC_3
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	clock->Clkss1 |= 0x00000008;
#else
	CLOCK *clock = (CLOCK *)CLOCK_BASE;
	DWORD idu_clk, dac_clk;

	//idu_clk = (clock->Clkss1 & 0x000000f0) >> 4;
	idu_clk = mGetIduCks();

	if (idu_clk >= 8)
		dac_clk = 4;
	else
		dac_clk = 0;

	clock->Clkss1 &= ~0x00000708;
	//clock->Clkss1 |= (dac_clk << 8);
	mSetTvCks(dac_clk);
#endif
}

#if (BRIGHTNESS_CTRL == LCD_BRIGHTNESS_HW1) // PWM
void Idu_PWMInit()
{
	IDU* idu = (IDU *)IDU_BASE;
	CLOCK *clock = (CLOCK*)CLOCK_BASE;
	GPIO *gpio = (GPIO *)GPIO_BASE;

	DtdRegSetting = 0x000085fc;    //VGH/BL active high, VGL active low
	idu->ATCON_DAC_ctrl = DtdRegSetting;

	//clock->Clkss_EXT3=0x600F01FF;		//LED CLK=375K,50% - PWM2, 
	//clock->Clkss_EXT6=0x600F01FF;		//DTD CLK=375K,50% - PWM0
	
	//clock->Clkss_EXT3=0x800800FF;		//LED CLK=750K,50%
	//clock->Clkss_EXT6=0x800800FF;		//DTD CLK=750K,50%
	
	//clock->Clkss_EXT3=0xA004008F;		//LED CLK=1500K,50%
	//clock->Clkss_EXT6=0xA004008F;		//DTD CLK=1500K,50%
    #if((TCON_ID == TCON_RGB24Mode_800x480 && PANEL_ID == PANEL_AT080TN03)||\
	(TCON_ID == TCON_RGB24Mode_240x400 && PANEL_ID == PANEL_ILI9327))	
        clock->Clkss_EXT3=0x6006009F;		//LED CLK=375K,70% 6V~7V- PWM2, 
	    clock->Clkss_EXT6=0x6006009F;		//DTD CLK=375K,70% 6V~7V- PWM0
	//#elif (TCON_ID == TCON_RGB24Mode_800x600 && PANEL_ID == PANEL_AT080TN01)
	//   clock->Clkss_EXT3=0x6002009F;		//LED CLK=375K,30% 10V~12V- PWM2, 
	//   clock->Clkss_EXT6=0x6002009F;		//DTD CLK=375K,30% 10V~12V- PWM0
	#endif

	clock->Clkss_EXT3 |= BIT28;         // Turn-on LED PWM Length Detector - PWM2
	clock->Clkss_EXT6 |= BIT28;         // Turn-on DTD PWM Length Detector - PWM0

	MP_DEBUG("clock->Clkss_EXT3 = %x", clock->Clkss_EXT3);
	MP_DEBUG("clock->Clkss_EXT6 = %x", clock->Clkss_EXT6);

	Gpio_ConfiguraionSet(GPIO_PGPIO_0, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 4);
}
    
#endif

#if (BRIGHTNESS_CTRL == LCD_BRIGHTNESS_HW2)

void Idu_BL_Ctrl(BYTE index)
{
	CLOCK *clock = (CLOCK*)CLOCK_BASE;

	if ((index >=0) && (index <= 4))
	{
		clock->Clkss_EXT1 &= 0xfffff00f;	//clear USBH
		clock->Clkss_EXT1 |= ((index + 2) << 4);
	}
}

void Idu_DTD_BL_On()
{
	IDU *idu = (IDU *)IDU_BASE;
	DtdRegSetting = DtdRegSetting & 0xffffbfff;
	idu->ATCON_DAC_ctrl = DtdRegSetting;
}

void Idu_DTD_BL_Off()
{
	IDU *idu = (IDU *)IDU_BASE;
	DtdRegSetting = DtdRegSetting | 0x00004000;
	idu->ATCON_DAC_ctrl = DtdRegSetting;
}
#endif

void Idu_DTD_VG_On()
{
	IDU *idu = (IDU *)IDU_BASE;
	DtdRegSetting = DtdRegSetting & 0xffffcfff;
	idu->ATCON_DAC_ctrl = DtdRegSetting;
}

void Idu_DTD_VG_Off()
{
	IDU *idu = (IDU *)IDU_BASE;
	DtdRegSetting = DtdRegSetting | 0x00003000;
	idu->ATCON_DAC_ctrl = DtdRegSetting;
}


void Idu_DTDInit(ST_TCON *pstTCON)
{
	IDU* idu = (IDU *)IDU_BASE;
	CLOCK *clock = (CLOCK*)CLOCK_BASE;
	GPIO *gpio = (GPIO *)GPIO_BASE;


	DtdRegSetting = 0x000085fc;    //VGH/BL active high, VGL active low
	idu->ATCON_DAC_ctrl = DtdRegSetting;

	//mEnableUsbdCks();
	//Wendy add for MP650 DTD

	clock->Clkss_EXT3=0x600F01FF;		//LED CLK=375K,50%
	clock->Clkss_EXT6=0x600F01FF;		//DTD CLK=375K,50%

//	clock->Clkss_EXT3=0x800800FF;		//LED CLK=750K,50%
//	clock->Clkss_EXT6=0x800800FF;		//DTD CLK=750K,50%

	clock->Clkss_EXT3 |= BIT28;         // Turn-on LED PWM Length Detector
	clock->Clkss_EXT6 |= BIT28;         // Turn-on DTD PWM Length Detector

	//clock->MdClken |= 0x14000000;		//enable dtd
	MP_DEBUG("clock->MdClken = %x", clock->MdClken);
	MP_DEBUG("clock->Clkss_EXT3 = %x", clock->Clkss_EXT3);
	MP_DEBUG("clock->Clkss_EXT6 = %x", clock->Clkss_EXT6);

	Gpio_ConfiguraionSet(GPIO_PGPIO_0, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, GPIO_DATA_HIGH, 4);
    #if (BRIGHTNESS_CTRL == LCD_BRIGHTNESS_HW2)
    Gpio_Config2GpioFunc(GPIO_PGPIO_2, GPIO_OUTPUT_MODE, 0, 2);
    Idu_DTD_BL_Off();
    #endif

}

void Idu_DTDOff(ST_TCON *pstTCON)
{
	IDU* idu = (IDU *)IDU_BASE;
	GPIO *gpio = (GPIO *)GPIO_BASE;

	if (pstTCON->bBusType == DISP_TYPE_RGB_A)
		idu->ATCON_DAC_ctrl = 0x000072ff;
	else
	{

		idu->ATCON_DAC_ctrl = 0x000070fc;
	}
}

#endif

void Idu_RGB24ModeInit(void)
{
#if New_RGB24
	IDU* idu = (IDU *)(IDU_BASE);

	idu->IduCtrl0 |= 0x08200000; //RGB mode, RGB888 enable
   	idu->IduCtrl0 &= ~0x02800000; //disable ITU656, 444 mode
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
   	idu->Gpoctrl1 &= ~0x80000000; //disable GPO model
#endif
#endif
}

///
///@ingroup IMAGE_WIN
///@brief Setting TV control.
///
///@param ST_TCON *pstTCON : The TCON to be used
///
///@retval NONE
///
///@remark This function will set the bus between MP612 and TCON. For digital signal, the TVCtrl0/1
///        will be set here.
///
static void Idu_BusInit(ST_TCON *pstTCON)
{
    register IDU *idu = (IDU *) (IDU_BASE);
    register GPIO *gpio = (GPIO *) (GPIO_BASE);
    mpDebugPrint("Idu_BusInit pstTCON->bBusType=%d",pstTCON->bBusType);

    // New section replace below old section, From MP600  Griffy ++
    if(pstTCON->bBusType == DISP_TYPE_CCIR601 ||pstTCON->bBusType == DISP_TYPE_CCIR656) //alternative function 1
	{
		gpio->Vgpcfg0 = 0x0000ffff;
		gpio->Vgpcfg1 = 0x0000ffff;
		idu->IduCtrl0 |= 0x00030000; //set Hsync/Vsync active low

		if (pstTCON->bBusType == DISP_TYPE_CCIR656)
			idu->IduCtrl0 |= 0x00800000; //sel_656
	}
	else if(pstTCON->bBusType == DISP_TYPE_RGB24 ||
			pstTCON->bBusType == DISP_TYPE_DVI_888 || pstTCON->bBusType == DISP_TYPE_RGB18 ||
			pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_666 || pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_888 ||
			pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666 || pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_888)
	{
		#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))  //set alternative function 1
			gpio->Vgpcfg0 = 0x0000ffff;	//gpio select IDU function
			gpio->Vgpcfg1 = 0x0000ffff;	//gpio select IDU function
			idu->IduCtrl0 |= 0x00030000;   //Hsync/Vsync active low
			idu->IduCtrl0 |= 0x00400000;
			if(pstTCON->bBusType != DISP_TYPE_RGB18)
				idu->IduCtrl0 &= ~0x00400000;	//disable dithering
		#else //mp612/615 //alternative function 3
			gpio->Vgpcfg0 = 0xffffffff;
			gpio->Vgpcfg1 = 0xffffffff;
			gpio->Gpcfg0 |= 0xff00ff00;	//GPIO b8~15 , alternative function 3
			gpio->Gpdat0 |= 0xff000000;    //GPIO b8~15, output low
			idu->TvCtrl0 |= 0x10000039; //b0~b7, basically 0x09, 0x0A, 0x0B, 0x0D, 0x0D, 0x0F are ok, just RGB output
		#endif
	}
	else if(pstTCON->bBusType == DISP_TYPE_DVI_666) //alternative function 2
    {
		#if(((CHIP_VER & 0xffff0000) == CHIP_VER_600) || ((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
			gpio->Vgpcfg0 = 0xffff0000;
			gpio->Vgpcfg1 = 0xffff0000;
			idu->IduCtrl1 |= 0x00080000;  	//656CLK_INV
			idu->IduCtrl0 |= 0x00230010;  	//RGB,H_INV,V_INV,threshold = 10
			#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
				 idu->IduCtrl0 |=0x00400000;
			#endif
		#else //mp612/mp615
			gpio->Vgpcfg0 = 0xffffffff;
			gpio->Vgpcfg1 = 0xffffffff;
			gpio->Gpcfg0 |= 0xfc00fc00;	//GPIO b10~15 , alternative function 3
			gpio->Gpdat0 |= 0xfc000000; 	//GPIO b10~15, output low
			idu->TvCtrl0 |= 0x10000039; //b0~b7, basically 0x09, 0x0A, 0x0B, 0x0D, 0x0D, 0x0F are ok, just RGB output
		#endif
	  }
	else if(pstTCON->bBusType == DISP_TYPE_RGB8_SERIAL)
	{
		gpio->Vgpcfg0 = (gpio->Vgpcfg0 & 0xFF00FF00) | 0x000000ff;	//gpio 0~7 select IDU function I
		gpio->Vgpcfg1 = (gpio->Vgpcfg1 & 0xFF87FF87) | 0x00000078;	//gpio select IDU function
		idu->IduCtrl0 |= 0x00030000;   //Hsync/Vsync active low
		idu->IduCtrl0 &= ~0x00400000;	//disable dithering
	}
    else if(pstTCON->bBusType == DISP_TYPE_ITU709)
	{
		gpio->Vgpcfg0 =  0x0000ffff;	//gpio select IDU function
		gpio->Vgpcfg1 = (gpio->Vgpcfg1 & 0xFF87FF87) | 0x00000078;	//gpio select IDU function
		idu->IduCtrl0 |= 0x00030000;   //Hsync/Vsync active low
		idu->IduCtrl0 &= ~0x00400000;	//disable dithering
	}


	// MP66x no support LVDS
	//#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	#if ((CHIP_VER & 0xffff0000) == CHIP_VER_650)
		if(pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_666 ||
		   pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_888 ||
		   pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666 ||
		   pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_888)
	    {
			MP_DEBUG("Set LVDS BUS");
			// 666 or 888
			if(pstTCON->bBusType == DISP_TYPE_LVDS_SINGLE_666 || pstTCON->bBusType == DISP_TYPE_LVDS_DOUBLE_666)
			{
				idu->ATCON_srgb_ctrl |= 0x00100000 ;
				idu->IduCtrl0 |= 0x00400000;
			}
			else
				idu->ATCON_srgb_ctrl &= ~0x00100000 ;

			// All use 2 channel
			idu->ATCON_srgb_ctrl |= 0x00020000 ;
			// Set full threshold
			idu->IduCtrl0 |= 0x00000018 ;
			// Set DMA request limit
			MP650_Set_DMA_Request_Limit();
			// LVDS enable
			idu->ATCON_srgb_ctrl &= ~0x00010000 ;
			IODelay(3);
			idu->ATCON_srgb_ctrl |= 0x00010000 ;
		}
		else
			idu->ATCON_srgb_ctrl &= ~0x00010000 ;
	#endif

    if (Idu_ChkBusWidth() == 0)    // Analog output
        Idu_TVCtrlInit (pstTCON);

    Idu_ColorConv(pstTCON);

    if (pstTCON->bInterlace == INTERLACE)
        idu->IduCtrl0 |= 0x00008000;
    else
        idu->IduCtrl0 &= (~0x00008000);
}

///
///@ingroup IMAGE_WIN
///@brief Call the specific TCON init funtion
///
///@param BYTE bId : The Id of TCON be used
///
///@retval NONE
///
///@remark Call TCON initial function here
///
static void Idu_TCONInit(BYTE bId)
{
	register IDU *idu = (IDU *) IDU_BASE;

    //fengrs 02/09 if tcon is MX88V44 or HX8817 need modify the makefile
#if (TCON_ID == TCON_MX88V44)
    	MX88V44Init();
#endif
#if (TCON_ID == TCON_HX8817A)
    	HIMAX_Init();
#endif
#if ((TCON_ID == TCON_MX88V430) ||(TCON_ID == TCON_MX88V430_WQVGA))
        MX88V430Init();
#endif
#if ((TCON_ID == TCON_MX88V431) ||(TCON_ID == TCON_MX88V431_CCIR601) ||(TCON_ID == TCON_MX88V431_WQVGA))
        //MX88V431Init();
#endif
#if (TCON_ID == TCON_ARK1829)
        ARK1829Init();
#endif
//below should be only used by MP600 ++
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_600 || (CHIP_VER & 0xffff0000) == CHIP_VER_650 || (CHIP_VER & 0xffff0000) == CHIP_VER_660)
#if ((TCON_ID == TCON_INTERNAL) || (TCON_ID == TCON_INTERNAL_800x600) || (TCON_ID == TCON_INTERNAL_800x480)||(TCON_ID == TCON_INTERNAL_480x640))
    	Idu_InternelTCONInit();
#endif
#if ((TCON_ID == TCON_RGB24Mode_320x240) ||\
     (TCON_ID == TCON_RGB24Mode_240x400) ||\
     (TCON_ID == TCON_RGB24Mode_640x480) ||\
     (TCON_ID == TCON_RGB24Mode_800x600_3D) ||\
     (TCON_ID == TCON_RGB24Mode_1024x768) ||\
     (TCON_ID == TCON_RGB24Mode_1024x600) ||\
     (TCON_ID == TCON_RGB24Mode_1280x800) ||\
     (TCON_ID == TCON_RGB24Mode_800x600) ||\ 
     (TCON_ID == TCON_RGB24Mode_800x480) ||\
     (TCON_ID == TCON_RGB24Mode_480x800) ||\
     (TCON_ID == TCON_LVDSMode_1024x600) ||\
     (TCON_ID == TCON_LVDSMode_1280x1024)||\
     (TCON_ID == TCON_RGB24Mode_1280x720)||\
     (TCON_ID == TCON_LVDSMode_1680x1050)||\ 
     (TCON_ID == TCON_LVDSMode_1366x768)||\
     (TCON_ID == TCON_LVDSMode_1366x768_CPT)||\
     (TCON_ID == TCON_LVDSMode_1440x900)||\
     (TCON_ID == TCON_RGB24Mode_FUJI_320x240) )
	{
		Idu_RGB24ModeInit();
		if(TCON_ID == TCON_RGB24Mode_240x400)
			 	ILI9327_Init();
		else if(TCON_ID == TCON_RGB24Mode_FUJI_320x240)
			    NOVA_Init();
		else
			;
	}
#endif

#if ((TCON_ID == TCON_I80Mode_128x128))

        idu->IduCtrl0 |= 0x00030000;   //Hsync/Vsync active low
		idu->IduCtrl0 |= 0x00400000;

		Idu_RGB24ModeInit();
			ILI9163_Init();
#endif

#if (PANEL_ID == PANEL_ADV7393)
    Idu_ADV7393Init();
#endif

	#if (PANEL_ID == PANEL_HDMI_720P)
    /*
	//mpDebugPrint("0x42=%x",I2CM_RdReg8Data8(0x72, 0x42));
	if(I2CM_RdReg8Data8(0x72, 0x42)==0xe0)
	{
	I2CM_WtReg8Data8(0x72, 0x41, 0x00);
	I2CM_WtReg8Data8(0x72, 0x98, 0x07);
	I2CM_WtReg8Data8(0x72, 0x9C, 0x38);
	I2CM_WtReg8Data8(0x72, 0x9D, 0x61);
	I2CM_WtReg8Data8(0x72, 0x9F, 0x70);
	I2CM_WtReg8Data8(0x72, 0xBB, 0xFF);
	I2CM_WtReg8Data8(0x72, 0xA2, 0x94);
	I2CM_WtReg8Data8(0x72, 0xA3, 0x94);
	I2CM_WtReg8Data8(0x72, 0xDE, 0x10);
	I2CM_WtReg8Data8(0x72, 0x15, 0x00);
	I2CM_WtReg8Data8(0x72, 0x16, 0x00);
	I2CM_WtReg8Data8(0x72, 0xD5, 0x00);
	I2CM_WtReg8Data8(0x72, 0xBA, 0xE0);//invert clk
	I2CM_WtReg8Data8(0x72, 0xD0, 0x30);
	I2CM_WtReg8Data8(0x72, 0xAF, 0x16);
	I2CM_WtReg8Data8(0x72, 0x01, 0x00);
	I2CM_WtReg8Data8(0x72, 0x02, 0x18);
	I2CM_WtReg8Data8(0x72, 0x03, 0x80);

	//I2CM_WtReg8Data8(0x72, 0x0A, 0x41);
	I2CM_WtReg8Data8(0x72, 0x0A, 0x40);
	I2CM_WtReg8Data8(0x72, 0x0c, 0x85);
	I2CM_WtReg8Data8(0x72, 0x0D, 0x10);
	I2CM_WtReg8Data8(0x72, 0x17, 0x02);

	I2CM_WtReg8Data8(0x72, 0x35, 0x41);
	I2CM_WtReg8Data8(0x72, 0x36, 0xD9);
	I2CM_WtReg8Data8(0x72, 0x37, 0x0A);
	I2CM_WtReg8Data8(0x72, 0x38, 0x00);
	I2CM_WtReg8Data8(0x72, 0x39, 0x2D);
	I2CM_WtReg8Data8(0x72, 0x3A, 0x00);

	}
	idu->IduCtrl0 &= 0xfffcffff;   //Hsync/Vsync active high
       */
    AD9889B_init();   
	#endif

#endif

#if (TCON_ID == TCON_RGB8SerialMode_320x240 || TCON_ID == TCON_RGB8SerialMode_480x240 || TCON_ID == TCON_RGB8SerialMode_320x240_ILI9342)

	Serial_RGB_Init();
	IODelay(100);
	#if (TCON_ID == TCON_RGB8SerialMode_320x240)
	AUO_2_5inchInit();
	#elif (TCON_ID == TCON_RGB8SerialMode_480x240)
	ORISE_Init();
	#else
	ILI9342_Init();
	#endif
	
#endif

}
///
///@ingroup IMAGE_WIN
///@brief Initial IDU scaling
///
///@param WORD wInnerWidth : Width of IDU module (win)
///       WORD wInnerHeight : Height of IDU module (win)
///       WORD wOuterWidth : Width of signal to TCON
///       WORD wOuterHeight : Height of signal to TCON
///
///@retval NONE
///
///@remark OSD won't be scaling. If output device is TV, only scaling down is availible.
///
static void Idu_ScaleInit(WORD wInnerWidth, WORD wInnerHeight, WORD wOuterWidth, WORD wOuterHeight)
{
	register IDU *idu = (IDU *) IDU_BASE;
	DWORD dw_HSF, dw_VSF, dw_HM, dw_VM;
	BYTE b_HN;

	//Horizontal
	if (wInnerWidth != wOuterWidth)
	{
		if (wInnerWidth > wOuterWidth)
		{						//Scaling Down
			dw_HM = wInnerWidth - wOuterWidth;
			b_HN = wInnerWidth / wOuterWidth;
			dw_HSF = (((wInnerWidth - 1) * 256) / ((wOuterWidth - 1) * b_HN)) - 256;
			dw_HSF++;
			if (b_HN)
				b_HN--;
			idu->IduCtrl0 |= 0x00000020;	//Gated Clock
		}
		else
		{						//Scaling Up
			b_HN = 0;
			dw_HM = wOuterWidth - wInnerWidth;
			dw_HSF = (((wInnerWidth - 1) * 256 + 1) / (wOuterWidth - 1)) & 0xff;
			idu->IduCtrl0 |= 0x00000100;	//H Scale Up
		}

		idu->HScaleCtrl0 = (dw_HM << 16) | (dw_HSF << 8) | b_HN;
	}

	//Vertical
	if (wInnerHeight != wOuterHeight)
	{
		if (wInnerHeight > wOuterHeight)
		{						//Scaling Down
			dw_VM = wInnerHeight - wOuterHeight;
			dw_VSF = (((wInnerHeight - 1) * 256) / (wOuterHeight - 1)) - 256;
			dw_VSF++;
		}
		else
		{						//Scaling Up
			dw_VM = wOuterHeight - wInnerHeight;
			dw_VSF = (wInnerHeight - 1) * 256 / (wOuterHeight - 1);
			idu->IduCtrl0 |= 0x00000200;	//V Scale Up
		}

		idu->VScaleCtrl0 = (dw_VM << 16) | (dw_VSF << 8);
	}
}
///
///@ingroup IMAGE_WIN
///@brief Init IDU module
///
///@param ST_TCON *pstTCON : TCON to be used.
///
///@retval NONE
///
///@remark The function call will initial IDU module for specific TCON.
///        It also initial three ImgWin for frame buffer and one OsdWin for OSD buffer.
//         The IDU wins are initial as inner size, and the OSD win is  initial as outer size
///         because it couldn't be scaling.
///
BYTE bIduInit = 0;
void IduInit()
{
    if(bIduInit)
        return; // had done already! 
   bIduInit = 1; // just do once

        
	EventCreate(IDU_EVENT_ID, OS_ATTR_PRIORITY | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR, 0);
	SemaphoreCreate(IDU_SEMA_ID, OS_ATTR_FIFO, 1);
	
	#if (IDU_CHANGEWIN_MODE == 1)
	SemaphoreCreate(IDU_WAIT_BE_SEMA_ID, OS_ATTR_PRIORITY, 2);
	#endif
}

#ifdef DEMO_PID
static void Idu_IduInit(ST_TCON *pstTCON,BYTE c)
{
    IduInit();
    
	DMA *tmpDma = (DMA *) DMA_BASE;
	CLOCK *clock = (CLOCK *) CLOCK_BASE;
	register IDU *idu = (IDU *) IDU_BASE;
	register BIU *biu = (BIU *) BIU_BASE;
	struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

	Idu_DisplayOff();			// disable idu and osd dma
	if (pstTCON->bId == TCON_NONE)
		return;
	if(c)
	{
		//Asynchronous reset for IDU module
		mpDebugPrint("Asynchronous reset for IDU module");
		biu->BiuArst &= 0xfffffff7;
		biu->BiuArst |= 0x00000008;
	}
	
	InitWins(pstScreen->wInnerWidth, pstScreen->wInnerHeight);
	Idu_WinDmaInit(Idu_GetCurrWin(), pstTCON->bInterlace);	//set DMA
	Idu_ClockInit(pstTCON); //dot clock griffy
	Idu_SetTVSync(pstTCON);
	Idu_BusInit(pstTCON);
	Idu_ScaleInit(pstScreen->wInnerWidth, pstScreen->wInnerHeight, pstTCON->wWidth, pstTCON->wHeight);
	//Jessamine 20070320, setup brighterness/gamma need to modify
	//Idu_InitGamma ();   //Mason 20060831
	if (c)
	{
		Idu_PaintWin(psCurrWin, RGB2YUV(9, 53, 88));
	}
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	Idu_SetCSC();
#endif
	ImgDisplayON(); 	//idu and osd and their dma enable
	
	Idu_TCONInit(pstTCON->bId);
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
#if(INTERNAL_DTD)
	Idu_DTDInit(pstTCON);
#else
	Idu_DTDOff(pstTCON);
#endif
#endif
	
#if OSD_ENABLE
	Idu_OSDInit((DWORD *) SystemGetMemAddr(OSD_BUF_MEM_ID), ALIGN_CUT_16(pstTCON->wWidth), pstTCON->wHeight, pstTCON->bInterlace, OSD_BIT_WIDTH,1);
#endif

#if (CHIP_VER == (CHIP_VER_612 | CHIP_VER_C))
	tmpDma->SdramCtl = 0x7f10901c;	//SDRAM timing, for C ver.
#elif ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
	tmpDma->SdramCtl &= 0xffcfffff;
	tmpDma->SdramCtl |= 0x00100000;
#endif
	
	
//UartOutText("Paint V_ColorBar!! ***\r\n");
//PaintVColorBar();
#if (PANEL_ID == PANEL_Q08009)
idu->IduCtrl0 &= ~0x00020000 ;
#endif
	
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
//idu->IduCtrl0 |= 0x00100000 ;
//modify IDU FIFO Request threshold to 56
idu->IduCtrl0 |= 0x00100018 ;
#if (PANEL_ID == PANEL_HDMI_720P)
idu->IduCtrl0 &=~0x00100000 ;
#endif
#endif
	
#ifdef Support_EPD
	idu->IduSts |= BIT31 ;
	while(!(idu->IduSts & BIT27));
#endif
	
#ifdef IDU_DEBUG
GPIO *gpio = (GPIO *)(GPIO_BASE);
	
//bus init
mpDebugPrint("\r\ngpio->Vgpcfg0=0x%08x, gpio->Vgpcfg1=0x%08x", gpio->Vgpcfg0, gpio->Vgpcfg1);
mpDebugPrint("gpio->Gpcfg0=0x%08x, gpio->Gpdat0=0x%08x", gpio->Gpcfg0, gpio->Gpdat0);
mpDebugPrint("\r\nidu->IduCtrl0=0x%08x, idu->IduCtrl1=0x%08x", idu->IduCtrl0, idu->IduCtrl1);
mpDebugPrint("idu->TvCtrl0=0x%08x, idu->TvCtrl1=0x%08x", idu->TvCtrl0, idu->TvCtrl1);
mpDebugPrint("clock->Clkss1=0x%08x", clock->Clkss1);
	
//TV sync
mpDebugPrint("\r\nidu->TvHCtrl0=0x%08x, idu->TvHCtrl1=0x%08x\r\nidu->TvVCtrl0=0x%08x, idu->TvVCtrl1=0x%08x",
	idu->TvHCtrl0, idu->TvHCtrl1, idu->TvVCtrl0, idu->TvVCtrl1);
	
//OSD
mpDebugPrint("\r\nidu->OsdHStr=0x%08x, idu->OsdHEnd=0x%08x, idu->OsdVStr=0x%08x, idu->OsdVEnd=0x%08x",
	idu->OsdHStr, idu->OsdHEnd, idu->OsdVStr, idu->OsdVEnd);
	
//color
mpDebugPrint("\r\nidu->CGainCtrl=0x%08x\r\nidu->YGammaR[0]=0x%08x, idu->YGammaR[1]=0x%08x, idu->YGammaR[2]=0x%08x, idu->YGamma[3]=0x%08x",
	idu->CGainCtrl, idu->YGammaR[0], idu->YGammaR[1], idu->YGammaR[2], idu->YGammaR[3]);
//mpDebugPrint("idu->TvCGainCtrl=0x%08x\r\nidu->TvYGammaR[0]=0x%08x, idu->TvYGammaR[1]=0x%08x, idu->TvYGammaR[2]=0x%08x, idu->TvYGamma[3]=0x%08x",
//	  idu->TvCGainCtrl, idu->TvYGammaR[0], idu->TvYGammaR[1], idu->TvYGammaR[2], idu->TvYGammaR[3]);
mpDebugPrint("idu->Iducsc0=0x%08x  idu->Iducsc1=0x%08x	idu->Iducsc2=0x%08x  idu->Iducsc3=0x%08x",
	idu->Iducsc0, idu->Iducsc1, idu->Iducsc2, idu->Iducsc3);
	
//internal init
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
mpDebugPrint("\r\nidu->Gpov0=%08x, idu->Gpoh0=%08x", idu->Gpov0, idu->Gpoh0);
mpDebugPrint("idu->Gpov1=%08x, idu->Gpoh1=%08x", idu->Gpov1, idu->Gpoh1);
mpDebugPrint("idu->Gpov2=%08x, idu->Gpoh2=%08x", idu->Gpov2, idu->Gpoh2);
mpDebugPrint("idu->Gpov3=%08x, idu->Gpoh3=%08x", idu->Gpov3, idu->Gpoh3);
mpDebugPrint("idu->Gpov4=%08x, idu->Gpoh4=%08x", idu->Gpov4, idu->Gpoh4);
mpDebugPrint("idu->Gpov5=%08x, idu->Gpoh5=%08x", idu->Gpov5, idu->Gpoh5);
mpDebugPrint("idu->Gpov6=%08x, idu->Gpoh6=%08x", idu->Gpov6, idu->Gpoh6);
mpDebugPrint("idu->Gpov7=%08x, idu->Gpoh7=%08x", idu->Gpov7, idu->Gpoh7);
mpDebugPrint("idu->Gpov8=%08x, idu->Gpoh8=%08x", idu->Gpov8, idu->Gpoh8);
mpDebugPrint("idu->Gpov9=%08x, idu->Gpoh9=%08x", idu->Gpov9, idu->Gpoh9);
mpDebugPrint("idu->Gpoctrl0=%08x, idu->Gpoctrl1=%08x", idu->Gpoctrl0, idu->Gpoctrl1);
mpDebugPrint("clock->PLLIduCfg=0x%08x", clock->PLLIduCfg);
mpDebugPrint("clock->MdClken=0x%08x", clock->MdClken);
mpDebugPrint("idu->ATCON_srgb_ctrl=0x%08x", idu->ATCON_srgb_ctrl);
mpDebugPrint("tmpDma->FDMACTL_EXT0=0x%08x", tmpDma->FDMACTL_EXT0);
mpDebugPrint("tmpDma->FDMACTL_EXT1=0x%08x", tmpDma->FDMACTL_EXT1);
#endif
	
#endif
}

#else
static void Idu_IduInit(ST_TCON *pstTCON)
{
    MP_DEBUG("%s", __func__);
    IduInit();
    
	DMA *tmpDma = (DMA *) DMA_BASE;
	CLOCK *clock = (CLOCK *) CLOCK_BASE;
	register IDU *idu = (IDU *) IDU_BASE;
	register BIU *biu = (BIU *) BIU_BASE;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
    int i;

	Idu_DisplayOff();			// disable idu and osd dma
    if (pstTCON->bId == TCON_NONE)
        return;
	//Asynchronous reset for IDU module
	mpDebugPrint("Asynchronous reset for IDU module");
	biu->BiuArst &= 0xfffffff7;
	biu->BiuArst |= 0x00000008;

    //InitWins(pstScreen->wInnerWidth, pstScreen->wInnerHeight);
	InitWins(pstScreen->wInnerWidth, pstScreen->wInnerHeight);
	Idu_WinDmaInit(Idu_GetCurrWin(), pstTCON->bInterlace);	//set DMA
	Idu_ClockInit(pstTCON); //dot clock griffy
    Idu_SetTVSync(pstTCON);
    Idu_BusInit(pstTCON);
    Idu_ScaleInit(pstScreen->wInnerWidth, pstScreen->wInnerHeight, pstTCON->wWidth, pstTCON->wHeight);
    //Jessamine 20070320, setup brighterness/gamma need to modify
    //Idu_InitGamma ();   //Mason 20060831
	Idu_PaintWin(psCurrWin, RGB2YUV(9, 53, 88));
	

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	Idu_SetCSC();
#endif
    ImgDisplayON();     //idu and osd and their dma enable
  
    Idu_TCONInit(pstTCON->bId);
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
#if(INTERNAL_DTD)
	Idu_DTDInit(pstTCON);
#else
	Idu_DTDOff(pstTCON);
#endif
#endif


#if OSD_ENABLE
      Idu_OSDInit((DWORD *) SystemGetMemAddr(OSD_BUF_MEM_ID),
      ALIGN_CUT_16(pstTCON->wWidth), pstTCON->wHeight, pstTCON->bInterlace, OSD_BIT_WIDTH,1);
  #endif


#if (CHIP_VER == (CHIP_VER_612 | CHIP_VER_C))
	tmpDma->SdramCtl = 0x7f10901c;	//SDRAM timing, for C ver.
#elif ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
	tmpDma->SdramCtl &= 0xffcfffff;
	tmpDma->SdramCtl |= 0x00100000;
#endif

#if (PANEL_ID == PANEL_Q08009)
idu->IduCtrl0 &= ~0x00020000 ;
#endif

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
//idu->IduCtrl0 |= 0x00100000 ;
//modify IDU FIFO Request threshold to 56
#if (STD_BOARD_VER == MP652_216LQFP_4LAYER)
idu->IduCtrl0 |= 0x00000018 ;
#else
idu->IduCtrl0 |= 0x00100018 ; 
#endif
#if ((PANEL_ID == PANEL_HDMI_720P)||(PANEL_ID == PANEL_ADV7393)||(IDU_CLKINVT==1))
    idu->IduCtrl0 &=~0x00100000 ;
#endif
#endif

#ifdef Support_EPD
    idu->IduSts |= BIT31 ;
    while(!(idu->IduSts & BIT27));
#endif

//mpDebugPrint("idu->Mpu_Cmd = 0x%x",&idu->Mpu_Cmd);
//mpDebugPrint("idu->Mpu_Par = 0x%x",&idu->Mpu_Par);
//mpDebugPrint("idu->Mpu_Ctrl = 0x%x",&idu->Mpu_Ctrl);
//mpDebugPrint("idu->Palette = 0x%x",&idu->Palette);

#if(PANEL_ID == PANEL_ILI9163C)
idu->IduCtrl0 &= ~BIT20 ; //--for I80 test
ILI9163_WRITE_FRAME();
#endif

mpDebugPrint("idu->IduCtrl0 = 0x%x", idu->IduCtrl0);
mpDebugPrint("idu->ATCON_srgb_ctrl = 0x%x", idu->ATCON_srgb_ctrl);




#ifdef IDU_DEBUG
GPIO *gpio = (GPIO *)(GPIO_BASE);

//bus init
mpDebugPrint("\r\ngpio->Vgpcfg0=0x%08x, gpio->Vgpcfg1=0x%08x", gpio->Vgpcfg0, gpio->Vgpcfg1);
mpDebugPrint("gpio->Gpcfg0=0x%08x, gpio->Gpdat0=0x%08x", gpio->Gpcfg0, gpio->Gpdat0);
mpDebugPrint("\r\nidu->IduCtrl0=0x%08x, idu->IduCtrl1=0x%08x", idu->IduCtrl0, idu->IduCtrl1);
mpDebugPrint("idu->TvCtrl0=0x%08x, idu->TvCtrl1=0x%08x", idu->TvCtrl0, idu->TvCtrl1);
mpDebugPrint("clock->Clkss1=0x%08x", clock->Clkss1);

//TV sync
mpDebugPrint("\r\nidu->TvHCtrl0=0x%08x, idu->TvHCtrl1=0x%08x\r\nidu->TvVCtrl0=0x%08x, idu->TvVCtrl1=0x%08x",
    idu->TvHCtrl0, idu->TvHCtrl1, idu->TvVCtrl0, idu->TvVCtrl1);

//OSD
mpDebugPrint("\r\nidu->OsdHStr=0x%08x, idu->OsdHEnd=0x%08x, idu->OsdVStr=0x%08x, idu->OsdVEnd=0x%08x",
    idu->OsdHStr, idu->OsdHEnd, idu->OsdVStr, idu->OsdVEnd);

//color
mpDebugPrint("\r\nidu->CGainCtrl=0x%08x\r\nidu->YGammaR[0]=0x%08x, idu->YGammaR[1]=0x%08x, idu->YGammaR[2]=0x%08x, idu->YGamma[3]=0x%08x",
    idu->CGainCtrl, idu->YGammaR[0], idu->YGammaR[1], idu->YGammaR[2], idu->YGammaR[3]);
//mpDebugPrint("idu->TvCGainCtrl=0x%08x\r\nidu->TvYGammaR[0]=0x%08x, idu->TvYGammaR[1]=0x%08x, idu->TvYGammaR[2]=0x%08x, idu->TvYGamma[3]=0x%08x",
//    idu->TvCGainCtrl, idu->TvYGammaR[0], idu->TvYGammaR[1], idu->TvYGammaR[2], idu->TvYGammaR[3]);
mpDebugPrint("idu->Iducsc0=0x%08x  idu->Iducsc1=0x%08x  idu->Iducsc2=0x%08x  idu->Iducsc3=0x%08x",
    idu->Iducsc0, idu->Iducsc1, idu->Iducsc2, idu->Iducsc3);

//internal init
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
mpDebugPrint("\r\nidu->Gpov0=%08x, idu->Gpoh0=%08x", idu->Gpov0, idu->Gpoh0);
mpDebugPrint("idu->Gpov1=%08x, idu->Gpoh1=%08x", idu->Gpov1, idu->Gpoh1);
mpDebugPrint("idu->Gpov2=%08x, idu->Gpoh2=%08x", idu->Gpov2, idu->Gpoh2);
mpDebugPrint("idu->Gpov3=%08x, idu->Gpoh3=%08x", idu->Gpov3, idu->Gpoh3);
mpDebugPrint("idu->Gpov4=%08x, idu->Gpoh4=%08x", idu->Gpov4, idu->Gpoh4);
mpDebugPrint("idu->Gpov5=%08x, idu->Gpoh5=%08x", idu->Gpov5, idu->Gpoh5);
mpDebugPrint("idu->Gpov6=%08x, idu->Gpoh6=%08x", idu->Gpov6, idu->Gpoh6);
mpDebugPrint("idu->Gpov7=%08x, idu->Gpoh7=%08x", idu->Gpov7, idu->Gpoh7);
mpDebugPrint("idu->Gpov8=%08x, idu->Gpoh8=%08x", idu->Gpov8, idu->Gpoh8);
mpDebugPrint("idu->Gpov9=%08x, idu->Gpoh9=%08x", idu->Gpov9, idu->Gpoh9);
mpDebugPrint("idu->Gpoctrl0=%08x, idu->Gpoctrl1=%08x", idu->Gpoctrl0, idu->Gpoctrl1);
mpDebugPrint("clock->PLLIduCfg=0x%08x", clock->PLLIduCfg);
mpDebugPrint("clock->MdClken=0x%08x", clock->MdClken);
mpDebugPrint("idu->ATCON_srgb_ctrl=0x%08x", idu->ATCON_srgb_ctrl);
mpDebugPrint("tmpDma->FDMACTL_EXT0=0x%08x", tmpDma->FDMACTL_EXT0);
mpDebugPrint("tmpDma->FDMACTL_EXT1=0x%08x", tmpDma->FDMACTL_EXT1);
#endif

#endif
}
#endif

///
///@ingroup IMAGE_WIN
///@brief API to initial IDU module
///
///@param BYTE bInitType : Initial type - 1:Low resolution, 2:Default resolution
///
///@retval NONE
///
///@remark This funtion will looking for TCON and panel to be used according to screen table and
///        setup parameter.
///

BYTE DisplayInit(BYTE InitType)
{
    MP_DEBUG("%s: InitType = %d, g_bDisplayStatus = %d", __func__, InitType, g_bDisplayStatus);
    
    BYTE i, bChanged = 0;
    //__asm("break 100");
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
    ST_SCREEN_TABLE *pstScreenTable = &g_mstScreenTable[g_psSystemConfig->sScreenSetting.bScreenIndex];

	mpDebugPrint("****PANEL_ID %d: %s  ****",  pstScreenTable->bPanelId,PANEL_ID_NAME[pstScreenTable->bPanelId]);
	mpDebugPrint("****TCON_ID  %d: %s  ****",   pstScreenTable->bTCONId,TCON_ID_NAME[pstScreenTable->bTCONId]);
	mpDebugPrint("****IDU_CLOCK : %d ****",  pstScreen->pstCurTCON->wIduClock);

	if (g_bDisplayStatus == DISPLAY_NOT_INIT)
  {
       /* 
        //Put your U/D R/L control here
		  */  
	}
	//if(InitType == DISPLAY_INIT_DEFAULT_RESOLUTION)


	if (pstScreenTable->bLowTCONId != BYPASS && InitType == DISPLAY_INIT_LOW_RESOLUTION)
	{
	    if (pstScreen->pstCurTCON->bId != pstScreenTable->bLowTCONId || g_bDisplayStatus == DISPLAY_NOT_INIT)
        {
            bChanged = 1;
	          Idu_TableSearch(pstScreenTable->bLowTCONId, pstScreenTable->bPanelId, 0, 0);
            #ifdef DEMO_PID
            Idu_IduInit(pstScreen->pstCurTCON,bChanged);
			#else
			Idu_IduInit(pstScreen->pstCurTCON);
			#endif
        }
		#ifdef DEMO_PID
		if( bChanged)
		{
			
				Idu_IduInit(pstScreen->pstCurTCON,0);
				mpDebugPrint("Idu_IduInit\n");
	    	
		}
		#endif
#if ((CHIP_VER & 0xffff0000) != CHIP_VER_650 && (CHIP_VER & 0xffff0000) != CHIP_VER_660)
        g_psDma->BtBtc |= 0x00800000;
#endif
	     g_bDisplayStatus = DISPLAY_INIT_LOW_RESOLUTION;
	}
	else
	{
	    if (pstScreen->pstCurTCON->bId != pstScreenTable->bTCONId || g_bDisplayStatus == DISPLAY_NOT_INIT)
        {
            bChanged = 1;
            Idu_TableSearch(pstScreenTable->bTCONId, pstScreenTable->bPanelId, pstScreenTable->wInnerWidth, pstScreenTable->wInnerHeight);
			#ifdef DEMO_PID
            Idu_IduInit(pstScreen->pstCurTCON,bChanged);
			#else
			Idu_IduInit(pstScreen->pstCurTCON);
			#endif
		}
		#ifdef DEMO_PID
		  if((bChanged == 0)&&(g_bDisplayStatus = DISPLAY_INIT_LOW_RESOLUTION));
		{
			
				Idu_IduInit(pstScreen->pstCurTCON,0);
				mpDebugPrint("Idu_IduInit(else)\n");
	    	
		}
		#endif
#if ((CHIP_VER & 0xffff0000) != CHIP_VER_650 && (CHIP_VER & 0xffff0000) != CHIP_VER_660)
        g_psDma->BtBtc &= ~(0x00800000);	// recover the priority of SDRAM auto reflash
#endif
	     g_bDisplayStatus = DISPLAY_INIT_DEFAULT_RESOLUTION;
	}


#if 1 //franklin
register IDU *idu = (IDU *) (IDU_BASE);
//MP_ALERT("---------------##### idu->IduCtrl0 = 0x%x", idu->IduCtrl0);
idu->IduCtrl0 &= ~BIT20;
//MP_ALERT("---------------fff2##### idu->IduCtrl0 = 0x%x", idu->IduCtrl0);

#endif
	
    return bChanged;
}

#if TWO_SCREEN_FOR_720P
void SetDisplayStatus(BYTE bStatus)
{
	g_bDisplayStatus = bStatus;
}
#endif

#if (IDU_CHANGEWIN_MODE == 0)
void DmaIduIsr()
{
	CHANNEL *iduDmaReg = (CHANNEL *) DMA_IDU_BASE;
  BOOL bufferEndA = 0;
  register IDU *idu = (IDU *)IDU_BASE;
    //MP_DEBUG("DmaIduIsr"); // log too many


  #if 0 //(PANEL_ID == PANEL_HDMI_720P)
  if(I2CM_RdReg8Data8(0x72, 0x41)==0x50)
  {
      mpDebugPrint("AD9889B sleep reinit");
      AD9889B_init();
  }
  #endif
  
  if (iduDmaReg->Control & BIT16)
  {
  	volatile ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;

		iduDmaReg->Control &= ~BIT16;	            // clear IDU buffer end interrupt

    if ((iduDmaReg->Control & BIT15) == 0)      // current operating on buffer A
    {   // mean buffer B's buffer end
        bufferEndA = FALSE;
        EventSet(IDU_EVENT_ID, IDU_FRAME_END_B);
        //UartOutText("B");
		}
    else
    {   // mean buffer A's buffer end
        bufferEndA = TRUE;
        EventSet(IDU_EVENT_ID, IDU_FRAME_END_A);
        //UartOutText("A");
    }

    if (needchangeWin)
    {
    		DWORD bufPixelCnt = nextImgWinPtr->wWidth * nextImgWinPtr->wHeight;

       	if (pstTCON->bInterlace == INTERLACE)
        {   // Frame width is (2 * Width)
        	  if (bufferEndA)
            {   // Change buffer A
                if ( iduDmaReg->StartA != (0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart) )
                {
                		//UartOutText("IA ");
                		iduDmaReg->StartA = (DWORD) nextImgWinPtr->pdwStart;
                    // End address is Start + (Width * (Height - 1) * 2) - 1
                    // line 1, 3, 5, ...., Height-1
                		iduDmaReg->EndA = iduDmaReg->StartA + ((bufPixelCnt - nextImgWinPtr->wWidth) << 1) - 1;
                }
            }
            else if (iduDmaReg->StartA == (0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart))
                {   // Change buffer B and must waiting for bufferA changed.
                    #if ANTIVIBR_ENABLE
                    if (iduDmaReg->StartB != iduDmaReg->StartA)
                    {
                        //UartOutText("IB ");

                        // even frame only mode, drop odd frame
                        iduDmaReg->StartB = iduDmaReg->StartA;
                		iduDmaReg->EndB = iduDmaReg->EndA;
                    }
                    #else
                    if ( iduDmaReg->StartB != (iduDmaReg->StartA + (nextImgWinPtr->wWidth << 1)) )
                    {
                        //UartOutText("IB ");

                        // B start address is A start address + Width
                        // line 2, 4, 6, ...., Height
                		iduDmaReg->StartB = iduDmaReg->StartA + (nextImgWinPtr->wWidth << 1);
                		iduDmaReg->EndB = iduDmaReg->EndA + (nextImgWinPtr->wWidth << 1);
                    }
                    #endif
                }

                // Check buffer A/B changed
                if ( (iduDmaReg->StartA == (0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart)) &&
                #if ANTIVIBR_ENABLE
                     (iduDmaReg->StartB == iduDmaReg->StartA) )
                #else
                     (iduDmaReg->StartB == (0x0FFFFFFF & (iduDmaReg->StartA + (nextImgWinPtr->wWidth << 1)))) )
                #endif
                {
                    //UartOutText("iR ");

                    SemaphoreRelease(IDU_SEMA_ID);
                    needchangeWin = FALSE;
                }
        }
       	else
       	{
    				if(bufferEndA)
    				{   // Change buffer A
                if (iduDmaReg->StartA != (0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart))
                {
                    #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		            if(g_IDU_444_422_Flag)	// 444 mode
		            {
			            MP_DEBUG("IDU DMA 444 mode 1");
                        if((idu->RST_Ctrl_Bypass & BIT31) == 0)
                        {
                            while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
                            idu->RST_Ctrl_Bypass = 0x8000aa02 ;
		                    iduDmaReg->LineCount = (0 << 16) + (nextImgWinPtr->wWidth * 3) - 1;
                         }
                        iduDmaReg->StartA = (DWORD) nextImgWinPtr->pdwStart;
        		        iduDmaReg->EndA = (DWORD) nextImgWinPtr->pdwStart + (bufPixelCnt *3) - 1;
                    }
		            else
                    #endif
                    {
                        if((idu->RST_Ctrl_Bypass & BIT31) != 0)
                        {
                            while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
                            //idu->RST_Ctrl_Bypass &= 0x7fffffff ;
			                idu->RST_Ctrl_Bypass &= 0x7fff55ff ;
		                    iduDmaReg->LineCount = (0 << 16) + (nextImgWinPtr->wWidth * 2) - 1;
                        }
        					iduDmaReg->StartA = (DWORD) nextImgWinPtr->pdwStart;
        					iduDmaReg->EndA = (DWORD) nextImgWinPtr->pdwStart + (bufPixelCnt << 1) - 1;
                    }
                  //mpDebugPrint(" A 0x%X, 0x%X", iduDmaReg->StartA, nextImgWinPtr->pdwStart);
                  //UartOutText("A");
                }
            }
            else
            {   // Change buffer B
                #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		        if(g_IDU_444_422_Flag)	// 444 mode
		        {
			        MP_DEBUG("IDU DMA 444 mode 1");
                    if((idu->RST_Ctrl_Bypass & BIT31) == 0)
                    {
                        while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
                        idu->RST_Ctrl_Bypass = 0x8000aa02 ;
		                iduDmaReg->LineCount = (0 << 16) + (nextImgWinPtr->wWidth * 3) - 1;
                     }
        			iduDmaReg->StartB = (DWORD) nextImgWinPtr->pdwStart;
        			iduDmaReg->EndB = (DWORD) nextImgWinPtr->pdwStart + (bufPixelCnt * 3) - 1;
                }
		        else
                #endif
                {
                     if((idu->RST_Ctrl_Bypass & BIT31) != 0)
                     {
                        while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
                         //idu->RST_Ctrl_Bypass &= 0x7fffffff ;
			             idu->RST_Ctrl_Bypass &= 0x7fff55ff ;
		                 iduDmaReg->LineCount = (0 << 16) + (nextImgWinPtr->wWidth * 2) - 1;
                     }
                if (iduDmaReg->StartB != (0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart))
                {
        					iduDmaReg->StartB = (DWORD) nextImgWinPtr->pdwStart;
        					iduDmaReg->EndB = (DWORD) nextImgWinPtr->pdwStart + (bufPixelCnt << 1) - 1;
                  //mpDebugPrint(" B 0x%X, 0x%X", iduDmaReg->StartB, nextImgWinPtr->pdwStart);
                  //UartOutText("B ");
                }
    				}
    	   }

            if ((iduDmaReg->StartA == (0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart)) &&
                (iduDmaReg->StartB == iduDmaReg->StartA) )
            {
                    //UartOutText("R");

                EventSet(IDU_EVENT_ID, IDU_CHANGE_WIN_FINISHED);
                SemaphoreRelease(IDU_SEMA_ID);
                needchangeWin = FALSE;
               	iduDmaReg->Control &= ~BIT24;       // Disable IDU DAM buffer end interrupt
            }
        	}
        }
    }
}
#else
void DmaIduIsr()
{
    CHANNEL *iduDmaReg = (CHANNEL *) DMA_IDU_BASE;
    BOOL bufferEndA = 0;
    BYTE i;

    MP_DEBUG("DmaIduIsr");
    #if 0 //(PANEL_ID == PANEL_HDMI_720P)
    if(I2CM_RdReg8Data8(0x72, 0x41)==0x50)
    {
        mpDebugPrint("AD9889B sleep reinit");
        AD9889B_init();
    }
    #endif

    if (iduDmaReg->Control & BIT16)
    {
        volatile ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;

        iduDmaReg->Control &= ~BIT16;               // clear IDU buffer end cause

        if ((iduDmaReg->Control & BIT15) == 0)      // current operating on buffer A
        {   // mean buffer B's buffer end
            bufferEndA = FALSE;
            EventSet(IDU_EVENT_ID, IDU_FRAME_END_B | IDU_FRAME_END_B2);
            //UartOutText("B");
        }
        else
        {   // mean buffer A's buffer end
            bufferEndA = TRUE;
            EventSet(IDU_EVENT_ID, IDU_FRAME_END_A | IDU_FRAME_END_A2);
            //UartOutText("A");
        }

        if (needchangeWin)
        {
            DWORD bufPixelCnt = nextImgWinPtr->wWidth * nextImgWinPtr->wHeight;
            DWORD newWinAddr = 0x0FFFFFFF & (DWORD) nextImgWinPtr->pdwStart;

            if (pstTCON->bInterlace == INTERLACE)
            {   // Frame width is (2 * Width)
                if (bufferEndA)
                {   // Change buffer A
                    if ( iduDmaReg->StartA != newWinAddr )
                    {
                        //UartOutText("IA ");
                        iduDmaReg->StartA = (DWORD) nextImgWinPtr->pdwStart;
                        // End address is Start + (Width * (Height - 1) * 2) - 1
                        // line 1, 3, 5, ...., Height-1
                        iduDmaReg->EndA = iduDmaReg->StartA + ((bufPixelCnt - nextImgWinPtr->wWidth) << 1) - 1;
                    }
                }
                else if (iduDmaReg->StartA == newWinAddr)
                {   // Change buffer B and must waiting for bufferA changed.
#if ANTIVIBR_ENABLE
                    if (iduDmaReg->StartB != iduDmaReg->StartA)
                    {
                        //UartOutText("IB ");

                        // even frame only mode, drop odd frame
                        iduDmaReg->StartB = iduDmaReg->StartA;
                        iduDmaReg->EndB = iduDmaReg->EndA;
                    }
#else
                    if ( iduDmaReg->StartB != (iduDmaReg->StartA + (nextImgWinPtr->wWidth << 1)) )
                    {
                        //UartOutText("IB ");

                        // B start address is A start address + Width
                        // line 2, 4, 6, ...., Height
                        iduDmaReg->StartB = iduDmaReg->StartA + (nextImgWinPtr->wWidth << 1);
                        iduDmaReg->EndB = iduDmaReg->EndA + (nextImgWinPtr->wWidth << 1);
                    }
#endif
                }

                // Check buffer A/B changed
                if ( (iduDmaReg->StartA == newWinAddr) &&
#if ANTIVIBR_ENABLE
                (iduDmaReg->StartB == iduDmaReg->StartA) )
#else
                (iduDmaReg->StartB == (0x0FFFFFFF & (iduDmaReg->StartA + (nextImgWinPtr->wWidth << 1)))) )
#endif
                {
                    //UartOutText("iR ");

                    iduDmaReg->Control &= ~BIT24;       // Disable IDU DMA buffer end interrupt
                    needchangeWin = FALSE;

                    if (needReleaseSemaphore)
                    {
                        needReleaseSemaphore = FALSE;
                        SemaphoreRelease(IDU_SEMA_ID);
                    }
                }
            }
            else
            {
                if (bufferEndA)
                {   // Change buffer A
                    if (iduDmaReg->StartA != newWinAddr)
                    {
                        iduDmaReg->StartA = (DWORD) nextImgWinPtr->pdwStart;
                        iduDmaReg->EndA = (DWORD) nextImgWinPtr->pdwStart + (bufPixelCnt << 1) - 1;
                        //mpDebugPrint(" A 0x%X, 0x%X", iduDmaReg->StartA, nextImgWinPtr->pdwStart);
                        //UartOutText("A");
                    }
                }
                else
                {   // Change buffer B
                    if (iduDmaReg->StartB != newWinAddr)
                    {
                        iduDmaReg->StartB = (DWORD) nextImgWinPtr->pdwStart;
                        iduDmaReg->EndB = (DWORD) nextImgWinPtr->pdwStart + (bufPixelCnt << 1) - 1;
                        //mpDebugPrint(" B 0x%X, 0x%X", iduDmaReg->StartB, nextImgWinPtr->pdwStart);
                        //UartOutText("B ");
                    }
                }

                if ((iduDmaReg->StartA == newWinAddr) &&
                    (iduDmaReg->StartB == newWinAddr) )
                {
                    //UartOutText("R");

                    EventSet(IDU_EVENT_ID, IDU_CHANGE_WIN_FINISHED);

                    if (needReleaseSemaphore)
                    {
                        needReleaseSemaphore = FALSE;
                        SemaphoreRelease(IDU_SEMA_ID);
                    }

                    needchangeWin = FALSE;
                    iduDmaReg->Control &= ~BIT24;       // Disable IDU DMA buffer end interrupt
                }
            }
        }
        else
        {
            iduDmaReg->Control &= ~BIT24;               // Disable IDU DMA buffer end interrupt
        }

        for (i = 0; i < _MAX_NUM_OF_BUFFER_END_INFO_PTR_; i++)
        {
            if (bufferEndInfoCallbakc[i] != NULL)
            {
                void (*callBackFunPtr) (void) = bufferEndInfoCallbakc[i];
                callBackFunPtr();
            }
        }
    }
}
#endif



#if GAMMA_new
void Idu_SetYgamma(void)
{
	register IDU *idu = (IDU *)IDU_BASE;
	BYTE *pgamma;

    pgamma = (BYTE *)(&gammatable[g_psSystemConfig->sScreenSetting.bContrast][0]);
	memset(pgamma, 0, sizeof(BYTE) * 16);

	MP_DEBUG("\n\r YGAMMA === ");
	MP_DEBUG(g_psSystemConfig->sScreenSetting.bContrast, 2);

	idu->YGammaR[0] =
	(*(pgamma + 3) << 24) + (*(pgamma+2) << 16) + (*(pgamma+1) << 8) + (*pgamma);
	idu->YGammaR[1] =
	(*(pgamma+7) << 24) + (*(pgamma+6) << 16) + (*(pgamma+5) << 8) + (*(pgamma+4));
	idu->YGammaR[2] =
	(*(pgamma+11) << 24) + (*(pgamma+10) << 16) + (*(pgamma+9) << 8) + (*(pgamma+8));
	idu->YGammaR[3] =
	(*(pgamma+15) << 24) + (*(pgamma+14) << 16) + (*(pgamma+13) << 8) + (*(pgamma+12));
}
#endif



#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
#define XSampleCount 34
#define BrightnessFactor 3 // 2
#define ContrastFactor 6 // 5
/**
 * @ingroup Color
 * @brief Idu Set Contrast and Brightness
 * @param Contrast_value:-8~+7  default=0
 * @param Brightness_value:-8~+7  default=0
 */
void Idu_SetContrastBrightness(int Contrast_value,int Brightness_value)
{
    int i,tmp;
    int R_tmp,G_tmp,B_tmp;
		register IDU *idu = (IDU *) IDU_BASE;
		int XSampleValue[XSampleCount]={0  ,	4,8,	16 ,	24 ,	32 ,	40 ,	48 ,	56 ,
																		64 ,	72 ,	80 ,	88 ,	96 ,	104,	112,	120,
																		128,	136,	144,	152,	160,	168,	176,	184,
																		192,	200,	208,	216,	224,	232,	240,	248,	255};

    BYTE R_GMA_Source[XSampleCount]={0  ,4,8  ,16 ,24 ,32 ,40 ,48 ,56 ,64 ,72 ,80 ,88 ,96 ,104,112,120,128,136,144,152,160,168,176,184,192,200,208,216,224,232,240,248,255};
		BYTE G_GMA_Source[XSampleCount]={0  ,4,8  ,16 ,24 ,32 ,40 ,48 ,56 ,64 ,72 ,80 ,88 ,96 ,104,112,120,128,136,144,152,160,168,176,184,192,200,208,216,224,232,240,248,255};
		BYTE B_GMA_Source[XSampleCount]={0  ,4,8  ,16 ,24 ,32 ,40 ,48 ,56 ,64 ,72 ,80 ,88 ,96 ,104,112,120,128,136,144,152,160,168,176,184,192,200,208,216,224,232,240,248,255};

		BYTE R_GMA_Target[XSampleCount];
		BYTE G_GMA_Target[XSampleCount];
		BYTE B_GMA_Target[XSampleCount];

		if(Contrast_value>7)Contrast_value=7;
		if(Contrast_value<-8)Contrast_value=-8;
		if(Brightness_value>7)Brightness_value=7;
		if(Brightness_value<-8)Brightness_value=-8;

    //mpDebugPrint("----------------bContrast = %d", Contrast_value);
    //mpDebugPrint("================bBrightness = %d", Brightness_value);

    for(i=0;i<XSampleCount;i++)
    {
        R_tmp=R_GMA_Source[i];
        G_tmp=G_GMA_Source[i];
        B_tmp=B_GMA_Source[i];
        tmp=Contrast_value<<ContrastFactor;
        if(i>17)
        {
            R_tmp=R_tmp+(((R_tmp-R_GMA_Source[17])*tmp)>>10);
            G_tmp=G_tmp+(((G_tmp-G_GMA_Source[17])*tmp)>>10);
            B_tmp=B_tmp+(((B_tmp-B_GMA_Source[17])*tmp)>>10);
        }
        else
        {
            R_tmp=R_tmp-(((R_GMA_Source[17]-R_tmp)*tmp)>>10);
            G_tmp=G_tmp-(((G_GMA_Source[17]-G_tmp)*tmp)>>10);
            B_tmp=B_tmp-(((B_GMA_Source[17]-B_tmp)*tmp)>>10);
        }

        if(R_tmp>255)R_tmp=255;
        if(R_tmp<0)R_tmp=0;

        if(G_tmp>255)G_tmp=255;
        if(G_tmp<0)G_tmp=0;

        if(B_tmp>255)B_tmp=255;
        if(B_tmp<0)B_tmp=0;

        if(Brightness_value<0)
        {
            R_tmp=R_tmp-((0-Brightness_value)<<BrightnessFactor);
            G_tmp=G_tmp-((0-Brightness_value)<<BrightnessFactor);
            B_tmp=B_tmp-((0-Brightness_value)<<BrightnessFactor);
        }
        else
        {
            R_tmp=R_tmp+(Brightness_value<<BrightnessFactor);
            G_tmp=G_tmp+(Brightness_value<<BrightnessFactor);
            B_tmp=B_tmp+(Brightness_value<<BrightnessFactor);
        }

        if(R_tmp>255)R_tmp=255;
        if(R_tmp<0)R_tmp=0;

        if(G_tmp>255)G_tmp=255;
        if(G_tmp<0)G_tmp=0;

        if(B_tmp>255)B_tmp=255;
        if(B_tmp<0)B_tmp=0;

				//mpDebugPrint("----------------[%d]= %d  %d  %d",i, R_tmp,G_tmp,B_tmp);

        R_GMA_Target[i]=R_tmp;
        G_GMA_Target[i]=G_tmp;
        B_GMA_Target[i]=B_tmp;
    }

		Idu_SetCSC();
		//R gamma
		idu->CGainCtrl |= (DWORD)1 << 31;//enalbe R gamma
		/**/
		for(i=0;i<8;i++)
			idu->YGammaR[i]=(R_GMA_Target[i*4+4]<<24)+(R_GMA_Target[i*4+3]<<16)+(R_GMA_Target[i*4+2]<<8)+(R_GMA_Target[i*4+1]);

		//G gamma
		for(i=0;i<8;i++)
			idu->GammaG[i]=(G_GMA_Target[i*4+4]<<24)+(G_GMA_Target[i*4+3]<<16)+(G_GMA_Target[i*4+2]<<8)+(G_GMA_Target[i*4+1]);

		//B gamma
		for(i=0;i<8;i++)
			idu->GammaB[i]=(B_GMA_Target[i*4+4]<<24)+(B_GMA_Target[i*4+3]<<16)+(B_GMA_Target[i*4+2]<<8)+(B_GMA_Target[i*4+1]);

		idu->Gamma_Min_Ctrl=(R_GMA_Target[0]<<16)+(G_GMA_Target[0]<<8)+(B_GMA_Target[0]);
		idu->Gamma_Max_Ctrl=(idu->Gamma_Max_Ctrl&0xFF000000)|(1<<24)|1<<28|(R_GMA_Target[XSampleCount-1]<<16)+(G_GMA_Target[XSampleCount-1]<<8)+(B_GMA_Target[XSampleCount-1]);


/*
for(i=0;i<8;i++)
	mpDebugPrint("GammaR=0x%08x", idu->YGammaR[i]);
for(i=0;i<8;i++)
	mpDebugPrint("GammaG=0x%08x", idu->GammaG[i]);
for(i=0;i<8;i++)
	mpDebugPrint("GammaB=0x%08x", idu->GammaB[i]);


//mpDebugPrint("-B gamma=--------0x%08x",(DWORD)B_GMA_Target[0]);
mpDebugPrint("+++idu->Gamma_Min_Ctrl=0x%08x", idu->Gamma_Min_Ctrl);
mpDebugPrint("idu->Gamma_Max_Ctrl=0x%08x", idu->Gamma_Max_Ctrl);
 */


//idu->Gamma_Min_Ctrl=0x00080808;
//mpDebugPrint("idu->Gamma_Min_Ctrl=0x%08x", idu->Gamma_Min_Ctrl);

}


#endif


#if VIDEO_ON
void Idu_ReinitIdu(void)
{
	CLOCK *clock = (CLOCK *)(CLOCK_BASE);
	ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;

	clock->MdClken &= ~(0x00000100); //disable IDU clk
	MP_DEBUG("AAA");
	clock->MdClken |= (0x00000100); //enable IDU clk

	DisplayInit(DISPLAY_INIT_LOW_RESOLUTION);
}
#endif

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
///
///@ingroup group_IDU
///@brief   IDU would change to 444 display mode
///
///@param   NONE
///
///@retval  NONE
///
///@remark  The function must be called before Idu_ChgWin
///
void Idu_Chg_444_Mode()
{
	register IDU *idu = (IDU *)IDU_BASE;
	MP_DEBUG("IDU set to 444 mode");
	g_IDU_444_422_Flag = 1 ;
	//idu->RST_Ctrl_Bypass |= 0x80000000;
}


///
///@ingroup group_IDU
///@brief   IDU would change to 422 display mode
///
///@param   NONE
///
///@retval  NONE
///
///@remark  The function must be called before Idu_ChgWin
///
void Idu_Chg_422_Mode()
{
	register IDU *idu = (IDU *)IDU_BASE;
	MP_DEBUG("IDU set to 422 mode");
	g_IDU_444_422_Flag = 0 ;
	//idu->RST_Ctrl_Bypass &= 0x7fffffff;
}

BOOL Idu_GetIs444Mode(void)
{
    if(g_IDU_444_422_Flag)
        return TRUE;
    else
        return FALSE;
}

void Idu_DMA2Bypass()
{
	register CHANNEL *dma;
	register IDU *idu = (IDU *)IDU_BASE;
	MP_DEBUG("Idu_DMA2Bypass");
	dma = (CHANNEL *) (DMA_IDU_BASE);

		dma->Control |= 0x01000000;	//dma buffer end interrupt
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt indicator

        if((idu->RST_Ctrl_Bypass & BIT0) != 0)
        {
		     MP_DEBUG("already on bypass mode\n");
			 return; 
		}
        #if 0  // maybe cause YUV444 hang up use another to wait buffer end 
		MP_DEBUG("1 idu->RST_Ctrl_Bypass=%x", idu->RST_Ctrl_Bypass);
		while ((dma->Control & 0x00018000) != 0x00018000){} // TaskYield();
		MP_DEBUG("2 idu->RST_Ctrl_Bypass=%x", idu->RST_Ctrl_Bypass);
        #endif
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x38000000);
		while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x00000000);
		while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x38000000);

	   int waitflg;
       waitflg=1;
       while(waitflg) // wait dma buffer end
       {
          if(((idu->RST_Ctrl_Bypass & 0x18000000)==0x10000000) && (((idu->RST_Ctrl_Bypass & 0x07ff0000)>>16)>=32))
          {
              if(((idu->RST_Ctrl_Bypass & 0x18000000)==0x10000000) && (((idu->RST_Ctrl_Bypass & 0x07ff0000)>>16)>=32))
              {
                  waitflg=0;
              }
          }
      }
	
        idu->RST_Ctrl_Bypass = 0x00005503 ;			// IDU Bypass mode enable
		idu->RST_Ctrl_Bypass = 0x00000003 ;
		/*
		while ((dma->Control & 0x00018000) != 0x00010000); // TaskYield();
		dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		idu->RST_Ctrl_Bypass = 0x00000503 ;			// IDU Bypass mode enable
		idu->RST_Ctrl_Bypass = 0x00000003 ;
		*/
		while((idu->RST_Ctrl_Bypass & 0x1) != 0x1);
        dma->Control &= 0xfffffffe ;

}

void Idu_Bypass2DMA(ST_IMGWIN * ImgWin)
{
	register CHANNEL *dma;
	register IDU *idu = (IDU *)IDU_BASE;
	DMA *dmabase = (DMA *)DMA_BASE;
	dma = (CHANNEL *) (DMA_IDU_BASE);
	DWORD Bypass_Data = 0 ;

  ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;
  int i, j=0, ret = 1 ;
	MP_DEBUG("Idu_Bypass2DMA");
  	dma->Control &= 0xFFFFFF7F;  //recover IPR address bit swap
  //update current win
  if (ImgWin == &sWin[0])
	{
		psCurrWin = &sWin[0];
		psNextWin = &sWin[1];
	}
	else
	{
		psCurrWin = &sWin[1];
		psNextWin = &sWin[0];
	}
    dma->StartA = ((DWORD) ImgWin->pdwStart | 0xa0000000);
	dma->EndA = ((DWORD) ImgWin->pdwStart | 0xa0000000) + (ImgWin->wWidth * ImgWin->wHeight * 2) - 1;
	dma->StartB = dma->StartA ;
	dma->EndB = dma->EndA ;

    while((idu->RST_Ctrl_Bypass & 0x1))
	{
		//Bypass_Data = idu->RST_Ctrl_Bypass ;
		MP_DEBUG("Bypass_Data=%x", Bypass_Data);
		if((idu->RST_Ctrl_Bypass & 0x38000000) == 0x38000000)
		//if(((Bypass_Data & 0x3fff0000) == 0x30800000) || ((Bypass_Data & 0x3fff0000) == 0x10800000))
		{
			while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x00000000);
			while((idu->RST_Ctrl_Bypass & 0x38000000) == 0x38000000);
			idu->RST_Ctrl_Bypass = 0x00005502 ;
			idu->RST_Ctrl_Bypass = 0x00000002 ;
			while(idu->RST_Ctrl_Bypass & 0x1);
			MP_DEBUG("idu->RST_Ctrl_Bypass=%x", idu->RST_Ctrl_Bypass);
            #if ((PANEL_ID == PANEL_HDMI_720P)||(PANEL_ID == PANEL_ADV7393))
            while((idu->RST_Ctrl_Bypass & 0x10000000)==0x10000000);
            #endif
            dma->Control |= 0x00000001 ;
			MP_DEBUG("IDU change to DMA mode from Bypass Mode, Bypass_Data=%x", idu->RST_Ctrl_Bypass);
			break ;
		}
	}
  	dmabase->res &= 0xFFFFFF7F; //DMA IPR b7
}


/*
// Title : Idu_SetCSC
// State :
// USED : YUV --> RGB
// NOTE : 1. Default : RGB --> YUV
//		  2. enable TV_CTRL0 bit[26]:CSC_EN
// YCbCr -> RGB
// R = ( Y + 1.371 ( Cr - 128 ))
// G = ( Y - 0.698 ( Cr - 128 ) - 0.336 ( Cb - 128 ))
// B = ( Y + 1.732 ( Cr - 128 ))
//=>DEC
// R = ( Y + 350 ( Cr - 128 )) / 256
// G = ( Y - 178 ( Cr - 128 ) - 86 ( Cb - 128 )) / 256
// B = ( Y + 443 ( Cb - 128 )) / 256
//=>HEX
// R = ( Y + 0x15E ( Cr - 128 )) / 256
// G = ( Y - 0xB2 ( Cr - 128 ) - 0x56 ( Cb - 128 ) ) / 256
// B = ( Y + 0x1BB ( Cb - 128 )) / 256
*/
int Idu_SetCSC()//YUV -> RGB
{
	register IDU *idu = (IDU *)IDU_BASE;
	idu->TvCtrl0 = 0x06000000;//enable CSC & disable YUV2RGB
    #if (PANEL_ID != PANEL_ADV7393 && PANEL_ID != PANEL_ILI9163C)
	idu->Iducsc0 = 0x04790f4d;
	idu->Iducsc1 = 0x166000ff;
	idu->Iducsc2 = 0x34aea0ff;
	idu->Iducsc3 = 0x000714ff;
    #else
    idu->TvCtrl0 |= BIT31;//enable ARGB
    idu->Iducsc0 = 0x00;
    idu->Iducsc1 = 0x100;//0x0d0;
    idu->Iducsc3 = 0x40000;//0x50000;
    idu->Iducsc2 = 0x10000000;//0x17000000;
    #endif
	idu->ATCON_srgb_ctrl |= 0x00000020 ;

	MP_DEBUG("idu->Iducsc0 =0x%08x", idu->Iducsc0);
	MP_DEBUG("idu->Iducsc1 =0x%08x", idu->Iducsc1);
	MP_DEBUG("idu->Iducsc2 =0x%08x", idu->Iducsc2);
	MP_DEBUG("idu->Iducsc3 =0x%08x", idu->Iducsc3);

}

int MP650_FPGA_Change_DMA_Priority_IDU(void)
{
    Dma_PriorityDefault();
/*
    DMA* dma = (DMA*)DMA_BASE ;

    dma->FDMACTL_EXT1 = 0x01003000 ;

    IODelay(100);
    dma->FDMACTL_EXT0 &= ~0x00000007 ;
    dma->FDMACTL_EXT0 |= 0x00000003 ;
    dma->FDMACTL_EXT0 &= ~0x00000100 ;

    dma->FDMACTL_EXT2 = 0x00100002 ;
    dma->FDMACTL_EXT3 = 0xffeffffd ;

    dma->FDMACTL_EXT1 |= 0x00000001 ;
*/
}

int MP650_Set_DMA_Request_Limit(void)
{
    DMA* dma = (DMA*)DMA_BASE ;

    dma->FDMACTL_EXT0 &= ~0x00000100 ;
}

void IDU_Show_BG_Color_Black(void)
{
		register IDU *idu = (IDU *)IDU_BASE;
    #if (PANEL_ID == PANEL_ADV7393)
		idu->BG_Ctrl = 0x00808010 ;  //YUV
    #else
		idu->BG_Ctrl = 0x00000010 ;  //RGB  
    #endif
		mpDebugPrint("idu->BG_Ctrl=0x%08x", idu->BG_Ctrl);
}

void IDU_Show_Original_Color(void)
{
		register IDU *idu = (IDU *)IDU_BASE;

    #if (PANEL_ID == PANEL_ADV7393)
		idu->BG_Ctrl = 0x00808000 ; //YUV
    #else
		idu->BG_Ctrl = 0x00000000 ; //RGB    
    #endif

		MP_DEBUG("idu->BG_Ctrl=0x%08x", idu->BG_Ctrl);
}

#if (TCON_ID == TCON_RGB8SerialMode_320x240 || TCON_ID == TCON_RGB8SerialMode_480x240 || TCON_ID == TCON_RGB8SerialMode_320x240_ILI9342)
void Serial_RGB_Init()
{
	IDU* idu = (IDU *)IDU_BASE;
	//[15]:RGB_DUMMY;
	//[11]:LCD8BIT_1PXC;
	//[0]:8 bit mode
	//idu->IduCtrl0 &= ~0x0b800000 ;
	idu->IduCtrl0 |= 0x00300000;
	idu->Gpoctrl1 &= 0x7fffffff ;
	#if (TCON_ID == TCON_RGB8SerialMode_320x240) 
    //idu->ATCON_srgb_ctrl = 0x0700cc11; //UP052_320mode
    idu->ATCON_srgb_ctrl = 0x07004601;//UP051_320mode
	#elif (TCON_ID == TCON_RGB8SerialMode_480x240)
	idu->ATCON_srgb_ctrl = 0x0700e601; //db, bd, fd, ee, df
	#else
	//idu->ATCON_srgb_ctrl = 0x07004601;
	idu->ATCON_srgb_ctrl = 0x07000001;
	#endif
}
#endif
#endif


