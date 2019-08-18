#define LOCAL_DEBUG_ENABLE 0

#include <sys/time.h>
#include <errno.h>
#include "typedef.h"
#include "linux/types.h"
#include "net_ftp.h"
#include "SysConfig.h"
//#include "fs_api.h"
#include "util_queue.h"
#include "net_netctrl.h"
#include "taskid.h"
#include <linux/if.h>
#include "global612.h"
#include "socket.h"


extern ST_FTP_STATE g_stFTPState2[MAX_FTP_SESSIONS+1];

typedef struct ftp_data_conn_s {
  int ftpDataSockId;
  int ftpControlSockId;

  U16 ftpLocalDataPort;
  U16 ftpPeerDataPort;

  U16 ftpPeerControlPort;

  U16 ftpFlags;
  U32 outLength;
  U32 inLength;
  U16 ftpReplyCode;

  U32 ftpLocalIpAddress;   /**< The IP address of the local host. */
  U32 ftpPeerIpAddress;   /**< The IP address of the remote host. */
  
  U16 lport;        /**< The local TCP port. */
  U16 rport;        /**< The remote TCP port */

  U32 idleTime;
} FTP_SESSION;


static FTP_SESSION ftpSessions[MAX_FTP_SESSIONS+1];         /* the last one is for session overflow */
static BYTE g_mbCmdBuf[FTP_STR_BUF_SIZE];
static int serverSockId;

//reply message
#define REPL_110 "110 Restart marker reply.\r\n"
#define REPL_120 "120 Try again in 2 minutes.\r\n"
#define REPL_125 "125 Data connection already open; transfer starting.\r\n"
#define REPL_150 "150 File status okay; about to open data connection.\r\n"
#define REPL_200 "200 Command okay.\r\n"
#define REPL_200_TYPE "200 Type set to I.\r\n"
#define REPL_202 "202 Command not implemented, superfluous at this site.\r\n"
#define REPL_211 "211 System status, or system help reply.\r\n"
#define REPL_211_STATUS "211-status of %s.\r\n"
#define REPL_211_END "211 End of status.\r\n"
#define REPL_212 "212 Directory status.\r\n"
#define REPL_213 "213 File status.\r\n"
#define REPL_214 "214 Help message.\r\n"
#define REPL_214_END "214 End Help message.\r\n"
#define REPL_215 "215 UNIX system type.\r\n"
#define REPL_220 "220 Service ready for new user.\r\n"
#define REPL_221 "221 Goodbye!\r\n"
#define REPL_225 "225 Data connection open; no transfer in progress.\r\n"
#define REPL_226 "226 Closing data connection.\r\n"
#define REPL_227 "227 Entering Passive Mode (%s,%s,%s,%s,%s,%s).\r\n"
#define REPL_230 "230 User logged in, proceed.\r\n"
#define REPL_250 "250 Requested file action okay, completed.\r\n"
#define REPL_257 "257 %s created.\r\n"
#define REPL_257_PWD "257 \"%s\" is current working dir.\r\n"
//#define REPL_331 "331 Only anonymous user is accepted.\r\n"
#define REPL_331 "331 User name okay, need password.\r\n"
#define REPL_331_ANON "331 Anonymous login okay, send your complete email as your password.\r\n"
#define REPL_332 "332 Need account for login.\r\n"
#define REPL_350 "350 Requested file action pending further information.\r\n"
#define REPL_421 "421 Service not available, closing control connection.\r\n"
#define REPL_421_TIMEOUT "421 Connection timed out - closing.\r\n"
#define REPL_425 "425 Can't open data connection.\r\n"
#define REPL_426 "426 Connection closed; transfer aborted.\r\n"
#define REPL_450 "450 Requested file action not taken.\r\n"
#define REPL_451 "451 Requested action aborted. Local error in processing.\r\n"
#define REPL_452 "452 Requested action not taken.\r\n"
#define REPL_500 "500 Syntax error, command unrecognized.\r\n"
#define REPL_501 "501 Syntax error in parameters or arguments.\r\n"
#define REPL_502 "502 Command not implemented.\r\n"
#define REPL_503 "503 Bad sequence of commands.\r\n"
#define REPL_504 "504 Command not implemented for that parameter.\r\n"
#define REPL_530 "530 Not logged in.\r\n"
#define REPL_530_NO_MORE_LOGIN "530 There are too many users.\r\n"
#define REPL_532 "532 Need account for storing files.\r\n"
#define REPL_550 "550 File not found or access denied.\r\n"
#define REPL_551 "551 Requested action aborted. Page type unknown.\r\n"
#define REPL_552 "552 Requested file action aborted.\r\n"
#define REPL_553 "553 Requested action not taken.\r\n"

