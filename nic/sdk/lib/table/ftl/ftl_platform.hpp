//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_PLATFORM_HPP__
#define __FTL_PLATFORM_HPP__

#include "lib/utils/utils.hpp"

namespace sdk {
namespace table {
namespace internal {

sdk_ret_t memrd(Apictx *ctx);
sdk_ret_t memwr(Apictx *ctx);
sdk_ret_t memclr(uint64_t memva, uint64_t mempa, uint32_t num_entries,
                 uint32_t entry_size);

static inline void
ftlv6_memcpy(void* d, void* s) {
    auto dst = (uint64_t*)d;
    auto src = (uint64_t*)s;
    sdk::lib::copy32(dst, src);
    sdk::lib::copy32(dst+4, src+4);
}

static inline void
ftlv4_memcpy(void* d, void* s) {
    auto dst = (uint64_t*)d;
    auto src = (uint64_t*)s;
    sdk::lib::copy32(dst, src);
}

static inline void
ftl_memcpy(void *d, void *s, uint32_t entry_size)
{
    if (entry_size == 32) {
        ftlv4_memcpy(d, s);
    } else {
        ftlv6_memcpy(d, s);
    }
}

} // namespace internal
} // namespace table
} // namespace sdk

#endif  // __FTL_PLATFORM_HPP__
