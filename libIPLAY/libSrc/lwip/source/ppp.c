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
#include "ui_timer.h"


#include <linux/types.h>
#include <linux/jiffies.h>
#include "ppp.h"
#if PPP_SUPPORT > 0
#include "randm.h"
#include "fsm.h"
#if PAP_SUPPORT > 0
#include "pap.h"
#endif
#if CHAP_SUPPORT > 0
#include "chap.h"
#endif
#include "ipcp.h"
#include "lcp.h"
#include "magic.h"
#include "auth.h"
#if VJ_SUPPORT > 0
#include "vj.h"
#endif

#include "pppdebug.h"

#include "net_tcp.h"
#include 	"devio.h"
#include "net_device.h"

#include "linux/list.h"

#include "wwan.h"
#include "edge.h"

// jeffery temp
#define EDGE_DEBUG 		0
//#define BILL1                                   /* to disable, undefine it  */

/*************************/
/*** LOCAL DEFINITIONS ***/
/*************************/

/*
 * The basic PPP frame.
 */
#define PPP_ADDRESS(p)  (((u_char *)(p))[0])
#define PPP_CONTROL(p)  (((u_char *)(p))[1])
#define PPP_PROTOCOL(p) ((((u_char *)(p))[2] << 8) + ((u_char *)(p))[3])

/* PPP packet parser states.  Current state indicates operation yet to be
 * completed. */
typedef enum {
    PDIDLE = 0,                 /* Idle state - waiting. */
    PDSTART,                    /* Process start flag. */
    PDADDRESS,                  /* Process address field. */
    PDCONTROL,                  /* Process control field. */
    PDPROTOCOL1,                /* Process protocol field 1. */
    PDPROTOCOL2,                /* Process protocol field 2. */
    PDDATA                      /* Process data byte. */
} PPPDevStates;

#define ESCAPE_P(accm, c) ((accm)[(c) >> 3] & pppACCMMask[c & 0x07])


/************************/
/*** LOCAL DATA TYPES ***/
/************************/
/*
 * PPP interface control block.
 */
typedef struct PPPControl_s {
    char openFlag;                      /* True when in use. */
    char oldFrame;                      /* Old framing character for fd. */
    sio_fd_t fd;                    /* File device ID of port. */
    int  kill_link;                     /* Shut the link down. */
    int  sig_hup;                       /* Carrier lost. */
    int  if_up;                         /* True when the interface is up. */
    int  errCode;                       /* Code indicating why interface is down. */
	//yuming, CHANGE
    //struct pbuf *inHead, *inTail;       /* The input packet. */
    struct sk_buff	*inHead, *inTail;
    PPPDevStates inState;               /* The input process state. */
    char inEscaped;                     /* Escape next character. */
    u16_t inProtocol;                   /* The input protocol code. */
    u16_t inFCS;                        /* Input Frame Check Sequence value. */
    int  mtu;                           /* Peer's mru */
    int  pcomp;                         /* Does peer accept protocol compression? */
    int  accomp;                        /* Does peer accept addr/ctl compression? */
    u_long lastXMit;                    /* Time of last transmission. */
    ext_accm inACCM;                    /* Async-Ctl-Char-Map for input. */
    ext_accm outACCM;                   /* Async-Ctl-Char-Map for output. */
#if VJ_SUPPORT > 0
    int  vjEnabled;                     /* Flag indicating VJ compression enabled. */
    struct vjcompress vjComp;           /* Van Jabobsen compression header. */
#endif

	//yuming, add
    //struct netif netif;

    struct ppp_addrs addrs;

    void (*linkStatusCB)(void *ctx, int errCode, void *arg);
    void *linkStatusCtx;

} PPPControl;

/*
 * Ioctl definitions.
 */

struct npioctl {
    int     protocol;           /* PPP procotol, e.g. PPP_IP */
    enum NPmode mode;
};



/***********************************/
/*** LOCAL FUNCTION DECLARATIONS ***/
/***********************************/
static void pppMain();
static void pppDrop(PPPControl *pc);
#if 1
static void pppInProc(int pd);
#else
static void pppInProc(int pd, u_char *s, int l);
#endif


/******************************/
/*** PUBLIC DATA STRUCTURES ***/
/******************************/

U08 u08EDGETaskEvent_ID = 0;

#if 1
#if MMS_ENABLE
#define buffer_size 98304
#define extend_buffer_size 10240
#else
#define buffer_size 32768
#define extend_buffer_size buffer_size
#endif

static U08 u08PPPUART_RX_ID = 0;

extern U08 u08PPPUART_ID;

U08 u08EDGESema_ID = 0;

static U08 u08EDGETaskId = 0;
static U08 u08EDGETIMERTaskId = 0;

static BYTE atbuf[buffer_size + extend_buffer_size];

BYTE *writePtr, *readPtr;
BYTE *startPtr=&atbuf[0], *endPtr=&atbuf[buffer_size];
static BYTE *tempbuffer = NULL;

struct edge_config{
	unsigned int  wind;	
	unsigned char stsf;
	unsigned char band;
	unsigned char pin_number[10];
	unsigned char*service_name;
	u_long ppp_idle_time;
#if MMS_ENABLE
	unsigned char mms_service_name[20];
	unsigned char mms_proxy[20];
	unsigned int 	mms_proxy_port;
	unsigned char mms_url[64];
#endif
};

struct sms_message{
    /* Attach link list */
    struct list_head list;
	int  num;
	unsigned char sender[32];
	unsigned char time[32];
	unsigned char *content;
	unsigned int  content_length;
#if MMS_ENABLE
	unsigned char mms_url[256];
	unsigned char* mms_message;
	unsigned int  mms_message_length;
#endif
};

struct sms_queue {
	struct list_head list;
	int num_messages;
};

struct list_head SMS_queue;
int total_sms_message = 0;

struct sms_queue sms_list;
#endif


#if (CDPF_LOCALE == en_US)
	static struct edge_config user_config = {
		.wind = 8333,				// enable some unsolicited WIND indications
		.stsf = 0,					// AT+STSF=0
		.band = 4,					// 4, 5,   4=US, 5=cn,tw
		.pin_number = "0000",
		.service_name = "attz.pandigital.com", //"trial.globalm2m.net", //"attz.pandigital.com"
		.ppp_idle_time = 10 * 60,	// 10 min
	};
#elif (CDPF_LOCALE == zh_TW)
	static struct edge_config user_config = {
		.wind = 8333,				// enable some unsolicited WIND indications
		.stsf = 0,					// AT+STSF=0
		.band = 5,					// 4, 5,   4=US, 5=cn,tw
		.pin_number = "0000",
		.service_name = "internet",
		.ppp_idle_time = 10 * 60,	// 10 min
	#if MMS_ENABLE
		"emome",
		"10.1.1.1",
		8080,
		"http://mms.emome.net:8002",
	#endif
	};
#elif (CDPF_LOCALE == zh_CN)
	static struct edge_config user_config = {
		.wind = 8333,				// enable some unsolicited WIND indications
		.stsf = 0,					// AT+STSF=0
		.band = 5,					// 4, 5,   4=US, 5=cn,tw
		.pin_number = "0000",
		.service_name = "cmnet",
		.ppp_idle_time = 10 * 60,	// 10 min
	#if MMS_ENABLE
		"cmwap",
		"10.0.0.172",
		80,
		"http://mmsc.monternet.com",
	#endif
	};
#else
	static struct edge_config user_config = {
		.wind = 8333,				// enable some unsolicited WIND indications
		.stsf = 0,					// AT+STSF=0
		.band = 5,					// 4, 5,   4=US, 5=cn,tw
		"0000",
		.service_name = "internet",
		.ppp_idle_time = 10 * 60,	// 10 min
	#if MMS_ENABLE
		"emome",
		"10.1.1.1",
		8080,
		"http://mms.emome.net:8002",
	#endif
	};
#endif	//CDPF_LOCALE


struct _TSignalStrength
{
	WORD rssi;
	WORD ber;
};

static BYTE ccid[32] = {0};
static BYTE imei[24] = {0};
static BYTE* pb_ccid = &ccid[0];
static BYTE* pb_imei = &imei[0];
static volatile struct _TSignalStrength st_signal = {0xffff,0xffff};
static volatile BYTE bRegisterState = 0;
static volatile BYTE boModemInitialized = FALSE;
#if 0
static volatile int frequency = -1;
#endif
static bool bPdpContextState;

static U08 u08PPPMainTaskId = 0;
static U08 u08PPPInputTaskId = 0;

U08 u08PPPMainSemaId = 0;
U08 u08PPPInSemaId = 0;


//U08 u08PPPMainEventId = 0;
U08 u08PPPMainRxId = 0;
static U08 u08PPPMessageId;

static U08 pppMainTaskWaiting;

u_long subnetMask;

u_long		ppp_last_tx;	/* Time of last Tx	*/
u_long		ppp_last_rx;	/* Time of last Rx	*/
u_long		ppp_last_recv;	/* Time of last Net_Recv_Data2 (in milliseconds) */

extern bool gsm_connected;
extern int Signal_Test_NumRetries;
#if USBOTG_HOST_DATANG
extern char operator[10+1];	//for Datang 3g USB Modem use
int AcT;
#endif
static PPPControl pppControl[NUM_PPP]; /* The PPP interface control blocks. */

/*
 * PPP Data Link Layer "protocol" table.
 * One entry per supported protocol.
 * The last entry must be NULL.
 */
struct protent *ppp_protocols[] = {
    &lcp_protent,
#if PAP_SUPPORT > 0
    &pap_protent,
#endif
#if CHAP_SUPPORT > 0
    &chap_protent,
#endif
#if CBCP_SUPPORT > 0
    &cbcp_protent,
#endif
    &ipcp_protent,
#if CCP_SUPPORT > 0
    &ccp_protent,
#endif
    NULL
};


/*
 * Buffers for outgoing packets.  This must be accessed only from the appropriate
 * PPP task so that it doesn't need to be protected to avoid collisions.
 */
u_char outpacket_buf[NUM_PPP][PPP_MRU+PPP_HDRLEN];  

/*****************************/
/*** LOCAL DATA STRUCTURES ***/
/*****************************/

/*
 * FCS lookup table as calculated by genfcstab.
 */
static const u_short fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

/* PPP's Asynchronous-Control-Character-Map.  The mask array is used
 * to select the specific bit for a character. */
static u_char pppACCMMask[] = {
    0x01,
    0x02,
    0x04,
    0x08,
    0x10,
    0x20,
    0x40,
    0x80
};

extern bool gsm_connected;
extern int edge_errno;

/***********************************/
/*** PUBLIC FUNCTION DEFINITIONS ***/
/***********************************/
typedef void (*EDGE_TIMER_CALLBACK)(void*);

typedef struct
{
    int used;
    EDGE_TIMER_CALLBACK proc;
	void* arg;
    U32 u32Interval;
    U32 u32Counter;
} ST_EDGE_TIMER_REC;

#define EDGETimerSize 4
ST_EDGE_TIMER_REC EDGETimerBank[EDGETimerSize];

int InsertEDGETimer(U32 dwOffsetValue, void* f, void(* Action)(void*));
int RemoveEDGETimer(void(* Action)(void*));
void SetUartRx();
void free_all_sms_messages();
void check_readPtr();
void list_sms();
BOOL __edge_setcmd2(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time);
void __edge_get_sms(char *buf, int index);
void ppp_callback();
void send_ppp_event(void *ctx, int errCode, void *arg);
bool q2687_ready(void);
int setcreg(int val);
void SignalTest_Request(void);
void gsm_sim_present(bool is_present);
BOOL __edge_setcmd4(const unsigned char * buf, const unsigned char * strResponse, char *inbuf, int size, int wait_time);
bool getCEERcmd();
void EDGETIMER_TASK();
int NetUsb_PppStatusChange(void *ctx, bool is_connected);



void TIMEOUT(void(* Action)(void*), void* f, U32 dwOffsetValue){
	InsertEDGETimer(dwOffsetValue,f, Action);
}

void UNTIMEOUT(void(* Action)(void), U32 dwOffsetValue){
	RemoveEDGETimer(Action);
}


void pppLinkStatusCallback(void * ctx, int errCode, void * arg)
{
}


/* Initialize the PPP subsystem. */

struct ppp_settings ppp_settings;

void init_netpool(){
#if 0
    netpool_mem_init(AR2524_MAX_NUM_BUFFERS,AR2524_MAX_NETPOOL_ALLOC_SIZE);
#else
    ar2524_netpool_init();
#endif
}

/**
 *
 * @ingroup PPP
 * @brief Initialize ppp stack
 *
 *
 */
void init_ppp(void)
{
	S32 status;
	pppInit();

	pppMainTaskWaiting = 1;

	if(!u08EDGETIMERTaskId)
	{
		status = mpx_TaskCreate(EDGETIMER_TASK, DRIVER_PRIORITY+1, TASK_STACK_SIZE*4);
		if(status < 0)
		{
			MP_DEBUG("PPP Task create fail %d", status);
			BREAK_POINT();
		}
		u08EDGETIMERTaskId = (U08)status;

		mpx_TaskStartup(u08EDGETIMERTaskId);
	}

	if(!u08PPPInputTaskId){
	    status = mpx_TaskCreate(ppp_callback, DRIVER_PRIORITY, TASK_STACK_SIZE*4);
	    if(status < 0)
	    {
	        MP_DEBUG("PPP Task create fail %d", status);
	        BREAK_POINT();
	    }
	    u08PPPInputTaskId = (U08)status;

	    mpx_TaskStartup(u08PPPInputTaskId);
	}

	if(!u08PPPMainTaskId){
	    status = mpx_TaskCreate(pppMain, DRIVER_PRIORITY, TASK_STACK_SIZE*4);
	    if(status < 0)
	    {
	        MP_DEBUG("PPP Task create fail %d", status);
	        BREAK_POINT();
	    }
	    u08PPPMainTaskId = (U08)status;

	    mpx_TaskStartup(u08PPPMainTaskId);
	}

}

/**
 *
 * @ingroup PPP
 * @brief Initialize PPP stack
 *
 *
 */
void pppInit(void)
{
    struct protent *protp;
    int i, j;
    //netpool_mem_init(RALINK_MAX_NUM_BUFFERS,RALINK_MAX_NETPOOL_ALLOC_SIZE);
	MP_DEBUG("pppInit");
	
	memset(&ppp_settings, 0, sizeof(ppp_settings));
	ppp_settings.usepeerdns = 1;
	ppp_settings.idle_time_limit = user_config.ppp_idle_time;
	pppSetAuth(PPPAUTHTYPE_NONE, NULL, NULL);

	magicInit();

	subnetMask = htonl(0xffffff00);
    
    for (i = 0; i < NUM_PPP; i++) {
        pppControl[i].openFlag = 0;

        /*
         * Initialize to the standard option set.
         */
        for (j = 0; (protp = ppp_protocols[j]) != NULL; ++j)
            (*protp->init)(i);
    }

#if LINK_STATS
    /* Clear the statistics. */
    memset(&lwip_stats.link, 0, sizeof(lwip_stats.link));
#endif
}

void pppSetAuth(enum pppAuthType authType, const char *user, const char *passwd)
{
    switch(authType) {
	case PPPAUTHTYPE_NONE:
	default:
#ifdef LWIP_PPP_STRICT_PAP_REJECT
	    ppp_settings.refuse_pap = 1;
#else
	    /* some providers request pap and accept an empty login/pw */
	    ppp_settings.refuse_pap = 0;
#endif
	    ppp_settings.refuse_chap = 1;
	    break;
	case PPPAUTHTYPE_ANY:
/* Warning: Using PPPAUTHTYPE_ANY might have security consequences.
 * RFC 1994 says:
 *
 * In practice, within or associated with each PPP server, there is a
 * database which associates "user" names with authentication
 * information ("secrets").  It is not anticipated that a particular
 * named user would be authenticated by multiple methods.  This would
 * make the user vulnerable to attacks which negotiate the least secure
 * method from among a set (such as PAP rather than CHAP).  If the same
 * secret was used, PAP would reveal the secret to be used later with
 * CHAP.
 *
 * Instead, for each user name there should be an indication of exactly
 * one method used to authenticate that user name.  If a user needs to
 * make use of different authentication methods under different
 * circumstances, then distinct user names SHOULD be employed, each of
 * which identifies exactly one authentication method.
 *
 */
	    ppp_settings.refuse_pap = 0;
	    ppp_settings.refuse_chap = 0;
	    break;
	case PPPAUTHTYPE_PAP:
	    ppp_settings.refuse_pap = 0;
	    ppp_settings.refuse_chap = 1;
	    break;
	case PPPAUTHTYPE_CHAP:
	    ppp_settings.refuse_pap = 1;
	    ppp_settings.refuse_chap = 0;
	    break;
    }

    if(user) {
	strncpy(ppp_settings.user, user, sizeof(ppp_settings.user)-1);
	ppp_settings.user[sizeof(ppp_settings.user)-1] = '\0';
    } else
	ppp_settings.user[0] = '\0';

    if(passwd) {
	strncpy(ppp_settings.passwd, passwd, sizeof(ppp_settings.passwd)-1);
	ppp_settings.passwd[sizeof(ppp_settings.passwd)-1] = '\0';
    } else
	ppp_settings.passwd[0] = '\0';
}

#define PPP_CALLBACK 0x00000001


typedef void (*PPP_CALLBACK_PROC)();


#ifdef LINUX
S08
tcpip_callback(void (*f)(void *ctx), void *ctx)
{
    U32 message[4];
    
    message[0] = PPP_CALLBACK;
    message[1] = (U32)f;
    message[2] = (U32)ctx;
    mpx_MessageSend(u08PPPMessageId, message, sizeof(message));

  	return ERR_OK;
}
#else
S08
tcpip_callback(void (*f)(void *ctx), void *ctx)
{
	(*f)(ctx);
  	return ERR_OK;
}
#endif


