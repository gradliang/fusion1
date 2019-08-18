#ifndef __FTP_SERVER_H__
#define __FTP_SERVER_H__

#define ISO_LF      0x0a
#define ISO_CR      0x0d
#define ISO_SP      0x20

#define FTP_REPLY_MSG_NUM       39  //Refer to FTP_String.h
#define FTP_STR_BUF_SIZE        256 //Global
//#define FTP_BLOCK_SIZE          512 //Local
#define FTP_DEFAULT_MSS         1460

#define FTP_CONN_DRIVE          SD_MMC

#define FTP_EVENT_CONNECT       BIT0
#define FTP_EVENT_ABORT         BIT1
#define FTP_EVENT_TIMEOUT       BIT2
#define FTP_EVENT_ACK           BIT3
#define FTP_EVENT_NEWDATA       BIT4
#define FTP_EVENT_REMIX         BIT5
#define FTP_EVENT_POLL          BIT6
#define FTP_EVENT_CLOSE         BIT7
#define FTP_EVENT_DP_CONNECT    BIT16
#define FTP_EVENT_DP_ABORT      BIT17
#define FTP_EVENT_DP_TIMEOUT    BIT18
#define FTP_EVENT_DP_ACK        BIT19
#define FTP_EVENT_DP_NEWDATA    BIT20
#define FTP_EVENT_DP_REMIX      BIT21
#define FTP_EVENT_DP_POLL       BIT22
#define FTP_EVENT_DP_CLOSE      BIT23
#define FTP_EVENT_SCAN_LIST     BIT31

#define FTP_ST_CONNECT(ftpstate)        (ftpstate.dwState.bitConnect)
#define FTP_ST_USER_OK(ftpstate)        (ftpstate.dwState.bitUserConfirm)
#define FTP_ST_PASS_OK(ftpstate)        (ftpstate.dwState.bitPassConfirm)
#define FTP_ST_PASSIVEMODE(ftpstate)    (ftpstate.dwState.bitPassiveMode)
#define FTP_ST_DP_CONNECT(ftpstate)     (ftpstate.dwState.bitDPConnect)
#define FTP_ST_SENDING(ftpstate)        (ftpstate.dwState.bitSending)
#define FTP_ST_RECEVING(ftpstate)       (ftpstate.dwState.bitReceving)
#define FTP_ST_TRANSMIT_END(ftpstate)   (ftpstate.dwState.bitTransmitEnd)
#define FTP_ST_SCAN_MEDIA(ftpstate)     (ftpstate.dwState.bitScanMedia)
#define FTP_ST_WAIT_ACK(ftpstate)       (ftpstate.dwState.bitWaitAck)
#define FTP_ST_WAIT_DP_ACK(ftpstate)    (ftpstate.dwState.bitWaitDPAck)
#define FTP_ST_WAIT_CLOSE(ftpstate)     (ftpstate.dwState.bitWaitClose)
#define FTP_ST_CUR_PATH(ftpstate)       (ftpstate.dwState.bitCurPath)
#define FTP_ST_CUR_TYPE(ftpstate)       (ftpstate.dwState.bitType)
#define FTP_ST_CUR_MODE(ftpstate)       (ftpstate.dwState.bitMode)
#define FTP_ST_CUR_STRU(ftpstate)       (ftpstate.dwState.bitStructure)

#define FTP_ML_PREV_CMD(ftpstate)       (ftpstate.dwMail.bitPrevCmd)
#define FTP_ML_COUNT(ftpstate)          (ftpstate.dwMail.bitCount)
/*-----------------------------------------------------------------------------------*/
#define SWAP_WORD(x)        (((x & 0xff) << 8) | ((x & 0xff00) >> 8))

/* note: these macros meet the date/time bit fields on FAT12/16/32 file systems only. They do not meet exFAT */
#define FTP_DATE_YEAR(x)    (((SWAP_WORD(x) & 0xfe00) >> 9) + 1980)
#define FTP_DATE_MONTH(x)   ((SWAP_WORD(x) & 0x01e0) >> 5)
#define FTP_DATE_DAY(x)     (SWAP_WORD(x) & 0x001f)

