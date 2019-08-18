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


//

extern S_SUB_NODE_T* g_SubNodeHead;
extern S_SUB_NODE_T* g_SubNodeTail;
extern S_SUB_NODE_T* g_SubNodeCur;
extern S_SUBTITLE_INFO_T g_sSubtitleInfo;

int SAMI_read_file(stream_t* st, E_CHAR_ENCODING utf16_src, int subtitle_sync_align)
{
    //__asm("break 100");
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
		
		//mpDebugPrint("p_sub_node = 0x%x", p_sub_node);
		ok = SAMI_read_line(st, &p_sub_node->s_sub, utf16_src, subtitle_sync_align);
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


static int SAMI_read_line(stream_t* st, S_SUBTITLE_T *current, int utf16_src, int subtitle_sync_align) 
{
    static char line[SUB_LINE_LEN+1];
    static char *s = NULL, *slacktime_s;
	static char *lang_s;
    char text[SUB_LINE_LEN+1], *p=NULL, *q;
    int state;
	int sub_slacktime = 20000; //20 sec
	int sub_no_text_pp=0;   // 1 => do not apply text post-processing
                        

    current->lines = current->start = current->end = 0;
    current->alignment = SUB_ALIGNMENT_BOTTOMCENTER;
	if (g_sSubtitleInfo.lang_index != LANGUAGE_UNKNOWN)
        state = 0;
	else
		state = -1;

    //__asm("break 100");
    /* read the first line */
    if (!s)
    {
	    if (!(s = sub_read_line(st, line, SUB_LINE_LEN, utf16_src, CHAR_ENCODING_UTF_8)))
	    {
			mpDebugPrint("SAMI_read_line return FAIL 1");
			return FAIL;
	    }	
    }
	
    do 
	{
	    switch (state) 
		{
		    case -1: /* find language lang: */
				lang_s = sub_stristr(s, "lang:");
				if (lang_s)
				{				    
				    if (sub_stristr(lang_s, "lang:en"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_ENGLISH;
				    }
					else if (sub_stristr(lang_s, "lang:de"))
				    {
					    g_sSubtitleInfo.lang_index = LANGUAGE_GERMANY;
					}
					else if (sub_stristr(lang_s, "lang:es"))
					{
					    g_sSubtitleInfo.lang_index = LANGUAGE_SPANISH;
					}
					else if (sub_stristr(lang_s, "lang:fr"))
					{
					    g_sSubtitleInfo.lang_index = LANGUAGE_FRENCH;
					}	
					else if (sub_stristr(lang_s, "lang:it"))
					{
					    g_sSubtitleInfo.lang_index = LANGUAGE_ITALY;
					}	
					else if (sub_stristr(lang_s, "lang:nl"))
					{
					    g_sSubtitleInfo.lang_index = LANGUAGE_DUTCH;
					}
					else if (sub_stristr(lang_s, "lang:pl"))
					{
					    g_sSubtitleInfo.lang_index = LANGUAGE_POLISH;
					}
					else if (sub_stristr(lang_s, "lang:pt"))
					{
					    g_sSubtitleInfo.lang_index = LANGUAGE_PORTUGEES;
					}
					else if (sub_stristr(lang_s, "lang:ru"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_RUSSIAN;
				    }	
					else if (sub_stristr(lang_s, "lang:sv"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_SWEDISH;
				    }
					else if (sub_stristr(lang_s, "lang:tr"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_TURKISH;
				    }
					#if FONT_GB2
					else if (sub_stristr(lang_s, "lang:zh-cn"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_S_CHINESE;
				    }
					#endif
					#if FONT_BIG5
					else if (sub_stristr(lang_s, "lang:zh-tw"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_T_CHINESE;
				    }
					#endif
					#if FONT_JIS
					else if (sub_stristr(lang_s, "lang:ja"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_JAPAN;
				    }
					#endif
					#if FONT_KSC
				    else if (sub_stristr(lang_s, "lang:ko") || sub_stristr(lang_s, "lang:kr"))
				    {
				        g_sSubtitleInfo.lang_index = LANGUAGE_KOREA;
				    }
					#endif
				    state = 0;
				}
				
			break;
			
	        case 0: /* find "START=" or "Slacktime:" */				
	            slacktime_s = sub_stristr(s, "Slacktime:");
	            if (slacktime_s)                    
                    sub_slacktime = strtol(slacktime_s+10, NULL, 0);

	            s = sub_stristr(s, "Start=");
	            if (s) 
				{				    
		            current->start = strtol(s + 6, &s, 0) + subtitle_sync_align;
					if (current->start < 0)
						current->start = 0;					
						
                    /* eat '>' */
                    for (; *s != '>' && *s != '\0'; s++);
                    s++;
		            state = 1; 
					continue;
	            }
	        break;

	        case 1: /* find (optionnal) "<P", skip other TAGs */
	            for  (; *s == ' ' || *s == '\t'; s++); /* strip blanks, if any */
	            if (*s == '\0') 
					break;
				
	            if (*s != '<') 
				{ 
				    state = 3; 
					p = text; 
					continue; 
				} /* not a TAG */
	            s++;
	            if (*s == 'P' || *s == 'p') 
				{ 
				    s++; 
					state = 2; 
					continue; 
				} /* found '<P' */

				for (; *s != '>' && *s != '\0'; s++); /* skip remains of non-<P> TAG */
	                if (s == '\0')
	                    break;
	            s++;
	        continue;

	        case 2: /* find ">" */
	            if ((s = strchr(s, '>'))) 
			    { 
			        s++; 
				    state = 3; 
				    p = text; 
				    continue; 
			    }
	        break;

	        case 3: /* get all text until '<' appears */
	            if (*s == '\0') 
					break;
	            else if (!strncasecmp (s, "<br>", 4)) 
				{
		            *p = '\0'; p = text; sub_trail_space(text);
		            if (text[0] != '\0')
		            {
		               current->text[current->lines++] = sub_strdup(text);		                
		            }   
					
		            s += 4;
	            }
	            else if ((*s == '{') && !sub_no_text_pp) 
				{ 
				    state = 5; ++s; 
					continue; 
				}
	            else if (*s == '<') 
				{ 
				    state = 4; 
				}
	            else if (!strncasecmp(s, "&nbsp", 5)) 
				{ 				    
				    *p++ = ' '; 
					s += 6; 					
				}
	            else if (*s == '\t') 
				{ 
				    *p++ = ' '; 
					s++; 
				}
	            else if (*s == '\r' || *s == '\n') 
				{ 
				    s++; 
				}
	            else *p++ = *s++;

	            /* skip duplicated space */
	            if (p > text + 2) 
					if (*(p-1) == ' ' && *(p-2) == ' ') 
						p--;

	          continue;

	          case 4: /* get current->end or skip <TAG> */
	              q = sub_stristr(s, "Start=");
	              if (q) 
				  {		              
		              current->end = strtol(q + 6, &q, 0) + subtitle_sync_align;
					  if (current->end < 0)
					  	  current->end = 0;
					  
		              *p = '\0'; sub_trail_space (text);
		              if (text[0] != '\0')
					  //	__asm("break 100");
		                  current->text[current->lines++] = sub_strdup(text);
		              if (current->lines > 0) 
					  { 
					      state = 99; 
						  break; 
					  }
		              state = 0; 
					  continue;
	              }
	              s = strchr(s, '>');
	              if (s) 
				  { 
				      s++; 
					  state = 3; 
					  continue; 
			      }
	          break;
			  
              case 5: /* get rid of {...} text, but read the alignment code */
	              if ((*s == '\\') && (*(s + 1) == 'a') && !sub_no_text_pp) 
				  {
                      if (sub_stristr(s, "\\a1") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_BOTTOMLEFT;
                          s = s + 3;
                      }
                      if (sub_stristr(s, "\\a2") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_BOTTOMCENTER;
                          s = s + 3;
                      } 
					  else if (sub_stristr(s, "\\a3") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_BOTTOMRIGHT;
                          s = s + 3;
                      } 
					  else if ((sub_stristr(s, "\\a4") != NULL) || (sub_stristr(s, "\\a5") != NULL) || (sub_stristr(s, "\\a8") != NULL)) 
					  {
                          current->alignment = SUB_ALIGNMENT_TOPLEFT;
                          s = s + 3;
                      } 
					  else if (sub_stristr(s, "\\a6") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_TOPCENTER;
                          s = s + 3;
                      } 
					  else if (sub_stristr(s, "\\a7") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_TOPRIGHT;
                          s = s + 3;
                      } 
					  else if (sub_stristr(s, "\\a9") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_MIDDLELEFT;
                          s = s + 3;
                      } 
					  else if (sub_stristr(s, "\\a10") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_MIDDLECENTER;
                          s = s + 4;
                      } 
					  else if (sub_stristr(s, "\\a11") != NULL) 
					  {
                          current->alignment = SUB_ALIGNMENT_MIDDLERIGHT;
                          s = s + 4;
                      }
	              }

				  if (*s == '}') 
				  	  state = 3;
	              ++s;
	              continue;
	         }

	         /* read next line */
	         if (state != 99 && !(s = sub_read_line(st, line, SUB_LINE_LEN, utf16_src, CHAR_ENCODING_UTF_8))) 
	         {
	             if (current->start > 0) 
		         {
		              break; // if it is the last subtitle
	             } 
		         else 
		         {
		             return FAIL;
	             }
	         }
         } while (state != 99);

    // For the last subtitle
    if (current->end <= 0) 
    {
        current->end = current->start + sub_slacktime + subtitle_sync_align;
		if (current->end < 0)
			current->end = 0;
	    *p = '\0'; sub_trail_space(text);
	    if (text[0] != '\0')
	    current->text[current->lines++] = sub_strdup(text);
    }

    return PASS;
}




#endif
