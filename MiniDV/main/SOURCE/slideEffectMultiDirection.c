/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "display.h"
#include "slideEffect.h"


extern ST_IMGWIN WinOriginal;

#if SLIDE_TRANSITION_BRICK

typedef struct
{
    WORD Top;
    WORD Left;
    WORD Height;
    WORD Width;
} ST_BRICK_DATA;

ST_BRICK_DATA BrickArray[BRICK_COUNT];

SWORD BrickReset()
{
    int index, row, column;
    WORD height0, width0, height1, width1;
    WORD height, top, left;
    ST_BRICK_DATA *brick;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

    height1 = pstScreen->wInnerHeight;
    height0 = (height1 / BRICK_ROW_COUNT) & 0xfffe;
    height1 = height1 - height0 * (BRICK_ROW_COUNT - 1);

    width1 = pstScreen->wInnerWidth;
    width0 = (width1 / BRICK_COLUMN_COUNT) & 0xfffe;
    width1 = (width1 - width0 * (BRICK_COLUMN_COUNT - 1)) & 0xfffe;

    index = 0;
    top = 0;

    for (row = 0; row < BRICK_ROW_COUNT; row++)
    {
        left = 0;

        if (row == BRICK_ROW_COUNT - 1)
            height = height1;
        else
            height = height0;

        for (column = 0; column < BRICK_COLUMN_COUNT; column++)
        {
            brick = BrickArray + index;
            brick->Top = top;
            brick->Left = left;
            brick->Height = height;

            if (column == BRICK_COLUMN_COUNT - 1)
                brick->Width = width1;
            else
                brick->Width = width0;

            left += brick->Width;
            index++;
        }

        top += height;

        if (Polling_Event())
            return FAIL;
    }

    return PASS;
}


SWORD BrickTransition(ST_IMGWIN *srcWin, WORD *pattern, BYTE *workingBuff)
{
    int ret;
    WORD counter;
    ST_IMGWIN *win, target;
    DWORD *origin;
    ST_BRICK_DATA *brick;

    // if the sizes of both the windows are not the same, then don't do it
    if (!CheckDimension(srcWin))
        return FAIL;

    TransWinReset(srcWin, workingBuff);

    if (Polling_Event())
        return FAIL;

    if (BrickReset() == FAIL)
        return FAIL;

    origin = WinOriginal.pdwStart;
    counter = BRICK_COUNT;
    win = Idu_GetCurrWin();

    while (counter)
    {
        DWORD dwFrameStartTime = GetSysTime();

        counter--;
        brick = BrickArray + *pattern;
        pattern++;
        __Block_Copy(&WinOriginal,win,brick->Left,brick->Top,brick->Width,brick->Height,brick->Left,brick->Top);

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwFrameStartTime) < BRICK_DELAY * SLIDEEFFECTTIME);
    }

    return PASS;
}



static WORD GeneralBrickTable[BRICK_COUNT];

void BrickSwap(WORD *a, WORD *b)
{
    WORD tmp;

    tmp = *a;
    *a  = *b;
    *b  = tmp;
}



void RowShift(WORD row)
{
    int i;
    WORD last = GeneralBrickTable[(row * BRICK_COLUMN_COUNT)+ BRICK_COLUMN_COUNT - 1];

    for(i = BRICK_COLUMN_COUNT - 1; i>0; i--)
    {
        GeneralBrickTable[(row * BRICK_COLUMN_COUNT)+ i]=
        GeneralBrickTable[(row * BRICK_COLUMN_COUNT)+ i - 1];
    }

    GeneralBrickTable[(row * BRICK_COLUMN_COUNT)] = last;
}



void ColShift(WORD col)
{
    int i;
    WORD last = GeneralBrickTable[((BRICK_ROW_COUNT-1)* BRICK_COLUMN_COUNT)+ col];

    for(i = BRICK_ROW_COUNT - 1; i>0; i--)
    {
        GeneralBrickTable[(i* BRICK_COLUMN_COUNT)+ col] =
        GeneralBrickTable[((i - 1)* BRICK_COLUMN_COUNT)+ col];
    }

    GeneralBrickTable[col] = last;
}



void BrickSeqRandom()
{
    int i, row, col;

    for(col = 0; col < BRICK_COLUMN_COUNT; col+=2)
    {
        for(row = 0; row + 1 < BRICK_ROW_COUNT; row+=2)
        {
            BrickSwap(GeneralBrickTable + ((row * BRICK_COLUMN_COUNT)+col),
                      GeneralBrickTable + (((row+1) * BRICK_COLUMN_COUNT)+col));
        }
    }

    for(row = 1; row < BRICK_ROW_COUNT; row++)
    {
        for(col = 0; col+1 < BRICK_COLUMN_COUNT; col+=2)
        {
            BrickSwap(GeneralBrickTable + ((row * BRICK_COLUMN_COUNT)+col),
                      GeneralBrickTable + ((row * BRICK_COLUMN_COUNT)+col+1));
        }
    }

    for(col = 0; col < BRICK_COLUMN_COUNT; col+=3)
    {
        for(row = 0; row + 1 < BRICK_ROW_COUNT; row+=2)
        {
            BrickSwap(GeneralBrickTable + ((row * BRICK_COLUMN_COUNT)+col),
                      GeneralBrickTable + (((row+1) * BRICK_COLUMN_COUNT)+col));
        }
    }
}



SWORD SpiralBrick(ST_IMGWIN * win, BYTE * workingBuff)
{
    BYTE direction, numStep;
    int row, col, i, k;

    i = 0;
    k = 0;

    // direction -- 0 move rignt, 1 move down, 2 move left, 3 move up
    direction = 1;
    numStep = 1;

    // Start brick's position setting
    if(BRICK_ROW_COUNT%2)
        row = (BRICK_ROW_COUNT>>1);
    else
        row = (BRICK_ROW_COUNT>>1) - 1;

    if(BRICK_COLUMN_COUNT%2)
        col = (BRICK_COLUMN_COUNT>>1);
    else
        col = (BRICK_COLUMN_COUNT>>1) - 1;

    GeneralBrickTable[k] = (row * BRICK_COLUMN_COUNT) + col;

    col++; //First step is moving right.
    k++;

    while( (0<=row) && (row<BRICK_ROW_COUNT) && (0<=col) && (col<BRICK_COLUMN_COUNT) )
    {
        GeneralBrickTable[k] = (row * BRICK_COLUMN_COUNT) + col;
        k++;
        i++;

        if(i>= numStep)
        {
            i = 0;

            if(direction == 2)
            {
                numStep++;
                direction++;
            }
            else if(direction == 4)
            {
                numStep++;
                direction = 1;
            }
            else
            {
                direction++;
            }
        }

        switch(direction)
        {
        case 1 :	//move right
            col++;
            break;

        case 2 :	//move down
            row++;
            break;

        case 3 :	//move left
            col--;
            break;

        case 4 :	//move up
            row--;
        }

        if (Polling_Event())
            return FAIL;
    }

end:
    return BrickTransition(win, GeneralBrickTable, workingBuff);
}