void ppp_callback()
{
    S32 status;
    U32 u32Message[8];
    
    status = mpx_MessageCreate(OS_ATTR_FIFO, 640);
    if(status < 0){
        DPrintf("NetCtrlInit: mailbox create fail");
        BREAK_POINT();
    }
    else
        u08PPPMessageId = (U08)status;
    
    while(1)
    {
        status = mpx_MessageReceive(u08PPPMessageId, (U08*)u32Message);
        //DPrintf("ppp_input: s=%d,m=%d",status,u32Message[0]);
        if(status > 0)
        {
            switch(u32Message[0])
            {            
                case PPP_CALLBACK:
                    (*((PPP_CALLBACK_PROC)u32Message[1]))(u32Message[2]);
                break;
            }
        }
    }

	MessageDestroy(u08PPPMessageId);

	TaskTerminate(u08PPPInputTaskId);
}

/**
 *
 * @ingroup PPP
 * @brief Open a new PPP connection using the given I/O device
 *
 * This initializes the PPP control block but does not
 * attempt to negotiate the LCP session.  If this port
 * connects to a modem, the modem connection must be
 * established before calling this
 *
 * @param fd file descriptor
 * @param linkStatusCB link status control block
 * @param linkStatusCtx link status
 *
 * @return Return a new PPP connection descriptor on success or
 * an error code (negative) on failure.
 */
int pppOpen(sio_fd_t fd, void (*linkStatusCB)(void *ctx, int errCode, void *arg), void *linkStatusCtx)
{
    PPPControl *pc;
    int pd;
	S32 status;

	MP_DEBUG("pppOpen");
//	pppInit();

    /* Find a free PPP session descriptor. Critical region? */
    for (pd = 0; pd < NUM_PPP && pppControl[pd].openFlag != 0; pd++);
    if (pd >= NUM_PPP)
        pd = PPPERR_OPEN;
    else
        pppControl[pd].openFlag = !0;

    /* Launch a deamon thread. */
    if (pd >= 0) {

        pppControl[pd].openFlag = 1;

        lcp_init(pd);
        pc = &pppControl[pd];
        pc->fd = fd;
        pc->kill_link = 0;
        pc->sig_hup = 0;
        pc->if_up = 0;
        pc->errCode = 0;
        pc->inState = PDIDLE;
        pc->inHead = NULL;
        pc->inTail = NULL;
        pc->inEscaped = 0;
        pc->lastXMit = 0;

#if VJ_SUPPORT > 0
        pc->vjEnabled = 0;
        vj_compress_init(&pc->vjComp);
#endif

        /* 
         * Default the in and out accm so that escape and flag characters
         * are always escaped. 
         */
        memset(pc->inACCM, 0, sizeof(ext_accm));
        pc->inACCM[15] = 0x60;
        memset(pc->outACCM, 0, sizeof(ext_accm));
        pc->outACCM[15] = 0x60;

	pc->linkStatusCB = linkStatusCB;
	pc->linkStatusCtx = linkStatusCtx;

#if 0
	sys_thread_new(pppMain, (void*)pd, PPP_THREAD_PRIO);
#else
	EventSet(u08PPPMainRxId , 0x10);

#endif
//use callback function 
#if 0
	if(!linkStatusCB) {
		while(pd >= 0 && !pc->if_up) {
			sys_msleep(5000);
			if (lcp_phase[pd] == PHASE_DEAD) {
				pppClose(pd);
				if (pc->errCode)
					pd = pc->errCode;
				else
					pd = PPPERR_CONNECT;
			}
		}
	}
#endif
    }
    return pd;
}

/**
 *
 * @ingroup PPP
 * @brief Close a PPP connection
 *
 * @param pd PPP connection descriptor
 *
 */
/* Close a PPP connection and release the descriptor. 
 * Any outstanding packets in the queues are dropped.
 * Return 0 on success, an error code on failure. */
int pppClose(int pd)
{
    PPPControl *pc = &pppControl[pd];
    int st = 0;

	MP_DEBUG("pppClose");

	ntrace_printf(0, "[PPP] pppClose: %u", pd);

//	tcpCloseAll();                              /* commented billwang 20100119 */


    /* Disconnect */
    pc->kill_link = !0;
	
    //pppMainWakeup(pd);
#if UART_EDGE
	SetUartRx();
#else	
    EventSet(u08PPPMainRxId , 0x1);
#endif
    
#if 0    
    if(!pc->linkStatusCB) {
	    while(st >= 0 && lcp_phase[pd] != PHASE_DEAD) {
		    sys_msleep(500);
		    break;
	    }
    }
#endif
    return st;
}

static void nPut(PPPControl *pc, struct sk_buff *nb)
{
	struct sk_buff *b;
	int c;

	//for(b = nb; b != NULL; b = b->next) {
	{
	b = nb;
		//UartOutText("==");
		//UartOutText(b->data);
		//UartOutText("==");
	    if((c = sio_write(pc->fd, b->data, b->len)) != b->len) {
		PPPDEBUG((LOG_WARNING,
			    "PPP nPut: incomplete sio_write(%d,, %u) = %d\n", pc->fd, b->len, c));
#if LINK_STATS
		lwip_stats.link.err++;
#endif /* LINK_STATS */
		pc->lastXMit = 0; /* prepend PPP_FLAG to next packet */
		//break;
	    }
	}
	pbuf_free(nb);

#if LINK_STATS
	lwip_stats.link.xmit++;
#endif /* LINK_STATS */
}


/* 
 * pppAppend - append given character to end of given pbuf.  If outACCM
 * is not NULL and the character needs to be escaped, do so.
 * If pbuf is full, append another.
 * Return the current pbuf.
 */
static struct sk_buff *pppAppend(u_char c, struct sk_buff *nb, ext_accm *outACCM)
{
    struct sk_buff *tb = nb;
    
    /* Make sure there is room for the character and an escape code.
     * Sure we don't quite fill the buffer if the character doesn't
     * get escaped but is one character worth complicating this? */
    /* Note: We assume no packet header. */
//yuming, add
#if 0
    if (nb && (PBUF_POOL_BUFSIZE - nb->len) < 2) {
	tb = pbuf_alloc(PBUF_RAW, 0, PBUF_POOL);
	if (tb) {
	    nb->next = tb;
        }
#if LINK_STATS
	else {
	    lwip_stats.link.memerr++;
	}
#endif /* LINK_STATS */
	nb = tb;
    }
#endif

    if (nb) {
	if (outACCM && ESCAPE_P(*outACCM, c)) {
            *((u_char*)nb->data + nb->len++) = PPP_ESCAPE;
            *((u_char*)nb->data + nb->len++) = c ^ PPP_TRANS;
        }
        else
            *((u_char*)nb->data + nb->len++) = c;
    }
        
    return tb;
}


/* Send a packet on the given connection. */
s8_t pppifOutput(struct sk_buff *pb, struct net_device *dev)
{
    int pd = 0;//(int)netif->state;
    u_short protocol = PPP_IP;
    PPPControl *pc = &pppControl[pd];
    u_int fcsOut = PPP_INITFCS;
    struct sk_buff *headMB = NULL, *tailMB = NULL, *p;
    u_char c;


	//add
	pb->data = pb->data + 14;
	//pb->len = pb->len - 14;

    /* Validate parameters. */
    /* We let any protocol value go through - it can't hurt us
     * and the peer will just drop it if it's not accepting it. */
	if (pd < 0 || pd >= NUM_PPP || !pc->openFlag || !pb) {
        PPPDEBUG((LOG_WARNING, "pppifOutput[%d]: bad parms prot=%d pb=%p\n",
                    pd, protocol, pb));
#if LINK_STATS
		lwip_stats.link.opterr++;
		lwip_stats.link.drop++;
#endif
		return ERR_ARG;
	}

    /* Check that the link is up. */
	if (lcp_phase[pd] == PHASE_DEAD) {
        PPPDEBUG((LOG_ERR, "pppifOutput[%d]: link not up\n", pd));
#if LINK_STATS
		lwip_stats.link.rterr++;
		lwip_stats.link.drop++;
#endif
		return ERR_RTE;
	}

    /* Grab an output buffer. */
	headMB = pbuf_alloc(PBUF_POOL_BUFSIZE);
    if (headMB == NULL) {
        PPPDEBUG((LOG_WARNING, "pppifOutput[%d]: first alloc fail\n", pd));
#if LINK_STATS
		lwip_stats.link.memerr++;
		lwip_stats.link.drop++;
#endif /* LINK_STATS */
        return ERR_MEM;
    }
        
#if VJ_SUPPORT > 0
    /* 
     * Attempt Van Jacobson header compression if VJ is configured and
     * this is an IP packet. 
     */
    if (protocol == PPP_IP && pc->vjEnabled) {
        switch (vj_compress_tcp(&pc->vjComp, pb)) {
        case TYPE_IP:
            /* No change...
            protocol = PPP_IP_PROTOCOL;
             */
            break;
        case TYPE_COMPRESSED_TCP:
            protocol = PPP_VJC_COMP;
            break;
        case TYPE_UNCOMPRESSED_TCP:
            protocol = PPP_VJC_UNCOMP;
            break;
        default:
            PPPDEBUG((LOG_WARNING, "pppifOutput[%d]: bad IP packet\n", pd));
#if LINK_STATS
			lwip_stats.link.proterr++;
			lwip_stats.link.drop++;
#endif
        	pbuf_free(headMB);
            return ERR_VAL;
        }
    }
#endif
        
    tailMB = headMB;
        
    /* Build the PPP header. */
    if ((sys_jiffies() - pc->lastXMit) >= PPP_MAXIDLEFLAG)
        tailMB = pppAppend(PPP_FLAG, tailMB, NULL);
    pc->lastXMit = sys_jiffies();
    if (!pc->accomp) {
        fcsOut = PPP_FCS(fcsOut, PPP_ALLSTATIONS);
        tailMB = pppAppend(PPP_ALLSTATIONS, tailMB, &pc->outACCM);
        fcsOut = PPP_FCS(fcsOut, PPP_UI);
        tailMB = pppAppend(PPP_UI, tailMB, &pc->outACCM);
    }
    if (!pc->pcomp || protocol > 0xFF) {
        c = (protocol >> 8) & 0xFF;
        fcsOut = PPP_FCS(fcsOut, c);
        tailMB = pppAppend(c, tailMB, &pc->outACCM);
    }
    c = protocol & 0xFF;
    fcsOut = PPP_FCS(fcsOut, c);
    tailMB = pppAppend(c, tailMB, &pc->outACCM);
    
    /* Load packet. */
	//for(p = pb; p; p = p->next) {
	p = pb;
    {
    	int n;
    	u_char *sPtr;

        sPtr = (u_char*)p->data;
        n = p->len;
        while (n-- > 0) {
            c = *sPtr++;
            
            /* Update FCS before checking for special characters. */
            fcsOut = PPP_FCS(fcsOut, c);
            
            /* Copy to output buffer escaping special characters. */
            tailMB = pppAppend(c, tailMB, &pc->outACCM);
        }
    }

    /* Add FCS and trailing flag. */
    c = ~fcsOut & 0xFF;
    tailMB = pppAppend(c, tailMB, &pc->outACCM);
    c = (~fcsOut >> 8) & 0xFF;
    tailMB = pppAppend(c, tailMB, &pc->outACCM);
    tailMB = pppAppend(PPP_FLAG, tailMB, NULL);
        
    /* If we failed to complete the packet, throw it away. */
    if (!tailMB) {
        PPPDEBUG((LOG_WARNING,
                    "pppifOutput[%d]: Alloc err - dropping proto=%d\n", 
                    pd, protocol));
        pbuf_free(headMB);
#if LINK_STATS
		lwip_stats.link.memerr++;
		lwip_stats.link.drop++;
#endif
        return ERR_MEM;
    }

	/* Send it. */
//    PPPDEBUG((LOG_INFO, "pppifOutput[%d]: proto=0x%04X\n", pd, protocol));
	//MP_DEBUG("pppifOutput[%d]: proto=0x%04X\n", pd, protocol);

    nPut(pc, headMB);

//#ifdef DRIVER_FREE_TXBUFFER
		pbuf_free(pb);
//#endif

    ppp_last_tx = jiffies;

    return ERR_OK;
}


/*
 * Write n characters to a ppp link.
 *  RETURN: >= 0 Number of characters written
 *           -1 Failed to write to device
 */
int pppWrite(int pd, const u_char *s, int n)
{
    PPPControl *pc = &pppControl[pd];
    u_char c;
    u_int fcsOut = PPP_INITFCS;
    struct sk_buff *headMB = NULL, *tailMB;
	headMB = pbuf_alloc(PBUF_POOL_BUFSIZE);
    if (headMB == NULL) {
#if LINK_STATS
		lwip_stats.link.memerr++;
		lwip_stats.link.proterr++;
#endif /* LINK_STATS */
		return PPPERR_ALLOC;
    }

    tailMB = headMB;
        
    /* If the link has been idle, we'll send a fresh flag character to
     * flush any noise. */
    if ((sys_jiffies() - pc->lastXMit) >= PPP_MAXIDLEFLAG)
        tailMB = pppAppend(PPP_FLAG, tailMB, NULL);
    pc->lastXMit = sys_jiffies();
     
    /* Load output buffer. */
    while (n-- > 0) {
        c = *s++;
        
        /* Update FCS before checking for special characters. */
        fcsOut = PPP_FCS(fcsOut, c);
        
        /* Copy to output buffer escaping special characters. */
        tailMB = pppAppend(c, tailMB, &pc->outACCM);
    }
    
    /* Add FCS and trailing flag. */
    c = ~fcsOut & 0xFF;
    tailMB = pppAppend(c, tailMB, &pc->outACCM);
    c = (~fcsOut >> 8) & 0xFF;
    tailMB = pppAppend(c, tailMB, &pc->outACCM);
    tailMB = pppAppend(PPP_FLAG, tailMB, NULL);
        
    /* If we failed to complete the packet, throw it away.
     * Otherwise send it. */
    if (!tailMB) {
		PPPDEBUG((LOG_WARNING,
                "pppWrite[%d]: Alloc err - dropping pbuf len=%d\n", pd, headMB->len));
/*                "pppWrite[%d]: Alloc err - dropping %d:%.*H", pd, headMB->len, LWIP_MIN(headMB->len * 2, 40), headMB->payload)); */
		pbuf_free(headMB);
#if LINK_STATS
		lwip_stats.link.memerr++;
		lwip_stats.link.proterr++;
#endif /* LINK_STATS */
		return PPPERR_ALLOC;
	}

    PPPDEBUG((LOG_INFO, "pppWrite[%d]: len=%d\n", pd, headMB->len));
/*     "pppWrite[%d]: %d:%.*H", pd, headMB->len, LWIP_MIN(headMB->len * 2, 40), headMB->payload)); */
	//MP_DEBUG("nput");
    nPut(pc, headMB);

    return PPPERR_NONE;
}


/*
 * ppp_send_config - configure the transmit characteristics of
 * the ppp interface.
 */
void ppp_send_config(
    int unit, 
    int mtu,
    u32_t asyncmap,
    int pcomp, 
    int accomp
)
{
    PPPControl *pc = &pppControl[unit];
    int i;
    
    pc->mtu = mtu;
    pc->pcomp = pcomp;
    pc->accomp = accomp;
    
    /* Load the ACCM bits for the 32 control codes. */
    for (i = 0; i < 32/8; i++)
        pc->outACCM[i] = (u_char)((asyncmap >> (8 * i)) & 0xFF);
    PPPDEBUG((LOG_INFO, "ppp_send_config[%d]: outACCM=%X %X %X %X\n",
                unit,
                pc->outACCM[0], pc->outACCM[1], pc->outACCM[2], pc->outACCM[3]));
}


/*
 * ppp_set_xaccm - set the extended transmit ACCM for the interface.
 */
void ppp_set_xaccm(int unit, ext_accm *accm)
{
    memcpy(pppControl[unit].outACCM, accm, sizeof(ext_accm));
    PPPDEBUG((LOG_INFO, "ppp_set_xaccm[%d]: outACCM=%X %X %X %X\n",
                unit,
                pppControl[unit].outACCM[0],
                pppControl[unit].outACCM[1],
                pppControl[unit].outACCM[2],
                pppControl[unit].outACCM[3]));
}


/*
 * ppp_recv_config - configure the receive-side characteristics of
 * the ppp interface.
 */
void ppp_recv_config(
    int unit, 
    int mru,
    u32_t asyncmap,
    int pcomp, 
    int accomp
)
{
    PPPControl *pc = &pppControl[unit];
    int i;
    
	(void)accomp;
	(void)pcomp;
	(void)mru;

    /* Load the ACCM bits for the 32 control codes. */
    for (i = 0; i < 32 / 8; i++)
        pc->inACCM[i] = (u_char)(asyncmap >> (i * 8));
    PPPDEBUG((LOG_INFO, "ppp_recv_config[%d]: inACCM=%X %X %X %X\n",
                unit,
                pc->inACCM[0], pc->inACCM[1], pc->inACCM[2], pc->inACCM[3]));
}


/*
 * get_idle_time - return how long the link has been idle.
 */
int get_idle_time(int u, struct ppp_idle *ip)
{   
	unsigned long		elapsed;
	(void)u;

	elapsed = (long)jiffies - (long)ppp_last_tx;
	ip->xmit_idle = elapsed / HZ;

	elapsed = (long)jiffies - (long)ppp_last_rx;
	ip->recv_idle = elapsed / HZ;

    return 1;
}

/*
 * get_idle_time2 - return how long the link has been idle.
 */
unsigned long get_idle_time2(void)
{   
	unsigned long		elapsed;

	elapsed = (long)SystemGetTimeStamp() - (long)ppp_last_recv;
	elapsed = elapsed / 1000;

    return elapsed;
}


/*
 * Return user specified netmask, modified by any mask we might determine
 * for address `addr' (in network byte order).
 * Here we scan through the system's list of interfaces, looking for
 * any non-point-to-point interfaces which might appear to be on the same
 * network as `addr'.  If we find any, we OR in their netmask to the
 * user-specified netmask.
 */
u32_t GetMask(u32_t addr)
{
    u32_t mask, nmask;
    
    htonl(addr);
    if (IN_CLASSA(addr))    /* determine network mask for address class */
        nmask = IN_CLASSA_NET;
    else if (IN_CLASSB(addr))
        nmask = IN_CLASSB_NET;
    else
        nmask = IN_CLASSC_NET;
    /* class D nets are disallowed by bad_ip_adrs */
    mask = subnetMask | htonl(nmask);
    
    /* XXX
     * Scan through the system's network interfaces.
     * Get each netmask and OR them into our mask.
     */
    
    return mask;
}

