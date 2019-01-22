// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_UTILS_HPP__
#define __SDK_UTILS_HPP__

#include "nic/sdk/lib/catalog/catalog.hpp"

namespace sdk {
namespace lib {

#define NUM_BITS_IN_BYTE  8

static inline uint16_t
count_bits_set (uint64_t mask)
{
    uint16_t count = 0;
    while (mask != 0) {
        count++;
        mask = mask & (mask - 1);
    }
    return count;
}

static inline int
ffs_msb (uint64_t mask) {
    if (mask == 0) {
        return 0;
    }

    return (sizeof(mask) * NUM_BITS_IN_BYTE) - __builtin_clzl(mask);
}

static inline void
memrev (uint8_t *dst, uint8_t *src, size_t size)
{
    for (src = src + (size - 1); size > 0; size--) {
        *dst++ = *src--;
    }
}

}    // namespace lib
}    // namespace sdk

#endif // __SDK_UTILS_HPP__

