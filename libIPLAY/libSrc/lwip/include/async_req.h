#ifndef __ASYNC_REQ_H__
#define __ASYNC_REQ_H__

#include "typedef.h"
#include "linux/list.h"

enum async_req_state {
	/**
	 * we are creating the request
	 */
	ASYNC_REQ_INIT,
	/**
	 * we are waiting the request to complete
	 */
	ASYNC_REQ_IN_PROGRESS,
	/**
	 * the request is finished
	 */
	ASYNC_REQ_DONE,
	/**
	 * A user error has occured
	 */
	ASYNC_REQ_USER_ERROR,
	/**
	 * Request timed out
	 */
	ASYNC_REQ_TIMED_OUT,
	/**
	 * No memory in between
	 */
	ASYNC_REQ_NO_MEMORY
};

struct async_req;

typedef void *(*async_req_fn)(struct async_req *, char *);

struct async_req {
	struct list_head list;

	enum async_req_state state;

	void *private_data;

	async_req_fn handler;

	char *buf;
	size_t size;

	struct event_context *ev;

	uint32_t error;

	/**
	 * @brief What to do on completion
	 *
	 * This is used for the user of an async request, fn is called when
	 * the request completes, either successfully or with an error.
	 */
	struct {
		/**
		 * @brief Completion function
		 * Completion function, to be filled by the API user
		 */
		void (*fn)(struct async_req *);
		/**
		 * @brief Private data for the completion function
		 */
		void *priv;
	} async;

	int result_code;
	int cme_error;
	char *resp;
};

bool _async_req_setup(struct async_req **preq,
		      void *pstate, size_t state_size);

#define async_req_setup(_preq, _pstate, type) \
	_async_req_setup((_preq), (_pstate), sizeof(type))

#endif