/*
 * sifvjcomp - config tcp header compression
 */
int sifvjcomp(
    int pd, 
    int vjcomp, 
    int cidcomp, 
    int maxcid
)
{
#if VJ_SUPPORT > 0
    PPPControl *pc = &pppControl[pd];
    
    pc->vjEnabled = vjcomp;
    pc->vjComp.compressSlot = cidcomp;
    pc->vjComp.maxSlotIndex = maxcid;
    PPPDEBUG((LOG_INFO, "sifvjcomp: VJ compress enable=%d slot=%d max slot=%d\n",
                vjcomp, cidcomp, maxcid));
#endif

    return 0;
}


int isPPPup(){
	PPPControl *pc = &pppControl[0];

	if(!pppMainTaskWaiting && pc->if_up)
		return TRUE;
	else
		return FALSE;
}

bool isDataMode()
{
	return pppMainTaskWaiting ? false : true;
}

/*
 * sifup - Config the interface up and enable IP packets to pass.
 */
int sifup(int pd)
{
    PPPControl *pc = &pppControl[pd];
    int st = 1;
    
    if (pd < 0 || pd >= NUM_PPP || !pc->openFlag) {
        st = 0;
        PPPDEBUG((LOG_WARNING, "sifup[%d]: bad parms\n", pd));
    } else {
//yuming, add
#if 0
		netif_remove(&pc->netif);
		if (netif_add(&pc->netif, &pc->addrs.our_ipaddr, &pc->addrs.netmask, &pc->addrs.his_ipaddr, (void *)pd, pppifNetifInit, ip_input)) {
        	
        	netif_set_up(&pc->netif);
        	pc->if_up = 1;
        	pc->errCode = PPPERR_NONE;

			PPPDEBUG((LOG_DEBUG, "sifup: unit %d: linkStatusCB=%lx errCode=%d\n", pd, pc->linkStatusCB, pc->errCode));
			if(pc->linkStatusCB)
				pc->linkStatusCB(pc->linkStatusCtx, pc->errCode, &pc->addrs);
		} else {
        	st = 0;
        	PPPDEBUG((LOG_ERR, "sifup[%d]: netif_add failed\n", pd));
		}
#else

		EnableNetWareTask();
		
		netif_carrier_on(&NicArray[NIC_INDEX_PPP]);

		NetDefaultIpSet(NIC_INDEX_PPP, pc->addrs.our_ipaddr.addr);
		NetDNSSet(NIC_INDEX_PPP, pc->addrs.dns1.addr, 0);
		NetDNSSet(NIC_INDEX_PPP, pc->addrs.dns2.addr, 1);
		NetSubnetMaskSet(NIC_INDEX_PPP, pc->addrs.netmask.addr);
		NetGatewayIpSet(NIC_INDEX_PPP, pc->addrs.our_ipaddr.addr);
		
        mpx_NetLinkStatusSet(NIC_INDEX_PPP, TRUE);
		if(NetDefaultNicGet() == NIC_INDEX_NULL)
			NetDefaultNicSet(NIC_INDEX_PPP);

		pc->if_up = 1;
		pc->errCode = PPPERR_NONE;

		if(pc->linkStatusCB)
			pc->linkStatusCB(pc->linkStatusCtx, pc->errCode, &pc->addrs);


#endif
    }

    return st;
}

/*
 * sifnpmode - Set the mode for handling packets for a given NP.
 */
int sifnpmode(int u, int proto, enum NPmode mode)
{
	(void)u;
	(void)proto;
	(void)mode;
    return 0;
}

/*
 * sifdown - Config the interface down and disable IP.
 */
int sifdown(int pd)
{
    PPPControl *pc = &pppControl[pd];
    int st = 1;
    
    if (pd < 0 || pd >= NUM_PPP || !pc->openFlag) {
        st = 0;
        PPPDEBUG((LOG_WARNING, "sifdown[%d]: bad parms\n", pd));
    } else {
#if 0
        pc->if_up = 0;
		netif_remove(&pc->netif);
		PPPDEBUG((LOG_DEBUG, "sifdown: unit %d: linkStatusCB=%lx errCode=%d\n", pd, pc->linkStatusCB, pc->errCode));
		if(pc->linkStatusCB)
			pc->linkStatusCB(pc->linkStatusCtx, PPPERR_CONNECT, NULL);
#else

		netif_carrier_off(&NicArray[NIC_INDEX_PPP]);

		NetDefaultIpSet(NIC_INDEX_PPP, 0);
		NetDNSSet(NIC_INDEX_PPP, 0, 0);
		NetDNSSet(NIC_INDEX_PPP, 0, 1);
		NetSubnetMaskSet(NIC_INDEX_PPP, 0);
		NetGatewayIpSet(NIC_INDEX_PPP, 0);

        mpx_NetLinkStatusSet(NIC_INDEX_PPP, FALSE);
		if(NicArray[NIC_INDEX_WIFI].u32DefaultIp == INADDR_ANY)
			NetDefaultNicSet(NIC_INDEX_NULL);

		pc->if_up = 0;

		if(pc->linkStatusCB)
			pc->linkStatusCB(pc->linkStatusCtx, PPPERR_CONNECT, NULL);
#endif
	}
    return st;
}


/*
 * sifaddr - Config the interface IP addresses and netmask.
 */
int sifaddr(
    int pd,             /* Interface unit ??? */
    u32_t o,        /* Our IP address ??? */
    u32_t h,        /* His IP address ??? */
    u32_t m,        /* IP subnet mask ??? */
    u32_t ns1,      /* Primary DNS */
    u32_t ns2       /* Secondary DNS */
)
{
    PPPControl *pc = &pppControl[pd];
    int st = 1;
    
    if (pd < 0 || pd >= NUM_PPP || !pc->openFlag) {
        st = 0;
        PPPDEBUG((LOG_WARNING, "sifup[%d]: bad parms\n", pd));
    } else {
		memcpy(&pc->addrs.our_ipaddr, &o, sizeof(o));
		memcpy(&pc->addrs.his_ipaddr, &h, sizeof(h));
		memcpy(&pc->addrs.netmask, &m, sizeof(m));
		memcpy(&pc->addrs.dns1, &ns1, sizeof(ns1));
		memcpy(&pc->addrs.dns2, &ns2, sizeof(ns2));
    }
    return st;
}

/*
 * cifaddr - Clear the interface IP addresses, and delete routes
 * through the interface if possible.
 */
int cifaddr(
    int pd,         /* Interface unit ??? */
    u32_t o,    /* Our IP address ??? */
    u32_t h     /* IP broadcast address ??? */
)
{
    PPPControl *pc = &pppControl[pd];
    int st = 1;
    
	(void)o;
	(void)h;
    if (pd < 0 || pd >= NUM_PPP || !pc->openFlag) {
        st = 0;
        PPPDEBUG((LOG_WARNING, "sifup[%d]: bad parms\n", pd));
    } else {
		IP4_ADDR(&pc->addrs.our_ipaddr, 0,0,0,0);
		IP4_ADDR(&pc->addrs.his_ipaddr, 0,0,0,0);
		IP4_ADDR(&pc->addrs.netmask, 255,255,255,0);
		IP4_ADDR(&pc->addrs.dns1, 0,0,0,0);
		IP4_ADDR(&pc->addrs.dns2, 0,0,0,0);
    }
    return st;
}

/*
 * sifdefaultroute - assign a default route through the address given.
 */
int sifdefaultroute(int pd, u32_t l, u32_t g)
{
    PPPControl *pc = &pppControl[pd];
    int st = 1;
    
	(void)l;
	(void)g;
    if (pd < 0 || pd >= NUM_PPP || !pc->openFlag) {
        st = 0;
        PPPDEBUG((LOG_WARNING, "sifup[%d]: bad parms\n", pd));
    } else {
    //yuming, add
		//netif_set_default(&pc->netif);
    }

    /* TODO: check how PPP handled the netMask, previously not set by ipSetDefault */

    return st;
}

/*
 * cifdefaultroute - delete a default route through the address given.
 */
int cifdefaultroute(int pd, u32_t l, u32_t g)
{
    PPPControl *pc = &pppControl[pd];
    int st = 1;
    
	(void)l;
	(void)g;
    if (pd < 0 || pd >= NUM_PPP || !pc->openFlag) {
        st = 0;
        PPPDEBUG((LOG_WARNING, "sifup[%d]: bad parms\n", pd));
    } else {
    	//yuming, add
		//netif_set_default(NULL);
    }

    return st;
}


/* these callbacks are necessary because lcp_* functions
   must be called in the same context as pppInput(),
   namely the tcpip_thread(), essentially because
   they manipulate timeouts which are thread-private
*/

static void
pppStartCB(void *arg)
{
    int pd = (int)arg;

	PPPDEBUG((LOG_DEBUG, "pppStartCB: unit %d\n", pd));
    lcp_lowerup(pd);
    lcp_open(pd);      /* Start protocol */
}

static void
pppStopCB(void *arg)
{
    int pd = (int)arg;

	PPPDEBUG((LOG_DEBUG, "pppStopCB: unit %d\n", pd));
    lcp_close(pd, "User request");
}

static void
pppHupCB(void *arg)
{
    int pd = (int)arg;

	PPPDEBUG((LOG_DEBUG, "pppHupCB: unit %d\n", pd));
    lcp_lowerdown(pd);
    link_terminated(pd);
}

/**********************************/
/*** LOCAL FUNCTION DEFINITIONS ***/
/**********************************/
/* The main PPP process function.  This implements the state machine according
 * to section 4 of RFC 1661: The Point-To-Point Protocol. */
void pppMain()
{
	int arg = 0;
    int pd = (int)arg;
    struct sk_buff	*p;
    PPPControl* pc;
	S32 status;
	DWORD dwNWEvent;  

	MP_DEBUG("PPP Task is up");

    pc = &pppControl[pd];

	//p = pbuf_alloc(PBUF_POOL_BUFSIZE);
	//if(!p) {
	//	LWIP_ASSERT("p != NULL", p);
	//	pc->errCode = PPPERR_ALLOC;
	//	goto out;
	//}
	//p->len = 2048;

	//BREAK_POINT();
	//create semaphore
    /* Create Semaphore */
    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    if(status < 0){
        MP_DEBUG("PPPTask: Create Sema fail");
        BREAK_POINT();
    }
    else
        u08PPPMainSemaId = (U08)status;


    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
    if(status < 0){
        MP_DEBUG("PPPTask: Create Sema fail");
        BREAK_POINT();
    }
    else
        u08PPPInSemaId = (U08)status;


    //status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
    //if(status < 0)
    //{
    //    MP_DEBUG("[NIC] PPPTask event create fail");
    //    BREAK_POINT();
    //}
    //else
    //    u08PPPMainEventId = (U08)status;

    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[NIC] PPPTask event create fail");
        BREAK_POINT();
    }
    else
        u08PPPMainRxId = (U08)status;

#if UART_EDGE
    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[NIC] PPPTask event create fail");
        BREAK_POINT();
    }
    else
        u08PPPUART_RX_ID = (U08)status;
#endif



	//yuming, set auth
	//pppSetAuth(PPPAUTHTYPE_ANY, "yuming", "yuming");
	//pppSetAuth(PPPAUTHTYPE_ANY,  "0000", "0000");

	while(1){
		MP_DEBUG("going waiting");
	pppMainTaskWaiting = 1;
	EventWait(u08PPPMainRxId, 0x10, OS_EVENT_OR, &dwNWEvent);
	pppMainTaskWaiting = 0;
	p = pbuf_alloc(PBUF_POOL_BUFSIZE);
	if(!p) {
		LWIP_ASSERT("p != NULL", p);
		pc->errCode = PPPERR_ALLOC;
		goto out;
	}
	p->len = PBUF_POOL_BUFSIZE;

		MP_DEBUG("going waiting out");
	/*
	 * Start the connection and handle incoming events (packet or timeout).
	 */	
	PPPDEBUG((LOG_INFO, "pppMain: unit %d: Connecting\n", pd));
	MP_DEBUG("pppMain: unit %d: Connecting\n", pd);
	tcpip_callback(pppStartCB, arg);

	while (lcp_phase[pd] != PHASE_DEAD) {
		if (pc->kill_link) {
			PPPDEBUG((LOG_DEBUG, "pppMainWakeup: unit %d kill_link -> pppStopCB\n", pd));
		pc->errCode = PPPERR_USER;
		/* This will leave us at PHASE_DEAD. */
			tcpip_callback(pppStopCB, arg);
		pc->kill_link = 0;
		}
		else if (pc->sig_hup) {
			PPPDEBUG((LOG_DEBUG, "pppMainWakeup: unit %d sig_hup -> pppHupCB\n", pd));
		pc->sig_hup = 0;
			tcpip_callback(pppHupCB, arg);
		} else {
#if 1
			EventWait(u08PPPUART_RX_ID, 0x1, OS_EVENT_OR, &dwNWEvent);
			if(writePtr != readPtr){
				pppInProc(pd);
			} else {
				PPPDEBUG((LOG_DEBUG, "pppMainWakeup: unit %d sio_read len=%d returned %d\n", pd, p->len, 0));
				sys_msleep(1); /* give other tasks a chance to run */
			}
#else
			int c = sio_read(pc->fd, p->data, p->len);
			if(c > 0) {
				//MP_DEBUG("len = %d", c);
				pppInProc(pd, p->data, c);
			} else {
				PPPDEBUG((LOG_DEBUG, "pppMainWakeup: unit %d sio_read len=%d returned %d\n", pd, p->len, c));
				//MP_DEBUG("pppMainWakeup: unit %d sio_read len=%d returned %d\n", pd, p->len, c);
				sys_msleep(1); /* give other tasks a chance to run */
			}
#endif
		}
	}
	PPPDEBUG((LOG_INFO, "pppMain: unit %d: PHASE_DEAD\n", pd));
	MP_DEBUG("pppMain: unit %d: PHASE_DEAD\n", pd);

	pbuf_free(p);

out:
	PPPDEBUG((LOG_DEBUG, "pppMain: unit %d: linkStatusCB=%lx errCode=%d\n", pd, pc->linkStatusCB, pc->errCode));
	MP_DEBUG("pppMain: unit %d: linkStatusCB=%lx errCode=%d\n", pd, pc->linkStatusCB, pc->errCode);
	if(pc->linkStatusCB)
		pc->linkStatusCB(pc->linkStatusCtx, pc->errCode ? pc->errCode : PPPERR_PROTOCOL, NULL);

	pc->openFlag = 0;

	}
}


struct pppInputHeader {
	u8_t RESERVED[14];
	u16_t proto;
	int unit;
};


/*
 * Pass the processed input packet to the appropriate handler.
 * This function and all handlers run in the context of the tcpip_thread
 */
static void pppInput(void *arg)
{
	struct sk_buff *nb = (struct sk_buff *)arg;
    u16_t protocol;
    int pd;
	static short pppInput_cnt;

	//MP_DEBUG("%s called, nb = %x", __FUNCTION__, nb);

	pd = ((struct pppInputHeader *)nb->data)->unit;
	protocol = ((struct pppInputHeader *)nb->data)->proto;

	//CHANGE
#if 0
    pbuf_header(nb, -(int)sizeof(struct pppInputHeader));
#else
	nb->data = nb ->data + sizeof(struct pppInputHeader);
	nb->len = nb->len - sizeof(struct pppInputHeader);
#endif


#if LINK_STATS
    lwip_stats.link.recv++;
#endif /* LINK_STATS */

    /*
     * Toss all non-LCP packets unless LCP is OPEN.
     * Until we get past the authentication phase, toss all packets
     * except LCP, LQR and authentication packets.
     */
    if((lcp_phase[pd] <= PHASE_AUTHENTICATE) && (protocol != PPP_LCP)) {
	    if(!((protocol == PPP_LQR) || (protocol == PPP_PAP) || (protocol == PPP_CHAP)) ||
			(lcp_phase[pd] != PHASE_AUTHENTICATE)) {
		PPPDEBUG((LOG_INFO, "pppInput: discarding proto 0x%04X in phase %d\n", protocol, lcp_phase[pd]));
		MP_DEBUG("pppInput: discarding proto 0x%04X in phase %d\n", protocol, lcp_phase[pd]);
		goto drop;
	    }
    }


	//MP_DEBUG("\r\nproto %x \r\n", protocol);
    switch(protocol) {
    case PPP_VJC_COMP:      /* VJ compressed TCP */
#if VJ_SUPPORT > 0
        PPPDEBUG((LOG_INFO, "pppInput[%d]: vj_comp in pbuf len=%d\n", pd, nb->len));
		MP_DEBUG("pppInput[%d]: vj_comp in pbuf len=%d\n", pd, nb->len);
        /*
         * Clip off the VJ header and prepend the rebuilt TCP/IP header and
         * pass the result to IP.
         */
        if (vj_uncompress_tcp(&nb, &pppControl[pd].vjComp) >= 0) {
//CHANGE
#if 0
            if (pppControl[pd].netif.input != NULL) {
              pppControl[pd].netif.input(nb, &pppControl[pd].netif);
            }
#else
			nb->data = nb->data - 14;
			IpPacketReceive(nb, 0, &NicArray[NIC_INDEX_PPP]);
#endif
			return;
        }
	/* Something's wrong so drop it. */
	PPPDEBUG((LOG_WARNING, "pppInput[%d]: Dropping VJ compressed\n", pd));
#else
        /* No handler for this protocol so drop the packet. */
        PPPDEBUG((LOG_INFO, "pppInput[%d]: drop VJ Comp in %d:%s\n", pd, nb->len, nb->data));
        MP_DEBUG("pppInput[%d]: drop VJ Comp in %d:%s\n", pd, nb->len, nb->data);
#endif /* VJ_SUPPORT > 0 */
	break;
    case PPP_VJC_UNCOMP:    /* VJ uncompressed TCP */
#if VJ_SUPPORT > 0
        PPPDEBUG((LOG_INFO, "pppInput[%d]: vj_un in pbuf len=%d\n", pd, nb->len));
        /*
         * Process the TCP/IP header for VJ header compression and then pass
         * the packet to IP.
         */
        if (vj_uncompress_uncomp(nb, &pppControl[pd].vjComp) >= 0) {
//CHANGE
#if 0
            if (pppControl[pd].netif.input != NULL) {
              pppControl[pd].netif.input(nb, &pppControl[pd].netif);
            }
#else
			nb->data = nb->data - 14;
			IpPacketReceive(nb, 0, &NicArray[NIC_INDEX_PPP]);
#endif
			return;
        }
	/* Something's wrong so drop it. */
	PPPDEBUG((LOG_WARNING, "pppInput[%d]: Dropping VJ uncompressed\n", pd));
#else
        /* No handler for this protocol so drop the packet. */
        PPPDEBUG((LOG_INFO,
                    "pppInput[%d]: drop VJ UnComp in %d:.*H\n", 
                    pd, nb->len, LWIP_MIN(nb->len * 2, 40), nb->data));
#endif /* VJ_SUPPORT > 0 */
	break;
    case PPP_IP:            /* Internet Protocol */
	    if ((pppInput_cnt++ % 10) == 0)
        PPPDEBUG((LOG_INFO, "pppInput[%d]: ip in pbuf len=%d\n", pd, nb->len));
		//MP_DEBUG("pppInput[%d]: ip in pbuf len=%d\n", pd, nb->len);
//CHANGE
#if 0
        if (pppControl[pd].netif.input != NULL) {
          pppControl[pd].netif.input(nb, &pppControl[pd].netif);
        }
#else

		nb->data = nb->data - 14;
		nb->len = nb->len + 14;

		IpPacketReceive(nb, 0, &NicArray[NIC_INDEX_PPP]);

		ppp_last_rx = jiffies;
#endif
		return;
    default:
	{
		struct protent *protp;
		int i;

		/*
		 * Upcall the proper protocol input routine.
		 */
		for (i = 0; (protp = ppp_protocols[i]) != NULL; ++i) {
			if (protp->protocol == protocol && protp->enabled_flag) {
				PPPDEBUG((LOG_INFO, "pppInput[%d]: %s len=%d\n", pd, protp->name, nb->len));
				//MP_DEBUG("pppInput[%d]: %s len=%d\n", pd, protp->name, nb->len);
				//yuming, add
				//nb = pppSingleBuf(nb);
				(*protp->input)(pd, nb->data, nb->len);
				ppp_last_rx = jiffies;
				goto out;
			}
		}

		/* No handler for this protocol so reject the packet. */
		PPPDEBUG((LOG_INFO, "pppInput[%d]: rejecting unsupported proto 0x%04X len=%d\n", pd, protocol, nb->len));
		MP_DEBUG("pppInput[%d]: rejecting unsupported proto 0x%04X len=%d\n", pd, protocol, nb->len);
		//CHANGE
#if 0
		pbuf_header(nb, sizeof(protocol));
#else
		nb->data = nb->data - 4;
		nb->len = nb->len + 4;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
		protocol = htons(protocol);
		memcpy(nb->data, &protocol, sizeof(protocol));
#endif
		lcp_sprotrej(pd, nb->data, nb->len);
	}
	break;
    }

drop:
#if LINK_STATS
    lwip_stats.link.drop++;
#endif

out:
    pbuf_free(nb);
    return;
}


