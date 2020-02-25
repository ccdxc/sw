// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __SDK_UTILS_HPP__
#define __SDK_UTILS_HPP__

#include "lib/catalog/catalog.hpp"

#ifdef USE_ARM64_SIMD
#include <arm_neon.h>
#endif

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

#ifndef SIM
#define CRC32X(crc, value) __asm__("crc32x %w[c], %w[c], %x[v]":[c]"+r"(crc):[v]"r"(value))
#define RBITX(value) __asm__("rbit %x0, %x1": "=r"(value) : "r"(value))
#define RBITW(value) __asm__("rbit %w0, %w1": "=r"(value) : "r"(value))
#define REVX(value) __asm__("rev %x0, %x1": "=r"(value) : "r"(value))
static inline uint32_t
crc32_aarch64(const uint64_t *p)
{
    uint32_t crc = 0;
    for (auto i = 0; i < 8; i++) {
        auto v = p[i];
        RBITX(v);
        REVX(v);
        CRC32X(crc, v);
    }
    RBITW(crc);
    return crc;
}
#endif

#ifdef USE_ARM64_SIMD
#define SWAP_PAIR_8B(input, offset_a, offset_b, scratch_a, scratch_b)       \
    scratch_a   = vld1_u8(input + offset_a);                                \
    scratch_b   = vld1_u8(input + offset_b);                                \
    scratch_a = vrev64_u8(scratch_a);                                       \
    scratch_b = vrev64_u8(scratch_b);                                       \
    vst1_u8(input + offset_a, scratch_b);                                   \
    vst1_u8(input + offset_b, scratch_a);                                   \

static inline void 
_swizzle64(uint8_t *entry)
{
    uint8x8_t   a, b;
    SWAP_PAIR_8B(entry, 0, 56, a, b);
    SWAP_PAIR_8B(entry, 8, 48, a, b);
    SWAP_PAIR_8B(entry, 16, 40, a, b);
    SWAP_PAIR_8B(entry, 24, 32, a, b);
    return;
}

static inline void 
_swizzle32(uint8_t *entry)
{
    uint8x8_t   a, b;
    SWAP_PAIR_8B(entry, 0, 24, a, b);
    SWAP_PAIR_8B(entry, 8, 16, a, b);
    return;
}

static inline void
swizzle(void *e, uint32_t entry_size) {
    if (entry_size == 32) {
        _swizzle32((uint8_t *)e);
    } else {
        _swizzle64((uint8_t *)e);
    }
}

#else

static inline void
_swizzle(uint8_t *entry, uint8_t size) {
    // swap bytes. hash key is always 64 bytes.
    for(auto i = 0; i < size/2; ++i) {
        uint8_t b = entry[i];
        entry[i] = entry[size - 1 - i];
        entry[size - 1 - i] = b;
    }
    return;
}

static inline void
swizzle(void *e, uint32_t entry_size) {
    _swizzle((uint8_t*)e, entry_size);
}

#endif // USE_ARM64_SIMD

static inline void
copy32(uint64_t *dst, const uint64_t *src) {
    auto a = src[0];
    auto b = src[1];
    auto c = src[2];
    auto d = src[3];
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    dst[3] = d;
}

}    // namespace lib
}    // namespace sdk

#endif // __SDK_UTILS_HPP__

