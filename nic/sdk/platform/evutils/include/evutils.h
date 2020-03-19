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

struct ev_loop *evutil_create_loop(void);

void evutil_run(EV_P);
void evutil_stop(EV_P);

/*
 * I/O: Runs after prepare handlers
 */
void evutil_add_fd(EV_P_ int fd, evutil_cb_t *rdcb, evutil_cb_t *wrcb, void *cbarg);
void evutil_remove_fd(EV_P_ const int fd);

struct pal_int;
void evutil_add_pal_int(EV_P_ struct pal_int *pal_int,
                        evutil_cb_t *isrcb, void *cbarg);
void evutil_remove_pal_int(EV_P_ struct pal_int *pal_int);

/*
 * Relative timers: Runs after I/O handlers
 */
typedef struct {
    ev_timer ev_timer;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_timer;

void evutil_timer_start(EV_P_ evutil_timer *evu_timer,
                        evutil_cb_t *cb, void *arg,
                        ev_tstamp after, ev_tstamp repeat);
void evutil_timer_stop(EV_P_ evutil_timer *evu_timer);
void evutil_timer_again(EV_P_ evutil_timer *evu_timer);

/*
 * Idle polling: Only runs when no-other higher priority watches are pending.
 */
typedef struct {
    ev_idle ev_idle;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_idle;

void evutil_add_idle(EV_P_ evutil_idle *evu_idle,
                     evutil_cb_t *cb, void *arg);
void evutil_remove_idle(EV_P_ evutil_idle *evu_idle);

/*
 * Prepare Polling: Runs after fork handlers
 */
typedef struct {
    ev_prepare ev_prepare;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_prepare;

void evutil_add_prepare(EV_P_ evutil_prepare *evu_prepare,
                        evutil_cb_t *cb, void *arg);
void evutil_remove_prepare(EV_P_ evutil_prepare *evu_prepare);

/*
 * Check Polling: Runs after idle handlers
 */
typedef struct {
    ev_check ev_check;
    evutil_cb_t *cb;
    void *cbarg;
} evutil_check;

void evutil_add_check(EV_P_ evutil_check *evu_check,
                     evutil_cb_t *cb, void *arg);
void evutil_remove_check(EV_P_ evutil_check *evu_check);

/*
 * Stat polling: Runs as soon as a file is touched
 */
typedef struct {
    ev_stat ev_stat;
    char* file;
    evutil_cb_t *create_modify_cb;
    void *create_modify_cbarg;
    evutil_cb_t *delete_cb;
    void *delete_cbarg;
} evutil_stat;

void
evutil_stat_start(EV_P_ evutil_stat* evu_stat,
                  char* file,
                  evutil_cb_t *create_modify_cb,
                  void *create_modify_cbarg,
                  evutil_cb_t *delete_cb,
                  void *delete_cbarg);

typedef void (evutil_system_cb_t)(pid_t pid, int status, void *cbarg);

pid_t evutil_system(EV_P_ const char *cmd,
                    evutil_system_cb_t *cb, void *cbarg);

#ifdef __cplusplus
}
#endif

#endif /* __EVUTILS_H__ */