/*
 * Drop the input packet.
 */
static void pppDrop(PPPControl *pc)
{
    if (pc->inHead != NULL) {
#if 0	    
        PPPDEBUG((LOG_INFO, "pppDrop: %d:%.*H\n", pc->inHead->len, min(60, pc->inHead->len * 2), pc->inHead->payload));
#endif	
        PPPDEBUG((LOG_INFO, "pppDrop: pbuf len=%d\n", pc->inHead->len));
		MP_DEBUG("pppDrop: pbuf len=%d\n", pc->inHead->len);
		if (pc->inTail && (pc->inTail != pc->inHead))
	    	pbuf_free(pc->inTail);
        pbuf_free(pc->inHead);
        pc->inHead = NULL;
        pc->inTail = NULL;
    }
#if VJ_SUPPORT > 0
    vj_uncompress_err(&pc->vjComp);
#endif

#if LINK_STATS
    lwip_stats.link.drop++;
#endif /* LINK_STATS */
}
#if 1
int ReadExisting(char *buf, int size);
char ppp_inbuf[2048];
/*
 * Process a received octet string.
 */
static void pppInProc(int pd)
{
    PPPControl *pc = &pppControl[pd];
    struct sk_buff *nextNBuf;
    u_char curChar;
	u_char *s;
	int length;
	
retry:
#if 1
	if (pc->kill_link)              /* speed up process of the close operation */
	{
		readPtr = writePtr;                     /* flush any input data */
		return;
	}
#endif
	length = ReadExisting(ppp_inbuf, sizeof ppp_inbuf);
	s = ppp_inbuf;

	if (length == 0)
		return;
//	if (length)
//        mpDebugPrint("[PPP] input=%d", length);

    PPPDEBUG((LOG_DEBUG, "pppInProc[%d]: got %d bytes\n", pd, length));
	//MP_DEBUG("pppInProc[%d]: got %d bytes\n", pd, l);
    while (length-- > 0) {
        curChar = *s++;

        /* Handle special characters. */
        if (ESCAPE_P(pc->inACCM, curChar)) {
            /* Check for escape sequences. */
            /* XXX Note that this does not handle an escaped 0x5d character which
             * would appear as an escape character.  Since this is an ASCII ']'
             * and there is no reason that I know of to escape it, I won't complicate
             * the code to handle this case. GLL */
            if (curChar == PPP_ESCAPE)
                pc->inEscaped = 1;
            /* Check for the flag character. */
            else if (curChar == PPP_FLAG) {
                /* If this is just an extra flag character, ignore it. */
                if (pc->inState <= PDADDRESS)
                    ;
                /* If we haven't received the packet header, drop what has come in. */
                else if (pc->inState < PDDATA) {
                    PPPDEBUG((LOG_WARNING,
                                "pppInProc[%d]: Dropping incomplete packet %d\n", 
                                pd, pc->inState));
					MP_DEBUG("pppInProc[%d]: Dropping incomplete packet %d\n", 
                                pd, pc->inState);
#if LINK_STATS
					lwip_stats.link.lenerr++;
#endif
                    pppDrop(pc);
                }
                /* If the fcs is invalid, drop the packet. */
                else if (pc->inFCS != PPP_GOODFCS) {
                    PPPDEBUG((LOG_INFO,
                                "pppInProc[%d]: Dropping bad fcs 0x%04X proto=0x%04X\n", 
                                pd, pc->inFCS, pc->inProtocol));
					MP_DEBUG("pppInProc[%d]: Dropping bad fcs 0x%4x proto=0x%4x\n", 
                                pd, pc->inFCS, pc->inProtocol);
#if LINK_STATS
					lwip_stats.link.chkerr++;
#endif
                    pppDrop(pc);
                }
                /* Otherwise it's a good packet so pass it on. */
                else {
                    
                    /* Trim off the checksum. */
#if 0
		    if(pc->inTail->len >= 2) {
			pc->inTail->len -= 2;

			pc->inTail->tot_len = pc->inTail->len;
			if (pc->inTail != pc->inHead) {
			    pbuf_cat(pc->inHead, pc->inTail);
			}
		    } else {
			pc->inTail->tot_len = pc->inTail->len;
			if (pc->inTail != pc->inHead) {
			    pbuf_cat(pc->inHead, pc->inTail);
			}

			pbuf_realloc(pc->inHead, pc->inHead->tot_len - 2);
		    }
#endif
                    /* Dispatch the packet thereby consuming it. */
		    if(tcpip_callback(pppInput, pc->inHead) != 0) {
                    	PPPDEBUG((LOG_ERR,
				    "pppInProc[%d]: tcpip_callback() failed, dropping packet\n", pd));
			pbuf_free(pc->inHead);
#if LINK_STATS
			lwip_stats.link.drop++;
#endif
		    }
                    pc->inHead = NULL;
                    pc->inTail = NULL;
                }
                    
                /* Prepare for a new packet. */
                pc->inFCS = PPP_INITFCS;
                pc->inState = PDADDRESS;
                pc->inEscaped = 0;
            }
            /* Other characters are usually control characters that may have
             * been inserted by the physical layer so here we just drop them. */
            else {
                PPPDEBUG((LOG_WARNING,
                            "pppInProc[%d]: Dropping ACCM char <%d>\n", pd, curChar));
				MP_DEBUG("pppInProc[%d]: Dropping ACCM char <%d>\n", pd, curChar);
            }
        }
        /* Process other characters. */
        else {
            /* Unencode escaped characters. */
            if (pc->inEscaped) {
                pc->inEscaped = 0;
                curChar ^= PPP_TRANS;
            }
            
            /* Process character relative to current state. */
            switch(pc->inState) {
            case PDIDLE:                    /* Idle state - waiting. */
                /* Drop the character if it's not 0xff
                 * we would have processed a flag character above. */
                if (curChar != PPP_ALLSTATIONS) {
                	break;
				}

				/* Fall through */
            case PDSTART:                   /* Process start flag. */
                /* Prepare for a new packet. */
                pc->inFCS = PPP_INITFCS;

				/* Fall through */
            case PDADDRESS:                 /* Process address field. */
                if (curChar == PPP_ALLSTATIONS) {
                    pc->inState = PDCONTROL;
                    break;
                }
                /* Else assume compressed address and control fields so
                 * fall through to get the protocol... */
            case PDCONTROL:                 /* Process control field. */
                /* If we don't get a valid control code, restart. */
                if (curChar == PPP_UI) {
                    pc->inState = PDPROTOCOL1;
                	break;
                }
#if 0
                else {
                    PPPDEBUG((LOG_WARNING,
                                "pppInProc[%d]: Invalid control <%d>\n", pd, curChar));
                    pc->inState = PDSTART;
                }
#endif
            case PDPROTOCOL1:               /* Process protocol field 1. */
                /* If the lower bit is set, this is the end of the protocol
                 * field. */
                if (curChar & 1) {
                    pc->inProtocol = curChar;
                    pc->inState = PDDATA;
                }
                else {
                    pc->inProtocol = (u_int)curChar << 8;
                    pc->inState = PDPROTOCOL2;
                }
                break;
            case PDPROTOCOL2:               /* Process protocol field 2. */
                pc->inProtocol |= curChar;
                pc->inState = PDDATA;
                break;
            case PDDATA:                    /* Process data byte. */
                /* Make space to receive processed data. */
                if (pc->inTail == NULL || pc->inTail->len == PBUF_POOL_BUFSIZE) {
#if 0
		    if(pc->inTail) {
			pc->inTail->tot_len = pc->inTail->len;
			if (pc->inTail != pc->inHead) {
			    pbuf_cat(pc->inHead, pc->inTail);
			}
		    }
#endif
                    /* If we haven't started a packet, we need a packet header. */
                    nextNBuf = pbuf_alloc(PBUF_POOL_BUFSIZE);

                    if (nextNBuf == NULL) {
                        /* No free buffers.  Drop the input packet and let the
                         * higher layers deal with it.  Continue processing
                         * the received pbuf chain in case a new packet starts. */
                        PPPDEBUG((LOG_ERR, "pppInProc[%d]: NO FREE MBUFS!\n", pd));
						MP_DEBUG("pppInProc[%d]: NO FREE MBUFS!\n", pd);
#if LINK_STATS
						lwip_stats.link.memerr++;
#endif /* LINK_STATS */
                        pppDrop(pc);
                        pc->inState = PDSTART;  /* Wait for flag sequence. */
			break;
                    }		    
			if (pc->inHead == NULL) {
			struct pppInputHeader *pih = nextNBuf->data;

			pih->unit = pd;
			pih->proto = pc->inProtocol;

			nextNBuf->len += sizeof(*pih);

			pc->inHead = nextNBuf;
		    }
		    pc->inTail = nextNBuf;
                }
                /* Load character into buffer. */
                ((u_char*)pc->inTail->data)[pc->inTail->len++] = curChar;
                break;
            }

            /* update the frame check sequence number. */
            pc->inFCS = PPP_FCS(pc->inFCS, curChar);
        }
    }
	if (!wwan_if.ppp_state || pc->kill_link)
		return;
	goto retry;
	//MP_DEBUG("%s end", __FUNCTION__);
}

#else
/*
 * Process a received octet string.
 */
static void pppInProc(int pd, u_char *s, int l)
{
    PPPControl *pc = &pppControl[pd];
    struct sk_buff *nextNBuf;
    u_char curChar;

    PPPDEBUG((LOG_DEBUG, "pppInProc[%d]: got %d bytes\n", pd, l));
	//MP_DEBUG("pppInProc[%d]: got %d bytes\n", pd, l);
	//NetPacketDump(s, l);
    while (l-- > 0) {
        curChar = *s++;
        
        /* Handle special characters. */
        if (ESCAPE_P(pc->inACCM, curChar)) {
            /* Check for escape sequences. */
            /* XXX Note that this does not handle an escaped 0x5d character which
             * would appear as an escape character.  Since this is an ASCII ']'
             * and there is no reason that I know of to escape it, I won't complicate
             * the code to handle this case. GLL */
            if (curChar == PPP_ESCAPE)
                pc->inEscaped = 1;
            /* Check for the flag character. */
            else if (curChar == PPP_FLAG) {
                /* If this is just an extra flag character, ignore it. */
                if (pc->inState <= PDADDRESS)
                    ;
                /* If we haven't received the packet header, drop what has come in. */
                else if (pc->inState < PDDATA) {
                    PPPDEBUG((LOG_WARNING,
                                "pppInProc[%d]: Dropping incomplete packet %d\n", 
                                pd, pc->inState));
					MP_DEBUG("pppInProc[%d]: Dropping incomplete packet %d\n", 
                                pd, pc->inState);
#if LINK_STATS
					lwip_stats.link.lenerr++;
#endif
                    pppDrop(pc);
                }
                /* If the fcs is invalid, drop the packet. */
                else if (pc->inFCS != PPP_GOODFCS) {
                    PPPDEBUG((LOG_INFO,
                                "pppInProc[%d]: Dropping bad fcs 0x%04X proto=0x%04X\n", 
                                pd, pc->inFCS, pc->inProtocol));
					MP_DEBUG("pppInProc[%d]: Dropping bad fcs 0x%4x proto=0x%4x\n", 
                                pd, pc->inFCS, pc->inProtocol);
					//BREAK_POINT();
#if LINK_STATS
					lwip_stats.link.chkerr++;
#endif
                    pppDrop(pc);
                }
                /* Otherwise it's a good packet so pass it on. */
                else {
                    
                    /* Trim off the checksum. */
#if 0
		    if(pc->inTail->len >= 2) {
			pc->inTail->len -= 2;

			pc->inTail->tot_len = pc->inTail->len;
			if (pc->inTail != pc->inHead) {
			    pbuf_cat(pc->inHead, pc->inTail);
			}
		    } else {
			pc->inTail->tot_len = pc->inTail->len;
			if (pc->inTail != pc->inHead) {
			    pbuf_cat(pc->inHead, pc->inTail);
			}

			pbuf_realloc(pc->inHead, pc->inHead->tot_len - 2);
		    }
#endif
                    /* Dispatch the packet thereby consuming it. */
		    if(tcpip_callback(pppInput, pc->inHead) != 0) {
                    	PPPDEBUG((LOG_ERR,
				    "pppInProc[%d]: tcpip_callback() failed, dropping packet\n", pd));
			pbuf_free(pc->inHead);
#if LINK_STATS
			lwip_stats.link.drop++;
#endif
		    }
                    pc->inHead = NULL;
                    pc->inTail = NULL;
                }
                    
                /* Prepare for a new packet. */
                pc->inFCS = PPP_INITFCS;
                pc->inState = PDADDRESS;
                pc->inEscaped = 0;
            }
            /* Other characters are usually control characters that may have
             * been inserted by the physical layer so here we just drop them. */
            else {
                PPPDEBUG((LOG_WARNING,
                            "pppInProc[%d]: Dropping ACCM char <%d>\n", pd, curChar));
				MP_DEBUG("pppInProc[%d]: Dropping ACCM char <%d>\n", pd, curChar);
            }
        }
        /* Process other characters. */
        else {
            /* Unencode escaped characters. */
            if (pc->inEscaped) {
                pc->inEscaped = 0;
                curChar ^= PPP_TRANS;
            }
            
            /* Process character relative to current state. */
            switch(pc->inState) {
            case PDIDLE:                    /* Idle state - waiting. */
                /* Drop the character if it's not 0xff
                 * we would have processed a flag character above. */
                if (curChar != PPP_ALLSTATIONS) {
                	break;
				}

				/* Fall through */
            case PDSTART:                   /* Process start flag. */
                /* Prepare for a new packet. */
                pc->inFCS = PPP_INITFCS;

				/* Fall through */
            case PDADDRESS:                 /* Process address field. */
                if (curChar == PPP_ALLSTATIONS) {
                    pc->inState = PDCONTROL;
                    break;
                }
                /* Else assume compressed address and control fields so
                 * fall through to get the protocol... */
            case PDCONTROL:                 /* Process control field. */
                /* If we don't get a valid control code, restart. */
                if (curChar == PPP_UI) {
                    pc->inState = PDPROTOCOL1;
                	break;
                }
#if 0
                else {
                    PPPDEBUG((LOG_WARNING,
                                "pppInProc[%d]: Invalid control <%d>\n", pd, curChar));
                    pc->inState = PDSTART;
                }
#endif
            case PDPROTOCOL1:               /* Process protocol field 1. */
                /* If the lower bit is set, this is the end of the protocol
                 * field. */
                if (curChar & 1) {
                    pc->inProtocol = curChar;
                    pc->inState = PDDATA;
                }
                else {
                    pc->inProtocol = (u_int)curChar << 8;
                    pc->inState = PDPROTOCOL2;
                }
                break;
            case PDPROTOCOL2:               /* Process protocol field 2. */
                pc->inProtocol |= curChar;
                pc->inState = PDDATA;
                break;
            case PDDATA:                    /* Process data byte. */
                /* Make space to receive processed data. */
                if (pc->inTail == NULL || pc->inTail->len == PBUF_POOL_BUFSIZE) {
#if 0
		    if(pc->inTail) {
			pc->inTail->tot_len = pc->inTail->len;
			if (pc->inTail != pc->inHead) {
			    pbuf_cat(pc->inHead, pc->inTail);
			}
		    }
#endif
                    /* If we haven't started a packet, we need a packet header. */
                    nextNBuf = pbuf_alloc(PBUF_POOL_BUFSIZE);
                    if (nextNBuf == NULL) {
                        /* No free buffers.  Drop the input packet and let the
                         * higher layers deal with it.  Continue processing
                         * the received pbuf chain in case a new packet starts. */
                        PPPDEBUG((LOG_ERR, "pppInProc[%d]: NO FREE MBUFS!\n", pd));
						MP_DEBUG("pppInProc[%d]: NO FREE MBUFS!\n", pd);
#if LINK_STATS
						lwip_stats.link.memerr++;
#endif /* LINK_STATS */
                        pppDrop(pc);
                        pc->inState = PDSTART;  /* Wait for flag sequence. */
			break;
                    }
					//MP_DEBUG("alloc nextBuf = %x", nextNBuf);
		    if (pc->inHead == NULL) {
			struct pppInputHeader *pih = nextNBuf->data;

			pih->unit = pd;
			pih->proto = pc->inProtocol;

			nextNBuf->len += sizeof(*pih);

			pc->inHead = nextNBuf;
		    }
		    pc->inTail = nextNBuf;
                }
                /* Load character into buffer. */
                ((u_char*)pc->inTail->data)[pc->inTail->len++] = curChar;
                break;
            }

            /* update the frame check sequence number. */
            pc->inFCS = PPP_FCS(pc->inFCS, curChar);
        }
    }
	avRandomize();
	//MP_DEBUG("%s end", __FUNCTION__);
}
#endif

