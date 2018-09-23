/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"
#include "pal.h"
#include "cmd.h"

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
linkpoll(int argc, char *argv[])
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
CMDFUNC(linkpoll, "linkpoll [-p<port>][-t <polltm>]");
