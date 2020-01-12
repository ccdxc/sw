
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "defs.h"

/*
 * Verify the integrity of a u-boot image by checking its CRC32.
 * All A/B U-Boot images conform to the following specification:
 *      - They begin with a struct uboot_header as described in defs.h.
 *      - Their size will be a multiple of 8 bytes.
 */

static inline uint32_t
crc32_acc(uint32_t sum, uint64_t data)
{
    asm volatile("crc32x %w[c], %w[c], %x[v]" : [c]"+r"(sum) : [v]"r"(data));
    return sum;
}

static uint32_t
crc32_image(uint64_t image_addr, uint32_t nbytes)
{
    const uint64_t *rp = (uint64_t *)image_addr;
    const uint64_t *ep = (uint64_t *)(image_addr + nbytes);
    uint32_t sum = ~0U;
                                                //     loword      hiword
    sum = crc32_acc(sum, *rp++);                //  0: inst        uboot_magic
    sum = crc32_acc(sum, *rp++);                //  8: uboot_size  boot0_magic
    sum = crc32_acc(sum, *rp++ & 0xffffffff);   // 10: crc32_magic crc32
    while (rp < ep) {
        sum = crc32_acc(sum, *rp++);
    }
    return ~sum;
}

int
is_uboot_valid(intptr_t image_addr, uint32_t part_size)
{
    const struct uboot_header *uh = (struct uboot_header *)image_addr;
    uint32_t usize;

    if (uh->size_magic != UBOOT_SIZE_MAGIC ||
        uh->crc_magic != UBOOT_CRC32_MAGIC) {
        return 0;
    }
    usize = uh->size;
    if ((usize & 0x7) || (usize > part_size)) {
        return 0;
    }
    return (uh->crc == crc32_image(image_addr, usize));
}