void ShowMessage(){
#if 0
	BYTE* temp;
	temp = readPtr;
	if(temp != 0x00){
		while(*temp != 0x00){
			MP_DEBUG("%s",temp);
			while(*temp!= 0x00)
				temp++;
			if(*(temp+1)!= 0x00)
				temp++;
		}
		readPtr = temp;
	}
	else
		MP_DEBUG("No signal back");
#endif

}

/*
 * @param show  Show signal info on UI if true
 */
bool SignalTest(bool show)
{
	int ret;
	BYTE buf[16];
	BYTE resp[32];
	uint32_t rssi;
	uint32_t ber;
	
	strcpy(buf, "at+csq\r");
	UartOutText(buf);

	ret =  __edge_setcmd2(buf, "+CSQ:", resp, sizeof resp, 30);
	if(ret < 0)
		return ret;

//	st_signal.ber = st_signal.rssi = 0;
	if(ret)
	{
		ret = sscanf(resp, "+CSQ: %u,%u", &rssi, &ber);
		mpDebugPrint("[CSQ] ret %d", ret);
		#if USBOTG_HOST_DATANG
		TaskSleep(3000);
		sendAtcmd("at^dtms?");
		#endif
		if(ret == 2)
		{
			st_signal.rssi = rssi;
			st_signal.ber = ber;
			mpDebugPrint("rssi=%d, ber=%d", st_signal.rssi, st_signal.ber);
			if (show)
#if CDPF_ENABLE
				ShowCurSignal();
#else
				;
#endif
			if (rssi == 99)                     /* unknown */
				return false;
			else
				return true;
		}
	}

//	CCED_Test();		//for test
//	ShowCurSignal();
	return false;
}

void SignalTest_Request(void)
{
	EventSet(u08EDGETaskEvent_ID, Signal_Test);
}

void EdgeStatus_Request(void)
{
	EventSet(u08EDGETaskEvent_ID, EdgeStatus_Read);
}

void CeerRead_Request(void)
{
	EventSet(u08EDGETaskEvent_ID, Ceer_Read);
}

#if 0
void CCED_Test()
{
	BYTE * temp;
	BYTE buf[16];
	BYTE buf2[128];
	int value[13];
	
	strcpy(buf, "AT+CCED=0,1\r\n");
	UartOutText(buf);
	Uart1Write(buf, strlen(buf));
	TaskSleep(300);

	memset(value, 0xff, sizeof(value));
	memset(buf2, 0, sizeof(buf2));
	temp = readPtr;
	if ( writePtr < readPtr )
	{
		memcpy(endPtr, startPtr, writePtr - startPtr);
		temp = strstr(temp, buf);
	}
	else
	{
		temp = strstr(temp, buf);
		if(temp > writePtr)
			temp = NULL;
	}
	if(temp)
	{
		temp += strlen(buf);
		if ( !strncmp(temp, "\r\n+CCED: ", sizeof("\r\n+CCED: ")-1))
		{
			BYTE *pbEnd;
			temp += sizeof("\r\n+CCED: ")-1;
			pbEnd = strstr(temp, "\r\n");
			if (pbEnd)
			{
				strncpy(buf2, temp, pbEnd-temp);
				{
					int i, size;
					BYTE *pbComma, *pStart;
					BYTE bStr1[32];
					pStart = buf2;
					for ( i = 0; i < 13; i++ )
					{
						memset(bStr1, 0, sizeof(bStr1));
						pbComma = strchr(pStart, ',');
						if ( pbComma )
						{
							size = pbComma-pStart;
							strncpy(bStr1, pStart, (size < sizeof(bStr1)) ? size : (sizeof(bStr1)-1) );
							pStart = pbComma+1;
						}
						else
						{
							size = strlen(pStart);
							strncpy(bStr1, pStart, (size < sizeof(bStr1)) ? size : (sizeof(bStr1)-1) );
						}
						//mpDebugPrint("bStr1 = %s", bStr1);
						if ( bStr1[0] )
							value[i] = atoi(bStr1);
						else
							value[i] = -1;
						//mpDebugPrint("value[%d] = %d", i, value[i]);
					}
				}
			}
		}		
	}

	frequency = value[5];
	
	readPtr = temp;	
	readPtr += strlen(temp);
	check_readPtr();

	mpDebugPrint(buf2);
}

const BYTE* _GetCurFrequency()
{
	if ( frequency < 0 )
		return "(none)";
	if ( frequency >=1 && frequency <= 124 )
		return "P900";
	if ( frequency >= 975 && frequency <= 1023 || frequency==0 )
		return "E900";
	if ( frequency >= 128 && frequency <= 251 )
		return "850";
	if ( (frequency >= 512 && frequency <= 885) && user_config.band==5 )
		return "DCS1800";
	if ( (frequency >= 512 && frequency <= 810) && user_config.band==4  )
		return "PCS1900";
	return "(none)";
}

#endif

void SetUartRx(){
	{
		if (pppMainTaskWaiting)
			EventSet(u08PPPUART_ID, 1);
		else
			EventSet(u08PPPUART_RX_ID, 0x1);
	}
}

void init_Pointer()
{
	tempbuffer = atbuf;

	if(!tempbuffer)
	{
		MP_DEBUG("memory allocate fail");
		BREAK_POINT();
	}

	writePtr = tempbuffer;
	readPtr = tempbuffer;
//	startPtr = tempbuffer;
//	endPtr = startPtr + buffer_size;

	MP_DEBUG("startPtr = %x, endPtr = %x", startPtr, endPtr);
}

#if UART_EDGE

int tot;
void UartGetInt(DWORD cause)
{
	int count = 0;

	while(WaitUart1Status(C_RXTHR_HIT)==PASS)
	{
		*writePtr++ = GetUart1Char();
		if(writePtr == endPtr)
			writePtr = startPtr;
		count++;
		if(count > 5000)
			break;
	}

	SetUartRx();
}
#endif

int iHasNewSMS;


GetSMSFlag(){
	return iHasNewSMS;
}

ResetSMSFlag(){
	iHasNewSMS = FALSE;
}

SetSMSFlag(){
	mpDebugPrint("@@@@@@@@ SetSMSFlag");
	iHasNewSMS = TRUE;
}


int InsertEDGETimer(U32 dwOffsetValue, void* f, void(* Action)(void*)){
	int i;
	for(i = 0; i <	EDGETimerSize;i++){
		if(!EDGETimerBank[i].used){
            EDGETimerBank[i].u32Interval = dwOffsetValue;
            EDGETimerBank[i].u32Counter = 0;
            EDGETimerBank[i].proc = Action;
            EDGETimerBank[i].arg = f;
			EDGETimerBank[i].used = TRUE;	
			break;
		}
	}
}

int RemoveEDGETimer( void(* Action)(void*)){
	int i;
	for(i = 0; i <	EDGETimerSize;i++){
		if(EDGETimerBank[i].used && EDGETimerBank[i].proc == Action){
			EDGETimerBank[i].used = FALSE;	
		}
	}
}

void EDGETIMER_TASK(){
	int i;

	while(1){
				TaskSleep(2000);
		for(i = 0; i <  EDGETimerSize;i++){
			if(EDGETimerBank[i].used){
				EDGETimerBank[i].u32Counter += 2;
				if(EDGETimerBank[i].u32Counter >= EDGETimerBank[i].u32Interval)
				{
					(EDGETimerBank[i].proc)(EDGETimerBank[i].arg);
					EDGETimerBank[i].used = FALSE;
				}
			}
		}
	}
}

/**
 *
 * @ingroup PPP
 * @brief debug function to list all sms messages in sms queue
 *
 */
void list_sms(){
	int index = 1;
	if(!list_empty(&SMS_queue)){
		struct sms_message *message;

		message = (struct sms_message*)SMS_queue.next;

		while((struct list_head*)message != &SMS_queue)
		{
			MP_DEBUG("===== Message %d =====", index);
			MP_DEBUG("index : %s", message->num);
			if (message->content)
				MP_DEBUG("content: %s", message->content);
			else
			{
				MP_DEBUG("sender : %s", message->sender);
				MP_DEBUG("rec_time: %s", message->time);
			}

			message = (struct MailNode*)message->list.next;
			index++;
		}
	}
}

int has_new_sms(){
	//first mail body
	int index = 1;
	if(!list_empty(&SMS_queue)){
		struct sms_message *message;

		message = (struct sms_message*)SMS_queue.next;
		
		while((struct list_head*)message != &SMS_queue)
		{
			MP_DEBUG("===== Message %d =====", index);
			MP_DEBUG("sender : %s", message->sender);
			MP_DEBUG("rec_time: %s", message->time);
			MP_DEBUG("content: %s", message->content);

			message = (struct MailNode*)message->list.next;
			index++;
		}
	}

	if(index > 1)
		return TRUE;
}

void check_readPtr()
{
#if 1 // origin
	if (readPtr >= endPtr)
	{
		readPtr -= buffer_size;
	}

	#if 0 //jeffery add , for debug, open this to check if readPtr out of the boundry
	if (readPtr >= endPtr)
	{
		mpDebugPrint("@@@@@ ERROR  readPtr >= endPtr; r= %x w= %x", readPtr,writePtr);
	}	
       else if (readPtr < startPtr)
	{
		mpDebugPrint("##### ERROR  readPtr < startPtr; r= %x w= %x", readPtr,writePtr);
	}
	#endif
#else
//	mpDebugPrint("1 readPtr %x writePtr %x", readPtr,writePtr);
	while (readPtr >= endPtr)
	{
		readPtr -= buffer_size;
	}

	if (readPtr < startPtr)
	{
		mpDebugPrint(" ###############  Something error");
		mpDebugPrint("2 readPtr %x writePtr %x", readPtr,writePtr);
		//readPtr = startPtr;
		readPtr = writePtr;
	}
#endif	
}
#if 0
BOOL __edge_setcmd(const unsigned char * buf, unsigned char * strResponse,int wait_times)
{
	Uart1Write(buf, strlen(buf));

	BYTE* temp;
	int response_len = strlen(strResponse);	
	
	while(1)
	{

		if(wait_times)
			wait_times--;
		else
			return FALSE;

		TaskSleep(1000);

		temp = readPtr;
		if(writePtr < readPtr)
		{
			memcpy(endPtr, startPtr, writePtr - startPtr);
			temp = strstr(temp, buf);
		}
		else
		{
			temp = strstr(temp, buf);
			if(temp > writePtr)
				temp = NULL;
		}

		if(temp)
		{
			temp += strlen(buf);

			//if(strncmp(temp, "\r\nOK\r\n", strlen("\r\nOK\r\n"))== 0){
			//	readPtr = temp + strlen("\r\nOK\r\n");
			if(strncmp(temp, strResponse, response_len)== 0)
			{
				readPtr = temp + response_len;
				check_readPtr();
			#if 0	
				if(readPtr >= endPtr)
				{
					readPtr -= buffer_size;				
				}
			#endif
				return TRUE;
			}
			else if(!wait_times)
			{
				while(*temp != 0x00)
					temp++;
				readPtr = temp;
				check_readPtr();
				#if 0
				if(readPtr >= endPtr)
				{
					readPtr -= buffer_size;				
				}
				#endif
				return FALSE;
			}
		}		
	}
}
#endif

inline BOOL __edge_simple_cmd(const unsigned char * buf)
{
#ifdef LINUX
	return __edge_setcmd(buf, "\r\nOK\r\n",3);
#else
	return __edge_setcmd2(buf, NULL,NULL, 0, 60);
#endif
}

/**
 * Set up APN (Access Point Name)
 *
 * @ingroup PPP
 * @brief set network service name 
 *
 * @return 1 : OK, 0: FAIL
 */
int setServiceName(char *service_name)
{
	int ret;
	unsigned char buf[64];
	
	memset(buf, '\0', 40);
//	sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"\r\n", user_config.service_name);
	snprintf(buf, sizeof buf, "at+cgdcont=1,\"IP\",\"%s\"\r", service_name);		
	UartOutText(buf);
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("at+cgdcont OK!");
	else
		mpDebugPrint("at+cgdcont Fail!");
	return ret;
}

bool setGprsAttach(int attach)
{
	int ret;
	unsigned char buf[40];

	sprintf(buf, "AT+CGATT=%d\r", attach);
	UartOutText(buf);

	ret =  __edge_simple_cmd(buf);
	if ( ret )
	{
		if (attach)
			mpDebugPrint("GPRS attach OK");
		else
			mpDebugPrint("GPRS detach OK");
		return true;
	}
	else
	{
		if (attach)
		{
			mpDebugPrint("GPRS attach failed");
#ifdef BILL1
			getCEERcmd();
#endif
		}
		else
			mpDebugPrint("GPRS detach failed");
		return false;
	}
}

/**
 * @ingroup PPP
 * @brief delete one sms message
 *
 * @param index sms message index
 *
 * @return 1 : OK, 0: FAIL
 */
int smsdelete(int index){
	unsigned char buf[40];

	if(index == 0)
		sprintf(buf, "at+cmgd=1,1\r");          /* delete all READ messages */
	else
		sprintf(buf, "at+cmgd=%d\r", index);
	
	return __edge_simple_cmd(buf);
}


/**
 *
 * @ingroup PPP
 * @brief reset modem 
 *
 * @return 1 : OK, 0: FAIL
 */
int setreset()
{
	int ret;
	unsigned char buf[40];
	
	sprintf(buf, "%s", "at+cfun=1\r");

	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("at+cfun=1 OK!");
	else
		mpDebugPrint("at+cfun=1 Fail!");
	return ret;
}

/**
 * @ingroup PPP
 * @brief set modem band
 *
 * @return 1 : OK, 0: FAIL
 */
int setband()
{
	int ret;
	unsigned char buf[40];

	memset(buf, '\0', 40);
	sprintf(buf, "at+wmbs=%d,1\r", user_config.band);
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("set band OK!");
	else
		mpDebugPrint("set band Fail!");
	return ret;
}

/**
 * @ingroup PPP
 * @brief set pin code 
 *
 * @return 1 : OK, 0: FAIL
 */
int setPINcode(char *pin)
{
	int ret;
	unsigned char buf[40];

	memset(buf, '\0', 40);
	sprintf(buf, "at+cpin=\"%s\"\r\n", pin);
	UartOutText(buf);

	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("set cpin OK!");
	else
		mpDebugPrint("set cpin Fail!");
	return ret;
}

#if MMS_ENABLE
int setmmsurl(){
	unsigned char buf[40];	

	memset(buf, '\0', 40);
	sprintf(buf, "at+mmsurl=\"%s\"\r\n", user_config.mms_url);
	
	return __edge_simple_cmd(buf);
}

int setmmsproxy(){
	unsigned char buf[40];
	
	memset(buf, '\0', 40);
	sprintf(buf, "at+mmsproxy=\"%s\",%d\r\n", user_config.mms_proxy, user_config.mms_proxy_port);
	
	return __edge_simple_cmd(buf);
}


int setgprsconfig(){
	unsigned char buf[40];

	memset(buf, '\0', 40);
	sprintf(buf, "at+gprsconfig=\"%s\"\r\n", user_config.mms_service_name);
	
	return __edge_simple_cmd(buf);	
}	
#endif

bool getcreg()
{
	int ret;
	BYTE * temp;
	unsigned char buf[20];
	unsigned char resp[40];
	char mode_str[40], stat_str[40], lac[40], cid[40];
    int val, mode;

	strcpy(buf, "at+creg?\r\n");
	UartOutText(buf);

	ret =  __edge_setcmd2(buf, "+CREG:", resp, sizeof resp, 30);

	if(ret)
				{
		ret = sscanf(resp, "+CREG: %[^,],%[^,]", 
				mode_str, stat_str);
//		mpDebugPrint("[CREG] ret %d", ret);
		if(ret == 2) {
			mode = atoi(mode_str);
			bRegisterState = atoi(stat_str);
			mpDebugPrint("AT+CREG?: mode=%d,stat=%d", mode, bRegisterState);
			return true;
		}
	}
	return false;
}

int setcreg(int val)
{
	int ret;
	unsigned char buf[20];

	sprintf(buf, "at+creg=%d\r", val);
	UartOutText(buf);

	ret = __edge_simple_cmd(buf);
	if ( ret )
	{
		mpDebugPrint("setcreg OK");
	}
	else
	{
		mpDebugPrint("setcreg Fail");
	}

	return ret;
}

