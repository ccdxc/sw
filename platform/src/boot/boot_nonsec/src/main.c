
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdint.h>
#include "dtls.h"
#include "defs.h"
#include "iomem.h"

void
main(void)
{
    /*
     * This image is not valid if the secure boot eFuse has been blown.
     */
    if (is_secure_boot_enabled()) {
        panic("secure boot not supported");
    }

    /*
     * Tell eSecure that we have booted.
     */
    esec_mark_boot_success();

    /*
     * Continue at the flash boot address
     */
    ((void (*)(void))ADDR_FLASH_BOOT)();
    panic("returned from flash boot");
}
