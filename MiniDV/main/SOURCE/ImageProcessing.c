/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : ImageProcessing.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "os.h"

#include "slideEffect.h"
#include "cv.h"
#if FACE_DETECTION
    #include "frontalface.txt"
#endif
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

typedef unsigned char (*CvColorMatchFunc)(int data);

static CvMemStorage *storage = 0;


static void *icvAlloc(size_t size, void* userdata)
{
    char *ptr, *ptr0 = (char*)ext_mem_malloc(
        (unsigned int)(size + CV_MALLOC_ALIGN*((size >= 4096) + 1) + sizeof(char*)));

    if( !ptr0 )
        return 0;

    // align the pointer
    ptr = (char*)cvAlignPtr(ptr0 + sizeof(char*) + 1, CV_MALLOC_ALIGN);
    *(char**)(ptr - sizeof(char*)) = ptr0;

    return ptr;
}

static int icvFree(void* ptr, void* userdata)
{
    // pointer must be aligned by CV_MALLOC_ALIGN
    if( ((size_t)ptr & (CV_MALLOC_ALIGN-1)) != 0 )
        return -1;
    ext_mem_free( *((char**)ptr - 1) );

    return 0;
}

static inline unsigned char icvIsSkinColor(int data)
{
    int cb, cr, luma, hue, chroma;

    cr = data & 0xff;
    luma = data & 0xff0000;
    if ((cr > 128) && (luma > 0x200000)) {      // luma > 32
        cb = data & 0xff00;
        if (cb < 0x8000) {      // hue > 90
            cb = (0x8000 - cb);
            cr = (cr - 128) << 24;
            hue = cr / cb;
//            if ((hue > SCALE_CONST(0.577)) && (hue <= SCALE_CONST(57.29))) {  // range of hue: (91, 150)
            if ((hue > SCALE_CONST(0.51)) && (hue <= SCALE_CONST(57.29))) {     // (91, 153)
                cr >>= 16;
                chroma = (cb * cb) + (cr * cr);
//                if (chroma < SCALE_CONST(3600))   // range of chroma: (0, 60)
                if (chroma < SCALE_CONST(4225))     // (0, 65)
                    return TRUE;
            }
        }
    }
    return FALSE;
}

static unsigned char icvIsNonSkinColor(int data)
{
    int cb, cr, luma, hue, chroma;

    cr = data & 0xff;
    luma = data & 0xff0000;
    if ((cr > 128) && (luma > 0x300000)) {      // luma > 48
        cb = data & 0xff00;
        if (cb == 0x8000) {     // hue = 90
            cr = (cr - 128) << 8;
            chroma = cr * cr;
            if ((chroma > SCALE_CONST(100)) && chroma < SCALE_CONST(4225))     // range of chroma: (10, 65)
                return FALSE;
        } else if (cb > 0x8000) {      // hue < 90
            cb -= 0x8000;
            cr = (cr - 128) << 24;
            hue = cr / cb;
            if (hue > SCALE_CONST(5.67)) {  // range of hue: (80, 90)
                cr >>= 16;
                chroma = (cb * cb) + (cr * cr);
                if ((chroma > SCALE_CONST(100)) && chroma < SCALE_CONST(4225))     // range of chroma: (10, 65)
                    return FALSE;
            }
        } else {      // hue > 90
            cb = (0x8000 - cb);
            cr = (cr - 128) << 24;
            hue = cr / cb;
            if (hue > SCALE_CONST(0.51)) {     // range of hue: (90, 153)
                cr >>= 16;
                chroma = (cb * cb) + (cr * cr);
                if ((chroma > SCALE_CONST(100)) && chroma < SCALE_CONST(4225))     // range of chroma: (10, 65)
                    return FALSE;
            }
        }
    }
    return TRUE;
}

static inline unsigned char icvIsRedeyeColor(int data)
{
    int cb, cr, hue;

    cr = data & 0xff;
    if (cr > 150) {
        cb = data & 0xff00;
        if (cb == 0x8000) {     // hue = 90
            return TRUE;
        } else if (cb > 0x8000) {      // hue < 90
            cb -= 0x8000;
            cr = (cr - 128) << 24;
            hue = cr / cb;
            if (hue > SCALE_CONST(1.732)) {  // range of hue: (60, 90)
                return TRUE;
            }
        } else {    // hue > 90
            cb = (0x8000 - cb);
            cr = (cr - 128) << 24;
            hue = cr / cb;
            if (hue > SCALE_CONST(2.14)) {  // range of hue: (90, 115)
                return TRUE;
            }
        }
    }
    return FALSE;
}

static inline unsigned char icvIsBlackColor(int data)
{
    int cb, cr, chroma;

    cr = data & 0xff;
    cb = (data & 0xff00) - 0x8000;
    cr = (cr - 128) << 8;
    chroma = (cb * cb) + (cr * cr);
    if (chroma < SCALE_CONST(400))     // (0, 20)
        return TRUE;
    return FALSE;
}

static unsigned char icvIsMonochromicImage(int *source, CvSize size, int offset)
{
    int *source_ptr, avg_cb, avg_cr, data, cb, cr, i, j, k;

    avg_cb = (*source >> 8) & 0xff;
    avg_cr = *source & 0xff;
    offset = (size.width << 1 + offset) * size.height >> 4;
    for (i = 0; i < 4; i++) {
        source_ptr = source + offset * i;
        for (j = 0 ; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                data = source_ptr[k];
                cb = (data >> 8) & 0xff;
                cr = data & 0xff;
                if (abs(cb - avg_cb) > 8)   return FALSE;
                if (abs(cr - avg_cr) > 8)   return FALSE;
            }
            source_ptr += size.width >> 3;
        }
    }

    return TRUE;
}

static CvRleBinaryImage *icvCreateSkinMap(int *source, CvSize size, int offset)
{
    CvRleBinaryImage *map = 0;
    CvRleRegion *region = 0;
    CvRun *run;
    int i, j;

    map = (CvRleBinaryImage *)cvRleCreateBinaryImage(size.width, size.height);
    if (!map)   return NULL;

    region = (CvRleRegion *)cvRleCreateRegion(0, 0, size.width - 1, size.height - 1);
    if (!region)    goto alloc_region_error;
    cvRleAddRegionToImage(region, map);

    for (i = 0; i < size.height; i++) {
        for (j = 0, run = 0; j < size.width; j += 2) {
            if (icvIsSkinColor(*source) == TRUE) {
                if (!run) {
                    run = (CvRun *)cvRleCreateRun(j, j, i);
                    if (!run) goto alloc_run_error;
                    cvRleAddRunToRegion(run, region);
                } else {
                    run->xR = j + 1;
                }
            } else {
//                *source = 0x8080;     // for internal check
                if (run) {
                    if ((j - run->xL) <= 8)     // filter out runs with length less than 8
                        cvRleRemoveRunFromRegion(run, region);
                    run = 0;    // finish this run
                }
            }
            source++;
        }
        source += (offset >> 2);
    }
    return map;

alloc_run_error:
    cvRleRemoveRegionFromImage(region, map);

alloc_region_error:
    cvRleReleaseBinaryImage(map);

    return NULL;
}

static CvRleBinaryImage *icvCreateRedeyeMap(int *source, CvRect *face, int offset)
{
    CvRleBinaryImage *map = 0;
    CvRleRegion *region = 0;
    CvRun *run;
    int i, j;

    map = (CvRleBinaryImage *)cvRleCreateBinaryImage(face->width, face->height);
    if (!map)   return NULL;

    region = (CvRleRegion *)cvRleCreateRegion(0, 0, face->width - 1, face->height - 1);
    if (!region)    goto alloc_region_error;
    cvRleAddRegionToImage(region, map);

    for (i = face->y; i < (face->y + face->height); i++) {
        for (j = face->x, run = 0; j < (face->x + face->width); j += 2) {
            if (icvIsRedeyeColor(*source) == TRUE) {
                if (!run) {
                    run = (CvRun *)cvRleCreateRun(j, j, i);
                    if (!run) goto alloc_run_error;
                    cvRleAddRunToRegion(run, region);
                } else {
                    run->xR = j + 1;
                }
            } else {
                if (run) {
                    if ((j - run->xL) > 16)     // filter out runs with length longer than 16
                        cvRleRemoveRunFromRegion(run, region);
                    run = 0;    // finish this run
                }
            }
            source++;
        }
        source += (offset >> 2);
    }
    return map;

alloc_run_error:
    cvRleRemoveRegionFromImage(region, map);

alloc_region_error:
    cvRleReleaseBinaryImage(map);

    return NULL;
}

