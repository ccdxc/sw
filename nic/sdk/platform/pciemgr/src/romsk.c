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

#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

#define ROMSK_BASE      PXB_(DHS_TGT_ROMASK)
#define ROMSK_COUNT     ASIC_(PXB_CSR_DHS_TGT_ROMASK_ENTRY_ARRAY_COUNT)
#define ROMSK_STRIDE    8

static u_int64_t
romsk_addr(const int index)
{
    assert(index >= 0 && index < ROMSK_COUNT);
    return ROMSK_BASE + (index * ROMSK_STRIDE);
}

static pciehw_sromsk_t *
pciehw_sromsk(const u_int32_t index)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();

    assert(pshmem);
    assert(index < PCIEHW_NROMSK);
    return &pshmem->sromsk[index];
}

static void
pciehw_romsk_get(const int index, u_int32_t *entry)
{
    u_int32_t w[2];

    pal_reg_rd32w(romsk_addr(index), w, 2);
    *entry = w[0];
}

static void
pciehw_romsk_set(const int index, const u_int32_t entry)
{
    const u_int32_t w[2] = { entry, };
    pal_reg_wr32w(romsk_addr(index), w, 2);
}

/*
 * Install an entry in hardware and shadow copy in software.
 */
static void
pciehw_romsk_install(const int index, const u_int32_t entry)
{
    pciehw_sromsk_t *sromsk = pciehw_sromsk(index);

    sromsk->entry = entry;
    sromsk->count++;
    pciehw_romsk_set(index, entry);
}

/*
 * Insert an entry in the romsk table, return the index in the table
 * or -1 if table full.
 */
static int
pciehw_romsk_insert(const u_int32_t entry)
{
    pciehw_sromsk_t *sromsk;
    int i, avail;

    avail = -1;
    sromsk = pciehw_sromsk(0);
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
        pciehw_romsk_install(avail, entry);
        return avail;
    }
    /*
     * New entry but we didn't have room for it.
     */
    return -1;
}

static void
pciehw_romsk_delete(const int index)
{
    pciehw_sromsk_t *sromsk = pciehw_sromsk(index);

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
pciehw_romsk_load(pciehwdev_t *phwdev)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    u_int32_t *cfgmskdw = (u_int32_t *)&pshmem->cfgmsk[hwdevh];
    int i = 0;

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        /* if indirect, we handle in sw so don't need romsk entry */
        if ((phwdev->cfgpmtf[i] & PMTF_INDIRECT) == 0) {
            int index = pciehw_romsk_insert(cfgmskdw[i]);
            if (index < 0) {
                pciesys_logerror("romsk_insert failed for dev %s\n",
                                 pciehwdev_get_name(phwdev));
                goto error_out;
            }
            phwdev->romsksel[i] = index;
        }
    }
    return 0;

 error_out:
    for ( ; i >= 0; i--) {
        pciehw_romsk_delete(phwdev->romsksel[i]);
    }
    return -1;
}

void
pciehw_romsk_unload(pciehwdev_t *phwdev)
{
    int i;

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        pciehw_romsk_delete(phwdev->romsksel[i]);
    }
}

void
pciehw_romsk_init(void)
{
    int index;

    for (index = 0; index < PCIEHW_NROMSK; index++) {
        pciehw_romsk_set(index, 0);
    }

    /*
     * Add entry=0xffffffff, a full writable entry.
     * index=0 selects romsk bypass in PMT entries
     * so all bits are writable.
     */
    index = pciehw_romsk_insert(0xffffffff);
    assert(index == 0);

    /*
     * add entry=0, a read-only entry
     * We add it first so we get a predictable index=1 for this
     * entry so we can use it for read-only catchall PMT entries.
     */
    index = pciehw_romsk_insert(0);
    assert(index == ROMSK_RDONLY);
}

/******************************************************************
 * debug
 */

void
pciehw_romsk_dbg(int argc, char *argv[])
{
    pciehw_sromsk_t *sromsk = pciehw_sromsk(0);
    u_int32_t entry[4];
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

    pciesys_loginfo("%3s  %-8s %-5s %-8s %-5s %-8s %-5s %-8s %-5s\n",
                    "idx",
                    "entry", "count",
                    "entry", "count",
                    "entry", "count",
                    "entry", "count");
    for (i = 0; i < PCIEHW_NROMSK; i += 4) {
        pciehw_romsk_get(i,   &entry[0]);
        pciehw_romsk_get(i+1, &entry[1]);
        pciehw_romsk_get(i+2, &entry[2]);
        pciehw_romsk_get(i+3, &entry[3]);

        if (all ||
            sromsk[i].count ||
            sromsk[i].count ||
            sromsk[i].count ||
            sromsk[i].count) {
            pciesys_loginfo("%3d: %08x %5d %08x %5d %08x %5d %08x %5d\n",
                            i,
                            entry[0], sromsk[i  ].count,
                            entry[1], sromsk[i+1].count,
                            entry[2], sromsk[i+2].count,
                            entry[3], sromsk[i+3].count);
        }
    }
}
