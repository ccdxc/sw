/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __EVUTILS_H__
#define __EVUTILS_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include "ev.h"

/*
 * evutils provides a thin layer around libev.  evutils uses the default
 * libev event loop EV_DEFAULT to process file descriptor and timer events.
 *
 * Typical usage is like this:
 *
 *     void
 *     my_socket_msg_cb(void *arg)
 *     {
 *         my_socket_ctx_t *my_socket_ctx = arg;
 *         process_my_socket_msg(my_socket_ctx);
 *         if (done) evutil_stop();
 *     }
 *
 *     int
 *     main(int argc, char *argv[])
 *     {
 *         int fd;
 *         my_socket_ctx_t my_socket_ctx;
 *
 *         fd = open_my_socket();
 *         my_socket_ctx.fd = fd;
 *         evutil_add_fd(fd, my_socket_msg_cb, NULL, &my_socket_ctx);
 *         ...
 *         evutil_run();
 *         exit(0);
 *     }
 *
 */

typedef void (evutil_cb_t)(void *cbarg);

void evutil_add_fd(int fd, evutil_cb_t *rdcb, evutil_cb_t *wrcb, void *cbarg);
void evutil_remove_fd(const int fd);
void evutil_run(void);
void evutil_stop(void);

typedef struct {
    ev_timer ev_timer;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_timer;

void evutil_timer_start(evutil_timer *evu_timer, 
                        evutil_cb_t *cb, void *arg,
                        ev_tstamp after, ev_tstamp repeat);
void evutil_timer_stop(evutil_timer *evu_timer);

#ifdef __cplusplus
}
#endif

#endif /* __EVUTILS_H__ */
