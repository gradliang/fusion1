
/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mptrace.h"
#include "dma_cache.h"

#if AAC_FAAD_AUDIO
#define FAAD_MIN_STREAMSIZE 768 /* 6144 bits/channel */
#define FAAD_NUM_CHANNELS   6

#define FAAD_BITSTREAM_CACHE (FAAD_MIN_STREAMSIZE * FAAD_NUM_CHANNELS)
#endif

#if MP3_MAD_AUDIO || WAV_ENABLE
#define BITSTREAM_CACHE 4096
#endif

BYTE *Bitstream_Buffer;
BYTE *Bitstream_Buffer2;
static WORD Cache_Ptr;
static WORD Cache_End_Ptr;
static int fend_flag = 0;

#if AAC_FAAD_AUDIO
/********************************/
/* Move From faad/source/common.c */
/********************************/

void *faad_malloc(size_t size)
{
    return (void *)mem_malloc(size);
}

/* common free function */
void faad_free(void *b)
{
    mem_free(b);
}
/********************************/
/* Move From mp4ff/source/mp4util.c */
/********************************/
void *mp4ff_malloc(uint32_t size)
{
    return (void *)mem_malloc(size);
}

void mp4ff_free(void *b)
{
    mem_free(b);
}

void *mp4ff_realloc(void *b, uint32_t newsize)
{
    return (void *)mem_reallocm(b, newsize);
}
#endif

/********************************/
/* create for libmad			*/
/********************************/
#if MP3_MAD_AUDIO
void *libmad_malloc(size_t size)
{
    return (void *)mem_malloc(size);
}

/* common free function */
void libmad_free(void *b)
{
    mem_free(b);
}
#endif

void dma_invalid_dcache(void)
{
   unsigned long flags;

   flags = read_c0_xcontext();
   flags &= ~1;
   write_c0_xcontext(flags);
   flags |= 1;
   write_c0_xcontext(flags);
}

void Free_BitstreamCache(void)
{
	if(Bitstream_Buffer){
		(DWORD)Bitstream_Buffer &= ~0x20000000;
		mem_free((void *)Bitstream_Buffer);
	    Bitstream_Buffer = NULL;
    	Bitstream_Buffer2 = NULL;
	}
	else{
		mpDebugPrint("Is freeing bistream cache needed");
	}
}
#if MP3_MAD_AUDIO || WAV_ENABLE
void ClearBitstreamCache(void)
{
    memset(Bitstream_Buffer, 0x0, 2 * BITSTREAM_CACHE);
}
#endif

void Init_BitstreamCache(enum FTYPE format)
{
    switch (format) {
        #if WAV_ENABLE || MP3_MAD_AUDIO
        case WAVE_TYPE :
        case MP3_TYPE :
            Bitstream_Buffer = (BYTE *)mem_malloc(2 * BITSTREAM_CACHE);
			Bitstream_Buffer = (BYTE *)((int)Bitstream_Buffer| 0x20000000);
            Bitstream_Buffer2 = Bitstream_Buffer + BITSTREAM_CACHE; 
            memset(Bitstream_Buffer, 0x0, 2 * BITSTREAM_CACHE);
            break;
        #endif  
        #if AAC_FAAD_AUDIO
        case AAC_TYPE :
            Bitstream_Buffer = (BYTE *) mem_malloc(2 * FAAD_BITSTREAM_CACHE); 
            Bitstream_Buffer2 = Bitstream_Buffer + FAAD_BITSTREAM_CACHE; 
            memset(Bitstream_Buffer, 0x0, 2 * FAAD_BITSTREAM_CACHE);
            break;
        #endif
    }

    Cache_Ptr = 0;
    Cache_End_Ptr = 0;
	fend_flag = 0;
}

BYTE * InitBitStreamBufferTo()
{
    return Bitstream_Buffer;
}

BYTE * GetCacheBuffer2()
{
    return Bitstream_Buffer2;
}

void SetCachePtr(int fptr)
{
    Cache_Ptr = fptr;
}

void SetCacheEndPtr(int fptr)
{
    Cache_End_Ptr = fptr;
}

DWORD GetCachePtr()
{
    return Cache_Ptr;
}

DWORD GetCacheEndPtr()
{
    return Cache_End_Ptr;
}

void SetFileEndFlag(BYTE value)
{
    fend_flag = value;
}

