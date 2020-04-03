/*
 * Copyright (c) 2018, Pensando Systems Inc.
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
#include <sys/queue.h>

#include "platform/evutils/include/evutils.h"

#include "pmserver.h"
#include "pciesvc_impl.h"

#define NCLIENTS 100

typedef struct pmsclient_s {
    int fd;
    struct sockaddr_in addr;
    socklen_t addrlen;
    unsigned int connected:1;
    unsigned int receiver:1;
    TAILQ_ENTRY(pmsclient_s) list;
} pmsclient_t;

typedef struct pmserver_s {
    int fd;
    pciemgrs_handler_t *handler;
    unsigned int init:1;
    unsigned int open:1;
    unsigned int sync_writes:1;
    unsigned int unix_socket:1;
    char unix_socket_path[265];
    pmsclient_t clients[NCLIENTS];
    TAILQ_HEAD(pmsclient_head, pmsclient_s) receivers;
} pmserver_t;

static pmserver_t pmserver;

static int
pciemgrs_socket(const char *addrstr)
{
    pmserver_t *pms = &pmserver;
    struct pmsockaddr a;
    int s;

    s = pmsocket(addrstr, &a);
    if (s < 0) return -1;

    if (a.sa.sa_family == AF_UNIX) {
        /* pre-emptively remove any stale socket */
        unlink(a.un.sun_path);

        strncpy(pms->unix_socket_path, a.un.sun_path,
                sizeof (pms->unix_socket_path));
        pms->unix_socket = 1;

        /*
         * Reads on unix-domain socket generates SIGPIPE
         * when client exits.
         */
        signal(SIGPIPE, SIG_IGN);
    } else {
        pms->unix_socket = 0;
    }

    if (bind(s, &a.sa, a.sz) < 0) {
        fprintf(stderr, "pciemgrs_socket: bind %s: %s\n",
                pms->unix_socket_path, strerror(errno));
        close(s);
        return -1;
    }

    if (pms->unix_socket) {
        if (getenv("SUDO_USER") != NULL) {
            uid_t uid = strtoul(getenv("SUDO_UID"), NULL, 10);
            gid_t gid = strtoul(getenv("SUDO_GID"), NULL, 10);

            /* give ownership back to original user */
            if (chown(pms->unix_socket_path, uid, gid)) {
                fprintf(stderr, "pciemgrs_socket: chown %s: %s\n",
                        pms->unix_socket_path, strerror(errno));
                /* continue anyway */
            }
        }
    }

    listen(s, 5);
    return s;
}

static void
client_disconnect(pmsclient_t *pmsc)
{
    evutil_remove_fd(EV_DEFAULT_ pmsc->fd);
    close(pmsc->fd);
    pmsc->fd = -1;
    pmsc->connected = 0;
    if (pmsc->receiver) {
        pmserver_t *pms = &pmserver;
        TAILQ_REMOVE(&pms->receivers, pmsc, list);
        pmsc->receiver = 0;
    }
}

static void
client_msg_cb(void *arg)
{
    pmserver_t *pms = &pmserver;
    pmsclient_t *pmsc = arg;
    pmmsg_t *m;
    int r;

    r = pciemgr_msgrecv(pmsc->fd, &m);
    if (r <= 0) {
        if (r < 0) {
            perror("client_msg_cb: pciemgr_msgrecv");
        }
        client_disconnect(pmsc);
        return;
    }

    /* in case handler needs to know client context */
    m->hdr.tag = pmsc;
    if (pms->handler) pms->handler(m);
    else pciemgr_msgfree(m);
}

static void
client_connect_cb(void *arg)
{
    pmserver_t *pms = arg;
    pmsclient_t *pmsc;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int fd;

    if (!pms->open) return;
    fd = accept(pms->fd, (struct sockaddr *)&addr, &addrlen);
    if (fd < 0) {
        perror("client_connect_cb: accept");
        return;
    }
    assert(fd < NCLIENTS);
    pmsc = &pms->clients[fd];
    pmsc->fd = fd;
    pmsc->addr = addr;
    pmsc->addrlen = addrlen;
    pmsc->connected = 1;
    evutil_add_fd(EV_DEFAULT_ fd, client_msg_cb, NULL, pmsc);
}

int
pciemgrs_open(const char *addrstr, pciemgrs_handler_t *handler)
{
    pmserver_t *pms = &pmserver;
    int fd;

    if (!pms->init) {
        TAILQ_INIT(&pms->receivers);
        pms->init = 1;
    }

    fd = pciemgrs_socket(addrstr);
    if (fd < 0) return -1;

    pms->fd = fd;
    pms->handler = handler;
    pms->open = 1;
    pms->sync_writes = 1;
    evutil_add_fd(EV_DEFAULT_ fd, client_connect_cb, NULL, pms);
    return fd;
}

void
pciemgrs_close(void)
{
    pmserver_t *pms = &pmserver;

    if (pms->open) {
        close(pms->fd);
        if (pms->unix_socket) {
            unlink(pms->unix_socket_path);
            pms->unix_socket = 0;
        }
        pms->open = 0;
        pms->fd = -1;
    }
}

int
pciemgrs_add_receiver(pmmsg_t *m)
{
    pmserver_t *pms = &pmserver;
    pmsclient_t *pmsc = m->hdr.tag;

    if (pmsc) {
        pmsc->receiver = 1;
        TAILQ_INSERT_TAIL(&pms->receivers, pmsc, list);
    }
    return 0;
}

int
pciemgrs_msgalloc(pmmsg_t **m, size_t len)
{
    return pciemgr_msgalloc(m, len);
}

void
pciemgrs_msgfree(pmmsg_t *m)
{
    pciemgr_msgfree(m);
}

/*
 * Broadcast msg to all receivers.
 */
int
pciemgrs_msgsend(pmmsg_t *m)
{
    pmserver_t *pms = &pmserver;
    pmsclient_t *pmsc;

    TAILQ_FOREACH(pmsc, &pms->receivers, list) {
        pciemgr_msgsend(pmsc->fd, m);
    }
    return 0;
}
