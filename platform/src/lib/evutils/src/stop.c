/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "ev.h"
#include "evutils.h"

void
evutil_stop(void)
{
#ifdef LIBEV
    ev_break(EV_DEFAULT_ EVBREAK_ALL);
#endif
}
