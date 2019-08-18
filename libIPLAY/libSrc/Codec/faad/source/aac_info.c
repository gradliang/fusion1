/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: aac_info.c,v 1.0 2008/02/18 08:20:11 menno Exp $
**/

#include "global612.h"
#include "common.h"
#include "structs.h"
#include "syntax.h"
#include "faad.h"
#include "mp4ff.h"
#include "aac_info.h"
#include "id3_tag.h"

/* FAAD file buffering routines */
#if 0
static unsigned char *aac_stream_buffer;
static unsigned char *stream_buffer, *cache_buffer;
static int buffer_start, buffer_end, at_eof;
#endif
#define AAC_TYPE 2

#if 0
void faad_init_stream_cache(void)
{
    aac_stream_buffer = (unsigned char *)faad_malloc(2 * FAAD_BITSTREAM_CACHE);
    memset(aac_stream_buffer, 0, 2 * FAAD_BITSTREAM_CACHE);
    stream_buffer = aac_stream_buffer;
    cache_buffer = aac_stream_buffer + FAAD_BITSTREAM_CACHE;
    buffer_start = buffer_end = 0;
    at_eof = 0;
}

void faad_uninit_stream_cache(void)
{
    faad_free(aac_stream_buffer);
    aac_stream_buffer = stream_buffer = cache_buffer = NULL;
}

unsigned char *faad_get_stream_buffer(void)
{
    return stream_buffer;
}

unsigned char *faad_get_cache_buffer(void)
{
    return cache_buffer;
}

void faad_set_buffer_start(int ptr)
{
    buffer_start = ptr;
}

void faad_set_buffer_end(int ptr)
{
    buffer_end = ptr;
}

int faad_get_buffer_start(void)
{
    return buffer_start;
}

int faad_get_buffer_end(void)
{
    return buffer_end;
}

int faad_fill_stream_buffer(void)
{
    faad_set_buffer_start(0);
    faad_set_buffer_end(MagicPixel_AAC_FAAD_bitstream_callback(stream_buffer, FAAD_BITSTREAM_CACHE, &at_eof));
    return at_eof;
}

int faad_fill_cache_buffer(void)
{
    faad_set_buffer_start(0);
    faad_set_buffer_end(MagicPixel_AAC_FAAD_bitstream_callback(cache_buffer, FAAD_BITSTREAM_CACHE, &at_eof));
    return at_eof;
}

int faad_move_remain_fill_cache(void)
{
    int start, end, bytes, bytes_read;
    
    start = faad_get_buffer_start();
    end = faad_get_buffer_end();
    bytes = end - start;

    faad_set_buffer_start(0);
    memmove(stream_buffer, stream_buffer + start, bytes);
    end = bytes;
    bytes = FAAD_BITSTREAM_CACHE - end;
    bytes_read = MagicPixel_AAC_FAAD_bitstream_callback(cache_buffer - bytes, FAAD_BITSTREAM_CACHE + bytes, &at_eof);
    if ((end + bytes_read) <= FAAD_BITSTREAM_CACHE) {
        faad_set_buffer_end(end + bytes_read);
        bytes = 0;
    } else {
        faad_set_buffer_end(FAAD_BITSTREAM_CACHE);
        bytes = end + bytes_read - FAAD_BITSTREAM_CACHE;
    }
    return bytes;
}

unsigned int faad_get_cur_pos(void)
{
    return MagicPixel_AAC_FAAD_getposition_callback() - FAAD_BITSTREAM_CACHE + faad_get_buffer_start();// + 1;
}
#endif

unsigned int faad_seek_buffer(unsigned int pos)
{
    unsigned int cur_pos;

    cur_pos = MagicPixel_AAC_FAAD_getposition_callback();
    if ((cur_pos - pos) <= FAAD_BITSTREAM_CACHE) {
        SetCachePtr(pos + FAAD_BITSTREAM_CACHE - cur_pos);
    } else {
       cur_pos = pos % FAAD_BITSTREAM_CACHE;
       MagicPixel_AAC_FAAD_fileseek_callback(pos - cur_pos);
       Fill_Cache_Buffer(AAC_TYPE);
       SetCachePtr(cur_pos);
    }
    return 0;
}

