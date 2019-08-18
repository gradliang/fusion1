/*------------------------------------------------------------------------------
--                                                                                             --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--      In the event of publication, the following notice is applicable:      --
--                                                                            --
--                   (C) COPYRIGHT 2004 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--         The entire notice above must be reproduced on all copies.          --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description : This module declares functions for corrupting data in 
--                    streams.
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: tb_stream_corrupt.h,v $
--  $Revision: 1.1 $
--  $Date: 2007/03/27 11:06:46 $
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents 
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef TB_STREAM_CORRUPT_H
#define TB_STREAM_CORRUPT_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

void TBInitializeRandom(u32 seed);

u32 TBRandomizeBitSwapInStream(u8* stream, 
                                                 u32 streamLen, 
                                                 char* odds);

u32 TBRandomizePacketLoss(char* odds, 
                                          u8* packetLoss);
                                          
u32 TBRandomizeU32(u32* value);

/* u32 RandomizeBitLossInStream(u8* stream, 
                                               u32* streamLen, 
					       char* odds); */

#endif /* TB_STREAM_CORRUPT_H */
