#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"


#include <string.h>
#include <sys/time.h>
#include <linux/types.h>
#include "typedef.h"
//#include "net_packet.h"
#include "socket.h"
#include "net_socket.h"
#include "net_ns.h"
#include "net_device.h"
#include "net_netdb.h"
#include "os.h"
#include "net_netctrl.h"
//#include "..\..\..\..\std_dpf\ui\include\timezone.h"
#include "iplaysysconfig.h"

/* NTP definitions.  Note that these assume 8-bit bytes - sigh.  There is
little point in parameterising everything, as it is neither feasible nor
useful.  It would be very useful if more fields could be defined as
unspecified.  The NTP packet-handling routines contain a lot of extra
assumptions. */

#define JAN_1970   2208988800.0        /* 1970 - 1900 in seconds */
#define NTP_SCALE  4294967296.0        /* 2^32, of course! */

#define NTP_PACKET_MIN       48        /* Without authentication */
#define NTP_PACKET_MAX       68        /* With authentication (ignored) */
#define NTP_DISP_FIELD        8        /* Offset of dispersion field */
#define NTP_REFERENCE        16        /* Offset of reference timestamp */
#define NTP_ORIGINATE        24        /* Offset of originate timestamp */
#define NTP_RECEIVE          32        /* Offset of receive timestamp */
#define NTP_TRANSMIT         40        /* Offset of transmit timestamp */

#define NTP_LI_FUDGE          0        /* The current 'status' */
#define NTP_VERSION           3        /* The current version */
#define NTP_VERSION_MAX       4        /* The maximum valid version */
#define NTP_STRATUM          15        /* The current stratum as a server */
#define NTP_STRATUM_MAX      15        /* The maximum valid stratum */
#define NTP_POLLING           8        /* The current 'polling interval' */
#define NTP_PRECISION         0        /* The current 'precision' - 1 sec. */

#define NTP_ACTIVE            1        /* NTP symmetric active request */
#define NTP_PASSIVE           2        /* NTP symmetric passive response */
#define NTP_CLIENT            3        /* NTP client request */
#define NTP_SERVER            4        /* NTP server response */
#define NTP_BROADCAST         5        /* NTP server broadcast */

#define NTP_INSANITY     3600.0        /* Errors beyond this are hopeless */
#define RESET_MIN            15        /* Minimum period between resets */
#define ABSCISSA            3.0        /* Scale factor for standard errors */

/* Local definitions and global variables (mostly options).  These are all of
the quantities that control the main actions of the program.  The first three 
are the only ones that are exported to other modules. */

const char *argv0 = NULL;              /* For diagnostics only - not NULL */
int verbose = 0,                       /* Default = 0, -v = 1, -V = 2, -W = 3 */
    operation = 0;                     /* Defined in header.h - see action */
const char *lockname = NULL;           /* The name of the lock file */
int unprivport = 0;			/* Use an unpriv port for query? */


#define op_client           1          /* Behave as a challenge client */
#define op_listen           2          /* Behave as a listening client */


#define	NTP_PORT	123	/* included for non-unix machines */

#define fprintf 
#define fatal 

#define SNTP_DEBUG 0

double dispersion = 0;

/* The unpacked NTP data structure, with all the fields even remotely relevant
to SNTP. */

typedef struct NTP_DATA {
    unsigned char status, version, mode, stratum, polling;
    signed char precision;
    double dispersion, reference, originate, receive, transmit, current;
} ntp_data;

static U08 UpdateTimerId;
static U08 PollingServerTimerId;

S32 Timeval_Sema = 0;

struct timeval current_timeval;

char* hostname;
static U32 sntp_server_addr;

void pack_ntp(unsigned char *, int, ntp_data *);
void unpack_ntp(ntp_data *, unsigned char *, int);


double current_time (double offset) {

/* Get the current UTC time in seconds since the Epoch plus an offset (usually
the time from the beginning of the century to the Epoch!) */

    //struct timeval current;


	//memset(&current, 0, sizeof(struct timeval));
	//TODO
    //errno = 0;
    //if (gettimeofday(&current,NULL))
    //    fatal(1,"unable to read current machine/system time",NULL);
    return offset+current_timeval.tv_sec+1.0e-6*current_timeval.tv_usec;
}




time_t convert_time (double value, int *millisecs) {

/* Convert the time to the ANSI C form. */

    time_t result = (time_t)value;

    if ((*millisecs = (int)(1000.0*(value-result))) >= 1000) {
        *millisecs = 0;
        ++result;
    }
    return result;
}