unsigned int faad_read_buffer(unsigned char *data, int length)
{
    int start, end, bytes, remain;
    unsigned int bytes_read = 0;

    start = GetCachePtr();
    end = GetCacheEndPtr();
    bytes = end - start;

    if (length > bytes) {
        remain = length - bytes;
        memcpy(data, (char *)InitBitStreamBufferTo()+start, bytes);
        bytes_read = bytes;
        Fill_Cache_Buffer(AAC_TYPE);
        bytes_read += faad_read_buffer(data + bytes, remain);
    } else {
        memcpy(data, (char *)InitBitStreamBufferTo()+start, length);
        SetCachePtr(start + length);
        bytes_read = length;
    }
    return bytes_read;
}

void faad_skip_buffer(int length)
{
    int start, end, bytes, remain;

    start = GetCachePtr();
    end = GetCacheEndPtr();
    bytes = end - start;

    if (length > bytes) {
        remain = length - bytes;
        Fill_Cache_Buffer(AAC_TYPE);
        faad_skip_buffer(remain);
    } else {
        SetCachePtr(start + length);
    }
}

uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
    return faad_read_buffer((unsigned char *)buffer, (int)length);
}

uint32_t seek_callback(void *user_data, uint64_t position)
{
    return faad_seek_buffer((unsigned int)position);
}

int get_aac_track(mp4ff_t *infile)
{
    /* find AAC track */
    int i, rc;
    int num_tracks = mp4ff_total_tracks(infile);

    for (i = 0; i < num_tracks; i++) {
        unsigned char *buffer = NULL;
        int buffer_size = 0;
        mp4AudioSpecificConfig mp4ASC;

        mp4ff_get_decoder_config(infile, i, &buffer, &buffer_size);

        if (buffer) {
            rc = NeAACDecAudioSpecificConfig(buffer, buffer_size, &mp4ASC);
            faad_free(buffer);

            if (rc < 0)
                continue;
            return i;
        }
    }

    /* can't decode this */
    return -1;
}

static int get_mp4_info(int file_length, NeAACDecFileInfo *info)
{
    mp4ff_t *infile;
    int track, num_samples, result = 0;
    NeAACDecHandle hDecoder;
    unsigned char *buffer = NULL;
    int buffer_size = 0;

    /* initialise the callback structure */
    mp4ff_callback_t *mp4cb = (mp4ff_callback_t *)faad_malloc(sizeof(mp4ff_callback_t));
    mp4cb->read = read_callback;
    mp4cb->seek = seek_callback;
    mp4cb->user_data = NULL;

    hDecoder = NeAACDecOpen();

    infile = mp4ff_open_read(mp4cb);
    if ((track = get_aac_track(infile)) < 0) {
        result = -1;
        goto exit;
    }

    mp4ff_get_decoder_config(infile, track, &buffer, &buffer_size);

    if (NeAACDecInit2(hDecoder, buffer, buffer_size, &info->sampling_rate, &info->channels) < 0) {
        faad_free(buffer);
        result = -1;
        goto exit;
    }
    faad_free(buffer);

    info->object_type = hDecoder->object_type;
    info->bitrate = mp4ff_get_avg_bitrate(infile, track);
    num_samples = mp4ff_num_samples(infile, track);
    info->length = (num_samples << 10) / info->sampling_rate;

exit:
    NeAACDecClose(hDecoder);
    mp4ff_close(infile);
    faad_free(mp4cb);
    return result;
}

#define ADIF_MAX_SIZE 64    /* more than enough */
#define ADTS_MAX_SIZE 16    /* more than enough */

