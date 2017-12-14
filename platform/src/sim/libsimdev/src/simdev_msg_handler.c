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
#include <errno.h>
#include <sys/types.h>

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simclient.h"
#include "src/sim/libsimdev/src/simdev_impl.h"
#include "src/sim/libsimdev/include/simdevices.h"

static void
simdev_handler(simdev_t *sd, simmsg_t *m)
{
    dev_ops_t *ops = sd->ops;
    u_int64_t val;

    switch (m->msgtype) {
    case SIMMSG_CFGRD:
        if (ops->cfgrd(sd, m, &val) == 0) {
            simdev_log("CFGRD[%s] 0x%02"PRIx64"  = 0x%0*"PRIx64"\n",
                       bdf_to_str(m->u.read.bdf),
                       m->u.read.addr, m->u.read.size * 2, val);
        } else {
            simdev_log("CFGRD[%s] 0x%"PRIx64" size %d failed\n",
                       bdf_to_str(m->u.read.bdf),
                       m->u.read.addr, m->u.read.size);
        }
        break;
    case SIMMSG_CFGWR:
        simdev_log("CFGWR[%s] 0x%02"PRIx64" <= 0x%0*"PRIx64"\n",
                   bdf_to_str(m->u.write.bdf),
                   m->u.write.addr, m->u.write.size * 2, m->u.write.val);
        ops->cfgwr(sd, m);
        break;
    case SIMMSG_MEMRD:
        if (ops->memrd(sd, m, &val) == 0) {
            simdev_log("MEMRD 0x%08"PRIx64"  = 0x%0*"PRIx64"\n",
                       m->u.read.addr, m->u.read.size * 2, val);
        } else {
            simdev_log("MEMRD 0x%08"PRIx64" size %d failed\n",
                       m->u.read.addr, m->u.read.size);
        }
        break;
    case SIMMSG_MEMWR:
        simdev_log("MEMWR 0x%08"PRIx64" <= 0x%0*"PRIx64"\n",
                   m->u.write.addr, m->u.write.size * 2, m->u.write.val);
        ops->memwr(sd, m);
        break;
    case SIMMSG_IORD:
        if (ops->iord(sd, m, &val) == 0) {
            simdev_log("IORD 0x%08"PRIx64"  = 0x%0*"PRIx64"\n",
                       m->u.read.addr, m->u.read.size * 2, val);
        } else {
            simdev_log("IORD 0x%08"PRIx64" size %d failed\n",
                       m->u.read.addr, m->u.read.size);
        }
        break;
    case SIMMSG_IOWR:
        simdev_log("IOWR 0x%08"PRIx64" <= 0x%0*"PRIx64"\n",
                   m->u.write.addr, m->u.write.size * 2, m->u.write.val);
        ops->iowr(sd, m);
        break;
    default:
        simdev_error("simdev_handler: unknown msg type %d\n", m->msgtype);
        break;
    }
}

void
simdev_msg_handler(int fd, simmsg_t *m)
{
    simdev_t *sd;

    switch (m->msgtype) {
    case SIMMSG_INIT:
        simdev_set_user(m->u.init.name);
        break;
    case SIMMSG_WRRESP:
        simdev_log("WRRESP 0x%08"PRIx64"\n", m->u.write.addr);
        break;
    case SIMMSG_SYNC_REQ:
        simdev_log("SYNC_REQ\n");
        simc_sync_ack();
        break;
    case SIMMSG_CFGRD:
    case SIMMSG_CFGWR:
    case SIMMSG_MEMRD:
    case SIMMSG_MEMWR:
    case SIMMSG_IORD:
    case SIMMSG_IOWR: {
        const u_int16_t bdf = m->u.generic.bdf;
        sd = simdev_by_bdf(bdf);
        if (sd == NULL) {
            sd = genericdev_get(bdf);
        }
        sd->fd = fd;
        simdev_handler(sd, m);
        break;
    }
    default:
        simdev_error("simdev_msg_handler: unknown msg type %d\n", m->msgtype);
        break;
    }
}
