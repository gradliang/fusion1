/**
 * \addtogroup apps
 * @{
 */

/**
 * \defgroup POP3 POP3 Mail Client
 * @{
 *
 * This example implements a POP3 mail client that is able to download mails
 * from a mail server. 
 *
 */

/**
 * \file
 * An example of POP3 client file.
 *
 */

/** @} */
/** @} */

#define LOCAL_DEBUG_ENABLE 0


#include <string.h>

#include "global612.h"
#include "mpTrace.h"

#include <linux/types.h>
#include "typedef.h"
#include "net_packet.h"
#include "socket.h"
#include "net_socket.h"
#include "net_ns.h"
#include "net_device.h"
#include "net_netdb.h"
#include "os.h"
#include "linux/list.h"
#include "pop3_main.h"

#include "openssl/rsa.h"
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#include "zipint.h"

struct ppp_connect{
	SSL_CTX* ctx;
	SSL*	 handle;
	X509*	 server_cert;
	u32      flags;
#define FLAG_SSL 0x1
};

#define SAVE_FILE 0

#define SHOW_FILE 0

#define UNZIP_FILE 0

#define WRITE_TO_SD 0

#if WRITE_TO_SD
#define POP3_READ_BUFFER_SIZE (256*1024)
#define POP3_PASER_BUFFER_SIZE (256*1024*3)

char *pop3_readbuf=NULL;
char *pop3_paserbuf=NULL;
STREAM *pop3handle = NULL;
DWORD pop3_paser_total_idx = 0;
DWORD pop3_paser_idx = 0;

DWORD pop3_tmp_filesize = 0;

#endif

char *stristr(const char *String, const char *Pattern)
{
      char *pptr, *sptr, *start;

      for (start = (char *)String; *start != NULL; start++)
      {
            /* find start of pattern in string */
            for ( ; ((*start!=NULL) && (toupper(*start) != toupper(*Pattern))); start++)
                  ;
            if (NULL == *start)
                  return NULL;

            pptr = (char *)Pattern;
            sptr = (char *)start;

            while (toupper(*sptr) == toupper(*pptr))
            {
                  sptr++;
                  pptr++;

                  /* if end of pattern then pattern was found */

                  if (NULL == *pptr)
                        return (start);
            }
      }
      return NULL;
}


void Save_Jpeg_File(char* name, int len, char* buffer){
    BOOL boDriveAdded = FALSE;
	BYTE bMcardId = SD_MMC;
	STREAM *handle = NULL;
	static DRIVE *sDrv;
	static int ret = 1;
	char* file_ext;
	int fswr = 0;


	file_ext = name + strlen(name);

	while(*file_ext != 0x2e){
		file_ext--;
	}
	*file_ext = '\0';
	file_ext++;

	strtoupper(name);
	strtoupper(file_ext);

    if (SystemCardPlugInCheck(bMcardId))
	{
		SystemDeviceInit(bMcardId);
#if 1
		if (!SystemCardPresentCheck(bMcardId))
		{	   
			mpDebugPrint("-E- SystemDeviceInit fail");
		}
		else
		{
			if (!(boDriveAdded = DriveAdd(bMcardId)))
				mpDebugPrint("-E- DriveAdd fail");
		}
#endif
		if (boDriveAdded)
		{
			DRIVE *drv;
			drv = DriveChange(bMcardId);
			if (DirReset(drv) != FS_SUCCEED)
				return;

			handle = FileSearch(drv, name, file_ext, E_FILE_TYPE);
			if (handle != NULL)
			{
					sDrv = DriveGet(bMcardId);
					ret = CreateFile(sDrv, name, file_ext);
					if (ret)
						UartOutText("create file fail\r\n");
					handle = FileOpen(sDrv);
					if (!handle)
						UartOutText("open file fail\r\n");
					else
					{
						fswr = FileWrite(handle, buffer, len);
						FileClose(handle);
					}
				}
			else
				{		
					sDrv=DriveGet(bMcardId);
					handle = FileOpen(sDrv);
					DeleteFile(handle);
					#if 1
					ret=CreateFile(sDrv, name, file_ext);
					if (ret)
						UartOutText("create file fail\r\n");
					handle=FileOpen(sDrv);
					if(!handle)
						UartOutText("open file fail\r\n");
					else
					{
						fswr = FileWrite(handle,buffer,len);
						FileClose(handle);
					}
					#endif
				}
		}
    }

}
void Save_Jpeg_File2Disk(BYTE bMcardId,char* name, int len, char* buffer)
{
	BOOL boDriveAdded = FALSE;
	STREAM *handle = NULL;
	static DRIVE *sDrv;
	static int ret = 1;
	char* file_ext;
	int fswr = 0; 
	file_ext = name + strlen(name);
	while(*file_ext != 0x2e){
		file_ext--;
		}
	*file_ext = '\0';
	file_ext++;
	strtoupper(name);
	strtoupper(file_ext);
	if(name[0] == 0)
		name = "AAA";
	DRIVE *drv;
	drv = DriveChange(bMcardId);
	if (DirReset(drv) != FS_SUCCEED)
		return NULL;
	mpDebugPrint("Save_Jpeg_File2Nand %s.%s",name,file_ext);
	//__asm("break 100");
	handle = FileSearch(drv, name, file_ext, E_FILE_TYPE);
	if (handle != NULL)
	{
		sDrv = DriveGet(bMcardId);
		ret = CreateFile(sDrv, name, file_ext);
		if (ret)
			UartOutText("create file fail\r\n");
		handle = FileOpen(sDrv);
		if (!handle)
			UartOutText("open file fail\r\n");
		else
		{
			fswr = FileWrite(handle, buffer, len);
			FileClose(handle);
		}
	}
	else
	{		
		sDrv = DriveGet(bMcardId);
		handle = FileOpen(sDrv);
		DeleteFile(handle);
		#if 1
		ret = CreateFile(sDrv, name, file_ext);
		if (ret)
			UartOutText("create file fail\r\n");
		handle = FileOpen(sDrv);
		if (!handle)
			UartOutText("open file fail\r\n");
		else
		{
			fswr = FileWrite(handle, buffer, len);
			FileClose(handle);
		}
		#endif
	}
}


int SSL_init(void){
#if Make_CURL
	MP_DEBUG("SSL_init");
	
	/* Lets get nice error messages */
	SSL_load_error_strings();
	
	/* Setup all the global SSL stuff */
	if (!SSL_library_init())
	  return 0;
#endif	
	return 1;
}

int SSL_close(struct ppp_connect* conn){
#if Make_CURL
	
	ERR_remove_state(0);
	
	if(conn->handle){
		(void)SSL_shutdown(conn->handle);
		SSL_set_connect_state(conn->handle);

		SSL_free (conn->handle);
		conn->handle = NULL;
	}
	if(conn->ctx) {
		SSL_CTX_free (conn->ctx);
	}
#else
 return 0;
#endif	
}

/**
 * @ingroup POP3
 * @brief Connect to server
 *
 *
 * @param conn connection session
 * @param addr server address
 */

int pop3_connect(struct ppp_connect *conn, U32 addr, unsigned short port){
	int ret = 0;
	int recvbuffer[512];
	int recvlength;
	
	ret = mpx_DoConnect(addr, port, TRUE);

	if(ret < 0){
		MP_DEBUG("Can't get socket");
		return 0;
	}

	memset(recvbuffer, 0, sizeof(recvbuffer));

	recvlength = recv( ret, recvbuffer , sizeof(recvbuffer),0);

	MP_DEBUG("length = %d", recvlength);
	MP_DEBUG("recvbuffer = %s", recvbuffer);

	if(strncmp(recvbuffer, "+OK", 3)){
		MP_DEBUG("connect is not accept");
		return 0;
	}	
	return ret;
}

int pop3_seed(){
    int len;
#if Make_CURL
    char *area;


	MP_DEBUG("pop3_seed");
    /* Changed call to RAND_seed to use the underlying RAND_add implementation
     * directly.  Do this in a loop, with the amount of additional entropy
     * being dependent upon the algorithm used by Curl_FormBoundary(): N bytes
     * of a 7-bit ascii set. -- Richard Gorton, March 11 2003.
     */

    do {
      area = Curl_FormBoundary();
      if(!area)
        return 3; /* out of memory */

      len = (int)strlen(area);

	  MP_DEBUG("len = %d", len);
      RAND_add(area, len, (len >> 1));

      mpx_Free(area); /* now remove the random junk */
    } while (!RAND_status());
#else
  return 0;
#endif
}

/**
 * @ingroup POP3
 * @brief Connect to server via SSL
 *
 *
 * @param conn connection session
 * @param addr server address
 */
