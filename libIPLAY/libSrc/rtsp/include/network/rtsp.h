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

#ifndef FN_RTSP_H
#define FN_RTSP_H

#include <time.h>
#include <config.h>
#include <netinet/in.h>

#include <glib.h>
#include <ev.h>

#include "rfc822proto.h"

struct Resource;
struct cfg_socket_t;
struct cfg_vhost_t;

/**
 * @addtogroup RTSP
 * @{
 */

#define RTSP_RESERVED 4096
#ifdef LINUX
#define RTSP_BUFFERSIZE (65536 + RTSP_RESERVED)
#else
#define RTSP_BUFFERSIZE (65536/4 + RTSP_RESERVED)
#endif

/**
 * @brief RTSP server states
 *
 * These are the constants used to define the states that an RTSP session can
 * have.
 *
 * The server state machine for RTSP is defined in RFC2326 Appendix A,
 * Subsection 2.
 */
typedef enum {
    RTSP_SERVER_INIT,
    RTSP_SERVER_READY,
    RTSP_SERVER_PLAYING,
    RTSP_SERVER_RECORDING
} RTSP_Server_State;

#define RTSP_EL "\r\n"

typedef struct RTSP_session {
    char *session_id;
    RTSP_Server_State cur_state;
    int started;
    GSList *rtp_sessions; // Of type RTP_session
    // mediathread resource
    struct Resource *resource;
    char *resource_uri;

    /**
     * @brief List of playback requests (of type @ref RTSP_Range)
     *
     * RFC 2326 Section 10.5 defines queues of PLAY requests, which
     * allows precise editing by the client; we keep a list here to
     * make it easier to actually implement the (currently missing)
     * feature.
     */
    GQueue *play_requests;
} RTSP_session;

/**
 * @brief Structure representing a playing range
 *
 * This structure contains the software-accessible range data as
 * provided by the client with the Range header (specified by RFC 2326
 * Section 12.29).
 *
 * This structure is usually filled in by the Ragel parser in @ref
 * ragel_parser_range_header, but is also changed when PAUSE requests
 * are received.
 */
typedef struct RTSP_Range {
    /** Seconds into the stream (NTP) to start the playback at */
    double begin_time;

    /** Seconds into the stream (NTP) to stop the playback at */
    double end_time;

    /** Real-time timestamp when to start the playback */
    double playback_time;
} RTSP_Range;

struct RTSP_Client;
struct HTTP_Tunnel_Pair;

typedef void (*rtsp_write_data)(struct RTSP_Client *client, GByteArray *data);

typedef struct RTSP_Client {
    /**
     * @brief Socket descriptor for the main connection of the client
     *
     * In case of HTTP tunnelling, the "HTTP" client object will hold
     * here the output descriptor, while the "RTSP" client object will
     * hold the input descriptor.
     */
    int sd;

    enum { RTSP_TCP, RTSP_SCTP } socktype;

    /**
     * @brief First channel free for interleaved and SCTP
     */
    int first_free_channel;

    /**
     * @brief Status of the connected client for the parser
     */
    RFC822_Parser_State status;

    /**
     * @brief Input buffer
     *
     * This is the input buffer as read straight from the sock socket;
     * GByteArray allows for automatic sizing of the array.
     */
    GByteArray *input;

    /**
     * @brief Current request being parsed
     *
     * @note The pointer is only valid when RTSP_Client::status is
     *       either Parser_Headers or Parser_Content.
     */
    RFC822_Request *pending_request;

    GQueue *out_queue;

    /**
     * @brief Hash table for interleaved and SCTP channels
     */
    GHashTable *channels;

    // Run-Time
    RTSP_session *session;
    guint64 last_cseq;

    rtsp_write_data write_data;

    struct HTTP_Tunnel_Pair *pair;

    //Events
    struct ev_loop *loop;

    ev_timer ev_timeout;

    ev_io ev_io_write;

    struct cfg_vhost_t *vhost;

    /**
     * @brief Local host bound to the socket
     *
     * This is not the same host as provided in @ref
     * cfg_socket_t::listen_on as that is the address bound by bind()
     * while this is the one used to reach the client itself.
     */
    char *local_host;

    char *remote_host;

    socklen_t sa_len;
    struct sockaddr *local_sa;
    struct sockaddr *peer_sa;
#ifdef HAVE_JSON //stats
    char *user_agent;
    size_t bytes_read;
    size_t bytes_sent;
#endif
} RTSP_Client;

typedef struct HTTP_Tunnel_Pair {
    RTSP_Client *rtsp_client;
    RTSP_Client *http_client;
    gint base64_state;
    guint base64_save;
} HTTP_Tunnel_Pair;

