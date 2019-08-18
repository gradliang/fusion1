
#define LOCAL_DEBUG_ENABLE 0

#include <linux/types.h>
#include "tevent.h"
#include "async_req.h"

void send_event(struct event_context *ev);

struct async_req *async_req_new(void)
{
	return mpx_Zalloc(sizeof(struct async_req));
}

void *dod1, *dod2;
static void async_req_finish(struct async_req *req, enum async_req_state state)
{
	req->state = state;
//	BREAK_POINT();
    MP_ASSERT(req);
    dod1 = req;
    dod2 = req->async.fn;
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

void async_req_done(struct async_req *req)
{
	struct event_context *ev;
	async_req_finish(req, ASYNC_REQ_DONE);
}

bool async_req_is_error(struct async_req *req, enum async_req_state *state,
			int *error)
{
	MP_DEBUG("--> %s", __func__);
	if (req->state == ASYNC_REQ_DONE) {
		return false;
	}
	if (req->state == ASYNC_REQ_USER_ERROR) {
		*error = req->error;
	}
	*state = req->state;
	mpDebugPrint("[REQ] returns isError");
	return true;
}

bool _async_req_setup(struct async_req **preq,
		      void *pstate, size_t state_size)
{
	struct async_req *req;
	void **ppstate = (void **)pstate;
	void *state;

	req = async_req_new();
	if (req == NULL) {
		return false;
	}
    if (preq)
    {
		*preq = req;
    }

    if (ppstate)
    {
		*ppstate = req;
    }

	return true;
}

void async_req_timeout(struct async_req *req)
{
	async_req_finish(req, ASYNC_REQ_TIMED_OUT);
}

void async_req_cancel(struct async_req *req)
{
	async_req_finish(req, ASYNC_REQ_USER_ERROR);
}
