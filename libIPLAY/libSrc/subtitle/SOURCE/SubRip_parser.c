#ifndef __GLOBAL612_H
#include "global612.h"
#endif

#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)

#ifndef MP_TRACE_H
#include "mpTrace.h"
#endif

#ifndef __STREAM_H
#include "stream.h"
#endif

#ifndef _SUB_COMMON_H_
#include "sub_common.h"
#endif




extern S_SUB_NODE_T* g_SubNodeHead;
extern S_SUB_NODE_T* g_SubNodeTail;
extern S_SUB_NODE_T* g_SubNodeCur;

int SubRip_read_file(stream_t* st, E_CHAR_ENCODING utf16, int subtitle_sync_align)
{
	if (NULL == st) 
	{
        mpDebugPrint("sub_stream_create fail: st = 0x%x", st);     
		return FAIL;
	}
	
	g_SubNodeHead = NULL;
	g_SubNodeTail = NULL;
	g_SubNodeCur = NULL;
	
	int ok;
	do
	{	    
	    S_SUB_NODE_T* p_sub_node = mem_malloc(sizeof(S_SUB_NODE_T));
		memset(p_sub_node, 0, sizeof(S_SUB_NODE_T));
		p_sub_node->p_next = NULL;
						
		ok = SubRip_read_line(st, &p_sub_node->s_sub, utf16, subtitle_sync_align);
		if (PASS == ok)
		{
		    if (NULL == g_SubNodeHead)
		    {		    
	            p_sub_node->p_prev = NULL;
	            g_SubNodeHead = p_sub_node; 
                g_SubNodeTail = p_sub_node;
	            g_SubNodeCur  = p_sub_node;					
		    }
		    else
		    {
		        p_sub_node->p_prev = g_SubNodeTail;
			    g_SubNodeTail->p_next = p_sub_node;
		        g_SubNodeTail = p_sub_node;	
		    }
		}
		else
			mem_free(p_sub_node);
	} while (PASS == ok);
	
	sub_stream_free(st);

	return PASS;	
}


static int SubRip_read_line(stream_t* st, S_SUBTITLE_T *current, int utf16_src, int subtitle_sync_align) 
{
    unsigned char* ptr;
    unsigned char line[SUB_LINE_LEN+1];
    unsigned long start_1, start_2, start_3, start_4, end_1, end_2, end_3, end_4;
    char *p=NULL, *q=NULL;
    int len;
    //int line_len;
	
    while (1) 
	{	    
	    
		ptr = sub_read_line(st, line, SUB_LINE_LEN, utf16_src, CHAR_ENCODING_UTF_8);
		if (NULL == ptr)
		{
            //mpDebugPrint("SubRip_read_line return FAIL 1");
			return FAIL;
		}
		
		int i=0;
			
	    if (sscanf (line, "%d:%d:%d,%d --> %d:%d:%d,%d", &start_1, &start_2, &start_3, &start_4, &end_1, &end_2, &end_3, &end_4) < 8) 
			continue;
		
	    current->start = start_1*3600000 + start_2*60000 + start_3*1000 + start_4 + subtitle_sync_align;
		if (current->start < 0)
			current->start = 0;		
		
	    current->end = end_1*3600000 + end_2*60000 + end_3*1000 + end_4 + subtitle_sync_align;
        if (current->end < 0)
			current->end = 0;		
		
	    
	    for (current->lines=0; current->lines < SUB_MAX_TEXT; current->lines++) 
		{
		    current->text[current->lines] = NULL;
		    ptr = sub_read_line(st, line, SUB_LINE_LEN, utf16_src, utf16_src);
		    if (NULL == ptr)
		    {
			    return PASS;
		    }
		
	        p=q=line;
		    
		    if (CHAR_ENCODING_UTF_8 == utf16_src)
		    {
	            for (q=p,len=0; *p && *p!='\r' && *p!='\n' && *p!='|' && strncmp(p,"[br]",4); p++,len++);
				if (0 == len)
					break;
				current->text[current->lines] = (unsigned char *) mem_malloc(len+1); // one byte for "\0"			    
		    }   
		    else
		    {
		       for (q=p,len=0; (*p || *(p+1)) && *p!='\r' && *p!='\n' && *p!='|' && strncmp(p,"[br]",4); p++,len++);		    
			   if (0 == len)
					break;
	 	       len--;	
			   current->text[current->lines] = (unsigned char *) mem_malloc(len+2); // two byte for "\0\0"	           
		    }   
	        if (!current->text[current->lines]) 	        	            
				return FAIL;

			if (CHAR_ENCODING_UTF_8 == utf16_src)
	            strncpy(current->text[current->lines], q, len);
			else
	            wcsncpy(current->text[current->lines], q, len);
            current->text[current->lines][len]='\0';
			if (CHAR_ENCODING_UTF_8 != utf16_src)
			    current->text[current->lines][len+1]='\0'; // add for UTF-16
			#if 0    
	        if (!*p || *p=='\r' || *p=='\n') 
				break;
	        if (*p=='|') p++;
	        else while (*p++!=']');
			#endif
	    }
	    break;
    }

	int i, j;
	
    return PASS;
}




#endif
