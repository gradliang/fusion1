/* Copyright (C) 1991-2001, 2003, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef	_NETINET_IN_H
#define	_NETINET_IN_H	1

#ifndef HAVE_LWIP
//#include <stdint.h>
#endif

#include "net_sockaddr.h"
/* Type to represent a port.  */
//typedef uint16_t in_port_t;

/* Internet address.  */
//typedef uint32_t in_addr_t;
/*
struct in_addr
  {
    in_addr_t s_addr;
  };
*/

/* Structure describing a generic socket address.  */
/*
struct sockaddr
  {
    __SOCKADDR_COMMON (sa_);	// Common data: address family and length.  
    char sa_data[14];		// Address data.  
  };
*/

/* Structure describing an Internet socket address.  */
/*
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    in_port_t sin_port;			// Port number.  
    struct in_addr sin_addr;		// Internet address.  

    // Pad to size of `struct sockaddr'.  
    unsigned char sin_zero[sizeof (struct sockaddr) -
			   __SOCKADDR_COMMON_SIZE -
			   sizeof (in_port_t) -
			   sizeof (struct in_addr)];
  };
*/
# define ntohl(x)	(x)
# define ntohs(x)	(x)
# define htonl(x)	(x)
# define htons(x)	(x)

#endif	/* netinet/in.h */

