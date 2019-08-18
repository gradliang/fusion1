#if (0 == PREVIEW_FLAT_VIDEO_FRAME)	

#include "global612.h"
#include "mptrace.h"


void YYCbCr_to_Y(unsigned char *yycbcr_image_buf, unsigned char *y_image_buf, int width, int height)
{ 
	int i, j;
	j = 0;
    
	for (i=0; i < 2*width*height; i++)
	{
	    //mpDebugPrint("i = %d", i);
	  	if (i%4 <= 1)
	  	{
	  	    *(y_image_buf + j) = *(yycbcr_image_buf + i);
	  	    j++;
	  	}
	}
}



int Edge_Detection(unsigned char* input_img_buf, unsigned short cols, unsigned short rows)
{   
   unsigned char* y_img_buf;
   unsigned int		x, y;
   int			i, j;
   long			sum;
   int			mask[5][5];
   unsigned int non_zero_num = 0;

  // mpDebugPrint("cols = %d, rows = %d", cols, rows);

   // mask of Laplacian
   mask[0][0] = -1; mask[0][1] = -1; mask[0][2] = -1; mask[0][3] = -1; mask[0][4] = -1;
   mask[1][0] = -1; mask[1][1] = -1; mask[1][2] = -1; mask[1][3] = -1; mask[1][4] = -1;
   mask[2][0] = -1; mask[2][1] = -1; mask[2][2] = 24; mask[2][3] = -1; mask[2][4] = -1;
   mask[3][0] = -1; mask[3][1] = -1; mask[3][2] = -1; mask[3][3] = -1; mask[3][4] = -1;
   mask[4][0] = -1; mask[4][1] = -1; mask[4][2] = -1; mask[4][3] = -1; mask[4][4] = -1;

   y_img_buf = (unsigned char *) mem_malloc(rows*cols);
   if (NULL == y_img_buf)
   {
	    return -1;
   }
   YYCbCr_to_Y(input_img_buf, y_img_buf, cols, rows);
   
   for (y=0; y<=(rows-1); y++)
   {
       //mpDebugPrint("y = %d", y);
	   for (x=0; x<=(cols-1); x++)
       {
	        sum = 0;

	        /* image boundaries */
	        if (y==0 || y==1 || y==rows-2 || y==rows-1)
		         sum = 0;
	        else if(x==0 || x==1 || x==cols-2 || x==cols-1)
		         sum = 0;

	        /* Convolution starts here */
	        else
           {
	            for (i=-2; i<=2; i++)
               {
		            for (j=-2; j<=2; j++)
                  {
		                sum = sum + (int)( (*(y_img_buf + x + i + (y + j)*cols)) * mask[i+2][j+2]);

		            }
	            }
	        }
	     if(sum>255)  sum=255;
	     if(sum<0)    sum=0;

        if (255 == sum)
            non_zero_num++;
	    }
   }
   //mpDebugPrint("non_zero_num = %d", non_zero_num);
   
   mem_free(y_img_buf);
   
   return non_zero_num;
}
#endif