int pop3_connect_ssl(struct ppp_connect *conn, U32 addr, unsigned short port){
	int ret = 0;
#if Make_CURL
	int recvbuffer[512];
	int recvlength;
	SSL_METHOD *req_method=NULL;
	int err;

	MP_DEBUG("pop3_connect_ssl");
	
	ret = mpx_DoConnect(addr, port, TRUE);

	if(ret < 0){
		MP_DEBUG("Can't get socket");
		return 0;
	}

	pop3_seed();

	req_method = SSLv23_client_method();

	conn->ctx = SSL_CTX_new(req_method);

	if(!conn->ctx) {
	  MP_DEBUG("SSL: couldn't create a context!");
	  closesocket(ret);
	  return 0;
	}

	/* OpenSSL contains code to work-around lots of bugs and flaws in various
	   SSL-implementations. SSL_CTX_set_options() is used to enabled those
	   work-arounds. The man page for this option states that SSL_OP_ALL enables
	   all the work-arounds and that "It is usually safe to use SSL_OP_ALL to
	   enable the bug workaround options if compatibility with somewhat broken
	   implementations is desired."

	*/
	SSL_CTX_set_options(conn->ctx, SSL_OP_ALL);


	conn->handle = SSL_new(conn->ctx);

	if (!conn->handle) {
	  	MP_DEBUG("SSL: couldn't create a context (handle)!");
		SSL_close(conn);
	  
	  	closesocket(ret);
	  	return 0;
	}


	SSL_set_connect_state(conn->handle);


	/* pass the raw socket into the SSL layers */
	if (!SSL_set_fd(conn->handle, ret)) {
	   MP_DEBUG("SSL: SSL_set_fd failed: %s",
			 ERR_error_string(ERR_get_error(),NULL));
	   SSL_close(conn);
	   closesocket(ret);
	   return 0;
	}

	err = SSL_connect(conn->handle);
	if(err != 1){
		MP_DEBUG("SSL connect fail");
		SSL_close(conn);
		closesocket(ret);
		return 0;
	}
	MP_DEBUG("err = %d", err);

	recvlength = SSL_read(conn->handle, recvbuffer, sizeof(recvbuffer));

	MP_DEBUG("length = %d", recvlength);
	MP_DEBUG("recvbuffer = %s", recvbuffer);


	if(strncmp(recvbuffer, "+OK", 3)){
		MP_DEBUG("connect is not accept");
		SSL_close(conn);
		closesocket(ret);
		return 0;
	}
#endif
	return ret;
}


int pop3_Auth(struct ppp_connect *conn, int socketid, char *username, char *password){
#if Make_CURL
	int cmdbuffer[50];
	int recvbuffer[512];
	int recvlength;

	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "USER %s\r\n", username);

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);

	memset(recvbuffer, 0, sizeof(recvbuffer));

	if(conn->flags == FLAG_SSL)
		recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
	else
		recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);

	MP_DEBUG("length = %d", recvlength);
	MP_DEBUG("recvbuffer = %s", recvbuffer);

	if(strncmp(recvbuffer, "+OK", 3)){
		MP_DEBUG("user is not accept");
		return 0;
	}

	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "PASS %s\r\n", password);

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	memset(recvbuffer, 0, sizeof(recvbuffer));

	if(conn->flags == FLAG_SSL)
		recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
	else
		recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);


	MP_DEBUG("length = %d", recvlength);
	MP_DEBUG("recvbuffer = %s", recvbuffer);

	if(strncmp(recvbuffer, "+OK", 3)){
		MP_DEBUG("pass is not accept");
		return 0;
	}
#endif	
	return 1;
}


int pop3_Dele(struct ppp_connect *conn, int socketid, int msg){
#if Make_CURL
	
	int cmdbuffer[50];
	int recvbuffer[512];
	int recvlength;
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "DELE %d\r\n", msg);

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	memset(recvbuffer, 0, sizeof(recvbuffer));

	recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);

	MP_DEBUG("length = %d", recvlength);
	MP_DEBUG("recvbuffer = %s", recvbuffer);
    //Don't need check
	//if(strncmp(recvbuffer, "+OK", 3)){
	//	return 0;
	//}
#endif	
	return 1;	
}

int pop3_Stat(struct ppp_connect *conn, int socketid){
#if Make_CURL
	
	int cmdbuffer[50];
	int recvbuffer[512];
	int recvlength;
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "STAT \r\n");

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	memset(recvbuffer, 0, sizeof(recvbuffer));

	if(conn->flags == FLAG_SSL)
		recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
	else
		recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);


	MP_DEBUG("STAT length = %d", recvlength);
	MP_DEBUG("STAT recvbuffer = %s", recvbuffer);

	if(strncmp(recvbuffer, "+OK", 3)){
		return 0;
	}

	//sscanf(recvbuffer, "+OK %d %d", msg, size);
#endif	
	return 1;	
}
/**
 * @ingroup POP3
 * @brief List a mail or all mails
 *
 * @param conn connection session
 * @param socketid socket ID
 * @param MailQ mail queue
 * @param num 0 for all, otherwise, for a specific mail
 */
int pop3_List(struct ppp_connect *conn, int socketid, struct list_head *MailQ, int num){
#if Make_CURL

	int cmdbuffer[50];
	char recvbuffer[512];
	char* list_buffer = NULL;
	char* list_buffer_temp = NULL;
	int list_length = 2048;
	int recvlength, totallength = 0;
	char* pointer, *temp;
	struct timeval tv;
	fd_set fdread;
	int status;

	tv.tv_sec = 4;
	tv.tv_usec = 0;


	FD_ZERO(&fdread);
	FD_SET(socketid, &fdread);
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	if(num)
		sprintf(cmdbuffer, "LIST %d\r\n", num);
	else
		sprintf(cmdbuffer, "LIST\r\n");

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	list_buffer = ext_mem_malloc(list_length);
	if(!list_buffer){
		MP_DEBUG("Memory is no enough");
		return 0;
	}
	
	memset(recvbuffer, 0, sizeof(recvbuffer));
	memset(list_buffer, 0, list_length);

	temp = list_buffer;

	if(conn->flags == FLAG_SSL){
		while(1){
			status = select(socketid+1, &fdread, 0, 0, &tv);
			if(status > 0){
				do{
					recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
					if((totallength + recvlength) > list_length){
						list_buffer_temp = ext_mem_reallocm(list_buffer, list_length + 2048);
						if(!list_buffer_temp){
							MP_DEBUG("Memory is no enough");
							ext_mem_free(list_buffer);
							return 0;
						}
						list_buffer = list_buffer_temp;
						temp = list_buffer + totallength;
						list_length += 2048;
					}
		
					memcpy(temp, recvbuffer, recvlength);
					temp += recvlength;
					totallength += recvlength;
				}
				while(SSL_pending(conn->handle));
			}
			else{
				break;
			}
		}
	}
	else{
		while(1){
			status = select(socketid+1, &fdread, 0, 0, &tv);
			if(status > 0){
				recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);
				if((totallength + recvlength) > list_length){
					list_buffer_temp = ext_mem_reallocm(list_buffer, list_length + 2048);
					if(!list_buffer_temp){
						MP_DEBUG("Memory is no enough");
						ext_mem_free(list_buffer);
						return 0;
					}
					list_buffer = list_buffer_temp;
					temp = list_buffer + totallength;
					list_length += 2048;
				}
	
				memcpy(temp, recvbuffer, recvlength);
				temp += recvlength;
				totallength += recvlength;
			}
			else{
				break;
			}
		}
	}


	MP_DEBUG("LIST length = %d", totallength);
	MP_DEBUG("LIST recvbuffer = %s", list_buffer);

	if(strncmp(list_buffer, "+OK", 3)){
		return 0;
	}

	pointer = list_buffer;
	temp = list_buffer;
	while(1){
		if(*temp == 0x2e && *(temp+1) == 0x0d && *(temp+2) == 0x0a)
			break;
		if(temp > list_buffer + totallength){
			MP_DEBUG("fail to parse list");
			return 0;
		}
		if(*temp == 0x0d){
			*temp = '\0';
			if(*pointer != 0x2b){
				struct MailNode *pnode = NULL;

				pnode = ext_mem_malloc(sizeof(struct MailNode));
				if(!pnode){
					MP_DEBUG("Memory is no enough");
					ext_mem_free(list_buffer);
					return 0;
				}
				
				memset(pnode, 0, sizeof(struct MailNode));

				sscanf(pointer, "%d %d", &pnode->num, &pnode->buff_len);

				INIT_LIST_HEAD(&pnode->AttachQ);

				list_add_tail((struct list_head *) pnode, MailQ);
				
				MP_DEBUG("msg num = %d size = %d", pnode->num, pnode->buff_len);				
			}
			pointer = temp + 2;
		}
		temp++;
	}

	ext_mem_free(list_buffer);
#endif	
	return 1;	
}

