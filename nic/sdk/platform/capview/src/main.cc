
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "cli.h"
#include "dtls.h"
#include "dev.h"

void *init_sbrk;

static struct {
    char *dbpath;
    bool emit_db;
} opts;

static char *progname;

static void
exit_usage(void)
{
    fprintf(stderr, "usage: %s [options] file\n", progname);
    fprintf(stderr, "\t-f path     capviewdb.bin path\n"
                    "\t-d          dump database to stdout\n");
    exit(1);
}

static int
do_options(int argc, char *argv[])
{
    int c;

    while ((c = getopt(argc, argv, "f:d")) != -1) {
        switch (c) {
        case 'f':
            if (access(optarg, R_OK) < 0) {
                perror(argv[0]);
                exit(1);
            }
            opts.dbpath = strdup(optarg);
            break;

        case 'd':
            opts.emit_db = true;
            break;

        default:
            return -1;
        }
    }
    return 0;
}

static const char *
dbpath(void)
{
    static const char *pathtab[] = {
        "/pensando/etc/capviewdb.bin",
        "/platform/etc/capviewdb.bin",
        "/mnt/etc/capviewdb.bin",
        "capviewdb.bin",
        "/nic/etc/capviewdb.bin",
    };
    if (opts.dbpath != NULL) {
        return opts.dbpath;
    }
    for (size_t i = 0; i < sizeof (pathtab) / sizeof (pathtab[0]); i++) {
        if (access(pathtab[i], R_OK) == 0) {
            return pathtab[i];
        }
    }
    fprintf(stderr, "%% No database\n");
    exit(1);
}

static void
emit_db(void)
{
    for (int i = 0; i < CVDBReg::nregs; i++) {
        CVDBReg r = CVDBReg::get_byidx(i);
        printf("0x%08x: %s [width=%u, nrows=%u, stride=%u]\n", r.addr(),
                r.name(), r.width(), r.nrows(), r.stride());
        printf("  %d fields\n", r.nfields());
        for (int j = 0; j < r.nfields(); j++) {
            CVDBField f = r.field(j);
            printf("  [%3d:%3d] %s\n", f.hi(), f.lo(), f.name());
        }
    }
}

static void
check_chip_version(const char *path)
{
    CVDBReg reg = CVDBReg::get_version();
    uint32_t rval, chip_bld, chip_ver;
    dev_read(&rval, reg.addr(), 1);
    CVDBField cbfld = reg.field_byname("chip_build");
    CVDBField cvfld = reg.field_byname("chip_version");
    if (cbfld == CVDBField::end() || cvfld == CVDBField::end()) {
        printf("WARNING: %s: missing chip_build or "
                "chip_version in %s\n", path, reg.name());
        return;
    }
    chip_bld = (rval >> cbfld.lo()) & ((1 << (cbfld.hi() - cbfld.lo())) - 1);
    chip_ver = (rval >> cvfld.lo()) & ((1 << (cvfld.hi() - cvfld.lo())) - 1);
    if (chip_bld != CVDBReg::dbbuild() || chip_ver != CVDBReg::dbver()) {
        printf("WARNING: %s, version %u.%u does not match chip %u.%u\n",
                path, CVDBReg::dbbuild(), CVDBReg::dbver(), chip_bld, chip_ver);
    }
}

int
main(int argc, char *argv[])
{
    init_sbrk = sbrk(0);

    progname = argv[0];
    if (do_options(argc, argv) < 0) {
        exit_usage();
    }

    const char *path = dbpath();
    if (CVDBReg::loadfile(path) < 0) {
        return 1;
    }
    check_chip_version(path);
    if (opts.emit_db) {
        emit_db();
    }
    cli_run();
    return 0;
}