static U08 ftpTaskId;
static U08 ftpTimerId;
static int messageSockId;

static ST_FTP_SERVER_PROPERTY serverRec;

void ftpserver_DP_GetFileChain(BYTE *pbBuf, DWORD dwSize, void *ctx,BYTE idx);
SWORD ftpserver_SendReplyMsg(WORD wMsgNum, void *ctx);
WORD ftpserver_SendMsg(BYTE *pbMsg, WORD wCnt, void *ctx);

static void FtpTask();
static void ftpTimerProc();
static void ftpKickClient(U08* message, FTP_SESSION *sess);
static S32 ftpServerSocketCreate();
static int ftpServerTcpReceive(int sockid, char *buf, size_t len);
static void ftpserverMessageReceive(U32 *msg);
int ftpServerCtrlMessage(int sid, const char *msg, size_t msg_len);

void ftpserver_SetReply(int msgId, void *ctx);


S32 mpx_FtpConnect(U32 u32DstAddr, U16 u16DstPort, U08** pOutWelcomeMsg, BOOL isBlocking)
{
    U16 sockId = 0;
    S32 error = NO_ERR;
    
    sockId = mpx_DoConnect(u32DstAddr, u16DstPort, isBlocking);

    if (sockId > 0)
    {
        if(pOutWelcomeMsg)
        {
            *pOutWelcomeMsg = ftp_malloc(FTP_BLOCK_SIZE);
            if(0 == *pOutWelcomeMsg)
            {
                DPrintf("[FTP] allocate buffer fail");
            }
            else
            {
                ST_SOCK_SET stReadSet;
                U16 readLength = 0;
                U16 totalLength = 0;
                U08* ptr = *pOutWelcomeMsg;
                BOOL finish = FALSE;
                U16 i;

                while(!finish)
                {
                    MPX_FD_ZERO(&stReadSet);
                    MPX_FD_SET(sockId, &stReadSet);
                    error = mpx_Select(&stReadSet, 0, 0, 3000);
                    if(error > 0)
                    {
                        error = mpx_SockRead(sockId, ptr, FTP_BLOCK_SIZE - totalLength);
                        if(error > 0)
                        {
                            readLength = (U16)error;
                            for(i = 0; i < readLength; i++)
                            {
                                if((ptr[i] == '2') && (ptr[i+1] == '2') && (ptr[i+2] == '0') && (ptr[i+3] == ' '))
                                {
                                    finish = TRUE;
                                }
                            }

                            ptr += readLength;
                            totalLength += readLength;
                        }
                        else if(error == 0)
                        {
                            closesocket(sockId);
                            sockId = 0;
                            finish = TRUE;
                        }
                    }
                }

                DPrintf("[FTP] welcome message length = %d", totalLength);
            }
        }
    }
        
    return sockId;
}