/**
 * @ingroup POP3
 * @brief Retrieve mails from server via SSL
 *
 * @param conn connection session
 * @param socketid socket ID
 * @param pnode mail node
 */
#define WANT_WRITE 0 
int pop3_Retr_ssl(struct ppp_connect* conn, int socketid, struct MailNode *pnode){
#if Make_CURL
	int cmdbuffer[50];
	int recvlength, totallength = 0;
	char* buffer_temp = NULL;
	char* temp;
	fd_set fdread;
	int status;
	struct timeval tv;
	int message_is_ok = 0;
	char filename[20];
#if WANT_WRITE//WRITE_TO_SD	
	BYTE bMcardId = SD_MMC;
	STREAM *handle = NULL;
	static DRIVE *sDrv;
	static int ret = 1;
	char *recvbuffer = NULL;
	BYTE file_num[4];
	char pop3_file_ext[32];
#else
	char recvbuffer[1024];
#endif	
	tv.tv_sec = 4;
	tv.tv_usec = 0;


	FD_ZERO(&fdread);
	FD_SET(socketid, &fdread);

    mpDebugPrint("%s pnode->num %d",__func__,pnode->num);
		
	temp = pnode->buffer;


	{
		//unsigned long flags = 1;
	
		//ioctlsocket(socketid, FIONBIO, &flags);
	}
#if WANT_WRITE//WRITE_TO_SD
	DRIVE *drv;
	drv = DriveChange(bMcardId);
	if (DirReset(drv) != FS_SUCCEED)
		return;
#endif	

	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	memset(filename, 0, sizeof(filename));

	sprintf(cmdbuffer, "RETR %d\r\n", pnode->num);

	MP_DEBUG("RETR %d", pnode->num);

	SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));


	
#if WANT_WRITE//WRITE_TO_SD
	recvbuffer = pop3_readbuf;
    mpDebugPrint("recvbuffer %x",recvbuffer);
    mmcp_memset(recvbuffer, 0, POP3_READ_BUFFER_SIZE);
	
	DecString(file_num,pnode->num,3,0);
	mpDebugPrint("file_num %s",file_num);
	snprintf(pop3_file_ext,32,"tmp%s",file_num);
   	handle = FileSearch(drv, "POP3",pop3_file_ext, E_FILE_TYPE);
    mpDebugPrint("handle %x",handle);
	if(handle == NULL)
	{
		sDrv = DriveGet(bMcardId);
		handle = FileOpen(sDrv);
		if(DeleteFile(handle) == FS_SUCCEED)
		{
			mpDebugPrint("DeleteFile SUCCEED");
			ret = CreateFile(sDrv, "POP3",pop3_file_ext);
			if (ret)
				UartOutText("create file fail\r\n");
			handle=FileOpen(sDrv);
			if(!handle)
				UartOutText("open file fail\r\n");

		}
		else
			mpDebugPrint("DeleteFile FAIL ");



    }
	else
	{
		sDrv = DriveGet(bMcardId);
		ret = CreateFile(sDrv, "POP3",pop3_file_ext);
		if (ret)
			UartOutText("create file fail\r\n");
		handle = FileOpen(sDrv);
		if (!handle)
			UartOutText("open file fail\r\n");

	}
#else
	memset(recvbuffer, 0, sizeof(recvbuffer));
#endif    

	while(1){
		status = select(socketid+1, &fdread, 0, 0, &tv);
		if(status > 0){
			do{
#if WANT_WRITE//WRITE_TO_SD	
				mmcp_memset(recvbuffer, 0, POP3_READ_BUFFER_SIZE);
				recvlength = SSL_read(conn->handle, recvbuffer , POP3_READ_BUFFER_SIZE);

#else				
				memset(recvbuffer, 0, sizeof(recvbuffer));
				recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
#endif				

				if(!strncmp(recvbuffer, "+OK", 3)){
					char *target;

					target = recvbuffer;
					while(*target != 0x0a){
						target++;
						recvlength--;

						if(recvlength <= 0){
							MP_DEBUG("Fail to get 0x0a");					
							return 0;
						}

					}
					target++;
					recvlength--;
				#if WANT_WRITE//WRITE_TO_SD
					ret = FileWrite(handle,target,recvlength);
					if(!ret)
						UartOutText("write file fail\r\n");
                #else 
					memcpy(temp, target, recvlength);
					temp += recvlength;
				#endif
	
					totallength += recvlength;

					message_is_ok = 1;
				}
				else{
					
					#if WANT_WRITE//WRITE_TO_SD
					    ret = FileWrite(handle,recvbuffer,recvlength);
						if(!ret)
							UartOutText("write file fail\r\n");
					#else
					if((totallength + recvlength) > pnode->buff_len){
						MP_DEBUG("ext_mem_reallocm %d to %d", pnode->buff_len, pnode->buff_len + 4096);
						buffer_temp = ext_mem_reallocm(pnode->buffer, pnode->buff_len + 4096);
						if(!buffer_temp){
							MP_DEBUG("Memory is no enough");
							return 0;
						}
						pnode->buffer = buffer_temp;
						temp = pnode->buffer + totallength;
						pnode->buff_len += 4096;
					}
					memcpy(temp, recvbuffer, recvlength);
					//MP_DEBUG("%s", recvbuffer);
					temp += recvlength;
					#endif
					totallength += recvlength;
				}
			}
			while(SSL_pending(conn->handle));
		}
		else if(status == 0){
			break;
		}
	}

	MP_DEBUG("length = %d", totallength);

	totallength -= 5;
	pnode->buff_len = totallength;	

#if WANT_WRITE//WRITE_TO_SD
	if(handle)
		FileClose(handle);
#endif	
	
	if(!message_is_ok){
		
		MP_DEBUG("MESSAGE is not OK");
		return 0;
	}
#endif	
	return 1;	
}

/**
 * @ingroup POP3
 * @brief Retrieve mails from server
 *
 *
 * @param socketid socket ID
 * @param pnode mail node
 */
int pop3_Retr(int socketid, struct MailNode *pnode){
#if Make_CURL
	int cmdbuffer[50];
	char recvbuffer[512];
	int recvlength, totallength = 0;
	char* buffer_temp = NULL;
	char* temp;
	fd_set fdread;
	int status;
	struct timeval tv;
	int message_is_ok = 0;
	char filename[20];

	tv.tv_sec = 4;
	tv.tv_usec = 0;


	FD_ZERO(&fdread);
	FD_SET(socketid, &fdread);

	temp = pnode->buffer;
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "RETR %d\r\n", pnode->num);

	MP_DEBUG("RETR %d", pnode->num);

	send(socketid, cmdbuffer, strlen(cmdbuffer), 0);

	memset(recvbuffer, 0, sizeof(recvbuffer));

	while(1){
		status = select(socketid+1, &fdread, 0, 0, &tv);
		if(status > 0){
			recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);
			if(!strncmp(recvbuffer, "+OK", 3)){
				char *target;

				target = recvbuffer;
				while(*target != 0x0a){
					target++;
					recvlength--;

					if(recvlength <= 0){
						MP_DEBUG("Fail to get 0x0a");					
						return 0;
					}

				}
				target++;
				recvlength--;
				
				memcpy(temp, target, recvlength);
				temp += recvlength;
				totallength += recvlength;

				message_is_ok = 1;
			}
			else{
				//if((totallength + recvlength) > pnode->buff_len){
				//	recvlength -= totallength + recvlength - pnode->buff_len;
				//}
				if((totallength + recvlength) > pnode->buff_len){
					MP_DEBUG("ext_mem_reallocm %d to %d", pnode->buff_len, pnode->buff_len + 4096);
					buffer_temp = ext_mem_reallocm(pnode->buffer, pnode->buff_len + 4096);
					if(!buffer_temp){
						MP_DEBUG("Memory is no enough");
						return 0;
					}
					pnode->buffer = buffer_temp;
					temp = pnode->buffer + totallength;
					pnode->buff_len += 4096;
				}

				memcpy(temp, recvbuffer, recvlength);
				temp += recvlength;
				totallength += recvlength;
			}
		}
		else{
			break;
		}
		//if(totallength == pnode->buff_len)
		//	break;
	}

	MP_DEBUG("length = %d", totallength);

	totallength -= 5;
	pnode->buff_len = totallength;

	if(!message_is_ok){
		return 0;
	}
#endif	
	return 1;	
}

/**
 * @ingroup POP3
 * @brief Exit from server
 *
 *
 * @param conn connection session
 * @param socketid socket ID
 */
int pop3_Quit(struct ppp_connect *conn, int socketid){
#if Make_CURL
	int cmdbuffer[50];
	int recvbuffer[512];
	int recvlength;
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "QUIT\r\n");

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	memset(recvbuffer, 0, sizeof(recvbuffer));

	if(conn->flags == FLAG_SSL)
		recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
	else
		recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);


	MP_DEBUG("QUIT length = %d", recvlength);
	MP_DEBUG("QUIT recvbuffer = %s", recvbuffer);

	if(strncmp(recvbuffer, "+OK", 3)){
		return 0;
	}
#endif	
	return 1;
}

int pop3_Top(struct ppp_connect *conn, int socketid){
#if Make_CURL
	int cmdbuffer[50];
	int recvbuffer[2048];
	int recvlength;
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "TOP 1 0 \r\n");

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	memset(recvbuffer, 0, sizeof(recvbuffer));

	if(conn->flags == FLAG_SSL)
		recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
	else
		recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);


	MP_DEBUG("TOP length = %d", recvlength);
	//MP_DEBUG("TOP recvbuffer = %s", recvbuffer);
	//NetAsciiDump(recvbuffer, recvlength);

	if(strncmp(recvbuffer, "+OK", 3)){
		return 0;
	}
#endif	
	return 1;
}

int pop3_Rset(struct ppp_connect *conn, int socketid){
#if Make_CURL
	int cmdbuffer[50];
	int recvbuffer[512];
	int recvlength;
	
	memset(cmdbuffer, 0, sizeof(cmdbuffer));

	sprintf(cmdbuffer, "RSET \r\n");

	if(conn->flags == FLAG_SSL)
		SSL_write(conn->handle, cmdbuffer, strlen(cmdbuffer));
	else
		send(socketid, cmdbuffer, strlen(cmdbuffer), 0);


	memset(recvbuffer, 0, sizeof(recvbuffer));

	if(conn->flags == FLAG_SSL)
		recvlength = SSL_read(conn->handle, recvbuffer , sizeof(recvbuffer));
	else
		recvlength = recv( socketid, recvbuffer , sizeof(recvbuffer),0);


	MP_DEBUG("Reset length = %d", recvlength);
	//MP_DEBUG("TOP recvbuffer = %s", recvbuffer);
	//NetAsciiDump(recvbuffer, recvlength);

	if(strncmp(recvbuffer, "+OK", 3)){
		return 0;
	}
#endif	
	return 1;
}

/**
 * @ingroup POP3
 * @brief Parse body part of mail
 *
 *
 * @param start_point mail's start
 * @param end_point mail's end
 * @param pnode mail node
 */
