/**
 * TODO: fix character escape code cross client's buffer will decode error.
 */
#define LOCAL_DEBUG_ENABLE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "netfs.h"
#include "netfs_pri.h"
#include "ndebug.h"

/* note: for system issue caused by the reason that ext_/main memory is cleared when some module calls re-init of those memory,
 *       use mm_xxxx() functions to replace ext_mem_xxxx() functions for small structures.
 *       For downloading pictures or XML files, we can still use ext_mem_xxxx() functions.
 */

#define xmlhtml_malloc(sz)   mm_malloc(sz)
#define xmlhtml_mfree(ptr)   mm_free(ptr)

/* the return value point to the last character of decoded character */
static const char *html_decode_character(const char *html_input, char *out_ch)
{
    char ch;
    const char *orig_html_input = html_input;

    ch = *html_input;
    if (ch != '&')
    {
        *out_ch = ch;
        return html_input;
    }
    
    html_input ++;
    ch = *html_input;
 
    /* 5.3.1 Numeric character references */
    if (ch == '#')
    {
        /* Escaped Encoding */
        html_input ++;
        ch = *html_input;
        *out_ch = 0;
        if (ch == 'x')
        {
            html_input ++;
            ch = *html_input;
                    
            while (ch)
            {
                if ('0' <= ch && ch <= '9')
                    *out_ch = ((*out_ch)<<4) | (ch - '0');
                else if ('a' <= ch && ch <= 'z')
                    *out_ch = ((*out_ch)<<4) | (ch - 'a' + 10);
                else if ('A' <= ch && ch <= 'Z')
                    *out_ch = ((*out_ch)<<4) | (ch - 'A' + 10);
                else
                    break;

                html_input ++;
                ch = *html_input;
            }
        }
        else
        {
            while ('0' <= ch && ch <= '9')
            {
                *out_ch = (*out_ch) * 10 + (ch - '0');
                html_input ++;
                ch = *html_input;
            }
        }

    }
    else
    {
        /* 5.3.2 Character entity references */
        if (!strncasecmp(html_input, "nbsp", 4))
        {
            *out_ch = ' ';
            html_input += 4;
        }
        else if (!strncasecmp(html_input, "quot", 4))
        {
            *out_ch = '"';
            html_input += 4;
        }
        else if (!strncasecmp(html_input, "amp", 2))
        {
            *out_ch = '&';
            html_input += 3;
        }
        else if (!strncasecmp(html_input, "lt", 2))
        {
            *out_ch = '<';
            html_input += 2;
        }
        else if (!strncasecmp(html_input, "gt", 2))
        {
            *out_ch = '>';
            html_input += 2;
        }

        ch = *html_input;
    }

    if (ch != ';')
    {
        /* error, not a legal escape character, */
        /* we treat & as normal character in case */
        html_input = orig_html_input;
        ch = *html_input;
        *out_ch = ch;
    }

    return html_input;
}

static void html_content(html_parser_t *parser, const char *content, int len)
{
    const char *ptr1, *ptr2;
    const char *tail;
    char ch;

    ptr1 = NULL;
    ptr2 = NULL;
    tail = content + len;
    while (tail - content > 0)
    {
        if (!ptr1)
            ptr1 = content;

        if (*content == '&')
        {
            ptr2 = content;
            if (ptr2 - ptr1 > 0)
                parser->content_handler(parser->user_data, ptr1, ptr2 - ptr1);

            content = html_decode_character(content, &ch);
            parser->content_handler(parser->user_data, &ch, 1);

            ptr1 = NULL;
            ptr2 = NULL;
        }

        content ++;
    }

    if (ptr1)
    {
        ptr2 = content;
        if (ptr2 - ptr1 > 0)
            parser->content_handler(parser->user_data, ptr1, ptr2 - ptr1);

        ptr1 = NULL;
        ptr2 = NULL;
    }
}

static void html_tag_attr_name(html_parser_t *parser, const char *attr_name)
{
    int i;

    //mpDebugPrint("attr_name='%s'\n", attr_name);

    for (i = 0; i < MAX_ATTRS; i++)
    {
        if (!parser->attrs[i])
        {
            parser->attrs[i] = xmlhtml_malloc(strlen(attr_name)+1);
            strcpy(parser->attrs[i], attr_name);
            break;
        }
    }
}

