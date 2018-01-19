/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "misc.h"
#include "bdf.h"
#include "pal.h"
#include "pciehsys.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "portmap.h"

static int
portmap_size(void)
{
    return PORTMAP_COUNT;
}

static u_int64_t
portmap_addr(const u_int32_t idx)
{
    assert(idx < portmap_size());
    return PORTMAP_BASE + (idx * PORTMAP_STRIDE);
}

static void
portmap_get(pciehw_t *phw, const u_int32_t idx, u_int64_t *entry)
{
    pal_reg_rd32w(portmap_addr(idx), (u_int32_t *)entry, PORTMAP_NWORDS);
}

static void
portmap_set(pciehw_t *phw, const u_int32_t idx, const u_int64_t entry)
{
    pal_reg_wr32w(portmap_addr(idx), (u_int32_t *)&entry, PORTMAP_NWORDS);
}

static void
portmap_setlif(pciehw_t *phw, const u_int32_t lif, const u_int8_t port)
{
    const u_int32_t idx = lif >> 4;
    const u_int8_t off = lif & 0xf;
    u_int64_t e;

    portmap_get(phw, idx, &e);
    e &= ~(0x7 << (off * 3));
    e |= (port & 0x7) << (off * 3);
    portmap_set(phw, idx, e);
}

/******************************************************************
 * apis
 */

int
pciehw_portmap_load(pciehw_t *phw, const u_int32_t lif, const u_int8_t port)
{
    portmap_setlif(phw, lif, port);
    return 0;
}

int
pciehw_portmap_unload(pciehw_t *phw, const u_int32_t lif)
{
    /* nothing to do */
    return 0;
}

void
pciehw_portmap_init(pciehw_t *phw)
{
    int i;

    for (i = 0; i < portmap_size(); i++) {
        portmap_set(phw, i, 0);
    }
}

/******************************************************************
 * debug
 */

static void
portmap_show(const int raw)
{
    pciehw_t *phw = pciehw_get();
    u_int64_t e;
    int i, l;

    pciehsys_log("portmap_size() = %d\n", portmap_size());

    pciehsys_log("%-4s  %-7s\n", "lif", "port");
    for (i = 0; i < portmap_size(); i++) {
        portmap_get(phw, i, &e);
        pciehsys_log("%4d:", i * 16);
        for (l = 0; l < 16; l++) {
           const u_int8_t port = (e >> (l * 3)) & 0x7;
            pciehsys_log(" %1d%s", port, l == 7 ? " " : "");
        }
        pciehsys_log("\n");
    }
}

void
pciehw_portmap_dbg(int argc, char *argv[])
{
    int opt, raw;

    raw = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
        case 'r':
            raw = 1;
            break;
        default:
            return;
        }
    }

    portmap_show(raw);
}
