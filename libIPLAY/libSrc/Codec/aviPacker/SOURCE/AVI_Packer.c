#define LOCAL_DEBUG_ENABLE 1
#include "global612.h"
#include "mpTrace.h"
#include <string.h>
#include "AVIplatform.h"
#include "AVIfmt.h"
#include "AVI_utility.h"

#if(RECORD_ENABLE)
WAVEFORMATEX wavformatex;
int chunksize;

AVIFILE avifile;

// callback function
//---------------------------------------------------------
//---------------------------------------------------------
int MagicPixel_AVIEN_INDEX(AVIINDEXENTRY *index,DWORD index_make)
{
	return aviStreamindex(index,index_make);
}

void MagicPixel_AVIEN_OutSeek_callback(int file_ptr)
{
    Fseek(avifile.pFile, file_ptr, SEEK_SET);
}

inline int MagicPixel_AVIEN_write_callback(unsigned char *buf, int len)//,AVIINDEXENTRY *index) 
{
    int numByteWrite;
	//if(buf[0]==0 &&buf[5]==0 &&buf[11]==0 &&buf[20]==0 &&buf[21]==0 )
   //  mpDebugPrint("====Write back:0===");
    numByteWrite = aviStreamCache(buf, len);//,index);
    //mpDebugPrint("	Write back: %d / %d", numByteWrite, len);

    return numByteWrite;   
}

void *MagicPixel_AVIEN_malloc_callback(DWORD size)
{
//#if (CHIP_VER == CHIP_VER_650)
//    return mem_malloc(size);
//#else
    return (void*)((DWORD)ext_mem_malloc(size) | 0xa0000000);
//#endif
}

void MagicPixel_AVIEN_free_callback(void *p)
{
//#if (CHIP_VER == CHIP_VER_650)
//    mem_free(p);
//#else
    ext_mem_free(p);
//#endif
}

void* MagicPixel_AVIEN_fileOpen_callback(const char *path, const char *mode)
{
//Stream *File_name;
  return  PathAPI__CreateFile(path);
	//return PathAPI__Fopen(path, mode);
}

int MagicPixel_AVIEN_fileClose_callback(void *handle)
{
	return FileClose(handle);	
}


// --- Warning : RIFF files are written in little-endian byte order!! ---

void build_LIST_strl_video()
{
    AVIStreamHeader  aviStreamHeader;
    BITMAPINFOHEADER bitMapInfoHeader;

    wLISTHeader("LIST", VIDEOstrl_SIZE, "strl");

    // --- strh ---
    wFourccHeader("strh", strh_SIZE);

    memset(&aviStreamHeader, 0, sizeof(AVIStreamHeader));
    fillHeader_Video_strh(&aviStreamHeader);
    //fwrite(&aviStreamHeader, sizeof(AVIStreamHeader), 1, pFile);
    MagicPixel_AVIEN_write_callback((char *)&aviStreamHeader, sizeof(AVIStreamHeader));

    // --- strf ---
    wFourccHeader("strf", VIDEOstrf_SIZE);
    memset(&bitMapInfoHeader, 0, sizeof(BITMAPINFOHEADER));
    fillHeader_Video_strf(&bitMapInfoHeader);
    //fwrite(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
    MagicPixel_AVIEN_write_callback((char *)&bitMapInfoHeader, sizeof(BITMAPINFOHEADER));

    // --- strd ---
    // --- strn ---
}

void build_LIST_strl_audio()
{
    AVIStreamHeader  aviStreamHeader;

    strl_audio_setting(&aviStreamHeader);
    wLISTHeader("LIST", AUDIOstrl_SIZE, "strl");

    // --- strh ---
    wFourccHeader("strh", strh_SIZE);
    //fwrite(&aviStreamHeader, sizeof(AVIStreamHeader), 1, pFile);
    MagicPixel_AVIEN_write_callback((char *)&aviStreamHeader, sizeof(AVIStreamHeader));

    // --- strf ---
    wFourccHeader("strf", AUDIOstrf_SIZE);
    //fwrite(&wavformatex, AUDIOstrf_SIZE, 1, pFile);
    chg_wavformatexEndian();
    MagicPixel_AVIEN_write_callback((char *)&wavformatex, AUDIOstrf_SIZE);
    // --- strd ---
    // --- strn ---
}

// Used to build junk section
int buildJUNKsection(STREAM *pf, unsigned int size)
{
    int cnt;
    wFourccHeader(pf, "JUNK", size);
    cnt = MagicPixel_AVIEN_write_callback("", size);

    return cnt;	
}

void build_AVI_Header()
{
    unsigned int size = 0;
    MainAVIHeader mainAviHeader;

    //Build LIST hdrl
#ifndef VIDEO_ONLY
    wLISTHeader("LIST",
            VIDEOstrl_SIZE + 8 + MAINAVIHEADER_SIZE + 8 + 4 + 100,
            "hdrl");
#else
    wLISTHeader("LIST",
            192,
            "hdrl");
#endif

    // --- avih ---
    size = MAINAVIHEADER_SIZE;
    wFourccHeader("avih", size);
    memset(&mainAviHeader, 0, sizeof(MainAVIHeader));
    fillAviMainHeader(&mainAviHeader);
    MagicPixel_AVIEN_write_callback((char *)&mainAviHeader, sizeof(MainAVIHeader));
    build_LIST_strl_video();
#ifndef VIDEO_ONLY
    build_LIST_strl_audio();
#endif
}

#endif
