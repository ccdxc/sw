/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
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
#include "pcietlp.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "req_int.h"

#define REQ_INT_NWORDS  3

typedef union req_int_u {
    struct {
        u_int64_t data:32;
        u_int64_t addrdw:34;
    } __attribute__((packed));
    u_int32_t w[REQ_INT_NWORDS];
} req_int_t;

void
req_int_set(const u_int64_t reg, const u_int64_t addr, const u_int32_t data)
{
    req_int_t in = { .data = data, .addrdw = addr >> 2 };

    assert((addr & 0x3) == 0);
    pal_reg_wr32w(reg, in.w, REQ_INT_NWORDS);
}

void
req_int_get(const u_int64_t reg, u_int64_t *addrp, u_int32_t *datap)
{
    req_int_t in;

    pal_reg_rd32w(reg, in.w, REQ_INT_NWORDS);
    *addrp = in.addrdw << 2;
    *datap = in.data;
}
