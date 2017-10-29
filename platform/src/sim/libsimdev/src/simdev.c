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
#include <sys/queue.h>
#include <sys/param.h>

#include "src/lib/misc/include/misc.h"
#include "src/lib/misc/include/bdf.h"
#include "src/lib/misc/include/maclib.h"
#include "src/sim/libsimdev/src/simdev_impl.h"
#include "src/sim/libsimdev/include/simdevices.h"

typedef struct simdevinfo_s {
    simdev_api_t *api;
    TAILQ_HEAD(, simdev_s) devlist;
} simdevinfo_t;

static simdevinfo_t simdevinfo;

simdevinfo_t *
simdevinfo_get(void)
{
    return &simdevinfo;
}

static simdev_api_t *
simdevinfo_api(void)
{
    simdevinfo_t *si = simdevinfo_get();
    return si->api;
}

static void
simdevinfo_api_set(simdev_api_t *api)
{
    simdevinfo_t *si = simdevinfo_get();
    si->api = api;
}

void
simdev_set_user(const char *user)
{
    simdev_api_t *api = simdevinfo_api();

    if (api && api->set_user) {
        api->set_user(user);
    }
}

void
simdev_log(const char *fmt, ...)
{
    simdev_api_t *api = simdevinfo_api();
    va_list arg;

    if (api && api->log) {
        va_start(arg, fmt);
        api->log(fmt, arg);
        va_end(arg);
    }
}

void
simdev_error(const char *fmt, ...)
{
    simdev_api_t *api = simdevinfo_api();
    va_list arg;

    if (api && api->log) {
        va_start(arg, fmt);
        api->error(fmt, arg);
        va_end(arg);
    }
}

int
simdev_doorbell(u_int64_t addr, u_int64_t data)
{
    simdev_api_t *api = simdevinfo_api();

    simdev_log("doorbell addr 0x%"PRIx64" data 0x%"PRIx64"\n", addr, data);
    if (api && api->doorbell) {
        return api->doorbell(addr, data);
    }
    return 0;
}

int
simdev_read_reg(u_int64_t addr, u_int32_t *data)
{
    simdev_api_t *api = simdevinfo_api();

    if (api && api->read_reg) {
        int r = api->read_reg(addr, data); 
        if (r == 0) {
            simdev_log("read_reg 0x08%"PRIx64" = 0x%x\n", addr, *data);
        } else {
            simdev_log("read_reg addr 0x%08"PRIx64" failed\n", addr);
        }
        return r;
    }
    return 0;
}

int
simdev_read_regs(u_int64_t addr, u_int32_t *data, int nw)
{
    int i;

    for (i = 0; i < nw; i++, addr += 4, data++) {
        if (simdev_read_reg(addr, data) < 0) {
            return -1;
        }
    }
    return 0;
}

int
simdev_write_reg(u_int64_t addr, u_int32_t data)
{
    simdev_api_t *api = simdevinfo_api();

    if (api && api->write_reg) {
        simdev_log("write_reg 0x08%"PRIx64" = 0x%x\n", addr, data);
        return api->write_reg(addr, data);
    }
    return 0;
}

int
simdev_write_regs(u_int64_t addr, u_int32_t *data, int nw)
{
    int i;

    for (i = 0; i < nw; i++, addr += 4, data++) {
        if (simdev_write_reg(addr, *data) < 0) {
            return -1;
        }
    }
    return 0;
}

static void
simdev_log_buf(u_int64_t addr, unsigned char *buf, size_t size)
{
    char fmtbuf[80];
    int offset, limited_size;

    limited_size = MIN(size, 256);
    for (offset = 0; offset < limited_size; offset += 16) {
        const int seglen = MIN(16, limited_size - offset);
        hex_format(fmtbuf, sizeof(fmtbuf), &buf[offset], seglen);
        simdev_log("    %08"PRIx64": %s\n", addr + offset, fmtbuf);
    }
    if (size > limited_size) {
        simdev_log("    %08"PRIx64": ...\n", addr + offset);
    }
}

