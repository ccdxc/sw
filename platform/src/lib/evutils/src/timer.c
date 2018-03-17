/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "ev.h"
#include "evutils.h"

static void
evutil_timercb(EV_P_ ev_timer *w, int revents)
{
    evutil_timer *evu_timer = (evutil_timer *)w;

    evu_timer->cb(evu_timer->cbarg);
}

void
evutil_timer_start(evutil_timer *evu_timer, 
                   evutil_cb_t *cb, void *arg,
                   ev_tstamp after, ev_tstamp repeat)
{
    evu_timer->cb = cb;
    evu_timer->cbarg = arg;
#ifdef LIBEV
    ev_timer_init(&evu_timer->ev_timer, evutil_timercb, after, repeat);
    ev_timer_start(EV_DEFAULT_ &evu_timer->ev_timer);
#else
    if (0) evutil_timercb(EV_DEFAULT_ &evu_timer->ev_timer, 0);
#endif
}

void
evutil_timer_stop(evutil_timer *evu_timer)
{
#ifdef LIBEV
    ev_timer_stop(EV_DEFAULT_ &evu_timer->ev_timer);
#endif
}
