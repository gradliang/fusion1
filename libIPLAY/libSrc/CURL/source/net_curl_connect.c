/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2007, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: connect.c,v 1.168 2007-03-26 23:23:46 yangtse Exp $
 ***************************************************************************/

#include "net_curl_setup.h"

#ifndef WIN32
/* headers for non-win32 */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> /* <netinet/tcp.h> may need it */
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h> /* for TCP_NODELAY */
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* required for free() prototype, without it, this crashes */
#endif              /* on macos 68K */

#if (defined(HAVE_FIONBIO) && defined(__NOVELL_LIBC__))
#include <sys/filio.h>
#endif
#if (defined(NETWARE) && defined(__NOVELL_LIBC__))
#undef in_addr_t
#define in_addr_t unsigned long
#endif
#ifdef VMS
#include <in.h>
#include <inet.h>
#endif

#endif  /* !WIN32 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "net_curl_urldata.h"
#include "net_curl_sendf.h"
//yuming, add
//#include "if2ip.h"
//yuming, add
//#include "strerror.h"
#include "net_curl_connect.h"
//yuming, add
//#include "memory.h"
#include "net_curl_select.h"
#include "net_curl_url.h" /* for Curl_safefree() */
#include "net_curl_multiif.h"
#include "net_sockaddr.h" /* required for Curl_sockaddr_storage */
//yuming, add
//#include "inet_ntop.h"
#include "net_curl_inet_pton.h"

/* The last #include file should be: */
#ifdef CURLDEBUG
#include "memdebug.h"
#endif

//yuming, add
#include "net_socket.h"

static curl_socket_t
singleipconnect(struct connectdata *conn,
                const Curl_addrinfo *ai, /* start connecting to this */
                long timeout_ms,
                bool *connected);

/*
 * waitconnect() waits for a TCP connect on the given socket for the specified
 * number of milliseconds. It returns:
 * 0    fine connect
 * -1   select() error
 * 1    select() timeout
 * 2    select() returned with an error condition fd_set
 */

#define WAITCONN_CONNECTED     0
#define WAITCONN_SELECT_ERROR -1
#define WAITCONN_TIMEOUT       1
#define WAITCONN_FDSET_ERROR   2

#ifdef NEW_NETSOCKET
#define HAVE_IOCTLSOCKET   1

/*
 * Curl_nonblock() set the given socket to either blocking or non-blocking
 * mode based on the 'nonblock' boolean argument. This function is highly
 * portable.
 */
int Curl_nonblock(curl_socket_t sockfd,    /* operate on this */
                  int nonblock   /* TRUE or FALSE */)
{
#undef SETBLOCK
#define SETBLOCK 0
#ifdef HAVE_O_NONBLOCK
  /* most recent unix versions */
  int flags;

  flags = fcntl(sockfd, F_GETFL, 0);
  if (FALSE != nonblock)
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
  else
    return fcntl(sockfd, F_SETFL, flags & (~O_NONBLOCK));
#undef SETBLOCK
#define SETBLOCK 1
#endif

#if defined(HAVE_FIONBIO) && (SETBLOCK == 0)
  /* older unix versions */
  int flags;

  flags = nonblock;
  return ioctl(sockfd, FIONBIO, &flags);
#undef SETBLOCK
#define SETBLOCK 2
#endif

#if defined(HAVE_IOCTLSOCKET) && (SETBLOCK == 0)
  /* Windows? */
  unsigned long flags;
  flags = nonblock;

  return ioctlsocket(sockfd, FIONBIO, &flags);
#undef SETBLOCK
#define SETBLOCK 3
#endif

#if defined(HAVE_IOCTLSOCKET_CASE) && (SETBLOCK == 0)
  /* presumably for Amiga */
  return IoctlSocket(sockfd, FIONBIO, (long)nonblock);
#undef SETBLOCK
#define SETBLOCK 4
#endif

#if defined(HAVE_SO_NONBLOCK) && (SETBLOCK == 0)
  /* BeOS */
  long b = nonblock ? 1 : 0;
  return setsockopt(sockfd, SOL_SOCKET, SO_NONBLOCK, &b, sizeof(b));
#undef SETBLOCK
#define SETBLOCK 5
#endif

#ifdef HAVE_DISABLED_NONBLOCKING
  return 0; /* returns success */
#undef SETBLOCK
#define SETBLOCK 6
#endif

#if (SETBLOCK == 0)
#error "no non-blocking method was found/used/set"
#endif
}
#endif

