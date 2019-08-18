#include "net_curl_setup.h"

#include <stdio.h>
#include "net_curl_curl.h"

#include <string.h>
#include "igo.h"
#include "api.h"
#include "net_socket.h"

#define YGPDEBUG 1
#define YGPPRINTF(msg...) {if(YGPDEBUG)DPrintf(msg);}

#define FileNameSize 128
#define URLSize 512

#define NUM_OF_MAX_PHOTOS 50

static U08 u08YGPShowTaskId = 0;
static U08 u08YGPDataTaskId = 0;

static U08 u08YGPTimerTaskId = 0;

static U08 u08YGPShowTaskSleep = 0;
static U08 u08YGPDataTaskSleep = 0;

static U08 u08YGPPhotoListChanged = 0;
static U08 u08YGPUpdatePhotoList =0;

static U08 NumOfPhotos = 0;
static U08 NumOfPhotos_backup = 0;

static U16 u16YGPPhotoList[NUM_OF_MAX_PHOTOS];
static U16 u16YGPPhotoList_Backup[NUM_OF_MAX_PHOTOS];

void Init_PhotoList(){
	U08 i;
	for(i = 0; i < NUM_OF_MAX_PHOTOS; i++){
		u16YGPPhotoList[i] = 0;
	}
}

void Init_PhotoList_Backup(){
	U08 i;
	for(i = 0; i < NUM_OF_MAX_PHOTOS; i++){
		u16YGPPhotoList_Backup[i] = 0;
	}
}

void Copy_PhotoLists(){
	U08 i;
	for(i = 0; i < NUM_OF_MAX_PHOTOS; i++){
		u16YGPPhotoList[i] = u16YGPPhotoList_Backup[i];
	}	
}

void Cmp_PhotoLists(){
	U08 i;
	for(i = 0; i < NUM_OF_MAX_PHOTOS; i++){
		if(u16YGPPhotoList[i] != u16YGPPhotoList_Backup[i]){
			u08YGPPhotoListChanged = 1;
			YGPPRINTF("Change show lists start");
			break;
		}
	}	
}


void Kill_Photos(){
	U08 i,j;
	U08 found = 0;
	U08 store_name[FileNameSize];
	U08 converted_store_name[FileNameSize];

	for(i = 0; i < NumOfPhotos; i++){
		for(j = 0; j < NumOfPhotos_backup; j++){
			if(u16YGPPhotoList[i] == u16YGPPhotoList_Backup[j]){
				found = 1;
			}
		}
		if(!found){
			memset(store_name, 0, FileNameSize);
			sprintf(store_name, "%d.jpg", u16YGPPhotoList[i]);
			UtilStringCopy0816(converted_store_name, store_name);
	
			if (mpx_DirNodeLocate(converted_store_name) == NO_ERR){
				mpx_DirFileDelete();
				YGPPRINTF("Delete file %s", store_name);
			}
			else{
				YGPPRINTF("Can't find file %s", store_name);
				BREAK_POINT();
			}
		}
		found = 0;
	}		
}

void YGP_timer(){
	mpx_TaskWakeup(u08YGPDataTaskId);
}

static void netTestDisplayImage(U32 stream, U32 size)
{
    ST_IP_IMAGE_INFO stImageInfo;
    ST_IP_IMAGE_SIZE stBoxSize;
    ST_IP_IMAGE_POINT stBoxPos;
    ST_FB_INFO *imageFbInfoPtr;
    ST_IP_RGB_COLOR stColor = {255, 0, 0};

    stBoxPos.u16H = 0;
    stBoxPos.u16V = 0;
    stBoxSize.u16H = 480;
    stBoxSize.u16V = 272;

    mpx_GdcImageWindowSet(stBoxPos, stBoxSize);

    imageFbInfoPtr = (ST_FB_INFO *) mpx_GdcFrameBufferInfoGet(LCD_PANEL_ACTIVE, mpx_GdcFrameBufferActiveGet(IDU_IMAGE_WINDOW));

    stImageInfo.stImageSize = imageFbInfoPtr->stVirtualSize;
    stImageInfo.pu08StartAddressY = (U08 *) imageFbInfoPtr->u32BitmapAddrY;
    stImageInfo.pu08StartAddressCbCr = (U08 *) imageFbInfoPtr->u32BitmapAddrCbCr;
    stImageInfo.u32PixelCounts = imageFbInfoPtr->u32Size;
    stImageInfo.eDataFormat = imageFbInfoPtr->u16Format;
    stImageInfo.eDataOrg = imageFbInfoPtr->u16Organization;

    mpx_GuiFBFill(*imageFbInfoPtr, stBoxPos, stBoxSize, stColor);
    mpx_ImageJpegLoad(&stImageInfo, stBoxPos, stBoxSize, STREAM_SOURCE_BY_ARRAY_POINT, 0, stream, size);
    mpx_GdcScreenRefresh();
}


