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
stat_file_cb (EV_P_ ev_stat *w, int revents)
{
    evutil_stat *evu_stat = (evutil_stat*)w;
    /* file evu_stat->file changed in some way */
    if (evu_stat->ev_stat.attr.st_nlink)
        evu_stat->create_modify_cb(evu_stat->create_modify_cbarg);
    else
        evu_stat->delete_cb(evu_stat->delete_cbarg);
}

void
evutil_stat_start(EV_P_ evutil_stat* evu_stat, 
                  char* file, 
                  evutil_cb_t *create_modify_cb, 
                  void *create_modify_cbarg, 
                  evutil_cb_t *delete_cb, 
                  void *delete_cbarg)
{
    evu_stat->file = file;
    evu_stat->create_modify_cb = create_modify_cb;
    evu_stat->create_modify_cbarg = create_modify_cbarg;
    evu_stat->delete_cb = delete_cb;
    evu_stat->delete_cbarg = delete_cbarg;
#ifdef LIBEV
    ev_stat_init (&evu_stat->ev_stat, stat_file_cb, file, 0.);
    ev_stat_start (EV_A_ &evu_stat->ev_stat);
#endif
}
