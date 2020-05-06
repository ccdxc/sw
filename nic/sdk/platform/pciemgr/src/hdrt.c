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
#include <sys/param.h>

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"
#include "hdrt.h"

#define HDRT_BASE       PXB_(DHS_ITR_PCIHDRT)
#define HDRT_COUNT      ASIC_(PXB_CSR_DHS_ITR_PCIHDRT_ENTRIES)
#define HDRT_STRIDE     ASIC_(PXB_CSR_DHS_ITR_PCIHDRT_ENTRY_BYTE_SIZE)
#define HDRT_NWORDS     3

static int
hdrt_size(void)
{
    return HDRT_COUNT;
}

static u_int64_t
hdrt_addr(const u_int32_t lif)
{
    assert(lif < hdrt_size());
    return HDRT_BASE + (lif * HDRT_STRIDE);
}

static void
hdrt_get(const u_int32_t lif, hdrt_t *hdrt)
{
    pal_reg_rd32w(hdrt_addr(lif), (u_int32_t *)hdrt, HDRT_NWORDS);
}

static void
hdrt_set(const u_int32_t lif, const hdrt_t *hdrt)
{
    pal_reg_wr32w(hdrt_addr(lif), (u_int32_t *)hdrt, HDRT_NWORDS);
}

static void
hdrt_set_itr(const u_int32_t lif, const u_int16_t bdf)
{
    hdrt_t h = { 0 };

    h.valid = 1;
    h.bdf = bdf;
    h.attr2_1_rd = 0x1; /* reads get Relaxed Ordering */
    hdrt_set(lif, &h);
}

/******************************************************************
 * apis
 */

int
pciehw_hdrt_load(const u_int32_t lifb,
                 const u_int32_t lifc,
                 const u_int16_t bdf)
{
    u_int32_t lif;

    for (lif = lifb; lif < lifb + lifc; lif++) {
        hdrt_set_itr(lif, bdf);
    }
    return 0;
}

int
pciehw_hdrt_unload(const u_int32_t lifb, const u_int32_t lifc)
{
    const hdrt_t h0 = { 0 };
    u_int32_t lif;

    for (lif = lifb; lif < lifb + lifc; lif++) {
        hdrt_set(lif, &h0);
    }
    return 0;
}

void
pciehw_hdrt_init(void)
{
    const hdrt_t h = { 0 };
    int i;

    for (i = 0; i < hdrt_size(); i++) {
        hdrt_set(i, &h);
    }
}

/******************************************************************
 * debug
 */

static void
hdrt_show_hdr(void)
{
    pciesys_loginfo("%-4s %-7s\n",
                    "lif", "bdf");
}

static void
hdrt_show_entry(const u_int32_t lif, hdrt_t *h)
{
    pciesys_loginfo("%-4d %-7s\n",
                    lif, bdf_to_str(h->bdf));
}

static void
hdrt_show(void)
{
    hdrt_t h;
    int i;

    hdrt_show_hdr();
    for (i = 0; i < hdrt_size(); i++) {
        hdrt_get(i, &h);
        if (h.valid) {
            hdrt_show_entry(i, &h);
        }
    }
}

void
pciehw_hdrt_dbg(int argc, char *argv[])
{
    int opt;

    optind = 0;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        default:
            return;
        }
    }

    hdrt_show();
}

void
pciehw_hdrt_show(int argc, char *argv[])
{
    hdrt_show();
}