#define FTP_SERVER_PORT     21
#define FTP_DATA_PORT       4998

enum{
    FTP_PATH_ROOT,
    FTP_PATH_PHOTO,
    FTP_PATH_MUSIC,
    FTP_PATH_VIDEO,
};

//Data Types, default type is "ASCII"
enum{
    FTP_DATA_TYPE_ASCII,
    FTP_DATA_TYPE_EBCDIC,
    FTP_DATA_TYPE_IMAGE,
    FTP_DATA_TYPE_LOCAL,
};

//Format control parameter for "ASCII" and "EBCDIC" type. Default type is "NON PRINT"
enum{
    FTP_DATA_TYPE_NON_PRINT,
    FTP_DATA_TYPE_TELNET_FMT_CTL,
    FTP_DATA_TYPE_CARRIAGE_CTL,
};

//Data structure
enum{
    FTP_DATA_STRU_FILE,     //Continuous sequece of data bytes.
    FTP_DATA_STRU_RECORD,   //Sequential records.
    FTP_DATA_STRU_PAGE,     //Independent indexed pages.
};

//Data mode
enum{
    FTP_DATA_MODE_STREAM,   //Transmitted as a stream of bytes.
    FTP_DATA_MODE_BLOCK,    //Transmitted as a series of data blocks.
    FTP_DATA_MODE_COMPRESS, //
};

//Page type in record structure
enum{
    FTP_PAGE_TYPE_LAST_PAGE,        //Header length = 4, Data length = 0. End of a paged structured transmission.
    FTP_PAGE_TYPE_SIMPLE_PAGE,      //Header length = 4. Normal type of simple paged files.
    FTP_PAGE_TYPE_DESCRIPTOR_PAGE,  //Transmit the descriptive information.
    FTP_PAGE_TYPE_ACCESS_CTL_PAGE,  //Header length = 5. Additional header field for paged files with page level access control information.
};

//Data transmission mode
enum{
    FTP_TRANS_MODE_STREAM,      //Transmitted as a stream of bytes.
    FTP_TRANS_MODE_BLOCK,       //Transmitted as a series of data blocks.
    FTP_TRANS_MODE_COMPRESSED,  //Transmitted three kinds of information.
};

//Descriptor code in block header
enum{
    FTP_BLOCK_EOR               = 0x80, //End of data block iS EOR.
    FTP_BLOCK_EOF               = 0x40, //End of data block is EOF.
    FTP_BLOCK_SUSPECTED_ERROR   = 0x20, //Suspected errors in data record.
    FTP_BLOCK_RESTART_MARKER    = 0x10, //Data block is restart marker.
};

//Information be sent in compressed mode
enum{
    FTP_COMPRESSED_REGULAR_DATA,
    FTP_COMPRESSED_BYTE_STRING,
    FTP_COMPRESSED_COMPRESSED_DATA,
};

//FTP commands
enum
{
//Access control commands
    FTP_CMD_USER,   //User name
    FTP_CMD_PASS,   //Password     
    FTP_CMD_ACCT,   //Account
    FTP_CMD_CWD,    //Change working directory     
    FTP_CMD_CDUP,   //Change to parent directory     
    FTP_CMD_SMNT,   //Structure mount     
    FTP_CMD_REIN,   //Reinitialize     
    FTP_CMD_QUIT,   //Logout     
//Transfer parameter commands
    FTP_CMD_PORT,   //Data port     
    FTP_CMD_PASV,   //Passive     
    FTP_CMD_TYPE,   //Reprentation type     
    FTP_CMD_STRU,   //File structure     
    FTP_CMD_MODE,   //Transfer mode
//FTP service commands    
    FTP_CMD_RETR,   //Retrieve     
    FTP_CMD_STOR,   //Store     
    FTP_CMD_STOU,   //Store unique     
    FTP_CMD_APPE,   //Append (with create)     
    FTP_CMD_ALLO,   //Allocate     
    FTP_CMD_REST,   //Restart     
    FTP_CMD_RNFR,   //Rename from
    FTP_CMD_RNTO,   //Rename to     
    FTP_CMD_ABOR,   //Abort     
    FTP_CMD_DELE,   //Delete     
    FTP_CMD_RMD,    //Remove directory     
    FTP_CMD_MKD,    //Make directory     
    FTP_CMD_PWD,    //Print working directory     
    FTP_CMD_LIST,   //List     
    FTP_CMD_NLST,   //Name list    
    FTP_CMD_SITE,   //Site parameters     
    FTP_CMD_SYST,   //System     
    FTP_CMD_STAT,   //Status     
    FTP_CMD_HELP,   //Help     
    FTP_CMD_NOOP,   //No-op    

