/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */
/*
 * This file was originally implemented in platform/src/sim_model_server.
 * It is copied here with slight modifications for athena standalone build.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nic/sdk/third-party/zmq/include/zmq.h"

typedef struct waitcb {
    void (*cbfs)(void *s, void *arg);
    void (*cbfd)(int fd, void *arg);
    void *s;
    int fd;
    void *arg;
} waitcb_t;

typedef struct waitinfo {
    zmq_pollitem_t *items;
    waitcb_t *waitcb;
    int nitems;
    int enabled;
} waitinfo_t;

static waitinfo_t waitinfo;

static int
extend_waitinfo(void)
{
    waitinfo_t *w = &waitinfo;
    zmq_pollitem_t *nitems;
    waitcb_t *nwaitcb;

    /* extend both of these together */
    nitems = (zmq_pollitem_t *)realloc(w->items, (w->nitems + 1) * sizeof (zmq_pollitem_t));
    nwaitcb = (waitcb_t *)realloc(w->waitcb, (w->nitems + 1) * sizeof (waitcb_t));

    if (nitems == NULL || nwaitcb == NULL) {
        if (nitems) free(nitems);
        if (nwaitcb) free(nwaitcb);
        return -1;
    }
    w->items = nitems;
    w->waitcb = nwaitcb;
    w->nitems++;
    return 0;
}

int
zmq_wait_add(void *socket, void (*waitcb)(void *s, void *arg), void *arg)
{
    waitinfo_t *w = &waitinfo;
    zmq_pollitem_t *pi;
    waitcb_t *cb;

    if (extend_waitinfo() < 0) {
        return -1;
    }

    pi = &w->items[w->nitems - 1];
    memset(pi, 0, sizeof(*pi));
    pi->socket = socket;
    pi->events = ZMQ_POLLIN;

    cb = &w->waitcb[w->nitems - 1];
    memset(cb, 0, sizeof(*cb));
    cb->cbfs = waitcb;
    cb->s = socket;
    cb->arg = arg;

    return 0;
}

int
zmq_wait_remove(void *socket)
{
    waitinfo_t *w = &waitinfo;
    zmq_pollitem_t *pi;
    waitcb_t *cb;
    int i;

    pi = w->items;
    cb = w->waitcb;
    for (i = 0; i < w->nitems; i++, pi++, cb++) {
        if (cb->s == socket) {
            cb->cbfs = NULL;
            pi->events = 0;
            // XXX consider shrink_waitinfo() to reclaim unused items
            return 0;
        }
    }
    return -1;
}

int
zmq_wait_add_fd(int fd, void (*waitcb)(int fd, void *arg), void *arg)
{
    waitinfo_t *w = &waitinfo;
    zmq_pollitem_t *pi;
    waitcb_t *cb;

    if (extend_waitinfo() < 0) {
        return -1;
    }

    pi = &w->items[w->nitems - 1];
    memset(pi, 0, sizeof(*pi));
    pi->fd = fd;
    pi->events = ZMQ_POLLIN;

    cb = &w->waitcb[w->nitems - 1];
    memset(cb, 0, sizeof(*cb));
    cb->cbfd = waitcb;
    cb->fd = fd;
    cb->arg = arg;

    return 0;
}

int
zmq_wait_remove_fd(int fd)
{
    waitinfo_t *w = &waitinfo;
    zmq_pollitem_t *pi;
    waitcb_t *cb;
    int i;

    pi = w->items;
    cb = w->waitcb;
    for (i = 0; i < w->nitems; i++, pi++, cb++) {
        if (cb->fd == fd) {
            cb->cbfd = NULL;
            pi->events = 0;
            // XXX consider shrink_waitinfo() to reclaim unused items
            return 0;
        }
    }
    return -1;
}

void 
zmq_wait_loop(void)
{
    waitinfo_t *w = &waitinfo;
    zmq_pollitem_t *pi;
    int i;

    w->enabled = 1;

    while (w->enabled) {

        zmq_poll(w->items, w->nitems, -1);

        for (pi = w->items, i = 0; i < w->nitems; i++, pi++) {
            if (pi->revents & ZMQ_POLLIN) {
                waitcb_t *cb = &w->waitcb[i];

                if (cb->cbfs) cb->cbfs(cb->s, cb->arg);
                else if (cb->cbfd) cb->cbfd(cb->fd, cb->arg);
            }
        }
    }
}

void 
zmq_wait_loop_exit(void)
{
    waitinfo.enabled = 0;
}
