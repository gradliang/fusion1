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
--  Abstract : H264 decoder and PP pipeline support
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: h264_pp_pipeline.h,v $
--  $Date: 2008/08/27 13:14:34 $
--  $Revision: 1.2 $
--
------------------------------------------------------------------------------*/

#ifndef H264_PP_PIPELINE_H
#define H264_PP_PIPELINE_H

#include "decppif.h"

i32 h264RegisterPP(const void *decInst, const void *ppInst,
                   void (*PPDecStart) (const void *, const DecPpInterface *),
                   void (*PPDecWaitEnd) (const void *),
                   void (*PPConfigQuery) (const void *, DecPpQuery *),
                   void (*PPDisplayIndex) (const void *, u32));

i32 h264UnregisterPP(const void *decInst, const void *ppInst);

#endif /* #ifdef H264_PP_PIPELINE_H */