static int read_adif_header(int file_length, NeAACDecFileInfo *info)
{
    bitfile ld;
    adif_header *adif;
    unsigned char buffer[ADIF_MAX_SIZE], sf_index;

    info->header_type = ADIF;

    adif = (adif_header *)faad_malloc(sizeof(adif_header));
    faad_read_buffer(buffer, ADIF_MAX_SIZE);
    faad_initbits(&ld, buffer, ADIF_MAX_SIZE);
    get_adif_header(adif, &ld);
    if (ld.error) {
        faad_endbits(&ld);
        faad_free(adif);
        return -1;
    }
    
    info->bitrate = adif->bitrate;
    info->object_type = adif->pce[0].object_type + 1;
    sf_index = adif->pce[0].sf_index;
    info->sampling_rate = get_sample_rate(sf_index);
    info->channels = adif->pce[0].channels;
    info->length = (file_length << 3) / info->bitrate;

    faad_endbits(&ld);
    faad_free(adif);
    return 0;
}

static int read_adts_header(int file_length, NeAACDecFileInfo *info, int seconds)
{
    unsigned char buffer[ADTS_MAX_SIZE], sync_err, id, sf_index;
    int frames, total_framelength = 0, frame_length;
    int frames_per_sec, bytes_per_frame, i, j;
    unsigned int cur_pos;

    info->header_type = ADTS;
    cur_pos = GetCurPos(AAC_TYPE);

    /* read all frames to ensure correct time and bitrate */
    for (frames = 0; /* */; frames++) {
        /* only go until we hit requested seconds worth */
        if (frames >= 43 * seconds)
            break;

        faad_read_buffer(buffer, ADTS_MAX_SIZE);
        /* check syncword */
//        if (!((buffer[0] == 0xFF) && ((buffer[1] & 0xF6) == 0xF0)))
//            break;

        /* try to recover from sync errors */
        sync_err = 1;
        for (i = 0, j = 0; i < (ADTS_MAX_SIZE - 1); i++, j++) {
            if ((buffer[i] == 0xFF) && ((buffer[i + 1] & 0xF6) == 0xF0)) {
                sync_err = 0;
                break;
            } else if (i == (ADTS_MAX_SIZE - 2)) {
                buffer[0] = buffer[i + 1];
                faad_read_buffer(buffer + 1, ADTS_MAX_SIZE - 1);
                i = -1;
            }
            if (j >= FAAD_MIN_STREAMSIZE)
                break;
        }
        if (sync_err) {
            return -1;
        } else if (i != 0) {
            for (j = 0; j < (ADTS_MAX_SIZE - i); j++)
                buffer[j] = buffer[i + j];
            faad_read_buffer(buffer + j, i);
        }

        if (frames == 0) {
            /* fixed ADTS header is the same for every frame, so we read it only once */
            /* syncword found, proceed to read in the fixed ADTS header */
            id = buffer[1] & 0x08;
            if (id == 0)
                info->version = 4;
            else    /* MPEG-2 */
                info->version = 2;
            info->object_type = ((buffer[2] & 0xC0) >> 6) + 1;
            sf_index = (buffer[2] & 0x3C) >> 2;
            info->sampling_rate = get_sample_rate(sf_index);
            info->channels = ((buffer[2] & 0x01) << 2) | ((buffer[3] & 0xC0) >> 6);
        }

        /* ...and the variable ADTS header */
        frame_length = ((((unsigned int)buffer[3] & 0x3)) << 11)
            | (((unsigned int)buffer[4]) << 3) | (buffer[5] >> 5);

        total_framelength += frame_length;
        faad_skip_buffer(frame_length - ADTS_MAX_SIZE);

        cur_pos += frame_length;
        if (cur_pos >= file_length)
            break;
    }
/*
    frames_per_sec = info->sampling_rate / 1024;
    if (frames != 0)
        bytes_per_frame = total_framelength / frames;
    else
        bytes_per_frame = 0;
    info->bitrate = (bytes_per_frame * frames_per_sec) << 3;
    info->length = frames / frames_per_sec;
*/
    /* since we only use limited seconds of aac data to get a rough bitrate, we must use a different
       method of calculating the overall length */
    if (frames != 0)
        bytes_per_frame = total_framelength / frames;
    else
        bytes_per_frame = 0;
    info->bitrate = (bytes_per_frame * info->sampling_rate) >> 7;
    info->length = (file_length << 3) / info->bitrate;

    return 0;
}