//#define INTERNAL_CHECK_ENABLE
#ifdef INTERNAL_CHECK_ENABLE
static void icvCheckBinaryMap(int *source, CvSize size, int offset, CvRleBinaryImage *map, int highlight)
{
    CvRleRegion *region;
    CvRun *run;
    int line_width, *row_ptr, *col_ptr, y, prev_x, i;

    line_width = (size.width >> 1) + (offset >> 2);
    list_for_each_entry( region, &map->region_list, region_node ) {
        row_ptr = col_ptr = source;
        y = prev_x = 0;
        list_for_each_entry( run, &region->run_list, run_node ) {
            if (run->y != y) {
                row_ptr = col_ptr = source + (run->y * line_width);
                prev_x = 0;
                y = run->y;
            }
            col_ptr += (run->xL - prev_x) >> 1;
            for (i = 0; i < (run->xR - run->xL + 1); i += 2) {
                *col_ptr++ = highlight;
            }
            prev_x = run->xR + 1;
        }
    }
}

static void icvCheckBinaryRegion(ST_IMGWIN *pWin, CvRleBinaryImage *map, CvSeq *regions)
{
    CvRleRegion *region;
    CvRect rect;
    int nRegions, *dst, offset, color = 0xFFFF3622, i, j;

    if (!map && !regions)
        return;

    if (map) {
        nRegions = map->nRegions;
        region = list_entry( map->region_list.next, CvRleRegion, region_node );
    } else if (regions) {
        nRegions = regions->total;
    }

    for (i = 0; i < nRegions; i++) {
        if (map) {
            rect = cvRect(region->left, region->top, region->right - region->left + 1,
                                region->bottom - region->top + 1);
        } else if (regions) {
            rect = *(CvRect *)cvGetSeqElem(regions, i);
        }
        rect.x += pWin->wClipLeft;
        rect.y += pWin->wClipTop;
        dst = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * rect.y) + (rect.x << 1));
        offset = (pWin->dwOffset - (rect.width << 1)) >> 2;

        for (j = 0; j < rect.width; j += 2)
            *dst++ = color;
        *dst = color;
        dst += offset;
        for (j = 1; j < rect.height; j++) {
            *dst = color;
            dst += (rect.width >> 1);
            *dst = color;
            dst += offset;
        }
        for (j = 0; j < rect.width; j += 2)
            *dst++ = color;
        *dst = color;

        if (map)
            region = list_entry( region->region_node.next, CvRleRegion, region_node );
    }
}

static void icvCheckDetectImage(short *source, short *target, CvSize size, int offset)
{
    int i, j;

    for (i = 0; i < size.height; i++) {
        for (j = 0; j < size.width; j += 2) {
            *target++ = *source++;
            *target++ = 0x8080;
        }
        target += (offset - (size.width << 1)) >> 1;
    }
}
#endif

static void icvExtractRegionLuma(short *source, CvSize size, int offset, short *target, CvSeq *regions)
{
    CvRect *rect;
    short *source_ptr, *target_ptr;
    int source_linewidth, target_linewidth, source_offset, target_offset, i, j, k;
    int *target_iptr;

    target_iptr = (int *)target;
    for (i = 0; i < (size.width * size.height) >> 2; i++) {
        *target_iptr++ = 0x80808080;
    }

    source_linewidth = size.width + (offset >> 1);
    target_linewidth = size.width >> 1;
    for (i = 0; i < regions->total; i++) {
        rect = (CvRect *)cvGetSeqElem(regions, i);
        rect->x &= 0xfffe, rect->width &= 0xfffe;
        source_ptr = source + (rect->y * source_linewidth) + rect->x;
        source_offset = source_linewidth - rect->width;
        target_ptr = target + (rect->y * target_linewidth) + (rect->x >> 1);
        target_offset = target_linewidth - (rect->width >> 1);

        for (j = 0; j < rect->height; j++) {
            for (k = 0; k < rect->width; k += 2) {
                *target_ptr++ = *source_ptr;
                source_ptr += 2;
            }
            source_ptr += source_offset;
            target_ptr += target_offset;
        }
    }
}

/*
 * special case of cvCvtColor() with code = CV_YYCbCr2YCbCr
 */
static void icvConvertColor(IplImage *src, IplImage *dst, CvSize size)
{
    char *dst_row = dst->imageData, *dptr;
    int *src_row = (int *)src->imageData, *sptr, cb, cr, i;
    int srcstep = src->widthStep >> 2, dststep = dst->widthStep;

    for (; size.height--; src_row += srcstep, dst_row += dststep) {
        sptr = src_row;
        dptr = dst_row;
        for (i = 0; i < size.width; i += 2, sptr++) {
            if (icvIsNonSkinColor(*sptr)) {
                *dptr++ = 0;  *dptr++ = 0;  *dptr++ = 0;
                *dptr++ = 0;  *dptr++ = 0;  *dptr++ = 0;
            } else {
                cb = (*sptr >> 8) & 0xff;
                cr = *sptr & 0xff;
                *dptr++ = (*sptr >> 24) & 0xff;
                *dptr++ = cb, *dptr++ = cr;
                *dptr++ = (*sptr >> 16) & 0xff;
                *dptr++ = cb, *dptr++ = cr;
            }
        }
    }
}

static void icvTableLookup(char *source, CvSize size, int offset, unsigned char *lut)
{
    short *yuv = (short *)source;
    int i, j, y1, y2;

    for (i = 0; i < size.height; i++) {
        for (j = 0; j < size.width; j += 2) {
            y1 = (*yuv >> 8) & 0xff;
            y2 = *yuv & 0xff;
            *yuv = (lut[y1] << 8) | lut[y2];
            yuv += 2;
        }
        yuv += (offset >> 1);
        TaskYield();
    }
}

static void icvCopyImage(int *source, CvSize size, int offset, int *target, int target_offset)
{
    int i, j;

    for (i = 0; i < size.height; i++) {
        for (j = 0 ; j < size.width; j += 2) {
            *target++ = *source++;
        }
        source += offset;
        target += target_offset;
    }
}

static void icvFloodFill(int *source, int offset, CvSize roi, CvPoint seed, int new_value,
                         CvRleRegion *region, CvColorMatchFunc match_func,
                         CvFFillSegment* buffer, int buffer_size)
{
    int *image = source + offset * seed.y;
    int i, L, R;
    int area = 0;
    int XMin, XMax, YMin = seed.y, YMax = seed.y;
    int _8_connectivity = 1;
    CvFFillSegment *buffer_end = buffer + buffer_size, *head = buffer, *tail = buffer;

    seed.x >>= 1, roi.width >>= 1;

    L = R = XMin = XMax = seed.x;

    image[L] = new_value;

    while (++R < roi.width && match_func(image[R]))
        image[R] = new_value;

    while (--L >= 0 && match_func(image[L]))
        image[L] = new_value;

    XMax = --R;
    XMin = ++L;
    ICV_PUSH(seed.y, L, R, R + 1, R, CV_UP);

    while (head != tail) {
        int k, YC, PL, PR, dir;
        ICV_POP(YC, L, R, PL, PR, dir);

        int data[][3] = {
            {-dir, L - _8_connectivity, R + _8_connectivity},
            {dir, L - _8_connectivity, PL - 1},
            {dir, PR + 1, R + _8_connectivity}};

        if (region) {
            area += R - L + 1;

            if( XMax < R ) XMax = R;
            if( XMin > L ) XMin = L;
            if( YMax < YC ) YMax = YC;
            if( YMin > YC ) YMin = YC;
        }

        for (k = 0; k < 3; k++) {
            dir = data[k][0];
            image = (int *)source + (YC + dir) * offset;
            int left = data[k][1];
            int right = data[k][2];

            if ((unsigned)(YC + dir) >= (unsigned)roi.height)
                continue;

            for (i = left; i <= right; i++) {
                if ((unsigned)i < (unsigned)roi.width && match_func(image[i])) {
                    int j = i;
                    image[i] = new_value;
                    while (--j >= 0 && match_func(image[j]))
                        image[j] = new_value;

                    while (++i < roi.width && match_func(image[i]))
                        image[i] = new_value;

                    ICV_PUSH(YC + dir, j + 1, i - 1, L, R, -dir);
                }
            }
        }
    }

    if (region) {
        region->area = area << 1;
        region->left = XMin << 1;
        region->top = YMin;
        region->right = XMax << 1;
        region->bottom = YMax;
    }
}

