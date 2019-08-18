#include "global612.h"
#include "mpTrace.h"
#include "Windef.h"
#define SUB_SAMPLE 2
  //pick 1 pixel per SUM_SAMPLE in one direction; i,e, 1/(SUB_SAMBLE*SUB_SAMPLE) in total
#define BLOCK_LEN 8
#define TH_S 16
#define TH_M 4

int motion_detect(ST_IMGWIN *prevWin, ST_IMGWIN * trgWin)
{
	unsigned char *trgBuffer, *prevBuffer, *adBuffer;	
	int sum_D = 0, avg_D = 0;
	int sum_block = 0;
	int i = 0, j = 0, x = 0, y = 0;
	int	frame_size = trgWin->wHeight * trgWin->wWidth;
	int num_of_moving_blk = 0;

	trgBuffer = (unsigned char *)trgWin->pdwStart;	
	prevBuffer = (unsigned char *)prevWin->pdwStart;	
	adBuffer = (unsigned char *)mem_malloc(frame_size/(SUB_SAMPLE*SUB_SAMPLE));
	if(frame_size % (SUB_SAMPLE*SUB_SAMPLE))
		mpDebugPrint("motion detection: The value of SUB_SAMPLE is too large");

	//Compute the absolute difference of subsampled pixels and store the result in adBuffer[]
	for(i = 0; i < trgWin->wHeight; i+=SUB_SAMPLE)
	{
		for(j = 0; j < trgWin->wWidth; j+=SUB_SAMPLE)
		{
#if (SUB_SAMPLE != 0 && !(SUB_SAMPLE % 2))
			adBuffer[(i*trgWin->wWidth)/(SUB_SAMPLE*SUB_SAMPLE) + j/SUB_SAMPLE] = abs(trgBuffer[2*(i*trgWin->wWidth + j)] - prevBuffer[2*(i*trgWin->wWidth + j)]);
#else
			mpDebugPrint("motion_detection: not implemented");
#endif
			sum_D += adBuffer[(i*trgWin->wWidth)/(SUB_SAMPLE*SUB_SAMPLE) + j/SUB_SAMPLE];
		}
	}
	//avg_D: the average of absolute difference	
	avg_D = sum_D / (frame_size/(SUB_SAMPLE*SUB_SAMPLE));

	for(i = 0; i < trgWin->wHeight; i += BLOCK_LEN)
	{
		for(j = 0; j < trgWin->wWidth; j += BLOCK_LEN)
		{
			sum_block = 0;
			for(x = 0; x < BLOCK_LEN; x += SUB_SAMPLE)
			{
				for(y = 0; y < BLOCK_LEN; y += SUB_SAMPLE)
				{
					//sum_block: the sum of absolute differeces of a block					
					sum_block += adBuffer[(i+x)*trgWin->wWidth/(SUB_SAMPLE*SUB_SAMPLE)+ (j+y)/SUB_SAMPLE];
				}
			}
			if(sum_block > max(avg_D * (BLOCK_LEN*BLOCK_LEN) * TH_M / (SUB_SAMPLE*SUB_SAMPLE), TH_S*(BLOCK_LEN*BLOCK_LEN)/(SUB_SAMPLE*SUB_SAMPLE)))
				num_of_moving_blk++;
			else
				;
		}
	}
	
	mem_free(adBuffer);
	return num_of_moving_blk;	
}