SWORD UpperLeftBrick(ST_IMGWIN * win, BYTE * workingBuff)
{
    WORD i, k;;
    WORD totalSlant;
    int row, col;

    k = 0;
    row = 0;
    col = 0;

    totalSlant = BRICK_COLUMN_COUNT + BRICK_ROW_COUNT - 1;

    for(i=0; i<totalSlant; i++)
    {   // set progressive increase sequence in oblique
        int tmpRow, tmpCol;

        tmpRow = row;
        tmpCol = col;

        while(tmpRow<BRICK_ROW_COUNT && tmpCol>=0)
        {
            GeneralBrickTable[k] = (tmpRow * BRICK_COLUMN_COUNT) + tmpCol;

            //each brick in the same slant, it's row add colum is equal.
            tmpRow++;
            tmpCol--;
            k++;
        }

        if(col < BRICK_COLUMN_COUNT - 1)
            col++;
        else
            row++;

        if (Polling_Event())
            return FAIL;
    }

    // For look like more random
    BrickSeqRandom();

    return BrickTransition(win, GeneralBrickTable, workingBuff);
}



SWORD LowerRightBrick(ST_IMGWIN * win, BYTE * workingBuff)
{
    WORD i, k;;
    WORD totalSlant;
    int row, col;

    k = 0;
    row = 0;
    col = BRICK_COLUMN_COUNT - 1;

    totalSlant = BRICK_COLUMN_COUNT + BRICK_ROW_COUNT - 1;

    for(i=0; i<totalSlant; i++)
    {
        int tmpRow, tmpCol;

        tmpRow = row;
        tmpCol = col;

        while(tmpRow<BRICK_ROW_COUNT && tmpCol<BRICK_COLUMN_COUNT)
        {
            GeneralBrickTable[k] = (tmpRow * BRICK_COLUMN_COUNT) + tmpCol;

            k++;
            tmpRow++;
            tmpCol++;
        }

        if(col > 0)
            col--;
        else
            row++;

        if (Polling_Event())
            return FAIL;
    }

    // For look like more random
    BrickSeqRandom();

    return BrickTransition(win, GeneralBrickTable, workingBuff);
}



SWORD TopDownBrick(ST_IMGWIN * win, BYTE * workingBuff)
{
    WORD i, k;
    WORD curRow, curCol;
    WORD whileCount;

    curRow = 0;
    curCol = 0;
    k = 0;

    whileCount = BRICK_COUNT - (BRICK_COLUMN_COUNT>>1);

    //First time
    for(i = 0; i < (BRICK_COLUMN_COUNT>>1); i++)
    {
        GeneralBrickTable[k] = curCol;
        curCol +=2;
        k++;
    }

    curRow++;
    curCol = 1;

    while(k < whileCount)
    {
        for(i = 0; i < (BRICK_COLUMN_COUNT>>1); i++)
        {
            GeneralBrickTable[k] = (curRow * BRICK_COLUMN_COUNT) + curCol;
            curCol +=2;
            k++;

            if(k >= whileCount)
                break;
        }

        //Fill bricks at the same colum above current row
        curCol %= 2;
        curRow--;

        for(i = 0; i < (BRICK_COLUMN_COUNT>>1); i++)
        {
            GeneralBrickTable[k] = (curRow * BRICK_COLUMN_COUNT) + curCol;
            curCol +=2;
            k++;

            if(k >= whileCount)
                break;
        }

        //Next time we move down two rows, and change start position of colum
        curRow += 2;
        curCol = ( (curCol % 2)? 0 : 1);

        if (Polling_Event())
            return FAIL;
    }

    //Last time, fill remainder bricks
    curCol = 0;

    for(i = 0; i < (BRICK_COLUMN_COUNT>>1); i++)
    {
        GeneralBrickTable[k] = ((BRICK_ROW_COUNT - 1)* BRICK_COLUMN_COUNT) + curCol;
        curCol +=2;
        k++;
    }

    return BrickTransition(win, GeneralBrickTable, workingBuff);
}



SWORD RandomBrick(ST_IMGWIN * win, BYTE * workingBuff)
{
    WORD i, j, k, shift_time;
    WORD row, col;
    DWORD allCounter = BRICK_ROW_COUNT * BRICK_COLUMN_COUNT;

    for(i=0; i<allCounter; i++)
    {
        GeneralBrickTable[i] = i ;
    }

    for(k=0; k<3; k++)
    {
        //First time random
        shift_time = 1;

        for(i=0; i<BRICK_COLUMN_COUNT; i++)
        {
            for(j=0; j < shift_time; j++)
            {
                ColShift(i);
            }

            shift_time++;
        }

        if (Polling_Event())
            return FAIL;

        //Second time random
        shift_time = 1;

        for(i=0; i<BRICK_ROW_COUNT; i++)
        {
            for(j=0; j < shift_time; j++)
            {
                RowShift(i);
            }

            shift_time++;
        }

        if (Polling_Event())
            return FAIL;

        BrickSeqRandom();
    }

    return BrickTransition(win, GeneralBrickTable, workingBuff);
}

#endif


#if (SLIDE_TRANSITION_BAR ||SLIDE_TRANSITION_MULTIEFFECT)

#define BAR_TOP_DOWN        0
#define BAR_BOTTOM_UP       1
#define BAR_LEFT_RIGHT      2
#define BAR_RIGHT_LEFT      3
#define BAR_IN_H            4
#define BAR_OUT_H           5
#define BAR_IN_V            6
#define BAR_OUT_V           7