void pop3_parsebody(char* start_point, char* end_point, struct MailNode *pnode){
#if Make_CURL

	char *temp, *target, *decode;
	char *boundary;
	char *filename;
	int isImage = 0, isText = 0,isZIP = 0;
	int image_type = 0;
	int base64_encode = 0;
	char* header_end;
    int ze=0;
	struct zip *za;
	struct zip_file *zf;
	int n, nsize;
    unsigned int ncrc;
	//char buf[8192];
    const char *fn;
	struct zip_error *error;
	int zip_file =0,i,unzip_size=0;
	char *unzip_buf = NULL;
	
	
	//NetAsciiDump(start_point,(end_point-start_point));
	
	temp = target = start_point;

	//aa
	while(target < end_point){
		if(*target == 0x0d && *(target + 1) == 0x0a 
			&& *(target + 2) == 0x0d && *(target + 3) == 0x0a)
			break;
		target++;
	}

	header_end = target;

	target = temp;

	//parse header
	target = stristr(temp, "Content-");
	while(target != NULL){
		if(target > header_end){
			MP_ASSERT(0);
			return;
		}
		target += strlen("Content-");
		if(strnicmp(target,"Type", strlen("Type")) == 0){
			target += strlen("Type");
			while(*target == 0x20 || *target == 0x3a){
				target++;
				if(target > header_end){
					MP_DEBUG("Fail to parse Content-Type");
					return;
				}
			}
			//mpDebugPrint("target %s",target);

			if((strnicmp(target, "image/jpeg", strlen("image/jpeg")) == 0) ||
				(strnicmp(target, "application/octet-stream", strlen("application/octet-stream")) == 0)||
				(strnicmp(target, "image/png", strlen("image/png")) == 0)||
				(strnicmp(target, "image/bmp", strlen("image/bmp")) == 0)||
				(strnicmp(target, "image/gif", strlen("image/gif")) == 0)||
				(strnicmp(target, "image/tif", strlen("image/tif")) == 0)||
				(strnicmp(target, "application/zip", strlen("application/zip")) == 0)){
				
				if(strnicmp(target, "application/zip", strlen("application/zip"))==0)
					isZIP = 1;
				else
				isImage = 1;
				
				
				if(strnicmp(target, "image/jpeg", strlen("image/jpeg")) == 0)	
				{
					target += strlen("image/jpeg");
					image_type = 0;
				}
				else if(strnicmp(target, "image/png", strlen("image/png")) == 0)	
				{
					target += strlen("image/png");
					image_type = 1;

				}
				else if(strnicmp(target, "image/bmp", strlen("image/bmp")) == 0)	
				{
					target += strlen("image/bmp");
					image_type = 2;

				}
				else if(strnicmp(target, "image/gif", strlen("image/gif")) == 0)	
				{
					target += strlen("image/gif");
					image_type = 3;

				}
				else if(strnicmp(target, "image/tif", strlen("image/tif")) == 0)	
				{
					target += strlen("image/tif");
					image_type = 4;

				}
				else if(strnicmp(target, "application/octet-stream", strlen("application/octet-stream")) == 0)	
					target += strlen("application/octet-stream");
				
				if(*target == 0x3b){
Type_parameter:
					target++;
					while(*target == 0x0d || *target == 0x0a || *target == 0x09 || *target == 0x20){
						target++;
						if(target > header_end){
							MP_DEBUG("Fail to parse Content-Type parameter");
							return;
						}
					}
					if(strnicmp(target, "name", strlen("name"))== 0){
						target += strlen("name");
						while(*target == 0x20 || *target == 0x3d){
							target++;
							if(target > header_end){
								MP_DEBUG("Fail to parse Content-Type name parameter");
								return;
							}
						}

						if(*target == 0x22){
							target++;
							filename = target;
							while(*target != 0x22){
								target++;
								if(target > header_end){
									MP_DEBUG("Fail to parse Content-Type name parameter");
									return;
								}
							}
							*target = '\0';
							target++;
							if(*target == 0x3b)
								goto Type_parameter;
						}
						else{
							filename = target;
							while(*target != 0x0d && *target != 0x3b){
								target++;
								if(target > header_end){
									MP_DEBUG("Fail to parse Content-Type name parameter");
									return;
								}
							}
							if(*target == 0x3b){
								*target = '\0';
								goto Type_parameter;
							}
							else{
								*target = '\0';
								target++;
							}
						}
					}
					else{
						while(*target != 0x0d){
							target++;
							if(*target == 0x3b)
								goto Type_parameter;
							if(target > header_end){
								MP_DEBUG("Fail to parse Content-Type parameter");
								return;
							}
						}
					}
					//check filename
					MP_DEBUG("filename = %s" , filename);
					if(isImage)
					{
					switch(image_type)
					{
					    case 0://JPG
					if(stristr(filename, "jpg") == NULL){
						MP_DEBUG("it's not a image file");
						isImage = 0;
					}
								break;
						case 1://png
								if(stristr(filename, "png") == NULL){
									MP_DEBUG("it's not a image file");
									isImage = 0;
								}
								break;
						case 2://bmp
								if(stristr(filename, "bmp") == NULL){
									MP_DEBUG("it's not a image file");
									isImage = 0;
								}
								break;
		
						case 3://gif
								if(stristr(filename, "gif") == NULL){
									MP_DEBUG("it's not a image file");
									isImage = 0;
								}
								break;
						case 4://tif
								if(stristr(filename, "tif") == NULL){
									MP_DEBUG("it's not a image file");
									isImage = 0;
								}
								break;

					}
				}
			}
			}
			else if(strnicmp(target, "text/plain", strlen("text/plain")) == 0){
				isText = 1;
				
				target += strlen("text/plain");
			}
			else if(strnicmp(target, "multipart", strlen("multipart")) == 0){
				target = stristr(temp, "boundary");

				if(target > header_end){
					MP_DEBUG("Fail to parse boundary");
					return;
				}
				
				target += strlen("boundary");
				
				while(*target == 0x20 || *target == 0x3d){
					target++;
					if(target > header_end){
						MP_DEBUG("Fail to parse Content-Type name parameter");
						return;
					}
				}
				
				if(*target ==  0x22){
					target++;
					boundary = target;
					while(*target != 0x22){
						target++;
				
						if(target > header_end){
							MP_DEBUG("Fail to Parse boundary"); 			
							return;
						}
					}
					*target= '\0';
				}
				else{
					boundary = target;
					while(*target != 0x0d){
						target++;
				
						if(target > header_end){
							MP_DEBUG("Fail to Parse boundary"); 			
							return;
						}
					}
					*target= '\0';
				}
				
				target++;
				
				temp = target;
				
				target = strstr(temp, boundary);
				
				if(target == NULL){
					MP_DEBUG("no body");
					return;
				}
				
				temp = target + strlen(boundary);
				
				while(*temp == 0x0d || *temp == 0x0a){
					temp++;
					if(temp > end_point){
						MP_DEBUG("Fail to Parse boundary"); 			
						return;
					}
				}
				
				//fetch body
				while((target = strstr(temp, boundary)) != NULL){
					target -= 4;
					*target= '\0';
					pop3_parsebody(temp, target, pnode);
					temp = target + strlen(boundary) + 4;
					if(temp[0] == '-' && temp[1] == '-'){
						target = temp;
						break;
					}
				}

			}		
			else{
				return;
			}
		}
		else if(strnicmp(target, "Transfer-Encoding", strlen("Transfer-Encoding")) == 0){
			target += strlen("Transfer-Encoding");
			while(*target == 0x20 || *target == 0x3a){
				target++;
				if(target > header_end){
					MP_DEBUG("Fail to parse Content-Transfer-Encoding");
					return;
				}
			}
			if(strnicmp(target, "base64", strlen("base64")) == 0){
				base64_encode = ENCBASE64;
			}
		}
		else if(strnicmp(target, "Disposition", strlen("Disposition")) == 0){
			int option = 0;
			target += strlen("Disposition");
			while(*target != 0x0d){
				if(*target == 0x3b){
					option = 1;
					break;
				}
				
				target++;
				
				if(target > header_end){
					MP_DEBUG("Fail to parse Content-Disposition");
					return;
				}
			}

			if(option){
Disposition_parameter:	
				target++;
				while(*target == 0x0d || *target == 0x0a || *target == 0x09 || *target == 0x20){
					target++;
				}
				if(strnicmp(target, "filename", strlen("filename"))== 0){
					target += strlen("filename");
					while(*target == 0x20 || *target == 0x3d){
						target++;
						if(target > header_end){
							MP_DEBUG("Fail to parse Content-Disposition parameter");
							return;
						}
					}
					
					if(*target == 0x22){
						target++;
						filename = target;
						while(*target != 0x22){
							target++;
							if(target > header_end){
								MP_DEBUG("Fail to parse Content-Disposition filename parameter");
								return;
							}
						}
						*target = '\0';
						target++;
						if(*target == 0x3b)
							goto Disposition_parameter;
					}
					else{
						filename = target;
						while(*target != 0x0d && *target != 0x3b){
							target++;
							if(target > header_end){
								MP_DEBUG("Fail to parse Content-Disposition filename parameter");
								return;
							}
						}
						if(*target == 0x3b){
							*target = '\0';
							goto Disposition_parameter;
						}
						else{
							*target = '\0';
							target++;
						}

					}
				}
				else{
					while(*target != 0x0d){
						target++;
						if(*target == 0x3b)
							goto Disposition_parameter;
						if(target > header_end){
							MP_DEBUG("Fail to parse Content-Disposition parameter");
							return;
						}
					}
				}
			}

			*target = '\0';
			target++;
		}
		temp = target;
		target = stristr(temp, "Content-");
	}

	if((isImage == 1)||(isZIP == 1)){
		struct AttachNode* anode = NULL;
		char* temp_buffer = NULL;
		int temp_buffer_len;
		int index = 0;

		anode = ext_mem_malloc(sizeof(struct AttachNode));
		if(!anode){
			MP_DEBUG("Memory is not enough");
			return;
		}

		memset(anode, 0, sizeof(struct AttachNode));		

		anode->coding = base64_encode;

		memcpy(anode->Name, filename, strlen(filename));

		//fetch image
		while(*header_end == 0x0d || *header_end == 0x0a){
			header_end++;
		}

		temp_buffer_len = end_point - header_end;
		temp_buffer = ext_mem_malloc(temp_buffer_len);
		if(!temp_buffer){
			MP_DEBUG("Memory is not enough");
			ext_mem_free(anode);
			return;
		}

		while(header_end < end_point){
			if(*header_end != 0x0d && *header_end != 0x0a){
				temp_buffer[index] = *header_end;
				index++;
			}
			header_end++;
		}
		anode->buff_len = index;
		anode->buffer = base64_decode(temp_buffer,index, &anode->buff_len);

		if(anode->buffer){
#if SAVE_FILE
		Save_Jpeg_File(anode->Name, anode->buff_len, anode->buffer);
		//Save_Jpeg_File2Disk(NAND,anode->Name, anode->buff_len, anode->buffer);
#endif

#if SHOW_FILE
        if(isImage)
        {
        ImagePOP3ViewPhoto(anode->buffer,anode->buff_len);
        //Not support PROGRESSIVE JPG
		//ImageAdhocDraw_Decode(anode->buffer,anode->buff_len);
        TaskSleep(2000);
        }
#endif		
#if UNZIP_FILE
        if(isZIP)
        {
        	if((za=pop3_zip_open(anode->Name, anode->buff_len, anode->buffer,&ze)) == NULL)
        	{
				mpDebugPrint("opening zip archive fail!!");
				fclose(za->zp);
        	}
			else
				{
				  zip_file = zip_get_num_files(za);
				  mpDebugPrint("zip_get_num_files %d",zip_file);
				  for(i=0;i<zip_file;i++)
				  {
					  if ((zf=pop3_zip_fopen_index(za, i, ZIP_FL_NOCASE,anode->buffer)) == NULL) {
					  	   mpDebugPrint("pop3_zip_fopen_index FAIL!!");
					  	}
					  else
					  	{
					  	      nsize = 0;
							  fn=_zip_get_name(za,i,0,error);
							  unzip_size = pop3_zip_get_unzip_size(za,i);
							  mpDebugPrint("%s",fn);
							  mpDebugPrint("unzip_size %d",unzip_size);
							  
							  unzip_buf = ext_mem_malloc(unzip_size);
							  if(unzip_buf!=NULL)
							  {
							      memset(unzip_buf,0x00,unzip_size);
						  	      while ((n=pop3_zip_fread(zf, unzip_buf+nsize, 8192,anode->buffer)) > 0) {
									nsize += n;
									ncrc = crc32(ncrc, (const Bytef *)(unzip_buf+nsize), n);

									//NetPacketDump(buf,sizeof(buf));
		   							}
								  //mpDebugPrint("zip file read %d",nsize);
								  Save_Jpeg_File(fn,unzip_size,unzip_buf);
								  ext_mem_free(unzip_buf);
							  }


					  	}
				  }
				  fclose(za->zp);
				  _zip_free(za);

				}
        }

		#endif
			list_add_tail((struct list_head *) anode, &pnode->AttachQ);

		}
		ext_mem_free(temp_buffer);

	}
	else if(isText == 1){
		char* temp_buffer = NULL;
		int temp_buffer_len;
		int index = 0;

		//fetch text
		header_end += 4;

		temp_buffer_len = end_point - header_end;
		temp_buffer = ext_mem_malloc(temp_buffer_len);
		if(!temp_buffer){
			MP_DEBUG("Memory is not enough");
			return;
		}

		if(base64_encode){
			while(header_end < end_point){
				if(*header_end != 0x0d && *header_end != 0x0a){
					temp_buffer[index] = *header_end;
					index++;
				}
				header_end++;
			}
			
			//pnode->text_len = index;

			pnode->text = base64_decode(temp_buffer,index, &pnode->text_len);
		
			ext_mem_free(temp_buffer);
		}
		else{
			pnode->text_len = temp_buffer_len;
			
			mmcp_memcpy(temp_buffer, header_end, pnode->text_len);
			
			pnode->text = temp_buffer;
		}
		mpDebugPrint("----------------Mail Text ------------%d",temp_buffer_len);
		//mpDebugPrint(pnode->text);
		NetAsciiDump(pnode->text,temp_buffer_len);
		//TODO UI need to integrate
		//MailText_Show(pnode->text);  
		mpDebugPrint("----------------Mail Text end------------");

	}
#endif
}

