#include "Avifmt.h"

int MagicPixel_AVIENC_getposition_callback(void);
void MagicPixel_AVIEN_AudioSeek_callback(int file_ptr);
void MagicPixel_AVIEN_ViedeoSeek_callback(int file_ptr);
int MagicPixel_AVIEN_Audio_callback(unsigned char *buf, int len, int *fend_flag);
int MagicPixel_AVIEN_Video_callback(unsigned char *buf, int len, int *fend_flag);
int MagicPixel_AVIEN_write_callback(unsigned char *buf, int len);
void MagicPixel_AVIEN_free_callback(void *p);
void *MagicPixel_AVIEN_malloc_callback(unsigned int size);
void MagicPixel_AVIEN_OutSeek_callback(int file_ptr);
int MagicPixel_AVIEN_INDEX(AVIINDEXENTRY *index,DWORD index_make);