SWORD SlideBarTransition(ST_IMGWIN * srcWin, BYTE pattern, BYTE * workingBuff)
{
    DWORD *dwOrigin;
    WORD index, barHCount, barVCount;
    ST_IMGWIN *win, target;
    WORD vStep, hStep, ii, jj, kk, maxNum;

	if(pattern > 8)
		pattern = 0;

    // make sure (srcWin->wHeight % barVCount) = 0 and (barVCount % 2) = 0 or will have garbage
    barVCount = BAR_TOPDOWN_HEIGHT;
    barHCount = BAR_LR_WIDTH;

    hStep = srcWin->wWidth	/ barHCount;
    vStep = srcWin->wHeight / barVCount;

    if (!CheckDimension(srcWin))
        return FAIL;

    TransWinReset(srcWin, workingBuff);

    dwOrigin = WinOriginal.pdwStart;
	win = Idu_GetCurrWin();

    switch (pattern)
    {
    case BAR_TOP_DOWN:
    case BAR_BOTTOM_UP:
        maxNum = barVCount;

        if (pattern == BAR_TOP_DOWN)
        {
            ii = 0;
            jj = 1;
        }
        else
        {
            ii = maxNum - 1;
            jj = -1;
        }

        for (index = 0; index < maxNum; index++, ii += jj)
        {
            DWORD dwFrameStartTime = GetSysTime();
			DWORD start_offset = (ii * vStep * (win->wWidth)) >> 1;

			mmcp_memcpy((BYTE *)(win->pdwStart + start_offset),
				(BYTE *)(dwOrigin + start_offset), win->wWidth*vStep<<1);

            do
            {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwFrameStartTime) < BAR_TOPDOWN_DELAY * SLIDEEFFECTTIME);
        }
        break;

    case BAR_LEFT_RIGHT:
    case BAR_RIGHT_LEFT:
        WinOriginal.wWidth = hStep;
        maxNum = barHCount;

        if (pattern == BAR_LEFT_RIGHT)
        {
            ii = 0;
            jj = 1;
        }
        else
        {
            ii = maxNum - 1;
            jj = -1;
        }

        for (index = 0; index < maxNum; index++, ii += jj)
        {
            DWORD dwFrameStartTime = GetSysTime();

			__Block_Copy(&WinOriginal,win,(ii * hStep),0,hStep,win->wHeight,(ii * hStep),0);

            do
            {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwFrameStartTime) < BAR_LR_DELAY * SLIDEEFFECTTIME);
        }
        break;

    case BAR_IN_H:
    case BAR_OUT_H:
        WinOriginal.wWidth = hStep;
        maxNum = barHCount >> 1;

        if (pattern == BAR_IN_H)
        {
            ii = 0;
            jj = 1;
            kk = barHCount - 1;
        }
        else
        {
            ii = (barHCount >> 1) - 1;
            jj = -1;
            kk = barHCount >> 1;
        }

        for (index = 0; index < maxNum; index++, ii += jj, kk -= jj)
        {
            DWORD dwFrameStartTime = GetSysTime();

			__Block_Copy(&WinOriginal,win,(ii * hStep),0,hStep,win->wHeight,(ii * hStep),0);

            do
            {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwFrameStartTime) < BAR_INOUTH_DELAY * SLIDEEFFECTTIME);

            dwFrameStartTime = GetSysTime();

			__Block_Copy(&WinOriginal,win,(kk * hStep),0,hStep,win->wHeight,(kk * hStep),0);
            do
            {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwFrameStartTime) < BAR_INOUTH_DELAY * SLIDEEFFECTTIME);
        }
        break;

    case BAR_IN_V:
    case BAR_OUT_V:
        WinOriginal.wHeight = vStep;
        maxNum = barVCount >> 1;

        if (pattern == BAR_IN_V)
        {
            ii = 0;
            jj = 1;
            kk = barVCount - 1;
        }
        else
        {
            ii = (barVCount >> 1) - 1;
            jj = -1;
            kk = barVCount >> 1;
        }

        for (index = 0; index < maxNum; index++, ii += jj, kk -= jj)
        {
            DWORD dwFrameStartTime = GetSysTime();
			DWORD start_offset;

			start_offset = (ii * vStep * (win->wWidth)) >> 1;
			mmcp_memcpy((BYTE *)(win->pdwStart + start_offset),
				(BYTE *)(dwOrigin + start_offset), win->wWidth*vStep<<1);

            do
            {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwFrameStartTime) < BAR_INOUTV_DELAY * SLIDEEFFECTTIME);

            dwFrameStartTime = GetSysTime();

			start_offset = (kk * vStep * win->wWidth) >> 1;
			mmcp_memcpy((BYTE *)(win->pdwStart + start_offset),
				(BYTE *)(dwOrigin + start_offset), win->wWidth*vStep<<1);

            do
            {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwFrameStartTime) < BAR_INOUTV_DELAY * SLIDEEFFECTTIME);
        }
        break;
    }

    return PASS;
}

SWORD SlideRandomBar(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin)	//Lighter 02/25
{
    BYTE Flag, InterNum = RANDOMBAR_STEP, temp[RANDOMBAR_STEP];
    WORD i, j;
    DWORD wBarSize;
    DWORD dwFrameStartTime;

    temp[0] = GetSysTime() % InterNum;

    //psTWin->pdwStart = (DWORD*)((DWORD)psTWin->pdwStart | BIT29);
    //psSWin->pdwStart = (DWORD*)((DWORD)psSWin->pdwStart | BIT29);

    if (Polling_Event())
        return FAIL;

    for (i = 1; i < InterNum; i++)
    {
        temp[i] = (GetSysTime() + 12) % InterNum;

        while (1)
        {
            Flag = 1;

            for (j = 0; j < i; j++)
            {
                if (temp[j] == temp[i])
                {
                    Flag = 0;
                    break;
                }
            }

            if (Flag)
                break;

            temp[i] += 5;

            if (temp[i] >= InterNum)
                temp[i] -= InterNum;

            if (Polling_Event())
                return FAIL;
        }
    }

    wBarSize = (psSWin->wWidth) * (psSWin->wHeight) / InterNum >> 1;

    for (i = 0; i < InterNum; i++)
    {
        dwFrameStartTime = GetSysTime();
		mmcp_memcpy((BYTE *)(psTWin->pdwStart + wBarSize * temp[i]),
               (BYTE *)(psSWin->pdwStart + wBarSize * temp[i]), wBarSize << 2);
        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwFrameStartTime) < RANDOMBAR_DELAY * SLIDEEFFECTTIME);
    }

    return PASS;
}



#endif

#if SLIDE_TRANSITION_EXPANSION

// 01.04.2010 Lighter modify
SWORD SlideLineExpansion(ST_IMGWIN * psSWin, BYTE * workingBuff, BYTE bDir)	//Mason 20060731
{
    BYTE i;
    WORD wStepLength;
    DWORD dwStartTime;
	ST_IMGWIN * psTWin;

    if (!CheckDimension(psSWin))
        return;
    TransWinReset(psSWin, workingBuff);

	psSWin = &WinOriginal;
	psTWin = Idu_GetNextWin();

	Slide_WinCopy(Idu_GetCurrWin(), psTWin);

    for (i = 1; i <= SLIDE_LINEEXPANSION_STEP; i ++)
    {
        dwStartTime = GetSysTime();

        if (bDir)   // Horizon Expansion
        {
            wStepLength = psTWin->wWidth * i / SLIDE_LINEEXPANSION_STEP;

            if (wStepLength > 64)
            {
                if(i == SLIDE_LINEEXPANSION_STEP)
                {
					Slide_WinCopy(psSWin, Idu_GetNextWin());
                    Idu_ChgWin(Idu_GetNextWin());
                }
                else
                {
					Ipu_ImageScaling(psSWin,psTWin,0, 0, psSWin->wWidth, psSWin->wHeight,
						(psTWin->wWidth - wStepLength) >> 1, 0,
						((psTWin->wWidth - wStepLength) >> 1)+wStepLength, psTWin->wHeight,0);
                }
            }
        }
        else    // Vertical Expansion
        {
            wStepLength = psTWin->wHeight * i / SLIDE_LINEEXPANSION_STEP;

            if (wStepLength > 32)
            {
                if(i == SLIDE_LINEEXPANSION_STEP)
                {
					Slide_WinCopy(psSWin, psTWin);
                }
                else
                {
					Ipu_ImageScaling(psSWin,psTWin,0, 0, psSWin->wWidth, psSWin->wHeight,
						0, (psTWin->wHeight - wStepLength) >> 1,
						psTWin->wWidth, ((psTWin->wHeight - wStepLength) >> 1)+wStepLength,0);
                }
            }
        }

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwStartTime) < LINEEXPANSION_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(psTWin);
		psTWin = Idu_GetNextWin();
    }
	Slide_WinCopy(psSWin, psTWin);
	Idu_ChgWin(psTWin);

    return PASS;
}


// 01.04.2010 Lighter modify
SWORD SlideCentralConstriction(ST_IMGWIN * psSWin, BYTE* workingBuff)
{
    int j;
    WORD wStepWidth, wStepHeight;
	DWORD dwStartTime;
	ST_IMGWIN * psTWin;

    if (!CheckDimension(psSWin))
        return;
    TransWinReset(psSWin, workingBuff);

	psSWin = &WinOriginal;
	psTWin = Idu_GetNextWin();
	ST_IMGWIN * psOWin = Win_New(psTWin->wWidth,psTWin->wHeight);
	Slide_WinCopy(Idu_GetCurrWin(), psOWin);

    for (j = SLIDE_EXPAN_STEP; j > 0; j-= 1)
    {
        dwStartTime = GetSysTime();

        wStepWidth = (psTWin->wWidth * j / SLIDE_EXPAN_STEP) & 0xfffc;
        wStepHeight = (psTWin->wHeight * j / SLIDE_EXPAN_STEP)  & 0xfffc;

		Ipu_ImageScaling(psOWin,psTWin,0, 0,psSWin->wWidth,psSWin->wHeight,
			(psTWin->wWidth - wStepWidth) >> 1, (psTWin->wHeight - wStepHeight) >> 1,
			(psTWin->wWidth + wStepWidth) >> 1, (psTWin->wHeight + wStepHeight) >> 1,0);

        do {
            if (Polling_Event())
            {
				Win_Free(psOWin);
                return FAIL;
            }
        } while (SystemGetElapsedTime(dwStartTime) < CENTRALEXPANSION_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(psTWin);
		psTWin = Idu_GetNextWin();
		Slide_WinCopy(psSWin, psTWin);
    }

	Slide_WinCopy(psSWin, psTWin);
	Idu_ChgWin(psTWin);

	Win_Free(psOWin);
	return PASS;
}



SWORD SlideCentralExpansion(ST_IMGWIN * psSWin, BYTE* workingBuff)
{
    BYTE j;
    WORD wStepWidth, wStepHeight;
	DWORD dwStartTime;
	ST_IMGWIN * psTWin;

	if (!CheckDimension(psSWin))
		return;
	TransWinReset(psSWin, workingBuff);

	psSWin = &WinOriginal;
	psTWin = Idu_GetNextWin();
	Slide_WinCopy(Idu_GetCurrWin(), psTWin);

    for (j = 0; j <= SLIDE_EXPAN_STEP; j+= 3)
    {
        dwStartTime = GetSysTime();

        wStepWidth = (psTWin->wWidth * j / SLIDE_EXPAN_STEP) & 0xfffc;
        wStepHeight = (psTWin->wHeight * j / SLIDE_EXPAN_STEP)  & 0xfffc;
		Ipu_ImageScaling(psSWin, psTWin, 0, 0, psSWin->wWidth, psSWin->wHeight,
			(psTWin->wWidth - wStepWidth) >> 1, (psTWin->wHeight - wStepHeight) >> 1,
			(psTWin->wWidth + wStepWidth) >> 1, (psTWin->wHeight + wStepHeight) >> 1,0);

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwStartTime) < CENTRALEXPANSION_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(psTWin);
		psTWin = Idu_GetNextWin();
		//Slide_WinCopy(psSWin, psTWin);
    }

	Slide_WinCopy(psSWin, psTWin);
	Idu_ChgWin(psTWin);

	return PASS;
}

#endif


#if SLIDE_TRANSITION_SCROLL

