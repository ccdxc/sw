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
evutil_idlecb(EV_P_ ev_idle *w, int revents)
{
    evutil_idle *evu_idle = (evutil_idle *)w;

    evu_idle->cb(evu_idle->cbarg);
}

void
evutil_add_idle(evutil_idle *evu_idle,
                evutil_cb_t *cb, void *arg)
{
    evu_idle->cb = cb;
    evu_idle->cbarg = arg;
#ifdef LIBEV
    ev_idle_init(&evu_idle->ev_idle, evutil_idlecb);
    ev_idle_start(EV_DEFAULT_ &evu_idle->ev_idle);
#else
    if (0) evutil_idlecb(EV_DEFAULT_ &evu_idle->ev_idle, 0);
#endif
}

void
evutils_remove_idle(evutil_idle *evu_idle)
{
#ifdef LIBEV
    ev_idle_stop(EV_DEFAULT_ &evu_idle->ev_idle);
#endif
}