void pack_ntp (unsigned char *packet, int length, ntp_data *data) {

/* Pack the essential data into an NTP packet, bypassing struct layout and
endian problems.  Note that it ignores fields irrelevant to SNTP. */

    int i, k;
    double d;

    memset(packet,0,(size_t)length);
    packet[0] = (data->status<<6)|(data->version<<3)|data->mode;
    packet[1] = data->stratum;
    packet[2] = data->polling;
    packet[3] = data->precision;
    d = data->originate/NTP_SCALE;
    for (i = 0; i < 8; ++i) {
        if ((k = (int)(d *= 256.0)) >= 256) k = 255;
        packet[NTP_ORIGINATE+i] = k;
        d -= k;
    }
    d = data->receive/NTP_SCALE;
    for (i = 0; i < 8; ++i) {
        if ((k = (int)(d *= 256.0)) >= 256) k = 255;
        packet[NTP_RECEIVE+i] = k;
        d -= k;
    }
    d = data->transmit/NTP_SCALE;
    for (i = 0; i < 8; ++i) {
        if ((k = (int)(d *= 256.0)) >= 256) k = 255;
        packet[NTP_TRANSMIT+i] = k;
        d -= k;
    }
}



void unpack_ntp (ntp_data *data, unsigned char *packet, int length) {

/* Unpack the essential data from an NTP packet, bypassing struct layout and
endian problems.  Note that it ignores fields irrelevant to SNTP. */

    int i;
    double d;

    data->current = current_time(JAN_1970);    /* Best to come first */
    data->status = (packet[0] >> 6);
    data->version = (packet[0] >> 3)&0x07;
    data->mode = packet[0]&0x07;
    data->stratum = packet[1];
    data->polling = packet[2];
    data->precision = packet[3];
    d = 0.0;
    for (i = 0; i < 4; ++i) d = 256.0*d+packet[NTP_DISP_FIELD+i];
    data->dispersion = d/65536.0;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+packet[NTP_REFERENCE+i];
    data->reference = d/NTP_SCALE;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+packet[NTP_ORIGINATE+i];
    data->originate = d/NTP_SCALE;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+packet[NTP_RECEIVE+i];
    data->receive = d/NTP_SCALE;
    d = 0.0;
    for (i = 0; i < 8; ++i) d = 256.0*d+packet[NTP_TRANSMIT+i];
    data->transmit = d/NTP_SCALE;
}




void make_packet (ntp_data *data, int mode) {

/* Create an outgoing NTP packet, either from scratch or starting from a
request from a client.  Note that it implements the NTP specification, even
when this is clearly misguided, except possibly for the setting of LI.  It
would be easy enough to add a sanity flag, but I am not in the business of
designing an alternative protocol (however much better it might be). */

    data->status = NTP_LI_FUDGE<<6;
    data->stratum = NTP_STRATUM;
    data->reference = data->dispersion = 0.0;
    if (mode == NTP_SERVER) {
        data->mode = (data->mode == NTP_CLIENT ? NTP_SERVER : NTP_PASSIVE);
        data->originate = data->transmit;
        data->receive = data->current;
    } else {
        data->version = NTP_VERSION;
        data->mode = mode;
        data->polling = NTP_POLLING;
        data->precision = NTP_PRECISION;
        data->receive = data->originate = 0.0;
    }
    data->current = data->transmit = current_time(JAN_1970);
}

static int read_packet(ntp_data *data, double *off, double *err){
	double x, y;
	
/* Now return the time information.  If it is a server response, it contains
enough information that we can be almost certain that we have not been fooled
too badly.	Heaven help us with broadcasts - make a wild kludge here, and see
elsewhere for other kludges. */

	if (dispersion < data->dispersion) dispersion = data->dispersion;
	if (operation == op_listen) {
		*off = data->transmit-data->current;
		*err = NTP_INSANITY;
	} else {
		x = data->receive-data->originate;
		y = (data->transmit == 0.0 ? 0.0 : data->transmit-data->current);
		*off = 0.5*(x+y);
		*err = x-y;
		x = data->current-data->originate;
		if (0.5*x > *err) *err = 0.5*x;
	}
	return 0;

}

void Update_timer(){

	SemaphoreWait(Timeval_Sema);
	current_timeval.tv_sec++;
	SemaphoreRelease(Timeval_Sema);

#if SNTP_DEBUG
	{
		int milli, len;
		time_t now;
		struct tm *gmt;
		static const char *months[] = {
			"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
		};
		char text[100];
		memset(text, 0 , 100);

		now = convert_time(current_time(0),&milli);
		//errno = 0;
		if ((gmt = localtime(&now)) == NULL)
			fatal(1,"unable to work out local time",NULL);
	
		mp_sprintf(text, "%d %s %d %d %d %d", 
			gmt->tm_year+1900,
			months[gmt->tm_mon],
			gmt->tm_mday,
			gmt->tm_hour,
			gmt->tm_min,
			gmt->tm_sec
			);
		UartOutText(text);

	}
#endif
}

