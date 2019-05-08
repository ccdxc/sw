/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PCIESVC_IMPL_H__
#define __PCIESVC_IMPL_H__

#include "pmmsg.h"

int pciemgr_msgsend(int fd, pmmsg_t *m);
int pciemgr_msgrecv(int fd, pmmsg_t **m);

struct pmsockaddr {
    union {
        struct sockaddr    sa;
        struct sockaddr_in in;
        struct sockaddr_un un;
    };
    size_t sz;
};

int pmsocket(const char *addrstr, struct pmsockaddr *a);

#endif /* __PCIESVC_IMPL_H__ */
