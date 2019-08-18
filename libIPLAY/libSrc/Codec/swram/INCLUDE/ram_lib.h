///
///@RAM decode API User Guide 
///The RAM decoder API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.

///
///@defgroup group_RM_decode API 
///


#ifndef _RAM_INCLUDE_LIBRARY_FILE
#define _RAM_INCLUDE_LIBRARY_FILE

///@ingroup group_RAM_decode
///@brief   This function will init RAM decoder.
///@param   int *ch                    return ch number
///@param   int *sampling_rate         return sample rate
///@param   int *frame_size            return pcm size of a frame
///@param   int *bitrate               return bitrate
///@param   int *total_time            return total time
///@param   char **song_singer         return song singer
///@param   char **song_name           return song name
///@param   char *sdram                sdram addreee
///@param   int sdram_size             sdram size
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ram_init(int *ch,int *sampling_rate,int *frame_size,int *bitrate,
    int *total_time,char **song_singer,char **song_name,char *sdram_buf,int sdram_size);



///@ingroup group_RAM_decode
///@brief   This function will decoder a OGG frame.
///@param   int *frame_sample          pcm size of a frame
///@param   int **pcm4_buf             pcm buffer address
///@param   int *play_time             play time for current frame
///@param   int EQ_enable              enable EQ or not
///@param   int *EQ_band               EQ band number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ram_decode(int *frame_sample,int **pcm4_buf,int *play_time,int EQ_enable,unsigned char *EQ_band);



///@ingroup group_RAM_decode
///@brief   This function will handle file access for a RAM file.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ram_bitstream_callback(unsigned char *buf, int len);



///@ingroup group_RAM_decode
///@brief   This function will seek for a RAM file.
///@param   int file_ptr               seek position
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
void MagicPixel_ram_fileseek_callback(int file_ptr);



///@ingroup group_RAM_decode
///@brief   This function will resync decoder for forward backword operation
///@param   int play_time              forward or backward next mini second
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ram_resync(int *play_time);



///@ingroup group_RAM_decode
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
void MagicPixel_ram_post_process(int mono_stereo,int stereo_reverse,int samples,int ch,int **pcm_buf,short *out_buf);

#endif

