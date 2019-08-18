///
///@ingroup		Image
///@defgroup	TIFF	TIFF
///
///	The APIs for TIFF Baseline file decoding
///
///	This file includes all the necessary interfaces to decode a TIFF file
///
#ifndef __TIFF_h
#define __TIFF_h

#include "global612.h"

#define	NOT_TIFF_FILE				-1
#define	TIFF_IMG_IFD_ERR				-2
#define	NOT_SUPPROTED_TIFF			-3
#define    NOT_SUPPORTED_TIFF_SIZE   	-4

#define TIFF_IDENTIFICATION			0x002A
//
// Define the IFD TAG
//
#define	TIFFTAG_IMAGEWIDTH		256	/*0x100*/	/* image width in pixels */	
#define	TIFFTAG_IMAGELENGTH		257	/*0x101*/	/* image height in pixels */
#define	TIFFTAG_BITSPERSAMPLE	258	/*0x102*/	/* bits per channel (sample) */
#define	TIFFTAG_COMPRESSION		259	/*0x103*/	/* data compression technique */
#define	    COMPRESSION_NONE			1		/* dump mode */
#define	    COMPRESSION_CCITTRLE		2		/* CCITT modified Huffman RLE */
#define	    COMPRESSION_CCITTFAX3		3		/* CCITT Group 3 fax encoding */
#define        COMPRESSION_CCITT_T4        	3      		 /* CCITT T.4 (TIFF 6 name) */
#define	    COMPRESSION_CCITTFAX4		4		/* CCITT Group 4 fax encoding */
#define        COMPRESSION_CCITT_T6        	4       	/* CCITT T.6 (TIFF 6 name) */
#define	    COMPRESSION_LZW			5       	/* Lempel-Ziv  & Welch */
#define	    COMPRESSION_OJPEG			6		/* !6.0 JPEG */
#define	    COMPRESSION_JPEG			7		/* %JPEG DCT compression */
#define	    COMPRESSION_NEXT			32766	/* NeXT 2-bit RLE */
#define	    COMPRESSION_CCITTRLEW	32771	/* #1 w/ word alignment */
#define	    COMPRESSION_PACKBITS		32773	/* Macintosh RLE */
#define	    COMPRESSION_THUNDERSCAN	32809	/* ThunderScan RLE */

#define	TIFFTAG_PHOTOMETRIC		262	/* photometric interpretation */	//0x106
#define	    PHOTOMETRIC_MINISWHITE	0		/* min value is white */	//BW
#define	    PHOTOMETRIC_MINISBLACK	1		/* min value is black */	//gray
#define	    PHOTOMETRIC_RGB			2		/* RGB color model */
#define	    PHOTOMETRIC_PALETTE		3		/* color map indexed */
#define	    PHOTOMETRIC_MASK			4		/* $holdout mask */
#define	    PHOTOMETRIC_SEPARATED	5		/* !color separations */
#define	    PHOTOMETRIC_YCBCR		6		/* !CCIR 601 */
#define	    PHOTOMETRIC_CIELAB		8		/* !1976 CIE L*a*b* */
#define	    PHOTOMETRIC_ICCLAB		9		/* ICC L*a*b* [Adobe TIFF Technote 4] */
#define	    PHOTOMETRIC_ITULAB		10		/* ITU L*a*b* */
#define        PHOTOMETRIC_LOGL			32844	/* CIE Log2(L) */
#define        PHOTOMETRIC_LOGLUV		32845	/* CIE Log2(L) (u',v') */

#define	TIFFTAG_STRIPOFFSETS		273	/*0x111*//* offsets to data strips */	
#define	TIFFTAG_ORIENTATION		274	/*0x112*//* +image orientation */
#define	    ORIENTATION_TOPLEFT		1	/* row 0 top, col 0 lhs */
#define	    ORIENTATION_TOPRIGHT		2	/* row 0 top, col 0 rhs */
#define	    ORIENTATION_BOTRIGHT		3	/* row 0 bottom, col 0 rhs */
#define	    ORIENTATION_BOTLEFT		4	/* row 0 bottom, col 0 lhs */
#define	    ORIENTATION_LEFTTOP		5	/* row 0 lhs, col 0 top */
#define	    ORIENTATION_RIGHTTOP		6	/* row 0 rhs, col 0 top */
#define	    ORIENTATION_RIGHTBOT		7	/* row 0 rhs, col 0 bottom */
#define	    ORIENTATION_LEFTBOT		8	/* row 0 lhs, col 0 bottom */

