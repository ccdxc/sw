/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "src/lib/pciehsvc/include/pciehsvc.h"
#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simserver.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

int
generic_cfgrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;
    u_int32_t val;

    if (pciehsvc_cfgrd(bdf, addr, size, &val) < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EINVAL);
        return -1;
    }
    sims_readres(sd->fd, bdf, bar, addr, size, val, 0);
    *valp = val;
    return 0;
}

void
generic_cfgwr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    if (pciehsvc_cfgwr(bdf, addr, size, val) < 0) {
        fprintf(stderr, "pciehsvc_cfgwr size %d addr 0x%"PRIx64" failed\n",
                size, addr);
    }
}

int
generic_memrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;

    if (pciehsvc_memrd(addr, size, valp) < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EINVAL);
        return -1;
    }        
    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

void
generic_memwr(simdev_t *sd, simmsg_t *m)
{
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    if (pciehsvc_memwr(addr, size, val) < 0) {
        fprintf(stderr, "pciehsvc_memwr size %d addr 0x%"PRIx64" failed\n",
                size, addr);
    }
}

int
generic_iord(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;
    u_int32_t val;

    if (pciehsvc_iord(addr, size, &val) < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EINVAL);
        return -1;
    }
    sims_readres(sd->fd, bdf, bar, addr, size, val, 0);
    *valp = val;
    return 0;
}

void
generic_iowr(simdev_t *sd, simmsg_t *m)
{
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    if (pciehsvc_iowr(addr, size, val) < 0) {
        fprintf(stderr, "pciehsvc_iowr addr 0x%"PRIx64" size %d failed\n",
                addr, size);
    }
}

int
generic_init(simdev_t *sd, const char *devparams)
{
    return 0;
}

void
generic_free(simdev_t *sd)
{
}

dev_ops_t generic_ops = {
    .init  = generic_init,
    .free  = generic_free,
    .cfgrd = generic_cfgrd,
    .cfgwr = generic_cfgwr,
    .memrd = generic_memrd,
    .memwr = generic_memwr,
    .iord  = generic_iord,
    .iowr  = generic_iowr,
};

static simdev_t genericdev;

int
genericdev_init(void)
{
    genericdev.ops = &generic_ops;
    return 0;
}

simdev_t *
genericdev_get(const u_int16_t bdf)
{
    genericdev.bdf = bdf;
    return &genericdev;
}
