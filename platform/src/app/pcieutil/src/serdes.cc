/*
 * Copyright (c) 2019-2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgr/include/pciehw.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "cmd.h"
#include "utils.hpp"

static uint16_t
default_lanemask(void)
{
    int port = default_pcieport();
    pcieport_t *p = pcieport_get(port);
    return p ? p->lanemask : 0xffff;
}

static void
serdesfw(int argc, char *argv[])
{
    int opt;
    uint16_t lanemask, lanes_ready;
    laneinfo_t build, revid, engbd;

    lanemask = default_lanemask();
    optind = 0;
    while ((opt = getopt(argc, argv, "l:")) != -1) {
        switch (opt) {
        case 'l':
            lanemask = strtoul(optarg, NULL, 0);
            break;
        default:
            printf("Usage: %s [-l <lanemask>]\n", argv[0]);
            return;
        }
    }

    pal_wr_lock(SBUSLOCK);
    lanes_ready = pciesd_lanes_ready(lanemask);
    pciesd_core_interrupt(lanes_ready, 0,    0, &build);
    pciesd_core_interrupt(lanes_ready, 0x3f, 0, &revid);
    pciesd_core_interrupt(lanes_ready, 0,    1, &engbd);
    pal_wr_unlock(SBUSLOCK);

    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;
        const int ready = (lanes_ready & lanebit) != 0;
        char buildrev[16] = { '\0' };

        if (lanemask & lanebit) {
            if (ready) {
                snprintf(buildrev, sizeof(buildrev),
                         "%04x %04x %04x",
                         build.lane[i], revid.lane[i], engbd.lane[i]);
            } else {
                strncpy(buildrev, "not ready", sizeof(buildrev));
            }
            printf("lane%-2d %s\n", i, buildrev);
        }
    }
}
CMDFUNC(serdesfw,
"pcie serdes fw version info",
"serdesfw [-l <lanemask>]\n"
"    -l <lanemask>      use lanemask (default port lanemask)\n");

static void
serdesint(int argc, char *argv[])
{
    int opt;
    u_int64_t starttm, stoptm;
    uint16_t lanemask, code, data;
    laneinfo_t result;
    int got_code, got_data, dotime;

    lanemask = 0xffff;
    got_code = 0;
    got_data = 0;
    dotime = 0;
    code = 0;
    data = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "c:d:l:t")) != -1) {
        switch (opt) {
        case 'c':
            code = strtoul(optarg, NULL, 0);
            got_code = 1;
            break;
        case 'd':
            data = strtoul(optarg, NULL, 0);
            got_data = 1;
            break;
        case 'l':
            lanemask = strtoul(optarg, NULL, 0);
            break;
        case 't':
            dotime = 1;
            break;
        default:
            fprintf(stderr,
                    "Usage: %s [-t][-l <lanemask>] -c <code> -d <data>\n",
                    argv[0]);
            return;
        }
    }

    if (!got_code || !got_data) {
        fprintf(stderr,
                "Usage: %s [-t][-l <lanemask>] -c <code> -d <data>\n",
                argv[0]);
        return;
    }

    pal_wr_lock(SBUSLOCK);
    starttm = gettimestamp();
    pciesd_core_interrupt(lanemask, code, data, &result);
    stoptm = gettimestamp();
    pal_wr_unlock(SBUSLOCK);

    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            printf("lane%-2d 0x%04x\n", i, result.lane[i]);
        }
    }
    if (dotime) {
        printf("elapsed time: %.6lf seconds\n",
               (stoptm - starttm) / 1000000.0);
    }
}
CMDFUNC(serdesint,
"pcie serdes send interrupt",
"serdesint [-t][-l <lanemask>] -c <code> -d <data>\n"
"    -c <code>          int code\n"
"    -d <data>          int data\n"
"    -l <lanemask>      use lanemask (default port lanemask)\n"
"    -t                 display elapsed time taken for interrupt\n");
