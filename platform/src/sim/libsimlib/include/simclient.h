/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __SIMCLIENT_H__
#define __SIMCLIENT_H__

#include "simmsg.h"

int simc_open(const char *myname, const char *addrstr,
              msg_handler_t handler);
void simc_close(void);

int simc_cfgrd(u_int16_t bdf, u_int16_t addr, u_int8_t size, u_int64_t *val);
int simc_cfgwr(u_int16_t bdf, u_int16_t addr, u_int8_t size, u_int64_t val);

int simc_memrd(u_int16_t bdf, u_int8_t bar,
               u_int64_t addr, u_int8_t size, u_int64_t *val);
int simc_memwr(u_int16_t bdf, u_int8_t bar,
               u_int64_t addr, u_int8_t size, u_int64_t val);

int simc_iord(u_int16_t bdf, u_int8_t bar, 
              u_int16_t addr, u_int8_t size, u_int64_t *val);
int simc_iowr(u_int16_t bdf, u_int8_t bar, 
              u_int16_t addr, u_int8_t size, u_int64_t val);

int simc_readres(u_int16_t bdf,
                 u_int64_t addr, u_int32_t size, void *buf, u_int8_t error);
int simc_writeres(u_int16_t bdf,
                  u_int64_t addr, u_int32_t size, u_int8_t error);

int simc_recv(simmsg_t *m);
int simc_recv_and_handle(void);
int simc_readn(void *buf, size_t size);
void simc_discard(size_t size);
int simc_sync_ack(void);

#endif /* __SIMCLIENT_H__ */
