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

#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"
#include "portmap.h"

#define PORTMAP_BASE    PXB_(DHS_ITR_PORTMAP)
#define PORTMAP_COUNT   ASIC_(PXB_CSR_DHS_ITR_PORTMAP_ENTRIES)
#define PORTMAP_STRIDE  ASIC_(PXB_CSR_DHS_ITR_PORTMAP_ENTRY_BYTE_SIZE)
#define PORTMAP_NWORDS  2

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
portmap_get(const u_int32_t idx, u_int64_t *entry)
{
    pal_reg_rd32w(portmap_addr(idx), (u_int32_t *)entry, PORTMAP_NWORDS);
}

static void
portmap_set(const u_int32_t idx, const u_int64_t entry)
{
    pal_reg_wr32w(portmap_addr(idx), (u_int32_t *)&entry, PORTMAP_NWORDS);
}

static void
portmap_setlif(const u_int32_t lif, const u_int8_t port)
{
    const u_int32_t idx = lif >> 4;
    const u_int8_t off = lif & 0xf;
    u_int64_t e;

    portmap_get(idx, &e);
    e &= ~(0x7 << (off * 3));
    e |= (port & 0x7) << (off * 3);
    portmap_set(idx, e);
}

/******************************************************************
 * apis
 */

int
pciehw_portmap_load(const u_int32_t lifb,
                    const u_int32_t lifc,
                    const u_int8_t port)
{
    u_int32_t lif;

    for (lif = lifb; lif < lifb + lifc; lif++) {
        portmap_setlif(lif, port);
    }
    return 0;
}

int
pciehw_portmap_unload(const u_int32_t lifb,
                      const u_int32_t lifc)
{
    /* nothing to do */
    return 0;
}

void
pciehw_portmap_init(void)
{
    int i;

    for (i = 0; i < portmap_size(); i++) {
        portmap_set(i, 0);
    }
}

/******************************************************************
 * debug
 */

static void
portmap_show(const int raw)
{
    u_int64_t e;
    int i, l;

    pciesys_loginfo("portmap_size() = %d\n", portmap_size());

    pciesys_loginfo("%-4s  %-7s\n", "lif", "port");
    for (i = 0; i < portmap_size(); i++) {
        portmap_get(i, &e);
        pciesys_loginfo("%4d:", i * 16);
        for (l = 0; l < 16; l++) {
           const u_int8_t port = (e >> (l * 3)) & 0x7;
            pciesys_loginfo(" %1d%s", port, l == 7 ? " " : "");
        }
        pciesys_loginfo("\n");
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
