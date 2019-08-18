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
#include "edge.h"




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

struct list_head gsm_read_list;

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





#define CR '\r'
#define LF '\n'

static U08 u08AtCmdTaskId;
static U08 u08AtCmdEventId;

static U08 u08WwanTaskId;
int Signal_Test_NumRetries;

extern U08 u08EDGETaskEvent_ID;
extern bool gsm_connected;

void AtCmd_Task(void);
bool async_req_is_error(struct async_req *req, enum async_req_state *state, int *error);
bool SignalTest(bool show);
void SignalTest_Request(void);
void EdgeConnect_Request(void);
int AtCmd_Init(void);

int init2(void)
{
	S32 status;
    struct async_req *req;

//    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_EVENT_CLEAR, 0);
//    MP_ASSERT(status > 0);
//    u08AtCmdEventId = status;

	gsm_sm_init();

	ntrace_init();

	AtCmd_Init();

    return 0;
fail:
    return -1;
}

int AtCmd_Init(void)
{
	S32 status;
    	struct async_req *req;

    	if(!at_req_list.lock)
	{
	    	status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	   	MP_ASSERT(status > 0);
		at_req_list.lock = status;
	}

	if(!u08PPPUART_ID)
	{
		status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
		MP_ASSERT(status > 0);
		u08PPPUART_ID = (U08)status;
	}

    INIT_LIST_HEAD(&at_req_list.list);
    INIT_LIST_HEAD(&gsm_read_list);
    
	if (!(req=async_req_new())) {
		goto fail;
	}

	req->handler = ATCMD_urc_handler;
	list_add_tail(&req->list, &at_req_list.list);

	gsm_init_timer();

	if(!u08AtCmdTaskId)
	{
	    	status = mpx_TaskCreate(AtCmd_Task,  DRIVER_PRIORITY, TASK_STACK_SIZE*4);
	    	MP_ASSERT(status > 0);
	    	u08AtCmdTaskId = status;
	}

    mpx_TaskStartup(u08AtCmdTaskId);
//    mpx_TaskStartup(u08WwanTaskId);
    return 0;
fail:
    return -1;
}

// AT command task
void AtCmd_Task(void)
{
	DWORD Event_ret;  
	static char atcmd_buf[4096];
	static int buf_len;
	int v;
	char *bp;
	struct async_req *req;

	mpDebugPrint("--> %s", __func__);
	while (1)
	{
//		mpDebugPrint("--> %s:%d", __func__, __LINE__);
		mpx_EventWait(u08PPPUART_ID, 0xf, OS_EVENT_OR, &Event_ret);
//		mpDebugPrint("--> %s:%d", __func__, __LINE__);

		mpDebugPrint("[AT] event (%x) rxed", Event_ret);
		do {
			v = ReadByte();
			if (v == -1)
            {
//                BREAK_POINT();
				break;
            }
			else if (v == CR)
				continue;
			else if (v == LF)
			{
				if (buf_len > 0)
				{
					/* end of a response line */
					if (buf_len < sizeof(atcmd_buf))
						atcmd_buf[buf_len++] = '\0';
					else
						atcmd_buf[sizeof(atcmd_buf)-1] = '\0';

                    list_for_each_entry(req, &at_req_list.list, list) {
						mpDebugPrint("[AT] call handler (%p) buf=%s", req->handler, atcmd_buf);
						if ( (*req->handler) (req, atcmd_buf) == NULL)
							break;
					}

					buf_len = 0;
				}
			}
			else
			{
				if (buf_len < sizeof(atcmd_buf))
					atcmd_buf[buf_len++] = v;
			}
		} while (1);

        TaskYield();
	}
}

