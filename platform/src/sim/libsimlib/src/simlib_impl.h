/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __SIMLIB_IMPL_H__
#define __SIMLIB_IMPL_H__

#include "simmsg.h"

#define SIM_DEFAULT_PORT        50000

ssize_t sim_readn(int fd, void *bufarg, size_t n);
ssize_t sim_writen(int fd, const void *bufarg, size_t n);
int sim_do_read(int s, simmsgtype_t msgtype, u_int16_t bdf, u_int8_t bar,
                u_int64_t addr, u_int32_t size, u_int64_t *val,
                msg_handler_t msg_handler);
int sim_do_write(int s, simmsgtype_t msgtype, u_int16_t bdf, u_int8_t bar,
                 u_int64_t addr, u_int32_t size, u_int64_t val);
void sim_discard(int s, size_t size);

int sim_wait_for_resp(int s, simmsgtype_t msgtype, simmsg_t *m,
                      msg_handler_t msg_handler);

struct simsockaddr {
    union {
        struct sockaddr    sa;
        struct sockaddr_in in;
        struct sockaddr_un un;
    };
    size_t sz;
};

int sim_socket(const char *addrstr, struct simsockaddr *a);

#endif /* __SIMLIB_IMPL_H__ */