S32 mpx_FtpServiceEnable()
{
    S32 status;
	struct sockaddr_un local, dest;

    if(ftpTaskId)
        return NO_ERR;
    
	status = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (status < 0) {
		MP_ALERT("socket(PF_UNIX)");
		return -4;
	}

	memset(&local, 0, sizeof(local));
	local.sun_family = AF_UNIX;
	strncpy(local.sun_path, "/ftp_server", sizeof(local.sun_path));
	if (bind(status, (struct sockaddr *) &local, sizeof(local)) < 0) {
		closesocket(status);
		MP_ALERT("ftp_server: bind(PF_UNIX) failed: %d", errno);
        MP_ASSERT(0);
		return -5;
	}

    messageSockId = status;
    MP_DEBUG1("[FTP] message socket = %d", messageSockId);

	status = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (status < 0) {
		return -1;
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, "/ftp_ctrl");
	if (bind(status, (struct sockaddr *) &local,
		    sizeof(local)) < 0) {
		closesocket(status);
		return -2;
	}

	dest.sun_family = AF_UNIX;
	strcpy(dest.sun_path, "/ftp_server");
	if (connect(status, (struct sockaddr *) &dest,
		    sizeof(dest)) < 0) {
		closesocket(status);
		return -3;
	}

	serverRec.ctrlSockId = status;
    MP_DEBUG1("[FTP] ctrl socket = %d", serverRec.ctrlSockId);
    
    if(ftpTaskId == 0)
    {
        status = mpx_TaskCreate(FtpTask, NETWORK_TASK_PRI, TASK_STACK_SIZE*2, "FTP");
        if(status < 0)
        {
            MP_DEBUG1("mpx_FtpServiceEnable: mpx_TaskCreate returns %d", status);
            MP_ASSERT(0);
        }
        
        ftpTaskId = (U08)status;
        MP_DEBUG1("mpx_FtpServiceEnable: mpx_TaskCreate returns %d", ftpTaskId);
    }
    
    serverRec.port = FTP_SERVER_PORT;

    
    DPrintf("[FTP] FtpTask id = %d", ftpTaskId);
    mpx_TaskStartup(ftpTaskId);
    mpx_TaskYield(10);
    
    status = NetTimerInstall(ftpTimerProc, NET_TIMER_1_SEC);
    if(status >= 0)
    {
        ftpTimerId = (U08)status;
    }
    else
    {
        DPrintf("[FTP] timer create fail");
        MP_ASSERT(0);
    }
    
    return NO_ERR;
}

S32 mpx_FtpServiceDisable()
{
    if(serverRec.welcomeMessage)
    {
        ftp_mfree(serverRec.welcomeMessage);
        serverRec.welcomeMessage = 0;
    }
    
    return NO_ERR;
}

