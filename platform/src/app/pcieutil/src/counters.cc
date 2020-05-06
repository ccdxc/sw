/*
 * Copyright (c) 2019-2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <cinttypes>

#include "platform/pcieport/include/portmap.h"
#include "utils.hpp"
#include "counters.hpp"
#include "pcieutilpd.h"
#include "cmd.h"

static void
show_port_counters(const int port, void *arg)
{
    const int flags = *(int *)arg;

    counterspd_show_port_counters(port, flags);
}

static void
counters(int argc, char *argv[])
{
    int opt, flags;

    flags = F_NONE;
    optind = 0;
    while ((opt = getopt(argc, argv, "ac")) != -1) {
        switch (opt) {
        case 'a':
            flags |= F_SHOWALL;
            break;
        case 'c':
            flags |= F_CLEAR;
            break;
        default:
            return;
        }
    }

    counterspd_show_global_counters(flags);

    portmap_init_from_catalog();
    portmap_foreach_port(show_port_counters, &flags);
}
CMDFUNC(counters,
"show pcie counters",
"counters [-ac]\n"
"    -a         show all counters, even if 0\n"
"    -c         clear all counters to 0\n");
