/*  
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1 


#include "global612.h"
#include "mpTrace.h"
//#include "heaputil_mem.h"
//#include <stdio.h>
#if EREADER_ENABLE
#if( !READERDISPLAY_ENABLE)
//#include "..\..\xml\include\expat.h"
#include "expat.h"
#include "xmlword.h"
#include "wordfunc.h"
//#include <string.h>

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define word_malloc   ext_mem_malloc
#define word_mfree   ext_mem_free
 
XML_WORD_t XML_WORD;
_XML_BUFF_link_t *g_XML_BUF;

static XML_info_t XML_info;

char * strncat(char *ret, register const char *s2, size_t n)
{
         register char *s1 = ret;
 
         if (n > 0) {
                 while (*s1++)
                         /* EMPTY */ ;
                 s1--;
                 while (*s1++ = *s2++)  {
                         if (--n > 0) continue;
                         *s1 = '\0';
                         break;
                 }
                 return ret;
         } else return s1;
}

static u_char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strcasecmp(const char *s1,const char *s2)
{
	register u_char	*cm = charmap,
			*us1 = (u_char *)s1,
			*us2 = (u_char *)s2;

	while (cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return(0);
	return(cm[*us1] - cm[*--us2]);
}

char *strrchr (register const char *s, int c)
{
  char *rtnval = 0;

  do {
    if (*s == c)
      rtnval = (char*) s;
  } while (*s++);
  return (rtnval);
}


//------------------------------------------------------------------------
STREAM * xml_fopen(char device_id, char *filename)
{
	STREAM *shandle;
	DRIVE *drv;
	SWORD swRet;
	BYTE  *pri_name, *ext; 
	BYTE  temp_filename[MAX_L_NAME_LENG];
	int   i;

	memset(temp_filename, 0, MAX_L_NAME_LENG);
	StringCopy08(temp_filename, filename);
	pri_name = temp_filename;
	for (i = (StringLength08(temp_filename) - 1); i > 0; i--)
	{
		if (temp_filename[i] == '.')
			break; /* found rightest '.' */
	}

	if (i)
	{
		ext = (BYTE *) &temp_filename[i + 1];
		temp_filename[i] = '\0';
	}
	else
	{
		mpDebugPrint("%s: -E- No extension name found !", __FUNCTION__);
		return NULL;
	}

	drv = DriveChange(device_id);

	// the code must be the first file of the selected card
	if (DirReset(drv) != FS_SUCCEED)
		return NULL;
		
	if (FileSearch(drv, pri_name, ext, E_FILE_TYPE) == FS_SUCCEED)
		shandle = FileOpen(drv);
	else
	{
		mpDebugPrint("%s: -E- File search failed !", __FUNCTION__);
		return NULL;
	}
	return shandle;
}

int xml_fclose(STREAM *shandle)
{

	return FileClose(shandle);
	
}

DWORD xml_fread(STREAM *shandle, BYTE *buffer, DWORD size) 
{

	return FileRead(shandle, buffer, size);

}

DWORD xml_fwrite(STREAM *shandle, BYTE *buffer, DWORD size) 
{

	return FileWrite(shandle, buffer, size);
}

DWORD xml_fFileSize(STREAM * shandle)
{
	return FileSizeGet(shandle);
}

int xml_fSeek(STREAM * shandle, DWORD position, DWORD origin )
{

	switch (origin)
	{
		case SEEK_SET:
			Seek(shandle,position);
			break;
		case SEEK_END:
			EndOfFile(shandle);
			break;
	}
	return 0;
}

void xml_rewind(STREAM * shandle)
{
	 SeekSet(shandle);
}

STREAM * xml_fcreate(STREAM *shandle)
{
	SWORD swRet;
	
	BYTE name[] = "debugout";
	BYTE ext[] = "XML";
	DRIVE *drv;
	int ret;

	drv = DriveChange(SD_MMC);
	if (DirReset(drv) != FS_SUCCEED)
		return NULL;

	
	ret = FileSearch(drv, name, ext,E_FILE_TYPE);
	if (ret != FS_SUCCEED) 
	{
		drv=DriveGet(SD_MMC);
		ret=CreateFile(drv, name, ext);
		if (ret)
		{
			UartOutText("create file fail\r\n");
			return NULL;
		}
		else
		{
			shandle = FileOpen(drv);			
			if(!shandle)
				return NULL;
		}
	}
	else
	{		
		drv=DriveGet(SD_MMC);
		shandle = FileOpen(drv);
		DeleteFile(shandle);

		ret=CreateFile(drv, name, ext);
		if (ret)
		{
			UartOutText("create file fail\r\n");
			return NULL;
		}
		shandle = FileOpen(drv);
		if(!shandle)
			return NULL;
	}
	return shandle;
}

#if Make_LRF
//-----------------------------------------------------------------------------
void lrf_content_handler(void *user_data, const char *s, int len)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
    	 
    	XMLCHAPTER_t   *ptag;
	XMLData_t *pdata;
	char *str;
			
	if(xml_info->state == XML_LRF_BLOCK_PRAR_FONT)
	{
		memset(xml_info->font,0,len+1);
		strncat(xml_info->font,s, len);	
		return;
	}
	else if(xml_info->state == XML_LRF_BLOCK_FONT)
		return;
	else if(xml_info->state == XML_LRF_BLOCK_IMG)
		return;
	else if(xml_info->state == XML_LRF_PAGE)
		return;	
	
}