static
int waitconnect(curl_socket_t sockfd, /* socket */
                long timeout_msec)
{
  int rc;
#ifdef mpeix
  /* Call this function once now, and ignore the results. We do this to
     "clear" the error state on the socket so that we can later read it
     reliably. This is reported necessary on the MPE/iX operating system. */
  (void)verifyconnect(sockfd, NULL);
#endif

  /* now select() until we get connect or timeout */
  rc = Curl_socket_ready(CURL_SOCKET_BAD, sockfd, (int)timeout_msec);
  if(-1 == rc)
    /* error, no connect here, try next */
    return WAITCONN_SELECT_ERROR;

  else if(0 == rc)
    /* timeout, no connect today */
    return WAITCONN_TIMEOUT;

  if(rc & CSELECT_ERR)
    /* error condition caught */
    return WAITCONN_FDSET_ERROR;

  /* we have a connect! */
  return WAITCONN_CONNECTED;
}

#ifdef NEW_NETSOCKET
/*
 * verifyconnect() returns TRUE if the connect really has happened.
 */
static bool verifyconnect(curl_socket_t sockfd, int *error)
{
  bool rc = TRUE;
#ifdef SO_ERROR
  int err = 0;
  socklen_t errSize = sizeof(err);

#ifdef WIN32
  /*
   * In October 2003 we effectively nullified this function on Windows due to
   * problems with it using all CPU in multi-threaded cases.
   *
   * In May 2004, we bring it back to offer more info back on connect failures.
   * Gisle Vanem could reproduce the former problems with this function, but
   * could avoid them by adding this SleepEx() call below:
   *
   *    "I don't have Rational Quantify, but the hint from his post was
   *    ntdll::NtRemoveIoCompletion(). So I'd assume the SleepEx (or maybe
   *    just Sleep(0) would be enough?) would release whatever
   *    mutex/critical-section the ntdll call is waiting on.
   *
   *    Someone got to verify this on Win-NT 4.0, 2000."
   */

#ifdef _WIN32_WCE
  Sleep(0);
#else
  SleepEx(0, FALSE);
#endif

#endif

  if (0 != getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void *)&err, &errSize))
    err = SOCKERRNO;
#ifdef _WIN32_WCE
  /* Old WinCE versions don't support SO_ERROR */
  if (WSAENOPROTOOPT == err) {
    SET_SOCKERRNO(0);
    err = 0;
  }
#endif
#ifdef __minix
  /* Minix 3.1.x doesn't support getsockopt on UDP sockets */
  if (EBADIOCTL == err) {
    SET_SOCKERRNO(0);
    err = 0;
  }
#endif
  if ((0 == err) || (EISCONN == err))
    /* we are connected, awesome! */
    rc = TRUE;
  else
    /* This wasn't a successful connect */
    rc = FALSE;
  if (error)
    *error = err;
#else
  (void)sockfd;
  if (error)
    *error = SOCKERRNO;
#endif
  return rc;
}
#endif

CURLcode Curl_store_ip_addr(struct connectdata *conn)
{
  char addrbuf[256];
  Curl_printable_address(conn->ip_addr, addrbuf, sizeof(addrbuf));

  /* save the string */
  Curl_safefree(conn->ip_addr_str);
  conn->ip_addr_str = strdup(addrbuf);
  if(!conn->ip_addr_str)
    return CURLE_OUT_OF_MEMORY; /* FAIL */

#ifdef PF_INET6
  if(conn->ip_addr->ai_family == PF_INET6)
    conn->bits.ipv6 = TRUE;
#endif

  return CURLE_OK;
}

