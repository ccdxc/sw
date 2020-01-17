/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include "misc.h"

static int
netif_ioctl(int s, const char *ifname, const int sioc, const u_int32_t addr)
{
    struct ifreq ifr;
    struct sockaddr_in sin;

    memset(&ifr, 0, sizeof(ifr));
    strncpy0(ifr.ifr_name, ifname, IFNAMSIZ);
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr);
    memcpy(&ifr.ifr_addr, &sin, sizeof(sin));
    return ioctl(s, sioc, &ifr);
}

enum { CLRIFF, SETIFF };

static int
netif_ifflags(const char *ifname, const int op, const int ifflags)
{
    struct ifreq ifr;
    int s = -1;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        goto error_out;
    }
    /* read flags... */
    memset(&ifr, 0, sizeof(ifr));
    strncpy0(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) {
        goto error_out;
    }
    /* ...modify flags... */
    if (op == SETIFF) {
        ifr.ifr_flags |= ifflags;
    } else {
        ifr.ifr_flags &= ~ifflags;
    }
    /* ...write flags. */
    strncpy0(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0) {
        goto error_out;
    }
    close(s);
    return 0;

 error_out:
    if (s != -1) close(s);
    return -1;
}

int
netif_up(const char *ifname)
{
    return netif_ifflags(ifname, SETIFF, IFF_UP | IFF_RUNNING);
}

int
netif_down(const char *ifname)
{
    return netif_ifflags(ifname, CLRIFF, IFF_UP);
}

int
netif_setip(const char *ifname, const u_int32_t ip, const u_int32_t nm)
{
    int s = -1;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        goto error_out;
    }
    if (netif_ioctl(s, ifname, SIOCSIFADDR, ip) < 0) {
        goto error_out;
    }
    if (ip && nm && netif_ioctl(s, ifname, SIOCSIFNETMASK, nm) < 0) {
        goto error_out;
    }
    close(s);
    return 0;

 error_out:
    if (s != -1) close(s);
    return -1;
}