void *AtCmd_handler(struct async_req *req, char *buf)
{
	mpDebugPrint("--> %s", __func__);
	int len = strlen(buf);
	if (len > 256) 
		len = 256;
	
	log_write(buf, len, "EDGE_R-> ");
	if (strcmp(buf, "OK") == 0)
	{
		req->result_code = RESULT_CODE_OK;
		async_req_done(req);
        return NULL;
	}
    else if (strcmp(buf, "ERROR") == 0)
	{
		req->result_code = RESULT_CODE_ERROR;
		async_req_done(req);
        return NULL;
	}
    else if (strncmp(buf, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
	{
		log_printf(1, "%s", buf);
		mpDebugPrint(buf);
		req->result_code = RESULT_CODE_ERROR;
        if (req->buf)
		{
            strncpy(req->buf, buf, req->size);
			req->buf[req->size-1] = '\0';
		}
		async_req_done(req);
        return NULL;
	}
    else if (strncmp(buf, "CONNECT", 7) == 0)
	{
        /* no OK/ERROR for PDP Context command */
		req->result_code = RESULT_CODE_OK;
        if (req->buf)
		{
            strncpy(req->buf, buf, req->size);
			req->buf[req->size-1] = '\0';
		}
		async_req_done(req);
        return NULL;
	}
    else if (strncmp(buf, "+CMGL:", 6) == 0)
	{
        sms_parse(buf);
        return NULL;
	}
    else if (req->buf && 
            (!req->resp || (strncmp(buf, req->resp, strlen(req->resp)) == 0)))
    {
        mpDebugPrint("[AT] %s", buf);
        strncpy(req->buf, buf, req->size);
		req->buf[req->size-1] = '\0';
        return NULL;
    }
    return buf;
}

void *AtCmdCmgl_handler(struct async_req *req, char *buf)
{
	mpDebugPrint("--> %s", __func__);
	if (strcmp(buf, "OK") == 0)
	{
		req->result_code = RESULT_CODE_OK;
		async_req_done(req);
        return NULL;
	}
    else if (strcmp(buf, "ERROR") == 0)
	{
		req->result_code = RESULT_CODE_ERROR;
		async_req_done(req);
        return NULL;
	}
    else if (strncmp(buf, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
	{
		log_printf(1, "%s", buf);
		req->result_code = RESULT_CODE_ERROR;
        if (req->buf)
		{
            strncpy(req->buf, buf, req->size);
			req->buf[req->size-1] = '\0';
		}
		async_req_done(req);
        return NULL;
	}
    else if (strncmp(buf, "+CMGL:", 6) == 0)
	{
        sms_parse(buf);
        return NULL;
	}
    else if (strncmp(buf, "+CMGR:", 6) == 0)
	{
		int idx;
		if (req->resp)
			idx = atoi(req->resp);
		else
			idx = -1;
        sms_parse_cmgr(buf, idx);
        return NULL;
	}
    else
    {
        mpDebugPrint("[SMS] %s", buf);
        sms_data(buf);
        return NULL;
    }
    return buf;
}

/*
 * Some AT commands don't return "OK" as a response.
 */
void *AtCmd_NoOk_handler(struct async_req *req, char *buf)
{
	mpDebugPrint("--> %s", __func__);
	int len = strlen(buf);
	if (len > 256) 
		len = 256;
	
	log_write(buf, len, "EDGE_R-> ");
    if (strcmp(buf, "ERROR") == 0)
	{
		req->result_code = RESULT_CODE_ERROR;
		async_req_done(req);
        return NULL;
	}
    else if (strncmp(buf, "+CME ERROR:", strlen("+CME ERROR:")) == 0)
	{
		log_printf(1, "%s", buf);
		req->result_code = RESULT_CODE_ERROR;
        if (req->buf)
		{
            strncpy(req->buf, buf, req->size);
			req->buf[req->size-1] = '\0';
		}
		async_req_done(req);
        return NULL;
	}
    else if (req->buf && 
            (!req->resp || (strncmp(buf, req->resp, strlen(req->resp)) == 0)))
    {
		req->result_code = RESULT_CODE_OK;
        mpDebugPrint("[AT] %s", buf);
        strncpy(req->buf, buf, req->size);
		req->buf[req->size-1] = '\0';
		async_req_done(req);
        return NULL;
    }
    return buf;
}
/*
 * Handle unsolicited result codes.
 */
void *ATCMD_urc_handler(struct async_req *req, char *buf)
{
    int ret;
    short event;
	bool modem_mode = false;
	
	if (strncmp(buf, "+WIND:", 6) == 0)
	{
		modem_mode = true;
		ret = sscanf(buf, "+WIND: %hd", &event);
		if(ret == 1)
        {
            mpDebugPrint("[WIND] event=%d", event);
            switch (event)
            {
                case 3:
                    gsmState.atCommand_ready = true;
                    break;
                case 1:
                    gsmState.sim_present = true;
                    break;
                case 4:
                    gsmState.sim_initialized = true;
                    break;
                case 15:
                    break;
                default:
                    break;
            }

        }
        return NULL;
	}
    else if (strncmp(buf, "+CMTI:", 6) == 0)
	{
		modem_mode = true;
        short index;
        char str[8];
		struct sms_read *rd_req;

		ntrace_printf(0, "[SMS] %s", buf);
        /* sms received */
		ret = sscanf(buf, "+CMTI: %[^,],%hd", str, &index);
//		mpDebugPrint("[SMS] sscanf returns %d", ret);
        if (ret == 2)
        {
            struct sms_read *rd_req = mpx_Malloc(sizeof(*rd_req));
            if (rd_req)
            {
                rd_req->msg_index = index;
                strncpy(rd_req->msg_storage, str, sizeof(rd_req->msg_storage));
                rd_req->msg_storage[sizeof(rd_req->msg_storage) - 1] = '\0';

                mpx_SemaphoreWait(u08EDGESema_ID);
                list_add_tail(&rd_req->list, &gsm_read_list);
                mpx_SemaphoreRelease(u08EDGESema_ID);

                EventSet(u08EDGETaskEvent_ID, EEVENT_REQ_SMS);
            }
        }
	}
    else if (strncmp(buf, "+CREG:", 6) == 0)
	{
        short creg_val;

		modem_mode = true;
		ret = sscanf(buf, "+CREG: %hd", &creg_val);
        if (ret == 1)
        {
			if (SetRegisterState(creg_val))
				EventSet(u08EDGETaskEvent_ID, EEVENT_GSM_REGISTER_STATUS);
#if USBOTG_HOST_DATANG
			if(creg_val==1)	//regist successful
				NetUsb_SignalRead();
#endif			
        }
	}
    else if (strncmp(buf, "+WGPRSIND:", 10) == 0)
	{
        int gprs;

		
		modem_mode = true;
		ret = sscanf(buf, "+WGPRSIND: %d", &gprs);
		if (ret == 1)
			SetGPRSState(gprs);
        return NULL;
	}
    else if (strncmp(buf, "+CGEV:", 6) == 0)
	{
		modem_mode = true;
		mpDebugPrint("[GPRS] %s",buf);
		log_printf(1, buf);
		ntrace_printf(0, buf);
	}
	else if (strncmp(buf, "NO CARRIER", 10) == 0)
	{
		modem_mode = true;
		ntrace_printf(0, buf);
	}
	else
		mpDebugPrint("[AT] URC=%s",buf);

	if (modem_mode && gsm_connected)
	{
		send_gprs_event(0);
	}
    return NULL;
}

void AtCmd_read_done(struct async_req *req)
{
	struct list_head *entry;
	struct tevent_req *subreq = req->async.priv;
	struct atcmd_resp_state *astate = req->private_data;
	enum async_req_state state;
	uint32_t error;

	mpDebugPrint("--> %s: result=%d", __func__, req->result_code);
	async_req_dequeue(req);

	astate->state = req->state;

	if (async_req_is_error(req, &state, &error)) {
		goto fail;
	}

	if (req->result_code == RESULT_CODE_OK)
		astate->result = RESULT_CODE_OK;
	else
		astate->result = RESULT_CODE_ERROR;

	tevent_req_done(subreq);
	send_event(req->ev);
	SAFE_FREE(req);
	return;

fail:
	send_event(req->ev);
	SAFE_FREE(req);
}

static void tevent_read_timeout(struct tevent_req *subreq)
{
	struct async_req *req = subreq->async.private_data;

	if (subreq->internal.state == TEVENT_REQ_TIMED_OUT)
		async_req_timeout(req);
}

static struct tevent_req *AtCmd_read_send(
					    struct event_context *ev,
					    char *pattern,
					    uint8_t *data, size_t size,
					    void *priv, int wait_time)
{
	struct async_req *req;
	struct tevent_req *result;
	struct dummy *state;
	struct tevent_fd *fde;

	mpDebugPrint("--> %s", __func__);

	result = tevent_req_create(&state,
			struct dummy);
	if (!result) {
		return NULL;
	}

	if (wait_time == 0)
		wait_time = 60;                         /* 60 seconds */

	if (!tevent_req_set_endtime(
				result, ev, timeval_current_ofs(wait_time, 0))) {
        BREAK_POINT();
		goto fail;
	}

	if (!(req=async_req_new())) {
		goto fail;
	}

	req->handler = AtCmd_handler;
	req->ev = ev;
	req->buf = data;
	req->size = size;
	req->resp = pattern;                    /* expected response string */
	req->private_data = priv;

	req->async.fn = AtCmd_read_done;
	req->async.priv = result;

	tevent_req_set_callback(result, tevent_read_timeout, req); // XXX

	async_req_enqueue(req);

	return result;
fail:
	SAFE_FREE(result);
	return NULL;
}

static struct tevent_req *AtCmd_read_send2(
					    struct event_context *ev,
					    char *pattern,
					    uint8_t *data, size_t size,
					    void *priv, int wait_time)
{
	struct async_req *req;
	struct tevent_req *result;
	struct dummy *state;
	struct tevent_fd *fde;

	mpDebugPrint("--> %s", __func__);

	result = tevent_req_create(&state,
			struct dummy);
	if (!result) {
		return NULL;
	}

	if (wait_time == 0)
		wait_time = 60;                         /* 60 seconds */

	if (!tevent_req_set_endtime(
				result, ev, timeval_current_ofs(wait_time, 0))) {
        BREAK_POINT();
		goto fail;
	}

	if (!(req=async_req_new())) {
		goto fail;
	}

	req->handler = AtCmdCmgl_handler;
	req->ev = ev;
	req->buf = data;
	req->size = size;
	req->resp = pattern;                    /* expected response string */
	req->private_data = priv;

	req->async.fn = AtCmd_read_done;
	req->async.priv = result;

	tevent_req_set_callback(result, tevent_read_timeout, req); // XXX

	async_req_enqueue(req);

	return result;
fail:
	SAFE_FREE(result);
	return NULL;
}

static struct tevent_req *AtCmd_read_send3(
					    struct event_context *ev,
					    char *pattern,
					    uint8_t *data, size_t size,
					    void *priv, int wait_time)
{
	struct async_req *req;
	struct tevent_req *result;
	struct dummy *state;
	struct tevent_fd *fde;

	mpDebugPrint("--> %s", __func__);

	result = tevent_req_create(&state,
			struct dummy);
	if (!result) {
		return NULL;
	}

	if (wait_time == 0)
		wait_time = 60;                         /* 60 seconds */

	if (!tevent_req_set_endtime(
				result, ev, timeval_current_ofs(wait_time, 0))) {
        BREAK_POINT();
		goto fail;
	}

	if (!(req=async_req_new())) {
		goto fail;
	}

	req->handler = AtCmd_NoOk_handler;
	req->ev = ev;
	req->buf = data;
	req->size = size;
	req->resp = pattern;                    /* expected response string */
	req->private_data = priv;

	req->async.fn = AtCmd_read_done;
	req->async.priv = result;

	tevent_req_set_callback(result, tevent_read_timeout, req); // XXX

	async_req_enqueue(req);

	return result;
fail:
	SAFE_FREE(result);
	return NULL;
}

/*
 * @param	wait_time Max wait time in seconds.
 */
BOOL __edge_setcmd2(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time)
{
	struct event_context *ev;
	struct tevent_req *req = NULL;
	struct atcmd_resp_state state;
	uint32_t error;
	int RetVal;

	ev = event_context_init();
	if (ev == NULL) {
		goto fail;
	}

	req = AtCmd_read_send(ev, strResponse, inbuf, size, &state, wait_time);
	MP_ASSERT(req);

#if UART_EDGE
	Uart1Write(buf, strlen(buf));
#else
	RetVal= AtCmd_UsbWrite(buf, strlen(buf));
	if(RetVal < 0)
		return RetVal;
#endif

	if (!tevent_req_poll(req, ev)) {
		BREAK_POINT();
		tevent_common_context_destructor(ev);
		goto fail;
	}

	tevent_common_context_destructor(ev);
	if (state.state == ASYNC_REQ_DONE)
	{
		if (state.result == RESULT_CODE_OK)
		{
			SAFE_FREE(ev);
			return true;
		}
	}

fail:
	SAFE_FREE(ev);
	return false;
}

BOOL __edge_setcmd3(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time)
{
	struct event_context *ev;
	struct tevent_req *req = NULL;
	struct atcmd_resp_state state;
	uint32_t error;

	ev = event_context_init();
	if (ev == NULL) {
		goto fail;
	}

	req = AtCmd_read_send2(ev, strResponse, inbuf, size, &state, wait_time);
	MP_ASSERT(req);

#if UART_EDGE
	Uart1Write(buf, strlen(buf));
#else
	AtCmd_UsbWrite(buf, strlen(buf));
#endif

	if (!tevent_req_poll(req, ev)) {
		BREAK_POINT();
		goto fail;
	}

	if (state.state == ASYNC_REQ_DONE)
	{
		if (state.result == RESULT_CODE_OK)
		{
			tevent_common_context_destructor(ev);
			SAFE_FREE(ev);
			return true;
		}
	}

fail:
	tevent_common_context_destructor(ev);
	SAFE_FREE(ev);
	return false;
}

BOOL __edge_setcmd4(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time)
{
	struct event_context *ev;
	struct tevent_req *req = NULL;
	struct atcmd_resp_state state;
	uint32_t error;

	ev = event_context_init();
	if (ev == NULL) {
		goto fail;
	}

	req = AtCmd_read_send3(ev, strResponse, inbuf, size, &state, wait_time);
	MP_ASSERT(req);

#if UART_EDGE
	Uart1Write(buf, strlen(buf));
#else
	AtCmd_UsbWrite(buf, strlen(buf));
#endif

	if (!tevent_req_poll(req, ev)) {
		BREAK_POINT();
		goto fail;
	}

	if (state.state == ASYNC_REQ_DONE)
	{
		if (state.result == RESULT_CODE_OK)
		{
			tevent_common_context_destructor(ev);
			SAFE_FREE(ev);
			SAFE_FREE(req);
			return true;
		}
	}

fail:
	tevent_common_context_destructor(ev);
	SAFE_FREE(ev);
	SAFE_FREE(req);
	return false;
}

struct ppp_state {
	bool gsm_IsRegistered; 
	bool gprs_IsAttached; 
	bool gsm_connected; 
	bool ppp_connected; 
	int  rssi; 
	int  ber; 
};

static void async_ppp_open_handler(struct tevent_context *ev, struct tevent_fd *fde,
			       void *private_data);
static void ppp_open_done(struct tevent_req *subreq);
struct tevent_req *async_user_input(struct tevent_context *ev);

struct tevent_req *async_ppp_open_send(struct tevent_context *ev)
{
	struct tevent_req *result;
	struct dummy *state;
	struct tevent_fd *fde;

	result = tevent_req_create(&state, struct dummy);
	if (result == NULL) {
		return result;
	}

	fde = tevent_add_fd(ev, WWAN_INTERFACE_PPP, async_ppp_open_handler,
			    result);
	if (fde == NULL) {
		SAFE_FREE(result);
		return NULL;
	}
	return result;
}

static void async_ppp_open_handler(struct tevent_context *ev,
			       struct tevent_fd *fde,
			       void *private_data)
{
	struct tevent_req *req = private_data;

	tevent_req_done(req);
}

struct async_req *ppp_open_send(
					    struct event_context *ev,
					    void *priv,
						uint32_t timeout)
{
	struct async_req *result;
	struct tevent_req *subreq;
	struct dummy *state;
	struct tevent_req *t = NULL;

	mpDebugPrint("--> %s", __func__);

	pppOpen(0, send_ppp_event, NULL);

	if (!async_req_setup(&result, &state,
			     struct dummy)) {
		return NULL;
	}

	subreq = async_ppp_open_send(ev);
	if (subreq == NULL) {
		goto fail;
	}
	tevent_req_set_callback(subreq, ppp_open_done, result);

	/* timer event */
	t = tevent_req_create(&state,
			struct dummy);
	if (!t) {
		goto fail;
	}
	if (!tevent_req_set_endtime(
				t, ev, timeval_current_ofs(timeout, 0))) {
        BREAK_POINT();
		goto fail;
	}
	tevent_req_set_callback(t, tevent_read_timeout, result);

	if (add_user_input_handler(ev, result) == NULL) {
		goto fail;
	}

	return result;
fail:
	SAFE_FREE(result);
	SAFE_FREE(subreq);
	SAFE_FREE(t);
	return NULL;
}

static void ppp_open_done(struct tevent_req *subreq)
{
	struct async_req *req = subreq->async.private_data;
	async_req_done(req);
}

static void async_gsm_connect_handler(struct tevent_context *ev,
			       struct tevent_fd *fde,
			       void *private_data)
{
	struct tevent_req *req = private_data;

	mpDebugPrint("--> %s", __func__);
	tevent_req_done(req);
}

struct tevent_req *async_gsm_connect_send(struct tevent_context *ev)
{
	struct tevent_req *result;
	struct dummy *state;
	struct tevent_fd *fde;

	result = tevent_req_create(&state, struct dummy);
	if (result == NULL) {
		return NULL;
	}

	fde = tevent_add_fd(ev, WWAN_INTERFACE_GPRS, async_gsm_connect_handler,
			    result);
	if (fde == NULL) {
		SAFE_FREE(result);
		return NULL;
	}
	return result;
}

struct async_req *gsm_connect_send(struct event_context *ev,
					    void *priv, uint32_t timeout)
{
	struct async_req *result;
	struct tevent_req *subreq = NULL;
	struct tevent_req *t = NULL;
	struct dummy *state;

	mpDebugPrint("--> %s", __func__);

	if (!async_req_setup(&result, NULL,
			     struct ppp_state)) {
		return NULL;
	}

	subreq = async_gsm_connect_send(ev);
	if (subreq == NULL) {
		goto fail;
	}
	tevent_req_set_callback(subreq, async_gsm_connect_done, result);

	/* timer event */
	t = tevent_req_create(&state,
			struct dummy);
	if (!t) {
		goto fail;
	}
	if (!tevent_req_set_endtime(
				t, ev, timeval_current_ofs(timeout, 0))) {
        BREAK_POINT();
		goto fail;
	}
	tevent_req_set_callback(t, tevent_read_timeout, result);

	if (add_user_input_handler(ev, result) == NULL) {
		goto fail;
	}

    EventSet(u08EDGETaskEvent_ID, EEVENT_REQ_GPRS);
	TaskYield();

	return result;
fail:
	SAFE_FREE(subreq);
	SAFE_FREE(result);
	SAFE_FREE(t);
	return NULL;
}

static void async_gsm_connect_done(struct tevent_req *subreq)
{
	struct async_req *req = subreq->async.private_data;
	mpDebugPrint("--> %s", __func__);
	async_req_done(req);
}

int ReadByte(void)
{
	uint8_t v;
	if ((long)readPtr == (long)writePtr)
		return -1;
	v = *readPtr++;
	if(readPtr >= endPtr)
		readPtr = startPtr;
	return (int)v;
}

int ReadExisting(char *buf, int size)
{
	uint8_t v;
	int i = 0, len;
	uint32_t wp;
	if (size == 0)
		goto done;
#if 0
	while (readPtr != writePtr)
	{
		v = *readPtr++;
		if(readPtr >= endPtr)
			readPtr = startPtr;
		buf[i++] = v;
		if (i >= size)
			break;
	}
#else
	/* faster version (maybe) */
	while (readPtr != writePtr)
	{
		wp = (uint32_t)writePtr;
		if ((uint32_t)readPtr < wp)
		{
			len = wp - (uint32_t)readPtr;
			len = MIN(len, size);
			memcpy(&buf[i], readPtr, len);
			i += len;
		}
		else
		{
			len = (uint32_t)endPtr - (uint32_t)readPtr;
			len = MIN(len, size);
			memcpy(&buf[i], readPtr, len);
			i += len;
		}
		readPtr += len;
		if(readPtr >= endPtr)
			readPtr = startPtr;
		size -= len;
		if (size == 0)
			break;
	}
#endif
done:
	return i;
}

/*
 * This is an async call
 */
static void pppConnect(void)
{
	EventSet(u08EDGETaskEvent_ID,PPP_connect);        /* trigger the PPP connection */
}

#if UART_EDGE
void run_edge_task()
{
	DWORD dwNWEvent;
	int ret;

	MP_DEBUG("run_edge_task");
	
	EnableNetWareTask();          /* We use network's timer service */
	
	while(1)
	{
		EventWait(u08EDGETaskEvent_ID, 0xffff, OS_EVENT_OR, &dwNWEvent);

		MP_DEBUG("[RUN_EDGE] dwNWEvent = %x",dwNWEvent);

		if(dwNWEvent & EdgeStatus_Read)
        {
			MP_DEBUG("EdgeStatus_Read event(%d)", gsm_connected);
			if (!gsm_connected)
			{
				MP_DEBUG("calling edge_status");
				ret = edge_status();
				if (edge_status_callback)
					edge_status_callback(ret);
			}
        }

		if(dwNWEvent & Signal_Test)
        {
			if (!gsm_connected)
			{
				if (!SignalTest(true))
				{
					if (Signal_Test_NumRetries > 0)
					{
						Signal_Test_NumRetries--;
						AddTimerProc(500, SignalTest_Request);
					}
				}
			}
        }

		if(dwNWEvent & Ceer_Read)
        {
			if (!gsm_connected)
			{
				getCEERcmd();
			}
        }

		if (dwNWEvent & PPP_connect)
			BREAK_POINT();

		if (dwNWEvent & EEVENT_TIMEOUT)
			gsm_HsmOnEvent(HEVENT_TIMEOUT);

		if (dwNWEvent & EEVENT_REQ_GPRS)
		{
			if (gsmState.sim_present)
				gsm_HsmOnEvent(HEVENT_GPRS_REQ);
		}

		if (dwNWEvent & EEVENT_PPP_CONNECTED)
			;                                   /* do nothing */

		if (dwNWEvent & EEVENT_PPP_DISCONNECTED)
			gsm_HsmOnEvent(HEVENT_GPRS_DISCONNECT);

		if (dwNWEvent & EEVENT_GSM_REGISTER_STATUS)
			gsm_HsmOnEvent(HEVENT_GSM_REGISTER_STATUS);

		if (dwNWEvent & EEVENT_REQ_SMS)
        {
			MP_DEBUG("[SMS] receive EEVENT_REQ_SMS event");
			if (!gsm_connected)
                sms_read_message();
        }
		if(dwNWEvent & PPP_Shutdown)
			__edge_ppp_shutdown();
		if (dwNWEvent & RegisterState_Test)
			__edge_ppp_getRegisterState();
	}
}
#endif

static volatile int sms_is_running;

void CDPF_ShowStartSms();
void CDPF_ShowEndSms();
static void sms_read_message(void)
{
	struct sms_read *rd_req;
	int tot = 0;
	DWORD dwTime, dwElapsed;
	DWORD dwHashKey;
	extern BOOL  boFirstTimeConnect;
	extern BYTE  g_bDisablePollingKey;
	
	dwHashKey = CDPF_GetPageHashKey();
	dwTime = SystemGetTimeStamp();
	CDPF_ClearKey();
	if ( dwHashKey == xpgHash("SlideShow", 9) && (g_bAniFlag & ANI_AUDIO))
	{
		CDPF_ShowStartSms();							//grad add
		g_bDisablePollingKey = 1;	//disable key
	}

	mpDebugPrint("sms_read_message");

	mpx_SemaphoreWait(u08EDGESema_ID);
	sms_is_running = true;
	
	while (!list_empty(&gsm_read_list)) {

		rd_req = list_first_entry(&gsm_read_list, struct sms_read, list);
		list_del(&rd_req->list);
		mpx_SemaphoreRelease(u08EDGESema_ID);

		if (gsmState.sim_present)
			tot += check_SMS(rd_req->msg_index);

		SAFE_FREE(rd_req);

		mpx_SemaphoreWait(u08EDGESema_ID);
	}
	sms_is_running = false;
	mpx_SemaphoreRelease(u08EDGESema_ID);

	mpDebugPrint("sms_read_message end");

	mpDebugPrint("[SMS] number of msgs=%d", tot);

	dwElapsed = GetElapsedMs2(dwTime);
	g_bDisablePollingKey = 0;
	CDPF_ClearKey();
	
	if (tot > 0)
	{
		SetSMSFlag();
		ntrace_printf(0, "[SMS] SetSMSFlag");
		//RemoveTimerProc(CDPF_ShowEndSms);
		//CDPF_ShowEndSms();
	}
	//else if (boFirstTimeConnect)
	//{
	//	if (AddTimerProc(10000, EdgeConnect_Request) == 0)
	//		boFirstTimeConnect = FALSE;
	//	//-----------------------------
	//	if ( dwElapsed < 3000 )
	//		AddTimerProc(3000-dwElapsed+1000, CDPF_ShowEndSms);
	//	else
	//		CDPF_ShowEndSms();
	//}
	else
	{
		if ( dwElapsed < 3000 )
			AddTimerProc(3000-dwElapsed+1000, CDPF_ShowEndSms);
		else
			CDPF_ShowEndSms();
	}
	
}

/*
 * 
 */
BOOL SMS_IsRunning(void)
{
	return sms_is_running ? TRUE : FALSE;
}

void gsm_GetStat(struct gsm_stat *sts)
{

}

struct async_req *req_save;
int dod5;
typedef void (*KEY_INPUT_CALLBACK)(DWORD key);
KEY_INPUT_CALLBACK wwan_key_callback;

DWORD key_save;
static void key_handler(DWORD key)
{
	if (key == KEY_EXIT)
	{
		key_save = KEY_EXIT;
		EventSet(u08PPPUART_ID, LEVENT_USER_INPUT);              /* send signal to event loop */
	}
}

int edge_errno;

/*
 * @param	timeout Max wait time in seconds.
 */
int EdgeConnect2(unsigned long timeout)
{
	struct event_context *ev = NULL;
	struct async_req *req = NULL;
	enum async_req_state state;
	uint32_t error;
	uint32_t start_time = jiffies;
	uint32_t diff;

	mpDebugPrint("Edge Connect2 ...");

	if (isPPPup())
		return PASS;

	if (!gsmState.sim_present)
	{
		edge_errno = EDGE_ERR_NO_SIM;
		return FAIL;
	}

	key_save = 0;
	wwan_key_callback = key_handler;
	if (!gsm_connected)
	{
		ev = event_context_init();
		if (ev == NULL) {
			goto fail;
		}

		req = gsm_connect_send(ev, NULL, timeout);
		MP_ASSERT(req);

		req_save = req;
		while (req->state < ASYNC_REQ_DONE) {
			event_loop_once(ev);
		}

		if (async_req_is_error(req, &state, &error)) {
			if (state == ASYNC_REQ_USER_ERROR)
			{
				MP_DEBUG("User abort");
				edge_errno = EDGE_ERR_EXIT_BY_USER;
			}
			else if (state == ASYNC_REQ_TIMED_OUT)
			{
				MP_DEBUG("Timed out");
				edge_errno = EDGE_ERR_TIMEOUT;
				EventSet(u08EDGETaskEvent_ID, EEVENT_PPP_DISCONNECTED);/* send signal to GSM call sm */
			}
			else
				edge_errno = EDGE_ERR_CONNECT_FAIL;
			if (edge_errno == EDGE_ERR_EXIT_BY_USER)
				log_printf(1, "GPRS attach canceled, to=%u", timeout);
			else
				log_printf(1, "GPRS attach failed errno = %d", edge_errno);
			tevent_common_context_destructor(ev);
			goto fail;
		}

		tevent_common_context_destructor(ev);
		SAFE_FREE(req);
		SAFE_FREE(ev);
		if (!gsm_connected)
			goto fail;
	}

	mpDebugPrint("--> %s:%d", __func__, __LINE__);

	if (!isPPPup()) {
#if 1
		/* check for timeout */
		diff = ((long)jiffies - (long)start_time) / HZ;
		if (timeout <= diff) {
			edge_errno = EDGE_ERR_TIMEOUT;
			goto fail;
		}
		timeout -= diff;
#endif
		ev = event_context_init();
		if (ev == NULL) {
			goto fail;
		}

		req = ppp_open_send(ev, NULL, timeout);
		MP_ASSERT(req);

		while (req->state < ASYNC_REQ_DONE) {
			event_loop_once(ev);
		}

		if (async_req_is_error(req, &state, &error)) {
			if (state == ASYNC_REQ_USER_ERROR)
			{
				MP_DEBUG("User abort");
				edge_errno = EDGE_ERR_EXIT_BY_USER;
			}
			else if (state == ASYNC_REQ_TIMED_OUT)
			{
				MP_DEBUG("Timed out");
				edge_errno = EDGE_ERR_TIMEOUT;
				pppClose(0);
			}
			else
				edge_errno = EDGE_ERR_CONNECT_FAIL;
			if (edge_errno == EDGE_ERR_EXIT_BY_USER)
				log_printf(1, "PPP connect canceled, to=%u", timeout);
			else
				log_printf(1, "PPP failed errno = %d", edge_errno);
			tevent_common_context_destructor(ev);
			goto fail;
		}

		tevent_common_context_destructor(ev);
		if (!isPPPup())
			goto fail;
	}

	wwan_key_callback = NULL;
	SAFE_FREE(ev);
	SAFE_FREE(req);
	return PASS;
fail:
	wwan_key_callback = NULL;
	SAFE_FREE(ev);
	SAFE_FREE(req);
	return FAIL;
}

#define MAX_LINE	(80)
extern char edge_status_buffer[14][MAX_LINE];
void edge_status_cb(int rc)
{
	EventSet(u08PPPUART_ID, 0x100);
}

#if CDPF_ENABLE
/*
 * Write EDGE status to NAND flash
 * @param	timeout Max wait time in seconds.
 */
int EdgeStatusWrite(STREAM*fp, unsigned long timeout)
{
#define	WAIT_TIME	(10 * 1000)
	int accu_time = 0;
	const int max = (timeout * 1000);
	short i;
	DWORD evts;
	int ret;
	mpDebugPrint("-> EdgeStatusWrite");

	if (!fp)
		goto fail;

	mpDebugPrint("-> %u%", jiffies);
	while (accu_time < max)
	{
		ret = mpx_EventWaitWto(u08PPPUART_ID, 0xf00, OS_EVENT_OR, &evts, WAIT_TIME);
		if (ret == OS_STATUS_OK)
		{
			if (evts & 0x100)
			{
				mpDebugPrint("0x100 event %d%,%u,%x", ret, accu_time, evts);
				break;
			}
		}
		accu_time += WAIT_TIME;
	}
	mpDebugPrint("<- %u%,%u", jiffies, accu_time);

	for (i=0; i< sizeof(edge_status_buffer)/sizeof(edge_status_buffer[0]); i++)
	{
		int len;
		if ((len=strlen(edge_status_buffer[i])) > 0)
		{
			FileWrite(fp, edge_status_buffer[i], len);
			FileWrite(fp, "\r\n", 2);
		}
		else
			FileWrite(fp, "No data\r\n", 9);
	}

	return PASS;
fail:
	return FAIL;
}
#endif

bool async_req_enqueue(struct async_req *req)
{
	spin_lock(&at_req_list.lock);
	list_add(&req->list, &at_req_list.list);
	spin_unlock(&at_req_list.lock);
}

bool async_req_dequeue(struct async_req *req)
{
	struct list_head *entry;

	spin_lock(&at_req_list.lock);
	entry = &req->list;
	list_del(entry);
	spin_unlock(&at_req_list.lock);
}

struct wwan_interface wwan_if;

void wwan_get_state(uint16_t *flags)
{
	if (wwan_if.ppp_state)
		*flags |= WWAN_INTERFACE_PPP;
	if (gsm_connected)
		*flags |= WWAN_INTERFACE_GPRS;
}

void wwan_get_state2(uint16_t *flags)
{
	if ( key_save == KEY_EXIT )
	{
		*flags |= WWAN_INTERFACE_EXIT;
		key_save = 0;
	}
}

int dod6;
void send_gprs_event(int gprs_connected)
{
	mpDebugPrint("--> %s", __func__);
	if (gprs_connected)
	{
		if (!wwan_if.gprs_state)
		wwan_if.gprs_state = true;
	}
	else
	{
		if (wwan_if.gprs_state)
			wwan_if.gprs_state = false;
		if (gsm_connected)
		{
			EventSet(u08EDGETaskEvent_ID, EEVENT_PPP_DISCONNECTED);/* send signal to GSM call sm */
		}
	}

	EventSet(u08PPPUART_ID, LEVENT_GPRS_STATUS);              /* send signal to event loop */
	EventSet(u08EDGETaskEvent_ID, PEVENT_MODEM_STATUS);  /* send signal to wwan sm */
}

void check_SMS_Request(void)
{
	struct sms_read *rd_req = mpx_Malloc(sizeof(*rd_req));
	if (rd_req)
	{
		rd_req->msg_index = 0;
		strcpy(rd_req->msg_storage, "SM");

		mpx_SemaphoreWait(u08EDGESema_ID);
		list_add_tail(&rd_req->list, &gsm_read_list);
		mpx_SemaphoreRelease(u08EDGESema_ID);

		mpDebugPrint("[SMS] set EEVENT_REQ_SMS to read all");
		EventSet(u08EDGETaskEvent_ID, EEVENT_REQ_SMS);
	}
}

bool q2687_ready(void)
{
	if (gsmState.atCommand_ready ||
		gsmState.sim_present ||
		gsmState.sim_initialized)
		return true;
	else
		return false;
}

#define	USER_ABORT	(-1)
static void async_user_input_handler(struct tevent_context *ev,
			       struct tevent_fd *fde,
			       void *private_data)
{
	struct tevent_req *req = private_data;

	tevent_req_error(req, USER_ABORT);
}

struct tevent_req *async_user_input(struct tevent_context *ev)
{
	struct tevent_req *result;
	struct dummy *state;
	struct tevent_fd *fde;

//        BREAK_POINT();
	result = tevent_req_create(&state, struct dummy);
	if (result == NULL) {
		return result;
	}

	fde = tevent_add_fd(ev, WWAN_INTERFACE_EXIT, async_user_input_handler,
			    result);
	if (fde == NULL) {
		SAFE_FREE(result);
		return NULL;
	}
	return result;
}

static void user_cancel(struct tevent_req *subreq)
{
	struct async_req *req = subreq->async.private_data;

	if (subreq->internal.state == TEVENT_REQ_USER_ERROR)
		async_req_cancel(req);
}

struct tevent_req *add_user_input_handler(struct tevent_context *ev,
		struct async_req *req)
{
	struct tevent_req *subreq;

	subreq = async_user_input(ev);
	if (subreq == NULL) {
		goto fail;
	}
	tevent_req_set_callback(subreq, user_cancel, req);
	return subreq;

fail:
	return NULL;
}

BOOL gsm_is_sim_present(void)
{
	return (gsmState.sim_present) ? TRUE : FALSE;
}

void gsm_sim_present(bool is_present)
{
	gsmState.sim_present = is_present;
}

void gprs_down(void)
{
	if (gsm_connected)
	{
		send_gprs_event(0);
	}
}


#endif

// vim: :noexpandtab:
