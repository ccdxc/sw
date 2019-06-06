//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
namespace sdk {
namespace table {
namespace FTL_MAKE_AFTYPE(internal) {

sdk_ret_t memrd(FTL_MAKE_AFTYPE(apictx) *ctx);
sdk_ret_t memwr(FTL_MAKE_AFTYPE(apictx) *ctx);

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

#define ftlv6_swizzle(_e) _swizzle64((uint8_t*)(_e))
#define ftlv4_swizzle(_e) _swizzle32((uint8_t*)(_e))

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
FTL_MAKE_AFTYPE(swizzle)(FTL_MAKE_AFTYPE(entry_t)* e) {
    _swizzle((uint8_t*)e, sizeof(*e));
}

#endif // USE_ARM64_SIMD

static inline void
_copy32(uint64_t *dst, const uint64_t *src) {
    auto a = src[0];
    auto b = src[1];
    auto c = src[2];
    auto d = src[3];
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    dst[3] = d;
}

static inline void
ftlv6_memcpy(void* d, void* s) {
    auto dst = (uint64_t*)d;
    auto src = (uint64_t*)s;
    _copy32(dst, src);
    _copy32(dst+4, src+4);
}

static inline void
ftlv4_memcpy(void* d, void* s) {
    auto dst = (uint64_t*)d;
    auto src = (uint64_t*)s;
    _copy32(dst, src);
}

} // namespace FTL_MAKE_AFTYPE(internal)
} // namespace table
} // namespace sdk
