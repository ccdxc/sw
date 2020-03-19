/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "cap_top_csr_defines.h"
#include "cap_pp_c_hdr.h"

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgr/include/pciehw.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "cmd.h"
#include "utils.hpp"

typedef union laneinfo_u {
    uint16_t lane[16];
    uint32_t w[8];
} laneinfo_t;

static uint64_t
pp_pcsd_interrupt_addr(const int lane)
{
    return PP_(CFG_PP_PCSD_INTERRUPT) + lane * 4;
}

static uint16_t
pciesd_poll_interrupt_in_progress(const uint16_t want)
{
    const int maxpolls = 100;
    uint16_t inprog;
    int polls = 0;

    do {
        inprog = pal_reg_rd32(PP_(STA_PP_PCSD_INTERRUPT_IN_PROGRESS));
    } while (inprog != want && ++polls < maxpolls);

    if (inprog != want) {
        fprintf(stderr, "interrupt timeout (want 0x%04x, got 0x%04x)\n",
                want, inprog);
        /* continue */
    }

    return inprog;
}

static void
pciesd_core_interrupt(const uint16_t lanemask,
                      const uint16_t code,
                      const uint16_t data,
                      laneinfo_t *dataout)
{
    const uint32_t codedata = ((uint32_t)data << 16) | code;

    /* set up interrupt code/data */
    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            pal_reg_wr32(pp_pcsd_interrupt_addr(i), codedata);
        }
    }

    /* issue interrupt request */
    pal_reg_wr32(PP_(CFG_PP_PCSD_INTERRUPT_REQUEST), lanemask);

    /* wait for interrupt-in-progress */
    pciesd_poll_interrupt_in_progress(lanemask);

    /* clear interrupt request */
    pal_reg_wr32(PP_(CFG_PP_PCSD_INTERRUPT_REQUEST), 0);

    /* wait for interrupt-complete */
    pciesd_poll_interrupt_in_progress(0);

    /* read interrupt response data */
    pal_reg_rd32w(PP_(STA_PP_PCSD_INTERRUPT_DATA_OUT), dataout->w, 8);
}

static void
pcie_sta_pp_sd_core_status(laneinfo_t *li)
{
    pal_reg_rd32w(PP_(STA_PP_SD_CORE_STATUS), li->w, 8);
}

/*
 * Read core status and figure out which pcie serdes lanes of lanemask
 * have the "ready" bit set.  See the Avago serdes documentation.
 */
static uint16_t
pciesd_lanes_ready(const uint16_t lanemask)
{
    laneinfo_t st;
    uint16_t lanes_ready = 0;

    pcie_sta_pp_sd_core_status(&st);
    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            if (st.lane[i] & (1 << 5)) {
                lanes_ready |= lanebit;
            }
        }
    }
    return lanes_ready;
}

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
    uint16_t lanemask, code, data;
    laneinfo_t result;
    int got_code, got_data;

    lanemask = 0xffff;
    got_code = 0;
    got_data = 0;
    code = 0;
    data = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "c:d:l:")) != -1) {
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
        default:
            printf("Usage: %s [-l <lanemask>] -c <code> -d <data>\n", argv[0]);
            return;
        }
    }

    if (!got_code || !got_data) {
        printf("Usage: %s [-l <lanemask>] -c <code> -d <data>\n", argv[0]);
        return;
    }

    pal_wr_lock(SBUSLOCK);
    pciesd_core_interrupt(lanemask, code, data, &result);
    pal_wr_unlock(SBUSLOCK);

    for (int i = 0; i < 16; i++) {
        const uint16_t lanebit = 1 << i;

        if (lanemask & lanebit) {
            printf("lane%-2d 0x%04x\n", i, result.lane[i]);
        }
    }
}
CMDFUNC(serdesint,
"pcie serdes send interrupt",
"serdesint [-l <lanemask>] -c <code> -d <data>\n"
"    -c <code>          int code\n"
"    -d <data>          int data\n"
"    -l <lanemask>      use lanemask (default port lanemask)\n");