bool getcced()
{
	int ret;
	BYTE * temp;
	unsigned char buf[20];
	unsigned char resp[40];
	char mode_str[40], stat_str[40], lac[40], cid[40];
    int val, mode;

	strcpy(buf, "AT+CCED=0,16\r\n");
	UartOutText(buf);

//	ret =  __edge_setcmd2(buf, "AT+CCED=0,16", resp, sizeof resp, 30);
	ret = __edge_setcmd2(buf, NULL,NULL, 0, 60);
#if 0
    	bRegisterState = 4;                         /* unknown */
	if(ret)
	{
		ret = sscanf(resp, "+CREG: %[^,],%[^,]", 
				mode_str, stat_str);
//		mpDebugPrint("[CREG] ret %d", ret);
		if(ret == 2) {
			mode = atoi(mode_str);
			bRegisterState = atoi(stat_str);
			mpDebugPrint("AT+CREG?: mode=%d,stat=%d", mode, bRegisterState);
			return true;
		}
	}
	return false;
#endif	
}

int setwfm()
{
	int ret;
	unsigned char buf[40];
	
	sprintf(buf, "%s", "at+wfm=1,\"B\"\r");
	
	ret = __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("setwfm OK");
	else
		mpDebugPrint("setwfm Fail !");
	return ret;
}

bool getCEERcmd()
{
	int ret;
	unsigned char buf[20];
	unsigned char resp[80];

	strcpy(buf, "at+ceer\r");
	UartOutText(buf);

	ret =  __edge_setcmd2(buf, "+CEER:", resp, sizeof resp, 0);

	if(ret)
	{
		log_printf(1, "%s", resp);
		mpDebugPrint("%s", resp);
		return true;
	}
	return false;
}

int Attention()
{
	int ret;
	char buf[20];

	sprintf(buf, "at\r");
	ret = __edge_simple_cmd(buf);
	return ret;
}

int setwind()
{
	int ret;
	unsigned char buf[40];
	
	memset(buf, '\0', 40);
	sprintf(buf, "at+wind=%d\r", user_config.wind);
	
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("setwind OK");
	else
		mpDebugPrint("setwind Fail !");
	return ret;
}

int setstsf()
{
	int ret;
	unsigned char buf[40];	
	
	memset(buf, '\0', 40);
	sprintf(buf, "at+stsf=%d\r", user_config.stsf);
	
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("setstsf OK");
	else
		mpDebugPrint("setstsf Fail !");
	return ret;
}

int setcnmi()
{
	int ret;
	unsigned char buf[40];
	
	sprintf(buf, "at+cnmi=2,1,0,0,0\r");
	UartOutText(buf);
	
	//ret =  __edge_simple_cmd(buf);
	ret = __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("setcnmi OK");
	else
		mpDebugPrint("setcnmi Fail !");
	return ret;
}

int setcmee(int cmee_val)
{
	int ret;
	unsigned char buf[40];
	
	sprintf(buf, "at+cmee=%d\r", cmee_val);
	
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("setcmee OK");
	else
		mpDebugPrint("setcmee Fail !");
	return ret;
}

/*
 * GPRS event reporting
 */
int setcgerep(int cgerep_val)
{
	int ret;
	unsigned char buf[40];

	sprintf(buf, "at+cgerep=%d\r", cgerep_val);
	
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("cgerep OK");
	else
		mpDebugPrint("cgerep Fail !");
	return ret;
}

int getcmee()
{
	int ret;
	unsigned char buf[40];
	unsigned char resp[40];
	
	sprintf(buf, "at+cmee=?\r");
	
	ret =  __edge_setcmd2(buf, "+CMEE:", resp, sizeof resp, 30);
	if ( ret )
		mpDebugPrint("getcmee OK: %s", resp);
	else
		mpDebugPrint("getcmee Fail !");
	return ret;
}

int isWOPENready()
{
	int ret,val = 0;
	unsigned char buf[40];
	unsigned char resp[40];

	sprintf(buf, "%s", "at+wopen?\r");

#ifdef LINUX
	ret =  __edge_setcmd(buf, "\r\n+WOPEN: 1\r\n", 3);
#else
	ret =  __edge_setcmd2(buf, "+WOPEN:", resp, sizeof resp, 3);
#endif
	if ( ret )
    {
		mpDebugPrint("[WOPEN]: %s", resp);
		if((1 == sscanf(resp, "+WOPEN:%u", &val)) ) {
			mpDebugPrint("+wopen= %u", val);
		}
    }

    if (val == 1)
		mpDebugPrint("+wopen=1 ready");
	else
		mpDebugPrint("+wopen=1 not ready");
	return val;
}

int setwopen(int value)
{
	int ret;
	unsigned char buf[40];		

	unsigned char * strResponse;
	
	if (value == 1)
		strResponse = "\r\nOK\r\n\r\nWaiting for MMS notification message : \r\n";
		
	sprintf(buf, "at+wopen=%d\r", value);

	if (value == 1)
		ret = __edge_simple_cmd(buf);
	else
		ret =  __edge_simple_cmd(buf);

	return ret;	
}

//Boost module CPU performance
int setWCPS()
{
	int ret;
	unsigned char buf[40];
	
	memset(buf, '\0', 40);
	sprintf(buf, "AT+WCPS=0,1\r");
	ret =  __edge_simple_cmd(buf);
	if ( ret )
		mpDebugPrint("set band OK!");
	else
		mpDebugPrint("set band Fail!");
	return ret;
}


#if UART_EDGE
extern int wgprs_state;
bool getWGPRS()
{
	static int rssi_interval;

	mpDebugPrint("[getWGPRS]");
	
	if (wgprs_state != GetGPRSState())
	{
		SignalTest_Request();
		rssi_interval = 0;

#if CDPF_ENABLE
		//Show WGPRS Status Grad, please modify below
		CDPF_SetCdpfUiEvent(CDPF_UI_EVENT_SHOW_SIGNAL);
#endif
/*
		Idu_OsdErase();
		Idu_OsdPaintArea(144, 484, 400, 80, OSD_LIGHT_FADE_B);

		switch(GetGPRSState()){
			case 0:
			Idu_OsdPutStr(Idu_GetOsdWin(), "No connection",
						174, 500, SETUP_WHITE);
				break;
			case 1:
			Idu_OsdPutStr(Idu_GetOsdWin(), "Connected to GPRS ....",
						174, 500, SETUP_WHITE);
				break;
			case 2:
			Idu_OsdPutStr(Idu_GetOsdWin(), "Connected to EDGE ....",
						174, 500, SETUP_WHITE);
				break;		
		}
*/
	}
	else if (rssi_interval++ > 5)
	{
		SignalTest_Request();
		rssi_interval = 0;
	}

	return true;
}

bool setWGPRS(void)
{
	int ret;
	unsigned char buf[20];

	strcpy(buf, "AT+WGPRS=9,1\r\n");
	UartOutText(buf);

	ret =  __edge_simple_cmd(buf);

	if(!ret)
		return false;

	return true;
}
#endif

/**
 * @ingroup PPP
 * @brief see if pin code is correct 
 *
 * @return 1 : OK, 0: FAIL
 */
int isPINready(bool OK_required)
{
	unsigned char buf[40];
	char resp[32];
	int ret, rc = FALSE;

	sprintf(buf, "%s", "at+cpin?\r");

#ifdef LINUX
	ret = __edge_setcmd(buf, "+CPIN: READY", 3);	
#else
	if (OK_required)
		ret = __edge_setcmd2(buf, "+CPIN:",resp, sizeof resp, 60);
	else
		ret = __edge_setcmd4(buf, "+CPIN:",resp, sizeof resp, 60);
	if ( ret )
    {
		mpDebugPrint("[CPIN]: %s", resp);
		if((1 == sscanf(resp, "+CPIN: %s", buf)) ) {
			if (strcmp(buf, "READY") == 0)
				rc = TRUE;
		}
    }
#endif
	return rc;

}

/**
 * @ingroup PPP
 * @brief free all sms messages in sms queue
 *
 */
void free_all_sms_messages()
{
	//free all
	struct sms_message *message;

	mpx_SemaphoreWait(u08EDGESema_ID);
	while (!list_empty(&SMS_queue)) {
		message = list_first_entry(&SMS_queue, struct sms_message, list);
		list_del(&message->list);
		total_sms_message--;

		mpx_Free(message->content);
		mpx_Free(message);
	}
	mpx_SemaphoreRelease(u08EDGESema_ID);
}

int DeleteSMS(struct sms_message* target_message){
	mpx_SemaphoreWait(u08EDGESema_ID);
	list_del((struct list_head *) target_message);
	//smsdelete(target_message->num);
	total_sms_message--;
	mpx_SemaphoreRelease(u08EDGESema_ID);
}


char *next_token(const char *src, char *dst, int size, int *ret_len)
{
	char *ch, *ret;
	int len;
	
	*ret_len = 0;

	ch = strchr(src, ',');
	if (ch)
	{
		len = (unsigned long)ch - (unsigned long)src;
		if (len < size)
		{
			if (len > 0)
				memcpy(dst, src, len);
			dst[len] = '\0';
		}
		else
		{
			MP_ALERT("next_token(1): dest buf is too small");
			len = -1;
		}
		ret = ch;
	}
	else
	{
		len = strlen(src);
		ret = src + len;
		if (len < size)
		{
			strcpy(dst, src);
		}
		else
		{
			MP_ALERT("next_token(2): dest buf is too small");
			len = -1;
		}
	}

	if (len > 0)
		*ret_len = len;
	return ret;
}

/* Format of the data part:
 * [SqnNum(1-10)|DownloadTicket|FIC|EmailCountNew|EmailCountDispatched|AvailableMediaCount|Sender email address] 
 * [1|S0PE0HPH9100224064207A00100049515-04|8901650500000004024|6|3|300|support@panhub.net]
 */
int sms_data(const char *sms)
{
	struct sms_message* msg;
	char *data;
	int len;
	uint16_t mail_cnt, seqno;
	char ticket[40], fic[24];
	int ret;
	msg = mpx_Zalloc(sizeof(struct sms_message));
	if (!msg)
		goto fail;

	MP_DEBUG("[SMS] data=%s", sms);

	len = strlen(sms);

	data = mpx_Malloc(len+1);
	if (!data)
		goto fail;

	strcpy(data, sms);

	memset(&msg->num, 0xaa, sizeof msg->num);

	msg->content = data;

	mpx_SemaphoreWait(u08EDGESema_ID);
	list_add_tail(&msg->list, &sms_list);
	sms_list.num_messages++;
	mpx_SemaphoreRelease(u08EDGESema_ID);

	ret = sscanf(data, "[%hu|%[^|]|%[^|]|%hu", 
				&seqno, ticket, fic, &mail_cnt);
	if (ret == 4)
	{
#if HAVE_CDPF
		cdpf_updNewMailCount(mail_cnt);
#endif
		MP_DEBUG("[SMS] EmailCountNew=%u", mail_cnt);
	}
	return 0;
fail:
	if (msg)
		mpx_Free(msg);
	return -1;
}

int sms_parse(const char *sms)
{
	struct sms_message* msg;
	char buf[64];
	char *end;
	int len;

	msg = mpx_Zalloc(sizeof(struct sms_message));
	if (!msg)
		goto fail;

	end = sms + strlen(sms);

	sms = strchr(sms, ' ');
	if (!sms)
		goto fail;

	sms++;

	sms = next_token(sms, buf, sizeof buf, &len);
	if (len <= 0)
		goto fail;

	msg->num = atoi(buf);

	sms++;

	/* skip <stat> */
	sms = next_token(sms, buf, sizeof buf, &len);
	if (len <= 0)
		goto fail;

	sms++;

	sms = next_token(sms, buf, sizeof buf, &len);
	if (len <= 0)
		goto fail;
	snprintf(msg->sender,sizeof msg->sender,"%s",buf);

	sms++;

	/* skip <alpha> */
	sms = next_token(sms, buf, sizeof buf, &len);

	sms++;

	/* <scts> */
	sms = next_token(sms, buf, sizeof buf, &len);
	if (len > 0)
		snprintf(msg->time,sizeof msg->time,"%s",buf);
	else
		msg->time[0] = '\0';

	mpx_SemaphoreWait(u08EDGESema_ID);
	list_add_tail(&msg->list, &sms_list);
	sms_list.num_messages++;
	mpx_SemaphoreRelease(u08EDGESema_ID);

	return 0;
fail:
	if (msg)
		mpx_Free(msg);
	return -1;
}

/*
 * +CMGR callback
 */
int sms_parse_cmgr(const char *sms, int index)
{
	struct sms_message* msg;
	char buf[64];
	char *end;
	int len;

	msg = mpx_Zalloc(sizeof(struct sms_message));
	if (!msg)
		goto fail;

	msg->num = index;

	end = sms + strlen(sms);

	sms = strchr(sms, ' ');
	if (!sms)
		goto fail;

	sms++;

	/* skip <stat> */
	sms = next_token(sms, buf, sizeof buf, &len);
	if (len <= 0)
		goto fail;

	sms++;

	/* <oa> */
	sms = next_token(sms, buf, sizeof buf, &len);
	if (len <= 0)
		goto fail;
	snprintf(msg->sender,sizeof msg->sender,"%s",buf);

	sms++;

	/* skip <alpha> */
	sms = next_token(sms, buf, sizeof buf, &len);

	sms++;

	/* <scts> */
	sms = next_token(sms, buf, sizeof buf, &len);
	if (len > 0)
		snprintf(msg->time,sizeof msg->time,"%s",buf);
	else
		msg->time[0] = '\0';

	mpx_SemaphoreWait(u08EDGESema_ID);
	list_add_tail(&msg->list, &sms_list);
	sms_list.num_messages++;
	mpx_SemaphoreRelease(u08EDGESema_ID);

	return 0;
fail:
	if (msg)
		mpx_Free(msg);
	return -1;
}

/**
 * @ingroup PPP
 * @brief check sms message received by modem
 *
 * @param index specific sms message's index
 *
 * @return 1 : OK, 0: FAIL
 */
