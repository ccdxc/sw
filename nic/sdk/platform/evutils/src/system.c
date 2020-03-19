/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>

#include "ev.h"
#include "evutils.h"

typedef struct evutil_child_ctx_s {
    ev_child evcw;              /* must be first item in this struct */
    evutil_system_cb_t *cb;     /* callback, if interested in exit status */
    void *cbarg;                /* callback arg */
} evutil_child_ctx_t;

static void
child_cb(EV_P_ ev_child *w, int revents)
{
    evutil_child_ctx_t *cctx = (evutil_child_ctx_t *)w;

    ev_child_stop(EV_A_ w);
    if (cctx->cb) {
        cctx->cb(w->rpid, w->rstatus, cctx->cbarg);
    }
    free(cctx);
}

pid_t
evutil_system(EV_P_ const char *cmd, evutil_system_cb_t *cb, void *cbarg)
{
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        /* fork failed */
        return -1;
    }
    if (pid == 0) {
        /* child */
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(127);
        /* NOTREACHED */
    }
    /* parent */
    if (cb) {
        evutil_child_ctx_t *cctx;

        cctx = malloc(sizeof(*cctx));
        assert(cctx != NULL);

        cctx->cb = cb;
        cctx->cbarg = cbarg;
        ev_child_init(&cctx->evcw, child_cb, pid, 0);
        ev_child_start(EV_A_ &cctx->evcw);
    }
    return pid;
}