static int icvCountSurroundingBlack(ST_IMGWIN *pWin, CvRect *inner, CvRect *outer)
{
    int *source, offset, x, y, width;
    int total = 0, white = 0, i, j;

    x = outer->x, y = outer->y;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    offset = (pWin->dwOffset - (outer->width << 1)) >> 2;
    for (i = y; i < inner->y; i++) {
        for (j = 0; j < outer->width; j += 2) {
            if (icvIsBlackColor(*source)) {
                white++;
//                *source = 0xFFFF8080;   // for internal check
            }
            total++, source++;
        }
        source += offset;
    }

    y = inner->y + inner->height;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    for (i = y; i < outer->y + outer->height; i++) {
        for (j = 0; j < outer->width; j += 2) {
            if (icvIsBlackColor(*source)) {
                white++;
//                *source = 0xFFFF8080;   // for internal check
            }
            total++, source++;
        }
        source += offset;
    }

    y = inner->y;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    width = inner->x - outer->x;
    offset = (pWin->dwOffset - (width << 1)) >> 2;
    for (i = y; i < y + inner->height; i++) {
        for (j = 0; j < width; j += 2) {
            if (icvIsBlackColor(*source)) {
                white++;
//                *source = 0xFFFF8080;   // for internal check
            }
            total++, source++;
        }
        source += offset;
    }

    x = inner->x + inner->width;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    for (i = y; i < y + inner->height; i++) {
        for (j = 0; j < width; j += 2) {
            if (icvIsBlackColor(*source)) {
                white++;
//                *source = 0xFFFF8080;   // for internal check
            }
            total++, source++;
        }
        source += offset;
    }

    return (white << SCALE_BITS) / total;
}

static int icvCountSurroundingSimilar(ST_IMGWIN *pWin, CvRect *inner, CvRect *outer, int avg_y, int avg_cb, int avg_cr)
{
    int *source, offset, x, y, width;
    int total = 0, similar = 0, y1, y2, cb, cr, i, j;

#define VAR_LUMA   30
#define VAR_CHROMA  10

    x = outer->x, y = outer->y;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    offset = (pWin->dwOffset - (outer->width << 1)) >> 2;
    for (i = y; i < inner->y; i++) {
        for (j = 0; j < outer->width; j += 2) {
            y1 = (*source >> 24) & 0xff;
            y2 = (*source >> 16) & 0xff;
            cb = (*source >> 8) & 0xff;
            cr = *source & 0xff;
            if (abs(cb - avg_cb) < VAR_CHROMA && abs(cr - avg_cr) < VAR_CHROMA) {
                if (abs(y1 - avg_y) < VAR_LUMA)   similar++;
                if (abs(y2 - avg_y) < VAR_LUMA)   similar++;
            }
            total += 2, source++;
        }
        source += offset;
    }

    y = inner->y + inner->height;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    for (i = y; i < outer->y + outer->height; i++) {
        for (j = 0; j < outer->width; j += 2) {
            y1 = (*source >> 24) & 0xff;
            y2 = (*source >> 16) & 0xff;
            cb = (*source >> 8) & 0xff;
            cr = *source & 0xff;
            if (abs(cb - avg_cb) < VAR_CHROMA && abs(cr - avg_cr) < VAR_CHROMA) {
                if (abs(y1 - avg_y) < VAR_LUMA)   similar++;
                if (abs(y2 - avg_y) < VAR_LUMA)   similar++;
            }
            total += 2, source++;
        }
        source += offset;
    }

    y = inner->y;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    width = inner->x - outer->x;
    offset = (pWin->dwOffset - (width << 1)) >> 2;
    for (i = y; i < y + inner->height; i++) {
        for (j = 0; j < width; j += 2) {
            y1 = (*source >> 24) & 0xff;
            y2 = (*source >> 16) & 0xff;
            cb = (*source >> 8) & 0xff;
            cr = *source & 0xff;
            if (abs(cb - avg_cb) < VAR_CHROMA && abs(cr - avg_cr) < VAR_CHROMA) {
                if (abs(y1 - avg_y) < VAR_LUMA)   similar++;
                if (abs(y2 - avg_y) < VAR_LUMA)   similar++;
            }
            total += 2, source++;
        }
        source += offset;
    }

    x = inner->x + inner->width;
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y) + (x << 1));
    for (i = y; i < y + inner->height; i++) {
        for (j = 0; j < width; j += 2) {
            y1 = (*source >> 24) & 0xff;
            y2 = (*source >> 16) & 0xff;
            cb = (*source >> 8) & 0xff;
            cr = *source & 0xff;
            if (abs(cb - avg_cb) < VAR_CHROMA && abs(cr - avg_cr) < VAR_CHROMA) {
                if (abs(y1 - avg_y) < VAR_LUMA)   similar++;
                if (abs(y2 - avg_y) < VAR_LUMA)   similar++;
            }
            total += 2, source++;
        }
        source += offset;
    }

    return (similar << SCALE_BITS) / total;
}

static unsigned char icvIsDuplicatedRedeye(CvRect *r1, CvRect *r2, int *distance)
{
    int dx = r1->x - r2->x, dy = r1->y - r2->y;
    int left1 = r1->x, right1 = r1->x + r1->width;
    int top1 = r1->y, bottom1 = r1->y + r1->height;
    int left2 = r2->x, right2 = r2->x + r2->width;
    int top2 = r2->y, bottom2 = r2->y + r2->height;
    int size1 = r1->width * r1->height, size2 = r2->width * r2->height;

    *distance = dx * dx + dy * dy;

    if ((right1 > left2) && (left1 < right2) && (bottom1 > top2) && (top1 < bottom2)) {     // overlapped
        if (abs(size1 - size2) < ((size1 * SCALE_CONST(0.1)) >> SCALE_BITS)) {    // similar size
            int width, height;
            if (left1 <= left2)
                width = (right1 < right2) ? (right1 - left2) : r2->width;
            else
                width = (right1 > right2) ? (right2 - left1) : r1->width;
            if (top1 <= top2)
                height = (bottom1 < bottom2) ? (bottom1 - top2) : r2->height;
            else
                height = (bottom1 > bottom2) ? (bottom2 - top1) : r1->height;
            if ((width * height) > ((size1 * SCALE_CONST(0.8)) >> SCALE_BITS))
                return TRUE;
        }
    }
    return FALSE;
}

static int icvFillEyeRegion(ST_IMGWIN *pWin, CvRect *inner, CvRect *outer, CvRleRegion *region, CvRleRegion *eye_region)
{
    CvFFillSegment *buffer = 0;
    int *source, *target= 0, offset, target_offset, buffer_size, ret = FALSE;

    if (!eye_region)    return FALSE;

    /* fill highlight region with black color for the following flood fill */
    CvSize size = cvSize(outer->width, outer->height);
    source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * outer->y) + (outer->x << 1));
    offset = (pWin->dwOffset - (outer->width << 1)) >> 2;

//    target = (int *)pWin->pdwStart + (pWin->dwOffset << 2) + 8;      // for internal check
//    target_offset = (pWin->dwOffset - (outer->width << 1)) >> 2;
    target = (int *)icvAlloc(outer->width * outer->height << 1, 0);
    target_offset = 0;
    if (!target)    return FALSE;
    icvCopyImage(source, size, offset, target, target_offset);
//    target_offset = pWin->dwOffset >> 2;    // for internal check
    target_offset = outer->width >> 1;

    CvRun *run;
    int x, y, xL, xR;
    list_for_each_entry( run, &region->run_list, run_node ) {
        y = run->y + pWin->wClipTop - outer->y;
        source = target + target_offset * y;
        xL = (run->xL + pWin->wClipLeft - outer->x) >> 1;
        xR = (run->xR + pWin->wClipLeft - outer->x + 1) >> 1;
        for (x = xL; x < xR; x++)
            source[x] = 0x8080;
    }

    /* perform flood fill on non-skin color */
    CvPoint seed = cvPoint(--x << 1, y);
    buffer_size = (outer->width > outer->height) ? (outer->width << 1) : (outer->height << 1);
    buffer = (CvFFillSegment*)icvAlloc(buffer_size * sizeof(buffer[0]), 0);
    if (!buffer) {
        icvFree(target, 0);
        return FALSE;
    }

    icvFloodFill(target, target_offset, size, seed, 0x7A7A7294, eye_region,
                    icvIsNonSkinColor, buffer, buffer_size);

    icvFree(target, 0);     icvFree(buffer, 0);
    return TRUE;
}

