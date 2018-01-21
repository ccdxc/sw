/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/param.h>

#include "misc.h"
#include "pal.h"

static void
usage(void)
{
    fprintf(stderr,
            "Usage: palmem [-s <size>] <addr>\n"
            "-s <size>          region size to read (default=16)\n");
}

int
main(int argc, char *argv[])
{
    u_int64_t pa, size, offset;
    int opt;
    u_int8_t *va;
    char buf[80];

    pa = -1;
    size = 16;
    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
        case 's':
            size = strtoull(optarg, NULL, 0);
            break;
        default:
            usage();
            exit(1);
        }
    }

    if (optind >= argc) {
        usage();
        exit(1);
    }

    pa = strtoull(argv[optind], NULL, 0);
    if (pa == 0) {
        usage();
        exit(1);
    }

    va = pal_mem_map(pa, size);
    if (va == NULL) {
        fprintf(stderr, "palmem: pal_mem_map failed\n");
        exit(1);
    }

    for (offset = 0; offset < size; offset += 16) {
        const u_int16_t len = MIN(16, size - offset);
        hex_format(buf, sizeof(buf), &va[offset], len);
        printf("0x%08"PRIx64": %s\n", pa + offset, buf);
    }

    pal_mem_unmap(va);
    exit(0);
}
