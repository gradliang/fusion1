

#include <string.h>
#include "AVIplatform.h"
#include "AVIfmt.h"
#include "AVI_utility.h"
#include "AVI_File.h"

#if(RECORD_ENABLE)
extern WAVEFORMATEX wavformatex;
extern AVIFILE avifile;
extern unsigned int jpg_cnt;	//This variable records total video frames
extern int fps_record;
extern DWORD    VIDEO_WIDTH;
extern DWORD    VIDEO_HEIGHT;

unsigned int dwEndianConvert(unsigned int in)
{
#ifdef AVIENDIAN_CHG
	unsigned int asize;
	asize = in;
	unsigned char *p = &asize;

    //mpDebugPrint("		size %x", *(unsigned int*)p);
    *(unsigned int *)p = (unsigned int)((*(p + 3) << 24)| (*(p +2) << 16) | (*(p + 1) << 8) | (*(p) << 0));
    //mpDebugPrint("		size2 %x", *(unsigned int*)p);

    return (unsigned int) *(unsigned int *)p;
#else
    return in;
#endif	
}

unsigned short wEndianConvert(unsigned short in)
{
#ifdef AVIENDIAN_CHG
	unsigned short asize2;
    asize2 = in;
    unsigned char *p = &asize2;

    //mpDebugPrint("		size %x", *(unsigned int*)p);
    *(short *)p = (short)((*(p + 1) << 8) | (*(p) << 0));
    //mpDebugPrint("		size2 %x", *(unsigned int*)p);

    return (short) *(short *)p;
#else
    return in;
#endif	
}
	
void wFourccHeader(const char *chunkId, unsigned int size)
{
    unsigned int p;
    unsigned char tmp[8];
    //mpDebugPrint("	write fourcc header %s...", chunkId);

    p = dwEndianConvert(size);
    memcpy(tmp, chunkId, 4);
    memcpy(&tmp[4], (char *)&p, 4);

    //mpDebugPrint("----------------------------");
    //mpDebugPrint("%x%x%x%x%x%x%x%x", *(chunkId+0), *(chunkId+1), *(chunkId+2), *(chunkId+3), *((char *)(&p) + 0), *((char *)(&p) + 1), *((char *)(&p) + 2), *((char *)(&p) + 3));
    //mpDebugPrint("%x%x%x%x%x%x%x%x", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7]);
    //mpDebugPrint("----------------------------");
	MagicPixel_AVIEN_write_callback((unsigned char *)((DWORD)tmp | 0xa0000000), 8);
}
void wFourccHeader_index(const char *chunkId, unsigned int size,STREAM * handle)
{
    unsigned int p;
    unsigned char tmp[8];
    //mpDebugPrint("	write fourcc header %s...", chunkId);

    p = dwEndianConvert(size);
    memcpy(tmp, chunkId, 4);
    memcpy(&tmp[4], (char *)&p, 4);

    //mpDebugPrint("----------------------------");
    //mpDebugPrint("%x%x%x%x%x%x%x%x", *(chunkId+0), *(chunkId+1), *(chunkId+2), *(chunkId+3), *((char *)(&p) + 0), *((char *)(&p) + 1), *((char *)(&p) + 2), *((char *)(&p) + 3));
    //mpDebugPrint("%x%x%x%x%x%x%x%x", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7]);
    //mpDebugPrint("----------------------------");
	//MagicPixel_AVIEN_write_callback((unsigned char *)((DWORD)tmp | 0xa0000000), 8);
	//+++oo
	FileWrite_withWriteBack_FileSize(handle,tmp, 8,1);

	//FileWrite(handle,tmp, 8);

}

// Only for chunk of RIFF amd LIST type
void wLISTHeader(char *chunkId, unsigned int size, char *fmt)
{
    wFourccHeader(chunkId, size);
    MagicPixel_AVIEN_write_callback((char *)fmt, 4);
}

