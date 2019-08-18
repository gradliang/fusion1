#ifndef __TEVENT_H__
#define __TEVENT_H__

//#include <stdbool.h>
#include <sys/time.h>

#include "typedef.h"

struct tevent_context;
struct tevent_timer;
struct tevent_req;
struct tevent_fd;

typedef void (*tevent_fd_handler_t)(struct tevent_context *ev,
				    struct tevent_fd *fde,
				    void *private_data);
typedef void (*tevent_timer_handler_t)(struct tevent_context *ev,
				       struct tevent_timer *te,
				       struct timeval current_time,
				       void *private_data);

struct tevent_timer *_tevent_add_timer(struct tevent_context *ev,
				       struct timeval next_event,
				       tevent_timer_handler_t handler,
				       void *private_data,
				       const char *handler_name,
				       const char *location);

bool _tevent_req_error(struct tevent_req *req,
		       uint32_t error,
		       const char *location);
#define tevent_req_error(req, error) \
	_tevent_req_error(req, error, __location__)

bool _tevent_req_nomem(const void *p,
		       struct tevent_req *req,
		       const char *location);
#define tevent_req_nomem(p, req) \
	_tevent_req_nomem(p, req, __location__)
#define tevent_add_timer(ev, next_event, handler, private_data) \
	_tevent_add_timer(ev, next_event, handler, private_data, \
			  #handler, __location__)

int event_loop_once(struct tevent_context *ev);
#define tevent_loop_once(ev) \
	event_loop_once(ev)

/* bits for file descriptor event flags */
#define TEVENT_FD_READ 1
#define TEVENT_FD_WRITE 2

/**
 * An async request moves between the following 4 states:
 */
enum tevent_req_state {
	/**
	 * we are creating the request
	 */
	TEVENT_REQ_INIT,
	/**
	 * we are waiting the request to complete
	 */
	TEVENT_REQ_IN_PROGRESS,
	/**
	 * the request is finished
	 */
	TEVENT_REQ_DONE,
	/**
	 * A user error has occured
	 */
	TEVENT_REQ_USER_ERROR,
	/**
	 * Request timed out
	 */
	TEVENT_REQ_TIMED_OUT,
	/**
	 * No memory in between
	 */
	TEVENT_REQ_NO_MEMORY,
	/**
	 * the request is already received by the caller
	 */
	TEVENT_REQ_RECEIVED
};

#define event_context	tevent_context

typedef void (*tevent_req_fn)(struct tevent_req *);

struct tevent_req *_tevent_req_create(void *pstate,
				      size_t state_size,
				      const char *type,
				      const char *location);

#define tevent_req_create(_pstate, _type) \
	_tevent_req_create((_pstate), sizeof(_type), \
			   #_type, __location__)

bool tevent_req_set_endtime(struct tevent_req *req,
			    struct tevent_context *ev,
			    struct timeval endtime);

void tevent_req_done(struct tevent_req *req);

bool tevent_req_poll(struct tevent_req *req,
		     struct tevent_context *ev);

struct timeval tevent_timeval_current_ofs(uint32_t secs, uint32_t usecs);

struct timeval tevent_timeval_add(const struct timeval *tv, uint32_t secs,
				  uint32_t usecs);

struct timeval tevent_timeval_current_ofs(uint32_t secs, uint32_t usecs);

#endif
