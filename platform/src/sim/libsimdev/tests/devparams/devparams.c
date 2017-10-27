/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "src/lib/misc/include/bdf.h"
#include "src/lib/misc/include/maclib.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

enum { BDF, MAC, INT, STR };

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
    while ((opt = getopt(argc, argv, "bmis")) != -1) {
        switch (opt) {
        case 'b': mode = BDF; break;
        case 'm': mode = MAC; break;
        case 'i': mode = INT; break;
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
    case STR: {
        char value[32];

        if (devparam_value(devparams, key, value, sizeof(value)) >= 0) {
            puts(value);
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
