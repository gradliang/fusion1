/* net_doc.h  
 *
 * Copyright (C) 2009 Magic Pixel Inc.
 * All Rights Reserved.
 *
 */

/* This file contains documentation for Doxygen and doesnot have
 * any significance with respect to C API
 */

#ifndef NET_DOC_H
#define NET_DOC_H


/**
 * @defgroup    NET_SOCKET      Network Sockets
 * @brief The Network sockets API 
 *
 * This is an implementation of BSD socket library.  All API functions follow 
 * BSD socket library's usage as much as possible.  But there are some exceptions,
 * which are listed below.
 *
 * <b>BSD Sockets Compatibility</b>
 *
 * @li select() vs. Unix's select()
 * <br>The 3 socket set parameters use ST_SOCK_SET structure.  But this should
 * be transparent to applications as long as proper macros 
 * (<em>FD_SET, FD_CLR, FD_ISSET, FD_ZERO</em>) are used to manipulate the bit fields.
 * @li closesocket() vs. Unix's close()
 * <br>File descriptor and socket descriptor are not treated equivalently on MP6XX 
 * platform.
 * @li ioctlsocket() vs. Unix's ioctl()
 * <br>File descriptor and socket descriptor are not treated equivalently on MP6XX 
 * platform.
 *
 * @{
 */

/**
 * @var errno
 *
 * Error code for last socket operation that failed.
 */

/**
 * @typedef fd_set
 *
 * The fd_set structure is used by various socket functions to place sockets 
 * into a "set" for various purposes.  The fd_set structure is defined in the
 * toolchain's C header files.
 */

/**
 * @def FD_SET
 *
 * Sets the bit of the file descriptor in the file descriptor set.
 */

/**
 * @def FD_CLR
 *
 * Clears the bit of the file descriptor in the file descriptor set.
 */

/**
 * @def FD_ISSET
 *
 * Returns a non-zero value if the bit for the file descriptor is set in 
 * the file descriptor set.
 */

/**
 * @def FD_ZERO
 *
 * Initializes the file descriptor set to have zero for all file descriptors.
 */

/** @} */

/**
 * @defgroup    NET_BUFFER      Network Packet Buffers
 * @brief The network packet buffer API 
 *
 * Network buffer is allocated to hold network packets.
 * The buffer pool(s) is allocated during system startup by calling netpool_mem_init().
 * @{
 */

/** @} */

/**
 * @defgroup    NET_HEAP      Network Heap Memory
 * @brief The heap memory allocation API 
 *
 * Heap memory (dynamic memory) used by the network modules.  Its size is determined 
 * during compile-time and defined by CHUNKSIZE.
 * To support network operations, this memory pool will never be re-initialized
 * amid changes of application modes.
 *
 * This memory is not suitable for the storage of downloaded XML/JPEG files from
 * servers/websites.  Use ext_mem_malloc() instead.
 *
 * @{
 */

/**
 * @def CHUNKSIZE
 *
 * Define the size of heap memory
 */

/** @} */

/**
 * @defgroup    NET_HTTP      HTTP Transfer
 * @brief The interface to transfer a URL.
 *
 * <b>CURL</b>, an open source project
 * (<a href="http://curl.haxx.se">
 * here</a>),
 * is ported to support transfer of a URL.  One of its libraries, <b>libcurl-easy</b>,
 * is used extensively.
 *
 * A high-level API (Net_Recv_Data()) calls libcurl-easy functions to 
 * accomplish the transfer.  Users can use this function without worrying 
 * about the details of libcurl-easy.
 *
 * XML/HTML or photo (JPEG) files on the Web can be large (over several MBytes
 * sometimes).  One of the system memory pools (<b>ext_mem_*</b>) is used for the storage
 * of these data.
 *
 * @{
 */

/**
 * @fn void *ext_mem_malloc(DWORD size)
 * @brief Allocates a memory block.
 * @param size Bytes to allocate
 * @retval A void pointer to the allocated space.
 * @retval A NULL if there is insufficient memory available.
 */

/**
 * @fn void ext_mem_free(void *rmem)
 * @brief Frees a memory block.
 * @param rmem Previously allocated memory block to be freed.
 * @retval None
 */

/** @} */

int errno;
typedef	long	fd_mask;
typedef	struct _types_fd_set {
	fd_mask	fds_bits[2];
} fd_set;

#  define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#  define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#  define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#  define	FD_ZERO(p)	(__extension__ (void)({ \
     size_t __i; \
     char *__tmp = (char *)p; \
     for (__i = 0; __i < sizeof (*(p)); ++__i) \
       *__tmp++ = 0; \
}))

void *ext_mem_malloc(DWORD size);
void ext_mem_free(void *rmem);

#endif /* NET_DOC_H */