#define	TIFFTAG_SAMPLESPERPIXEL	277	/*0x115*//* samples per pixel */

#define	TIFFTAG_ROWSPERSTRIP	278	/*0x116*//* rows per strip of data */
#define	TIFFTAG_STRIPBYTECOUNTS	279	/*0x117*//* bytes counts for strips */

#define	TIFFTAG_MINSAMPLEVALUE	280	/* +minimum sample value */
#define	TIFFTAG_MAXSAMPLEVALUE	281	/* +maximum sample value */
#define	TIFFTAG_XRESOLUTION		282	/*0x11A*/		/* pixels/resolution in x */
#define	TIFFTAG_YRESOLUTION		283	/*0x11B*/		/* pixels/resolution in y */

#define	TIFFTAG_PLANARCONFIG	284	/*0x11C*/		/* storage organization */
#define	    PLANARCONFIG_CONTIG		1	/* single image plane */
#define	    PLANARCONFIG_SEPARATE		2	/* separate planes of data */

#define	TIFFTAG_RESOLUTIONUNIT	296	/* units of resolutions */
#define	    RESUNIT_NONE				1	/* no meaningful units */
#define	    RESUNIT_INCH				2	/* english */
#define	    RESUNIT_CENTIMETER		3	/* metric */
#define	TIFFTAG_PAGENUMBER		297	/* page numbers of multi-page */
#define	TIFFTAG_COLORMAP		320	/* RGB map for pallette image */

#define TIFFTAG_SubIFDs          330 /*0x014A Offset to child IFDs,ARW file maybe the offset of BitStream data*/
#define TIFFTAG_JPEGIFOFFSET     513 /*0x0201 JPEGInterChangeFormate*/
#define TIFFTAG_JPEGIFBYTECOUNT  514 /*0x0514*/

#define TIFFTAG_YCBCRSUBSAMPLING 530 /*Specifies the subsampling factors used for the chrominance components of a YCbCr image.*/
#define TIFFTAG_ExifIFD          34665 /*0x8769 Offset ,subIFD*/

/*-----------------------------------------------------*/
#define MEM_BASE_SIZE 8
/*need  Totoal memory size = 3*iRowBytes*MEM_BASE_SIZE*wScale */


typedef struct{
	DWORD	dwDirectoryEntries;		
	
	DWORD	dwImageWidth;		//0x0100
	DWORD	dwImageHeight;		//0101
	
	DWORD	wSampleCount;		//0102	//sample count
	DWORD	wBitsPerSample;		//0102	//value or offset

	DWORD	dwCompression;		//0x0103
	WORD	PhotoMetric;			//0x0106
	
	DWORD    dwStripCount;			
	DWORD    dwStripOffsets;		//0x0111
	DWORD    dwRowPerStrip;		//0x116
	DWORD    dwStripByteCountOffset;		//0x117
	
	DWORD 	dwOrientation;			//0x0112
	DWORD    dwSamplesPerPixel;		//0x0115
	DWORD    dwXResolution;		//0x011a
	DWORD	dwYResolution;		//0x011b
	DWORD	dwPlanarConfiguration;	//0x11c
	DWORD    dwResolutionUnit;		//0x0128
	DWORD    dwColorMap;			//0x0140
	DWORD 	dwByteOrder;			//neil add for big endian 20080317	

                    //Frank Lin add for TIFFTAG_YCBCRSUBSAMPLING
    DWORD   dwSubIFDsOffset;/*for ARW 0x014A*/
	DWORD   dwJPEGIFOffset;/*0x0201*/
	DWORD   dwJPEGIFByteCount;/*0x202*/
	DWORD   dwYCbCrSubsampleHorizVert;   
	
}ST_TIFF_INFO;

typedef struct{
	
	WORD 	wTempTAG;
	WORD 	wValueType;
	DWORD 	dwValueCount;
	DWORD	dwValue;
}ST_TIFF_IFD_TAG;

int TIFF_Decoder_Init(IMAGEFILE *psImage);
int TIFF_Decoder_DecodeImage(IMAGEFILE *psImage);
#endif


