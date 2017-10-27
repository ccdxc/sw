/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>

#include "simclient.h"
#include "bdf.h"

static int simfd = -1;

static int
strtoargv(char *s, char *argv[], const int maxac)
{
    int argc = 0;

    while (*s && argc < maxac-1) {
        if (isspace(*s)) { s++; continue; }
        argv[argc++] = s;               // save beginning of arg
        while (*s && !isspace(*s)) s++; // find end of arg
        if (*s) *s++ = '\0';            // null-terminate arg
    }
    argv[argc] = NULL;                  // null-terminate list
    return (argc);
}

static u_int8_t simmem[4096] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78 };

static int
process_memrd(int fd, simmsg_t *m)
{
    const u_int16_t bdf = m->u.read.bdf;
    const u_int64_t addr = m->u.read.addr;
    const u_int32_t size = m->u.read.size;

    if (addr + size > sizeof(simmem)) {
        simc_readres(fd, bdf, size, NULL, EINVAL);
        return -1;
    }

    return simc_readres(fd, bdf, size, &simmem[addr], 0);
}

static void
process_memwr(int fd, simmsg_t *m)
{
    const u_int64_t addr = m->u.write.addr;
    const u_int32_t size = m->u.write.size;

    if (addr + size <= sizeof(simmem)) {
        simc_readn(&simmem[addr], size);
    } else {
        simc_discard(size);
    }
}

static void
msg_handler(int fd, simmsg_t *m)
{
    switch (m->msgtype) {
    case SIMMSG_MEMRD:
        process_memrd(fd, m);
        break;
    case SIMMSG_MEMWR:
        process_memwr(fd, m);
        break;
    default:
        fprintf(stderr, "unknown msg type %d\n", m->msgtype);
        break;
    }
}

static int
t_open(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s <name>\n", argv[0]);
        return -1;
    }
    simfd = simc_open(argv[1], NULL, msg_handler);
    if (simfd < 0) {
        fprintf(stderr, "open failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

static int
t_cfgrd(int argc, char *argv[])
{
    u_int64_t addr, val;
    u_int16_t bdf, size;
    int r;

    if (argc != 4) {
        fprintf(stderr, "usage: %s <bdf> <addr> <size>\n", argv[0]);
        return -1;
    }
    bdf = bdf_from_str(argv[1]);
    addr = strtoul(argv[2], NULL, 0);
    size = strtoul(argv[3], NULL, 0);
    r = simc_cfgrd(bdf, addr, size, &val);
    if (r != 0) {
        fprintf(stderr, "simc_cfgrd(%s, 0x%"PRIx64" %d) failed\n",
                bdf_to_str(bdf), addr, size);
        return -1;
    }
    printf("simc_cfgrd(%s 0x%"PRIx64", %d) = 0x%"PRIx64"\n",
           bdf_to_str(bdf), addr, size, val);
    return 0;
}

static int
t_cfgwr(int argc, char *argv[])
{
    u_int64_t addr, val;
    u_int16_t bdf, size;
    int r;

    if (argc != 5) {
        fprintf(stderr, "usage: %s <bdf> <addr> <size> <val>\n", argv[0]);
        return -1;
    }
    bdf = bdf_from_str(argv[1]);
    addr = strtoul(argv[2], NULL, 0);
    size = strtoul(argv[3], NULL, 0);
    val = strtoul(argv[4], NULL, 0);
    r = simc_cfgwr(bdf, addr, size, val);
    if (r < 0) {
        fprintf(stderr,
                "simc_cfgwr(%s, 0x%"PRIx64", %d, 0x%"PRIx64") failed\n",
                bdf_to_str(bdf), addr, size, val);
        return -1;
    }
    return 0;
}

static int
t_memrd(int argc, char *argv[])
{
    u_int64_t addr, val;
    u_int16_t bdf, size;
    int bar, r;

    if (argc != 5) {
        fprintf(stderr, "usage: %s <bdf> <bar> <addr> <size>\n", argv[0]);
        return -1;
    }
    bdf = bdf_from_str(argv[1]);
    bar = strtoul(argv[2], NULL, 0);
    addr = strtoul(argv[3], NULL, 0);
    size = strtoul(argv[4], NULL, 0);
    r = simc_memrd(bdf, bar, addr, size, &val);
    if (r != 0) {
        fprintf(stderr, "simc_memrd(%s, %d, 0x%"PRIx64", %d) failed\n",
                bdf_to_str(bdf), bar, addr, size);
        return -1;
    }
    printf("simc_memrd(%s, %d, 0x%"PRIx64", %d) = 0x%"PRIx64"\n",
           bdf_to_str(bdf), bar, addr, size, val);
    return 0;
}

static int
t_memwr(int argc, char *argv[])
{
    u_int64_t addr, val;
    u_int16_t bdf, size;
    int bar, r;

    if (argc != 6) {
        fprintf(stderr,
                "usage: %s <bdf> <bar> <addr> <size> <val>\n", argv[0]);
        return -1;
    }
    bdf = bdf_from_str(argv[1]);
    bar = strtoul(argv[2], NULL, 0);
    addr = strtoul(argv[3], NULL, 0);
    size = strtoul(argv[4], NULL, 0);
    val = strtoul(argv[5], NULL, 0);
    r = simc_memwr(bdf, bar, addr, size, val);
    if (r < 0) {
        fprintf(stderr,
                "simc_barwr(%s, %d, 0x%"PRIx64", %d, 0x%"PRIx64") failed\n",
                bdf_to_str(bdf), bar, addr, size, val);
        return -1;
    }
    return 0;
}

static int
t_iord(int argc, char *argv[])
{
    return 0;
}

static int
t_iowr(int argc, char *argv[])
{
    return 0;
}

static int
t_close(int argc, char *argv[])
{
    if (argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return -1;
    }
    simc_close();
    simfd = -1;
    return 0;
}

typedef struct cmdtab_s {
    const char *name;
    int (*func)(int argc, char *argv[]);
} cmdtab_t;

static cmdtab_t cmds[] = {
#define CMDENT(name) \
    { #name, t_##name }
    CMDENT(open),
    CMDENT(close),
    CMDENT(cfgrd),
    CMDENT(cfgwr),
    CMDENT(memrd),
    CMDENT(memwr),
    CMDENT(iord),
    CMDENT(iowr),
    { NULL, NULL }
};

static void
process_cmdline(void)
{
    char line[80];
    cmdtab_t *c;
    int ac;
    char *av[8];

    if (fgets(line, sizeof(line), stdin) != NULL) {
        ac = strtoargv(line, av, sizeof(av) / sizeof (av[0]));
        if (ac) {
            for (c = cmds; c->name; c++) {
                if (strcmp(c->name, av[0]) == 0) {
                    c->func(ac, av);
                    break;
                }
            }
            if (c->name == NULL) {
                fprintf(stderr, "%s: unknown command\n", av[0]);
            }
        }
        fputs("simclient: ", stdout); fflush(stdout);
    } else {
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
    fd_set rfds;
    int maxfd, r;

    fputs("simclient> ", stdout); fflush(stdout);
    while (1) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        maxfd = 0;
        if (simfd >= 0) {
            FD_SET(simfd, &rfds);
            maxfd = simfd;
        }

        r = select(maxfd + 1, &rfds, NULL, NULL, NULL);

        if (r < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(0, &rfds)) {
            process_cmdline();
        }
        if (simfd >= 0 && FD_ISSET(simfd, &rfds)) {
            simc_recv_and_handle();
        }
    }
    exit(0);
}
