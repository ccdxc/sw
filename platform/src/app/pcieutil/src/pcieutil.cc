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
#include <sys/types.h>
#include <sys/param.h>

#include "pal.h"

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
