/*****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2009, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      log.h
*
* Programmer:    Mingfen lin
*                MPX E360 division
*
* Created: 	 04/28/2009
*
* Description: AV util log include file   
*              
*        
* Change History (most recent first):
*     <1>     04/28/2009    Mingfen Lin    first file
*****************************************************************/

#ifndef AVUTIL_LOG_H
#define AVUTIL_LOG_H

//#include <stdarg.h>
//#include "avutil.h"

/**
 * Describes the class of an AVClass context structure. That is an
 * arbitrary struct of which the first field is a pointer to an
 * AVClass struct (e.g. AVCodecContext, AVFormatContext etc.).
 */
typedef struct AVCLASS AVClass;
struct AVCLASS {
    /**
     * The name of the class; usually it is the same name as the
     * context structure type to which the AVClass is associated.
     */
    const char* class_name;

    /**
     * A pointer to a function which returns the name of a context
     * instance \p ctx associated with the class.
     */
    const char* (*item_name)(void* ctx);

    /**
     * a pointer to the first option specified in the class if any or NULL
     *
     * @see av_set_default_options()
     */
    const struct AVOption *option;
};

/* av_log API */

#define AV_LOG_QUIET    -8

/**
 * Something went really wrong and we will crash now.
 */
#define AV_LOG_PANIC     0

/**
 * Something went wrong and recovery is not possible.
 * For example, no header was found for a format which depends
 * on headers or an illegal combination of parameters is used.
 */
#define AV_LOG_FATAL     8

/**
 * Something went wrong and cannot losslessly be recovered.
 * However, not all future data is affected.
 */
#define AV_LOG_ERROR    16

/**
 * Something somehow does not look correct. This may or may not
 * lead to problems. An example would be the use of '-vstrict -2'.
 */
#define AV_LOG_WARNING  24

#define AV_LOG_INFO     32
#define AV_LOG_VERBOSE  40

/**
 * Stuff which is only useful for libav* developers.
 */
#define AV_LOG_DEBUG    48

/**
 * Sends the specified message to the log if the level is less than or equal
 * to the current av_log_level. By default, all logging messages are sent to
 * stderr. This behavior can be altered by setting a different av_vlog callback
 * function.
 *
 * @param avcl A pointer to an arbitrary struct of which the first field is a
 * pointer to an AVClass struct.
 * @param level The importance level of the message, lower values signifying
 * higher importance.
 * @param fmt The format string (printf-compatible) that specifies how
 * subsequent arguments are converted to output.
 * @see av_vlog
 */
#ifdef __GNUC__
void av_log(void*, int level, const char *fmt, ...) __attribute__ ((__format__ (__printf__, 3, 4)));
#else
void av_log(void*, int level, const char *fmt, ...);
#endif

//void av_vlog(void*, int level, const char *fmt, va_list);
//int av_log_get_level(void);
//void av_log_set_level(int);
//void av_log_set_callback(void (*)(void*, int, const char*, va_list));
//void av_log_default_callback(void* ptr, int level, const char* fmt, va_list vl);

#endif /* AVUTIL_LOG_H */
