
#ifndef __PP__ADAPTER_H
#define __PP__ADAPTER_H

#include "mpeg2hwd_pp_pipeline.h"
#include "mpeg4_pp_pipeline.h"
#include "h264_pp_pipeline.h"

i32 mpeg2RegisterPP_adapter
					(const void *decInst, const void *ppInst,
                    void (*PPRun) (const void *, const DecPpInterface *),
                    void (*PPEndCallback) (const void *),
                    void (*PPConfigQuery) (const void *, DecPpQuery *),
                    void (*PPDisplayIndex) (const void *, u32),
                    void (*PPBufferData) (const void *, u32, u32, u32));

i32 mpeg2UnregisterPP_adapter(const void *decInst, const void *ppInst);

i32 mpeg4RegisterPP_adapter(const void *decInst, const void *ppInst,
                    void (*PPRun) (const void *, const DecPpInterface *),
                    void (*PPEndCallback) (const void *),
                    void (*PPConfigQuery) (const void *, DecPpQuery *),
                    void (*PPDisplayIndex)(const void *, u32),
		    void (*PPBufferData) (const void *, u32, u32, u32));

i32 mpeg4UnregisterPP_adapter(const void *decInst, const void *ppInst);

i32 h264RegisterPP_adapter(const void *decInst, const void *ppInst,
                   void (*PPDecStart) (const void *, const DecPpInterface *),
                   void (*PPDecWaitEnd) (const void *),
                   void (*PPConfigQuery) (const void *, DecPpQuery *),
                   void (*PPDisplayIndex) (const void *, u32));

i32 h264UnregisterPP_adapter(const void *decInst, const void *ppInst);



#endif

