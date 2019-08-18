/*------------------------------------------------------------------------------
--                                                                            --
--           This confidential and proprietary software may be used           --
--              only as authorized by a licensing agreement from              --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--      In the event of publication, the following notice is applicable:      --
--                                                                            --
--                   (C) COPYRIGHT 2005 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--    The entire notice above must be reproduced on all authorized copies.    --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Abstract  : Parameter parser
--
------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: tb_params.c,v $
--  $Revision: 1.25 $
--  $Date: 2009/03/11 14:31:47 $
--
------------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "tb_cfg.h"
#include "deccfg.h"
#include "ppcfg.h"
#include "dwl.h"
#include "refbuffer.h"

#ifdef _ASSERT_USED
#include <assert.h>
#endif
#ifdef _ASSERT_USED
#define ASSERT(expr) assert(expr)
#else
//#define ASSERT(expr) if(!(expr)) { printf("assert failed, file: %s line: %d :: %s.\n", __FILE__, __LINE__, #expr); abort(); }
#define	ASSERT
#endif

/*------------------------------------------------------------------------------
    Blocks
------------------------------------------------------------------------------*/
#define BLOCK_TB_PARAMS ("TbParams")
#define BLOCK_DEC_PARAMS ("DecParams")
#define BLOCK_PP_PARAMS ("PpParams")

/*------------------------------------------------------------------------------
    Keys
------------------------------------------------------------------------------*/
#define KEY_PACKET_BY_PACKET      ("PacketByPacket")
#define KEY_NAL_UNIT_STREAM       ("NalUnitStream")
#define KEY_SEED_RND              ("SeedRnd")
#define KEY_STREAM_BIT_SWAP       ("StreamBitSwap")
#define KEY_STREAM_BIT_LOSS       ("StreamBitLoss")
#define KEY_STREAM_PACKET_LOSS    ("StreamPacketLoss")
#define KEY_STREAM_HEADER_CORRUPT ("StreamHeaderCorrupt")
#define KEY_STREAM_TRUNCATE       ("StreamTruncate")
#define KEY_SLICE_UD_IN_PACKET    ("SliceUdInPacket")
#define KEY_FIRST_TRACE_FRAME     ("FirstTraceFrame")

#define KEY_REFBU_ENABLED         ("refbuEnable")
#define KEY_REFBU_DIS_INTERLACED  ("refbuDisableInterlaced")
#define KEY_REFBU_DIS_DOUBLE      ("refbuDisableDouble")
#define KEY_REFBU_DIS_EVAL_MODE   ("refbuDisableEvalMode")
#define KEY_REFBU_DIS_CHECKPOINT  ("refbuDisableCheckpoint")
#define KEY_REFBU_DIS_OFFSET      ("refbuDisableOffset")
#define KEY_REFBU_DIS_TOPBOT      ("refbuDisableTopBotSum")
#define KEY_REFBU_DATA_EXCESS     ("refbuAdjustValue")

#define KEY_BUS_WIDTH_64BIT       ("BusWidthEnable64Bit")
#define KEY_MEM_LATENCY           ("MemLatencyClks")
#define KEY_MEM_NONSEQ            ("MemNonSeqClks")
#define KEY_MEM_SEQ               ("MemSeqClks")

#define KEY_SUPPORT_MPEG2         ("SupportMpeg2")
#define KEY_SUPPORT_VC1           ("SupportVc1")
#define KEY_SUPPORT_JPEG          ("SupportJpeg")
#define KEY_SUPPORT_MPEG4         ("SupportMpeg4")
#define KEY_SUPPORT_H264          ("SupportH264")
#define KEY_SUPPORT_VP6           ("SupportVp6")
#define KEY_SUPPORT_PJPEG         ("SupportPjpeg")
#define KEY_SUPPORT_SORENSON      ("SupportSorenson")
#define KEY_SUPPORT_AVS           ("SupportAvs")
#define KEY_SUPPORT_RV            ("SupportRv")
#define KEY_SUPPORT_JPEGE         ("SupportJpegE")
#define KEY_SUPPORT_NON_COMPLIANT ("SupportNonCompliant")
#define KEY_MAX_DEC_PIC_WIDTH     ("MaxDecPicWidth")

#define KEY_SUPPORT_PPD           ("SupportPpd")
#define KEY_SUPPORT_DITHER        ("SupportDithering")
#define KEY_SUPPORT_SCALING       ("SupportScaling")
#define KEY_SUPPORT_DEINT         ("SupportDeinterlacing")
#define KEY_SUPPORT_ABLEND        ("SupportAlphaBlending")
#define KEY_MAX_PP_OUT_PIC_WIDTH  ("MaxPpOutPicWidth")
#define KEY_HW_VERSION            ("HwVersion")

#define KEY_OUTPUT_PICTURE_ENDIAN ("OutputPictureEndian")
#define KEY_BUS_BURST_LENGTH      ("BusBurstLength")
#define KEY_ASIC_SERVICE_PRIORITY ("AsicServicePriority")
#define KEY_OUTPUT_FORMAT         ("OutputFormat")
#define KEY_LATENCY_COMPENSATION  ("LatencyCompensation")
#define KEY_CLOCK_GATING          ("ClockGating")
#define KEY_DATA_DISCARD          ("DataDiscard")
#define KEY_MEMORY_ALLOCATION     ("MemoryAllocation")
#define KEY_RLC_MODE_FORCED       ("RlcModeForced")
#define KEY_ERROR_CONCEALMENT     ("ErrorConcealment")
#define KEY_JPEG_MCUS_SLICE            ("JpegMcusSlice")
#define KEY_JPEG_INPUT_BUFFER_SIZE     ("JpegInputBufferSize")

#define KEY_INPUT_PICTURE_ENDIAN  ("InputPictureEndian")
#define KEY_WORD_SWAP             ("WordSwap")
#define KEY_FORCE_MPEG4_IDCT      ("ForceMpeg4Idct")

