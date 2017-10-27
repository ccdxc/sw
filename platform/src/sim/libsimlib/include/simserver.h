/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __SIMSERVER_H__
#define __SIMSERVER_H__

#include "simmsg.h"

int sims_open(const char *addr, msg_handler_t handler);
void sims_close(int s);
int sims_open_client(int serverfd);
void sims_close_client(int clientfd);
int sims_client_send(int clientfd, simmsg_t *m);
int sims_client_recv(int clientfd, simmsg_t *m);
int sims_client_recv_and_handle(int clientfd);
int sims_memrd(int clientfd, const u_int16_t bdf, 
               const u_int64_t addr, const u_int32_t size, void *buf);
int sims_memwr(int clientfd, const u_int16_t bdf,
               const u_int64_t addr, const u_int32_t size, const void *buf);
int sims_readres(int clientfd, u_int16_t bdf, u_int8_t bar,
                 u_int64_t addr, u_int8_t size, 
                 u_int64_t val, u_int8_t error);

#endif /* __SIMSERVER_H__ */
