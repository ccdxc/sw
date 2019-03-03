/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/time.h>

#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pcieport/include/portcfg.h"

#include "cmd.h"

static void
port(int argc, char *argv[])
{
    int opt, port;

    port = 0;
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
CMDFUNC(port, "port [-p<port>]");

static void
ports(int argc, char *argv[])
{
    if (pcieport_open(0, INHERIT_ONLY) < 0) {
        fprintf(stderr, "pcieport_open failed\n");
        return;
    }

    pcieport_showports();

    pcieport_close(0);
}
CMDFUNC(ports, "ports");

static void
portstats(int argc, char *argv[])
{
    int opt, port, do_clear;
    unsigned int flags;

    flags = PSF_NONE;
    port = 0;
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

    if (port < 0 || port >= PCIEPORT_NPORTS) {
        pciesys_logerror("port %d out of range\n", port);
        return;
    }
    if (pcieport_open(port, INHERIT_ONLY) < 0) {
        fprintf(stderr, "pcieport_open failed\n");
        return;
    }

    if (do_clear) pcieport_clearportstats(port, flags);
    else          pcieport_showportstats(port, flags);

    pcieport_close(port);
}
CMDFUNC(portstats, "portstats [-ac][-p<port>]");
