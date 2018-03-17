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
evutil_run(void)
{
#ifdef LIBEV
    ev_run(EV_DEFAULT_ 0);
#endif
}
