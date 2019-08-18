///
///@WMA decode API User Guide 
///The WMA decoder API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.

///
///@defgroup group_WMA_decode API 
///


//*@@@+++@@@@******************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//*@@@---@@@@******************************************************************
/*
 * Windows Media Audio (WMA) Decoder API (implementation)
 *
 * Copyright (c) Microsoft Corporation 1999.  All Rights Reserved.
 */

#if 0
int MagicPixel_ASF_AO_Init(int *ch,int *srate,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char **song_album,
                        char **song_genre,char **song_year,char **song_track,int utf16);
#else

///@ingroup group_WMA_decode
///@brief   This function will init WMA decoder.
///@param   int *ch                    return ch number
///@param   int *srate                 return sample rate
///@param   int *bitrate               return bitrate
///@param   int *total_time            return total time
///@param   char **song_singer         return song singer
///@param   char **song_name           return song name
///@param   char **song_comment        return song comment
///@param   char **song_album          return song album
///@param   char **song_genre          return song genre
///@param   char **song_year           return song year
///@param   char **song_track          return song track
///@param   int utf16                  utf16
///@param   int *image_size            return image_size
///@param   int *image_offset          return image offset
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ASF_AO_Init(int *ch,int *srate,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char **song_album,
                        char **song_genre,char **song_year,char **song_track,int utf16,
                        unsigned int *, unsigned int *);
#endif



///@ingroup group_WMA_decode
///@brief   This function will decoder a WMA frame.
///@param   int *out_samples           return pcm size of a frame
///@param   int **pcm_buf[2][2]        pcm buffer address
///@param   int *play_time             play time for current frame
///@param   int *ch                    channer number
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ASF_AO_Decode(int *out_samples,int *pcm_buf[2][2],int *play_time, int eq_enable);



///@ingroup group_WMA_decode
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
void MagicPixel_ASF_AO_Post_Process(int mono_stereo,int stereo_reverse,int samples,int ch,int *pcm_buf[2][2],short *out_buf);



///@ingroup group_WMA_decode
///@brief   If out_samples does not equal to zero, copy part of pcm data back to decoded buffer.
///         This is WMA decoder flow.
///@param   int out_samples     output pcm sample number
///
///@return  NONE
///
///@remark
///
void MagicPixel_ASF_AO_Post_Decode(int out_samples);
void MagicPixel_ASF_AO_EQ_Process(unsigned char *EQ_band);



///@ingroup group_WMA_decode
///@brief   This function will handle file access for a WMA file.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_ASF_bitstream_callback(unsigned char *buf, int len);



///@ingroup group_WMA_decode
///@brief   This function will seek for a WMA file.
///@param   int file_ptr               seek position
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
void MagicPixel_ASF_fileseek_callback(int file_ptr);

///@ingroup group_WMA_decode
///@brief   This function will check asf header status.
///         If there is a new data block, call function - MagicPixel_WMA_Get_NewBlock()
///@remark
///
void MagicPixel_WMA_NewData_CallBack();
void MagicPixel_WMA_Set_EQ_CallBack(int band, int value);
void MagicPixel_WMA_bitstream_callback(unsigned char *buf,int len);
int MagicPixel_ASF_AO_resync(int play_time);
int getTitleLen();
int getAuthorLen();
int getAlbumLen();
int getGenreLen();
int getYearLen();
int getTrackLen(); 
