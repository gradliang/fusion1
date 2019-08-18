
#include <linux/types.h>
#include <tevent.h>
#include <tevent_internal.h>
#include "ndebug.h"

/* free memory if the pointer is valid and zero the pointer */
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) {mpx_Free(x); (x)=NULL;} } while(0)
#endif
struct tevent_req *_tevent_req_create(void *pdata,
				    size_t data_size,
				    const char *type,
				    const char *location)
{
	struct tevent_req *req;
	void **ppdata = (void **)pdata;
	void *data;

	req = mpx_Zalloc(sizeof(struct tevent_req));
	if (req == NULL) {
		return NULL;
	}
	req->internal.state		= TEVENT_REQ_IN_PROGRESS;

	return req;
}

static void tevent_req_finish(struct tevent_req *req,
			      enum tevent_req_state state)
{
	req->internal.state = state;
	if (req->async.fn != NULL) {
		req->async.fn(req);
	}
}

/**
 * @brief An async request has successfully finished
 * @param[in] req	The finished request
 *
 * async_req_done is to be used by implementors of async requests. When a
 * request is successfully finished, this function calls the user's completion
 * function.
 */

void tevent_req_done(struct tevent_req *req)
{
	tevent_req_finish(req, TEVENT_REQ_DONE);
}

bool tevent_req_poll(struct tevent_req *req,
		     struct tevent_context *ev)
{
	while (tevent_req_is_in_progress(req)) {
		int ret;

		ret = tevent_loop_once(ev);
		if (ret != 0) {
			BREAK_POINT();
			return false;
		}
	}

	return true;
}

bool _tevent_req_error(struct tevent_req *req,
		       uint32_t error,
		       const char *location)
{
	if (error == 0) {
		return false;
	}

	req->internal.error = error;
	tevent_req_finish(req, TEVENT_REQ_USER_ERROR);
	return true;
}

bool _tevent_req_nomem(const void *p,
		       struct tevent_req *req,
		       const char *location)
{
	if (p != NULL) {
		return false;
	}
	tevent_req_finish(req, TEVENT_REQ_NO_MEMORY);
	return true;
}

static void tevent_req_timedout(struct tevent_context *ev,
			       struct tevent_timer *te,
			       struct timeval now,
			       void *private_data)
{
	struct tevent_req *req = private_data;

    tevent_common_timed_destructor(te);
	SAFE_FREE(req->internal.timer);

	tevent_req_finish(req, TEVENT_REQ_TIMED_OUT);
	SAFE_FREE(req);
}

bool tevent_req_set_endtime(struct tevent_req *req,
			    struct tevent_context *ev,
			    struct timeval endtime)
{
	SAFE_FREE(req->internal.timer);

	req->internal.timer = tevent_add_timer(ev, endtime,
					       tevent_req_timedout,
					       req);
	if (tevent_req_nomem(req->internal.timer, req)) {
		return false;
	}

	return true;
}

void tevent_req_set_callback(struct tevent_req *req, tevent_req_fn fn, void *pvt)
{
	req->async.fn = fn;
	req->async.private_data = pvt;
}

bool tevent_req_is_in_progress(struct tevent_req *req)
{
	if (req->internal.state == TEVENT_REQ_IN_PROGRESS) {
		return true;
	}

	return false;
}

