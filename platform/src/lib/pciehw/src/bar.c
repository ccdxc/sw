/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "bdf.h"
#include "pal.h"
#include "pciehsys.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"

static pciehwbar_t *
pciehw_bar_get(pciehwdev_t *phwdev, const int idx)
{
    return &phwdev->bar[idx];
}

static pciehwbar_t *
pciehw_bar_find(const char *name, const int idx)
{
    pciehwdev_t *phwdev = pciehwdev_find_by_name(name);
    return phwdev ? pciehw_bar_get(phwdev, idx) : NULL;
}

int
pciehw_bar_finalize(pciehdev_t *pdev)
{
    pciehwdev_t *phwdev = pciehdev_get_hwdev(pdev);
    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    pciehbar_t *bar;
    int r;

    for (bar = pciehbars_get_first(pbars);
         bar;
         bar = pciehbars_get_next(pbars, bar)) {

        r = pciehw_prt_load(phwdev, bar);
        if (r < 0) {
            /* XXX pciehw_bar_unload(); */
            return -1;
        }
        r = pciehw_pmt_alloc(phwdev, bar);
        if (r < 0) {
            /* XXX pciehw_bar_unload(); */
            return -1;
        }
    }
    return 0;
}

int
pciehw_bar_init(pciehw_t *phw)
{
    pciehw_prt_init(phw);
    pciehw_pmt_init(phw);
    return 0;
}

/******************************************************************
 * debug
 */

static void
bar_show_bar_hdr(void)
{
    pciehsys_log("%-3s %-4s %-9s %-7s "
                 "%-10s %-6s %-5s\n",
                 "idx", "pmti", "prts", "prtsize",
                 "baraddr", "barsz", "flags");
}

static void
bar_show_bar(const int idx, pciehwbar_t *phwbar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_spmt_t *spmt = &phwmem->spmt[phwbar->pmti];

    pciehsys_log("%3d %4d %4d +%-3d 0x%05x "
                 "0x%08"PRIx64" 0x%04x %c\n",
                 idx,
                 phwbar->pmti,
                 phwbar->prtbase,
                 phwbar->prtcount,
                 phwbar->prtsize,
                 spmt->baraddr,
                 spmt->barsize,
                 spmt->loaded ? 'l' : '-');
}

static void
bar_show_dev(pciehwdev_t *phwdev)
{
    pciehwbar_t *phwbar;
    int i;

    pciehsys_log("%s handle %d:\n",
                 pciehwdev_get_name(phwdev), pciehwdev_geth(phwdev));
    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid) {
            bar_show_bar(i, phwbar);
        }
    }
}

static void
bar_show_all(void)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev;
    int i;

    bar_show_bar_hdr();
    phwdev = &phwmem->dev[1];
    for (i = 1; i <= phwmem->allocdev; i++, phwdev++) {
        bar_show_dev(phwdev);
    }
}

// setaddr -d debug0 -b 4 0x12345678
// load -d debug0 -b 4

static void
cmd_setaddr(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwbar_t *phwbar;
    pciehw_spmt_t *spmt;
    int opt, idx;
    char *name;
    u_int64_t baraddr;

    idx = -1;
    name = NULL;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:b:")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        case 'b':
            idx = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (name == NULL || idx == -1 || optind >= argc) {
        pciehsys_log("Usage: setaddr -d <devname> -b <baridx> <addr>\n");
        return;
    }

    baraddr = strtoull(argv[optind], NULL, 0);

    phwbar = pciehw_bar_find(name, idx);
    if (phwbar == NULL) {
        pciehsys_log("setaddr: %s bar %d not found\n", name, idx);
        return;
    }
    if (!phwbar->valid) {
        pciehsys_log("setaddr: %s bar %d invalid\n", name, idx);
    }

    spmt = &phwmem->spmt[phwbar->pmti];
    spmt->baraddr = baraddr;
}

static void
cmd_load(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehwdev_t *phwdev;
    pciehwbar_t *phwbar;
    int opt, idx;
    char *name;

    idx = -1;
    name = NULL;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:b:")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        case 'b':
            idx = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (name == NULL || idx == -1) {
        pciehsys_log("Usage: setaddr -d <devname> -b <baridx> <addr>\n");
        return;
    }

    phwdev = pciehwdev_find_by_name(name);
    if (phwdev == NULL) {
        pciehsys_log("setaddr: %s not found\n", name);
        return;
    }
    phwbar = pciehw_bar_get(phwdev, idx);
    if (phwbar == NULL) {
        pciehsys_log("setaddr: %s bar %d not found\n", name, idx);
        return;
    }
    if (!phwbar->valid) {
        pciehsys_log("setaddr: %s bar %d invalid\n", name, idx);
    }

    pciehw_pmt_load_bar(phw, phwdev, phwbar);
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
    CMDENT(setaddr, "setaddr", ""),
    CMDENT(load, "load", ""),
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

void
pciehw_bar_dbg(int argc, char *argv[])
{
    cmd_t *c;

    if (argc <= 1) {
        bar_show_all();
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciehsys_log("%s: debug command not found\n", argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}
