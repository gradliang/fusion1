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
--  Abstract : post-processor external mode testbench
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: tb_md5.h,v $
--  $Date: 2007/03/27 11:06:45 $
--  $Revision: 1.1 $
--
------------------------------------------------------------------------------*/

#ifndef TB_MD5_H
#define TB_MD5_H

#include "basetype.h"
#include <stdio.h>

extern u32 TBWriteFrameMD5Sum(FILE* fOut, u8* pYuv, u32 yuvSize, u32 frameNumber);

#endif
