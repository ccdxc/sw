/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <sys/time.h>

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "platform/pal/include/pal.h"

void sigintHandler(int sig_num) 
{ 
    printf("\n Stopping the watchdog\n"); 
    pal_watchdog_stop();
    _exit(EXIT_SUCCESS);
}

static void
usage(void)
{
    fprintf(stderr, "pal_watchdog (sw | hw | none) (stop)\n");
    exit(1);
}

int
main(int argc, char *argv[])
{

    pal_watchdog_action_t wdt = NO_WDT;

    if (argc < 2) {
        usage();
        return -1;
    }

    if (argc > 2 && strcmp(argv[2], "stop") == 0) {
        signal(SIGINT, sigintHandler);
    }

    if (strcmp(argv[1], "sw") == 0) {
        wdt = PANIC_WDT;
    } else if (strcmp(argv[1], "hw") == 0) {
        wdt = RESET_WDT;
    } else if (strcmp(argv[1], "none") == 0) {
        wdt = NO_WDT;
    } else {
        usage();
        return -1;
    }

    if (pal_watchdog_init(wdt) < 0) {
        printf("Unable to initialize the wdt");
        return -1;
    }

    while (1) {
        pal_watchdog_kick();
        sleep(1);
    }
    return 0;
}