static void html_tag_attr_value(html_parser_t *parser, const char *attr_value)
{
    const char *ptr1, *ptr2;
    const char *tail;
    char *new_attr_value;
    char ch;
    int len;
    int i;
    
    len = strlen(attr_value);
    if (len >= 2 && *attr_value == '\"')
    {
        attr_value ++;
        len -= 2;
    }
    else if (len >= 2 && *attr_value == '\'')
    {
        attr_value ++;
        len -= 2;
    }
    
    new_attr_value = (char *) xmlhtml_malloc(len+1);

    ptr1 = NULL;
    ptr2 = NULL;
    tail = attr_value + len;
    new_attr_value[0] = 0;
    while (tail - attr_value > 0)
    {
        if (!ptr1)
            ptr1 = attr_value;
    
        if (*attr_value == '&')
        {
            ptr2 = attr_value;
            if (ptr2 - ptr1 > 0)
                strncat(new_attr_value, ptr1, ptr2 - ptr1);

            attr_value = html_decode_character(attr_value, &ch);
            strncat(new_attr_value, &ch, 1);

            ptr1 = NULL;
            ptr2 = NULL;
        }
    
        attr_value ++;
    }

    if (ptr1)
    {
        ptr2 = attr_value;
        if (ptr2 - ptr1 > 0)
        {
            strncat(new_attr_value, ptr1, ptr2 - ptr1);
        }

        ptr1 = NULL;
        ptr2 = NULL;
    }

//mpDebugPrint("attr_value='%s'\n", new_attr_value);

    for (i = 0; i < MAX_ATTRS; i++)
    {
        if (!parser->attrs[i])
        {
            parser->attrs[i] = new_attr_value;
            break;
        }
    }

}

static void html_tag_start(html_parser_t *parser, const char *tag_name, char **attrs)
{
//mpDebugPrint("tag_start='%s'\n", tag_name);

    parser->tag_start_handler(parser->user_data, tag_name, attrs);
}

static void html_tag_end(html_parser_t *parser, const char *tag_name)
{
//mpDebugPrint("tag_end='%s'\n", tag_name);

    parser->tag_end_handler(parser->user_data, tag_name);
}

static void push_tag(html_parser_t *parser, const char *tag_name)
{
    struct tag_entry *entry;
    int length;

	//mpDebugPrint("push(%s)\n", tag_name);
    entry = (struct tag_entry *) xmlhtml_malloc(sizeof(struct tag_entry));
    entry->name[0] = 0;
    strncat(entry->name, tag_name, MAX_TAG_NAME);

    entry->prev = parser->tag_list.prev;
    entry->next = &parser->tag_list;
    parser->tag_list.prev->next = entry;
    parser->tag_list.prev = entry;
}

static const char *top_tag(html_parser_t *parser)
{
    if (parser->tag_list.prev == &parser->tag_list)
        return NULL;

    return parser->tag_list.prev->name;
}

static void pop_tag(html_parser_t *parser)
{
    struct tag_entry *entry;

    if (parser->tag_list.prev == &parser->tag_list)
        return;

    entry = parser->tag_list.prev;
    parser->tag_list.prev = entry->prev;
    parser->tag_list.prev->next = &parser->tag_list;
    
	//mpDebugPrint("pop(%s)\n", entry->name);
    xmlhtml_mfree(entry);
}

