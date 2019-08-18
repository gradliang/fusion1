/**
 * \defgroup PPP PPP & AT Commands
 * @{
 *
 */


/**
 * \file
 * Point-to-Point Protocol (PPP) and AT commands implementation.  These are 
 * mainly used with GSM 2G/3G connections.
 *
 */

/** @} */

#define LOCAL_DEBUG_ENABLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global612.h"
#include "mpTrace.h"


#include <linux/types.h>
#if SMS_ENABLE

#include "net_tcp.h"
#include 	"devio.h"
#include "net_device.h"

#include "linux/list.h"

#include "wwan.h"


/**
 * @ingroup PPP
 * @brief check sms message received by modem
 *
 * @param index specific sms message's index
 *
 * @return 1 : OK, 0: FAIL
 */
int check_SMS(int index)
{	
	char buf[40];
	int i, tot;
	struct sms_message *msg;

	mpDebugPrint("@@@@@@@@@@@ check_SMS");
	
	if(!index)
		sprintf(buf, "%s", "at+cmgl=\"ALL\"\r");
	else
		sprintf(buf, "at+cmgr=%d\r", index);

	__edge_get_sms(buf, index);

			if(!index)
		mpDebugPrint("+CMGL: received messages=%d", sms_list.num_messages);

	tot = 0;
	mpx_SemaphoreWait(u08EDGESema_ID);
	while (!list_empty(&sms_list.list)) {

		msg = list_first_entry(&sms_list.list, struct sms_message, list);
		list_del(&msg->list);
		mpx_SemaphoreRelease(u08EDGESema_ID);

		if (!msg->content)
			smsdelete(msg->num);

					mpx_SemaphoreWait(u08EDGESema_ID);
#if 0
        list_add_tail(&msg->list, &SMS_queue);
#else
		mpx_Free(msg->content);
		mpx_Free(msg);
#endif
		tot++;
	}
	sms_list.num_messages = 0;
					mpx_SemaphoreRelease(u08EDGESema_ID);

	return (tot >> 1);		
}

#endif /* SMS_ENABLE */

// vim: :noexpandtab:
