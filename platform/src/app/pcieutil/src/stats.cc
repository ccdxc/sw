/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "nic/sdk/platform/pciemgr/include/pciemgr.h"

#include "cmd.h"
#include "utils.hpp"

static void
stats(int argc, char *argv[])
{
    int opt, port, do_clear;
    unsigned int flags;

    port = default_pcieport();
    flags = PMGRSF_NONE;
    do_clear = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "acp:")) != -1) {
        switch (opt) {
        case 'a':
            flags |= PMGRSF_ALL;
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

    if (pciehdev_open(NULL) < 0) {
        fprintf(stderr, "pciehdev_open failed\n");
        return;
    }

    if (do_clear) pciehw_stats_clear(port, flags);
    else          pciehw_stats_show(port, flags);

    pciehdev_close();
}
CMDFUNC(stats,
"show pcie stats",
"counters [-ac][-p <port>]\n"
"    -a         show all stats, even if 0\n"
"    -c         clear all stats to 0\n"
"    -p <port>  poll port <port> (default port 0)\n");
