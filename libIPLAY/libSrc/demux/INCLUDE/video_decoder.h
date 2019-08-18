/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      video_decoder.h
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

#ifndef __VIDEO_DECODER_H
#define __VIDEO_DECODER_H

#include "stheader.h"
#include "mpc_info.h"
#include "display.h"

#ifndef FOURCC
#define FOURCC(a,b,c,d) ((a<<24)|(b<<16)|(c<<8)|(d))
#endif

/* Detail FOURCC list available at http://www.fourcc.org/codecs.php */
// modified by Andy, 2009/03/19
//#define JPEG			FOURCC('G', 'P', 'J', 'M')	/* Motion JPEG */
//#define jpeg			FOURCC('g', 'p', 'j', 'm')	/* Motion JPEG */            
#define JPEG			FOURCC('G', 'E', 'P', 'J')	/* JPEG */
#define jpeg            FOURCC('g', 'e', 'p', 'j')	/* JPEG */
#define mjpg            FOURCC('g', 'p', 'j', 'm')	/* Motion JPEG */ 
#define MJPG			FOURCC('G', 'P', 'J', 'M')	/* Motion JPEG */

//end of modified
#define dmb1              FOURCC('1', 'b', 'm', 'd')		/* Motion JPEG, by Matrox */
#define MPEG1		0x10000001	
#define MPEG2		0x10000002

#define DVR  FOURCC(' ', 'R', 'V', 'D')
#define hdv2 FOURCC('2', 'v', 'd', 'h')
#define mpg1 FOURCC('1', 'g', 'p', 'm')
#define PIM1 FOURCC('1', 'M', 'I', 'P')
#define VCR2 FOURCC('2', 'R', 'C', 'V')
#define mpg2 FOURCC('2', 'g', 'p', 'm')
#define MPG2 FOURCC('2', 'G', 'P', 'M')
#define MPEG FOURCC('G', 'E', 'P', 'M')
#define hdv3 FOURCC('3', 'v', 'd', 'h')
#define mx5p FOURCC('p', '5', 'x', 'm')
#define MMES FOURCC('S', 'E', 'M', 'M')
#define mmes FOURCC('s', 'e', 'm', 'm')


#define MPEG4_DIVX	FOURCC('X', 'V', 'I', 'D')	/* DivX v4 or later version */
#define MPEG4_divx	FOURCC('x', 'v', 'i', 'd')	/* DIVX */
#define MPEG4_DX50	FOURCC('0', '5', 'X', 'D')	/* DivX v5 */
#define MPEG4_XVID	FOURCC('D', 'I', 'V', 'X')	/* XVID */
#define MPEG4_xvid	FOURCC('d', 'i', 'v', 'x')	/* XVID */

#define DIV3			FOURCC('3', 'V', 'I', 'D')	/* Low motion codec, by DivX */
#define div3			FOURCC('3', 'v', 'i', 'd')	/* Low motion codec, by DivX */
#define DIV4			FOURCC('4', 'V', 'I', 'D')	/* Fast motion codec, by DivX */
#define div4			FOURCC('4', 'v', 'i', 'd')	/* Fast motion codec, by DivX */
#define MP43               FOURCC('3', '4', 'P', 'M')		/* another variation from Microsoft */
#define mp43              FOURCC('3', '4', 'p', 'm')

#define mp4v              FOURCC('v', '4', 'p', 'm')
#define MP4V							FOURCC('V', '4', 'P', 'M')
#define s263              FOURCC('3', '6', '2', 's')
#define h263              FOURCC('3', '6', '2', 'h')
#define FLV1              FOURCC('F', 'L', 'V', '1')
#define h264              FOURCC('4', '6', '2', 'h')
#define H264              FOURCC('4', '6', '2', 'H')
#define x264              FOURCC('4', '6', '2', 'x')
#define X264              FOURCC('4', '6', '2', 'X')
#define avc1              FOURCC('1', 'c', 'v', 'a')
#define AVC1              FOURCC('1', 'C', 'V', 'A')
#define davc              FOURCC('c', 'v', 'a', 'd')
#define DAVC              FOURCC('C', 'V', 'A', 'D')

#define M4S2              FOURCC('2', 'S', '4', 'M')

