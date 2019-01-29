/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "ev.h"
#include "evutils.h"

typedef struct evutil_fd_ctx_s {
    ev_io evio;                 /* must be first item in this struct */
    evutil_cb_t *rdcb;          /* read callback, if interested in reads */
    evutil_cb_t *wrcb;          /* write callback, if interested in writes */
    void *cbarg;                /* callback arg */
} evutil_fd_ctx_t;

#define EVUTIL_NFDS     256

static evutil_fd_ctx_t evutil_fd_ctx[EVUTIL_NFDS];

static void
evutil_fd_cb(EV_P_ ev_io *w, int revents)
{
    evutil_fd_ctx_t *fdctx = (evutil_fd_ctx_t *)w;
    evutil_cb_t *rdcb = fdctx->rdcb;
    evutil_cb_t *wrcb = fdctx->wrcb;

    if ((revents & EV_READ) && rdcb) {
        rdcb(fdctx->cbarg);
    }
    if ((revents & EV_WRITE) && wrcb) {
        wrcb(fdctx->cbarg);
    }
}

void
evutil_add_fd(int fd, evutil_cb_t *rdcb, evutil_cb_t *wrcb, void *cbarg)
{
    evutil_fd_ctx_t *fdctx;
    int events;

    assert(fd >= 0 && fd < EVUTIL_NFDS);
    fdctx = &evutil_fd_ctx[fd];
    fdctx->rdcb = rdcb;
    fdctx->wrcb = wrcb;
    fdctx->cbarg = cbarg;

    events = 0;
    if (fdctx->rdcb) events |= EV_READ;
    if (fdctx->wrcb) events |= EV_WRITE;
#ifdef LIBEV
    ev_io_init(&fdctx->evio, evutil_fd_cb, fd, events);
    ev_io_start(EV_DEFAULT_ &fdctx->evio);
#else
    if (0) evutil_fd_cb(EV_DEFAULT_ &fdctx->evio, 0);
#endif
}

void
evutil_remove_fd(const int fd)
{
    evutil_fd_ctx_t *fdctx;

    assert(fd >= 0 && fd < EVUTIL_NFDS);
    fdctx = &evutil_fd_ctx[fd];
#ifdef LIBEV
    ev_io_stop(EV_DEFAULT_ &fdctx->evio);
#endif
    fdctx->rdcb = fdctx->wrcb = NULL;
}
