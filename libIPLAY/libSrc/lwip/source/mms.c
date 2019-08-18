/**
 * \defgroup MMS MMS related codes
 * @{
 *
 */


/**
 * \file
 * Multimedia Messaging Service (MMS) implementation.
 *
 */

/** @} */

#define LOCAL_DEBUG_ENABLE 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global612.h"
#include "mpTrace.h"


#include <linux/types.h>
#include "ppp.h"

#if MMS_ENABLE

void ParseJPG(BYTE* message_start, int length)
{
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


int StartGetMMS()
{
	DWORD dwNWEvent;  
	BYTE* temp, *start = NULL, *end = NULL, *message_start = NULL;;
	int wait_times = 3;
	int length = 0, message_length = 0;

	while(1){

		if(wait_times)
			wait_times--;
		else
			return FALSE;

		TaskSleep(1000);

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
			//Parse header part
			parseHeader(message_start, message_start + message_length, 0, NULL);
			return TRUE;
		}
	}
}


int SetMMSCMD()
{
	unsigned char buf[40];
	
	sprintf(buf, "at+mmsrecv\r\n");
	
	return __edge_simple_cmd(buf);	
}


void ParseMMS()
{
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

int setmmsurl()
{
	unsigned char buf[40];	
	
	memset(buf, '\0', 40);
	sprintf(buf, "at+mmsurl=\"%s\"\r\n", user_config.mms_url);
	
	return __edge_simple_cmd(buf);
}


int setmmsproxy()
{
	unsigned char buf[40];
	
	memset(buf, '\0', 40);
	sprintf(buf, "at+mmsproxy=\"%s\",%d\r\n", user_config.mms_proxy, user_config.mms_proxy_port);
	
	return __edge_simple_cmd(buf);
}

int setgprsconfig()
{
	unsigned char buf[40];

	memset(buf, '\0', 40);
	sprintf(buf, "at+gprsconfig=\"%s\"\r\n", user_config.mms_service_name);
	
	return __edge_simple_cmd(buf);	
}	
void fetch_MMS()
{
	int i;

	int application_id = 0;
	int message_type = 0;
	int content_type = 0;
	BYTE *ptr, *header;
	int header_length;
	int total_length = 0;
	BYTE* buffer;
	
	MP_DEBUG("====================================================");
	
	if(!list_empty(&SMS_queue)){
		
		struct sms_message *pnode;

		//first mail body
		pnode = (struct sms_message*)SMS_queue.next;

		while((struct list_head*)pnode != &SMS_queue){
			application_id = message_type = content_type = 0;

			//check
			for(i = 0; i < pnode->content_length; i++){
				ptr = &pnode->content[i];

				if((*ptr == 0xaf) && (*(ptr+1) == 0x84)){
					header = ptr;
					header_length = pnode->content_length - i;
					application_id = 1;
				}
				if((*ptr == 0x8c) && (*(ptr+1) == 0x82)){
					message_type = 1;
				}

				if(strncmp(ptr, "application/vnd.wap.mms-message", strlen("application/vnd.wap.mms-message"))== 0){
					content_type = 1;
				}
				if(application_id && message_type && content_type)
					break;
			}

			if(application_id && message_type && content_type){
				BYTE* transaction_id;
				MP_DEBUG("find MMS message header_length = %d", header_length);
				parseHeader(header, header + header_length, 0x98, &transaction_id);
				MP_DEBUG("transaction_id = %s", transaction_id);
				memcpy(pnode->mms_url, user_config.mms_url, strlen(user_config.mms_url));
				memcpy(pnode->mms_url+ strlen(user_config.mms_url), transaction_id, strlen(transaction_id));
				MP_DEBUG("pnode->mms_url = %s", pnode->mms_url);
				buffer = mm_malloc(1024 * 128);
				total_length = Get_MMS_File(pnode->mms_url, buffer, user_config.mms_proxy,user_config.mms_proxy_port);
				MP_DEBUG("pointer = %x, length = %d", buffer, total_length);

				pnode->mms_message = buffer;
				pnode->mms_message_length = total_length;
				
			}

			pnode = (struct sms_message*)pnode->list.next;
		}
	}
	MP_DEBUG("====================================================");
}

#endif /* MMS_ENABLE */

// vim: :noexpandtab:
