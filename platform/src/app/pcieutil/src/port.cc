/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/time.h>

#include "lib/catalog/catalog.hpp"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pcieport/include/portmap.h"
#include "nic/sdk/platform/pcieport/include/portcfg.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd.hpp"

#include "cmd.h"
#include "utils.hpp"

static void
port(int argc, char *argv[])
{
    int opt, port;

    port = default_pcieport();
    optind = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }
    if (pcieport_open(port, INHERIT_ONLY) < 0) {
        fprintf(stderr, "pcieport_open failed\n");
        return;
    }
    pcieport_showport(port);
    pcieport_close(port);
}
CMDFUNC(port,
"port information",
"port [-p <port>]\n"
"    -p <port>  poll port <port> (default port 0)\n");

static void
ports(int argc, char *argv[])
{
    uint32_t portmask;
    int port;

    portmap_init_from_catalog();
    portmask = portmap_portmask();

    /* find and open the first active port to map in the shared data */
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        if (portmask & (1 << port)) {
            if (pcieport_open(port, INHERIT_ONLY) < 0) {
                fprintf(stderr, "pcieport_open %d failed\n", port);
                return;
            }
            break;
        }
    }
    if (port >= PCIEPORT_NPORTS) {
        return;
    }
    pcieport_showports();
    pcieport_close(port);
}
CMDFUNC(ports,
"display port summary",
"ports\n");

static void
portstats(int argc, char *argv[])
{
    int opt, port, do_clear;
    unsigned int flags;

    flags = PSF_NONE;
    port = default_pcieport();
    do_clear = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "acp:")) != -1) {
        switch (opt) {
        case 'a':
            flags |= PSF_ALL;
            break;
        case 'c':
            do_clear = 1;
            break;
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (pcieport_open(port, INHERIT_ONLY) < 0) {
        fprintf(stderr, "pcieport_open %d failed\n", port);
        return;
    }

    if (do_clear) pcieport_clearportstats(port, flags);
    else          pcieport_showportstats(port, flags);

    pcieport_close(port);
}
CMDFUNC(portstats,
"port statistics",
"portstats [-ac][-p <port>]\n"
"    -a         show all stats, even if 0\n"
"    -c         clear all stats to 0\n"
"    -p <port>  poll port <port> (default port 0)\n");

static void
portmap_host(const int host, void *arg)
{
    const int port = portmap_pcieport(host);
    if (port < 0) {
        fprintf(stderr, "host%d bad port\n", host);
        return;
    }

    pcieport_spec_t ps;
    if (portmap_getspec(port, &ps) < 0) {
        fprintf(stderr, "host%d no spec\n", host);
        return;
    }

    printf("host%d port%d gen%dx%d\n", host, port, ps.gen, ps.width);
}

static void
portmap(int argc, char *argv[])
{
    portmap_init_from_catalog();
    portmap_foreach_host(portmap_host, NULL);
}
CMDFUNC(portmap,
"port map to host ports",
"portmap\n");
