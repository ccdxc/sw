/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#include "src/lib/misc/include/bdf.h"
#include "src/lib/misc/include/maclib.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

enum { BDF, MAC, INT, U64, STR };

static void
usage(void)
{
    fprintf(stderr, "Usage: devparams [-bmi] <devparams> <key>\n");
}

int
main(int argc, char *argv[])
{
    char *devparams;
    char *key;
    int opt, mode;

    mode = STR;
    while ((opt = getopt(argc, argv, "bmisL")) != -1) {
        switch (opt) {
        case 'b': mode = BDF; break;
        case 'm': mode = MAC; break;
        case 'i': mode = INT; break;
        case 'L': mode = U64; break;
        case 's': mode = STR; break;
        default:
            usage();
            exit(1);
        }
    }

    if (optind + 2 != argc) {
        usage();
        exit(1);
    }

    devparams = argv[optind];
    key = argv[optind + 1];

    switch (mode) {
    case BDF: {
        u_int16_t bdf;

        if (devparam_bdf(devparams, key, &bdf) >= 0) {
            printf("%s\n", bdf_to_str(bdf));
        } else {
            puts("<not found>");
        }
        break;
    }
    case MAC: {
        mac_t m;

        if (devparam_mac(devparams, key, &m) >= 0) {
            printf("%s\n", mac_to_str(&m));
        } else {
            puts("<not found>");
        }
        break;
    }
    case INT: {
        int i;

        if (devparam_int(devparams, key, &i) >= 0) {
            printf("%d\n", i);
        } else {
            puts("<not found>");
        }
        break;
    }
    case U64: {
        u_int64_t i;

        if (devparam_u64(devparams, key, &i) >= 0) {
            printf("0x%"PRIx64"\n", i);
        } else {
            puts("<not found>");
        }
        break;
    }
    case STR: {
        char str[32];

        if (devparam_str(devparams, key, str, sizeof(str)) >= 0) {
            puts(str);
        } else {
            puts("<not found>");
        }
        break;
    }
    default:
        break;
    }

    exit(0);
}

int 
sim_server_read_clientmem(const u_int64_t addr, 
                          void *buf,
                          const size_t len)
{
    return 0;
}
