
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "defs.h"
#include "cap_ms_c_hdr.h"
#include "iomem.h"
#include "bsm.h"
#include "version.h"

static bsm_t bsm;

static const char *fwid_to_part[4] = {
    [FW_MAIN_A] = "uboota",
    [FW_MAIN_B] = "ubootb",
    [FW_GOLD]   = "golduboot",
    [FW_DIAG]   = "golduboot",
};

static void
boot_uboot_image(int fwid)
{
    intptr_t image_addr;
    uint32_t part_size;

    printf("Boot fwid %d", fwid);

    if (board_get_part(fwid_to_part[fwid], &image_addr, &part_size) < 0) {
        puts(" @ NO PART");
        logf("boot0: no partition for fwid %d\n", fwid);
        return;
    }

    printf(" @ 0x%08lx...", image_addr);

    /*
     * Verify the integrity of the selected u-boot image.
     * If the image is not valid then return to the caller.
     * Otherwise continue, and this function will not return.
     */
    if (!is_uboot_valid(image_addr, part_size)) {
        puts(" INVALID");
        logf("boot0: u-boot@0x%08lx invalid\n", image_addr);
        return;
    }
    puts(" OK");

    /*
     * Start the watchdog to catch hang-ups.
     */
    if (bsm.wdt) {
        wdt_enable_chip_reset(BSM_WDT);
        wdt_start(BSM_WDT, 0xf); /* ~5s on Capri with 416MHz AHB */
    }

    /*
     * Log if this is a retry.
     */
    if (bsm.attempt > 0) {
        logf("boot0: try track=%u, attempt=%u, fwid=%u\n",
                bsm.track, bsm.attempt, bsm.fwid);
    }

    /*
     * Jump to the selected u-boot.  Should not return.
     */
    bsm.stage = BSM_STG_UBOOT;
    bsm_save(&bsm);
    uart_wait_idle();
    ((void (*)(void))image_addr)();

    /*
     * The selected u-boot should never return.  If it does then we
     * cannot trust any processor state, so issue an immediate WDT reset
     * to restart.
     */
    wdt_sys_reset();
}

/*
 * Select the boot track.
 * If the Force Golden Firmware GPIO is asserted, then we boot FW_GOLD,
 * otherwise we follow the firmware selector.
 */
static int
select_track(void)
{
    if (gpio_bit(GPIO_PIN_FORCE_GOLDEN)) {
        return FW_GOLD;
    } else {
        return get_pri_fw();
    }
}

static int
select_image(void)
{
    const bsm_fwid_map_t *fwid_map = board_bsm_fwid_map();

    bsm_load(&bsm);
    if (!bsm.running) {
        // New boot
        bsm.wdt = board_bsm_wdt_disable() ? 0 : 1;
        bsm.attempt = 0;
        bsm.track = select_track();
    } else {
        // Restart, new attempt 
        logf("boot0: restart from stage=%u, track=%u, attempt=%u, fwid=%u\n",
                bsm.stage, bsm.track, bsm.attempt, bsm.fwid);
        if (bsm.attempt == BSM_ATTEMPT_MASK) {
            wdt_pause(BSM_WDT, 1);
            panic("No boot path found");
        }
        ++bsm.attempt;
    }

    bsm.autoboot = 1;
    bsm.running = 1;
    bsm.stage = BSM_STG_BOOT0;
    bsm.fwid = fwid_map->map[bsm.track][bsm.attempt];
    bsm_save(&bsm);

    return bsm.fwid;
}

/*
 * The program can start due to two events:
 * 1. Reset (either at power-on or due to a watchdog timer)
 * 2. Jump (a restart jump from u-boot)
 */
int
main(void)
{
    int id;

    /*
     * Pause the BSM WDT in case it's still running.
     */
    wdt_pause(BSM_WDT, 1);

    /*
     * Initialize I/O.
     */
    uart_init();
    qspi_init();
    gpio_init();

    printf("Boot0 v%d, Id 0x%02x\n", BOOT0_VERSION, get_cpld_id());

    /*
     * Loop around trying to boot.
     */
    for (;;) {
        id = select_image();
        boot_uboot_image(id);
    }

    return 0;
}