void lrf_start_element_handler(void *user_data, const char *name, const char **attr)
{
	
	XML_info_t   *xml_info = (XML_info_t *) user_data;
       XMLCHAPTER_t   *ptag;
	XMLData_t *pdata, *pdata1;
	
	if(!strcasecmp(name, "bfont"))
	{
			xml_info->state = XML_LRF_BLOCK_FONT;
			
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));
			//memset(xml_info->font,0,MAX_DESC);
			
			pdata->tagnum = XML_LRF_BLOCK_FONT;		

			while (*attr)
             		{
             			if (!strcasecmp(*attr, "fontsize"))
             			{
             				attr ++;								
	      				pdata->fontSize = atoi(*attr);
				}
             			else if (!strcasecmp(*attr, "faceface"))
             			{
             				attr ++;                    				
					sprintf(pdata->fontstyle,"%s",*attr);
				}
				else if (!strcasecmp(*attr, "textcolor"))
              		{
              			attr ++;                    			
                    			sprintf(pdata->fontColor,"%s",*attr);
                		}		
                		else
                		{
                    			attr ++;
                		}                
              		attr ++;
             		}
		
			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}		
			return ;

	}		
	else if(!strcasecmp(name, "font"))
	{
			xml_info->state = XML_LRF_BLOCK_PRAR_FONT;
		
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));
			//memset(xml_info->font,0,MAX_DESC);
			
			pdata->tagnum = XML_LRF_BLOCK_PRAR_FONT;		

			while (*attr)
             		{
             			if (!strcasecmp(*attr, "len"))
             			{
             				attr ++;								
	      				pdata->strlength = atoi(*attr);
				}
				else if (!strcasecmp(*attr, "attr"))
             			{
             				attr ++;                    				
					sprintf(pdata->fontAttr,"%s",*attr);
				}
             			else
                		{
                    			attr ++;
                		}                
              		attr ++;
             		}
		
			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}	
			return ;
	}			
	else	 if(!strcasecmp(name, "img"))
	{
			xml_info->state = XML_LRF_BLOCK_IMG;
		
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));
			//memset(xml_info->font,0,MAX_DESC);
			
			pdata->tagnum = XML_LRF_BLOCK_IMG;		

			while (*attr)
             		{
             			if (!strcasecmp(*attr, "link"))
             			{
             				attr ++;								
	      				sprintf(pdata->imglink,"%s",*attr);
				}
				else if (!strcasecmp(*attr, "rect"))
             			{
             				attr ++;                    				
					sprintf(pdata->imgrect,"%s",*attr);
				}
				else if (!strcasecmp(*attr, "size"))
             			{
             				attr ++;                    				
					sprintf(pdata->imgsize,"%s",*attr);
				}
             			else
                		{
                    			attr ++;
                		}                
              		attr ++;
             		}
		
			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}	
			return ;
	}	
	else if(!strcasecmp(name, "page")) //page is the lrf chapter
	{				  
			xml_info->state = XML_LRF_PAGE;

			ptag = (XMLCHAPTER_t *)word_malloc(sizeof(XMLCHAPTER_t));
			memset(ptag,0,sizeof(XMLCHAPTER_t));
			ptag->tagnum = XML_LRF_PAGE;
	
			XML_info.cur_tag->next = ptag;
			ptag->pre = XML_info.cur_tag;
	 		XML_info.cur_tag= ptag; 		
			return ;
	}
}

void lrf_end_element_handler(void *user_data, const char *name)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
	XMLData_t *pdata;	
	char *str;
	int len;

	if(xml_info->state == XML_LRF_BLOCK_PRAR_FONT)
	{
			//xml_info->state = XML_LRF_BLOCK_PARA;
		
			len = strlen(xml_info->font)+1;

			str = (char *)word_malloc(len);
			*(str+len)='\0';
		
			memcpy(str,xml_info->font,len);
		
			XML_info.cur_data->strdata= str;
			XML_info.cur_data->strlength = len;		
			return;
	}	
	else if(xml_info->state == XML_LRF_BLOCK_FONT)
		return;
	else if(xml_info->state == XML_LRF_BLOCK_IMG)
		return;
	else if(xml_info->state == XML_LRF_PAGE)
		return;	
}
#endif

#if Make_PDF
//-------------------------------------------------------------------
void pdf_content_handler(void *user_data, const char *s, int len)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
    	 
    	XMLCHAPTER_t   *ptag;
	XMLData_t *pdata;
	char *str;

	if(xml_info->state == XML_BOOK)
		return;
	else if(xml_info->state == XML_PAGE)
		return;
	else if(xml_info->state == XML_FONT)
		return;
	else if(xml_info->state == XML_PARA)
	{
		strncat(xml_info->font,s, len);
		return;
	}
	else if(xml_info->state == XML_IMG)
	{
		return;
	}

}	

void pdf_start_element_handler(void *user_data, const char *name, const char **attr)
{
	
	XML_info_t   *xml_info = (XML_info_t *) user_data;
       XMLCHAPTER_t   *ptag;
	XMLData_t *pdata, *pdata1;

	if(xml_info->state == XML_BOOK)
	{
		if(!strcasecmp(name, "book"))
		{		
			//mpDebugPrint("C %s",name);  
		  
			xml_info->state = XML_BOOK;

			ptag = (XMLCHAPTER_t *)word_malloc(sizeof(XMLCHAPTER_t));
			memset(ptag,0,sizeof(XMLCHAPTER_t));
			ptag->tagnum = XML_BOOK;
	
			XML_info.cur_tag->next = ptag;
			ptag->pre = XML_info.cur_tag;
	 		XML_info.cur_tag= ptag; 
		
			return ;
		}	
		else if(!strcasecmp(name, "page"))
		{
			//mpDebugPrint("P %s",name);  

			xml_info->state = XML_PAGE;
		
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));

			pdata->tagnum = XML_FONT;

			while (*attr)
             		{
             			if (!strcasecmp(*attr, "number"))
             			{
             				attr ++;								
	      				pdata->pagenum = atoi(*attr);
				}
            			else
              		{
              			attr ++;
              		}                
              		attr ++;
             		}
		
			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}
			return ;
		}	
	}	
	else if(xml_info->state == XML_PAGE)
	{
		if(!strcasecmp(name, "font"))
		{
			//mpDebugPrint("F %s",name);  

			xml_info->state = XML_FONT;
		
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));
			memset(xml_info->font,0,MAX_DESC);
			
			pdata->tagnum = XML_FONT;		

			while (*attr)
             		{
             			if (!strcasecmp(*attr, "size"))
             			{
             				attr ++;								
	      				pdata->fontSize = atoi(*attr);
				}
             			else if (!strcasecmp(*attr, "style"))
             			{
             				attr ++;                    				
					sprintf(pdata->fontstyle,"%s",*attr);
				}
				else if (!strcasecmp(*attr, "color"))
              		{
              			attr ++;                    			
                    			sprintf(pdata->fontColor,"%s",*attr);
                		}		
                		else
                		{
                    			attr ++;
                		}                
              		attr ++;
             		}
		
			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}
		
			return ;
		}
	}	
	else if(xml_info->state == XML_FONT)
	{
		if(!strcasecmp(name, "para"))
		{
			//mpDebugPrint("P %s",name);  

			xml_info->state = XML_PARA;
		
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));

			pdata->tagnum = XML_PARA;

			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}
			return ;
		}	
		else if(!strcasecmp(name, "img"))
		{
			//mpDebugPrint("F %s",name);  

			xml_info->state = XML_IMG;
		
			pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
			memset(pdata,0,sizeof(XMLData_t));
			memset(xml_info->font,0,MAX_DESC);
			
			pdata->tagnum = XML_IMG;		

			while (*attr)
             		{
             			if (!strcasecmp(*attr, "width"))
             			{
             				attr ++;								
	      				pdata->imgwidth = atoi(*attr);
				}
             			else if (!strcasecmp(*attr, "height"))
             			{
             				attr ++;                    				
					pdata->imgheight = atoi(*attr);
				}
				else if (!strcasecmp(*attr, "src"))
              		{
              			attr ++;                    			
                    			sprintf(pdata->imglink,"%s",*attr);
                		}		
                		else
                		{
                    			attr ++;
                		}                
              		attr ++;
             		}
		
			if(!XML_info.cur_tag->data)
			{
				XML_info.cur_tag->data = pdata;
				XML_info.cur_data = pdata;
			}	
			else
			{
				XML_info.cur_data->data = pdata;
				XML_info.cur_data = pdata;
			}
		
			return ;
		}
	} 		
}

