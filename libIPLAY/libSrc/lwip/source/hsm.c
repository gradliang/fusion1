/** hsm.c -- Hierarchical State Machine implementation
 * M. Samek, 01/07/00
 */
#define LOCAL_DEBUG_ENABLE 0
#include "hsm.h"

//#define EMBEDDED_OS	1

#define MP_DEBUG(...)                  do { } while (0)

#ifdef EMBEDDED_OS
#include "eth_debug.h"
#endif /* EMBEDDED_OS */

#define eprintf mpDebugPrint
#define printf mpDebugPrint

                  /* state machine insturmentation macros */ 
#define HSM_INSTR
#ifndef HSM_INSTR
#define ON_ENTRY(ctx_, state_) ((void)0)
#define ON_EXIT(ctx_, state_)  ((void)0)
#else
#define ON_ENTRY(ctx_, state_) \
        printf("%s::%s-ENTRY;", (ctx_), (state_))
#define ON_EXIT(ctx_, state_)  \
        printf("%s::%s-EXIT;",  (ctx_), (state_))
#endif

static Msg const startMsg = { START_EVT };
static Msg const entryMsg = { ENTRY_EVT };
static Msg const exitMsg  = { EXIT_EVT };
#define MAX_STATE_NESTING 8
                                            /* State ctor */
void StateCtor(State *me, char const *name, 
               State *super, EvtHndlr hndlr) {
  me->name  = name;
  me->super = super;
  me->hndlr = hndlr;
}
                                              /* Hsm ctor */
void HsmCtor(Hsm *me, char const *name, EvtHndlr topHndlr) {
  StateCtor(&me->top, "top", 0, topHndlr);
  me->name = name;
}
                         /* enter and start the top state */
void HsmOnStart(Hsm *me) {
  State *entryPath[MAX_STATE_NESTING];
  register State **trace;
  register State *s;
  me->curr = &me->top;
  me->next = 0;
  ON_ENTRY(me->name, me->curr->name);
  StateOnEvent(me->curr, me, &entryMsg);
  while (StateOnEvent(me->curr, me, &startMsg), me->next) {
    for (s = me->next, trace = entryPath, *trace = 0; 
         s != me->curr; s = s->super)
      *(++trace) = s;             /* trace path to target */
    while (s = *trace--) {   /* retrace entry from source */
      ON_ENTRY(me->name, s->name);
      StateOnEvent(s, me, &entryMsg);
    }
    me->curr = me->next;
    me->next = 0;
  }
  eprintf("HsmOnStart:: exit\n");
}
                                /* state machine "engine" */
void HsmOnEvent(Hsm *me, Msg const *msg) 
{
  State *entryPath[MAX_STATE_NESTING];
  register State **trace;
  register State *s;

  eprintf("HsmOnEvent:: Enter (evt=%d)\n", msg->evt);
  for (s = me->curr; s; s = s->super) {
    if ((msg = StateOnEvent(s, me, msg)) == 0) {
		MP_DEBUG("trace 1");
      if (me->next) {          /* state transition taken? */
        for (s = me->next, trace = entryPath, *trace = 0; 
             s != me->curr; s = s->super)
		{
			MP_DEBUG("trace 1.1");
          *(++trace) = s;         /* trace path to target */
		}
		MP_DEBUG("trace 2");
        while (s = *trace--) {  /* retrace entry from LCA */
          ON_ENTRY(me->name, s->name);
          StateOnEvent(s, me, &entryMsg);
        }
		MP_DEBUG("trace 3");
        me->curr = me->next;
        me->next = 0;
        while (StateOnEvent(me->curr, me, &startMsg), 
               me->next) {
			MP_DEBUG("trace 4");
          for (s = me->next, trace = entryPath, 
               *trace = 0; s != me->curr; s = s->super)
            *(++trace) = s;      /* record path to target */
          while (s = *trace--) {    /* retrace  the entry */
            ON_ENTRY(me->name, s->name);
            StateOnEvent(s, me, &entryMsg);
          }
          me->curr = me->next;
          me->next = 0;
        }
      }
      break;                           /* event processed */
    }
//BREAK_POINT();
  }
  eprintf("HsmOnEvent:: Exit\n");
}
     /* exit current states and all superstates up to LCA */
void HsmExit_(Hsm *me, unsigned char toLca) {
  register State *s;
  for (s = me->curr; toLca > 0; --toLca, s = s->super) {
    ON_EXIT(me->name, s->name);
    StateOnEvent(s, me, &exitMsg);
  }
  me->curr = s;
}
             /* find # of levels to Least Common Ancestor */
unsigned char HsmToLCA_(Hsm *me, State *target) {
  State *s, *t;
  unsigned char toLca = 1;
  for (s = me->curr->super; s != 0; ++toLca, s = s->super)
    for (t = target; t != 0; t = t->super)
      if (s == t)
        return toLca;
  return 0;
}