static void FtpTask()
{
    S32 ftpStatus = NO_ERR;
    U32 message[8];
    U16 controlSockId = 0;
    U16 dataSockId;
    U08* ftpServerBuffer = (U08*)ftp_malloc(FTP_BLOCK_SIZE);
    struct sockaddr_in peerAddr;
    ST_SOCK_SET stReadSet, stWriteSet;
    ST_SOCK_SET *wfds, *rfds;
    U16 dataPort = 0;
    U16 msgLength;
    U08* userName = 0;
    U08 loginFail = 0;
    BOOL queryEnable = FALSE;
    int i, len, ret;
    FTP_SESSION *fconn;
    
    serverRec.flag = 0;
    memset(message, 0, sizeof(message));
    
    if(!ftpServerBuffer )
    {
        DPrintf("[FTP] server allocate buffer fail");
        goto FINISH;
    }
    
    
    serverRec.u08CurNodePosition = 0;
    
    while(1)
    {
        MPX_FD_ZERO(&stReadSet);
        MPX_FD_ZERO(&stWriteSet);
        wfds = rfds = NULL;
        
        if(messageSockId > 0)
        {
            MPX_FD_SET(messageSockId, &stReadSet);
            rfds = &stReadSet;
        }

        if(serverRec.flag & SERVER_ENABLE)
        {
            if(serverSockId)
            {
                MPX_FD_SET(serverSockId, &stReadSet);
                rfds = &stReadSet;
            }
            for (i=0; i<= MAX_FTP_SESSIONS; i++)
            {
                controlSockId = ftpSessions[i].ftpControlSockId;
                if(controlSockId)
                {
                    MPX_FD_SET(controlSockId, &stReadSet);
                    rfds = &stReadSet;

                    if (ftpSessions[i].ftpReplyCode)
                    {
                        MPX_FD_SET(controlSockId, &stWriteSet);
                        wfds = &stWriteSet;
                    }
                }
            }


            for (i=0; i< MAX_FTP_SESSIONS; i++)
            {
                dataSockId = ftpSessions[i].ftpDataSockId;
                if(dataSockId)
                {
                    if ((ftpSessions[i].ftpFlags & FTPF_SENDING) || !(ftpSessions[i].ftpFlags & FTPF_DATAISCONNECTED))
                    {
                        MPX_FD_SET(dataSockId, &stWriteSet);
                        wfds = &stWriteSet;
                    }

                    if (ftpSessions[i].ftpFlags & FTPF_DATAISCONNECTED)
                    {
                        MPX_FD_SET(dataSockId, &stReadSet);
                        rfds = &stReadSet;
                    }
                }
            }
        }
        
        MP_DEBUG("[FTP] select(0,rfds,wfds,0,0)");
        ftpStatus = select(0, rfds, wfds, NULL, NULL);
        MP_DEBUG("[FTP] select returns %d,0x%x,0x%x", ftpStatus, stReadSet.u32Mask, stWriteSet.u32Mask);
        if(ftpStatus > 0)
        {
            if(MPX_FD_ISSET(messageSockId, &stReadSet))
            {
                if (ftpServerTcpReceive(messageSockId, g_mbCmdBuf, FTP_STR_BUF_SIZE) > 0)
                {
                    ftpserverMessageReceive((U32 *)g_mbCmdBuf);
                }
                else
                {
                    MP_DEBUG("[FTP] message socket is closed.");
                    ret = closesocket(messageSockId);
                    messageSockId = 0;
                }
            }
            
            if(MPX_FD_ISSET(serverSockId, &stReadSet))
            {
                ftpStatus = accept(serverSockId, (struct sockaddr *)&peerAddr, NULL);
                MP_DEBUG2("FtpTask: accept(%d) returns %d", serverSockId,ftpStatus);
                if(ftpStatus > 0)
                {
                    for (i=0; i< MAX_FTP_SESSIONS; i++)
                    {
                        if (ftpSessions[i].ftpControlSockId == 0)
                            break;
                    }

                    if(i == MAX_FTP_SESSIONS)         /* too many FTP sessions */
                    {
                        ftpserver_SendReplyMsg2(530, ftpStatus);
                        if (ftpSessions[i].ftpControlSockId == 0)
                        {
                            fconn = &ftpSessions[i];
                            memset(fconn, 0, sizeof(*fconn));
                            fconn->ftpControlSockId = (int)ftpStatus;
                            shutdown(ftpStatus, 1);      /* no more to send */
                        }
                        else
                        {
                            closesocket(ftpStatus);
                            MP_DEBUG1("FtpTask: closesocket(%d)", ftpStatus);
                        }
                    }
                    else
                    {
                        struct ifreq ifr;
                        struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
                        loginFail = 0;

                        memset(&ftpSessions[i], 0, sizeof(FTP_SESSION));
                        ftpSessions[i].ftpControlSockId = ftpStatus;

                        ftpSessions[i].ftpPeerIpAddress = ntohl(peerAddr.sin_addr.s_addr);
                        ftpSessions[i].rport = ntohs(peerAddr.sin_port);

                        unsigned long val = 1;
                        ioctlsocket(ftpStatus, FIONBIO, &val);
                        if (ioctlsocket(ftpStatus, SIOCGIFADDR, (unsigned long *)&ifr) == 0)
                            ftpSessions[i].ftpLocalIpAddress = htonl(sin->sin_addr.s_addr);

                        NetTimerRun(ftpTimerId);
#ifdef MP600
                        ftpserver_SetReply(220,&ftpSessions[i]);
#endif
                    }

                }
            }
            
            for (i=0; i<= MAX_FTP_SESSIONS; i++)
            {
                controlSockId = ftpSessions[i].ftpControlSockId;
                fconn = &ftpSessions[i];

                if (controlSockId && MPX_FD_ISSET(controlSockId, &stReadSet))
                {
                    ret = ftpServerTcpReceive(controlSockId, g_mbCmdBuf, FTP_STR_BUF_SIZE);
                    MP_DEBUG3("FtpTask: recv(%d) returns %d (%d)", controlSockId,ret,getErrno());
                    if(ret > 0)
                    {
                        fconn->idleTime = 0;
                        NetTimerStop(ftpTimerId);

                        if (FAIL == ftpserver_ParseCommand(&g_mbCmdBuf, &ftpSessions[i],i))
                            memset(&g_mbCmdBuf, 0, FTP_STR_BUF_SIZE);

                        NetTimerRun(ftpTimerId);
                    }
                    else 
                    {
                        closesocket(controlSockId);
                        fconn->ftpControlSockId = 0;
                        controlSockId = ftpSessions[i].ftpControlSockId;
                    }
                }

                if (controlSockId && MPX_FD_ISSET(controlSockId, &stWriteSet))
                {
                    fconn->idleTime = 0;
                    NetTimerStop(ftpTimerId);

                    if (ftpSessions[i].ftpReplyCode)
                    {
                        ret = ftpserver_SendReplyMsg(fconn->ftpReplyCode, &ftpSessions[i]);
                        if (fconn->ftpReplyCode == 221) /* Goodbye */
                        {
                            shutdown(controlSockId, 0);
                            shutdown(controlSockId, 1);
                        }
                        fconn->ftpReplyCode = 0;
                    }

                    if(ret < 0)
                    {
                        DPrintf("[FTP] control connection aborted (errno=%d)", getErrno());
                        ret = closesocket(controlSockId);
                        MP_DEBUG2("FtpTask: closesocket(%d) returns %d", controlSockId,ret);
                        fconn->ftpControlSockId = 0;
                    }


                    NetTimerRun(ftpTimerId);
                }
            }

            for (i=0; i< MAX_FTP_SESSIONS; i++)
            {
                dataSockId = ftpSessions[i].ftpDataSockId;
				
                fconn = &ftpSessions[i];

                if(dataSockId && MPX_FD_ISSET(dataSockId, &stReadSet))
                {
                    int ret;
                    ret = recv(dataSockId, ftpServerBuffer, FTP_BLOCK_SIZE, 0);
                    MP_DEBUG2("FtpTask: recv(%d) returns %d", dataSockId,ret);
                    if(ret > 0)
                    {
                        fconn->idleTime = 0;
                        NetTimerStop(ftpTimerId);

                        if (fconn->ftpFlags & FTPF_RECEVING)
                        {
                            ftpserver_DP_GetFileChain(ftpServerBuffer, (U32)ret, fconn,i);
                            fconn->inLength += ret;
                        }

                        NetTimerRun(ftpTimerId);
                    }
                    else
                    {
                        if (ret == 0)
                    {
                            if (fconn->ftpFlags & FTPF_RECEVING)
                            {
                                MP_DEBUG1("[FTP] file received complete size=%u", fconn->inLength);
                                ftpserver_SetReply(226, fconn);
                                ftpserver_DP_GetFileFinish(i);
                            }
                        }
                        closesocket(fconn->ftpDataSockId);
                        fconn->ftpDataSockId = 0;
                        dataSockId = ftpSessions[i].ftpDataSockId;
						
                    }
                }

                if(dataSockId && MPX_FD_ISSET(dataSockId, &stWriteSet))
                {
                    fconn->idleTime = 0;
                    NetTimerStop(ftpTimerId);

                    ftpSessions[i].ftpFlags |= FTPF_DATAISCONNECTED; /* mark as connected */
                    mpDebugPrint("ftpSessions[i].ftpFlags %x",ftpSessions[i].ftpFlags);
                    if (ftpSessions[i].ftpFlags & FTPF_SENDING)
                    {
                        mpDebugPrint("FTP_ML_PREV_CMD(g_stFTPState2[%d]) %x",i,FTP_ML_PREV_CMD(g_stFTPState2[i]));
                        if (FTP_CMD_LIST == FTP_ML_PREV_CMD(g_stFTPState2[i]))
                            len = ftpserver_DP_SendList( ftpSessions[i].outLength, &ftpSessions[i]);
                        else if ((FTP_CMD_RETR == FTP_ML_PREV_CMD(g_stFTPState2[i])))
                            len = ftpserver_DP_SendFileChain(fconn,i);

                        if (len > 0)
                            fconn->outLength += len;
                    }

                    NetTimerRun(ftpTimerId);
                }
            }
        }
    }
    
FINISH:
    if(serverSockId)
        closesocket(serverSockId);
    if(serverRec.ctrlSockId > 0)
        closesocket(serverRec.ctrlSockId);
    if(ftpServerBuffer)
        //mpx_Free(ftpServerBuffer);
		ftp_mfree(ftpServerBuffer);
    
}








