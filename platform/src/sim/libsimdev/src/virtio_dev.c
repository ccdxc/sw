/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simserver.h"
#include "src/sim/libsimdev/src/dev_utils.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

typedef struct devparams_s {
    int lif;
    int adq_type;
    int adq_count;
    int adq_qidbase;
    int txq_type;
    int txq_count;
    int txq_qidbase;
    int rxq_type;
    int rxq_count;
    int rxq_qidbase;
    int intr_base;
    int intr_count;
} devparams_t;

static simdev_t *current_sd;

static int
dev_lif(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->lif;
}

static int
dev_intrb(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->intr_base;
}

static int
bar0_rd(int bar, u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    u_int64_t base = 0xc1000000;
    u_int32_t val;

    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

static int
bar0_wr(int bar, u_int64_t offset, u_int8_t size, u_int64_t val)
{
    u_int64_t base = 0xc1000000;

    if (simdev_write_reg(base + offset, val) < 0) {
        return -1;
    }
    return 0;
}

static int
bar1_rd(int bar, u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    int r = 0;

    if (offset < 0x1000) {
        const int intrb = dev_intrb(current_sd);
        r = msixtbl_rd(intrb, offset, size, valp);
    } else {
        const int lif = dev_lif(current_sd);
        r = msixpba_rd(lif, offset - 0x1000, size, valp);
    }
    return r;
}

static int
bar1_wr(int bar, u_int64_t offset, u_int8_t size, u_int64_t val)
{
    int r = 0;

    if (offset < 0x1000) {
        const int intrb = dev_intrb(current_sd);
        r = msixtbl_wr(intrb, offset, size, val);
    } else {
        const int lif = dev_lif(current_sd);
        r = msixpba_wr(lif, offset - 0x1000, size, val);
    }
    return r;
}

/*
 * ================================================================
 * process messages
 * ----------------------------------------------------------------
 */

static int
virtio_memrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;
    int r;

    current_sd = sd;

    switch (bar) {
    case 1:
        r = bar1_rd(bar, addr, size, valp);
        break;
    default:
        simdev_error("memrd: unhandled bar %d\n", bar);
        r = -1;
        break;
    }

    if (r < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EFAULT);
        return -1;
    }

    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

static void
virtio_memwr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const u_int8_t  bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    switch (bar) {
    case 1:
        bar1_wr(bar, addr, size, val);
        break;
    default:
        simdev_error("memwr: unhandled bar %d\n", bar);
        break;
    }
    sims_writeres(sd->fd, bdf, bar, addr, size, 0);
}

static int
virtio_iord(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;
    int r;

    current_sd = sd;

    switch (bar) {
    case 0:
        r = bar0_rd(bar, addr, size, valp);
        break;
    default:
        simdev_error("iord: unhandled bar %d\n", bar);
        r = -1;
        break;
    }

    if (r < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EFAULT);
        return -1;
    }

    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

static void
virtio_iowr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const u_int8_t  bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    switch (bar) {
    case 0:
        bar0_wr(bar, addr, size, val);
        break;
    default:
        simdev_error("iowr: unhandled bar %d\n", bar);
        break;
    }
    sims_writeres(sd->fd, bdf, bar, addr, size, 0);
}

static int
virtio_init(simdev_t *sd, const char *devparams)
{
    devparams_t *dp;

    if (devparam_str(devparams, "help", NULL, 0) == 0) {
        simdev_error("virtio params:\n"
                     "    lif=<lif>\n"
                     "    adq_type=<adq_type>\n"
                     "    adq_count=<adq_count>\n"
                     "    adq_qidbase=<adq_qidbase>\n"
                     "    txq_type=<txq_type>\n"
                     "    txq_count=<txq_count>\n"
                     "    txq_qidbase=<txq_qidbase>\n"
                     "    rxq_type=<rxq_type>\n"
                     "    rxq_count=<rxq_count>\n"
                     "    rxq_qidbase=<rxq_qidbase>\n"
                     "    intr_base=<intr_base>\n"
                     "    intr_count=<intr_count>\n");
        return -1;
    }

    dp = calloc(1, sizeof(devparams_t));
    if (dp == NULL) {
        simdev_error("devparams alloc failed: no mem\n");
        return -1;
    }
    sd->priv = dp;

#define GET_PARAM(P, TYP) \
    devparam_##TYP(devparams, #P, &dp->P)

    GET_PARAM(lif, int);
    GET_PARAM(adq_type, int);
    GET_PARAM(adq_count, int);
    GET_PARAM(adq_qidbase, int);
    GET_PARAM(txq_type, int);
    GET_PARAM(txq_count, int);
    GET_PARAM(txq_qidbase, int);
    GET_PARAM(rxq_type, int);
    GET_PARAM(rxq_count, int);
    GET_PARAM(rxq_qidbase, int);
    GET_PARAM(intr_base, int);
    GET_PARAM(intr_count, int);

    return 0;
}

static void
virtio_free(simdev_t *sd)
{
    free(sd->priv);
    sd->priv = NULL;
}

dev_ops_t virtio_ops = {
    .init  = virtio_init,
    .free  = virtio_free,
    .cfgrd = generic_cfgrd,
    .cfgwr = generic_cfgwr,
    .memrd = virtio_memrd,
    .memwr = virtio_memwr,
    .iord  = virtio_iord,
    .iowr  = virtio_iowr,
};
