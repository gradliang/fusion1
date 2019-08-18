/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2007-2012  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __CONNMAN_LOG_H
#define __CONNMAN_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

struct connman_debug_desc {
	const char *name;
	const char *file;
#define CONNMAN_DEBUG_FLAG_DEFAULT (0)
#define CONNMAN_DEBUG_FLAG_PRINT   (1 << 0)
#define CONNMAN_DEBUG_FLAG_ALIAS   (1 << 1)
	unsigned int flags;
} __attribute__((aligned(8)));

#define CONNMAN_DEBUG_DEFINE(name) \
	static struct connman_debug_desc __debug_alias_ ## name \
	__attribute__((used, section("__debug"), aligned(8))) = { \
		#name, __FILE__, CONNMAN_DEBUG_FLAG_ALIAS \
	};

extern unsigned char TaskGetId(void);
extern char *task_get_name(int);
/**
 * DBG:
 * @fmt: format string
 * @arg...: list of arguments
 *
 * Simple macro around connman_debug() which also include the function
 * name it is called in.
 */
#undef DBG
#define DBG(fmt, arg...) do { \
		int tid = TaskGetId(); \
		connman_debug("%s[%u]: %s:%s() " fmt, \
					task_get_name(tid), tid, __FILE__, __FUNCTION__ , ## arg); \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* __CONNMAN_LOG_H */
