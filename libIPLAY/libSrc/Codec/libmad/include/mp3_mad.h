#ifndef MP3_HOST_LIBRARY_INCLUDE_FILE
#define MP3_HOST_LIBRARY_INCLUDE_FILE

int MagicPixel_mp3_mad_resync(int audio_video,int play_time);
int MagicPixel_mp3_mad_init(int file_size,int *ch,int *srate,int *frame_size,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char **song_comment,char **song_album,char **song_genre,
                        char **song_year,char **song_track,int *mpeg2,int *layer,BYTE filebrowser);
int MagicPixel_mp3_mad_decode(int error_control,int EQ_enable,int **pcm_buf,int *play_time,unsigned char **EQ_band,int *ch);
void MagicPixel_mp3_mad_post_process(int mono_stereo,int stereo_reverse,int samples,int ch,int **pcm_buf,short *out_buf);

int MagicPixel_MP3_MAD_bitstream_callback(unsigned char *buf, int len,int *fend_flag);
int MagicPixel_MP3_MAD_getposition_callback(void);
void MagicPixel_MP3_MAD_fileseek_callback(int file_ptr);

#endif
