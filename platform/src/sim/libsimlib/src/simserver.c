/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simserver.h"
#include "simlib_impl.h"

typedef struct simserver_s {
    int serverfd;
    msg_handler_t handler;
    msg_handler_t saved_handler;
    simmsg_t *pending_msgs;
    int open:1;
    int sync_writes:1;
    int unix_socket:1;
    char unix_socket_path[265];
} simserver_t;

static simserver_t simserver;

int
sims_memrd(int clientfd, const u_int16_t bdf, 
           const u_int64_t addr, const u_int32_t size, void *buf)
{
    simserver_t *ss = &simserver;
    simmsg_t m = {
        .msgtype = SIMMSG_MEMRD,
        .u.read.bdf = bdf,
        .u.read.addr = addr,
        .u.read.size = size,
    };
    int r;

    r = sims_client_send(clientfd, &m);
    if (r >= 0) {
        r = sim_wait_for_resp(clientfd, SIMMSG_RDRESP, &m, ss->handler);
        if (r >= 0 && m.u.readres.error == 0) {
            r = sim_readn(clientfd, buf, size);
        } else {
            r = -m.u.readres.error;
        }
    }
    return r;
}

int
sims_memwr(int clientfd, const u_int16_t bdf, 
           const u_int64_t addr, const u_int32_t size, const void *buf)
{
    simserver_t *ss = &simserver;
    simmsg_t m = {
        .msgtype = SIMMSG_MEMWR,
        .u.write.bdf = bdf,
        .u.write.addr = addr,
        .u.write.size = size,
    };
    int r;

    r = sims_client_send(clientfd, &m);
    if (r >= 0) {
        r = sim_writen(clientfd, buf, size);
    }
    if (ss->sync_writes) {
        r = sim_wait_for_resp(clientfd, SIMMSG_WRRESP, &m, ss->handler);
    }
    return r;
}

int
sims_readres(int clientfd, u_int16_t bdf, u_int8_t bar,
             u_int64_t addr, u_int8_t size, u_int64_t val, u_int8_t error)
{
    simmsg_t m = {
        .msgtype = SIMMSG_RDRESP,
        .u.readres.bdf = bdf,
        .u.readres.bar = bar,
        .u.readres.addr = addr,
        .u.readres.size = size,
        .u.readres.val = val,
        .u.readres.error = error,
    };

    return sims_client_send(clientfd, &m);
}

int
sims_writeres(int clientfd, u_int16_t bdf, u_int8_t bar,
              u_int64_t addr, u_int8_t size, u_int8_t error)
{
    simmsg_t m = {
        .msgtype = SIMMSG_WRRESP,
        .u.writeres.bdf = bdf,
        .u.writeres.bar = bar,
        .u.writeres.addr = addr,
        .u.writeres.size = size,
        .u.writeres.error = error,
    };

    return sims_client_send(clientfd, &m);
}

static void
sims_pend_msgs(int fd, simmsg_t *m)
{
    simserver_t *ss = &simserver;
    simmsg_t *newm, **pp;

    newm = malloc(sizeof(simmsg_t));
    assert(newm != NULL);
    *newm = *m;
    newm->link = NULL;

    /* append newm to pending_msgs list */
    pp = &ss->pending_msgs;
    while (*pp) {
        pp = &((*pp)->link);
    }
    *pp = newm;
}

int
sims_sync_request(int clientfd)
{
    simserver_t *ss = &simserver;

    assert(ss->pending_msgs == NULL);
    ss->saved_handler = ss->handler;
    ss->handler = sims_pend_msgs;
    return 0;
}

int
sims_sync_release(int clientfd)
{
    simserver_t *ss = &simserver;
    simmsg_t *m;
    int npending;
    static int maxpending;

    /* restore saved handler */
    ss->handler = ss->saved_handler;
    m = ss->pending_msgs;
    if (ss->pending_msgs) {
        ss->pending_msgs = NULL;
    }
    /* call handler for each pending msg */
    npending = 0;
    while (m) {
        simmsg_t *m_to_free;
        npending++;
        ss->handler(clientfd, m);
        m_to_free = m;
        m = m->link;
        free(m_to_free);
    }
    if (npending > maxpending) {
        maxpending = npending;
    }
    return 0;
}

static int
sims_socket(const char *addrstr)
{
    simserver_t *ss = &simserver;
    struct simsockaddr a;
    int s;

    s = sim_socket(addrstr, &a);
    if (s < 0) return -1;

    if (a.sa.sa_family == AF_UNIX) {
        /* pre-emptively remove any stale socket */
        unlink(a.un.sun_path);

        strncpy(ss->unix_socket_path, a.un.sun_path,
                sizeof (ss->unix_socket_path));
        ss->unix_socket = 1;

        /*
         * Reads on unix-domain socket generates SIGPIPE
         * when client exits.
         */
        signal(SIGPIPE, SIG_IGN);
    } else {
        ss->unix_socket = 0;
    }

    if (bind(s, &a.sa, a.sz) < 0) {
        perror(ss->unix_socket_path);
        close(s);
        return -1;
    }

    if (ss->unix_socket) {
        if (getenv("SUDO_USER") != NULL) {
            uid_t uid = strtoul(getenv("SUDO_UID"), NULL, 10);
            gid_t gid = strtoul(getenv("SUDO_GID"), NULL, 10);

            /* give ownership back to original user */
            if (chown(ss->unix_socket_path, uid, gid)) {
                perror(ss->unix_socket_path);
            }
        }
    }

    listen(s, 5);
    return s;
}

int
sims_open(const char *addrstr, msg_handler_t handler)
{
    int s;

    s = sims_socket(addrstr);
    if (s < 0) return -1;

    simserver.serverfd = s;
    simserver.open = 1;
    simserver.sync_writes = 1;
    simserver.handler = handler;
    return s;
}

void
sims_close(int serverfd)
{
    if (simserver.open) {
        close(serverfd);
        if (simserver.unix_socket) {
            unlink(simserver.unix_socket_path);
            simserver.unix_socket = 0;
        }
        simserver.open = 0;
    }
}

int
sims_open_client(int serverfd)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int clientfd;

    if (!simserver.open) return -1;
    clientfd = accept(serverfd, (struct sockaddr *)&addr, &addrlen);
    return clientfd;
}

void
sims_close_client(int clientfd)
{
    close(clientfd);
}

int
sims_client_send(int clientfd, simmsg_t *m)
{
    return sim_writen(clientfd, m, sizeof(*m));
}

int
sims_client_recv(int clientfd, simmsg_t *m)
{
    return sim_readn(clientfd, m, sizeof(*m));
}

int
sims_client_recv_and_handle(int clientfd)
{
    simmsg_t m;
    int n;

    if ((n = sims_client_recv(clientfd, &m)) < 0) {
        return -1;
    }
    if (n > 0 && simserver.handler) {
        simserver.handler(clientfd, &m);
    }
    return n;
}