void pdf_end_element_handler(void *user_data, const char *name)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
	//XMLTag_t   *ptag;
	XMLData_t *pdata;	
	char *str;
	int len;

	if(xml_info->state == XML_PARA)
	{
		xml_info->state  = XML_FONT;

		len = strlen(xml_info->font)+1;

		str = (char *)word_malloc(len);
		*(str+len)='\0';
		
		memcpy(str,xml_info->font,len);
		
		XML_info.cur_data->strdata= str;
		XML_info.cur_data->strlength = len;
	
		return;
	}
	else if(xml_info->state == XML_IMG)
	{
		xml_info->state  = XML_FONT;
		return;
	}
	else if(xml_info->state == XML_FONT)
	{
		xml_info->state  = XML_PAGE;
		return;
	}
	else if(xml_info->state == XML_PAGE)
	{
		xml_info->state  = XML_BOOK;
		return;
	}
	else if(xml_info->state == XML_BOOK)
	{
		xml_info->state  = XML_NULL;
		return;
	}

}

#endif

#if Make_UNRTF
//-----------------------------------------------------------------------------
void rtf_content_handler(void *user_data, const char *s, int len)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
    	 
    	XMLCHAPTER_t   *ptag;
	XMLData_t *pdata;
	char *str;

	if(xml_info->state == XML_PARA)
	{
		return;
	}
	else if(xml_info->state == XML_FONT)
	{
		strncat(xml_info->font,s, len);
		return;
	}
	//=========================================	
	else if(xml_info->state == XML_INFOTABLE)
	{
		return;		
	}
	else if(xml_info->state == XML_TGROUP)
	{
		return;		
	}
	else if(xml_info->state == XML_TBODY)
	{
		return;		
	}
	else if(xml_info->state == XML_ROW)
	{
		return;		
	}
	else if(xml_info->state == XML_ENTRY)
	{
		strncat(xml_info->font,s, len);
		return;
	}

}
void rtf_start_element_handler(void *user_data, const char *name, const char **attr)
{
	
	XML_info_t   *xml_info = (XML_info_t *) user_data;
       XMLCHAPTER_t   *ptag;
	XMLData_t *pdata, *pdata1;

	if(!strcasecmp(name, "para"))
	{
		//mpDebugPrint("P %s",name);  

		xml_info->state = XML_PARA;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_PARA;

		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		return ;
	}
	else if(!strcasecmp(name, "font"))
	{
		//mpDebugPrint("F %s",name);  

		xml_info->state = XML_FONT;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));
		memset(xml_info->font,0,MAX_DESC);
			
		pdata->tagnum = XML_FONT;		

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "len"))
             		{
             			attr ++;								
	      			pdata->strlength = atoi(*attr);
			}
            		else if (!strcasecmp(*attr, "size"))
             		{
             			attr ++;								
	      			pdata->fontSize = atoi(*attr);
			}
             		else if (!strcasecmp(*attr, "face"))
             		{
             			attr ++;                    				
				sprintf(pdata->fontstyle,"%s",*attr);
			}
			else if (!strcasecmp(*attr, "color"))
              	{
              		attr ++;                    			
                    		sprintf(pdata->fontColor,"%s",*attr);
                	}		
                	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
		
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "informaltable"))
	{
		//mpDebugPrint("I %s",name);  

		xml_info->state = XML_INFOTABLE;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_INFOTABLE;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "frame"))
             		{
             			attr ++;								
	      			sprintf(pdata->frame,"%s",*attr);
			}
            		else if (!strcasecmp(*attr, "colsep"))
             		{
             			attr ++;								
	      			pdata->colsep = atoi(*attr);
			}
             		else if (!strcasecmp(*attr, "rowsep"))
              	{
              		attr ++;                    			
                    		pdata->rowsep = atoi(*attr);
                	}		
                	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
	else if(!strcasecmp(name, "tgroup"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_TGROUP;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_TGROUP;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "cols"))
             		{
             			attr ++;								
	      			pdata->cols = atoi(*attr);
			}
            		else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
		
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "tbody"))
	{
		xml_info->state = XML_TBODY;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_TBODY;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "row"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_ROW;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_ROW;

		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "entry"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_ENTRY;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));
		memset(xml_info->font,0,MAX_DESC);

		pdata->tagnum = XML_ENTRY;

		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}		
	else if(!strcasecmp(name, "book"))
	{		
		//mpDebugPrint("C %s",name);  
		  
		xml_info->state = XML_BOOK;

		ptag = (XMLCHAPTER_t *)word_malloc(sizeof(XMLCHAPTER_t));
		memset(ptag,0,sizeof(XMLCHAPTER_t));
		ptag->tagnum = XML_BOOK;
	
		XML_info.cur_tag->next = ptag;
		ptag->pre = XML_info.cur_tag;
	 	XML_info.cur_tag= ptag; 
		
		return ;
	}	
}

void rtf_end_element_handler(void *user_data, const char *name)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
	//XMLTag_t   *ptag;
	XMLData_t *pdata;	
	char *str;
	int len;

	if(!strcasecmp(name, "para"))
	{
		return;
	}
	else if(!strcasecmp(name, "font"))
	{
		len = strlen(xml_info->font)+1;

		str = (char *)word_malloc(len);
		*(str+len)='\0';
		
		memcpy(str,xml_info->font,len);
		
		XML_info.cur_data->strdata= str;
		XML_info.cur_data->strlength = len;
		return;
	}
	else if(!strcasecmp(name, "informaltable"))
	{
		return;
	}
	else if(!strcasecmp(name, "entry"))
	{
		len = strlen(xml_info->font)+1;

		str = (char *)word_malloc(len);
		*(str+len)='\0';
		
		memcpy(str,xml_info->font,len);
		
		XML_info.cur_data->strdata= str;
		XML_info.cur_data->strlength = len;
		return;
	}
	else if(!strcasecmp(name, "book"))
	{
		return;
	}	
	return;
}