#define KEY_MULTI_BUFFER          ("MultiBuffer")

#define KEY_CH_8PIX_ILEAV         ("Ch8PixIleavOutput")


/*------------------------------------------------------------------------------
    Implement reading interer parameter
------------------------------------------------------------------------------*/
#define IMPLEMENT_PARAM_INTEGER( b, k, tgt) \
    if(!strcmp(block, b) && !strcmp(key, k)) { \
        char *endptr; \
        tgt = strtol( value, &endptr, 10 ); \
        if(*endptr) return TB_CFG_INVALID_VALUE; \
    }
/*------------------------------------------------------------------------------
    Implement reading string parameter
------------------------------------------------------------------------------*/
#define IMPLEMENT_PARAM_STRING( b, k, tgt) \
    if(!strcmp(block, b) && !strcmp(key, k)) { \
        strncpy(tgt, value, sizeof(tgt)); \
    }
/*------------------------------------------------------------------------------
    Implement structure allocation upon parsing a specific block.
------------------------------------------------------------------------------*/
#define IMPLEMENT_ALLOC_BLOCK( b, tgt, type ) \
    if(key && !strcmp(key, b)) { \
        register type ** t = (type **)&tgt; \
        if(!*t) { \
            *t = (type *)malloc(sizeof(type)); \
            ASSERT(*t); \
            memset(*t, 0, sizeof(type)); \
        } else return CFG_DUPLICATE_BLOCK; \
    }

/*------------------------------------------------------------------------------

   <++>.<++>  Function: ReadParam

        Functional description:
          Read parameter callback function.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
TBCfgCallbackResult TBReadParam( char * block, char * key, char * value, TBCfgCallbackParam state, void *cbParam )
{
    TBCfg * tbCfg = (TBCfg *)cbParam;

    ASSERT(key);
    ASSERT(tbCfg);

    switch(state) {
        case TB_CFG_CALLBACK_BLK_START:
	    /*printf("CFG_CALLBACK_BLK_START\n");
	    printf("block == %s\n", block);
	    printf("key == %s\n", key);
	    printf("value == %s\n\n", value);*/
	    /*IMPLEMENT_ALLOC_BLOCK( BLOCK_TD_PARAMS, tbCfg->tbParams, TbParams );
	    IMPLEMENT_ALLOC_BLOCK( BLOCK_DEC_PARAMS, tbCfg->decParams, DecParams );
            IMPLEMENT_ALLOC_BLOCK( BLOCK_PP_PARAMS, tbCfg->ppParams, PpParams );*/
	    break;
	case TB_CFG_CALLBACK_VALUE:
	    /*printf("CFG_CALLBACK_VALUE\n");
            printf("block == %s\n", block);
	    printf("key == %s\n", key);
	    printf("value == %s\n\n", value);*/
	    /* TbParams */
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_PACKET_BY_PACKET, tbCfg->tbParams.packetByPacket);
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_NAL_UNIT_STREAM, tbCfg->tbParams.nalUnitStream);
        IMPLEMENT_PARAM_INTEGER( BLOCK_TB_PARAMS, KEY_SEED_RND, tbCfg->tbParams.seedRnd);
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_STREAM_BIT_SWAP, tbCfg->tbParams.streamBitSwap);
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_STREAM_BIT_LOSS, tbCfg->tbParams.streamBitLoss);
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_STREAM_PACKET_LOSS, tbCfg->tbParams.streamPacketLoss);
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_STREAM_HEADER_CORRUPT, tbCfg->tbParams.streamHeaderCorrupt);
	    IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_STREAM_TRUNCATE, tbCfg->tbParams.streamTruncate);
        IMPLEMENT_PARAM_STRING( BLOCK_TB_PARAMS, KEY_SLICE_UD_IN_PACKET, tbCfg->tbParams.sliceUdInPacket);
        IMPLEMENT_PARAM_INTEGER( BLOCK_TB_PARAMS, KEY_FIRST_TRACE_FRAME, tbCfg->tbParams.firstTraceFrame);

	    /* DecParams */
	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_OUTPUT_PICTURE_ENDIAN, tbCfg->decParams.outputPictureEndian);
	    IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_BUS_BURST_LENGTH, tbCfg->decParams.busBurstLength);
	    IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_ASIC_SERVICE_PRIORITY, tbCfg->decParams.asicServicePriority);
 	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_OUTPUT_FORMAT, tbCfg->decParams.outputFormat);
	    IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_LATENCY_COMPENSATION, tbCfg->decParams.latencyCompensation);
	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_CLOCK_GATING, tbCfg->decParams.clockGating);
	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_DATA_DISCARD, tbCfg->decParams.dataDiscard);
	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_MEMORY_ALLOCATION, tbCfg->decParams.memoryAllocation);
	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_RLC_MODE_FORCED, tbCfg->decParams.rlcModeForced);
	    IMPLEMENT_PARAM_STRING( BLOCK_DEC_PARAMS, KEY_ERROR_CONCEALMENT, tbCfg->decParams.errorConcealment);
	    IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_JPEG_MCUS_SLICE, tbCfg->decParams.jpegMcusSlice);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_JPEG_INPUT_BUFFER_SIZE, tbCfg->decParams.jpegInputBufferSize);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DIS_INTERLACED, tbCfg->decParams.refbuDisableInterlaced);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DIS_DOUBLE, tbCfg->decParams.refbuDisableDouble);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DIS_EVAL_MODE, tbCfg->decParams.refbuDisableEvalMode);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DIS_CHECKPOINT, tbCfg->decParams.refbuDisableCheckpoint);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DIS_OFFSET, tbCfg->decParams.refbuDisableOffset);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DIS_TOPBOT, tbCfg->decParams.refbuDisableTopBotSum);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_DATA_EXCESS, tbCfg->decParams.refbuDataExcessMaxPct);
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_REFBU_ENABLED, tbCfg->decParams.refbuEnable );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_MPEG2, tbCfg->decParams.mpeg2Support );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_VC1, tbCfg->decParams.vc1Support );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_JPEG, tbCfg->decParams.jpegSupport );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_MPEG4, tbCfg->decParams.mpeg4Support );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_H264, tbCfg->decParams.h264Support );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_VP6, tbCfg->decParams.vp6Support );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_PJPEG, tbCfg->decParams.pJpegSupport );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_SORENSON, tbCfg->decParams.sorensonSupport );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_AVS, tbCfg->decParams.avsSupport );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_RV, tbCfg->decParams.rvSupport );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_JPEGE, tbCfg->decParams.jpegESupport );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_MAX_DEC_PIC_WIDTH, tbCfg->decParams.maxDecPicWidth );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_SUPPORT_NON_COMPLIANT, tbCfg->decParams.supportNonCompliant );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_HW_VERSION, tbCfg->decParams.hwVersion );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_BUS_WIDTH_64BIT, tbCfg->decParams.busWidth64bitEnable );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_MEM_LATENCY, tbCfg->decParams.latency );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_MEM_NONSEQ, tbCfg->decParams.nonSeqClk );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_MEM_SEQ, tbCfg->decParams.seqClk );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_FORCE_MPEG4_IDCT, tbCfg->decParams.forceMpeg4Idct );
        IMPLEMENT_PARAM_INTEGER( BLOCK_DEC_PARAMS, KEY_CH_8PIX_ILEAV, tbCfg->decParams.ch8PixIleavOutput );		

        /* PpParams */
	    IMPLEMENT_PARAM_STRING( BLOCK_PP_PARAMS, KEY_OUTPUT_PICTURE_ENDIAN, tbCfg->ppParams.outputPictureEndian);
	    IMPLEMENT_PARAM_STRING( BLOCK_PP_PARAMS, KEY_INPUT_PICTURE_ENDIAN, tbCfg->ppParams.inputPictureEndian);
	    IMPLEMENT_PARAM_STRING( BLOCK_PP_PARAMS, KEY_WORD_SWAP, tbCfg->ppParams.wordSwap);
	    IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_BUS_BURST_LENGTH, tbCfg->ppParams.busBurstLength);
	    IMPLEMENT_PARAM_STRING( BLOCK_PP_PARAMS, KEY_CLOCK_GATING, tbCfg->ppParams.clockGating);
	    IMPLEMENT_PARAM_STRING( BLOCK_PP_PARAMS, KEY_DATA_DISCARD, tbCfg->ppParams.dataDiscard);
        IMPLEMENT_PARAM_STRING( BLOCK_PP_PARAMS, KEY_MULTI_BUFFER, tbCfg->ppParams.multiBuffer);
        IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_SUPPORT_PPD, tbCfg->ppParams.ppdExists);
        IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_SUPPORT_DITHER, tbCfg->ppParams.ditheringSupport);
        IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_SUPPORT_SCALING, tbCfg->ppParams.scalingSupport);
        IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_SUPPORT_DEINT, tbCfg->ppParams.deinterlacingSupport);
        IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_SUPPORT_ABLEND, tbCfg->ppParams.alphaBlendingSupport);
        IMPLEMENT_PARAM_INTEGER( BLOCK_PP_PARAMS, KEY_MAX_PP_OUT_PIC_WIDTH, tbCfg->ppParams.maxPpOutPicWidth );

        break;

    }
    return TB_CFG_OK;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBPrintCfg

        Functional description:
          Prints the cofiguration to stdout.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