//#define DEBUG_REDEYE_DETECTION
static void icvDetectRedEye(ST_IMGWIN *pWin, CvRleBinaryImage *map, CvRleBinaryImage *redeye_map, CvRect *image, CvRect *face)
{
    CvRleRegion *eye_region, *region, *temp;
    CvRect inner, outer;
    int max_size, i;
#ifdef DEBUG_REDEYE_DETECTION
    char debug[80];
#endif

    eye_region = (CvRleRegion *)cvRleCreateRegion(0, 0, 0, 0);
    if (!eye_region)    return;
    max_size = (image->width > image->height) ? (image->width * image->width >> 10) :
                    (image->height * image->height >> 10);

    list_for_each_entry_safe( region, temp, &map->region_list, region_node ) {
        inner = cvRect(region->left, region->top, region->right - region->left + 1,
                            region->bottom - region->top + 1);
        inner.x += pWin->wClipLeft;
        inner.y += pWin->wClipTop;

        /* check region size */
        int size = inner.width * inner.height;
        if ((size <= 4) || (size > max_size)) {
            cvRleRemoveRegionFromImage(region, map);
            continue;
        }

        if (face) {
            int percentage = (size << SCALE_BITS) / (face->width * face->height);
            if (percentage > SCALE_CONST(0.05)) {
                cvRleRemoveRegionFromImage(region, map);
                continue;
            }
        }

        /* check roundness */
        int roundness = (inner.width > inner.height) ? (inner.width << SCALE_BITS) / inner.height :
                                            (inner.height << SCALE_BITS) / inner.width;
        if (roundness > SCALE_CONST(2.5)) {
            cvRleRemoveRegionFromImage(region, map);
            continue;
        }

        roundness = (region->area << SCALE_BITS) / (inner.width * inner.height);
        if (roundness < SCALE_CONST(0.45)) {
            cvRleRemoveRegionFromImage(region, map);
            continue;
        }

        /* check redness contrast between the inner square and its surrounding margins */
        inner.x &= 0xfffe, inner.width = (inner.width + 1) & 0xfffe;
        outer = cvRect(inner.x - (inner.width >> 1), inner.y - (inner.height >> 1),
                            inner.width << 1, inner.height << 1);
        if (outer.x < pWin->wClipLeft)  outer.x = pWin->wClipLeft;
        if (outer.y < pWin->wClipTop)   outer.y = pWin->wClipTop;
        outer.x &= 0xfffe;

        int surrounding = icvCountSurroundingBlack(pWin, &inner, &outer);
#ifdef DEBUG_REDEYE_DETECTION
        sprintf(debug, "\r\n(%d, %d), %dx%d -> %d%% pixels with low chroma color", inner.x, inner.y,
                    inner.width, inner.height, surrounding * 100 >> SCALE_BITS);
        UartOutText(debug);
#endif
        if (surrounding < SCALE_CONST(0.45)) {
            cvRleRemoveRegionFromImage(region, map);
            continue;
        }

        /* check surrounding non-skin pixels, should be inside an eye region */
        outer = cvRect( inner.x - (inner.width * 3), inner.y - (inner.height * 2),
                            inner.width * 7, inner.height * 5);
        if (outer.x < pWin->wClipLeft)  outer.x = pWin->wClipLeft;
        if (outer.y < pWin->wClipTop)   outer.y = pWin->wClipTop;
        outer.x &= 0xfffe, outer.width = (outer.width + 1) & 0xfffe;

        if (icvFillEyeRegion(pWin, &inner, &outer, region, eye_region)) {
#ifdef DEBUG_REDEYE_DETECTION
            sprintf(debug, "\r\n(%d, %d), %dx%d -> eye area is %d, %d%% of region area", inner.x, inner.y,
                        inner.width, inner.height, eye_region->area, eye_region->area * 100 / region->area);
            UartOutText(debug);
#endif
            /* check if it's an eye region by area*/
            int portion = eye_region->area * 100 / region->area;
            if ((portion < 150) || (portion > 1800)) {
                cvRleRemoveRegionFromImage(region, map);
                continue;
            }
        }

        /* collect color information of this region */
        CvRun *run;
        int *source, data, average_y, average_cb, average_cr, xL, xR, y;
        average_y = average_cb = average_cr = 0;
        list_for_each_entry( run, &region->run_list, run_node ) {
            xL = run->xL + pWin->wClipLeft;     xR = run->xR + pWin->wClipLeft;
            y = run->y + pWin->wClipTop;
            source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * y));
            for (i = xL; i <= xR; i++) {
                data = source[i >> 1];
                if (i & 1)
                    average_y += (data >> 16) & 0xff;
                else
                    average_y += (data >> 24) & 0xff;
                average_cb += (data >> 8) & 0xff;
                average_cr += data & 0xff;
            }
        }
        average_y /= region->area;
        average_cb /= region->area;
        average_cr /= region->area;

        /* check surrounding pixels with similar color */
        outer = cvRect( inner.x - inner.width, inner.y - inner.height,
                            inner.width * 3, inner.height * 3);
        if (outer.x < pWin->wClipLeft)  outer.x = pWin->wClipLeft;
        if (outer.y < pWin->wClipTop)   outer.y = pWin->wClipTop;
        outer.x &= 0xfffe, outer.width = (outer.width + 1) & 0xfffe;

        surrounding = icvCountSurroundingSimilar(pWin, &inner, &outer, average_y, average_cb, average_cr);
#ifdef DEBUG_REDEYE_DETECTION
        sprintf(debug, "\r\n(%d, %d), %dx%d -> %d%% pixels with similar color", inner.x, inner.y,
                    inner.width, inner.height, surrounding * 100 >> SCALE_BITS);
        UartOutText(debug);
#endif
        if (surrounding > SCALE_CONST(0.04)) {
            cvRleRemoveRegionFromImage(region, map);
            continue;
        }

        /* remove duplicated and close regions */
        int distance, diagonal1, diagonal2;
        CvRleRegion *redeye = list_entry( redeye_map->region_list.next, CvRleRegion, region_node );
        for (i = 0; i < redeye_map->nRegions; i++) {
            CvRect rect = cvRect(redeye->left, redeye->top, redeye->right - redeye->left + 1,
                            redeye->bottom - redeye->top + 1);
            rect.x += pWin->wClipLeft;
            rect.y += pWin->wClipTop;
            if (icvIsDuplicatedRedeye(&rect, &inner, &distance)) {
#ifdef DEBUG_REDEYE_DETECTION
                sprintf(debug, "\r\nduplicated with (%d, %d), %dx%d", rect.x, rect.y, rect.width, rect.height);
                UartOutText(debug);
#endif
                break;
            } else {
                diagonal1 = (rect.width * rect.width) + (rect.height * rect.height);
                diagonal2 = (inner.width * inner.width) + (inner.height * inner.height);
                if (diagonal2 > diagonal1)  diagonal1 = diagonal2;
#ifdef DEBUG_REDEYE_DETECTION
                sprintf(debug, "\r\n(%d, %d), %dx%d to (%d, %d), %dx%d is %d", inner.x, inner.y,
                    inner.width, inner.height, rect.x, rect.y, rect.width, rect.height, distance / diagonal1);
                UartOutText(debug);
#endif
                if (distance < (diagonal1 * 9)) {
                    region->area = redeye->area = -1;   // marked to be removed later
                }
            }
            redeye = list_entry( redeye->region_node.next, CvRleRegion, region_node );
        }
        if (i != redeye_map->nRegions) {
            cvRleRemoveRegionFromImage(region, map);
            continue;
        }

        cvRleExtractRegionFromImage(region, map);
        cvRleAddRegionToImage(region, redeye_map);
    }

    cvRleReleaseRegion(eye_region);
}

