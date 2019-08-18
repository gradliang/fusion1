#ifndef __FTP_STRING__
#define __FTP_STRING__

static const BYTE *m_pbFTP_Commands[FTP_CMD_TOTAL_NUM] = {
    "USER",    "PASS",    "ACCT",    "CWD",    "CDUP",    "SMNT",
    "REIN",    "QUIT",    "PORT",    "PASV",   "TYPE",    "STRU",
    "MODE",    "RETR",    "STOR",    "STOU",   "APPE",    "ALLO",
    "REST",    "RNFR",    "RNTO",    "ABOR",   "DELE",    "RMD",
    "MKD",     "PWD",     "LIST",    "NLST",   "SITE",    "SYST",
    "STAT",    "HELP",    "NOOP"
};

static const WORD m_wFTP_Reply_Code_Mapping[FTP_REPLY_MSG_NUM] = {
    110, 120, 125, 150, 200, 202, 211, 212, 213, 214,
    215, 220, 221, 225, 226, 227, 230, 250, 257, 331,
    332, 350, 421, 425, 426, 450, 451, 452, 500, 501,
    502, 503, 504, 530, 532, 550, 551, 552, 553
};

static const BYTE *m_pbFTP_Reply_Strings[FTP_REPLY_MSG_NUM] = {
#if 1    
    "Restart marker reply.", //110
    "Service ready in nnn minutes.",  //120
    "Data connection already open; transfer stating.",  //125
    "Opening data connection.", //150
    "Command okay.", //200
    "Command not implemented, superfluous at this state.",  //202
    "System status, or system help reply",  //211
    "Directory status.",    //212
    "File status.",  //213
    "Help message.", //214
    "uItron powered by MP600.", //215
    //"NAME system type.", //215
    "MPX M600 FTP Server. Service ready for new user.",   //220
    //"Service ready for new user.",   //220
    "Goodbye!",  //221
    "Data connection open; no transfer in progress.",   //225
    "Transfer complete. Closing data connection.", //226
    "Entering Passive Mode.",   //227
    "User logged in, proceed.", //230
    "Requested file action okay, completed.",   //250
    "PATHNAME create.", //257
    "User name okay, need password.",   //331
    "Need account for login.",  //332
    "Requested file action pending further information.",   //350
    "Service not available, closing control connection.",    //421
    "Can't open data connection.",  //425
    "Connection closed; transfer aborted.", //426
    "Requested file action not taken. File unavailable", //450
    "Requested action aborted : local error in processing.", //451
    "Requested action not taken. Insufficient storage space in system.",  //452
    "Syntax error, command unrecognized.",  //500
    "Syntax error in parameters or arguments.", //501
    "Command not implemented.", //502
    "Bad sequence of commands.",    //503
    "Command not implemented for that parameter.",  //504
    "Not logged in.",   //530
    "Need account for storing files.",  //532
    "Requested action not taken. File unavailable.",  //550
    "Requesta action aborted : page type unknown.", //551
    "Requested file action aborted. Exceeded stroage allocation.",   //552
    "Requested action not taken. File name not allowed"   //553    
#else
    "110-Restart marker reply.", //110
    "120-Service ready in nnn minutes.",  //120
    "125-Data connection already open; transfer stating.",  //125
    "150-File status okay; about to open data connection.", //150
    "200-Command okay." //200
    "202-Command not implemented, superfluous at this state.",  //202
    "211-System status, or system help reply",  //211
    "212-Directory status.",    //212
    "213-File status.",  //213
    "214-Help message.", //214
    "215-NAME system type.", //215
    "220-Service ready for new user.",   //220
    "221-Goodbye!",  //221
    "225-Data connection open; no transfer in progress.",   //225
    "226-Closing data connection. Requested file action successful.", //226
    "227-Entering Passive Mode.",   //227
    "230-User logged in, proceed.", //230
    "250-Requested file action okay, completed.",   //250
    "257-PATHNAME create.", //257
    "331-User name okay, need password.",   //331
    "332-Need account for login.",  //332
    "350-Requested file action pending further information.",   //350
    "421-Service no available, closing control connection.",    //421
    "425-Can't open data connection.",  //425
    "426-Connection closed; transfer aborted.", //426
    "450-Requested file action not taken. File unavailable", //450
    "451-Requested action aborted : local error in processing.", //451
    "452-Requested action not taken. Insufficient storage space in system.",  //452
    "500-Syntax error, command unrecognized.",  //500
    "501-Syntax error in parameters or arguments.", //501
    "502-Command not implemented.", //502
    "503-Bad sequence of commands.",    //503
    "504-Command not implemented for that parameter.",  //504
    "530-Not logged in.",   //530
    "532-Need account for storing files.",  //532
    "550-Requested action not taken. File unavailable.",  //550
    "551-Requesta action aborted : page type unknown.", //551
    "552-Requested file action aborted. Exceeded stroage allocation.",   //552
    "553-Requested action not taken. File name not allowed"   //553    
#endif
};
#endif

