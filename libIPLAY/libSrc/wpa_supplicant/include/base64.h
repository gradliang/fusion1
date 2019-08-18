/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef BASE64_H
#define BASE64_h

unsigned char * base64_encode(const unsigned char *src, size_t len,
			      size_t *out_len);
unsigned char * base64_decode(const unsigned char *src, size_t len,
			      size_t *out_len);

/* definitions for other modules which call base64_encode()/base64_decode() to easily use consistent memory alloc/free functions ! */
/* note: Don't use os_malloc()/os_free() or mm_malloc()/mm_free() here!
 *       Because current mm_malloc()'s memory pool is small and SMTP/POP3 modules may use base64_en/decode for email attachment
 *       file (ex: pictures) processing. Using mm_malloc()/mm_free() here may cause subsequent memory allocation problem for those
 *       network applications (ex: Netfs) which also use mm_malloc()/mm_free().
 */
#define base64_malloc(sz)   ext_mem_malloc(sz)
#define base64_mfree(ptr)   ext_mem_free(ptr)


#endif /* BASE64_H */
