/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2007 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description : Common Decoder API definitions
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: decapicommon.h,v $
--  $Date: 2009/03/11 14:27:34 $
--  $Revision: 1.12 $
--
------------------------------------------------------------------------------*/
#ifndef __DECAPICOMMON_H__
#define __DECAPICOMMON_H__

#include "basetype.h"

#define MPEG4_NOT_SUPPORTED             (u32)(0x00)
#define MPEG4_SIMPLE_PROFILE            (u32)(0x01)
#define MPEG4_ADVANCED_SIMPLE_PROFILE   (u32)(0x02)
#define H264_NOT_SUPPORTED              (u32)(0x00)
#define H264_BASELINE_PROFILE           (u32)(0x01)
#define H264_MAIN_PROFILE               (u32)(0x02)
#define H264_HIGH_PROFILE               (u32)(0x03)
#define VC1_NOT_SUPPORTED               (u32)(0x00)
#define VC1_SIMPLE_PROFILE              (u32)(0x01)
#define VC1_MAIN_PROFILE                (u32)(0x02)
#define VC1_ADVANCED_PROFILE            (u32)(0x03)
#define MPEG2_NOT_SUPPORTED             (u32)(0x00)
#define MPEG2_MAIN_PROFILE              (u32)(0x01)
#define JPEG_NOT_SUPPORTED              (u32)(0x00)
#define JPEG_BASELINE                   (u32)(0x01)
#define JPEG_PROGRESSIVE                (u32)(0x02)
#define PP_NOT_SUPPORTED                (u32)(0x00)
#define PP_SUPPORTED                    (u32)(0x01)
#define PP_DITHERING                    (u32)(0x10000000)
#define PP_SCALING                      (u32)(0x0C000000)
#define PP_DEINTERLACING                (u32)(0x02000000)
#define PP_ALPHA_BLENDING               (u32)(0x01000000)
#define SORENSON_SPARK_NOT_SUPPORTED    (u32)(0x00)
#define SORENSON_SPARK_SUPPORTED        (u32)(0x01)
#define VP6_NOT_SUPPORTED               (u32)(0x00)
#define VP6_SUPPORTED                   (u32)(0x01)
#define REF_BUF_NOT_SUPPORTED           (u32)(0x00)
#define REF_BUF_SUPPORTED               (u32)(0x01)
#define REF_BUF_INTERLACED              (u32)(0x02)
#define REF_BUF_DOUBLE                  (u32)(0x04)
#define AVS_NOT_SUPPORTED               (u32)(0x00)
#define AVS_SUPPORTED                   (u32)(0x01)
#define JPEG_EXT_NOT_SUPPORTED          (u32)(0x00)
#define JPEG_EXT_SUPPORTED              (u32)(0x01)
#define RV_NOT_SUPPORTED                (u32)(0x00)
#define RV_SUPPORTED                    (u32)(0x01)

#define H264_NOT_SUPPORTED_FUSE					 (u32)(0x00)
#define H264_FUSE_ENABLED						 (u32)(0x01)
#define MPEG4_NOT_SUPPORTED_FUSE				 (u32)(0x00)
#define MPEG4_FUSE_ENABLED						 (u32)(0x01)
#define MPEG2_NOT_SUPPORTED_FUSE				 (u32)(0x00)
#define MPEG2_FUSE_ENABLED						 (u32)(0x01)
#define SORENSON_SPARK_NOT_SUPPORTED_FUSE		 (u32)(0x00)
#define SORENSON_SPARK_ENABLED					 (u32)(0x01)
#define JPEG_NOT_SUPPORTED_FUSE					 (u32)(0x00)
#define JPEG_FUSE_ENABLED						 (u32)(0x01)
#define VP6_NOT_SUPPORTED_FUSE					 (u32)(0x00)
#define VP6_FUSE_ENABLED						 (u32)(0x01)
#define VC1_NOT_SUPPORTED_FUSE					 (u32)(0x00)
#define VC1_FUSE_ENABLED						 (u32)(0x01)
#define JPEG_PROGRESSIVE_NOT_SUPPORTED_FUSE		 (u32)(0x00)
#define JPEG_PROGRESSIVE_FUSE_ENABLED    		 (u32)(0x01)
#define REF_BUF_NOT_SUPPORTED_FUSE				 (u32)(0x00)
#define REF_BUF_FUSE_ENABLED					 (u32)(0x01)
#define AVS_NOT_SUPPORTED_FUSE		             (u32)(0x00)
#define AVS_FUSE_ENABLED		                 (u32)(0x01)
#define RV_NOT_SUPPORTED_FUSE	                 (u32)(0x00)
#define RV_FUSE_ENABLED		                     (u32)(0x01)

