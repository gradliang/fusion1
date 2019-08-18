
#include <linux/types.h>
#include <tevent.h>
#include <tevent_internal.h>
#include <tevent_util.h>

/**
  compare two timeval structures. 
  Return -1 if tv1 < tv2
  Return 0 if tv1 == tv2
  Return 1 if tv1 > tv2
*/
int tevent_timeval_compare(const struct timeval *tv1, const struct timeval *tv2)
{
	if (tv1->tv_sec  > tv2->tv_sec)  return 1;
	if (tv1->tv_sec  < tv2->tv_sec)  return -1;
	if (tv1->tv_usec > tv2->tv_usec) return 1;
	if (tv1->tv_usec < tv2->tv_usec) return -1;
	return 0;
}

/*
  destroy a timed event
*/
int tevent_common_timed_destructor(struct tevent_timer *te)
{
	if (te->event_ctx) {
		DLIST_REMOVE(te->event_ctx->timer_events, te);
	}

	return 0;
}

/*
  add a timed event
  return NULL on failure (memory allocation error)
*/
struct tevent_timer *_tevent_add_timer(struct tevent_context *ev, 
					     struct timeval next_event,
					     tevent_timer_handler_t handler,
					     void *private_data,
					     const char *handler_name,
					     const char *location)
{
	struct tevent_timer *te, *last_te, *cur_te;

	te = mpx_Zalloc(sizeof (struct tevent_timer));
	if (te == NULL) return NULL;

	te->event_ctx		= ev;
	te->next_event		= next_event;
	te->handler		= handler;
	te->private_data	= private_data;
	te->handler_name	= handler_name;

	/* keep the list ordered */
	last_te = NULL;
	for (cur_te = ev->timer_events; cur_te; cur_te = cur_te->next) {
		/* if the new event comes before the current one break */
		if (tevent_timeval_compare(&te->next_event, &cur_te->next_event) < 0) {
			break;
		}

		last_te = cur_te;
	}

	DLIST_ADD_AFTER(ev->timer_events, te, last_te);

//	talloc_set_destructor(te, tevent_common_timed_destructor);

	return te;
}

/*
  add a timed event
  return NULL on failure (memory allocation error)
*/
struct tevent_timer *tevent_common_add_timer(struct tevent_context *ev,
					     struct timeval next_event,
					     tevent_timer_handler_t handler,
					     void *private_data,
					     const char *handler_name,
					     const char *location)
{
	struct tevent_timer *te, *last_te, *cur_te;

	te = mpx_Zalloc(sizeof(struct tevent_timer));
	if (te == NULL) return NULL;

	te->event_ctx		= ev;
	te->next_event		= next_event;
	te->handler		= handler;
	te->private_data	= private_data;
	te->handler_name	= handler_name;

	/* keep the list ordered */
	last_te = NULL;
	for (cur_te = ev->timer_events; cur_te; cur_te = cur_te->next) {
		/* if the new event comes before the current one break */
		if (tevent_timeval_compare(&te->next_event, &cur_te->next_event) < 0) {
			break;
		}

		last_te = cur_te;
	}

	DLIST_ADD_AFTER(ev->timer_events, te, last_te);

//	talloc_set_destructor(te, tevent_common_timed_destructor);

	return te;
}