void Polling_Server(){
    S32 err = NO_ERR;
    U16 sockId = 0;
    ST_SOCK_ADDR localAddr, peerAddr;
    ST_SOCK_SET stReadSet;
    U08 SendCounter = 0;
    U16 selects = 0;
	U32 length;
	struct timeval timeout;

	double x, y;
	
	ntp_data data;

	unsigned char transmit[NTP_PACKET_MIN];
	unsigned char receive[NTP_PACKET_MAX+1];

    char text[100];

    if(NetDefaultIpGet() == NULL){
        DPrintf("no IP Address, please set IP Address again");
        return;
    }

    if(NetDNSGet(0) == INADDR_NONE){
        DPrintf("no DNS server, please set DNS server again");
        return;
    }
	
    err = mpx_Socket(AF_INET, SOCK_DGRAM);
    if(err <= 0){
        DPrintf("no Socket ID");
        goto ERROR;
    }

    sockId = (U16)err;
    mpx_SockAddrSet(&peerAddr, sntp_server_addr, NTP_PORT);
    mpx_SockAddrSet(&localAddr, NetDefaultIpGet(), mpx_NewLocalPort()); 
    mpx_Bind(sockId, &localAddr);     

	
	memset(&data, 0, sizeof(ntp_data));

	make_packet(&data,NTP_CLIENT);

	pack_ntp(transmit,NTP_PACKET_MIN,&data);

	/* set a suitable timeout to play around with */
	timeout.tv_sec = 4;
	timeout.tv_usec = 0;



Send:

	sendto(sockId, transmit, NTP_PACKET_MIN, 0, &peerAddr, sizeof(peerAddr));

	//TaskSleep(200);

	//mpx_SendTo(sockId, transmit, NTP_PACKET_MIN, 0, &peerAddr);
	
	//TaskSleep(200);

	//mpx_SendTo(sockId, transmit, NTP_PACKET_MIN, 0, &peerAddr);

	do
	{
		selects = 0;
		MPX_FD_ZERO(&stReadSet);
		MPX_FD_SET(sockId, &stReadSet);
		err = select(0, &stReadSet, 0, 0, &timeout); //3 seconds
		
		if(err > 0)
		{
			S32 status;
			U16 type;
			U16 length;
			
			selects = (U16)err;
			status = recvfrom(sockId, receive, NS_PACKETSZ, 0, &peerAddr, sizeof(peerAddr));
			if(status > 0)
				length = (U16)status;
			else{
				DPrintf("selected but nothing Recv");
				BREAK_POINT();
				//select = 0;
			}
		}
		else
		{
			DPrintf("SNTP wait %1d response timeout", SendCounter);
			if(SendCounter == 2)
				goto ERROR;
			else{
				SendCounter++;
				goto Send;
			}
		}
	}while(!select);

	unpack_ntp(&data,receive,length);

	read_packet(&data, &x, &y);

	SemaphoreWait(Timeval_Sema);
	current_timeval.tv_sec += x;
	#if TIME_ZONE
	sntp_setting(current_timeval.tv_sec);
	#endif
	SemaphoreRelease(Timeval_Sema);


	//format_time(text,75,x,y,0.0,-1.0,data.precision);
	//UartOutText(text);

	// free
    if(sockId)
    {
        mpx_SocketClose(sockId);
    }


	return;

ERROR:
	// free
    if(sockId)
    {
        mpx_SocketClose(sockId);
    }

}

void sntp_init(char* hostname){
	S32 status;

	sntp_set_server(hostname);

	Timeval_Sema = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);	

	if(Timeval_Sema <= 0){
		DPrintf("Timeval_Sema create fail");
		BREAK_POINT();
	}

	status = NetTimerInstall(Update_timer, NET_TIMER_250_MS*4);
    if(status >= 0)
    {
        UpdateTimerId = (U08)status;
        NetTimerRun(UpdateTimerId);
    }
    else
    {
        DPrintf("[IP] timer create fail");
        BREAK_POINT();
    }

	status = NetTimerInstall(Polling_Server, NET_TIMER_250_MS*240);
	if(status >= 0)
	{
		PollingServerTimerId = (U08)status;
		NetTimerRun(PollingServerTimerId);
	}
	else
	{
		DPrintf("[IP] timer create fail");
		BREAK_POINT();
	}

}


sntp_set_server(char* hostname){
	if (! isdigit(hostname[0])) {
		struct hostent *host;
		struct in_addr *curr;
		host = gethostbyname(hostname);
        if (curr = (struct in_addr *)host->h_addr_list[0])
            sntp_server_addr = ntohl(curr->s_addr);

	} else {
		sntp_server_addr = inet_addr(hostname);
	}
}
