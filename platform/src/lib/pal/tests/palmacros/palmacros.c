/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include "platform/src/lib/pal/src/mm_int.h"

int main() {
    u_int64_t top = 0;
    u_int64_t size = 0;
    SETREGIONSIZE(&top, 0xBEEF00);
    SETCOHERENT(&top);
    SETCACHEABLE(&top);
    SETEXCL(&top);
    size = GETREGIONSIZE(&top);
    assert(size == 0xbee000);
    assert(ISCACHEABLE(&top));
    assert(ISCOHERENT(&top));
    assert(ISEXCL(&top));

    top = 0;
    SETREGIONSIZE(&top, 0x1000);
    SETALLOCATED(&top);
    SETSHAREABLE(&top);
    size = GETREGIONSIZE(&top);
    assert(size == 0x1000);
    assert(ISALLOCATED(&top));
    assert(ISSHAREABLE(&top));

    printf("\nAll tests passed.");

    return 0;
}