void Get_XML(U08* FileName){
	CURL *curl;
	CURLcode res;
	size_t index = 0;
	size_t status;
	size_t CurrentFileSize = 0;
	U08 store_name[FileNameSize];
	U08 converted_store_name[FileNameSize];
	U08 URL[URLSize];
	U08* pch = NULL, *pch_head = NULL;
	U32 FileBufferSize = 64*1024; //64K
	U32 FileBuffer = 0;

	FileBuffer = mpx_Malloc(FileBufferSize);
	if(!FileBuffer){
		YGPPRINTF("FileBuffer allocate fail");
		return 0;
	}	  
		  
	memset(FileBuffer, 0, FileBufferSize);

	memset(store_name, 0, FileNameSize);
	sprintf(store_name, "%s", FileName);
	UtilStringCopy0816(converted_store_name, store_name);
	if(mpx_DirNodeLocate(converted_store_name)){
		YGPPRINTF("Can't find %s", FileName);
		return 0;
	}
	else{
		YGPPRINTF("%s found, Open it", FileName);
		status = mpx_FileOpen();
		if(status <= 0){
			YGPPRINTF("Can't Open file %s", FileName);
			return 0;
		}
	}
	
	CurrentFileSize = mpx_FileRead(status, FileBuffer, FileBufferSize);
	mpx_FileClose(status);

	curl = curl_easy_init();
	if(curl){
		pch = strstr(FileBuffer, "url");
		if(pch != NULL){
			while(*pch != 0x22){
				pch++;
			}
			pch_head = ++pch;
			while(*pch != 0x22){
				pch++;
			}

			memset(URL, 0, URLSize);
			memcpy(URL, pch_head, pch - pch_head);

			memset(store_name, 0, FileNameSize);
			sprintf(store_name, "%d.xml", index);
			UtilStringCopy0816(converted_store_name, store_name);
	
			if (mpx_DirNodeLocate(converted_store_name) == NO_ERR){
				mpx_DirFileDelete();
				YGPPRINTF("Delete file %s", store_name);
			}
			
			status = mpx_FileCreateOpen(converted_store_name, 0, FILE_CREATE_OPEN);

			curl_easy_setopt(curl, CURLOPT_URL, URL);
			
			curl_easy_setopt(curl, CURLOPT_FILE, status);
			
			res = curl_easy_perform(curl);

			mpx_FileClose(status);
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	if(FileBuffer)
		mpx_Free(FileBuffer);
	return 0;	
}


void Get_Pictures(U08* FileName){
	CURL *curl;
	CURLcode res;
	size_t index = 0;
	size_t status;
	size_t CurrentFileSize = 0;
	U08 store_name[FileNameSize];
	U08 converted_store_name[FileNameSize];
	U08 URL[URLSize];
	U08* pch = NULL, *pch_temp = NULL, *pch_head = NULL;
	U32 FileBufferSize = 64*1024; //64K
	U32 FileBuffer = 0;

	FileBuffer = mpx_Malloc(FileBufferSize);
	if(!FileBuffer){
		YGPPRINTF("FileBuffer allocate fail");
		return 0;
	}	  
		  
	memset(FileBuffer, 0, FileBufferSize);

	memset(store_name, 0, FileNameSize);
	sprintf(store_name, "%s", FileName);
	UtilStringCopy0816(converted_store_name, store_name);
	if(mpx_DirNodeLocate(converted_store_name)){
		YGPPRINTF("Can't find %s", FileName);
		return 0;
	}
	else{
		YGPPRINTF("%s found, Open it", FileName);
		status = mpx_FileOpen();
		if(status <= 0){
			YGPPRINTF("Can't Open file %s", FileName);
			return 0;
		}
	}
	
	CurrentFileSize = mpx_FileRead(status, FileBuffer, FileBufferSize);
	mpx_FileClose(status);

	curl = curl_easy_init();
	if(curl) {
		U08 First_init = 0;
		U08 u08Photos;
		U16 uid;
		
		pch = strstr(FileBuffer, "photos");
		while(*pch != 0x22){
			pch++;
		}
		pch_head = ++pch;
		while(*pch != 0x22){
			pch++;
		}

		*pch = '\0';
		u08Photos = atoi(pch_head);
		if(!NumOfPhotos){
			NumOfPhotos = u08Photos;
			First_init = 1;
			mpx_TaskWakeup(u08YGPShowTaskId);
		}
		else{
			NumOfPhotos_backup = u08Photos;
		}
		
		while(index != u08Photos){
			pch++;
			pch = strstr(pch, "photo_url");
			while(*pch != 0x22){
				pch++;
			}
			pch_head = ++pch;
			while(*pch != 0x22){
				pch++;
			}

			memset(URL, 0, URLSize);
			memcpy(URL, pch_head, pch - pch_head);

			pch++;
			pch = strstr(pch, "uid");
			while(*pch != 0x22){
				pch++;
			}
			pch_head = ++pch;
			while(*pch != 0x22){
				pch++;
			}
			*pch = '\0';
			uid = atoi(pch_head);

			memset(store_name, 0, FileNameSize);
			sprintf(store_name, "%d.jpg", uid);
			UtilStringCopy0816(converted_store_name, store_name);
	
			if (mpx_DirNodeLocate(converted_store_name)){
				status = mpx_FileCreateOpen(converted_store_name, 0, FILE_CREATE_OPEN);

				curl_easy_setopt(curl, CURLOPT_URL, URL);
				
				curl_easy_setopt(curl, CURLOPT_FILE, status);
				
				res = curl_easy_perform(curl);

				mpx_FileClose(status);				
			}

			if(First_init){
				u16YGPPhotoList[index] = uid;
			}
			else
				u16YGPPhotoList_Backup[index] = uid;

			YGPPRINTF("index=%d, uid=%d", index, uid);
			index++;

			if(u08YGPShowTaskSleep)
				mpx_TaskWakeup(u08YGPShowTaskId);
			
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	if(FileBuffer)
		mpx_Free(FileBuffer);
	return 0;	

}


void YGP_ShowTask(){
	U08 i;
	U32 ImageFileBufferSize = 1024*1024; //1M
	U32 ImageBuffer = 0;
    S32 ImageFileSize = 0;
    S32 status;
	U08 store_name[FileNameSize];
	U08 converted_store_name[FileNameSize];

    ImageBuffer = mpx_Malloc(ImageFileBufferSize);
    if(!ImageBuffer){
        YGPPRINTF("ImageBuffer allocate fail");
		BREAK_POINT();
    }  

	if(!NumOfPhotos){
		u08YGPShowTaskSleep = 1;
		mpx_TaskSleep();
	}

	if(!NumOfPhotos){
		YGPPRINTF("Can't get num of photos");
		BREAK_POINT();
	}
	
	while(1){
		for(i = 0; i < NumOfPhotos; i++){
			if(u16YGPPhotoList[i] == 0){
				u08YGPShowTaskSleep = 1;
				mpx_TaskSleep();	
			}
			
			if(u16YGPPhotoList[i] == 0){
				YGPPRINTF("Can't get photo %d", i);
				BREAK_POINT();	
			}
			
			memset(store_name, 0, FileNameSize);
			sprintf(store_name, "%d.jpg", u16YGPPhotoList[i]);
			UtilStringCopy0816(converted_store_name, store_name);
			if(mpx_DirNodeLocate(converted_store_name)){
				YGPPRINTF("Can't find %s", store_name);
				BREAK_POINT();
			}
			else{
				YGPPRINTF("%s found, Open it", store_name);
				status = mpx_FileOpen();
				if(status <= 0){
					YGPPRINTF("Can't Open file %s", store_name);
					BREAK_POINT();
				}
			}

            memset(ImageBuffer, 0, ImageFileBufferSize);
            
            ImageFileSize = mpx_FileRead(status, ImageBuffer, ImageFileBufferSize);
            
            mpx_FileClose(status);

			YGPPRINTF("show i = %d, uid = %d", i, u16YGPPhotoList[i]);
            netTestDisplayImage(ImageBuffer, ImageFileSize);
			mpx_TaskYield(10000);
		}
		if(u08YGPPhotoListChanged && !u08YGPUpdatePhotoList){
			Kill_Photos();
			NumOfPhotos = NumOfPhotos_backup;
			Copy_PhotoLists();
			u08YGPPhotoListChanged = 0;
			YGPPRINTF("Change show lists close");
		}
	}
}

void YGP_DataTask(){
	S32 status;
	
    status = mpx_TimerCreate(YGP_timer, OS_TIMER_ROUGH, 60, 1); // network timer period = 250 ms
    if(status < 0)
    {
        YGPPRINTF("YGP timer create fail");
        BREAK_POINT();
    }
    
    u08YGPTimerTaskId = (U08)status;
	
	Get_XML("get_device_photo.txt");
	Get_Pictures("0.xml");

	mpx_TimerStart(u08YGPTimerTaskId, -1);

	mpx_TaskSleep();
	
	while(1){
		Get_XML("get_device_photo.txt");
		u08YGPUpdatePhotoList = 1;
		Get_Pictures("0.xml");
		u08YGPUpdatePhotoList = 0;
		Cmp_PhotoLists();
		mpx_TaskSleep();
	}
}


void YGP_SlideShow(){
    S32 status;

	Init_PhotoList();
	Init_PhotoList_Backup();
    
    //status = mpx_TaskCreate(YGP_ShowTask, 25, TASK_STACK_SIZE*2);
    status = mpx_TaskCreate(YGP_ShowTask, NETWORK_TASK_PRI+1, TASK_STACK_SIZE*2);

    if(status < 0)
    {
        YGPPRINTF("YGP_ShowTask create fail");
        BREAK_POINT();
    }
    u08YGPShowTaskId = (U08)status;
    
    mpx_TaskStartup(u08YGPShowTaskId);

	//status = mpx_TaskCreate(YGP_DataTask, 20, TASK_STACK_SIZE*2);
	status = mpx_TaskCreate(YGP_DataTask, NETWORK_TASK_PRI, TASK_STACK_SIZE*2);

	if(status < 0)
	{
		YGPPRINTF("YGP_ShowTask create fail");
		BREAK_POINT();
	}
	u08YGPDataTaskId = (U08)status;

	mpx_TaskStartup(u08YGPDataTaskId);
}