void fillAviMainHeader(MainAVIHeader *h)
{

	
#if AUTO_GET_RECORD_FPS_TIMER
	if (fps_record)
		h->dwMicroSecPerFrame =dwEndianConvert(1000000/fps_record);
	else
		h->dwMicroSecPerFrame =dwEndianConvert(166667);
#else
	switch(fps_record)
  	{
	case 60:	
	h->dwMicroSecPerFrame = dwEndianConvert(16667);
	break;
	case 50:	
	h->dwMicroSecPerFrame = dwEndianConvert(20000);
	break;
	case 30:	
	h->dwMicroSecPerFrame = dwEndianConvert(33333);
	break;
	case 24:	
	h->dwMicroSecPerFrame = dwEndianConvert(41666);
	break;
    case 20:
	h->dwMicroSecPerFrame = dwEndianConvert(50000);
    break;
	case 16:
	h->dwMicroSecPerFrame = dwEndianConvert(62500);	
    break;
	case 12:
	h->dwMicroSecPerFrame =dwEndianConvert( 83333); // 12 frame in one sec	
    break;
	case 6:
	h->dwMicroSecPerFrame =dwEndianConvert(166667);// 6 frame in one sec	
    break;
	default:
	h->dwMicroSecPerFrame =166667;	
	break;	
    }
#endif
	//+++++++++++++++++++++++
	//h->dwMaxBytesPerSec   = 43090;         // max. transfer rate
    //h->dwPaddingGranularity;
	mpDebugPrint("build_AVI_Header_jpg_cnt_2: %d",jpg_cnt);

    h->dwFlags            = dwEndianConvert((int)(AVIF_HASINDEX));	//(int)(AVIF_HASINDEX | AVIF_MUSTUSEINDEX);
    h->dwTotalFrames      = dwEndianConvert(jpg_cnt);
#ifndef VIDEO_ONLY
    h->dwStreams          = dwEndianConvert(2);
#else
    h->dwStreams          = 1;
#endif

    h->dwWidth            = dwEndianConvert(VIDEO_WIDTH);
    h->dwHeight           = dwEndianConvert(VIDEO_HEIGHT);
}

void video_INDEXENTRY_init(AVIINDEXENTRY *entry)
{
    extern int chunksize;    //It should be rearrange

    entry->fccType       = mmioFOURCC('0', '0', 'd', 'c');
    entry->dwFlag        = dwEndianConvert(AVIIF_LIST);

#ifndef VIDEO_ONLY
    entry->dwChunkOffset = 4 + chunksize + 8 * AUDCNT;
#else
    entry->dwChunkOffset = 4;
#endif

    entry->dwChunkLength = 0;
}

void fillHeader_Video_strh(AVIStreamHeader  *h)
{
    h->fccType    = mmioFOURCC('v', 'i', 'd', 's');
    h->fccHandler = mmioFOURCC('m', 'j', 'p', 'g');
    //int     dwFlags;                  /* Contains AVITF_* flags */
    //int     wPriority;
    //int     dwInitialFrames;
    h->dwScale    = dwEndianConvert(1);	

	h->dwRate     = dwEndianConvert(fps_record);
	//++++++++++++++
                     /* dwRate / dwScale == samples/second */
    h->dwStart    = dwEndianConvert(0);
    h->dwLength   = dwEndianConvert(jpg_cnt);	// dwEndianConvert(IMG_CNT /8);         /* In units above... */
    h->dwSuggestedBufferSize = dwEndianConvert(1 << 20);
    //int     dwQuality;
    //int     dwSampleSize;
    h->rcFrame.right  = wEndianConvert(VIDEO_WIDTH);
    h->rcFrame.bottom = wEndianConvert(VIDEO_HEIGHT);
}

void fillHeader_Video_strf(BITMAPINFOHEADER *h)
{
    h->biSize        = dwEndianConvert(40);
    h->biWidth       = dwEndianConvert(VIDEO_WIDTH);
    h->biHeight      = dwEndianConvert(VIDEO_HEIGHT);
    h->biPlanes      = wEndianConvert(1);
    h->biBitCount    = wEndianConvert(24);
    h->biCompression = mmioFOURCC('M', 'J', 'P', 'G');
    //h->biSizeImage   = VIDEO_WIDTH * VIDEO_HEIGHT * 3;  
}

