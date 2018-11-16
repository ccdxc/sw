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

#include "platform/src/lib/pal/include/pal.h"
#include "cmd.h"

extern cmd_t __start_cmdtab[];
extern cmd_t __stop_cmdtab[];

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
cmd_pending(int argc, char *argv[]) __attribute__((used));
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

static cmd_t *
cmd_lookup(const char *name)
{
    for (cmd_t *c = __start_cmdtab; c < __stop_cmdtab; c++) {
        if (strcmp(name, c->name) == 0) return c;
    }
    return NULL;
}

static void
help(int argc, char *argv[])
{
    if (argc <= 1) {
        printf("Usage: pcieutil <cmd>[args...]\n"
               "Available commands:");
        int pos = 80;
        const char *sep = "";
        for (cmd_t *c = __start_cmdtab; c < __stop_cmdtab; c++) {
            int len = strlen(sep) + strlen(c->name);
            if (pos + len >= 80) {
                fputs("\n    ", stdout);
                pos = 4;
                sep = "";
            }
            printf("%s%s", sep, c->name);
            sep = ", ";
            pos += len;
        }
        putchar('\n');
        return;
    }

    cmd_t *c = cmd_lookup(argv[1]);
    if (c == NULL) {
        printf("%s: not found\n", argv[1]);
        return;
    }

    printf("Usage: %s\n", c->help);
}
CMDFUNC(help, "help [<cmd>]");

int
main(int argc, char *argv[])
{
    cmd_t *c;

    if (argc <= 1) {
        c = cmd_lookup("help");
        c->func(argc, argv);
        exit(0);
    }

    c = cmd_lookup(argv[1]);
    if (c == NULL) {
        printf("%s: not found\n", argv[1]);
        exit(1);
    }

    argc--;
    argv++;
    c->func(argc, argv);
    exit(0);
}