static int search_syncword(int file_length)
{
    unsigned char buffer[ADTS_MAX_SIZE], sync_err = 1;
    int skip_bytes, i;

    faad_read_buffer(buffer, ADTS_MAX_SIZE);
    for (i = 0, skip_bytes = 0; i < (ADTS_MAX_SIZE - 1); i++, skip_bytes++) {
        if ((buffer[i] == 0xFF) && ((buffer[i + 1] & 0xF6) == 0xF0)) {
            sync_err = 0;
            break;
        } else if (i == (ADTS_MAX_SIZE - 2)) {
            buffer[0] = buffer[i + 1];
            faad_read_buffer(buffer + 1, ADTS_MAX_SIZE - 1);
            i = -1;
        }
        if (skip_bytes >= file_length)
            break;
    }
    if (sync_err) {
        return -1;
    }

    return skip_bytes;
}

static int get_aac_info(int file_length, NeAACDecFileInfo *info, int seconds)
{
    int tagsize = 0, result;
    unsigned char buffer[10];
    unsigned int data_start;

    /* check ID3v1 */
    MagicPixel_AAC_FAAD_fileseek_callback(file_length - 128);
    result = Fill_Cache_Buffer(AAC_TYPE);
    faad_read_buffer(buffer, 4);
    if (strncmp(buffer, "TAG", 3) == 0)
        tagsize = 128;

    /* check ID3v2 or ID3v2.3 */
    MagicPixel_AAC_FAAD_fileseek_callback(0);
    result = Fill_Cache_Buffer(AAC_TYPE);
    faad_read_buffer(buffer, 10);
    if (strncmp(buffer, "ID3", 3) == 0) {
        /* high bit is not used */
        tagsize = (buffer[6] << 21) | (buffer[7] << 14) |
            (buffer[8] <<  7) | (buffer[9] <<  0);

        /* skip the tag */
        faad_skip_buffer(tagsize);
        tagsize += 10;
    } else {
        /* go back to the beginning */
        faad_seek_buffer(0);
    }
    if (file_length)
        file_length -= tagsize;

    /* determine the header type of the file */
    data_start = GetCurPos(AAC_TYPE);
    faad_read_buffer(buffer, 4);
    faad_seek_buffer(data_start);

    info->header_type = RAW;
    if(strncmp(buffer, "ADIF", 4) == 0) {
        result = read_adif_header(file_length, info);
    } else if ((buffer[0] == 0xFF) && ((buffer[1] & 0xF6) == 0xF0)) {
        /* No ADIF, check for ADTS header */
        result = read_adts_header(file_length, info, seconds);
    } else {
        /* Unknown/headerless AAC file */
        if ((result = search_syncword(file_length)) != -1) {   /* try to skip redundant data */
            data_start += result;
            faad_seek_buffer(data_start);
            result = read_adts_header(file_length, info, seconds);
        }
    }
    faad_seek_buffer(data_start);

    return result;
}

static int read_string(unsigned char *buffer, char *ending, int max_len)
{
    int len = 0, ending_len = strlen(ending);
    unsigned int cur_pos;

    cur_pos = GetCurPos(AAC_TYPE);
    while (len < max_len) {
        faad_read_buffer(buffer + len, 1);
        len++;
        if ((len >= ending_len) && (strncmp(buffer + len - ending_len, ending, ending_len) == 0))
            break;
    }

    if (len == max_len) {
        faad_seek_buffer(cur_pos);
        return -1;
    } else {
        *(buffer + len - ending_len) = '\0';
        return 0;
    }
}