/* Used within the multi interface. Try next IP address, return TRUE if no
   more address exists */
static bool trynextip(struct connectdata *conn,
                      int sockindex,
                      bool *connected)
{
  curl_socket_t sockfd;
  Curl_addrinfo *ai;

  /* first close the failed socket */
#ifdef NEW_NETSOCKET
  closesocket(conn->sock[sockindex]);
#else
  mpx_SocketClose(conn->sock[sockindex]);
#endif
  conn->sock[sockindex] = CURL_SOCKET_BAD;
  *connected = FALSE;

  if(sockindex != FIRSTSOCKET)
    return TRUE; /* no next */

  /* try the next address */
  ai = conn->ip_addr->ai_next;

  while (ai) {
    sockfd = singleipconnect(conn, ai, 0L, connected);
    if(sockfd != CURL_SOCKET_BAD) {
      /* store the new socket descriptor */
      conn->sock[sockindex] = sockfd;
      conn->ip_addr = ai;

      Curl_store_ip_addr(conn);
      return FALSE;
    }
    ai = ai->ai_next;
  }
  return TRUE;
}

/*
 * Curl_is_connected() is used from the multi interface to check if the
 * firstsocket has connected.
 */

CURLcode Curl_is_connected(struct connectdata *conn,
                           int sockindex,
                           bool *connected)
{
  int rc;
  struct SessionHandle *data = conn->data;
  CURLcode code = CURLE_OK;
  curl_socket_t sockfd = conn->sock[sockindex];
  long allow = DEFAULT_CONNECT_TIMEOUT;
  long allow_total = 0;
  long has_passed;

  DEBUGASSERT(sockindex >= FIRSTSOCKET && sockindex <= SECONDARYSOCKET);

  *connected = FALSE; /* a very negative world view is best */

  /* Evaluate in milliseconds how much time that has passed */
  has_passed = Curl_tvdiff(Curl_tvnow(), data->progress.t_startsingle);

  /* subtract the most strict timeout of the ones */
  if(data->set.timeout && data->set.connecttimeout) {
    if (data->set.timeout < data->set.connecttimeout)
      allow_total = allow = data->set.timeout;
    else
      allow = data->set.connecttimeout;
  }
  else if(data->set.timeout) {
    allow_total = allow = data->set.timeout;
  }
  else if(data->set.connecttimeout) {
    allow = data->set.connecttimeout;
  }

  if(has_passed > allow ) {
    /* time-out, bail out, go home */
    failf(data, "Connection time-out after %ld ms", has_passed);
    return CURLE_OPERATION_TIMEOUTED;
  }
  if(conn->bits.tcpconnect) {
    /* we are connected already! */
    Curl_expire(data, allow_total);
    *connected = TRUE;
    return CURLE_OK;
  }

  Curl_expire(data, allow);

  /* check for connect without timeout as we want to return immediately */
  rc = waitconnect(sockfd, 0);

  if(WAITCONN_CONNECTED == rc) {
    int error;
#ifdef NEW_NETSOCKET
    if (verifyconnect(sockfd, &error)) {
      /* we are connected, awesome! */
      *connected = TRUE;
      return CURLE_OK;
    }
#endif
    /* nope, not connected for real */
    data->state.os_errno = error;
    infof(data, "Connection failed\n");
    if(trynextip(conn, sockindex, connected)) {
      code = CURLE_COULDNT_CONNECT;
    }
  }
  else if(WAITCONN_TIMEOUT != rc) {
    int error = 0;

    /* nope, not connected  */
    if (WAITCONN_FDSET_ERROR == rc) {
      //(void)verifyconnect(sockfd, &error);
      data->state.os_errno = error;
      //yuming, add
      //infof(data, "%s\n",Curl_strerror(conn,error));
    }
    else
      infof(data, "Connection failed\n");

    if(trynextip(conn, sockindex, connected)) {
      error = SOCKERRNO;
      data->state.os_errno = error;
      //yuming, add
      //failf(data, "Failed connect to %s:%d; %s",
      //      conn->host.name, conn->port, Curl_strerror(conn,error));
      code = CURLE_COULDNT_CONNECT;
    }
  }
  /*
   * If the connection failed here, we should attempt to connect to the "next
   * address" for the given host.
   */

  return code;
}