static void icvRemoveCloseRegions(CvRleBinaryImage *map)
{
    CvRleRegion *region, *temp;

    list_for_each_entry_safe( region, temp, &map->region_list, region_node ) {
        if (region->area == -1) {
            cvRleRemoveRegionFromImage(region, map);
        }
    }
}

static void icvCorrectRedEye(ST_IMGWIN *pWin, CvRleBinaryImage *map)
{
    CvRleRegion *region;
    CvRun *run, *temp;

    /* collect red-eye runs for each region */
    list_for_each_entry( region, &map->region_list, region_node ) {
        int *source, offset, i, j;

        /* remove original runs */
        list_for_each_entry_safe( run, temp, &region->run_list, run_node ) {
            cvRleRemoveRunFromRegion(run, region);
        }

        /* re-calculate red-eye runs */
        region->left += pWin->wClipLeft;    region->right += pWin->wClipLeft;
        region->top += pWin->wClipTop;  region->bottom += pWin->wClipTop;
        CvRect redeye = cvRect(region->left, region->top, region->right - region->left + 1,
                            region->bottom - region->top + 1);

        source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * redeye.y) + (redeye.x << 1));
        offset = pWin->dwOffset - (redeye.width << 1);
        for (i = redeye.y; i < (redeye.y + redeye.height); i++) {
            for (j = redeye.x, run = 0; j < (redeye.x + redeye.width); j += 2) {
                if (icvIsRedeyeColor(*source) == TRUE) {
                    if (!run) {
                        run = (CvRun *)cvRleCreateRun(j, j, i);
                        cvRleAddRunToRegion(run, region);
                    } else {
                        run->xR = j + 1;
                    }
                } else {
                    if (run) {
                        if ((j - run->xL) > 16)     // filter out runs with length longer than 16
                            cvRleRemoveRunFromRegion(run, region);
                        run = 0;    // finish this run
                    }
                }
                source++;
            }
            source += (offset >> 2);
        }
    }

    /* correct red-eye regions */
    list_for_each_entry( region, &map->region_list, region_node ) {
        int *source, data, average_y, p, dp, y1, y2, cb, cr, i, j;

        /* calculate average luminance */
        average_y = 0;
        list_for_each_entry( run, &region->run_list, run_node ) {
            source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * run->y));
            for (i = run->xL; i <= run->xR; i++) {
                data = source[i >> 1];
                if (i & 1)
                    average_y += (data >> 16) & 0xff;
                else
                    average_y += (data >> 24) & 0xff;
            }
        }
        average_y /= region->area * 25;

        /* adjust luminance and de-saturate */
        list_for_each_entry( run, &region->run_list, run_node ) {
            source = (int *)((char *)pWin->pdwStart + (pWin->dwOffset * run->y));

            dp = SCALE_CONST(0.25);
#ifdef LEFT_SIDE_TAPER
            p = dp;
            for (i = run->xL - 1; i < run->xL + 1; i++) {
                data = source[i >> 1];
                y1 = (data >> 24) & 0xff;
                y2 = (data >> 16) & 0xff;
                cb = ((data >> 8) & 0xff) - 0x80;
                cr = (data & 0xff) - 0x80;
                if (i & 1) {
                    y2 -= (average_y * p) >> SCALE_BITS;
                } else {
                    y1 -= (average_y * p) >> SCALE_BITS;
                }
                cb = (((SCALE_CONST(1.0) - p) * cb) >> SCALE_BITS) + 0x80;
                cr = (((SCALE_CONST(1.0) - p) * cr) >> SCALE_BITS) + 0x80;
                source[i >> 1] = (y1 << 24) | (y2 << 16) | (cb << 8) | cr;
                p += dp;
            }

            for (i = run->xL + 2; i < run->xR - 1; i++) {
#else
            for (i = run->xL; i < run->xR - 1; i++) {
#endif
                data = source[i >> 1];
                y1 = (data >> 24) & 0xff;
                y2 = (data >> 16) & 0xff;
                y1 -= average_y;
                y2 -= average_y;
                source[i >> 1] = (y1 << 24) | (y2 << 16) | (0x80 << 8) | 0x80;
            }

            p = SCALE_CONST(0.75);
            for (i = run->xR - 1; i < run->xR + 2; i++) {
                data = source[i >> 1];
                y1 = (data >> 24) & 0xff;
                y2 = (data >> 16) & 0xff;
                cb = ((data >> 8) & 0xff) - 0x80;
                cr = (data & 0xff) - 0x80;
                if (i & 1) {
                    y2 -= (average_y * p) >> SCALE_BITS;
                } else {
                    y1 -= (average_y * p) >> SCALE_BITS;
                }
                cb = (((SCALE_CONST(1.0) - p) * cb) >> SCALE_BITS) + 0x80;
                cr = (((SCALE_CONST(1.0) - p) * cr) >> SCALE_BITS) + 0x80;
                source[i >> 1] = (y1 << 24) | (y2 << 16) | (cb << 8) | cr;
                p -= dp;
            }
        }
    }
}

static void icvGetImgWinInfo(ST_IMGWIN *pSrcWin, ST_IMGWIN *pTrgWin)
{
    pTrgWin->pdwStart = pSrcWin->pdwStart;
    pTrgWin->dwOffset = pSrcWin->dwOffset;

    if ((pSrcWin->wClipRight - pSrcWin->wClipLeft) > 0) {
        pTrgWin->wWidth = pSrcWin->wClipRight - pSrcWin->wClipLeft;
        pTrgWin->wHeight = pSrcWin->wClipBottom - pSrcWin->wClipTop;
        pTrgWin->wClipLeft = pSrcWin->wClipLeft;
        pTrgWin->wClipRight = pSrcWin->wClipRight;
        pTrgWin->wClipTop = pSrcWin->wClipTop;
        pTrgWin->wClipBottom = pSrcWin->wClipBottom;
    } else {
        pTrgWin->wWidth = pSrcWin->wWidth;
        pTrgWin->wHeight = pSrcWin->wHeight;
        pTrgWin->wClipLeft = 0;
        pTrgWin->wClipTop = 0;
        pTrgWin->wClipRight = pSrcWin->wWidth;
        pTrgWin->wClipBottom = pSrcWin->wHeight;
    }
}

static IplImage* icvImgWin2IplImage(ST_IMGWIN *pWin)
{
    CvSize size;
    int width, height;
    IplImage *image = 0;

    if ((pWin->wClipRight - pWin->wClipLeft) > 0) {
        width = pWin->wClipRight - pWin->wClipLeft;
        height = pWin->wClipBottom - pWin->wClipTop;
    } else {
        width = pWin->wWidth;
        height = pWin->wHeight;
    }
    size = cvSize(width, height);

    /* set channels to 2 for YCbCr 4:2:2 to avoid error from cvInitMatHeader() (step < min_step) */
    if (image = (IplImage *)cvCreateImageHeader(size, 8, 2)) {
        image->imageData = (char *)pWin->pdwStart + (pWin->dwOffset * pWin->wClipTop) + (pWin->wClipLeft << 1);
        image->widthStep = pWin->dwOffset;
    }

    return image;
}

//////////////////////////////////////////////////////////////////////////////////////////

