#ifndef __BTFTP_CLIENT_H
#define __BTFTP_CLIENT_H

#include "btsetting.h"

#define BT_FTP_CLIENT_FOLDER_NUM    20
#define BT_FTP_CLIENT_FILE_NUM    20
#define BT_FTP_CLIENT_HEADER_NUM    2
#define BT_FTP_CLIENT_BODY_HEADER_NUM    8
#define BT_FTP_CLIENT_RECURIVE_FOLDER_NUM    20 // means one folder can include 20 folders

//#if ((BT_PROFILE_TYPE & BT_FTP_CLIENT) == BT_FTP_CLIENT)

//rick ftp client//////////////////////////////////////////////////

typedef struct{
    char *headerstring;
    unsigned long stringsize;
}ST_BT_FTP_CLIENT_STRING;

#if BLUETOOTH == ENABLE

////////////////////////////////////////////////////
char XML_VERSION_STRING[] = "?xml version=";
#define XML_VERSION_STRING_SIZE    13

char FOLDER_LISTING_VERSION_STRING[] = "folder-listing version=";
#define FOLDER_LISTING_VERSION_STRING_SIZE 23
//////////////////////////////////////////////////////

/////////////////////////////////////////////////////
char PARENT_FOLDER[] = "parent-folder ";
#define PARENT_FOLDER_SIZE 14

char FOLDER_NAME[] = "folder name=";
#define FOLDER_NAME_SIZE    12

char FILE_NAME[] = "file name=";
#define FILE_NAME_SIZE  10

char FILE_SIZE[] = "size=";
#define FILE_SIZE_SIZE   5

char MODIFIED_STRING[] = "modified=";
#define MIDIFIED_STRING_SIZE   9

char USER_PERM[] = "user-perm=";
#define USER_PERM_SIZE  10

char MEM_TYPE[] = "mem-type=";
#define MEM_TYPE_SIZE   9

char FOLDER_LISTING_END_STRING[] = "/folder-listing";
#define FOLDER_LISTING_END_STRING_SIZE 15
///////////////////////////////////////////////////////
ST_BT_FTP_CLIENT_STRING BT_FTP_CLIENT_STRINGS[BT_FTP_CLIENT_HEADER_NUM+BT_FTP_CLIENT_BODY_HEADER_NUM]={
    {XML_VERSION_STRING,XML_VERSION_STRING_SIZE},
    {FOLDER_LISTING_VERSION_STRING,FOLDER_LISTING_VERSION_STRING_SIZE},

    {PARENT_FOLDER,PARENT_FOLDER_SIZE},
    {FOLDER_NAME,FOLDER_NAME_SIZE},
    {FILE_NAME,FILE_NAME_SIZE},
    {FILE_SIZE,FILE_SIZE_SIZE},
    {MODIFIED_STRING,MIDIFIED_STRING_SIZE},
    {USER_PERM,USER_PERM_SIZE},
    {MEM_TYPE,MEM_TYPE_SIZE},
    {FOLDER_LISTING_END_STRING,FOLDER_LISTING_END_STRING_SIZE},
};

//////////////////////////////////////////////////////////////
#endif//#if BLUETOOTH == ENABLE


#endif

