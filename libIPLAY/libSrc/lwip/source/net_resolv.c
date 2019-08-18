/**
 * \addtogroup apps
 * @{
 */ 

/**
 * \defgroup resolv DNS resolver
 * @{
 *
 * The uIP DNS resolver functions are used to lookup a hostname and
 * map it to a numerical IP address. It maintains a list of resolved
 * hostnames that can be queried with the resolv_lookup()
 * function. New hostnames can be resolved using the resolv_query()
 * function.
 *
 * When a hostname has been resolved (or found to be non-existant),
 * the resolver code calls a callback function called resolv_found()
 * that must be implemented by the module that uses the resolver.
 */

/**
 * \file
 * DNS host name to IP address resolver.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * This file implements a DNS host name to IP address resolver.
 */

/*
 * Copyright (c) 2002-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: resolv.c,v 1.5 2006/06/11 21:46:37 adam Exp $
 *
 */

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"

#include <linux/types.h>
#include "typedef.h"
#include "net_sys.h"
#include "socket.h"
#include "net_netdb.h"
#include "uip.h"
#include "net_netctrl.h"

#include <string.h>

/** \internal The maximum number of retries when asking for a name. */
#define MAX_RETRIES 8

struct namemap {
#define STATE_UNUSED 0
#define STATE_NEW    1
#define STATE_ASKING 2
#define STATE_DONE   3
#define STATE_ERROR  4
  u8_t state;
  u8_t tmr;
  u8_t retries;
  u8_t seqno;
  u8_t err;
  char name[32];
  u32_t ttl;
  u32_t ipaddr;
};

#define RESOLV_ENTRIES 8
#define MAX_PROCESSED_ENTRIES 4


static struct namemap names[RESOLV_ENTRIES];

static u8_t seqno;
static u8_t u08ResolvTimerId;


static void resolv_timer_func()
{
    static u16_t n_index;
    u16_t i;
    register struct namemap *namemapptr;
    //MP_DEBUG("resolv_timer_func is called");
    for(i=0; i < MAX_PROCESSED_ENTRIES; ++i) {
        namemapptr = &names[n_index];
        if(namemapptr->state == STATE_DONE) {
            if (namemapptr->ttl > 0)
            {
                namemapptr->ttl--;
                //MP_DEBUG("resolv_timer_func ttl=%d:", namemapptr->ttl);
                if (namemapptr->ttl == 0)
                {
                    MP_DEBUG("resolv_timer_func timeout n=%s:", namemapptr->name);
                    namemapptr->state = STATE_UNUSED;
                }
            }
        }
        else if(namemapptr->state == STATE_ERROR) {
//            ns_send(namemapptr->name, n_index);
        }

        n_index++;
        if (n_index >= RESOLV_ENTRIES)
            n_index = 0;
    }
}

/*---------------------------------------------------------------------------*/
/** \internal
 * Runs through the list of names to see if there are any that have
 * not yet been queried and, if so, sends out a query.
 */
/*---------------------------------------------------------------------------*/
#if 0
static void
check_entries(void)
{
    static u8_t i;
    static u8_t n;
    register struct namemap *namemapptr;

    for(i = 0; i < RESOLV_ENTRIES; ++i) {
        namemapptr = &names[i];
        if(namemapptr->state == STATE_NEW ||
                namemapptr->state == STATE_ASKING) {
            if(namemapptr->state == STATE_ASKING) {
                if(--namemapptr->tmr == 0) {
                    if(++namemapptr->retries == MAX_RETRIES) {
                        namemapptr->state = STATE_ERROR;
                        resolv_found(namemapptr->name, NULL);
                        continue;
                    }
                    namemapptr->tmr = namemapptr->retries;
                } else {
                    /*	  printf("Timer %d\n", namemapptr->tmr);*/
                    /* Its timer has not run out, so we move on to next
                       entry. */
                    continue;
                }
            } else {
                namemapptr->state = STATE_ASKING;
                namemapptr->tmr = 1;
                namemapptr->retries = 0;
            }

            break;
        }
    }
}
#endif
/*---------------------------------------------------------------------------*/
/**
 * Queues a name so that a question for the name will be sent out.
 *
 * \param name The hostname that is to be queried.
 */
/*---------------------------------------------------------------------------*/
int get_hostinfo(S08* name, struct hostent *h, u32_t *pttl, u8_t seqno);
u32_t resolv_query(char *name)
{
  static u8_t i;
  static u8_t lseq, lseqi;
  register struct namemap *nameptr;
      
  lseq = lseqi = 0;
  
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    nameptr = &names[i];
    if(nameptr->state == STATE_UNUSED) {
      break;
    }
    if(seqno - nameptr->seqno > lseq) {
      lseq = seqno - nameptr->seqno;
      lseqi = i;
    }
  }

  if(i == RESOLV_ENTRIES) {
    i = lseqi;
    nameptr = &names[i];
  }

  /*  printf("Using entry %d\n", i);*/

  strcpy(nameptr->name, name);
  nameptr->state = STATE_NEW;
  nameptr->seqno = seqno;
  ++seqno;

  nameptr->state = STATE_ASKING;
  nameptr->tmr = 1;
  nameptr->retries = 0;

  struct hostent h;
  struct in_addr *curr;
  u32_t ttl;
  int ret = get_hostinfo(name, &h, &ttl, nameptr->seqno);
  if (ret==0 && (curr = (struct in_addr *)h.h_addr_list[0]))
  {
      nameptr->ipaddr = ntohl(curr->s_addr);
      nameptr->state = STATE_DONE;
      nameptr->ttl = ttl;
      return nameptr->ipaddr;
  }
  else
  {
      nameptr->state = STATE_ERROR;
      return INADDR_NONE;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Look up a hostname in the array of known hostnames.
 *
 * \note This function only looks in the internal array of known
 * hostnames, it does not send out a query for the hostname if none
 * was found. The function resolv_query() can be used to send a query
 * for a hostname.
 *
 * \return A pointer to a 4-byte representation of the hostname's IP
 * address, or NULL if the hostname was not found in the array of
 * hostnames.
 */
/*---------------------------------------------------------------------------*/
u32_t resolv_lookup(char *name)
{
  static u8_t i;
  struct namemap *nameptr;
  
  /* Walk through the list to see if the name is in there. If it is
     not, we return NULL. */
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    nameptr = &names[i];
    if(nameptr->state == STATE_DONE &&
       strlen(name) == strlen(nameptr->name) &&
       strcmp(name, nameptr->name) == 0) {
      return nameptr->ipaddr;
    }
  }
  return INADDR_NONE;
}
/*---------------------------------------------------------------------------*/
/**
 * Initalize the resolver.
 */
/*---------------------------------------------------------------------------*/
void
resolv_init(void)
{
  u8_t i;
  int status;
  
  for(i = 0; i < RESOLV_ENTRIES; ++i) {
    names[i].state = STATE_UNUSED;
  }

  status = NetTimerInstall(resolv_timer_func, NET_TIMER_1_SEC);
  if(status >= 0)
  {
      u08ResolvTimerId = (u8_t)status;
      NetTimerRun(u08ResolvTimerId);
  }
  else
  {
      BREAK_POINT();
  }
}

/*---------------------------------------------------------------------------*/

/** @} */
/** @} */