void strl_audio_setting(AVIStreamHeader *aviStreamHeader)
{
    unsigned int sample;
    unsigned short ch;
    aviStreamHeader->fccType = mmioFOURCC('a', 'u', 'd', 's');
    aviStreamHeader->fccHandler = 0;
    aviStreamHeader->dwFlags    = 0;
    aviStreamHeader->wPriority  = 0;
    aviStreamHeader->wLanguage  = 0;
    aviStreamHeader->dwInitialFrames = 0;
#if 0    
    aviStreamHeader->dwScale = AVIENDIAN_PAD(wavformatex.nChannels);
    sample = dwEndianConvert(wavformatex.nSamplesPerSec);
    ch = wEndianConvert(wavformatex.nChannels);
    aviStreamHeader->dwRate =  dwEndianConvert(sample*ch);
    aviStreamHeader->dwSampleSize   = AVIENDIAN_PAD(2 * wavformatex.nChannels);

#else

    aviStreamHeader->dwScale = AVIENDIAN_PAD(wavformatex.nChannels);
    sample = (wavformatex.nSamplesPerSec);
    ch     = (wavformatex.nChannels);
    aviStreamHeader->dwRate =  dwEndianConvert(sample * ch);
    aviStreamHeader->dwSampleSize   = AVIENDIAN_PAD(1 * wavformatex.nChannels);
#endif
    aviStreamHeader->dwStart  = 0;
    aviStreamHeader->dwLength = 0;//aviStreamHeader->dwRate * 0; //important?
    aviStreamHeader->dwSuggestedBufferSize = 0;
    aviStreamHeader->dwQuality      = 0;
    aviStreamHeader->rcFrame.bottom = 0;
    aviStreamHeader->rcFrame.left   = 0;
    aviStreamHeader->rcFrame.right  = 0;
    aviStreamHeader->rcFrame.top    = 0;
}

void chg_wavformatexEndian()
{
    wavformatex.wFormatTag      = wEndianConvert(wavformatex.wFormatTag);
    wavformatex.nChannels       = wEndianConvert(wavformatex.nChannels);
    wavformatex.nSamplesPerSec  = dwEndianConvert(wavformatex.nSamplesPerSec);
    wavformatex.nAvgBytesPerSec = dwEndianConvert(wavformatex.nAvgBytesPerSec);
    wavformatex.nBlockAlign     = wEndianConvert(wavformatex.nBlockAlign);
    wavformatex.wBitsPerSample  = wEndianConvert(wavformatex.wBitsPerSample);
}

#if 0
// This function will write all data of inFile to outFile
// If the file size is odd, patch one byte NULL data at last.
void writeFile_audio(void)
{
    int ret, cnt = 0;
    char buff[4096];
    char *pbuf = MPX_NONCACHE((unsigned int)buff);
    int fend_flag;
	
    while(1)
    {
        ret = MagicPixel_AVIEN_Audio_callback(pbuf, 4096, &fend_flag);
        cnt += MagicPixel_AVIEN_write_callback(pbuf, ret);

        if(fend_flag)
            break;
    }
	
    if(cnt & 1)
    {
        cnt += MagicPixel_AVIEN_write_callback("\0", 1);
        //printf("	<write file>padding even...\n");
    }	
}


int writeFile_video(void)
{
    int ret, cnt = 0;
    char buff[4096];
    char *pbuf = MPX_NONCACHE((unsigned int)buff);
    int fend_flag;
	
    while(1)
    {
        ret = MagicPixel_AVIEN_Video_callback(pbuf, 4096, &fend_flag);
        cnt += MagicPixel_AVIEN_write_callback(pbuf, ret);

        if(fend_flag)
            break;
    }

    if(cnt & 1)
    {
        cnt += MagicPixel_AVIEN_write_callback("\0", 1);
        //printf("	<write file>padding even...\n");
    }	

    return cnt;
}
#endif

#endif
