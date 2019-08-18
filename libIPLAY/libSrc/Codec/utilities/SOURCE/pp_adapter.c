/*
this file is because codec library source is invisible for others
*/

#include "global612.h"
#include "pp_adapter.h"

/*------------------------------------------------------------------------------

    Function: mpeg2RegisterPP()

        Functional description:
            Register the pp for mpeg-2 pipeline

        Inputs:
            decInst     Decoder instance
            const void  *ppInst - post-processor instance
            (*PPRun)(const void *) - decoder calls this to start PP
            void (*PPEndCallback)(const void *) - decoder calls this
                        to notify PP that a picture was done.

        Outputs:
            none

        Returns:
            i32 - return 0 for success or a negative error code

------------------------------------------------------------------------------*/

i32 mpeg2RegisterPP_adapter
					(const void *decInst, const void *ppInst,
                    void (*PPRun) (const void *, const DecPpInterface *),
                    void (*PPEndCallback) (const void *),
                    void (*PPConfigQuery) (const void *, DecPpQuery *),
                    void (*PPDisplayIndex) (const void *, u32),
                    void (*PPBufferData) (const void *, u32, u32, u32))
{
#if VCODEC_MPEG12_ENABLE
	return mpeg2RegisterPP(decInst, ppInst,
							PPRun,
							PPEndCallback,
							PPConfigQuery,
							PPDisplayIndex,
							PPBufferData);
#else
	return 0;
#endif
}

/*------------------------------------------------------------------------------

    Function: mpeg2UnregisterPP()

        Functional description:
            Unregister the pp from mpeg-2 pipeline

        Inputs:
            decInst     Decoder instance
            const void  *ppInst - post-processor instance

        Outputs:
            none

        Returns:
            i32 - return 0 for success or a negative error code

------------------------------------------------------------------------------*/

i32 mpeg2UnregisterPP_adapter(const void *decInst, const void *ppInst)
{
#if VCODEC_MPEG12_ENABLE
	return mpeg2UnregisterPP(decInst, ppInst);
#else
	return 0;
#endif
}

/*------------------------------------------------------------------------------

    Function: mpeg4RegisterPP()

        Functional description:
            Register the pp for mpeg-4 pipeline

        Inputs:
            decInst     Decoder instance
            const void  *ppInst - post-processor instance
            (*PPRun)(const void *) - decoder calls this to start PP
            void (*PPEndCallback)(const void *) - decoder calls this
                        to notify PP that a picture was done.

        Outputs:
            none

        Returns:
            i32 - return 0 for success or a negative error code

------------------------------------------------------------------------------*/

i32 mpeg4RegisterPP_adapter(const void *decInst, const void *ppInst,
                    void (*PPRun) (const void *, const DecPpInterface *),
                    void (*PPEndCallback) (const void *),
                    void (*PPConfigQuery) (const void *, DecPpQuery *),
                    void (*PPDisplayIndex)(const void *, u32),
		    void (*PPBufferData) (const void *, u32, u32, u32))
{
#if VCODEC_MPEG4_ENABLE
	return mpeg4RegisterPP(decInst, ppInst,
                    PPRun,
                    PPEndCallback,
                    PPConfigQuery,
                    PPDisplayIndex,
					PPBufferData);
#else
	return 0;
#endif

}

/*------------------------------------------------------------------------------

    Function: mpeg4RegisterPP()

        Functional description:
            Unregister the pp from mpeg-4 pipeline

        Inputs:
            decInst     Decoder instance
            const void  *ppInst - post-processor instance

        Outputs:
            none

        Returns:
            i32 - return 0 for success or a negative error code

------------------------------------------------------------------------------*/

i32 mpeg4UnregisterPP_adapter(const void *decInst, const void *ppInst)
{
#if VCODEC_MPEG4_ENABLE
	return mpeg4UnregisterPP(decInst, ppInst);
#else
	return 0;
#endif
}



/*------------------------------------------------------------------------------
    Function name   : h264RegisterPP
    Description     :
    Return type     : i32
    Argument        : const void * decInst
    Argument        : const void  *ppInst
    Argument        : (*PPRun)(const void *)
    Argument        : void (*PPEndCallback)(const void *)
------------------------------------------------------------------------------*/
i32 h264RegisterPP_adapter(const void *decInst, const void *ppInst,
                   void (*PPDecStart) (const void *, const DecPpInterface *),
                   void (*PPDecWaitEnd) (const void *),
                   void (*PPConfigQuery) (const void *, DecPpQuery *),
                   void (*PPDisplayIndex) (const void *, u32))
{
#if VCODEC_H264_ENABLE
	return h264RegisterPP(decInst, ppInst,
                   PPDecStart,
                   PPDecWaitEnd,
                   PPConfigQuery,
                   PPDisplayIndex);
#else
	return 0;
#endif
}

/*------------------------------------------------------------------------------
    Function name   : h264UnregisterPP
    Description     :
    Return type     : i32
    Argument        : const void * decInst
    Argument        : const void  *ppInst
------------------------------------------------------------------------------*/
i32 h264UnregisterPP_adapter(const void *decInst, const void *ppInst)
{
#if VCODEC_H264_ENABLE
	return h264UnregisterPP(decInst, ppInst);
#else
	return 0;
#endif
}



