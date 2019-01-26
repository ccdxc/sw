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

void evutil_run(void);
void evutil_stop(void);

/*
 * I/O: Runs after prepare handlers
 */
void evutil_add_fd(int fd, evutil_cb_t *rdcb, evutil_cb_t *wrcb, void *cbarg);
void evutil_remove_fd(const int fd);

/*
 * Relative timers: Runs after I/O handlers
 */
typedef struct {
    ev_timer ev_timer;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_timer;

void evutil_timer_start(evutil_timer *evu_timer,
                        evutil_cb_t *cb, void *arg,
                        ev_tstamp after, ev_tstamp repeat);
void evutil_timer_stop(evutil_timer *evu_timer);

/*
 * Idle polling: Only runs when no-other higher priority watches are pending.
 */
typedef struct {
    ev_idle ev_idle;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_idle;

void evutil_add_idle(evutil_idle *evu_idle,
                     evutil_cb_t *cb, void *arg);
void evutil_remove_idle(evutil_idle *evu_idle);

/*
 * Prepare Polling: Runs after fork handlers
 */
typedef struct {
    ev_prepare ev_prepare;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_prepare;

void evutil_add_prepare(evutil_prepare *evu_prepare,
                     evutil_cb_t *cb, void *arg);
void evutil_remove_prepare(evutil_prepare *evu_prepare);

/*
 * Check Polling: Runs after idle handlers
 */
typedef struct {
    ev_check ev_check;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_check;

void evutil_add_check(evutil_check *evu_check,
                     evutil_cb_t *cb, void *arg);
void evutil_remove_check(evutil_check *evu_check);

#ifdef __cplusplus
}
#endif

#endif /* __EVUTILS_H__ */