void ftpTimerProc()
{
    short i;
    for (i=0; i< MAX_FTP_SESSIONS; i++)
    {
        if (ftpSessions[i].ftpFlags)
    {
            ftpSessions[i].idleTime++;
            if(ftpSessions[i].idleTime >= MPX_FTP_CLIENT_IDLE_TIMEOUT)
        {
            U32 message[2];
            
            NetTimerStop(ftpTimerId);
            
            message[0] = FTP_MSG_CLIENT_IDLE_TIMEOUT;
                message[1] = i;
            
                ftpServerCtrlMessage(serverRec.ctrlSockId, (char *)message, sizeof message);
            }
        }
    }
}

void ftpKickClient(U08* message, FTP_SESSION *fconn)
{
    int ret;
    if(message)
        ftpserver_SendMsg(message, strlen(message), fconn);
    
    ret = closesocket(fconn->ftpControlSockId); //XXX shutdown;
    MP_DEBUG2("ftpKickClient: closesocket(%d) returns %d", fconn->ftpControlSockId,ret);
    fconn->ftpControlSockId = 0;
    NetTimerStop(ftpTimerId);
    fconn->idleTime = 0;
}





S32 mpx_FtpServerEnable()
{
    U32 message;
    int ret;
    
    message = FTP_MSG_SERVER_ENABLE;
    ret = ftpServerCtrlMessage(serverRec.ctrlSockId, (char *)&message, sizeof message);
    MP_ASSERT(ret == 0);
    
    return NO_ERR;
}

