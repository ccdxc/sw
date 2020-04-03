/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pmclient.h"
#include "pciesvc_impl.h"

typedef struct pmclient_s {
    int s;
    unsigned int open:1;
    unsigned int sync_writes:1;
} pmclient_t;

static pmclient_t pmclient;

static int
pciemgrc_socket(const char *addrstr)
{
    struct pmsockaddr a;
    int s;

    s = pmsocket(addrstr, &a);
    while (s >= 0 && connect(s, &a.sa, a.sz) == -1 && errno != EISCONN) {
        /* retry connect if signal interrupted us */
        if (errno == EINTR) continue;
        close(s);
        s = -1;
    }
    return s;
}

int
pciemgrc_open(const char *myname, const char *addrstr, const int receiver)
{
    int s = pciemgrc_socket(addrstr);
    if (s >= 0) {
        pmmsg_t m = {
            .hdr = {
                .msgtype = PMMSG_OPEN,
                .msglen = sizeof(pmmsg_open_t),
            },
        };

        pmclient.open = 1;
        pmclient.sync_writes = 1;
        pmclient.s = s;
        strncpy(m.open.name, myname, sizeof(m.open.name) - 1);
        m.open.receiver = receiver;
        pciemgrc_msgsend(&m);
    }
    return s;
}

void
pciemgrc_close(void)
{
    int s = pmclient.s;
    close(s);
    pmclient.s = -1;
    pmclient.open = 0;
}

int
pciemgrc_msgalloc(pmmsg_t **m, size_t len)
{
    return pciemgr_msgalloc(m, len);
}

void
pciemgrc_msgfree(pmmsg_t *m)
{
    pciemgr_msgfree(m);
}

int
pciemgrc_msgsend(pmmsg_t *m)
{
    return pciemgr_msgsend(pmclient.s, m);
}

int
pciemgrc_msgrecv(pmmsg_t **m)
{
    return pciemgr_msgrecv(pmclient.s, m);
}