#ifdef SO_NOSIGPIPE
/* The preferred method on Mac OS X (10.2 and later) to prevent SIGPIPEs when
   sending data to a dead peer (instead of relying on the 4th argument to send
   being MSG_NOSIGNAL). Possibly also existing and in use on other BSD
   systems? */
static void nosigpipe(struct connectdata *conn,
                      curl_socket_t sockfd)
{
  struct SessionHandle *data= conn->data;
  int onoff = 1;
  if(setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&onoff,
                sizeof(onoff)) < 0)
    infof(data, "Could not set SO_NOSIGPIPE: %s\n",
          Curl_strerror(conn, SOCKERRNO));
}
#else
#define nosigpipe(x,y)
#endif

static CURLcode bindlocal(struct connectdata *conn,
                          curl_socket_t sockfd)
{
  struct SessionHandle *data = conn->data;
  const char *dev = data->set.device;
  struct sockaddr_in *addr;
  struct sockaddr_in me;
  int ret;

  addr = (struct sockaddr_in *)&me;
  memset(addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  addr->sin_port = 0;
  if (dev)
      addr->sin_addr.s_addr = htonl(NetInterfaceIpGet(dev));
  else
      addr->sin_addr.s_addr = htonl(NetDefaultIpGet());
  if (addr->sin_addr.s_addr == CURL_INADDR_NONE)
      return CURLE_INTERFACE_FAILED;

  ret = bind(sockfd, (struct sockaddr *)addr, sizeof(*addr));
  if (ret >= 0)
      return CURLE_OK;
  else
      return CURLE_INTERFACE_FAILED;
}

/* singleipconnect() connects to the given IP only, and it may return without
   having connected if used from the multi interface. */
static curl_socket_t
singleipconnect(struct connectdata *conn,
                const Curl_addrinfo *ai,
                long timeout_ms,
                bool *connected)
{
    char addr_buf[128];
    int rc;
    int error;
    bool isconnected;
    struct SessionHandle *data = conn->data;
    curl_socket_t sockfd;
#ifdef LINUX
    CURLcode res;
#else
    int res;
#endif
    ST_SOCK_ADDR localAddr, peerAddr;
    struct sockaddr_in *addr;
    struct sockaddr_in me;

    CURLPRINTF("singleipconnect");
    
#ifdef NEW_NETSOCKET
    sockfd = socket(ai->ai_family, conn->socktype, IPPROTO_TCP);
#else
    sockfd = mpx_Socket(ai->ai_family, conn->socktype);
#endif
    if (sockfd == CURL_SOCKET_BAD)
        return CURL_SOCKET_BAD;

    *connected = FALSE; /* default is not connected */

    Curl_printable_address(ai, addr_buf, sizeof(addr_buf));
    infof(data, "  Trying %s... ", addr_buf);

    //yuming, add
#if 0
    if(data->set.tcp_nodelay)
        tcpnodelay(conn, sockfd);
#endif

    nosigpipe(conn, sockfd);

    //yuming, add
#if 0
    if(data->set.fsockopt) {
        /* activate callback for setting socket options */
        error = data->set.fsockopt(data->set.sockopt_client, sockfd, CURLSOCKTYPE_IPCXN);
        if (error) {
            sclose(sockfd); /* close the socket and bail out */
            return CURL_SOCKET_BAD;
        }
    }
#endif

    /* possibly bind the local end to an IP, interface or port */
#ifdef NEW_NETSOCKET
#if 0
    addr = (struct sockaddr_in *)&me;
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = 0;
    addr->sin_addr.s_addr = htonl(NetDefaultIpGet());
    res = bind(sockfd, (struct sockaddr *)addr, sizeof(*addr));
#else
    res = bindlocal(conn, sockfd);
#endif
#else
    mpx_SockAddrSet(&localAddr, NetDefaultIpGet(), mpx_NewLocalPort());
    res = mpx_Bind(sockfd, &localAddr);
#endif
    if(res) {
        closesocket(sockfd); /* close socket and bail out */
        return CURL_SOCKET_BAD;
    }

#ifdef NEW_NETSOCKET
  /* set socket non-blocking */
  Curl_nonblock(sockfd, TRUE);
#endif

#ifndef NEW_NETSOCKET
    addr = (struct sockaddr_in *)ai->ai_addr;
    peerAddr.u16Family = addr->sin_family;
    peerAddr.u16Port = addr->sin_port;
    memcpy(peerAddr.u08Address, (char *)&(addr->sin_addr), sizeof(struct in_addr));
    CURLPRINTF("peerAddr.u16Family %d", peerAddr.u16Family);
    CURLPRINTF("peerAddr.u16Port %d", peerAddr.u16Port);
#endif
    
    /* Connect TCP sockets, bind UDP */
    if(conn->socktype == SOCK_STREAM)
#ifdef NEW_NETSOCKET
        rc = connect(sockfd, ai->ai_addr, ai->ai_addrlen);
#else
        rc = mpx_Connect(sockfd, &peerAddr);
#endif
    else
        rc = 0;


    //CURLPRINTF("rc =  %d", rc);
#ifdef NEW_NETSOCKET
  if(-1 == rc) {
    error = SOCKERRNO;

    switch (error) {
    case EINPROGRESS:
    case EWOULDBLOCK:
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
      /* On some platforms EAGAIN and EWOULDBLOCK are the
       * same value, and on others they are different, hence
       * the odd #if
       */
    case EAGAIN:
#endif
      rc = waitconnect(sockfd, timeout_ms);
      break;
    default:
      /* unknown error, fallthrough and try another address! */
//      failf(data, "Failed to connect to %s: %s",
//            addr_buf, Curl_strerror(conn,error));
      data->state.os_errno = error;
      break;
    }
  }
#endif

#ifdef NEW_NETSOCKET
  /* The 'WAITCONN_TIMEOUT == rc' comes from the waitconnect(), and not from
     connect(). We can be sure of this since connect() cannot return 1. */
  if((WAITCONN_TIMEOUT == rc) &&
     (data->state.used_interface == Curl_if_multi)) {
    /* Timeout when running the multi interface */
    return sockfd;
  }

  isconnected = verifyconnect(sockfd, &error);
#endif
  if(!rc && isconnected) {
    /* we are connected, awesome! */
    *connected = TRUE; /* this is a true connect */
#ifdef NEW_NETSOCKET
    if (!Conn(sockfd))
        CURLPRINTF("panic socketid %d not connected", sockfd);
#endif
    CURLPRINTF("connected");
    infof(data, "connected\n");
    return sockfd;
  }
  else if(WAITCONN_TIMEOUT == rc)
    infof(data, "Timeout\n");
  else {
    data->state.os_errno = error;
    //yuming, add
    //infof(data, "%s\n", Curl_strerror(conn, error));
  }

  /* connect failed or timed out */
#ifdef NEW_NETSOCKET
  closesocket(sockfd);
#else
  mpx_SocketClose(sockfd);
#endif
  
  return CURL_SOCKET_BAD;
}

/*
 * TCP connect to the given host with timeout, proxy or remote doesn't matter.
 * There might be more than one IP address to try out. Fill in the passed
 * pointer with the connected socket.
 */

CURLcode Curl_connecthost(struct connectdata *conn,  /* context */
                          const struct Curl_dns_entry *remotehost, /* use this one */
                          curl_socket_t *sockconn,   /* the connected socket */
                          Curl_addrinfo **addr,      /* the one we used */
                          bool *connected)           /* really connected? */
{
    struct SessionHandle *data = conn->data;
    curl_socket_t sockfd = CURL_SOCKET_BAD;
    int aliasindex;
    int num_addr;
    Curl_addrinfo *ai;
    Curl_addrinfo *curr_addr;

    struct timeval after;
    struct timeval before = Curl_tvnow();

    /*************************************************************
    * Figure out what maximum time we have left
    *************************************************************/
    long timeout_ms= DEFAULT_CONNECT_TIMEOUT;
    long timeout_per_addr;

    CURLPRINTF("Curl_connecthost");

    *connected = FALSE; /* default to not connected */

    if(data->set.timeout || data->set.connecttimeout) {
        long has_passed;

        /* Evaluate in milliseconds how much time that has passed */
        has_passed = Curl_tvdiff(Curl_tvnow(), data->progress.t_startsingle);

#ifndef min
#define min(a, b)   ((a) < (b) ? (a) : (b))
#endif

        /* get the most strict timeout of the ones converted to milliseconds */
        if(data->set.timeout && data->set.connecttimeout) {
            if (data->set.timeout < data->set.connecttimeout)
                timeout_ms = data->set.timeout;
            else
                timeout_ms = data->set.connecttimeout;
        }
        else if(data->set.timeout)
            timeout_ms = data->set.timeout;
        else
            timeout_ms = data->set.connecttimeout;

        /* subtract the passed time */
        timeout_ms -= has_passed;

        if(timeout_ms < 0) {
            /* a precaution, no need to continue if time already is up */
            failf(data, "Connection time-out");
            return CURLE_OPERATION_TIMEOUTED;
        }
    }
    Curl_expire(data, timeout_ms);

    /* Max time for each address */
    num_addr = Curl_num_addresses(remotehost->addr);
    CURLPRINTF("num_addr in Curl_addrinfo %d", num_addr);
    timeout_per_addr = timeout_ms / num_addr;

    ai = remotehost->addr;

    /* Below is the loop that attempts to connect to all IP-addresses we
    * know for the given host. One by one until one IP succeeds.
    */

    if(data->state.used_interface == Curl_if_multi)
        /* don't hang when doing multi */
        timeout_per_addr = 0;

    /*
    * Connecting with a Curl_addrinfo chain
    */
    for (curr_addr = ai, aliasindex=0; curr_addr;
        curr_addr = curr_addr->ai_next, aliasindex++) {

        /* start connecting to the IP curr_addr points to */
        sockfd = singleipconnect(conn, curr_addr, timeout_per_addr, connected);

        if(sockfd != CURL_SOCKET_BAD)
            break;

        /* get a new timeout for next attempt */
        after = Curl_tvnow();
        timeout_ms -= Curl_tvdiff(after, before);
        if(timeout_ms < 0) {
            failf(data, "connect() timed out!");
            return CURLE_OPERATION_TIMEOUTED;
        }
        before = after;
    }  /* end of connect-to-each-address loop */

    if (sockfd == CURL_SOCKET_BAD) {
        /* no good connect was made */
        *sockconn = CURL_SOCKET_BAD;
        failf(data, "couldn't connect to host");
        return CURLE_COULDNT_CONNECT;
    }

    /* leave the socket in non-blocking mode */

    /* store the address we use */
    if(addr)
        *addr = curr_addr;

    /* allow NULL-pointers to get passed in */
    if(sockconn)
        *sockconn = sockfd;    /* the socket descriptor we've connected */

    data->info.numconnects++; /* to track the number of connections made */

    return CURLE_OK;
}
