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
#include <cinttypes>
#include <sys/types.h>
#include <sys/param.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/pal/include/pal.h"
#include "cmd.h"

extern cmd_t __start_cmdtab[];
extern cmd_t __stop_cmdtab[];

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

static int
cmd_cmp(const void *a1, const void *a2)
{
    const cmd_t *c1 = (cmd_t *)a1;
    const cmd_t *c2 = (cmd_t *)a2;

    return strcmp(c1->name, c2->name);
}

static void
help(int argc, char *argv[])
{
    if (argc <= 1) {
        const size_t ncmds = __stop_cmdtab - __start_cmdtab;
        const size_t cmdsz = sizeof(cmd_t);
        cmd_t *cmdtab = __start_cmdtab;

        /* sort cmds alphabetically for help display */
        qsort(cmdtab, ncmds, cmdsz, cmd_cmp);

        printf("Usage: pcieutil <cmd>[args...]\n"
               "Available commands:\n");
        for (cmd_t *c = cmdtab; c < &cmdtab[ncmds]; c++) {
            printf("    %-*s %s\n", 16, c->name, c->desc);
        }
        return;
    }

    cmd_t *c = cmd_lookup(argv[1]);
    if (c == NULL) {
        printf("%s: not found\n", argv[1]);
        return;
    }

    printf("Usage: %s\n", c->usage);
}
CMDFUNC(help,
"display command help",
"help [<cmd>]\n"
"    no args    display all commands and description\n"
"    <cmd>      usage for <cmd>\n");

int
cmd_run(int argc, char *argv[])
{
    cmd_t *c;

    if (argc <= 1) {
        c = cmd_lookup("help");
        c->func(argc, argv);
        return 0;
    }

    c = cmd_lookup(argv[1]);
    if (c == NULL) {
        printf("%s: not found\n", argv[1]);
        return 1;
    }

    argc--;
    argv++;
    c->func(argc, argv);
    return 0;
}

int
cmd_runstr(char *cmdstr)
{
    char *argv[16];
    int argc;

    argc = strtoargv(cmdstr, argv, sizeof(argv) / sizeof(argv[0]));
    return cmd_run(argc, argv);
}

int
main(int argc, char *argv[])
{
    return cmd_run(argc, argv);
}