//#define CHECK_SCALED_IMAGE
int cvScaleImage(ST_IMGWIN *pSrcWin, ST_IMGWIN *pTrgWin, CvSize *pSrcSize, CvSize *pTrgSize,
                                int nMaxWidth, int nMaxHeight, int maximize)
{
    ST_IMGWIN tempWin;
    int width, height;

    icvGetImgWinInfo(pSrcWin, &tempWin);
    *pSrcSize = cvSize(tempWin.wWidth, tempWin.wHeight);

    /* calculate target image size */
    if (maximize) {
        width = tempWin.wWidth & 0xfffe;
        height = tempWin.wHeight;
        while (width * height > nMaxWidth * nMaxHeight) {
            width -= 16;
            height = width * pSrcSize->height / pSrcSize->width;
        }
    } else {
        if (tempWin.wHeight * nMaxWidth > tempWin.wWidth * nMaxHeight) {
            height = nMaxHeight;
            width = height * pSrcSize->width / pSrcSize->height;
            width &= 0xfffe;
        } else {
            width = nMaxWidth & 0xfffe;
            height = width * pSrcSize->height / pSrcSize->width;
        }
    }
    pTrgSize->width = width;
    pTrgSize->height = height;

    memset(pTrgWin, 0, sizeof(ST_IMGWIN));
#ifdef CHECK_SCALED_IMAGE   // for internal check
    /* pSrcWin must contain information of the display buffer */
    pTrgWin->pdwStart = pSrcWin->pdwStart;
    pTrgWin->dwOffset = pSrcWin->dwOffset;
    pTrgWin->wWidth = pSrcWin->wWidth;
    pTrgWin->wHeight = pSrcWin->wHeight;
#else
    pTrgWin->pdwStart = (DWORD *)icvAlloc(pTrgSize->width * pTrgSize->height << 1, 0);
    pTrgWin->dwOffset = pTrgSize->width << 1;
    if (!pTrgWin->pdwStart)
        return FAIL;
    pTrgWin->wWidth = pTrgSize->width;
    pTrgWin->wHeight = pTrgSize->height;
#endif

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_620) || ((CHIP_VER & 0xffff0000) == CHIP_VER_630))
    tempWin.pdwStart = (DWORD *)((BYTE *)tempWin.pdwStart + (tempWin.dwOffset * tempWin.wClipTop) + (tempWin.wClipLeft << 1));
    Ipu_ImageScaling(&tempWin, pTrgWin, pTrgSize->width, pTrgSize->height, 0, 0);
#else
    Ipu_ImageScaling(pSrcWin, pTrgWin, tempWin.wClipLeft, tempWin.wClipTop, tempWin.wClipRight, tempWin.wClipBottom,
                     0, 0, pTrgSize->width, pTrgSize->height, 0);
#endif

    pTrgWin->wClipRight = pTrgSize->width;
    pTrgWin->wClipBottom = pTrgSize->height;

    return PASS;
}

#if FACE_DETECTION
/*
 * nScale: Defines the minimal face to detect relative to the maximum of image width and height.
 *         (The valid range of nScale is [2, 10] currently.)
 * nMaxFaceNum: The max number of faces to detect.
 */
CvSeq *cvDetectFaces(ST_IMGWIN *pWin, int nScale, int nMaxFaceNum)
{
    CvHaarClassifierCascade *cascade = 0;
    ST_IMGWIN small_image;
    IplImage *image, *gray;
    CvSize size, small_size;
    char *source, pre_processing = TRUE;
    int offset, i;
    CvSeq *faces = 0;

    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    /* load classifier cascade */
    cascade = (CvHaarClassifierCascade *)&frontalface;

    /* resize the original image to a smaller resolution for better performance */
    if (cvScaleImage(pWin, &small_image, &size, &small_size, 160, 120, 1) != PASS)
        return faces;

    image = icvImgWin2IplImage(&small_image);
    source = image->imageData;
    offset = image->widthStep - (small_size.width << 1);
    gray = (IplImage *)cvCreateImage(small_size, 8, 1);

    /* if the image is monochromic, do not perform pre-processing */
    if (icvIsMonochromicImage((int *)source, small_size, offset))
        pre_processing = FALSE;

    if (pre_processing) {
        /* extract skin color from original color image */
        CvRleBinaryImage *skin_map = (CvRleBinaryImage *)icvCreateSkinMap((int *)source, small_size, offset);
#ifdef INTERNAL_CHECK_ENABLE
        icvCheckBinaryMap((int *)source, small_size, offset, skin_map, 0xFFFF0080);   // for internal check
#endif
        TaskYield();

        /* convert color image to gray image according to extracted skin tone map */
        if (skin_map) {
#if 0       /* perform erosion and dilation, marked due to bad performance */
            CvRleElement *element = (CvRleElement *)cvRleCreateStructuringElement(5, 5, 0, 0, CV_SHAPE_RECT, 0);
            cvRleErode(skin_map, element);
            cvRleDilate(skin_map, element);
            cvRleReleaseStructuringElement(&element);
#endif
            cvRleDivideRegions(skin_map);
            cvRleFilterAndSortRegions(skin_map, (small_size.width > small_size.height) ?
                                      (small_size.width / nScale) : (small_size.height / nScale));
#ifdef INTERNAL_CHECK_ENABLE
            icvCheckBinaryRegion(&small_image, skin_map, 0);     // for internal check
#endif
            if (skin_map->nRegions == 0) {
                cvCvtColor(image, gray, CV_YYCbCr2GRAY);
                pre_processing = FALSE;
            } else {
                CvSeq *regions = (CvSeq *)cvRleGetCombinedRegions(skin_map, storage);
#ifdef INTERNAL_CHECK_ENABLE
                icvCheckBinaryRegion(&small_image, 0, regions);     // for internal check
#endif
                icvExtractRegionLuma((short *)source, small_size, offset, (short *)gray->imageData, regions);
            }
            cvRleReleaseBinaryImage(skin_map);
        } else {
            cvCvtColor(image, gray, CV_YYCbCr2GRAY);
            pre_processing = FALSE;
        }
    } else {
        cvCvtColor(image, gray, CV_YYCbCr2GRAY);
    }
    TaskYield();

#ifdef INTERNAL_CHECK_ENABLE
    icvCheckDetectImage((short *)gray->imageData, (short *)image->imageData, small_size, image->widthStep);  // for internal check
#endif

    /* perform histogram equalization for more accuracy */
    if (!pre_processing)
        cvEqualizeHist(gray, gray);
    cvClearMemStorage(storage);

    if (cascade) {
        int min_length = (small_size.width > small_size.height) ? (small_size.width / nScale) : (small_size.height / nScale);
        if (min_length < 20)    min_length = 20;

        CvSeq *small_faces = (CvSeq *)cvHaarDetectObjects(gray, cascade, storage,
                                          SCALE_CONST(0.85), 1, 0, cvSize(min_length, min_length));
        if (small_faces) {
            if (small_faces->total == 0) {
                faces = small_faces;
            } else {
                faces = (CvSeq *)cvCreateSeq(0, sizeof(CvSeq), sizeof(CvAvgComp), storage);
                for (i = 0; i < small_faces->total; i++) {
                    CvAvgComp r = *(CvAvgComp*)cvGetSeqElem(small_faces, i);
                    r.rect.x = ((r.rect.x * size.width / small_size.width) + 1) & 0xfffe;
                    r.rect.width = (r.rect.width * size.width / small_size.width) & 0xfffe;
                    r.rect.y = ((r.rect.y * size.height / small_size.height) + 1) & 0xfffe;
                    r.rect.height = r.rect.width;
                    r.rect.x += pWin->wClipLeft;    r.rect.y += pWin->wClipTop;
                    cvSeqPush(faces, &r);
                }
            }
        }
    }

    cvReleaseImage(&gray);
    cvReleaseImageHeader(&image);
    icvFree(small_image.pdwStart, 0);
    if (cascade)
        cvReleaseHaarClassifierCascade(&cascade);

    return faces;
}

void cvUninitialFaceEngine(void)
{
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}
#endif  /* FACE_DETECTION */

/*
 * rcFace: The rectangle of face region. It could be NULL, if this parameter is NULL,
 *         the library will detect face automatically.
 * nFaces: The number of faces.
 * nLevel: Clean level of face beautifier, with range between [0, 100]. Less for more
 *         detail and larger for smoother.
 */
