/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

/*
 * Sizes are encoded in the PRT entry using this format.
 *
 * If bit10 == 0, then
 *
 *     10 9          0
 *     +-+-----+-----+
 *     |0|   sizedw  |
 *     +-+-----+-----+
 *
 *     effective size = sizedw;
 *
 * If bit11 == 1, then
 *
 *     10 9   5 4   0
 *     +-+-----+-----+
 *     |1| rv  | ru  |
 *     +-+-----+-----+
 *
 * ru is a power-of-2 "resource units", and
 * rv is the "resource value" multiplier.
 * so effective size is:
 *
 *     effective size = rv << ru;
 *
 * All sizes here are 4-byte "dwords".
 */
static u_int32_t
prt_size_encode(const u_int64_t size)
{
    const u_int64_t sizedw = size >> 2;
    u_int32_t rv, ru;

    /* multiple of dword */
    assert((size & 0x3) == 0);

    /*
     * Any size value < (1 << 10) is encoded
     * as simple value;
     */
    if (sizedw < (1 << 10)) {
        return sizedw;
    }

    /*
     * Size >= (1 << 10) are encoded such that
     * ru is a power-of-2 "resource units", and
     * rv is the "resource value" multiplier.
     * so effective size is:
     *
     *     effective size = rv << ru;
     */
    ru = MIN(ffsll(sizedw) - 1, 31);
    rv = sizedw >> ru;

    assert(rv < (1 << 5));
    return (1 << 10) | (rv << 5) | ru;
}

static u_int64_t
prt_size_decode(const u_int32_t size_enc)
{
    u_int64_t sizedw;

    if (size_enc & (1 << 10)) {
        const u_int64_t rv = (size_enc >> 5) & 0x1f;
        const u_int64_t ru = (size_enc     ) & 0x1f;

        sizedw = rv << ru;
    } else {
        sizedw = size_enc;
    }
    return sizedw << 2;
}

int
main(int argc, char *argv[])
{
    int i;

    for (i = 1; i < argc; i++) {
        const u_int64_t size = strtoull(argv[i], NULL, 0);
        const u_int32_t size_enc = prt_size_encode(size);
        const u_int64_t size_lim = prt_size_decode(size_enc);

        printf("size 0x%"PRIx64" enc 0x%x lim 0x%"PRIx64"\n",
               size, size_enc, size_lim);
    }
    exit(0);
}
