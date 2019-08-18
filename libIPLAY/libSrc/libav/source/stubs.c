/*
 * various utility functions for use within Libav
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* #define DEBUG */

#include "libavformat/avformat.h"
#include "libavformat/avio_internal.h"
#include "libavformat/internal.h"
#include "libavcodec/internal.h"
#include "libavcodec/bytestream.h"
#include "libavutil/opt.h"
#include "libavutil/dict.h"
#include "libavutil/pixdesc.h"
#include "libavformat/metadata.h"
#include "libavformat/id3v2.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/mathematics.h"
#include "libavutil/parseutils.h"
#include "libavutil/time.h"
#include "libavformat/riff.h"
#include "libavformat/audiointerleave.h"
#include "libavformat/url.h"
#include <stdarg.h>
#if CONFIG_NETWORK
#include "libavformat/network.h"
#endif

#undef NDEBUG
#include <assert.h>

#include "libavutil/internal.h"

void ff_dynarray_add(intptr_t **tab_ptr, int *nb_ptr, intptr_t elem)
{
    /* see similar avconv.c:grow_array() */
    int nb, nb_alloc;
    intptr_t *tab;
        MP_ASSERT(0);

}

char *
stpcpy(char *s1, const char *s2)
{
    strcpy(s1, s2);

    return (s1 + strlen(s1));
}

int ff_network_inited_globally;
int av_log2(unsigned v)
{
#ifdef LINUX
    return ff_log2(v);
#else
    return 0;
#endif
}

#include <time.h>
#if HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

/* init static data */
av_cold void ff_dsputil_static_init(void)
{
#ifdef LINUX
    int i;

    for(i=0;i<256;i++) ff_cropTbl[i + MAX_NEG_CROP] = i;
    for(i=0;i<MAX_NEG_CROP;i++) {
        ff_cropTbl[i] = 0;
        ff_cropTbl[i + MAX_NEG_CROP + 256] = 255;
    }

    for(i=0;i<512;i++) {
        ff_squareTbl[i] = (i - 256) * (i - 256);
    }

    for(i=0; i<64; i++) ff_inv_zigzag_direct16[ff_zigzag_direct[i]]= i+1;
#else
    MP_ASSERT(0);
#endif
}

int av_image_check_size(unsigned int w, unsigned int h, int log_offset, void *log_ctx)
{
#ifdef LINUX
    ImgUtils imgutils = { &imgutils_class, log_offset, log_ctx };

    if ((int)w>0 && (int)h>0 && (w+128)*(uint64_t)(h+128) < INT_MAX/8)
        return 0;

    av_log(&imgutils, AV_LOG_ERROR, "Picture size %ux%u is invalid\n", w, h);
#else
    MP_ASSERT(0);
#endif
    return AVERROR(EINVAL);
}

int av_bitstream_filter_filter(AVBitStreamFilterContext *bsfc,
                               AVCodecContext *avctx, const char *args,
                     uint8_t **poutbuf, int *poutbuf_size,
                     const uint8_t *buf, int buf_size, int keyframe){
    *poutbuf= (uint8_t *) buf;
    *poutbuf_size= buf_size;
    return bsfc->filter->filter(bsfc, avctx, args, poutbuf, poutbuf_size, buf, buf_size, keyframe);
}

static const AVOption ffio_url_options[] = {
    { NULL },
};

static void *ffio_url_child_next(void *obj, void *prev)
{
    AVIOContext *s = obj;
    return prev ? NULL : s->opaque;
}

static const AVClass *ffio_url_child_class_next(const AVClass *prev)
{
    return prev ? NULL : &ffurl_context_class;
}

const char *av_default_item_name(void *ptr)
{
    return (*(AVClass **) ptr)->class_name;
}

int av_samples_get_buffer_size(int *linesize, int nb_channels, int nb_samples,
                               enum AVSampleFormat sample_fmt, int align)
{
    int line_size;
    MP_ASSERT(0);
    return 0;
}
int ff_thread_decode_frame(AVCodecContext *avctx,
                           AVFrame *picture, int *got_picture_ptr,
                           AVPacket *avpkt)
{
    MP_ASSERT(0);
    return 0;
}
int av_expr_parse_and_eval(double *d, const char *s,
                           const char * const *const_names, const double *const_values,
                           const char * const *func1_names, double (* const *funcs1)(void *, double),
                           const char * const *func2_names, double (* const *funcs2)(void *, double, double),
                           void *opaque, int log_offset, void *log_ctx)
{
#ifdef LINUX
    AVExpr *e = NULL;
    int ret = av_expr_parse(&e, s, const_names, func1_names, funcs1, func2_names, funcs2, log_offset, log_ctx);
#else
    MP_ASSERT(0);
#endif
    return 0;
}

#include "libavutil/crc.h"
const AVCRC *av_crc_get_table(AVCRCId crc_id)
{
#ifdef LINUX
#if !CONFIG_HARDCODED_TABLES
    if (!av_crc_table[crc_id][FF_ARRAY_ELEMS(av_crc_table[crc_id]) - 1])
        if (av_crc_init(av_crc_table[crc_id],
                        av_crc_table_params[crc_id].le,
                        av_crc_table_params[crc_id].bits,
                        av_crc_table_params[crc_id].poly,
                        sizeof(av_crc_table[crc_id])) < 0)
            return NULL;
#endif
    return av_crc_table[crc_id];
#else
    MP_ASSERT(0);
    return 0;
#endif
}

#include <stdint.h>

uint32_t av_crc(const AVCRC *ctx, uint32_t crc,
                const uint8_t *buffer, size_t length)
{
    const uint8_t *end = buffer + length;
#ifdef LINUX

#if !CONFIG_SMALL
    if (!ctx[256]) {
        while (((intptr_t) buffer & 3) && buffer < end)
            crc = ctx[((uint8_t) crc) ^ *buffer++] ^ (crc >> 8);

        while (buffer < end - 3) {
            crc ^= av_le2ne32(*(const uint32_t *) buffer); buffer += 4;
            crc = ctx[3 * 256 + ( crc        & 0xFF)] ^
                  ctx[2 * 256 + ((crc >> 8 ) & 0xFF)] ^
                  ctx[1 * 256 + ((crc >> 16) & 0xFF)] ^
                  ctx[0 * 256 + ((crc >> 24)       )];
        }
    }
#endif
    while (buffer < end)
        crc = ctx[((uint8_t) crc) ^ *buffer++] ^ (crc >> 8);
#else
    MP_ASSERT(0);
#endif

    return crc;
}

void ff_thread_free(AVCodecContext *avctx)
{
    MP_ASSERT(0);
}