void rtsp_write_string(RTSP_Client *client, GString *str);

void rtsp_client_incoming_cb(struct ev_loop *loop, ev_io *w, int revents);

void RTSP_handler(RTSP_Client * rtsp);
gboolean rtsp_process_complete(RTSP_Client *rtsp);

/**
 * RTSP high level functions, mapping to the actual RTSP methods
 *
 * @defgroup rtsp_methods Method functions
 *
 * @{
 */

typedef void (*rtsp_method_function)(RTSP_Client * rtsp, RFC822_Request *req);

void RTSP_describe(RTSP_Client * rtsp, RFC822_Request *req);
void RTSP_setup(RTSP_Client * rtsp, RFC822_Request *req);
void RTSP_play(RTSP_Client * rtsp, RFC822_Request *req);
void RTSP_pause(RTSP_Client * rtsp, RFC822_Request *req);
void RTSP_teardown(RTSP_Client * rtsp, RFC822_Request *req);
void RTSP_options(RTSP_Client * rtsp, RFC822_Request *req);
/**
 * @}
 */

static inline void rtsp_quick_response(struct RTSP_Client *client, RFC822_Request *req, int code)
{
    /* We want to make sure we respond with an RTSP protocol every
       time if we called this function in particular. */
    RFC822_Protocol proto;
    switch ( req->proto ) {
    case RFC822_Protocol_RTSP10:
        proto = req->proto;
        break;
    default:
        proto = RFC822_Protocol_RTSP10;
        break;
    }

    rfc822_quick_response(client, req, proto, code);
}

gboolean rtsp_check_invalid_state(RTSP_Client *client,
                                  const RFC822_Request *req,
                                  RTSP_Server_State invalid_state);

gboolean rtsp_connection_limit(RTSP_Client *rtsp, RFC822_Request *req);

#ifdef ENABLE_SCTP
void rtsp_sctp_send_rtsp(RTSP_Client *client, GByteArray *data);
void rtsp_sctp_read_cb(struct ev_loop *, ev_io *, int);
#endif

void rtsp_tcp_read_cb(struct ev_loop *, ev_io *, int);
void rtsp_write_data_queue(RTSP_Client *client, GByteArray *data);
void rtsp_tcp_write_cb(struct ev_loop *, ev_io *, int);

void rtsp_interleaved_receive(RTSP_Client *rtsp, int channel, uint8_t *data, size_t len);

RTSP_session *rtsp_session_new(RTSP_Client *rtsp);
void rtsp_session_free(RTSP_session *session);
void rtsp_session_editlist_append(RTSP_session *session, RTSP_Range *range);
void rtsp_session_editlist_free(RTSP_session *session);

void rtsp_do_pause(RTSP_Client *rtsp);

/**
 * @defgroup ragel Ragel parsing
 *
 * @brief Functions and data structure for parsing of RTSP protocol.
 *
 * This group enlists all the shared data structure between the
 * parsers written using Ragel (http://www.complang.org/ragel/) and
 * the users of those parsers, usually the method handlers (see @ref
 * rtsp_methods).
 *
 * @{
 */

/**
 * @defgroup ragel_transport Transport: header parsing
 *
 * @{
 */

GSList *ragel_parse_transport_header(const char *header);
/**
 *@}
 */

/**
 * @defgroup ragel_range Range: header parsing
 *
 * @{
 */

struct RTSP_Range;

gboolean ragel_parse_range_header(const char *header,
                                  RTSP_Range *range);

/**
 *@}
 */

size_t ragel_parse_request_line(const char *msg, const size_t length, RFC822_Request *req);

int ragel_read_rtsp_headers(GHashTable *headers, const char *msg,
                            size_t length, size_t *read_size);
int ragel_read_http_headers(GHashTable *headers, const char *msg,
                            size_t length, size_t *read_size);
/**
 *@}
 */

gboolean HTTP_handle_headers(RTSP_Client *rtsp);
gboolean HTTP_handle_content(RTSP_Client *rtsp);
gboolean HTTP_handle_idle(RTSP_Client *rtsp);
void http_tunnel_initialise();

#ifdef HAVE_JSON
#error dod1
void stats_account_read(RTSP_Client *rtsp, size_t bytes);
void stats_account_sent(RTSP_Client *rtsp, size_t bytes);
void feng_send_statistics(RTSP_Client *rtsp);
#else
#define stats_account_read(a, b)
#define stats_account_sent(a, b)
#endif
/**
 * @}
 */

/**
 * @defgroup clients Clients-handling functions
 *
 * @{
 */
void clients_init();
void clients_cleanup();
void clients_each(GFunc func, gpointer user_data);
/**
 * @}
 */

#endif // FN_RTSP_H
