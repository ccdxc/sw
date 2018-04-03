/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/param.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "pal.h"

/*
 * Register convenience macros.
 */
#define PP_(REG) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + CAP_PP_CSR_ ##REG## _BYTE_ADDRESS)

#define PXC_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXC_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_C_ ##REG## _BYTE_ADDRESS)

#define PXP_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXP_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_P_ ##REG## _BYTE_ADDRESS)

/* sta_rst flags */
#define STA_RSTF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_RST_ ##REG## _FIELD_MASK)

/* sta_mac flags */
#define STA_MACF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_MAC_ ##REG## _FIELD_MASK)

/* cfg_mac flags */
#define CFG_MACF_(REG) \
    (CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_ ##REG## _FIELD_MASK)

static void
usage(void)
{
    fprintf(stderr, "pcieutil [cmd][cmd-args ...]\n");
}

static void
cmd_pending(int argc, char *argv[])
{
    time_t tm_now, tm_last;
    u_int32_t samples, rxcrbfr, axi_pending;
    u_int32_t showtime_us;
    u_int32_t val;
    int opt;

    optind = 0;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            showtime_us = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    tm_last = 0;
    samples = rxcrbfr = axi_pending = 0;
    while (1) {

        val = pal_reg_rd32(0x0719a408); // pxb_sta_tgt_rxcrbfr_debug_0
        rxcrbfr += (val >> 20);

        val = pal_reg_rd64(0x0719a300); // pxb_sta_tgt_axi_pending
        axi_pending += (val & 0xff);

        samples++;

        time(&tm_now);
        if (tm_last == 0) tm_last = tm_now;
        if (difftime(tm_now, tm_last) > 1) {

            printf("rxcrbfr %3d axi_pending %3d (samples %d)\n",
                   rxcrbfr / samples, axi_pending / samples, samples);
            samples = rxcrbfr = axi_pending = 0;
            tm_last = tm_now;
        }
    }
    if (showtime_us) return; /* XXX */
}

static void
cmd_linkstate(int argc, char *argv[])
{
    struct linkstate {
        unsigned int perstn:1;
        unsigned int phystatus:1;
        unsigned int portgate:1;
        unsigned int crs:1;
        unsigned int ltssm_en:1;
        unsigned int ltssm_st:5;
        unsigned int fifo_rd:8;
        unsigned int fifo_wr:8;
    } ost, nst;
    int port, polltm_us, opt;

    port = 0;
    polltm_us = 0;

    optind = 0;
    while ((opt = getopt(argc, argv, "p:t:")) != -1) {
        switch (opt) {
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

    printf("perstn\n");
    printf("|phystatus\n");
    printf("||portgate_open\n");
    printf("|||cfg_retry\n");
    printf("||||ltssm_en\n");
    printf("|||||  fifo\n");
    printf("Ppgrl  rd/wr  ltssm\n");

    memset(&ost, 0, sizeof(ost));
    memset(&nst, 0, sizeof(nst));
    while (1) {
        const u_int32_t sta_rst = pal_reg_rd32(PXC_(STA_C_PORT_RST, port));
        const u_int32_t sta_mac = pal_reg_rd32(PXC_(STA_C_PORT_MAC, port));
        const u_int32_t cfg_mac = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, port));
        u_int16_t portfifo[8], depths;
#define PORTFIFO_DEPTH \
        (CAP_ADDR_BASE_PXB_PXB_OFFSET +                 \
         CAP_PXB_CSR_STA_ITR_PORTFIFO_DEPTH_BYTE_ADDRESS)

        nst.perstn = (sta_rst & STA_RSTF_(PERSTN)) != 0;
        nst.phystatus = (sta_rst & STA_RSTF_(PHYSTATUS_OR)) != 0;
        nst.portgate = (sta_mac & STA_MACF_(PORTGATE_OPEN)) != 0;
        nst.ltssm_en = (cfg_mac & CFG_MACF_(0_2_LTSSM_EN)) != 0;
        nst.crs = (cfg_mac & CFG_MACF_(0_2_CFG_RETRY_EN)) != 0;
        nst.ltssm_st = (sta_mac & 0x1f);

        pal_reg_rd32w(PORTFIFO_DEPTH, (u_int32_t *)portfifo, 4);
        depths = portfifo[port];

        nst.fifo_wr = depths;
        nst.fifo_rd = depths >> 8;

        /* fold small depths to 0's */
        if (nst.fifo_wr <= 2) nst.fifo_wr = 0;
        if (nst.fifo_rd <= 2) nst.fifo_rd = 0;

        if (memcmp(&nst, &ost, sizeof(nst)) != 0) {

            printf("%c%c%c%c%c %3u/%-3u %u\n",
                   nst.perstn ? 'P' : '-',
                   nst.phystatus ? 'p' :'-',
                   nst.portgate ? 'g' : '-',
                   nst.crs ? 'r' : '-',
                   nst.ltssm_en ? 'l' : '-',
                   nst.fifo_rd,
                   nst.fifo_wr,
                   nst.ltssm_st);

            ost = nst;
        }

        if (polltm_us) usleep(polltm_us);
    }
}

typedef struct cmd_s {
    const char *name;
    void (*f)(int argc, char *argv[]);
    const char *desc;
    const char *helpstr;
} cmd_t;

static cmd_t cmdtab[] = {
#define CMDENT(name, desc, helpstr) \
    { #name, cmd_##name, desc, helpstr }
    CMDENT(pending, "pending transactions", ""),
    CMDENT(linkstate, "show link state", ""),
    { NULL, NULL }
};

static cmd_t *
cmd_lookup(cmd_t *cmdtab, const char *name)
{
    cmd_t *c;

    for (c = cmdtab; c->name; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

static void
do_cmd(int argc, char *argv[])
{
    cmd_t *c;

    c = cmd_lookup(cmdtab, argv[0]);
    if (c == NULL) {
        fprintf(stderr, "%s: command not found\n", argv[0]);
        return;
    }
    c->f(argc, argv);
}

int
main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
            usage();
            exit(1);
        }
    }

    argv += optind;
    argc -= optind;

    if (argc <= 0) {
        usage();
        exit(1);
    }

    do_cmd(argc, argv);

    exit(0);
}
