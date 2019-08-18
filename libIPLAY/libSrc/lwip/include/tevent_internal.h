
#include "typedef.h"

struct tevent_req {
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
		tevent_req_fn fn;
		/**
		 * @brief Private data for the completion function
		 */
		void *private_data;
	} async;

	/**
	 * @brief Private state pointer for the actual implementation
	 *
	 * The implementation doing the work for the async request needs to
	 * keep around current data like for example a fd event. The user of
	 * an async request should not touch this.
	 */
	void *data;

	/**
	 * @brief Internal state of the request
	 *
	 * Callers should only access this via functions and never directly.
	 */
	struct {
		/**
		 * @brief The talloc type of the data pointer
		 *
		 * This is filled by the tevent_req_create() macro.
		 *
		 * This for debugging only.
		 */
		const char *private_type;

		/**
		 * @brief The location where the request was created
		 *
		 * This uses the __location__ macro via the tevent_req_create()
		 * macro.
		 *
		 * This for debugging only.
		 */
		const char *create_location;

		/**
		 * @brief The location where the request was finished
		 *
		 * This uses the __location__ macro via the tevent_req_done(),
		 * tevent_req_error() or tevent_req_nomem() macro.
		 *
		 * This for debugging only.
		 */
		const char *finish_location;

		/**
		 * @brief The external state - will be queried by the caller
		 *
		 * While the async request is being processed, state will remain in
		 * TEVENT_REQ_IN_PROGRESS. A request is finished if
		 * req->state>=TEVENT_REQ_DONE.
		 */
		enum tevent_req_state state;

		/**
		 * @brief status code when finished
		 *
		 * This status can be queried in the async completion function. It
		 * will be set to 0 when everything went fine.
		 */
		uint64_t error;

		/**
		 * @brief the immediate event used by tevent_req_post
		 *
		 */
		struct tevent_immediate *trigger;

		/**
		 * @brief the timer event if tevent_req_set_timeout was used
		 *
		 */
		struct tevent_timer *timer;
	} internal;
};
struct tevent_ops {
	/* conntext init */
	int (*context_init)(struct tevent_context *ev);

	/* timed_event functions */
	struct tevent_timer *(*add_timer)(struct tevent_context *ev,
					  struct timeval next_event,
					  tevent_timer_handler_t handler,
					  void *private_data,
					  const char *handler_name);

	/* loop functions */
	int (*loop_once)(struct tevent_context *ev);
	int (*loop_wait)(struct tevent_context *ev);
};

struct tevent_fd {
	struct tevent_fd *prev, *next;
	struct tevent_context *event_ctx;
	int fd;
	uint16_t flags; /* see TEVENT_FD_* flags */
	tevent_fd_handler_t handler;
	/* this is private for the specific handler */
	void *private_data;
};
struct tevent_timer {
	struct tevent_timer *prev, *next;
	struct tevent_context *event_ctx;
	struct timeval next_event;
	tevent_timer_handler_t handler;
	/* this is private for the specific handler */
	void *private_data;
	/* this is for debugging only! */
	const char *handler_name;
};

struct tevent_context {
	/* the specific events implementation */
	const struct tevent_ops *ops;

	/* list of fd events - used by common code */
	struct tevent_fd *fd_events;

	/* list of timed events - used by common code */
	struct tevent_timer *timer_events;

	void *private_data;

	int event;
};

struct tevent_timer *tevent_common_add_timer(struct tevent_context *ev,
					     struct timeval next_event,
					     tevent_timer_handler_t handler,
					     void *private_data,
					     const char *handler_name,
					     const char *location);