static void html_element(html_parser_t *parser, const char *element, int len)
{
    char ch,buf1[10];
    int i;

    /* flat html tag and callback handle function */
    while (len > 0)
    {
        ch = *element;
        
        /* html token */
        if (parser->state == HTML_CONTENT)
        {
            if (ch == '<')
            {
                /* notify client */
                *parser->last_content = 0;
                html_content(parser, parser->content, parser->last_content-parser->content);
                parser->content[0] = 0;
                parser->last_content = parser->content;

                parser->state = HTML_TAG_NAME;
            }
            else
            {
                if (parser->last_content - parser->content + 2 < MAX_CONTENT)
                {
                    *parser->last_content = ch;
                    parser->last_content ++;
                }
            }
        }
        else if (parser->state == HTML_TAG_NAME && (!parser->tag_name[0] && (ch == '!' || ch == '?')))
        {
            if (ch == '?')
                parser->state = HTML_PI;
            else
                parser->state = HTML_CDATA_COMMENT;
        }
        else if (parser->state == HTML_TAG_NAME)
        {
            if (ch == '/')
            {
                if (parser->last_tag_name - parser->tag_name)
                {
                    /* XML 1.0: 3.1 Empty-Element Tags [44] */
                    *parser->last_tag_name = 0;
                    push_tag(parser, parser->tag_name);

                    /* notify client */
                    html_tag_start(parser, parser->tag_name, parser->attrs);
                }
                
                parser->flag |= HTML_FLAG_TAG_CLOSE;
            }
            else if (ch == '>')
            {
                /* no attribute case */
                *parser->last_tag_name = 0;
                if (parser->flag & HTML_FLAG_TAG_CLOSE)
                {
                    /* XML 1.0: 3.1 End-Tags [42] */

                    /* close all tag until equal to this tag name */
                    while (top_tag(parser) && strcasecmp(top_tag(parser), parser->tag_name))
                    {
                        /* notify client */
                        html_tag_end(parser, top_tag(parser));
                        pop_tag(parser);
                    }

                    /* close this tag name */
                    if (top_tag(parser) && !strcasecmp(top_tag(parser), parser->tag_name))
                    {
                        /* notify client */
                        html_tag_end(parser, parser->tag_name);
                        pop_tag(parser);
                    }
                    parser->flag &= ~HTML_FLAG_TAG_CLOSE;
                }
                else
                {
                    /* XML 1.0: 3.1 Start-Tags [40] */
                    push_tag(parser, parser->tag_name);

                    /* notify client */
                    html_tag_start(parser, parser->tag_name, parser->attrs);
                }

                parser->tag_name[0] = 0;
                parser->last_tag_name = parser->tag_name;

                parser->state = HTML_CONTENT;
            }
            else if (ch == ' ' || ch == '\t')
            {
                *parser->last_tag_name = 0;
                push_tag(parser, parser->tag_name);
                parser->state = HTML_TAG_ATTR;
            }
            else
            {
                /* append tag name */
                if (parser->last_tag_name - parser->tag_name + 2 < MAX_TAG_NAME)
                {
                    *parser->last_tag_name = ch;
                    parser->last_tag_name ++;
                }
            }
        }
        else if (parser->state == HTML_TAG_ATTR)
        {
            /* allow '>', '/', ' ', '\t' in attr value if it has been quoted */
            /* allow '=' in attr value even it don't quote */
	     sprintf(buf1,"%c",parser->flag);
	     UartOutText(buf1);
//mpDebugPrint("ch='%c'%x", ch, parser->flag);
            if ( ch == '/' && ( !(parser->flag&HTML_FLAG_ATTR_VALUE) ||
                                !(parser->flag&(HTML_FLAG_ATTR_DOUBLE_QUOTE|HTML_FLAG_ATTR_QUOTE)) ) )
            {
                parser->flag |= HTML_FLAG_TAG_CLOSE;
            }
            else if ( ch == '>' && ( !(parser->flag&HTML_FLAG_ATTR_VALUE) || 
                                     !(parser->flag&(HTML_FLAG_ATTR_DOUBLE_QUOTE|HTML_FLAG_ATTR_QUOTE)) ) )
            {   
//mpDebugPrint(">>>>>>>>>>>>>>>>>attr value end\n");
                if ((parser->flag&HTML_FLAG_ATTR_NAME) && (parser->flag&HTML_FLAG_ATTR_VALUE))
                {
                    *parser->last_attr_data = 0;
                    html_tag_attr_value(parser, parser->attr_data);
                    parser->attr_data[0] = 0;
                    parser->last_attr_data = parser->attr_data;
                }
                
                if (parser->flag & HTML_FLAG_TAG_CLOSE)
                {
                    /* XML 1.0: 3.1 Empty-Element Tags [44] */
                    html_tag_start(parser, parser->tag_name, parser->attrs);
                    html_tag_end(parser, parser->tag_name);

                    parser->flag &= ~HTML_FLAG_TAG_CLOSE;
                    pop_tag(parser);
                }
                else
                {
                    /* notify client */
                    html_tag_start(parser, parser->tag_name, parser->attrs);
                }

                for (i = 0; i < MAX_ATTRS; i++)
                    if (parser->attrs[i])
                    {
                        xmlhtml_mfree(parser->attrs[i]);
                        parser->attrs[i] = NULL;
                    }

                /* clear flag */
                parser->flag &= ~HTML_FLAG_ATTR_QUOTE;
                parser->flag &= ~HTML_FLAG_ATTR_DOUBLE_QUOTE;
                parser->flag &= ~HTML_FLAG_ATTR_NAME;
                parser->flag &= ~HTML_FLAG_ATTR_VALUE;

                parser->tag_name[0] = 0;
                parser->last_tag_name = parser->tag_name;
                parser->state = HTML_CONTENT;
            }
            else if (ch == ' ' || ch == '\t' || ch == '\'' || ch == '\"')
            {
                if (parser->flag&HTML_FLAG_ATTR_NAME)
                {
                    if (parser->last_attr_data - parser->attr_data + 2 < MAX_ATTR_DATA)
                    {
                        *parser->last_attr_data = ch;
                        parser->last_attr_data ++;
                    }
                }

                if ((parser->flag&HTML_FLAG_ATTR_NAME) && (parser->flag&HTML_FLAG_ATTR_VALUE))
                {
                    if (parser->last_attr_data - parser->attr_data == 1)
                    {
                        /* the first character of attr value */
                        if (ch == '\'')
                            parser->flag |= HTML_FLAG_ATTR_QUOTE;
                        else if (ch == '\"')
                            parser->flag |= HTML_FLAG_ATTR_DOUBLE_QUOTE;
                    }
                    else
                    {
                        if ( (ch == '\'' && (parser->flag&HTML_FLAG_ATTR_QUOTE)) ||
                             (ch == '\"' && (parser->flag&HTML_FLAG_ATTR_DOUBLE_QUOTE)) )
                        {
                            *parser->last_attr_data = 0;
                            html_tag_attr_value(parser, parser->attr_data);
                            parser->attr_data[0] = 0;
                            parser->last_attr_data = parser->attr_data;

                            /* clear flag */
                            parser->flag &= ~HTML_FLAG_ATTR_QUOTE;
                            parser->flag &= ~HTML_FLAG_ATTR_DOUBLE_QUOTE;
                            parser->flag &= ~HTML_FLAG_ATTR_NAME;
                            parser->flag &= ~HTML_FLAG_ATTR_VALUE;
                        }
                        else if ( ((ch == ' ' || ch == '\t') && 
                                  !(parser->flag&(HTML_FLAG_ATTR_QUOTE|HTML_FLAG_ATTR_DOUBLE_QUOTE))) )
                        {
                            /* remove last character */
                            parser->last_attr_data --;
                            *parser->last_attr_data = 0;

                            html_tag_attr_value(parser, parser->attr_data);
                            parser->attr_data[0] = 0;
                            parser->last_attr_data = parser->attr_data;

                            /* clear flag */
                            parser->flag &= ~HTML_FLAG_ATTR_QUOTE;
                            parser->flag &= ~HTML_FLAG_ATTR_DOUBLE_QUOTE;
                            parser->flag &= ~HTML_FLAG_ATTR_NAME;
                            parser->flag &= ~HTML_FLAG_ATTR_VALUE;
                        }
                        
                    }
                }
            }
            else if (ch == '=' && !(parser->flag&HTML_FLAG_ATTR_VALUE))
            {   /* allow '=' in attr value */
                
                if ((parser->flag&HTML_FLAG_ATTR_NAME) && !(parser->flag&HTML_FLAG_ATTR_VALUE))
                {
                    *parser->last_attr_data = 0;
                    html_tag_attr_name(parser, parser->attr_data);

                    /* attr value start */
                    parser->flag |= HTML_FLAG_ATTR_VALUE;
                    parser->attr_data[0] = 0;
                    parser->last_attr_data = parser->attr_data;
                }
            }
            else
            {
                if (parser->last_attr_data - parser->attr_data + 2 < MAX_ATTR_DATA)
                {
                    *parser->last_attr_data = ch;
                    parser->last_attr_data ++;
                }

                if (!(parser->flag & HTML_FLAG_ATTR_NAME))
                {
                    /* attr name start */
                    parser->flag |= HTML_FLAG_ATTR_NAME;
                }
            }
        }
        else if (parser->state == HTML_PI)
        {
            if (ch == '?')
                parser->state = HTML_PI_E1;
        }
        else if (parser->state == HTML_PI_E1)
        {
            if (ch == '>')
                parser->state = HTML_CONTENT;
            else
                parser->state = HTML_PI;
        }
        else if (parser->state == HTML_CDATA_COMMENT)
        {
            if (ch == '[')
                parser->state = HTML_CDATA_S1;
            else if (ch == '-')
                parser->state = HTML_COMMENT_S1;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA_S2)
        {
            if (ch == 'C')
                parser->state = HTML_CDATA_S2;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA_S2)
        {
            if (ch == 'D')
                parser->state = HTML_CDATA_S3;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA_S3)
        {
            if (ch == 'A')
                parser->state = HTML_CDATA_S4;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA_S4)
        {
            if (ch == 'T')
                parser->state = HTML_CDATA_S5;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA_S5)
        {
            if (ch == 'A')
                parser->state = HTML_CDATA_S6;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA_S6)
        {
            if (ch == '[')
                parser->state = HTML_CDATA;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_CDATA)
        {
            if (parser->last_content - parser->content + 2 < MAX_CONTENT)
            {
                *parser->last_content = ch;
                parser->last_content ++;
            }

            if (ch == ']')
                parser->state = HTML_CDATA_E1;
        }
        else if (parser->state == HTML_CDATA_E1)
        {
            if (parser->last_content - parser->content + 2 < MAX_CONTENT)
            {
                *parser->last_content = ch;
                parser->last_content ++;
            }

            if (ch == ']')
                parser->state = HTML_CDATA_E2;
            else
                parser->state = HTML_CDATA;
        }
        else if (parser->state == HTML_CDATA_E2)
        {
            if (parser->last_content - parser->content + 2 < MAX_CONTENT)
            {
                *parser->last_content = ch;
                parser->last_content ++;
            }
            if (ch == '>')
            {
                *parser->last_content = 0;
                html_content(parser, parser->content, parser->last_content-parser->content-3);
                parser->content[0] = 0;
                parser->last_content = parser->content;

                parser->state = HTML_CONTENT;
            }
            else
                parser->state = HTML_CDATA;
        }
        else if (parser->state == HTML_COMMENT_S1)
        {
            if (ch == '-')
                parser->state = HTML_COMMENT;
            else
                parser->state = HTML_CONTENT;
        }
        else if (parser->state == HTML_COMMENT)
        {
            if (ch == '-')
                parser->state = HTML_COMMENT_E1;
        }
        else if (parser->state == HTML_COMMENT_E1)
        {
            if (ch == '-')
                parser->state = HTML_COMMENT_E2;
            else
                parser->state = HTML_COMMENT;
        }
        else if (parser->state == HTML_COMMENT_E2)
        {
            if (ch == '>')
                parser->state = HTML_CONTENT;
            else
                parser->state = HTML_COMMENT;
        }
        
        element ++;
        len --;
    }
}

