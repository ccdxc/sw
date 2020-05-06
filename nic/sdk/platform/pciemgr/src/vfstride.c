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

#define VFSTRIDE_BASE   PXB_(CFG_TGT_VF_CFG_STRIDE)
#define VFSTRIDE_COUNT  ASIC_(PXB_CSR_CFG_TGT_VF_CFG_STRIDE_ARRAY_COUNT)
#define VFSTRIDE_STRIDE \
    (ASIC_(PXB_CSR_CFG_TGT_VF_CFG_STRIDE_ARRAY_ELEMENT_SIZE) * 4)

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
    /* hw verifies addr <= addrmaxdw, so subtract 1 dw from addrmax here */
    const u_int32_t addrmaxdw = (addrmax - 4) >> 2;
    union {
        struct {
            u_int32_t p:5;
            u_int32_t b:5;
            u_int32_t d:5;
            u_int32_t f:5;
            u_int32_t addrmaxdw:10;
#ifdef ASIC_ELBA
            u_int32_t psen:1;   /* port stride enable */
            u_int32_t bsen:1;   /* bus  stride enable */
            u_int32_t dsen:1;   /* dev  stride enable */
            u_int32_t fsen:1;   /* fnc  stride enable */
#endif
        } __attribute__((packed));
        u_int32_t w;
    } e;

    e.w = 0;

    assert(pshift < (1 << 5));
    assert(bshift < (1 << 5));
    assert(dshift < (1 << 5));
    assert(fshift < (1 << 5));

    e.p = pshift;
    e.b = bshift;
    e.d = dshift;
    e.f = fshift;
    assert((addrmax & 0x3) == 0);
    assert(addrmaxdw < (1 << 10));
    e.addrmaxdw = addrmaxdw;

    pal_reg_wr32(vfstride_addr(idx), e.w);
}

void
pciehw_vfstride_init(void)
{
    u_int8_t pshf, bshf, dshf, fshf;
    int i;

    for (i = 0; i < VFSTRIDE_COUNT; i++) {
        pal_reg_wr32(vfstride_addr(i), 0);
    }

    /* insert these entries */
    pshf = 0;
    fshf = (ffs(PCIEHW_CFGSZ) - 1) - 2; /* -2: dwords */
    dshf = fshf + 3;
    bshf = dshf + 8;
    vfstride_set(VFSTRIDE_IDX_DEVCFG, pshf, bshf, dshf, fshf, PCIEHW_CFGSZ);
    vfstride_set(VFSTRIDE_IDX_4K,     0, 0, 0, 0, 0x1000);
}
