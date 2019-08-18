#ifndef AVIFMT_H
#define AVIFMT_H

typedef unsigned int FOURCC;

/* The RECT structure */
typedef struct tagRECT
{
    WORD  left;
    WORD  top;
    WORD  right;
    WORD  bottom;
} RECT;

typedef struct
{
    DWORD	dwMicroSecPerFrame;       // frame display rate (or 0L)    <Reference Only>
    DWORD	dwMaxBytesPerSec;         // max. transfer rate            <Reference Only>
    DWORD	dwPaddingGranularity;     // pad to multiples of this
                                  // size; normally 2K.
    DWORD	dwFlags;                  // the ever-present flags
    DWORD	dwTotalFrames;            // # frames in file
    DWORD	dwInitialFrames;          //                               <No Use>
    DWORD	dwStreams;
    DWORD	dwSuggestedBufferSize;

    DWORD	dwWidth;
    DWORD	dwHeight;

    DWORD	dwReserved[4];
} MainAVIHeader;

typedef struct
{
    FOURCC  fccType;
    FOURCC  fccHandler;
    DWORD     dwFlags;                  /* Contains AVITF_* flags */
    WORD    wPriority;
    WORD    wLanguage;
    DWORD     dwInitialFrames;
    DWORD     dwScale;	
    DWORD     dwRate;                   /* dwRate / dwScale == samples/second */
    DWORD     dwStart;
    DWORD     dwLength;                 /* In units above... */
    DWORD     dwSuggestedBufferSize;
    DWORD     dwQuality;
    DWORD     dwSampleSize;
    RECT    rcFrame;
} AVIStreamHeader;

// strf format for audio
typedef struct
{
    WORD   wFormatTag;
    WORD   nChannels;
    DWORD     nSamplesPerSec;
    DWORD     nAvgBytesPerSec;
    WORD   nBlockAlign;
    WORD   wBitsPerSample;
} WAVEFORMATEX;

// strf format for video
typedef struct
{
    DWORD 	biSize;
    DWORD  	biWidth;
    DWORD  	biHeight;
    WORD 	biPlanes;
    WORD 	biBitCount;
    DWORD 	biCompression;
    DWORD 	biSizeImage;
    DWORD  	biXPelsPerMeter;
    DWORD  	biYPelsPerMeter;
    DWORD 	biClrUsed;
    DWORD 	biClrImportant;
} BITMAPINFOHEADER;


typedef struct
{
    FOURCC  fccType;
    DWORD     dwFlag;
    DWORD     dwChunkOffset;		// Position of chunk
    DWORD     dwChunkLength;		// Length of chunk
} AVIINDEXENTRY;

#ifdef AVIPLATFORM_PC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (unsigned int)(unsigned char)(ch0) |         \
        ( (unsigned int)(unsigned char)(ch1) << 8 )  |  \
        ( (unsigned int)(unsigned char)(ch2) << 16 ) |  \
        ( (unsigned int)(unsigned char)(ch3) << 24 ) )
#else
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (unsigned int)(unsigned char)(ch0) << 24 |   \
        ( (unsigned int)(unsigned char)(ch1) << 16 )|   \
        ( (unsigned int)(unsigned char)(ch2) << 8 ) |   \
        ( (unsigned int)(unsigned char)(ch3) << 0 ) )
#endif

/* flags for use in <dwFlags> in AVIFileHdr */
#define AVIF_HASINDEX		0x00000010	// Index at end of file?
#define AVIF_MUSTUSEINDEX	0x00000020
#define AVIF_ISINTERLEAVED	0x00000100
#define AVIF_TRUSTCKTYPE	0x00000800	// Use CKType to find key frames?
#define AVIF_WASCAPTUREFILE	0x00010000
#define AVIF_COPYRIGHTED	0x00020000

/* Flags for AVIINDEXENTRY */
#define AVIIF_LIST          0x00000001L // chunk is a 'LIST'
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
#define AVIIF_NOTIME	    0x00000100L // this frame doesn't take any time
#define AVIIF_COMPUSE       0x0FFF0000L // these bits are for compressor use


#define MAINAVIHEADER_SIZE  56
#define strh_SIZE           56
#define VIDEOstrf_SIZE      40
#define VIDEOstrl_SIZE      (strh_SIZE + 8 + VIDEOstrf_SIZE + 8 + 4)

#define AUDIOstrf_SIZE      16
#define AUDIOstrl_SIZE      92
#define AUDIOHDLR_SIZE      168
#define AUDIOIDX1_SIZE      16
#define BUFFER_SIZE         4096

//wav define
#define WAV_HEADER_SIZE     44
#define WAV_START           20

//#define VIDEO_ONLY
#define AUDCNT              40   // 1939236/49152 = 40

#endif

