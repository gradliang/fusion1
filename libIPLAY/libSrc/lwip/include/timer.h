/**
 * \defgroup timer Timer library
 *
 * The timer library provides functions for setting, resetting and
 * restarting timers, and for checking if a timer has expired. An
 * application must "manually" check if its timers have expired; this
 * is not done automatically.
 *
 * A timer is declared as a \c struct \c timer and all access to the
 * timer is made by a pointer to the declared timer.
 *
 * \note The timer library uses the \ref clock "Clock library" to
 * measure time. Intervals should be specified in the format used by
 * the clock library.
 *
 * @{
 */


/**
 * \file
 * Timer library header file.
 * \author
 * Adam Dunkels <adam@sics.se>
 */


#ifndef __TIMER_H__
#define __TIMER_H__

#include "clock.h"

/**
 * A timer.
 *
 * This structure is used for declaring a timer. The timer must be set
 * with timer_set() before it can be used.
 *
 * \hideinitializer
 */
typedef struct {
  clock_time_t start;
  clock_time_t interval;
} timer;

void timer_set(timer *t, clock_time_t interval);
void timer_reset(timer *t);
void timer_restart(timer *t);
int timer_expired(timer *t);

typedef struct _wifi_timer {
  unsigned long expires;
  unsigned long max_interval;
}wifi_timer_t;

#endif /* __TIMER_H__ */

/** @} */
