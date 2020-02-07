/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include "pal.h"

int pal_watchdog_init(pal_watchdog_action_t wdttype) {
    return 0;
}

int pal_watchdog_kick() {
    return 0;
}

int pal_watchdog_stop() {
    return 0;
}
