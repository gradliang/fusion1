

//#include <stdio.h>
#include <stdarg.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <string.h>
//#include <errno.h>
#include "global612.h"
#include "txt_mpx.h"
//#include "pdbconv.h"
//#include "pdbrep.h"
//#include "pdbio.h"
#include "..\..\xml\include\xmlword.h"
//typedef STREAM FILE ;
extern XML_WORD_t XML_WORD;
TXT_INFO_t *gp_TXT_INFO;
typedef STREAM FILE;
#define EOF -1
#define fprintf(fd, fmt,...) txt_fprintf(fmt, ...)

FILE * txt_fopen(unsigned char device_id,char *filename)
{
	STREAM *shandle;
	DRIVE *drv;
	SWORD swRet;
	char *ptr,*file; 
	static DRIVE *sDrv;
	int ret;

	ptr = strchr(filename,'.');
	file = filename;
	*(file+ (ptr -filename)) = '\0';

	drv = DriveChange(device_id);
	if (DirReset(drv) != FS_SUCCEED)
		return NULL;

	
	ret = FileSearch(drv, file, "txt",E_FILE_TYPE);
	if (ret != FS_SUCCEED) 
	{
		sDrv=DriveGet(device_id);
		ret=CreateFile(sDrv, file, "txt");
		if (ret)
		{
			UartOutText("create file fail\r\n");
			return NULL;
		}
		else
		{
			shandle = FileOpen(sDrv);			
			if(!shandle)
				return NULL;
		}
	}
	else
	{		
		sDrv=DriveGet(device_id);
		shandle = FileOpen(sDrv);
		DeleteFile(shandle);

		ret=CreateFile(sDrv, file, "txt");
		if (ret)
		{
			UartOutText("create file fail\r\n");
			return NULL;
		}
		shandle = FileOpen(sDrv);
		if(!shandle)
			return NULL;
	}
	return shandle;
}

DWORD txt_fwrite(FILE *shandle, BYTE *buffer, DWORD size) 
{
	//mpDebugPrint("z_fwrite = %p", shandle);
	return FileWrite(shandle, buffer, size);
}

static char str[128*1024];

void txt_fprintf(FILE *fp,const char *fmt, ... )
{
	int size = 128*1024;

	int len;    

	va_list args;
	
	va_start(args, fmt);		/* get variable arg list address */

	vsnprintf(str, size, fmt, args);	/* process fmt & args into buf */	

	va_end(args);

	len = strlen((BYTE *)str);

	mpDebugPrint("len = %d", len);
#if TXT_DEBUG	
	word_xml_Write2Mem(&str,len);
#endif	
    txt_data_link(&str,len);
}

void txt_Write2file(FILE *fp)
{
#if( !READERDISPLAY_ENABLE)
	_XML_BUFF_link_t *ptr;

	ptr = XML_WORD.XML_BUF;	
	if(ptr == NULL)
		return;
	
	while(ptr)
	{
		FileWrite(fp,ptr->BUFF,ptr->buff_len);
		ptr = ptr->link;			
	}
#endif
}

#if 0
int txt_putc(int c, FILE * shandle)
{
	
	return (int)FileWrite(shandle,&c, 1);
}


int txt_getc(FILE * shandle)
{
	char c;
	FileRead(shandle, &c, 1);
	return c;
}
#endif

void txt_info_init(TXT_INFO_t *txt_base)
{
	gp_TXT_INFO = txt_base;
		
	memset(gp_TXT_INFO,0,sizeof(TXT_INFO_t));

	gp_TXT_INFO->cur_txt = &gp_TXT_INFO->tag_txt;
}

void txt_info_exit()
{
	TXT_DATA_t *pdata,*p;

	ext_mem_free(gp_TXT_INFO->txt_name);
	gp_TXT_INFO->txt_name = NULL;
	
	ext_mem_free(gp_TXT_INFO->txt_type);
	gp_TXT_INFO->txt_type = NULL;
	
	ext_mem_free(gp_TXT_INFO->txt_creator);
	gp_TXT_INFO->txt_creator = NULL;
		
	gp_TXT_INFO->cur_txt = &gp_TXT_INFO->tag_txt;

	pdata = gp_TXT_INFO->cur_txt->next;

	gp_TXT_INFO->cur_txt->next = NULL;
	
	while(pdata)
	{	
		p = pdata;	
		ext_mem_free(pdata->data);
		pdata = pdata->next;
		ext_mem_free(p);
	}
}