#define	MJPG_FOURCC(four_cc)	(four_cc == MJPG || four_cc == mjpg || four_cc == JPEG || four_cc == jpeg || four_cc == dmb1)
#define	H264_FOURCC(four_cc)	(four_cc == h264 || four_cc == H264 || four_cc == x264 || four_cc == X264 || four_cc == avc1 || four_cc == AVC1 || four_cc == davc || four_cc == DAVC)

#define CONTROL_UNKNOWN -1
#define IMGFMT_YV12 0x32315659


#define MP_MAX_PLANES	4

typedef struct
{
    unsigned short flags;
    unsigned char type;
    unsigned char bpp;  // bits/pixel. NOT depth! for RGB it will be n*8
    unsigned int imgfmt;
    int edged_y_width;//,edged_y_height;  //orig stored dimensions
    int edged_uv_width;//,edged_uv_height;  //orig stored dimensions 
    int x,y,display_width,display_height;  // display dimensions
    int interlace;
	
    unsigned char* planes[MP_MAX_PLANES];
    //DWORD * planes[MP_MAX_PLANES];
    unsigned int stride[5];
    
    char * qscale;
    int qstride;
    int pict_type; // 0->unknown, 1->I, 2->P, 3->B
    int fields;
    int qscale_type; // 0->mpeg1/4/h263, 1->mpeg2
    int num_planes;
    /* these are only used by planar formats Y,U(Cb),V(Cr) */
    int chroma_width;
    int chroma_height;
    int chroma_x_shift; // horizontal
    int chroma_y_shift; // vertical
    /* for private use by filter or vo driver (to store buffer id or dmpi) */
    void* priv;
	float pts;
	unsigned int picId;
} mp_image_t;

typedef mp_codec_info_t vd_info_t;

typedef struct
{
	vd_info_t *info;
	int (*init)(sh_video_t *sh, int image_count);
	void (*uninit)(sh_video_t *sh);
	int (*control)(sh_video_t *sh,int cmd,void* arg, ...);   
	int (*change_mode)(sh_video_t *sh);
	int (*decode)(mp_image_t  *mpi, int image_no, sh_video_t *sh, void* data, int len, int flags);
		
#ifdef DIRECT_RENDER
	mp_image_t* (*decode_render)(ST_IMGWIN *win,sh_video_t *sh,void* data,int len,int flags);
	void (*uninit_decode_render)(sh_video_t *sh);
#endif
} vd_functions_t;
//commod for control
#define	CMD_VD_CLOSED_GOP					0	///> command to get closed_gop/closed_gov in current gop/gov
#define	CMD_VD_PICTURE_CODING_TYPE			1	///> command to get coding type in codec definition
#define	CMD_VD_PICTURE_CODING_TYPE_DISPLAY	2	///> command to get coding type in display definition, not codec definition.  ref. picture_coding_type_d_T in avsync.c
#define	CMD_VD_PICTURE_CODING_TYPE_B		3	///> command to get if this is a B type
#define	CMD_VD_BROKEN_LINK					4	///> command to get broken_link in current gop/gov
#define	CMD_VD_RESET						5	///> command to get broken_link in current gop/gov
#define CMD_VD_FLUSH_OUTPUT_QUEUE           6   ///> command to flush the output queue of video decoders

typedef struct
{
   vd_functions_t  video_decoder;
} video_decoder_t;

extern video_decoder_t* init_best_video_codec(sh_video_t*);
extern int decode_video(mp_image_t *video_mpi,int frame_no,sh_video_t *sh_video,unsigned char *start,int in_size,int drop_frame);
int display_video(mp_image_t * const video_mpi, const int direct_render);
int display_video_bypass2dma(mp_image_t * const video_mpi);
extern void uninit_video_decode();
void reset_video_decoder();

#ifdef DIRECT_RENDER
extern void uninit_video_decode_render();
#endif
const unsigned char* next_start_pattern(const unsigned char *start, int len, const unsigned int pattern);
int control_video(sh_video_t * const sh_video, const int cmd, void *arg, const unsigned char * const start, const unsigned int len);

#define	MAX_VIDEO_DECODE_WIDTH		1280
#define	MAX_VIDEO_DECODE_HEIGHT		720
#define	VIDEO_DECODE_MEM_SWAP_WIDTH	1280

#endif