int cvBeautifyFace(ST_IMGWIN *pWin, CvRect *rcFace, int nFaces, int nLevel)
{
    CvRect *faces, *alloc_faces = 0;
    char *source;
    int offset, i, channels = 3;

    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    IplImage *image = icvImgWin2IplImage(pWin);
    CvSize size = cvSize(image->width, image->height);
    source = image->imageData;
    offset = image->widthStep - (size.width << 1);

    /* if the image is monochromic, we only deal with the luminance channel */
    if (icvIsMonochromicImage((int *)source, size, offset))
        channels = 1;

    if (rcFace == 0) {      // no specified face region
        ST_IMGWIN small_image;
        CvSize image_size, small_size;
        CvSeq *regions;

        if (cvScaleImage(pWin, &small_image, &image_size, &small_size, 160, 120, 1) != PASS) {
            cvReleaseImageHeader(&image);
            return FAIL;
        }
        if (channels == 1) {
#if FACE_DETECTION
            cvReleaseMemStorage(&storage);
            regions = (CvSeq *)cvDetectFaces(&small_image, 10, 3);
            cvUninitialFaceEngine();
            storage = (CvMemStorage *)cvCreateMemStorage(0);
#else
            regions = (CvSeq *)cvCreateSeq(0, sizeof(CvSeq), sizeof(CvRect), storage);
            /* no face region found, regions->total = 0 */
#endif
        } else {
            /* extract skin tone regions as possible face regions */
            source = (char *)small_image.pdwStart + (small_image.dwOffset * small_image.wClipTop) + (small_image.wClipLeft << 1);
            offset = small_image.dwOffset - (small_size.width << 1);
            CvRleBinaryImage *skin_map = (CvRleBinaryImage *)icvCreateSkinMap((int *)source, small_size, offset);
            cvRleDivideRegions(skin_map);
            cvRleFilterAndSortRegions(skin_map, (small_size.width > small_size.height) ?
                                    (small_size.width / 10) : (small_size.height / 10));
            regions = (CvSeq *)cvRleGetRegions(skin_map, storage);
            cvRleReleaseBinaryImage(skin_map);
        }
        icvFree(small_image.pdwStart, 0);

        nFaces = regions->total;
        rcFace = (CvRect *)icvAlloc(nFaces * sizeof(CvRect), 0);
        alloc_faces = faces = rcFace;
        for (i = 0; i < regions->total; i++) {
            CvRect *rect = (CvRect *)cvGetSeqElem(regions, i);
            rect->x = ((rect->x * image_size.width / small_size.width) + 1) & 0xfffe;
            rect->width = (rect->width * image_size.width / small_size.width) & 0xfffe;
            rect->y = ((rect->y * image_size.height / small_size.height) + 1) & 0xfffe;
            rect->height = rect->height * image_size.height / small_size.height;
            rect->x += pWin->wClipLeft;    rect->y += pWin->wClipTop;
            *faces++ = *rect;
        }
    }
    TaskYield();

    if (nFaces != 0) {
        faces = rcFace;
        for (i = 0; i < nFaces; i++) {
            image->width = faces->width, image->height = faces->height;
            image->imageData = (char *)pWin->pdwStart + (pWin->dwOffset * faces->y) + (faces->x << 1);
            size = cvSize(image->width, image->height);
            IplImage *temp_face = (IplImage *)cvCreateImage(size, 8, channels);

            if (channels == 1)
                cvCvtColor(image, temp_face, CV_YYCbCr2GRAY);
            else
                icvConvertColor(image, temp_face, size);
            cvSmooth(temp_face, temp_face, CV_BILATERAL, 5, 0, nLevel, 3);
            cvCvtColor(temp_face, image, (channels == 1) ? CV_GRAY2YYCbCr : CV_YCbCr2YYCbCr);
            TaskYield();

            cvReleaseImage(&temp_face);
            faces++;
        }
    }

    if (alloc_faces)
        icvFree(alloc_faces, 0);
    cvReleaseImageHeader(&image);

    return nFaces;
}