void html_init(html_parser_t *parser, void *user_data)
{
    memset(parser, 0, sizeof(html_parser_t));
    parser->state = HTML_NULL;
    parser->tag_list.prev = &parser->tag_list;
    parser->tag_list.next = &parser->tag_list;
    parser->last_content = parser->content;
    parser->last_tag_name = parser->tag_name;
    parser->last_attr_data = parser->attr_data;
    parser->user_data = user_data;
    parser->state = HTML_CONTENT;
}

void html_set_content_handler(html_parser_t *parser, html_content_handler_t content_handler)
{
    parser->content_handler         = content_handler;
}

void html_set_tag_start(html_parser_t *parser, html_tag_start_handler_t tag_start_handler)
{
    parser->tag_start_handler       = tag_start_handler;
}

void html_set_tag_end(html_parser_t *parser, html_tag_end_handler_t tag_end_handler)
{
    parser->tag_end_handler         = tag_end_handler;
}

void html_parse(html_parser_t *parser, const char *html_input, int len)
{
    html_element(parser, html_input, len);
}

void html_exit(html_parser_t *parser)
{
    int i;
    
    for (i = 0; i < MAX_ATTRS; i++)
        if (parser->attrs[i])
        {
            xmlhtml_mfree(parser->attrs[i]);
            parser->attrs[i] = NULL;
        }

    while (top_tag(parser))
        pop_tag(parser);
}