#endif

#if Make_ANTIWORD
//-------------------------------------------------------------------
void doc_content_handler(void *user_data, const char *s, int len)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
    	 
    	XMLCHAPTER_t   *ptag;
	XMLData_t *pdata;
	char *str;
	if(len ==1)
	{
		if(*s == '\n')
			return;
		//UartOutText(XML_info.cur_data->strdata);
	}
	else if(xml_info->state == XML_PARA)
	{
		return;
	}
	else if(xml_info->state == XML_FONT)
	{
		strncat(xml_info->font,s, len);
		return;
	}
	else if(xml_info->state == XML_IMG)
	{
		return;
	}
	else if(xml_info->state == XML_TITLE)
	{
		return;
	}
//=========================================	
	else if(xml_info->state == XML_INFOTABLE)
	{
		return;		
	}
	else if(xml_info->state == XML_TGROUP)
	{
		return;		
	}
	else if(xml_info->state == XML_TBODY)
	{
		return;		
	}
	else if(xml_info->state == XML_ROW)
	{
		return;		
	}
	else if(xml_info->state == XML_ENTRY)
	{
		strncat(xml_info->font,s, len);
		return;
	}
//=========================================		
	else if(xml_info->state == XML_ORDERLIST)
	{
		return;

	}
//=========================================	
	else if(xml_info->state == XML_LISTITEM)
	{
		return;

	}
//=========================================	
	else if(xml_info->state == XML_SECT1)
	{
		return;

	}
	else if(xml_info->state == XML_SECT2)
	{
		return;

	}
	else if(xml_info->state == XML_SECT3)
	{
		return;

	}
	else if(xml_info->state == XML_SECT4)
	{
		return;

	}
	else if(xml_info->state == XML_SECT5)
	{
		return;

	}
//=========================================		
	else if(xml_info->state == XML_ITEMIZEDLIST)
	{
		return;

	}
//=========================================
	else if(xml_info->state == XML_CHAPTER)
	{
		return;

	}
	
}

void doc_start_element_handler(void *user_data, const char *name, const char **attr)
{
	
	XML_info_t   *xml_info = (XML_info_t *) user_data;
       XMLCHAPTER_t   *ptag;
	XMLData_t *pdata, *pdata1;	


	if(!strcasecmp(name, "para"))
	{
		//mpDebugPrint("P %s",name);  

		xml_info->state = XML_PARA;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_PARA;

		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		return ;
	}
	else if(!strcasecmp(name, "font"))
	{
		//mpDebugPrint("F %s",name);  

		xml_info->state = XML_FONT;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));
		memset(xml_info->font,0,MAX_DESC);
			
		pdata->tagnum = XML_FONT;		

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "len"))
             		{
             			attr ++;								
	      			pdata->strlength = atoi(*attr);
			}
            		else if (!strcasecmp(*attr, "size"))
             		{
             			attr ++;								
	      			pdata->fontSize = atoi(*attr);
			}
             		else if (!strcasecmp(*attr, "style"))
             		{
             			attr ++;                    				
				sprintf(pdata->fontstyle,"%s",*attr);
			}
			else if (!strcasecmp(*attr, "color"))
              	{
              		attr ++;                    			
                    		sprintf(pdata->fontColor,"%s",*attr);
                	}		
                	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
		
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "img"))
	{
		//mpDebugPrint("F %s",name);  

		xml_info->state = XML_IMG;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));
		memset(xml_info->font,0,MAX_DESC);
			
		pdata->tagnum = XML_IMG;		

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "link"))
             		{
             			attr ++;								
	      			sprintf(pdata->imglink,"%s",*attr);
			}
            		else if (!strcasecmp(*attr, "width"))
             		{
             			attr ++;								
	      			pdata->imgwidth = atoi(*attr);
			}
             		else if (!strcasecmp(*attr, "height"))
             		{
             			attr ++;                    				
				pdata->imgheight = atoi(*attr);
			}
			else if (!strcasecmp(*attr, "horsizescaled"))
              	{
              		attr ++;                    			
                    		pdata->imghorsizescaled = atoi(*attr);
                	}
			else if (!strcasecmp(*attr, "versizescaled"))
              	{
              		attr ++;                    			
                    		pdata->imgversizescaled = atoi(*attr);
                	}
                	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
		
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "title"))
	{
		if(xml_info->state == XML_BOOK)
			return;
		
		//mpDebugPrint("T %s",name); 
		
		xml_info->state = XML_TITLE;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));
		
		pdata->tagnum = XML_TITLE;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
//======================================================	
	else if(!strcasecmp(name, "informaltable"))
	{
		//mpDebugPrint("I %s",name);  

		xml_info->state = XML_INFOTABLE;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_INFOTABLE;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "frame"))
             		{
             			attr ++;								
	      			sprintf(pdata->frame,"%s",*attr);
			}
            		else if (!strcasecmp(*attr, "colsep"))
             		{
             			attr ++;								
	      			pdata->colsep = atoi(*attr);
			}
             		else if (!strcasecmp(*attr, "rowsep"))
              	{
              		attr ++;                    			
                    		pdata->rowsep = atoi(*attr);
                	}		
                	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
	else if(!strcasecmp(name, "tgroup"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_TGROUP;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_TGROUP;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "cols"))
             		{
             			attr ++;								
	      			pdata->cols = atoi(*attr);
			}
            		else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
		
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "tbody"))
	{
		xml_info->state = XML_TBODY;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_TBODY;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "row"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_ROW;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_ROW;

		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	
	else if(!strcasecmp(name, "entry"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_ENTRY;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));
		memset(xml_info->font,0,MAX_DESC);

		pdata->tagnum = XML_ENTRY;

		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}		
//======================================================
	else if(!strcasecmp(name, "orderedlist"))
	{
		//mpDebugPrint("O %s",name); 

		xml_info->state = XML_ORDERLIST;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_ORDERLIST;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "numeration"))
             		{
             			attr ++;								
	      			sprintf(pdata->numeration,"%s",*attr);
			}
            		else if (!strcasecmp(*attr, "inheritnum"))
             		{
             			attr ++;								
	      			sprintf(pdata->inheritnum,"%s",*attr);
			}
             		else if (!strcasecmp(*attr, "continuation"))
              	{
              		attr ++;                    			
                    		sprintf(pdata->continuation,"%s",*attr);
                	}		
                	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
		
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
//=================================================	
	else if(!strcasecmp(name, "listitem"))
	{
		//mpDebugPrint("O %s",name); 

		xml_info->state = XML_LISTITEM;
		
		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_LISTITEM;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "mark"))
             		{
             			attr ++;								
	      			sprintf(pdata->mark,"%s",*attr);
			}
            		else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}	