    FTP_CMD_TOTAL_NUM,
};

//Reply code of first digit
enum{
    REPLY_POSITIVE_PRELIMINARY          = 100,  //The request action is being initiated.
    REPLY_POSITIVE_COMPLETION           = 200,  //The requested action has been successfully completed.
    REPLY_POSITIVE_INTERMEDIATE         = 300,  //The command has been accepted, but the requested action is being held in abeyance.
    REPLY_TRANSIENT_NEGATIVE_COMPLETION = 400,  //The command was not accpeted and the requested action did not take place, but the error confistion is temporary and the action may be requested again.
    REPLY_PERMAENT_NEGATIVE_COMPLETION  = 500,  //The command was not accepted and the requested action did not take place.
};

//Reply code of second digit
enum{
    REPLY_SYNTAX            = 0,    //Refer to syntax error.
    REPLY_INFORMATION       = 10,   //To request for information
    REPLY_CONNECTIONS       = 20,   //Refer to the control and data connections.
    REPLY_AUTHENT_N_ACCOUNT = 30,   //For login process and accouting procedures
    REPLY_UNSPECIFIED       = 40,   //Unspecified yet.
    REPLY_FILE_SYSTEM       = 50,   //Indicate the status of the server file system.
};

#if 0
typedef struct{
    BYTE bCmdIndex;
    BYTE bType;
    BYTE bStructure;
    BYTE bMode;
    BYTE m_bUserName[16];
    BYTE m_bPassword[16];
    BYTE m_bPath[FTP_STR_BUF_SIZE];
    //Not support currently
    //BYTE bMarker;
    //BYTE m_bAccount[16];
    //DWORD dwIPAddr;
    //WORD wPort;
}ST_FTP_CMD;
#endif

typedef struct{
    BYTE m_bUsername[16];
    BYTE m_bPassword[16];
    BYTE bAuthLevel;
    BYTE bRev1;
    BYTE bRev2;
    BYTE bRev3;
}ST_FTP_AUTH;

typedef struct{
    struct{
        DWORD bitConnect        :1;
        DWORD bitUserConfirm    :1;
        DWORD bitPassConfirm    :1;
        DWORD bitPassiveMode    :1;
        DWORD bitDPConnect      :1;
        DWORD bitSending        :1;
        DWORD bitReceving       :1;
        DWORD bitTransmitEnd    :1;
        DWORD bitScanMedia      :1;
        DWORD bitWaitAck        :1;
        DWORD bitWaitDPAck      :1;
        DWORD bitWaitClose      :1;
        DWORD bitCurPath        :3;
        DWORD bitType           :3;
        DWORD bitMode           :2;
        DWORD bitStructure      :2;
        DWORD bitReserve        :8;
    }dwState;
    struct{
        DWORD bitPrevCmd        :5;
        DWORD bitMessage        :10;
        DWORD bitCount          :7;
        DWORD bitReserve        :10;
    }dwMail;
    ST_FTP_AUTH *pstCurUser;
}ST_FTP_STATE;

extern BYTE g_bXpgStatus;

void ftpserver_init(void);
#endif