S32 mpx_FtpServerDisable()
{
    U32 message;
    
    message = FTP_MSG_SERVER_DISABLE;
    ftpServerCtrlMessage(serverRec.ctrlSockId, (char *)&message, sizeof message);
    
    return NO_ERR;
}

static S32 ftpServerSocketCreate()
{
    struct sockaddr_in *localAddr;
    struct ifreq ifr;

    int ftpStatus = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int ret;
    
    if(ftpStatus <= 0)
    {
        DPrintf("[FTP] server socket create fail err=%d", ftpStatus);
        return 0;
    }
    
    MP_DEBUG1("[FTP] server socket id = %d", ftpStatus);
    
    if ((ret = ioctlsocket(ftpStatus, SIOCGIFADDR, (unsigned long *)&ifr)) != 0)
    {
        DPrintf("[FTP] ioctlsocket return err=%d", ret);
        return 0;
    }

    localAddr =  (struct sockaddr_in *)&ifr.ifr_addr;
    localAddr->sin_port = serverRec.port;

    ret = bind(ftpStatus, (struct sockaddr *)localAddr, sizeof(*localAddr));
    MP_DEBUG4("[FTP] bind(%d,0x%x,%d) returns = %d", ftpStatus, localAddr->sin_addr.s_addr,localAddr->sin_port, ret);

    ret = listen(ftpStatus, 0);
    MP_DEBUG2("[FTP] listen(%d) returns = %d", ftpStatus, ret);
    
    return ftpStatus;
}

U16 ftpParsePort(U08* buffer, void *ctx)
{
    U08 port1[8], port2[8];
    U08* ptr = buffer;
    U16 dataPort = 0;
    U16 i;
    U16 tempVal;

    while((*ptr < '0') || (*ptr > '9')) //skip non-digit data
        ptr++;

    for(i = 0; i < 4; i++)
        while(*ptr++ != ',');

    i = 0;
    while(*ptr != ',')
        port1[i++] = *ptr++;
    port1[i] = 0;

    ptr++;
    i = 0;
    while((*ptr >= '0') && (*ptr <= '9'))
        port2[i++] = *ptr++;
    port2[i] = 0;

    i = 0;
    dataPort = 0;
    while(port1[i])
    {
        dataPort = dataPort * 10 + (port1[i] - 0x30);
        i++;
    }
    dataPort *= 256;

    i = 0;
    tempVal = 0;
    while(port2[i])
    {
        tempVal = tempVal * 10 + (port2[i] - 0x30);
        i++;
    }

    dataPort += tempVal;
    
    ((FTP_SESSION *)ctx)->ftpPeerDataPort = dataPort;

    return dataPort;
}

