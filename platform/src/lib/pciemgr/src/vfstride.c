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
#include "pciesys.h"
#include "pciehw_impl.h"

#define VFSTRIDE_BASE   \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + \
     CAP_PXB_CSR_CFG_TGT_VF_CFG_STRIDE_BYTE_ADDRESS)
#define VFSTRIDE_COUNT  \
    CAP_PXB_CSR_CFG_TGT_VF_CFG_STRIDE_ARRAY_COUNT
#define VFSTRIDE_STRIDE \
    (CAP_PXB_CSR_CFG_TGT_VF_CFG_STRIDE_ARRAY_ELEMENT_SIZE * 4)

static u_int64_t
vfstride_addr(const int idx)
{
    return VFSTRIDE_BASE + (idx * VFSTRIDE_STRIDE);
}

static void
vfstride_set(const int idx,
             const u_int8_t pshift,
             const u_int8_t bshift,
             const u_int8_t dshift,
             const u_int8_t fshift,
             const u_int32_t addrmax)
{
    union {
        struct {
            u_int32_t p:5;
            u_int32_t b:5;
            u_int32_t d:5;
            u_int32_t f:5;
            u_int32_t addrmaxdw:10;
        } __attribute__((packed));
        u_int32_t w;
    } e;

    e.p = pshift;
    e.b = bshift;
    e.d = dshift;
    e.f = fshift;
    assert((addrmax >> 2) < (1 << 10));
    assert((addrmax & 0x3) == 0);
    e.addrmaxdw = addrmax >> 2;

    pal_reg_wr32(vfstride_addr(idx), e.w);
}

int
pciehw_vfstride_load(pciehwdev_t *phwdev)
{
    /* XXX stub for now */
    vfstride_set(0, 0, 0, 0, 0, 0x400);
    phwdev->stridesel = 0;
    return 0;
}

void
pciehw_vfstride_unload(pciehwdev_t *phwdev)
{
    /* XXX */
}

void
pciehw_vfstride_init(void)
{
    int i;

    for (i = 0; i < VFSTRIDE_COUNT; i++) {
        pal_reg_wr32(vfstride_addr(i), 0);
    }
}