int check_SMS(int index){	
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

#if 1 
// jeffery 20091123. for Jasper certification
//    DPF Context Activation Failures  
//    retry will at @ 1(initial attempt), 2, 3,4,5, 15, 30,60, 120, 180.... minutes

static DWORD g_dwDelayArry[] = 
{ 1, 1, 1,1,1,10,15,30,60};

static DWORD g_delayCount = (sizeof(g_dwDelayArry) / sizeof(DWORD));
static DWORD g_delayIndex = 0;
static DWORD g_prevConnectTime = 0;
static BOOL g_boReconnectTooFast = FALSE;

void __ppp_reconnect_reset()
{
	g_prevConnectTime = GetSysTime();
	g_delayIndex = 0;
}

void __ppp_reconnect_nextDelay()
{
	g_prevConnectTime = GetSysTime();
	if (g_delayIndex +1 < g_delayCount) 
		g_delayIndex++;
}

DWORD __ppp_reconnect_getDelay()
{
	static BOOL boFirstTime = TRUE;

	if (boFirstTime)
	{
		__ppp_reconnect_reset();
		boFirstTime = FALSE;
		return 0;		
	}

	if (g_delayIndex >= g_delayCount)
	{	
		g_delayIndex = g_delayCount -1;
	}

	return (6000 * g_dwDelayArry[g_delayIndex]);
}

#if 0
DWORD __ppp_reconnect_next_allow_time()
{
	return 
g_prevConnectTime + __ppp_reconnect_getDelay();
}
#endif

#if UART_EDGE
BOOL __ppp_reconnect_check()
{
	char buf[40];
	DWORD dwElapseTime = SystemGetElapsedTime(g_prevConnectTime);
	sprintf(buf, "Elapse = %d", dwElapseTime);
	mpDebugPrint(buf);
	ShowReconnectInfo(buf, 0);
	
	if (dwElapseTime < __ppp_reconnect_getDelay())
	{
		mpDebugPrint("reconnec check = False");
		ShowReconnectInfo("reconnec check = False", 1);
		return FALSE;
	}	
	
	mpDebugPrint("reconnec check = TRUE");
	ShowReconnectInfo("reconnec check = TRUE", 1);
	return TRUE;
}
#endif
#endif

static bool __edge_get_ccid()
{
	BYTE* temp;
	BYTE buf[20];
	char resp[64];
    int ret;
	if ( ccid[0] )
		return;

	mpDebugPrint("--> %s", __func__);
	strcpy(buf, "AT+CCID\r");

	ret =  __edge_setcmd2(buf, "+CCID:", resp, sizeof resp, 60);

    if (ret)
			{
		if((1 == sscanf(resp, "+CCID: \"%[^\"]", ccid)) ) {
            mpDebugPrint("get CCID: %s", ccid);
            return true;
		}
	}	
//    mpDebugPrint("get CCID: tot=%d", tot);
    mpDebugPrint("get CCID: Failed");
	return false;
}

int __edge_get_imei()
{
	BYTE buf[20];
	BYTE resp[24+1];
    int ret;
	
	mpDebugPrint("--> %s", __func__);
	if ( imei[0] )
		return;

	strcpy(buf, "AT+CGSN\r");

    	imei[0] = '\0';
	ret =  __edge_setcmd2(buf, "", imei, sizeof imei, 60);
	mpDebugPrint("get IMEI: %s", imei);
	return ret;
}

bool edge_pdp_context()
{
	BYTE buf[20];
	char resp[64];
    int ret;
    int baud_rate;

	mpDebugPrint("--> %s", __func__);
	strcpy(buf, "atd*99***1#\r");

	ret =  __edge_setcmd2(buf, NULL, resp, sizeof resp, 60);

    if (ret)
	{
		bPdpContextState = true;
		ret = sscanf(resp, "CONNECT %u", &baud_rate);
        if (ret == 1)
            mpDebugPrint("activate PDP Context: OK;Baud=%u", baud_rate);
        else
			mpDebugPrint("activate PDP Context: OK");

		return true;
	}
			else
			{
		bPdpContextState = false;
		mpDebugPrint("activate PDP Context: Failed");
#ifdef BILL1
		getCEERcmd();
#endif
		return false;
			}
}

/*
 * Activate PDP context
 */
int edge_pdp_context2()
{
	int ret;
	unsigned char buf[20];
	char resp[32];
	unsigned int cmee_code;

	strcpy(buf, "at+cgact=1,1\r");
	UartOutText(buf);

	ret =  __edge_setcmd2(buf, NULL, resp, sizeof resp, 30);

	if(!ret)
	{
		ret = sscanf(resp, "+CME ERROR: %u", &cmee_code);
		if(ret == 1) 
		{
			mpDebugPrint("[CGACT] CME ERROR=%d", cmee_code);
			return cmee_code;
		}
	}	

	return 0;
}

void __edge_get_sms(char *buf, int index)
{
    int ret;
	char i_str[4];

	mpDebugPrint("--> %s", __func__);

	snprintf(i_str, sizeof i_str, "%d", index);		

	ret =  __edge_setcmd3(buf, i_str, NULL, 0, 120);

    if (ret)
	{
		if(!index)
			mpDebugPrint("+CMGL: OK");
		else
			mpDebugPrint("+CMGR: OK");
		return;
	}	
	if(!index)
		mpDebugPrint("+CMGL: failed");
	else
		mpDebugPrint("+CMGR: failed");
	return;
}

#if UART_EDGE
void __edge_task_after()
{
	Uart1Write("at+cfun=1\r\n", sizeof("at+cfun=1\r\n"));
	TaskSleep(3000);

	ShowMessage();

	MP_DEBUG("==========================CHECK END======================");

	
	Uart1Write("at+csq\r\n", sizeof("at+csq\r\n"));
	TaskSleep(5000);
	ShowMessage();

	Uart1Write("at+csq\r\n", sizeof("at+csq\r\n"));
	TaskSleep(5000);
	ShowMessage();

	Uart1Write("at+csq\r\n", sizeof("at+csq\r\n"));
	TaskSleep(5000);
	ShowMessage();

	Uart1Write("at+csq\r\n", sizeof("at+csq\r\n"));
	TaskSleep(5000);
	ShowMessage();

	Uart1Write("at+csq\r\n", sizeof("at+csq\r\n"));
	TaskSleep(5000);
	ShowMessage();
	
	unsigned char buf[40];
	sprintf(buf, "at+cgdcont=1,\"IP\",\"%s\"\r\n", user_config.service_name);    
	Uart1Write(buf, strlen(buf));

	TaskSleep(4000);

	ShowMessage();

	Uart1Write("atd*99***1#\r\n", sizeof("atd*99***1#\r\n"));

	MP_DEBUG("done ppp dialup");

	//example for pppOpen and pppClose

	pppOpen(0, NULL, NULL);

	TaskSleep(60000);
}

void __edge_ppp_disconnect()
{	
	if (isPPPup())
		pppClose(0);
	else
		MP_DEBUG("PPP_disconnect: PPP not open");
}

void __edge_ppp_shutdown()		//grad add
{
	Uart1Write("ATH\r\n", sizeof("ATH\r\n")-1);
	TaskSleep(50);
	ShowMessage();
	mpDebugPrint("hang-up call finish.");

	Uart1Write("AT+CPOF\r\n", sizeof("AT+CPOF\r\n")-1);	//need long time
	TaskSleep(2000);
	ShowMessage();
	mpDebugPrint("shutdown modem finish.");
	
	Uart1Write("AT+CPOF=1\r\n", sizeof("AT+CPOF=1\r\n")-1);
	TaskSleep(50);
	ShowMessage();
	mpDebugPrint("power off modem finish.");
}

void __edge_ppp_getRegisterState()	//grad add
{
	getcreg();
}

void EDGE_TASK()
{
	S32 status;
	DWORD dwNWEvent;  

	MP_DEBUG("EDGE_TASK");

	if(NetDefaultNicGet() == NIC_INDEX_NULL)
		init_netpool();

    status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
    if(status < 0)
    {
        DPrintf("[NIC] PPPTask event create fail");
        BREAK_POINT();
    }
    else
        u08EDGETaskEvent_ID = (U08)status;

#if 1 // SMS_ENABLE || MMS_ENABLE	
	status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	if(status < 0)
	{
		DPrintf("[NIC] PPPTask event create fail");
		BREAK_POINT();
	}
	else
		u08EDGESema_ID = (U08)status;
#endif

	INIT_LIST_HEAD(&SMS_queue);
	INIT_LIST_HEAD(&sms_list.list);

    init2();
    init3();
	
	EnaUart1Int();

	short cnt = 2;
	do {
		if (q2687_ready())
			break;
		TaskSleep(100);
	} while (cnt--);

#if 0
	//AT
	Attention();
#endif

	setcmee(1);
	setcgerep(2);

#if MMS_Enable
	if(!isWOPENready()){
		MP_DEBUG("set wopen 1");
		MP_DEBUG("ret = %d",setwopen(1));
	}
#else
	setwopen(0);
#endif


#if EDGE_DEBUG
	getcced(); // jeffery test
#endif
	memset(imei, 0, sizeof(imei));
	memset(ccid, 0, sizeof(ccid));
	__edge_get_imei();
	if (__edge_get_ccid())
		gsm_sim_present(true);

	//set band
	MP_DEBUG("set band");

	//if (user_config.band != 4) //Frank Kao 011910. In this project, when band = 4 and AT&T SIM card is used, the band is set to 4 by default
		setband();
	
#if EDGE_DEBUG
	__edge_simple_cmd("at+wmbs?\n\r");
#endif

	MP_DEBUG("set wfm");
	setwfm();

#if 0 //MMS_ENABLE // ???    //Frank Kao 011910 this two commands should not be set, they are configured by module 
	MP_DEBUG("set stsf");
	setstsf();
#endif 

	MP_DEBUG("set wind");

	setwind();

	MP_DEBUG("set wgprs");
	setWGPRS();

	MP_DEBUG("set reset");

	//setreset();              //Frank Kao 011910 this performs reset/restart of module, not necessary

	if (user_config.band != 4 && 
		!strcmp(user_config.service_name, "internet") ) //Frank Kao 011910. In this project, when band = 4 and AT&T SIM card is used, there is no need to set pin code
	{													//china mobile not need set PIN
		if(!isPINready(false))
		{
			MP_DEBUG("set code");
			setPINcode(user_config.pin_number);
		}
		else
		{
			mpDebugPrint("Pin ready.");
		}
	}

//	setWCPS();
	
	MP_DEBUG("set service");
	setServiceName(user_config.service_name);

	MP_DEBUG("%x,%x",readPtr, writePtr);

#if MMS_ENABLE
	//SET MMS
	MP_DEBUG("set gprsconfig");
	if(!setgprsconfig()){
		MP_DEBUG("set gprsconfig fail");
	}
	MP_DEBUG("set mmsproxy");
	
	if(!setmmsproxy()){
		MP_DEBUG("set mmsproxy fail");
	}
	MP_DEBUG("set mmsurl");
	if(!setmmsurl()){
		MP_DEBUG("set mmsurl fail");
	}
#endif

#ifdef LINUX
	setcnmi();
#endif

	//setGprsAttach(1);

#ifdef LINUX
	MP_DEBUG("get register state");
	getcreg();
#else
	setcreg(1);
#endif
#if EDGE_DEBUG
	getcced();
	
	__edge_simple_cmd("at+wmbs?\n\r");
#endif	

#if 0
	SignalTest(true);
#else
	Signal_Test_NumRetries = 10;
	SignalTest_Request();
#endif
	ResetSMSFlag();
	
	gsm_sm_start();

	avRandomInit();

	boModemInitialized = TRUE;
	run_edge_task();

	__edge_task_after();
RE_CONNECT:	
	
//	__edge_reconnect();
	goto RE_CONNECT;
}

void EDGE_TASK_ForISP()
{
	S32 status;
	DWORD dwNWEvent;
	
	MP_DEBUG("EDGE_TASK");
		
	pppMainTaskWaiting = 1;
	if(NetDefaultNicGet() == NIC_INDEX_NULL)
		init_netpool();
	
	status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
	if(status < 0)
	{
		DPrintf("[NIC] PPPTask event create fail");
		BREAK_POINT();
	}
	else
		u08EDGETaskEvent_ID = (U08)status;
	
	status = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE|OS_ATTR_EVENT_CLEAR, 0);
	MP_ASSERT(status > 0);
	u08PPPUART_ID = (U08)status;
	
#if 1 // SMS_ENABLE || MMS_ENABLE	
	status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	if(status < 0)
	{
		DPrintf("[NIC] PPPTask event create fail");
		BREAK_POINT();
	}
	else
		u08EDGESema_ID = (U08)status;
#endif
	
	init2();
    init3();
	
	INIT_LIST_HEAD(&SMS_queue);
	
#if UART_EDGE
	HUartRegisterCallBackFunc(HUART_A_INDEX, UartGetInt);
#endif
		
	init_Pointer();
		
	EnaUart1Int();
		TaskSleep(2000);

	short cnt = 20;
	do {
		if (q2687_ready())
			break;
		TaskSleep(100);
	} while (cnt--);

#if 0
	//AT
	Attention();
#endif

	memset(imei, 0, sizeof(imei));
	memset(ccid, 0, sizeof(ccid));
	__edge_get_imei();
	if (__edge_get_ccid())
		gsm_sim_present(true);
		
	boModemInitialized = TRUE;

	while(1)
	{
		EventWait(u08EDGETaskEvent_ID, 0xfff, OS_EVENT_OR, &dwNWEvent);
		
		if(dwNWEvent & PPP_Shutdown)
		{
			__edge_ppp_shutdown();
		}
	}
}
#endif

#if integrated_board
void EDGE_Power_On(){
	GPIO *Gpio;	
	Gpio = (GPIO *)(GPIO_BASE);	

	Gpio->Gpdat0 |= 0x00200020;

}

void showUART(){
	HUART *uart = (HUART *)HUART1_BASE;
	GPIO *Gpio = (GPIO *)(GPIO_BASE);	

	MP_DEBUG("uartc  = %x", uart->HUartC);
	MP_DEBUG("bufctl = %x", uart->HUartBufCtl);
	MP_DEBUG("GPIO->Gpdat0 = %x", Gpio->Gpdat0);

	AddTimerProc(3000, showUART);
}

void EDGE_Power_Off(){
	GPIO *Gpio;	
	Gpio = (GPIO *)(GPIO_BASE);	
	HUART *uart = (HUART *)HUART1_BASE;

	Gpio->Gpdat0 |= 0x00200000;
}

#endif

#if UART_EDGE
void EDGE_init()
{
	S32 status;
	GPIO *Gpio = (GPIO *)(GPIO_BASE);	
	MP_DEBUG("EDGE_init");
#if integrated_board
	EDGE_Power_On();
	MP_DEBUG("GPIO->Gpdat0 = %x", Gpio->Gpdat0);
#endif

	init_Pointer();
	
	HUartRegisterCallBackFunc(HUART_A_INDEX, UartGetInt);

	if(!u08EDGETaskId)
	{
	    status = mpx_TaskCreate(EDGE_TASK, DRIVER_PRIORITY, TASK_STACK_SIZE*4);
	    if(status < 0)
	    {
	        MP_DEBUG("PPP Task create fail %d", status);
	        BREAK_POINT();
	    }
	    u08EDGETaskId = (U08)status;

		mpx_TaskStartup(u08EDGETaskId);
	}

	init_ppp();
}

void EDGE_Init_ForISP()
{
	S32 status;
	GPIO *Gpio = (GPIO *)(GPIO_BASE);	
	MP_DEBUG("EDGE_init");
#if integrated_board
	EDGE_Power_On();
	MP_DEBUG("GPIO->Gpdat0 = %x", Gpio->Gpdat0);
#endif

	if(!u08EDGETaskId)
	{
	    status = mpx_TaskCreate(EDGE_TASK_ForISP, DRIVER_PRIORITY, TASK_STACK_SIZE*8);
	    if(status < 0)
	    {
	        MP_DEBUG("PPP Task create fail %d", status);
	        BREAK_POINT();
	    }
	    u08EDGETaskId = (U08)status;
		mpx_TaskStartup(u08EDGETaskId);
	}

	if(!u08EDGETIMERTaskId)
	{
		status = mpx_TaskCreate(EDGETIMER_TASK, DRIVER_PRIORITY+1, TASK_STACK_SIZE*4);
		if(status < 0)
		{
			MP_DEBUG("PPP Task create fail %d", status);
			BREAK_POINT();
		}
		u08EDGETIMERTaskId = (U08)status;

		mpx_TaskStartup(u08EDGETIMERTaskId);
	}

}
#endif

BOOL IsRegisterStateOK(void)
{
	BYTE state;
	state = bRegisterState;
	if ( state == 1 || state == 5 )
		return TRUE;
	else
		return FALSE;
}

BOOL IsSignalOK(void)
{
	WORD rssi, ber;
	rssi = st_signal.rssi;
	ber  = st_signal.ber;
	mpDebugPrint("IsSignalOK: s=%d,e=%d", rssi, ber);
	if ( rssi == 99)
		return FALSE;
	if ( ber != 0 && ber != 99)
		return FALSE;
	if ( rssi >= 10 )
		return TRUE;
	else
		return FALSE;
}

static int TaskWaitPoolingExitKey(DWORD sleeptime)
{
	DWORD dwStartTime, dwNowTime, dwElapsedTime;
	BYTE bKeyCode = 0;
	
	dwStartTime = SystemGetTimeStamp();
	do
	{
		dwNowTime = SystemGetTimeStamp();
		dwElapsedTime = (DWORD)(dwNowTime - dwStartTime);
		//-------
		TaskYield();
		bKeyCode = Polling_Key();
		TaskYield();
		if ( bKeyCode == KEY_EXIT )
			return EDGE_ERR_EXIT_BY_USER;
		TaskYield();
	}
	while( dwElapsedTime < sleeptime );
	
	return PASS;
}

int EdgeConnect()
{
	int ret;
	extern BOOL boFirstTimeConnect;
	
	EnableNetWareTask();
#if CDPF_ENABLE
	boFirstTimeConnect = false;
	EntityCheckDisable();
#endif
	ret = EdgeConnect2(4 * 60);                 /* (4*60) = 4 min */

	if (ret < 0)
	{
		ntrace_printf(0, "[EDGE] EdgeConnect2 returns errno=%d", edge_errno);
		ntrace_write2sd();
		mpDebugPrint("EdgeConnect2 errno = %d", edge_errno);
        log_printf(1, "EdgeConnect2 errno = %d", edge_errno);
        log_printf(1, "RSSI=%u,BER=%u", st_signal.rssi, st_signal.ber);
		CeerRead_Request();
		SignalTest_Request();
		if (edge_errno)
			return edge_errno;
		else
			return EDGE_ERR_CONNECT_FAIL;
	}	
	else
		return PASS;
}

void EdgeDisconnect()
{
	mpDebugPrint("Edge Disconnect ...");
	pppClose(0);
}

void EdgeShutDown(void)
{
	mpDebugPrint("Edge ShutDown ...");
	EventSet(u08EDGETaskEvent_ID, PPP_Shutdown);
}

void EdgeGetRegisterState(void)
{
	mpDebugPrint("Edge Get Register State ...");
	EventSet(u08EDGETaskEvent_ID, RegisterState_Test);
}

void EdgeGetImeiCcid(BYTE ** _ccid, BYTE** _imei)
{
#if 1 //(CDPF_LOCALE == en_US)
	*_ccid = pb_ccid;
	*_imei = pb_imei;
#else

	//*_ccid = "8901650907002187862";		//8901650907002187862 = a3.1vsmr@panhub.net
	//*_imei = "310650700218786";

	*_ccid = "8901650907002187607";		//8901650907002187607 = a3.1vljz@panhub.net
	*_imei = "310650700218760";
#endif
}

void EdgeGetRealImeiCcid(BYTE ** _ccid, BYTE** _imei)
{
	*_ccid = ccid;
	*_imei = imei;
}

void EdgeGetCurSignalValue(WORD* rssi,WORD* ber)
{
	*rssi = st_signal.rssi;
	*ber  = st_signal.ber;
}

BOOL IsModemInitialized()
{
	return boModemInitialized;
}


#if MMS_ENABLE
void ParseJPG(BYTE* message_start, int length){
	BYTE* filename;
	BYTE* photo = NULL;
	BYTE* temp;

	temp = message_start;
	while(temp <=(message_start + length)){
		//if(strnicmp(temp, "JPG", strlen("JPG"))== 0){
			//photo= temp + strlen("JPG");
			//if(*photo = 0x3e)
			//	photo += 2;
			//else
			//	*photo+= 1;
			//MP_DEBUG("%x", *photo);
			if(*temp == 0xff && *(temp+1) == 0xd8){
				MP_DEBUG("find JPG header");
				photo = temp;
				break;
			}
			//else{
			//	MP_DEBUG("JPG header error");
			//	photo = NULL;
			//}
			//filename = temp + strlen("jpg");
		//}
		temp++;
	}

	if(photo){
		temp = photo;
		while(*temp != 0xff || *(temp + 1) != 0xd9){
			temp++;
			if(temp >= (message_start + length))
				MP_DEBUG("parse JPG error");
		}
		temp += 2;
		*temp = 0x00;

		MP_DEBUG("photo length = %d", temp - photo);

		//SAVE PHOTO
		{
			DRIVE* pstTargetDrv;
			STREAM *tHandle;
			BOOL boExistFileDeleted, iRet;
			WORD name[] = {'m','m','s',0};
			WORD fullname[] = {'m','m','s','.','j','p','g',0};

			mpDebugPrint("save file");
			pstTargetDrv=DriveGet(NAND);
			mpDebugPrint("delete file");
			boExistFileDeleted =  FileDeleteLN_W(pstTargetDrv, fullname);
			if ( boExistFileDeleted )
			{
				mpDebugPrint("same filename exist, delete old file");
			}
			mpDebugPrint("create file");
			iRet = CreateFileW(pstTargetDrv, name, "jpg");

			if (iRet) MP_ALERT("create file fail\r\n");
				tHandle=FileOpen(pstTargetDrv);

			if(!pstTargetDrv)
				MP_ALERT("open file fail\r\n");
			mpDebugPrint("write file");
			iRet=FileWrite(tHandle, photo, temp - photo);

			if(!iRet)
				MP_ALERT("write file fail\r\n");

			FileClose(tHandle);
		}
	}
	
	
}


