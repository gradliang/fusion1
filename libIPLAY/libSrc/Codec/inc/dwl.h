/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description : Sytem Wrapper Layer
--
------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: dwl.h,v $
--  $Revision: 1.14 $
--  $Date: 2009/03/11 14:27:34 $
--
------------------------------------------------------------------------------*/
#ifndef __DWL_H__
#define __DWL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"
#include "decapicommon.h"

#ifdef MEMWATCH
	#include "memwatch.h"
#endif

#ifdef	PLATFORM800TC
	#include "mpTrace.h"
	#define	DPrintfLF(f, ...)	do{DPrintf(f, ##__VA_ARGS__);UartOutText("\r\n");}while(0)
	#define	mpDebugPrintN		DPrintf
	#define	mpDebugPrint		DPrintfLF
	#ifdef	MP_ALERT
		#undef	MP_ALERT
	#endif
	#define	MP_ALERT			DPrintfLF
	#ifdef	MP_ASSERT
		#undef	MP_ASSERT
	#endif
	#define MP_ASSERT(exp)	do{if (!(exp)) {DPrintfLF("%s, %d, %s", __FILE__,__LINE__,#exp); }}while(0)
#endif


#define DWL_OK                      0
#define DWL_ERROR                  -1

#define DWL_HW_WAIT_OK              DWL_OK
#define DWL_HW_WAIT_ERROR           DWL_ERROR
#define DWL_HW_WAIT_TIMEOUT         1

#define DWL_CLIENT_TYPE_H264_DEC         1U
#define DWL_CLIENT_TYPE_MPEG4_DEC        2U
#define DWL_CLIENT_TYPE_JPEG_DEC         3U
#define DWL_CLIENT_TYPE_PP               4U
#define DWL_CLIENT_TYPE_VC1_DEC          5U
#define DWL_CLIENT_TYPE_MPEG2_DEC        6U
#define DWL_CLIENT_TYPE_VP6_DEC          7U
#define DWL_CLIENT_TYPE_AVS_DEC          9U /* TODO: fix */
#define DWL_CLIENT_TYPE_RV_DEC           8U

//MagicPixel wrapper
#define	HX170WRAPPER_REG_START	0x200	/* mpx added register */
#define	HX170WRAPPER_REG_END	0x20C	/* [lower, upper)   the 1st invalid offset */

#ifdef	PLATFORM800TC
#define DEC_IO_BASE                 (BASE_CPU_REGISTER+0x30000)
#else
#define DEC_IO_BASE                 0xa8030000
#endif
#define DEC_IO_SIZE                 HX170WRAPPER_REG_END	/*	((100+1) * 4) */   /* bytes */


#define Wrapper_reg0 				*((volatile int *)(DEC_IO_BASE + 0x200))
#define Wrapper_reg1 				*((volatile int *)(DEC_IO_BASE + 0x204))
#define Wrapper_reg2 				*((volatile int *)(DEC_IO_BASE + 0x208))
#define PPREG(offset) 				*((volatile int *)(DEC_IO_BASE + offset))
#define DECREG(offset) 				*((volatile int *)(DEC_IO_BASE + offset))
#define	WRAPPER_IN_ON				(Wrapper_reg0 |= 0x200000)	//wrap pp input from IDU format of 422 YYCbCr to On2 format of 422 YCbYCr (PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED)
#define	WRAPPER_IN_OFF				(Wrapper_reg0 &= ~0x200000)
#define IF_WRAPPER_IN				(Wrapper_reg0 & 0x200000)
#define	WRAPPER_OUT_ON				(Wrapper_reg0 |= 0x400000)	//wrap pp outout from On2 format 422 YCbYCr (PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED) to IDU format of 422 YYCbCr
#define	WRAPPER_OUT_OFF				(Wrapper_reg0 &= ~0x400000)
#define IF_WRAPPER_OUT				(Wrapper_reg0 & 0x400000)
#define	WRAPPER_SCALING_RAM			(Wrapper_reg0 |= 0x800000)
#define	WRAPPER_SCALING_RAM_OFF		(Wrapper_reg0 &= ~0x800000)
#define	WRAPPER_WR_REQ_SEPARATE		(Wrapper_reg0 |= 0x4)
#define WRAPPER_WR_REQ_SEPARATE_OFF (Wrapper_reg0 &= ~0x4)
#define WRAPPER_RD_REQ_SEPARATE 	(Wrapper_reg0 |= 0x2)
#define WRAPPER_RD_REQ_SEPARATE_OFF (Wrapper_reg0 &= ~0x2)
#define	BOOTAREA(offset)			*((volatile unsigned char *)(0x80000000 + offset))



    /* Linear memory area descriptor */
    typedef struct DWLLinearMem
    {
        u32 *virtualAddress;
        u32 busAddress;
        u32 size;
    } DWLLinearMem_t;

    /* DWLInitParam is used to pass parameters when initializing the DWL */
    typedef struct DWLInitParam
    {
        u32 clientType;
    } DWLInitParam_t;

    /* Hardware configuration description */

    typedef struct DWLHwConfig
    {
        u32 maxDecPicWidth;  /* Maximum video decoding width supported  */
        u32 maxPpOutPicWidth;   /* Maximum output width of Post-Processor */
        u32 h264Support;     /* HW supports h.264 */
        u32 jpegSupport;     /* HW supports JPEG */
        u32 mpeg4Support;    /* HW supports MPEG-4 */
        u32 vc1Support;      /* HW supports VC-1 Simple */
        u32 mpeg2Support;    /* HW supports MPEG-2 */
        u32 ppSupport;       /* HW supports post-processor */
        u32 ppConfig;        /* HW post-processor functions bitmask */
        u32 sorensonSparkSupport;   /* HW supports Sorenson Spark */
        u32 refBufSupport;   /* HW supports reference picture buffering */
        u32 vp6Support;      /* HW supports VP6 */
        u32 avsSupport;      /* HW supports AVS */
        u32 jpegESupport;    /* HW supports JPEG extensions */
        u32 rvSupport;       /* HW supports REAL */
		u32 mvcSupport;      /* HW supports H264 MVC extension */
    } DWLHwConfig_t;

	typedef struct DWLHwFuseStatus
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
		u32 rvSupportFuse;       /* one of the REAL values defined above */

    } DWLHwFuseStatus_t;

