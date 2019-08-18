
#define LOCAL_DEBUG_ENABLE 0

#include <linux/types.h>
#include <tevent.h>
#include <tevent_internal.h>
#include <tevent_util.h>

/* free memory if the pointer is valid and zero the pointer */
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) {mpx_Free(x); (x)=NULL;} } while(0)
#endif

int tevent_common_context_destructor(struct tevent_context *ev)
{
	struct tevent_fd *fd, *fn;
	struct tevent_timer *te, *tn;

	for (fd = ev->fd_events; fd; fd = fn) {
		fn = fd->next;
		fd->event_ctx = NULL;
		DLIST_REMOVE(ev->fd_events, fd);
        SAFE_FREE(fd->private_data);
        SAFE_FREE(fd);
	}

	for (te = ev->timer_events; te; te = tn) {
		tn = te->next;
		te->event_ctx = NULL;
		DLIST_REMOVE(ev->timer_events, te);
        SAFE_FREE(te->private_data);
        SAFE_FREE(te);
	}

	return 0;
}

/*
  do a single event loop using the events defined in ev 
*/
int event_loop_once(struct tevent_context *ev)
{
	int ret;

	MP_DEBUG("--> %s", __func__);
//    mpDebugPrint("%s: %p", __location__, ev->ops->loop_once);
	ret = ev->ops->loop_once(ev);


done:
	return ret;
}


struct tevent_fd *tevent_add_fd(struct tevent_context *ev,
					   uint16_t flags,
				       tevent_fd_handler_t handler,
				       void *private_data)
{
	struct tevent_fd *fde;

	fde = mpx_Zalloc(sizeof(struct tevent_fd));
	if (!fde) return NULL;

	fde->event_ctx		= ev;
	fde->flags			= flags;
	fde->handler		= handler;
	fde->private_data	= private_data;

	DLIST_ADD(ev->fd_events, fde);

//	talloc_set_destructor(fde, tevent_common_fd_destructor);
	return fde;
}