static int get_shoutcast_stream_info(int file_length, NeAACDecFileInfo *info)
{
    int result;
    unsigned char item[16], content[128];
    unsigned int data_start;

    faad_skip_buffer(12);
    while (1) {
        if (read_string(item, ":", 16) == -1)
            return -1;
        data_start = GetCurPos(AAC_TYPE);
        faad_read_buffer(content, 1);
        if (strncmp(content, " ", 1) != 0)
            faad_seek_buffer(data_start);
        if (read_string(content, "\r\n", 128) == -1)
            return -1;

        if (strncmp(item, "content-type", 12) == 0) {
            if (strncmp(content, "audio/aacp", 10) != 0)
                return -1;
        } else if (strncmp(item, "icy-metaint", 11) == 0) {
            info->block_size = atoi(content);
        } else if (strncmp(item, "icy-br", 6) == 0) {
            info->bitrate = atoi(content);
        }

        data_start = GetCurPos(AAC_TYPE);
        faad_read_buffer(item, 2);
        if (strncmp(item, "\r\n", 2) != 0)
            faad_seek_buffer(data_start);
        else
            break;
    }

    data_start = GetCurPos(AAC_TYPE);
    file_length -= data_start;
    if ((result = search_syncword(file_length)) != -1) {   /* try to skip redundant data */
        unsigned char buffer[ADTS_MAX_SIZE], id, sf_index;

        data_start += result;
        file_length -= result;
        faad_seek_buffer(data_start);

        info->header_type = ADTS;
        faad_read_buffer(buffer, ADTS_MAX_SIZE);
        faad_seek_buffer(data_start);
        /* syncword found, proceed to read in the fixed ADTS header */
        id = buffer[1] & 0x08;
        if (id == 0)
            info->version = 4;
        else    /* MPEG-2 */
            info->version = 2;
        info->object_type = ((buffer[2] & 0xC0) >> 6) + 1;
        sf_index = (buffer[2] & 0x3C) >> 2;
        info->sampling_rate = get_sample_rate(sf_index);
        info->channels = ((buffer[2] & 0x01) << 2) | ((buffer[3] & 0xC0) >> 6);
        info->skip_bytes = result;
        result = 0;
    }

    if (info->bitrate)
        info->length = (file_length << 3) / (info->bitrate * 1000);     /* roughly calculate */
    if (!info->block_size)
        info->shoutcast = 0;

    return result;
}

static int id3_v1_parse(id3v1_tag_t *tag, int file_length)
{
    int result;
    unsigned char genre, track, buffer[10];

    MagicPixel_AAC_FAAD_fileseek_callback(file_length - 128);
    result = Fill_Cache_Buffer(AAC_TYPE);
    faad_read_buffer(buffer, 3);

    if (strncmp(buffer, "TAG", 3) == 0) {   /* ID3 tag version 1 */
        faad_read_buffer(tag->title, 30);
        faad_read_buffer(tag->artist, 30);
        faad_read_buffer(tag->album, 30);
        faad_read_buffer(tag->year, 4);
        faad_read_buffer(tag->comment, 30);
        faad_read_buffer(&genre, 1);
        track = 0;
        if (tag->comment[28] == 0 && tag->comment[29] != 0)
            track = tag->comment[29];
        sprintf((char *)&tag->genre, "%d", genre);
        sprintf((char *)&tag->track, "%d", track);
        return 0;
    }
    return -1;
}

static void id3_v2_read_text_frame(char **frame, unsigned int size)
{
    *frame = (char *)faad_malloc(size);
    if (*frame != NULL) {
        faad_read_buffer(*frame, size - 1);
        (*frame)[size - 1] = 0;
    } else {
        faad_skip_buffer(size - 1);
    }
}

