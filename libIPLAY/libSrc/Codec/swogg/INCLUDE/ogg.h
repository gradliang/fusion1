///
///@OGG decode API User Guide 
///The OGG decoder API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.

///
///@defgroup group_OGG_decode API 
///

/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 ********************************************************************/
#ifndef _OGG_H
#define _OGG_H

///@ingroup group_OGG_decode
///@brief   This function will init OGG decoder.
///@param   int file_size              The OGG file file size
///@param   int *ch                    return ch number
///@param   int *srate                 return sample rate
///@param   int *frame_size            return pcm size of a frame
///@param   int *bitrate               return bitrate
///@param   int *total_time            return total time
///@param   char **song_singer         return song singer
///@param   char **song_name           return song name
///@param   char *sdram                sdram addreee
///@param   int sdram_maxsize          sdram size
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_OGG_init(int file_size,int *ch,int *srate,int *frame_size,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char *sdram, int sdram_maxsize);
void MagicPixel_Vorbis_bitstream_callback(unsigned char *buf,int len, int *fending);


///@ingroup group_OGG_decode
///@brief   This function will handle file access for a OGG file.
///@param   unsigned char *buf         read buf address
///@param   int len                    read length
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_OGG_bitstream_callback(unsigned char *buf, int len, int *fending);



///@ingroup group_OGG_decode
///@brief   This function will resync decoder for forward backword operation
///@param   int next_page              search next page from current file position
///@param   int play_time              forward or backward next mini second
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_OGG_resync(int next_page,int play_time);



///@ingroup group_OGG_decode
///@brief   This function will decoder a OGG frame.
///@param   int *out_samples           pcm size of a frame
///@param   int *pcm_buf               pcm buffer address
///@param   int *play_time             play time for current frame
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int MagicPixel_OGG_decode(int *out_samples,int **pcm_buf,int *play_time);



///@ingroup group_OGG_decode
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
void MagicPixel_OGG_post_process(int mono_stereo,int stereo_reverse,int samples,int ch,int **pcm_buf,short *out_buf);

#endif  /* _OGG_H */






