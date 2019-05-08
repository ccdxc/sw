/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef _PAL_WATCHDOG_H_
#define _PAL_WATCHDOG_H_

/*
 *
 */
typedef enum {
    RESET_WDT = 0,
    PANIC_WDT,
    NO_WDT
} pal_watchdog_action_t;

int pal_watchdog_init(pal_watchdog_action_t wdttype);
int pal_watchdog_kick();
int pal_watchdog_stop();
#endif

