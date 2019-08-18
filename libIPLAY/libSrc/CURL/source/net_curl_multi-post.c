/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: multi-post.c,v 1.4 2006-10-13 14:01:19 bagder Exp $
 *
 * This is an example application source code using the multi interface
 * to do a multipart formpost without "blocking".
 */
 
#include "net_curl_setup.h"

#include <stdio.h>
#include "net_curl_curl.h"

#include <string.h>
#include "igo.h"
#include "api.h"
#include "net_socket.h"

int Multi_Post_HTTP(void)
{
    CURL *curl;
    CURLcode res;

    CURLM *multi_handle;
    int still_running;

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;
    char buf[] = "Expect:";

    U08 FileHandle;
    U08 store_name[40];
    U08 converted_store_name[80];

#if 0    
    //change drive to save photo
    mpx_DriveChange(0);

    sprintf(store_name, "%s.jpg", "13");
    UtilStringCopy0816(converted_store_name, store_name);
    if (mpx_DirNodeLocate(converted_store_name) == NO_ERR)
    {
        CURLPRINTF("13.jpg found, Open it\n");
        FileHandle = mpx_FileOpen();
        if( FileHandle <= 0 )
            return ERR_AUDIO_FILE_OPEN_ERROR;
    }
    else{
        CURLPRINTF("13.jpg not found, return\n");
        return 0;
     }
#endif

 /* Add simple name/content section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "tags",
               CURLFORM_COPYCONTENTS, "auto-upload", CURLFORM_END);

 /* Add simple name/content section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "perms",
               CURLFORM_COPYCONTENTS, "write", CURLFORM_END);

 /* Add simple name/content section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "auth_token",
               CURLFORM_COPYCONTENTS, "72157600195008787-f1efa4e6d9223365", CURLFORM_END);

 /* Add simple name/content section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "api_sig",
               CURLFORM_COPYCONTENTS, "e1dfee7508d5fb8d293f4394cdcae8cf", CURLFORM_END);

 /* Add simple name/content section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "is_public",
               CURLFORM_COPYCONTENTS, "1", CURLFORM_END);

     /* Add simple name/content section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "api_key",
               CURLFORM_COPYCONTENTS, "2d7076217eb2dc94997cba1bb61bd5b5", CURLFORM_END);

 /* Add file/contenttype section */
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "photo",
               CURLFORM_FILE, "1000.jpg",
               CURLFORM_CONTENTTYPE, "image/jpeg", CURLFORM_END);

  curl = curl_easy_init();
  multi_handle = curl_multi_init();

  /* initalize custom header list (stating that Expect: 100-continue is not
     wanted */
  headerlist = curl_slist_append(headerlist, buf);
  if(curl && multi_handle) {
    int perform=0;

    /* set options */
    //curl_easy_setopt(curl, CURLOPT_PROXYTYPE, 0);
    //curl_easy_setopt(curl, CURLOPT_PROXY, "192.168.47.10");
    //curl_easy_setopt(curl, CURLOPT_PROXYPORT, 8002);

    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, "http://flickr.com/services/upload");
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    //set filehandle
    //curl_easy_setopt(curl, CURLOPT_FILE, FileHandle);

    curl_multi_add_handle(multi_handle, curl);

    while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running));

    while(still_running) {

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
         function calls, *and* you make sure that maxfd is bigger than -1
         so that the call to select() below makes sense! */

      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

      switch(rc) {
      case -1:
        /* select error */
        break;
      case 0:
        printf("timeout!\n");
      default:
        /* timeout or readable/writable sockets */
        printf("perform!\n");
        while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running));
        printf("running: %d!\n", still_running);
        break;
      }

    }

    curl_multi_cleanup(multi_handle);

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* then cleanup the formpost chain */
    curl_formfree(formpost);

    /* free slist */
    curl_slist_free_all (headerlist);

    //write finish, close file
    //mpx_FileClose(FileHandle);

  }
  return 0;
}
