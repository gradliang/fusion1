#define LOCAL_DEBUG_ENABLE 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global612.h"
#include "mpTrace.h"
#include "ndebug.h"
#include "../../STD_DPF/main/include/ui_timer.h"


#include <linux/types.h>
#include <linux/list.h>
#include <linux/jiffies.h>

#include "net_tcp.h"
#include 	"devio.h"
#include "net_device.h"

#include "async_req.h"
#include "tevent.h"
#include "tevent_internal.h"
#include "ttime.h"
#include "SysConfig.h"
#include "net_nic.h"

#include "gsm.h"
#include "ppp.h"
#include "async_req.h"
#include "wwan.h"




enum {
    RESULT_CODE_UNKNOWN,
    RESULT_CODE_OK,
    RESULT_CODE_ERROR,
};

struct at_req_queue {
	struct list_head list;
	int lock;
};

struct at_req_queue at_req_list;

struct atcmd_resp_state {
	enum async_req_state state;
	int result;
};

struct gsm_state {
    bool sim_present;
    bool atCommand_ready;
    bool sim_initialized;                       /* SIM in fully initialized */
};

struct gsm_state gsmState;

struct sms_read {
    struct list_head list;
    short msg_index;
    char msg_storage[8];
};

static struct list_head gsm_read_list;

U08 u08PPPUART_ID;
extern U08 u08EDGESema_ID;
extern BYTE *writePtr, *readPtr;
extern BYTE *startPtr, *endPtr;
#if 1

/*************************/
/*** LOCAL DEFINITIONS ***/
/*************************/

/* free memory if the pointer is valid and zero the pointer */
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) {mpx_Free(x); (x)=NULL;} } while(0)
#endif

/************************/
/*** LOCAL DATA TYPES ***/
/************************/

struct dummy {
	int dummy;
};
struct gsm_stat {
	int dummy;
};



/***********************************/
/*** LOCAL FUNCTION DECLARATIONS ***/
/***********************************/

int ReadByte(void);
int ReadExisting(char *buf, int size);
void *ok_handler(struct async_req *req, char *buf);
void *AtCmd_handler(struct async_req *req, char *buf);
void *ATCMD_urc_handler(struct async_req *req, char *buf);
static void async_gsm_connect_done(struct tevent_req *subreq);
bool async_req_enqueue(struct async_req *req);
bool async_req_dequeue(struct async_req *req);
static void sms_read_message(void);
struct tevent_context *event_context_init(void);
void send_ppp_event(void *ctx, int errCode, void *arg);
bool SetRegisterState(int state);
void check_SMS_Request(void);
struct tevent_req *add_user_input_handler(struct tevent_context *ev, struct async_req *req);

/******************************/
/*** PUBLIC DATA STRUCTURES ***/
/******************************/





extern U08 u08EDGETaskEvent_ID;

void AtCmd_Task(void);
bool async_req_is_error(struct async_req *req, enum async_req_state *state, int *error);
bool SignalTest(bool show);
void SignalTest_Request(void);
void EdgeConnect_Request(void);
int AtCmd_Init(void);

int threeg_init(void)
{
	S32 status;
    struct async_req *req;

//	threeg_sm_init();

	AtCmd_Init();

    return 0;
fail:
    return -1;
}

#endif

// vim: :noexpandtab:
