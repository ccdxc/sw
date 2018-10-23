/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cpld.h"

static void
usage(void)
{
    fprintf(stderr, "cpld (-r addr | -w addr data)\n");
    exit(1);
}

int
main(int argc, char *argv[])
{

    uint8_t addr, data;

    if (argc < 3) {
        return 1;
    }
    addr = strtoul(argv[2], NULL, 0);
    if (strcmp(argv[1], "-r") == 0) {
        printf("\nREAD : %x", cpld_read(addr));
    } else if (strcmp(argv[1], "-w") == 0) {
        if (argc < 4) {
            usage();
        }
        data = strtoul(argv[3], NULL, 0);
        cpld_write(addr, data);
    } else {
        usage();
    }

    return 0;
}
