
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdint.h>
#include "cap_ms_c_hdr.h"
#include "defs.h"
#include "iomem.h"

int
is_secure_boot_enabled(void)
{
    uint64_t addr = MS_(STA_PK_EFUSE_VAL) + 4 * (EFUSE_SECURE_BOOT_BIT / 32);
    int shift = EFUSE_SECURE_BOOT_BIT % 32;
    uint32_t v = readreg(addr);
    return (((v >> shift) & EFUSE_SECURE_BOOT_MASK) != 0);
}
