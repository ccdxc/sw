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

#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "third-party/asic/capri/model/cap_top/csr_defines/cap_ms_c_hdr.h"

#define MS_CFG_WDT CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_CFG_WDT_BYTE_ADDRESS
#define CAP_MS_CSR_CFG_WDT_PAUSE_LSB 8
#define HW_WATCHDOG_DEVICE "/dev/watchdog0"
#define SW_WATCHDOG_DEVICE "/dev/watchdog1"
#define HW_WATCHDOG_TIMEOUT 5
#define SW_WATCHDOG_TIMEOUT 60

#ifdef __aarch64__
static int wdtfd = -1;
static pal_watchdog_action_t gwdt = NO_WDT;
#endif

int pal_watchdog_init(pal_watchdog_action_t wdttype) {
#ifdef __aarch64__
    int fd = -1;
    uint32_t data;
    int timeout;

    if (wdttype == NO_WDT) {
        pal_mem_trace("No WDT requested\n");
        return 0;
    }

    if (wdttype == RESET_WDT) {
        //enable the WDT incase it is paused
        data = pal_reg_rd32(MS_CFG_WDT);
        pal_mem_trace("Current value of the ms_cfg_reset reg is %d\n", data);
        //Clear bit 8. Pause bit for HW WDT.
        //Assume the WDT is always paused.
        data = data & ~(1 << CAP_MS_CSR_CFG_WDT_PAUSE_LSB);
        pal_reg_wr32(MS_CFG_WDT, data);
    }

    //Watchdog is enumerated as follows:
    //watchdog0 - HW watchdog
    //watchdog1 - SW watchdog
    if (wdttype == RESET_WDT) {
        if ((fd = open(HW_WATCHDOG_DEVICE, O_RDWR)) < 0) {
            pal_mem_trace("Failed to open the watchdog device\n");
            return -1;
        }
        timeout = HW_WATCHDOG_TIMEOUT;
        if (ioctl(fd, WDIOC_SETTIMEOUT, &timeout) < 0) {
            pal_mem_trace("unable to set the timeout\n");
            return -1;
        }
    } else if (wdttype == PANIC_WDT) {
        if ((fd = open(SW_WATCHDOG_DEVICE, O_RDWR)) < 0) {
            pal_mem_trace("Failed to open the watchdog device\n");
            return -1;
        }
        timeout = SW_WATCHDOG_TIMEOUT;
        if (ioctl(fd, WDIOC_SETTIMEOUT, &timeout) < 0) {
            pal_mem_trace("unable to set the timeout\n");
            return -1;
        }
    }

    //kick the WDT as soon as we open it
    if (ioctl(fd, WDIOC_KEEPALIVE, 0) < 0) {
        pal_mem_trace("Unable to kick the WDT\n");
        return -1;
    }

    wdtfd = fd;
    gwdt = wdttype;
#endif
    return 0;
}

int pal_watchdog_kick() {
#ifdef __aarch64__
    //no watchdog initialized
    if (gwdt == NO_WDT) {
        return 0;
    }

    //kick the wdt
    if (ioctl(wdtfd, WDIOC_KEEPALIVE, 0) < 0) {
        pal_mem_trace("Unable to kick the WDT\n");
        close(wdtfd);
        wdtfd = -1;
        return -1;
    }
#endif
    return 0;
}

int pal_watchdog_stop() {
#ifdef __aarch64__
    uint32_t data;
    static const char V = 'V';
    //no watchdog initialized
    if (gwdt == NO_WDT) {
        return 0;
    }

    if (gwdt == RESET_WDT) {
        //pause the WDT
        data = pal_reg_rd32(MS_CFG_WDT);
        pal_mem_trace("Current value of the ms_cfg_reset reg is %d\n", data);
        //Clear bit 8. Pause bit for HW WDT.
        data = data | (1 << CAP_MS_CSR_CFG_WDT_PAUSE_LSB);
        pal_reg_wr32(MS_CFG_WDT, data);
    } else if (gwdt == PANIC_WDT) {
        //Close the watchdog properly
        if (write(wdtfd, &V, 1) < 0) {
            pal_mem_trace("Unable to close the WDT\n");
            close(wdtfd);
            wdtfd = -1;
            return -1;
        }
    }
    close(wdtfd);
    wdtfd = -1;
#endif
    return 0;
}
