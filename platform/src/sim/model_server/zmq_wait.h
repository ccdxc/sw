/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __ZMQ_WAIT_H__
#define __ZMQ_WAIT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

int zmq_wait_add(void *s, void (*waitcb)(void *s, void *arg), void *arg);
int zmq_wait_remove(void *s);
int zmq_wait_add_fd(int fd, void (*waitcb)(int fd, void *arg), void *arg);
int zmq_wait_remove_fd(int fd);
void zmq_wait_loop(void);
void zmq_wait_loop_exit(void);

#ifdef __cplusplus
}
#endif

#endif // __ZMQ_WAIT_H__