void ftpServerHandleCmdLIST(void *ctx)
{
    FTP_SESSION *dconn = (FTP_SESSION *)ctx;
    S32 ftpStatus;
    
    ftpStatus = mpx_FtpConnect(dconn->ftpPeerIpAddress, dconn->ftpPeerDataPort, 0, FALSE);
    if(ftpStatus > 0)
    {
        dconn->ftpFlags = 0;
        dconn->ftpFlags |= FTPF_SENDING;
        dconn->outLength = 0;

        dconn->ftpDataSockId = ftpStatus;
    }
    else
    {
        ftpserver_SetReply(425,dconn);
    }
}

void ftpServerHandleCmdRETR(void *ctx,BYTE idx)
{
    FTP_SESSION *dconn = (FTP_SESSION *)ctx;
    S32 ftpStatus;
    
    ftpStatus = mpx_FtpConnect(dconn->ftpPeerIpAddress, dconn->ftpPeerDataPort, 0, FALSE);
    if(ftpStatus > 0)
    {
        dconn->ftpFlags = 0;
        dconn->ftpFlags |= FTPF_SENDING;
        dconn->outLength = 0;

        ftpserver_DP_SendFile(dconn,idx);


        dconn->ftpDataSockId = ftpStatus;
    }
    else
    {
        ftpserver_SetReply(425,dconn);
    }
    
}

int ftpServerHandleCmdSTOR(void *ctx,BYTE idx)
{
    FTP_SESSION *dconn = (FTP_SESSION *)ctx;
    S32 ftpStatus;
    
    ftpStatus = ftpserver_DP_GetFile(ctx,idx);                 /* Prepare */
    if (ftpStatus != 0)
        return FTP_ERROR;

    ftpStatus = mpx_FtpConnect(dconn->ftpPeerIpAddress, dconn->ftpPeerDataPort, 0, FALSE);
    if(ftpStatus > 0)
    {
        dconn->ftpFlags = 0;
        dconn->ftpFlags |= FTPF_RECEVING;
        dconn->inLength = 0;

        dconn->ftpDataSockId = ftpStatus;
    }
    else
    {
        ftpserver_SetReply(425,dconn);
    }

    return FTP_OK;
}

void ftpServerEnterPASVMode(void *ctx)
{
    FTP_SESSION *dconn = (FTP_SESSION *)ctx;
    
    dconn->ftpFlags |= FTPF_PASSIVEMODE;
}

/* 
 * Close the data connection
 */
void ftpserverSendFinished(void *ctx)
{
    FTP_SESSION *fconn = (FTP_SESSION *)ctx;

    MP_DEBUG("[FTP] data sending finished (socket=%d)", fconn->ftpDataSockId);
    fconn->ftpFlags &= ~FTPF_SENDING;
    MP_DEBUG("[FTP] file download complete size=%u", fconn->outLength);

    if (fconn->ftpDataSockId)
        shutdown(fconn->ftpDataSockId, 1);      /* no more to send */
}

void ftpserver_Abort(void *ctx)
{
    FTP_SESSION *fconn = (FTP_SESSION *)ctx;
    int ret;

    if (fconn->ftpDataSockId)
    {
        ret =closesocket(fconn->ftpDataSockId);
        MP_DEBUG2("ftpserver_Abort: closesocket(%d) returns %d", fconn->ftpDataSockId,ret);
        fconn->ftpDataSockId = 0;
    }
}

int ftp_send(const void *data, int len, int isCtrl, void *ctx)
{
    FTP_SESSION *dconn = (FTP_SESSION *)ctx;
    int sid;
    int ret;

    if (isCtrl)
        sid = dconn->ftpControlSockId;
    else
        sid = dconn->ftpDataSockId;

    MP_DEBUG1("ftp_send: send(%d)", sid);
    ret = send(sid, data, len, 0);
    if (ret != len)
    {
        mpx_TaskYield(1);
        MP_DEBUG2("ftp_send: send(%d) returns %d", sid,ret);
    }

    return ret;
}