#define PP_NOT_SUPPORTED_FUSE					 (u32)(0x00)
#define PP_FUSE_ENABLED						     (u32)(0x01)
#define PP_FUSE_DEINTERLACING_ENABLED            (u32)(0x40000000)
#define PP_FUSE_ALPHA_BLENDING_ENABLED           (u32)(0x20000000)
#define MAX_PP_OUT_WIDHT_1920_FUSE_ENABLED       (u32)(0x00008000)
#define MAX_PP_OUT_WIDHT_1280_FUSE_ENABLED       (u32)(0x00004000)
#define MAX_PP_OUT_WIDHT_720_FUSE_ENABLED        (u32)(0x00002000)
#define MAX_PP_OUT_WIDHT_352_FUSE_ENABLED        (u32)(0x00001000)

/* Macro to copy support flags and picture max width from DWL HW config 
 * to Decoder HW config */
#define SET_DEC_BUILD_SUPPORT(decHwCfg, dwlHwCfg) \
    decHwCfg.maxDecPicWidth = dwlHwCfg.maxDecPicWidth; \
    decHwCfg.maxPpOutPicWidth = dwlHwCfg.maxPpOutPicWidth; \
    decHwCfg.h264Support = dwlHwCfg.h264Support; \
    decHwCfg.jpegSupport = dwlHwCfg.jpegSupport; \
    decHwCfg.jpegESupport = dwlHwCfg.jpegESupport; \
    decHwCfg.mpeg2Support = dwlHwCfg.mpeg2Support; \
    decHwCfg.mpeg4Support = dwlHwCfg.mpeg4Support; \
    decHwCfg.vc1Support = dwlHwCfg.vc1Support; \
    decHwCfg.ppSupport = dwlHwCfg.ppSupport; \
    decHwCfg.ppConfig = dwlHwCfg.ppConfig; \
    decHwCfg.sorensonSparkSupport = dwlHwCfg.sorensonSparkSupport; \
    decHwCfg.vp6Support = dwlHwCfg.vp6Support; \
//    decHwCfg.vp7Support = dwlHwCfg.vp7Support; \
//    decHwCfg.vp8Support = dwlHwCfg.vp8Support; \
    decHwCfg.refBufSupport = dwlHwCfg.refBufSupport; \
    decHwCfg.avsSupport = dwlHwCfg.avsSupport; \
    decHwCfg.rvSupport = dwlHwCfg.rvSupport; \
//    decHwCfg.customMpeg4Support = dwlHwCfg.customMpeg4Support; \
//    decHwCfg.mvcSupport = dwlHwCfg.mvcSupport;


typedef struct DecHwConfig_
{
    u32 mpeg4Support;        /* one of the MPEG4 values defined above */
    u32 h264Support;         /* one of the H264 values defined above */
    u32 vc1Support;          /* one of the VC1 values defined above */
    u32 mpeg2Support;        /* one of the MPEG2 values defined above */
    u32 jpegSupport;         /* one of the JPEG values defined above */
	u32 jpegProgSupport;     /* one of the Progressive JPEG values defined above */
    u32 maxDecPicWidth;      /* maximum picture width in decoder */
    u32 ppSupport;           /* PP_SUPPORTED or PP_NOT_SUPPORTED */
    u32 ppConfig;            /* Bitwise list of PP function */
    u32 maxPpOutPicWidth;    /* maximum post-processor output picture width */
    u32 sorensonSparkSupport;   /* one of the SORENSON_SPARK values defined above */
    u32 refBufSupport;       /* one of the REF_BUF values defined above */
    u32 vp6Support;          /* one of the VP6 values defined above */
    u32 avsSupport;          /* one of the AVS values defined above */
    u32 jpegESupport;        /* one of the JPEG EXT values defined above */    
    u32 rvSupport;           /* one of the HUKKA values defined above */
} DecHwConfig;

typedef struct DWLHwFuseStatus_
{
	u32 h264SupportFuse;     /* HW supports h.264 */
    u32 mpeg4SupportFuse;    /* HW supports MPEG-4 */
    u32 mpeg2SupportFuse;    /* HW supports MPEG-2 */
    u32 sorensonSparkSupportFuse;   /* HW supports Sorenson Spark */
	u32 jpegSupportFuse;     /* HW supports JPEG */
    u32 vp6SupportFuse;      /* HW supports VP6 */
    u32 vc1SupportFuse;      /* HW supports VC-1 Simple */
	u32 jpegProgSupportFuse; /* HW supports Progressive JPEG */
    u32 ppSupportFuse;       /* HW supports post-processor */
    u32 ppConfigFuse;        /* HW post-processor functions bitmask */
    u32 maxDecPicWidthFuse;  /* Maximum video decoding width supported  */
    u32 maxPpOutPicWidthFuse; /* Maximum output width of Post-Processor */
    u32 refBufSupportFuse;   /* HW supports reference picture buffering */
	u32 avsSupportFuse;      /* one of the AVS values defined above */
	u32 rvSupportFuse;       /* one of the HUKKA values defined above */

} DWLHwFuseStatus;


#endif /* __DECAPICOMMON_H__ */
