
#ifndef __NET_SMBCLIENT_
#define __NET_SMBCLIENT_

# define DEPRECATED_SMBC_INTERFACE

#define SMBCLIENT_BROWSE		0

#define SMBC_WORKGROUP      1
#define SMBC_SERVER         2
#define SMBC_FILE_SHARE     3
#define SMBC_PRINTER_SHARE  4
#define SMBC_COMMS_SHARE    5
#define SMBC_IPC_SHARE      6
#define SMBC_DIR            7
#define SMBC_FILE           8
#define SMBC_LINK           9

#include "lib/tevent/tevent.h"
#include "lib/tevent/tevent_internal.h"
struct mevent_context {
	/* the specific events implementation */
	const struct tevent_ops *ops;

	/* list of fd events - used by common code */
	struct tevent_fd *fd_events;

	/* list of timed events - used by common code */
	struct tevent_timer *timer_events;

	/* list of immediate events - used by common code */
	struct tevent_immediate *immediate_events;

	/* list of signal events - used by common code */
	struct tevent_signal *signal_events;

	/* this is private for the events_ops implementation */
	void *additional_data;

	/* pipe hack used with signal handlers */
	struct tevent_fd *pipe_fde;

	/* debugging operations */
	struct tevent_debug_ops debug_ops;

	/* info about the nesting status */
	struct {
		bool allowed;
		uint32_t level;
		tevent_nesting_hook hook_fn;
		void *hook_private;
	} nesting;
};

#endif  /* __NET_SMBCLIENT_ */