int StartGetMMS(){
	DWORD dwNWEvent;  
	BYTE* temp, *start = NULL, *end = NULL, *message_start = NULL;;
	int wait_times = 3;
	int length = 0, message_length = 0;

	while(1){

		if(wait_times)
			wait_times--;
		else
			return FALSE;

		TaskSleep(3000);

		temp = readPtr;
		if(writePtr < readPtr){
			length = ((U32)writePtr - (U32)startPtr) + ((U32)endPtr - (U32)readPtr);
			memcpy(endPtr, startPtr, writePtr - startPtr);
		}
		else{
			length = (U32)writePtr - (U32)readPtr;
			if(temp > writePtr)
				temp = NULL;
		}

		MP_DEBUG("length = %d", length);


		while(temp <= (readPtr + length)){
			if(strncmp(temp, "\r\nCONNECT\r\n", strlen("\r\nCONNECT\r\n"))== 0){
				start = temp + strlen("\r\nCONNECT\r\n");
			}
			if(strncmp(temp, "\r\nOK\r\n", strlen("\r\nOK\r\n"))== 0){
				end = temp;
				readPtr = temp + strlen("\r\nOK\r\n");
			}			
			temp++;
		}

		if(start && end){
			message_start = start;
			message_length = end - start;
			MP_DEBUG("GET ALL MMS Message length = %d", message_length);
			NetPacketDump(message_start, message_length);
			ParseJPG(message_start, message_length);
			return TRUE;
		}

#if 0

		if(temp){

			res_ok = temp + length - 6;

			if(memcmp(res_ok, "\r\nOK\r\n", 6) != 0)
				res_ok = 0;


			if(strncmp(temp, "\r\nCONNECT\r\n", strlen("\r\CONNECT\r\n"))== 0){
				readPtr = temp + strlen("\r\CONNECT\r\n");
				if(readPtr > endPtr){
					readPtr -= buffer_size;				
				}

				return TRUE;
			}
			else if(!wait_times){
				while(*temp != 0x00)
					temp++;
				readPtr = temp;
				if(readPtr > endPtr){
					readPtr -= buffer_size;				
				}

				return FALSE;
			}
		}		
#endif
	}
}


int SetMMSCMD(){
	unsigned char buf[40];
	DWORD dwNWEvent;  
	BYTE* temp;
	int wait_times = 3;

	memset(buf, '\0', 40);
	sprintf(buf, "at+mmsrecv\r\n");

	Uart1Write(buf, strlen(buf));

	while(1){

		if(wait_times)
			wait_times--;
		else
			return FALSE;

		TaskSleep(1000);

		temp = readPtr;
		if(writePtr < readPtr){
			memcpy(endPtr, startPtr, writePtr - startPtr);
			temp = strstr(temp, buf);
		}
		else{
			temp = strstr(temp, buf);
			if(temp > writePtr)
				temp = NULL;
		}

		if(temp){
			temp += strlen(buf);

			if(strncmp(temp, "\r\nOK\r\n", strlen("\r\nOK\r\n"))== 0){
				readPtr = temp + strlen("\r\nOK\r\n");
				check_readPtr();
			#if 0	
				if(readPtr >= endPtr){
					readPtr -= buffer_size;				
				}
			#endif
				return TRUE;
			}
			else if(!wait_times){
				while(*temp != 0x00)
					temp++;
				readPtr = temp;
				check_readPtr();
			#if 0	
				if(readPtr >= endPtr){
					readPtr -= buffer_size;				
				}
			#endif
				return FALSE;
			}
		}
	}
}


void ParseMMS(){
	BYTE* temp;
	BYTE* message;
	int message_num;
	int length = 0;
	int i;

	NetPacketDump(readPtr,500);

	temp = readPtr;
	if(writePtr < readPtr){
		memcpy(endPtr, startPtr, writePtr - startPtr);
		length = ((U32)writePtr - (U32)startPtr) + ((U32)endPtr - (U32)readPtr);
	}
	else{
		length = (U32)writePtr - (U32)readPtr;
	}

	MP_DEBUG("length = %d", length);

	TaskSleep(12000);

	if(length > 0){
		if(strncmp(temp, "\r\n-----\r\n", strlen("\r\n-----\r\n")) == 0){
			temp += strlen("\r\n-----\r\n");
			while(strncmp(temp,"\r\n-----\r\n", strlen("\r\n-----\r\n")) != 0){
				temp++;
				if(temp > (readPtr + length)){
					MP_DEBUG("ERROR");
					break;
				}
			}
			temp += strlen("\r\n-----\r\n");
		}
		else
			MP_DEBUG("Message Error");
		if(strncmp(temp, "\r\n+MMSRECV:", strlen("\r\n+MMSRECV:")) == 0){
			MP_DEBUG("find mms");
			temp += strlen("\r\n+MMSRECV:");
			if(*temp == 0x22){
				temp++;
				while(*temp != 0x22){
					temp++;
					if(temp > (readPtr + length)){
						MP_DEBUG("ERROR");
						break;
					}
				}
				readPtr = temp += 3;
				check_readPtr();
				if(!SetMMSCMD())
					MP_DEBUG("set mmsrecv fail");
				i = 5;
				while(i > 0){
					i--;
					MP_DEBUG("fetch %d time", i);
					if(!StartGetMMS())
						MP_DEBUG("recv mms fail");
					else
						break;
				}
			}
		}
		else
			MP_DEBUG("Message Error2");
	}
}
#endif //MMS_ENABLE


//Frank Kao 010110 added for SMS

ClearSMS(){
	BYTE i;

	free_all_sms_messages();

	for(i=1; i<=30; i++){
		smsdelete(i);
		TaskYield();
	}
}

bool SetRegisterState(int state)
{
	if (bRegisterState != state)
	{
		bRegisterState = state;
		return true;
	}
	else
		return false;
}

void send_ppp_event(void *ctx, int errCode, void *arg)
{
	mpDebugPrint("--> %s(err=%d)", __func__, errCode);
	if (errCode == PPPERR_NONE)
	{
		if (!wwan_if.ppp_state)
		{
			wwan_if.ppp_state = true;
			EventSet(u08EDGETaskEvent_ID, EEVENT_PPP_CONNECTED);   /* send signal to GSM call sm */
			EventSet(u08EDGETaskEvent_ID, PEVENT_PPP_STATUS);   /* send signal to WWAN sm */
#if CDPF_ENABLE
			CDPF_SetCdpfUiEvent(CDPF_UI_EVENT_PPP_CONNECTED);
#endif
#if	HAVE_USB_MODEM
			if (ctx)
				NetUsb_PppStatusChange(ctx, true);
			pppUP();
#endif
		}
	}
	else if (errCode == PPPERR_CONNECT ||         			/* connection lost */
			 errCode == PPPERR_USER)
	{
		if (wwan_if.ppp_state)
		{
			wwan_if.ppp_state = false;
			EventSet(u08EDGETaskEvent_ID, EEVENT_PPP_DISCONNECTED);/* send signal to GSM call sm */
			EventSet(u08EDGETaskEvent_ID, PEVENT_PPP_STATUS);   /* send signal to WWAN sm */
#if HAVE_USB_MODEM
			pppDOWN();
#endif
		}
#if HAVE_USB_MODEM
		else
			pppERROR(errCode);
#endif
	}

	EventSet(u08PPPUART_ID, LEVENT_PPP_STATUS);              /* send signal to event loop */
}

int iGPRSState=-1;

void ResetGPRSState()
{
	iGPRSState=-1;
}

void SetGPRSState(int state)
{
	iGPRSState= state;
}

int GetGPRSState()
{
	return iGPRSState;
}

#if PPP_DEBUG > 0
int ppp_debug_level = LOG_INFO;
void ppp_trace(int level, const char *format,...)
{
	va_list ap;
    char buffer[128];
	const int buflen = 128;
	int size;

    if (level > ppp_debug_level)
		return;
    va_start(ap, format);
    size = vsnprintf(buffer, buflen, format, ap);
    va_end(ap);

	mpDebugPrint(buffer);
#if NET_TRACE_ENABLE > 0
	if (buffer[size-1] == '\n')
		buffer[size-1] = '\0';
	if (buffer[size-2] == '\r')
		buffer[size-2] = '\0';
	ntrace_printf(0, "[PPP] %s", buffer);
#endif
}
#endif

#endif /* PPP_SUPPORT */


#if CDPF_ENABLE
int SetBankValueByIniFile(const BYTE* text)
{
	int bank;
	mpDebugPrint("ini BANK = %s", text);

	if ( text == NULL )
		return 0;
	if ( text[0] == 0 )
		return 0;
	if ( !strcasecmp(text, "default") || !strcasecmp(text, "none") || !strcasecmp(text, "auto")
		 || !strcasecmp(text, "no") )
		return 0;
	bank = atoi(text);
	user_config.band = bank;
}

const BYTE* SetApnValueByIniFile(const BYTE* text)
{
	static BYTE newapn[48];
	mpDebugPrint("ini APN = %s", text);

	if ( text == NULL )
		return NULL;
	if ( text[0] == 0 )
		return NULL;
	if ( !strcasecmp(text, "default") || !strcasecmp(text, "none") || !strcasecmp(text, "auto")
		 || !strcasecmp(text, "no") )
		return NULL;
	strncpy(newapn, text, sizeof(newapn)-1);
	newapn[sizeof(newapn)-1] = 0;
	user_config.service_name = newapn;
	return newapn;
}

const BYTE* SetIccidByIniFile(const BYTE* text)
{
	static BYTE newiccid[32];
	mpDebugPrint("ini ICCID = %s", text);

	if ( text == NULL )
		return NULL;
	if ( text[0] == 0 )
		return NULL;
	if ( !strcasecmp(text, "default") || !strcasecmp(text, "none") || !strcasecmp(text, "auto")
		 || !strcasecmp(text, "no") )
		return NULL;
	strncpy(newiccid, text, sizeof(newiccid)-1);
	newiccid[sizeof(newiccid)-1] = 0;
	pb_ccid = newiccid;
	return newiccid;
}

const BYTE* SetImeiByIniFile(const BYTE* text)
{
	static BYTE newimei[32];
	mpDebugPrint("ini IMEI = %s", text);

	if ( text == NULL )
		return NULL;
	if ( text[0] == 0 )
		return NULL;
	if ( !strcasecmp(text, "default") || !strcasecmp(text, "none") || !strcasecmp(text, "auto")
		 || !strcasecmp(text, "no") )
		return NULL;
	strncpy(newimei, text, sizeof(newimei)-1);
	newimei[sizeof(newimei)-1] = 0;
	pb_imei = newimei;
	return newimei;
}

int get_band_value()
{
	return user_config.band;
}

const BYTE* get_apn_value()
{
	return user_config.service_name;
}
#endif

/*
 * AT+COPS=?
 *
 */
char * getcops1(char *buf, int size)
{
	int ret;
	uint16_t mode, format;
	char oper[32];
	char *resp = buf;

	ret =  __edge_setcmd2("AT+COPS=?\r", "+COPS:", resp, size, 30);

	if(ret)
	{
		MP_DEBUG(resp);
	}
	else
	{
		buf[0] = '\0';
	}
	return buf;
}

/*
 * AT+COPS?
 *
 * +COPS: <mode>[,<format>,<oper>]
 */
char *getcops2(char *buf, int size)
{
	int ret;
	uint16_t mode, format;
	char oper[32];
	char *resp = buf;

	ret =  __edge_setcmd2("AT+COPS?\r", "+COPS:", resp, size, 30);

	if(ret)
	{
//		ret = sscanf(resp, "+COPS: %hd,%hd,%s", 
//				&mode, &format, oper);
		MP_DEBUG(resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

/*
 * AT+CIMI
 *
 * IMSI of the SIM card
 * 15 digits number
 */
char * getimsi(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("AT+CIMI\r", NULL, resp, size, 30);

	if(ret)
	{
		MP_DEBUG("+CIMI: %s", resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

/*
 * AT+CGMR
 *
 */
char * getcgmr(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("AT+CGMR\r", NULL, resp, size, 30);

	if(ret)
	{
		MP_DEBUG("+CGMR: %s", resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

/*
 * ATI
 *
 * Request Identification Information I
 *
 */
char * getati(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("ATI\r", NULL, resp, size, 30);

	if(ret)
	{
		MP_DEBUG("ATI: %s", resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

/*
 * AT+CRSM
 *
 * Restricted SIM Access
 *
 * AT+CRSM=176,28542,0,0,11
 */
char * getcrsm1(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("AT+CRSM=176,28542,0,0,11\r", "+CRSM:", resp, size, 30);

	if(ret)
	{
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

/*
 * AT+CRSM
 *
 * Restricted SIM Access
 *
 * AT+CRSM=176,28499,0,0,14
 */
char * getcrsm2(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("AT+CRSM=176,28499,0,0,14\r", "+CRSM:", resp, size, 30);

	if(ret)
	{
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

/*
 * AT+CRSM
 *
 * Restricted SIM Access
 *
 * AT+CRSM=176,28539,0,0,12
 */
char * getcrsm3(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("AT+CRSM=176,28539,0,0,12\r", "+CRSM:", resp, size, 30);

	if(ret)
	{
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';

	return buf;
}

char * getcreg2(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("at+creg?\r", "+CREG:", resp, size, 30);

	if(ret) {
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';
	return buf;
}

char * getcgatt(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("at+cgatt?\r", "+CGATT:", resp, size, 30);

	if(ret) {
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';
	return buf;
}

char * getcgact(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("at+cgact?\r", "+CGACT:", resp, size, 30);

	if(ret) {
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';
	return buf;
}

char * getcgdcont2(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("at+cgdcont?\r", "+CGDCONT:", resp, size, 30);

	if(ret) {
		MP_DEBUG("%s", resp);
	}
	else
		buf[0] = '\0';
	return buf;
}

char * getCEERcmd2(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("at+ceer\r", "+CEER:", resp, size, 0);

	if(ret)
		MP_DEBUG(resp);
	else
		buf[0] = '\0';
	return buf;
}

char * getcsq(char *buf, int size)
{
	int ret;
	char *resp = buf;

	ret =  __edge_setcmd2("at+csq\r", "+CSQ:", resp, size, 30);

	if(ret)
		MP_DEBUG(resp);
	else
		buf[0] = '\0';
	return buf;
}

#if CDPF_ENABLE
/*
 * A code example to print various EDGE status on the console.
 *
 * The following AT commands are called for troubleshooting purpose.
 *
 * at+cops=?
 * At+cops?
 * At+cimi
 * At+Cgmr
 * Ati
 * AT+CRSM=176,28542,0,0,11
 * AT+CRSM=176,28499,0,0,14
 * AT+CRSM=176,28539,0,0,12
 * At+creg?
 * At+cgatt?
 * At+csq
 * At+cgact?
 * At+ceer
 * At+cgdcont?
 */
#if 0
int edge_status(void)
{
#define BUF_SIZE	(80+1)
	char buf[BUF_SIZE];

	mpDebugPrint("%s", __func__);

	if (gsm_connected)
		return -1;

	mpDebugPrint(getcops1(buf, BUF_SIZE));
	mpDebugPrint(getcops2(buf, BUF_SIZE));
	mpDebugPrint(getimsi(buf, BUF_SIZE));
	mpDebugPrint(getcgmr(buf, BUF_SIZE));
	mpDebugPrint(getati(buf, BUF_SIZE));
	mpDebugPrint(getcrsm1(buf, BUF_SIZE));
	mpDebugPrint(getcrsm2(buf, BUF_SIZE));
	mpDebugPrint(getcrsm3(buf, BUF_SIZE));
	mpDebugPrint(getcreg2(buf, BUF_SIZE));
	mpDebugPrint(getcgatt(buf, BUF_SIZE));
	mpDebugPrint(getcsq(buf, BUF_SIZE));
	mpDebugPrint(getcgact(buf, BUF_SIZE));
	mpDebugPrint(getCEERcmd2(buf, BUF_SIZE));
	mpDebugPrint(getcgdcont2(buf, BUF_SIZE));

	return 0;
}
#else
/*
 * Read various EDGE status info
 */
#define MAX_LINE	(80)
char edge_status_buffer[14][MAX_LINE];
int edge_status(void)
{
#define BUF_SIZE	(80+1)
	char buf[BUF_SIZE];
	int len;
	int debug_idx = 0;

	mpDebugPrint("%s", __func__);

	if (gsm_connected)
		return -1;

	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcops1(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcops2(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getimsi(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcgmr(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getati(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcrsm1(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcrsm2(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcrsm3(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcreg2(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcgatt(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcsq(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcgact(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getCEERcmd2(buf, BUF_SIZE)));
	len = snprintf(&edge_status_buffer[debug_idx++], MAX_LINE+1, 
			(getcgdcont2(buf, BUF_SIZE)));

	return 0;
}
#endif


#endif

#if HAVE_USB_MODEM > 0
bool threeg_get_ccid(void)
{
	return __edge_get_ccid();
}
bool threeg_get_imei(void)
{
	return __edge_get_imei();
	//return true;
}
#endif

#if USBOTG_HOST_DATANG
	int sendAtcmd(char *cmd)
{
	int ret;
	unsigned char buf[31];
	char resp[32], oper[16];
	int r;
	int temp_AcT;

	memset(buf, '\0', sizeof buf);
	sprintf(buf, "%s\r", cmd);

	if((strcmp(buf, "at+copn\r")) == 0)
	{
		ret = __edge_setcmd4(buf, "+COPN:",resp, sizeof resp, 60);
		if ( ret )
    		{
			r = sscanf(resp, "+COPN: %*c%[0-9]%*c", oper);
			if (r == 1)
			{
				snprintf(operator,sizeof operator, oper);
				if((strcmp(operator,"46692")) == 0)
					strcpy(operator,"Chunghwa");
				else if((strcmp(operator,"46000")) == 0)
					strcpy(operator, "ChinaMobile");
			}
			
    		}
	}
	else if((strcmp(buf, "at^dtms?\r")) == 0)
	{
		ret = __edge_setcmd2(buf, "^DTMS:",resp, sizeof resp, 60);
		mpDebugPrint("[at^dtms?]: %s", resp);
		if ( ret )
    		{
			r = sscanf(resp, "^DTMS: %d", &temp_AcT);
			if (r == 1)
			{
				AcT = temp_AcT;
			}
		}
	}
	else
		ret =  __edge_simple_cmd(buf);
	
	if ( ret )
		mpDebugPrint("%s OK!", cmd);
	else
		mpDebugPrint("%s Fail!", cmd);
	return ret;
}
#endif
// vim: :noexpandtab:
