/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

#include "platform/src/lib/pciemgr/include/pciehsvc.h"
#include "platform/src/lib/misc/include/maclib.h"
#include "platform/src/lib/misc/include/bdf.h"
#include "platform/src/sim/libsimlib/include/simserver.h"
#include "platform/src/sim/libsimdev/include/simdevices.h"

typedef struct simctx_s {
    int serverfd;
    int clientfd;
    char user[32];
} simctx_t;

static simctx_t simctx;
static int verbose_flag;

static void
verbose(const char *fmt, ...)
{
    va_list arg;

    if (verbose_flag) {
        va_start(arg, fmt);
        vprintf(fmt, arg);
        va_end(arg);
    }
}

simctx_t *
simctx_get(void)
{
    return &simctx;
}

void 
simd_set_user(const char *user)
{
    simctx_t *sc = simctx_get();

    verbose("================\n");
    verbose("init %s\n", user);
    verbose("----------------\n");
    strncpy(sc->user, user, sizeof(sc->user) - 1);
}

static void
simd_log(const char *fmt, va_list ap)
{
    if (verbose_flag) {
        vprintf(fmt, ap);
    }
}

static void
simd_error(const char *fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}

static int
simd_doorbell(u_int64_t addr, u_int64_t data)
{
    return 0;
}

static int
simd_read_reg(u_int64_t addr, u_int32_t *data)
{
    *data = 0;
    return 0;
}

static int
simd_write_reg(u_int64_t addr, u_int32_t data)
{
    return 0;
}

static char membuf[4096];

static int
simd_read_mem(u_int64_t addr, void *buf, size_t size)
{
    memcpy(buf, &membuf[addr % sizeof(membuf)], size);
    return 0;
}

static int
simd_write_mem(u_int64_t addr, void *buf, size_t size)
{
    memcpy(&membuf[addr % sizeof(membuf)], buf, size);
    return 0;
}

static simdev_api_t simd_api = {
    .set_user  = simd_set_user,
    .log       = simd_log,
    .error     = simd_error,
    .doorbell  = simd_doorbell,
    .read_reg  = simd_read_reg,
    .write_reg = simd_write_reg,
    .read_mem  = simd_read_mem,
    .write_mem = simd_write_mem,
};

static void
sighand(int s)
{
    simctx_t *sc = simctx_get();

    sims_close_client(sc->clientfd);
    sims_close(sc->serverfd);
    exit(0);
}

int
main(int argc, char *argv[])
{
    simctx_t *sc = simctx_get();
    int a;

    if (simdev_open(&simd_api) < 0) {
        fprintf(stderr, "simdev_open failed\n");
        exit(1);
    }

    for (a = 1; a < argc; a++) {
        /* -v */
        if (strcmp(argv[a], "-v") == 0) {
            verbose_flag = 1;
        }
        /* -d <devparams> */
        if (strcmp(argv[a], "-d") == 0) {
            a++;
            if (a >= argc) {
                fprintf(stderr, "-d missing arg\n");
                exit(1);
            }
            if (simdev_add_dev(argv[a]) < 0) {
                fprintf(stderr, "simdev_add_dev %s failed\n", argv[a]);
                exit(1);
            }
        }
    }

    pciehsvc_open(NULL);
    sc->serverfd = sims_open(NULL, simdev_msg_handler);
    if (sc->serverfd < 0) {
        exit(1);
    }

    signal(SIGINT, sighand);
    signal(SIGTERM, sighand);
    signal(SIGQUIT, sighand);

    while (1) {
        int r;

        verbose("waiting for clients to connect...\n");

        sc->clientfd = sims_open_client(sc->serverfd);

        while ((r = sims_client_recv_and_handle(sc->clientfd)) > 0)
            continue;

        if (r < 0) {
            perror("sims_client_recv_and_handle");
        }
        sims_close_client(sc->clientfd);
        verbose("%s closed\n", sc->user);
    }
    sims_close(sc->serverfd);
    pciehsvc_close();
    exit(0);
}