int Fill_Cache_Buffer(enum FTYPE format)
{   
    SetCachePtr(0);
    switch (format) {

#if WAV_ENABLE
	case WAVE_TYPE :
	    SetCacheEndPtr(MagicPixel_WAV_bitstream_callback(Bitstream_Buffer, BITSTREAM_CACHE, &fend_flag));
	    break;
#endif

#if MP3_MAD_AUDIO
	case MP3_TYPE :
	    SetCacheEndPtr(MagicPixel_MP3_MAD_bitstream_callback(Bitstream_Buffer, BITSTREAM_CACHE, &fend_flag));
	    break;
#endif
#if AAC_FAAD_AUDIO
	case AAC_TYPE :
	    SetCacheEndPtr(MagicPixel_AAC_FAAD_bitstream_callback(Bitstream_Buffer, FAAD_BITSTREAM_CACHE, &fend_flag));
	    break;
#endif 

	default:
		mpDebugPrint("[Error] Fill Cache Buffer fail...");
    }
    return fend_flag;
}

int Fill_Cache_Buffer2(enum FTYPE format)
{
    SetCachePtr(0);
    switch (format) {
#if MP3_MAD_AUDIO
	case MP3_TYPE :
		SetCacheEndPtr(MagicPixel_MP3_MAD_bitstream_callback(Bitstream_Buffer2, BITSTREAM_CACHE, &fend_flag));
		break;
#endif
#if AAC_FAAD_AUDIO
    case AAC_TYPE :
        SetCacheEndPtr(MagicPixel_AAC_FAAD_bitstream_callback(Bitstream_Buffer2, FAAD_BITSTREAM_CACHE, &fend_flag));
        break;
#endif

	default:
		mpDebugPrint("[Error] illegal Fill cache buffer type");
    }

	return fend_flag;
}

int MoveReservedBytesandFillinCache(WORD ret, enum FTYPE format)
{
    int start, end, bytes, bytes_read;

    MP_DEBUG2("Bitstream_Buffer=0x%x ret=%d",Bitstream_Buffer ,ret );

	switch (format)
	{
#if WAV_ENABLE
	case WAVE_TYPE :
		SetCachePtr(0);
		memmove(Bitstream_Buffer, Bitstream_Buffer + ret, GetCacheEndPtr() - ret);
		bytes = MagicPixel_WAV_bitstream_callback(Bitstream_Buffer + BITSTREAM_CACHE - ret,ret, &fend_flag);
		SetCacheEndPtr(BITSTREAM_CACHE);

		return BITSTREAM_CACHE;
#endif

#if MP3_MAD_AUDIO
	case MP3_TYPE :
		start = GetCachePtr();
		end = GetCacheEndPtr();
		bytes = end - start;

		SetCachePtr(0);
		memmove(Bitstream_Buffer, Bitstream_Buffer + start, bytes);
		end = bytes;
		bytes = BITSTREAM_CACHE - end;
		bytes_read = MagicPixel_MP3_MAD_bitstream_callback(Bitstream_Buffer2 - bytes, BITSTREAM_CACHE + bytes, &fend_flag);

		if ((end + bytes_read) <= BITSTREAM_CACHE) {
			SetCacheEndPtr(end + bytes_read);
			bytes = 0;
		} else {
			SetCacheEndPtr(BITSTREAM_CACHE);
			bytes = end + bytes_read - BITSTREAM_CACHE;
		}

		return bytes;
#endif
#if AAC_FAAD_AUDIO
    case AAC_TYPE :
        start = GetCachePtr();
        end = GetCacheEndPtr();
        bytes = end - start;

        SetCachePtr(0);
        memmove(Bitstream_Buffer, Bitstream_Buffer + start, bytes);
        end = bytes;
        bytes = FAAD_BITSTREAM_CACHE - end;
        bytes_read = MagicPixel_AAC_FAAD_bitstream_callback(Bitstream_Buffer2 - bytes, FAAD_BITSTREAM_CACHE + bytes, &fend_flag);

        if ((end + bytes_read) <= FAAD_BITSTREAM_CACHE) {
            SetCacheEndPtr(end + bytes_read);
            bytes = 0;
        } else {
            SetCacheEndPtr(FAAD_BITSTREAM_CACHE);
            bytes = end + bytes_read - FAAD_BITSTREAM_CACHE;
        }

        return bytes;        
#endif
    }
}

BYTE GetFileEndFlag()
{
    return fend_flag;
}

int GetCurPos(enum FTYPE format)
{
    switch (format) {
#if WAV_ENABLE
		case WAVE_TYPE :
#endif
#if MP3_MAD_AUDIO
		case MP3_TYPE :
		    return MagicPixel_MP3_MAD_getposition_callback() - BITSTREAM_CACHE + GetCachePtr();
#endif
#if AAC_FAAD_AUDIO
        case AAC_TYPE :
            return MagicPixel_AAC_FAAD_getposition_callback() - FAAD_BITSTREAM_CACHE + GetCachePtr();
#endif
		default:
			return 0;
    }
}  