void TBPrintCfg(const TBCfg* tbCfg)
{
    /* TbParams */
    printf("tbCfg->tbParams.packetByPacket: %s\n", tbCfg->tbParams.packetByPacket);
    printf("tbCfg->tbParams.nalUnitStream: %s\n", tbCfg->tbParams.nalUnitStream);
    printf("tbCfg->tbParams.seedRnd: %d\n", tbCfg->tbParams.seedRnd);
    printf("tbCfg->tbParams.streamBitSwap: %s\n", tbCfg->tbParams.streamBitSwap);
    printf("tbCfg->tbParams.streamBitLoss: %s\n", tbCfg->tbParams.streamBitLoss);
    printf("tbCfg->tbParams.streamPacketLoss: %s\n", tbCfg->tbParams.streamPacketLoss);
    printf("tbCfg->tbParams.streamHeaderCorrupt: %s\n", tbCfg->tbParams.streamHeaderCorrupt);
    printf("tbCfg->tbParams.streamTruncate: %s\n", tbCfg->tbParams.streamTruncate);
    printf("tbCfg->tbParams.sliceUdInPacket: %s\n", tbCfg->tbParams.sliceUdInPacket);
    printf("tbCfg->tbParams.firstTraceFrame: %s\n", tbCfg->tbParams.firstTraceFrame);

    /* DecParams */
    printf("tbCfg->decParams.outputPictureEndian: %s\n", tbCfg->decParams.outputPictureEndian);
    printf("tbCfg->decParams.busBurstLength: %d\n", tbCfg->decParams.busBurstLength);
    printf("tbCfg->decParams.asicServicePriority: %d\n", tbCfg->decParams.asicServicePriority);
    printf("tbCfg->decParams.outputFormat: %s\n", tbCfg->decParams.outputFormat);
    printf("tbCfg->decParams.latencyCompensation: %d\n", tbCfg->decParams.latencyCompensation);
    printf("tbCfg->decParams.clockGating: %s\n", tbCfg->decParams.clockGating);
    printf("tbCfg->decParams.dataDiscard: %s\n", tbCfg->decParams.dataDiscard);
    printf("tbCfg->decParams.memoryAllocation: %s\n", tbCfg->decParams.memoryAllocation);
    printf("tbCfg->decParams.rlcModeForced: %s\n", tbCfg->decParams.rlcModeForced);
    printf("tbCfg->decParams.errorConcealment: %s\n", tbCfg->decParams.errorConcealment);
    printf("tbCfg->decParams.jpegMcusSlice: %d\n",tbCfg->decParams.jpegMcusSlice);
    printf("tbCfg->decParams.jpegInputBufferSize: %d\n", tbCfg->decParams.jpegInputBufferSize);

    /* PpParams */
    printf("tbCfg->ppParams.outputPictureEndian: %s\n", tbCfg->ppParams.outputPictureEndian);
    printf("tbCfg->ppParams.inputPictureEndian: %s\n", tbCfg->ppParams.inputPictureEndian);
    printf("tbCfg->ppParams.wordSwap: %s\n", tbCfg->ppParams.wordSwap);
    printf("tbCfg->ppParams.multiBuffer: %s\n", tbCfg->ppParams.multiBuffer);
    printf("tbCfg->ppParams.busBurstLength: %d\n", tbCfg->ppParams.busBurstLength);
    printf("tbCfg->ppParams.clockGating: %s\n", tbCfg->ppParams.clockGating);
    printf("tbCfg->ppParams.dataDiscard: %s\n", tbCfg->ppParams.dataDiscard);
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBSetDefaultCfg

        Functional description:
          Sets the default configuration.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
void TBSetDefaultCfg(TBCfg* tbCfg)
{
    /* TbParams */
    strcpy(tbCfg->tbParams.packetByPacket, "DISABLED");
    strcpy(tbCfg->tbParams.nalUnitStream, "DISABLED");
    tbCfg->tbParams.seedRnd = 1;
    strcpy(tbCfg->tbParams.streamBitSwap, "0");
    strcpy(tbCfg->tbParams.streamBitLoss, "0");
    strcpy(tbCfg->tbParams.streamPacketLoss, "0");
    strcpy(tbCfg->tbParams.streamHeaderCorrupt, "DISABLED");
    strcpy(tbCfg->tbParams.streamTruncate, "DISABLED");
    strcpy(tbCfg->tbParams.sliceUdInPacket, "DISABLED");
    tbCfg->tbParams.firstTraceFrame = 0;
    tbCfg->decParams.forceMpeg4Idct = 0;	

    /* DecParams */
#if (DEC_X170_OUTPUT_PICTURE_ENDIAN == DEC_X170_BIG_ENDIAN)
    strcpy(tbCfg->decParams.outputPictureEndian, "BIG_ENDIAN");
#else
    strcpy(tbCfg->decParams.outputPictureEndian, "LITTLE_ENDIAN");
#endif

    tbCfg->decParams.busBurstLength = DEC_X170_BUS_BURST_LENGTH;
    tbCfg->decParams.asicServicePriority = DEC_X170_ASIC_SERVICE_PRIORITY;

#if (DEC_X170_OUTPUT_FORMAT == DEC_X170_OUTPUT_FORMAT_RASTER_SCAN)
    strcpy(tbCfg->decParams.outputFormat, "RASTER_SCAN");
#else
    strcpy(tbCfg->decParams.outputFormat, "TILED");
#endif

    tbCfg->decParams.latencyCompensation = DEC_X170_LATENCY_COMPENSATION;

#if (DEC_X170_INTERNAL_CLOCK_GATING == 0)
    strcpy(tbCfg->decParams.clockGating, "DISABLED");
#else
    strcpy(tbCfg->decParams.clockGating, "ENABLED");
#endif

#if (DEC_X170_DATA_DISCARD_ENABLE == 0)
    strcpy(tbCfg->decParams.dataDiscard, "DISABLED");
#else
    strcpy(tbCfg->decParams.dataDiscard, "ENABLED");
#endif

    strcpy(tbCfg->decParams.memoryAllocation, "INTERNAL");
    strcpy(tbCfg->decParams.rlcModeForced, "DISABLED");
    strcpy(tbCfg->decParams.errorConcealment, "PICTURE_FREEZE");
    tbCfg->decParams.jpegMcusSlice = 0;
    tbCfg->decParams.jpegInputBufferSize = 0;
    tbCfg->decParams.ch8PixIleavOutput = 0;	

    tbCfg->decParams.refbuEnable = 1;
    tbCfg->decParams.refbuDisableInterlaced = 1;
    tbCfg->decParams.refbuDisableDouble = 1;
    tbCfg->decParams.refbuDisableEvalMode = 1;
    tbCfg->decParams.refbuDisableCheckpoint = 1;
    tbCfg->decParams.refbuDisableOffset = 1;
    tbCfg->decParams.refbuDisableTopBotSum = 1;
#ifdef DEC_X170_REFBU_ADJUST_VALUE 
    tbCfg->decParams.refbuDataExcessMaxPct = DEC_X170_REFBU_ADJUST_VALUE;
#else
    tbCfg->decParams.refbuDataExcessMaxPct = 130;
#endif

    tbCfg->decParams.mpeg2Support = 1;
    tbCfg->decParams.vc1Support = 3; /* Adv profile */
    tbCfg->decParams.jpegSupport = 1;
    tbCfg->decParams.mpeg4Support = 2; /* ASP */
    tbCfg->decParams.h264Support = 3; /* High */
    tbCfg->decParams.vp6Support = 1;
    tbCfg->decParams.pJpegSupport = 1;
    tbCfg->decParams.sorensonSupport = 1;
    tbCfg->decParams.avsSupport = 1;
    tbCfg->decParams.rvSupport = 1;
    tbCfg->decParams.jpegESupport = 1;
    tbCfg->decParams.supportNonCompliant = 0;
    tbCfg->decParams.maxDecPicWidth = 1920;
    tbCfg->decParams.hwVersion = 8190;

    tbCfg->decParams.busWidth64bitEnable = (DEC_X170_REFBU_WIDTH == 64);
    tbCfg->decParams.latency = DEC_X170_REFBU_LATENCY;
    tbCfg->decParams.nonSeqClk = DEC_X170_REFBU_NONSEQ;
    tbCfg->decParams.seqClk = DEC_X170_REFBU_SEQ;

    /* PpParams */
    strcpy(tbCfg->ppParams.outputPictureEndian, "BIG_ENDIAN");
    strcpy(tbCfg->ppParams.inputPictureEndian, "BIG_ENDIAN");
    strcpy(tbCfg->ppParams.wordSwap, "DISABLED");
    tbCfg->ppParams.busBurstLength = PP_X170_BUS_BURST_LENGTH;

    strcpy(tbCfg->ppParams.multiBuffer, "ENABLED");


#if (PP_X170_INTERNAL_CLOCK_GATING == 0)
    strcpy(tbCfg->ppParams.clockGating, "DISABLED");
#else
    strcpy(tbCfg->ppParams.clockGating, "ENABLED");
#endif

#if (DEC_X170_DATA_DISCARD_ENABLE == 0)
    strcpy(tbCfg->ppParams.dataDiscard, "DISABLED");
#else
    strcpy(tbCfg->pParams.dataDiscard, "ENABLED");
#endif

    tbCfg->ppParams.ppdExists = 1;
    tbCfg->ppParams.ditheringSupport = 1;
    tbCfg->ppParams.scalingSupport = 1; /* Lo/Hi performance? */
    tbCfg->ppParams.deinterlacingSupport = 1;
    tbCfg->ppParams.alphaBlendingSupport = 1;
    tbCfg->ppParams.maxPpOutPicWidth = 1920;

}

u32 TBCheckCfg(const TBCfg* tbCfg)
{
    /* TbParams */
    /*if (tbCfg->tbParams.maxPics)
    {
    }*/

    if (strcmp(tbCfg->tbParams.packetByPacket, "ENABLED") &&
            strcmp(tbCfg->tbParams.packetByPacket, "DISABLED"))
    {
        printf("Error in TbParams.PacketByPacket: %s\n", tbCfg->tbParams.packetByPacket);
	    return 1;
    }

    if (strcmp(tbCfg->tbParams.nalUnitStream, "ENABLED") &&
            strcmp(tbCfg->tbParams.nalUnitStream, "DISABLED"))
    {
        printf("Error in TbParams.NalUnitStream: %s\n", tbCfg->tbParams.nalUnitStream);
    	return 1;
    }

    /*if (strcmp(tbCfg->tbParams.streamBitSwap, "0") == 0 &&
            strcmp(tbCfg->tbParams.streamHeaderCorrupt, "ENABLED") == 0)
    {
        printf("Stream header corrupt requires enabled stream bit swap (see test bench configuration)\n");
        return 1;
    }*/

    if (strcmp(tbCfg->tbParams.streamPacketLoss, "0") &&
            strcmp(tbCfg->tbParams.packetByPacket, "DISABLED") == 0)
    {
        printf("Stream packet loss requires enabled packet by packet mode (see test bench configuration)\n");
        return 1;
    }

    if (strcmp(tbCfg->tbParams.streamHeaderCorrupt, "ENABLED") &&
            strcmp(tbCfg->tbParams.streamHeaderCorrupt, "DISABLED"))
    {
        printf("Error in TbParams.StreamHeaderCorrupt: %s\n", tbCfg->tbParams.streamHeaderCorrupt);
	    return 1;
    }

    if (strcmp(tbCfg->tbParams.streamTruncate, "ENABLED") &&
            strcmp(tbCfg->tbParams.streamTruncate, "DISABLED"))
    {
        printf("Error in TbParams.StreamTruncate: %s\n", tbCfg->tbParams.streamTruncate);
    	return 1;
    }

    if (strcmp(tbCfg->tbParams.sliceUdInPacket, "ENABLED") &&
            strcmp(tbCfg->tbParams.sliceUdInPacket, "DISABLED"))
    {
        printf("Error in TbParams.streamTruncate: %s\n", tbCfg->tbParams.sliceUdInPacket);
    	return 1;
    }

    /* DecParams */
    if (strcmp(tbCfg->decParams.outputPictureEndian, "LITTLE_ENDIAN") &&
            strcmp(tbCfg->decParams.outputPictureEndian, "BIG_ENDIAN"))
    {
        printf("Error in DecParams.OutputPictureEndian: %s\n", tbCfg->decParams.outputPictureEndian);
	    return 1;
    }

    if (tbCfg->decParams.busBurstLength  > 31)
    {
        printf("Error in DecParams.BusBurstLength: %d\n", tbCfg->decParams.busBurstLength);
	    return 1;
    }

    if (tbCfg->decParams.asicServicePriority > 4)
    {
        printf("Error in DecParams.AsicServicePriority: %d\n", tbCfg->decParams.asicServicePriority);
    	return 1;
    }

    if (strcmp(tbCfg->decParams.outputFormat, "RASTER_SCAN") &&
            strcmp(tbCfg->decParams.outputFormat, "TILED"))
    {
        printf("Error in DecParams.OutputFormat: %s\n", tbCfg->decParams.outputFormat);
	    return 1;
    }

    if (tbCfg->decParams.latencyCompensation > 63 ||
            tbCfg->decParams.latencyCompensation < 0)
    {
        printf("Error in DecParams.LatencyCompensation: %d\n", tbCfg->decParams.latencyCompensation);
    	return 1;
    }

    if (strcmp(tbCfg->decParams.clockGating, "ENABLED") &&
            strcmp(tbCfg->decParams.clockGating, "DISABLED"))
    {
        printf("Error in DecParams.ClockGating: %s\n", tbCfg->decParams.clockGating);
	    return 1;
    }

    if (strcmp(tbCfg->decParams.dataDiscard, "ENABLED") &&
            strcmp(tbCfg->decParams.dataDiscard, "DISABLED"))
    {
        printf("Error in DecParams.DataDiscard: %s\n", tbCfg->decParams.dataDiscard);
    	return 1;
    }

    if (strcmp(tbCfg->decParams.memoryAllocation, "INTERNAL") &&
            strcmp(tbCfg->decParams.memoryAllocation, "EXTERNAL"))
    {
        printf("Error in DecParams.MemoryAllocation: %s\n", tbCfg->decParams.memoryAllocation);
	    return 1;
    }

    if (strcmp(tbCfg->decParams.rlcModeForced, "DISABLED") &&
            strcmp(tbCfg->decParams.rlcModeForced, "ENABLED"))
    {
        printf("Error in DecParams.RlcModeForced: %s\n", tbCfg->decParams.rlcModeForced);
    	return 1;
    }

    /*if (strcmp(tbCfg->decParams.rlcModeForced, "ENABLED") == 0 &&
            strcmp(tbCfg->decParams.errorConcealment, "PICTURE_FREEZE") == 0)
    {
        printf("MACRO_BLOCK DecParams.ErrorConcealment must be enabled if RLC coding\n");
	    return 1;
    }*/

    /*
    if (strcmp(tbCfg->decParams.rlcModeForced, "ENABLED") == 0 &&
            (strcmp(tbCfg->tbParams.packetByPacket, "ENABLED") == 0 ||
	        strcmp(tbCfg->tbParams.nalUnitStream, "ENABLED") == 0))
    {
        printf("TbParams.PacketByPacket and TbParams.NalUnitStream must not be enabled if RLC coding\n");
	    return 1;
    }
    */ /* why is that above? */

    if (strcmp(tbCfg->tbParams.nalUnitStream, "ENABLED") == 0 &&
	    strcmp(tbCfg->tbParams.packetByPacket, "DISABLED") == 0)
    {
        printf("TbParams.PacketByPacket must be enabled if NAL unit stream is used\n");
	    return 1;
    }

    if (strcmp(tbCfg->tbParams.sliceUdInPacket, "ENABLED") == 0 &&
	    strcmp(tbCfg->tbParams.packetByPacket, "DISABLED") == 0)
    {
        printf("TbParams.PacketByPacket must be enabled if slice user data is included in packet\n");
	    return 1;
    }

    /*if (strcmp(tbCfg->decParams.errorConcealment, "MACRO_BLOCK") &&
        strcmp(tbCfg->decParams.errorConcealment, "PICTURE_FREEZE"))
    {
        printf("Error in DecParams.ErrorConcealment: %s\n", tbCfg->decParams.errorConcealment);
	    return 1;
    }*/

    /*if (tbCfg->decParams.mcusSlice)
    {
    }*/

    if (tbCfg->decParams.jpegInputBufferSize != 0 &&
            ((tbCfg->decParams.jpegInputBufferSize > 0 && tbCfg->decParams.jpegInputBufferSize < 5120) ||
            tbCfg->decParams.jpegInputBufferSize > 16776960 ||
            tbCfg->decParams.jpegInputBufferSize % 256 != 0))
    {
        printf("Error in DecParams.inputBufferSize: %d\n", tbCfg->decParams.jpegInputBufferSize);
    	return 1;
    }


    /* PpParams */
    if (strcmp(tbCfg->ppParams.outputPictureEndian, "LITTLE_ENDIAN") &&
            strcmp(tbCfg->ppParams.outputPictureEndian, "BIG_ENDIAN") &&
	        strcmp(tbCfg->ppParams.outputPictureEndian, "PP_CFG"))
    {
        printf("Error in PpParams.OutputPictureEndian: %s\n", tbCfg->ppParams.outputPictureEndian);
	    return 1;
    }

    if (strcmp(tbCfg->ppParams.inputPictureEndian, "LITTLE_ENDIAN") &&
            strcmp(tbCfg->ppParams.inputPictureEndian, "BIG_ENDIAN") &&
	    strcmp(tbCfg->ppParams.inputPictureEndian, "PP_CFG"))
    {
        printf("Error in PpParams.InputPictureEndian: %s\n", tbCfg->ppParams.inputPictureEndian);
	    return 1;
    }

    if (strcmp(tbCfg->ppParams.wordSwap, "ENABLED") &&
            strcmp(tbCfg->ppParams.wordSwap, "DISABLED") &&
	    strcmp(tbCfg->ppParams.wordSwap, "PP_CFG"))
    {
        printf("Error in PpParams.WordSwap: %s\n", tbCfg->ppParams.wordSwap);
   	    return 1;
    }

    if (tbCfg->ppParams.busBurstLength  > 31)
    {
        printf("Error in PpParams.BusBurstLength: %d\n", tbCfg->ppParams.busBurstLength);
    	return 1;
    }

    if (strcmp(tbCfg->ppParams.clockGating, "ENABLED") &&
            strcmp(tbCfg->ppParams.clockGating, "DISABLED"))
    {
        printf("Error in PpParams.ClockGating: %s\n", tbCfg->ppParams.clockGating);
        return 1;
    }

    if (strcmp(tbCfg->ppParams.dataDiscard, "ENABLED") &&
            strcmp(tbCfg->ppParams.dataDiscard, "DISABLED"))
    {
        printf("Error in PpParams.DataDiscard: %s\n", tbCfg->ppParams.dataDiscard);
        return 1;
    }

    return 0;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetPPDataDiscard

        Functional description:
          Gets the integer values of PP data disgard.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetPPDataDiscard(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->ppParams.dataDiscard, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->ppParams.dataDiscard, "DISABLED")  == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetPPClockGating

        Functional description:
          Gets the integer values of PP clock gating.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetPPClockGating(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->ppParams.clockGating, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->ppParams.clockGating, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetPPWordSwap

        Functional description:
          Gets the integer values of PP word swap.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetPPWordSwap(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->ppParams.wordSwap, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->ppParams.wordSwap, "DISABLED") == 0)
    {
        return 0;
    }
    else if (strcmp(tbCfg->ppParams.wordSwap, "PP_CFG") == 0)
    {
        return 2;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetPPInputPictureEndian

        Functional description:
          Gets the integer values of PP input picture endian.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetPPInputPictureEndian(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->ppParams.inputPictureEndian, "BIG_ENDIAN") == 0)
    {
        return PP_X170_PICTURE_BIG_ENDIAN;
    }
    else if (strcmp(tbCfg->ppParams.inputPictureEndian, "LITTLE_ENDIAN") == 0)
    {
        return PP_X170_PICTURE_LITTLE_ENDIAN;
    }
    else if (strcmp(tbCfg->ppParams.inputPictureEndian, "PP_CFG") == 0)
    {
        return 2;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetPPOutputPictureEndian

        Functional description:
          Gets the integer values of PP out picture endian.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetPPOutputPictureEndian(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->ppParams.outputPictureEndian, "BIG_ENDIAN") == 0)
    {
        return PP_X170_PICTURE_BIG_ENDIAN;
    }
    else if (strcmp(tbCfg->ppParams.outputPictureEndian, "LITTLE_ENDIAN") == 0)
    {
        return PP_X170_PICTURE_LITTLE_ENDIAN;
    }
    else if (strcmp(tbCfg->ppParams.outputPictureEndian, "PP_CFG") == 0)
    {
        return 2;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecErrorConcealment

        Functional description:
          Gets the integer values of decoder error concealment.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecErrorConcealment(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->decParams.errorConcealment, "MACRO_BLOCK") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->decParams.errorConcealment, "PICTURE_FREEZE") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecRlcModeForced

        Functional description:
          Gets the integer values of decoder rlc mode forced.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecRlcModeForced(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->decParams.rlcModeForced, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->decParams.rlcModeForced, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecMemoryAllocation

        Functional description:
          Gets the integer values of decoder memory allocation.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecMemoryAllocation(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->decParams.memoryAllocation, "INTERNAL") == 0)
    {
        return 0;
    }
    else if (strcmp(tbCfg->decParams.memoryAllocation, "EXTERNAL") == 0)
    {
        return 1;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecDataDiscard

        Functional description:
          Gets the integer values of decoder data disgard.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecDataDiscard(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->decParams.dataDiscard, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->decParams.dataDiscard, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecClockGating

        Functional description:
          Gets the integer values of decoder clock gating.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecClockGating(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->decParams.clockGating, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->decParams.clockGating, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecOutputFormat

        Functional description:
          Gets the integer values of decoder output format.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecOutputFormat(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->decParams.outputFormat, "RASTER_SCAN") == 0)
    {
        return DEC_X170_OUTPUT_FORMAT_RASTER_SCAN;
    }
    else if (strcmp(tbCfg->decParams.outputFormat, "TILED") == 0)
    {
        return DEC_X170_OUTPUT_FORMAT_TILED;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecOutputPictureEndian

        Functional description:
          Gets the integer values of decoder output format.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecOutputPictureEndian(const TBCfg* tbCfg)
{
     if (strcmp(tbCfg->decParams.outputPictureEndian, "BIG_ENDIAN") == 0)
    {
        return DEC_X170_BIG_ENDIAN;
    }
    else if (strcmp(tbCfg->decParams.outputPictureEndian, "LITTLE_ENDIAN") == 0)
    {
        return DEC_X170_LITTLE_ENDIAN;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBPacketByPacket

        Functional description:
          Gets the integer values of TB packet by packet.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBPacketByPacket(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->tbParams.packetByPacket, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->tbParams.packetByPacket, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBNalUnitStream

        Functional description:
          Gets the integer values of TB NALU unit stream.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBNalUnitStream(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->tbParams.nalUnitStream, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->tbParams.nalUnitStream, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBStreamHeaderCorrupt

        Functional description:
          Gets the integer values of TB header corrupt.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBStreamHeaderCorrupt(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->tbParams.streamHeaderCorrupt, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->tbParams.streamHeaderCorrupt, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBStreamTruncate

        Functional description:
          Gets the integer values of TB stream truncate.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBStreamTruncate(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->tbParams.streamTruncate, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->tbParams.streamTruncate, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBStreamTruncate

        Functional description:
          Gets the integer values of TB stream truncate.

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBSliceUdInPacket(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->tbParams.sliceUdInPacket, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->tbParams.sliceUdInPacket, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBmultiBuffer

        Functional description:


        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBMultiBuffer(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->ppParams.multiBuffer, "ENABLED") == 0)
    {
        return 1;
    }
    else if (strcmp(tbCfg->ppParams.multiBuffer, "DISABLED") == 0)
    {
        return 0;
    }
    else
    {
         ASSERT(0);
         return -1;
    }
}



/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecRefbuEvalMode

        Functional description:
          Gets the integer values of TB disable reference buffer eval mode

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecRefbuEvalMode(const TBCfg* tbCfg)
{
    return !tbCfg->decParams.refbuDisableEvalMode;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecRefbuCheckpoint

        Functional description:
          Gets the integer values of TB disable reference buffer eval mode

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecRefbuCheckpoint(const TBCfg* tbCfg)
{
    return !tbCfg->decParams.refbuDisableCheckpoint;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecRefbuOffset

        Functional description:
          Gets the integer values of TB disable reference buffer eval mode

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecRefbuOffset(const TBCfg* tbCfg)
{
    return !tbCfg->decParams.refbuDisableOffset;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecRefbuEnabled

        Functional description:
          Gets the integer values of TB disable reference buffer eval mode

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecRefbuEnabled(const TBCfg* tbCfg)
{
    return tbCfg->decParams.refbuEnable;
}


/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDec64BitEnable

        Functional description:
          Gets the integer values of TB disable reference buffer eval mode

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDec64BitEnable(const TBCfg* tbCfg)
{
    return tbCfg->decParams.busWidth64bitEnable;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecSupportNonCompliant

        Functional description:

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecSupportNonCompliant(const TBCfg* tbCfg)
{
    return tbCfg->decParams.supportNonCompliant;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetDecRefbuEnabled

        Functional description:
          Gets the integer values of TB disable reference buffer eval mode

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
void TBGetHwConfig(const TBCfg* tbCfg, DWLHwConfig_t * pHwCfg )
{

    u32 tmp;

    pHwCfg->maxDecPicWidth = tbCfg->decParams.maxDecPicWidth;
    pHwCfg->maxPpOutPicWidth = tbCfg->ppParams.maxPpOutPicWidth;

    pHwCfg->h264Support = tbCfg->decParams.h264Support;
    if (tbCfg->decParams.hwVersion < 8190)
        pHwCfg->h264Support = pHwCfg->h264Support ? 1 : 0;
    pHwCfg->jpegSupport = tbCfg->decParams.jpegSupport;
    pHwCfg->mpeg4Support = tbCfg->decParams.mpeg4Support;
    pHwCfg->mpeg2Support = tbCfg->decParams.mpeg2Support;
    pHwCfg->vc1Support = tbCfg->decParams.vc1Support;
    pHwCfg->ppSupport = tbCfg->ppParams.ppdExists;
    tmp = 0;
    if( tbCfg->ppParams.ditheringSupport )  tmp |= PP_DITHERING;
    if( tbCfg->ppParams.scalingSupport )  tmp |= PP_SCALING;
    if( tbCfg->ppParams.deinterlacingSupport )  tmp |= PP_DEINTERLACING;
    if( tbCfg->ppParams.alphaBlendingSupport )  tmp |= PP_ALPHA_BLENDING;
    pHwCfg->ppConfig = tmp;
    pHwCfg->sorensonSparkSupport = tbCfg->decParams.sorensonSupport;
    pHwCfg->refBufSupport = 
        ( tbCfg->decParams.refbuEnable ? REF_BUF_SUPPORTED : 0 ) |
        ( (!tbCfg->decParams.refbuDisableInterlaced) ? REF_BUF_INTERLACED : 0 ) |
        ( (!tbCfg->decParams.refbuDisableDouble) ? REF_BUF_DOUBLE : 0 );
    pHwCfg->vp6Support = tbCfg->decParams.vp6Support;
    pHwCfg->avsSupport = tbCfg->decParams.avsSupport;
    if (tbCfg->decParams.hwVersion < 9170)
        pHwCfg->rvSupport = 0;
    else
        pHwCfg->rvSupport = tbCfg->decParams.rvSupport;
    pHwCfg->jpegESupport = tbCfg->decParams.jpegESupport;

}



/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBSetRefbuMemModel

        Functional description:
          Override reference buffer memory model parameters

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
void TBSetRefbuMemModel( const TBCfg* tbCfg, refBuffer_t *pRefbu )
{
    pRefbu->busWidthInBits = 32 + 32*tbCfg->decParams.busWidth64bitEnable;
    pRefbu->currMemModel.latency = tbCfg->decParams.latency;
    pRefbu->currMemModel.nonseq = tbCfg->decParams.nonSeqClk;
    pRefbu->currMemModel.seq = tbCfg->decParams.seqClk;
    pRefbu->dataExcessMaxPct = tbCfg->decParams.refbuDataExcessMaxPct;
    pRefbu->mbWeight = 
        pRefbu->decModeMbWeights[tbCfg->decParams.busWidth64bitEnable];
    if( pRefbu->memAccessStatsFlag == 0 )
    {
        if( tbCfg->decParams.busWidth64bitEnable && ! (DEC_X170_REFBU_WIDTH == 64) )
        {
            pRefbu->memAccessStats.seq >>= 1;
        }
        else if( !tbCfg->decParams.busWidth64bitEnable && (DEC_X170_REFBU_WIDTH == 64) )
        {
            pRefbu->memAccessStats.seq <<= 1;
        }
        pRefbu->memAccessStatsFlag = 1;
    }

}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetIntraFreezeEnable

        Functional description:
          Override reference buffer memory model parameters

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecIntraFreezeEnable(const TBCfg* tbCfg)
{
    if (strcmp(tbCfg->decParams.errorConcealment, "INTRA_FREEZE") == 0)
    {
        return 1;
    }
    return 0;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetIntraFreezeEnable

        Functional description:
          Override reference buffer memory model parameters

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecDoubleBufferSupported(const TBCfg* tbCfg)
{
    return !tbCfg->decParams.refbuDisableDouble;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetIntraFreezeEnable

        Functional description:
          Override reference buffer memory model parameters

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecTopBotSumSupported(const TBCfg* tbCfg)
{
    return !tbCfg->decParams.refbuDisableTopBotSum;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBFirstTraceFrame

        Functional description:

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetTBFirstTraceFrame(const TBCfg* tbCfg)
{
     return tbCfg->tbParams.firstTraceFrame;
}

/*------------------------------------------------------------------------------

   <++>.<++>  Function: TBGetTBFirstTraceFrame

        Functional description:

        Inputs:

        Outputs:

------------------------------------------------------------------------------*/
u32 TBGetDecForceMpeg4Idct(const TBCfg* tbCfg)
{
     return tbCfg->decParams.forceMpeg4Idct;
}

u32 TBGetDecCh8PixIleavOutput(const TBCfg* tbCfg)
{
     return tbCfg->decParams.ch8PixIleavOutput;
}

