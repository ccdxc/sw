/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <cinttypes>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#include "cmd.h"

typedef union {
    struct {
        uint64_t portmap_sram_ecc_err:1;
        uint64_t unsupported_err:1;
        uint64_t addrdw:50;
        uint64_t lif:11;
        uint64_t aspace:1;

        uint64_t host_be:2;
        uint64_t arsize:3;
        uint64_t arlen:4;
        uint64_t dword_cnt:7;
    } __attribute__((packed));
    uint32_t w[3];
} rdhdr_t;

typedef union {
    struct {
        uint64_t portmap_sram_ecc_err:1;
        uint64_t unsupported_err:1;
        uint64_t addrdw:50;
        uint64_t lif:11;
        uint64_t aspace:1;

        uint64_t pcie_msg:1;
        uint64_t last_byte_en:4;
        uint64_t first_byte_en:4;
        uint64_t dword_cnt:7;
    } __attribute__((packed));
    uint32_t w[3];
} wrhdr_t;

#define RDHDR_BASE      PXB_(DHS_ITR_RDHDR_ENTRY)
#define RDHDR_STRIDE    16
#define RDHDR_NENTRIES  CAP_PXB_CSR_DHS_ITR_RDHDR_ENTRY_ARRAY_COUNT

#define WRHDR_BASE      PXB_(DHS_ITR_WRHDR_ENTRY)
#define WRHDR_STRIDE    16
#define WRHDR_NENTRIES  CAP_PXB_CSR_DHS_ITR_WRHDR_ENTRY_ARRAY_COUNT

static uint64_t
rdhdr_addr(const uint32_t entry)
{
    return RDHDR_BASE + (entry * RDHDR_STRIDE);
}

static uint64_t
wrhdr_addr(const uint32_t entry)
{
    return WRHDR_BASE + (entry * WRHDR_STRIDE);
}

static void
rdhdr_read(const uint32_t entry, rdhdr_t *h)
{
    pal_reg_rd32w(rdhdr_addr(entry), h->w, 3);
}

static void
wrhdr_read(const uint32_t entry, wrhdr_t *h)
{
    pal_reg_rd32w(wrhdr_addr(entry), h->w, 3);
}

static void
rwhdr_raw(const uint32_t entry, const uint32_t *w)
{
    printf("%u: 0x%08x%08x%08x\n", entry, w[2], w[1], w[0]);
}

static void
rdhdr_raw(const uint32_t entry, rdhdr_t *h)
{
    rwhdr_raw(entry, h->w);
}

static void
wrhdr_raw(const uint32_t entry, wrhdr_t *h)
{
    rwhdr_raw(entry, h->w);
}

static void
rdhdr_show(const uint32_t entry, rdhdr_t *h)
{
    printf("%u: lif=%-4" PRIu64 " addr=0x%013" PRIx64 " size=%" PRIu32 "\n",
           entry, h->lif, h->addrdw << 2, h->dword_cnt << 2);
}

static void
wrhdr_show(const uint32_t entry, wrhdr_t *h)
{
    printf("%u: lif=%-4" PRIu64 " addr=0x%013" PRIx64 " size=%" PRIu32 " %s\n",
           entry, h->lif, h->addrdw << 2, h->dword_cnt << 2,
           h->pcie_msg ? "pcie_msg" : "");
}

/*
 * Display the itr_rdhdr_entry table of pending reads to the host.
 * Note that this list is not a ring but is indexed by axi id (pcie tag?).
 * The id's are allocated in "first available" order starting at 0.
 * So the entry at 0 gets overwritten often and the upper entries
 * are used less frequently.
 */
static void
show_rdhdr(const int raw)
{
    rdhdr_t rdhdr[RDHDR_NENTRIES];
    int i;

    for (i = 0; i < RDHDR_NENTRIES; i++) {
        rdhdr_read(i, &rdhdr[i]);
    }
    for (i = 0; i < RDHDR_NENTRIES; i++) {
        if (raw) {
            rdhdr_raw(i, &rdhdr[i]);
        } else {
            rdhdr_show(i, &rdhdr[i]);
        }
    }
}

/*
 * Display the itr_wrhdr_entry table of writes to the host.
 * Note that this list is not a ring but is indexed by axi id (pcie tag?).
 * As with the rdhdr table the id's are allocated in "first available"
 * order starting at 0. So the entry at 0 gets overwritten often and
 * the upper entries are used less frequently.  Furthermore, writes
 * are acknowledged locally since there is no response from the host.
 * That means writes stay in the table for much less time and the
 * id will likely get reallocated soon and the table overwritten.
 */
static void
show_wrhdr(const int raw)
{
    wrhdr_t wrhdr[WRHDR_NENTRIES];
    int i;

    for (i = 0; i < WRHDR_NENTRIES; i++) {
        wrhdr_read(i, &wrhdr[i]);
    }
    for (i = 0; i < WRHDR_NENTRIES; i++) {
        if (raw) {
            wrhdr_raw(i, &wrhdr[i]);
        } else {
            wrhdr_show(i, &wrhdr[i]);
        }
    }
}

static void
itr_rwhdr(int argc, char *argv[])
{
    int opt, showrd, showwr, raw;

    showrd = 0;
    showwr = 0;
    raw = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "rwR")) != -1) {
        switch (opt) {
        case 'r':
            showrd = 1;
            break;
        case 'w':
            showwr = 1;
            break;
        case 'R':
            raw = 1;
            break;
        default:
            return;
        }
    }
    if (!showrd && !showwr) {
        showrd = 1;
        showwr = 1;
    }

    if (showrd) {
        printf("==== itr_rdhdr ====\n");
        show_rdhdr(raw);
    }
    if (showwr) {
        printf("==== itr_wrhdr ====\n");
        show_wrhdr(raw);
    }
}
CMDFUNC(itr_rwhdr,
"show itr_rd/wrhdr_entry[]",
"itr_rwhdr [-rwR]\n"
"    -r         idr_rdhdr only (default both)\n"
"    -w         idr_wrhdr only (default both)\n"
"    -R         raw format\n");
