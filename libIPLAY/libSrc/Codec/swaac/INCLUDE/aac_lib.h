///
///@AAC decode API User Guide 
///The AAC decoder API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.

///
///@defgroup group_AAC_decode API 
///
#ifndef _AAC_DECODER_LIB_INCLUDE_FILE_
#define _AAC_DECODER_LIB_INCLUDE_FILE_


#define MP4_OK             0
#define MP4_FORMAT_FAIL    0x10000
#define MP4_PLAY_END       0x20000
#define MP4_RESYNC_END     0x30000
#define MP4_RESYNC_ADIF    0x40000
#define AAC_OK             0



///@ingroup group_AAC_decode
///@brief   This function will decoder a AAC frame.
///@param   int EQ_enable              enable EQ or not
///@param   int *play_time             play time for current frame
///@param   int **EQ_band              EQ band number
///@param   int *ch                    channer number
///@param   int *pcm_buf               pcm buffer address
///@param   int *frame_size            return pcm size of a frame
///@param   int *fending               return file end or not
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_mp4_decode(int EQ_enable,int **pcm_buf,int *play_time,unsigned char **EQ_band,int *ch,
                          int *frame_size,int *fending);



//for audio-only API
///@ingroup group_AAC_decode
///@brief   This function will resync decoder for forward backword operation
///@param   int play_time              forward or backward next mini second
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_MP4_resync(int play_time);


///@ingroup group_AAC_decode
///@brief   This function will handle file access for a AAC file.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///@param   int *fend_flag             file end flag
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_MP4_bitstream_callback(unsigned char *buf, int len,int *fending);



///@ingroup group_AAC_decode
///@brief   This function will seek for a AAC file.
///@param   int file_ptr               seek position
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
void MagicPixel_MP4_fileseek_callback(int file_ptr);



///@ingroup group_MP3_decode
///@brief   This function will get current position for a AAC file.
///@param   NONE
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_MP4_getposition_callback(void);



///@ingroup group_AAC_decode
///@brief   This function will init AAC decoder.
///@param   int filelen                The AAC file file size
///@param   int *srate                 return sample rate
///@param   int *ch                    return ch number
///@param   int *bitrate               return bitrate
///@param   int *total_time            return total time
///@param   int *total_frame           return total frame number
///@param   int *header_type           return total file header type
///@param   int *sdram_buf             sdram buffer address
///@param   int *sdram_size            sdram buffer size
///@param   char *song_singer          return song singer
///@param   char **song_name           return song name
///@param   char **song_comment        return song comment
///@param   char **song_album          return song album
///@param   char **song_genre          return song genre
///@param   char **song_year           return song year
///@param   char **song_track          return song track
///@param   int fbrowser               enable file browser init or not
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_mp4_init(unsigned int filelen, int *srate, int *ch,int *bitrate,int *total_time,int *total_frame,
                        int *header_type,int *sdram_buf,int sdram_size,
                        char **song_singer,char **song_name,char **song_comment,char **song_album,char **song_genre,
                        char **song_year,char *song_track,int fbrowser);

//for audio-video API


///@ingroup group_AAC_decode
///@brief   This function will init AAC decoder for AV case
///@param   unsigned char *buf_ptr     read AAC data for AV init
///@param   int buf_len                read AAC data length for AV init
///@param   int *samplerate            return sample rate
///@param   int *ch                    return ch number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_avAAC_init(unsigned char *buf_ptr,int buf_len,int *samplerate,int *ch);



///@ingroup group_AAC_decode
///@brief   This function will handle file access for AV case.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_avAAC_bitstream_callback(unsigned char *buf, int len);



///@ingroup group_AAC_decode
///@brief   This function will get AAC frame data for AV case.
///@param   int frame_size             AAC frame size
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
void MagicPixel_AAC_fill_frame(int frame_size);



///@ingroup group_AAC_decode
///@brief   This function will decoder a AAC frame for AV case.
///@param   int EQ_enable              enable EQ or not
///@param   int *pcm_buf               pcm buffer address
///@param   int **EQ_band              EQ band number
///@param   int *ch                    channer number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_aac_decode(int EQ_enable,int **pcm_buf,unsigned char **EQ_band,int *ch);




//for audio-only and audio-video

///@ingroup group_AAC_decode
///@brief   This function will post process pcm buffer for 16-bit output.
///@param   int mono_stereo            enable mono to stereo or not
///@param   int stereo_reverse         enable stereo reverse or not
///@param   int samples                output pcm sample number
///@param   int ch                     channel number
///@param   int **pcm_buf              pcm buffer address
///@param   int *out_buf               output pcm buffer address
///
///@return  NONE
///
///@remark
///
void MagicPixel_AAC_post_process(int mono_stereo,int stereo_reverse,int samples,int ch,int **pcm_buf,short *out_buf);



///@ingroup group_AAC_decode
///@brief   This function will malloc memory for mp4 header.
///@param   unsigned int size          malloc size
///
///@return  malloc memory address
///
///@remark
///
unsigned char *MagicPixel_MP4_malloc(unsigned int size);



///@ingroup group_AAC_decode
///@brief   This function will free memory for mp4 header.
///@param   unsigned char *buf         free memory address
///
///@return  NONE
///
///@remark
///
void MagicPixel_MP4_free(unsigned char *buf);

#endif