static void id3_v2_read_comment(char **comment, unsigned int size)
{
    unsigned char buffer[10];
    unsigned int language;

    if (size <= 6) {
        faad_skip_buffer(size - 1);
        return;
    }

    *comment = (char *)faad_malloc(size - 4);
    if (*comment != NULL) {
        faad_read_buffer(buffer, 3);
        language = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
        faad_skip_buffer(1);        /* skip content descriptors */
        faad_read_buffer(*comment, 1);
        if ((*comment)[0] != 0) {
            faad_read_buffer(*comment + 1, size - 6);   /* read actual text string */
        } else {
            faad_read_buffer(*comment, size - 6);   /* read actual text string */
            (*comment)[size - 6] = 0;
        }
        (*comment)[size - 5] = 0;
    } else {
        faad_skip_buffer(size - 1);
    }
}

static int id3_v2_parse(id3v2_tag_t *tag, unsigned int version, unsigned int flags, unsigned int tagsize)
{
    unsigned char buffer[10];
    unsigned int frame_id, frame_size, text_encoding;
    unsigned int extended_header_size;
    char **ptr;
    int remain = tagsize, i;

    /* free previously allocated memory */
    ptr = (char **)&tag->title;
    for (i = 0; i < sizeof(id3v2_tag_t); i += 4, ptr++) {
        if (*ptr) {
            faad_free(*ptr);
            *ptr = NULL;
        }
    }

    if (version == 0x0200) {    /* ID3 tag version 2 */
        do {
            faad_read_buffer(buffer, 7);
            frame_id = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
            frame_size = (buffer[3] << 16) | (buffer[4] << 8) | buffer[5];
            if ((frame_id == 0) || (frame_size == 0) || ((frame_size + 6) > remain))
                return 0;
            text_encoding = buffer[6];

            switch (frame_id) {
            case TT2:
                id3_v2_read_text_frame((char **)&tag->title, frame_size);
                break;
            case TP1:
                id3_v2_read_text_frame((char **)&tag->artist, frame_size);
                break;
            case TAL:
                id3_v2_read_text_frame((char **)&tag->album, frame_size);
                break;
            case TYE:
                id3_v2_read_text_frame((char **)&tag->year, frame_size);
                break;
            case COM:
                id3_v2_read_comment((char **)&tag->comment, frame_size);
                break;
            case TCO:
                id3_v2_read_text_frame((char **)&tag->genre, frame_size);
                break;
            case TRK:
                id3_v2_read_text_frame((char **)&tag->track, frame_size);
                break;
            default:
                faad_skip_buffer(frame_size - 1);
                break;
            }
            remain -= frame_size + 6;
        } while (remain > 6);
    } else if (version == 0x0300) {     /* ID3 tag version 2.3.0 */
        do {
            if (flags & 0x40) {     /* the header is followed by an extended header */
                faad_read_buffer(buffer, 4);
                extended_header_size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
                faad_skip_buffer(extended_header_size);
            }
            faad_read_buffer(buffer, 11);
            frame_id = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
            frame_size = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
            if ((frame_id == 0) || (frame_size == 0) || ((frame_size + 10) > remain))
                return 0;
            /* skip flags */
            text_encoding = buffer[10];

            switch (frame_id) {
            case TIT2:
                id3_v2_read_text_frame((char **)&tag->title, frame_size);
                break;
            case TPE1:
                id3_v2_read_text_frame((char **)&tag->artist, frame_size);
                break;
            case TALB:
                id3_v2_read_text_frame((char **)&tag->album, frame_size);
                break;
            case TYER:
                id3_v2_read_text_frame((char **)&tag->year, frame_size);
                break;
            case COMM:
                id3_v2_read_comment((char **)&tag->comment, frame_size);
                break;
            case TCON:
                id3_v2_read_text_frame((char **)&tag->genre, frame_size);
                break;
            case TRCK:
                id3_v2_read_text_frame((char **)&tag->track, frame_size);
                break;
            default:
                faad_skip_buffer(frame_size - 1);
                break;
            }
            remain -= frame_size + 10;
        } while (remain > 10);
    } else {
        return -1;
    }
    return 0;
}

static id3v1_tag_t id3v1_tag;
static id3v2_tag_t id3v2_tag;

