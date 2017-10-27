/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simclient.h"
#include "simlib_impl.h"

typedef struct simclient_s {
    int s;
    int open:1;
    msg_handler_t handler;
} simclient_t;

static simclient_t simclient;

static int
simc_do_write(simmsgtype_t msgtype,
              u_int16_t bdf, u_int8_t bar,
              u_int64_t addr, u_int8_t size, u_int64_t val)
{
    int s = simclient.s;

    if (!simclient.open) return -EBADF;

    return sim_do_write(s, msgtype, bdf, bar, addr, size, val);
}

static int
simc_do_read(simmsgtype_t msgtype,
             u_int16_t bdf, u_int8_t bar,
             u_int64_t addr, u_int8_t size, u_int64_t *val)
{
    simclient_t *sc = &simclient;
    int s = sc->s;
    int r;

    if (!sc->open) return -EBADF;

    r = sim_do_read(s, msgtype, bdf, bar, addr, size, val, sc->handler);
    return r;
}

static int
simc_socket(const char *addrstr)
{
    struct simsockaddr a;
    int s;

    s = sim_socket(addrstr, &a);
    while (s >= 0 && connect(s, &a.sa, a.sz) == -1 && errno != EISCONN) {
        /* retry connect if signal interrupted us */
        if (errno == EINTR) continue;
        close(s);
        s = -1;
    }
    return s;
}

int
simc_open(const char *myname, const char *addrstr, msg_handler_t handler)
{
    simmsg_t m = {
        .msgtype = SIMMSG_INIT,
    };
    int s = simc_socket(addrstr);
    if (s >= 0) {
        simclient.open = 1;
        simclient.s = s;
        simclient.handler = handler;
        strncpy(m.u.init.name, myname, sizeof(m.u.init.name) - 1);
        sim_writen(s, &m, sizeof(m));
    }
    return s;
}

void
simc_close(void)
{
    int s = simclient.s;
    close(s);
    simclient.s = -1;
    simclient.handler = NULL;
    simclient.open = 0;
}

int
simc_cfgrd(u_int16_t bdf, u_int16_t addr, u_int8_t size, u_int64_t *val)
{
    return simc_do_read(SIMMSG_CFGRD, bdf, 0, addr, size, val);
}

int
simc_cfgwr(u_int16_t bdf, u_int16_t addr, u_int8_t size, u_int64_t val)
{
    return simc_do_write(SIMMSG_CFGWR, bdf, 0, addr, size, val);
}

int
simc_memrd(u_int16_t bdf, u_int8_t bar,
           u_int64_t addr, u_int8_t size, u_int64_t *val)
{
    return simc_do_read(SIMMSG_MEMRD, bdf, bar, addr, size, val);
}

int
simc_memwr(u_int16_t bdf, u_int8_t bar,
           u_int64_t addr, u_int8_t size, u_int64_t val)
{
    return simc_do_write(SIMMSG_MEMWR, bdf, bar, addr, size, val);
}

int
simc_iord(u_int16_t bdf, u_int8_t bar, 
          u_int16_t addr, u_int8_t size, u_int64_t *val)
{
    return simc_do_read(SIMMSG_IORD, bdf, bar, addr, size, val);
}

int
simc_iowr(u_int16_t bdf, u_int8_t bar,
          u_int16_t addr, u_int8_t size, u_int64_t val)
{
    return simc_do_write(SIMMSG_IOWR, bdf, bar, addr, size, val);
}

int
simc_readres(u_int16_t bdf,
             u_int64_t addr, u_int32_t size, void *buf, u_int8_t error)
{
    int s = simclient.s;
    simmsg_t m = {
        .msgtype = SIMMSG_RDRESP,
        .u.readres.bdf = bdf,
        .u.readres.addr = addr,
        .u.readres.size = size,
        .u.readres.error = error,
    };
    int r;

    if (!simclient.open) return -EBADF;

    r = sim_writen(s, &m, sizeof(m));
    if (r >= 0 && error == 0) {
        r = sim_writen(s, buf, size);
    }
    return r;
}

int
simc_readn(void *buf, size_t size)
{
    if (!simclient.open) return -EBADF;

    return sim_readn(simclient.s, buf, size);
}

void
simc_discard(size_t size)
{
    sim_discard(simclient.s, size);
}

int
simc_recv(simmsg_t *m)
{
    return simc_readn(m, sizeof(*m));
}

int
simc_recv_and_handle(void)
{
    simmsg_t m;
    int n;

    if ((n = simc_recv(&m)) > 0) {
        if (simclient.handler) {
            simclient.handler(simclient.s, &m);
        }
    }
    return n;
}
