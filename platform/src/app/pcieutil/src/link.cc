/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/time.h>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pcieport/include/portcfg.h"

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "cmd.h"
#include "utils.hpp"

static const char *
ltssm_str(const unsigned int ltssm)
{
    static const char *ltssm_strs[] = {
        [0x00] = "detect.quiet",
        [0x01] = "detect.active",
        [0x02] = "polling.active",
        [0x03] = "polling.compliance",
        [0x04] = "polling.configuration",
        [0x05] = "config.linkwidthstart",
        [0x06] = "config.linkwidthaccept",
        [0x07] = "config.lanenumwait",
        [0x08] = "config.lanenumaccept",
        [0x09] = "config.complete",
        [0x0a] = "config.idle",
        [0x0b] = "recovery.receiverlock",
        [0x0c] = "recovery.equalization",
        [0x0d] = "recovery.speed",
        [0x0e] = "recovery.receiverconfig",
        [0x0f] = "recovery.idle",
        [0x10] = "L0",
        [0x11] = "L0s",
        [0x12] = "L1.entry",
        [0x13] = "L1.idle",
        [0x14] = "L2.idle",
        [0x15] = "<reserved>",
        [0x16] = "disable",
        [0x17] = "loopback.entry",
        [0x18] = "loopback.active",
        [0x19] = "loopback.exit",
        [0x1a] = "hotreset",
    };
    const int nstrs = sizeof(ltssm_strs) / sizeof(ltssm_strs[0]);
    if (ltssm >= nstrs) return "unknown";
    return ltssm_strs[ltssm];
}

static u_int64_t
gettimestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

u_int32_t
pcieport_get_sta_p_port_mac(const int port)
{
    return pal_reg_rd32(PXP_(STA_P_PORT_MAC, port));
}

int
pcieport_get_mac_lanes_reversed(const int port)
{
    const u_int32_t sta_mac = pcieport_get_sta_p_port_mac(port);
    return (sta_mac & STA_P_PORT_MACF_(LANES_REVERSED)) != 0;
}

static void
linkpoll(int argc, char *argv[])
{
    struct linkstate {
        unsigned int perstn:1;
        unsigned int phystatus:1;
        unsigned int portgate:1;
        unsigned int crs:1;
        unsigned int ltssm_en:1;
        unsigned int ltssm_st:5;
        unsigned int reversed:1;
        unsigned int fifo_rd:8;
        unsigned int fifo_wr:8;
        int gen;
        int width;
    } ost, nst;
    u_int64_t otm, ntm;
    int port, polltm_us, opt, showall;

    port = default_pcieport();
    polltm_us = 0;
    showall = 0;

    optind = 0;
    while ((opt = getopt(argc, argv, "ap:t:")) != -1) {
        switch (opt) {
        case 'a':
            showall = 1;
            break;
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 't':
            polltm_us = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    printf("              perstn (pcie refclk good)\n");
    printf("              |phystatus (phy out of reset)\n");
    printf("              ||ltssm_en (link training ready)\n");
    printf("              |||portgate_open (traffic can flow)\n");
    printf("              ||||cfg_retry off (cfg transactions allowed)\n");
    printf("              |||||                 reversed lanes\n");
    printf("              |||||  fifo           |\n");
    printf(" +time (sec)  Pplgr  rd/wr  genGxW  r ltssm\n");

    memset(&ost, 0, sizeof(ost));
    memset(&nst, 0, sizeof(nst));
    otm = 0;
    ntm = 0;
    while (1) {
        const u_int32_t sta_rst = pal_reg_rd32(PXC_(STA_C_PORT_RST, port));
        const u_int32_t sta_mac = pal_reg_rd32(PXC_(STA_C_PORT_MAC, port));
        const u_int32_t cfg_mac = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, port));
        u_int16_t portfifo[8], depths;

        nst.perstn = (sta_rst & STA_RSTF_(PERSTN)) != 0;
        nst.phystatus = (sta_rst & STA_RSTF_(PHYSTATUS_OR)) != 0;
        nst.portgate = (sta_mac & STA_C_PORT_MACF_(PORTGATE_OPEN)) != 0;
        nst.ltssm_en = (cfg_mac & CFG_MACF_(0_2_LTSSM_EN)) != 0;
        nst.crs = (cfg_mac & CFG_MACF_(0_2_CFG_RETRY_EN)) != 0;
        nst.ltssm_st = (sta_mac & 0x1f);
        nst.reversed = pcieport_get_mac_lanes_reversed(port);

        // protect against mac reset events
        // gen/width registers are inaccessible when mac is reset.
        // (this is not perfect, still racy)
        if (pcieport_is_accessible(port)) {
            portcfg_read_genwidth(port, &nst.gen, &nst.width);
        } else {
            nst.gen = 0;
            nst.width = 0;
        }

        pal_reg_rd32w(PXB_(STA_ITR_PORTFIFO_DEPTH), (u_int32_t *)portfifo, 4);
        depths = portfifo[port];

        nst.fifo_wr = depths;
        nst.fifo_rd = depths >> 8;

        /* fold small depths to 0's */
        if (!showall && nst.fifo_wr <= 2) nst.fifo_wr = 0;
        if (!showall && nst.fifo_rd <= 2) nst.fifo_rd = 0;

        /* fold early detect states quiet/active, too many at start */
        if (!showall && nst.ltssm_st == 1) nst.ltssm_st = 0;

        if (memcmp(&nst, &ost, sizeof(nst)) != 0) {
            ntm = gettimestamp();
            if (otm == 0) otm = ntm;

            printf("[+%010.6lf] %c%c%c%c%c %3u/%-3u gen%dx%-2d %c 0x%02x %s\n",
                   (ntm - otm) / 1000000.0,
                   nst.perstn           ? 'P' : '-',
                   nst.phystatus == 0   ? 'p' :'-',
                   nst.ltssm_en         ? 'l' : '-',
                   nst.portgate         ? 'g' : '-',
                   nst.crs == 0         ? 'r' : '-',
                   nst.fifo_rd,
                   nst.fifo_wr,
                   nst.gen,
                   nst.width,
                   nst.reversed ? 'r' : ' ',
                   nst.ltssm_st,
                   ltssm_str(nst.ltssm_st));

            ost = nst;
            otm = ntm;
        }

        if (polltm_us) usleep(polltm_us);
    }
}
CMDFUNC(linkpoll,
"poll pcie link state",
"linkpoll [-a][-p <port>][-t <polltm>]\n"
"    -a         show all state changes (default ignores small changes)\n"
"    -p <port>  poll port <port> (default port 0)\n"
"    -t <polltm> <polltm> microseconds between poll events (default 0)\n");
