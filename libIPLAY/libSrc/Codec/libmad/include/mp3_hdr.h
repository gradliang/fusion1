/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mp3_hdr.h
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/

//#include "stream.h"

int MP3_Bitrate;

struct mad_file_info {
    unsigned char shoutcast;
    unsigned char reserved[3];
    unsigned long bitrate;
    /* for SHOUTcast streaming */
    unsigned long block_size;
    unsigned long skip_bytes;
};

//int mp_get_mp3_header(unsigned char* hbuf,int* chans, int* freq);
//int mp_get_mp3_header_BitRate(unsigned char* hbuf,int* chans, int* freq, int* bitrate);
//int mp_decode_first_frame(unsigned char* hbuf, int* sframe, int* freq);
//int mp_search_mp3_sync(unsigned char* hbuf, int* freq);
unsigned int mp_get_vbr_framenum(unsigned char * hbuf);
//int GetID3Length(uint8_t* hbuf);

#define MP3_BITSTREAM_CACHE 4096
#define MAD_INPUT_BUFFER_SIZE MP3_BITSTREAM_CACHE
#define mp_decode_mp3_header(hbuf)  mp_get_mp3_header(hbuf,NULL,NULL)

static inline int mp_check_mp3_header(unsigned int head){
    if( (head & 0x0000e0ff) != 0x0000e0ff ||  
        (head & 0x00fc0000) == 0x00fc0000) return 0;
    if(mp_decode_mp3_header((unsigned char*)(&head))<=0) return 0;
    return 1;
}