/* HW ID retriving, static implementation */
    u32 DWLReadAsicID(void);

/* HW configuration retrieving, static implementation */
    void DWLReadAsicConfig(DWLHwConfig_t * pHwCfg);

/* HW fuse retrieving, static implementation */
	void DWLReadAsicFuseStatus(DWLHwFuseStatus_t * pHwFuseSts);

/* DWL initilaization and release */
    const void *DWLInit(DWLInitParam_t * param);
    i32 DWLRelease(const void *instance);

/* HW sharing */
    i32 DWLReserveHw(const void *instance);
    void DWLReleaseHw(const void *instance);

/* Frame buffers memory */
    i32 DWLMallocRefFrm(const void *instance, u32 size, DWLLinearMem_t * info);
    void DWLFreeRefFrm(const void *instance, DWLLinearMem_t * info);

/* SW/HW shared memory */
    i32 DWLMallocLinear(const void *instance, u32 size, DWLLinearMem_t * info);
    void DWLFreeLinear(const void *instance, DWLLinearMem_t * info);

/* D-Cache coherence */
    void DWLDCacheRangeFlush(const void *instance, DWLLinearMem_t * info);  /* NOT in use */
    void DWLDCacheRangeRefresh(const void *instance, DWLLinearMem_t * info);    /* NOT in use */

/* Register access */
    void DWLWriteReg(const void *instance, u32 offset, u32 value);
    u32 DWLReadReg(const void *instance, u32 offset);

    void DWLWriteRegAll(const void *instance, const u32 * table, u32 size); /* NOT in use */
    void DWLReadRegAll(const void *instance, u32 * table, u32 size);    /* NOT in use */

/* HW starting/stopping */
    void DWLEnableHW(const void *instance, u32 offset, u32 value);
    void DWLDisableHW(const void *instance, u32 offset, u32 value);

/* HW synchronization */
    i32 DWLWaitHwReady(const void *instance, u32 timeout);

/* SW/SW shared memory */
    void *DWLmalloc(u32 n);
    void DWLfree(void *p);
    void *DWLcalloc(u32 n, u32 s);
    void *DWLmemcpy(void *d, const void *s, u32 n);
    void *DWLmemset(void *d, i32 c, u32 n);

#define	VCODEC_ERROR_MSG_ISR			(1<<0)
#define	VCODEC_ERROR_MSG_REGISTER		(1<<1)
#define	VCODEC_ERROR_MSG_STRM_PROCESSED	(1<<1)
void SetVCodecErrorMsg(const u32 sVCodecErrorMsg);
u32 GetVCodecErrorMsg();


#ifdef __cplusplus
}
#endif

#endif                       /* __DWL_H__ */
