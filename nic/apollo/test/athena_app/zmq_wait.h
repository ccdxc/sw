/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */
/*
 * This file was originally implemented in platform/src/sim_model_server.
 * It is copied here with slight modifications for athena standalone build.
 */
 
#ifndef __ZMQ_WAIT_H__
#define __ZMQ_WAIT_H__

int zmq_wait_add(void *s, void (*waitcb)(void *s, void *arg), void *arg);
int zmq_wait_remove(void *s);
int zmq_wait_add_fd(int fd, void (*waitcb)(int fd, void *arg), void *arg);
int zmq_wait_remove_fd(int fd);
void zmq_wait_loop(void);
void zmq_wait_loop_exit(void);

#endif // __ZMQ_WAIT_H__