int pop3_Parse(struct list_head *MailQ){
	int i;
	
	if(!list_empty(MailQ)){
		struct MailNode *pnode;

		pnode = (struct MailNode*)MailQ->next;

		while((struct list_head*)pnode != MailQ){		
			char *temp, *target = NULL, *decode;
			char *boundary;
			int length;

			MP_DEBUG("get %d", pnode->num);

			temp = pnode->buffer;

			//first find subject
			target = stristr(temp, "Subject:");
			
			target += strlen("Subject:");

			while(*target == 0x20){
				target++;

				if(target > (pnode->buffer + pnode->buff_len)){
					MP_DEBUG("Fail to Parse Content-Type");				
					return 0;
				}
			}	

			i = 0;
			while(*target != 0x0d){
				pnode->subject[i] = *target;
				
				target++;
				i++;
				
				if(i >= MAX_SUBJECT_LENGTH){
					MP_DEBUG("subject is too long");				
					return 0;
				}

				if(target > (pnode->buffer + pnode->buff_len)){
					MP_DEBUG("Fail to Parse Subject");				
					return 0;
				}
			}

			pnode->subject[i] = 0; // jeffery 20090902
			mpDebugPrint ("Mail Subject = %s", pnode->subject);

			//check Content-Type
			target = stristr(temp, "Content-Type:");

			target += strlen("Content-Type:");

			while(*target == 0x20){
				target++;

				if(target > (pnode->buffer + pnode->buff_len)){
					MP_DEBUG("Fail to Parse Content-Type");				
					return 0;
				}
			}			

			if(strnicmp(target, "multipart", strlen("multipart")) != 0)
				goto NEXTPNODE;

			//find boundary
			target = stristr(temp, "boundary=");

			target += strlen("boundary=");

			if(*target ==  0x22){
				target++;
				boundary = target;
				while(*target != 0x22){
					target++;

					if(target > (pnode->buffer + pnode->buff_len)){
						MP_DEBUG("Fail to Parse boundary");				
						return 0;
					}
				}
				*target= '\0';
			}
			else{
				boundary = target;
				while(*target != 0x0d){
					target++;

					if(target > (pnode->buffer + pnode->buff_len)){
						MP_DEBUG("Fail to Parse boundary");				
						return 0;
					}
				}
				*target= '\0';
			}

			target++;

			temp = target;

			target = strstr(temp, boundary);

			if(target == NULL){
				MP_DEBUG("no body");
				return 0;
			}

			temp = target + strlen(boundary);

			while(*temp == 0x0d || *temp == 0x0a){
				temp++;
				if(temp > (pnode->buffer + pnode->buff_len)){
					MP_DEBUG("Fail to Parse boundary"); 			
					return 0;
				}
			}

			//fetch body
			while((target = strstr(temp, boundary)) != NULL){
				target -= 4;
				*target= '\0';
				pop3_parsebody(temp, target, pnode);
				temp = target + strlen(boundary) + 4;
				if(temp[0] == '-' && temp[1] == '-')
					break;
			}
			

#if 0			

			target = strstr(temp, "image/jpeg");
			if(target){
				struct AttachNode* anode;

				anode = mm_malloc(sizeof(struct AttachNode));
				memset(anode, 0, sizeof(struct AttachNode));

				temp = target;
				target = strstr(temp, "filename");
				
				while(*target != 0x3d){
					target++;

					if(target > (pnode->buffer + pnode->buff_len)){
						MP_DEBUG("Fail to Parse File");
						mm_free(anode);

						return 0;
					}
				}

				temp = target + 1;

				while(*target!= 0x0d){
					target++;

					if(target > (pnode->buffer + pnode->buff_len)){
						MP_DEBUG("Fail to Parse File");
						mm_free(anode);

						return 0;
					}
				}

				*target = '\0';

				memcpy(anode->Name, temp, target - temp);

				target++;

				while(*target == 0x0d || *target == 0x0a){
					target++;

					if(target > (pnode->buffer + pnode->buff_len)){
						MP_DEBUG("Fail to Parse File");
						mm_free(anode);

						return 0;
					}
				}

				temp = target;

				target = strstr(temp, "--");

				*target = '\0';

				anode->buff_len = target - temp;

				anode->buffer = mm_malloc(anode->buff_len);

				memcpy(anode->buffer, temp, anode->buff_len);

				//decode 
				decode= base64_decode(anode->buffer,anode->buff_len, &length);

				mm_free(anode->buffer);

				anode->buffer = decode;
				anode->buff_len = length;

				MP_DEBUG("photo length = %d", anode->buff_len);
				
				list_add_tail((struct list_head *) anode, &pnode->AttachQ);

				temp = target + 1;

				target = strstr(temp, "image/jpeg");
			}

#endif

NEXTPNODE:
			pnode = (struct MailNode*)pnode->list.next;
			TaskYield();
		}
	}	
}

