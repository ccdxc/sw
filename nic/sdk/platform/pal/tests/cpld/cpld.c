/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "platform/pal/include/pal_cpld.h"

static void
dumpregisters()
{
    int i;

    printf("The cpld register are:\n");

    for(i = 0; i < 0x33; i++) {
        printf("\n%02x : %02x", i, cpld_read(i));
    }

    printf("\n0x80 : %02x", cpld_read(0x80));
}

static void
usage(void)
{
    fprintf(stderr, "cpld (-r addr | -w addr data | -sy frequency(0..3) | -sg frequency(0..3) | -disp)\n");
    exit(1);
}

int
main(int argc, char *argv[])
{

    uint8_t addr, data, freq;

    if (strcmp(argv[1], "-r") == 0) {
        if (argc < 3) {
            usage();
            return 1;
        }
        addr = strtoul(argv[2], NULL, 0);
        printf("\nREAD : %x", cpld_read(addr));
    } else if (strcmp(argv[1], "-w") == 0) {
        if (argc < 4) {
            usage();
        }
        addr = strtoul(argv[2], NULL, 0);
        data = strtoul(argv[3], NULL, 0);
        cpld_write(addr, data);
    } else if (strcmp(argv[1], "-sg") == 0) {
        if (argc < 3) {
            usage();
            return 1;
        }
        freq = strtoul(argv[2], NULL, 0);
        if (freq < 0 || freq > 3) {
            printf("The frequency is a value between 0 and 3\n");
            return 1;
        }
        pal_system_set_led(LED_COLOR_GREEN, freq);
    } else if (strcmp(argv[1], "-sy") == 0) {
        freq = strtoul(argv[2], NULL, 0);
        if (freq < 0 || freq > 3) {
            printf("The frequency is a value between 0 and 3\n");
            return 1;
        }
        pal_system_set_led(LED_COLOR_YELLOW, freq);
    } else if (strcmp(argv[1], "-disp") == 0) {
        dumpregisters();
    } else {
        usage();
    }

    return 0;
}