void Flip_YCbCr(BYTE *pImageBuffer, WORD wWidth, WORD wHeight)
{
	WORD i, j;
	BYTE temp_buf;

	for (i = 0; i < wHeight; i ++)
	{
		for (j = 0; j < wWidth; j += 4)
		{
			temp_buf = *(pImageBuffer + i * wWidth * 2 + j);		//Y1
			*(pImageBuffer + i * wWidth * 2 + j) = *(pImageBuffer + (i + 1) * wWidth * 2 - j - 3);	//Yn
			*(pImageBuffer + (i + 1) * wWidth * 2 - j - 3) = temp_buf;	//exchaneg Y1 and Yn

			temp_buf = *(pImageBuffer + i * wWidth * 2 + j + 1);		//Y2
			*(pImageBuffer + i * wWidth * 2 + j + 1) = *(pImageBuffer + (i + 1) * wWidth * 2 - j - 4);	//Yn-1
			*(pImageBuffer + (i + 1) * wWidth * 2 - j - 4) = temp_buf;	//exchaneg Y2 and Yn-1

			temp_buf = *(pImageBuffer + i * wWidth * 2 + j + 2);		//Cb1
			*(pImageBuffer + i * wWidth * 2 + j + 2) = *(pImageBuffer + (i + 1 ) * wWidth * 2 - j - 2);	//Cbn
			*(pImageBuffer + (i + 1) * wWidth * 2 - j - 2) = temp_buf;	//exchaneg Cb1 and Cbn

			temp_buf = *(pImageBuffer + i * wWidth * 2 + j + 3);		//Cr1
			*(pImageBuffer + i * wWidth * 2 + j + 3) = *(pImageBuffer + (i + 1) * wWidth * 2 - j - 1);	//Crn
			*(pImageBuffer + (i + 1) * wWidth * 2 - j - 1) = temp_buf;	//exchaneg Cr1 and Crn
		}
	}
}

//Gino modify from Slide CrossComb at 071026
SWORD SlideScroll(ST_IMGWIN * psSWin, BYTE* workingBuff)
{
	WORD wWidth, wHeight;
	WORD InterNum = SCROLL_WIDTH;
	WORD i, j, k;

	WORD left, top, right, bottom;

	WORD CountSlide;
	DWORD dwFrameStartTime;

	//Image buffer for scroll effect
	BYTE *pIBuf_a1, *pIBuf_a2;
	BYTE *pIBuf_b1, *pIBuf_b2;
	//pointer for identification of reading buf_a or buf_b
	BYTE *pIBuf_r1, *pIBuf_r2;
	BYTE *pIBuf_w;

	WORD scroll_right, wWidth_d2;

	//brightness control
	BYTE brightness[8] = {20, 18, 15, 13, 13, 11, 11, 10};
	ST_IMGWIN * psTWin = Idu_GetNextWin();

    if (!CheckDimension(psSWin))
        return;
    TransWinReset(psSWin, workingBuff);
	Slide_WinCopy(Idu_GetCurrWin(), psTWin);

	psSWin = &WinOriginal;

	// if big screen cell size 1/4
	if (psSWin->wWidth >= 1024) {
		InterNum <<= 1;
	}

	wWidth = InterNum;
	wHeight = psSWin->wHeight;// / bSlideStep;
	top = 0;
	bottom = top + wHeight;

	pIBuf_a1 = (BYTE *)ext_mem_malloc((wWidth * wHeight)<<1);
	pIBuf_a2 = (BYTE *)ext_mem_malloc((wWidth * wHeight)<<1);

	pIBuf_b1 = (BYTE *)ext_mem_malloc((wWidth * wHeight)<<1);
	pIBuf_b2 = (BYTE *)ext_mem_malloc((wWidth * wHeight)<<1);

	wWidth_d2 = wWidth >> 1;

	mpCatchWinToBuffer(pIBuf_a1, psTWin, psTWin->wWidth - wWidth_d2, 0, wWidth_d2, wHeight);
	mpCatchWinToBuffer(pIBuf_a2, psTWin, psTWin->wWidth - wWidth, 0, wWidth_d2, wHeight);

	for (CountSlide = 0, scroll_right = psSWin->wWidth - wWidth_d2, left = 0;
		left < psSWin->wWidth; left += wWidth_d2, CountSlide ++, scroll_right -= wWidth_d2)
	{
		dwFrameStartTime = GetSysTime();
		switch (CountSlide & 0x3)
		{
			case 0:
				pIBuf_w = pIBuf_b1;
				pIBuf_r1 = pIBuf_b2;
				pIBuf_r2 = pIBuf_a1;
				break;
			case 1:
				pIBuf_w = pIBuf_b2;
				pIBuf_r1 = pIBuf_a1;
				pIBuf_r2 = pIBuf_a2;
				break;
			case 2:
				pIBuf_w = pIBuf_a1;
				pIBuf_r1 = pIBuf_a2;
				pIBuf_r2 = pIBuf_b1;
				break;
			case 3:
				pIBuf_w = pIBuf_a2;
				pIBuf_r1 = pIBuf_b1;
				pIBuf_r2 = pIBuf_b2;
				break;
			default:
				break;
		}

		if (left + wWidth_d2 > psSWin->wWidth)
			wWidth_d2 = psSWin->wWidth - left;

		right = psSWin->wWidth - left;
		if (right < wWidth_d2)
			right = wWidth_d2;

		if (left + wWidth_d2 <= psTWin->wWidth)
			mpCatchWinToBuffer(pIBuf_w, psTWin, scroll_right - wWidth, 0, wWidth_d2, wHeight);

		//Copy from new photo
		__Block_Copy(psSWin,psTWin,right - wWidth_d2, 0,psSWin->wWidth-right+wWidth_d2,bottom,right - wWidth_d2, 0);
		__Block_Copy(psSWin,psTWin,right - wWidth_d2, 0,wWidth_d2,bottom,right - wWidth_d2, 0);
		if (left +wWidth < psTWin->wWidth)
		{
			Flip_YCbCr(pIBuf_r2, wWidth_d2, wHeight);

			for (i = 0; i < wHeight; i ++)
			{
				k = 0;
				for (j = 0; j < wWidth_d2 << 1; j+=4)
				{
					if (CountSlide != 0){
					*(pIBuf_r1 + i * wWidth_d2 * 2 + j) = *(pIBuf_r1 + i * wWidth_d2 * 2 + j) / brightness[k] * brightness [7 - k];
					*(pIBuf_r1 + i * wWidth_d2 * 2 + j + 1) = *(pIBuf_r1 + i * wWidth_d2 * 2 + j + 1)  / brightness[k] * brightness [7 - k];}

					*(pIBuf_r2 + i * wWidth_d2 * 2 + j) = *(pIBuf_r2 + i * wWidth_d2 * 2 + j) / brightness[7 - k] * 10;
					*(pIBuf_r2 + i * wWidth_d2 * 2 + j + 1) = *(pIBuf_r2 + i * wWidth_d2 * 2 + j + 1) / brightness[7 - k] * 10;

					if (InterNum == SCROLL_WIDTH)
						k++;
					else
						if (j & 0x1000)
							k++;
				}
			}

			if (CountSlide != 0)// && scroll_right < psSWin->wWidth)
				mpCopyBufferToWin(pIBuf_r1, psTWin, scroll_right - wWidth, 0, wWidth_d2, wHeight);

			//if (scroll_right  < psSWin->wWidth)
			mpCopyBufferToWin(pIBuf_r2, psTWin, scroll_right - wWidth_d2, 0, wWidth_d2, wHeight);
		}
		// delay for a while and polling event
		do {
			if (Polling_Event())
			{
				ext_mem_free(pIBuf_a1);
				ext_mem_free(pIBuf_a2);
				ext_mem_free(pIBuf_b1);
				ext_mem_free(pIBuf_b2);
				return FAIL;
			}
		}
		while (GetElapsedMs2(dwFrameStartTime) < SCROLL_DELAY);          // jeffery 080716
		Idu_ChgWin(psTWin);
		psTWin = Idu_GetNextWin();
	}
	Slide_WinCopy(psSWin, psTWin);
	Idu_ChgWin(psTWin);
	ext_mem_free(pIBuf_a1);
	ext_mem_free(pIBuf_a2);
	ext_mem_free(pIBuf_b1);
	ext_mem_free(pIBuf_b2);
	return PASS;
}

