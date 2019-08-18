#ifndef __NET_FTP_H
#define __NET_FTP_H

#include "net_socket.h"
#include "net_packet.h"
#include "error.h"

#define FTP_OK                       NO_ERR
#define FTP_ERROR                    (-1)

#define FTP_BLOCK_SIZE              4096

typedef struct
{
    U16 nameLength;
    U16 dirFlag;
    U32 size;
    U08 nameString[256];
} ST_FTP_FILE_PROPERTY;

typedef struct
{
    ST_FTP_FILE_PROPERTY file;
    U32 next;
} ST_FTP_LIST_NODE;

typedef struct
{
    ST_FTP_LIST_NODE* head;
    ST_FTP_LIST_NODE* tail;
    U16 itemNum;
} ST_FTP_FILE_LIST;


#define SERVER_ENABLE                       0x0001

typedef struct
{
    U16 port;
    U16 flag;
    U08* serverName;
    U08* welcomeMessage;
    U08 u08CurNodePosition;
    U08 u08Reserved;
    int ctrlSockId;
} ST_FTP_SERVER_PROPERTY;


#define FTP_MSG_INQUIRY_START               0x00000001
#define FTP_MSG_INQUIRY_STOP                0x00000002
#define FTP_MSG_CLIENT_IDLE_TIMEOUT         0x00000003
#define FTP_MSG_SERVER_ENABLE               0x00000004
#define FTP_MSG_SERVER_DISABLE              0x00000005
                                            
#ifndef MP600
#define MPX_FTP_PRIVATE_PORT                1022
#define MPX_FTP_CTRL_ID                     'MPX '
                                            
#define MPX_MSG_SERVER_DISCOVER             0x01
#define MPX_MSG_SERVER_REPLY                0x02
#else
/* FTP session flags */
#define FTPF_SENDING                        0x0001
#define FTPF_RECEVING                       0x0002
#define FTPF_PASSIVEMODE                    0x0004
#define FTPF_DATAISCONNECTED                0x0008 /* data connection is connected */
#endif
                                            
#define MPX_SERVER_NAME_LENGTH              20

#define MPX_FTP_CLIENT_IDLE_TIMEOUT         120

#define ftp_malloc(sz)   ext_mem_malloc(sz)
#define ftp_mfree(ptr)   ext_mem_free(ptr)

#define MAX_FTP_SESSIONS       8

typedef struct
{
    U32 mpxId;
    U08 id;
    U08 length;
    U08 data[256];
} ST_MPX_FTP_MESSAGE;

//FTP client API
S32 mpx_FtpConnect(U32 u32DstAddr, U16 u16DstPort, U08** pOutWelcomeMsg, BOOL isBlocking);
void mpx_FtpDisconnect(U16 u16Sid);
S32 mpx_FtpRead(U16 u16Sid, U08* u08Data, U32 u32BufferSize);
U16 mpx_FtpSendCommandW(U16 u16Sid, U08* message, U16 messageSize, U08* buffer);
U08* mpx_FtpUserName(U16 u16Sid, U08* username);
U08* mpx_FtpPassword(U16 u16Sid, U08* password);
U08* mpx_FtpBinaryModeSet(U16 u16Sid);
U08* mpx_FtpCurrentDir(U16 u16Sid);

//FTP server API
S32 mpx_FtpServerEnable(void);
S32 mpx_FtpServerDisable();

//FTP common API
S32 mpx_FtpServiceEnable();
S32 mpx_FtpServiceDisable();

//FTP debug print
void mpx_FtpRtnMessage(U08* message);

#ifdef MP600
#include "net_ftpserver.h"
#endif

#endif
