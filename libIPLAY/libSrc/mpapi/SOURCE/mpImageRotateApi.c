

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/

#include "global612.h"
#include "mpTrace.h"
#include "display.h"


///
///@ingroup ImageRotation
///@brief Rotate 90 degree clockwise
///
///@param ST_IMGWIN * target          : Target image window
///@param	ST_IMGWIN * source        : Source image window
///
///@retval NONE
///
///@remark  Need to allocate a buffer for transition
///
void Img_Rotate90(ST_IMGWIN * target, ST_IMGWIN * source)
{
	register ST_PIXEL *ip0, *ip1, *op;
	register DWORD cb, cr;
	int height, width, line;
	ST_IMGWIN tmpWin;

	tmpWin.pdwStart = target->pdwStart;

	width = source->wHeight;
	height = source->wWidth;
	tmpWin.dwOffset = target->dwOffset;
	tmpWin.wWidth = width;
	tmpWin.wHeight = height;

	ip0 = (ST_PIXEL *) source->pdwStart;

	while (width > 0)
	{
		line = height;
		ip1 = ip0 + (source->dwOffset >> 2);
		op = (ST_PIXEL *) (tmpWin.pdwStart + ((width >> 1) - 1));

		while (line > 0)
		{
			cb = (ip0->Cb + ip1->Cb) >> 1;
			cr = (ip0->Cr + ip1->Cr) >> 1;

			op->Y0 = ip1->Y0;
			op->Y1 = ip0->Y0;
			op->Cb = cb;
			op->Cr = cr;
			op += (tmpWin.dwOffset >> 2);

			op->Y0 = ip1->Y1;
			op->Y1 = ip0->Y1;
			op->Cb = cb;
			op->Cr = cr;
			op += (tmpWin.dwOffset >> 2);

			ip0++;
			ip1++;
			line -= 2;
		}

		ip0 = ip0 - (source->wWidth >> 1) + (source->dwOffset >> 1);
		width -= 2;
	}
}

///
///@ingroup ImageRotation
///@brief Rotate 90 degree counter-clockwise, i.e. rotate 270 degree clockwise
///
///@param ST_IMGWIN * target          : Target image window
///@param	ST_IMGWIN * source        : Source image window
///
///@retval NONE
///
///@remark  Need to allocate a buffer for transition
///
void Img_Rotate270(ST_IMGWIN * target, ST_IMGWIN * source)
{
	register ST_PIXEL *ip0, *ip1, *op;
	register DWORD cb, cr;
	ST_PIXEL *outputLineStart;
	int height, width, line;
	ST_IMGWIN tmpWin;

	tmpWin.pdwStart = target->pdwStart;

	width = source->wHeight;
	height = source->wWidth;
	tmpWin.dwOffset = target->dwOffset;
	tmpWin.wWidth = width;
	tmpWin.wHeight = height;

	outputLineStart =
		(ST_PIXEL *) (tmpWin.pdwStart + ((tmpWin.dwOffset >> 1) * (tmpWin.wHeight - 1) >> 1));

	ip0 = (ST_PIXEL *) source->pdwStart;

	while (width > 0)
	{
		line = height;
		ip1 = ip0 + (source->dwOffset >> 2);
		op = outputLineStart;

		while (line > 0)
		{
			cb = (ip0->Cb + ip1->Cb) >> 1;
			cr = (ip0->Cr + ip1->Cr) >> 1;

			op->Y0 = ip0->Y0;
			op->Y1 = ip1->Y0;
			op->Cb = cb;
			op->Cr = cr;
			op -= (tmpWin.dwOffset >> 2);

			op->Y0 = ip0->Y1;
			op->Y1 = ip1->Y1;
			op->Cb = cb;
			op->Cr = cr;
			op -= (tmpWin.dwOffset >> 2);

			ip0++;
			ip1++;
			line -= 2;
		}

		ip0 = ip0 - (source->wWidth >> 1) + (source->dwOffset >> 1);
		width -= 2;
		outputLineStart++;
	}
}

///
///@ingroup ImageRotation
///@brief Rotate 180 degree clockwise
///
///@param ST_IMGWIN * target          : Target image window
///@param	ST_IMGWIN * source        : Source image window
///
///@retval NONE
///
///@remark  Need to allocate a buffer for transition
///
void Img_Rotate180(ST_IMGWIN * target, ST_IMGWIN * source)
{
	register ST_PIXEL *ip, *op;
	register int pixel, line;

	target->wWidth = source->wWidth;
	target->wHeight = source->wHeight;
	target->dwOffset = source->dwOffset;

	pixel = target->wHeight * target->wWidth;
	line = source->wWidth;
	ip = (ST_PIXEL *) source->pdwStart;
	op = (ST_PIXEL *) (target->pdwStart + (pixel / 2) - 1);

	while (pixel > 0)
	{
		op->Y0 = ip->Y1;
		op->Y1 = ip->Y0;
		op->Cb = ip->Cb;
		op->Cr = ip->Cr;

		ip++;
		op--;
		pixel -= 2;
	}
}

