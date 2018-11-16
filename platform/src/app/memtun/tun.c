
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <sys/ioctl.h>
#include <net/if.h>
#ifndef __FreeBSD__
#include <linux/if_tun.h>
#endif
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "dtls.h"

int
tun_open(char **namp)
{
#ifndef __FreeBSD__
    struct ifreq ifr;
    int r;
#endif
    int fd;
    char path[20];

#ifdef __FreeBSD__
    *namp = strdup("tun100");
    snprintf(path, 20, "/dev/%s", *namp);
#else
    snprintf(path, 20, "/dev/net/tun");
#endif

    if ((fd = open(path, O_RDWR)) < 0) {
        perror(path);
        return -1;
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);

#ifndef __FreeBSD__
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    r = ioctl(fd, TUNSETIFF, &ifr);
    if (r < 0) {
        perror("TUNSETIFF");
        close(fd);
        return -1;
    }
    *namp = strdup(ifr.ifr_name);
#endif
    return fd;
}

static int
_e_ioctl(int fd, uint32_t cmd, const char *cmds, void *arg)
{
    if (ioctl(fd, cmd, arg) < 0) {
        perror(cmds);
        return -1;
    }
    return 0;
}
#define e_ioctl(fd, cmd, arg)       _e_ioctl(fd, cmd, #cmd, arg)

static int
if_down(int s, struct ifreq *ifr)
{
    int r;
    if ((r = e_ioctl(s, SIOCGIFFLAGS, ifr)) == 0) {
        ifr->ifr_flags &= ~IFF_UP;
        r = e_ioctl(s, SIOCSIFFLAGS, ifr);
    }
    return r;
}

static int
if_up(int s, struct ifreq *ifr)
{
    int r;
    if ((r = e_ioctl(s, SIOCGIFFLAGS, ifr)) == 0) {
        ifr->ifr_flags |= IFF_UP;
        r = e_ioctl(s, SIOCSIFFLAGS, ifr);
    }
    return r;
}

#ifndef __FreeBSD__
static int
if_setxaddr(int s, struct ifreq *ifr, uint32_t cmd, char *cmds, uint32_t addr)
{
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr);
    memcpy(&ifr->ifr_addr, &sin, sizeof (sin));
    return _e_ioctl(s, cmd, cmds, ifr);
}

static int
if_setifaddr(int s, struct ifreq *ifr, uint32_t addr)
{
    return if_setxaddr(s, ifr, SIOCSIFADDR, "SIOCSIFADDR", addr);
}

static int
if_setifdstaddr(int s, struct ifreq *ifr, uint32_t addr)
{
    return if_setxaddr(s, ifr, SIOCSIFDSTADDR, "SIOCSIFDSTADDR", addr);
}
#endif

int
tun_setaddrs(const char *ifname, uint32_t local, uint32_t remote)
{
    struct ifreq ifr;
    int s, r;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&ifr, 0, sizeof (ifr));
    strcpy(ifr.ifr_name, ifname);
    r = -1;
    if (if_down(s, &ifr) < 0) {
        goto out;
    }
#ifndef __FreeBSD__
    if (if_setifaddr(s, &ifr, local) < 0) {
        goto out;
    }
    if (if_setifdstaddr(s, &ifr, remote) < 0) {
        goto out;
    }
#endif
    r = if_up(s, &ifr);
out:
    close(s);
    return r;
}
