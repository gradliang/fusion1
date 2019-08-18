/*****************************************************************************
 *                                  _   _ ____  _     
 *  Project                     ___| | | |  _ \| |    
 *                             / __| | | | |_) | |    
 *                            | (__| |_| |  _ <| |___ 
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: multi-double.c,v 1.4 2006-10-13 14:01:19 bagder Exp $
 *
 * This is a very simple example using the multi interface.
 */

#include "net_curl_setup.h"

#include <stdio.h>
#include "net_curl_curl.h"

#include <string.h>
#include "igo.h"
#include "api.h"
#include "net_socket.h"

U32 LinkFileBufferSize = 4*1024; //512K

U32 LinkBuffer = 0;

/*
 * Simply download two HTTP files!
 */
int Multi_Double_HTTP(void)
{
    CURL *http_handle;
    CURL *http_handle2;
    CURLM *multi_handle;
    U08 store_name[40];
    U08 converted_store_name[80];
    U08 FileHandle, FileHandle2;
    size_t LinkFileSize, LinkFileSizeIndex = 0;

    int still_running; /* keep number of running handles */

    CURLPRINTF("Multi_Double_HTTP");

    //change drive to save photo
    mpx_DriveChange(0);

    //get link.txt
    if( mpx_DirNodeLocate((U16*)"\0/\0l\0i\0n\0k\0.\0t\0x\0t\0") )
    {
        DPrintf("Can't find link.txt");
        return OPEN_AUDIO_PARSER_ERROR;
    }
    else
    {
        DPrintf("link.txt found, Open it\n");
        FileHandle = mpx_FileOpen();
        if( FileHandle <= 0 )
            return ERR_AUDIO_FILE_OPEN_ERROR;
    }

    //allocate image buffer
    LinkBuffer = mpx_Malloc(LinkFileBufferSize);
    if(!LinkBuffer){
        DPrintf("Image allocate fail");
        return 0;
    }     

    memset(LinkBuffer, 0, LinkFileBufferSize);

    LinkFileSize = mpx_FileRead(FileHandle, LinkBuffer, LinkFileBufferSize);
    CURLPRINTF("LinkFileSize = %d", LinkFileSize);

    mpx_FileClose(FileHandle);

    http_handle = curl_easy_init();
    http_handle2 = curl_easy_init();

    /* set options */
    curl_easy_setopt(http_handle, CURLOPT_PROXYTYPE, 0);
    curl_easy_setopt(http_handle, CURLOPT_PROXY, "192.168.47.10");
    curl_easy_setopt(http_handle, CURLOPT_PROXYPORT, 8002);

    curl_easy_setopt(http_handle, CURLOPT_URL, "http://farm1.static.flickr.com/227/486407163_6d3b741eaf.jpg");

    memset(store_name, 0, 40);
    memset(converted_store_name, 0, 80);
    sprintf(store_name, "%s.jpg", "486407163_6d3b741eaf");
    UtilStringCopy0816(converted_store_name, store_name);
    if (mpx_DirNodeLocate(converted_store_name) == NO_ERR)
    {
        mpx_DirFileDelete();
        DPrintf("Delete file %s", store_name);
    }
    
    FileHandle = mpx_FileCreateOpen(converted_store_name, 0, FILE_CREATE_OPEN);
    
    curl_easy_setopt(http_handle, CURLOPT_FILE, FileHandle);

    /* set options */
    //curl_easy_setopt(http_handle2, CURLOPT_PROXYTYPE, 0);
    //curl_easy_setopt(http_handle2, CURLOPT_PROXY, "192.168.47.10");
    //curl_easy_setopt(http_handle2, CURLOPT_PROXYPORT, 8002);
    
    curl_easy_setopt(http_handle2, CURLOPT_URL, "http://farm1.static.flickr.com/143/325342927_50f6f268ff.jpg");

    memset(store_name, 0, 40);
    memset(converted_store_name, 0, 80);
    sprintf(store_name, "%s.jpg", "325342927_50f6f268ff");
    UtilStringCopy0816(converted_store_name, store_name);
    if (mpx_DirNodeLocate(converted_store_name) == NO_ERR)
    {
        mpx_DirFileDelete();
        DPrintf("Delete file %s", store_name);
    }
    
    FileHandle2 = mpx_FileCreateOpen(converted_store_name, 0, FILE_CREATE_OPEN);
    
    curl_easy_setopt(http_handle2, CURLOPT_FILE, FileHandle2);

    /* init a multi stack */
    multi_handle = curl_multi_init();

    /* add the individual transfers */
    curl_multi_add_handle(multi_handle, http_handle);
    curl_multi_add_handle(multi_handle, http_handle2);

  /* we start some action by calling perform right away */
  while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running));

  while(still_running) {
#if 0
    struct timeval timeout;
    int rc; /* select() return code */

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    /* get file descriptors from the transfers */
    curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    /* In a real-world program you OF COURSE check the return code of the
       function calls, *and* you make sure that maxfd is bigger than -1 so
       that the call to select() below makes sense! */

    rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

    switch(rc) {
        case -1:
            /* select error */
        break;
        case 0:
        break;
        default:
            /* timeout or readable/writable sockets */
            while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running));
        break;
        }
#else
        while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running));
#endif
    }

    CURLPRINTF("curl_multi_cleanup\n");
    
    curl_multi_cleanup(multi_handle);

    curl_easy_cleanup(http_handle);
    curl_easy_cleanup(http_handle2);

    //write finish, close file
    mpx_FileClose(FileHandle);
    mpx_FileClose(FileHandle2);
    
    return 0;
}