int
simdev_read_mem(u_int64_t addr, void *buf, size_t size)
{
    simdev_api_t *api = simdevinfo_api();

    if (api && api->read_mem) {
        int r = api->read_mem(addr, buf, size);

        if (r == 0) {
            simdev_log("read_mem addr 0x%"PRIx64" size %ld\n", addr, size);
            simdev_log_buf(addr, buf, size);
        }
        return r;
    }
    return 0;
}

int
simdev_write_mem(u_int64_t addr, void *buf, size_t size)
{
    simdev_api_t *api = simdevinfo_api();

    if (api && api->write_mem) {
        simdev_log("write_mem addr 0x%"PRIx64" size %ld\n", addr, size);
        simdev_log_buf(addr, buf, size);
        return api->write_mem(addr, buf, size);
    }
    return 0;
}

static int
simdev_initialize(simdev_t *sd, const char *devparams)
{
    char type[32];
    u_int16_t bdf;

    if (devparam_str(devparams, "type", type, sizeof(type)) < 0) {
        simdev_error("missing device type\n");
        return -1;
    }
    if (devparam_bdf(devparams, "bdf", &bdf) < 0) {
        simdev_error("missing device bdf\n");
        return -1;
    }

    sd->bdf = bdf;
    if (strcmp(type, "eth") == 0) {
        sd->ops = &eth_ops;
    } else if (strcmp(type, "generic") == 0) {
        sd->ops = &generic_ops;
    } else {
        simdev_error("unknown device type: %s\n", type);
        return -1;
    }

    return sd->ops->init(sd, devparams);
}

static void
simdev_delete(simdev_t *sd)
{
    if (sd->ops) {
        sd->ops->free(sd);
    }
    free(sd);
}

static simdev_t *
simdev_new(const char *devparams)
{
    simdev_t *sd;

    sd = calloc(1, sizeof(simdev_t));
    if (sd == NULL) {
        simdev_error("simdev_new failed: no mem\n");
        return NULL;
    }
    if (simdev_initialize(sd, devparams) < 0) {
        simdev_delete(sd);
        return NULL;
    }
    return sd;
}

int
simdev_open(simdev_api_t *api)
{
    simdevinfo_t *si = simdevinfo_get();

    TAILQ_INIT(&si->devlist);
    simdevinfo_api_set(api);
    genericdev_init();
    return 0;
}

void
simdev_close(void)
{
    simdevinfo_t *si = simdevinfo_get();
    simdev_t *sd;

    while (!TAILQ_EMPTY(&si->devlist)) {
        sd = TAILQ_FIRST(&si->devlist);
        TAILQ_REMOVE(&si->devlist, sd, list);
        simdev_delete(sd);
    }
    simdevinfo_api_set(NULL);
}

simdev_t *
simdev_by_bdf(const u_int16_t bdf)
{
    simdevinfo_t *si = simdevinfo_get();
    simdev_t *sd = NULL;

    TAILQ_FOREACH(sd, &si->devlist, list) {
        if (sd->bdf == bdf) {
            break;
        }
    }
    return sd;
}

static int
simdev_add(simdev_t *sd)
{
    simdevinfo_t *si = simdevinfo_get();

    /*
     * Check for existing device at this bdf.
     * We want bdf to be unique.
     */
    if (simdev_by_bdf(sd->bdf) != NULL) {
        simdev_error("duplicate bdf: %s\n", bdf_to_str(sd->bdf));
        return -1;
    }

    TAILQ_INSERT_TAIL(&si->devlist, sd, list);
    return 0;
}

int
simdev_add_dev(const char *devparams)
{
    simdev_t *sd;

    sd = simdev_new(devparams);
    if (sd == NULL) {
        return -1;
    }
    if (simdev_add(sd) < 0) {
        simdev_delete(sd);
        return -1;
    }
    return 0;
}
