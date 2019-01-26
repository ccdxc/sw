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
evutil_checkcb(EV_P_ ev_check *w, int revents)
{
    evutil_check *evu_check = (evutil_check *)w;

    evu_check->cb(evu_check->cbarg);
}

void
evutil_add_check(evutil_check *evu_check,
                evutil_cb_t *cb, void *arg)
{
    evu_check->cb = cb;
    evu_check->cbarg = arg;
#ifdef LIBEV
    ev_check_init(&evu_check->ev_check, evutil_checkcb);
    ev_check_start(EV_DEFAULT_ &evu_check->ev_check);
#else
    if (0) evutil_checkcb(EV_DEFAULT_ &evu_check->ev_check, 0);
#endif
}

void
evutil_remove_check(evutil_check *evu_check)
{
#ifdef LIBEV
    ev_check_stop(EV_DEFAULT_ &evu_check->ev_check);
#endif
}