//=======================================================	
	else if(!strcasecmp(name, "sect1"))
	{
		//mpDebugPrint("S %s",name);

		xml_info->state = XML_SECT1;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_SECT1;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
	else if(!strcasecmp(name, "sect2"))
	{
		//mpDebugPrint("S %s",name);

		xml_info->state = XML_SECT2;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_SECT2;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
	else if(!strcasecmp(name, "sect3"))
	{
		//mpDebugPrint("S %s",name);

		xml_info->state = XML_SECT3;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_SECT3;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
	else if(!strcasecmp(name, "sect4"))
	{
		//mpDebugPrint("S %s",name);

		xml_info->state = XML_SECT4;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_SECT4;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
	else if(!strcasecmp(name, "sect5"))
	{
		//mpDebugPrint("S %s",name);

		xml_info->state = XML_SECT5;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_SECT5;
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
//====================================================		
	else if(!strcasecmp(name, "itemizedlist"))
	{
		//mpDebugPrint("I %s",name);

		xml_info->state = XML_ITEMIZEDLIST;

		pdata = (XMLData_t *)word_malloc(sizeof(XMLData_t));
		memset(pdata,0,sizeof(XMLData_t));

		pdata->tagnum = XML_ITEMIZEDLIST;

		while (*attr)
             	{
             		if (!strcasecmp(*attr, "mark"))
             		{
             			attr ++;								
	      			sprintf(pdata->mark,"%s",*attr);
			}
                    	else
                	{
                    		attr ++;
                	}                
              	attr ++;
             	}
	
		if(!XML_info.cur_tag->data)
		{
			XML_info.cur_tag->data = pdata;
			XML_info.cur_data = pdata;
		}	
		else
		{
			XML_info.cur_data->data = pdata;
			XML_info.cur_data = pdata;
		}
		
		return ;
	}
//=====================================================	
	else if(!strcasecmp(name, "chapter"))
	{		
		//mpDebugPrint("C %s",name);  
		  
		xml_info->state = XML_CHAPTER;

		ptag = (XMLCHAPTER_t *)word_malloc(sizeof(XMLCHAPTER_t));
		memset(ptag,0,sizeof(XMLCHAPTER_t));
		ptag->tagnum = XML_CHAPTER;
	
		XML_info.cur_tag->next = ptag;
		ptag->pre = XML_info.cur_tag;
	 	XML_info.cur_tag= ptag; 
		
		return ;
	}	
}

void doc_end_element_handler(void *user_data, const char *name)
{
	XML_info_t   *xml_info = (XML_info_t *) user_data;
	//XMLTag_t   *ptag;
	XMLData_t *pdata;	
	char *str;
	int len;

	//mpDebugPrint("E name = %s",name);
	
	if(!strcasecmp(name, "para"))
	{
		return;
	}
	else if(!strcasecmp(name, "font"))
	{
		len = strlen(xml_info->font)+1;

		str = (char *)word_malloc(len);
		*(str+len)='\0';
		
		memcpy(str,xml_info->font,len);
		
		XML_info.cur_data->strdata= str;
		XML_info.cur_data->strlength = len;
		return;
	}
	if(!strcasecmp(name, "img"))
	{
		return;
	}
	else if(!strcasecmp(name, "title"))
	{
		return;
	}
	else if(!strcasecmp(name, "informaltable"))
	{
		return;
	}
	else if(!strcasecmp(name, "entry"))
	{
		len = strlen(xml_info->font)+1;

		str = (char *)word_malloc(len);
		*(str+len)='\0';
		
		memcpy(str,xml_info->font,len);
		
		XML_info.cur_data->strdata= str;
		XML_info.cur_data->strlength = len;
		return;
	}
	else if(!strcasecmp(name, "orderedlist"))
	{
		return;
	}
	else if(!strcasecmp(name, "sect1"))
	{	
		return;
	}
	else if(!strcasecmp(name, "itemizedlist"))
	{
		return;
	}
	else if(!strcasecmp(name, "chapter"))
	{
		return;
	}	
	return;
}	

#endif

static int word_xml_link(XML_info_t  *xml_info,BYTE FileType)
{
	XML_Parser parser;
    	int              ret = 0;
    	int              len;
		
	enum XML_Status status;
	
    	char *data; 
    	_XML_BUFF_link_t *ptr;
	
    	mpDebugPrint("word_xml_link ");

	xml_info->state = XML_BOOK;
		
   	// Get Data from Remote Site and Parse it 
    	parser = XML_ParserCreate(NULL);    	

	
	switch(FileType)
	{
#if Make_ANTIWORD	
		case EBOOK_TYPE_DOC:
			XML_SetUserData(parser, xml_info);
    			XML_SetElementHandler(parser, doc_start_element_handler, doc_end_element_handler);
    			XML_SetCharacterDataHandler(parser, doc_content_handler);
			break;
#endif

#if Make_PDF
		case EBOOK_TYPE_PDF:
			XML_SetUserData(parser, xml_info);
    			XML_SetElementHandler(parser, pdf_start_element_handler, pdf_end_element_handler);
    			XML_SetCharacterDataHandler(parser, pdf_content_handler);
			break;
#endif

#if Make_UNRTF
		case EBOOK_TYPE_RTF:
			XML_SetUserData(parser, xml_info);
    			XML_SetElementHandler(parser, rtf_start_element_handler, rtf_end_element_handler);
    			XML_SetCharacterDataHandler(parser, rtf_content_handler);
			break;
#endif

#if Make_LRF
		case EBOOK_TYPE_LRF:
			XML_SetUserData(parser, xml_info);
    			XML_SetElementHandler(parser, lrf_start_element_handler, lrf_end_element_handler);
    			XML_SetCharacterDataHandler(parser, lrf_content_handler);
			break;	
#endif			
	}

	ptr = XML_WORD.XML_BUF;	
		
       while (ptr != NULL)
    	{
       	data = ptr->BUFF;
        	len = ptr->buff_len;
		mpDebugPrint("word_xml_link => len = %d",len);	
		
        	if (len == MAX_DESC)
        	{
        		status = XML_Parse(parser, data, len, 0); 

			if (status != XML_STATUS_OK)
        		{
            			mpDebugPrint("rss_init: %s at line %d, column %d\n",
                		XML_ErrorString(XML_GetErrorCode(parser)),
                		XML_GetCurrentLineNumber(parser),
                		XML_GetCurrentColumnNumber(parser));
				break;		
        		}
        	}	
		else
		{	
			status = XML_Parse(parser, data, len, 1);

			if (status != XML_STATUS_ERROR) 
				XML_ParserReset(parser, 0);
			break;

		}	
        	ptr = ptr->link;	
	}
	XML_ParserFree(parser);
	return status;
}


int word_readxmlfile(STREAM *shandle)
{
	 int fsize;
	 int readsize;
	 _XML_BUFF_link_t *pbuf;	


	 XML_WORD.total_len = FileSizeGet(shandle);
	 fsize =  XML_WORD.total_len;
	 pbuf = XML_WORD.ptr;
	 
	 while(fsize)
	 {
	 	
	 	readsize = (fsize > MAX_DESC) ? MAX_DESC :fsize;			
	
		xml_fread(shandle,pbuf->BUFF,readsize);	

		mpDebugPrint("word_xmlinit fread = %d, read = %d ",fsize ,readsize);
		pbuf->buff_len = readsize;		
		fsize -=readsize;

		if(fsize)
		{
			pbuf = (_XML_BUFF_link_t *)word_malloc(sizeof(_XML_BUFF_link_t));
			memset(pbuf,0,sizeof(_XML_BUFF_link_t));
			XML_WORD.ptr->link = pbuf;
			XML_WORD.ptr = pbuf;
		}
	 }
}

void word_xmlfile(STREAM * sfileout,const char *format, ...)
{

	char buffer[8192];
	va_list argptr; 
	 int cnt; 
 	va_start(argptr, format);

 	cnt = vsnprintf(buffer,8192 ,format, argptr);
	FileWrite(sfileout,&buffer,cnt);
	va_end(argptr);

}

void word_xmltest(BYTE FileType)
{
	XMLCHAPTER_t *ptag;
	XMLData_t *pdata;
	int i ,len;
	STREAM *sfileout;
#define OUT_TEST 0

	ptag =  XML_info.tag_list.next;

	if (ptag == NULL)
		return;

#if (OUT_TEST == 1)

#if Make_LRF
		switch(FileType)
		{
			case EBOOK_TYPE_LRF:
				while(ptag)
				{
					if(ptag->tagnum == XML_LRF_PAGE)
					{	
						MP_DEBUG("<page>");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
							case XML_LRF_HEADER_BLOCK:
							case XML_LRF_BLOCK:
								//UartOutText("<block>");
								break;
							case XML_LRF_BLOCK_FONT:
								UartOutText("<Block font>");
								mpDebugPrint("%s, %d, %s",pdata->fontstyle,pdata->fontSize,pdata->fontColor);
								break;
							case XML_LRF_BLOCK_IMG:
								UartOutText("<img>");
								mpDebugPrint("%s, %s, %s",pdata->imglink,pdata->imgrect,pdata->imgsize);
								break;	
							case XML_LRF_BLOCK_PRAR_FONT:
								UartOutText("<Para font>");
								mpDebugPrint("%d, %s",pdata->strlength,pdata->strdata);
								break;						
					  	}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				   }					
				   break;
#endif

#if Make_ANTIWORD
			 case EBOOK_TYPE_DOC:
				while(ptag)
				{
					if(ptag->tagnum == XML_CHAPTER)
					{
					MP_DEBUG("<chapter>");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
						case XML_TITLE:
							MP_DEBUG("<title>");
							break;
						case XML_FONT:
							MP_DEBUG("<font>");
							mpDebugPrint("%d, %s, %d, %s",pdata->strlength,pdata->fontstyle,pdata->fontSize,pdata->fontColor);
							UartOutText(pdata->strdata);
							UartOutText("\r\n");
							break;
						case XML_PARA:
							MP_DEBUG("<para>");
							break;
						case XML_IMG:
							mpDebugPrint("<img>");
							mpDebugPrint("%s, %d, %d, %d,%d",pdata->imglink,pdata->imgwidth,pdata->imgheight,pdata->imghorsizescaled,pdata->imgversizescaled);
							UartOutText("\r\n");
							break;

						//Table    
						case XML_INFOTABLE:
							MP_DEBUG("<informaltable>");					
							mpDebugPrint("%s, %d, %d",pdata->frame,pdata->colsep,pdata->rowsep);					
							UartOutText("\r\n");
							break;
						case XML_TGROUP:
							MP_DEBUG("<tgroup>");					
							mpDebugPrint("%d",pdata->cols);
							UartOutText("\r\n");
							break;
						case XML_TBODY:
							MP_DEBUG("<tbody>");					
							break;
						case XML_ROW:
							MP_DEBUG("<row>");					
							break;
						case XML_ENTRY:
							MP_DEBUG("<entry>");					
							UartOutText(pdata->strdata);					
							UartOutText("\r\n");				
							break;
					
						//Order List
						case XML_ORDERLIST:
							MP_DEBUG("<orderedlist>");					
							mpDebugPrint("%s, %s, %s",pdata->numeration,pdata->inheritnum,pdata->continuation);					
							UartOutText("\r\n");
							break;
						case XML_LISTITEM:
							MP_DEBUG("<listitem>");					
							break;
					
						//SECTTION
						case XML_SECT1:
							MP_DEBUG("<sect1>");
							break;					
						case XML_SECT2:
							MP_DEBUG("<sect2>");
							break;
						case XML_SECT3:
							MP_DEBUG("<sect3>");
							break;
						case XML_SECT4:
							MP_DEBUG("<sect4>");
							break;
						case XML_SECT5:
							MP_DEBUG("<sect5>");
							break;
					
						//ITEMIZEDLIST
						case XML_ITEMIZEDLIST:
							MP_DEBUG("<itemizedlist>");
							break;
						//XML_PAGE
						case XML_PAGE:
							MP_DEBUG("<page>");
							break;
						}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}
				break;
#endif
#if Make_PDF
			case EBOOK_TYPE_PDF:
				while(ptag)
				{
					if(ptag->tagnum == XML_CHAPTER)
					{
						MP_DEBUG("<chapter>");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
						case XML_FONT:
							MP_DEBUG("<font>");
							mpDebugPrint("%d, %s, %d, %s",pdata->strlength,pdata->fontstyle,pdata->fontSize,pdata->fontColor);
							break;
						case XML_PARA:
							MP_DEBUG("<para>");
							UartOutText(pdata->strdata);
							UartOutText("\r\n");
							break;
						case XML_IMG:					
							MP_DEBUG("<img>");
							mpDebugPrint("%s, %d, %d",pdata->imglink,pdata->imgwidth,pdata->imgheight);
							break;					
						}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}
				break;
#endif
#if Make_UNRTF
			case EBOOK_TYPE_RTF:
				while(ptag)
				{
					if(ptag->tagnum == XML_CHAPTER)
					{
						MP_DEBUG("<chapter>");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
						case XML_FONT:
							MP_DEBUG("<font>");
							mpDebugPrint("%d, %s, %d, %s",pdata->strlength,pdata->fontstyle,pdata->fontSize,pdata->fontColor);
							UartOutText(pdata->strdata);
							UartOutText("\r\n");
							break;
						case XML_PARA:
							MP_DEBUG("<para>");
							break;
						case XML_IMG:
							break;
						}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}
				break;
#endif				
		}	
//-------------------------------------------------------------------
#elif (OUT_TEST == 2)

#if Make_LRF
		switch(FileType)
		{
			case EBOOK_TYPE_LRF:
		
				while(ptag)
				{
					if(ptag->tagnum == XML_LRF_PAGE)
					{	
						word_xmlfile(sfileout,"<page>");
						word_xmlfile(sfileout,"\r\n");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
							case XML_LRF_HEADER_BLOCK:
							case XML_LRF_BLOCK:
								//word_xmlfile(sfileout,"<block>");
								break;
							case XML_LRF_BLOCK_FONT:
								word_xmlfile(sfileout,"<Block font>");
								word_xmlfile(sfileout,"%s, %d, %s",pdata->fontstyle,pdata->fontSize,pdata->fontColor);
								word_xmlfile(sfileout,"\r\n");
								break;
							case XML_LRF_BLOCK_IMG:
								word_xmlfile(sfileout,"<img>");
								word_xmlfile(sfileout,"%s, %s, %s",pdata->imglink,pdata->imgrect,pdata->imgsize);
								break;	
							case XML_LRF_BLOCK_PRAR_FONT:
								word_xmlfile(sfileout,"<Para font>");
								word_xmlfile(sfileout,"%d, %s",pdata->strlength,pdata->strdata);
								word_xmlfile(sfileout,"\r\n");
								break;						
					  	}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}					
				break;
#endif

#if Make_ANTIWORD
			case EBOOK_TYPE_DOC:	
		
				while(ptag)
				{
					if(ptag->tagnum == XML_CHAPTER)
					{
						word_xmlfile(sfileout,"<chapter>");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
						case XML_TITLE:
							MP_DEBUG("<title>");
							break;
						case XML_FONT:
							word_xmlfile(sfileout,"<font>");
							word_xmlfile(sfileout,"%d, %s, %d, %s",pdata->strlength,pdata->fontstyle,pdata->fontSize,pdata->fontColor);
							word_xmlfile(sfileout,pdata->strdata);
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_PARA:
							MP_DEBUG("<para>");
							break;
						case XML_IMG:
							word_xmlfile(sfileout,"<img>");
							word_xmlfile(sfileout,"%s, %d, %d, %d, %d",pdata->imglink,pdata->imgwidth,pdata->imgheight,pdata->imghorsizescaled,pdata->imgversizescaled);
							word_xmlfile(sfileout,"\r\n");							
							break;

						//Table    
						case XML_INFOTABLE:
							word_xmlfile(sfileout,"<informaltable>");					
							word_xmlfile(sfileout,"%s, %d, %d",pdata->frame,pdata->colsep,pdata->rowsep);					
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_TGROUP:
							word_xmlfile(sfileout,"<tgroup>");					
							word_xmlfile(sfileout,"%d",pdata->cols);
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_TBODY:
							word_xmlfile(sfileout,"<tbody>");
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_ROW:
							word_xmlfile(sfileout,"<row>");
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_ENTRY:
							word_xmlfile(sfileout,"<entry>");					
							word_xmlfile(sfileout,pdata->strdata);					
							word_xmlfile(sfileout,"\r\n");				
							break;
					
						//Order List
						case XML_ORDERLIST:
							word_xmlfile(sfileout,"<orderedlist>");					
							word_xmlfile(sfileout,"%s, %s, %s",pdata->numeration,pdata->inheritnum,pdata->continuation);					
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_LISTITEM:
							word_xmlfile(sfileout,"<listitem>");
							word_xmlfile(sfileout,"\r\n");
							break;
					
						//SECTTION
						case XML_SECT1:
							word_xmlfile(sfileout,"<sect1>");
							word_xmlfile(sfileout,"\r\n");
							break;					
						case XML_SECT2:
							word_xmlfile(sfileout,"<sect2>");
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_SECT3:
							word_xmlfile(sfileout,"<sect3>");
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_SECT4:
							word_xmlfile(sfileout,"<sect4>");
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_SECT5:
							word_xmlfile(sfileout,"<sect5>");
							word_xmlfile(sfileout,"\r\n");
							break;
					
						//ITEMIZEDLIST
						case XML_ITEMIZEDLIST:
							word_xmlfile(sfileout,"<itemizedlist>");
							word_xmlfile(sfileout,"\r\n");
							break;
						//XML_PAGE
						case XML_PAGE:
							word_xmlfile(sfileout,"<page>");
							word_xmlfile(sfileout,"\r\n");
							break;
						}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}
				break;
#endif

			case EBOOK_TYPE_PDF:
			
				while(ptag)
				{
					if(ptag->tagnum == XML_CHAPTER)
					{
						word_xmlfile(sfileout,"<chapter>");
						word_xmlfile(sfileout,"\r\n");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
						case XML_FONT:
							word_xmlfile(sfileout,"<font>");
							word_xmlfile(sfileout,"%d, %s, %d, %s",pdata->strlength,pdata->fontstyle,pdata->fontSize,pdata->fontColor);
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_PARA:
							word_xmlfile(sfileout,"<para>");
							word_xmlfile(sfileout,pdata->strdata);
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_IMG:
							word_xmlfile(sfileout,"<img>");
							word_xmlfile(sfileout,"%s, %d, %d",pdata->imglink,pdata->imgwidth,pdata->imgheight);
							word_xmlfile(sfileout,"\r\n");
							break;					
						}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}
				break;

#if Make_UNRTF
			case EBOOK_TYPE_RTF:
			
				while(ptag)
				{
					if(ptag->tagnum == XML_CHAPTER)
					{
						word_xmlfile(sfileout,"<chapter>");
						word_xmlfile(sfileout,"\r\n");
					}
					pdata = ptag->data;
				
					while(pdata)
					{
						switch(pdata->tagnum)
						{				
						case XML_FONT:
							word_xmlfile(sfileout,"<font>");
							word_xmlfile(sfileout,"%d, %s, %d, %s",pdata->strlength,pdata->fontstyle,pdata->fontSize,pdata->fontColor);
							word_xmlfile(sfileout,pdata->strdata);
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_PARA:
							word_xmlfile(sfileout,"<para>");
							word_xmlfile(sfileout,"\r\n");
							break;
						case XML_IMG:
							break;
						}
						pdata = pdata->data;
					}
					ptag = ptag->next;
				}
				break;
#endif				
		}	
#endif	
}

void word_xmlexit()
{
	XMLCHAPTER_t *ptag,*pfreetag;
	XMLData_t *pdata,*pfreedata;
	int i ,len;
	
	ptag =  XML_info.tag_list.next;		
		
	while(ptag)
	{
		pdata = ptag->data;
		
		while(pdata)
		{
			pfreedata= pdata;
			pdata =pdata->data;
			word_mfree(pfreedata->strdata);
			word_mfree(pfreedata);
		}
		ptag->data = NULL;
		
		pfreetag = ptag;
		ptag = ptag->next;
		word_mfree(pfreetag);
	}

	XML_info.tag_list.next = NULL;
	XML_info.cur_tag = NULL; 
	XML_info.cur_data = NULL;
}

void word_Pdfxmlexit()
{
#if Make_PDF
	Ereader_PDFxmlexit(&XML_info);
#endif
}

int word_xmlinit(BYTE FileType)
{
	int     ret;	

 	memset(&XML_info, 0, sizeof(XML_info_t));
       XML_info.cur_tag= &XML_info.tag_list;
	XML_info.cur_data= NULL;

	ret = word_xml_link(&XML_info,FileType);	
	word_xmltagfree(); //free tag memory alloc buffer
	
	//word_xmltest(FileType);

	#if 0
	if(ret == XML_STATUS_OK)
	{
		//DisplayWinInit();
		//FE_INIT();
		//reader_xml_init_chapter(&XML_info,FileType);		
	}
	else 
	{
		mpDebugPrint("word_xmlinit fail !!! = %d",ret);
	}
	#endif
	
	return ret;
	
}

void word_xmltaginit()
{
	_XML_BUFF_link_t *xml_buff;
	mpDebugPrint("word_xmltaginit");	
	memset(&XML_WORD, 0, sizeof(XML_WORD_t));
	xml_buff = (_XML_BUFF_link_t *)word_malloc(sizeof(_XML_BUFF_link_t));
	memset(xml_buff,0,sizeof(_XML_BUFF_link_t));
	XML_WORD.XML_BUF = xml_buff;
	XML_WORD.ptr= XML_WORD.XML_BUF;		
}

void word_xmltagfree()
{
	_XML_BUFF_link_t *xml_buff,*ptr;

	mpDebugPrint("word_xmltagfree");	
	
	xml_buff = XML_WORD.XML_BUF;

	if(xml_buff == NULL)
		return;

	while(xml_buff)
	{
		ptr = xml_buff->link;
		word_mfree(xml_buff);
		xml_buff = ptr;
	}
	XML_WORD.XML_BUF = NULL;
}

DWORD word_xml_Write2Mem(BYTE *buf,int len)
{
	BYTE *tar;	 
    int copylen, wLength=0;
	_XML_BUFF_link_t *xml_buff;

	XML_WORD.total_len += len;
	//mpDebugPrint("XML_WORD.total_len = %d",XML_WORD.total_len);
		
	while(len >0)
	{
		TaskYield();
	
		if((XML_WORD.ptr->buff_len) < MAX_DESC)
    		{
    			tar  = XML_WORD.ptr->BUFF + XML_WORD.ptr->buff_len;
			
			copylen = MIN(len, MAX_DESC - XML_WORD.ptr->buff_len);
		
    			memcpy(tar,buf,copylen);

			buf += copylen;
			XML_WORD.ptr->buff_len += copylen;	
			len -= copylen;
			wLength += copylen;
		}
		else
		{
			xml_buff = (_XML_BUFF_link_t  *)word_malloc(sizeof(_XML_BUFF_link_t));
			if(!xml_buff)
			{
				MP_ALERT("word_xml_Write2Mem: out of memory");
                    		BREAK_POINT();
			}
			memset(xml_buff,0,sizeof(_XML_BUFF_link_t)); 
                	
                	XML_WORD.ptr->link = xml_buff;
                	XML_WORD.ptr = xml_buff;
		}	
	}		
	return wLength;
}


void word_xml_Write2Mem_test(int test)
{

	if(test == 0)     // No output test
		return;
	else if(test == 1)  //output to uart
	{
		_XML_BUFF_link_t *ptr;

		ptr = XML_WORD.XML_BUF;	

		if(ptr == NULL)
			return;
		
		while(ptr)
		{
			UartOutText(ptr->BUFF);
			ptr = ptr->link;			
		}
	}
	else if(test == 2) //output to file
	{
       	STREAM *sfileout;
		_XML_BUFF_link_t *ptr;

		sfileout = xml_fcreate(sfileout);
       	if(!sfileout) 
       	{	
       		mpDebugPrint("File create fail!!!");
			return ;
       	}
	   
		ptr = XML_WORD.XML_BUF;	
		if(ptr == NULL)
			return;
	
		while(ptr)
		{
			xml_fwrite(sfileout,ptr->BUFF,ptr->buff_len);
			ptr = ptr->link;			
		}
		xml_fclose(sfileout);
	}
}


int PDF_Typesettinginit(int ret)
{	
#if Make_PDF
	if(ret == XML_STATUS_OK)
	{
		mpDebugPrint(" PDF_Typesettinginit xml_info->tag_list.next =  %x",XML_info.tag_list.next);
		PDF_StartTypesetting(&XML_info);
	}
	else 
	{
		mpDebugPrint("word_xmlinit fail !!! = %d",ret);
	}
	return ret;
#endif
	
}
int PdfPagechange_TypesettingInit(int ret)
{
#if Make_PDF
	if(ret == XML_STATUS_OK)
	{
		mpDebugPrint(" PDF_Typesettinginit xml_info->tag_list.next =  %x",XML_info.tag_list.next);
		PdfPageChange_DataSet(&XML_info);
	}
	else 
	{
		mpDebugPrint("word_xmlinit fail !!! = %d",ret);
	}
	return ret;
#endif
}

int Typesettinginit(int ret)
{	
	if(ret == XML_STATUS_OK)
	{
		#if Make_ANTIWORD
			reader_xml_init_chapter(&XML_info);	
		#endif
	}
	else 
	{
		mpDebugPrint("word_xmlinit fail !!! = %d",ret);
	}
	return ret;
	
}

#endif//READERDISPLAY_ENABLE
#endif  // #if EREADER_ENABLE