#endif


#if SLIDE_TRANSITION_PUSH
SWORD SlidePush(ST_IMGWIN * psSWin, BYTE * workingBuff, BYTE Type)	//Lighter 2009.10.12
{
	register WORD i,offset,offset1, WTemp = 0, limi;
	DWORD dwFrameStartTime;
	ST_IMGWIN sTWin;
	register ST_IMGWIN *psNWin, *psTWin, *psCWin;

	//SetDataCacheInvalid();
	dwFrameStartTime = GetSysTime();
	TransWinReset(psSWin, workingBuff);
	psTWin = &WinOriginal;

	//mpDebugPrint("psTWin 0x%x, 0x%x",psTWin,psTWin->pdwStart);

	switch(Type)
	{
		//PUSH_LEFT_RIGHT
		case 0:
			offset = psTWin->wWidth/PUSH_H_STEP;
			limi = psTWin->wWidth;
			break;
		//PUSH_RIGHT_LEFT
		case 1:
			offset = psTWin->wWidth/PUSH_H_STEP;
			limi = psTWin->wWidth;
			break;
		//PUSH_TOP_DOWN
		case 2:
			offset = psTWin->wHeight/PUSH_V_STEP;
			limi = psTWin->wHeight;
			break;
		//PUSH_DOWN_TOP
		case 3:
			offset = psTWin->wHeight/PUSH_V_STEP;
			limi = psTWin->wHeight;
			break;
	}

	if(offset < 2)
	{
		offset = 2;
	}
	else
	{
		offset = ALIGN_2(offset);
	}

	offset1 = offset;
	//offset = 4;
	WTemp = offset<<1;//offset;

	do
	{
		psNWin = Idu_GetNextWin();
		psCWin = Idu_GetCurrWin();

		switch(Type)
		{
			//PUSH_LEFT_RIGHT
			case 0:
				__Block_Copy(psCWin,psNWin,WTemp-offset,0,psNWin->wWidth-WTemp,psNWin->wHeight,WTemp,0);
				__Block_Copy(psTWin,psNWin,psNWin->wWidth-WTemp,0,WTemp,psNWin->wHeight,0,0);
				break;
			//PUSH_RIGHT_LEFT
			case 1:
				__Block_Copy(psCWin,psNWin,offset,0,psNWin->wWidth-WTemp,psNWin->wHeight,0, 0);
				__Block_Copy(psTWin,psNWin,0,0,WTemp,psNWin->wHeight,psNWin->wWidth-WTemp, 0);
				break;
			//PUSH_TOP_DOWN
			case 2:
				__Block_Copy(psCWin,psNWin,0,WTemp-offset,psNWin->wWidth,psNWin->wHeight-WTemp,0, WTemp);
				__Block_Copy(psTWin,psNWin,0,psNWin->wHeight-WTemp,psNWin->wWidth,WTemp,0, 0);
				break;
			//PUSH_DOWN_TOP
			case 3:
				__Block_Copy(psCWin,psNWin,0,offset,psNWin->wWidth,psNWin->wHeight-WTemp,0, 0);
				__Block_Copy(psTWin,psNWin,0,0,psNWin->wWidth,WTemp,0, psNWin->wHeight-WTemp);
				break;
		}

		Idu_ChgWin(psNWin);
		// delay for a while and polling event
		if (Polling_Event())
			return FAIL;

		if((WTemp<PUSH_SPEED_LINE)||(WTemp>(limi-PUSH_SPEED_LINE)))
		{
			offset = 2;
		}
		else if((WTemp<(PUSH_SPEED_LINE<<2))||(WTemp>(limi-(PUSH_SPEED_LINE<<2))))
		{
			offset = ALIGN_2(offset1>>2);
		}
		else if((WTemp<(PUSH_SPEED_LINE<<2))||(WTemp>(limi-(PUSH_SPEED_LINE<<2))))
		{
			offset = ALIGN_2(offset1>>1);
		}
		else
		{
			offset = offset1;
		}

		WTemp += offset;

	}while(WTemp < limi);

	Slide_WinCopy(psTWin, psNWin);

	Idu_ChgWin(psNWin);

	return PASS;
}

#endif

//------------------------------------------------------------------------------
#if SLIDE_TRANSITION_GRID
// -----------------------------------------------------------
// Grid Effect
// jeffery 080918
// -----------------------------------------------------------

ST_IMGWIN * g_pTempWin;
void (* g_func_GridDraw)(int,int,int);

