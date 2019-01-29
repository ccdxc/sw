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
evutil_preparecb(EV_P_ ev_prepare *w, int revents)
{
    evutil_prepare *evu_prepare = (evutil_prepare *)w;

    evu_prepare->cb(evu_prepare->cbarg);
}

void
evutil_add_prepare(evutil_prepare *evu_prepare,
                evutil_cb_t *cb, void *arg)
{
    evu_prepare->cb = cb;
    evu_prepare->cbarg = arg;
#ifdef LIBEV
    ev_prepare_init(&evu_prepare->ev_prepare, evutil_preparecb);
    ev_prepare_start(EV_DEFAULT_ &evu_prepare->ev_prepare);
#else
    if (0) evutil_preparecb(EV_DEFAULT_ &evu_prepare->ev_prepare, 0);
#endif
}

void
evutils_remove_prepare(evutil_prepare *evu_prepare)
{
#ifdef LIBEV
    ev_prepare_stop(EV_DEFAULT_ &evu_prepare->ev_prepare);
#endif
}
