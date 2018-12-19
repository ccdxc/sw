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

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
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

/*
 * The pcie request hardware provides a single base register
 * CFG_TGT_REQ_*_INT.addrdw to configure notify interrupts.
 * Each per-port interrupt is sent to the address
 * (CFG_TGT_REQ_*_INT.adddw << 2) + (port * 4),
 * and the data is a constant for all ports from CFG_TGT_REQ_*_INT.data.
 *
 * This routine provides the abstraction that we can configure each
 * port independently.  When the first port is configured we set the
 * base port0 values for msgaddr0/msgdata0 and configure the hw to match.
 * Subsequent ports msgaddr/data are validated to be sure they match
 * what the hw will do.
 */
int
req_int_init(const u_int64_t reg, const char *label,
             const int port, u_int64_t msgaddr, u_int32_t msgdata)
{
    u_int64_t msgaddr0;
    u_int32_t msgdata0;
    int r = 0;

    /* validate port is within range */
    if (port < 0 || port >= PCIEHW_NPORTS) {
        pciesys_logerror("%s port%d invalid port\n", label, port);
        return -1;
    }

    /*
     * First time through set msgaddr0/data0 and hw to match.
     * Doesn't matter which port we configure first,
     * but subsequent ports must follow the pattern
     *     msgaddr = msgaddr0 + (port * 4)
     *     msgdata = msgdata0
     */
    req_int_get(reg, &msgaddr0, &msgdata0);
    if (port == 0 || msgaddr0 == 0) {
        msgaddr0 = msgaddr - (port * 4);
        msgdata0 = msgdata;
        req_int_set(reg, msgaddr0, msgdata0);
    }

    if (msgaddr != msgaddr0 + (port * 4)) {
        pciesys_logerror("%s port%d "
                         "msgaddr 0x%" PRIx64 " doesn't align with port0 0x%" PRIx64 "\n",
                         label, port,
                         msgaddr, msgaddr0);
        r = -1;
    }
    if (msgdata != msgdata0) {
        pciesys_logerror("%s port%d "
                         "msgdata 0x%x doesn't match port0 0x%x\n",
                         label, port,
                         msgdata, msgdata0);
        r = -1;
    }
    return r;
}
