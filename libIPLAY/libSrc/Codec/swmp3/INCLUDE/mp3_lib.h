///
///@MP3 decode API User Guide 
///The MP3 decoder API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.

///
///@defgroup group_MP3_decode API 
///


#ifndef MP3_HOST_LIBRARY_INCLUDE_FILE
#define MP3_HOST_LIBRARY_INCLUDE_FILE


#define MPEG1_BITSTREAM          0x00
#define MPEG2_BITSTREAM          0x01

#define MP_LAYER_I     1          /* Layer I */
#define MP_LAYER_II    2          /* Layer II */
#define MP_LAYER_III   3         /* Layer III */

enum mp3_err
{
	MP3_OK = 0x00,
	MP3_CANCEL = 0x01,
	MP3_FILE_NOEXIST = 0x02,
	MP3_FILE_CORRUPT = 0x03,
	MP3_INIT_FAIL = 0x04,
	MP3_TARGET_FAIL = 0x05,
	MP3_DEVICE_FAIL = 0x06,
	MP3_HUFFMAN_FAIL = 0x07,
	MP3_STEREO_FAIL = 0x08,
	MP3_CRC_FAIL= 0x09,
	MP3_BITSTREAM_FAIL = 0x0a,
	MP3_SIDE_FAIL = 0x0b,
	MP3_FREE_BITRATE = 0x0c,
	MP3_RESYNC_FAIL = 0x0d,
	MP3_RESYNC_END = 0x0e,
};

///@ingroup group_MP3_decode
///@brief   This function will resync decoder for forward backword operation
///@param   int audio_video            The flag about AO:0 or AV:1
///@param   int play_time              forward or backward next mini second
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_mp3_resync(int audio_video,int play_time);



///@ingroup group_MP3_decode
///@brief   This function will init MP3 decoder.
///@param   int file_size              The mp3 file file size
///@param   int EQ_enable              enable EQ or not
///@param   int *ch                    return ch number
///@param   int *srate                 return sample rate
///@param   int *frame_size            return pcm size of a frame
///@param   int *bitrate               return bitrate
///@param   int *total_time            return total time
///@param   char **song_singer         return song singer
///@param   char **song_name           return song name
///@param   char **song_comment        return song comment
///@param   char **song_album          return song album
///@param   char **song_genre          return song genre
///@param   char **song_year           return song year
///@param   char **song_track          return song track
///@param   int *mpeg2                 return mpeg2 number
///@param   int *layer                 return layer number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_mp3_init(int file_size,int EQ_enable,int *ch,int *srate,int *frame_size,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char **song_comment,char **song_album,char **song_genre,
                        char **song_year,char *song_track,int *mpeg2,int *layer);



///@ingroup group_MP3_decode
///@brief   This function will decoder a MP3 frame.
///@param   int error_control          enable error contral, default 1
///@param   int EQ_enable              enable EQ or not
///@param   int *pcm_buf               pcm buffer address
///@param   int *play_time             play time for current frame
///@param   int **EQ_band              EQ band number
///@param   int *ch                    channer number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_mp3_decode(int error_control,int EQ_enable,int **pcm_buf,int *play_time,unsigned char **EQ_band,int *ch);



///@ingroup group_MP3_decode
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
void MagicPixel_mp3_post_process(int mono_stereo,int stereo_reverse,int samples,int ch,int **pcm_buf,short *out_buf);


///@ingroup group_MP3_decode
///@brief   This function will handle file access for a MP3 file.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///@param   int *fend_flag             file end flag
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_MP3_bitstream_callback(unsigned char *buf, int len,int *fend_flag);



///@ingroup group_MP3_decode
///@brief   This function will get current position for a MP3 file.
///@param   NONE
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_MP3_getposition_callback(void);


///@ingroup group_MP3_decode
///@brief   This function will seek for a MP3 file.
///@param   int file_ptr               seek position
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
void MagicPixel_MP3_fileseek_callback(int file_ptr);

//ID3
//int MagicPixel_MP3_id3_comp(char *id3_buf, char *id3_tag, int size);
//void MagicPixel_MP3_id3_copy(char *g_song, char *ptr, int size);

#endif