int ftp_send2(const void *data, int len, int sid)
{
    int ret;

    MP_DEBUG1("ftp_send: send(%d)", sid);
    ret = send(sid, data, len, 0);
    MP_DEBUG2("ftp_send: send(%d) returns %d", sid,ret);

    return ret;
}

void ftpserver_SetReply(int msgId, void *ctx)
{
    FTP_SESSION *sess = (FTP_SESSION *)ctx;

    sess->ftpReplyCode = msgId;
}

void ftp_GetLocalIpAddress(U32 *ipaddr, void *ctx)
{
    FTP_SESSION *sess = (FTP_SESSION *)ctx;

    if (ipaddr)
        *ipaddr = sess->ftpLocalIpAddress;
}

void *memSave;
dpr(char *str)
{
    if (!str)
        DPrintf("[FTP] *memSave= 0x%x", *(U32 *)memSave);
    else
        DPrintf("[FTP] %s: *memSave= 0x%x",str, *(U32 *)memSave);
}

void ftpserverMessageReceive(U32 *msg)
{
    FTP_SESSION *fconn;
    short i;
    int ftpStatus;
    MP_DEBUG("%s", __FUNCTION__);
    switch(msg[0])
    {
        case FTP_MSG_CLIENT_IDLE_TIMEOUT:
            {
                MP_ASSERT(msg[1] < MAX_FTP_SESSIONS);
                if (msg[1] < MAX_FTP_SESSIONS)
                {
                    fconn = &ftpSessions[msg[1]];
                    MP_DEBUG("[FTP] session times out %d", msg[1]);
                    if(fconn->ftpDataSockId)
                    {
                        closesocket(fconn->ftpDataSockId);
                        fconn->ftpDataSockId = 0;
                    }
                    if(fconn->ftpControlSockId)
                    {
                        ftpKickClient(REPL_421, fconn);
                    }
                }
            }
            break;

        case FTP_MSG_SERVER_ENABLE:
            {
                if (!(serverRec.flag & SERVER_ENABLE))
                {
                    ftpStatus = ftpServerSocketCreate();
                    if(ftpStatus > 0)
                    {
                        serverSockId = (U16)ftpStatus;
                        MP_DEBUG("[FTP] server socket = %d", serverSockId);
                    }

                    serverRec.flag |= SERVER_ENABLE;
                }
            }
            break;

        case FTP_MSG_SERVER_DISABLE:
            {
                if (serverRec.flag & SERVER_ENABLE)
                {
                    for (i=0; i< MAX_FTP_SESSIONS; i++)
                    {
                        fconn = &ftpSessions[i];
                        if(fconn->ftpControlSockId > 0)
                        {
                            ftpKickClient(REPL_421_TIMEOUT, fconn);
                        }
                        if(fconn->ftpDataSockId > 0)
                        {
                            closesocket(fconn->ftpDataSockId);
                            fconn->ftpDataSockId = 0;
                        }
                        fconn->ftpFlags = 0;
                    }

                    if(serverSockId)
                    {
                        MP_DEBUG("[FTP] close server socket");
                        closesocket(serverSockId);
                        serverSockId = 0;
                    }

                    serverRec.flag &= ~SERVER_ENABLE;
                }
            }
            break;
    }
}

int dod1;
int ftpServerTcpReceive(int sockid, char *buf, size_t len)
{
    int ret;
    MP_DEBUG1("FtpTask: recv(%d)", sockid);
    ret = recv(sockid, buf, len,0);
    MP_DEBUG3("FtpTask: recv(%d) returns %d (%d)", sockid,ret,getErrno());
    if(ret == 0)
    {
        MP_DEBUG("[FTP] TCP connection is closed gracefully.");
    }
    else if(ret < 0)
    {
        MP_DEBUG1("[FTP] TCP connection is aborted (%d).", getErrno());
    }

    return ret;
}

int ftpServerCtrlMessage(int sid, const char *msg, size_t msg_len)
{
    MP_DEBUG("%s", __FUNCTION__);
	if (send(sid, msg, msg_len, 0) <= 0) {
        MP_ASSERT(0);
		return -1;
	}

	return 0;
}