#if WRITE_TO_SD
int pop3_File_Analyze(struct list_head *MailQ){
	
	BYTE bMcardId = SD_MMC;
	static DRIVE *sDrv;
	static int ret = 1;
	DRIVE *drv;
	char file_num[4];
	char pop3_file_ext[32];
	DWORD filesize = 0;
	DWORD readlen = 0;
	char *target = NULL;
	char *temp = NULL;
	DWORD fileread = 0;
	int i,step;
	DWORD text_len = 0;
	BYTE *tmptarget = NULL;
	DWORD filelen = 0;
	char *filename=NULL;
	DWORD filereadcnt = 0,image_start,image_end;
	DWORD fileread_total_cnt = 0;
	DWORD image_start_size = 0;
	DWORD image_end_size = 0;
	char *end_p = NULL;
	int image_len;

	BYTE *Pattern = "From: ";
	struct MailNode *pnode;
	struct AttachNode *anode=NULL;
	
	pnode = (struct MailNode*)MailQ->next;

	drv = DriveChange(bMcardId);
	if (DirReset(drv) != FS_SUCCEED)
		return;
	DecString(file_num,1,3,0);
	mpDebugPrint("file_num %s",file_num);
	snprintf(pop3_file_ext,32,"tmp%s",file_num);

   	pop3handle = FileSearch(drv, "POP3", pop3_file_ext, E_FILE_TYPE);
	
	if(pop3handle == NULL)
	{
		mpDebugPrint("Found tmp File");
		sDrv = DriveGet(bMcardId);
		pop3handle = FileOpen(sDrv);
		filelen = filesize = FileSizeGet(pop3handle);
		mpDebugPrint("File size %d",filesize);
		step = 0;
		while(filesize)
		{
		    memset(pop3_readbuf,0x00,POP3_READ_BUFFER_SIZE);

			readlen = FileRead(pop3handle, pop3_readbuf, POP3_READ_BUFFER_SIZE);
			filereadcnt++;
			fileread_total_cnt++;
			//mpDebugPrint("File read %d",readlen);
			//mpDebugPrint("pop3_readbuf %x",pop3_readbuf);
			fileread+=readlen;
			filesize-=readlen;
			target = temp = pop3_readbuf;
PASER_Pattern:			
			//first find subject
			//mpDebugPrint("Pattern %s",Pattern);
			target = stristr(temp, Pattern);
			//mpDebugPrint("target %x",target);
			if(target!=NULL)
			{ 
			  switch(step)
			  	{
			  	  case 0:
					    target+=strlen("From: ");
					    while(*target == 0x20){
						target++;
						}	

						i = 0;
						pnode->fromaddress = ext_mem_malloc(MAX_FROM_LENGTH);
						memset(pnode->fromaddress,0x00,MAX_FROM_LENGTH);
						while(*target != 0x0d){
							pnode->fromaddress[i] = *target;
							target++;
							i++;
							if(i >= MAX_FROM_LENGTH){
								MP_DEBUG("FROM is too long");				
								return 0;
							}
						}
                       pnode->fromaddress[i]= 0;
					   mpDebugPrint ("Mail from = %s", pnode->fromaddress);
				  	   //For next Pattern
					   temp = target;
					   step++;
					   Pattern = "To: ";
					   goto PASER_Pattern;
			  	  case 1:
					   target+=strlen("To: ");
					   while(*target == 0x20){
						target++;
						}	

						i = 0;
						pnode->toaddress = ext_mem_malloc(MAX_TO_LENGTH);
						memset(pnode->toaddress,0x00,MAX_TO_LENGTH);

						while(*target != 0x0d){
							pnode->toaddress[i] = *target;
							target++;
							i++;
							if(i >= MAX_TO_LENGTH){
								MP_DEBUG("TO is too long");				
								return 0;
							}
						}
                       pnode->toaddress[i]= 0;
					   mpDebugPrint ("Mail to = %s", pnode->toaddress);
				  	   //For next Pattern
				  	   temp = target;
					   step++;
					   Pattern = "Subject: ";
				  	   goto PASER_Pattern;
			  	  case 2:
					  target += strlen("Subject:");
					  while(*target == 0x20){
						target++;
						}	
						
						i = 0;
						while(*target != 0x0d){
							pnode->subject[i] = *target;
							target++;
							i++;
							if(i >= MAX_SUBJECT_LENGTH){
								//mpDebugPrint("target %x i %x",target,i);
								MP_DEBUG("subject is too long");				
								return 0;
							}
						}
						pnode->subject[i] = 0; 
						mpDebugPrint ("Mail Subject = %s", pnode->subject);
						//For next Pattern
						step++;
						temp = target;
						Pattern = "Date: ";
						goto PASER_Pattern;
					case 3:
						target+=strlen("Date: ");
						while(*target == 0x20){
						target++;
						}	
						i = 0;
						pnode->maildata = ext_mem_malloc(MAX_DATA_LENGTH);
						memset(pnode->maildata,0x00,MAX_DATA_LENGTH);

						while(*target != 0x0d){
							pnode->maildata[i] = *target;
							target++;
							i++;
							if(i >= MAX_DATA_LENGTH){
								//mpDebugPrint("target %x i %x",target,i);
								MP_DEBUG("data is too long");				
								return 0;
							}
						}
						pnode->maildata[i] = 0; 
						mpDebugPrint ("Mail data = %s", pnode->maildata);
						//For next Pattern
						step++;
						temp = target;
						Pattern = "Content-Type: ";
					   goto PASER_Pattern;
					case 4:
						//NetAsciiDump(target,64);
						target+=strlen("Content-Type: ");
						if(strnicmp(target, "text/plain", strlen("text/plain")) == 0)
						{
						  target += strlen("text/plain;");
						  temp = target;
						  target = stristr(temp,"Content-Transfer-Encoding:");
						  target += strlen("Content-Transfer-Encoding:");
						  while((*target != 0x0d)&&(*(target+1)!= 0x0a)){
							target++;
							}	
						  
						  target+=4;
						  tmptarget = target;
						  while(*tmptarget != 0x0d)
						  {
						    text_len++;
							tmptarget++;
						  }
						  mpDebugPrint("text_len %d",text_len);
						  pnode->text = ext_mem_malloc(text_len);
						  i=0;
						  while(*target != 0x0d)
						  {
						    pnode->text[i]=*target;
							target++;
							i++;
						  }
						  pnode->text[i] = 0;
                          mpDebugPrint("Mail Text %s",pnode->text);
						  //For next Pattern
						  step++;
						  Pattern = "------=_NextPart_";
						}
						temp = target;
					   goto PASER_Pattern;

					case 5:

						//mpDebugPrint("fileread %d",fileread);
						end_p = target;
						
                        if(fileread==filelen)
                        {
							while(*end_p!=0x0d)
								end_p--;
							image_end = (end_p-2);

							//mpDebugPrint("file read count %d",filereadcnt);
							image_end_size = (end_p-2)-pop3_readbuf;
							image_len = ((filereadcnt-1)*POP3_READ_BUFFER_SIZE)-image_start_size+image_end_size;
							mpDebugPrint("image size %d",image_len);
							//mpDebugPrint("image_end %x %d",image_end,image_end_size);
							filereadcnt = 0;
							mpDebugPrint("anode %x",anode);
							anode->buffer = NULL;
							anode->buff_len = image_len;
						    list_add_tail(anode, &pnode->AttachQ);
							target+=strlen("------=_NextPart");

							
                        }						
						else	
                        {
	                        target = stristr(temp, "Content-Type: ");
							if(target!=NULL)
							{
							   if(pnode->attachment)
							   {
								 while(*end_p!=0x0d)
								 	end_p--;
								 image_end = (end_p-2);
								 
 								 //mpDebugPrint("file read count %d",filereadcnt);
								 image_end_size = (end_p-2)-pop3_readbuf;
								 image_len = ((filereadcnt-1)*POP3_READ_BUFFER_SIZE)-image_start_size+image_end_size;
								 mpDebugPrint("image size %d",image_len);
								 //mpDebugPrint("image_end %x %d",image_end,image_end_size);
								 filereadcnt = 1;
								 anode->buffer = NULL;
								 anode->buff_len = image_len;
								 list_add_tail(anode, &pnode->AttachQ);

							   }
								
  								target+=strlen("Content-Type: ");
							   //if(strnicmp(target,"image/jpeg", strlen("image/jpeg")) == 0)
							   if((strnicmp(target, "image/jpeg", strlen("image/jpeg")) == 0) ||
									(strnicmp(target, "application/octet-stream", strlen("application/octet-stream")) == 0)||
									(strnicmp(target, "image/png", strlen("image/png")) == 0)||
									(strnicmp(target, "image/bmp", strlen("image/bmp")) == 0)||
									(strnicmp(target, "image/gif", strlen("image/gif")) == 0)||
									(strnicmp(target, "image/tif", strlen("image/tif")) == 0)||
									(strnicmp(target, "application/zip", strlen("application/zip")) == 0)){
									
									if(strnicmp(target, "application/zip", strlen("application/zip"))==0)
										target+=strlen("application/zip");
									else if(strnicmp(target, "image/jpeg", strlen("image/jpeg")) == 0)	
									{
										target += strlen("image/jpeg");
									}
									else if(strnicmp(target, "image/png", strlen("image/png")) == 0)	
									{
										target += strlen("image/png");

									}
									else if(strnicmp(target, "image/bmp", strlen("image/bmp")) == 0)	
									{
										target += strlen("image/bmp");

									}
									else if(strnicmp(target, "image/gif", strlen("image/gif")) == 0)	
									{
										target += strlen("image/gif");

									}
									else if(strnicmp(target, "image/tif", strlen("image/tif")) == 0)	
									{
										target += strlen("image/tif");

									}
									else if(strnicmp(target, "application/octet-stream", strlen("application/octet-stream")) == 0)	
										target += strlen("application/octet-stream");

								    //target+=strlen("image/jpeg");
								   	pnode->attachment++;
									temp = target;
									mpDebugPrint("pnode->attachment %d",pnode->attachment);
									target = stristr(temp, "name=");
									if(target!=NULL)
									{
									    anode = ext_mem_malloc(sizeof(struct AttachNode));

										target+=strlen("name=");
										while(*target==0x09||*target==0x22)
											target++;
										
										filename = target;

										while(*target != 0x22)
											target++;
										*target = '\0';
										target++;
										mpDebugPrint("name %s",filename);
										temp = target;
										target = stristr(temp, "Content-Transfer-Encoding: ");
										if(target)
										{
										   target+=strlen("Content-Transfer-Encoding: ");
										   if(strnicmp(target,"base64", strlen("base64")) == 0)
										   {
											  mpDebugPrint("Use BASE64 encode!!");
										   }
										}
										temp = target;
										target = stristr(temp, filename);
										target+=strlen(filename);
										while((*target==0x22)||(*target==0x0d)||(*target==0x0a))
											target++;
										
										image_start = target;
										image_start_size = target-pop3_readbuf;
										mpDebugPrint("fileread_total_cnt %d",fileread_total_cnt);
										mpDebugPrint("image_start %x %d",image_start,image_start_size);
										strcpy(anode->Name,filename);
										anode->start_position = ((fileread_total_cnt-1)*POP3_READ_BUFFER_SIZE)+image_start_size;
									}

								}

							}
							//For next Pattern
						    temp = target;
					        goto PASER_Pattern;

                        }
					break;	

			  	}
			}
			//else
			//	mpDebugPrint("fileread %d",fileread);
			
			TaskYield();
		}
		
		

        FileClose(pop3handle);
	}
	else
		mpDebugPrint("Can't found tmp File");
	
	//mpDebugPrint("fileread %d",fileread);
    mpDebugPrint("Mail attachment %d",pnode->attachment);
	return 0;

}
int pop3_File_Decode(struct list_head *MailQ){
	int decode_len = 0;
    BYTE *temp_buffer = NULL;
	BYTE bMcardId = SD_MMC;
	char file_num[4];
	char pop3_file_ext[32];
	DRIVE *drv;
	DWORD readlen = 0;
	static DRIVE *sDrv;
	
	drv = DriveChange(bMcardId);
	if (DirReset(drv) != FS_SUCCEED)
		return;
	DecString(file_num,1,3,0);
	mpDebugPrint("file_num %s",file_num);
	snprintf(pop3_file_ext,32,"tmp%s",file_num);

   	pop3handle = FileSearch(drv, "POP3", pop3_file_ext, E_FILE_TYPE);

	if(pop3handle == NULL)
	{
		mpDebugPrint("Found tmp File");
		sDrv = DriveGet(bMcardId);
		pop3handle = FileOpen(sDrv);

		while(!list_empty(MailQ))
		{
				struct MailNode *pnode;

				//first mail body
				pnode = (struct MailNode*)MailQ->next;
				list_del((struct list_head *) pnode);
				if(pnode->buffer)
					ext_mem_free(pnode->buffer);

				if(pnode->text)
					ext_mem_free(pnode->text);

				//for picture
				while(!list_empty(&pnode->AttachQ)){
					struct AttachNode *anode;

					//first pictiure
					anode = (struct AttachNode*)pnode->AttachQ.next;
					temp_buffer = ext_mem_malloc(anode->buff_len);
					memset(temp_buffer,0x00,anode->buff_len);
					Seek(pop3handle, anode->start_position);
			        readlen = FileRead(pop3handle, temp_buffer, anode->buff_len);
					anode->buffer = base64_decode(temp_buffer,anode->buff_len, &decode_len);
					
					Save_Jpeg_File(anode->Name, decode_len, anode->buffer);
					
					mpDebugPrint("picture name %s",anode->Name);
					mpDebugPrint("picture position %d",anode->start_position);
		            mpDebugPrint("picture size %d",anode->buff_len);
					mpDebugPrint("picture real size %d",decode_len);
					
					if(temp_buffer)
						ext_mem_free(temp_buffer);

					list_del((struct list_head *) anode);
					if(anode->buffer)
						ext_mem_free(anode->buffer);
					ext_mem_free(anode);

				}
				

			}
			//FileClose(pop3handle);
			DeleteFile(pop3handle);

	}
}

