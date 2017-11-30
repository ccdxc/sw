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
            "Usage: palmem [-P <map_pa>][-S <mapsize>]"
            "[-o <offset>][-s <size>]\n"
            "-P <map_pa>        pal phys addr\n"
            "-S <mapsize>       pal region size\n"
            "-o <offset>        region offset to read\n"
            "-s <size>          region size to read\n");
}

int
main(int argc, char *argv[])
{
    u_int64_t pa, size;
    u_int64_t offset, sz, endoffset;
    int opt;
    u_int8_t *va;
    char buf[80];

    pa = 0;
    size = 16;
    offset = 0;
    sz = size;
    while ((opt = getopt(argc, argv, "P:S:o:s:")) != -1) {
        switch (opt) {
        case 'P':
            pa = strtoull(optarg, NULL, 0);
            break;
        case 'S':
            size = strtoull(optarg, NULL, 0);
            break;
        case 'o':
            offset = strtoull(optarg, NULL, 0);
            break;
        case 's':
            sz = strtoul(optarg, NULL, 0);
            break;
        default:
            usage();
            exit(1);
        }
    }

    if (offset + sz > size) {
        fprintf(stderr, "palmem: offset out of range\n");
        exit(1);
    }

    va = pal_mem_map(pa, size);
    if (va == NULL) {
        fprintf(stderr, "palmem: pal_mem_map failed\n");
        exit(1);
    }

    endoffset = offset + sz;
    for (; offset < endoffset; offset += 16) {
        const u_int16_t len = MIN(16, endoffset - offset);
        hex_format(buf, sizeof(buf), &va[offset], len);
        printf("%08"PRIx64": %s\n", pa + offset, buf);
    }

    pal_mem_unmap(va);
    exit(0);
}