/***** GRID *****/ // in slideEffect.h
//#define GRID_ROWS 		4
//#define GRID_COLUMES    8
//#define GRID_LINEWIDTH  4
WORD wGridPx=412;   // 0
WORD wGridPy=92;   // 0
WORD wGridWidth=384; //pWin->wWidth;
WORD wGridHeight=400; //pWin->wHeight;

void __WinGrayout()
{
    ST_IMGWIN * pWin = Idu_GetCurrWin();

	DWORD * src = pWin->pdwStart;
	int i;

	for (i = 0;i <  (pWin->wWidth * pWin->wHeight)/2;i++)
	{
		*src = ((((*src) >> 1) & (0x7f7f0000)) << 1) | 0x00008080;
		src++;
	}
}

void DrawGridLines(ST_IMGWIN *pWin)
{
    MP_DEBUG("%s", __FUNCTION__);
	int i;
	WORD x = wGridPx;
	WORD y = wGridPy;
	WORD w = wGridWidth; //pWin->wWidth;
	WORD h = wGridHeight; //pWin->wHeight;
	int t;

	t = h / GRID_ROWS;
	for(i=1;i < GRID_ROWS;i++)
	{
		//Idu_PaintWinArea(pWin, 0, i*t - GRID_LINEWIDTH/2,w, GRID_LINEWIDTH/2, GRID_LINECOLOR);
		Idu_PaintWinArea(pWin, x, i*t - GRID_LINEWIDTH/2, w, GRID_LINEWIDTH/2, GRID_LINECOLOR);
		//Idu_PaintWinArea(pWin, 0, i*t ,w, GRID_LINEWIDTH/2, GRID_LINECOLOR2);
		Idu_PaintWinArea(pWin, x, i*t ,w, GRID_LINEWIDTH/2, GRID_LINECOLOR2);
	}

	t = w / GRID_COLUMES;
	mpDebugPrint("t = w / GRID_COLUMNS = %d", t);
	for (i =1;i < GRID_COLUMES; i++)
	{
		//Idu_PaintWinArea(pWin, i*t - GRID_LINEWIDTH/2, 0, GRID_LINEWIDTH/2, h, GRID_LINECOLOR);
		Idu_PaintWinArea(pWin, x + i*t - GRID_LINEWIDTH/2, y, GRID_LINEWIDTH/2, h, GRID_LINECOLOR);
		//Idu_PaintWinArea(pWin, i*t , 0, GRID_LINEWIDTH/2, h, GRID_LINECOLOR2);
		Idu_PaintWinArea(pWin, x + i*t , y, GRID_LINEWIDTH/2, h, GRID_LINECOLOR2);
	}
	
}

void __Img_Scale(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin, int x, int y,int w,int h, int sw, int sh, int tw, int th)
{
	Ipu_ImageScaling(pSrcWin, pTrgWin
			  , x + ((w - sw) >>1)
		      , y + ((h - sh) >>1)
		      , x + ((w + sw) >>1)
		      , y + ((h + sh) >>1)
		      , x + ((w - tw) >>1)
		      , y + ((h - th) >>1)
		      , x + ((w + tw) >>1)
		      , y + ((h + th) >>1),0);

}

void Draw_Grid(int x, int y, int step)
{
    MP_DEBUG("%s", __FUNCTION__);
	ST_IMGWIN * pWin = Idu_GetCurrWin();
	ST_IMGWIN * pSrcWin = Idu_GetNextWin();

	int w = ALIGN_2(wGridWidth / GRID_COLUMES); //ALIGN_2(pWin->wWidth / GRID_COLUMES);
	int h = wGridHeight / GRID_ROWS; //  pWin->wHeight / GRID_ROWS;
	BYTE flag = (x == (GRID_COLUMES-1))? 1: 0;

	x *= w;
	y *= h;

	if(flag)
	{
		w = pWin->wWidth - x;
	}
	int tw = (step * w)/EFFECT_STEP;
	int th = (step * h)/EFFECT_STEP;

	__Img_Scale(pSrcWin,pWin, x,y,w,h, w,h, tw,th);

}

void __Draw_OneGrid(ST_IMGWIN *pWin,int x,int y, int w, int h,int sw,int sh,int tw,int th)
{
    MP_DEBUG("%s", __FUNCTION__);
	Idu_PaintWin(g_pTempWin,COLOR_BACKGROUND);

	Ipu_ImageScaling(pWin,g_pTempWin
		  		, x + ((w - sw) >>1)
	      		, y + ((h - sh) >>1)
	      		, x + ((w + sw) >>1)
	      		, y + ((h + sh) >>1)
	      		, ((w - tw) >>1)
	      		, ((h - th) >>1)
	      		, ((w + tw) >>1)
	      		, ((h + th) >>1),0);

	__Block_Copy(g_pTempWin,pWin,0,0,w,h,x,y);

}

void Draw_Grid_2(int x, int y, int step)
{
    MP_DEBUG("%s", __FUNCTION__);
	ST_IMGWIN * pWin = Idu_GetCurrWin();
	ST_IMGWIN * pNextWin = Idu_GetNextWin();

	int w = ALIGN_2(wGridWidth / GRID_COLUMES); //ALIGN_2(pWin->wWidth / GRID_COLUMES);
	int h = wGridHeight / GRID_ROWS; //pWin->wHeight / GRID_ROWS;
	BYTE flag = (x == (GRID_COLUMES-1))? 1: 0;

	x *= w;
	y *= h;

	if(flag)
	{
		w = pWin->wWidth - x;
	}

    int sw,sh,tw,th;
	int temp;

	if(step < HALF_STEP)
	{
		temp = HALF_STEP - step;
		tw = (temp * w)/HALF_STEP;
		th = (temp * h)/HALF_STEP;

		sw = ((temp+1) * w)/HALF_STEP;
		sh = ((temp+1) * h)/HALF_STEP;

		__Draw_OneGrid(pWin,x,y,w,h,sw,sh,tw,th);

	}
	else if (step == HALF_STEP)
	{
		Idu_PaintWinArea(pWin, x, y, w,h, COLOR_BACKGROUND);
	}
	else
	{
		tw = ((step- HALF_STEP) * w)/HALF_STEP;
		th = ((step- HALF_STEP) * h)/HALF_STEP;
		__Img_Scale(pNextWin, pWin, x, y, w, h
		      		, w,h,tw,th);
	}
}

