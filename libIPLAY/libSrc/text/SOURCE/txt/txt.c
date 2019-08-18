
#include "global612.h"
#include <string.h>
#include "txt_mpx.h"

#define MAX_DESC        8192

#define EOF -1

typedef STREAM FILE;

static TXT_INFO_t TXT_INFO;
extern TXT_INFO_t *gp_TXT_INFO;

static char *readfile(FILE *fp,int size) 
{
  int c;
  int n = 4096;
 
  char *file,*fpt;  
	
  file = (char *)ext_mem_malloc(sizeof(char)*size);
  fpt = file;
  while(size)
  {
	if(size > MAX_DESC)
	{
		FileRead(fp,fpt,MAX_DESC);
		fpt += MAX_DESC;	
		size -= MAX_DESC;
		//mpDebugPrint("111 file = %p, size =%d ",file,size);
	}
	else
	{
		FileRead(fp,fpt,size);
		fpt += size;
		size =0;
		//mpDebugPrint("222 file = %p, size =%d ",file,size);
	}
  }
  return file;
}

static int filepos;

int f_getc(char *file)
{
	int ch;

	ch =*(file + filepos);
	 filepos++;
	 
	return ch;

}

char txt_getc(char* file,int size)
{		
	int numrecs = 0;
	char *ptr;
	int len;
	
    register char ch1,ch2;
	
#if TXT_DEBUG
	FILE *fout;
	fout = txt_fopen(SD_MMC,"txtdebug.txt");
#endif

	filepos = 0; 
	ptr = file+filepos;
	
	while(size)
	{
		txt_data_paragraph(numrecs++);
		
		while(size)
		{
			ch1 = (char)f_getc(file);
			size--;
			if(ch1 == '\r')
			{
				ch2 = (char)f_getc(file);
				size--;
				
    	 	 	if(ch2 == '\n')
    	 	 	{
					len = (file+filepos) - ptr;
#if TXT_DEBUG	
					word_xml_Write2Mem(ptr,len);
#endif
					txt_data_link(ptr,len);

					ptr = file+filepos;

					break;
    	 	 	}
			}	
		}
	}
	
	len = (file+filepos) - ptr;
#if TXT_DEBUG	
	word_xml_Write2Mem(ptr,len);
	txt_Write2file(fout);
	FileClose(fout);	
#endif	
	txt_data_link(ptr,len);
	
	gp_TXT_INFO->total_numrecs = numrecs;
}

int main_txt(FILE *fp) 
{
	
	char *file;
	int size;
	
	size = (int)FileSizeGet(fp);

	if(size == 0)
       	return FAIL; 

	if((file = readfile(fp,size)) == FAIL)
		return;

	txt_info_init(&TXT_INFO);
	
	txt_getc(file,size);	
	ext_mem_free(file);
    
}