void cvUninitialFaceBeautifier(void)
{
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

/*
 * rcFace: The rectangle of face region. It could be NULL, if this parameter is NULL,
 *         the library will detect red-eye in full image, otherwise, it will detect
 *         in the specified face region.
 * nFaces: The number of faces.
 */
int cvRemoveRedEye(ST_IMGWIN *pSrcWin, CvRect *rcFace, int nFaces, ST_IMGWIN *pTrgWin)
{
    ST_IMGWIN tempWin;
    CvSize source_size, temp_size;
    char *source;
    int offset, i, redeyes = 0;
    CvRect *faces, *alloc_faces = 0;

    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    /* resize the original image to a specific resolution for more accuracy */
    if (cvScaleImage(pSrcWin, &tempWin, &source_size, &temp_size, 800, 600, 0) != PASS)
        return FAIL;

    source = (char *)tempWin.pdwStart + (tempWin.dwOffset * tempWin.wClipTop) + (tempWin.wClipLeft << 1);
    offset = tempWin.dwOffset - (tempWin.wWidth << 1);
    /* if the image is monochromic, we won't find any red-eye */
    if (icvIsMonochromicImage((int *)source, temp_size, offset)) {
        icvFree(tempWin.pdwStart, 0);
        return 0;
    }

    if (rcFace == 0) {      // no specified face region
        ST_IMGWIN small_image;
        CvSize image_size, small_size;

        /* extract skin tone regions as possible face regions */
        if (cvScaleImage(&tempWin, &small_image, &image_size, &small_size, 160, 120, 1) != PASS) {
            icvFree(tempWin.pdwStart, 0);
            return FAIL;
        }
        source = (char *)small_image.pdwStart + (small_image.dwOffset * small_image.wClipTop) + (small_image.wClipLeft << 1);
        offset = small_image.dwOffset - (small_size.width << 1);
        CvRleBinaryImage *skin_map = (CvRleBinaryImage *)icvCreateSkinMap((int *)source, small_size, offset);
        cvRleDivideRegions(skin_map);
        cvRleFilterAndSortRegions(skin_map, (small_size.width > small_size.height) ?
                                    (small_size.width / 10) : (small_size.height / 10));
        CvSeq *regions = (CvSeq *)cvRleGetRegions(skin_map, storage);
        cvRleReleaseBinaryImage(skin_map);
        icvFree(small_image.pdwStart, 0);

        nFaces = regions->total;
        rcFace = (CvRect *)icvAlloc(nFaces * sizeof(CvRect), 0);
        alloc_faces = faces = rcFace;
        for (i = 0; i < regions->total; i++) {
            CvRect *rect = (CvRect *)cvGetSeqElem(regions, i);
            rect->x = ((rect->x * image_size.width / small_size.width) + 1) & 0xfffe;
            rect->width = (rect->width * image_size.width / small_size.width) & 0xfffe;
            rect->y = ((rect->y * image_size.height / small_size.height) + 1) & 0xfffe;
            rect->height = rect->height * image_size.height / small_size.height;
            *faces++ = *rect;
        }
    } else {
        faces = rcFace;
        for (i = 0; i < nFaces; i++) {
            faces->x -= pSrcWin->wClipLeft;    faces->y -= pSrcWin->wClipTop;
            faces->x = ((faces->x * temp_size.width / source_size.width) + 1) & 0xfffe;
            faces->width = (faces->width * temp_size.width / source_size.width) & 0xfffe;
            faces->y = ((faces->y * temp_size.height / source_size.height) + 1) & 0xfffe;
            faces->height = faces->height * temp_size.height / source_size.height;
            faces++;
        }
    }
    TaskYield();

    if (nFaces != 0) {
        ST_IMGWIN trgWin;
        CvRleRegion *region;
        CvRleBinaryImage *redeye_map = 0;
        CvRect image = cvRect(0, 0, tempWin.wWidth, tempWin.wHeight);
        redeye_map = (CvRleBinaryImage *)cvRleCreateBinaryImage(image.width, image.height);

        /* detect red-eye in each face region */
        faces = rcFace;
        for (i = 0; i < nFaces; i++) {
            source = (char *)tempWin.pdwStart + (tempWin.dwOffset * faces->y) + (faces->x << 1);
            offset = tempWin.dwOffset - (faces->width << 1);
            CvRleBinaryImage *temp_map = (CvRleBinaryImage *)icvCreateRedeyeMap((int *)source, faces, offset);
            if (temp_map) {
                cvRleDivideRegions(temp_map);
                icvDetectRedEye(&tempWin, temp_map, redeye_map, &image, alloc_faces ? faces : 0);
                cvRleReleaseBinaryImage(temp_map);
            }
            faces++;
        }
        icvRemoveCloseRegions(redeye_map);
        TaskYield();

        /* calculate corresponding red-eye region positions in the target window */
        icvGetImgWinInfo(pTrgWin, &trgWin);
        list_for_each_entry( region, &redeye_map->region_list, region_node ) {
#ifdef DEBUG_REDEYE_DETECTION
            char debug[80];
            sprintf(debug, "\r\nred-eye: (%d, %d), %dx%d", region->left, region->top,
                    region->right - region->left + 1, region->bottom - region->top + 1);
            UartOutText(debug);
#endif
            region->left = ((region->left * trgWin.wWidth / temp_size.width) - 1) & 0xfffe;
            region->top = (region->top * trgWin.wHeight / temp_size.height) - 1;
            region->right = (region->right * trgWin.wWidth / temp_size.width) + 1;
            if ((region->right & 1) == 0)   region->right++;
            region->bottom = (region->bottom * trgWin.wHeight / temp_size.height) + 1;
            region->area = (region->right - region->left + 1) * (region->bottom - region->top + 1);
        }

#ifdef INTERNAL_CHECK_ENABLE
        icvCheckBinaryRegion(pTrgWin, redeye_map, 0);      // for internal check
#endif
        icvCorrectRedEye(pTrgWin, redeye_map);
        redeyes = redeye_map->nRegions;
        cvRleReleaseBinaryImage(redeye_map);
    }

    if (alloc_faces)
        icvFree(alloc_faces, 0);
    icvFree(tempWin.pdwStart, 0);

    return redeyes;
}

void cvUninitialRedEyeEngine(void)
{
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

#define ADJUST_YGAMMA_ONLY
extern BYTE red_gamma[], green_gamma[], blue_gamma[];

/*
 * nLevels: The desired levels of luminance adjustment for dark areas.
 *          The value range is from 0 to 10.
 */
int cvDynamicLighting(ST_IMGWIN *pWin, int nLevels)
{
    ST_IMGWIN small_image;
    CvSize image_size, small_size;

    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    /* calculate intensity mapping function using small image */
    if (cvScaleImage(pWin, &small_image, &image_size, &small_size, 320, 240, 1) != PASS)
        return FAIL;

    IplImage *image = icvImgWin2IplImage(&small_image);
    IplImage *gray = (IplImage *)cvCreateImage(small_size, 8, 1);
    cvCvtColor(image, gray, CV_YYCbCr2GRAY);

    CvHistogram *hist = (CvHistogram *)cvCalculateHist(gray);
    CvMat *lut = (CvMat *)cvCreateMat(1, 256, 0);
    cvEqualizeSubHist(hist, lut, storage, nLevels);
    TaskYield();

#ifdef ADJUST_YGAMMA_ONLY
    char *source = (char *)pWin->pdwStart + (pWin->dwOffset * pWin->wClipTop) + (pWin->wClipLeft << 1);
    int offset = pWin->dwOffset - (image_size.width << 1);
    icvTableLookup(source, image_size, offset, lut->data.ptr);
#else
    BYTE *gamma_table = (BYTE *)icvAlloc(3 * 256, 0);
    memcpy(gamma_table, red_gamma, 256);
    memcpy(gamma_table + 256, green_gamma, 256);
    memcpy(gamma_table + 512, blue_gamma, 256);
    memcpy(red_gamma, lut->data.ptr, 256);
    memcpy(green_gamma, lut->data.ptr, 256);
    memcpy(blue_gamma, lut->data.ptr, 256);
    asm_gamma_correct(pWin, pWin);
    memcpy(red_gamma, gamma_table, 256);
    memcpy(green_gamma, gamma_table + 256, 256);
    memcpy(blue_gamma, gamma_table + 512, 256);
    icvFree(gamma_table, 0);
#endif

    cvReleaseMat(&lut);
    cvReleaseHist(&hist);
    cvReleaseImage(&gray);
    cvReleaseImageHeader(&image);
    icvFree(small_image.pdwStart, 0);
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

/*
 * nAzimuth: This is about lighting according to the points of the compass (0 ~ 359).
 *           If you suppose South is at the top of your image, then East (0) is on the left.
 *           Increasing value goes counter-clockwise.
 * nElevation: That's height from horizon (0), in principle up to zenith (90),
 *             but here up to the opposite horizon (180).
 * nDepth: The distance of the light source. Light decreases when value increases.
 */
void cvEmbossFiltering(ST_IMGWIN *pWin, int nAzimuth, int nElevation, int nDepth)
{
    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    IplImage *image = icvImgWin2IplImage(pWin);
    CvSize size = cvSize(image->width, image->height);
    IplImage *temp_gray = (IplImage *)cvCreateImage(size, 8, 1);
    IplImage *gray = (IplImage *)cvCreateImage(size, 8, 1);

    cvCvtColor(image, temp_gray, CV_YYCbCr2GRAY);
    cvEmboss(temp_gray, gray, nAzimuth, nElevation, nDepth);
    cvCvtColor(gray, image, CV_GRAY2YY8080);

    cvReleaseImage(&gray);
    cvReleaseImage(&temp_gray);
    cvReleaseImageHeader(&image);
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

/*
 * nWidth: The desired width of the blocks.
 * nHeight:  The height of the blocks.
 */
void cvMosaicFiltering(ST_IMGWIN *pWin, int nWidth, int nHeight)
{
    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    IplImage *image = icvImgWin2IplImage(pWin);
    cvPixelize(image, image, nWidth, nHeight);

    cvReleaseImageHeader(&image);
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

/*
 * nLevels: The number of levels (2 ~ 256) in each RGB channel that will be used to describe the
 *          active layer. The total number of colors is the combination of these levels.
 */
void cvPosterFiltering(ST_IMGWIN *pWin, int nLevels)
{
    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    IplImage *image = icvImgWin2IplImage(pWin);
    CvSize size = cvSize(image->width, image->height);
    IplImage *rgb_image = (IplImage *)cvCreateImage(size, 8, 3);

    cvCvtColor(image, rgb_image, CV_YYCbCr2RGB);
    cvPosterize(rgb_image, rgb_image, nLevels);
    cvCvtColor(rgb_image, image, CV_RGB2YYCbCr);

    cvReleaseImage(&rgb_image);
    cvReleaseImageHeader(&image);
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

/*
 * nShiftAmount: The maximum shift, between 1 and 100 pixels.
 */
void cvJitterFiltering(ST_IMGWIN *pWin, int nShiftAmount)
{
    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    IplImage *image = icvImgWin2IplImage(pWin);

    if ((pWin->wClipRight - pWin->wClipLeft) > 0) {
        int extra_lines = (pWin->wClipTop > nShiftAmount) ? nShiftAmount : pWin->wClipTop;
        image->imageData = (char *)pWin->pdwStart + (pWin->dwOffset * (pWin->wClipTop - extra_lines))
                                       + (pWin->wClipLeft << 1);
        image->height += extra_lines * 2;
    }

    CvSize size = cvSize(image->width, image->height);
    IplImage *ycbcr_image = (IplImage *)cvCreateImage(size, 8, 3);

    cvCvtColor(image, ycbcr_image, CV_YYCbCr2YCbCr);
    cvShift(ycbcr_image, ycbcr_image, nShiftAmount, 1, 0x008080);   /* 0: horizontal, 1: vertical */
    cvCvtColor(ycbcr_image, image, CV_YCbCr2YYCbCr);

    cvReleaseImage(&ycbcr_image);
    cvReleaseImageHeader(&image);
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}

/*
 * nMaskSize: The size of the brush mask used to paint the oily render.
 *            Larger values produce an oilier render.
 * nExponent: Oil paint exponent.
 * bIntensity: Using intensity algorithm changes the mode of operation to help
 *             preserve detail and coloring.
 */
void cvOilifyFiltering(ST_IMGWIN *pWin, int nMaskSize, int nExponent, unsigned char bIntensity)
{
    cvSetMemoryManager((CvAllocFunc)icvAlloc, (CvFreeFunc)icvFree, (CvYieldFunc)TaskYield, NULL);
    storage = (CvMemStorage *)cvCreateMemStorage(0);

    IplImage *image = icvImgWin2IplImage(pWin);
    CvSize size = cvSize(image->width, image->height);
    IplImage *rgb_image = (IplImage *)cvCreateImage(size, 8, 3), *gray = 0;

    cvCvtColor(image, rgb_image, CV_YYCbCr2RGB);
    if( bIntensity ) {
        gray = (IplImage *)cvCreateImage(size, 8, 1);
        cvCvtColor(image, gray, CV_YYCbCr2GRAY);
        cvOilify(rgb_image, rgb_image, gray->imageData, nMaskSize, nExponent);
    } else {
        cvOilify(rgb_image, rgb_image, NULL, nMaskSize, nExponent);
    }
    cvCvtColor(rgb_image, image, CV_RGB2YYCbCr);

    if( gray )  cvReleaseImage(&gray);
    cvReleaseImage(&rgb_image);
    cvReleaseImageHeader(&image);
    if (storage)
        cvReleaseMemStorage(&storage);
    cvReleaseErrContext();
}
