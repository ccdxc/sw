/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "ev.h"
#include "evutils.h"

struct ev_loop *
evutil_create_loop(void)
{
#ifdef LIBEV
    return ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
#endif
}