#endif
/**
 * @ingroup POP3
 * @brief Fetch mails from POP3 server
 *
 *
 * @param hostname server ip or name
 * @param port server listen port
 * @param username user name
 * @param password password
 * @param ssl 1: use SSL, 0: not use SSL
 * @return Mail queue
 *
 */
void* pop3_fetch_messages(char* hostname, unsigned short port, char* username, char* password, bool ssl,unsigned char mail_type){
	U32 addr;
	int socketid, ret;
	int recvlength;
	int msg = 0, size;
	struct hostent *host;
	struct list_head* MailQ = NULL;

	struct ppp_connect conn;
	
    mpDebugPrint("pop3_fetch_messages hostname %s port %d",hostname,port);
	mpDebugPrint("pop3_fetch_messages username %s password %s",username,password);

	MailQ = ext_mem_malloc(sizeof(struct list_head));
#if WRITE_TO_SD
	pop3_readbuf = ext_mem_malloc(POP3_READ_BUFFER_SIZE);
    if(!pop3_readbuf)
    {
      mpDebugPrint("Malloc pop3_readbuf FAIL!!");
    }
	pop3_paserbuf = ext_mem_malloc(POP3_PASER_BUFFER_SIZE);
    if(!pop3_paserbuf)
    {
      mpDebugPrint("Malloc pop3_paserbuf FAIL!!");
    }
		
#endif
	
    mpDebugPrint("sizeof(struct list_head) %d",sizeof(struct list_head));
	if(!MailQ){
		MP_DEBUG("Memory is no enough");
		return 0;
	}

	INIT_LIST_HEAD(MailQ);

	if (! isdigit(hostname[0])) {
		struct hostent *host;
		struct in_addr *curr;
		host = gethostbyname(hostname);
		if(host == NULL){
			MP_DEBUG("Can't find host");
			ext_mem_free(MailQ);   // 20090901
			return 0;
		}
        if (curr = (struct in_addr *)host->h_addr_list[0])
            addr = ntohl(curr->s_addr);

	} else {
		addr = inet_addr(hostname);
	}

	if(!ssl){
		conn.flags = 0;
		if(!(socketid = pop3_connect(&conn, addr, port))){
			MP_DEBUG("Connect Fail");
			ext_mem_free(MailQ);
			return 0;
		}
	}
	else{
		conn.flags = FLAG_SSL;
		//init SSL
		SSL_init();
#if	1	
		//For firewall set static ip
		switch(mail_type)
		{
		   case 0://No static ip
                break;
		   case 1://GMAIL
		   		addr = 0x4A7D436D;//74.125.67.109
		   		break;
		   case 2://HOTMAIL
		   		addr = 0x41363327;//65.54.51.39
			    break;
		   case 3://Yahoo com
		   		addr = 0x43C387BB;//67.195.135.187
			    break;
	
		}
#endif		
		
		if(!(socketid = pop3_connect_ssl(&conn, addr, port))){
			MP_DEBUG("Connect Fail");
			ext_mem_free(MailQ);
			return 0;
		}
	}

	if(!pop3_Auth(&conn, socketid, username, password)){
		MP_DEBUG("AUTH is not accept");
		goto QUIT;
	}

#if 0
	if(!pop3_Stat(&conn, socketid)){
		MP_DEBUG("STAT is not accept");
		goto QUIT;
	}
#endif

	if(!pop3_List(&conn, socketid, MailQ, 0)){
		MP_DEBUG("LIST is not accept");
		goto QUIT;
	}
#if 0
	if(!pop3_Top(&conn, socketid)){
		MP_DEBUG("TOP is not accept");

	}
#endif	


#if 1
	if(!list_empty(MailQ)){
		//TODO UI need to integrate
		//Msg_Show_ReceiveMail(); //jeffery 20090901
		struct MailNode *pnode = NULL;

		pnode = (struct MailNode*)MailQ->next;

		while((struct list_head*)pnode != MailQ){		

			pnode->buffer = ext_mem_malloc(pnode->buff_len);
			if(!pnode->buffer){
				MP_DEBUG("Memory is no enough");
				goto QUIT;
			}

			memset(pnode->buffer, 0, pnode->buff_len);
			if(conn.flags == FLAG_SSL){
				if(!pop3_Retr_ssl(&conn, socketid, pnode)){
					MP_DEBUG("RETR is not accept");
					goto QUIT;
				}
			}
			else{
				if(!pop3_Retr(socketid, pnode)){
					MP_DEBUG("RETR is not accept");
					goto QUIT;
				}
			}

			pnode = (struct MailNode*)pnode->list.next;
			
			TaskYield();
		}
	}
	else
	{	//TODO UI need to integrate
		//Msg_Show_NoMail(); //jeffery 20090901
		mpDebugPrint("List Empty");
	}
#endif


	//MP_DEBUG("mailbox %d %d", msg, size);

	

#if 0
	if(!pop3_Dele(&conn,socketid,msg)){
		MP_DEBUG("Dele is not accept");
		//return 0;
	}
#endif
#if 0
	if(!pop3_Rset(&conn, socketid)){
		MP_DEBUG("RSTE is not accept");

	}
#endif	

#if 1

	if(!pop3_Quit(&conn, socketid)){
		MP_DEBUG("QUIT is not accept");
		//return 0;
	}
#endif

	if(conn.flags == FLAG_SSL){
		MP_DEBUG("SSL_close");
		SSL_close(&conn);
	}

	closesocket(socketid);

	//parse attach file
#if WRITE_TO_SD
    pop3_File_Analyze(MailQ);
    pop3_File_Decode(MailQ);
	
    if(pop3_readbuf)
		ext_mem_free(pop3_readbuf);
	if(pop3_paserbuf)
		ext_mem_free(pop3_paserbuf);
#else
	pop3_Parse(MailQ);
#endif

	return MailQ;

QUIT:

#if 1

	if(!pop3_Quit(&conn, socketid)){
		MP_DEBUG("QUIT is not accept");
		//return 0;
	}
#endif

	if(conn.flags == FLAG_SSL){
		MP_DEBUG("SSL_close");
		SSL_close(&conn);
	}

	closesocket(socketid);

	ClearMailQ(MailQ);

	return NULL;

}

/**
 * @ingroup POP3
 * @brief Show mail
 *
 *
 * @param MailQ Mail queue
 *
 */
void mail_show(struct list_head* MailQ){
	//for mail body
	if(!list_empty(MailQ)){
		struct MailNode *pnode;

		//first mail body
		pnode = (struct MailNode*)MailQ->next;
		
		do {

			//for picture attachment
			if(!list_empty(&pnode->AttachQ)){
				struct AttachNode *anode;

				//first pictiure of this mail
				anode = (struct AttachNode*)pnode->AttachQ.next;

				do {
					/* ----------  Process the picture: begins  ---------- */
					/* anode->buffer has the picture */
					/* ----------  Process the picture: ends  ---------- */
			
					/* ----------  Fetch the next picture, if any  ---------- */
					anode = (struct AttachNode*)anode->list.next;

				} while((struct list_head*)anode != &pnode->AttachQ);
			}

			pnode = (struct MailNode*)pnode->list.next;
		} while((struct list_head*)pnode != MailQ);

	}
}

/**
 * @ingroup POP3
 * @brief Clear all mails in mail queue
 *
 * @param MailQ Mail queue
 *
 */

void ClearMailQ(struct list_head* MailQ){	
	//for mail body
	while(!list_empty(MailQ)){
		struct MailNode *pnode;

		//first mail body
		pnode = (struct MailNode*)MailQ->next;
		
		list_del((struct list_head *) pnode);
		if(pnode->buffer)
			ext_mem_free(pnode->buffer);

		if(pnode->text)
			ext_mem_free(pnode->text);

		//for picture
		while(!list_empty(&pnode->AttachQ)){
			struct AttachNode *anode;

			//first pictiure
			anode = (struct AttachNode*)pnode->AttachQ.next;

			list_del((struct list_head *) anode);
			if(anode->buffer)
				ext_mem_free(anode->buffer);
			ext_mem_free(anode);

		}

		ext_mem_free(pnode);
	}

	ext_mem_free(MailQ);
}


