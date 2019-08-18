///
///@AC3 decode API User Guide 
///The AC3 decoder API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.

///
///@defgroup group_AC3_decode API 
///

#ifndef AC3_LIBRARY_INCLUDE_FILE_
#define AC3_LIBRARY_INCLUDE_FILE_

///@ingroup group_AC3_decode
///@brief   This function will init AC3 decoder.
///@param   int *ch                    return ch number
///@param   int *srate                 return sample rate
///@param   int *frame_size            return pcm size of a frame
///@param   int *bitrate               return bitrate
///@param   int *total_time            return total time
///@param   int file_size              file size
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ac3_init(int *ch,int *srate,int *frame_size,int *bitrate,int *total_time, int file_size);



///@ingroup group_MP3_decode
///@brief   This function will decoder a AC3 frame.
///@param   int *frame_size            return pcm size of a frame
///@param   int *pcm_buf               pcm buffer address
///@param   int *ch                    channer number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ac3_decode(int *frame_size,int **pcm_buf,int *ch,int *play_time,int filesize);




///@ingroup group_AC3_decode
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
void MagicPixel_ac3_post_process(int mono_stereo,int stereo_reverse,int samples,int ch,int **pcm_buf,short *out_buf);



///@ingroup group_AC3_decode
///@brief   This function will handle file access for a AC3 file.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ac3_bitstream_callback(unsigned char *buf, int len);
int MagicPixel_ac3_resync(int file_ptr,int play_time);


#define AC3_OK       0
#define AC3_FAIL     0x80000001


#endif