void Draw_GridFlip(int x, int y, int step)
{
    MP_DEBUG("%s", __FUNCTION__);
	ST_IMGWIN * pWin = Idu_GetCurrWin();
	ST_IMGWIN * pNextWin = Idu_GetNextWin();

	int w = ALIGN_2(wGridWidth / GRID_COLUMES); //ALIGN_2(pWin->wWidth / GRID_COLUMES);
	int h = wGridHeight / GRID_ROWS; //pWin->wHeight / GRID_ROWS;
	BYTE flag = (x == (GRID_COLUMES-1))? 1: 0;

	x *= w;
	y *= h;

	if(flag)
	{
		w = pWin->wWidth - x;
	}
    int sw,sh,tw,th;
	int temp;

	if(step < HALF_STEP)
	{
		temp = HALF_STEP - step;
		tw = (temp * w)/HALF_STEP;
		th = h;

		sw = ((temp+1) * w)/HALF_STEP;
		sh = h;

		__Draw_OneGrid(pWin,x,y,w,h,sw,sh,tw,th);

        }
	else if (step == HALF_STEP)
        {
		Idu_PaintWinArea(pWin, x, y, w,h, COLOR_BACKGROUND);
	}
	else
	{
		tw = ((step- HALF_STEP) * w)/HALF_STEP;
		__Img_Scale(pNextWin, pWin, x, y, w, h
		      		, w,h,tw,h);
	}
}

void Draw_GridFlip_2(int x, int y, int step)
{
    MP_DEBUG("%s", __FUNCTION__);
	ST_IMGWIN * pWin = Idu_GetCurrWin();
	ST_IMGWIN * pNextWin = Idu_GetNextWin();

	int w = ALIGN_2(wGridWidth / GRID_COLUMES); //ALIGN_2(pWin->wWidth / GRID_COLUMES);
	int h = wGridHeight / GRID_ROWS; //pWin->wHeight / GRID_ROWS;
	BYTE flag = (x == (GRID_COLUMES-1))? 1: 0;

	x *= w;
	y *= h;

	if(flag)
	{
		w = pWin->wWidth - x;
	}
    int sw,sh,tw,th;
	int temp;

	if (step < HALF_STEP)
	{
		temp = HALF_STEP - step;
		tw = w;
		th = (temp * h)/HALF_STEP;

		sw = w;
		sh = ((temp+1) * h)/HALF_STEP;

		__Draw_OneGrid(pWin,x,y,w,h,sw,sh,tw,th);

	}
	else if (step == HALF_STEP)
	{
		Idu_PaintWinArea(pWin, x, y, w,h, COLOR_BACKGROUND);
	}
	else
	{
		th = ((step- HALF_STEP) * h)/HALF_STEP;
		__Img_Scale(pNextWin, pWin, x, y, w, h
		      		, w,h,w,th);
	}
}

// ------------------------------------------------------------------------
int grid_arry[GRID_COLUMES][GRID_ROWS];

void Grid_Init()
{
    MP_DEBUG("%s", __FUNCTION__);
	int x,y;
	#if 0	// Random block take too long time
	{
		for (x = 0;x < GRID_COLUMES; x++)
		{
			for(y= 0;y < GRID_ROWS; y++)
			{
				grid_arry[x][y] = RandomGen() % 16 + 2;
			}
		}
	}
	#else
	{
		for(y= 0;y < GRID_ROWS; y++)
		{
			for (x = 0;x < GRID_COLUMES; x++)
			{
				grid_arry[x][y] = y*GRID_COLUMES+x;
			}
		}
	}
	#endif
}

BOOL __Grid_OneStep(int step)
{
    MP_DEBUG("%s", __FUNCTION__);
	int x,y,temp;
	DWORD dwStartTime;
	for (x = 0;x < GRID_COLUMES; x++)
	{
		for(y= 0;y < GRID_ROWS; y++)
		{
			dwStartTime = GetSysTime();
			temp = step - grid_arry[x][y];
			if (temp >= 1 && temp <= EFFECT_STEP)
			{
				g_func_GridDraw(x,y,temp);
				if (__SlideReturn(dwStartTime, GRID_DELAY))
					return FALSE;
			}
		}
	}
	return TRUE;
}

void __Grid_SelectDrawFunc(int n)
{
    MP_DEBUG("%s", __FUNCTION__);
	switch (n)
	{
		case 1:
			g_func_GridDraw = Draw_Grid;
			break;
		case 2:
			g_func_GridDraw = Draw_GridFlip;
			break;
		case 3:
			g_func_GridDraw =Draw_Grid_2;
			break;
		default:
			g_func_GridDraw = Draw_GridFlip_2;
	}
}

SWORD Effect_Grid(BYTE * workingBuff)
{
    MP_DEBUG("%s", __FUNCTION__);
	int i,step;
	int swRet = FAIL;
	DWORD dwStartTime = GetSysTime();

	ST_IMGWIN * pWin = Idu_GetNextWin();

    //TransWinReset(pWin, workingBuff);
#if 1    
    wGridPx=0;
    wGridPy=0;
    wGridWidth=pWin->wWidth;
    wGridHeight=pWin->wHeight;    
#endif

	int w = ALIGN_2(wGridWidth / GRID_COLUMES); //ALIGN_2(pWin->wWidth / GRID_COLUMES);
	int h = wGridHeight / GRID_ROWS; //pWin->wHeight / GRID_ROWS;

	g_pTempWin = Win_New(w,h);
	if (!g_pTempWin)
		goto GRID_RELEASE;

	__Grid_SelectDrawFunc(RandomGen() % 4);
	Grid_Init();
	//__WinGrayout();
	//DrawGridLines(pWin);
	DrawGridLines(Idu_GetCurrWin());
	step = EFFECT_STEP + GRID_COLUMES*GRID_ROWS;

	if (__SlideReturn(dwStartTime ,SGRID_DELAY))
		goto GRID_RELEASE;

	for(i = 0; i < step ; i++)
	{
		dwStartTime = GetSysTime();
		if (!__Grid_OneStep(i)) //jeffery 081013
			goto GRID_RELEASE;

		if (__SlideReturn(dwStartTime ,SGRID_DELAY))
			goto GRID_RELEASE;
	}
	//Slide_WinCopy(&WinOriginal,pWin);
	Idu_ChgWin(pWin);
	swRet = PASS;

GRID_RELEASE:
	Win_Free(g_pTempWin);
	return PASS;
}

#endif