#if 0
void pdb_info(PDB_INFO_t *pdb_info,struct pdb *db,int type)
{
	char *str;
	int len;
	switch(type)
	{
		case PDB_NAME:
			str = QUOTE(db->info.name);
			len = strlen(str);
			pdb_info->pdb_name = (char *)ext_mem_malloc(sizeof(char)*len+1);
			strncpy(pdb_info->pdb_name,str,len);
			*(pdb_info->pdb_name+len) = '\0';
			break;

		case PDB_TYPE:
			str = QUOTE(db->info.type);
			len = strlen(str);
			pdb_info->pdb_type= (char *)ext_mem_malloc(sizeof(char)*len+1);
			strncpy(pdb_info->pdb_type,str,len);
			*(pdb_info->pdb_type+len) = '\0';			
			break;

		case PDB_CREATOR:
			str = QUOTE(db->info.creator);
			len = strlen(str);
			pdb_info->pdb_creator = (char *)ext_mem_malloc(sizeof(char)*len+1);
			strncpy(pdb_info->pdb_creator,str,len);
			*(pdb_info->pdb_creator+len) = '\0';
			break;
	}
}
#endif

void txt_data_link(char *buf,int len)
{
	int plen;
	char *pstr,*str;
	
	if( gp_TXT_INFO->cur_txt->data == NULL)
    {
		gp_TXT_INFO->cur_txt->data = (char *)ext_mem_malloc(len+1);
		strncpy(gp_TXT_INFO->cur_txt->data,buf,len);
		*(gp_TXT_INFO->cur_txt->data+len) = '\0';

		gp_TXT_INFO->cur_txt->datalen = len;
    }
	else
	{
		pstr = gp_TXT_INFO->cur_txt->data;
        plen = gp_TXT_INFO->cur_txt->datalen;
		
		str = (char *)ext_mem_malloc(plen+len+1);
		strncpy(str,pstr,plen);
		*(str+plen)='\0';
		
		strncat(str,buf,len);
		*(str+plen+len)='\0';

		ext_mem_free(pstr);
		
		gp_TXT_INFO->cur_txt->data = str;		
		gp_TXT_INFO->cur_txt->datalen = plen+len;
	}		
}

void txt_data_paragraph(int numrecs)
{
	TXT_DATA_t *pdata;
	
	pdata = (TXT_DATA_t *)ext_mem_malloc(sizeof(TXT_DATA_t));
	memset(pdata,0,sizeof(TXT_DATA_t));
	
	gp_TXT_INFO->cur_txt->next = pdata;
	pdata->pre = gp_TXT_INFO->cur_txt;
	gp_TXT_INFO->cur_txt = pdata;
	
	gp_TXT_INFO->cur_txt->numrecs = numrecs;	
}

#if 0
void txt_data_pre()
{
	TXT_DATA_t *pdata;

	pdata = gp_TXT_INFO->cur_txt;
		
	if(pdata->pre == &gp_TXT_INFO->tag_txt)
	{
		return;
	}	
	else
	{
		pdata = pdata->pre;
		gp_TXT_INFO->cur_txt = gp_TXT_INFO->cur_txt->pre;
	}
	mpDebugPrint("====================================================");
    mpDebugPrint("%d %d",pdata->numrecs, pdata->datalen);
	UartOutText(pdata->data);
}
#endif
#if 0
void txt_data_next()
{
	TXT_DATA_t *pdata;

	pdata = gp_TXT_INFO->cur_txt;
	
	if(pdata->next == NULL)
	{
		return;
	}	
	else
	{
		pdata = pdata->next;
		gp_TXT_INFO->cur_txt = gp_TXT_INFO->cur_txt->next;
	}
	mpDebugPrint("====================================================");
    mpDebugPrint("%d %d",pdata->numrecs, pdata->datalen);
	UartOutText(pdata->data);
}
#endif
#if 0
void txt_data_first()
{
	TXT_DATA_t *pdata;
	
	gp_TXT_INFO->cur_txt = &gp_TXT_INFO->tag_txt;
	pdata = gp_TXT_INFO->cur_txt->next;
	gp_TXT_INFO->cur_txt = pdata;

	mpDebugPrint("====================================================");
    mpDebugPrint("%d %d",pdata->numrecs, pdata->datalen);
	UartOutText(pdata->data);
}
#endif

TXT_DATA_t *Get_txt_data_first()
{
	TXT_DATA_t *pdata;
	
	gp_TXT_INFO->cur_txt = &gp_TXT_INFO->tag_txt;
	pdata = gp_TXT_INFO->cur_txt->next;
	gp_TXT_INFO->cur_txt = pdata;
	//mpDebugPrint("pdata =%x numrecs =%d datalen =%d",pdata,pdata->numrecs, pdata->datalen);

	return pdata;
}