static void get_id3_tag(int file_length, char **song_singer, char **song_name, char **song_comment,
                 char **song_album, char **song_genre, char **song_year, char **song_track)
{
    int result;
    unsigned char buffer[10];

    memset(&id3v1_tag, 0, sizeof(id3v1_tag_t));
    *song_name = (char *)&id3v1_tag.title;
    *song_singer = (char *)&id3v1_tag.artist;
    *song_comment = (char *)&id3v1_tag.comment;
    *song_album = (char *)&id3v1_tag.album;
    *song_genre = (char *)&id3v1_tag.genre;
    *song_year  = (char *)&id3v1_tag.year;
    *song_track = (char *)&id3v1_tag.track;

    /* check ID3v2 or ID3v2.3 */
    MagicPixel_AAC_FAAD_fileseek_callback(0);
    result = Fill_Cache_Buffer(AAC_TYPE);
    faad_read_buffer(buffer, 10);

    if (strncmp(buffer, "ID3", 3) != 0) {   /* no ID3 tag version 2 or 2.3.0 */
        result = id3_v1_parse(&id3v1_tag, file_length);     /* parse ID3v1 */
    } else {    /* ID3 tag version 2 or 2.3.0 */
        unsigned int version, flags, tagsize;

        version = (buffer[3] << 8) | buffer[4];
        flags = buffer[5];
        tagsize = (buffer[6] << 21) | (buffer[7] << 14) |
            (buffer[8] <<  7) | (buffer[9] <<  0);
        result = id3_v2_parse(&id3v2_tag, version, flags, tagsize);

        if (result != -1) {
            if (id3v2_tag.title)    *song_name = id3v2_tag.title;
            if (id3v2_tag.artist)    *song_singer = id3v2_tag.artist;
            if (id3v2_tag.comment)    *song_comment = id3v2_tag.comment;
            if (id3v2_tag.album)    *song_album = id3v2_tag.album;
            if (id3v2_tag.genre)    *song_genre = id3v2_tag.genre;
            if (id3v2_tag.year)    *song_year  = id3v2_tag.year;
            if (id3v2_tag.track)    *song_track = id3v2_tag.track;
        }
    }
}

NeAACDecFileInfo aacinfo;

int MagicPixel_AAC_FAAD_init(int file_size,int *ch,int *srate,int *frame_size,int *bitrate,int *total_time,
                        char **song_singer,char **song_name,char **song_comment,char **song_album,char **song_genre,
                        char **song_year,char **song_track,int *mpeg2,int *layer,BYTE filebrowser)
{
    unsigned char header[10];
    int result;

    memset(&aacinfo, 0, sizeof(NeAACDecFileInfo));
    Init_BitstreamCache(AAC_TYPE);
    MagicPixel_AAC_FAAD_fileseek_callback(0);
    result = Fill_Cache_Buffer(AAC_TYPE);
    faad_read_buffer(header, 10);
    if (strncmp(header, "ICY 200 OK", 10) == 0)
    	aacinfo.shoutcast = 1;
    else if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
        aacinfo.mp4file = 1;
    faad_seek_buffer(0);

    if (aacinfo.shoutcast) {
        result = get_shoutcast_stream_info(file_size, &aacinfo);
    } else if (aacinfo.mp4file) {
        result = get_mp4_info(file_size, &aacinfo);
        /* go back to the beginning */
        faad_seek_buffer(0);
    } else {
        if (filebrowser == 1) {
            get_id3_tag(file_size, song_singer, song_name, song_comment,
                        song_album, song_genre, song_year, song_track);
            result = get_aac_info(file_size, &aacinfo, 20);
        } else {
            result = get_aac_info(file_size, &aacinfo, 60);
        }
    }

    *ch = aacinfo.channels;
    *bitrate = aacinfo.bitrate;
    *srate = aacinfo.sampling_rate;
    *total_time = aacinfo.length;
    if (result != -1)
        result = can_decode_ot(aacinfo.object_type);

    if (filebrowser == 1)
        Free_BitstreamCache();

    return result;                     
}

