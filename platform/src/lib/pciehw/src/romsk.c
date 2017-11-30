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

#include "pal.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "pciehsys.h"

static u_int64_t
romsk_addr(const int index)
{
#define ROMSK_BASE      \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_ROMASK_BYTE_ADDRESS)
#define ROMSK_COUNT     \
    CAP_PXB_CSR_DHS_TGT_ROMASK_ENTRY_ARRAY_COUNT
#define ROMSK_STRIDE    8

    assert(index >= 0 && index < ROMSK_COUNT);
    return ROMSK_BASE + (index * ROMSK_STRIDE);
}

static pciehw_sromsk_t *
pciehw_sromsk(pciehw_t *phw, const u_int32_t index)
{
    assert(phw->pciehwmem);
    assert(index < PCIEHW_NROMSK);
    return &phw->pciehwmem->sromsk[index];
}

static void
pciehw_romsk_get(pciehw_t *phw, const int index, u_int64_t *entry)
{
    *entry = pal_reg_rd64(romsk_addr(index));
}

static void
pciehw_romsk_set(pciehw_t *phw, const int index, const u_int64_t entry)
{
    pal_reg_wr64(romsk_addr(index), entry);
}

/*
 * Install an entry in hardware and shadow copy in software.
 */
static void
pciehw_romsk_install(pciehw_t *phw, const int index, const u_int64_t entry)
{
    pciehw_sromsk_t *sromsk = pciehw_sromsk(phw, index);

    sromsk->entry = entry;
    sromsk->count++;
    pciehw_romsk_set(phw, index, entry);
}

static int
pciehw_romsk_insert(pciehw_t *phw, const u_int64_t entry)
{
    pciehw_sromsk_t *sromsk;
    int i, avail;

    avail = -1;
    sromsk = pciehw_sromsk(phw, 0);
    for (i = 0; i < PCIEHW_NROMSK; i++, sromsk++) {
        /*
         * Entry matches an existing entry, just add another user
         * for this entry and return the same index.
         */
        if (sromsk->entry == entry) {
            sromsk->count++;
            return i;
        }
        /*
         * If this entry is unused, remember it in avail.
         * We use this first avail to add a new entry
         * if we need it below.
         */
        if (sromsk->count == 0 && avail < 0) {
            avail = i;
        }
    }
    /*
     * No existing entry matched, add a new entry
     * if we have an available slot.
     */
    if (avail >= 0) {
        pciehw_romsk_install(phw, avail, entry);
        return avail;
    }
    /*
     * New entry but we didn't have room for it.
     */
    return -1;
}

static void
pciehw_romsk_delete(pciehw_t *phw, const int index)
{
    pciehw_sromsk_t *sromsk = pciehw_sromsk(phw, index);

    /*
     * Is there any reason to remove unused entries
     * from hardware?  For now, just decrement count.
     */
    assert(sromsk->count > 0);
    sromsk->count--;
}

/******************************************************************
 * apis
 */

int
pciehw_romsk_load(pciehw_t *phw, pciehwdev_t *phwdev)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    u_int32_t *cfgmskdw = (u_int32_t *)&phwmem->cfgmsk[hwdevh];
    int i = 0;

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        int index = pciehw_romsk_insert(phw, cfgmskdw[i]);
        if (index < 0) {
            pciehsys_error("romsk_insert failed for dev %s\n", 
                           pciehwdev_get_name(phwdev));
            goto error_out;
        }
        phwdev->romsksel[i] = index;
    }
    return 0;

 error_out:
    for ( ; i >= 0; i--) {
        pciehw_romsk_delete(phw, cfgmskdw[i]);
    }
    return -1;
}

void
pciehw_romsk_unload(pciehw_t *phw, pciehwdev_t *phwdev)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    u_int32_t *cfgmskdw = (u_int32_t *)&phwmem->cfgmsk[hwdevh];
    int i;

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        pciehw_romsk_delete(phw, cfgmskdw[i]);
    }
}

void
pciehw_romsk_init(pciehw_t *phw)
{
    pciehw_sromsk_t *sromsk;
    int index;

    sromsk = pciehw_sromsk(phw, 0);
    pciehw_memset(sromsk, 0, sizeof(*sromsk));

    for (index = 0; index < PCIEHW_NROMSK; index++) {
        pciehw_romsk_set(phw, index, 0);
    }

    /*
     * Add entry=0xffffffff, a full writable entry.
     * index=0 selects romsk bypass in PMT entries
     * so all bits are writable.
     */
    index = pciehw_romsk_insert(phw, 0xffffffff);
    assert(index == 0);

    /*
     * add entry=0, a read-only entry
     * We add it first so we get a predictable index=1 for this
     * entry so we can use it for read-only catchall PMT entries.
     */
    index = pciehw_romsk_insert(phw, 0);
    assert(index == ROMSK_RDONLY);
}

/******************************************************************
 * debug
 */

void
pciehw_romsk_dbg(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_sromsk_t *sromsk = pciehw_sromsk(phw, 0);
    u_int64_t entry[4];
    int i, opt, all;

    all = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "a")) != -1) {
        switch (opt) {
        case 'a':
            all = 1;
            break;
        default:
            return;
        }
    }

    pciehsys_log("%3s  %-8s %-5s %-8s %-5s %-8s %-5s %-8s %-5s\n",
                 "idx",
                 "entry", "count",
                 "entry", "count",
                 "entry", "count",
                 "entry", "count");
    for (i = 0; i < PCIEHW_NROMSK; i += 4) {
        pciehw_romsk_get(phw, i,   &entry[0]);
        pciehw_romsk_get(phw, i+1, &entry[1]);
        pciehw_romsk_get(phw, i+2, &entry[2]);
        pciehw_romsk_get(phw, i+3, &entry[3]);

        if (all ||
            sromsk[i].count ||
            sromsk[i].count ||
            sromsk[i].count ||
            sromsk[i].count) {
            pciehsys_log("%3d: %08x %5d %08x %5d %08x %5d %08x %5d\n",
                         i,
                         (u_int32_t)entry[0], sromsk[i  ].count,
                         (u_int32_t)entry[1], sromsk[i+1].count,
                         (u_int32_t)entry[2], sromsk[i+2].count,
                         (u_int32_t)entry[3], sromsk[i+3].count);
        }
    }
}
