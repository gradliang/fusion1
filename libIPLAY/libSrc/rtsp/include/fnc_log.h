/* *
 * This file is part of Feng
 *
 * Copyright (C) 2009 by LScube team <team@lscube.org>
 * See AUTHORS for more details
 *
 * feng is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * feng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with feng; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */


#ifndef FN_FNC_LOG_H
#define FN_FNC_LOG_H

#include <config.h>
#include <stdarg.h>
#include <errno.h>

enum {  FNC_LOG_OUT,
        FNC_LOG_SYS,
        FNC_LOG_FILE };

enum {
    FMC_LOG_NONE,       //!< No logging
    FNC_LOG_FATAL,      //!< Fatal error
    FNC_LOG_ERR,        //!< Recoverable error
    FNC_LOG_WARN,       //!< Warning
    FNC_LOG_INFO,       //!< Informative message
    FNC_LOG_CLIENT,     //!< Client response
    FNC_LOG_DEBUG,      //!< Debug
    FNC_LOG_VERBOSE,    //!< Overly verbose debug
};

void fnc_log(unsigned int level, const char *fmt, ...);

#ifdef TRACE
#define fnc_log(level, fmt, string...)                              \
    fnc_log(level, "[%s - %d]" fmt, __FILE__, __LINE__ , ## string)
#endif

void _fnc_perror(int errno_val, const char *function, const char *comment);

#define fnc_perror(comment) _fnc_perror(errno, __func__, comment)

void fnc_log_init();

#endif // FN_FNC_LOG_H
